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
#include <set>
#include <map>
#include <pthread.h>
#include <functional>

#include "profile.h"

namespace diamond {

using namespace std;

#define LOCK_DURATION_MS (5*1000)

void DiamondInit();
void DiamondInit(const std::string &server, int nshards, int closestReplica);
void DiamondInit(const std::string &hostname, const std::string &port);
std::string GetThreadId();

class DObject
{
public:
    static int MultiMap(std::vector<DObject *> &objects, std::vector<std::string> &keys);
    static int Map(DObject &addr, const std::string &key);
    static void TransactionBegin(void);
    static int TransactionCommit(void);
    static void BeginReactive(uint64_t reactive_id);
    static uint64_t GetNextNotification(bool blocking);
    static void NotificationInit(std::function<void (void)> callback);

    static void SetLinearizable();
    static void SetSnapshotIsolation();
    static void SetEventual();
     
protected:
    DObject() : _key("dummykey") {};
    DObject(const std::string &key) : _key(key) {};
    virtual ~DObject() {};
    std::string _key;
    pthread_mutex_t  _objectMutex = PTHREAD_MUTEX_INITIALIZER;

    virtual std::string Serialize() = 0;
    virtual void Deserialize(const std::string &s) = 0;
    int Push();
    int Pull();
    int Increment(int inc);

private:
    uint64_t _lockid = 0;
    long _locked = 0;
};

class DString : public DObject 
{
public:
    DString() : DObject(), _s("dummystring") {};
    DString(const std::string &s, const std::string &key) : DObject(key), _s(s) {};
    ~DString() {};
    std::string Value();
    void Set(const std::string &s);
    DString & operator=(const std::string &s) { Set(s); return *this; };
        
private:
    std::string _s;

    std::string Serialize();
    void Deserialize(const std::string &s);
};
    
class DLong : public DObject
{
public:
    DLong() {};
    DLong(const uint64_t l, const std::string &key) : DObject(key), _l(l) {};
    ~DLong() {};
    uint64_t Value();
    void Set(const uint64_t l);
    DLong & operator=(const uint64_t l) { Set(l); return *this; };
    DLong & operator+=(const uint64_t i);
    DLong & operator-=(const uint64_t i);

private:
    uint64_t _l;

    std::string Serialize();
    void Deserialize(const std::string &s);
    void SetNotProtected(const uint64_t l);
};


class DCounter : public DObject
{
public:
    DCounter() {};
    DCounter(const int c, const std::string &key) : DObject(key), _counter(c) {};
    ~DCounter() {};
    int Value();
    void Set(const int val);
    DCounter & operator=(const int val) { Set(val); return *this; };
    DCounter & operator++();
    DCounter & operator--();
    DCounter & operator+=(const uint64_t i);
    DCounter & operator-=(const uint64_t i);

private:
    int _counter;

    std::string Serialize();
    void Deserialize(const std::string &s);
    void IncrementNotProtected(const int val);
};


class DSet : public DObject
{
public:
    DSet() {};
    DSet(std::unordered_set<uint64_t> set, const std::string &key) : DObject(key), _set(set) {};
    ~DSet() {};
    std::unordered_set<uint64_t> Members();
    std::vector<uint64_t> MembersAsVector();
    bool InSet(const uint64_t val);
    void Add(const uint64_t val);
    void Add(const std::unordered_set<uint64_t> &set);
    void Remove(const uint64_t val);
    void Clear();
    int Size();
    DSet & operator=(const std::unordered_set<uint64_t> &set) { Add(set); return *this; };
    
private:
    std::unordered_set<uint64_t> _set;

    std::string Serialize();
    void Deserialize(const std::string &s);
};

class DStringSet : public DObject
{
public:
    DStringSet() {};
    DStringSet(std::unordered_set<std::string> set, const std::string &key) : DObject(key), _set(set) {};
    ~DStringSet() {};
    std::unordered_set<std::string> Members();
    std::vector<std::string> MembersAsVector();
    bool InSet(const std::string &val);
    void Add(const std::string &val);
    void Add(const std::unordered_set<std::string> &set);
    void Remove(const std::string &val);
    void Clear();
    DStringSet & operator=(const std::unordered_set<std::string> &set) { Add(set); return *this; };
    
private:
    std::unordered_set<std::string> _set;

    std::string Serialize();
    void Deserialize(const std::string &s);
};

class DList : public DObject
{
public:
    DList() {};
    DList(std::vector<uint64_t> vec, const std::string &key) : DObject(key), _vec(vec) {};
    DList(const std::string &key) : DObject(key) {};
    ~DList() {};
    std::vector<uint64_t> Members();
    uint64_t Value(const int index);
    int Index(const uint64_t val); /* Returns the index of the first copy of val, or -1 if not present */

    void Append(const uint64_t val);
    void Append(const std::vector<uint64_t> &vec);
    void Insert(const int index, const uint64_t val);
    void Erase(const int index);
    void Remove(const uint64_t val); /* Removes the first copy of val, if present */
    void Set(const int index, const uint64_t val);
    uint64_t Lpop();
    void Clear();
    int Size();
    DList & operator=(const std::vector<uint64_t> &vec) { Append(vec); return *this; };
    uint64_t operator[](const int index) { return Value(index); }
    
private:
    std::vector<uint64_t> _vec;

    std::string Serialize();
    void Deserialize(const std::string &s);
    int IndexNotProtected(const uint64_t val); /* Returns the index of the first copy of val, or -1 if not present */
};

class DStringList : public DObject
{
public:
    DStringList() {};
    DStringList(std::vector<std::string> vec, const std::string &key) : DObject(key), _vec(vec) {};
    DStringList(const std::string &key) : DObject(key) {};
    ~DStringList() {};
    std::vector<std::string> Members();
    std::string Value(const int index);
    int Index(const std::string val); /* Returns the index of the first copy of val, or -1 if not present */
    void Append(const std::string val);
    void Append(const std::vector<std::string> &vec);
    void Insert(const int index, const std::string val);
    void Erase(const int index);
    void Remove(const std::string val); /* Removes the first copy of val, if present */
    void Set(const int index, const std::string val);
    std::string Lpop();
    void Clear();
    int Size();
    DStringList & operator=(const std::vector<std::string> &vec) { Append(vec); return *this; };
    std::string operator[](const int index) { return Value(index); }
    
private:
    std::vector<std::string> _vec;

    std::string Serialize();
    void Deserialize(const std::string &s);
    int IndexNotProtected(const std::string val); /* Returns the index of the first copy of val, or -1 if not present */
};

class DBooleanList : public DObject
{
public:
    DBooleanList() {};
    DBooleanList(std::vector<bool> vec, const std::string &key) : DObject(key), _vec(vec) {};
    DBooleanList(const std::string &key) : DObject(key) {};
    ~DBooleanList() {};
    std::vector<bool> Members();
    bool Value(const int index);
    void Append(const bool val);
    void Append(const std::vector<bool> &vec);
    void Insert(const int index, const bool val);
    void Erase(const int index);
    void Set(const int index, const bool val);
    void Clear();
    int Size();
    DBooleanList & operator=(const std::vector<bool> &vec) { Append(vec); return *this; };
    bool operator[](const int index) { return Value(index); }
    
private:
    std::vector<bool> _vec;

    std::string Serialize();
    void Deserialize(const std::string &s);
    int IndexNotProtected(const std::string val); /* Returns the index of the first copy of val, or -1 if not present */
};

// class DStringQueue : public DObject
// {
// public:
//     DStringQueue() {};
//     DStringQueue(const std::string &key) : DObject(key) {};
//     ~DStringQueue() {};
//     void Queue(const std::string val);
//     void Dequeue(const std::string val); /* Removes the first copy of val, if present */
//     int Size();
//     int Clear();
// //    DStringQueue & operator=(const std::vector<std::string> &vec) { Append(vec); return *this; };
    
// private:
//     DStringList list;

//     std::string Serialize();
//     void Deserialize(const std::string &s);
// };


class DID : public DObject 
{
public:
    DID() : DObject(), _s("dummystring") {};
    DID(const std::string &s, const std::string &key) : DObject(key), _s(s) {};
    ~DID() {};
    std::string Value();
    void Generate();
        
private:
    std::string _s;

    std::string Serialize();
    void Deserialize(const std::string &s);
};

class DBoolean : public DObject
{
public:
    DBoolean() : _b() {};
    DBoolean(const bool b, const std::string &key) : DObject(key), _b(b) {};
    ~DBoolean() {};
    bool Value();
    void Set(const bool b);
    DBoolean & operator=(const bool b) { Set(b); return *this; };

private:
    bool _b;

    std::string Serialize();
    void Deserialize(const std::string &s);
    void SetNotProtected(const bool b);
};

} // namespace diamond

#endif 
