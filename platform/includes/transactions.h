#ifndef TRANSACTIONS_H
#define TRANSACTIONS_H

#include "includes/data_types.h"
#include <cstdarg>
#include <functional>
#include <event2/event.h>
#include <thread>

namespace diamond {

typedef std::function<void (void)> txn_function_t;

event_base * txnEventBase;
std::thread * txnThread;

void startTxnManager();
void startTxnManagerHelper();
int execute_txn(txn_function_t func);

}

#endif
