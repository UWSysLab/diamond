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

#ifndef _VERSIONED_KV_STORE_H_
#define _VERSIONED_KV_STORE_H_

#include "lib/assert.h"
#include "lib/message.h"
#include "store/common/timestamp.h"
#include "store/common/version.h"

#include <set>
#include <map>
#include <unordered_map>
#include <fstream>
#include <iostream>

class VersionedKVStore
{

public:
    VersionedKVStore();
    virtual ~VersionedKVStore();

    bool Get(const std::string &key, Version &value);
    bool Get(const std::string &key, const Timestamp &t, Version &value);
    bool GetRange(const std::string &key, const Timestamp &t, Interval &range);
    bool GetLastRead(const std::string &key, Timestamp &readTime);
    bool GetLastRead(const std::string &key, const Timestamp &t, Timestamp &readTime);
    void Put(const std::string &key, const std::string &value, const Timestamp &t);
    void Put(const std::string &key, const Version &v);
    void CommitGet(const std::string &key, const Timestamp &readTime, const Timestamp &commit);
    void Remove(const std::string &key);
    
protected:

    /* Global store which keeps key -> (timestamp, value) list. */
    std::unordered_map< std::string, std::set<Version> > store;
    bool inStore(const std::string &key);
    bool getValue(const std::string &key, const Timestamp &t, std::set<Version>::iterator &it);
};

#endif  /* _VERSIONED_KV_STORE_H_ */
