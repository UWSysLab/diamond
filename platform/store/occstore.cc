// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
/***********************************************************************
 *
 * store/strongstore/occstore.cc:
 *   Key-value store with support for strong consistency using OCC
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

#include "occstore.h"

using namespace std;

namespace strongstore {

OCCStore::OCCStore() : store() { }
OCCStore::~OCCStore() { }

int
OCCStore::Get(const uint64_t tid, const string &key, Version &value, const Timestamp &timestamp)
{
    Debug("[%lu] GET %s at %lu", tid, key.c_str(), timestamp);
    
    if (store.Get(key, timestamp, value)) {
        return REPLY_OK;
    } else {
        return REPLY_NOT_FOUND;
    }
}

int
OCCStore::Prepare(const uint64_t tid, const Transaction &txn)
{    
    Debug("[%lu] START PREPARE", tid);

    if (prepared.find(tid) != prepared.end()) {
        Debug("[%lu] Already prepared!", tid);
        return REPLY_OK;
    }

    // Do OCC checks.
    unordered_set<string> pWrites, pReads, pIncrements;
    getPreparedOps(pWrites, pReads, pIncrements);
    
    // Check for conflicts with the read set.
    if (txn.IsolationMode() == LINEARIZABLE) {
        for (auto &read : txn.GetReadSet()) {
            Version cur;
            const string &key = read.first;
            const Interval &valid = read.second;
            store.Get(key, cur);

            // If this key has been written since we read it, abort.
            if (cur.GetTimestamp() > valid.Start()) {
                Debug("[%lu] ABORT LINEARIZABLE rw conflict key:%s %lu %lu",
                      tid, key.c_str(), cur.GetTimestamp(),
                      valid.Start());
            
                Abort(tid);
                return REPLY_FAIL;
            }

            // If there is a pending write for this key, abort.
            if (pWrites.find(key) != pWrites.end()) {
                Debug("[%lu] ABORT LINEARIZABLE rw conflict w/ prepared key:%s",
                      tid, read.first.c_str());
                Abort(tid);
                return REPLY_FAIL;
            }

            if (pIncrements.find(key) != pIncrements.end()) {
                Debug("[%lu] ABORT LINEARIZABLE ri conflict w/ prepared key:%s",
                      tid, read.first.c_str());
                Abort(tid);
                return REPLY_FAIL;
            }
        }
    }

    if (txn.IsolationMode() == LINEARIZABLE || txn.IsolationMode() == SNAPSHOT_ISOLATION) {
        // Check for conflicts with the write set.
        for (auto &write : txn.GetWriteSet()) {
            const string &key = write.first;
            
            // if there is a pending write, always abort
            if (pWrites.find(key) != pWrites.end()) {
                Debug("[%lu] ABORT ww conflict w/ prepared key:%s", tid,
                      key.c_str());
                Abort(tid);
                return REPLY_FAIL;
            }

            // if there is a pending increment, always abort
            if (pIncrements.find(key) != pIncrements.end()) {
                Debug("[%lu] ABORT wi conflict w/ prepared key:%s", tid,
                      key.c_str());
                Abort(tid);
                return REPLY_FAIL;
            }

            if (txn.IsolationMode() == LINEARIZABLE) {
                // If there is a pending read for this key, abort to stay linearizable.
                if (pReads.find(key) != pReads.end()) {
                    Debug("[%lu] ABORT LINEARIZABLE rw conflict w/ prepared key:%s", tid,
                          key.c_str());
                    Abort(tid);
                    return REPLY_FAIL;
                }
            } else if (txn.IsolationMode() == SNAPSHOT_ISOLATION) {
                // If SI, check that the snapshot hasn't been written
                Version cur;
                if (store.Get(key, cur) && cur.GetTimestamp() > txn.GetTimestamp() &&
                    txn.GetReadSet().find(write.first) != txn.GetReadSet().end()) {
                    Debug("[%lu] ABORT SNAPSHOT ISOLATION rw conflict w/ prepared key:%s", tid,
                          key.c_str());
                    Abort(tid);
                    return REPLY_FAIL;
                }
            }
        }

        // Check for conflicts with the increment set
        for (auto &inc : txn.GetIncrementSet()) {
            const string &key = inc.first;

            // don't need to check for pending increments, they commute
            
            if (txn.IsolationMode() == LINEARIZABLE) {
                // Check for pending reads
                if (pReads.find(key) != pReads.end()) {
                    Debug("[%lu] ABORT LINEARIZABLE ri conflict w/ prepared key:%s", tid,
                          key.c_str());
                    Abort(tid);
                    return REPLY_FAIL;
                }
            } else if (txn.IsolationMode() == SNAPSHOT_ISOLATION) {
                // If SI, check that the snapshot hasn't been written
                Version cur;
                if (store.Get(key, cur) && cur.GetTimestamp() > txn.GetTimestamp() &&
                    txn.GetReadSet().find(inc.first) != txn.GetReadSet().end()) {
                    Debug("[%lu] ABORT SNAPSHOT ISOLATION rw conflict w/ prepared key:%s", tid,
                          key.c_str());
                    Abort(tid);
                    return REPLY_FAIL;
                }
            }
        }
    }

    // Otherwise, prepare this transaction for commit
    prepared[tid] = txn;
    Debug("[%lu] PREPARED TO COMMIT", tid);
    return REPLY_OK;
}

void
OCCStore::Commit(const uint64_t tid, const Timestamp &timestamp, const Transaction &txn)
{
    if (committed.find(tid) == committed.end()) {
        Transaction t;
        Debug("[%lu] COMMIT", tid);
        if (prepared.find(tid) != prepared.end()) {
            t = prepared[tid];
        } else {
            t = txn;
        }

        for (auto &write : t.GetWriteSet()) {
            store.Put(write.first, // key
                      write.second, // value
                      timestamp); // timestamp
        }

        for (auto &inc : t.GetIncrementSet()) {
            store.Increment(inc.first,
                            inc.second,
                            timestamp);
        }
        prepared.erase(tid);
        committed[tid] = t;
    }
}

void
OCCStore::Abort(const uint64_t tid)
{
    Debug("[%lu] ABORT", tid);
    prepared.erase(tid);
}

void
OCCStore::Load(const string &key, const string &value, const Timestamp &timestamp)
{
    store.Put(key, value, timestamp);
}

void
OCCStore::getPreparedOps(unordered_set<string> &reads, unordered_set<string> &writes, unordered_set<string> &increments)
{
    for (auto &t : prepared) {
        for (auto &write : t.second.GetWriteSet()) {
            writes.insert(write.first);
        }
        for (auto &read : t.second.GetReadSet()) {
            reads.insert(read.first);
        }
        for (auto &incr : t.second.GetIncrementSet()) {
            increments.insert(incr.first);
        }
    }
}

void
OCCStore::Subscribe(const set<string> &keys, const string &address, map<string, Version> &values) {
    return store.Subscribe(keys, address, values);
}

void
OCCStore::GetFrontendNotifications(const Timestamp &timestamp, const uint64_t tid, vector<FrontendNotification> &notifications) {
    Transaction t;
    if (committed.find(tid) != committed.end()) {
        t = committed[tid];
    } else {
        Panic("No transaction with tid %lu is committed", tid);
    }

    set<string> keys;
    for (auto &write : t.GetWriteSet()) {
        keys.insert(write.first);
    }
    for (auto &inc : t.GetIncrementSet()) {
        keys.insert(inc.first);
    }
    store.GetFrontendNotifications(timestamp, keys, notifications);
    fillCacheEntries(t, notifications);
}

void
OCCStore::fillCacheEntries(const Transaction &txn, std::vector<FrontendNotification> &notifications) {
    for (auto &n : notifications) {
        vector<pair<string, Timestamp> > keys;
        for (auto it = n.values.begin(); it != n.values.end(); it++) {
            keys.push_back(pair<string, Timestamp>(it->first, it->second.GetTimestamp()));
        }
        for (auto it = keys.begin(); it != keys.end(); it++) {
            string key = it->first;
            Timestamp timestamp = it->second;
            Version value;
            int ret = Get(0, key, value, timestamp); //TODO: is txnid 0 fine, since Get ignores it?
            if (ret != REPLY_OK) {
                Panic("Cached value for %s at timestamp %lu not found", key.c_str(), value.GetTimestamp());
            }
            n.values[key] = value;
        }
    }
}
    
} // namespace strongstore
