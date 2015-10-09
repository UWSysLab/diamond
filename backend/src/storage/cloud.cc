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
#include <sstream>
#include <async.h>
#include <adapters/libuv.h>
#include <algorithm>
#include <iterator>

#include "includes/profile.h"

namespace diamond {

using namespace std;


// Initializing static variables of Cloud
Cloud* Cloud::_instance = NULL; 
bool Cloud::_connected = false;

std::string serverAddress;


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
#if defined(__ANDROID__) || defined(__APPLE)
void pubsubAsync(uv_async_t *handle);
#else
void pubsubAsync(uv_async_t *handle, int v);
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
        //PROFILE_ENTER("SLEEP1");
        //sleep(1);
        //PROFILE_EXIT("SLEEP1");
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

    Notice("Connecting...\n");
    redis = redisConnect(host.c_str(), 6379);
    Notice("Connected.\n");
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
    vector<bool> exist;
    return MultiGet(keys, values, exist);    
}

int
Cloud::MultiGet(const vector<string> & keys, vector<string> & values, vector<bool> & exist) {
    redisReply *reply;

    if (values.size() != 0) {
        Panic("Values list not empty");
    }

    values.resize(keys.size());
    exist.resize(keys.size());

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
            exist.at(i) = true;
        }
        else if (subreply->type == REDIS_REPLY_NIL) {
            values.at(i) = "";
            exist.at(i) = false;
        }
    }

    freeReplyObject(reply);
    return ERR_OK;
}

int
Cloud::Lrange(const string &key, long start, long stop, vector<string> & ret) {
    redisReply *reply;

    if (!_connected) {
        return ERR_UNAVAILABLE;
    }

    reply = (redisReply *)redisCommand(GetRedisContext(), "LRANGE %s %ld %ld", key.c_str(), start, stop);

    if (reply == NULL){
        Panic("reply == null");
    }
    if(reply->type != REDIS_REPLY_ARRAY) {
        Panic("Lrange reply is not an array");
        freeReplyObject(reply);
        return ERR_EMPTY;
    }

    for (size_t i = 0; i < reply->elements; i++) {
        redisReply *subreply = reply->element[i];
        if (subreply->type == REDIS_REPLY_STRING) {
            ret.push_back(string(subreply->str));
        }
    }

    freeReplyObject(reply);
    return ERR_OK;
}

int
Cloud::Lindex(const string &key, long index, string &value) {
    redisReply *reply;

    if (!_connected) {
        return ERR_UNAVAILABLE;
    }

    reply = (redisReply *)redisCommand(GetRedisContext(), "LINDEX %s %ld", key.c_str(), index);

    if (reply == NULL){
        Panic("reply == null");
    }

    if (reply->type == REDIS_REPLY_STRING) {
        value = string(reply->str);
    }
    else if (reply->type == REDIS_REPLY_NIL) {
        Panic("Lindex() index out of range");
    }

    freeReplyObject(reply);
    return ERR_OK;
}

int
Cloud::Llen(const string &key, long &value) {
    redisReply *reply;

    if (!_connected) {
        return ERR_UNAVAILABLE;
    }

    reply = (redisReply *)redisCommand(GetRedisContext(),  "LLEN %s", key.c_str());

    if (reply == NULL){
        Panic("reply == null");
    }

    if (reply->type == REDIS_REPLY_INTEGER) {
        value = reply->integer;
    }
    else {
        Panic("Llen reply is not an integer");
    }
    freeReplyObject(reply);
    return ERR_OK;
}

int
Cloud::Del(const string &key) {
    redisReply *reply;

    if (!_connected) {
        return ERR_UNAVAILABLE;
    }

    reply = (redisReply *)redisCommand(GetRedisContext(), "DEL %s", key.c_str());

    if (reply == NULL) {
        Panic("reply == null");
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
Cloud::MultiWriteExec(std::map<string, string >& keyValues)
{
    redisReply *reply;

    if (!_connected) {
        return ERR_UNAVAILABLE;
    }
    redisContext* context = GetRedisContext();    

    // Send the requests
    LOG_REQUEST("MULTIWRITE-EXEC BATCH", "");
    redisAppendCommand(context, "MULTI");
    auto it = keyValues.begin();
    for (;it!=keyValues.end();it++){
        string key = it->first;
        string value = it->second;
        redisAppendCommand(context, "SET %s %s", key.c_str(), value.c_str());
    }
    redisAppendCommand(context,  "EXEC");
    LOG_REPLY("MULTIWRITE-EXEC BATCH", reply);


    // Get the replies
    redisGetReply(context,(void **)&reply); // reply for MULTI
    freeReplyObject(reply);

    for(unsigned int i = 0; i<keyValues.size();i++){
        redisGetReply(context,(void **)&reply); // reply for GET
        freeReplyObject(reply);
        if (reply == NULL) {
            Panic("reply == null");
        }
    }

    redisGetReply(context,(void **)&reply); // reply for EXECs
    freeReplyObject(reply);

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
    pubsubWaiter psWaiter;

    if (!_connected) {
        return ERR_UNAVAILABLE;
    }


    pthread_mutex_lock(&asyncConnectionMutex);

    auto it = keys.begin();
    for (;it!=keys.end();it++){
        string* channel = new string(string(NOTIFICATION_CHANNEL_PREFIX) + *it);
        Notice("Adding channel \"%s\" to psWaiters\n", channel->c_str());
        auto psWaitersChannel = &psWaiters[*channel];
        psWaitersChannel->insert(&psWaiter);
    }

    async.data = NULL;
    uv_async_send(&async);

    // Wait for confirmation that we subscribed all the channels
    while(psWaiter.channelsSubscribed.size() != keys.size() && (psWaiter.RSTouched == false)){
        Notice("Waiting for all the channels to be subscribed (%lu vs %lu)\n", psWaiter.channelsSubscribed.size(), keys.size());
        pthread_cond_wait(&psWaiter.condChannelSubscribed, &asyncConnectionMutex); 
    }

    // Check the values of the RS because values might have changed since they were last read by the app 
    const vector<string> vKeys = vector<string>(keys.begin(),keys.end());
    vector<string> vValues;
    vector<bool> vExist;
    MultiGet(vKeys, vValues, vExist);

    assert(vKeys.size() == vValues.size());
    unsigned int i;
    bool RSDifferent = false;
    for (i=0;i<vKeys.size();i++){
        string key = vKeys.at(i);
        string newValue = vValues.at(i);
        // XXX: This does not fix the problem entirely
        //   If the value does not exist now, * presumme * that it didn't exist before and
        //   therefore wait for a pubsub message
        //   Without this condition Retry would return always just because 
        //   MGET and Get+Deserialize+Serialize see inconsistent values
        bool newExist = vExist.at(i); 
        Notice("MGET (key=%s, newValue=%s, old value = %s)\n", 
                key.c_str(), newValue.c_str(), lastReadValues[key].c_str() );
        if(lastReadValues[key] != newValue && newExist){
            Notice("New value detected by MGET (key=%s, newValue=%s, old value = %s)\n", 
                key.c_str(), newValue.c_str(), lastReadValues[key].c_str() );
            RSDifferent = true;
            break;
        }
    }


    if(!RSDifferent){
        // If we didn't detect new values with the MGET, then wait for a pubsub notification
        while(psWaiter.RSTouched == false){
            Notice("Waiting for a notification from pubsubCallback()\n");
            pthread_cond_wait(&psWaiter.condUpdated, &asyncConnectionMutex); 
        }
    }

    // Clean up tasks: 
    //   1) remove the waiter  
    //   2) remove the channel if no more waiters on that channel
    
    for (it = keys.begin();it!=keys.end();it++){
        string channel = NOTIFICATION_CHANNEL_PREFIX + *it;
        auto psWaitersChannel = &psWaiters[channel];
        psWaitersChannel->erase(&psWaiter);
        if(psWaitersChannel->size()==0){
            Notice("Removing channel \"%s\" from psWaiters\n", channel.c_str());
            psWaiters.erase(channel);
        }
    }

    // This is important to make hiredis actually unsubscribes the channels
    async.data = NULL;
    uv_async_send(&async);


    pthread_mutex_unlock(&asyncConnectionMutex);
    //sleep(5);
    return ERR_OK;
}


void pubsubCallback(redisAsyncContext *c, void *r, void *privdata) {
    pthread_mutex_lock(&asyncConnectionMutex);

    redisReply *reply = (redisReply*)r;
    assert(reply != NULL);

    // We can get 3 types of replies:  1. "subscribe"   2. "message"   3. "unsubscribe"
    assert(reply->elements == 3);
    Notice("Pubsub callback: %s, %s, (str: %s, int: %lld)\n", 
        reply->element[0]->str, reply->element[1]->str, reply->element[2]->str, reply->element[2]->integer);
   
//     Notice("psWaiters.size(): %lu\n", psWaiters.size());
//     for(auto iter = psWaiters.begin(); iter != psWaiters.end(); iter++)
//     {
//         string channel =  iter->first;
//         auto set = iter->second;
//         Notice("  Channel: \"%s\" (%lu)\n", channel.c_str(), set.size());
//     }
   
    string channel = string(reply->element[1]->str);
//    Notice("Channel: %s\n", channel.c_str());

    assert(channel.find(string(NOTIFICATION_CHANNEL_PREFIX)) == 0);
    string key = channel.substr(strlen(NOTIFICATION_CHANNEL_PREFIX));

    auto psWaitersChannel = &psWaiters[channel];
//    Notice("psWaitersChannel size: %lu\n", psWaitersChannel->size());
     
    if(strcmp(reply->element[0]->str, "message")==0){
        // Wake up all the waiters on this key
        Notice("Pubsub message received\n");
        auto it = psWaitersChannel->begin();
        for(;it!=psWaitersChannel->end();it++){
            auto psWaiter = *it;
            Notice("Signaling that RS was touched (key = %s)\n", key.c_str());
            psWaiter->RSTouched = true;
            pthread_cond_signal(&psWaiter->condChannelSubscribed);
            pthread_cond_signal(&psWaiter->condUpdated);
        }

    } else if(strcmp(reply->element[0]->str, "subscribe")==0) {
        // Wake up all waiters on this key that wait for the subscriptions to complete 
        auto it = psWaitersChannel->begin();
        Notice("Pubsub subscribe confirmation received\n");
        for(;it!=psWaitersChannel->end();it++){
            auto psWaiter = *it;
            Notice("Signaling that channel was subscribed (key = %s)\n", key.c_str());
            psWaiter->channelsSubscribed.insert(channel);
            pthread_cond_signal(&psWaiter->condChannelSubscribed);
        }
    } else if(strcmp(reply->element[0]->str, "unsubscribe")==0) {
        // XXX: Probably need to be carefull with races related with subscribe + unsubscribe messages
        // XXX: could gc the the psWaiters
    } else {
        assert(0);
    }

    pthread_mutex_unlock(&asyncConnectionMutex);
}


void connectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        Notice("Error: %s\n", c->errstr);
        return;
    }
    Notice("Connected.\n");

    // Signal connection
    sem_post(&semAsyncConnected);
}

void disconnectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        Notice("Error: %s\n", c->errstr);
        return;
    }
    Notice("Disconnected...\n");
    assert(0);
}


// Calling this function should wake the async thread wake up at the event loop
#if defined(__ANDROID__) || defined(__APPLE__)
void pubsubAsync(uv_async_t *handle) {
#else
void pubsubAsync(uv_async_t *handle, int v) {
#endif
    static set<string> channelsSubscribed;
    int res;

    pthread_mutex_lock(&asyncConnectionMutex);

//     Notice("channelsSubscribed.size(): %lu\n", channelsSubscribed.size());
//     Notice("psWaiters.size(): %lu\n", psWaiters.size());
//     for(auto iter = psWaiters.begin(); iter != psWaiters.end(); iter++)
//     {
//         string channel =  iter->first;
//         auto set = iter->second;
//         Notice("  Channel: \"%s\" (%lu)\n", channel.c_str(), set.size());
//     }


    // Find out the list of channels that should be subscribed
    set<string> channelsNeeded;
    for(auto iter = psWaiters.begin(); iter != psWaiters.end(); iter++)
    {
        string channel =  iter->first;
        channelsNeeded.insert(channel);
    }


    // Find out the list of channels that we haven't yet subscribed 
    vector<string> channelsToSubscribe;
    set_difference(channelsNeeded.begin(),channelsNeeded.end(),
                    channelsSubscribed.begin(),channelsSubscribed.end(), 
                    std::back_inserter(channelsToSubscribe));

    Notice("Subscribing %lu channels\n", channelsToSubscribe.size());
    if(channelsToSubscribe.size() > 0){        
        const char *argv[1024];
        assert(channelsToSubscribe.size()<1024-1);

        argv[0] = "SUBSCRIBE";
        int i = 1;
        for(auto iter = channelsToSubscribe.begin(); iter != channelsToSubscribe.end(); iter++){
            string channel = (*iter);
            argv[i] = channel.c_str();
            channelsSubscribed.insert(channel);
            i++;
        }      

        LOG_REQUEST("SUBSCRIBE", "");
        res = redisAsyncCommandArgv(asyncContext, pubsubCallback, NULL, channelsToSubscribe.size()+1, argv, NULL);
        assert(res == REDIS_OK);
    }


    // Find out the list of channels that we haven't yet unsubscribed 
    vector<string> channelsToUnsubscribe;
    set_difference(channelsSubscribed.begin(),channelsSubscribed.end(),
                    channelsNeeded.begin(),channelsNeeded.end(), 
                    std::back_inserter(channelsToUnsubscribe));
    Notice("Unsubscribe %lu channels\n", channelsToUnsubscribe.size());
    if(channelsToUnsubscribe.size() > 0){        
        const char *argv[1024];
        assert(channelsToUnsubscribe.size()<1024-1);

        argv[0] = "UNSUBSCRIBE";
        int i = 1;
        for(auto iter = channelsToUnsubscribe.begin(); iter != channelsToUnsubscribe.end(); iter++){
            string channel = *iter;
            argv[i] = channel.c_str();
            channelsSubscribed.erase(channel);
            i++;
        }      

        LOG_REQUEST("UNSUBSCRIBE", "");
        res = redisAsyncCommandArgv(asyncContext, pubsubCallback, NULL, channelsToUnsubscribe.size()+1, argv, NULL);
        assert(res == REDIS_OK);
    }

    pthread_mutex_unlock(&asyncConnectionMutex);
}




void* connectAsyncThread(void *threadArg){
    signal(SIGPIPE, SIG_IGN);

    pthread_mutex_lock(&asyncConnectionMutex);

    loop = uv_default_loop();

    asyncContext = redisAsyncConnect(serverAddress.c_str(), 6379);
    if (asyncContext->err) {
        /* Let *c leak for now... */
        Notice("Error: %s\n", asyncContext->errstr);
        assert(0);
    }

    uv_async_init(loop, &async, pubsubAsync);

    redisLibuvAttach(asyncContext,loop);
    redisAsyncSetConnectCallback(asyncContext,connectCallback);
    redisAsyncSetDisconnectCallback(asyncContext,disconnectCallback);

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
