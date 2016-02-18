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

rtxn_id reactive_txn(txn_function_t func) {
    rtxn_id id = generateId();
    DObject::TransactionBegin(global_ts); // run transaction at time global_ts
    func();
    std::set<std::string> readset = DObject::ReactiveCommit();
    if (!funcSet.containsValue(func)) {
        funcSet.insert(func);
        funcMap[id] = func;
        readsetMap[id] = readset;

        registerRtxn(id, global_ts, readset);
    }
    return id;
}

// main reactive loop
void runReactiveLoop() {
    while(true) {
        rtxn_id id = getNextPendingRtxn(); // keep id<->ts map in diamondclient
        txn_function_t func = funcMap.get(id);
        DObject::ReactiveBegin(id, ts); // run transaction at time ts
        func();
        DObject::ReactiveCommit(id); // check for and initiate read-set-change in here
    }
}

#endif
