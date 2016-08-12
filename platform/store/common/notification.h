#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <limits.h>
#include <set>
#include <string>

#include "version.h"
#include "timestamp.h"
#include "lib/tcptransport.h"

#define NO_NOTIFICATION ULLONG_MAX

class ReactiveTransaction {
 public:
   ReactiveTransaction() { };
   ~ReactiveTransaction() { delete client; };
   
    uint64_t frontend_index; // ((client_id << 32) | reactive_id)
    uint64_t reactive_id;
    uint64_t client_id;
    Timestamp next_timestamp;
    Timestamp last_timestamp;
    std::set<std::string> keys;
    TransportAddress *client;
};

#endif //NOTIFICATION_H
