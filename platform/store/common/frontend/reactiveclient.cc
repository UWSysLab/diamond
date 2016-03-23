// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * store/common/frontend/reactiveclient.cc:
 *   Single shard caching client implementation that supports reactive
 *   notifications.
 *
 * Copyright 2015 Irene Zhang <iyzhang@cs.washington.edu>
 *                Niel Lebeck <nl35@cs.washington.edu>
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

#include "reactiveclient.h"

using namespace std;

void
ReactiveClient::Commit(const uint64_t tid, const Transaction &txn, Promise *promise)
{
    Debug("COMMIT [%lu]", tid);
    Promise p(COMMIT_TIMEOUT);
    Promise *pp = (promise != NULL) ? promise : &p;
    CacheClient::Commit(tid, txn, pp);

    int reply = pp->GetReply();

    if (txn.IsReactive()) {
        if (reply != REPLY_OK) {
            Panic("Reactive transaction failed to commit");
        }

        std::set<std::string> regset = txn.GetRegSet();
        uint64_t reactive_id = txn.GetReactiveId();
        Timestamp timestamp = txn.GetTimestamp();

        // Register reactive transaction if it is new or if its registration set has changed
        if (regMap.find(reactive_id) == regMap.end() || regMap[reactive_id] != regset) {
            Promise rp(COMMIT_TIMEOUT);
            txnclient->Register(reactive_id, timestamp, regset, &rp);
            int reply = rp.GetReply();
            if (reply == REPLY_OK) {
                regMap[reactive_id] = regset;
            }
            else {
                //TODO: implement registration retry
                Panic("Registration retry not implemented");
            }
        }
        else { // Otherwise, ack notification
            txnclient->ReplyToNotification(reactive_id, timestamp);
        }
        
    }
}

void
ReactiveClient::GetNextNotification(bool blocking,
                                    Promise *promise) {
    Debug("GET_NEXT_NOTIFICATION");
    Promise p(COMMIT_TIMEOUT);
    Promise *pp = (promise != NULL) ? promise : &p;
    txnclient->GetNextNotification(blocking, pp);
    if (pp->GetReactiveId() != NO_NOTIFICATION) {
        map<string, Version> values = pp->GetValues();
        processNotification(values);
    }
}

void
ReactiveClient::NotificationInit(std::function<void (void)> callback) {
    txnclient->NotificationInit(callback);
    
}

void
ReactiveClient::processNotification(const map<string, Version> &values) {
    for (auto &pair : values) {
        Debug("Adding [%s] with ts %lu to the cache (from notification)", pair.first.c_str(), pair.second.GetTimestamp());
        cache.Put(pair.first, pair.second);
    }
}
