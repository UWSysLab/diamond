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

void
VersionedKVStore::getValue(const string &key, const Timestamp &t, set<Version>::iterator &it)
{
    Version v(t);
    it = store[key].upper_bound(v);

    // if there is no valid version at this timestamp
    if (it == store[key].begin()) {
        it = store[key].end();
    } else {
        it--;
    }
}


/* Returns the most recent value and timestamp for given key.
 * Error if key does not exist. */
bool
VersionedKVStore::get(const string &key, Version &value)
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
VersionedKVStore::get(const string &key, const Timestamp &t, Version &value)
{
    if (inStore(key)) {
        set<Version>::iterator it;
        getValue(key, t, it);
        if (it != store[key].end()) {
            value = *it;
            return true;
        }
    }
    return false;
}

bool
VersionedKVStore::getRange(const string &key, const Timestamp &t,
			   pair<Timestamp, Timestamp> &range)
{
    if (inStore(key)) {
        set<Version>::iterator it;
        getValue(key, t, it);

        if (it != store[key].end()) {
            range.first = it->GetTimestamp();
            it++;
            if (it != store[key].end()) {
                range.second = it->GetTimestamp();
            }
            return true;
        }
    }
    return false;
}

void
VersionedKVStore::put(const string &key, const string &value, const Timestamp &t)
{
    // Key does not exist. Create a list and an entry.
    put(key, Version(t, value));
}

void
VersionedKVStore::put(const string &key, const Version &v)
{
    // Key does not exist. Create a list and an entry.
    store[key].insert(v);
}

/*
 * Commit a read by updating the timestamp of the latest read txn for
 * the version of the key that the txn read.
 */
void
VersionedKVStore::commitGet(const string &key, const Timestamp &readTime, const Timestamp &commit)
{
    // Hmm ... could read a key we don't have if we are behind ... do we commit this or wait for the log update?
    if (inStore(key)) {
        set<Version>::iterator it;
        getValue(key, readTime, it);
        
        if (it != store[key].end()) {
            // figure out if anyone has read this version before
            if (lastReads.find(key) != lastReads.end() &&
                lastReads[key].find(it->GetTimestamp()) != lastReads[key].end()) {
                if (lastReads[key][it->GetTimestamp()] < commit) {
                    lastReads[key][it->GetTimestamp()] = commit;
                }
            }
        }
    } // otherwise, ignore the read
}

bool
VersionedKVStore::getLastRead(const string &key, Timestamp &lastRead)
{
    if (inStore(key)) {
        Version v = *(store[key].rbegin());
        if (lastReads.find(key) != lastReads.end() &&
            lastReads[key].find(v.GetTimestamp()) != lastReads[key].end()) {
            lastRead = lastReads[key][v.GetTimestamp()];
            return true;
        }
    }
    return false;
}    

/*
 * Get the latest read for the write valid at timestamp t
 */
bool
VersionedKVStore::getLastRead(const string &key, const Timestamp &t, Timestamp &lastRead)
{
    if (inStore(key)) {
        set<Version>::iterator it;
        getValue(key, t, it);
        ASSERT(it != store[key].end());

        // figure out if anyone has read this version before
        if (lastReads.find(key) != lastReads.end() &&
            lastReads[key].find(it->GetTimestamp()) != lastReads[key].end()) {
            lastRead = lastReads[key][it->GetTimestamp()];
            return true;
        }
    }
    return false;	
}
