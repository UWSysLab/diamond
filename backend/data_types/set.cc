// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * data_types/set.cc
 *   Diamond set type
 *
 **********************************************************************/

#include "includes/data_types.h"
#include "lib/assert.h"
#include "lib/message.h"
#include <unordered_map>

namespace diamond {

using namespace std;

extern Client diamondclient;
static unordered_map<string, DIDSet *> cache;
   
int
Set::Map(const DIDSet *addr, const string &key) {

   // take a look in the cache first
   std::unordered_map<string, DIDSet*>::const_iterator find = cache.find(key);
   if (find != cache.end()) {
      addr = find->second();
      return 0;
   }
   
   if (!diamondclient.IsConnected()) {
      Panic("Cannot map objects before connecting to backing store server");
   }

   string value;
   
   int ret = diamondclient.Read(key, value);

   if (ret != RPC_OK) {
      return ret;
   }

   addr = new Set(atol(value.c_str()), key);
   return 0;
}

void
Set::Increment()
{
   _counter++;

   char buf[50];
   sprintf(buf, "%i", counter);
   diamondclient.Write(key, string(buf));
}

void
Set::Decrement()
{
   _counter--;
   char buf[50];
   sprintf(buf, "%i", counter);
   diamondclient.Write(key, string(buf));
}
   
} // namespace diamond
