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

string
DStringQueue::Serialize()
{
    return _l.Serialize();
}

void
DStringQueue::Deserialize(const string &s)
{
    _l.Deserialize(s);
}



void
DStringQueue::Queue(const string val)
{
    _l.Append(val);
}



string 
DStringQueue::Dequeue() {
    pthread_mutex_lock(&_objectMutex); 
    if(_l.Size()>0){
        string ret = _l.Value(0);
        _l.Remove(0);
        return ret;
    }
    return "";
    pthread_mutex_unlock(&_objectMutex); 
}



void
DStringQueue::Clear() {
    _l.Clear();
}

int
DStringQueue::Size() {
    return _l.Size();
}

} // namespace diamond
