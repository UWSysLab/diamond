// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * data_types/Counter.cc
 *   Diamond counter type
 *
 **********************************************************************/

#include "storage/cloud.h"
#include "includes/data_types.h"
#include "lib/assert.h"
#include "lib/message.h"
#include <unordered_map>

namespace diamond {

using namespace std;

int
DCounter::Value() {
    Pull();
    return _counter;
}
        
void
DCounter::Set(int val)
{
    _counter = val;
    Push();
}

std::string DCounter::Serialize() {
    char buf[50];
    sprintf(buf, "%i", _counter);
    return string(buf);
}

void DCounter::Deserialize(const std::string &s) {
    _counter = atoi(s.c_str());
}

} // namespace diamond
