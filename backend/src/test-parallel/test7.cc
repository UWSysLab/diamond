// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 3 -*-

#include "storage/cloud.h"
#include "includes/data_types.h"
#include "test.h"

// Tests the release consistency
// 
// Run this process
// Sucess if both clients print that the test passed
//
// Usage:
//   $ ./test7
// 

#define TEST_NAME "Test 7"

void* thread1(void* arg);
void* thread2(void* arg);

int shared;

void* thread1(void* arg ){
    DLong l1;


    int ret = DLong::Map(l1, std::string("11"));
    EXPECT_EQ(ret, ERR_OK);


    l1.Lock();

    sleep(2);
    l1 = 10;    // Initial value
    shared = 10;

    sleep(2);

    EXPECT_EQ(shared, 10);
    EXPECT_EQ(l1.Value(), 10);
    l1.Unlock();

    printf("Client 1 (%s) passed the test!\n", TEST_NAME);
    return 0;
}

void* thread2(void *arg){
    DLong l1,l2;


    int ret = DLong::Map(l1, std::string("11"));
    EXPECT_EQ(ret, ERR_OK);

    sleep(1);
    EXPECT_EQ(l1.Value(), 0); // Shouldn't have seen the initial value yet
    sleep(5);
    EXPECT_EQ(l1.Value(), 0); // Shouldn't have seen the initial value yet

    
    l1.Lock();

    EXPECT_EQ(l1.Value(), 10);

    l1 = 20;
    EXPECT_EQ(l1.Value(), 20);


    l1.Unlock();

    printf("Client 2 (%s) passed the test!\n", TEST_NAME);
    return 0;
}


int main(void){
    DiamondInit();

    DLong l1;
    int ret;
    pthread_t t1,t2;
   
    ret = DLong::Map(l1, std::string("11"));
    EXPECT_EQ(ret, ERR_OK);
    l1 = 0;
    shared = 0;

    l1.SetGlobalConsistency(RELEASE_CONSISTENCY);

    ret = pthread_create(&t1, NULL, thread1, NULL);
    assert(ret == 0);

    ret = pthread_create(&t2, NULL, thread2, NULL);
    assert(ret == 0);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("%s passed the test!\n", TEST_NAME);
}





