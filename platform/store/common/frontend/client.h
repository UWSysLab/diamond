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
#include <vector>
#include <map>

class Client
{
public:
    Client();
    ~Client();

    // Begin a transaction.
    virtual void Begin();

    // Get the value corresponding to key.
    virtual int Get(const std::string &key, std::string &value);

    // Get the value corresponding to key.
    virtual int MultiGet(const std::vector<std::string> &keys, std::map<std::string, std::string> &value);

    // Set the value for the given key.
    virtual int Put(const std::string &key, const std::string &value);

    // Commit all Get(s) and Put(s) since Begin().
    virtual bool Commit();
    
    // Abort all Get(s) and Put(s) since Begin().
    virtual void Abort();

    // Returns statistics (vector of integers) about most recent transaction.
    virtual std::vector<int> Stats();
};

#endif /* _CLIENT_API_H_ */
