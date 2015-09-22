// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 3 -*-

#include "storage/cloud.h"
#include "includes/data_types.h"
#include "test.h"

// Tests the transactions
// Creates a R/W conflict and checks that one of them aborts and the other commits
// 
// Run this process
// Sucess if both clients print that the test passed
//
// Usage:
//   $ ./test8
// 

#define TEST_NAME "Test 8"

void* thread1(void* arg);
void* thread2(void* arg);

int shared;

void* thread1(void* arg ){
    DLong l1;


    int ret = DLong::Map(l1, std::string("11"));
    EXPECT_EQ(ret, ERR_OK);


    DObject::TransactionBegin();

    l1 = 10;    // Initial value

    sleep(2);

    l1 = 11;

    EXPECT_EQ(l1.Value(), 11);

    int committed = DObject::TransactionCommit();
    EXPECT_EQ(committed, true);

    printf("Client 1 (%s) passed the test!\n", TEST_NAME);
    return 0;
}

void* thread2(void *arg){
    DLong l1;


    int ret = DLong::Map(l1, std::string("11"));
    EXPECT_EQ(ret, ERR_OK);

    DObject::TransactionBegin();

    sleep(1);
    long value = l1.Value();
    sleep(1);

    int committed = DObject::TransactionCommit();
    EXPECT_EQ(committed, false);

    printf("Client 2 (%s) passed the test (value=%ld)!\n", TEST_NAME, value);
    return 0;
}


int main(void){
    DLong l1;
    int ret;
    pthread_t t1,t2;
   
    ret = DLong::Map(l1, std::string("11"));
    //EXPECT_EQ(ret, ERR_OK);
    l1 = 0;
    shared = 0;

    ret = pthread_create(&t1, NULL, thread1, NULL);
    assert(ret == 0);

    ret = pthread_create(&t2, NULL, thread2, NULL);
    assert(ret == 0);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("%s passed the test!\n", TEST_NAME);
}





