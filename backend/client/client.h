// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * client/client.h:
 *   Diamond client-side logic
 *
 **********************************************************************/

#ifndef _CLIENT_H_
#define _CLIENT_H_

#define RPC_OK 0
#define RPC_UNCONNECTED 1
#define RPC_ERR 2

#include <unordered_map>
#include <string>
#include <redox.hpp>

namespace diamond {

class Client
{
public:
    Client() {};
    virtual ~Client(_connected = false;);
    int Connect(const std::string &host);
    bool IsConnected();
    int Read(const std::string &key, std::string &value);
    int Write(const std::string &key, const std::string &value);

private:
    bool _connected;
    redox::Redox _redis;
};

} // namespace diamond

#endif 
