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
OCCStore::Get(const uint64_t tid, const string &key, Version &value)
{
    Debug("[%lu] GET %s", tid, key.c_str());

    // Get latest from store
    if (store.get(key, value)) {
        Debug("[%lu] GET %s %lu", tid, key.c_str(), value.GetTimestamp());
        return REPLY_OK;
    } else {
        return REPLY_NOT_FOUND;
    }
}

int
OCCStore::Get(const uint64_t tid, const string &key, const Timestamp &timestamp, Version &value)
{
    Debug("[%lu] GET %s", tid, key.c_str());
    
    // Get version at timestamp from store
    if (store.get(key, timestamp, value)) {
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
    for (auto &read : txn.getReadSet()) {
        Version cur;
        bool ret = store.get(read.first, cur);

	    // ASSERT(ret);
        if (!ret)
            continue;

        // If this key has been written since we read it, abort.
        if (cur.GetTimestamp() > read.second) {
            Debug("[%lu] ABORT rw conflict key:%s %lu %lu",
                  tid, read.first.c_str(), cur.GetTimestamp(),
                  read.second);
            
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

    // Check for conflicts with the write set.
    for (auto &write : txn.getWriteSet()) {
        // If there is a pending read or write for this key, abort.
        if (pRW.find(write.first) != pRW.end()) {
            Debug("[%lu] ABORT ww conflict w/ prepared key:%s", tid,
                    write.first.c_str());
            Abort(tid);
            return REPLY_FAIL;
        }
    }

    // Otherwise, prepare this transaction for commit
    prepared[tid] = txn;
    Debug("[%lu] PREPARED TO COMMIT", tid);
    return REPLY_OK;
}

void
OCCStore::Commit(const uint64_t tid, uint64_t timestamp)
{
    Debug("[%lu] COMMIT", tid);
    ASSERT(prepared.find(tid) != prepared.end());

    Transaction txn = prepared[tid];

    for (auto &write : txn.getWriteSet()) {
        store.put(write.first, // key
                    write.second, // value
                    Timestamp(timestamp)); // timestamp
    }

    prepared.erase(tid);
}

void
OCCStore::Abort(const uint64_t tid, const Transaction &txn)
{
    Debug("[%lu] ABORT", tid);
    prepared.erase(tid);
}

void
OCCStore::Load(const string &key, const string &value, const Timestamp &timestamp)
{
    store.put(key, value, timestamp);
}

set<string>
OCCStore::getPreparedWrites()
{
    // gather up the set of all writes that we are currently prepared for
    set<string> writes;
    for (auto &t : prepared) {
        for (auto &write : t.second.getWriteSet()) {
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
        for (auto &write : t.second.getWriteSet()) {
            readwrites.insert(write.first);
        }
        for (auto &read : t.second.getReadSet()) {
            readwrites.insert(read.first);
        }
    }
    return readwrites;
}

} // namespace strongstore
