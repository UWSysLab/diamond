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

#ifndef _PUB_VERSIONED_KV_STORE_H_
#define _PUB_VERSIONED_KV_STORE_H_

#include "lib/tcptransport.h"
#include "store/common/backend/versionstore.h"
#include "store/common/notification.h"

class PubVersionStore : public VersionedKVStore
{

public:
    PubVersionStore();
    virtual ~PubVersionStore();

    void Subscribe(const TCPTransportAddress &remote,
                   const Timestamp timestamp,
                   const std::set<std::string> &keys);
    void Unsubscribe(const TCPTransportAddress &remote,
                     const std::set<std::string> &keys);
    void Publish(const Timestamp &timestamp,
                 const std::set<std::string> &keys,
                 std::map<TCPTransportAddress, std::set<std::string>> &notifications);
    
    
protected:
    std::map<std::string, std::map<TCPTransportAddress, Timestamp>> subscribers; // indexed by key
};

#endif  /* _VERSIONED_KV_STORE_H_ */