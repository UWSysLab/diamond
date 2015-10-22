
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

// Keeps all the state for the transactions in progress (per thread)
static std::map<long, TransactionState> transactionStates;

pthread_mutex_t  transactionMutex = PTHREAD_MUTEX_INITIALIZER; // Protects the global transaction structures

// Used by diamond_profile.h macros
pthread_mutex_t  profileMutex = PTHREAD_MUTEX_INITIALIZER;
long profileEnterTs[MAX_THREAD_ID];

// Used to simulate that the network is offline
bool networkConnectivity = false;

// Prefetch
static bool globalPrefetchEnabled = true;
static set<set<string>> globalPrefetchSets; //XXX: Optimize the lookup? and the insertion?

Cloud* cloudstore = NULL;
Stalestorage stalestorage;

bool debugMultiMapIndividual = false;

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
    //int res = 0;

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
    
    // Disable stale reads outside of transactions for now
    // ret = stalestorage.Read(_key,value);
    // if(ret == ERR_NOTFOUND){

        // If it's not on the stalestore

        ret = cloudstore->Read(_key, value);
        if (ret != ERR_EMPTY && ret != ERR_OK) {
            // Bad reply case
            Panic("Unable to pull");
            value = "";
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
    // Function always used inside a transaction
    string value;
    LOG_RC("PullAlwaysWatch()"); 

    int ret;
    
    ret = stalestorage.Read(_key,value);
    if(ret == ERR_NOTFOUND){
        // If it's not on the stale store
        TransactionState *ts  = GetTransactionState();

        if(ts->optionReadLocalOnly || ts->aborted){
            // If the transaction is read-local then never contact the servers
            ts->aborted = true;
            return 0;
        }

        ret = Prefetch(_key, value);
        if(ret == ERR_NOTFOUND){
            // If we still dont have the contents 

            ret = cloudstore->WatchRead(_key, value);
            if (ret != ERR_EMPTY && ret != ERR_OK) {
                Panic("Unable to pull");
                return ret;
            }
            ts->cloudAtomicReadCount++;
        }
   

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

        // PF: Writes don't require WATCH because they are delayed till 
        //     the write and atomically performed (with the guarantee that 
        //     the rs has not changed)
        //cloudstore->Watch(this->GetKey());  

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

    // Just for debugging
    if(debugMultiMapIndividual){
        for (size_t i = 0; i < keys.size(); i++) {
            string currentKey = keys.at(i);
            Map(*objects.at(i), currentKey);
        }
        return 0;
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
    
    ts->cloudAtomicReadCount = 0;
    ts->rs.clear();
    ts->ws.clear();
    ts->localView.clear();
    ts->txPrefetchKeys.clear();
    ts->txPrefetchKeyValues.clear();
    ts->optionLearnPrefetchSet = false;
    ts->optionReadLocalOnly = false;
    ts->aborted = false;

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

void
DObject::SetTransactionPrefetchKeys(set<string> &txPrefetchKeys)
{
    TransactionState *ts = GetTransactionState();
    ts->txPrefetchKeys = txPrefetchKeys;
}


void 
DObject::PrefetchKeySet(string& key, string &value, const set<string>& keySet){
    vector<string> keys;
    vector<string> values;
    TransactionState *ts = GetTransactionState();

    // convert set to vector
    std::copy(keySet.begin(), keySet.end(), std::back_inserter(keys));

    int res = cloudstore->MultiWatchRead(keys, values);
    assert(res == ERR_OK);
    assert(keys.size() == values.size());

    ts->cloudAtomicReadCount++;

    unsigned int i; 
    for(i = 0;i<keys.size();i++){
        ts->txPrefetchKeyValues[keys.at(i)] = values.at(i);
    }

    value = ts->txPrefetchKeyValues[key];

    stalestorage.ViewAdd(ts->txPrefetchKeyValues);
}

// XXX: Initialize all the prefetch structures
int
DObject::Prefetch(string key, string &value)
{
    if(!globalPrefetchEnabled){
        return ERR_NOTFOUND;
    }

    TransactionState *ts = GetTransactionState();

    // Check if we have already prefetched this key
    auto prefetchKeyValue = ts->txPrefetchKeyValues.find(key);
    if(prefetchKeyValue != ts->txPrefetchKeyValues.end()){
        value = prefetchKeyValue->second;
        return ERR_OK;
    }

    if(ts->cloudAtomicReadCount == 0){
        // Consider pre-fetching
        // If prefetching fails, it will only fail once because the cloud read will bump the cloudAtomicReadCount 
        
        // 1. First consider prefetching using the TX specific prefetchKeys
        auto prefetchKey = ts->txPrefetchKeys.find(key);
        if(prefetchKey != ts->txPrefetchKeys.end()){
            LOG_PREFETCH_ARGS("Prefetching %ld keys (tx option)\n", ts->txPrefetchKeys.size())

            PrefetchKeySet(key, value, ts->txPrefetchKeys);
            return ERR_OK;
        }

        // 2. Consider prefetcing using the global associations
        const set<string> * bestMatch = NULL;
        auto itPrefetchSet = globalPrefetchSets.begin();
        for(;itPrefetchSet!=globalPrefetchSets.end();itPrefetchSet++){
            auto keyMatch = itPrefetchSet->find(key);
            if((keyMatch != itPrefetchSet->end()) && ((bestMatch == NULL) || (itPrefetchSet->size()>bestMatch->size()))){
                // Try to find the biggest match
                // XXX: Is this the best heuristic
                bestMatch = (&*itPrefetchSet);
            }
        }
        if(bestMatch != NULL){
            LOG_PREFETCH_ARGS("Prefetching %ld keys (best match from global set)\n", bestMatch->size())
            PrefetchKeySet(key, value, *bestMatch);
            return ERR_OK;
        }
    }
    return ERR_NOTFOUND;
}

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
DObject::TransactionExecute(enum txFinishAction (*tx)(void*), void * txArg, unsigned int maxAttempts)
{
    unsigned int attempts = 0;
    int committed = 0;
    
    Notice("Executing transaction\n");

    while((attempts < maxAttempts) && (maxAttempts > 0)){
        attempts++;
        TransactionBegin();
        int ret = tx(txArg);
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


void
DObject::TransactionHandleDisconnect(void){


}

void
DObject::TransactionHandleTimeout(void){


}


bool
DObject::TransactionExecute(enum txFinishAction (*tx)(void*), 
                            enum txInterruptAction (*disconnected)(void*), 
                            enum txInterruptAction (*timedOut)(void*), 
                            void * txArg, unsigned int maxAttempts, unsigned long timeoutMs)
{
    unsigned int attempts = 0;
    int committed = 0;
    
    Notice("Executing transaction\n");

    while((attempts < maxAttempts) && (maxAttempts > 0)){
        attempts++;
        TransactionBegin();
        int ret = tx(txArg);
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

    TransactionState *ts = GetTransactionState();
    LOG_TX_DUMP_RS()
    LOG_TX_DUMP_WS()

    bool consistent = stalestorage.ViewEnd();
    if(!consistent){
        LOG_TX("TRANSACTION COMMIT -> aborted (inconsistent reads)");
        SetTransactionInProgress(false);
        pthread_mutex_unlock(&transactionMutex);
        return false;
    }

    if(ts->aborted){
        LOG_TX("TRANSACTION COMMIT -> aborted (read-local only)");
        SetTransactionInProgress(false);
        pthread_mutex_unlock(&transactionMutex);
        return false;
    }

// WITH BATCHING
    std::map<string, string> keyValuesWS;

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
    if((keyValuesWS.size() > 0) || (ts->cloudAtomicReadCount > 1)){
        // If one or more writes were made -> send to the cloud
        // If two or more reads -> send to the cloud for consistency check (and unwatch them)
        res = cloudstore->MultiWriteExec(keyValuesWS);
    }else if(ts->cloudAtomicReadCount == 1){
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

        PrefetchLearn(*txRS);

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

void
DObject::PrefetchLearn(set<string> &rs){
    TransactionState* ts = GetTransactionState();
    if((ts->optionLearnPrefetchSet) && (rs.size()>1)){
        LOG_PREFETCH_ARGS("Learning prefetchset with %ld keys\n", rs.size())
        globalPrefetchSets.insert(rs);
    }
}

// Option methods should be called from within a transaction
void
DObject::TransactionOptionPrefetch(set<string> &txPrefetchKeys)
{
    if(!IsTransactionInProgress()){
        Panic("TransactionOptionPrefetch() should be called inside a transaction");
    }
    TransactionState *ts = GetTransactionState();
    if((ts->rs.size() > 0) || (ts->ws.size() > 0)){
        Panic("TransactionOptionPrefetch() should be called inside a transaction before any read/write are performed");
    }
    SetTransactionPrefetchKeys(txPrefetchKeys);
}

void
DObject::TransactionOptionPrefetch(vector<DObject *> &txPrefetch)
{
    set<DObject *> prefetchSet;
    for (auto it = txPrefetch.begin(); it != txPrefetch.end(); it++) {
        prefetchSet.insert(*it);
    }
    TransactionOptionPrefetch(prefetchSet);
}

void
DObject::TransactionOptionPrefetch(set<DObject*> &txPrefetch)
{
    if(!IsTransactionInProgress()){
        Panic("TransactionOptionPrefetch() should be called inside a transaction");
    }
    TransactionState *ts = GetTransactionState();
    if((ts->rs.size() > 0) || (ts->ws.size() > 0)){
        Panic("TransactionOptionPrefetch() should be called inside a transaction before any read/write are performed");
    }

    set<string> txPrefetchKeys;
    auto it = txPrefetch.begin();
    for(;it!=txPrefetch.end();it++){
        txPrefetchKeys.insert((*it)->GetKey());
    }

    SetTransactionPrefetchKeys(txPrefetchKeys);
}


void 
DObject::TransactionOptionLocalOnly(bool enable)
{
    if(!IsTransactionInProgress()){
        Panic("TransactionOptionPrefetchAuto() should be called inside a transaction");
    }
    TransactionState* ts = GetTransactionState();
    ts->optionReadLocalOnly = enable;
}


void 
DObject::TransactionOptionPrefetchAuto(bool enable)
{
    if(!IsTransactionInProgress()){
        Panic("TransactionOptionPrefetchAuto() should be called inside a transaction");
    }
    TransactionState* ts = GetTransactionState();
    ts->optionLearnPrefetchSet = enable;
}

void 
DObject::PrefetchGlobalAddSet(set<DObject*> &prefetchSet)
{
    set<string> keysPrefetchSet = GetKeys(prefetchSet);
    PrefetchGlobalAddSet(keysPrefetchSet);
}

void 
DObject::PrefetchGlobalAddSet(vector<DObject*> &prefetchSet)
{
    set<DObject*> pfSet;
    for (auto it = prefetchSet.begin(); it != prefetchSet.end(); it++) {
        pfSet.insert(*it);
    }
    PrefetchGlobalAddSet(pfSet);
}

void 
DObject::PrefetchGlobalAddSet(vector<string> &prefetchSet)
{
    set<string> pfSet;
    for (auto it = prefetchSet.begin(); it != prefetchSet.end(); it++) {
        pfSet.insert(*it);
    }
    PrefetchGlobalAddSet(pfSet);
}

void 
DObject::PrefetchGlobalAddSet(set<string> &prefetchSet){
    globalPrefetchSets.insert(prefetchSet);
}

void 
DObject::PrefetchGlobalRemoveSet(set<DObject*> &prefetchSet)
{
    set<string> keysPrefetchSet = GetKeys(prefetchSet);
    PrefetchGlobalRemoveSet(keysPrefetchSet);
}

void 
DObject::PrefetchGlobalRemoveSet(set<string> &prefetchSet)
{
    globalPrefetchSets.erase(prefetchSet);
}

set<string>
DObject::GetKeys(set<DObject*> &objs){
    set<string> keys;
    
    auto it = objs.begin();

    for(;it!=objs.end();it++){
        keys.insert((*it)->GetKey());
    }
    return keys;
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
DObject::SetGlobalPrefetch(bool enable)
{
    globalPrefetchEnabled = enable;
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
DObject::SetGlobalRedisWait(bool enable, int replicas, int timeout)
{
    cloudstore->RedisWaitSet(enable, replicas, timeout);
}


void
DObject::DebugSleep(long seconds){
    Notice("Sleeping for %ld seconds\n", seconds);
    sleep(seconds);
}

void
DObject::DebugMultiMapIndividualSet(bool enable){
    debugMultiMapIndividual = enable;

}

} // namespace diamond


