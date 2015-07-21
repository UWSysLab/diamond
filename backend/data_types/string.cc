// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * data_types/string.cc
 *   Diamond string data type
 *
 **********************************************************************/

#include "includes/data_types.h"
#include "lib/assert.h"
#include "lib/message.h"
#include <unordered_map>

namespace diamond {

using namespace std;

extern Client diamondclient;
static unordered_map<string, DString*> cache;
    
int
Map(const DString *addr, const string &key) {
    // take a look in the cache first
    std::unordered_map<string,DString*>::const_iterator find = cache.find(key);
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

    addr = new DString(value, key);
    return 0;
}


std::string
DString::Value() {
    string s;
    int ret = diamondclient.Read(_key, s);

    if (ret == RPC_OK) {
        _s = s;
    }
    return _s;
}

void
DString::Set(const std::string &s)
{
    _s = s;
    diamondclient.Write(_key, _s);
}


} // namespace diamond
