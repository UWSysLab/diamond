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
#include "diamond-proto.pb.h"

#include <string>
#include <mutex>
#include <condition_variable>

namespace diamond {
namespace frontend {

class Client : public TransportReceiver, public TxnClient
{
public:
    /* Constructor needs path to shard config. */
    Client(const std::string &configPath, 
	   Transport *transport,
	   uint64_t client_id);
    ~Client();

    void Begin(const uint64_t tid);
    void BeginRO(const uint64_t tid,
                 const Timestamp &timestamp = MAX_TIMESTAMP);
    
    // Get the value corresponding to key (valid at given timestamp).
    void Get(const uint64_t tid,
             const std::string &key,
             const Timestamp &timestamp = MAX_TIMESTAMP,
             Promise *promise = NULL);

    void MultiGet(const uint64_t tid,
                  const std::vector<std::string> &key,
                  const Timestamp &timestamp = MAX_TIMESTAMP,
                  Promise *promise = NULL);

    // Set the value for the given key.
    void Put(const uint64_t tid,
             const std::string &key,
             const std::string &value,
             Promise *promise = NULL);

    // Prepare the transaction.
    void Prepare(const uint64_t tid,
                 const Transaction &txn = Transaction(),
                 Promise *promise = NULL);

    // Commit all Get(s) and Put(s) since Begin().
    void Commit(const uint64_t tid,
                const Transaction &txn = Transaction(),
                Promise *promise = NULL);
    
    // Abort all Get(s) and Put(s) since Begin().
    void Abort(const uint64_t tid,
               Promise *promise = NULL);

private:
    transport::Configuration *config;
    Transport *transport; // Transport layer.
    uint64_t client_id; // Unique ID for this client.
    uint32_t msgid;
    
    std::map<uint32_t, Promise *> waiting; // waiting threads
    
    void ReceiveMessage(const TransportAddress &remote,
                        const std::string &type,
                        const std::string &data);

};

} // namespace frontend
} // namespace diamond

#endif /* _DIAMOND_FRONTEND_CLIENT_H_ */
