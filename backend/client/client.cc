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

Client::~Client()
{
    _redis.disconnect();
}
int
Client::Connect(const std::string &host)
{
    if (_redis.connect(host)) {
	_connected = true;
	return RPC_OK;
    } else {
	return RPC_UNCONNECTED;
    }
}

bool
Client::IsConnected()
{
    return _connected;
}
    
int
Client::Read(const string &key, string &value)
{
    if (!_connected) {
	return RPC_UNCONNECTED;
    }

    try {
	value = redis.get(key);
    } catch (exception &e) {
	return RPC_ERR;
    }

    return RPC_OK;
}

int
Client::Write(const string &key, const string &value)
{
    if (!_connected) {
	return RPC_UNCONNECTED;
    }

    if (redis.set(key, value)) {
	return RPC_OK;
    } else {
	return RPC_ERR;
    }
}

} // namespace diamond
