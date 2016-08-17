// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
/***********************************************************************
 *
 * frontend/frontendserver.h:
 *   Front-end servers providing access to the diamond store
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

#ifndef _DIAMOND_FRONTEND_SERVER_H_
#define _DIAMOND_FRONTEND_SERVER_H_

#include <mutex>

#include "lib/configuration.h"
#include "lib/tcptransport.h"
#include "diamond-proto.pb.h"
#include "notification-proto.pb.h"
#include "store/client.h"
#include "store/common/backend/txnstore.h"
namespace diamond {
namespace frontend {

class Server : public TransportReceiver
{
public:
    Server(const string &configPath,
           const int nShards,
           const int closestReplica,
           Transport *transport);
    virtual ~Server();
    
protected:
    void ReceiveMessage(const TransportAddress &remote,
                        const std::string &type, const std::string &data);
    void ReceiveError(int error) { };
    void HandleGet(const TransportAddress &remote,
                   const proto::GetMessage &msg);
    void HandleCommit(const TransportAddress &remote,
                      const proto::CommitMessage &msg);
    void HandleAbort(const TransportAddress &remote,
                     const proto::AbortMessage &msg);
    void HandleRegister(const TransportAddress &remote,
                        const RegisterMessage &msg);
    void HandleDeregister(const TransportAddress &remote,
                          const DeregisterMessage &msg);
    void HandlePublish(const Timestamp &timestamp,
                       const std::vector<string> &keys);
    void HandleNotificationReply(const TransportAddress &remote,
                                 const NotificationReply &msg);
    
private:
    Transport *transport;
    strongstore::Client *store;

    std::unordered_map<uint64_t, ReactiveTransaction> transactions; // map frontend index to data structure
    std::unordered_map<std::string, std::set<uint64_t> > listeners; // map key to the frontend indices of the reactive transactions listening to it
    std::unordered_map<std::string, Version> cachedValues; // map keys to their cached values

    uint64_t getFrontendIndex(uint64_t client_id, uint64_t reactive_id);
    void SendNotification(const uint64_t client_id, const uint64_t reactive_id, const Timestamp timestamp);
    void UpdateCache(const ReactiveTransaction &rt, const Timestamp timestamp);

    // callbacks
    void GetCallback(const TransportAddress *remote,
                     const proto::GetMessage msg,
                     Promise *promise);
    void CommitCallback(const TransportAddress *remote,
                        const proto::CommitMessage msg,
                        Promise *promise);
    void SubscribeCallback(const TransportAddress *remote,
                           const RegisterMessage msg,
                           Promise *promise);
    void UpdateCacheCallback(const uint64_t client_id,
                             const uint64_t reactive_id,
                             const Timestamp next_timestamp,
                             Promise *promise);

};

} // namespace frontend
} // namespace diamond

#endif  /* _DIAMOND_FRONTEND_SERVER_H */
