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
    config = new transport::Configuration(configStream);
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
    Debug("BEGIN [%lu]", tid);
}

void
Client::BeginRO(const uint64_t tid, const Timestamp &timestamp)
{
    Debug("BEGIN READ-ONLY [%lu]", tid);
}

void
Client::Get(const uint64_t tid,
            const string &key,
            const Timestamp &timestamp,
            Promise *promise)
{
    Debug("Sending GET [%s]", key.c_str());

    vector<string> keys;
    keys.push_back(key);
    MultiGet(tid, keys, timestamp, promise);
}

void
Client::MultiGet(const uint64_t tid,
                 const vector<string> &keys,
                 const Timestamp &timestamp,
                 Promise *promise)
{
    Debug("Sending MULTIGET [%lu keys] at %lu", keys.size(), timestamp);

    // Fill out protobuf
    GetMessage msg;
    msg.set_clientid(client_id);
    for (auto key : keys) {
        msg.add_keys(key);
    }
    msg.set_txnid(tid);
    msg.set_msgid(msgid++);
    msg.set_timestamp(timestamp);

    // Send message
    transport->Timer(0, [=]() {
            if (transport->SendMessageToReplica(this, 0, msg)) {
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
        ((txn.IsolationMode() == SNAPSHOT_ISOLATION) && txn.GetWriteSet().empty())) {
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
    if (txn.IsolationMode() == EVENTUAL) {
        if (promise != NULL) {
            promise->Reply(REPLY_OK);
        }
	promise = NULL;
    }
    
    Debug("Sending COMMIT (mode=%i, t=%lu", txn.IsolationMode(), txn.GetTimestamp());
    
    CommitMessage msg;
    msg.set_txnid(tid);
    txn.Serialize(msg.mutable_txn());
    msg.set_msgid(msgid++);
    msg.set_clientid(client_id);

    // Send messages
    transport->Timer(0, [=]() {
            if (transport->SendMessageToReplica(this, 0, msg)) {
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

    AbortMessage msg;
    msg.set_txnid(tid);
    msg.set_msgid(msgid++);
    msg.set_clientid(client_id);

    // Send messages
    transport->Timer(0, [=]() {
            if (transport->SendMessageToReplica(this, 0, msg)) {
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

    if (type == getReply.GetTypeName()) {
        // Handle Get
        getReply.ParseFromString(data);
        auto it = waiting.find(getReply.msgid());

        if (it != waiting.end()) {
	    Debug("Received GET response [%u] %i", getReply.msgid(), getReply.status());
            map<string, Version> ret;
            int status = getReply.status(); 
            if (status == REPLY_OK) {
                for (int i = 0; i < getReply.replies_size(); i++) {
		    ret[getReply.replies(i).key()] = Version(getReply.replies(i));
                }
            }
            it->second->Reply(status, ret);
            waiting.erase(it);
        }
    } else if (type == commitReply.GetTypeName()) {
        commitReply.ParseFromString(data);
        auto it = waiting.find(commitReply.msgid());

        if (it != waiting.end() && it->second != NULL) {
	    Debug("Received COMMIT response [%u] %i", commitReply.msgid(), commitReply.status());
            it->second->Reply(commitReply.status(), commitReply.timestamp());
            waiting.erase(it);
        }
    } else if (type == abortReply.GetTypeName()) {
        abortReply.ParseFromString(data);
        auto it = waiting.find(abortReply.msgid());

        if (it != waiting.end() && it->second != NULL) {
	    Debug("Received ABORT response [%u]", abortReply.msgid());
            it->second->Reply(abortReply.status());
            waiting.erase(it);
        }
    } else if (type == notification.GetTypeName()) {
        notification.ParseFromString(data);
        uint64_t reactive_id = notification.reactiveid();
        Timestamp timestamp = notification.timestamp();
        //TODO: parse cache entries and pass them back through the promise
        std::map<std::string, Version> cache_entries;
        notification_lock.lock();
        if (reactive_promise != NULL) {
            notification_lock.unlock();
            reactive_promise->Reply(REPLY_OK, timestamp, cache_entries, reactive_id);
            reactive_promise = NULL;
        }
        else {
            pending_notifications.push(std::pair<uint64_t, Timestamp>(reactive_id, timestamp));
            notification_lock.unlock();
        }
    }
}

void
Client::GetNextNotification(Promise *promise) {
    notification_lock.lock();
    if (reactive_promise != NULL) {
        Panic("Error: GetNextNotification called from multiple threads");
    }
    if (!pending_notifications.empty()) {
        std::pair<uint64_t, Timestamp> notification = pending_notifications.front();
        pending_notifications.pop();
        notification_lock.unlock();
        //TODO: store cache entries and pass them back through the promise
        std::map<std::string, Version> cache_entries;
        promise->Reply(REPLY_OK, notification.second, cache_entries, notification.first);
    }
    else {
        reactive_promise = promise;
        notification_lock.unlock();
    }
}

void
Client::Register(const uint64_t reactive_id,
                 const Timestamp timestamp,
                 const std::set<std::string> keys,
                 Promise *promise) {

    RegisterMessage msg;
    msg.set_clientid(client_id);
    for (auto key : keys) {
        msg.add_keys(key);
    }
    msg.set_reactiveid(reactive_id);
    msg.set_msgid(msgid++);
    msg.set_timestamp(timestamp);

    // Send message
    transport->Timer(0, [=]() {
            if (transport->SendMessageToReplica(this, 0, msg)) {
                if (promise != NULL)
                    waiting[msg.msgid()] = promise;
            } else if (promise != NULL) {
                promise->Reply(REPLY_NETWORK_FAILURE);
            }
        });
}

} // namespace frontend
} // namespace diamond
