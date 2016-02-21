#ifndef REACTIVE_TRANSACTIONS_H
#define REACTIVE_TRANSACTIONS_H

typedef uint64_t rtxn_id;
typedef struct rtxn_info {
    rtxn_id id;
    timestamp ts;
}

rtxn_info getPendingReactiveTxn();
rtxn_id generateId();
void removeReactiveTxn(rtxn_id id);

// no longer visible to language bindings
void registerReactiveTxn(rtxn_id id, timestamp ts, const std::set<std::string> & readset);
void updateReadSet(rtxn_id id, timestamp ts, const std::set<std::string> & readset);

/*
 * This is the code that will go in each binding's TransactionManager
 */

std::set<txn_function_t> funcSet;
std::map<rtxn_id, txn_function_t> funcMap;

rtxn_id reactive_txn(txn_function_t func) {
    rtxn_id id = generateId();
    DObject::ReactiveBegin(id); // run transaction at time of last reactive txn 
    func();
    DObject::TransactionCommit(); // check for and initiate registration in here
    if (!funcSet.containsValue(func)) {
        funcSet.insert(func);
        funcMap[id] = func;
    }
    return id;
}

// main reactive loop
void runReactiveLoop() {
    while(true) {
        rtxn_id id = getPendingReactiveTxn(); // keep id<->ts map in diamondclient
        txn_function_t func = funcMap.get(id);
        DObject::ReactiveBegin(id); // diamondclient will run transaction at time ts
        func();
        DObject::TransactionCommit(); // check for and initiate read-set-change in here
    }
}

#endif
