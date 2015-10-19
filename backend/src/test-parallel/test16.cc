// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 3 -*-

#include "storage/cloud.h"
#include "includes/data_types.h"
#include "test.h"

// Tests the prefetching  
// 
// Run this process
// Sucess if both clients print that the test passed
//
// Usage:
//   $ ./test16
// 

#define TEST_NAME "Test 16"

void* thread1(void* arg);
void* thread2(void* arg);

DLong a,b,c;

enum txResult thread1TxABwrite(void * arg){
    DObject::TransactionOptionLearnPrefetchSet(true);
    a = 1;
    b = 2;
    c = 3;
    return COMMIT;
}


enum txResult thread1TxA(void * arg){

    DObject::TransactionOptionLearnPrefetchSet(true);
    int local_a = a.Value();


    printf("Tx1: a=%d, b=---\n", local_a);
    return COMMIT;
}

enum txResult thread1TxAB(void * arg){
    DObject::TransactionOptionLearnPrefetchSet(true);

    int local_a = a.Value();
    int local_b = b.Value();

    printf("Tx1: a=%d, b=%d\n", local_a, local_b);

    return COMMIT;
}

enum txResult thread1TxABC(void * arg){
    DObject::TransactionOptionLearnPrefetchSet(true);

    int local_a = a.Value();
    int local_b = b.Value();
    int local_c = c.Value();

    printf("Tx1: a=%d, b=%d, c=%d\n", local_a, local_b, local_c);

    return COMMIT;
}


void* thread1(void* arg ){
    bool committed;

    committed = DObject::TransactionExecute(thread1TxABwrite, NULL, 4);
    EXPECT_TRUE(committed);

    int i;
    for(i=0;i<3;i++){
        committed = DObject::TransactionExecute(thread1TxA, NULL, 4);
        EXPECT_TRUE(committed);
    }

    for(i=0;i<3;i++){
        committed = DObject::TransactionExecute(thread1TxAB, NULL, 4);
        EXPECT_TRUE(committed);
    }

    for(i=0;i<3;i++){
        committed = DObject::TransactionExecute(thread1TxABC, NULL, 4);
        EXPECT_TRUE(committed);
    }


    printf("Client 1 (%s) passed the test !\n", TEST_NAME);
    return 0;
}




void* thread2(void *arg){
    printf("Client 2 (%s) passed the test!\n", TEST_NAME);
    return 0;
}


int main(void){
    int ret;
    pthread_t t1,t2;
    
    DiamondInit();

    ret = DLong::Map(a, std::string("a123"));
    ret = DLong::Map(b, std::string("b123"));
    ret = DLong::Map(c, std::string("c123"));


    ret = pthread_create(&t1, NULL, thread1, NULL);
    assert(ret == 0);

    ret = pthread_create(&t2, NULL, thread2, NULL);
    assert(ret == 0);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("%s passed the test!\n", TEST_NAME);
}





