// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * data_types/boollist.cc
 *   Diamond boolean list type
 *
 **********************************************************************/

#include "includes/data_types.h"
#include "lib/assert.h"
#include "lib/message.h"
#include <unordered_map>
#include <inttypes.h>

namespace diamond {

using namespace std;

string
DBooleanList::Serialize()
{
    string ret = "";
        
    for (auto i : _vec) {
        char buf[50];
        sprintf(buf, "%" PRIu64 "\n", (uint64_t)i);
        ret += string(buf);
    }
    return ret;
}

void
DBooleanList::Deserialize(const string &s)
{
    _vec.clear();

    string::size_type lastPos = 0;
    
    string::size_type pos = s.find_first_of("\n", lastPos);

    while (lastPos != pos && pos != std::string::npos) {
        bool i = (bool)atol(s.substr(lastPos, pos - lastPos).c_str());
        _vec.push_back(i);
        
        lastPos = s.find_first_not_of("\n", pos);
        // Find next "non-delimiter"
        pos = s.find_first_of("\n", lastPos);
    }
}

vector<bool>
DBooleanList::Members()
{
    pthread_mutex_lock(&_objectMutex); 

    Pull();
    vector<bool> ret = _vec;

    pthread_mutex_unlock(&_objectMutex); 
    return ret;
}

bool
DBooleanList::Value(const int index)
{
    pthread_mutex_lock(&_objectMutex); 

    Pull();
    bool ret = _vec.at(index);

    pthread_mutex_unlock(&_objectMutex); 
    return ret;
}

void
DBooleanList::Append(const bool val)
{
    pthread_mutex_lock(&_objectMutex); 

    Pull();
    _vec.push_back(val);
    Push();

    pthread_mutex_unlock(&_objectMutex); 
}

void
DBooleanList::Append(const vector<bool> &vec)
{
    pthread_mutex_lock(&_objectMutex); 

    Pull();
    for (auto e : vec) {
        _vec.push_back(e);
    }
    Push();

    pthread_mutex_unlock(&_objectMutex); 
}

void
DBooleanList::Insert(const int index, const bool val) {
    pthread_mutex_lock(&_objectMutex); 

    Pull();
    _vec.insert(index + _vec.begin(), val);
    Push();

    pthread_mutex_unlock(&_objectMutex); 
}

void
DBooleanList::Erase(const int index) {
    pthread_mutex_lock(&_objectMutex); 

    Pull();
    _vec.erase(index + _vec.begin());
    Push();

    pthread_mutex_unlock(&_objectMutex); 
}

void
DBooleanList::Clear() {
    pthread_mutex_lock(&_objectMutex); 
    Pull();
    _vec.clear();
    Push();
    pthread_mutex_unlock(&_objectMutex); 
}

int
DBooleanList::Size() {
    pthread_mutex_lock(&_objectMutex); 

    Pull();
    int ret = _vec.size();

    pthread_mutex_unlock(&_objectMutex); 
    return ret;
}

void DBooleanList::Set(const int index, const bool val) {
    pthread_mutex_lock(&_objectMutex);

    Pull();
    _vec[index] = val;
    Push();

    pthread_mutex_unlock(&_objectMutex);
}

} // namespace diamond
