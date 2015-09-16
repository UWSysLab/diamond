// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * storage/local.cc:
 *   Diamond client for local storage
 *
 **********************************************************************/

#include "storage/cloud.h"

namespace diamond {

using namespace std;

Local::~Local()
{
    unqlite_close(_db);
}
    
int
Local::Open(const std::string &filename)
{
    int ret;
    
    if (_db) {
        return ERR_OK;
    }
    ret = unqlite_open(&_db, filename, UNQLITE_OPEN_CREATE);
    if (ret == UNQLITE_ERR_OK) {
        return ERR_OK;
    } else {
        return ERR_UNAVAILABLE;
    }
}

bool
Local::IsOpen()
{
    return _db;
}
    
int
Local::Read(const string &key, string &value)
{
    if (!_open) {
        return ERR_UNAVAILABLE;
    }

    size_t len;
    char *buf;
    int ret;

    // Get size
    ret = unqlite_fetch(_db, key.c_str(), -1, NULL, &len);

    if (ret != UNQLITE_ERR_OK) {
        return ERR_LOCAL;
    }

    buf = (char *)malloc(len);

    if (buf == NULL) {
        return ERR_LOCAL;
    }

    ret = unqlite_fetch(_db, key.c_str(), -1, buf, len);
    
    if (ret != UNQLITE_ERR_OK) {
        free(buf);
        return ERR_LOCAL;
    }

    value = string(buf, len);
    free(buf);
    return ERR_OK;
}

int
Local::Write(const string &key, const string &value)
{
    if (!_open) {
        return ERR_UNAVAILABLE;
    }

    int ret = unqlite_store(_db, key.c_str(), -1, value.c_str(), value.len());

    if (ret == UNQLITE_ERR_OK) {
        return ERR_OK;
    }
    return ERR_LOCAL;
}

} // namespace diamond
