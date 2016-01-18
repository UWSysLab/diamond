// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:

#include "includes/data_types.h"
#include "lib/assert.h"
#include "lib/message.h"
#include "lib/timestamp.h"
#include <unordered_map>
#include <inttypes.h>

namespace diamond {

   using namespace std;

bool
DBoolean::Value() {
   pthread_mutex_unlock(&_objectMutex); 

   Pull();
   bool ret = _b;

   pthread_mutex_unlock(&_objectMutex); 
   return ret;
}

void
DBoolean::Set(const bool b)
{
   pthread_mutex_lock(&_objectMutex); 
    
   SetNotProtected(b);

   pthread_mutex_unlock(&_objectMutex); 
}

void
DBoolean::SetNotProtected(const bool b)
{
   _b = b;
   Push();
}

   std::string
   DBoolean::Serialize() {
      char buf[50];
      sprintf(buf, "%" PRIu64 "", _b);
      return string(buf);
   }

void
DBoolean::Deserialize(const std::string &s) {
   _b = (bool)atol(s.c_str());
}

} // namespace diamond
