// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * common/transaction.cc
 *   A transaction implementation.
 *
 **********************************************************************/

#include "transaction.h"

using namespace std;

Transaction::Transaction(const TransactionMessage &msg) 
{
    timestamp = msg.timestamp();
    for (int i = 0; i < msg.readset_size(); i++) {
        ReadMessage readMsg = msg.readset(i);
        readSet[readMsg.key()] = readMsg.readtime();
    }

    for (int i = 0; i < msg.writeset_size(); i++) {
        WriteMessage writeMsg = msg.writeset(i);
        writeSet[writeMsg.key()] = writeMsg.value();
    }
}

Transaction::~Transaction() { }

const unordered_map<string, Timestamp>&
Transaction::GetReadSet() const
{
    return readSet;
}

const unordered_map<string, string>&
Transaction::GetWriteSet() const
{
    return writeSet;
}

void
Transaction::AddReadSet(const string &key,
                        const Timestamp &readTime)
{
    readSet[key] = readTime;
}

void
Transaction::AddWriteSet(const string &key,
                         const string &value)
{
    writeSet[key] = value;
}

void
Transaction::Serialize(TransactionMessage *msg) const
{
    msg->set_timestamp(timestamp);
    for (auto read : readSet) {
        ReadMessage *readMsg = msg->add_readset();
        readMsg->set_key(read.first);
        readMsg->set_readtime(read.second);
    }

    for (auto write : writeSet) {
        WriteMessage *writeMsg = msg->add_writeset();
        writeMsg->set_key(write.first);
        writeMsg->set_value(write.second);
    }
}
