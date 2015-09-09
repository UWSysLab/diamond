
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
    int res;
    char value[50];
    int max_tries = 1000;
    int delay_ns = 1;
    int max_delay_ns = 1000 * 1000;

    if(_locked){
        Panic("Object already locked");
    }

    _lockid = getTimestamp();
    sprintf(value, "%" PRIu64 "", _lockid);

    while(max_tries--){
        res = cloudstore->Write(_key + string("-lock"), string(value), WRITE_IFF_NOT_EXIST, LOCK_DURATION_MS);
        if(res == ERR_OK){
            _locked = true;
            return;
        }else if(res == ERR_NOT_PERFORMED){
            usleep(delay_ns);
            delay_ns = std::min(delay_ns * 2, max_delay_ns);
        }else{
            Panic("NYI");
        }
    }
    Panic("Unable to aquire lock");
}

void
DObject::ContinueLock(){
    if(!_locked){
        Panic("Object not locked");
    }
    Panic("NYI");
}


void
DObject::Unlock(){
    // From redlock
    string m_unlockScript = string("if redis.call('get', KEYS[1]) == ARGV[1] then return redis.call('del', KEYS[1]) else return 0 end");
    int res;
    char value[50];

    sprintf(value, "%" PRIu64 "", _lockid);
    res = cloudstore->RunOnServer(m_unlockScript, _key + string("-lock"), string(value));
    assert(res == ERR_OK);

    if(!_locked){
        Panic("Object not locked");
    }
    _locked = false;
}

void
DObject::Signal(){
    if(!_locked){
        Panic("Object not locked");
    }
    int res;
    string value;
    string empty = "";

    res = cloudstore->Pop(_key + string("-lock-wait"), value, false);
    assert(res == ERR_OK);
    assert(value != "");

    res = cloudstore->Push(_key + string("-lock-wait-") + value, empty);
    assert(res == ERR_OK);
}

void
DObject::Broadcast(){
    if(!_locked){
        Panic("Object not locked");
    }
    int res;
    string value;
    string empty = "";

    while(1){
        res = cloudstore->Pop(_key + string("-lock-wait"), value, false);
        if(res != ERR_OK){
            break;
        }
        res = cloudstore->Push(_key + string("-lock-wait-") + value, empty);
        assert(res == ERR_OK);
    }
}


void
DObject::Wait(){
    int res;
    char lockid[50];
    string value;

    if(!_locked){
        Panic("Object not locked");
    }


    sprintf(lockid, "%" PRIu64 "", _lockid);

    // A. Unlock & Sleep
    res = cloudstore->Push(_key + string("-lock-wait"), string(lockid));
    assert(res == ERR_OK);
    Unlock();
    res = cloudstore->Pop(_key + string("-lock-wait-") + string(lockid), value, true);
    assert(res == ERR_OK);


    // B. Reaquire lock
    Lock();
}

} // namespace diamond


