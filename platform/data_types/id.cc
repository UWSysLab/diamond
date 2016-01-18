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

std::string
DID::Value() {
    pthread_mutex_lock(&_objectMutex); 

    Pull(); 
    std::string ret = _s;

    pthread_mutex_unlock(&_objectMutex); 
    return ret;
}

void
DID::Generate()
{
    pthread_mutex_lock(&_objectMutex); 
   
    // XXX: Should seed properly
    long int r = random();
    _s = r;
    Push();

    pthread_mutex_unlock(&_objectMutex); 
}

std::string
DID::Serialize() {
    return _s;
}

void DID::Deserialize(const std::string &s) {
    _s = s;
}

} // namespace diamond
