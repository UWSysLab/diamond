// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
/***********************************************************************
 *
 * replica.cc:
 *   common functions for replica implementation regardless of
 *   replication protocol
 *
 * Copyright 2013-2015 Irene Zhang <iyzhang@cs.washington.edu>
 *                     Naveen Kr. Sharma <nksharma@cs.washington.edu>
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

#include "server.h"

namespace diamond {
namespace frontend {

using namespace proto;
using namespace std;

Server::Server(Transport *transport, strongstore::Client *client)
    : transport(transport), store(client)
{
    sendNotificationTimeout = new Timeout(transport, 100, [this]() {
            sendNotifications();
        });
}

Server::~Server()
{
    
}

void
Server::ReceiveMessage(const TransportAddress &remote,
                       const string &type, const string &data)
{
    static GetMessage get;
    static CommitMessage commit;
    static AbortMessage abort;
    static RegisterMessage reg;
    static NotifyFrontendMessage notify;
    static NotificationReply reply;

    if (type == get.GetTypeName()) {
        get.ParseFromString(data);
        HandleGet(remote, get);
    } else if (type == commit.GetTypeName()) {
        commit.ParseFromString(data);
        HandleCommit(remote, commit);
    } else if (type == abort.GetTypeName()) {
        abort.ParseFromString(data);
        HandleAbort(remote, abort);
    } else if (type == reg.GetTypeName()) {
        reg.ParseFromString(data);
        HandleRegister(remote, reg);
    } else if (type == notify.GetTypeName()) {
        notify.ParseFromString(data);
        HandleNotifyFrontend(remote, notify);
    } else if (type == reply.GetTypeName()) {
        reply.ParseFromString(data);
        HandleNotificationReply(remote, reply);
    } else {
        Panic("Received unexpected message type in OR proto: %s",
              type.c_str());
    }
}

uint64_t
Server::getFrontendIndex(uint64_t client_id, uint64_t reactive_id) {
    return ((client_id << 32) | reactive_id);
}

void
Server::HandleNotificationReply(const TransportAddress &remote,
                                const NotificationReply &msg) {
    Debug("Handling NOTIFICATION-REPLY");
    uint64_t frontend_index = getFrontendIndex(msg.clientid(), msg.reactiveid());
    transactions[frontend_index].last_timestamp = msg.timestamp();
}

/*
 * For every key, find all the reactive transactions listening to it
 * and update the next timestamps to run them at, and update the cached
 * value for the key.
 */
void
Server::HandleNotifyFrontend(const TransportAddress &remote,
                             const NotifyFrontendMessage &msg) {
    Debug("Handling NOTIFY-FRONTEND");
    for (int i = 0; i < msg.replies_size(); i++) {
        std::string key = msg.replies(i).key();
        Version value(msg.replies(i));
        Timestamp timestamp = value.GetTimestamp();
        cachedKeys.insert(key);
        values[key] = value;
        for (auto it = listeners[key].begin(); it != listeners[key].end(); it++) {
            if (transactions[*it].next_timestamp < timestamp) {
                transactions[*it].next_timestamp = timestamp;
            }
        }
    }

    NotifyFrontendAck ack;
    ack.set_txnid(msg.txnid());
    ack.set_address(string(GetAddress().getHostname() + ":" + GetAddress().getPort()));
    transport->SendMessage(this, remote, ack);

    if (!sendNotificationTimeout->Active()) {
        sendNotificationTimeout->Start();
    }
}

void
Server::sendNotifications() {
    Debug("Sending notifications");
    bool anyOutstanding = false;
    for (auto it = transactions.begin(); it != transactions.end(); it++) {
        ReactiveTransaction rt = it->second;
        if (rt.next_timestamp > rt.last_timestamp) {
            anyOutstanding = true;
            Debug("Sending NOTIFICATION: reactive_id %lu, timestamp %lu, client %s:%s", rt.reactive_id, rt.next_timestamp, rt.client_hostname.c_str(), rt.client_port.c_str());
            Notification notification;
            notification.set_clientid(rt.client_id);
            notification.set_reactiveid(rt.reactive_id);
            notification.set_timestamp(rt.next_timestamp);
            for (auto key : rt.keys) {
                if (cachedKeys.find(key) != cachedKeys.end()) {
                    Version value = values[key];
                    ReadReply * reply = notification.add_replies();
                    reply->set_key(key);
                    value.Serialize(reply);
                }
            }
            transport->SendMessage(this, rt.client_hostname, rt.client_port, notification);
            Debug("FINISHED sending NOTIFICATION: reactive_id %lu, timestamp %lu, client %s:%s", rt.reactive_id, rt.next_timestamp, rt.client_hostname.c_str(), rt.client_port.c_str());
        }
    }
    if (!anyOutstanding) {
        sendNotificationTimeout->Stop();
    }
}

void
Server::HandleRegister(const TransportAddress &remote,
                       const RegisterMessage &msg)
{
    Debug("Handling REGISTER");
    for (int i = 0; i < msg.keys_size(); i++) {
        Debug("Registering %s", msg.keys(i).c_str());
    }

    set<string> subscribeSet; // set of keys from regSet that we aren't already subscribed to
    for (int i = 0; i < msg.keys_size(); i++) {
        string key = msg.keys(i);
        if (listeners.find(key) == listeners.end()) {
            subscribeSet.insert(key);
        }
    }

    map<string, Version> subscribeValues;
    callback_t cb =
	std::bind(&Server::SubscribeCallback,
		  this,
		  remote.clone(),
		  msg,
		  placeholders::_1);

    store->Subscribe(subscribeSet, GetAddress(), cb);
}

void
Server::SubscribeCallback(const TransportAddress *remote,
			  const RegisterMessage msg,
			  Promise *promise)
{
    set<string> regSet;
    for (int i = 0; i < msg.keys_size(); i++) {
        string key = msg.keys(i);
        regSet.insert(key);
    }

    map<string, Version> subscribeValues = promise->GetValues();
    for (auto &pair : subscribeValues) {
        if (values.find(pair.first) == values.end()) {
            values[pair.first] = pair.second;
        }
    }

    Timestamp timestamp = msg.timestamp();
    for (auto it = regSet.begin(); it != regSet.end(); it++) {
        Timestamp keyTimestamp = values[*it].GetTimestamp();
        if (timestamp < keyTimestamp) {
            timestamp = keyTimestamp;
        }
    }

    ReactiveTransaction rt;
    rt.frontend_index = getFrontendIndex(msg.clientid(), msg.reactiveid());
    rt.reactive_id = msg.reactiveid();
    rt.client_id = msg.clientid();
    rt.last_timestamp = msg.timestamp();
    rt.next_timestamp = timestamp;
    rt.keys = regSet;
    rt.client_hostname = remote->getHostname();
    rt.client_port = remote->getPort();

    transactions[rt.frontend_index] = rt;

    for (auto it = rt.keys.begin(); it != rt.keys.end(); it++) {
        listeners[*it].insert(rt.frontend_index);
    }

    RegisterReply reply;
    reply.set_status(REPLY_OK);
    reply.set_msgid(msg.msgid());
    transport->Timer(0, [=]() { 
	    transport->SendMessage(this, *remote, reply);
	    sendNotificationTimeout->Reset();
	    delete remote;
	    });
    delete promise;
}

void
Server::HandleGet(const TransportAddress &remote,
                  const GetMessage &msg)
{
    pair<Timestamp,string> value;

    vector<string> keys;

    for (int i = 0; i < msg.keys_size(); i++) {
	Debug("GET %s", msg.keys(i).c_str());
        keys.push_back(msg.keys(i));
    }

    callback_t cb =
	std::bind(&Server::GetCallback,
	     this,
	     remote.clone(),
	     msg,
	     placeholders::_1);

    if (keys.size() > 1) {
        store->MultiGet(msg.txnid(), keys, cb,
			msg.timestamp());
    } else {
        store->Get(msg.txnid(), keys[0], cb,
		   msg.timestamp());
    }
}
    
void
Server::GetCallback(const TransportAddress *remote,
		    const GetMessage msg,
		    Promise *promise)
{
    map<string, Version> values = promise->GetValues();
    GetReply reply;
    reply.set_status(promise->GetReply());
    reply.set_msgid(msg.msgid());
    if (promise->GetReply() == REPLY_OK) {
	for (auto value : values) {
	    ReadReply *rep = reply.add_replies();
	    Debug("GET %s %lu",
		  value.first.c_str(),
		  value.second.GetInterval().End());
	    rep->set_key(value.first);
	    value.second.Serialize(rep);
        }
    }

    transport->Timer(0, [=]() {
	    transport->SendMessage(this, *remote, reply);
	    delete remote;
	});

}

void
Server::HandleCommit(const TransportAddress &remote,
                     const CommitMessage &msg)
{
    Transaction txn(msg.txn());
    callback_t cb =
	std::bind(&Server::CommitCallback,
	     this,
	     remote.clone(),
	     msg,
	     placeholders::_1);

    store->Commit(msg.txnid(), cb, txn);
}

void
Server::CommitCallback(const TransportAddress *remote,
		       const CommitMessage msg,
		       Promise *promise)
{
    CommitReply reply;
    reply.set_status(promise->GetReply());
    reply.set_txnid(msg.txnid());
    reply.set_msgid(msg.msgid());
    reply.set_timestamp(promise->GetTimestamp());
    transport->Timer(0, [=]() {
	    transport->SendMessage(this, *remote, reply);
	    delete remote;
	});
    delete promise;
}

void
Server::HandleAbort(const TransportAddress &remote,
                    const AbortMessage &msg)
{
    store->Abort(msg.txnid());

    AbortReply reply;
    reply.set_status(REPLY_OK);
    reply.set_txnid(msg.txnid());
    reply.set_msgid(msg.msgid());
    transport->SendMessage(this, remote, reply);
}

} // namespace frontend
} // namespace diamond

int
main(int argc, char **argv)
{
    unsigned int maxShard = 1;
    int closestReplica = 0;
    const char *configPath = NULL;
    const char *backendConfig = NULL;
    // Parse arguments
    int opt;
    while ((opt = getopt(argc, argv, "c:i:m:e:s:N:b:")) != -1) {
        switch (opt) {
        case 'c':
            configPath = optarg;
            break;

        case 'b':
            backendConfig = optarg;
            break;

        case 'i':
        {
            char *strtolPtr;
            closestReplica = strtoul(optarg, &strtolPtr, 10);
            if ((*optarg == '\0') || (*strtolPtr != '\0') || (closestReplica < 0))
            {
                fprintf(stderr, "option -i requires a numeric arg\n");
            }
            break;
        }
        
        case 'N':
        {
            char *strtolPtr;
            maxShard = strtoul(optarg, &strtolPtr, 10);
            if ((*optarg == '\0') || (*strtolPtr != '\0') || (maxShard <= 0))
            {
                fprintf(stderr, "option -e requires a numeric arg\n");
            }
            break;
        }

        default:
            fprintf(stderr, "Unknown argument %s\n", argv[optind]);
        }


    }

    if (!configPath) {
        fprintf(stderr, "option -c is required\n");
    }

    if (!backendConfig) {
        fprintf(stderr, "option -b is required\n");
    }

    if (closestReplica == -1) {
        fprintf(stderr, "option -i is required\n");
    }

    // Load configuration
    std::ifstream configStream(configPath);
    if (configStream.fail()) {
        fprintf(stderr, "unable to read configuration file: %s\n", configPath);
    }
    transport::Configuration config(configStream);

    TCPTransport transport(0.0, 0.0, 0);

    strongstore::Client *client = new strongstore::Client(backendConfig, maxShard, closestReplica);

    diamond::frontend::Server server = diamond::frontend::Server(&transport, client);

    // The server's transport for handling incoming connections
    transport.Register(&server, config, 0);

    transport.Run();

    return 0;
}
