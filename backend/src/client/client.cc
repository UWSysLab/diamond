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

Client::~Client()
{
 
}
    
int
Client::Connect(const std::string &host)
{
    if (_connected) {
        return RPC_OK;
    }
    _redis = redisConnect(host.c_str(), 6379);
    if (_redis != NULL && _redis->err == 0) {
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

    redisReply *reply = (redisReply *)redisCommand(_redis, "GET %s", key.c_str());

    if (reply != NULL) {
        if (reply->type == REDIS_REPLY_STRING) {
            value = string(reply->str);
            freeReplyObject(reply);
            return RPC_OK;
        }
    }

    freeReplyObject(reply);
    return RPC_ERR;
}

int
Client::Write(const string &key, const string &value)
{
    if (!_connected) {
        return RPC_UNCONNECTED;
    }

    redisReply *reply = (redisReply *)redisCommand(_redis, "SET %s %s", key.c_str(), value.c_str());

    if (reply != NULL) {
        freeReplyObject(reply);
        return RPC_OK;
    }

    freeReplyObject(reply);
    return RPC_OK;
}

} // namespace diamond
