// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * store/common/frontend/cacheclient.cc:
 *   Single shard caching client implementation.
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

#include "cacheclient.h"

using namespace std;

CacheClient::CacheClient(TxnClient* txnclient)
{
    this->txnclient = txnclient;
}

CacheClient::~CacheClient() { }

/* Begins a transaction. */
void
CacheClient::Begin(uint64_t tid)
{
    Debug("BEGIN [%lu]", tid);
    txnclient->Begin(tid);
}

/* Get value for a key.
 * Returns 0 on success, else -1. */
void
CacheClient::Get(const uint64_t tid, const string &key, Promise *promise)
{
    Debug("GET %s", key.c_str());
    cache_lock.lock();

    Version value;
    // look for it in the cache
    if (cache.get(key, value)) {
        // Make some decision about the timestamp?
        cache_lock.unlock();

	Debug("CACHE HIT %s at ts %lu", key.c_str(), value.GetTimestamp());
        if (promise != NULL)
            promise->Reply(REPLY_OK, key, value);
        return;
    }

    cache_lock.unlock();
    // Otherwise, get latest value from server.
    Promise p(GET_TIMEOUT);
    Promise *pp = (promise != NULL) ? promise : &p;

    txnclient->Get(tid, key, pp);
    if (pp->GetReply() == REPLY_OK) {
        Debug("Adding [%s] with ts %lu to the cache", key.c_str(), pp->GetValue(key).GetTimestamp());
        cache_lock.lock();
        cache.put(key, pp->GetValue(key));
        cache_lock.unlock();
    }
}

/* Get value for a key.
 * Returns 0 on success, else -1. */
void
CacheClient::Get(const uint64_t tid, const string &key, const Timestamp &timestamp, Promise *promise)
{
    Debug("GET %s", key.c_str());
    cache_lock.lock();

    Version value;
    // look for it in the cache
    if (cache.get(key, timestamp, value)) {
        // Make some decision about the timestamp?
        cache_lock.unlock();
        
	Debug("CACHE HIT %s at ts %lu", key.c_str(), value.GetTimestamp());
        if (promise != NULL)
            promise->Reply(REPLY_OK, key, value);
        return;
    }

    cache_lock.unlock();
    // Otherwise, get latest value from server.
    Promise p(GET_TIMEOUT);
    Promise *pp = (promise != NULL) ? promise : &p;

    txnclient->Get(tid, key, timestamp, pp);
    if (pp->GetReply() == REPLY_OK) {
        Debug("Adding [%s] with ts %lu to the cache", key.c_str(), pp->GetValue(key).GetTimestamp());
        cache_lock.lock();
        cache.put(key, pp->GetValue(key));
        cache_lock.unlock();
    }
}

void
CacheClient::MultiGet(const uint64_t tid, const vector<string> &keys, Promise *promise)
{
    Debug("MULTIGET %lu", keys.size());

    cache_lock.lock();

    map<string, Version> keysRead;
    vector<string> keysToRead;
    Version value;

    for (auto &key : keys) {
        // look for it in the cache
        if (cache.get(key, value)) {
            // Make some decision about the timestamp?

            keysRead[key] = value;  
        } else {
            keysToRead.push_back(key);
        }
    }
    cache_lock.unlock();
    
    if (keysRead.size() == keys.size()) {
        // we found everything in the cache. Hooray!
        if (promise != NULL)
            promise->Reply(REPLY_OK, keysRead);
        return;
    }
    
    // Get latest value from server, ignoring consistent reads
    Promise p(GET_TIMEOUT);
    Promise *pp = (promise != NULL) ? promise : &p;

    txnclient->MultiGet(tid, keysToRead, pp);
    if (pp->GetReply() == REPLY_OK){
        map<string, Version> values = pp->GetValues();

        cache_lock.lock();
        for (auto &value : values) {
            Debug("Adding [%s] with ts %lu to the cache", value.first.c_str(), value.second.GetTimestamp());
            cache.put(value.first, value.second);
            keysRead[value.first] = value.second;
        }
        cache_lock.unlock();
        pp->Reply(REPLY_OK, keysRead);
    }
}

void
CacheClient::MultiGet(const uint64_t tid, const vector<string> &keys, const Timestamp &timestamp, Promise *promise)
{
    Debug("MULTIGET %lu", keys.size());

    cache_lock.lock();

    map<string, Version> keysRead;
    vector<string> keysToRead;
    Version value;

    for (auto &key : keys) {
        // look for it in the cache
        if (cache.get(key, timestamp, value)) {
            // Make some decision about the timestamp?

            keysRead[key] = value;  
        } else {
            keysToRead.push_back(key);
        }
    }
    cache_lock.unlock();
    
    if (keysRead.size() == keys.size()) {
        // we found everything in the cache. Hooray!
        if (promise != NULL)
            promise->Reply(REPLY_OK, keysRead);
        return;
    }
    
    Promise p(GET_TIMEOUT);
    Promise *pp = (promise != NULL) ? promise : &p;

    txnclient->MultiGet(tid, keysToRead, timestamp, pp);
    if (pp->GetReply() == REPLY_OK){
        map<string, Version> values = pp->GetValues();

        cache_lock.lock();
        for (auto &value : values) {
            Debug("Adding [%s] with ts %lu to the cache", value.first.c_str(), value.second.GetTimestamp());
            cache.put(value.first, value.second);
            keysRead[value.first] = value.second;
        }
        cache_lock.unlock();
        pp->Reply(REPLY_OK, keysRead);
    }
}

/* Set value for a key. (Always succeeds).
 * Returns 0 on success, else -1. */
void
CacheClient::Put(const uint64_t tid, const string &key, const string &value, Promise *promise)
{
    Debug("PUT %s %s", key.c_str(), value.c_str());
    txnclient->Put(tid, key, value, promise);
}

/* Prepare the transaction. */
void
CacheClient::Prepare(const uint64_t tid, const Transaction &txn, Promise *promise)
{
    Debug("PREPARE [%lu]", tid);
    txnclient->Prepare(tid, txn, promise);
}

void
CacheClient::Commit(const uint64_t tid, const Transaction &txn, const Timestamp &timestamp, Promise *promise)
{
    Debug("COMMIT [%lu]", tid);
    Promise p(COMMIT_TIMEOUT);
    Promise *pp = (promise != NULL) ? promise : &p;
    txnclient->Commit(tid, txn, timestamp, pp);

    // update the cache
    if (pp->GetReply() == REPLY_OK) {
        cache_lock.lock();
        for (auto &write : txn.getWriteSet()) {
	    Debug("Adding [%s] with ts %lu to the cache", write.first.c_str(), pp->GetTimestamp());
            cache.put(write.first, write.second, pp->GetTimestamp());
        }
        cache_lock.unlock();
    } else if (pp->GetReply() == REPLY_FAIL) {
	cache_lock.lock();
	for (auto &read : txn.getReadSet()) {
	    Debug("Removing [%s] from the cache", read.first.c_str());
	    cache.remove(read.first);
	}
	cache_lock.unlock();
    }
}

/* Aborts the ongoing transaction. */
void
CacheClient::Abort(const uint64_t tid, const Transaction &txn, Promise *promise)
{
    Debug("ABORT [%lu]", tid);

    Promise p(COMMIT_TIMEOUT);
    Promise *pp = (promise != NULL) ? promise : &p;

    txnclient->Abort(tid, txn, pp);
    if (pp->GetReply() == REPLY_OK) {
	cache_lock.lock();
	for (auto &read : txn.getReadSet()) {
	    Debug("Removing [%s] from the cache", read.first.c_str());
	    cache.remove(read.first);
	}
	cache_lock.unlock();
    }
}
