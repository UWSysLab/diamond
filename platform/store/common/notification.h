#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <map>
#include <set>
#include <string>

#include "timestamp.h"
#include "lib/tcptransport.h"

class FrontendNotification {
public:
    std::string address;
    std::map<std::string, Timestamp> timestamps;
};

class ReactiveTransaction {
public:
    uint64_t reactive_id;
    uint64_t client_id;
    Timestamp next_timestamp;
    Timestamp last_timestamp;
    std::set<std::string> keys;
    std::string client_hostname;
    std::string client_port;
};

#endif //NOTIFICATION_H
