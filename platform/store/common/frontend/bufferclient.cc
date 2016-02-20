// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * store/common/frontend/bufferclient.cc:
 *   Single shard buffering client implementation.
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

#include "bufferclient.h"

using namespace std;

BufferClient::BufferClient(TxnClient* txnclient)
{
    this->txnclient = txnclient;
}

BufferClient::~BufferClient() { }

/* Begins a transaction. */
void
BufferClient::Begin(const uint64_t tid)
{
    Debug("BEGIN [%lu]", tid);
    txns_lock.lock();
    // Initialize data structures.
    if (txns.find(tid) == txns.end()) {
        txnclient->Begin(tid);
    	txns[tid] = Transaction(LINEARIZABLE);
    }
    txns_lock.unlock();
}

void
BufferClient::BeginRO(const uint64_t tid, const Timestamp &timestamp)
{
    Debug("BEGIN READ-ONLY [%lu]", tid);
    txns_lock.lock();
    // Initialize data structures.
    if (txns.find(tid) == txns.end()) {
        txnclient->BeginRO(tid);
    	txns[tid] = Transaction(READ_ONLY, timestamp);
    }
    txns_lock.unlock();
}

/* Get value for a key.
 * Returns 0 on success, else -1. */
void
BufferClient::Get(const uint64_t tid, const string &key, Promise *promise)
{
    Debug("GET %s", key.c_str());
    txns_lock.lock();
    if (txns.find(tid) == txns.end()) {
        txnclient->Begin(tid);
    }
    Transaction &txn = txns[tid];
    txns_lock.unlock();

    
    auto it = txn.GetWriteSet().find(key);
    // Read your own writes, check the write set first.
    if (it != txn.GetWriteSet().end()) {
        map<string, Version> ret;
        ret[key] = Version(0, it->second);
        promise->Reply(REPLY_OK, ret);
        return;
    }
    
    // Otherwise, get latest value from server.
    Promise p(GET_TIMEOUT);
    Promise *pp = (promise != NULL) ? promise : &p;

    // Check whether we have a timestamp
    txnclient->Get(tid, key, txn.GetTimestamp(), pp);
    
    if (pp->GetReply() == REPLY_OK) {
        Debug("Adding [%s] with ts %lu to the read set", key.c_str(), pp->GetValue(key).GetTimestamp());
        txn.AddReadSet(key, pp->GetValue(key).GetInterval());
        if (((txn.IsolationMode() == READ_ONLY) || (txn.IsolationMode() == SNAPSHOT_ISOLATION)) &&
            !txn.HasTimestamp()) {
            txn.SetTimestamp(pp->GetValue(key).GetTimestamp());
	    Debug("Setting ts to %lu", txn.GetTimestamp());
        }
    }
}

void
BufferClient::MultiGet(const uint64_t tid, const vector<string> &keys, Promise *promise)
{
    Debug("MULTIGET %lu", keys.size());
    txns_lock.lock();
    if (txns.find(tid) == txns.end()) {
        txnclient->Begin(tid);
    }
    Transaction &txn = txns[tid];
    txns_lock.unlock();

    vector<string> keysToRead;
    for (auto key : keys) {
        // Read your own writes, check the write set first.
        if (txn.GetWriteSet().find(key) == txn.GetWriteSet().end()) {
            keysToRead.push_back(key);
        }
    }

    if (keysToRead.size() > 0) {
        // Get latest value from server, ignoring consistent reads
        Promise p(GET_TIMEOUT);
        Promise *pp = (promise != NULL) ? promise : &p;

        txnclient->MultiGet(tid, keysToRead, txn.GetTimestamp(), pp);
        
        if (pp->GetReply() == REPLY_OK){
            std::map<string, Version> values = pp->GetValues();
            for (auto &value : values) {
                Debug("Adding [%s] with ts %lu to the read set", value.first.c_str(), value.second.GetTimestamp());
                txn.AddReadSet(value.first, value.second.GetInterval());
            }
            if (((txn.IsolationMode() == READ_ONLY) || (txn.IsolationMode() == SNAPSHOT_ISOLATION)) &&
                !txn.HasTimestamp()) {
                txn.SetTimestamp((values.begin())->second.GetTimestamp());
		Debug("Setting ts to %lu", txn.GetTimestamp());
            }
        }
    }
}
        

/* Set value for a key. (Always succeeds).
 * Returns 0 on success, else -1. */
void
BufferClient::Put(const uint64_t tid, const string &key, const string &value, Promise *promise)
{
    Debug("PUT [%lu] %s %s", tid, key.c_str(), value.c_str());
    txns_lock.lock();
    if (txns.find(tid) == txns.end()) {
        txnclient->Begin(tid);
    }
    Transaction &txn = txns[tid];
    txns_lock.unlock();

    if (txn.IsReadOnly()) {
        Panic("Can't do a put in a read only transaction!");
    }
    
    // Update the write set.
    txn.AddWriteSet(key, value);
    promise->Reply(REPLY_OK);
}

/* Prepare the transaction. */
void
BufferClient::Prepare(const uint64_t tid, Promise *promise)
{
    Debug("PREPARE [%lu]", tid);

    txns_lock.lock();
    if (txns.find(tid) != txns.end()) {
        Transaction txn = txns[tid];
        txns_lock.unlock();
        txnclient->Prepare(tid, txns[tid], promise);
    } else {
        txns_lock.unlock();
    }
}

void
BufferClient::Commit(const uint64_t tid, Promise *promise)
{
    Debug("COMMIT [%lu]", tid);

    Transaction txn;
    txns_lock.lock();
    if (txns.find(tid) != txns.end()) {
        txn = txns[tid];
        txns.erase(tid);
        txns_lock.unlock();
    } else {
        // couldn't find the transaction
        txns_lock.unlock();
        return;
    }
    
	// If SI with no writes or read-only, just locally check the read set
    // Commit all reads locally
    if ((txn.IsolationMode() == READ_ONLY) ||
        ((txn.IsolationMode() == SNAPSHOT_ISOLATION) && txn.GetWriteSet().empty())) {
        // Run local checks
        Interval i(0);
        for (auto &read : txn.GetReadSet()) {
            Intersect(i, read.second);
        }
        if (i.Start() <= i.End()) {
            if (promise != NULL) {
                promise->Reply(REPLY_OK, txn.GetTimestamp());
            }
        } else {
            if (promise != NULL) {
                promise->Reply(REPLY_FAIL);
            }
        }
        return;
    }
    
    // If eventual consistency, do no checks and don't wait for a response
    if (txn.IsolationMode() == EVENTUAL) {
        if (promise != NULL) {
            promise->Reply(REPLY_OK);
        }
        txnclient->Commit(tid, txn, NULL);
    } else {
        // Otherwise go wide-area for checks
        txnclient->Commit(tid, txn, promise);
    } 
}

/* Aborts the ongoing transaction. */
void
BufferClient::Abort(const uint64_t tid, Promise *promise)
{
    Debug("ABORT [%lu]", tid);
    txns_lock.lock();
    if (txns.find(tid) != txns.end()) {
        txns.erase(tid);
        txns_lock.unlock();
        txnclient->Abort(tid, promise);
    } else {
        txns_lock.unlock();
    }
}

