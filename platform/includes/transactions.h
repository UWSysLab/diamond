#ifndef TRANSACTIONS_H
#define TRANSACTIONS_H

#include "includes/data_types.h"
#include <cstdarg>
#include <functional>
#include <event2/event.h>
#include <event2/thread.h>
#include <thread>

namespace diamond {

typedef std::function<void (void)> txn_function_t;

typedef struct txn_info {
    txn_function_t func;
} txn_info_t;

class TxnManager {
  public:
    void Start();
    int ExecuteTxn(txn_function_t func);

  private:
    txn_function_t globalFunc;
    event_base * txnEventBase;
    std::thread * txnThread;

    void startHelper();
    static void executeTxnCallback(evutil_socket_t fd, short what, void * arg);
    static void signalCallback(evutil_socket_t fd, short what, void * arg);
};

TxnManager * txnManager = NULL;

void StartTxnManager();
int execute_txn(txn_function_t func);

}

#endif
