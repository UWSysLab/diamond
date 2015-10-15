// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 3 -*-

#include "storage/cloud.h"
#include "includes/data_types.h"
#include "test.h"

// Tests the stale reads transactions
// 
// Run this process
// Sucess if both clients print that the test passed
//
// Usage:
//   $ ./test14
// 

#define TEST_NAME "Test 14"

void* thread1(void* arg);
void* thread2(void* arg);

DLong a,b;

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

enum txResult thread1TxB(void * arg){
    int local_b = b.Value();

    printf("Tx1: a=---, b=%d\n", local_b);

    return COMMIT;
}


void* thread1(void* arg ){
    bool committed;

    committed = DObject::TransactionExecute(thread1TxA, NULL, 4);
    EXPECT_TRUE(committed);
    sleep(1);


    committed = DObject::TransactionExecute(thread1TxAB, NULL, 4);
    EXPECT_TRUE(committed);
    sleep(1);

    committed = DObject::TransactionExecute(thread1TxAB, NULL, 4);
    EXPECT_TRUE(committed);
    sleep(2);
    committed = DObject::TransactionExecute(thread1TxAB, NULL, 4);
    EXPECT_TRUE(committed);

    printf("Client 1 (%s) passed the test !\n", TEST_NAME);
    return 0;
}




enum txResult thread2Tx1(void * arg){
    a = 1; 
    b = 1;
    return COMMIT;
}


enum txResult thread2Tx2(void * arg){
    a = 2;
    b = 3;
    return COMMIT;
}


enum txResult thread2Tx3(void * arg){
    b = 5;
    return COMMIT;
}



enum txResult thread2Tx4(void * arg){
    a = 4;
    return COMMIT;
}



void* thread2(void *arg){
    bool committed;

    sleep(2);

    committed = DObject::TransactionExecute(thread2Tx1, NULL, 2);
    EXPECT_TRUE(committed);

    committed = DObject::TransactionExecute(thread2Tx2, NULL, 2);
    EXPECT_TRUE(committed);

    committed = DObject::TransactionExecute(thread2Tx3, NULL, 2);
    EXPECT_TRUE(committed);

    committed = DObject::TransactionExecute(thread2Tx4, NULL, 2);
    EXPECT_TRUE(committed);



    sleep(2);
    b = 0;

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
    DLong::SetGlobalMaxStaleness(1500);

    ret = pthread_create(&t1, NULL, thread1, NULL);
    assert(ret == 0);

    ret = pthread_create(&t2, NULL, thread2, NULL);
    assert(ret == 0);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("%s passed the test!\n", TEST_NAME);
}





