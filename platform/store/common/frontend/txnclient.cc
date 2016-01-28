// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * store/common/frontend/txnclient.cc
 *   Client interface for a single replicated shard.
 *
 * Copyright 2015 Irene Zhang <iyzhang@cs.washington.edu>
 *                Naveen Kr. Sharma <nksharma@cs.washington.edu>
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

#include "store/common/frontend/txnclient.h"

using namespace std;

TxnClient::TxnClient() { }
TxnClient::~TxnClient() { }

void
TxnClient::Begin(const uint64_t tid)
{
    Panic("Unimplemented BEGIN");
}
    
void
TxnClient::Get(const uint64_t tid,
               const string &key,
               Promise *promise)
{
    Panic("Unimplemented GET");
    return;
}

void
TxnClient::Get(const uint64_t tid, 
               const string &key,
               const Timestamp &timestamp,
               Promise *promise)
{
    Panic("Unimplemented GET");
    return;
}

void
TxnClient::MultiGet(const uint64_t tid,
                    const vector<string> &keys,
                    Promise *promise)
{
    Panic("Unimplemented MULTIGET");
    return;
}

void
TxnClient::MultiGet(const uint64_t tid, 
                    const vector<string> &keys,
                    const Timestamp &timestamp,
                    Promise *promise)
{
    Panic("Unimplemented MULTIGET");
    return;
}

void
TxnClient::Put(const uint64_t tid,
               const string &key,
               const string &value,
               Promise *promise)
{
    Panic("Unimplemented PUT");
    return;
}

void
TxnClient::Prepare(const uint64_t tid,
                   const Transaction &txn,
                   Promise *promise)
{
    Panic("Unimplemented PREPARE");
}

void
TxnClient::Commit(const uint64_t tid,
                  const Transaction &txn,
                  const Timestamp &timestamp,
                  Promise *promise)
{
    Panic("Unimplemented COMMIT");
    return;
}
    
void
TxnClient::Abort(const uint64_t tid,
                 const Transaction &txn,
                 Promise *promise)
{
    Panic("Unimplemented ABORT");
    return;
}

/* Takes a key and number of shards; returns shard corresponding to key. */
uint64_t
TxnClient::key_to_shard(const string &key, const uint64_t nshards)
{
    uint64_t hash = 5381;
    const char* str = key.c_str();
    for (unsigned int i = 0; i < key.length(); i++) {
        hash = ((hash << 5) + hash) + (uint64_t)str[i];
    }

    return (hash % nshards);
}
