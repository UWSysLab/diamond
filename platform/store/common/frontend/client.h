// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * common/client.h:
 *   Interface for a multiple shard transactional client.
 *
 **********************************************************************/

#ifndef _CLIENT_API_H_
#define _CLIENT_API_H_

#include "lib/assert.h"
#include "lib/message.h"
#include <string>
#include <set>
#include <map>

class Client
{
public:
    Client() { };
    virtual ~Client() { };

    // Begin a transaction.
    virtual void Begin() = 0;
    virtual void BeginRO() = 0;

    // Get the value corresponding to key.
    virtual int Get(const std::string &key, std::string &value) = 0;

    // Get the value corresponding to key.
    virtual int MultiGet(const std::set<std::string> &keys, std::map<std::string, std::string> &value) = 0;

    // Set the value for the given key.
    virtual int Put(const std::string &key, const std::string &value) = 0;

    // Commit all Get(s) and Put(s) since Begin().
    virtual bool Commit() = 0;
    
    // Abort all Get(s) and Put(s) since Begin().
    virtual void Abort() = 0;
};

#endif /* _CLIENT_API_H_ */
