// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * client/client.cc:
 *   Diamond client
 *
 **********************************************************************/

#include "client/client.h"
#include "includes/data_types.h"
#include "lib/assert.h"
#include "lib/message.h"
#include <unordered_map>

namespace diamond {

using namespace std;

extern Client diamondclient;
static unordered_map<string, DLong> cache;

int
DLong::Map(DLong &addr, const string &key)
{

    addr._key = key;
   // take a look in the cache first
   auto find = cache.find(key);
   if (find != cache.end()) {
      addr._l = find->second._l;
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

   addr._l = atol(value.c_str());
   cache[key] = addr;
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
DLong::Set(const uint64_t l)
{
    _l = l;
    char buf[50];
    sprintf(buf, "%lu", _l);
    diamondclient.Write(_key, string(buf));
}

   
} // namespace diamond
