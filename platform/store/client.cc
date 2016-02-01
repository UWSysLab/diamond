// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * store/strongstore/client.cc:
 *   Client to transactional storage system with strong consistency
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

#include "client.h"

using namespace std;

namespace strongstore {

Client::Client(string configPath, int nShards,
                int closestReplica)
    : transport()
{
    // Initialize all state here;
    client_id = 0;
    while (client_id == 0) {
        random_device rd;
        mt19937_64 gen(rd());
        uniform_int_distribution<uint64_t> dis;
        client_id = dis(gen);
    }

    nshards = nShards;
    cclient.reserve(nshards);

    Debug("Initializing Diamond Store client with id [%lu]", client_id);

    /* Start a client for time stamp server. */
    string tssConfigPath = configPath + ".tss.config";
    ifstream tssConfigStream(tssConfigPath);
    if (tssConfigStream.fail()) {
	fprintf(stderr, "unable to read configuration file: %s\n",
		tssConfigPath.c_str());
    }
    transport::Configuration tssConfig(tssConfigStream);
    tss = new replication::VRClient(tssConfig, &transport);
    
    /* Start a client for each shard. */
    for (int i = 0; i < nShards; i++) {
        string shardConfigPath = configPath + to_string(i) + ".config";
        ShardClient *shardclient = new ShardClient(shardConfigPath,
                                                   &transport,
                                                   client_id, i,
                                                   closestReplica);
        cclient[i] = new CacheClient(shardclient);
    }

    /* Run the transport in a new thread. */
    clientTransport = new thread(&Client::run_client, this);

    Debug("Diamond Store client [%lu] created!", client_id);
}

Client::~Client()
{
    transport.Stop();
    delete tss;
    for (auto b : cclient) {
        delete b;
    }
    clientTransport->join();
}

/* Runs the transport event loop. */
void
Client::run_client()
{
    transport.Run();
}

/* Begins a transaction. All subsequent operations before a commit() or
 * abort() are part of this transaction.
 *
 * Return a TID for the transaction.
 */
void
Client::Begin(const uint64_t tid)
{
    Debug("BEGIN Transaction");
}

/* Returns the value corresponding to the supplied key. */
int
Client::Get(const uint64_t tid, const string &key, Version &value)
{
    // Contact the appropriate shard to get the value.
    int i = key_to_shard(key, nshards);

    // Send the GET operation to appropriate shard.
    Promise promise(GET_TIMEOUT);

    cclient[i]->Get(tid, key, &promise);
    value = promise.GetValue(key);

    return promise.GetReply();
}

int
Client::Get(const uint64_t tid, const string &key, const Timestamp &t, Version &value)
{
    // Contact the appropriate shard to get the value.
    int i = key_to_shard(key, nshards);

    // Send the GET operation to appropriate shard.
    Promise promise(GET_TIMEOUT);

    cclient[i]->Get(tid, key, t, &promise);
    value = promise.GetValue(key);

    return promise.GetReply();
}

int
Client::MultiGet(const uint64_t tid, const vector<string> &keys,
                 map<string, Version> &values)
{
    map<int, vector<string>> participants;

    for (auto &key : keys) {
        int i = key_to_shard(key, nshards);
        participants[i].push_back(key);
    }

    vector<Promise *> promises;
    for (auto &p : participants) {
        // Send the GET operation to appropriate shard.
        Promise *promise = new Promise(GET_TIMEOUT);
        promises.push_back(promise);

        cclient[p.first]->MultiGet(tid, p.second, promise);
    }

    int r = REPLY_OK;
    for (auto p : promises) {
        if (p->GetReply() != REPLY_OK) {
            r = p->GetReply();
        }
        for (auto &v : p->GetValues()) {
            values[v.first] = v.second;
        }
        delete p;
    }
    return r;
}
int
Client::MultiGet(const uint64_t tid, const vector<string> &keys,
                 const Timestamp &timestamp,
                 map<string, Version> &values)
{
    map<int, vector<string>> participants;

    for (auto &key : keys) {
        int i = key_to_shard(key, nshards);
        participants[i].push_back(key);
    }

    vector<Promise *> promises;
    for (auto &p : participants) {
        // Send the GET operation to appropriate shard.
        promises.push_back(new Promise(GET_TIMEOUT));

        cclient[p.first]->MultiGet(tid, p.second, timestamp, promises.back());
    }

    int r = REPLY_OK;
    for (auto p : promises) {
        if (p->GetReply() != REPLY_OK) {
            r = p->GetReply();
        }
        for (auto &v : p->GetValues()) {
            values[v.first] = v.second;
        }
        delete p;
    }
    return r;
}
/* Sets the value corresponding to the supplied key. */
// int
// Client::Put(const uint64_t tid, const string &key, const string &value)
// {
//     Warning("Should not call PUT on backend store");
//     return REPLY_OK;
// }

void
Client::Put(const uint64_t tid,
	    const std::string &key,
	    const std::string &value,
	    Promise *promise)
{
    Panic("Don't call this function!");
}
    
void
Client::Prepare(const uint64_t tid, 
		const Transaction &txn,
		Promise *promise)
{
    Panic("Don't call this function!");
}
    
int
Client::Prepare(const uint64_t tid, const map<int, Transaction> &participants, Timestamp &ts)
{
    int status;

    // 1. Send commit-prepare to all shards.
    Debug("PREPARE Transaction");
    list<Promise *> promises;

    for (auto &p : participants) {
        Debug("Sending prepare to shard [%d]", p.first);
        promises.push_back(new Promise(PREPARE_TIMEOUT));
        cclient[p.first]->Prepare(tid, p.second, promises.back());
    }

    // In the meantime ... go get a timestamp for OCC
    unique_lock<mutex> lk(cv_m);

    transport.Timer(0, [=]() { 
            Debug("Sending request to TimeStampServer");
            tss->Invoke("", bind(&Client::tssCallback, this,
                                 placeholders::_1,
                                 placeholders::_2));
        });
        
    Debug("Waiting for TSS reply");
    cv.wait(lk);
    ts = stol(replica_reply, NULL, 10);
    Debug("TSS reply received: %lu", ts);

    // 2. Wait for reply from all shards. (abort on timeout)
    Debug("Waiting for PREPARE replies");

    status = REPLY_OK;
    for (auto p : promises) {
        // If any shard returned false, abort the transaction.
        if (p->GetReply() != REPLY_OK) {
            if (status != REPLY_FAIL) {
                status = p->GetReply();
            }
        }
        // Also, find the max of all prepare timestamp returned.
        if (p->GetTimestamp() > ts) {
            ts = p->GetTimestamp();
        }
        delete p;
    }
    return status;
}

/* Attempts to commit the ongoing transaction. */
bool
Client::Commit(const uint64_t tid, const Transaction &txn, Timestamp &ts)
{
    // Implementing 2 Phase Commit
    ts = 0;
    int status;
    map<int, Transaction> participants;

    // split up the transaction across shards
    for (auto &r : txn.getReadSet()) {
        int i = key_to_shard(r.first, nshards);
        participants[i].addReadSet(r.first, r.second);
    }

    for (auto &w : txn.getWriteSet()) {
        int i = key_to_shard(w.first, nshards);
        participants[i].addWriteSet(w.first, w.second);
    }
    
    for (int i = 0; i < COMMIT_RETRIES; i++) {
        status = Prepare(tid, participants, ts);
        if (status == REPLY_OK || status == REPLY_FAIL) {
            break;
        }
    }

    if (status == REPLY_OK) {
        // Send commits
        Debug("COMMIT Transaction at [%lu]", ts);

        for (auto &p : participants) {
            Debug("Sending commit to shard [%d]", p.first);
            cclient[p.first]->Commit(tid, p.second, ts);
        }
        return true;
    }

    // 4. If not, send abort to all shards.
    Abort(tid, participants);
    return false;
}

void
Client::Abort(const uint64_t tid, const map<int, Transaction> &participants) {
    for (auto &p : participants) {
        cclient[p.first]->Abort(tid, p.second);
    }
}    
/* Aborts the ongoing transaction. */
void
Client::Abort(const uint64_t tid, const Transaction &txn)
{
    Debug("ABORT Transaction");
    map<int, Transaction> participants;

    // split up the transaction across shards
    for (auto r : txn.getReadSet()) {
        int i = key_to_shard(r.first, nshards);
        participants[i].addReadSet(r.first, r.second);
    }

    for (auto w : txn.getWriteSet()) {
        int i = key_to_shard(w.first, nshards);
        participants[i].addWriteSet(w.first, w.second);
    }

    Abort(tid, participants);
}

/* Return statistics of most recent transaction. */
vector<int>
Client::Stats()
{
    vector<int> v;
    return v;
}

/* Callback from a tss replica upon any request. */
void
Client::tssCallback(const string &request, const string &reply)
{
    lock_guard<mutex> lock(cv_m);
    Debug("Received TSS callback [%s]", reply.c_str());

    // Copy reply to "replica_reply".
    replica_reply = reply;
    
    // Wake up thread waiting for the reply.
    cv.notify_all();
}

/* Takes a key and number of shards; returns shard corresponding to key. */
uint64_t
Client::key_to_shard(const string &key, const uint64_t nshards)
{
    uint64_t hash = 5381;
    const char* str = key.c_str();
    for (unsigned int i = 0; i < key.length(); i++) {
        hash = ((hash << 5) + hash) + (uint64_t)str[i];
    }

    return (hash % nshards);
}

} // namespace strongstore
