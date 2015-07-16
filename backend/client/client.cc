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
using namespace redox;

Client::Client(string &host, int port)
{
    rdx.connect(host, port);
}

Client::~Client()
{
  rdx.disconnect();
}

int
Client::Map(int *addr, string &key)
{
    string value;
    try {
        value = rdx.get(key);
    } catch (exception &e) {
        return -1;
    }

    cache[key] = value;
    *addr = (uint64_t) atol(value.c_str());
    return 0;
}

int
Client::Read(string &key)
{
    string value;
    try {
        value = rdx.get(key);
    } catch (exception &e) {
        return 0;
    }
    cache[key] = value;
    
    return (uint64_t) atol(value.c_str());
}

void
Client::Write(string &key, int value)
{
    char buf[50];
    sprintf(buf, "%i", value);
    cache[key] = (string)buf;
    rdx.set(key, cache[key]);
}

} // namespace diamond
