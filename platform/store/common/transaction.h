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
#include "timestamp.h"
#include "common-proto.pb.h"

#include <unordered_map>

class Transaction {
private:
    Timestamp timestamp = MAX_TIMESTAMP;
    bool readOnly = false;
    
    // map between key and timestamp at
    // which the read happened and how
    // many times this key has been read
    std::unordered_map<std::string, Timestamp> readSet;

    // map between key and value(s)
    std::unordered_map<std::string, std::string> writeSet;

public:
    Transaction() {};
    Transaction(const Timestamp timestamp) : timestamp(timestamp) {};
    Transaction(const Timestamp timestamp, bool readOnly) : timestamp(timestamp), readOnly(readOnly) {};
    Transaction(const TransactionMessage &msg);
    ~Transaction();

    const Timestamp GetTimestamp() const { return timestamp; };
    void SetTimestamp(const Timestamp &ts) { timestamp = ts; };
    const bool IsReadOnly() const { return readOnly; }
    const bool HasTimestamp() const { return timestamp < MAX_TIMESTAMP; };
    const std::unordered_map<std::string, Timestamp>& GetReadSet() const;
    const std::unordered_map<std::string, std::string>& GetWriteSet() const;
    
    void AddReadSet(const std::string &key, const Timestamp &readTime);
    void AddWriteSet(const std::string &key, const std::string &value);
    void Serialize(TransactionMessage *msg) const;
};

#endif /* _TRANSACTION_H_ */
