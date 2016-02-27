#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <map>
#include <string>

class FrontendNotification {
public:
    std::string address;
    std::map<std::string, Timestamp> timestamps;
};

#endif //NOTIFICATION_H
