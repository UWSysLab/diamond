// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * store/strongstore/server.cc:
 *   Implementation of a single transactional key-value server with strong consistency.
 *
 * Copyright 2015 Irene Zhang <iyzhang@cs.washington.edu>
 *                Naveen Kr. Sharma <nks@cs.washington.edu>
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
#include "store/common/version.h"

#include <sstream>

using namespace std;

namespace strongstore {

using namespace proto;
TCPTransport transport(0.0, 0.0, 0);

Server::Server(const replication::ReplicaConfig &config,
               int index, int batchSize)
{
    store = new strongstore::PubStore();
    replica = new replication::VRReplica(config, index, &transport, batchSize, this);
}

Server::~Server()
{
    delete store;
    delete replica;
}
    
void
Server::LeaderUpcall(opnum_t opnum, const string &str1,
                     const TransportAddress &remote,
                     bool &replicate, string &str2)
{
    Debug("Received LeaderUpcall: %lu %s", opnum, str1.c_str());

    Request request;
    Reply reply;
    
    request.ParseFromString(str1);

    switch (request.op()) {
    case strongstore::proto::Request::GET:
        HandleGet(request, str2);
        replicate = false;
        break;
    case strongstore::proto::Request::PREPARE:
        replicate = HandlePrepare(request, str2);
        if (replicate) str2 = str1;
        break;
    case strongstore::proto::Request::COMMIT:
        HandleCommit(request);
        replicate = true;
        str2 = str1;
        break;
    case strongstore::proto::Request::ABORT:
        replicate = true;
        str2 = str1;
        break;
    case strongstore::proto::Request::SUBSCRIBE:
        HandleSubscribe((TCPTransportAddress &)remote,
                        request, str2);
        replicate = false;
        break;
    case strongstore::proto::Request::UNSUBSCRIBE:
        HandleUnsubscribe((TCPTransportAddress &)remote,
                          request, str2);
        replicate = false;
        break;
    case strongstore::proto::Request::ACK:
        HandleAck((const TCPTransportAddress &)remote,
                  request, str2);
        replicate = false;
        break;
    default:
        Panic("Unrecognized operation.");
    }
}

void
Server::HandleGet(const Request &request,
                  string &str2)
{
    Version val;
    int status;
    Reply reply;

    for (int i = 0; i < request.get().keys_size(); i++) {
        string key = request.get().keys(i);
        status = store->Get(request.txnid(), key, val,
                            request.get().timestamp());

        if (status == REPLY_OK) {
            ReadReply *rep = reply.add_replies();
            //ASSERT(val.GetInterval().End() != MAX_TIMESTAMP);

            rep->set_key(key);
            val.Serialize(rep);
        } else {
            reply.set_status(status);
        }
    }
    reply.set_status(REPLY_OK); 
    reply.SerializeToString(&str2);
}

bool
Server::HandlePrepare(const Request &request,
                      string &str2)
{
    Transaction txn = Transaction(request.prepare().txn());
    int status = store->Prepare(request.txnid(),
                txn);
    Reply reply;

    if (status == REPLY_OK) {
    return true;
    } else {
    reply.set_status(status);
    reply.SerializeToString(&str2);
    return false;
    }
}

void
Server::HandleCommit(const Request &request)
{
    if (request.commit().has_txn()) {
        store->Commit(request.txnid(),
                      request.commit().timestamp(),
                      Transaction(request.commit().txn()));
    } else {
        store->Commit(request.txnid(),
                      request.commit().timestamp());
    }
    // Add frontend notifications for this txn to the queue
    transport.Timer(0, [=]() {
            Publish(request.txnid(),
                    request.commit().timestamp());
    });
}

void
Server::Publish(const uint64_t tid,
                const Timestamp timestamp) {
    map<TCPTransportAddress, set<string>> notifications;
    store->Publish(tid,
                   timestamp,
                   notifications);

    for (auto &n : notifications) {
        Debug("Publishing to frontend %s at ts %lu notifications %lu",
              n.first.getHostname().c_str(), timestamp, notifications.size());
        PublishMessage msg;
        for (auto &v : n.second) {
            msg.add_keys(v);
        }
        msg.set_timestamp(timestamp);
        transport.Timer(0, [=]() {
                transport.SendMessage(replica,
                                      n.first,
                                      msg); 
            });

    }

    // if we still have notifications to send for
    // this transaction, then keep looping
    if (notifications.size() > 0) {
        transport.Timer(100, [=]() {
                Publish(tid, timestamp);
            });
    }


}

void
Server::HandleSubscribe(const TCPTransportAddress &remote,
                        const Request &request,
                        string &str2)
{
    Debug("Handling SUBSCRIBE");
    if (request.subscribe().keys_size() > 0) {
        set<string> keys;
    
        for (int i = 0; i < request.subscribe().keys_size(); i++) {
            Debug("%s", request.subscribe().keys(i).c_str());
            keys.insert(request.subscribe().keys(i));
        }

        store->Subscribe(remote, request.subscribe().timestamp(), keys);
    }

    Reply rep;
    rep.set_status(REPLY_OK);
    rep.SerializeToString(&str2);
}

void
Server::HandleUnsubscribe(const TCPTransportAddress &remote,
                          const Request &request,
                          string &str2)
{
    Debug("Handling UNSUBSCRIBE");
    if (request.unsubscribe().keys_size() > 0) {
    set<string> keys;
    
    for (int i = 0; i < request.unsubscribe().keys_size(); i++) {
        Debug("%s", request.unsubscribe().keys(i).c_str());
        keys.insert(request.unsubscribe().keys(i));
    }
    store->Unsubscribe(remote, keys);
    }

    Reply rep;
    rep.set_status(REPLY_OK);
    rep.SerializeToString(&str2);
}

void
Server::HandleAck(const TCPTransportAddress &remote,
                  const Request &request,
                  string &str2)
{
    if (request.ack().keys_size() > 0) {
    set<string> keys;
    
    for (int i = 0; i < request.ack().keys_size(); i++) {
        Debug("Received NOTIFY-FRONTEND-ACK \
                   from %s for key %s at ts %lu",
                  remote.getHostname().c_str(),
                  request.ack().keys(i).c_str(),
                  request.ack().timestamp());
        keys.insert(request.ack().keys(i));
    }
    store->Subscribe(remote, request.ack().timestamp(), keys);
    }

    Reply rep;
    rep.set_status(REPLY_OK);
    rep.SerializeToString(&str2);
}

    

    
/* Gets called when a command is issued using client.Invoke(...) to this
 * replica group. 
 * opnum is the operation number.
 * str1 is the request string passed by the client.
 * str2 is the reply which will be sent back to the client.
 */
void
Server::ReplicaUpcall(opnum_t opnum,
                      const string &str1,
                      string &str2)
{
    Debug("Received Upcall: %lu %s", opnum, str1.c_str());
    Request request;
    Reply reply;
    int status = REPLY_OK;
    
    request.ParseFromString(str1);

    switch (request.op()) {
    case strongstore::proto::Request::GET:
        return;
    case strongstore::proto::Request::PREPARE:
        // get a prepare timestamp and return to client
        store->Prepare(request.txnid(), Transaction(request.prepare().txn()));
        break;
    case strongstore::proto::Request::COMMIT:
    if (request.commit().has_txn()) {
        store->Commit(request.txnid(),
              request.commit().timestamp(),
              Transaction(request.commit().txn()));
    } else {
        store->Commit(request.txnid(),
              request.commit().timestamp());
    }
        reply.set_timestamp(request.commit().timestamp());
        break;
    case strongstore::proto::Request::ABORT:
        store->Abort(request.txnid());
        break;
    case strongstore::proto::Request::SUBSCRIBE:
        break;
    case strongstore::proto::Request::UNSUBSCRIBE:
        break;
    default:
        Panic("Unrecognized operation.");
    }
    reply.set_status(status);
    reply.SerializeToString(&str2);
}

void
Server::UnloggedUpcall(const string &str1,
                       string &str2)
{
    Request request;
    Reply reply;
    request.ParseFromString(str1);

    ASSERT(request.op() == strongstore::proto::Request::GET);

    HandleGet(request, str2);
 }

void
Server::Load(const string &key,
             const string &value,
             const Timestamp timestamp)
{
    store->Load(key, value, timestamp);
}

void
Server::Run()
{
    transport.Run();
}
    
} // namespace strongstore

int
main(int argc, char **argv)
{
    int index = -1;
    unsigned int myShard=0, maxShard=1, nKeys=1;
    const char *configPath = NULL;
    const char *keyPath = NULL;
    unsigned int batchSize = 256;

    // Parse arguments
    int opt;
    while ((opt = getopt(argc, argv, "c:i:m:e:s:f:n:N:k:B:")) != -1) {
        switch (opt) {
        case 'c':
            configPath = optarg;
            break;
            
        case 'i':
        {
            char *strtolPtr;
            index = strtoul(optarg, &strtolPtr, 10);
            if ((*optarg == '\0') || (*strtolPtr != '\0') || (index < 0))
            {
                fprintf(stderr, "option -i requires a numeric arg\n");
            }
            break;
        }
        
        case 'k':
        {
            char *strtolPtr;
            nKeys = strtoul(optarg, &strtolPtr, 10);
            if ((*optarg == '\0') || (*strtolPtr != '\0'))
            {
                fprintf(stderr, "option -e requires a numeric arg\n");
            }
            break;
        }

        case 'n':
        {
            char *strtolPtr;
            myShard = strtoul(optarg, &strtolPtr, 10);
            if ((*optarg == '\0') || (*strtolPtr != '\0'))
            {
                fprintf(stderr, "option -e requires a numeric arg\n");
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

        case 'B':
        {
            char *strtolPtr;
            batchSize = strtoul(optarg, &strtolPtr, 10);
            if ((*optarg == '\0') || (*strtolPtr != '\0') || (batchSize <= 0))
            {
                fprintf(stderr, "option -B requires a numeric arg\n");
            }
            break;
        }

        case 'f':   // Load keys from file
        {
            keyPath = optarg;
            break;
        }

        default:
            fprintf(stderr, "Unknown argument %s\n", argv[optind]);
        }


    }

    if (!configPath) {
        fprintf(stderr, "option -c is required\n");
    }

    if (index == -1) {
        fprintf(stderr, "option -i is required\n");
    }

    // Load configuration
    std::ifstream configStream(configPath);
    if (configStream.fail()) {
        fprintf(stderr, "unable to read configuration file: %s\n", configPath);
    }
    replication::ReplicaConfig config(configStream);

    if (index >= config.n) {
        fprintf(stderr, "replica index %d is out of bounds; "
                "only %d replicas defined\n", index, config.n);
    }

    strongstore::Server server(config, index, batchSize);
    
    if (keyPath) {
        string key;
        ifstream in;
        in.open(keyPath);
        if (!in) {
            fprintf(stderr, "Could not read keys from: %s\n", keyPath);
            exit(0);
        }

        for (unsigned int i = 0; i < nKeys; i++) {
            getline(in, key);
            
            uint64_t hash = 5381;
            const char* str = key.c_str();
            for (unsigned int j = 0; j < key.length(); j++) {
                hash = ((hash << 5) + hash) + (uint64_t)str[j];
            }

            if (hash % maxShard == myShard) {
                server.Load(key, "null", Timestamp());
            }
        }
        in.close();
    }

    server.Run();
    return 0;
}
