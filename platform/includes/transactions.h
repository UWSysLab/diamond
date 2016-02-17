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
typedef uint64_t txn_id;

typedef struct txn_info {
    txn_function_t func;
    txn_callback_t callback;
} txn_info_t;

class TxnManager {
  public:
    void Start();
    int ExecuteTxn(txn_function_t func, txn_callback_t callback);
    txn_id ReactiveTxn(txn_function_t func);

  private:
    event_base * txnEventBase;
    std::thread * txnThread;
    std::thread * reactiveThread;
    std::vector<txn_function_t> reactiveList;

    void startHelper();
    void reactiveLoop();
    static void executeTxnCallback(evutil_socket_t fd, short what, void * arg);
    static void signalCallback(evutil_socket_t fd, short what, void * arg);
};

TxnManager * txnManager = NULL;

void StartTxnManager();
int execute_txn(txn_function_t func, txn_callback_t callback);
int execute_txn(txn_function_t func);
txn_id reactive_txn(txn_function_t func);

void reactive_stop(txn_id id);
void abort_txn();

}

#endif
