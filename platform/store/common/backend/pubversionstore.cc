// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * store/common/backend/versionstore.cc:
 *   Timestamped version store
 *
 * Copyright 2015 Irene Zhang <iyzhang@cs.washington.edu>
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

#include "pubversionstore.h"

using namespace std;

PubVersionStore::PubVersionStore() { }
    
PubVersionStore::~PubVersionStore() { }

void
PubVersionStore::Subscribe(const TCPTransportAddress &remote,
			    const Timestamp timestamp,
			    const set<string> &keys)
{
    for (auto &key : keys) {
        if (subscribers[key].count(remote) == 0 ||
            subscribers[key][remote] < timestamp) {	    
            subscribers[key][remote] = timestamp;
        }
    }
}

void
PubVersionStore::Unsubscribe(const TCPTransportAddress &remote,
			      const set<string> &keys)
{
    for (auto &key : keys) {
        subscribers[key].erase(remote);
    }
}

void
PubVersionStore::Publish(const Timestamp &timestamp,
                          const set<string> &keys,
                          map<TCPTransportAddress, set<string>> &notifications) {
    for (auto &key : keys) {
        for (auto &address : subscribers[key]) {
            if (timestamp > address.second) {
                Debug("timestamp %lu and subscription %lu", timestamp, address.second);
                notifications[address.first].insert(key);
            }
        }
    }
}
