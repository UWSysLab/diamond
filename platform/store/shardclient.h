// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * store/strongstore/strongclient.h:
 *   Single shard strong consistency transactional client interface.
 *
 * Copyright 2015 Irene Zhang <iyzhang@cs.washington.edu>
 *                Naveen Kr. Sharma <naveenks@cs.washington.edu>
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

#ifndef _STRONG_SHARDCLIENT_H_
#define _STRONG_SHARDCLIENT_H_

#include "lib/assert.h"
#include "lib/message.h"
#include "lib/transport.h"
#include "includes/error.h"
#include "store/common/frontend/client.h"
#include "replication/client.h"
#include "store/common/frontend/asyncclient.h"
#include "store/common/timestamp.h"
#include "store/common/transaction.h"
#include "store-proto.pb.h"

namespace strongstore {

class ShardClient : public AsyncClient
{
public:
    /* Constructor needs path to shard config. */
    ShardClient(
        const std::string &configPath, 
        Transport *transport,
        const uint64_t client_id,
        const int shard,
        const int closestReplica);
    ~ShardClient();
    
    // Get the value corresponding to key (valid at given timestamp).
    virtual void MultiGet(const uint64_t tid,
                          const std::set<std::string> &key,
                          callback_t callback,
                          const Timestamp timestamp = MAX_TIMESTAMP);

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

    virtual void SetPublish(publish_handler_t publish);

private:
    Transport *transport; // Transport layer.
    uint64_t client_id; // Unique ID for this client.
    int shard; // which shard this client accesses
    int replica; // which replica to use for reads

    replication::VRClient *client; // Client proxy.

    /* Timeout for Get requests, which only go to one replica. */
    void GetTimeout();

    /* Callbacks for hearing back from a shard for an operation. */
    void GetCallback(callback_t callback,
                     const std::string &,
                     const std::string &);
    void PrepareCallback(callback_t callback,
                         const std::string &,
                         const std::string &);
    void GenericCallback(callback_t callback,
                         const std::string &,
                         const std::string &);
    void HandlePublish(publish_handler_t publish,
                       const string &type,
                       const string &data);
    /* Helper Functions for starting and finishing requests */
    void StartRequest();
    void WaitForResponse();
    void FinishRequest(const std::string &reply_str);
    void FinishRequest();
    int SendGet(const std::string &request_str);

};

} // namespace strongstore

#endif /* _STRONG_SHARDCLIENT_H_ */
