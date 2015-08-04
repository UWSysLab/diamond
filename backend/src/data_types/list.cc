// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * data_types/list.cc
 *   Diamond list type
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
static unordered_map<string, DList> cache;
   
int
DList::Map(DList &addr, const string &key)
{
    addr._key = key;
    // take a look in the cache first
    auto find = cache.find(key);
    if (find != cache.end()) {
        addr._vec = find->second._vec;
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
DList::Serialize()
{
    string ret = "";
        
    for (auto i : _vec) {
        char buf[50];
        sprintf(buf, "%" PRIu64 "\n", i);
        ret += string(buf);
    }
    return ret;
}

void
DSet::Deserialize(string &s)
{
    _vec.clear();

    string::size_type lastPos = 0;
    
    string::size_type pos = s.find_first_of("\n", lastPos);

    while (lastPos != pos) {
        uint64_t i = atol(s.substr(lastPos, pos - lastPos).c_str());
        _vec.insert(i);
        
        lastPos = s.find_first_not_of("\n", pos);
        // Find next "non-delimiter"
        pos = s.find_first_of("\n", lastPos);
    }

}

list <uint64_t>
DList::Members()
{
    string s;
    diamondclient.Read(_key, s);
    Deserialize(s);
    return _vec;
}

int
DList::Index(const uint64_t val)
{
    string s;
    diamondclient.Read(_key, s);
    Deserialize(s);
    for (auto it = _vec.begin(); it != _vec.end(); it++) {
        if (*it == val) {
            return (it - vec.begin());
        }
    }
    return -1;
}

uint64_t
DList::Value(const int index)
{
    string s;
    diamondclient.Read(_key, s);
    Deserialize(s);
    return _vec.at(index);
}

void
DList::Append(const uint64_t val)
{
    _vec.push_back(val);
    diamondclient.Write(_key, Serialize());
}

void
DList::Append(const vector<uint64_t> &vec)
{
    for (auto e : vec) {
        _vec.push_back(e);
    }
    diamondclient.Write(_key, Serialize());
}

void
DList::Insert(const int index, const uint64_t val) {
    _vec.insert(index + _vec.begin(), val);
    diamondclient.Write(_key, Serialize());
}

void
DList::Erase(const int index) {
    _vec.erase(index + _vec.begin());
    diamondclient.Write(_key, Serialize());
}

void
DList::Remove(const uint64_t val) {
    _vec.erase(Index(val));
    diamondclient.Write(_key, Serialize());
}

} // namespace diamond
