// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
/***********************************************************************
 *
 * store/txnstore/lib/txnstore.h:
 *   Interface for a single node transactional store serving as a
 *   server-side backend
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

#ifndef _TXN_STORE_H_
#define _TXN_STORE_H_

#include "lib/assert.h"
#include "lib/message.h"
#include "store/common/transaction.h"
#include "store/common/timestamp.h"
#include "store/common/version.h"
#include "store/common/notification.h"

namespace strongstore {
    
class TxnStore
{
public:

    TxnStore() { };
    virtual ~TxnStore() { };

    virtual int Get(const uint64_t tid, const std::string &key, Version &value,
                    const Timestamp &timestamp = MAX_TIMESTAMP) = 0;

    // check whether we can commit this transaction (and lock the read/write set)
    virtual int Prepare(const uint64_t tid, const Transaction &txn) = 0;

    // commit the transaction
    virtual void Commit(const uint64_t tid, const Timestamp &timestamp, const Transaction &txn = Transaction()) = 0;

    // abort a running transaction
    virtual void Abort(const uint64_t tid) = 0;

    virtual void Load(const std::string &key, const std::string &value, const Timestamp &timestamp) = 0;

    virtual void Subscribe(const std::set<std::string> &keys, const std::string &address, std::map<std::string, Version> &values) = 0;
    virtual void AddFrontendNotifications(const Timestamp &timestamp, const uint64_t tid) = 0;
    virtual void AckFrontendNotification(const uint64_t tid, const std::string &address) = 0;
    virtual void GetUnackedFrontendNotifications(const uint64_t tid, std::vector<FrontendNotification> &notifications) = 0;
    
};
}
#endif /* _TXN_STORE_H_ */
