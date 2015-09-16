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

Cloud* cloudstore = Cloud::Instance();    

std::string
DString::Value() {
    string s;
    int ret = cloudstore->Read(_key, s);

    if (ret == ERR_OK) {
        Deserialize(s);
    }
    return _s;
}

void
DString::Set(const std::string &s)
{
    _s = s;
    cloudstore->Write(_key, Serialize());
}

std::string
DString::Serialize() {
    return _s;
}

void DString::Deserialize(const std::string &s) {
    _s = s;
}

} // namespace diamond
