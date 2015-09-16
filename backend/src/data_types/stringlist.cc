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
    string s;
    cloudstore->Read(_key, s);
    Deserialize(s);
    return _vec;
}

int
DStringList::Index(const string val)
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

string
DStringList::Value(const int index)
{
    string s;
    cloudstore->Read(_key, s);
    Deserialize(s);
    return _vec.at(index);
}

void
DStringList::Append(const string val)
{
    string s;
    cloudstore->Read(_key, s);
    Deserialize(s);
    _vec.push_back(val);
    cloudstore->Write(_key, Serialize());
}

void
DStringList::Append(const vector<string> &vec)
{
    string s;
    cloudstore->Read(_key, s);
    Deserialize(s);
    for (auto e : vec) {
        _vec.push_back(e);
    }
    cloudstore->Write(_key, Serialize());
}

void
DStringList::Insert(const int index, const string val) {
    string s;
    cloudstore->Read(_key, s);
    Deserialize(s);
    _vec.insert(index + _vec.begin(), val);
    cloudstore->Write(_key, Serialize());
}

void
DStringList::Erase(const int index) {
    string s;
    cloudstore->Read(_key, s);
    Deserialize(s);
    _vec.erase(index + _vec.begin());
    cloudstore->Write(_key, Serialize());
}

void
DStringList::Remove(const string val) {
    string s;
    cloudstore->Read(_key, s);
    Deserialize(s);
    _vec.erase(Index(val) + _vec.begin());
    cloudstore->Write(_key, Serialize());
}

void
DStringList::Clear() {
    string s;
    cloudstore->Read(_key, s);
    Deserialize(s);
    _vec.clear();
    cloudstore->Write(_key, Serialize());
}

int
DStringList::Size() {
    string s;
    cloudstore->Read(_key, s);
    Deserialize(s);
    return _vec.size();
}

} // namespace diamond
