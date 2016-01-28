// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * store/strongstore/client.h:
 *   Transactional client interface.
 *
 * Copyright 2015 Irene Zhang  <iyzhang@cs.washington.edu>
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
 
#ifndef _STRONG_CLIENT_H_
#define _STRONG_CLIENT_H_

#include "lib/assert.h"
#include "lib/message.h"
#include "lib/configuration.h"
#include "lib/tcptransport.h"
#include "store/common/frontend/txnclient.h"
#include "store/common/frontend/client.h"
#include "replication/client.h"
#include "store/common/frontend/cacheclient.h"
#include "store-proto.pb.h"
#include "store/shardclient.h"

#include <condition_variable>
#include <mutex>
#include <string>
#include <set>
#include <thread>

namespace strongstore {

class Client : public ::TxnClient
{
public:
    Client(string configPath, int nshards, int closestReplica);
    ~Client();

    // Overriding functions from ::Client
    void Begin(const uint64_t tid);
    int Get(const uint64_t tid, const std::string &key,
            Version &value);
    int Get(const uint64_t tid, const std::string &key,
            const Timestamp &timestamp,
            Version &value);

    int MultiGet(const uint64_t tid, const std::vector<std::string> &keys, std::map<std::string, Version> &value);

    int MultiGet(const uint64_t tid, const std::vector<std::string> &key,
                 const Timestamp &timestamp,
                 std::map<std::string, Version> &value);

    bool Commit(const uint64_t tid, const Transaction &txn, Timestamp &ts);
    void Abort(const uint64_t tid, const Transaction &txn);
    std::vector<int> Stats();

private:
    /* Private helper functions. */
    void run_client(); // Runs the transport event loop.

    // timestamp server call back
    void tssCallback(const string &request, const string &reply);

    // local Abort function
    void Abort(const uint64_t tid, const std::map<int, Transaction> &participants);
    // local Prepare function
    int Prepare(const uint64_t tid, const std::map<int, Transaction> &participants, Timestamp &ts);
    // Unique ID for this client.
    uint64_t client_id;

    // index of closest replica to read 
    int closestReplica = 0;
    
    // Number of shards in SpanStore.
    long nshards;

    // Transport used by paxos client proxies.
    TCPTransport transport;
    
    // Thread running the transport event loop.
    std::thread *clientTransport;

    // Caching client for each shard.
    std::vector<CacheClient *> cclient;

    // Timestamp server shard.
    replication::VRClient *tss; 

    // Synchronization variables.
    std::condition_variable cv;
    std::mutex cv_m;
    string replica_reply;

    // Time spend sleeping for commit.
    int commit_sleep;
};

} // namespace strongstore

#endif /* _STRONG_CLIENT_H_ */
