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

// Callee should hold the _objectMutex
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

// Callee should hold the _objectMutex
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

    string s;
    cloudstore->Read(_key, s);
    Deserialize(s);
    
    pthread_mutex_unlock(&_objectMutex);
    return _vec;
}

int
DList::IndexNotProtected(const uint64_t val)
{
    string s;
    cloudstore->Read(_key, s);
    Deserialize(s);
    for (auto it = _vec.begin(); it != _vec.end(); it++) {
        if (*it == val) {
            return (it - _vec.begin());
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

    string s;
    cloudstore->Read(_key, s);
    Deserialize(s);
    
    pthread_mutex_unlock(&_objectMutex);
    return _vec.at(index);
}

void
DList::Append(const uint64_t val)
{
    pthread_mutex_lock(&_objectMutex);

    string s;
    cloudstore->Read(_key, s);
    Deserialize(s);
    _vec.push_back(val);
    cloudstore->Write(_key, Serialize());
    
    pthread_mutex_unlock(&_objectMutex);
}

void
DList::Append(const vector<uint64_t> &vec)
{
    pthread_mutex_lock(&_objectMutex);

    string s;
    cloudstore->Read(_key, s);
    Deserialize(s);
    for (auto e : vec) {
        _vec.push_back(e);
    }
    cloudstore->Write(_key, Serialize());
    
    pthread_mutex_unlock(&_objectMutex);
}

void
DList::Insert(const int index, const uint64_t val) {
    pthread_mutex_lock(&_objectMutex);

    string s;
    cloudstore->Read(_key, s);
    Deserialize(s);
    _vec.insert(index + _vec.begin(), val);
    cloudstore->Write(_key, Serialize());
    
    pthread_mutex_unlock(&_objectMutex);
}

void
DList::Erase(const int index) {
    pthread_mutex_lock(&_objectMutex);

    string s;
    cloudstore->Read(_key, s);
    Deserialize(s);
    _vec.erase(index + _vec.begin());
    cloudstore->Write(_key, Serialize());
    
    pthread_mutex_unlock(&_objectMutex);
}

void
DList::Remove(const uint64_t val) {
    pthread_mutex_lock(&_objectMutex);

    string s;
    cloudstore->Read(_key, s);
    Deserialize(s);
    _vec.erase(IndexNotProtected(val) + _vec.begin());
    cloudstore->Write(_key, Serialize());
    
    pthread_mutex_unlock(&_objectMutex);
}

void
DList::Clear() {
    pthread_mutex_lock(&_objectMutex);

    string s;
    cloudstore->Read(_key, s);
    Deserialize(s);
    _vec.clear();
    cloudstore->Write(_key, Serialize());
    
    pthread_mutex_unlock(&_objectMutex);
}

int
DList::Size() {
    pthread_mutex_lock(&_objectMutex);

    string s;
    cloudstore->Read(_key, s);
    Deserialize(s);
    
    pthread_mutex_unlock(&_objectMutex);
    return _vec.size();
}

} // namespace diamond
