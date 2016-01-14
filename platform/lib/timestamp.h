#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>

#ifdef ANDROID

inline static uint64_t getTimestamp() {
    struct timespec tstart={0,0};
    clock_gettime(CLOCK_MONOTONIC, &tstart);
    return tstart.tv_nsec;
}

inline static uint64_t getTimeMillis() {
    struct timespec tstart={0,0};
    clock_gettime(CLOCK_MONOTONIC, &tstart);
    return (tstart.tv_sec * 1000) + (tstart.tv_nsec / (1000 * 1000));
}

#else // !ANDROID

inline static uint64_t getTimestamp() {
    uint32_t low, high;
	asm volatile ("rdtsc" : "=a" (low), "=d" (high));
	return (uint64_t)high << 32 | low;
} 

inline static uint64_t getTimeMillis() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return (time.tv_sec * 1000) + (time.tv_usec / 1000);
}

#endif // ANDROID


#endif // TIMESTAMP_H

