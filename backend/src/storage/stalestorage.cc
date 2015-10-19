

#include "lib/assert.h"
#include "stalestorage.h"

#include "storage/cloud.h"
#include "lib/assert.h"
#include <string>
#include <vector>
#include <set>
#include <map>
#include <sstream>
#include <async.h>
#include <algorithm>
#include <iterator>

#include "includes/profile.h"

namespace diamond {

using namespace std;


static pthread_mutex_t  stalestorageMutex = PTHREAD_MUTEX_INITIALIZER; // Protects the global transaction structures


void
Stalestorage::ViewBegin(void)
{
    pthread_mutex_lock(&stalestorageMutex);
    
    if(IsEnabled()){
        LOG_STALEREADS("ViewBegin\n");

        TransactionView *tv = GetTransactionView();
        if(tv==NULL){
            long tid = getThreadID();
            _transactionViews[tid] = TransactionView();
            tv = GetTransactionView();
            assert(tv);
            // previousInconsistent is only initialized here the first time a thread executes a transaction
            // in subsequent transactions its initialized by the ViewEnd()
            tv->previousInconsistent = false;
        }
        tv->stalestorageRead = false; 
        tv->currenConsistent = true;
        tv->sv = NULL;
    }
    pthread_mutex_unlock(&stalestorageMutex);
}

TransactionView*
Stalestorage::GetTransactionView(void)
{
    long tid = getThreadID();
    auto cv = _transactionViews.find(tid);
    if(cv == _transactionViews.end()){
        return NULL;
    }else{
        return &cv->second;
    }
}

bool
Stalestorage::IsViewInUse(StaleView * v){
    auto it = _transactionViews.begin();

    for(;it!=_transactionViews.end();it++){
        if(v == it->second.sv){
            return true;
        }
    }
    return false;
}

// Return true if during this view all values read were from a consistent view
bool
Stalestorage::ViewEnd(){
    pthread_mutex_lock(&stalestorageMutex);
    if(IsEnabled()){
        LOG_STALEREADS("ViewEnd\n");
        TransactionView *tv = GetTransactionView();
        tv->previousInconsistent = !tv->currenConsistent;

        pthread_mutex_unlock(&stalestorageMutex);
        return tv->currenConsistent;
    }

    pthread_mutex_unlock(&stalestorageMutex);
    return true;
}


void
Stalestorage::ViewAdd(map<string, string> rs, map<string, string> ws){
    if(IsEnabled()){
        map<string, string> keyValues;
        keyValues.insert(ws.begin(), ws.end());
        keyValues.insert(rs.begin(), rs.end());
   
        ViewAdd(keyValues);
    }
}

void
Stalestorage::ViewAdd(map<string, string> keyValues){
    pthread_mutex_lock(&stalestorageMutex);

    if(IsEnabled()){
        TransactionView * tv = GetTransactionView();
        if(tv->currenConsistent == false){
            // Don't store view if tx didn't see a consistent view 
            LOG_STALEREADS("ViewAdd: not storing because view was not consistent\n");
            pthread_mutex_unlock(&stalestorageMutex);
            return;
        }
        if(tv->stalestorageRead == true){
            // Don't store view if the tx used the consistent store (otherwise we would extend beyond the staleness limit)
            LOG_STALEREADS("ViewAdd: not storing because stalestorage was read\n");
            pthread_mutex_unlock(&stalestorageMutex);
            return;
        }

        StaleView sv;

        sv.timestamp = getTimeMillis();
        sv.keyValues = keyValues;
        assert(keyValues.size() > 0 );

        LOG_STALEREADS_ARGS("ViewAdd (%ld elements)\n", keyValues.size());
        LOG_STALEREADS_DUMP();

        _views.push_back(sv);

        // GC
        // XXX: this could be improved by taking out only useless stale views
        while(_views.size() > MAX_STALEREADS_VIEWS){
            // Should check that nobody is using this view
            StaleView *sv = &_views.front();
            if(IsViewInUse(sv)){
                // Only do garbage collection if other transactions are not using the view about to be deleted
                break;
            }
            LOG_STALEREADS("ViewAdd: removing older element\n");
            _views.pop_front();
        }
    }

    pthread_mutex_unlock(&stalestorageMutex);
}


bool
Stalestorage::GetLastView(set<string> keys, StaleView* &result){
    auto it = _views.rbegin();
    assert(keys.size() > 0);
    long minTimestamp = getTimeMillis() - maxStalenessMs; 

    // Find the most recent view that contains all the keys
    for(;it != _views.rend();it++){
        StaleView view = *it;
        bool keyNotFound = false;
        auto itKeys = keys.begin();

        for(;itKeys!=keys.end();itKeys++){
            string key = *itKeys;
            auto exist = view.keyValues.find(key);
            if(exist == view.keyValues.end()){
                keyNotFound = true;
            }
        }

        if(keyNotFound == false && minTimestamp < view.timestamp){
            result = &(*it);
            return true;
        }
    }

    return false;
}


int
Stalestorage::Read(string key, string& value)
{
    pthread_mutex_lock(&stalestorageMutex);

    if(IsEnabled()){
        TransactionView *tv = GetTransactionView();
        assert(tv);  // Not inside a transaction?!?
        StaleView *current = tv->sv;

        if(tv->previousInconsistent){
            LOG_STALEREADS("Read: Don't use stalestorage because previous tx saw inconsistent reads\n");

            pthread_mutex_unlock(&stalestorageMutex);
            return ERR_NOTFOUND;
        }

        if(current==NULL){
            // If we don't have a view yet (i.e., we haven't made stale reads yet)
            set<string> keys;
            StaleView* last;
            
            keys.insert(key);
            bool found = GetLastView(keys, last);
            if(!found){
                LOG_STALEREADS("Read: Stale not found\n");

                pthread_mutex_unlock(&stalestorageMutex);
                return ERR_NOTFOUND;
            }else{
                value = last->keyValues[key];
                tv->sv = last;
                tv->stalestorageRead = true;
                LOG_STALEREADS_ARGS("Read: First stale (key: \'%s\' value: \'%s\')\n", key.c_str(), value.c_str());

                pthread_mutex_unlock(&stalestorageMutex);
                return ERR_OK;
            }

        }else{
            // If we already made stale reads
            auto exists = current->keyValues.find(key);
            if(exists == current->keyValues.end()){
                LOG_STALEREADS_ARGS("Read: Consistent stale not found (key: \'%s\' value: \'%s\')\n", key.c_str(), value.c_str());

                pthread_mutex_unlock(&stalestorageMutex);
                return ERR_NOTFOUND;
            }
            value = exists->second;
            tv->stalestorageRead = true;
            LOG_STALEREADS_ARGS("Read: Consistent stale (key: \'%s\' value: \'%s\')\n", key.c_str(), value.c_str());

            pthread_mutex_unlock(&stalestorageMutex);
            return ERR_OK;
        }
    }else{
        pthread_mutex_unlock(&stalestorageMutex);
        return ERR_NOTFOUND;
    }
}

void
Stalestorage::SetStaleness(bool e)
{
    pthread_mutex_lock(&stalestorageMutex);
    enabled = e;
    pthread_mutex_unlock(&stalestorageMutex);
}


void
Stalestorage::SetMaxStaleness(long max)
{
    pthread_mutex_lock(&stalestorageMutex);
    maxStalenessMs = max;
    pthread_mutex_unlock(&stalestorageMutex);
}

bool
Stalestorage::IsEnabled(void)
{
    return enabled;
}


void
Stalestorage::DebugDump()
{
    if(IsEnabled()){
        auto it = _views.rbegin();
        int i = 0;
        Notice("%ld entries\n", _views.size());
        // Find the most recent view that contains all the keys
        for(;it != _views.rend();it++){
            StaleView view = *it;
            auto itKeyValues = view.keyValues.begin();
            string viewKeyValues;
            for(;itKeyValues!=view.keyValues.end();itKeyValues++){
                string key = itKeyValues->first;
                string value = itKeyValues->second;
                viewKeyValues = "\"" + key + "\":\"" + value + "\", "+ viewKeyValues;
            }
            Notice("%d %ld %s\n", i, view.timestamp, viewKeyValues.c_str());
            i++;
        }
    }
}




}


