// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
/***********************************************************************
 *
 * store/pubstore.cc:
 *   Key-value store with support for notifications
 *
 * Copyright 2013-2015 Irene Zhang <iyzhang@cs.washington.edu>
 *                     Naveen Kr. Sharma <naveenks@cs.washington.edu>
 *                     Dan R. K. Ports  <drkp@cs.washington.edu>
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

#include "pubstore.h"

using namespace std;

namespace strongstore {

PubStore::PubStore() : { }
PubStore::~PubStore() { }


void
PubStore::Subscribe(const TCPTransportAddress &remote,
		    const Timestamp timestamp,
		    const set<string> &keys)
{
    store.Subscribe(remote, timestamp, keys);
}

void
PubStore::Unsubscribe(const TCPTransportAddress &remote,
		      const set<string> &keys)
{
    store.Unsubscribe(remote, keys);
}

void
PubStore::Publish(const uint64_t tid,
		  const Timestamp timestamp
		  map<TCPTransportAddress, set<string>> &notifications)
{
    Transaction t;
    // Get transaction
    if (committed.find(tid) != committed.end()) {
        t = committed[tid];
    } else {
        Panic("No transaction with tid %lu is committed", tid);
    }

    // get set of keys that changed
    set<string> keys = t.GetWriteSet();
    for (auto &inc : t.GetIncrementSet()) {
        keys.insert(inc.first);
    }

    // get notifications that need to be sent
    store.GetNotifications(tid, keys, notifications);
}

void
PubStore::AckPending(const TCPTransportAddress &remote,
		     const Timestamp timestamp,
		     const set<string> &keys) {
    store.subscribe(remote, timestamp, keys);
}
    
} // namespace strongstore
