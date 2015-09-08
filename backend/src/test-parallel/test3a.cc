// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 3 -*-
/***********************************************************************
 *
 * kvstore-test.cc:
 *   test cases for simple key-value store class
 *
 * Copyright 2015 Irene Zhang  <iyzhang@cs.washington.edu>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************************/

#include "storage/cloud.h"
#include "includes/data_types.h"
#include "test.h"

// This client (test3a) tests Wait() while the other client (test3b) tests Signal()
// 
// Run the two clients at the same time (within a one second window)
// Success if both client print that the test passed
//
// Usage:
//   $ ./test3a & ./test3b & wait ; wait 
// 



int main(void){
    DLong l1;
    int iterations = 0;

    int ret = DLong::Map(l1, std::string("11"));
    EXPECT_EQ(ret, ERR_OK);

    l1.Lock();
    l1 = 1;    // Initial value

    sleep(1);

    if (l1.Value() == 1){
        iterations++;
        l1.Wait();
    }

    EXPECT_EQ(l1.Value(), 0);
    l1.Unlock();

    EXPECT_EQ(iterations, 1);
    printf("Client test3a (%d) passed the test! (%d iterations)\n", getpid(), iterations);
}




