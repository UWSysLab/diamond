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
#include "lib/timestamp.h"
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
        Deserialize(s);
    }
    return _l;
}
        
void
DLong::Set(const uint64_t l)
{
    _l = l;
    cloudstore->Write(_key, Serialize());
}

std::string
DLong::Serialize() {
    char buf[50];
    sprintf(buf, "%" PRIu64 "", _l);
    return string(buf);
}

void
DLong::Deserialize(const std::string &s) {
    _l = atol(s.c_str());
}

} // namespace diamond
