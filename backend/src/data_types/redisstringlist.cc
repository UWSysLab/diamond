// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * data_types/list.cc
 *   Diamond list type
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


vector<string>
DRedisStringList::Members()
{
    vector<string> ret;
    cloudstore->Lrange(_key, 0, -1, ret);
    return ret;
}

string
DRedisStringList::Value(const int index)
{
    string ret;
    cloudstore->Lindex(_key, index, ret);
    return ret;
}

void
DRedisStringList::Append(const string val)
{
    cloudstore->Rpush(_key, val);
}

void
DRedisStringList::Append(const vector<string> &vec)
{
    Panic("Not implemented yet");
}


void
DRedisStringList::EraseFirst() {
    string val;
    cloudstore->Lpop(_key, val, false);
}

void
DRedisStringList::Clear() {
    cloudstore->Del(_key);
}

long
DRedisStringList::Size() {
    long ret;
    cloudstore->Llen(_key, ret);
    return ret;
}

} // namespace diamond
