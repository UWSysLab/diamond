// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * data_types/set.cc
 *   Diamond string set type
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

string
DStringSet::Serialize()
{
    string ret = "";
        
    for (auto i : _set) {
        ret = ret + i + "\n";
    }
    return ret;
}

void
DStringSet::Deserialize(const string &s)
{
    _set.clear();

    string::size_type lastPos = 0;
    
    string::size_type pos = s.find_first_of("\n", lastPos);

    while (lastPos != pos && pos != std::string::npos) {
        string i = s.substr(lastPos, pos - lastPos);
        _set.insert(i);
        
        lastPos = s.find_first_not_of("\n", pos);
        // Find next "non-delimiter"
        pos = s.find_first_of("\n", lastPos);
    }

}

unordered_set<string>
DStringSet::Members()
{
    pthread_mutex_lock(&_objectMutex); 

    Pull();
    unordered_set<string> ret = _set;

    pthread_mutex_unlock(&_objectMutex); 
    return ret;
}

bool
DStringSet::InSet(const string &val)
{
    pthread_mutex_lock(&_objectMutex); 

    Pull();
    bool ret =  _set.count(val) > 0;

    pthread_mutex_unlock(&_objectMutex); 
    return ret;
}

void
DStringSet::Add(const string &val)
{
    pthread_mutex_lock(&_objectMutex); 

    Pull();
    _set.insert(val);
    Push();

    pthread_mutex_unlock(&_objectMutex); 
}

void
DStringSet::Add(const unordered_set<string> &set)
{
    pthread_mutex_lock(&_objectMutex); 

    Pull();
    for (auto e : set) {
        _set.insert(e);
    }
    Push();

    pthread_mutex_unlock(&_objectMutex); 
}

void
DStringSet::Remove(const string &val)
{
    pthread_mutex_lock(&_objectMutex); 

    Pull();
    _set.erase(val);
    Push();

    pthread_mutex_unlock(&_objectMutex); 
}

void
DStringSet::Clear()
{
    pthread_mutex_lock(&_objectMutex); 

    Pull();
    _set.clear();
    Push();

    pthread_mutex_unlock(&_objectMutex); 
}
    
} // namespace diamond
