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
#define ERR_EMPTY 6

#define WRITE_ALWAYS 1
#define WRITE_IFF_NOT_EXIST 2
#define WRITE_IFF_EXIST 3

#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <csignal>

#include <semaphore.h>
#include <stdlib.h>
#include "hiredis.h"

#include <signal.h>
#include <unistd.h>
#include <assert.h>
//#include "lib/assert.h"
#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include <unordered_map>
#include <vector>
#include <set>
#include <map>
#include <list>

namespace diamond {



class Cloud
{
public:
    static Cloud* Instance();
    static void SetConnected(bool);
    static bool GetConnected();


    int Connect(const std::string &host);
    int Reconnect(const std::string &host);
    bool IsConnected();
    int MultiGet(const std::vector<std::string> &keys, std::vector<std::string> &values);
    int Read(const std::string &key, std::string &value);
    int ReadMulti(const std::list<std::string> &keys, std::list<std::string> &values);

    int Write(const std::string &key, const std::string &value);
    int Write(const std::string &key, const std::string &value, int write_cond, long expire_ms);
    int Rpush(const std::string &key, const std::string &value);
    int Lpop(const std::string &key, std::string &value, bool block);
    int Multi(void);
    int Exec(void);
    int Watch(const std::string &key);
    int Unwatch(void);
    int Wait(const std::set<std::string> &keys, std::map<std::string, std::string> &lastReadValues);

    int RunOnServer(const std::string &script, const std::string &resource, const std::string &value);

    redisContext* GetRedisContext();

private:
    static Cloud *_instance;
    static bool _connected;

    // Keep one connection per thread
    std::unordered_map<int, redisContext*> _redisContexts;

    //redisContext *_redis;

    // contructor and destructor should be private in Singleton
    Cloud();
    virtual ~Cloud();
};


long getThreadID();

#ifdef DEBUG_HIREDIS

#define LOG_REQUEST(req, cmd) { printf("[%ld] Issuing " #req  " request (command = \"%s\")\n", getThreadID(), cmd);}
#define LOG_REPLY(req, reply) { printf("[%ld] Received " #req " reply (reply->type = %d, reply->str = \"%s\")\n", getThreadID(), reply->type, reply->str);}

#else // DEBUG_HIREDIS

#define LOG_REQUEST(req, cmd) { }
#define LOG_REPLY(req, reply) { }

#endif // DEBUG_HIREDIS


extern Cloud* cloudstore;

} // namespace diamond

#endif 
