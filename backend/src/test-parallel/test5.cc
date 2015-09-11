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

// Tests the Lock, Unlock, Signal and Wait with multithreaded app
// 
// Run this process
// Sucess if both clients print that the test passed
//
// Usage:
//   $ ./test5
// 


void* dlong_lock_client1(void* arg);
void* dlong_lock_client2(void* arg);

int* dlong_lock_local;

void* dlong_lock_client1(void* arg ){
    DLong l1;

    //Cloud::NewInstance();

    int ret = DLong::Map(l1, std::string("11"));
    EXPECT_EQ(ret, ERR_OK);

    sleep(1);

    l1.Lock();
    l1 = 10;    // Initial value
    *dlong_lock_local = 10;

    printf("Client 1 signaling\n");
    l1.Signal();

    sleep(1);

    EXPECT_EQ(*dlong_lock_local, 10);
    EXPECT_EQ(l1.Value(), 10);
    l1.Unlock();

    printf("Client 1 test5 passed the test!\n");
    return 0;
}

void* dlong_lock_client2(void *arg){
    DLong l1;

    //Cloud::NewInstance();

    int ret = DLong::Map(l1, std::string("11"));
    EXPECT_EQ(ret, ERR_OK);

    
    l1.Lock();

    while(l1.Value() == 0){
        printf("Client 2 waiting\n");
        l1.Wait();
    }

    l1 = 5; // Initial value
    *dlong_lock_local = 5;

    EXPECT_EQ(*dlong_lock_local, 5);
    EXPECT_EQ(l1.Value(), 5);
    l1.Unlock();

    printf("Client 2 test5 passed the test!\n");
    return 0;
}


int main(void){
    DLong l1;
    DLong l2;
    int ret;
    pthread_t t1,t2;
   
    dlong_lock_local = (int*) mmap(NULL, sizeof(*dlong_lock_local), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    assert(dlong_lock_local);



    ret = DLong::Map(l1, std::string("11"));
    EXPECT_EQ(ret, ERR_OK);
    l1 = 0;
    *dlong_lock_local = 0;

    ret = pthread_create(&t1, NULL, dlong_lock_client1, NULL);
    assert(ret == 0);

    ret = pthread_create(&t2, NULL, dlong_lock_client2, NULL);
    assert(ret == 0);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);


    printf("Test5 passed the test!\n");
}





