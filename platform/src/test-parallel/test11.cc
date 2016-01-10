// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 3 -*-

#include "storage/cloud.h"
#include "includes/data_types.h"
#include "test.h"

// Tests the retry of the transactions
// 
// Run this process
// Sucess if both clients print that the test passed
//
// Usage:
//   $ ./test11
// 

#define TEST_NAME "Test 11"

void* thread1(void* arg);
void* thread2(void* arg);

DLong a,b;

void* thread1(void* arg ){
    int tx_attempts = 0;
    int committed = 0;

    sleep(1);

    while(!committed){
        EXPECT_TRUE(tx_attempts<10);
        tx_attempts++;

        DObject::TransactionBegin();

        a = 10;    
        EXPECT_EQ(a.Value(), 10);
        sleep(2);
        b = 11;
        EXPECT_EQ(b.Value(), 11);

        committed = DObject::TransactionCommit();
        sleep(0);
    }

    printf("Client 1 (%s) passed the test (tx_attempts=%d)!\n", TEST_NAME, tx_attempts);
    return 0;
}

void* thread2(void *arg){
    int tx_attempts = 0;
    int committed = 0;

    while(!committed){
        EXPECT_TRUE(tx_attempts<10);
        tx_attempts++;

        DObject::TransactionBegin();

        if(a.Value() != 10){ 
            printf("Retrying\n");
            DObject::TransactionRetry();
            
            continue;
        }

        a = 10;    
        EXPECT_EQ(a.Value(), 10);
        sleep(2);
        b = 11;
        EXPECT_EQ(b.Value(), 11);

        committed = DObject::TransactionCommit();
        sleep(0);
    }

    printf("Client 2 (%s) passed the test (tx_attempts=%d)!\n", TEST_NAME, tx_attempts);
    return 0;
}


int main(void){
    int ret;
    pthread_t t1,t2;
    
    DiamondInit();

    ret = DLong::Map(a, std::string("a"));
    ret = DLong::Map(b, std::string("b"));
    //EXPECT_EQ(ret, ERR_OK);
    a = 0;
    b = 0;

    ret = pthread_create(&t1, NULL, thread1, NULL);
    assert(ret == 0);

    ret = pthread_create(&t2, NULL, thread2, NULL);
    assert(ret == 0);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("%s passed the test!\n", TEST_NAME);
}





