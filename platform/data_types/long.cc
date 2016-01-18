// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * client/client.cc:
 *   Diamond client
 *
 **********************************************************************/

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
    pthread_mutex_unlock(&_objectMutex); 

    Pull();
    uint64_t ret = _l;

    pthread_mutex_unlock(&_objectMutex); 
    return ret;
}

void
DLong::Set(const uint64_t l)
{
    pthread_mutex_lock(&_objectMutex); 
    
    SetNotProtected(l);

    pthread_mutex_unlock(&_objectMutex); 
}

void
DLong::SetNotProtected(const uint64_t l)
{
    _l = l;
    Push();
}

DLong & 
DLong::operator+=(const uint64_t i) 
{
    pthread_mutex_lock(&_objectMutex); 

    SetNotProtected(_l + i); 

    pthread_mutex_unlock(&_objectMutex); 
    return *this;
}

DLong & 
DLong::operator-=(const uint64_t i) 
{ 
    pthread_mutex_lock(&_objectMutex); 

    SetNotProtected(_l - i); 
        
    pthread_mutex_unlock(&_objectMutex); 

    return *this;
};      

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
