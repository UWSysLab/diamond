// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * store/common/frontend/asynccacheclient.cc:
 *   Asynchronous caching client implementation.
 *
 * Copyright 2015 Irene Zhang <iyzhang@cs.washington.edu>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************************/

#include "asynccacheclient.h"

using namespace std;

AsyncCacheClient::AsyncCacheClient(AsyncClient* client)
    : client(client)
{
    this->cachingEnabled = true;
}

AsyncCacheClient::~AsyncCacheClient() { }

/* Begins a transaction. */
void
AsyncCacheClient::Begin(const uint64_t tid)
{
    Debug("BEGIN [%lu]", tid);
    client->Begin(tid);
}

void
AsyncCacheClient::BeginRO(const uint64_t tid,
                     const Timestamp timestamp)
{
    Debug("BEGIN [%lu]", tid);
    client->BeginRO(tid, timestamp);
}

void
AsyncCacheClient::SetCaching(bool cachingEnabled) {
    this->cachingEnabled = cachingEnabled;
}

// update publications handler
void
AsyncCacheClient::SetPublish(publish_handler_t publish) {
    Debug("Set message handler");
    this->publish = publish;
    client->SetPublish(bind(&AsyncCacheClient::HandlePublish,
                            this,
                            placeholders::_1,
                            placeholders::_2));
}

void
AsyncCacheClient::HandlePublish(const Timestamp timestamp,
                                const set<string> &keys)
{
    for (auto &key : keys) {
        // invalidate cache
        cache.Remove(key);
    }
    publish(timestamp, keys);
}

void
AsyncCacheClient::MultiGet(const uint64_t tid,
                           const set<string> &keys,
                           callback_t callback,
                           const Timestamp timestamp)
{
    map<string, Version> keysRead;
    set<string> keysToRead;

    if (cachingEnabled) {
        Debug("MULTIGET %lu", keys.size());

        cache_lock.lock();

        Version value;

        for (auto &key : keys) {
            // look for it in the cache
            if (cache.Get(key, timestamp, value)) {
                keysRead[key] = value;
            } else {
                keysToRead.insert(key);
            }
        }
        cache_lock.unlock();
    } else {
        keysToRead = keys;
    }
    
    if (keysRead.size() == keys.size()) {
        // we found everything in the cache. Hooray!
        Promise promise;
        promise.Reply(REPLY_OK, keysRead);
        callback(promise);
        return;
    }

    if (cachingEnabled) {
        client->MultiGet(tid,
                            keysToRead,
                            bind(&AsyncCacheClient::GetCallback,
                                 this,
                                 callback,
                                 placeholders::_1),
                            timestamp);
    } else {
        client->MultiGet(tid, keysToRead,
                            callback, timestamp);
    }
}

void 
AsyncCacheClient::GetCallback(callback_t callback,
                              Promise &promise)
{
    Debug("GET Callback");
    if (promise.GetReply() == REPLY_OK) {
        cache_lock.lock();
        // Make sure that we've capped the validity range
        
        for (auto &v : promise.GetValues()) {
            Debug("Adding [%s] with ts %lu to the cache",
                  v.first.c_str(),
                  v.second.GetTimestamp());
            //ASSERT(v.second.GetInterval().End() != MAX_TIMESTAMP);
            cache.Put(v.first, v.second);
        }
        cache_lock.unlock();
    }
    callback(promise);
}

/* Set value for a key. (Always succeeds).
 * Returns 0 on success, else -1. */
void
AsyncCacheClient::Put(const uint64_t tid,
                 const string &key,
                 const string &value,
                 callback_t callback)
{
    Debug("PUT %s %s", key.c_str(), value.c_str());
    client->Put(tid, key, value, callback);
}

/* Prepare the transaction. */
void
AsyncCacheClient::Prepare(const uint64_t tid,
                          callback_t callback,
                          const Transaction &txn)
{
    if (!cachingEnabled) {
        client->Prepare(tid, callback, txn);
        return;
    }
    
    Debug("PREPARE [%lu]", tid);
    client->Prepare(tid, 
                    bind(&AsyncCacheClient::PrepareCallback,
                         this,
                         callback,
                         tid,
                         txn,
                         placeholders::_1),
                    txn);
}

void
AsyncCacheClient::PrepareCallback(callback_t callback,
                                  const uint64_t tid,
                                  const Transaction txn,
                                  Promise &promise)
{
    //save the transactions for later
    if (promise.GetReply() == REPLY_OK) {
        Debug("PREPARE Callback caching transaction for later");
        cache_lock.lock();
        if (prepared.find(tid) == prepared.end()) {
            prepared[tid] = txn;
        }
        cache_lock.unlock();
    }
    callback(promise);
}

void
AsyncCacheClient::Commit(const uint64_t tid,
                         callback_t callback,
                         const Transaction &txn)
{

    if (!cachingEnabled) {
        client->Commit(tid, callback, txn);
    } else {
        Debug("COMMIT [%lu]", tid);
        client->Commit(tid,
                       bind(&AsyncCacheClient::CommitCallback,
                            this,
                            callback,
                            tid,
                            txn,
                            placeholders::_1),
                       txn);
    }    
}

void
AsyncCacheClient::CommitCallback(callback_t callback,
                                 const uint64_t tid,
                                 const Transaction txn,
                                 Promise &promise)
{
    // update the cache
    // cache_lock.lock();

    // auto it = prepared.find(tid);
    // const Transaction &t = (it != prepared.end()) ? it->second : txn;
            
    // if (promise.GetReply() == REPLY_OK) {
    //     // for (auto &write : t.GetWriteSet()) {
    // 	//     Debug("Adding write [%s] with ts %lu to the cache",
    //     //           write.first.c_str(),
    //     //           promise.GetTimestamp());
    //     //     cache.Put(write.first,
    //     //               write.second,
    //     //               promise.GetTimestamp());
    //     // }
    // } else if (promise.GetReply() == REPLY_FAIL) {
    //     for (auto &read : t.GetReadSet()) {
    //         Debug("Removing stale [%s] from the cache",
    //               read.first.c_str());
    //         cache.Remove(read.first);
    //     }
    // }
    // prepared.erase(tid);
    // cache_lock.unlock();
    // for (auto &inc : t.GetIncrementSet()) {
    //     Debug("Removing [%s] from the cache", inc.first.c_str());
    //     cache.Remove(inc.first);
    // }
}
    
/* Aborts the ongoing transaction. */
void
AsyncCacheClient::Abort(const uint64_t tid,
                        callback_t callback)
{
    if (!cachingEnabled) {
        client->Abort(tid, callback);
    }
    
    Debug("ABORT [%lu]", tid);

    client->Abort(tid,
                  bind(&AsyncCacheClient::AbortCallback,
                       this,
                       callback,
                       tid,
                       placeholders::_1));
}

void
AsyncCacheClient::AbortCallback(callback_t callback,
                                const uint64_t tid,
                                Promise &promise)
{
    cache_lock.lock();
    auto it = prepared.find(tid); 
    if (it != prepared.end()) {
        // clear out the read set
        for (auto &read : it->second.GetReadSet()) {
                Debug("Removing [%s] from the cache",
                      read.first.c_str());
                cache.Remove(read.first);
        }
        prepared.erase(tid);
    }
    cache_lock.unlock();
    callback(promise);
}

void
AsyncCacheClient::Subscribe(const uint64_t reactive_id,
                       const std::set<std::string> &keys,
                       const Timestamp timestamp,
                       callback_t callback) {
    client->Subscribe(reactive_id,
                      keys,
                      timestamp,
                      callback);
}

void
AsyncCacheClient::Unsubscribe(const uint64_t reactive_id,
                         const std::set<std::string> &keys,
                         callback_t callback) {
    client->Unsubscribe(reactive_id,
                        keys,
                        callback);
}

void
AsyncCacheClient::Ack(const uint64_t reactive_id,
                      const std::set<std::string> &keys,
                      const Timestamp timestamp,
                      callback_t callback) {
    client->Ack(reactive_id,
                keys,
                timestamp,
                callback);
}

