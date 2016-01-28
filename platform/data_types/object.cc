#include "includes/data_types.h"
#include "includes/error.h"
#include "lib/assert.h"
#include "lib/message.h"
#include "lib/timestamp.h"
#include "client/diamondclient.h"
#include <unordered_map>
#include <map>
#include <inttypes.h>
#include <set>

namespace diamond {

using namespace std;

diamond::DiamondClient* store = NULL;

void DiamondInit(const std::string &configPath, int nshards, int closestReplica) {
   store = new diamond::DiamondClient(configPath);
}

void DiamondInit() {
   DiamondInit("../../test/local", 1, 0);
}

void
DObject::TransactionBegin(void)
{
   Debug("TRANSACTION BEGIN");

   store->Begin();
}

int
DObject::TransactionCommit(void)
{
   Debug("TRANSACTION COMMIT");

   return store->Commit();
}

// XXX: Add an assert so that we don't map inside a transaction?
int
DObject::Map(DObject &addr, const string &key)
{
    addr._key = key;
    
    return addr.Pull();
}

// XXX: Ensure return codes are correct
int
DObject::Pull(){
    string value;
    Debug("Pull()"); 

    int ret;
    
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
    
    Deserialize(value);
    return 0;
}


int
DObject::Push(){
    Debug("Push"); 

    string value = Serialize();

    return store->Put(_key, value);
}

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


