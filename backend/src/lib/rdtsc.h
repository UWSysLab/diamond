#ifndef RDTSC_H
#define RDTSC_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

inline uint64_t rdtsc() {
    uint32_t low, high;
	asm volatile ("rdtsc" : "=a" (low), "=d" (high));
	return (uint64_t)high << 32 | low;
}



#endif //RDTSC_H

