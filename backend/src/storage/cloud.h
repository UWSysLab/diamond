// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * storage/cloud.h:
 *   Diamond logic for accessing cloud storage
 *
 **********************************************************************/

#ifndef _CLOUD_H_
#define _CLOUD_H_

#define ERR_OK 0
#define ERR_UNAVAILABLE 1
#define ERR_NETWORK 2
#define ERR_LOCAL 3

#include <string>
#include "hiredis.h"

namespace diamond {

class Cloud
{
public:
    Cloud() {Connect("coldwater.cs.washington.edu");};
    virtual ~Cloud();
    int Connect(const std::string &host);
    bool IsConnected();
    int Read(const std::string &key, std::string &value);
    int Write(const std::string &key, const std::string &value);

private:
    bool _connected = false;
    redisContext *_redis;
};

} // namespace diamond

#endif 
