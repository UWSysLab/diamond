// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * store/strongstore/server.h:
 *   A single transactional server replica.
 *
 * Copyright 2015 Irene Zhang <iyzhang@cs.washington.edu>
 *                Naveen Kr. Sharma <nksharma@cs.washington.edu>
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

#ifndef _STRONG_SERVER_H_
#define _STRONG_SERVER_H_

#include "lib/tcptransport.h"
#include "replication/replica.h"
#include "store/pubstore.h"
#include "common-proto.pb.h"
#include "store-proto.pb.h"
#include "notification-proto.pb.h"

#include <thread>

namespace strongstore {

class Server : public replication::AppReplica
{
public:
    Server(const replication::ReplicaConfig &config,
           int index, int batchSize);
    virtual ~Server();

    virtual void LeaderUpcall(opnum_t opnum,
			      const string &str1,
			      const TransportAddress &remote,
			      bool &replicate,
			      string &str2);
    virtual void ReplicaUpcall(opnum_t opnum,
			       const string &str1,
			       string &str2);
    virtual void UnloggedUpcall(const string &str1,
				string &str2);
    void Load(const string &key,
	      const string &value,
	      const Timestamp timestamp);
    void Run();
private:
    PubStore *store;
    replication::VRReplica *replica;
    
    void HandleGet(const proto::Request &request,
                   string &str2);
    bool HandlePrepare(const proto::Request &request,
                       string &str2);
    void HandleCommit(const proto::Request &request);
    void HandleSubscribe(const TCPTransportAddress &remote,
                         const proto::Request &request,
                         string &str2);
    void HandleUnsubscribe(const TCPTransportAddress &remote,
                           const proto::Request &request,
                           string &str2);
    void HandleAck(const TCPTransportAddress &remote,
                   const proto::Request &request,
                   string &str2);
    void Publish(const uint64_t tid, const Timestamp timestamp);
};

} // namespace strongstore

#endif /* _STRONG_SERVER_H_ */
