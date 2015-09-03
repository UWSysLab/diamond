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
#define ERR_REMOTE 4
#define ERR_NOT_PERFORMED 5

#define WRITE_ALWAYS 1
#define WRITE_IFF_NOT_EXIST 2
#define WRITE_IFF_EXIST 3

#include <string>
#include <semaphore.h>
#include <stdlib.h>
#include "hiredis.h"
#include "adapters/libevent.h"

namespace diamond {



class Cloud
{
public:

    static Cloud* Instance();
    static struct event_base * GetEventBase();
    static void SetConnected(bool);
    static bool GetConnected();

    static int GetCallbackRet();
    static void SetCallbackRet(int);
    static std::string &GetCallbackReply();
    static void SetCallbackReply(std::string& reply);

    int Connect(const std::string &host);
    int AsyncConnect(const std::string &host);
    bool IsConnected();
    int Read(const std::string &key, std::string &value);
    int Write(const std::string &key, const std::string &value);
    int Write(const std::string &key, const std::string &value, int write_cond, long expire_ms);
    int Push(const std::string &key, const std::string &value);
    int Pop(const std::string &key, std::string &value, bool block);

    int RunOnServer(const std::string &script, const std::string &resource, const std::string &value);

private:
    static Cloud *_instance;
    static struct event_base *_base;
    static bool _connected;

    redisAsyncContext *_redisAsync;
    sem_t hiredis_sem;
    static std::string _callbackReply;
    static int _callbackRet;

    // contructor and destructor should be private in Singleton
    Cloud();
    virtual ~Cloud();
};

extern Cloud* cloudstore;

void evalCallback(redisAsyncContext *c, void *r, void *privdata);
void writeCallback(redisAsyncContext *c, void *r, void *privdata);
void readCallback(redisAsyncContext *c, void *r, void *privdata);
void connectCallback(const redisAsyncContext *c, int status);
void disconnectCallback(const redisAsyncContext *c, int status);
void popCallback(redisAsyncContext *c, void *r, void *privdata);
void pushCallback(redisAsyncContext *c, void *r, void *privdata);

} // namespace diamond

#endif 
