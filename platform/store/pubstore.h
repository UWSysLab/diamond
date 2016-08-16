// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
/***********************************************************************
 *
 * store/strongstore/notifystore.h:
 *   OCC Key-value store with support for notifications
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

#ifndef _STRONG_PUB_STORE_H_
#define _STRONG_PUB_STORE_H_

#include "lib/assert.h"
#include "lib/message.h"
#include "includes/error.h"
#include "store/occstore.h"

#include <vector>
#include <map>
#include <unordered_set>
#include <list>
#include <mutex>

namespace strongstore {

class PubStore : public OCCStore
{
public:
    PubStore();
    ~PubStore();

    void Subscribe(const TCPTransportAddress &remote,
		   const Timestamp timestamp,
		   const std::set<std::string> &keys);
    void Unsubscribe(const TCPTransportAddress &remote,
		     const std::set<std::string> &keys);
    void Publish(const uint64_t txn_id,
		 const Timestamp timestamp,
		 map<TCPTransportAddress, std::set<std::string>> notifications);
    void AckPending(const TCPTransportAddress &remote,
		    const Timestamp timestamp,
		    const std::set<string> &keys);

};

} // namespace strongstore

#endif /* _STRONG_PUB_STORE_H_ */
