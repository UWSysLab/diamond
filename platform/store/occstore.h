// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
/***********************************************************************
 *
 * store/strongstore/occstore.h:
 *   Key-value store with support for transactions with strong consistency using OCC.
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

#ifndef _STRONG_OCC_STORE_H_
#define _STRONG_OCC_STORE_H_

#include "lib/assert.h"
#include "lib/message.h"
#include "includes/error.h"
#include "store/common/backend/commstore.h"
#include "store/common/backend/txnstore.h"
#include "store/common/transaction.h"
#include "store/common/version.h"

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <list>

namespace strongstore {

class OCCStore : public TxnStore
{
public:
    OCCStore();
    ~OCCStore();

    // Overriding from TxnStore.
    int Get(const uint64_t tid, const std::string &key, Version &value,
            const Timestamp &timestamp = MAX_TIMESTAMP);

    // check whether we can commit this transaction (and lock the read/write set)
    int Prepare(const uint64_t tid, const Transaction &txn);

    // commit the transaction
    void Commit(const uint64_t tid, const Timestamp &timestamp, const Transaction &txn = Transaction());

    // abort a running transaction
    void Abort(const uint64_t tid);

    void Load(const std::string &key, const std::string &value, const Timestamp &timestamp);

    Timestamp Subscribe(const std::set<std::string> &keys, const std::string &address);
    void GetFrontendNotifications(const Timestamp &timestamp, const uint64_t tid, std::vector<FrontendNotification> &notifications);
    void GetFrontendNotifications(const Timestamp &timestamp, const Transaction &txn, std::vector<FrontendNotification> &notifications);

private:
    // Data store.
    CommutativeStore store;

    std::unordered_map<uint64_t, Transaction> prepared;
    std::unordered_map<uint64_t, Transaction> committed;

    void getPreparedOps(std::unordered_set<std::string> &reads, std::unordered_set<std::string> &writes, std::unordered_set<std::string> &increments);

    void fillCacheEntries(const Transaction &txn, std::vector<FrontendNotification> &notifications);
};

} // namespace strongstore

#endif /* _STRONG_OCC_STORE_H_ */
