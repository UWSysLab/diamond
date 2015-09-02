// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * storage/cloud.h:
 *   Diamond logic for accessing cloud storage
 *
 **********************************************************************/

#ifndef _CLOUD_H_
#define _CLOUD_H_

#define ERR_OK 0
#define ERR_UNAVAILABLE 1
#define ERR_NETWORK 2
#define ERR_LOCAL 3

#include <string>
#include <stdlib.h>
#include "hiredis.h"
#include "adapters/libevent.h"

namespace diamond {


class Cloud
{
public:

	static Cloud* Instance();
	static struct event_base * GetEventBase();
	static int GetCallbackRet();
	static void SetCallbackRet(int);
	static std::string *GetCallbackReply();
	static void SetCallbackReply(std::string *reply);
	static void SetConnected(bool);
	static bool GetConnected();


    int Connect(const std::string &host);
	int AsyncConnect(const std::string &host);
    bool IsConnected();
    int Read(const std::string &key, std::string &value);
    int Write(const std::string &key, const std::string &value);
	int Subscribe(std::string &channel);
	int Unsubscribe(std::string &channel);
	int Publish(std::string &channel, std::string &message);

private:
	static Cloud *_instance;
	static struct event_base *_base;
	static int _callbackRet;
	static std::string *_callbackReply;
    static bool _connected;

    redisContext *_redis; // obsolete: ?
	redisAsyncContext *_redisAsync;

	// contructor and destructor should be private in Singleton
    Cloud();
    virtual ~Cloud();
};

extern Cloud* cloudstore;

void writeCallback(redisAsyncContext *c, void *r, void *privdata);
void readCallback(redisAsyncContext *c, void *r, void *privdata);
void connectCallback(const redisAsyncContext *c, int status);
void disconnectCallback(const redisAsyncContext *c, int status);

} // namespace diamond

#endif 
