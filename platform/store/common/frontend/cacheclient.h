// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * store/common/frontend/cacheclient.h:
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

#ifndef _CACHE_CLIENT_H_
#define _CACHE_CLIENT_H_

#include "includes/error.h"
#include "lib/assert.h"
#include "lib/message.h"
#include "store/common/transaction.h"
#include "store/common/promise.h"
#include "store/common/frontend/txnclient.h"
#include "store/common/backend/versionstore.h"

#include <string>
#include <vector>

class CacheClient {

public:
    CacheClient(TxnClient *txnclient);
    ~CacheClient();

    // Begin a transaction with given tid.
    void Begin(uint64_t tid);

    // Get value corresponding to key.
    void Get(const uint64_t tid, const string &key, Promise *promise = NULL);

    // Get value corresponding to key.
    void Get(const uint64_t tid, const string &key, const Timestamp &timestamp, Promise *promise = NULL);
    
    // Get value corresponding to key.
    void MultiGet(const uint64_t tid, const std::vector<string> &keys, Promise *promise = NULL);

    // Get value corresponding to keys.
    void MultiGet(const uint64_t tid, const std::vector<string> &keys, const Timestamp &timestamp, Promise *promise = NULL);

    // Put value for given key.
    void Put(const uint64_t tid, const string &key, const string &value, Promise *promise = NULL);

    // Prepare 
    void Prepare(const uint64_t tid, const Transaction &txn, Promise *promise = NULL); 

    // Commit the transaction.
    void Commit(const uint64_t tid, const Transaction &txn, const Timestamp &timestamp = 0, Promise *promise = NULL);

    // Abort the running transaction.
    void Abort(const uint64_t tid, const Transaction &txn, Promise *promise = NULL);

private:
    // Underlying single shard transaction client implementation.
    TxnClient* txnclient;

    // Read cache
    VersionedKVStore cache;
    std::mutex cache_lock;
};

#endif /* _CACHE_CLIENT_H_ */