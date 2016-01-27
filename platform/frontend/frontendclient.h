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

#ifndef _DIAMOND_FRONTEND_CLIENT_H_
#define _DIAMOND_FRONTEND_CLIENT_H_

#include "lib/assert.h"
#include "lib/message.h"
#include "lib/transport.h"
#include "includes/error.h"
#include "store/common/frontend/txnclient.h"
#include "store/common/timestamp.h"
#include "store/common/transaction.h"
#include "store-proto.pb.h"

#include <string>
#include <mutex>
#include <condition_variable>

namespace diamond {

class FrontendClient : public TransportReceiver, public TxnClient
{
public:
    /* Constructor needs path to shard config. */
    FrontendClient(
        const std::string &configPath, 
        Transport *transport,
        uint64_t client_id);
    ~ShardClient();

    // Overriding from TxnClient
    void Begin(uint64_t id);
    void Get(uint64_t id,
            const std::string &key,
            Promise *promise = NULL);
    void Get(uint64_t id,
            const std::string &key,
            const Timestamp &timestamp, 
            Promise *promise = NULL);
    void Prepare(uint64_t id, 
                 const Transaction &txn,
                 const Timestamp &timestamp = Timestamp(),
                 Promise *promise = NULL);
    void Commit(uint64_t id, 
                const Transaction &txn,
                uint64_t timestamp,
                Promise *promise = NULL);
    void Abort(uint64_t id, 
               const Transaction &txn,
               Promise *promise = NULL);

private:
    Transport *transport; // Transport layer.
    uint64_t client_id; // Unique ID for this client.

    Promise *waiting; // waiting thread

    void ReceiveMessage(const TransportAddress &remote,
                        const std::string &type,
                        const std::string &data);

};

} // namespace diamond

#endif /* _DIAMOND_FRONTEND_CLIENT_H_ */
