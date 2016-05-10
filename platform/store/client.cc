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
        cclient[i] = new ShardClient(shardConfigPath,
				     &transport,
				     client_id, i,
				     closestReplica);
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

void
Client::Get(const uint64_t tid,
	    const string &key,
	    callback_t callback,
	    const Timestamp &timestamp)
{
    // Contact the appropriate shard to get the value.
    int i = key_to_shard(key, nshards);
    cclient[i]->Get(tid, key, callback, timestamp);
}

void
Client::MultiGet(const uint64_t tid,
		 const vector<string> &keys,
                 callback_t callback,
                 const Timestamp &timestamp)
{
    map<int, vector<string>> participants;

    for (auto &key : keys) {
        int i = key_to_shard(key, nshards);
        participants[i].push_back(key);
    }

    vector<Promise *> *promises = new vector<Promise *>();
    callback_t cb = bind(&Client::MultiGetCallback,
			 this, callback,
			 participants.size(),
			 promises,
			 placeholders::_1);
    for (auto &p : participants) {
        // Send the GET operation to appropriate shard.
	cclient[p.first]->MultiGet(tid, p.second, 
				   cb, timestamp);
    }
}
    
void
Client::MultiGetCallback(callback_t callback,
			 size_t total,
			 vector<Promise *> *promises,
			 Promise *promise)
{
    promises->push_back(promise);
    Debug("MultiGetCall back"); 
    if (promises->size() == total) {
	map<string, Version> values;
	int r = REPLY_OK;
	for (auto p : *promises) {
	    if (p->GetReply() != REPLY_OK) {
		r = p->GetReply();
	    }
	    for (auto &v : p->GetValues()) {
		values[v.first] = v.second;
	    }
	    delete p;
	}
	delete promises;
	Promise *pp = new Promise();
	pp->Reply(r, values);
	callback(pp);
    }
}

void
Client::PrepareInternal(const uint64_t tid,
			const map<int, Transaction> &participants,
			callback_t callback)
{
    // 1. Send commit-prepare to all shards.
    Debug("PREPARE Transaction");
    vector<Promise *> *promises = new vector<Promise *>();
    uint64_t *ts = new uint64_t();
    *ts = 0;
    callback_t cb =
	bind(&Client::PrepareCallback,
	     this, callback,
	     participants.size(),
	     promises, ts,
	     placeholders::_1);

    for (auto &p : participants) {
        Debug("Sending prepare to shard [%d]", p.first);
	cclient[p.first]->Prepare(tid, cb,
				  p.second);
    }

    // In the meantime ... go get a timestamp for OCC
    transport.Timer(0, [=]() {
            Debug("Sending request to TimeStampServer");
	    function<void (const string &, const string &)> cb =
		bind(&Client::tssCallback,
		     this, callback,
		     participants.size(),
		     promises, ts,
		     placeholders::_1,
		     placeholders::_2);
            tss->Invoke("", cb);});
}

void
Client::tssCallback(callback_t callback,
		    size_t total,
		    vector<Promise *> *promises,
		    uint64_t *ts,
		    const string &request,
		    const string &reply)
{
    Debug("tsscallback back"); 

    *ts = stol(reply, NULL, 10);
    PrepareCallback(callback,
		    total,
		    promises, ts,
		    NULL);
}
		
/* Attempts to commit the ongoing transaction. */
void
Client::Commit(const uint64_t tid,
	       callback_t callback,
	       const Transaction &txn)
{
    // Implementing 2 Phase Commit
    map<int, Transaction> participants;

    // split up the transaction across shards
    for (auto &r : txn.GetReadSet()) {
        int i = key_to_shard(r.first, nshards);
        if (participants.find(i) == participants.end()) {
            participants[i] =
		Transaction(txn.IsolationMode(),
			    txn.GetTimestamp());
        }
        participants[i].AddReadSet(r.first, r.second);
    }

    for (auto &w : txn.GetWriteSet()) {
        int i = key_to_shard(w.first, nshards);
        if (participants.find(i) == participants.end()) {
            participants[i] =
		Transaction(txn.IsolationMode(),
			    txn.GetTimestamp());
        }
        participants[i].AddWriteSet(w.first, w.second);
    }

    for (auto &inc : txn.GetIncrementSet()) {
        int i = key_to_shard(inc.first, nshards);
        if (participants.find(i) == participants.end()) {
            participants[i] =
		Transaction(txn.IsolationMode(),
			    txn.GetTimestamp());
        }
        participants[i].AddIncrementSet(inc.first,
					inc.second);
    }

    // Do two phase commit for linearizable and SI
    function<void (Promise *)> cb =
	bind(&Client::CommitCallback,
	     this,
	     tid,
	     callback,
	     participants,
	     placeholders::_1);
    PrepareInternal(tid, participants, cb);
}

void
Client::PrepareCallback(callback_t callback,
			size_t total,
			vector<Promise *> *promises,
			uint64_t *ts,
			Promise *promise)
{
    Debug("Prepare callback"); 
    if (promise != NULL) {
	promises->push_back(promise);
    }
    Debug("Prepare callback size %lu", promises->size()); 
    // check whether we're done
    if (promises->size() == total && *ts > 0) {
	int r = REPLY_OK;
	for (auto p : *promises) {
	    // check what the reply was
	    if (p->GetReply() != REPLY_OK) {
		r = p->GetReply();
	    }
	    delete p;
	}

	Promise *pp = new Promise();
	pp->Reply(r, *ts);
	delete promises;
	delete ts;
	// call commit callback
	callback(pp);
    }

}

void
Client::CommitCallback(uint64_t tid,
		       callback_t callback,
		       map<int, Transaction> participants,
		       Promise *promise) {

    if (promise->GetReply() == REPLY_OK) {
	// Send commits
	Debug("COMMIT Transaction at [%lu]",
	      promise->GetTimestamp());
	for (auto &p : participants) {
	    Transaction &txn2 = p.second;
	    if (txn2.IsolationMode() == LINEARIZABLE ||
		txn2.IsolationMode() == SNAPSHOT_ISOLATION) {
		Debug("Sending commit to shard [%d]",
		      p.first);
		txn2.SetTimestamp(promise->GetTimestamp());
		cclient[p.first]->Commit(tid, NULL, txn2);
	    }
	}
    } else {
	AbortInternal(tid, participants);
    }
    callback(promise);
}    

void
Client::AbortInternal(const uint64_t tid,
		      const map<int, Transaction> &participants) {
    for (auto &p : participants) {
	if (p.second.IsolationMode() == LINEARIZABLE ||
	    p.second.IsolationMode() == SNAPSHOT_ISOLATION) {
	    cclient[p.first]->Abort(tid);
	}
    }
}    

/* Aborts the ongoing transaction. */
void
Client::Abort(const uint64_t tid)
{
    // Ignore external abort calls at this level
    Debug("ABORT Transaction");
}

    
/* Return statistics of most recent transaction. */
vector<int>
Client::Stats()
{
    vector<int> v;
    return v;
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

void
Client::Subscribe(const set<string> &keys,
                  const TransportAddress &address,
		  callback_t callback) {
    map<int, set<string> > participants;

    for (auto &key : keys) {
        int i = key_to_shard(key, nshards);
        participants[i].insert(key);
    }

    if (participants.size() == 0) {
        Promise *w = new Promise();
        w->Reply(REPLY_OK);
        callback(w);
    }
    else {
        vector<Promise *> *promises = new vector<Promise *>();
        callback_t cb = bind(&Client::MultiGetCallback,
                             this, callback,
                             participants.size(),
                             promises,
                             placeholders::_1);
        for (auto &p : participants) {
            cclient[p.first]->Subscribe(p.second, address, cb);
        }
    }
}

void
Client::Unsubscribe(const set<string> &keys,
                  const TransportAddress &address,
		  callback_t callback) {
    map<int, set<string> > participants;

    for (auto &key : keys) {
        int i = key_to_shard(key, nshards);
        participants[i].insert(key);
    }

    if (participants.size() == 0) {
        Promise *w = new Promise();
        w->Reply(REPLY_OK);
        callback(w);
    }
    else {
        vector<Promise *> *promises = new vector<Promise *>();
        callback_t cb = bind(&Client::MultiGetCallback,
                             this, callback,
                             participants.size(),
                             promises,
                             placeholders::_1);
        for (auto &p : participants) {
            cclient[p.first]->Unsubscribe(p.second, address, cb);
        }
    }
}

} // namespace strongstore
