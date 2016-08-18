  // -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
  /***********************************************************************
 *
 * vr/client.cc:
 *   Viewstamped Replication clinet
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

#include "replication/common/client.h"
#include "request.pb.h"
#include "lib/assert.h"
#include "lib/message.h"
#include "lib/transport.h"
#include "replication/client.h"
#include "vr-proto.pb.h"
#include "store-proto.pb.h"
#include "includes/error.h"

namespace replication {

VRClient::VRClient(const ReplicaConfig &config,
                   Transport *transport,
		   publish_handler_t publications,
                   const uint64_t clientid)
    : Client(config, transport, clientid), publications(publications)
{
    lastReqId = 0;
}

VRClient::~VRClient()
{
}

void
VRClient::Invoke(const string &request,
                 continuation_t continuation)
{
    ++lastReqId;
    pendingRequests[lastReqId] =
	PendingRequest(request, false, continuation);
    SendRequest(lastReqId);
}

void
VRClient::InvokeUnlogged(const int replicaIdx,
                         const string &request,
                         continuation_t continuation)
{
    ++lastReqId;
    pendingRequests[lastReqId] =
	PendingRequest(request, true, continuation);

    proto::UnloggedRequestMessage reqMsg;
    reqMsg.mutable_req()->set_op(pendingRequests[lastReqId].request);
    reqMsg.mutable_req()->set_clientid(clientid);
    reqMsg.mutable_req()->set_clientreqid(lastReqId);
    Debug("SENDING UNLOGGED REQUEST: %lu %lu", clientid, lastReqId);
    
    transport->SendMessageToHost(this, leader, reqMsg);
}

void
VRClient::SendRequest(const int reqid) {
    
    proto::RequestMessage reqMsg;
    reqMsg.mutable_req()->set_op(pendingRequests[reqid].request);
    reqMsg.mutable_req()->set_clientid(clientid);
    reqMsg.mutable_req()->set_clientreqid(reqid);
    
    Debug("SENDING REQUEST: %lu %lu", clientid, reqid);
    transport->SendMessageToHost(this, leader, reqMsg);
}   
void
VRClient::ReceiveMessage(const TransportAddress &remote,
                         const string &type,
                         const string &data)
{
    static proto::ReplyMessage reply;
    static strongstore::proto::PublishMessage publish;
        
    if (type == reply.GetTypeName()) {
        reply.ParseFromString(data);
        HandleReply(remote, reply);
    } else if (type == publish.GetTypeName()) {
    	publish.ParseFromString(data);
    	HandlePublish(remote, publish);
    } else {
        Client::ReceiveMessage(remote, type, data);
    }
}

void
VRClient::HandleReply(const TransportAddress &remote,
                      const proto::ReplyMessage &msg)
{
    uint64_t reqId = msg.clientreqid();
    
    if (pendingRequests.find(reqId) == pendingRequests.end()) {
        Warning("Received reply when no request was pending");
        return;
    }

    if (msg.has_status() && msg.status() == REPLY_RETRY) {
	transport->Timer(RETRY_WAIT, [=]() {
		SendRequest(reqId);
	    });
    } else if (msg.has_reply()) {    
	Debug("Client received reply: %lu", reqId);

	pendingRequests[reqId].continuation(pendingRequests[reqId].request, msg.reply());
	pendingRequests.erase(reqId);
    }
}

void
VRClient::HandlePublish(const TransportAddress &remote,
                        const strongstore::proto::PublishMessage &msg)
{
    std::vector<string> keys;
    for (int i = 0; i < msg.keys_size(); i++) {
	keys.push_back(msg.keys(i));
    }
    publications(msg.timestamp(), keys);

    string request_str;
    strongstore::proto::Request request;
    request.set_op(strongstore::proto::Request::ACK);
    *(request.mutable_ack()) = msg;
    Debug("Client acking timestamp %lu", request.ack().timestamp());
    request.SerializeToString(&request_str);
    Invoke(request_str, [] (const string &str1, const string &str2) {});
}

void
VRClient::ReceiveError(const int error)
{
    // only works for one failure :)
    if (leader == 0) {
	leader = 1;

	for (auto r : pendingRequests) {
	    if (r.second.unlogged) {
		transport->Timer(RETRY_WAIT, [=]() {
			proto::UnloggedRequestMessage reqMsg;
			reqMsg.mutable_req()->set_op(r.second.request);
			reqMsg.mutable_req()->set_clientid(clientid);
			reqMsg.mutable_req()->set_clientreqid(r.first);
		Debug("SENDING UNLOGGED REQUEST: %lu %lu", clientid, r.first);
		transport->SendMessageToHost(this, leader, reqMsg);
		    });
	    } else {
		transport->Timer(RETRY_WAIT, [=]() {
			SendRequest(r.first);
		    });
	    }
	}
    }
}
} // namespace replication
