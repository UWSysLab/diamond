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

string
DStringList::Serialize()
{
    string ret = "";
        
    for (auto i : _vec) {
        ret = ret + i + "\n";
    }
    return ret;
}

void
DStringList::Deserialize(const string &s)
{
    _vec.clear();

    string::size_type lastPos = 0;
    
    string::size_type pos = s.find_first_of("\n", lastPos);

    while (lastPos != pos && pos != std::string::npos) {
        string i = s.substr(lastPos, pos - lastPos);
        _vec.push_back(i);
        
        lastPos = s.find_first_not_of("\n", pos);
        // Find next "non-delimiter"
        pos = s.find_first_of("\n", lastPos);
    }

}

vector<string>
DStringList::Members()
{
    pthread_mutex_lock(&_objectMutex); 

    Pull();
    vector<string> ret = _vec;

    pthread_mutex_unlock(&_objectMutex); 
    return ret;
}

int
DStringList::IndexNotProtected(const string val)
{
    Pull();
    for (auto it = _vec.begin(); it != _vec.end(); it++) {
        if (*it == val) {
            return (it - _vec.begin());
        }
    }
    return -1;
}

int
DStringList::Index(const string val)
{
    pthread_mutex_lock(&_objectMutex); 

    int ret = IndexNotProtected(val);
    
    pthread_mutex_unlock(&_objectMutex); 
    return ret;
}

string
DStringList::Value(const int index)
{
    pthread_mutex_lock(&_objectMutex); 

    Pull();
    string ret = _vec.at(index);

    pthread_mutex_unlock(&_objectMutex); 
    return ret;
}

void
DStringList::Append(const string val)
{
    pthread_mutex_lock(&_objectMutex); 

    Pull();
    _vec.push_back(val);
    Push();

    pthread_mutex_unlock(&_objectMutex); 
}

void
DStringList::Append(const vector<string> &vec)
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
DStringList::Insert(const int index, const string val) {
    pthread_mutex_lock(&_objectMutex); 

    Pull();
    _vec.insert(index + _vec.begin(), val);
    Push();

    pthread_mutex_unlock(&_objectMutex); 
}

void
DStringList::Erase(const int index) {
    pthread_mutex_lock(&_objectMutex); 

    Pull();
    _vec.erase(index + _vec.begin());
    Push();

    pthread_mutex_unlock(&_objectMutex); 
}

void
DStringList::Remove(const string val) {
    pthread_mutex_lock(&_objectMutex); 

    Pull();
    _vec.erase(IndexNotProtected(val) + _vec.begin());
    Push();

    pthread_mutex_unlock(&_objectMutex); 
}

void
DStringList::Clear() {
    pthread_mutex_lock(&_objectMutex); 
    Pull();
    _vec.clear();
    Push();
    pthread_mutex_unlock(&_objectMutex); 
}

int
DStringList::Size() {
    pthread_mutex_lock(&_objectMutex); 

    Pull();
    int ret = _vec.size();

    pthread_mutex_unlock(&_objectMutex); 
    return ret;
}

void
DStringList::Set(const int index, const string val) {
    pthread_mutex_lock(&_objectMutex);

    Pull();
    _vec[index] = val;
    Push();

    pthread_mutex_unlock(&_objectMutex);
}

} // namespace diamond
