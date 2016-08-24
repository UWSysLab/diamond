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

Server::Server(const string &configPath,
               const int nShards,
               const int closestReplica,
               Transport *transport)
    : transport(transport)
{
    store =
        new strongstore::Client(configPath,
                                nShards,
                                closestReplica,
                                transport);
    store->SetPublish(bind(&Server::HandlePublish,
                           this,
                           placeholders::_1,
                           placeholders::_2));
}

Server::~Server()
{
    
}

// Handle messages from frontend client (aka diamond mobile client)
void
Server::ReceiveMessage(const TransportAddress &remote,
                       const string &type, const string &data)
{
    static GetMessage get;
    static CommitMessage commit;
    static AbortMessage abort;
    static RegisterMessage reg;
    static DeregisterMessage deregister;
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
    } else if (type == deregister.GetTypeName()) {
        deregister.ParseFromString(data);
        HandleDeregister(remote, deregister);
    } else if (type == reply.GetTypeName()) {
        reply.ParseFromString(data);
        HandleNotificationReply(remote, reply);
    } else {
        Panic("Received unexpected message type in OR proto: %s",
              type.c_str());
    }
}

uint64_t
Server::getFrontendIndex(uint64_t client_id,
                         uint64_t reactive_id) {
    return ((client_id << 32) | reactive_id);
}

void
Server::HandleNotificationReply(const TransportAddress &remote,
                                const NotificationReply &msg) {
    Debug("Handling NOTIFICATION-REPLY for client_id \
           %lu, reactive_id %lu, timestamp %lu",
          msg.clientid(),
          msg.reactiveid(),
          msg.timestamp());
    uint64_t frontend_index = getFrontendIndex(msg.clientid(),
                                               msg.reactiveid());
    ReactiveTransaction *rt = transactions[frontend_index];
    if (msg.timestamp() > rt->last_timestamp) {
        rt->last_timestamp = msg.timestamp();
    }

    store->Ack(msg.reactiveid(),
               rt->keys,
               msg.timestamp(),
               [] (Promise &promise) { });
}
    
/*
 * For every key, find all the reactive transactions listening to it
 * and update the next timestamps to run them at, and update the cached
 * value for the key.
 */
void
Server::HandlePublish(const Timestamp timestamp,
                      const set<string> &keys)
{
    set<ReactiveTransaction *> rts;
    
    for (auto &key : keys) { // trigger notifications
        for (auto listener : listeners[key]) {
            ReactiveTransaction *rt = transactions[listener];
            if (rt->next_timestamp < timestamp)
                rt->next_timestamp = timestamp;
            if (rt->last_timestamp < timestamp)
                rts.insert(rt);
        }
    }

    Debug("Handling PUBLISH at timestamp %lu for %lu",
          timestamp, rts.size());

    for (auto rt : rts) {
        Debug("PUBLISH to client id %lu reactive id %lu keys %lu",
              rt->client_id,
              rt->reactive_id,
              rt->keys.size());

        // do a multiget with a callback that sends the notification
        store->MultiGet(rt->reactive_id,
                        rt->keys,
                        bind(&Server::NotificationGetCallback,
                             this,
                             rt,
                             timestamp,
                             placeholders::_1),
                        timestamp);
    }
}

void
Server::NotificationGetCallback(const ReactiveTransaction *rt,
                                const Timestamp timestamp,
                                Promise &promise)
{
    if (promise.GetReply() == REPLY_OK) {
        // Schedule a notification for the client
        SendNotification(rt, timestamp, promise.GetValues());
    }
}
        
void
Server::SendNotification(const ReactiveTransaction *rt,
                         const Timestamp timestamp,
                         const map<string, Version> &values) {
    if (rt->last_timestamp < timestamp) {
        Notification notification;

        notification.set_clientid(rt->client_id);
        notification.set_reactiveid(rt->reactive_id);
        notification.set_timestamp(timestamp);

        //ASSERT(values.size() == rt->keys.size());
        
        for (auto &v : values) {
            const string &key = v.first;
            const Version &value = v.second;
            Debug("Packing entry %s (%lu, %lu)",
                  key.c_str(),
                  value.GetInterval().Start(),
                  value.GetInterval().End());
            ReadReply * reply = notification.add_replies();
            reply->set_key(key);
            value.Serialize(reply);
        }
        transport->SendMessage(this, *(rt->client), notification);
        Debug("FINISHED sending NOTIFICATION: reactive_id %lu, \
          timestamp %lu, client %lu keys %lu",
              rt->reactive_id, timestamp, rt->client_id, rt->keys.size());

        // schedule a check back in 50ms
        transport->Timer(50, [=] {
                SendNotification(rt, timestamp, values);
        });
    }
}
    
void
Server::HandleRegister(const TransportAddress &remote,
                       const RegisterMessage &msg)
{
    Debug("Handling REGISTER %lu", msg.timestamp());
    set<string> regSet;
    for (int i = 0; i < msg.keys_size(); i++) {
        Debug("Registering %s", msg.keys(i).c_str());
        regSet.insert(msg.keys(i));
    }

    // subscribe to keys from regSet that no reactive transaction is
    // already listening to
    set<string> subscribeSet;
    for (const string &key : regSet) {
        if (listeners[key].size() == 0) {
            subscribeSet.insert(key);
        }
    }

    // unsubscribe to keys from the old registration set that are not
    // in the new regSet and that no other reactive transaction is
    // listening to
    set<string> unsubscribeSet;
    uint64_t frontendIndex =
        getFrontendIndex(msg.clientid(), msg.reactiveid());
    ReactiveTransaction *rt;
    if (transactions.count(frontendIndex)) {
        rt = transactions[frontendIndex];
        set<string> oldRegSet = rt->keys;
        for (const string &key : oldRegSet) {
            if (!regSet.count(key) &&
                listeners[key].size() == 1 &&
                listeners[key].count(frontendIndex)) {
                unsubscribeSet.insert(key);
            }

            if (!regSet.count(key)) {
                listeners[key].erase(frontendIndex);
            }
        }
    } else {
        rt = new ReactiveTransaction(frontendIndex,
                                     msg.reactiveid(),
                                     msg.clientid(),
                                     regSet,
                                     remote.clone());
        
        rt->last_timestamp = Timestamp(0);
        rt->next_timestamp = msg.timestamp();
    }
    
    if (subscribeSet.size() > 0) {
        callback_t cb =
        std::bind(&Server::SubscribeCallback,
                  this,
                  rt,
                  rt->next_timestamp,
                  placeholders::_1);

        store->Subscribe(msg.reactiveid(),
                         subscribeSet,
                         rt->next_timestamp,
                         cb);
    } else {
        Promise p;
        p.Reply(REPLY_OK);
        SubscribeCallback(rt, rt->next_timestamp, p);
    }

    if (unsubscribeSet.size() > 0) {
        store->Unsubscribe(msg.reactiveid(),
                           unsubscribeSet,
                           [] (Promise &promise) { });
    }

    RegisterReply reply;
    reply.set_status(REPLY_OK);
    reply.set_msgid(msg.msgid());
    transport->SendMessage(this, remote, reply);

}

void
Server::SubscribeCallback(ReactiveTransaction *rt,
                          const Timestamp timestamp,
                          Promise &promise)
{
    if (promise.GetReply() == REPLY_OK) {
        transactions[rt->frontend_index] = rt;

        for (auto &key : rt->keys) {
            listeners[key].insert(rt->frontend_index);
        }

        // do a multiget and send the first notification
        store->MultiGet(rt->reactive_id,
                        rt->keys,
                        bind(&Server::NotificationGetCallback,
                             this,
                             rt,
                             timestamp,
                             placeholders::_1),
                        timestamp);
    }
}


void
Server::HandleDeregister(const TransportAddress &remote,
                         const DeregisterMessage &msg)
{
    Debug("Handling DEREGISTER");

    DeregisterReply reply;
    reply.set_msgid(msg.msgid());

    uint64_t frontendIndex =
        getFrontendIndex(msg.clientid(), msg.reactiveid());
    if (!transactions.count(frontendIndex)) {
        Debug("No reactive transaction with client_id %lu, \
               reactive_id %lu",
              msg.clientid(), msg.reactiveid());
        reply.set_status(REPLY_NOT_FOUND);
    } else {
        ReactiveTransaction *rt = transactions[frontendIndex];

        // unsubscribe to keys that no other reactive transaction is
        // listening to
        set<string> unsubscribeSet;
        for (const string &key : rt->keys) {
            if (listeners[key].size() == 1 &&
                listeners[key].count(frontendIndex)) {
                unsubscribeSet.insert(key);
            }
        }
        store->Unsubscribe(msg.reactiveid(),
                           unsubscribeSet,
                           [] (Promise &promise) { });

        // remove this reactive transaction from the listener set of
        // each key
        for (const string &key : rt->keys) {
            listeners[key].erase(frontendIndex);
        }

        // delete ReactiveTransaction object from transactions map
        transactions.erase(frontendIndex);
        delete rt;
        reply.set_status(REPLY_OK);
    }

    transport->SendMessage(this, remote, reply);
}

void
Server::HandleGet(const TransportAddress &remote,
                  const GetMessage &msg)
{
    pair<Timestamp,string> value;

    set<string> keys;

    for (int i = 0; i < msg.keys_size(); i++) {
	Debug("GET %s", msg.keys(i).c_str());
        keys.insert(msg.keys(i));
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
        store->MultiGet(msg.txnid(), keys, cb,
                        msg.timestamp());
    }
}
    
void
Server::GetCallback(const TransportAddress *remote,
		    const GetMessage msg,
		    Promise &promise)
{
    map<string, Version> values = promise.GetValues();
    GetReply reply;
    reply.set_status(promise.GetReply());
    reply.set_msgid(msg.msgid());
    if (promise.GetReply() == REPLY_OK) {
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
		       Promise &promise)
{
    CommitReply reply;
    reply.set_status(promise.GetReply());
    reply.set_txnid(msg.txnid());
    reply.set_msgid(msg.msgid());
    reply.set_timestamp(promise.GetTimestamp());
    transport->Timer(0, [=]() {
	    transport->SendMessage(this, *remote, reply);
	    delete remote;
	});
}

void
Server::HandleAbort(const TransportAddress &remote,
                    const AbortMessage &msg)
{
    store->Abort(msg.txnid(), [] (Promise &promise) { });

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
        fprintf(stderr,
		"unable to read configuration file: %s\n",
		configPath);
    }
    transport::Configuration config(configStream);

    TCPTransport transport(0.0, 0.0, 0);

    diamond::frontend::Server server =
	diamond::frontend::Server(backendConfig,
                                  maxShard,
                                  closestReplica,
                                  &transport);

    // The server's transport for handling incoming connections
    transport.Register(&server, config, 0);

    transport.Run();

    return 0;
}
