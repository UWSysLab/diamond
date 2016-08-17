// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
/***********************************************************************
 *
 * replication/vr/client.h:
 *   dummy implementation of replication interface that just uses a
 *   single replica and passes commands directly to it
 *
 * Copyright 2013 Dan R. K. Ports  <drkp@cs.washington.edu>
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

#ifndef _VR_CLIENT_H_
#define _VR_CLIENT_H_

#include "replication/common/client.h"
#include "replication/common/configuration.h"
#include "vr-proto.pb.h"
#include "store-proto.pb.h"
#include <unordered_map>
#include <vector>

#define RETRY_WAIT 100

namespace replication {

typedef std::function<void (const Timestamp &, const std::vector<string> &)> publish_handler_t;

class VRClient : public Client
{
public:
    VRClient(const ReplicaConfig &config,
             Transport *transport,
	     publish_handler_t publications,
             const uint64_t clientid = 0);
    virtual ~VRClient();
    virtual void Invoke(const string &request,
                        continuation_t continuation);
    virtual void InvokeUnlogged(const int replicaIdx,
                                const string &request,
                                continuation_t continuation);
    virtual void ReceiveMessage(const TransportAddress &remote,
                                const string &type, const string &data);
    virtual void ReceiveError(const int error);
protected:
    uint64_t lastReqId;
    publish_handler_t publications;

    struct PendingRequest
    {
        string request;
	bool unlogged = false;
        continuation_t continuation;
        
	inline PendingRequest() { };
        inline PendingRequest(string request,
                              bool unlogged,
                              continuation_t continuation)
            : request(request),
              unlogged(unlogged),
              continuation(continuation) { };
    };

    std::unordered_map<uint64_t, PendingRequest> pendingRequests;
    int leader = 0;
    void HandleReply(const TransportAddress &remote,
                     const proto::ReplyMessage &msg);
    void HandlePublish(const TransportAddress &remote,
                       const strongstore::proto::PublishMessage &msg);
    void SendRequest(const int reqid);
};
} // namespace replication

#endif  /* _VR_CLIENT_H_ */
