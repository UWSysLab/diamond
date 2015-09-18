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
DStringList::Serialize()
{
    string ret = "";
        
    for (auto i : _vec) {
        char buf[50];
        sprintf(buf, "%s\n", i.c_str());
        ret += string(buf);
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
    Pull();
    return _vec;
}

int
DStringList::Index(const string val)
{
    Pull();
    for (auto it = _vec.begin(); it != _vec.end(); it++) {
        if (*it == val) {
            return (it - _vec.begin());
        }
    }
    return -1;
}

string
DStringList::Value(const int index)
{
    Pull();
    return _vec.at(index);
}

void
DStringList::Append(const string val)
{
    Pull();
    _vec.push_back(val);
    Push();
}

void
DStringList::Append(const vector<string> &vec)
{
    Pull();
    for (auto e : vec) {
        _vec.push_back(e);
    }
    Push();
}

void
DStringList::Insert(const int index, const string val) {
    Pull();
    _vec.insert(index + _vec.begin(), val);
    Push();
}

void
DStringList::Erase(const int index) {
    Pull();
    _vec.erase(index + _vec.begin());
    Push();
}

void
DStringList::Remove(const string val) {
    Pull();
    _vec.erase(Index(val) + _vec.begin());
    Push();
}

void
DStringList::Clear() {
    Pull();
    _vec.clear();
    Push();
}

int
DStringList::Size() {
    Pull();
    return _vec.size();
}

} // namespace diamond
