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

namespace diamond {

using namespace std;
    
DiamondClient::DiamondClient(string configPath)
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
    frontend::Client *frontendclient = new frontend::Client(configPath + ".config",
                                                            &transport,
                                                            client_id);
    bclient = new BufferClient((TxnClient *)new CacheClient(frontendclient));

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
DiamondClient::OngoingTransaction(uint64_t txnid)
{
    thread::id tid = this_thread::get_id();
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
void
DiamondClient::Begin()
{
    thread::id tid = this_thread::get_id();
    Debug("BEGIN Transaction");
    uint64_t txnid = 0;
    if (!OngoingTransaction(txnid)) {
        // there's no ongoing transaction, start one
        txnid_lock.lock();
        uint64_t txnid = ++txnid_counter;
        txnid_lock.unlock();
        ongoing[tid] = txnid;
        bclient->Begin(txnid);
    }
}

/* Returns the value corresponding to the supplied key. */
int
DiamondClient::Get(const string &key, string &value)
{
    uint64_t txnid = 0;
    // start a transaction if there isn't one
    if (!OngoingTransaction(txnid)) {
        Begin();
        OngoingTransaction(txnid);
    }
	
    // Send the GET operation to appropriate shard.
    Promise promise(GET_TIMEOUT);

    bclient->Get(txnid, key, &promise);
    value = promise.GetValues()[key].GetValue();

    return promise.GetReply();
}

int
DiamondClient::MultiGet(const vector<string> &keys, map<string, string> &values)
{
    uint64_t txnid = 0;
    // start a transaction if there isn't one
    if (!OngoingTransaction(txnid)) {
        Begin();
        OngoingTransaction(txnid);
    }
	
    // Send the GET operation to appropriate shard.
    Promise promise(GET_TIMEOUT);

    bclient->MultiGet(txnid, keys, &promise);
    for (auto i : promise.GetValues()) {
        values[i.first] = i.second.GetValue();
    }

    return promise.GetReply();
}

/* Sets the value corresponding to the supplied key. */
int
DiamondClient::Put(const string &key, const string &value)
{
    uint64_t txnid = 0;
    // start a transaction if there isn't one
    if (!OngoingTransaction(txnid)) {
        Begin();
        OngoingTransaction(txnid);
    }
	
    Promise promise(PUT_TIMEOUT);

    // Buffering, so no need to wait.
    bclient->Put(txnid, key, value, &promise);
    return promise.GetReply();
}

/* Attempts to commit the ongoing transaction. */
bool
DiamondClient::Commit()
{
    uint64_t txnid = 0;
    // if there isn't a transaction just ignore
    if (!OngoingTransaction(txnid)) {
        Warning("No ongoing transaction.");
        return true;
    }
    
    Promise promise(COMMIT_TIMEOUT);

    bclient->Commit(txnid, 0, &promise);
    return promise.GetReply();
}

/* Aborts the ongoing transaction. */
void
DiamondClient::Abort()
{
    uint64_t txnid = 0;
    // if there isn't a transaction just ignore
    if (!OngoingTransaction(txnid)) {
        Warning("No ongoing transaction.");
        return;
    }
    
    Debug("ABORT Transaction");
    bclient->Abort(txnid);
}

} // namespace diamond
