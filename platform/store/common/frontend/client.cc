// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * common/client.cc:
 *   Interface for a multiple shard transactional client.
 *
 **********************************************************************/

#include "store/common/frontend/client.h"

using namespace std;

Client::Client() { }
Client::~Client() { }

void
Client::Begin()
{
    Panic("BEGIN Unimplemented");
}
    
int
Client::Get(const string &key, string &value)
{
    Panic("GET Unimplemented");
    return 0;
}

int
Client::MultiGet(const vector<string> &keys, map<string, string> &values)
{
    Panic("GET Unimplemented");
    return 0;
}

int
Client::Put(const string &key, const string &value)
{
    Panic("PUT Unimplemented");
    return 0;
}

bool
Client::Commit()
{
    Panic("COMMIT Unimplemented");
    return false;
}
    
void
Client::Abort()
{
    Panic("ABORT Unimplemented");
}

vector<int>
Client::Stats()
{
    Panic("STATS Unimplemented");
    vector<int> v;
    return v;
}
