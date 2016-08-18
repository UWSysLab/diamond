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

#include "versionstore.h"

using namespace std;

VersionedKVStore::VersionedKVStore() { }
    
VersionedKVStore::~VersionedKVStore() { }

bool
VersionedKVStore::inStore(const string &key)
{
    return store.find(key) != store.end() && store[key].size() > 0;
}

bool
VersionedKVStore::getValue(const string &key, const Timestamp &t, set<Version>::iterator &it)
{
    Version v(t);
    it = store[key].upper_bound(v);

    // if there is no valid version at this timestamp
    if (it == store[key].begin()) {
        return false;
    }
    it--;
    return true;
}


/* Returns the most recent value and timestamp for given key.
 * Error if key does not exist. */
bool
VersionedKVStore::Get(const string &key, Version &value)
{
    // check for existence of key in store
    if (inStore(key)) {
        value = *(store[key].rbegin());
        return true;
    }
    return false;
}
    
/* Returns the value valid at given timestamp.
 * Error if key did not exist at the timestamp. */
bool
VersionedKVStore::Get(const string &key, const Timestamp &t, Version &value)
{
    if (t == MAX_TIMESTAMP) {
        return Get(key, value);
    }
    if (inStore(key)) {
        set<Version>::iterator it;
        if (getValue(key, t, it) && it->GetInterval().End() >= t) {
            value = *it;
            return true;
        }
    }
    return false;
}

bool
VersionedKVStore::GetRange(const string &key, const Timestamp &t,
			   Interval &range)
{
    if (inStore(key)) {
        set<Version>::iterator it;
	if (getValue(key, t, it)) {
	    range = it->GetInterval();
            return true;
        }
    }
    return false;
}

void
VersionedKVStore::Put(const string &key, const string &value, const Timestamp &t)
{
    // Key does not exist. Create a list and an entry.
    Put(key, Version(t, value));
}

void
VersionedKVStore::Put(const string &key, const Version &v)
{
    // Key does not exist. Create a list and an entry.
    if (store.find(key) != store.end()) {
        if (v.GetTimestamp() == 0) {
            store.erase(key);
        }
        else {
            set<Version>::iterator it = --(store[key].end());
            if (v.GetInterval().Start() < it->GetInterval().End()) {
                Version v1 = *it;
                store[key].erase(it);
                v1.SetEnd(v.GetInterval().Start());
                store[key].insert(v1);
            }
        }
    }
    store[key].insert(v);
}

/*
 * Commit a read by updating the timestamp of the latest read txn for
 * the version of the key that the txn read.
 */
void
VersionedKVStore::CommitGet(const string &key, const Timestamp &readTime, const Timestamp &commit)
{
    set<Version>::iterator it;
    if (getValue(key, readTime, it)) {
        if (commit < it->GetInterval().End()) {
            Version v1 = *it;
            store[key].erase(it);
            v1.SetEnd(commit);
            store[key].insert(v1);
        }
    }
}

void
VersionedKVStore::Remove(const string &key) {
    auto it = store.find(key);
    if (it != store.end()) {
        store.erase(it);
    }
}

bool
VersionedKVStore::GetLastRead(const string &key, Timestamp &lastRead)
{
    if (inStore(key)) {
        Version v = *(store[key].rbegin());
        if (v.GetInterval().End() != MAX_TIMESTAMP) { 
            lastRead = v.GetInterval().End();
            return true;
        }
    }
    return false;
}    

/*
 * Get the latest read for the write valid at timestamp t
 */
bool
VersionedKVStore::GetLastRead(const string &key,
			      const Timestamp &t,
			      Timestamp &lastRead)
{
    if (inStore(key)) {
        set<Version>::iterator it;
        if (getValue(key, t, it)) {
	    Version v = *it;
	    // figure out if anyone has read this version before
	    if (v.GetInterval().End() != MAX_TIMESTAMP) { 
		lastRead = v.GetInterval().End();
		return true;
	    }
	}
    }
    return false;	
}

void
VersionedKVStore::Subscribe(const TCPTransportAddress &remote,
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
VersionedKVStore::Unsubscribe(const TCPTransportAddress &remote,
			      const set<string> &keys)
{
    for (auto &key : keys) {
        subscribers[key].erase(remote);
    }
}

void
VersionedKVStore::Publish(const Timestamp &timestamp,
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
