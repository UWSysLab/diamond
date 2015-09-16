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
#include <map>
#include <pthread.h>

namespace diamond {

#define LOCK_DURATION_MS (5*1000)

class DObject
{
public:
//    virtual DObject() = 0; // Abstract class

	void Lock();
	void ContinueLock();
	void Unlock();
	void Signal();
	void Broadcast();
	void Wait();
   static int MultiMap(std::map<DObject *, std::string> & keyMap);


protected:
    DObject() {};
    DObject(const std::string &key) : _key(key) {};
    std::string _key;
    pthread_mutex_t  _objectMutex = PTHREAD_MUTEX_INITIALIZER;

private:
    // mutex to protect local fields of the object
	uint64_t _lockid = 0;
	long _locked = 0;

	void LockNotProtected(); // Callee should hold the _objectMutex
	void UnlockNotProtected(); // Callee should hold the _objectMutex
};


class DString : public DObject 
{
public:
    DString() {};
    DString(const std::string &s, const std::string &key) : DObject(key), _s(s) {};
    ~DString() {};
    static int Map(DString &addr, const std::string &key);
    std::string Value();
    void Set(const std::string &s);
    DString & operator=(const std::string &s) { Set(s); return *this; };
        
private:
    std::string _s;
};
    
class DLong : public DObject
{
public:
    DLong() {};
    DLong(const uint64_t l, const std::string &key) : DObject(key), _l(l) {};
    ~DLong() {};
    static int Map(DLong &addr, const std::string &key);
    uint64_t Value();
    void Set(const uint64_t l);
    DLong & operator=(const uint64_t l) { Set(l); return *this; };
    DLong & operator+=(const uint64_t i) { Set(_l + i); return *this; };
    DLong & operator-=(const uint64_t i) { Set(_l - i); return *this; };

private:
    uint64_t _l;
};


class DCounter : public DObject
{
public:
    DCounter() {};
    DCounter(const int c, const std::string &key) : DObject(key), _counter(c) {};
    ~DCounter() {};
    static int Map(DCounter &addr, const std::string &key);
    int Value();
    void Set(const int val);
    DCounter & operator=(const int val) { Set(val); return *this; };
    DCounter & operator++() { Set(_counter + 1); return *this; };
    DCounter & operator--() { Set(_counter - 1); return *this; };
    DCounter & operator+=(const uint64_t i) { Set(_counter + i); return *this; };
    DCounter & operator-=(const uint64_t i) { Set(_counter - i); return *this; };

private:
    int _counter;
};

class DSet : public DObject
{
public:
    DSet() {};
    DSet(std::unordered_set<uint64_t> set, const std::string &key) : DObject(key), _set(set) {};
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
    std::unordered_set<uint64_t> _set;

    std::string Serialize();
    void Deserialize(std::string &s);
};

class DList : public DObject
{
public:
    DList() {};
    DList(std::vector<uint64_t> vec, const std::string &key) : DObject(key), _vec(vec) {};
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
    uint64_t Lpop();
    void Clear();
    int Size();
    DList & operator=(const std::vector<uint64_t> &vec) { Append(vec); return *this; };
    
private:
    std::vector<uint64_t> _vec;

    std::string Serialize();
    void Deserialize(std::string &s);
    int IndexNotProtected(const uint64_t val); /* Returns the index of the first copy of val, or -1 if not present */
};

class DStringList : public DObject
{
public:
    DStringList() {};
    DStringList(std::vector<std::string> vec, const std::string &key) : DObject(key), _vec(vec) {};
    DStringList(const std::string &key) : DObject(key) {};
    ~DStringList() {};
    static int Map(DStringList &addr, const std::string &key);
    std::vector<std::string> Members();
    std::string Value(const int index);
    int Index(const std::string val); /* Returns the index of the first copy of val, or -1 if not present */
    void Append(const std::string val);
    void Append(const std::vector<std::string> &vec);
    void Insert(const int index, const std::string val);
    void Erase(const int index);
    void Remove(const std::string val); /* Removes the first copy of val, if present */
    std::string Lpop();
    void Clear();
    int Size();
    DStringList & operator=(const std::vector<std::string> &vec) { Append(vec); return *this; };
    
private:
    std::vector<std::string> _vec;

    std::string Serialize();
    void Deserialize(std::string &s);
};

} // namespace diamond

#endif 
