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
#include <set>

#define LINEARIZABLE 0
#define SNAPSHOT_ISOLATION 1
#define EVENTUAL 2
#define READ_ONLY 3

class Transaction {
public:
    Transaction() : mode(LINEARIZABLE) { };
    Transaction(int mode) : mode(mode) { };
    Transaction(int mode, const Timestamp timestamp) : mode(mode), timestamp(timestamp) {};
    Transaction(int mode, const Timestamp timestamp, uint64_t reactive_id) : mode(mode), timestamp(timestamp), reactive_id(reactive_id), reactive(true) {};
    Transaction(const TransactionMessage &msg);
    ~Transaction();

    const Timestamp GetTimestamp() const { return timestamp; };
    void SetTimestamp(const Timestamp &ts) { timestamp = ts; };
    const bool IsReadOnly() const { return mode == READ_ONLY; };
    const int IsolationMode() const { return mode; };
    const bool HasTimestamp() const { return timestamp < MAX_TIMESTAMP; };
    const bool IsReactive() const { return reactive; };
    const uint64_t GetReactiveId() const { return reactive_id; };
    const std::unordered_map<std::string, Interval>& GetReadSet() const;
    const std::unordered_map<std::string, std::string>& GetWriteSet() const;
    const std::set<std::string>& GetRegSet() const;
    const std::unordered_map<std::string, int>& GetIncrementSet() const;
    
    void AddReadSet(const std::string &key, const Interval &readVersion);
    void ClearReadSet();
    void AddWriteSet(const std::string &key, const std::string &value);
    void AddRegSet(const std::string &key);
    void AddIncrementSet(const std::string &key, const int inc);
    void Serialize(TransactionMessage *msg) const;
private:
    int mode;
    Timestamp timestamp = MAX_TIMESTAMP;
    uint64_t reactive_id = 0;
    bool reactive = false;
    
    // map between key and timestamp at
    // which the read happened and how
    // many times this key has been read
    std::unordered_map<std::string, Interval> readSet;

    // map between key and value(s)
    std::unordered_map<std::string, std::string> writeSet;

    // set of keys to register for reactive transactions
    std::set<std::string> regSet;

    // increment set
    std::unordered_map<std::string, int> incrementSet;
};

#endif /* _TRANSACTION_H_ */
