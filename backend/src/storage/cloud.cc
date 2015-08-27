// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * storage/cloud.cc:
 *   Diamond client for cloud storage
 *
 **********************************************************************/

#include "storage/cloud.h"
#include <signal.h>
#include <unistd.h>
#include <assert.h>
#include "lib/assert.h"

namespace diamond {

using namespace std;


// Initializing static variables of Cloud
Cloud* Cloud::_instance = NULL; 
struct event_base* Cloud::_base = NULL;
int Cloud::_callbackRet = 0;
std::string *Cloud::_callbackReply = 0;
bool Cloud::_connected = false;

Cloud::Cloud()
{
	AsyncConnect("coldwater.cs.washington.edu");
}

Cloud::~Cloud()
{
 
}

Cloud*
Cloud::Instance(void)
{
	if(!_instance){
		_instance = new Cloud();
	}
	return _instance;
}

struct event_base *
Cloud::GetEventBase(void)
{
	assert(_base);
	return _base;
}    

int
Cloud::GetCallbackRet()
{
	return _callbackRet;
}

void
Cloud::SetCallbackRet(int ret)
{
	_callbackRet = ret;
}

void
Cloud::SetCallbackReply(std::string *reply)
{
	_callbackReply = reply;
}

std::string *
Cloud::GetCallbackReply()
{
	return _callbackReply;
}

void
Cloud::SetConnected(bool con)
{
	_connected = con;
}

bool
Cloud::GetConnected()
{
	return _connected;
}

// int
// Cloud::Connect(const std::string &host)
// {
//     if (_connected) {
//         return ERR_OK;
//     }
//     _redis = redisConnect(host.c_str(), 6379);
//     if (_redis != NULL && _redis->err == 0) {
//         _connected = true;
//         return ERR_OK;
//     } else {
//         return ERR_UNAVAILABLE;
//     }
// }
// 


int
Cloud::AsyncConnect(const std::string &host)
{
    signal(SIGPIPE, SIG_IGN);
	Cloud::_base = event_base_new();

	printf("Connecting (\"%s\")\n", host.c_str());
    _redisAsync = redisAsyncConnect(host.c_str(), 6379);
    if (_redisAsync->err) {
        /* Let *c leak for now... */
        printf("Error: %s\n", _redisAsync->errstr);
        return 1;
    }

    redisLibeventAttach(_redisAsync, Cloud::GetEventBase());
    redisAsyncSetConnectCallback(_redisAsync, connectCallback);
    redisAsyncSetDisconnectCallback(_redisAsync, disconnectCallback);

	// Wait for connection
    event_base_dispatch(Cloud::GetEventBase());
    return Cloud::GetCallbackRet();
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

    SetCallbackReply(&value);
	assert(_redisAsync);
	int res = redisAsyncCommand(_redisAsync, readCallback, NULL, "GET %s", key.c_str());


    if (res != REDIS_OK) {
        Panic("Unable to issue GET command");
    }

    event_base_dispatch(Cloud::GetEventBase());
    return Cloud::GetCallbackRet();
}

int
Cloud::Write(const string &key, const string &value)
{
    if (!_connected) {
        return ERR_UNAVAILABLE;
    }

	assert(_redisAsync);
    int res = redisAsyncCommand(_redisAsync, writeCallback, NULL, "SET %s %s", key.c_str(), value.c_str());


    if (res != REDIS_OK) {
        Panic("Unable to issue SET command");
    }

    event_base_dispatch(Cloud::GetEventBase());
    return Cloud::GetCallbackRet();
}

    
int
Cloud::Subscribe(string &channel)
{

    if (!_connected) {
        return ERR_UNAVAILABLE;
    }

    redisReply *reply = (redisReply *)redisCommand(_redis, "SUBSCRIBE %s", channel.c_str());

    if (reply != NULL) {
        freeReplyObject(reply);
        return ERR_OK;
    }

    freeReplyObject(reply);
    return ERR_OK;

}

int
Cloud::Unsubscribe(string &channel)
{

    if (!_connected) {
        return ERR_UNAVAILABLE;
    }
	assert(0);
    redisReply *reply = (redisReply *)redisCommand(_redis, "UNSUBSCRIBE %s", channel.c_str());

    if (reply != NULL) {
        freeReplyObject(reply);
        return ERR_OK;
    }

    freeReplyObject(reply);
    return ERR_OK;

}

int 
Cloud::Publish(string &channel, string &message)
{
    if (!_connected) {
        return ERR_UNAVAILABLE;
    }

	assert(0);
    redisReply *reply = (redisReply *)redisCommand(_redis, "PUBLISH %s %s", channel.c_str(), message.c_str());

    if (reply != NULL) {
        freeReplyObject(reply);
        return ERR_OK;
    }

    freeReplyObject(reply);
    return ERR_OK;
}


// Callback functions 

void readCallback(redisAsyncContext *c, void *r, void *privdata) {
    redisReply *reply = (redisReply* )r;
    if (reply == NULL) return;

	if(reply->type == REDIS_REPLY_STRING){
	    //printf("Read callback: argv[%s]: %s\n", (char*)privdata, reply->str);
		*Cloud::GetCallbackReply() = string(reply->str);
	}else{
		Panic("NYI: read returned something other than a string");
	}
	Cloud::SetCallbackRet(ERR_OK);
	event_base_loopbreak(Cloud::GetEventBase());

	// reply is freed by the callee
}

void writeCallback(redisAsyncContext *c, void *r, void *privdata) {
    redisReply *reply = (redisReply* )r;
    if (reply == NULL) return;
    //printf("Write callback: argv[%s]: %s\n", (char*)privdata, reply->str);

	Cloud::SetCallbackRet(ERR_OK);
	event_base_loopbreak(Cloud::GetEventBase());

	// reply is freed by the callee
}

void connectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
	
	assert(!Cloud::GetConnected());

	printf("Connected...\n");
	Cloud::SetConnected(true);

	Cloud::SetCallbackRet(ERR_OK);
	event_base_loopbreak(Cloud::GetEventBase());
}

void disconnectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
    printf("Disconnected...\n");
	Cloud::SetConnected(false);

	Cloud::SetCallbackRet(ERR_OK);
	event_base_loopbreak(Cloud::GetEventBase());
}



} // namespace diamond



// int
// Cloud::Read(const string &key, string &value)
// {
//     if (!_connected) {
//         return ERR_UNAVAILABLE;
//     }
// 
//     redisReply *reply = (redisReply *)redisCommand(_redis, "GET %s", key.c_str());
// 
//     if (reply != NULL) {
//         if (reply->type == REDIS_REPLY_STRING) {
//             value = string(reply->str);
//             freeReplyObject(reply);
//             return ERR_OK;
//         }
//     }
// 
//     freeReplyObject(reply);
//     return ERR_NETWORK;
// }
// 
// int
// Cloud::Write(const string &key, const string &value)
// {
//     if (!_connected) {
//         return ERR_UNAVAILABLE;
//     }
// 
//     redisReply *reply = (redisReply *)redisCommand(_redis, "SET %s %s", key.c_str(), value.c_str());
// 
//     if (reply != NULL) {
//         freeReplyObject(reply);
//         return ERR_OK;
//     }
// 
//     freeReplyObject(reply);
//     return ERR_OK;
// }
// 


