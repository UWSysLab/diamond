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
#include <sstream>
#include <thread>

namespace diamond {

using namespace std;

diamond::DiamondClient* store = NULL;

void DiamondInit(const std::string &configPath, int nshards, int closestReplica) {
   store = new diamond::DiamondClient(configPath);
}

void DiamondInit() {
   DiamondInit("../../test/local", 1, 0);
}

void DiamondInit(const string &hostname, const string &port) {
    store = new diamond::DiamondClient(hostname, port);
}

string GetThreadId() {
    stringstream ss;
    ss << this_thread::get_id();
    return ss.str();
}

void
DObject::SetCaching(bool cachingEnabled) {
    store->SetCaching(cachingEnabled);
}

void
DObject::TransactionBegin(void)
{
   Debug("TRANSACTION BEGIN");

   store->Begin();
}

bool
DObject::TransactionCommit(void)
{
   Debug("TRANSACTION COMMIT");

   return store->Commit();
}

void
DObject::BeginRO(void)
{
   Debug("READ-ONLY TRANSACTION BEGIN");

   store->BeginRO();
}

void DObject::BeginReactive(uint64_t reactive_id) {
    Debug("BEGIN REACTIVE TRANSACTION");

    store->BeginReactive(reactive_id);
}

uint64_t DObject::GetNextNotification(bool blocking) {
    return store->GetNextNotification(blocking);
}

void DObject::NotificationInit(std::function<void (void)> callback) {
    store->NotificationInit(callback);
}

void DObject::Deregister(uint64_t reactive_id) {
    store->Unsubscribe(reactive_id);
}

void
DObject::SetLinearizable() {
    store->SetIsolationLevel(LINEARIZABLE);
}

void
DObject::SetSnapshotIsolation() {
    store->SetIsolationLevel(SNAPSHOT_ISOLATION);
}

void
DObject::SetEventual() {
    store->SetIsolationLevel(EVENTUAL);
}

// XXX: Add an assert so that we don't map inside a transaction?
int
DObject::Map(DObject &addr, const string &key)
{
    addr._key = key;
    return 0;
}

// XXX: Ensure return codes are correct
int
DObject::Pull()
{
    string value;
    Debug("Pull()"); 
    int ret = store->Get(_key, value);
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
DObject::Push()
{
    Debug("Push()"); 
    string value = Serialize();
    return store->Put(_key, value);
}

int
DObject::Increment(int inc)
{
   Debug("Increment(%i)", inc);
   return store->Increment(_key, inc);
}
   
int
DObject::MultiMap(vector<DObject *> &objects, vector<string> &keys)  {

    if (keys.size() != objects.size()) {
        Panic("Mismatch between number of keys and DObjects");
    }

    for (size_t i = 0; i < keys.size(); i++) {
        pthread_mutex_lock(&objects.at(i)->_objectMutex);

        string currentKey = keys.at(i);
        objects.at(i)->_key = currentKey;

        pthread_mutex_unlock(&objects.at(i)->_objectMutex);
    }

    return 0;
}

} // namespace diamond


