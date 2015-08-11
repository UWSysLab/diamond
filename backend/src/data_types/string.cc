// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * data_types/string.cc
 *   Diamond string data type
 *
 **********************************************************************/

#include "storage/cloud.h"
#include "includes/data_types.h"
#include "lib/assert.h"
#include "lib/message.h"
#include <unordered_map>

namespace diamond {

using namespace std;

Cloud cloudstore;    
static unordered_map<string, DString> cache;
    
int
DString::Map(DString &addr, const string &key) {
    // take a look in the cache first
    addr._key = key;
    
    auto find = cache.find(key);
    if (find != cache.end()) {
        addr._s = find->second._s;
        return 0;
    }

    if (!cloudstore.IsConnected()) {
       Panic("Cannot map objects before connecting to backing store server");
    }

    string value;
   
    int ret = cloudstore.Read(key, value);

    if (ret != RPC_OK) {
       return ret;
    }

    addr._s = value;
    cache[key] = addr;
    return RPC_OK;
}


std::string
DString::Value() {
    string s;
    int ret = cloudstore.Read(_key, s);

    if (ret == RPC_OK) {
        _s = s;
    }
    return _s;
}

void
DString::Set(const std::string &s)
{
    _s = s;
    cloudstore.Write(_key, _s);
}

} // namespace diamond
