// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * storage/cloud.cc:
 *   Diamond client for cloud storage
 *
 **********************************************************************/

#include "storage/cloud.h"

namespace diamond {

using namespace std;

Cloud::~Cloud()
{
 
}
    
int
Cloud::Connect(const std::string &host)
{
    if (_connected) {
        return ERR_OK;
    }
    _redis = redisConnect(host.c_str(), 6379);
    if (_redis != NULL && _redis->err == 0) {
        _connected = true;
        return ERR_OK;
    } else {
        return ERR_UNAVAILABLE;
    }
}

bool
Cloud::IsConnected()
{
    return _connected;
}
    
int
Cloud::Read(const string &key, string &value)
{
    if (!_connected) {
        return ERR_UNAVAILABLE;
    }

    redisReply *reply = (redisReply *)redisCommand(_redis, "GET %s", key.c_str());

    if (reply != NULL) {
        if (reply->type == REDIS_REPLY_STRING) {
            value = string(reply->str);
            freeReplyObject(reply);
            return ERR_OK;
        }
    }

    freeReplyObject(reply);
    return ERR_NETWORK;
}

int
Cloud::Write(const string &key, const string &value)
{
    if (!_connected) {
        return ERR_UNAVAILABLE;
    }

    redisReply *reply = (redisReply *)redisCommand(_redis, "SET %s %s", key.c_str(), value.c_str());

    if (reply != NULL) {
        freeReplyObject(reply);
        return ERR_OK;
    }

    freeReplyObject(reply);
    return ERR_OK;
}

} // namespace diamond
