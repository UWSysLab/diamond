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


// Initializing static variables of Cloud
Cloud* Cloud::_instance = NULL; 
bool Cloud::_connected = false;

Cloud::Cloud()
{
    //Connect("coldwater.cs.washington.edu");
    Connect("localhost");
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

// void
// Cloud::NewInstance(void)
// {
//     _instance = new Cloud();
//     _instance->Reconnect("localhost");    
// }


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

int
Cloud::Connect(const std::string &host)
{
    if (_connected) {
        return ERR_OK;
    }
    printf("Connecting...\n");
    _redis = redisConnect(host.c_str(), 6379);
    printf("Connected.\n");
    if (_redis != NULL && _redis->err == 0) {
        _connected = true;
        return ERR_OK;
    } else {
        return ERR_UNAVAILABLE;
    }
}

// int
// Cloud::Reconnect(const std::string &host)
// {
//     printf("Reconnecting...\n");
//     _redis = redisConnect(host.c_str(), 6379);
//     printf("Reconnected.\n");
//     if (_redis != NULL && _redis->err == 0) {
//         _connected = true;
//         return ERR_OK;
//     } else {
//         return ERR_UNAVAILABLE;
//     }
// }
// 

bool
Cloud::IsConnected()
{
    return _connected;
}


int
Cloud::Read(const string &key, string &value)
{
    char cmd[256];
    redisReply *reply;

    if (!_connected) {
        return ERR_UNAVAILABLE;
    }

    sprintf(cmd, "GET %s", key.c_str());
    LOG_REQUEST("GET", cmd);
    reply = (redisReply *)redisCommand(_redis, cmd);
    LOG_REPLY("GET", reply);


    if (reply == NULL){
        Panic("reply == null");
    }

    if (reply->type == REDIS_REPLY_STRING) {
        value = string(reply->str);
        freeReplyObject(reply);
        return ERR_OK;
    }else if(reply->type == REDIS_REPLY_NIL){
        // XXX: Return the same code?
        freeReplyObject(reply);
        return ERR_OK;
    }
    return ERR_OK;
}

int
Cloud::Write(const string &key, const string &value)
{
    char cmd[256];
    redisReply *reply;

    if (!_connected) {
        return ERR_UNAVAILABLE;
    }

    sprintf(cmd, "SET %s %s", key.c_str(), value.c_str());
    LOG_REQUEST("SET", cmd);
    reply = (redisReply *)redisCommand(_redis, cmd);
    LOG_REPLY("SET", reply);

    if (reply == NULL) {
        Panic("reply == null");
    }

    freeReplyObject(reply);
    return ERR_OK;
}



int
Cloud::Write(const string &key, const string &value, int write_cond, long expire_ms)
{
    char cmd[256];
    redisReply *reply;

    if (!_connected) {
        return ERR_UNAVAILABLE;
    }

    std::string write_cond_option;
    switch(write_cond){
        case WRITE_ALWAYS:
            write_cond_option = "";
            break;
        case WRITE_IFF_EXIST:
            write_cond_option = " XX";
            break;
        case WRITE_IFF_NOT_EXIST:
            write_cond_option = " NX";
            break;
        default:
            Panic("Write condition invalid");
    }

    // FIXME: command is not currently escaped (using "%s" as the format string is not supported)
    sprintf(cmd, "SET %s %s PX %ld%s", key.c_str(), value.c_str(), expire_ms, write_cond_option.c_str());
    LOG_REQUEST("SET NX/XX", cmd);
    reply = (redisReply *)redisCommand(_redis, cmd);
    LOG_REPLY("SET NX/XX", reply);


    if (reply == NULL) {
        Panic("reply == null");
    }

    if (reply->type == REDIS_REPLY_NIL){
        freeReplyObject(reply);
        return ERR_NOT_PERFORMED;
    }else{
        freeReplyObject(reply);
        return ERR_OK;
    }
}


int 
Cloud::RunOnServer(const string &script, const string &resource, const string &value)
{
    char cmd[256];
    redisReply *reply;

    if (!_connected) {
        return ERR_UNAVAILABLE;
    }

    sprintf(cmd, "EVAL %s 1 %s %s", script.c_str(), resource.c_str(), value.c_str());
    LOG_REQUEST("EVAL", cmd);
    reply = (redisReply *)redisCommand(_redis,  "EVAL %s 1 %s %s", script.c_str(), resource.c_str(), value.c_str());
    LOG_REPLY("EVAL", reply);

    if (reply == NULL) {
        Panic("reply == null");
    }

    freeReplyObject(reply);
    return ERR_OK;
}

int
Cloud::Push(const string &key, const string &value)
{
    char cmd[256];
    redisReply *reply;

    if (!_connected) {
        return ERR_UNAVAILABLE;
    }

    sprintf(cmd, "RPUSH %s %s", key.c_str(), value.c_str());
    LOG_REQUEST("RPUSH", cmd);
    reply = (redisReply *)redisCommand(_redis, cmd);
    LOG_REPLY("RPUSH", reply);

    if (reply == NULL) {
        Panic("reply == null");
    }

    freeReplyObject(reply);
    return ERR_OK;
}

int
Cloud::Pop(const string &key, string &value, bool block)
{
    char cmd[256];
    redisReply *reply;

    if (!_connected) {
        return ERR_UNAVAILABLE;
    }


    if(block){
        sprintf(cmd, "BLPOP %s", key.c_str());
        LOG_REQUEST("BLPOP", cmd);
        reply = (redisReply *)redisCommand(_redis, cmd);
        LOG_REPLY("BLPOP", reply);
    }else{
        sprintf(cmd, "LPOP %s", key.c_str());
        LOG_REQUEST("LPOP", cmd);
        reply = (redisReply *)redisCommand(_redis, cmd);
        LOG_REPLY("LPOP", reply);
    }

    if (reply == NULL) {
        Panic("reply == null");
    }

    freeReplyObject(reply);
    return ERR_OK;
}



// 
// int 
// Cloud::RunOnServer(const string &script, const string &resource, const string &value)
// {
//     char s[256];
//     if (!_connected) {
//         return ERR_UNAVAILABLE;
//     }
// 
//     sem_wait(&hiredis_sem); // Limit the number of concurrent hiredis calls
//     
//     sprintf(s, "EVAL %s 1 %s %s", script.c_str(), resource.c_str(), value.c_str());
// 
//     int res = redisAsyncCommand(_redisAsync, evalCallback, NULL, "EVAL %s 1 %s %s", script.c_str(), resource.c_str(), value.c_str());
// 
// 
//     if (res != REDIS_OK) {
//         Panic("Unable to issue EVAL command");
//     }
// 
//     event_base_dispatch(Cloud::GetEventBase());
//     sem_post(&hiredis_sem);
//     return Cloud::GetCallbackRet();
// }
// 
// 
// int
// Cloud::Push(const string &key, const string &value)
// {
//     if (!_connected) {
//         return ERR_UNAVAILABLE;
//     }
// 
//     sem_wait(&hiredis_sem); // Limit the number of concurrent hiredis calls
//     int res = redisAsyncCommand(_redisAsync, pushCallback, NULL, "RPUSH %s %s", key.c_str(), value.c_str());
// 
//     if (res != REDIS_OK) {
//         Panic("Unable to issue RPUSH command");
//     }
// 
//     event_base_dispatch(Cloud::GetEventBase());
//     sem_post(&hiredis_sem);
//     return Cloud::GetCallbackRet();
// }
// 
// int
// Cloud::Pop(const string &key, string &value, bool block)
// {
//     int res;
// 
//     if (!_connected) {
//         return ERR_UNAVAILABLE;
//     }
// 
//     sem_wait(&hiredis_sem); // Limit the number of concurrent hiredis calls
//     if(block){
//         res = redisAsyncCommand(_redisAsync, popCallback, NULL, "BLPOP %s 0", key.c_str());
//     }else{
//         res = redisAsyncCommand(_redisAsync, popCallback, NULL, "LPOP %s", key.c_str());
//     }
// 
//     if (res != REDIS_OK) {
//         Panic("Unable to issue BLPOP/LPOP command");
//     }
// 
//     event_base_dispatch(Cloud::GetEventBase());
//     sem_post(&hiredis_sem);
//     return Cloud::GetCallbackRet();
// }


// Callback functions 

// // hiredis/src/read.h:#define REDIS_REPLY_STRING 1
// // hiredis/src/read.h:#define REDIS_REPLY_ARRAY 2
// // hiredis/src/read.h:#define REDIS_REPLY_INTEGER 3
// // hiredis/src/read.h:#define REDIS_REPLY_NIL 4
// // hiredis/src/read.h:#define REDIS_REPLY_STATUS 5
// // hiredis/src/read.h:#define REDIS_REPLY_ERROR 6
// 
// 
// void evalCallback(redisAsyncContext *c, void *r, void *privdata) {
//     redisReply *reply = (redisReply* )r;
//     if (reply == NULL) return;
// 
//     if(reply->type == REDIS_REPLY_INTEGER){
//         //char str[128];
//         //string* reply = (string*) new string;
//     //  Cloud::Instance()->GetCallbackReply() = string("");
//         //*reply;
//     }else{
//         Panic("NYI: eval returned something other than a string");
//     }
//     Cloud::SetCallbackRet(ERR_OK);
//     event_base_loopbreak(Cloud::GetEventBase());
// 
//     // Reply is freed by the callee
// }
// 
// // FIXME: Not sure we're allocating correctly the string with the value read
// void readCallback(redisAsyncContext *c, void *r, void *privdata) {
//     redisReply *reply = (redisReply* )r;
//     if (reply == NULL) return;
// 
//     if(reply->type == REDIS_REPLY_STRING){
//         //printf("Read callback: argv[%s]: %s\n", (char*)privdata, reply->str);
//         string s = string(reply->str);
//         Cloud::SetCallbackReply(s);
//         Cloud::SetCallbackRet(ERR_OK);
//     }else{
//         Cloud::SetCallbackRet(ERR_NETWORK); // Is this the right error if we get NIL
//     }
//     event_base_loopbreak(Cloud::GetEventBase());
// 
//     // reply is freed by the callee
// }
// 
// void writeCallback(redisAsyncContext *c, void *r, void *privdata) {
//     redisReply *reply = (redisReply* )r;
//     if (reply == NULL) {
//         Panic("reply == NULL");
//     }
// 
//     //printf("Write callback: argv[%s]: %s (type: %d)\n", (char*)privdata, reply->str, reply->type);
// 
//     if(reply->type == REDIS_REPLY_ERROR){
//         Cloud::SetCallbackRet(ERR_REMOTE);
//     }else if (reply->type == REDIS_REPLY_NIL){
//         Cloud::SetCallbackRet(ERR_NOT_PERFORMED);
//     }else{
//         Cloud::SetCallbackRet(ERR_OK);
//     }
// 
//     event_base_loopbreak(Cloud::GetEventBase());
// 
//     // reply is freed by the callee
// }
// 
// void popCallback(redisAsyncContext *c, void *r, void *privdata) {
//     redisReply *reply = (redisReply* )r;
//     if (reply == NULL) return;
// 
//     if(reply->type == REDIS_REPLY_STRING){
//         //printf("Pop callback: argv[%s]: %s\n", (char*)privdata, reply->str);
//         string s = string(reply->str);
//         Cloud::SetCallbackReply(s);
//         Cloud::SetCallbackRet(ERR_OK);
//     }else{
//         Cloud::SetCallbackRet(ERR_NETWORK); // Is this the right error if we get NIL
//     }
//     event_base_loopbreak(Cloud::GetEventBase());
// 
//     // reply is freed by the callee
// }
// 
// void pushCallback(redisAsyncContext *c, void *r, void *privdata) {
//     redisReply *reply = (redisReply* )r;
//     if (reply == NULL) {
//         Panic("reply == NULL");
//     }
// 
//     //printf("Push callback: argv[%s]: %s (type: %d)\n", (char*)privdata, reply->str, reply->type);
// 
//     if(reply->type == REDIS_REPLY_ERROR){
//         Cloud::SetCallbackRet(ERR_REMOTE);
//     }else if (reply->type == REDIS_REPLY_NIL){
//         Cloud::SetCallbackRet(ERR_NOT_PERFORMED);
//     }else{
//         Cloud::SetCallbackRet(ERR_OK);
//     }
//     event_base_loopbreak(Cloud::GetEventBase());
// 
//     // reply is freed by the callee
// }
// 
// 
// void connectCallback(const redisAsyncContext *c, int status) {
//     if (status != REDIS_OK) {
//         printf("Error: %s\n", c->errstr);
//         return;
//     }
//     
//     assert(!Cloud::GetConnected());
// 
//     printf("Connected...\n");
//     Cloud::SetConnected(true);
// 
//     Cloud::SetCallbackRet(ERR_OK);
//     event_base_loopbreak(Cloud::GetEventBase());
// }
// 
// void disconnectCallback(const redisAsyncContext *c, int status) {
//     if (status != REDIS_OK) {
//         printf("Error: %s\n", c->errstr);
//         return;
//     }
//     printf("Disconnected...\n");
//     Cloud::SetConnected(false);
// 
//     Cloud::SetCallbackRet(ERR_OK);
//     event_base_loopbreak(Cloud::GetEventBase());
// }
// 


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


long gettid(){
    long tid;
    tid = syscall(SYS_gettid);
    return tid;
}


