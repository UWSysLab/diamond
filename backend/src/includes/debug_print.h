#ifndef DEBUG_PRINT_H
#define DEBUG_PRINT_H

/*
 * Provides a portable debug print method that writes to the Android log when
 * built for Android, and to the standard output when built for x86.
 */

#ifdef __ANDROID__

#include <android/log.h>

#define APPNAME "Diamond"

void debugPrint(const char * str) {
    __android_log_print(ANDROID_LOG_VERBOSE, APPNAME, str);
}

#else //__ANDROID__

void debugPrint(const char * str) {
    printf("%s", str);
}

#endif //__ANDROID__

#endif
