// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * storage/stalestorage.h.h:
 *   Diamond logic for accessing local cache
 *
 **********************************************************************/

#ifndef _STALESTORAGE_H_
#define _STALESTORAGE_H_

#include <stdlib.h>
#include <stdio.h>
#include <csignal>

#include <semaphore.h>
#include <stdlib.h>
#include "hiredis.h"

#include <signal.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include <unordered_map>
#include <vector>
#include <set>
#include <map>
#include <list>
#include <string>



namespace diamond {

using namespace std;

#define MAX_STALEREADS_VIEWS 20

typedef struct StructStaleView
{
    long timestamp;
    std::map<std::string, std::string> keyValues;
} StaleView;


typedef struct StructTransactionView{
    StaleView *sv;
    bool currenConsistent;
    bool previousInconsistent;      // Prevents us from going into an infinite cycle because of stale
    bool stalestorageRead;
} TransactionView;


class Stalestorage
{
public:
    Stalestorage() {};
    virtual ~Stalestorage() {};

    void ViewBegin(void);
    bool ViewEnd(void);

    void ViewAdd(std::map<std::string, std::string> rs, std::map<std::string, std::string> ws);
    void ViewAdd(map<string, string> keyValues);

    void SetStaleness(bool enable);
    void SetMaxStaleness(long maxStalenessMs);

    void DebugDump();

    int Read(std::string key, std::string& value);

    bool enabled = false;
    long maxStalenessMs = 100; 

private:
    TransactionView* GetTransactionView(void);
    StaleView* GetCurrentView(void);
    bool IsViewInUse(StaleView * v);
    bool IsLastViewInconsistent(void);
    bool GetLastView(set<string> keys, StaleView* &result);
    bool IsEnabled(void);

    std::list<StaleView> _views;
    std::map<long, TransactionView> _transactionViews; // tid - > StaleView
};




} // namespace diamond



//#define DEBUG_STALEREADS

#ifdef DEBUG_STALEREADS

#define LOG_STALEREADS(str) {\
    Notice("[%ld] STALEREADS: " str , getThreadID());\
}
#define LOG_STALEREADS_ARGS(str, ...) {\
    Notice("[%ld] STALEREADS: " str , getThreadID(), __VA_ARGS__);\
}
#define LOG_STALEREADS_DUMP(){\
    DebugDump();\
}

#else  // DEBUG_STALEREADS

#define LOG_STALEREADS(str) {}
#define LOG_STALEREADS_ARGS(str, ...) {}
#define LOG_STALEREADS_DUMP(){}

#endif // DEBUG_STALEREADS





#endif
