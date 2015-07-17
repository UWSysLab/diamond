// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * includes/data_types.h
 *   Diamond data type definitions
 *
 **********************************************************************/

#ifndef _DATA_TYPES_H_
#define _DATA_TYPES_H_

#include "client/client.h"
#include <unordered_set>

namespace diamond {

class DString
{
public:
    DString(const std::string &s, const std::string &key) : _s(s) {};
    ~DString() {};
    static int Map(const DString* addr, const std::string &key);
    
private:
    std::string _s;
    std::string _key;
};
    
class DLong
{
public:
    DLong(uint64_t l, const std::string &key) : _l(l), _key(key) {};
    ~DLong() {};
    static int Map(const DLong* addr, const std::string &key);

private:
    uint64_t _l;
    std::string _key;
};


class DCounter
{
public:
    DCounter(uint64_t c, const std::string &key) : _counter(c), _key(key) {};
    ~DCounter() {};
    static int Map(const DCounter *addr, const std::string &key);
    void Increment();
    void Decrement();

private:
    int _counter;
    std::string _key;
    
};

class DIDSet
{
public:
    DIDSet(std::unordered_set<uint64_t> set) : _set(set) {};
    ~DIDSet();
    static int Map(const DIDSet *addr, const std::string &key);
    
private:
    std::string _key;
    std::unordered_set<uint64_t> _set;
};

} // namespace diamond

#endif 
