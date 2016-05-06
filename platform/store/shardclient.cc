// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * store/txnstore/shardclient.h:
 *   Single shard transactional client interface.
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

#include "shardclient.h"

namespace strongstore {

using namespace std;
using namespace proto;

ShardClient::ShardClient(const string &configPath,
                       Transport *transport, uint64_t client_id, int
                       shard, int closestReplica)
    : transport(transport), client_id(client_id), shard(shard)
{ 
    ifstream configStream(configPath);
    if (configStream.fail()) {
        fprintf(stderr, "unable to read configuration file: %s\n",
                configPath.c_str());
    }
    transport::Configuration config(configStream);

    client = new replication::VRClient(config, transport);

    if (closestReplica == -1) {
	replica = client_id % config.n;
    } else {
	replica = closestReplica;
    }
}

ShardClient::~ShardClient()
{ 
    delete client;
}

/* Sends BEGIN to a single shard indexed by i. */
void
ShardClient::Get(const uint64_t tid,
		 const string &key,
		 callback_t callback,
		 const Timestamp &timestamp)
{
    MultiGet(tid, vector<string>({key}), callback, timestamp);
}

void
ShardClient::MultiGet(const uint64_t tid,
		      const vector<string> &keys,
		      callback_t callback,
                      const Timestamp &timestamp)
{
    // Send the GET operation to appropriate shard.
    Debug("[shard %i] Sending %lu GETS", shard, keys.size());

    // create request
    string request_str;
    Request request;
    request.set_op(Request::GET);
    request.set_txnid(tid);
    for (auto &i : keys) {
        request.mutable_get()->add_keys(i);  
    }
    request.mutable_get()->set_timestamp(timestamp);
    request.SerializeToString(&request_str);

    transport->Timer(0, [=]() {
            client->InvokeUnlogged(replica,
                                   request_str,
                                   bind(&ShardClient::GetCallback,
                                        this,
					callback,
                                        placeholders::_1,
                                        placeholders::_2));
        });
}

void
ShardClient::Prepare(const uint64_t tid,
                     callback_t callback,
		     const Transaction &txn)
{
    Debug("[shard %i] Sending PREPARE: %lu", shard, tid);

    // create prepare request
    string request_str;
    Request request;
    request.set_op(Request::PREPARE);
    request.set_txnid(tid);
    txn.Serialize(request.mutable_prepare()->mutable_txn());
    request.SerializeToString(&request_str);

    transport->Timer(0, [=]() {
            client->Invoke(request_str,
                           bind(&ShardClient::PrepareCallback,
                                this,
				callback,
                                placeholders::_1,
                                placeholders::_2));
        });
}

void
ShardClient::Commit(const uint64_t tid,
		    callback_t callback,
		    const Transaction &txn)
{

    Debug("[shard %i] Sending COMMIT: %lu", shard, tid);

    // create commit request
    string request_str;
    Request request;
    request.set_op(Request::COMMIT);
    request.set_txnid(tid);
    if (txn.IsolationMode() == EVENTUAL) {
        txn.Serialize(request.mutable_commit()->mutable_txn());
    }
    request.mutable_commit()->set_timestamp(txn.GetTimestamp());
    request.SerializeToString(&request_str);

    transport->Timer(0, [=]() {
        client->Invoke(request_str,
            bind(&ShardClient::CommitCallback,
		 this,
		 callback,
		 placeholders::_1,
		 placeholders::_2));
    });
}

/* Aborts the ongoing transaction. */
void
ShardClient::Abort(const uint64_t tid)
{
    Debug("[shard %i] Sending ABORT: %lu", shard, tid);
    
    // create abort request
    string request_str;
    Request request;
    request.set_op(Request::ABORT);
    request.set_txnid(tid);
    request.SerializeToString(&request_str);

    transport->Timer(0, [=]() {
	    client->Invoke(request_str,
			   bind(&ShardClient::AbortCallback,
				this,
				placeholders::_1,
				placeholders::_2));
    });
}

/* Callback from a shard replica on get operation completion. */
void
ShardClient::GetCallback(callback_t callback, const string &request_str, const string &reply_str)
{
    /* Replies back from a shard. */
    Reply reply;
    reply.ParseFromString(reply_str);

    Debug("[shard %i] Received GET callback [%d]",
	  shard, reply.status());
    Promise *w = new Promise();
    map<string, Version> ret;

    if (reply.status() == REPLY_OK) {
        for (int i = 0; i < reply.replies_size(); i++) {
            ReadReply rep = reply.replies(i);
            ret[rep.key()] = Version(rep);
            ASSERT(ret[rep.key()].GetInterval().End() != MAX_TIMESTAMP);
        }
    }
    w->Reply(reply.status(), ret);
    callback(w);
}

/* Callback from a shard replica on prepare operation completion. */
void
ShardClient::PrepareCallback(callback_t callback, const string &request_str, const string &reply_str)
{
    Reply reply;

    reply.ParseFromString(reply_str);
    Debug("[shard %i] Received PREPARE callback [%d]",
	  shard, reply.status());

    Promise *w = new Promise();
    if (reply.has_timestamp()) {
	w->Reply(reply.status(), reply.timestamp());
    } else {
	w->Reply(reply.status(), Timestamp());
    }
    callback(w);
}

/* Callback from a shard replica on commit operation completion. */
void
ShardClient::CommitCallback(callback_t callback, const string &request_str, const string &reply_str)
{
    // COMMITs always succeed.
    Reply reply;
    reply.ParseFromString(reply_str);
    ASSERT(reply.status() == REPLY_OK);
    Debug("[shard %i] Received COMMIT2 callback [%d]",
	  shard, reply.status());
}

/* Callback from a shard replica on abort operation completion. */
void
ShardClient::AbortCallback(const string &request_str, const string &reply_str)
{
    // ABORTs always succeed.
    Reply reply;
    reply.ParseFromString(reply_str);
    ASSERT(reply.status() == REPLY_OK);

    Debug("[shard %i] Received ABORT callback [%d]",
	  shard, reply.status());
}

/*
 * Subscribe to the given keys in the backend partition. If the set
 * of keys is empty, returns immediately with timestamp 0.
 */
void
ShardClient::Subscribe(const set<string> &keys,
                       const TransportAddress &myAddress,
		       callback_t callback) {
    if (keys.size() == 0) {
        Debug("[shared %i] SUBSCRIBE set is empty", shard);
        Promise *promise = new Promise();
	promise->Reply(REPLY_OK, 0);
        callback(promise);
	return;
    }

    Debug("[shard %i] Sending SUBSCRIBE", shard);

    // create request
    string request_str;
    Request request;
    request.set_op(Request::SUBSCRIBE);
    request.set_txnid(0);
    string address(myAddress.getHostname() + ":" + myAddress.getPort());
    request.mutable_subscribe()->set_address(address);
    
    for (auto &i : keys) {
        request.mutable_subscribe()->add_keys(i);
    }
    request.SerializeToString(&request_str);

    transport->Timer(0, [=]() {
            client->Invoke(request_str,
                           bind(&ShardClient::SubscribeCallback,
                                this,
				callback,
                                placeholders::_1,
                                placeholders::_2));
        });
}

void
ShardClient::SubscribeCallback(callback_t callback,
			       const string &request_str,
			       const string &reply_str) {
    Reply reply;
    reply.ParseFromString(reply_str);
    ASSERT(reply.status() == REPLY_OK);

    map<string, Version> values;
    for (int i = 0; i < reply.replies_size(); i++) {
        ReadReply rep = reply.replies(i);
        values[rep.key()] = Version(rep);
    }

    Promise *w = new Promise();
    w->Reply(reply.status(), values);
    Debug("[shard %i] Received SUBSCRIBE callback [%d]", shard, reply.status());
    callback(w);
}

} // namespace strongstore
