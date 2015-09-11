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

// Ping-pong test for Lock/Unlock/Signal/Wait
// 
// Run this process
// Sucess if the clients print ping and pong in alternation
//
// Usage:
//   $ ./test6
// 


void* dlong_lock_client1(void* arg);
void* dlong_lock_client2(void* arg);

const int PING = 10;
const int PONG = 20;

void* dlong_lock_client1(void* arg ){
    DLong l1;

    int ret = DLong::Map(l1, std::string("test6::num"));
    EXPECT_EQ(ret, ERR_OK);

    while (true) {
        l1.Lock();
        while (l1.Value() == PING) {
            l1.Wait();
        }
        l1.Set(PING);
        l1.Signal();
        l1.Unlock();
        printf("ping\n");
    }

    return 0;
}

void* dlong_lock_client2(void *arg){
    DLong l1;

    int ret = DLong::Map(l1, std::string("test6::num"));
    EXPECT_EQ(ret, ERR_OK);

    while (true) {
        l1.Lock();
        while (l1.Value() == PONG) {
            l1.Wait();
        }
        l1.Set(PONG);
        l1.Signal();
        l1.Unlock();
        printf("pong\n");
    }

    return 0;
}


int main(void){
    DLong l1;
    int ret;
    pthread_t t1,t2;
   
    ret = DLong::Map(l1, std::string("11"));
    EXPECT_EQ(ret, ERR_OK);
    l1 = PING;

    ret = pthread_create(&t1, NULL, dlong_lock_client1, NULL);
    assert(ret == 0);

    ret = pthread_create(&t2, NULL, dlong_lock_client2, NULL);
    assert(ret == 0);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
}





