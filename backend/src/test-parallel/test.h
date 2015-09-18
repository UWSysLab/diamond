#ifndef TEST_H
#define TEST_H

#include <semaphore.h>
#include <unistd.h>
#include <assert.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

#include <cassert>
#include <vector>
#include <iostream>

#include <unistd.h>


using namespace diamond;

// MACRO from: https://stackoverflow.com/questions/2193544/how-to-print-additional-information-when-assert-fails
#define EXPECT_EQ(left,right) {         \
        if(!((left) == (right))){       \
            std::cerr << "!!ASSERT FAILED: " << #left << "==" << #right << " @ " << __FILE__ << " (" << __LINE__ << "). " << #left << "=" << (left) << "; " << #right << "=" << (right) << std::endl; \
            exit(2); \
        }}



#endif // TEST_H
