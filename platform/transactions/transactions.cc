#include "includes/transactions.h"

#include <pthread.h>

namespace diamond {
    void * execute_txn_helper(void * arg) {
        DObject::TransactionBegin();
        txn_function_t * funcPtr = (txn_function_t *)arg;
        txn_function_t func = *funcPtr;
        func();
        DObject::TransactionCommit();
        delete funcPtr;
        return NULL;
    }

    int execute_txn(txn_function_t func) {
        pthread_t thread;
        txn_function_t * funcPtr = new txn_function_t();
        pthread_create(&thread, NULL, execute_txn_helper, (void *)funcPtr);
        return 0; // what should we return?
    }
}
