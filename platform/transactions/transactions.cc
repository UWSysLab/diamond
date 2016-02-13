#include "includes/transactions.h"

#include <signal.h>
#include "lib/message.h"

namespace diamond {
    void StartTxnManager() {
        evthread_use_pthreads();
        txnEventBase = event_base_new();
        evthread_make_base_notifiable(txnEventBase);

        event * sigtermEvent = evsignal_new(txnEventBase, SIGTERM, signalCallback, NULL);
        event * sigintEvent = evsignal_new(txnEventBase, SIGINT, signalCallback, NULL);

        evsignal_add(sigtermEvent, NULL);
        evsignal_add(sigintEvent, NULL);

        txnThread = new std::thread(&StartTxnManagerHelper);
    }

    void StartTxnManagerHelper() {
        printf("about to call event_base_dispatch\n");
        event_base_dispatch(txnEventBase);
        printf("event_base_dispatch returned\n");
    }

    void signalCallback(evutil_socket_t fd, short what, void * arg) {
        Debug("Transaction manager terminating on SIGTERM/SIGINT");
        event_base_loopbreak(txnEventBase);
    }

    int execute_txn(txn_function_t func) {
        printf("execute_txn called\n");
        //function_holder_t * funcHolder = new function_holder_t();
        //event * ev = event_new(txnEventBase, -1, 0, execute_txn_callback, funcHolder);
        globalFunc = func;
        event * ev = event_new(txnEventBase, -1, 0, execute_txn_callback, NULL);
        event_add(ev, NULL);
        event_active(ev, 0, 1);
        return 0;
        //int result = event_base_once(txnEventBase, -1, 0, execute_txn_callback, funcHolder, NULL);
        //return result;
    }

    void execute_txn_callback(evutil_socket_t fd, short what, void * arg) {
        printf("callback called\n");
        //function_holder_t * funcHolder = (function_holder_t *)arg;
        //funcHolder->func();
        //delete funcHolder;
        globalFunc();
    }
}
