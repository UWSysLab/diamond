
#include "storage/cloud.h"
#include "includes/data_types.h"
#include "lib/assert.h"
#include "lib/message.h"
#include "lib/timestamp.h"
#include <unordered_map>
#include <inttypes.h>

namespace diamond {

using namespace std;


void
DObject::Lock(){
    pthread_mutex_lock(&_objectMutex);
    LockNotProtected();
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

    long lockid = getTimestamp();
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

    res = cloudstore->Pop(_key + string("-lock-wait"), value, false);
    assert(res == ERR_OK);
    assert(value != "");

    res = cloudstore->Push(_key + string("-lock-wait-") + value, empty);
    assert(res == ERR_OK);

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
        res = cloudstore->Pop(_key + string("-lock-wait"), value, false);
        if(res != ERR_OK){
            break;
        }
        res = cloudstore->Push(_key + string("-lock-wait-") + value, empty);
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
    res = cloudstore->Push(_key + string("-lock-wait"), string(lockid));
    assert(res == ERR_OK);
    UnlockNotProtected();
    res = cloudstore->Pop(_key + string("-lock-wait-") + string(lockid), value, true);
    assert(res == ERR_OK);


    // B. Reaquire lock
    LockNotProtected();

    pthread_mutex_unlock(&_objectMutex);
}

} // namespace diamond


