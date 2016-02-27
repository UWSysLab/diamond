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
    } else {
        Panic("Received unexpected message type in OR proto: %s",
              type.c_str());
    }
}

void
Server::HandleNotifyFrontend(const TransportAddress &remote,
                             const NotifyFrontendMessage &msg) {
    Debug("Handling NOTIFY-FRONTEND");
    // For every key, find all the reactive transactions listening to it,
    // and update the next timestamps to run them at
    for (int i = 0; i < msg.replies_size(); i++) {
        std::string key = msg.replies(i).key();
        Timestamp timestamp = msg.replies(i).timestamp();
        for (auto it = listeners[key].begin(); it != listeners[key].end(); it++) {
            transactions[*it].next_timestamp = timestamp;
        }
    }

    // Send notifications for each reactive transaction that has one pending
    for (auto it = transactions.begin(); it != transactions.end(); it++) {
        ReactiveTransaction rt = it->second;
        Debug("Sending NOTIFICATION: reactive_id %lu, client %s:%s", rt.reactive_id, rt.client_hostname.c_str(), rt.client_port.c_str());
        if (rt.next_timestamp != rt.last_timestamp) {
            Notification notification;
            notification.set_clientid(rt.client_id);
            notification.set_reactiveid(rt.reactive_id);
            notification.set_timestamp(rt.next_timestamp);
            transport->SendMessage(this, rt.client_hostname, rt.client_port, notification);
        }
    }
}

void
Server::HandleRegister(const TransportAddress &remote,
                       const RegisterMessage &msg)
{
    std::set<std::string> regSet;
    for (int i = 0; i < msg.keys_size(); i++) {
        Debug("REGISTER %s", msg.keys(i).c_str());
        regSet.insert(msg.keys(i));
    }
    Timestamp timestamp;
    int status = store->Subscribe(regSet, GetAddress(), timestamp);

    ReactiveTransaction rt;
    rt.reactive_id = msg.reactiveid();
    rt.client_id = msg.clientid();
    rt.last_timestamp = msg.timestamp();
    rt.next_timestamp = timestamp;
    rt.keys = regSet;
    rt.client_hostname = remote.getHostname();
    rt.client_port = remote.getPort();

    transactions[rt.reactive_id] = rt;

    for (auto it = rt.keys.begin(); it != rt.keys.end(); it++) {
        listeners[*it].insert(rt.reactive_id);
    }

    RegisterReply reply;
    reply.set_status(status);
    reply.set_msgid(msg.msgid());
    transport->SendMessage(this, remote, reply);
}

void
Server::HandleGet(const TransportAddress &remote,
                  const GetMessage &msg)
{
    pair<Timestamp,string> value;
    int status;

    vector<string> keys;
    map<string, Version> values;

    for (int i = 0; i < msg.keys_size(); i++) {
	Debug("GET %s", msg.keys(i).c_str());
        keys.push_back(msg.keys(i));
    }

    if (keys.size() > 1) {
        status = store->MultiGet(msg.txnid(), keys, values, msg.timestamp());
    } else {
        Version v;
        status = store->Get(msg.txnid(), keys[0], v, msg.timestamp());
        values[keys[0]] = v;
    }

    GetReply reply;
    reply.set_status(status);
    reply.set_msgid(msg.msgid());
    if (status == REPLY_OK) {
	for (auto value : values) {
	    ReadReply *rep = reply.add_replies();
	    rep->set_key(value.first);
	    value.second.Serialize(rep);
        }
    }

    transport->SendMessage(this, remote, reply);
}

void
Server::HandleCommit(const TransportAddress &remote,
                     const CommitMessage &msg)
{
    Timestamp ts;
    CommitReply reply;
    Transaction txn(msg.txn());
    if (store->Commit(msg.txnid(), txn, ts)) {
        reply.set_status(REPLY_OK);
    } else {
        reply.set_status(REPLY_FAIL);
    }
    reply.set_txnid(msg.txnid());
    reply.set_msgid(msg.msgid());
    reply.set_timestamp(ts);
    transport->SendMessage(this, remote, reply);
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
