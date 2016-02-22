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
    mode = msg.mode();
    timestamp = msg.timestamp();
    for (int i = 0; i < msg.readset_size(); i++) {
        ReadMessage readMsg = msg.readset(i);
        readSet[readMsg.key()] = readMsg.readtime();
    }

    for (int i = 0; i < msg.writeset_size(); i++) {
        WriteMessage writeMsg = msg.writeset(i);
        writeSet[writeMsg.key()] = writeMsg.value();
    }

    for (int i = 0; i < msg.incrementset_size(); i++) {
        IncrementMessage incMsg = msg.incrementset(i);
        incrementSet[incMsg.key()] = incMsg.inc();
    }
}

Transaction::~Transaction() { }

const unordered_map<string, Interval>&
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
                        const Interval &readVersion)
{
    readSet[key] = readVersion;
}

void
Transaction::ClearReadSet()
{
    readSet.clear();
}

void
Transaction::AddWriteSet(const string &key,
                         const string &value)
{
    writeSet[key] = value;
}

void
Transaction::AddIncrementSet(const string &key,
                             const int inc)
{
    incrementSet[key] = inc;
}

void
Transaction::Serialize(TransactionMessage *msg) const
{
    msg->set_mode(mode);
    msg->set_timestamp(timestamp);
    for (auto read : readSet) {
        ReadMessage *readMsg = msg->add_readset();
        readMsg->set_key(read.first);
        readMsg->set_readtime(read.second.Start());
        readMsg->set_end(read.second.End());
    }

    for (auto write : writeSet) {
        WriteMessage *writeMsg = msg->add_writeset();
        writeMsg->set_key(write.first);
        writeMsg->set_value(write.second);
    }

    for (auto inc : incrementSet) {
        IncrementMessage *incMsg = msg->add_incrementset();
        incMsg->set_key(inc.first);
        incMsg->set_inc(inc.second);
    }

}
