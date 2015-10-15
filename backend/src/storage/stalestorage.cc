

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


//bool Stalestorage::enabled = false;
//long Stalestorage::maxStalenessMs = 100;

void
Stalestorage::ViewBegin(void)
{
    //long threadID = getThreadID();
    if(IsEnabled()){
        long tid = getThreadID();
        _currentViews[tid] = NULL;
        LOG_STALEREADS("ViewBegin\n");
    }
}

StaleView* 
Stalestorage::GetCurrentView(void)
{
    long tid = getThreadID();
    auto cv = _currentViews.find(tid);
    if(cv == _currentViews.end()){
        return NULL;
    }else{
        return cv->second;
    }
}

bool
Stalestorage::IsViewInUse(StaleView * v){
    auto it = _currentViews.begin();

    for(;it!=_currentViews.end();it++){
        if(v == it->second){
            return true;
        }
    }
    return false;
}

bool 
Stalestorage::IsLastViewInconsistent(void)
{
    long tid = getThreadID();

    auto la = _lastAttemptFailed.find(tid);
    if(la == _lastAttemptFailed.end()){
        return NULL;
    }else{
        return la->second;
    }
}


// Return true if during this view all values read were from a consistent view
bool
Stalestorage::ViewEnd(){
    // XXX: TODO
    if(IsEnabled()){
        LOG_STALEREADS("ViewEnd\n");
        return true;
    }
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
    if(IsEnabled()){
        StaleView sv;

        sv.timestamp = getTimeMillis();
        sv.keyValues = keyValues;
        assert(keyValues.size() > 0 );

        LOG_STALEREADS_ARGS("ViewAdd (%ld elements)\n", keyValues.size());
        LOG_STALEREADS_DUMP();

        _views.push_back(sv);

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
    if(IsEnabled()){
        StaleView *current = GetCurrentView();

        if(current==NULL){
            // If we don't have a view yet (i.e., we haven't made stale reads yet)
            set<string> keys;
            StaleView* last;
            
            keys.insert(key);
            bool found = GetLastView(keys, last);
            if(!found){
                LOG_STALEREADS("Read: Stale not found\n");
                return ERR_NOTFOUND;
            }else{
                value = last->keyValues[key];
                long tid = getThreadID();
                _currentViews[tid] = last;
                LOG_STALEREADS_ARGS("Read: First stale (key: \'%s\' value: \'%s\')\n", key.c_str(), value.c_str());
                return ERR_OK;
            }

        }else{
            // If we already made stale reads
            auto exists = current->keyValues.find(key);
            if(exists == current->keyValues.end()){
                LOG_STALEREADS_ARGS("Read: Consistent stale not found (key: \'%s\' value: \'%s\')\n", key.c_str(), value.c_str());
                return ERR_NOTFOUND;
            }
            value = exists->second;
            LOG_STALEREADS_ARGS("Read: Consistent stale (key: \'%s\' value: \'%s\')\n", key.c_str(), value.c_str());
            return ERR_OK;
        }
    }else{
        return ERR_NOTFOUND;
    }
}


// int 
// Stalestorage::WatchRead(string key, string& value)
// {
// 
// 
// }



void
Stalestorage::SetStaleness(bool e)
{
    enabled = e;
}


void
Stalestorage::SetMaxStaleness(long max)
{
    maxStalenessMs = max;
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


