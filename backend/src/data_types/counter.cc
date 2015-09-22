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
    pthread_mutex_lock(&_objectMutex); 

    Pull();
    int ret = _counter;

    pthread_mutex_unlock(&_objectMutex); 
    return ret;
}
        
void
DCounter::Set(int val)
{
    pthread_mutex_lock(&_objectMutex); 

    _counter = val;
    Push();

    pthread_mutex_unlock(&_objectMutex); 
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
