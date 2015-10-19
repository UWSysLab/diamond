// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 3 -*-

#include "storage/cloud.h"
#include "includes/data_types.h"
#include "test.h"

// Tests the performance of stale reads 
// 
// Run this process
// Sucess if both clients print that the test passed
//
// Usage:
//   $ ./test15
// 

#define TEST_NAME "Test 15"

void* thread1(void* arg);
void* thread2(void* arg);

DLong a,b;

enum txResult thread1TxABwrite(void * arg){
    a = 1;
    b = 2;
    return COMMIT;
}


enum txResult thread1TxA(void * arg){
    int local_a = a.Value();

    printf("Tx1: a=%d, b=---\n", local_a);
    return COMMIT;
}

enum txResult thread1TxAB(void * arg){
    int local_a = a.Value();
    int local_b = b.Value();

    printf("Tx1: a=%d, b=%d\n", local_a, local_b);

    return COMMIT;
}


void* thread1(void* arg ){
    bool committed;

    committed = DObject::TransactionExecute(thread1TxABwrite, NULL, 4);
    EXPECT_TRUE(committed);
    sleep(1);

    int i;
    for(i=0;i<10000;i++){
        committed = DObject::TransactionExecute(thread1TxA, NULL, 4);
        EXPECT_TRUE(committed);
    }
    sleep(1);

    for(i=0;i<10000;i++){
        committed = DObject::TransactionExecute(thread1TxAB, NULL, 4);
        EXPECT_TRUE(committed);
    }
    sleep(2);

    printf("Client 1 (%s) passed the test !\n", TEST_NAME);
    return 0;
}






void* thread2(void *arg){
    bool committed;

    printf("Client 2 (%s) passed the test!\n", TEST_NAME);
    return 0;
}


int main(void){
    int ret;
    pthread_t t1,t2;
    
    DiamondInit();

    ret = DLong::Map(a, std::string("a123"));
    ret = DLong::Map(b, std::string("b123"));

    DLong::SetGlobalStaleness(true);
    DLong::SetGlobalMaxStaleness(1000);

    ret = pthread_create(&t1, NULL, thread1, NULL);
    assert(ret == 0);

    ret = pthread_create(&t2, NULL, thread2, NULL);
    assert(ret == 0);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("%s passed the test!\n", TEST_NAME);
}





