#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#ifdef ANDROID

inline static uint64_t getTimestamp() {
    struct timespec tstart={0,0};
    clock_gettime(CLOCK_MONOTONIC, &tstart);
    return tstart.tv_nsec;
}

#else // !ANDROID

inline static uint64_t getTimestamp() {
    uint32_t low, high;
	asm volatile ("rdtsc" : "=a" (low), "=d" (high));
	return (uint64_t)high << 32 | low;
} 

#endif // ANDROID


#endif // TIMESTAMP_H

