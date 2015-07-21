// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * client/client.cc:
 *   Diamond client
 *
 **********************************************************************/

#include "includes/data_types.h"
#include "lib/assert.h"
#include "lib/message.h"
#include <unordered_map>

namespace diamond {

using namespace std;

extern Client diamondclient;
static unordered_map<string, DLong*> cache;

int
DLong::Map(const DLong *addr, const string &key) {

   // take a look in the cache first
   std::unordered_map<string, DLong*>::const_iterator find = cache.find(key);
   if (find != cache.end()) {
      addr = find->second;
      return 0;
   }
   
   if (!diamondclient.IsConnected()) {
      Panic("Cannot map objects before connecting to backing store server");
   }

   string value;
   
   int ret = diamondclient.Read(key, value);

   if (ret != RPC_OK) {
      return ret;
   }

   addr = new DLong(atol(value.c_str()), key);
   return 0;
}

uint64_t
DLong::Value() {
    string s;
    int ret = diamondclient.Read(_key, s);

    if (ret == RPC_OK) {
        _l = atol(s.c_str());
    }
    return _l;
}
        
void
DLong::Set(uint64_t l)
{
    _l = l;
    char buf[50];
    sprintf(buf, "%lu", _l);
    diamondclient.Write(_key, string(buf));
}

   
} // namespace diamond