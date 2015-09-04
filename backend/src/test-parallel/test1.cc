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

// NOTE: This test does not work correctly possibly because the hiredis 
//     is not multi-threaded safe and additionally the hiredis may not
//     work correctly with fork
//     Lots of strange things can happen including:
//        - hiredis commands that don't return 
//        - hiredis commands that return unexpected results in a seemingly 
//       non-deterministic way   

int dlong_lock_client1(void);
int dlong_lock_client2(void);

int* dlong_lock_local;

int dlong_lock_client1(void){
    DLong l1;

    //Cloud::NewInstance();

    int ret = DLong::Map(l1, std::string("11"));
    EXPECT_EQ(ret, ERR_OK);

    l1.Lock();
    l1 = 10;    // Initial value
    *dlong_lock_local = 10;

    sleep(1);

    EXPECT_EQ(*dlong_lock_local, 10);
    EXPECT_EQ(l1.Value(), 10);
    l1.Unlock();

    printf("Client 1 finished\n");
    exit(0);
}

int dlong_lock_client2(void){
    DLong l1;

    //Cloud::NewInstance();

    int ret = DLong::Map(l1, std::string("11"));
    EXPECT_EQ(ret, ERR_OK);

    l1.Lock();
    l1 = 5; // Initial value
    *dlong_lock_local = 5;

    sleep(1);

    EXPECT_EQ(*dlong_lock_local, 5);
    EXPECT_EQ(l1.Value(), 5);
    l1.Unlock();

    printf("Client 2 finished\n");
    exit(0);
}


int main(void){
    DLong l1;
    DLong l2;
//    int ret;
    int status;

   
    dlong_lock_local = (int*) mmap(NULL, sizeof(*dlong_lock_local), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    assert(dlong_lock_local);

//    ret = DLong::Map(l1, std::string("11"));
//    EXPECT_EQ(ret, ERR_OK);
//    l1 = 42;


    if(0 == fork()){
        // Fork client 1
        dlong_lock_client1();

    }else if(0 == fork()){
        // Fork client 2
        dlong_lock_client2();
    }

    wait(&status);
    assert(WIFSIGNALED(status)==0);
    wait(&status);
    assert(WIFSIGNALED(status)==0);

    printf("Lock test passed!\n");
}





