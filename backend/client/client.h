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
namespace diamond {

class Client
{
public:
    Client(std::string configPath);
    virtual ~Client();
    uint64_t* Map(uint64_t key);
    uint64_t Read(uint64_t key);
    void Write(uint64_t key, uint64_t value);

private:
    std::unordered_map<uint64_t, uint64_t> store;
};

} // namespace diamond

#endif 
