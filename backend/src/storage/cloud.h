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

#define RPC_OK 0
#define RPC_UNCONNECTED 1
#define RPC_ERR 2

#include <unordered_map>
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
