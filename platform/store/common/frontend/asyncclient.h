// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * store/common/frontend/asyncclient.h
 *   Asynchronous storage client interface.
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

#ifndef _ASYNC_CLIENT_H_
#define _ASYNC_CLIENT_H_

#include "store/common/promise.h"
#include "store/common/timestamp.h"
#include "store/common/transaction.h"

#include <string>
#include <set>

typedef std::function<void (Promise &)> callback_t;
typedef std::function<void (const Timestamp, const std::set<string> &)> publish_handler_t;

class AsyncClient
{
public:
    AsyncClient() { };
    virtual ~AsyncClient() { };

    virtual void Begin(const uint64_t tid) { };
    virtual void BeginRO(const uint64_t tid,
                         const Timestamp timestamp = MAX_TIMESTAMP) { };

    // Get the value corresponding to key (valid at given timestamp).
    virtual void MultiGet(const uint64_t tid,
                          const std::set<std::string> &key,
			  callback_t callback,
                          const Timestamp timestamp = MAX_TIMESTAMP) = 0;

    // Set the value for the given key.
    virtual void Put(const uint64_t tid,
                     const std::string &key,
                     const std::string &value,
                     callback_t callback)
        {Panic("Should not call this");};

    // Prepare the transaction.
    virtual void Prepare(const uint64_t tid,
                         callback_t callback,
                         const Transaction &txn = Transaction())
        {Panic("Should not call this here");};

    // Commit all Get(s) and Put(s) since Begin().
    virtual void Commit(const uint64_t tid,
                        callback_t callback,
                        const Transaction &txn = Transaction()) = 0;

    // Abort all Get(s) and Put(s) since Begin().
    virtual void Abort(const uint64_t tid,
                       callback_t callback) = 0;

    // Subscribe to notifications
    virtual void Subscribe(const uint64_t reactive_id,
                           const std::set<std::string> &keys,
                           const Timestamp timestamp,
                           callback_t callback) = 0;

    virtual void Unsubscribe(const uint64_t reactive_id,
                             const std::set<std::string> &keys,
                             callback_t callback) = 0;

    virtual void Ack(const uint64_t reactive_id,
                     const std::set<std::string> &keys,
                     const Timestamp timestamp,
                     callback_t callback) = 0;

    virtual void SetPublish(publish_handler_t publish) = 0;
};

#endif /* _ASYNC_CLIENT_H_ */
