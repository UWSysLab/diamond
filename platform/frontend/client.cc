// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * frontend/client.cc
 *   Client for talking to Diamond frontend server
 *
 **********************************************************************/

#include "client.h"

using namespace std;

namespace diamond {
namespace frontend {

using namespace proto;
using namespace std;
    
Client::Client(const string &configPath, Transport *transport,
	       uint64_t client_id) :
    transport(transport), client_id(client_id)
{ 
    ifstream configStream(configPath);
    if (configStream.fail()) {
        fprintf(stderr, "unable to read configuration file: %s\n",
                configPath.c_str());
    }
    init(new transport::Configuration(configStream));
}

Client::Client(const string &hostname, const string &port, Transport *transport,
	       uint64_t client_id) :
    transport(transport), client_id(client_id)
{ 
    transport::HostAddress frontendAddress(hostname, port);
    vector<transport::HostAddress> addresses;
    addresses.push_back(frontendAddress);
    init(new transport::Configuration(addresses));
}

void
Client::init(transport::Configuration *transportConfig) {
    config = transportConfig;
    transport->Register(this, *config, -1);
    
    msgid = 0;
}

Client::~Client() 
{ 
    delete config;
}

void
Client::Begin(const uint64_t tid)
{
    Debug("BEGIN [%llu]", tid);
}

void
Client::BeginRO(const uint64_t tid,
                const Timestamp timestamp)
{
    Debug("BEGIN READ-ONLY [%llu]", tid);
}

void
Client::SetNotify(notification_handler_t notify)
{
    hasNotificationHandler = true;
    this->notify = notify;
}
    
void
Client::MultiGet(const uint64_t tid,
                 const set<string> &keys,
                 const Timestamp timestamp,
                 Promise *promise)
{
    Debug("Sending MULTIGET [%u keys] at %llu", keys.size(), timestamp);

    // Fill out protobuf
    // Send message
    transport->Timer(0, [=]() {
            GetMessage msg;
            msg.set_clientid(client_id);
            for (auto key : keys) {
                msg.add_keys(key);
            }
            msg.set_txnid(tid);
            msg.set_timestamp(timestamp);
            msg.set_msgid(msgid++);
            if (transport->SendMessageToHost(this, 0, msg)) {
                if (promise != NULL)
                    waiting[msg.msgid()] = promise;
            } else if (promise != NULL) {
                promise->Reply(REPLY_NETWORK_FAILURE);
            }
        });
}

void
Client::Put(const uint64_t tid,
            const string &key, 
            const string &value,
            Promise *promise)
{
    Debug("Sending PUT [%s %s]", key.c_str(), value.c_str());
    Panic("Don't support PUT");
}

void
Client::Prepare(const uint64_t tid,
                const Transaction &txn,
                Promise *promise)
{
    Debug("Ignore PREPARE");
    Panic("Don't support PREPARE");
}

void
Client::Commit(const uint64_t tid,
               const Transaction &txn,
               Promise *promise)
{
    // If SI with no writes or read-only, just locally check the read set
    // Commit all reads locally
    if ((txn.IsolationMode() == READ_ONLY) ||
        ((txn.IsolationMode() == SNAPSHOT_ISOLATION) &&
         txn.GetWriteSet().empty() &&
         txn.GetIncrementSet().empty())) {
        // Run local checks
        Interval i(0);
        for (auto &read : txn.GetReadSet()) {
            Intersect(i, read.second);
        }
        if (i.Start() <= i.End()) {
            if (promise != NULL) promise->Reply(REPLY_OK, txn.GetTimestamp());
        } else {
            if (promise != NULL)  promise->Reply(REPLY_FAIL);
	}
        return;
    }
    
    // If eventual consistency, do no checks and don't wait for a response
    //if (txn.IsolationMode() == EVENTUAL) {
    //    if (promise != NULL) {
    //        promise->Reply(REPLY_OK);
    //    }
    //    promise = NULL;
    //}
    
    Debug("Sending COMMIT (mode=%i, t=%llu", txn.IsolationMode(), txn.GetTimestamp());
    

    // Send messages
    transport->Timer(0, [=]() {
            CommitMessage msg;
            msg.set_txnid(tid);
            txn.Serialize(msg.mutable_txn());
            msg.set_clientid(client_id);
            msg.set_msgid(msgid++);
            if (transport->SendMessageToHost(this, 0, msg)) {
		if (promise != NULL)
		    waiting[msg.msgid()] = promise;
            } else if (promise != NULL) {
                promise->Reply(REPLY_NETWORK_FAILURE);
            }
        });
}

void
Client::Abort(const uint64_t tid,
              Promise *promise)
{
    Debug("Sending ABORT");


    // Send messages
    transport->Timer(0, [=]() {
            AbortMessage msg;
            msg.set_txnid(tid);
            msg.set_clientid(client_id);
            msg.set_msgid(msgid++);
            if (transport->SendMessageToHost(this, 0, msg)) {
                if (promise != NULL) 
                    waiting[msg.msgid()] = promise;
            } else if (promise != NULL) {
                promise->Reply(REPLY_NETWORK_FAILURE);
            }
        });
}

// Callbacks from the messages that happen in the transport thread
void
Client::ReceiveMessage(const TransportAddress &remote,
                       const string &type,
                       const string &data)
{
    Debug("Received reply type: %s", type.c_str());

    static GetReply getReply;
    static CommitReply commitReply;
    static AbortReply abortReply;
    static Notification notification;
    static RegisterReply regReply;
    static DeregisterReply deregReply;

    if (type == getReply.GetTypeName()) {
        // Handle Get
        getReply.ParseFromString(data);
        auto it = waiting.find(getReply.msgid());

        if (it != waiting.end()) {
            Debug("Received GET response [%u] %i",
                  getReply.msgid(), getReply.status());
            map<string, Version> ret;
            int status = getReply.status(); 
            if (status == REPLY_OK) {
                for (int i = 0; i < getReply.replies_size(); i++) {
                    string key = getReply.replies(i).key();
                    Version v = Version(getReply.replies(i));
                    ret[key] = v;
                    ASSERT(v.GetInterval().End() != MAX_TIMESTAMP);
                    Debug("Received Get timestamp %s %llu",
                          key.c_str(), v.GetInterval().End());
                }
            }
            it->second->Reply(status, ret);
            waiting.erase(it);
        }
    } else if (type == commitReply.GetTypeName()) {
        commitReply.ParseFromString(data);
        auto it = waiting.find(commitReply.msgid());

        if (it != waiting.end() && it->second != NULL) {
	    Debug("Received COMMIT response [%u] %i",
                  commitReply.msgid(), commitReply.status());
            it->second->Reply(commitReply.status(),
                              commitReply.timestamp());
            waiting.erase(it);
        }
    } else if (type == abortReply.GetTypeName()) {
        abortReply.ParseFromString(data);
        auto it = waiting.find(abortReply.msgid());

        if (it != waiting.end() && it->second != NULL) {
	    Debug("Received ABORT response [%u]",
                  abortReply.msgid());
            it->second->Reply(abortReply.status());
            waiting.erase(it);
        }
    } else if (type == notification.GetTypeName()) {
        notification.ParseFromString(data);
        uint64_t reactive_id = notification.reactiveid();
        Timestamp timestamp = notification.timestamp();
        Debug("Received NOTIFICATION (reactive_id %lu, timestamp %lu)",
              reactive_id, timestamp);

        map<string, Version> values;
        for (int i = 0; i < notification.replies_size(); i++) {
            string key = notification.replies(i).key();
            Version value(notification.replies(i));
            values[key] = value;
        }
        
        if (hasNotificationHandler) {
            notify(reactive_id, timestamp, values);
        }
                
        // Ack notification
        Ack(reactive_id, set<string>(), timestamp);
    } else if (type == regReply.GetTypeName()) {
        regReply.ParseFromString(data);
        auto it = waiting.find(regReply.msgid());

        if (it != waiting.end() && it->second != NULL) {
	    Debug("Received REGISTER response [%u] %i",
                  regReply.msgid(), regReply.status());
            it->second->Reply(regReply.status());
            waiting.erase(it);
        }
    } else if (type == deregReply.GetTypeName()) {
        deregReply.ParseFromString(data);
        auto it = waiting.find(deregReply.msgid());

        if (it != waiting.end() && it->second != NULL) {
	    Debug("Received DEREGISTER response [%u] %i",
                  deregReply.msgid(), deregReply.status());
            it->second->Reply(deregReply.status());
            waiting.erase(it);
        }
    }
}

void
Client::Subscribe(const uint64_t reactive_id,
                  const std::set<std::string> &keys,
                  const Timestamp timestamp,
                  Promise *promise) {
    Debug("Sending REGISTER for reactive_id %llu at timestamp %llu",
          reactive_id, timestamp);
    for (auto &key : keys) {
        Debug("Registering key: %s", key.c_str());
    }
    
    // Send message
    transport->Timer(0, [=]() {
            RegisterMessage msg;
            msg.set_clientid(client_id);
            for (auto &key : keys) {
                msg.add_keys(key);
            }
            msg.set_reactiveid(reactive_id);
            msg.set_timestamp(timestamp);
            msg.set_msgid(msgid++);
            if (transport->SendMessageToHost(this, 0, msg)) {
                if (promise != NULL)
                    waiting[msg.msgid()] = promise;
            } else if (promise != NULL) {
                promise->Reply(REPLY_NETWORK_FAILURE);
            }
        });
}

void
Client::Unsubscribe(const uint64_t reactive_id,
                    const set<string> &keys,
                    Promise *promise) {
    Debug("Sending DEREGISTER for reactive_id %llu", reactive_id);
    // Send message
    transport->Timer(0, [=]() {
            DeregisterMessage msg;
            msg.set_clientid(client_id);
            msg.set_reactiveid(reactive_id);
            msg.set_msgid(msgid++);
            if (transport->SendMessageToHost(this, 0, msg)) {
                if (promise != NULL)
                    waiting[msg.msgid()] = promise;
            } else if (promise != NULL) {
                promise->Reply(REPLY_NETWORK_FAILURE);
            }
        });
}

void
Client::Ack(const uint64_t reactive_id,
            const set<string> &keys,
            const Timestamp timestamp,
            Promise *promise)
{
    Debug("Sending ACK for reactive_id \
           %llu at timestamp %llu",
          reactive_id, timestamp);

    // Send message
    transport->Timer(0, [=]() {
            NotificationReply msg;
            msg.set_clientid(client_id);
            msg.set_reactiveid(reactive_id);
            msg.set_timestamp(timestamp);
            msg.set_msgid(msgid++);
	    transport->SendMessageToHost(this, 0, msg);
	});
}

} // namespace frontend
} // namespace diamond
