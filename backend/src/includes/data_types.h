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

#include "profile.h"

namespace diamond {

using namespace std;

#define LOCK_DURATION_MS (5*1000)

enum DConsistency {RELEASE_CONSISTENCY, SEQUENTIAL_CONSISTENCY};
enum txResult {COMMIT, ROLLBACK, RETRY};

void DiamondInit();
void DiamondInit(const std::string &server);


typedef struct structTransactionState {
    int cloudReadCount; // Counts the reads that hit the redis server

    std::set<std::string> rs; 
    std::set<std::string> ws; 
    std::map<std::string, std::string > localView; // Content of the values read/written inside the transaction

    std::set<string> txPrefetchSet; // User-specified set of keys that are expected to be used together in this specific transaction
} TransactionState;

class DObject
{
public:
	void Lock();
	void ContinueLock();
	void Unlock();
	void Signal();
	void Broadcast();
	void Wait();
    static int MultiMap(std::vector<DObject *> &objects, std::vector<std::string> &keys);
    static int Map(DObject &addr, const std::string &key);


    static void TransactionBegin(void);
    static void TransactionBegin(set<string> &txPrefetchSet);
    static void TransactionBegin(set<DObject*> &txPrefetchSet);
    static int TransactionCommit(void);
    static void TransactionRollback(void);
    static void TransactionRetry(void);
    static bool TransactionExecute(enum txResult (*txHandler)(void*), void * txArg, unsigned int maxAttempts);
    static void TransactionLearnPrefetchSet();

    static void PrefetchGlobalAddSet(set<DObject*> &prefetchSet); 
    static void PrefetchGlobalAddSet(set<string> &prefetchSet); 
    static void PrefetchGlobalRemoveSet(set<DObject*> &prefetchSet); 
    static void PrefetchGlobalRemoveSet(set<string> &prefetchSet); 

    static void SetNetworkConnectivity(bool connectivity);
    std::string GetKey(void);

    static void SetGlobalPrefetch(bool enable);
    static void SetGlobalStaleness(bool enable);
    static void SetGlobalMaxStaleness(long maxStalenessMs);
    void SetGlobalConsistency(enum DConsistency dc); // Made obsolete by the transactions?!

    static void DebugSleep(long seconds);

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

private:
	uint64_t _lockid = 0;
	long _locked = 0;

    static bool prefetchEnabled;

	void LockNotProtected(); // Callee should hold the _objectMutex
	void UnlockNotProtected(); // Callee should hold the _objectMutex
    int PushAlways();
    int PullAlways();
    int PullAlwaysWatch();

    static bool IsTransactionInProgress(void);
    static void SetTransactionInProgress(bool res);
    static void SetTransactionPrefetchSet(set<string> &txPrefetchSet);
    static set<string> GetKeys(set<DObject*> &objs);

    static TransactionState* GetTransactionState(void);
  
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
    void SetNotProtected(const int val);
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
    uint64_t Lpop();
    void Clear();
    int Size();
    DList & operator=(const std::vector<uint64_t> &vec) { Append(vec); return *this; };
    
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
    std::string Lpop();
    void Clear();
    int Size();
    DStringList & operator=(const std::vector<std::string> &vec) { Append(vec); return *this; };
    
private:
    std::vector<std::string> _vec;

    std::string Serialize();
    void Deserialize(const std::string &s);
    int IndexNotProtected(const std::string val); /* Returns the index of the first copy of val, or -1 if not present */
};

class DRedisStringList
{
public:
    DRedisStringList() {};
    DRedisStringList(const std::string &key) : _key(key) {};
    ~DRedisStringList() {};
    std::vector<std::string> Members();
    std::string Value(const int index);
    void Append(const std::string &val);
    void Append(const std::vector<std::string> &vec);
    void EraseFirst();
    void Remove(const std::string &val);
    void Clear();
    long Size();
    DRedisStringList & operator=(const std::vector<std::string> &vec) { Append(vec); return *this; };
    
private:
    std::string _key;
};



} // namespace diamond


//#define DEBUG_RC

#ifdef DEBUG_RC
#define LOG_RC(str) { Notice("[%ld] Key %s: %s\n", getThreadID(), this->_key.c_str(), str);}
#else // DEBUG_RC
#define LOG_RC(str) { }
#endif // DEBUG_RC



//#define DEBUG_TX

#ifdef DEBUG_TX
#define LOG_TX(str) {\
    Notice("[%ld] %s\n", getThreadID(), str);\
}
#define LOG_TX_DUMP_RS() {\
        std::set<string>* txRS = GetTransactionRS();\
        int i = 0;\
        Notice("[%ld] RS size = %ld\n", getThreadID(), txRS->size());\
        auto it = txRS->begin();\
        std::map<string, string >* locals = GetTransactionLocals();\
        for (; it != txRS->end(); it++,i++) {\
             const char* value = (*locals)[*it].c_str();\
             const char* key =  (*it).c_str();\
             Notice("[%ld] RS slot %d: Key = %s, Value = %s\n", getThreadID(), i, key, value);\
        }\
    }
#define LOG_TX_DUMP_WS() {\
        std::set<string>* txWS = GetTransactionWS();\
        int i = 0;\
        Notice("[%ld] WS size = %ld\n", getThreadID(), txWS->size());\
        auto it = txWS->begin();\
        std::map<string, string >* locals = GetTransactionLocals();\
        for (; it != txWS->end(); it++,i++) {\
             const char* value = (*locals)[*it].c_str();\
             const char* key =  (*it).c_str();\
             Notice("[%ld] WS slot %d: Key = %s, Value = %s\n", getThreadID(), i, key, value);\
        }\
    }
#else  // DEBUG_TX
#define LOG_TX(str) {}
#define LOG_TX_DUMP_RS() {}
#define LOG_TX_DUMP_WS() {}
#endif // DEBUG_TX



#endif 
