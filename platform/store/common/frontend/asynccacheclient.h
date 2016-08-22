// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * store/common/frontend/asynccacheclient.h:
 *   An asynchronous caching client implementation
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

#ifndef _ASYNC_CACHE_CLIENT_H_
#define _ASYNC_CACHE_CLIENT_H_

#include "includes/error.h"
#include "lib/assert.h"
#include "lib/message.h"
#include "store/common/transaction.h"
#include "store/common/promise.h"
#include "store/common/frontend/asyncclient.h"
#include "store/common/backend/versionstore.h"

#include <string>
#include <set>

class AsyncCacheClient : public AsyncClient
{

public:
    AsyncCacheClient(AsyncClient *client);
    virtual ~AsyncCacheClient();

    virtual void Begin(const uint64_t tid);
    virtual void BeginRO(const uint64_t tid,
                         const Timestamp timestamp = MAX_TIMESTAMP);

    virtual void MultiGet(const uint64_t tid,
                          const std::set<std::string> &key,
                          callback_t callback,
                          const Timestamp timestamp = MAX_TIMESTAMP);

    // Set the value for the given key.
    virtual void Put(const uint64_t tid,
                     const std::string &key,
                     const std::string &value,
                     callback_t callback);

    // Prepare the transaction.
    virtual void Prepare(const uint64_t tid,
                         callback_t callback,
                         const Transaction &txn = Transaction());

    // Commit all Get(s) and Put(s) since Begin().
    virtual void Commit(const uint64_t tid,
                        callback_t callback,
                        const Transaction &txn = Transaction());
    
    // Abort all Get(s) and Put(s) since Begin().
    virtual void Abort(const uint64_t tid,
               callback_t callback);

    // Subscribe to notifications
    virtual void Subscribe(const uint64_t reactive_id,
                   const std::set<std::string> &keys,
                   const Timestamp timestamp,
                   callback_t callback);

    virtual void Unsubscribe(const uint64_t reactive_id,
                     const std::set<std::string> &keys,
                     callback_t callback);

    virtual void Ack(const uint64_t reactive_id,
             const std::set<std::string> &keys,
             const Timestamp timestamp,
             callback_t callback);

    virtual void SetCaching(bool cachingEnabled);

    virtual void SetPublish(publish_handler_t publish);
protected:
    // Underlying single shard transaction client implementation.
    AsyncClient* client;
    publish_handler_t publish;
    
    // Read cache
    VersionedKVStore cache;
    std::mutex cache_lock;
    std::map<uint64_t, Transaction> prepared;

    // Flag controlling whether caching is used on Get and Multiget
    bool cachingEnabled;

    // Callbacks
    void GetCallback(callback_t callback,
                     Promise &promise);
    void PrepareCallback(callback_t callback,
                         const uint64_t tid,
                         const Transaction txn,
                         Promise &promise);
    void CommitCallback(callback_t callback,
                        const uint64_t tid,
                        const Transaction txn,
                        Promise &promise);
    void AbortCallback(callback_t callback,
                       const uint64_t tid,
                       Promise &promise);
};

#endif /* _ASYNC_CACHE_CLIENT_H_ */
