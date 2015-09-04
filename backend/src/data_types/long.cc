// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * client/client.cc:
 *   Diamond client
 *
 **********************************************************************/

#include "storage/cloud.h"
#include "includes/data_types.h"
#include "lib/assert.h"
#include "lib/message.h"
#include "lib/rdtsc.h"
#include <unordered_map>
#include <inttypes.h>

namespace diamond {

using namespace std;

static unordered_map<string, DLong> cache;

int
DLong::Map(DLong &addr, const string &key)
{

    addr._key = key;
   // take a look in the cache first
   auto find = cache.find(key);
   if (find != cache.end()) {
      addr._l = find->second._l;
      return ERR_OK;
   }
   
   if (!cloudstore->IsConnected()) {
      Panic("Cannot map objects before connecting to backing store server");
   }

   string value;
   
   int ret = cloudstore->Read(key, value);

   if (ret != ERR_OK) {
      return ret;
   }

   addr._l = atol(value.c_str());
   cache[key] = addr;
   return ERR_OK;
}

uint64_t
DLong::Value() {
    string s;
    int ret = cloudstore->Read(_key, s);

    if (ret == ERR_OK) {
        _l = atol(s.c_str());
    }
    return _l;
}
        
void
DLong::Set(const uint64_t l)
{
    _l = l;
    char buf[50];
    sprintf(buf, "%" PRIu64 "", _l);
    cloudstore->Write(_key, string(buf));
}

void
DLong::Lock(){
    int res;
    char value[50];
    int max_tries = 1000;
    int delay_ns = 1;
    int max_delay_ns = 1000 * 1000;

    if(_locked){
        Panic("Object already locked");
    }

    _lockid = rdtsc();
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
DLong::ContinueLock(){
    if(!_locked){
        Panic("Object not locked");
    }
    Panic("NYI");
}


void
DLong::Unlock(){
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
DLong::Signal(){
    if(!_locked){
        Panic("Object not locked");
    }
    int res;
    string value;
    string empty = "";
   
    res = cloudstore->Pop(_key + string("-lock-wait"), value, false);
    assert(res == ERR_OK);

    res = cloudstore->Push(_key + string("-lock-wait-") + value, empty);
    assert(res == ERR_OK);
}

void
DLong::Broadcast(){
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
DLong::Wait(){
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
