#ifndef PROFILE_H
#define PROFILE_H


#include "lib/timestamp.h"


#define MAX_THREAD_ID 65536

namespace diamond{

extern pthread_mutex_t profileMutex;
extern long profileEnterTs[MAX_THREAD_ID];

}


#ifdef PROFILE



#define PROFILE_ENTER(str)                                      \
{                                                               \
    pthread_mutex_lock(&diamond::profileMutex);                          \
    long tId = getThreadID();                                   \
    long ts = getTimestamp();                                   \
                                                                \
    assert(tId<MAX_THREAD_ID);                                  \
                                                                \
    diamond::profileEnterTs[tId] = ts;                                   \
                                                                \
    pthread_mutex_unlock(&diamond::profileMutex);                        \
}

/*    printf("PROFILE_ENTER(%s)\n", ts, tId, str);  */      


#define PROFILE_EXIT(str)                                          \
{                                                               \
    pthread_mutex_lock(&diamond::profileMutex);                          \
                                                                \
    long tId = getThreadID();                                   \
    assert(tId<MAX_THREAD_ID);                                  \
                                                                \
    long enterTs = diamond::profileEnterTs[tId];                         \
    long exitTs = getTimestamp();                               \
                                                                \
    printf("PROFILE_EXIT %ld %ld %s\n", exitTs - enterTs, tId, str);         \
                                                                \
                                                                \
    pthread_mutex_unlock(&diamond::profileMutex);                        \
}



#else

#define PROFILE_ENTER(str) {}
#define PROFILE_EXIT(str)  {}                       

#endif

#endif // PROFILE_H
