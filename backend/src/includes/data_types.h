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
#include <string>
#include <vector>

namespace diamond {

class DString
{
public:
    DString() {};
    DString(const std::string &s, const std::string &key) : _s(s) {};
    ~DString() {};
    static int Map(DString &addr, const std::string &key);
    std::string Value();
    void Set(const std::string &s);
    DString & operator=(const std::string &s) { Set(s); return *this; };
        
private:
    std::string _s;
    std::string _key;
};
    
class DLong
{
public:
    DLong() {};
    DLong(const uint64_t l, const std::string &key) : _l(l), _key(key) {};
    ~DLong() {};
    static int Map(DLong &addr, const std::string &key);
    uint64_t Value();
    void Set(const uint64_t l);
    DLong & operator=(const uint64_t l) { Set(l); return *this; };
    DLong & operator+=(const uint64_t i) { Set(_l + i); return *this; };
    DLong & operator-=(const uint64_t i) { Set(_l - i); return *this; };
    
private:
    uint64_t _l;
    std::string _key;
};


class DCounter
{
public:
    DCounter() {};
    DCounter(const int c, const std::string &key) : _counter(c), _key(key) {};
    ~DCounter() {};
    static int Map(DCounter &addr, const std::string &key);
    int Value();
    void Set(const int val);
    DCounter & operator=(const int val) { Set(val); return *this; };
    DCounter operator++() { Set(_counter + 1); return *this; };
    DCounter operator--() { Set(_counter - 1); return *this; };
    DCounter & operator+=(const uint64_t i) { Set(_counter + i); return *this; };
    DCounter & operator-=(const uint64_t i) { Set(_counter - i); return *this; };

private:
    int _counter;
    std::string _key;
    
};

class DSet
{
public:
    DSet() {};
    DSet(std::unordered_set<uint64_t> set, const std::string &key) : _key(key), _set(set) {};
    ~DSet() {};
    static int Map(DSet &addr, const std::string &key);
    std::unordered_set<uint64_t> Members();
    bool InSet(const uint64_t val);
    void Add(const uint64_t val);
    void Add(const std::unordered_set<uint64_t> &set);
    void Remove(const uint64_t val);
    void Clear();
    DSet & operator=(const std::unordered_set<uint64_t> &set) { Add(set); return *this; };
    
private:
    std::string _key;
    std::unordered_set<uint64_t> _set;

    std::string Serialize();
    void Deserialize(std::string &s);
};

class DList
{
public:
    DList() {};
    DList(std::vector<uint64_t> vec, const std::string &key) : _key(key), _vec(vec) {};
    ~DList() {};
    static int Map(DList &addr, const std::string &key);
    std::vector<uint64_t> Members();
    uint64_t Value(const int index);
    int Index(const uint64_t val); /* Returns the index of the first copy of val, or -1 if not present */
    void Append(const uint64_t val);
    void Append(const std::vector<uint64_t> &vec);
    void Insert(const int index, const uint64_t val);
    void Erase(const int index);
    void Remove(const uint64_t val); /* Removes the first copy of val, if present */
    void Clear();
    DList & operator=(const std::vector<uint64_t> &vec) { Append(vec); return *this; };
    
private:
    std::string _key;
    std::vector<uint64_t> _vec;

    std::string Serialize();
    void Deserialize(std::string &s);
};

} // namespace diamond

#endif 
