// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * data_types/set.cc
 *   Diamond set type
 *
 **********************************************************************/

#include "client/client.h"
#include "includes/data_types.h"
#include "lib/assert.h"
#include "lib/message.h"
#include <unordered_map>
#include <inttypes.h>

namespace diamond {

using namespace std;

extern Client diamondclient;
static unordered_map<string, DSet> cache;
   
int
DSet::Map(DSet &addr, const string &key)
{
    addr._key = key;
    // take a look in the cache first
    auto find = cache.find(key);
    if (find != cache.end()) {
        addr._set = find->second._set;
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

    addr.Deserialize(value);
    cache[key] = addr;
    
    return 0;
}

string
DSet::Serialize()
{
    string ret = "";
        
    for (auto i : _set) {
        char buf[50];
        sprintf(buf, "%" PRIu64 "\n", i);
        ret += string(buf);
    }
    return ret;
}

void
DSet::Deserialize(string &s)
{
    _set.clear();

    string::size_type lastPos = 0;
    
    string::size_type pos = s.find_first_of("\n", lastPos);

    while (lastPos != pos && pos != std::string::npos) {
        uint64_t i = atol(s.substr(lastPos, pos - lastPos).c_str());
        _set.insert(i);
        
        lastPos = s.find_first_not_of("\n", pos);
        // Find next "non-delimiter"
        pos = s.find_first_of("\n", lastPos);
    }

}

unordered_set<uint64_t>
DSet::Members()
{
    string s;
    diamondclient.Read(_key, s);
    Deserialize(s);
    return _set;
}

bool
DSet::InSet(const uint64_t val)
{
    string s;
    diamondclient.Read(_key, s);
    Deserialize(s);
    return _set.count(val) > 0;
}

void
DSet::Add(const uint64_t val)
{
    string s;
    diamondclient.Read(_key, s);
    Deserialize(s);
    _set.insert(val);
    diamondclient.Write(_key, Serialize());
}

void
DSet::Add(const unordered_set<uint64_t> &set)
{
    string s;
    diamondclient.Read(_key, s);
    Deserialize(s);
    for (auto e : set) {
        _set.insert(e);
    }
    diamondclient.Write(_key, Serialize());
}

void
DSet::Remove(const uint64_t val)
{
    string s;
    diamondclient.Read(_key, s);
    Deserialize(s);
    _set.erase(val);
    diamondclient.Write(_key, Serialize());
}

void
DSet::Clear()
{
    string s;
    diamondclient.Read(_key, s);
    Deserialize(s);
    _set.clear();
    diamondclient.Write(_key, Serialize());
}
    
} // namespace diamond
