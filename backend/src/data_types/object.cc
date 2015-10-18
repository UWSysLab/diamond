
#include "storage/cloud.h"
#include "storage/stalestorage.h"
#include "includes/data_types.h"
#include "lib/assert.h"
#include "lib/message.h"
#include "lib/timestamp.h"
#include <unordered_map>
#include <map>
#include <inttypes.h>
#include <set>


namespace diamond {

using namespace std;

// Obsolete: Used for the release consistency 
static std::set<DObject*> rcRS;
static std::set<DObject*> rcWS;
static enum DConsistency globalConsistency = SEQUENTIAL_CONSISTENCY;


// static std::set<int> transationsTID; // set with the TIDs of the running transactions
// static std::map<int, std::set<string> > transactionsRS; // map with the RS for each transaction
// static std::map<int, std::set<string> > transactionsWS; // map with the WS for each transaction
// static std::map<int, std::map<string, string > > transactionsLocal; // map with the local values of the objects for each tx

// Keeps all the state for the transactions in progress (per thread)
static std::map<long, TransactionState> transactionStates;

pthread_mutex_t  transactionMutex = PTHREAD_MUTEX_INITIALIZER; // Protects the global transaction structures

// Used by diamond_profile.h macros
pthread_mutex_t  profileMutex = PTHREAD_MUTEX_INITIALIZER;
long profileEnterTs[MAX_THREAD_ID];

// Used to simulate that the network is offline
bool networkConnectivity = false;


Cloud* cloudstore = NULL;
Stalestorage stalestorage;


void DiamondInit(const std::string &server) {
    cloudstore = Cloud::Instance(server);
}

void DiamondInit() {
    DiamondInit("localhost");
}


// XXX: Add an assert so that we don't map inside a transaction?
int
DObject::Map(DObject &addr, const string &key)
{
    PROFILE_ENTER("MAP");
    pthread_mutex_lock(&addr._objectMutex);

    addr._key = key;
   
    if (!cloudstore->IsConnected()) {
        Panic("Cannot map objects before connecting to backing store server");
    }
    
    int res = addr.Pull();

    pthread_mutex_unlock(&addr._objectMutex);
    PROFILE_EXIT("MAP");
    return res;
}

// XXX: Ensure return codes are correct

int
DObject::PullAlways(){
    string value;
    LOG_RC("PullAlways()"); 

    int ret;
    
    // Disable stale reads for now
    // ret = stalestorage.Read(_key,value);
    // if(ret == ERR_NOTFOUND){

        // If it's not on the stalestore

        ret = cloudstore->Read(_key, value);
        if (ret != ERR_EMPTY && ret != ERR_OK) {
            return ret;
        }

        if (ret == ERR_EMPTY) {
            value = "";
        }
    // }

    Deserialize(value);
    return 0;
}

// BATCHING VERSION
int
DObject::PullAlwaysWatch(){
    string value;
    LOG_RC("PullAlwaysWatch()"); 

    int ret;
    
    ret = stalestorage.Read(_key,value);
    if(ret == ERR_NOTFOUND){
        TransactionState *ts  = GetTransactionState();
        // If it's not on the stalestore

        ret = cloudstore->WatchRead(_key, value);
        if (ret != ERR_EMPTY && ret != ERR_OK) {
            return ret;
        }
   
        ts->cloudReadCount++;

        if (ret == ERR_EMPTY) {
            value = "";
        }
    }

    Deserialize(value);
    return 0;
}


// XXX: need to use locks in the readers/writers

int
DObject::Pull(){
    string value;

    // XXX: use the tx lock

    if(IsTransactionInProgress()){
        TransactionState *ts  = GetTransactionState();
        std::set<string>* txRS = &ts->rs;
        std::set<string>* txWS = &ts->ws;
        std::map<string, string >* locals = &ts->localView;

        if((txRS->find(this->GetKey()) != txRS->end()) || (txWS->find(this->GetKey()) != txWS->end())){
            // Don't pull if the object is already in our WS or our RS

            txRS->insert(this->GetKey());

            // Use our local TX value
            string value;
            value = (*locals)[this->GetKey()];
            Deserialize(value);
            return 0;
        }
        txRS->insert(this->GetKey());
        
        // WITHOUT BATCHING
        //cloudstore->Watch(this->GetKey());
        //int res = PullAlways();
        // WITHOUT BATCHING


        // WITH BATCHING
        int res = PullAlwaysWatch();

        // Add new value to our local TX view
        string value;
        value = Serialize();
        (*locals)[this->GetKey()]=value;

        return res;

    }else if(globalConsistency == SEQUENTIAL_CONSISTENCY){
        return PullAlways();

    } else {
        // Release consistency

        if((rcRS.find(this) != rcRS.end()) || (rcWS.find(this) != rcWS.end())){
            // Don't do anything if object is in the WS or RS
            LOG_RC("Pull(): Object in rcRS or rcWS -> Returning local copy");
            return 0;
        }
        LOG_RC("Pull(): Object neither in rcRS nor in rcWS -> Calling PullAlways()");
        LOG_RC("Pull(): Adding object to rcRS"); 
        rcRS.insert(this);

        return PullAlways();
    }
}

int
DObject::PushAlways(){
    string value;
    LOG_RC("PushAlways()"); 

    value = Serialize();

    int ret = cloudstore->Write(_key, value);
    if (ret != ERR_OK) {
        return ret;
    }
    return 0;
}


int
DObject::Push(){

    if(IsTransactionInProgress()){
        TransactionState *ts = GetTransactionState();
        // Add object to our WS
        std::set<string>* txWS = &ts->ws;
        txWS->insert(this->GetKey());

        cloudstore->Watch(this->GetKey());  // PF: Watch probably can be avoided on writes; if read-after-writes deals with it properly

        // Add new value to our local TX view
        string value;
        value = Serialize();
        std::map<string, string >* locals = &ts->localView;
        (*locals)[this->GetKey()]=value;

        // Do not push to storage yet, wait for commit
        return 0; 

    }else if(globalConsistency == SEQUENTIAL_CONSISTENCY){
        return PushAlways();

    }else{
        LOG_RC("Push(): Adding object to rcWS");
        rcWS.insert(this);
        return 0; 
    }
}

// XXX: Thread-safety: make sure Multi-get is thread-safe
// We're not protecting against situations where the user modifies the parameters concurrently with MultiMap
int
DObject::MultiMap(vector<DObject *> &objects, vector<string> &keys)  {

    if (keys.size() != objects.size()) {
        Panic("Mismatch between number of keys and DObjects");
    }

    vector<string> values;
    int ret = cloudstore->MultiGet(keys, values);
    if (ret != ERR_OK) {
        return ret;
    }

    if (keys.size() != values.size()) {
        Panic("Mismatch between number of keys and values returned by MultiGet");
    }

    for (size_t i = 0; i < keys.size(); i++) {
        pthread_mutex_lock(&objects.at(i)->_objectMutex);

        string currentKey = keys.at(i);
        objects.at(i)->_key = currentKey;
        objects.at(i)->Deserialize(values.at(i));

        pthread_mutex_unlock(&objects.at(i)->_objectMutex);
    }

    return 0;
}


void
DObject::Lock(){
    pthread_mutex_lock(&_objectMutex);
    LockNotProtected();

    if(globalConsistency == RELEASE_CONSISTENCY){
        rcRS.clear();
        // Could also load the value of the current object in the background?
        LOG_RC("Lock(): Clearing rcRS");
    }

    pthread_mutex_unlock(&_objectMutex);
}


void
DObject::LockNotProtected(){
    int res;
    char value[50];
    int max_tries = 1000;
    int delay_ns = 1;
    int max_delay_ns = 1000 * 1000;

    int tid = getThreadID();

    if(_locked == tid){
        Panic("Current thread already holds the lock");
    }

    uint64_t lockid = getTimestamp();
    sprintf(value, "%" PRIu64 "", lockid);

    while(max_tries--){
        res = cloudstore->Write(_key + string("-lock"), string(value), WRITE_IFF_NOT_EXIST, LOCK_DURATION_MS);
        if(res == ERR_OK){
            _locked = tid;
            _lockid = lockid;
            return;
        }else if(res == ERR_NOT_PERFORMED){
            
            // Release the lock during sleep to prevent deadlocks
            pthread_mutex_unlock(&_objectMutex);
            usleep(delay_ns);
            pthread_mutex_lock(&_objectMutex);

            delay_ns = std::min(delay_ns * 2, max_delay_ns);
        }else{
            Panic("NYI");
        }
    }
    Panic("Unable to aquire lock");
}

void
DObject::ContinueLock(){
    pthread_mutex_lock(&_objectMutex);
    if(_locked != getThreadID()){
        Panic("Current thread does not hold the lock");
    }
    Panic("NYI");
    pthread_mutex_unlock(&_objectMutex);
}

void
DObject::Unlock(){

    pthread_mutex_lock(&_objectMutex);
    UnlockNotProtected();

    if(globalConsistency == RELEASE_CONSISTENCY){
        LOG_RC("Unlock(): Pushing everything")
        set<DObject*>::iterator it;
        for (it = rcWS.begin(); it != rcWS.end(); it++) {
            (*it)->PushAlways();
        }
        rcWS.clear();
        LOG_RC("Unlock(): Clearing rcWS")
    }

    pthread_mutex_unlock(&_objectMutex);
}

void
DObject::UnlockNotProtected(){
    // From redlock
    string m_unlockScript = string("if redis.call('get', KEYS[1]) == ARGV[1] then return redis.call('del', KEYS[1]) else return 0 end");
    int res;
    char value[50];

    sprintf(value, "%" PRIu64 "", _lockid);
    res = cloudstore->RunOnServer(m_unlockScript, _key + string("-lock"), string(value));
    assert(res == ERR_OK);

    if(_locked != getThreadID()){
        Panic("Current thread does not hold the lock");
    }
    _locked = 0;
}


void
DObject::Signal(){
    int res;
    string value;
    string empty = "";

    pthread_mutex_lock(&_objectMutex);
    if(_locked != getThreadID()){
        Panic("Current thread does not hold the lock");
    }

    res = cloudstore->Lpop(_key + string("-lock-wait"), value, false);

    assert((res == ERR_OK) || (res == ERR_EMPTY));
    if(res == ERR_OK){
        assert(value != "");
        res = cloudstore->Rpush(_key + string("-lock-wait-") + value, empty);
        assert(res == ERR_OK);
    }

    // If we can assume _key is immutable then we can there is no need to hold the mutex till the end
    pthread_mutex_unlock(&_objectMutex);
}

void
DObject::Broadcast(){
    int res;
    string value;
    string empty = "";

    pthread_mutex_lock(&_objectMutex);
    if(_locked != getThreadID()){
        Panic("Current thread does not hold the lock");
    }
    
    while(1){
        res = cloudstore->Lpop(_key + string("-lock-wait"), value, false);
        if(res != ERR_OK){
            break;
        }
        res = cloudstore->Rpush(_key + string("-lock-wait-") + value, empty);
        assert(res == ERR_OK);
    }

    // If we can assume _key is immutable then we can there is no need to hold the mutex till the end
    pthread_mutex_unlock(&_objectMutex);
}


void
DObject::Wait(){
    int res;
    char lockid[50];
    string value;

    pthread_mutex_lock(&_objectMutex);
    if(_locked != getThreadID()){
        Panic("Current thread does not hold the lock");
    }


    sprintf(lockid, "%" PRIu64 "", _lockid);

    // A. Unlock & Sleep
    res = cloudstore->Rpush(_key + string("-lock-wait"), string(lockid));
    assert(res == ERR_OK);
    UnlockNotProtected();
    pthread_mutex_unlock(&_objectMutex);

    res = cloudstore->Lpop(_key + string("-lock-wait-") + string(lockid), value, true);
    
    pthread_mutex_lock(&_objectMutex);
    assert(res == ERR_OK);


    // B. Reaquire lock
    LockNotProtected();

    pthread_mutex_unlock(&_objectMutex);
}

void
DObject::SetGlobalConsistency(enum DConsistency dc)
{
    LOG_RC("SetGlobalConsistency");
    if((dc == RELEASE_CONSISTENCY) && (globalConsistency == SEQUENTIAL_CONSISTENCY)){
        // Updating from RELEASE_CONSISTENCY to SEQUENTIAL_CONSISTENCY
        set<DObject*>::iterator it;
        for (it = rcWS.begin(); it != rcWS.end(); it++) {
            (*it)->PushAlways();
        }
        rcWS.clear();
    }

    globalConsistency = dc;
}


bool
DObject::IsTransactionInProgress(void)
{
    long threadID = getThreadID();

    auto find = transactionStates.find(threadID);
    if(find != transactionStates.end()){
        return true;
    }else{
        return false;
    }
}

void
DObject::SetTransactionInProgress(bool inProgress)
{
    long threadID = getThreadID();

    if(!IsTransactionInProgress()){
        LOG_TX("change state to \"transaction in progress\"");
        assert(inProgress);
        TransactionState ts;
        transactionStates[threadID] = ts;
    }else{
        LOG_TX("change state to \"transaction not in progress\"");
        assert(!inProgress);
        transactionStates.erase(threadID);
        return;
    }


    TransactionState *ts = GetTransactionState();
    
    ts->cloudReadCount = 0;
    ts->rs.clear();
    ts->ws.clear();
    ts->localView.clear();

//     auto txWS = GetTransactionWS();
//     auto txRS = GetTransactionRS();
//     auto locals = GetTransactionLocals();
// 
//     txWS->clear();
//     txRS->clear();
//     locals->clear();
}

TransactionState*
DObject::GetTransactionState(void){
    long tid = getThreadID();
   
    if(!IsTransactionInProgress()){
        return NULL;
    }

    TransactionState *ts;
    ts = &transactionStates[tid];
    return ts;
}


// DObject::GetTransactionRS(void){
//     long tid = getThreadID();
//     
//     std::set<string> *s;
//     s = &transactionsRS[tid];
//     return s;
// }
// 
// 
// std::set<string>*
// DObject::GetTransactionRS(void){
//     long tid = getThreadID();
//     
//     std::set<string> *s;
//     s = &transactionsRS[tid];
//     return s;
// }
// 
// std::set<string>*
// DObject::GetTransactionWS(void){
//     long tid = getThreadID();
//     
//     std::set<string> *s;
//     s = &transactionsWS[tid];
//     return s;
// }
// 
// std::map<string, string >*
// DObject::GetTransactionLocals(void){
//     long tid = getThreadID();
//     
//     std::map<string, string >* m;
//     m = &transactionsLocal[tid];
//     return m;
// }
// 
// 

// XXX: Ensure that it's ok to call the Begin, Commit, Rollback and Retry concurrently from different threads

void
DObject::TransactionBegin(void)
{
    pthread_mutex_lock(&transactionMutex);

    LOG_TX("TRANSACTION BEGIN");

    SetTransactionInProgress(true);
    stalestorage.ViewBegin();

    pthread_mutex_unlock(&transactionMutex);

    // XXX: Prevent locks from being acquired during a Tx

}




// API notes:
//  - txHandler MUST NOT call any TX function
//  - txHandler just returns the txResult 
//  - txHandler does not need to worry about control-flow issues related to 
//  - txHandler should just use local values
bool
DObject::TransactionExecute(enum txResult (*txHandler)(void*), void * txArg, unsigned int maxAttempts)
{
    unsigned int attempts = 0;
    int committed = 0;
    
    Notice("Executing transaction\n");

    while((attempts < maxAttempts) && (maxAttempts > 0)){
        attempts++;
        TransactionBegin();
        int ret = txHandler(txArg);
        switch(ret){
            case COMMIT:
                committed = TransactionCommit();
                if(committed){
                    Notice("Committed after %d tries\n", attempts);
                    return true;
                }
                // Just continue
                break;
            case ROLLBACK:
                Notice("Rollbacked after %d tries\n", attempts);
                return false; 
                break;
            case RETRY:
                TransactionRetry();
                // Just continue
                break;
        }
    }
    Notice("Give up after %d tries (max retries: %d)\n", attempts, maxAttempts);
    return false;
}


int
DObject::TransactionCommit(void)
{
    pthread_mutex_lock(&transactionMutex);

    LOG_TX_DUMP_RS()
    LOG_TX_DUMP_WS()

    bool consistent = stalestorage.ViewEnd();
    if(!consistent){
        LOG_TX("TRANSACTION COMMIT -> aborted (inconsistent reads)");
        SetTransactionInProgress(false);
        pthread_mutex_unlock(&transactionMutex);
        return false;
    }

// WITH BATCHING
    std::map<string, string> keyValuesWS;
    TransactionState *ts = GetTransactionState();

    std::map<string, string >* locals = &ts->localView;
    std::set<string>* txWS = &ts->ws;
    auto it = txWS->begin();
    for (; it != txWS->end(); it++) {
        // Push our local Tx values for all objects in our WS
        string key = *it;
        string value = (*locals)[key];
        keyValuesWS[key] = value;
    }
    int res;
    if((keyValuesWS.size() > 0) || (ts->cloudReadCount > 1)){
        // If one or more writes were made -> send to the cloud
        // If two or more reads -> send to the cloud for consistency check (and unwatch them)
        res = cloudstore->MultiWriteExec(keyValuesWS);
    }else if(ts->cloudReadCount == 1){
        // If one read was made -> unwatch it
        res = cloudstore->Unwatch();
    }else{
        // If there were no writes and no read hit the cloud -> do nothing
        res = ERR_OK;
    }




// <WITHOUT BATCHING>
//     // Begin storage transaction 
//     // (the Watch commands executed earlier detect the conflicts)
//     int res = cloudstore->Multi();
//     assert(res == ERR_OK);
// 
//     string value;
//     std::map<string, string >* locals = GetTransactionLocals();
//     std::set<string>* txWS = GetTransactionWS();
//     auto it = txWS->begin();
//     for (; it != txWS->end(); it++) {
//         // Push our local Tx values for all objects in our WS
//         string key = *it;
//         value = (*locals)[key];
// 
//         int ret = cloudstore->Write(key, value);
//         assert(ret == ERR_OK); 
// 
//     }
// 
//     // Try to commit the storage transaction
//     res = cloudstore->Exec();
// </WITHOUT BATCHING>
 


    if(res == ERR_EMPTY){
        // XXX: Need to revert the changes to the WS
        LOG_TX("TRANSACTION COMMIT -> aborted");
        SetTransactionInProgress(false);
        pthread_mutex_unlock(&transactionMutex);
        return false;
    }else{
        std::map<string, string> keyValuesRS;
        TransactionState *ts = GetTransactionState();

        std::map<string, string >* locals = &ts->localView;
        std::set<string>* txRS = &ts->rs;
        auto it = txRS->begin();
        for (; it != txRS->end(); it++) {
            // Push our local Tx values for all objects in our RS
            string key = *it;
            string value = (*locals)[key];
            keyValuesRS[key] = value;
        }     
        stalestorage.ViewAdd(keyValuesRS, keyValuesWS);
        //stalestorage.DebugDump();

        LOG_TX("TRANSACTION COMMIT -> committed");
        SetTransactionInProgress(false);
        pthread_mutex_unlock(&transactionMutex);
        return true;
    }
}

void
DObject::TransactionRollback(void)
{

    pthread_mutex_lock(&transactionMutex);

    LOG_TX("TRANSACTION ROLLBACK");

    int res = cloudstore->Unwatch(); 
    assert(res == ERR_OK); // Is this assert really necessary?

    SetTransactionInProgress(false);

    pthread_mutex_unlock(&transactionMutex);

}

void
DObject::TransactionRetry(void)
{
    // Implement with pooling for now


    LOG_TX("TRANSACTION RETRY");

    pthread_mutex_lock(&transactionMutex);

    TransactionState *ts = GetTransactionState();
    std::set<string>* txRS = &ts->rs;
    std::map<string, string >* locals = &ts->localView;

    int res = cloudstore->Unwatch(); 
    assert(res == ERR_OK); // Is this assert really necessary?

    pthread_mutex_unlock(&transactionMutex);
    
    int ret = cloudstore->Wait(*txRS, *locals);
    assert(ret == ERR_OK); 


    pthread_mutex_lock(&transactionMutex);
    SetTransactionInProgress(false);
    pthread_mutex_unlock(&transactionMutex);

}

// used to simulate an offline situation
void
DObject::SetNetworkConnectivity(bool connectivity)
{
    networkConnectivity = connectivity;

    if(networkConnectivity){
        Notice("Network connectivity: off\n");
    }else{
        Notice("Network connectivity: on\n");
    }
}



void 
DObject::SetGlobalStaleness(bool enable)
{
    stalestorage.SetStaleness(enable);
}

void
DObject::SetGlobalMaxStaleness(long maxStalenessMs)
{
    stalestorage.SetMaxStaleness(maxStalenessMs);
}

std::string
DObject::GetKey(){
    return _key;
}

void
DObject::DebugSleep(long seconds){
    Notice("Sleeping for %ld seconds\n", seconds);
    sleep(seconds);
}


} // namespace diamond


