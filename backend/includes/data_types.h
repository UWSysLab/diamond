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

#include <unordered_set>

namespace diamond {

Client _diamondclient;

class String
{
public:
    String(std::string s) : s(s);
    ~String();
    static int Map(String* addr, std::string key);

private:
    std::string _s;
}
    
class Long
{
public:
    Long(uint64_t l, std::string key) : _l(l), _key(key);
    ~Long();
    static int Map(const Long* addr, const std::string &key);
    
private:
    std::string _key;
    uint64_t _l;
};

class Counter
{
public:
    Counter(uint64_t c, std::string key) : _counter(c), _key(key);
    ~Counter();
    static int Map(const Counter *addr, const std::string &key);
    void Increment();
    void Decrement();
private:
    std::string _key;
    int _counter;
}

// class Set
// {
// public:
//     Set(std::unordered_set set) : _set(set);
//     ~Set();
//     static int Map(const Set *addr, const std::string &key);
// private:
//     std::string _key;
//     std::unordered_set<std::string> _set;
// }
    
} // namespace diamond

#endif 
