// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * common/transaction.h:
 *   A Transaction representation.
 *
 **********************************************************************/

#ifndef _TRANSACTION_H_
#define _TRANSACTION_H_

#include "lib/assert.h"
#include "lib/message.h"
#include "interval.h"
#include "common-proto.pb.h"

#include <unordered_map>

#define LINEARIZABLE 0
#define SNAPSHOT_ISOLATION 1
#define EVENTUAL 2
#define READ_ONLY 3

class Transaction {
public:
    Transaction() {};
    Transaction(int mode) : mode(mode) { };
    Transaction(int mode, const Timestamp timestamp) : mode(mode), timestamp(timestamp) {};
    Transaction(const TransactionMessage &msg);
    ~Transaction();

    const Timestamp GetTimestamp() const { return timestamp; };
    void SetTimestamp(const Timestamp &ts) { timestamp = ts; };
    const bool IsReadOnly() const { return mode == READ_ONLY; };
    const int IsolationMode() const { return mode; };
    const bool HasTimestamp() const { return timestamp < MAX_TIMESTAMP; };
    const std::unordered_map<std::string, Interval>& GetReadSet() const;
    const std::unordered_map<std::string, std::string>& GetWriteSet() const;
    
    void AddReadSet(const std::string &key, const Interval &readVersion);
    void AddWriteSet(const std::string &key, const std::string &value);
    void Serialize(TransactionMessage *msg) const;
private:
    int mode = LINEARIZABLE;
    Timestamp timestamp = MAX_TIMESTAMP;
    
    // map between key and timestamp at
    // which the read happened and how
    // many times this key has been read
    std::unordered_map<std::string, Interval> readSet;

    // map between key and value(s)
    std::unordered_map<std::string, std::string> writeSet;


};

#endif /* _TRANSACTION_H_ */
