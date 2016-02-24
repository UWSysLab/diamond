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

class Client : public TxnClient
{
public:
    Client(string configPath, int nshards, int closestReplica);
    ~Client();

    // Overriding functions from TxnClient
    // Begin a transaction.
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

    // Blocking versions
    int Get(const uint64_t tid,
            const std::string &key,
            Version &value,
            const Timestamp &timestamp = MAX_TIMESTAMP);

    int MultiGet(const uint64_t tid,
                 const std::vector<std::string> &key,
                 std::map<string, Version> &values,
                 const Timestamp &timestamp = MAX_TIMESTAMP);

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

    // Blocking commit
    bool Commit(const uint64_t tid,
                const Transaction &txn,
                Timestamp &timestamp);
    
    // Abort all Get(s) and Put(s) since Begin().
    void Abort(const uint64_t tid,
               Promise *promise = NULL);
    std::vector<int> Stats();

    void Subscribe(const std::set<std::string> &keys,
                   Promise *promise = NULL);

    // Blocking version
    int Subscribe(const std::set<std::string> &keys,
                  Timestamp &timestamp);

    void GetNextNotification(Promise *promise);

    void Register(const uint64_t reactive_id,
                  const Timestamp timestamp,
                  const std::set<std::string> &keys,
                  Promise *promise = NULL);

private:
    /* Private helper functions. */
    void run_client(); // Runs the transport event loop.

    // get a timestamp from the timeserver
    uint64_t getTSS();
    
    // timestamp server call back
    void tssCallback(const string &request, const string &reply);

    // local Abort function
    void Abort(const uint64_t tid, const std::map<int, Transaction> &participants);
    // local Prepare function
    int Prepare(const uint64_t tid, const std::map<int, Transaction> &participants, Timestamp &ts);

    // Sharding logic: Given key, generates a number b/w 0 to nshards-1
    uint64_t key_to_shard(const std::string &key, const uint64_t nshards);

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
