// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 3 -*-

#include "storage/cloud.h"
#include "includes/data_types.h"
#include "test.h"

// Tests the transactions
// 
// Run this process
// Sucess if both clients print that the test passed
//
// Usage:
//   $ ./test13
// 

#define TEST_NAME "Test 13"

void* thread1(void* arg);
void* thread2(void* arg);

DLong a,b;


enum txFinishAction thread1Tx(void * arg){
    
    if(a.Value() != 1){
        return RETRY;
    }

    return COMMIT;
}

void* thread1(void* arg ){
    bool committed = DObject::TransactionExecute(thread1Tx, NULL, 4);
    EXPECT_TRUE(committed);

    printf("Client 1 (%s) passed the test !\n", TEST_NAME);
    return 0;
}




enum txFinishAction thread2Tx(void * arg){
    b = 1;
    return COMMIT;
}



void* thread2(void *arg){
    bool committed;

    sleep(2);

    committed = DObject::TransactionExecute(thread2Tx, NULL, 2);
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
    ret = DLong::Map(b, std::string("a123"));

    ret = pthread_create(&t1, NULL, thread1, NULL);
    assert(ret == 0);

    ret = pthread_create(&t2, NULL, thread2, NULL);
    assert(ret == 0);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("%s passed the test!\n", TEST_NAME);
}





