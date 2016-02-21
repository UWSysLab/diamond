#include "includes/transactions.h"

#include <signal.h>
#include "lib/message.h"

namespace diamond {
    void TxnManager::Start() {
        evthread_use_pthreads();
        txnEventBase = event_base_new();
        evthread_make_base_notifiable(txnEventBase);

        event * sigtermEvent = evsignal_new(txnEventBase, SIGTERM, signalCallback, this);
        event * sigintEvent = evsignal_new(txnEventBase, SIGINT, signalCallback, this);

        evsignal_add(sigtermEvent, NULL);
        evsignal_add(sigintEvent, NULL);

        txnThread = new std::thread(&TxnManager::startHelper, this);
        reactiveThread = new std::thread(&TxnManager::reactiveLoop, this);
    }

    uint64_t TxnManager::getNextReactiveTxn() {
        while(true) {
            sleep(1);
        }
        return 0;
    }

    void TxnManager::reactiveLoop() {
        while(true) {
            uint64_t reactive_id = getNextReactiveTxn();
            txn_function_t func = funcMap[reactive_id];
            DObject::BeginReactive(reactive_id);
            func();
            DObject::TransactionCommit();
        }
    }

    void TxnManager::startHelper() {
        event_base_dispatch(txnEventBase);
    }

    void TxnManager::signalCallback(evutil_socket_t fd, short what, void * arg) {
        Debug("Transaction manager terminating on SIGTERM/SIGINT");
        TxnManager * txnManager = (TxnManager *)arg;
        event_base_loopbreak(txnManager->txnEventBase);
    }

    int TxnManager::ExecuteTxn(txn_function_t func, txn_callback_t callback) {
        txn_info_t * info = new txn_info_t();
        info->func = func;
        info->callback = callback;
        event * ev = event_new(txnEventBase, -1, 0, executeTxnCallback, info);
        event_add(ev, NULL);
        event_active(ev, 0, 1);
        return 0;
    }

    void TxnManager::executeTxnCallback(evutil_socket_t fd, short what, void * arg) {
        txn_info_t * info = (txn_info_t *)arg;
        DObject::TransactionBegin();
        info->func();
        int committed = DObject::TransactionCommit();
        info->callback(committed);
        delete info;
    }

    //TODO: figure out how to handle duplicate registrations of the same function in C++
    //TODO: recovery?
    uint64_t TxnManager::ReactiveTxn(txn_function_t func) {
        uint64_t reactive_id = generateId();
        funcMap[reactive_id] = func;
        reactiveList.push_back(func);

        reg_info_t * info = new reg_info_t();
        info->func = func;
        info->reactive_id = reactive_id;
        event * ev = event_new(txnEventBase, -1, 0, registerCallback, info);
        event_add(ev, NULL);
        event_active(ev, 0, 1);

        return reactive_id;
    }

    void TxnManager::registerCallback(evutil_socket_t fd, short what, void * arg) {
        reg_info_t * info = (reg_info_t *)arg;
        DObject::BeginReactive(info->reactive_id);
        info->func();
        int committed = DObject::TransactionCommit();
        if (!committed) {
            Panic("Reactive transaction did not commit");
        }
        delete info;
    }

    void StartTxnManager() {
        if (txnManager == NULL) {
            txnManager = new TxnManager();
        }
        txnManager->Start();
    }

    int execute_txn(txn_function_t func) {
        int ret = execute_txn(func, [] (int committed) {});
        return ret;
    }

    int execute_txn(txn_function_t func, txn_callback_t callback) {
        if (txnManager == NULL) {
            Panic("txnManager is null"); 
        }
        int ret = txnManager->ExecuteTxn(func, callback);
        return ret;
    }

    uint64_t reactive_txn(txn_function_t func) {
        if (txnManager == NULL) {
            Panic("txnManager is null"); 
        }
        uint64_t reactive_id = txnManager->ReactiveTxn(func);
        return reactive_id;
    }

    void reactive_stop(uint64_t reactive_id) {
        //TODO: implement
    }
    void abort_txn() {
        //TODO: implement
    }

    uint64_t TxnManager::generateId() {
        uint64_t result = nextId;
        nextId++;
        return result;
    }
}
