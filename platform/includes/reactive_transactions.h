#ifndef REACTIVE_TRANSACTIONS_H
#define REACTIVE_TRANSACTIONS_H

typedef uint64_t rtxn_id;
typedef struct rtxn_info {
    rtxn_id id;
    timestamp ts;
}

rtxn_info getNextPendingRtxn();
rtxn_id getNewId();
void registerRtxn(rtxn_id id, timestamp ts, const std::set<std::string> & readset);
void updateReadSet(rtxn_id id, timestamp ts, const std::set<std::string> & readset);
void removeRtxn(rtxn_id id);

// We need a method DObject::ReactiveCommit() that is like TransactionCommit()
// but returns a std::set<std::string> holding the read set

/*
 * This is the code that will go in each binding's TransactionManager
 */

std::set<txn_function_t> funcSet;
std::map<rtxn_id, txn_function_t> funcMap;
std::map<rtxn_id, std::set<std::string> > readsetMap;

void reactive_txn(txn_function_t func) {
    DObject::TransactionBegin(global_ts); // run transaction at time global_ts
    func();
    std::set<std::string> readset = DObject::ReactiveCommit();
    if (!funcSet.containsValue(func)) {
        rtxn_id id = getNewId();
        funcSet.insert(func);
        funcMap[id] = func;
        readsetMap[id] = readset;

        registerRtxn(id, global_ts, readset);
    }
}

// main reactive loop
void runReactiveLoop() {
    while(true) {
        rtxn_info info = getNextPendingRtxn();
        rtxn_id id = info.id;
        timestamp ts = info.ts;
        txn_function_t func = funcMap.get(id);
        DObject::TransactionBegin(ts); // run transaction at time ts
        func();
        std::set<std::string> readset = DObject::ReactiveCommit();
        oldReadset = readsetMap.get(id);
        if (readset != oldReadset) {
            updateReadSet(id, ts, readset);
        }
    }
}

#endif
