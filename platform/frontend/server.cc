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

    if (type == get.GetTypeName()) {
        get.ParseFromString(data);
        HandleGet(remote, get);
    } else if (type == commit.GetTypeName()) {
        commit.ParseFromString(data);
        HandleCommit(remote, commit);
    } else if (type == abort.GetTypeName()) {
        abort.ParseFromString(data);
        HandleAbort(remote, abort);
    } else {
        Panic("Received unexpected message type in OR proto: %s",
              type.c_str());
    }
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
        if (msg.has_timestamp()) {
            status = store->MultiGet(msg.txnid(), keys, msg.timestamp(), values);
        } else {
            status = store->MultiGet(msg.txnid(), keys, values);
        }
    } else {
        Version v;
        if (msg.has_timestamp()) {
            status = store->Get(msg.txnid(), keys[0], msg.timestamp(), v);
        } else {
            status = store->Get(msg.txnid(), keys[0], v);
        }
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
    bool ret = store->Commit(msg.txnid(), Transaction(msg.txn()), ts);

    CommitReply reply;
    if (ret) {
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
    store->Abort(msg.txnid(), Transaction(msg.txn()));

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
