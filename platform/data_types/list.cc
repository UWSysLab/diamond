// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * data_types/list.cc
 *   Diamond list type
 *
 **********************************************************************/

#include "includes/data_types.h"
#include "lib/assert.h"
#include "lib/message.h"
#include <unordered_map>
#include <inttypes.h>

namespace diamond {

using namespace std;

// Thread-safety: Callee should hold the _objectMutex
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

// Thread-safety: Callee should hold the _objectMutex
void
DList::Deserialize(const string &s)
{
    _vec.clear();

    string::size_type lastPos = 0;
    
    string::size_type pos = s.find_first_of("\n", lastPos);

    while (lastPos != pos && pos != std::string::npos) {
        uint64_t i = atol(s.substr(lastPos, pos - lastPos).c_str());
        _vec.push_back(i);
        
        lastPos = s.find_first_not_of("\n", pos);
        // Find next "non-delimiter"
        pos = s.find_first_of("\n", lastPos);
    }

}


vector<uint64_t>
DList::Members()
{
    pthread_mutex_lock(&_objectMutex);

    Pull();
    vector<uint64_t> ret = _vec;

    pthread_mutex_unlock(&_objectMutex);
    return ret;
}

// Thread-safety: Callee should hold the _objectMutex
int
DList::IndexNotProtected(const uint64_t val)
{
    Pull();

    for (auto it = _vec.begin(); it != _vec.end(); it++) {
        if (*it == val) {
            int ret = (it - _vec.begin());
            return ret; 
        }
    }

    return -1;
}


int
DList::Index(const uint64_t val)
{
    pthread_mutex_lock(&_objectMutex);

    int res = IndexNotProtected(val);

    pthread_mutex_unlock(&_objectMutex);
    return res;
}

uint64_t
DList::Value(const int index)
{
    pthread_mutex_lock(&_objectMutex);

    Pull();
    uint64_t ret = _vec.at(index);

    pthread_mutex_unlock(&_objectMutex);
    return ret; 
}

void
DList::Append(const uint64_t val)
{
    pthread_mutex_lock(&_objectMutex);

    Pull();
    _vec.push_back(val);
    Push();    

    pthread_mutex_unlock(&_objectMutex);
}

void
DList::Append(const vector<uint64_t> &vec)
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
DList::Insert(const int index, const uint64_t val) {
    pthread_mutex_lock(&_objectMutex);

    Pull();
    _vec.insert(index + _vec.begin(), val);
    Push();
    
    pthread_mutex_unlock(&_objectMutex);
}

void
DList::Erase(const int index) {
    pthread_mutex_lock(&_objectMutex);

    Pull();
    _vec.erase(index + _vec.begin());
    Push();
    
    pthread_mutex_unlock(&_objectMutex);
}

void
DList::Remove(const uint64_t val) {
    pthread_mutex_lock(&_objectMutex);

    Pull();
    _vec.erase(IndexNotProtected(val) + _vec.begin());
    Push();
    
    pthread_mutex_unlock(&_objectMutex);
}

void
DList::Clear() {
    pthread_mutex_lock(&_objectMutex);

    Pull();
    _vec.clear();
    Push();
    
    pthread_mutex_unlock(&_objectMutex);
}

int
DList::Size() {
    pthread_mutex_lock(&_objectMutex);

    Pull();
    int ret = _vec.size();

    pthread_mutex_unlock(&_objectMutex);
    return ret;
}

} // namespace diamond
