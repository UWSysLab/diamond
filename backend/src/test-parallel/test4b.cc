// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 3 -*-

#include "storage/cloud.h"
#include "includes/data_types.h"
#include "test.h"


// The other client (test4a) tests Wait() while this client (test4b) tests Broadcast()
// 
// Run three clients (two test4a and one test4b) at the same time (within a one second window)
// Success if all three clients print that the test passed
//
// Usage:
//   $ ./test4a & ./test4a & ./test4b & wait ; wait ; wait
// 



int main(void){
    DiamondInit();

    DLong l1;

    int ret = DLong::Map(l1, std::string("11"));
    EXPECT_EQ(ret, ERR_OK);

    sleep(2);

    l1.Lock();

    l1 = 123456;
    l1.Broadcast();

    l1.Unlock();

    printf("Client test3b (%d) passed the test!\n", getpid());
}





