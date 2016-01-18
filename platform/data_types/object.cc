#include "includes/data_types.h"
#include "includes/error.h"
#include "lib/assert.h"
#include "lib/message.h"
#include "lib/timestamp.h"
#include "store/client.h"
#include <unordered_map>
#include <map>
#include <inttypes.h>
#include <set>

namespace diamond {

using namespace std;

// Used by diamond_profile.h macros
pthread_mutex_t  profileMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t  transactionMutex = PTHREAD_MUTEX_INITIALIZER; // Protects the global transaction structures

long profileEnterTs[MAX_THREAD_ID];

// Used to simulate that the network is offline
bool networkConnectivity = false;

strongstore::Client* store = NULL;
bool debugMultiMapIndividual = false;

void DiamondInit(const std::string &configPath, int nshards, int closestReplica) {
   store = new strongstore::Client(configPath, nshards, closestReplica);
}

void DiamondInit() {
   DiamondInit("./replicas.config", 3, 0);
}

void
DObject::TransactionBegin(void)
{
   pthread_mutex_lock(&transactionMutex);

   Debug("TRANSACTION BEGIN");

   store->Begin();

   pthread_mutex_unlock(&transactionMutex);
}

int
DObject::TransactionCommit(void)
{
   pthread_mutex_lock(&transactionMutex);
   Debug("TRANSACTION COMMIT");

   int ret = store->Commit();

   pthread_mutex_unlock(&transactionMutex);
   return ret;
}

// XXX: Add an assert so that we don't map inside a transaction?
int
DObject::Map(DObject &addr, const string &key)
{
    PROFILE_ENTER("MAP");
    pthread_mutex_lock(&addr._objectMutex);

    addr._key = key;
   
    int res = addr.Pull();
    //int res = 0;

    pthread_mutex_unlock(&addr._objectMutex);
    PROFILE_EXIT("MAP");
    return res;
}

// XXX: Ensure return codes are correct
int
DObject::Pull(){
    string value;
    Debug("Pull()"); 

    int ret;
    
    // Disable stale reads outside of transactions for now
    // ret = stalestorage.Read(_key,value);
    // if(ret == ERR_NOTFOUND){

        // If it's not on the stalestore

        ret = store->Get(_key, value);
        if (ret != REPLY_NOT_FOUND && ret != REPLY_OK) {
            // Bad reply case
            Panic("Unable to pull");
            value = "";
            return ret;
        }

        if (ret == REPLY_NOT_FOUND) {
            value = "";
        }
    // }

    Deserialize(value);
    return 0;
}


int
DObject::Push(){
    string value;
    Debug("PushAlways()"); 

    value = Serialize();

    int ret = store->Put(_key, value);
    if (ret != REPLY_OK) {
        return ret;
    }
    return 0;
}

// XXX: Thread-safety: make sure Multi-get is thread-safe
// We're not protecting against situations where the user modifies the parameters concurrently with MultiMap
int
DObject::MultiMap(vector<DObject *> &objects, vector<string> &keys)  {

    if (keys.size() != objects.size()) {
        Panic("Mismatch between number of keys and DObjects");
    }

    return 0;
    // vector<string> values;
    // // int ret = cloudstore->MultiGet(keys, values);
    // // if (ret != REPLY_OK) {
    // //     return ret;
    // // }

    // if (keys.size() != values.size()) {
    //     Panic("Mismatch between number of keys and values returned by MultiGet");
    // }

    // for (size_t i = 0; i < keys.size(); i++) {
    //     pthread_mutex_lock(&objects.at(i)->_objectMutex);

    //     string currentKey = keys.at(i);
    //     objects.at(i)->_key = currentKey;
    //     objects.at(i)->Deserialize(values.at(i));

    //     pthread_mutex_unlock(&objects.at(i)->_objectMutex);
    // }

    // return 0;
}

} // namespace diamond


