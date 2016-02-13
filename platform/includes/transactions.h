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
typedef struct function_holder { txn_function_t func; } function_holder_t;

txn_function_t globalFunc;
event_base * txnEventBase;
std::thread * txnThread;

void StartTxnManager();
void StartTxnManagerHelper();
int execute_txn(txn_function_t func);
void execute_txn_callback(evutil_socket_t fd, short what, void * arg);
void signalCallback(evutil_socket_t fd, short what, void * arg);

}

#endif
