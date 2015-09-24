// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * storage/cloud.cc:
 *   Diamond client for cloud storage
 *
 **********************************************************************/

#include "storage/cloud.h"
#include "lib/assert.h"
#include <string>
#include <vector>
#include <set>
#include <map>

#include <async.h>
#include <adapters/libuv.h>

namespace diamond {

using namespace std;


// Initializing static variables of Cloud
Cloud* Cloud::_instance = NULL; 
bool Cloud::_connected = false;

std::string serverAddress;
//std::string serverAddress = "coldwater.cs.washington.edu";
//std::string serverAddress = "localhost";


static const string notificationChannelPrefix = "__keyspace@0__:";

typedef struct structPubsubWaiter{
    std::set<string> channelsSubscribed;
    pthread_cond_t condChannelSubscribed = PTHREAD_COND_INITIALIZER;

    bool updated = false;
    pthread_cond_t condUpdated = PTHREAD_COND_INITIALIZER;

    std::map<string, string>* lastReadValues;    
} pubsubWaiter;


static int asyncConnectionSigleton = 0;
static redisAsyncContext *asyncContext = NULL;
static std::map<std::string, std::set<pubsubWaiter *> > psWaiters;
static uv_loop_t* loop;
// mutex protects the global asyncConnection variables including the hiredis context
static pthread_mutex_t  asyncConnectionMutex = PTHREAD_MUTEX_INITIALIZER; 
// semaphore to signal that we're connected 
static sem_t semAsyncConnected;
static uv_async_t async;

int connectAsync();
void pubsubCallback(redisAsyncContext *c, void *r, void *privdata);
#ifdef __ANDROID__
void dummy_async(uv_async_t *handle);
#else
void dummy_async(uv_async_t *handle, int v);
#endif


Cloud::Cloud(const string &server)
{
    Connect(server);
}

Cloud::~Cloud()
{
 
}

Cloud*
Cloud::Instance(const string &server)
{
    if(!_instance){
        serverAddress = server;
        _instance = new Cloud(server);
    }
    else {
        if (server != serverAddress) {
            Panic("Cloud is already connected to another server");
        }
    }
    return _instance;
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

int
Cloud::Connect(const std::string &host)
{
    redisContext* redis;
    long threadID;

    connectAsync();

    Notice("Connecting...");
    redis = redisConnect(host.c_str(), 6379);
    Notice("Connected.");
    if (redis != NULL && redis->err == 0) {
        _connected = true;
        threadID = getThreadID();
        _redisContexts[threadID] = redis; 
        return ERR_OK;
    } else {
        Panic("Unable to connect");
        return ERR_UNAVAILABLE;
    }
}

bool
Cloud::IsConnected()
{
    return _connected;
}


redisContext*
Cloud::GetRedisContext()
{
    long threadID = getThreadID();
    auto find = _redisContexts.find(threadID);

    if (find != _redisContexts.end()) {
        return find->second;
    }else{
        Connect(serverAddress);

        find = _redisContexts.find(threadID);
        assert(find != _redisContexts.end());
        return find->second;
    }
}

int
Cloud::MultiGet(const vector<string> & keys, vector<string> & values) {
    redisReply *reply;

    if (values.size() != 0) {
        Panic("Values list not empty");
    }

    values.resize(keys.size());

    vector<const char *> argv(keys.size() + 1);
    vector<size_t> argvlen(keys.size() + 1);

    string mgetCmd("MGET");
    argv[0] = mgetCmd.c_str();
    argvlen[0] = mgetCmd.size();
    for (size_t i = 0; i < keys.size(); i++) {
        argv[i + 1] = keys.at(i).c_str();
        argvlen[i + 1] = keys.at(i).size();
    }
    reply = (redisReply *)redisCommandArgv(GetRedisContext(), keys.size() + 1, &(argv[0]), &(argvlen[0]));

    if (reply == NULL){
        Panic("reply == null");
    }
    if(reply->type != REDIS_REPLY_ARRAY) {
        Panic("MultiGet reply is not an array");
        freeReplyObject(reply);
        return ERR_EMPTY;
    }

    for (size_t i = 0; i < reply->elements; i++) {
        redisReply *subreply = reply->element[i];
        if (subreply->type == REDIS_REPLY_STRING) {
            values.at(i) = string(subreply->str);
        }
        else if (subreply->type == REDIS_REPLY_NIL) {
            values.at(i) = "";
        }
    }

    freeReplyObject(reply);
    return ERR_OK;
}

int
Cloud::Read(const string &key, string &value)
{
    redisReply *reply;

    if (!_connected) {
        return ERR_UNAVAILABLE;
    }

    //sprintf(cmd, "GET %s", key.c_str());
    LOG_REQUEST("GET", "");
    reply = (redisReply *)redisCommand(GetRedisContext(),  "GET %s", key.c_str());
    LOG_REPLY("GET", reply);


    if (reply == NULL){
        Panic("reply == null");
    }

    if (reply->type == REDIS_REPLY_STRING) {
        value = string(reply->str);
        freeReplyObject(reply);
        return ERR_OK;
    }else if(reply->type == REDIS_REPLY_NIL){
        freeReplyObject(reply);
        return ERR_EMPTY;
    }
    freeReplyObject(reply);
    return ERR_OK;
}


int
Cloud::Write(const string &key, const string &value)
{
    redisReply *reply;

    if (!_connected) {
        return ERR_UNAVAILABLE;
    }

    //sprintf(cmd, "SET %s %s", key.c_str(), value.c_str());
    LOG_REQUEST("SET", "");
    reply = (redisReply *)redisCommand(GetRedisContext(),  "SET %s %s", key.c_str(), value.c_str());
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
    redisReply *reply;

    if (!_connected) {
        return ERR_UNAVAILABLE;
    }

    std::string write_cond_option;

    if(write_cond == WRITE_ALWAYS){
        //sprintf(cmd, "SET %s %s PX %ld", key.c_str(), value.c_str(), expire_ms);
        LOG_REQUEST("SET ALWAYS", "");
        reply = (redisReply *)redisCommand(GetRedisContext(), "SET %s %s PX %ld", 
                                    key.c_str(), value.c_str(), expire_ms);
        LOG_REPLY("SET ALWAYS", reply);
    }else{
        switch(write_cond){
            case WRITE_IFF_EXIST:
                write_cond_option = "XX";
                break;
            case WRITE_IFF_NOT_EXIST:
                write_cond_option = "NX";
                break;
            default:
                Panic("Write condition invalid");
        }
        //sprintf(cmd, "SET %s %s PX %ld %s", key.c_str(), value.c_str(), expire_ms, write_cond_option.c_str());
        LOG_REQUEST("SET NX/XX", "");
        reply = (redisReply *)redisCommand(GetRedisContext(), "SET %s %s PX %ld %s", 
                                    key.c_str(), value.c_str(), expire_ms, write_cond_option.c_str());
        LOG_REPLY("SET NX/XX", reply);
    }


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
    redisReply *reply;

    if (!_connected) {
        return ERR_UNAVAILABLE;
    }

    //sprintf(cmd, "EVAL %s 1 %s %s", script.c_str(), resource.c_str(), value.c_str());
    LOG_REQUEST("EVAL", "");
    reply = (redisReply *)redisCommand(GetRedisContext(),  "EVAL %s 1 %s %s", script.c_str(), resource.c_str(), value.c_str());
    LOG_REPLY("EVAL", reply);

    if (reply == NULL) {
        Panic("reply == null");
    }

    freeReplyObject(reply);
    return ERR_OK;
}

int
Cloud::Rpush(const string &key, const string &value)
{
    redisReply *reply;

    if (!_connected) {
        return ERR_UNAVAILABLE;
    }

    //sprintf(cmd, "RPUSH %s %s", key.c_str(), value.c_str());
    LOG_REQUEST("RPUSH", "");
    reply = (redisReply *)redisCommand(GetRedisContext(), "RPUSH %s %s", key.c_str(), value.c_str());
    LOG_REPLY("RPUSH", reply);

    if (reply == NULL) {
        Panic("reply == null");
    }

    freeReplyObject(reply);
    return ERR_OK;
}

int
Cloud::Lpop(const string &key, string &value, bool block)
{
    redisReply *reply;

    if (!_connected) {
        return ERR_UNAVAILABLE;
    }


    if(block){
        //sprintf(cmd, "BLPOP %s %d", key.c_str(), 0);
        LOG_REQUEST("BLPOP", "");
        reply = (redisReply *)redisCommand(GetRedisContext(), "BLPOP %s %d", key.c_str(), 0);
        LOG_REPLY("BLPOP", reply);
    }else{
        //sprintf(cmd, "LPOP %s", key.c_str());
        LOG_REQUEST("LPOP", "");
        reply = (redisReply *)redisCommand(GetRedisContext(), "LPOP %s", key.c_str());
        LOG_REPLY("LPOP", reply);
    }

    if (reply == NULL) {
        Panic("reply == null");
    }

    if (reply->type == REDIS_REPLY_STRING) {
        value = string(reply->str);
        freeReplyObject(reply);
        return ERR_OK;
    }else if(reply->type == REDIS_REPLY_NIL){
        freeReplyObject(reply);
        return ERR_EMPTY;
    }
    return ERR_OK;
}

int
Cloud::Multi(void)
{
    redisReply *reply;

    if (!_connected) {
        return ERR_UNAVAILABLE;
    }

    LOG_REQUEST("MULTI", "");
    reply = (redisReply *)redisCommand(GetRedisContext(), "MULTI");
    LOG_REPLY("MULTI", reply);

    if (reply == NULL) {
        Panic("reply == null");
    }

    freeReplyObject(reply);
    return ERR_OK;
}


int
Cloud::Exec(void)
{
    redisReply *reply;

    if (!_connected) {
        return ERR_UNAVAILABLE;
    }

    LOG_REQUEST("EXEC", "");
    reply = (redisReply *)redisCommand(GetRedisContext(), "EXEC");
    LOG_REPLY("EXEC", reply);

    if (reply == NULL) {
        Panic("reply == null");
    }
    
    if(reply->type == REDIS_REPLY_NIL){
        // Transaction failed
        freeReplyObject(reply);
        return ERR_EMPTY;
    }

    // Transaction succeded
    freeReplyObject(reply);
    return ERR_OK;
}

int
Cloud::Watch(const string &key)
{
    redisReply *reply;

    if (!_connected) {
        return ERR_UNAVAILABLE;
    }

    LOG_REQUEST("WATCH", "");
    reply = (redisReply *)redisCommand(GetRedisContext(), "WATCH %s", key.c_str());
    LOG_REPLY("WATCH", reply);

    if (reply == NULL) {
        Panic("reply == null");
    }

    freeReplyObject(reply);
    return ERR_OK;
}



int
Cloud::Unwatch(void)
{
    redisReply *reply;

    if (!_connected) {
        return ERR_UNAVAILABLE;
    }

    LOG_REQUEST("UNWATCH", "");
    reply = (redisReply *)redisCommand(GetRedisContext(), "UNWATCH");
    LOG_REPLY("UNWATCH", reply);

    if (reply == NULL) {
        Panic("reply == null");
    }

    freeReplyObject(reply);
    return ERR_OK;
}




int
Cloud::Wait(const std::set<std::string> &keys,  std::map<string, string> &lastReadValues)
{
    int res;
    string subChannels;
    pubsubWaiter psWaiter;

    if (!_connected) {
        return ERR_UNAVAILABLE;
    }


    pthread_mutex_lock(&asyncConnectionMutex);

    auto it = keys.begin();
    int first_key = 1;
    for (;it!=keys.end();it++){
        string channel = notificationChannelPrefix + *it;
        if(first_key){
            first_key = 0;
        }else{
            subChannels += " ";
        }
//        printf("psWaiters.size(): %lu\n", psWaiters.size());
        auto psWaitersChannel = &psWaiters[channel];
//        printf("psWaiters.size(): %lu\n", psWaiters.size());
//        printf("Conds.size(): %lu\n", conds->size());
        psWaitersChannel->insert(&psWaiter);
        subChannels+=channel;
//         for(auto iter = psWaiters.begin(); iter != psWaiters.end(); iter++)
//         {
//             string k =  iter->first;
//             auto psWaiterChannel = iter->second;
//             printf("keys: \"%s\" (%d)\n", k.c_str(), psWaiterChannel.size());
//         }
// 
//         printf("psWaitersChannel->size(): %lu\n", psWaitersChannel->size());
    }
    printf("WAIT(): subscribing to channels: \"%s\"\n", subChannels.c_str());



    LOG_REQUEST("ASYNC SUBSCRIBE", "");
    res = redisAsyncCommand(asyncContext, pubsubCallback, (char*)"end-1", "SUBSCRIBE %s", subChannels.c_str());
    assert(res == REDIS_OK);
    //LOG_REPLY("SUBSCRIBE", reply);

// XXX: Also have to pool once because of races (but it's not possible with the same connection!)

    // This is important to make hiredis actually send the subscribe request
    // XXX: Probably the request should be done in the async
    async.data = NULL;
    uv_async_send(&async);

    while(psWaiter.channelsSubscribed.size() != keys.size() && (psWaiter.updated == false)){
        pthread_cond_wait(&psWaiter.condChannelSubscribed, &asyncConnectionMutex); 
    }

    
    const vector<string> vKeys = vector<string>(keys.begin(),keys.end());
    vector<string> vValues;
    MultiGet(vKeys, vValues);

    while(psWaiter.updated == false){
        pthread_cond_wait(&psWaiter.condUpdated, &asyncConnectionMutex); 
    }

    // Clean up tasks: 
    //   1) remove the conditional variable from the waiters 
    //   2) unsubscribe the channels if we're the last subscriber

    string unsubChannels;

    it = keys.begin();
    first_key = 1;
    for (;it!=keys.end();it++){
        string channel = notificationChannelPrefix + *it;
        auto psWaitersChannel = &psWaiters[channel];
        psWaitersChannel->erase(&psWaiter);
        if(psWaitersChannel->size()==0){
            if(first_key){
                first_key = 0;
            }else{
                unsubChannels += " ";
            }
            unsubChannels+=channel;
        }
    }
    printf("WAIT(): unsubscribing from channels: \"%s\"\n", unsubChannels.c_str());


    LOG_REQUEST("ASYNC UNSUBSCRIBE", "");
    res = redisAsyncCommand(asyncContext, pubsubCallback, (char*)"end-1", "UNSUBSCRIBE %s", unsubChannels.c_str());
    assert(res == REDIS_OK);
    //LOG_REPLY("UNSUBSCRIBE", reply);

    // This is important to make hiredis actually send the subscribe request
    // XXX: Probably the request should be done in the async
    async.data = NULL;
    uv_async_send(&async);



    pthread_mutex_unlock(&asyncConnectionMutex);
    return ERR_OK;
}

void getCallback(redisAsyncContext *c, void *r, void *privdata) {
    pthread_mutex_lock(&asyncConnectionMutex);

    redisReply *reply = (redisReply*)r;
    if (reply == NULL) return;
    printf("getCallback argv[%s]: %s\n", (char*)privdata, reply->str);

    pthread_mutex_unlock(&asyncConnectionMutex);
}

void pubsubCallback(redisAsyncContext *c, void *r, void *privdata) {
    pthread_mutex_lock(&asyncConnectionMutex);

    redisReply *reply = (redisReply*)r;
    assert(reply != NULL);
    //printf("pubsubCallback argv[%s]: %s\n", (char*)privdata, reply->str);


    // We can get 3 types of replies:
    //   1. "subscribe"
    //   2. "message"
    //   3. "unsubscribe"
//     if (reply->type == REDIS_REPLY_ARRAY) {
//         for (unsigned int j = 0; j < reply->elements; j++) {
//             printf("  %u) %s\n", j, reply->element[j]->str);
//         }
//     }
// 
    // XXX: Simplify this code and document...
    assert(reply->elements == 3);

    string channel = string(reply->element[1]->str);
    int keyspacePos = channel.find(notificationChannelPrefix);
    assert(keyspacePos == 0);

    string key = channel.substr(notificationChannelPrefix.size());
    
    if(strcmp(reply->element[0]->str, "message")==0){
        // Wake up all the waiters on this key

//        printf("channel=%s key=%s\n", channel.c_str(), key.c_str());
//         printf("psWaiters.size(): %lu\n", psWaiters.size());
// 
//         for(auto iter = psWaiters.begin(); iter != psWaiters.end(); iter++)
//         {
//             string k =  iter->first;
//             auto set = iter->second;
//             printf("keys: \"%s\" (%d)\n", k.c_str(), set.size());
//         }

         auto psWaitersChannel = &psWaiters[channel];
//        auto itConds = psWaiters.find(channel);

//        printf("psWaitersChannel->size(): %lu\n", psWaitersChannel->size());
        auto it = psWaitersChannel->begin();
        for(;it!=psWaitersChannel->end();it++){
            auto psWaiter  = *it;
//            printf("!!!!!!!!!!!!! Signaling condUpdated (key = %s)!!!!!!!!!!\n", key.c_str());
            psWaiter->updated = true;
            pthread_cond_signal(&psWaiter->condChannelSubscribed);
            pthread_cond_signal(&psWaiter->condUpdated);
        }
//        printf("psWaitersChannel.size(): %lu\n", psWaitersChannel->size());

    } else if(strcmp(reply->element[0]->str, "subscribe")==0) {

         auto psWaitersChannel = &psWaiters[channel];
//        auto itConds = psWaiters.find(channel);

//        printf("psWaitersChannel->size(): %lu\n", psWaitersChannel->size());
        auto it = psWaitersChannel->begin();
        for(;it!=psWaitersChannel->end();it++){
            auto psWaiter  = *it;
//            printf("!!!!!!!!!!!!! Signaling condChannelSubscribed (key = %s)!!!!!!!!!!\n", key.c_str());
            psWaiter->channelsSubscribed.insert(channel);
            pthread_cond_signal(&psWaiter->condChannelSubscribed);
        }
//        printf("psWaitersChannel.size(): %lu\n", psWaitersChannel->size());



    } else if(strcmp(reply->element[0]->str, "unsubscribe")==0) {
        // XXX: Probably need to be carefull with races related with subscribe + unsubscribe messages

    } else {
        assert(0);
    }

    pthread_mutex_unlock(&asyncConnectionMutex);
}


void connectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
    printf("Connected...\n");

    // Signal connection
    sem_post(&semAsyncConnected);
}

void disconnectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
    printf("Disconnected...\n");
    assert(0);
}


// Calling this function should wake the async thread wake up at the event loop
#ifdef __ANDROID__
void dummy_async(uv_async_t *handle) {
#else
void dummy_async(uv_async_t *handle, int v) {
#endif
    // XXX: Probably it would be better to make the hiredis subscribe calls from here

    //double percentage = *((double*) handle->data);
    //fprintf(stderr, "Downloaded %.2f%%\n", percentage);
}


void* connectAsyncThread(void *threadArg){
    signal(SIGPIPE, SIG_IGN);

    pthread_mutex_lock(&asyncConnectionMutex);

    loop = uv_default_loop();

    asyncContext = redisAsyncConnect(serverAddress.c_str(), 6379);
    if (asyncContext->err) {
        /* Let *c leak for now... */
        printf("Error: %s\n", asyncContext->errstr);
        assert(0);
    }

    uv_async_init(loop, &async, dummy_async);

    redisLibuvAttach(asyncContext,loop);
    redisAsyncSetConnectCallback(asyncContext,connectCallback);
    redisAsyncSetDisconnectCallback(asyncContext,disconnectCallback);
//    const char *value = "abc";
//    redisAsyncCommand(c, NULL, NULL, "SET key %b", value, strlen(value));
//    redisAsyncCommand(c, getCallback, (char*)"end-1", "GET key");

//    redisAsyncCommand(asyncContext, pubsubCallback, (char*)"end-1", "SUBSCRIBE %s", "__keyspace@0__:a");

    pthread_mutex_unlock(&asyncConnectionMutex);

    uv_run(loop, UV_RUN_DEFAULT);
    return 0;
}


int connectAsync() {

    pthread_mutex_lock(&asyncConnectionMutex);

    if(asyncConnectionSigleton == 0){
        // Spawn only one thread for the asynchronous connections
        asyncConnectionSigleton = 1;
        pthread_t t1;
        sem_init(&semAsyncConnected, 0, 0);
        pthread_mutex_unlock(&asyncConnectionMutex);

        int ret = pthread_create(&t1, NULL, connectAsyncThread, NULL);
        assert(ret == 0);

        // Wait for connection
        sem_wait(&semAsyncConnected);
    }else{
        pthread_mutex_unlock(&asyncConnectionMutex);
    }

    return 0;
}

long getThreadID(){
    long tid;
    tid = syscall(SYS_gettid);
    return tid;
}

} // namespace diamond
