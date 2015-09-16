// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * storage/local.h:
 *   Diamond logic for accessing local storage
 *
 **********************************************************************/

#ifndef _LOCAL_H_
#define _LOCAL_H_

#include <string>
#include "unqlite.h"

namespace diamond {

class Local
{
public:
    Local() {};
    virtual ~Local();
    int Open(const std::string &filename);
    bool IsOpen();
    int Read(const std::string &key, std::string &value);
    int Write(const std::string &key, const std::string &value);

private:
    unqlite *_db;
};

} // namespace diamond

#endif 
