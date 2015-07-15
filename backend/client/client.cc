// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * client/client.cc:
 *   Diamond client
 *
 **********************************************************************/

#include "client/client.h"

namespace diamond {

using namespace std;

Client::Client(string configPath)
{
}

Client::~Client()
{
    
}

uint64_t*
Client::Map(uint64_t key)
{
    return &store[key];
}

uint64_t
Client::Read(uint64_t key)
{
    return store[key];
}

void
Client::Write(uint64_t key, uint64_t value)
{
    store[key] = value;
}

} // namespace diamond
