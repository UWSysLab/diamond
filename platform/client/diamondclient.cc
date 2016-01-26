// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * client/diamondclient.cc:
 *   Client to Diamond frontend servers
 *
 * Copyright 2016 Irene Zhang <iyzhang@cs.washington.edu>
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

#include "diamondclient.h"

using namespace std;

namespace Diamond {

DiamondClient::Client(string configPath)
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
    txnid_counter = (client_id/10000)*10000;

    Debug("Initializing Diamond Store client with id [%lu]", client_id);

    /* Start a client for the front-end. */
    string shardConfigPath = configPath + to_string(i) + ".config";
    DiamondFrontendClient *frontendclient = new DiamondFrontendClient(configPath,
								      &transport,
								      client_id);
    cclient = new CacheClient(frontendclient);

    /* Run the transport in a new thread. */
    clientTransport = new thread(&DiamondClient::run_client, this);

    Debug("Diamond Store client [%lu] created!", client_id);
}

DiamondClient::~DiamondClient()
{
    transport.Stop();
    clientTransport->join();
}

/* Runs the transport event loop. */
void
DiamondClient::run_client()
{
    transport.Run();
}

/* check whether there is an ongoing transaction and return it if found */
bool
DiamondClient::OngoingTransaction(uint64_t &txnid = 0)
{
    thread::id tid = thread::get_id();
    if (ongoing.find(tid) == ongoing.end()) {
	return false;
    } else {
	txnid = ongoing[tid];
	return true;
    }
}
    
/* Begins a transaction. All subsequent operations before a commit() or
 * abort() are part of this transaction.
 */
uint64_t
DiamondClient::Begin()
{
    thread::id tid = thread::get_id();
    Debug("BEGIN Transaction");
    if (!OngoingTransaction()) {
 	// there's no ongoing transaction, start one
	txnid_lock.lock();
	uint64_t txnid = ++txnid_counter;
	txnid_lock.unlock();
	ongoing[tid] = txnid;
	cclient->Begin(txnid);
    }
    return ongoing[tid];
}

/* Returns the value corresponding to the supplied key. */
int
DiamondClient::Get(const string &key, string &value)
{

    uint64_t txnid;
    // start a transaction if there isn't one
    if (!OngoingTransaction(txnid)) {
	txnid = Begin();
    }
	
    // Send the GET operation to appropriate shard.
    Promise promise(GET_TIMEOUT);

    cclient->Get(txnid, key, &promise);
    value = promise.GetValue();

    return promise.GetReply();
}

/* Sets the value corresponding to the supplied key. */
int
DiamondClient::Put(const string &key, const string &value)
{
    uint64_t txnid;
    // start a transaction if there isn't one
    if (!OngoingTransaction(txnid)) {
	txnid = Begin();
    }
	
    Promise promise(PUT_TIMEOUT);

    // Buffering, so no need to wait.
    cclient->Put(txnid, key, value, &promise);
    return promise.GetReply();
}

/* Attempts to commit the ongoing transaction. */
bool
DiamondClient::Commit()
{
    uint64_t txnid;
    // if there isn't a transaction just ignore
    if (!OngoingTransaction(txnid)) {
	Warning("No ongoing transaction.");
	return true;
    }
    
    Promise promise(COMMIT_TIMEOUT);

    cclient->Commit(txnid, &promise);
    return promise.GetReply();
}

/* Aborts the ongoing transaction. */
void
DiamondClient::Abort()
{
    uint64_t txnid;
    // if there isn't a transaction just ignore
    if (!OngoingTransaction(txnid)) {
	Warning("No ongoing transaction.");
	return true;
    }
    
    Debug("ABORT Transaction");
    cclient[p]->Abort(txnid);
}

} // namespace diamond
