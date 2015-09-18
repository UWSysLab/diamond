// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * data_types/set.cc
 *   Diamond set type
 *
 **********************************************************************/

#include "storage/cloud.h"
#include "includes/data_types.h"
#include "lib/assert.h"
#include "lib/message.h"
#include <unordered_map>
#include <inttypes.h>

namespace diamond {

using namespace std;

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
DSet::Deserialize(const string &s)
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
    Pull();
    return _set;
}

bool
DSet::InSet(const uint64_t val)
{
    Pull();
    return _set.count(val) > 0;
}

void
DSet::Add(const uint64_t val)
{
    Pull();
    _set.insert(val);
    Push();
}

void
DSet::Add(const unordered_set<uint64_t> &set)
{
    Pull();
    for (auto e : set) {
        _set.insert(e);
    }
    Push();
}

void
DSet::Remove(const uint64_t val)
{
    Pull();
    _set.erase(val);
    Push();
}

void
DSet::Clear()
{
    Pull();
    _set.clear();
    Push();
}
    
} // namespace diamond
