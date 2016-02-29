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

Server::Server()
    : transport()
{
    store = new strongstore::OCCStore();

    transportThread = new thread(&Server::runTransport, this);
}

Server::~Server()
{
    delete store;
}

void
Server::runTransport() {
    transport.Run();
}

void
Server::ExecuteGet(Request request, string &str2)
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
	    rep->set_key(key);
            val.Serialize(rep);
        } else {
	    reply.set_status(status);
	    reply.SerializeToString(&str2);
	    return;
	}
    }
    reply.set_status(REPLY_OK);	
    reply.SerializeToString(&str2);
}
    
void
Server::LeaderUpcall(opnum_t opnum, const string &str1, bool &replicate, string &str2)
{
    Debug("Received LeaderUpcall: %lu %s", opnum, str1.c_str());

    Request request;
    Reply reply;
    int status;
    
    request.ParseFromString(str1);

    switch (request.op()) {
    case strongstore::proto::Request::GET:
        ExecuteGet(request, str2);
        replicate = false;
        break;
    case strongstore::proto::Request::PREPARE:
        // Prepare is the only case that is conditionally run at the leader
        status = store->Prepare(request.txnid(), Transaction(request.prepare().txn()));

        // if prepared, then replicate result
        if (status == 0) {
            replicate = true;
            // get a prepare timestamp and send along to replicas
            request.SerializeToString(&str2);
        } else {
            // if abort, don't replicate
            replicate = false;
            reply.set_status(status);
            reply.SerializeToString(&str2);
        }
        break;
    case strongstore::proto::Request::COMMIT:
	if (request.commit().has_txn()) {
            sendNotifications(store->GetFrontendNotifications(request.commit().timestamp(), Transaction(request.commit().txn())));
	} else {
            sendNotifications(store->GetFrontendNotifications(request.commit().timestamp(), request.txnid()));
	}
        replicate = true;
        str2 = str1;
        break;
    case strongstore::proto::Request::ABORT:
        replicate = true;
        str2 = str1;
        break;
    case strongstore::proto::Request::SUBSCRIBE:
        replicate = true;
        str2 = str1;
        break;
    default:
        Panic("Unrecognized operation.");
    }
}

void Server::sendNotifications(const vector<FrontendNotification> &notifications) {
    for (auto it = notifications.begin(); it != notifications.end(); it++) {
        FrontendNotification notification = *it;
        Debug("Sending NOTIFY-FRONTEND to frontend %s:", it->address.c_str());
        NotifyFrontendMessage msg;
        for (auto it2 = notification.timestamps.begin(); it2 != notification.timestamps.end(); it2++) {
            string key = it2->first;
            Timestamp timestamp = it2->second;
            Debug("%s, %lu", key.c_str(), timestamp);
            ReadReply * reply = msg.add_replies();
            reply->set_key(key);
            reply->set_timestamp(timestamp);

            //TODO: more legit values?
            reply->set_value("");
            reply->set_end(0);
            reply->set_op(0);
        }
        stringstream ss(it->address);
        string hostname;
        getline(ss, hostname, ':');
        string port;
        getline(ss, port, ':');
        transport.SendMessage(NULL, hostname, port, msg);
    }
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
	    store->Commit(request.txnid(), request.commit().timestamp(), Transaction(request.commit().txn()));
	} else {
	    store->Commit(request.txnid(), request.commit().timestamp());
	}
        reply.set_timestamp(request.commit().timestamp());
        break;
    case strongstore::proto::Request::ABORT:
        store->Abort(request.txnid());
        break;
    case strongstore::proto::Request::SUBSCRIBE:
        {
            Debug("Handling SUBSCRIBE");
            string address = request.subscribe().address(); //TODO: make sure this is how we want to pass around addresses
            set<string> keys;
            for (int i = 0; i < request.subscribe().keys_size(); i++) {
                Debug("%s", request.subscribe().keys(i).c_str());
                keys.insert(request.subscribe().keys(i));
            }
            Timestamp timestamp = store->Subscribe(keys, address);
            reply.set_timestamp(timestamp);
        }
        break;
    default:
        Panic("Unrecognized operation.");
    }
    reply.set_status(status);
    reply.SerializeToString(&str2);
}

void
Server::UnloggedUpcall(const string &str1, string &str2)
{
    Request request;
    Reply reply;
    request.ParseFromString(str1);

    ASSERT(request.op() == strongstore::proto::Request::GET);

    ExecuteGet(request, str2);
 }

void
Server::Load(const string &key, const string &value, const Timestamp timestamp)
{
    store->Load(key, value, timestamp);
}

} // namespace strongstore

int
main(int argc, char **argv)
{
    int index = -1;
    unsigned int myShard=0, maxShard=1, nKeys=1;
    const char *configPath = NULL;
    const char *keyPath = NULL;

    // Parse arguments
    int opt;
    while ((opt = getopt(argc, argv, "c:i:m:e:s:f:n:N:k:")) != -1) {
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
    transport::Configuration config(configStream);

    if (index >= config.n) {
        fprintf(stderr, "replica index %d is out of bounds; "
                "only %d replicas defined\n", index, config.n);
    }

    TCPTransport transport(0.0, 0.0, 0);

    //strongstore::Server server = strongstore::Server();
    strongstore::Server server;
    replication::VRReplica replica(config, index, &transport, 1, &server);
    
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

    transport.Run();

    return 0;
}
