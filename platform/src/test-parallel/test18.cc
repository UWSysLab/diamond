// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 3 -*-

#include "storage/cloud.h"
#include "includes/data_types.h"
#include "test.h"

// Tests the prefetching  
// 
// Run this process
// Sucess if both clients print that the test passed
// Note: Should also run "redis-cli MONITOR" and check that there were 
//   multi gets and multi watches issued for the last 3 TXs
//
// Usage:
//   $ ./test17
// 

#define TEST_NAME "Test 17"

void* thread1(void* arg);
void* thread2(void* arg);

DStringList l;


/*
        while(committed == 0) {
            writeTimeStart = System.nanoTime();
            Diamond.DObject.TransactionBegin();
            messageList.Append(fullMsg);
            if (messageList.Size() > MESSAGE_LIST_SIZE) {
                messageList.Erase(0);
            }
            committed = Diamond.DObject.TransactionCommit();
            if (committed == 0) {
                numAborts++;
            }
        }
*/

enum txFinishAction thread1TxABwrite(void * arg){
    std::string a = "a";
    l.Append(a);
    if(l.Size() > 10){
        l.Erase(0);
    }
    return COMMIT;
}


void* thread1(void* arg ){
    bool committed;

    committed = DObject::TransactionExecute(thread1TxABwrite, NULL, 4);
    EXPECT_TRUE(committed);

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

    ret = DLong::Map(l, "key1");

    printf("After map\n");

    ret = pthread_create(&t1, NULL, thread1, NULL);
    assert(ret == 0);

    ret = pthread_create(&t2, NULL, thread2, NULL);
    assert(ret == 0);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("%s passed the test!\n", TEST_NAME);
}





