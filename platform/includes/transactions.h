#ifndef TRANSACTIONS_H
#define TRANSACTIONS_H

#include "includes/data_types.h"
#include <cstdarg>
#include <event2/event.h>
#include <event2/thread.h>
#include <functional>
#include <thread>

namespace diamond {

typedef std::function<void (void)> txn_function_t;
typedef std::function<void (int)> txn_callback_t;

typedef struct txn_info {
    txn_function_t func;
    txn_callback_t callback;
} txn_info_t;

typedef struct reg_info {
    txn_function_t func;
    uint64_t reactive_id;
} reg_info_t;

class TxnManager {
  public:
    void Start();
    int ExecuteTxn(txn_function_t func, txn_callback_t callback);
    uint64_t ReactiveTxn(txn_function_t func);
    void ReactiveStop(uint64_t reactive_id);

  private:
    event_base * txnEventBase;
    std::thread * txnThread;
    std::thread * reactiveThread;
    std::vector<txn_function_t> reactiveList;
    std::map<uint64_t, txn_function_t> funcMap;
    uint64_t nextId = 0;

    void startHelper();
    void reactiveLoop();
    uint64_t generateId();
    uint64_t getNextReactiveTxn();
    static void executeTxnCallback(evutil_socket_t fd, short what, void * arg);
    static void signalCallback(evutil_socket_t fd, short what, void * arg);
    static void registerCallback(evutil_socket_t fd, short what, void * arg);
    static void reactiveStopCallback(evutil_socket_t fd, short what, void * arg);
};

TxnManager * txnManager = NULL;

void StartTxnManager();
int execute_txn(txn_function_t func, txn_callback_t callback);
int execute_txn(txn_function_t func);
uint64_t reactive_txn(txn_function_t func);

void reactive_stop(uint64_t reactive_id);
void abort_txn();

}

#endif
