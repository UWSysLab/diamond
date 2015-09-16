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
