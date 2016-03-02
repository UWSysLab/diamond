#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <unordered_map>
#include <set>
#include <string>

#include "version.h"
#include "timestamp.h"
#include "lib/tcptransport.h"

class FrontendNotification {
public:
    std::string address;
    std::unordered_map<std::string, Version> values;
    // Timestamps will be filled in VersionedKVStore::GetFrontendNotifications()
    // The rest of the Version objects will be filled in OCCStore::fillCacheEntries()
};

class ReactiveTransaction {
public:
    uint64_t frontend_index; // ((client_id << 32) | reactive_id)
    uint64_t reactive_id;
    uint64_t client_id;
    Timestamp next_timestamp;
    Timestamp last_timestamp;
    std::set<std::string> keys;
    std::unordered_map<std::string, Version> values; // cached values
    std::string client_hostname;
    std::string client_port;
};

#endif //NOTIFICATION_H
