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
    Debug("[%lu] GET %s", tid, key.c_str());
    
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
    set<string> pWrites = getPreparedWrites();
    set<string> pRW = getPreparedReadWrites();

    // Check for conflicts with the read set.
    if (txn.IsolationMode() == LINEARIZABLE) {
        for (auto &read : txn.GetReadSet()) {
            Version cur;
            const string &key = read.first;
            const Interval &valid = read.second;
            bool ret = store.Get(key, cur);

            ASSERT(ret);
            
            // If this key has been written since we read it, abort.
            if (cur.GetTimestamp() > valid.Start()) {
                Debug("[%lu] ABORT LINEARIZABLE rw conflict key:%s %lu %lu",
                      tid, key.c_str(), cur.GetTimestamp(),
                      valid.Start());
            
                Abort(tid);
                return REPLY_FAIL;
            }

            // If there is a pending write for this key, abort.
            if (pWrites.find(read.first) != pWrites.end()) {
                Debug("[%lu] ABORT rw conflict w/ prepared key:%s",
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
            if (txn.IsolationMode() == LINEARIZABLE) {
                // If there is a pending read or write for this key, abort.
                if (pRW.find(key) != pRW.end()) {
                    Debug("[%lu] ABORT LINEARIZABLE ww conflict w/ prepared key:%s", tid,
                          key.c_str());
                    Abort(tid);
                    return REPLY_FAIL;
                }
            } else if (txn.IsolationMode() == SNAPSHOT_ISOLATION) {
                // if there is a pending write, abort
                if (pWrites.find(key) != pWrites.end()) {
                    Debug("[%lu] ABORT SNAPSHOT ISOLATION ww conflict w/ prepared key:%s", tid,
                          key.c_str());
                    Abort(tid);
                    return REPLY_FAIL;
                }
                // if there is write between the read snapshot and now
                Version cur;
                bool ret = store.Get(key, cur);
                ASSERT(ret);
                if (cur.GetTimestamp() > txn.GetTimestamp()) {
                    Debug("[%lu] ABORT SNAPSHOT ISOLATION ww conflict w/ prepared key:%s", tid,
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

    prepared.erase(tid);
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

set<string>
OCCStore::getPreparedWrites()
{
    // gather up the set of all writes that we are currently prepared for
    set<string> writes;
    for (auto &t : prepared) {
        for (auto &write : t.second.GetWriteSet()) {
            writes.insert(write.first);
        }
    }
    return writes;
}

set<string>
OCCStore::getPreparedReadWrites()
{
    // gather up the set of all writes that we are currently prepared for
    set<string> readwrites;
    for (auto &t : prepared) {
        for (auto &write : t.second.GetWriteSet()) {
            readwrites.insert(write.first);
        }
        for (auto &read : t.second.GetReadSet()) {
            readwrites.insert(read.first);
        }
    }
    return readwrites;
}

} // namespace strongstore
