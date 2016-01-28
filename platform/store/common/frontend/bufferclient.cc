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
    txns_lock.lock();
    // Initialize data structures.
    if (txns.find(tid) == txns.end()) {
        txnclient->Begin(tid);
    }
    txns_lock.unlock();
}

/* Get value for a key.
 * Returns 0 on success, else -1. */
void
BufferClient::Get(const uint64_t tid, const string &key, Promise *promise)
{
    txns_lock.lock();
    if (txns.find(tid) == txns.end()) {
        txnclient->Begin(tid);
    }
    Transaction txn = txns[tid];
    txns_lock.unlock();

    auto it = txn.getWriteSet().find(key);
    // Read your own writes, check the write set first.
    if (it != txn.getWriteSet().end()) {
        map<string, Version> ret;
        ret[key] = Version(0,it->second);
        promise->Reply(REPLY_OK, ret);
        return;
    }

    // Consistent reads, check the read set.
    auto it2 = txn.getReadSet().find(key);
    if (it2 != txn.getReadSet().end()) {
        // read from the server at same timestamp.
        txnclient->Get(tid, key, it2->second, promise);
        return;
    }
    
    // Otherwise, get latest value from server.
    Promise p(GET_TIMEOUT);
    Promise *pp = (promise != NULL) ? promise : &p;

    txnclient->Get(tid, key, pp);
    if (pp->GetReply() == REPLY_OK) {
        Debug("Adding [%s] with ts %lu", key.c_str(), pp->GetValue(key).GetTimestamp());
        txn.addReadSet(key, pp->GetValue(key).GetTimestamp());
    }

}

void
BufferClient::MultiGet(const uint64_t tid, const vector<string> &keys, Promise *promise)
{
    txns_lock.lock();
    if (txns.find(tid) == txns.end()) {
        txnclient->Begin(tid);
    }
    Transaction txn = txns[tid];
    txns_lock.unlock();

    vector<string> keysToRead;
    for (auto key : keys) {
        // Read your own writes, check the write set first.
        if (txn.getWriteSet().find(key) == txn.getWriteSet().end()) {
            keysToRead.push_back(key);
        }
    }

    if (keysToRead.size() > 0) {
        // Get latest value from server, ignoring consistent reads
        Promise p(GET_TIMEOUT);
        Promise *pp = (promise != NULL) ? promise : &p;

        txnclient->MultiGet(tid, keysToRead, pp);
        if (pp->GetReply() == REPLY_OK){
            std::map<string,Version> values = pp->GetValues();
            for (auto value : values) {
                Debug("Adding [%s] with ts %lu", value.first.c_str(), value.second.GetTimestamp());
                txn.addReadSet(value.first, value.second.GetTimestamp());
            }
        }
    }
}
        

/* Set value for a key. (Always succeeds).
 * Returns 0 on success, else -1. */
void
BufferClient::Put(const uint64_t tid, const string &key, const string &value, Promise *promise)
{
    txns_lock.lock();
    if (txns.find(tid) == txns.end()) {
        txnclient->Begin(tid);
    }
    Transaction txn = txns[tid];
    txns_lock.unlock();

    // Update the write set.
    txn.addWriteSet(key, value);
    promise->Reply(REPLY_OK);
}

/* Prepare the transaction. */
void
BufferClient::Prepare(const uint64_t tid, Promise *promise)
{
    if (txns.find(tid) != txns.end()) {
        txnclient->Prepare(tid, txns[tid], promise);
    }
}

void
BufferClient::Commit(const uint64_t tid, const Timestamp &timestamp, Promise *promise)
{
    if (txns.find(tid) != txns.end()) {
        txnclient->Commit(tid, txns[tid], timestamp, promise);
        txns.erase(tid);
    }
}

/* Aborts the ongoing transaction. */
void
BufferClient::Abort(const uint64_t tid, Promise *promise)
{
    if (txns.find(tid) != txns.end()) {
        txnclient->Abort(tid, txns[tid], promise);
        txns.erase(tid);
    }

}
