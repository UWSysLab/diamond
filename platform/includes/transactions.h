#ifndef TRANSACTIONS_H
#define TRANSACTIONS_H

#include "includes/data_types.h"
#include <cstdarg>
#include <functional>

namespace diamond {

    typedef std::function<void (void)> txn_function_t;

    void * execute_txn_helper(void * arg);
    int execute_txn(txn_function_t func);
}

#endif
