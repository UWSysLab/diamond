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

#include <unordered_map>
#include <string>
#include <redox.hpp>

namespace diamond {

class Client
{
public:
    Client(std::string &host, int port);
    virtual ~Client();
    int Map(int *addr, std::string &key);
    int Read(std::string &key);
    void Write(std::string &key, int value);

private:
    std::unordered_map<std::string, std::string> cache;
    redox::Redox rdx;
};

} // namespace diamond

#endif 
