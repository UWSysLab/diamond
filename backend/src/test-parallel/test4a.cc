// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 3 -*-

#include "storage/cloud.h"
#include "includes/data_types.h"
#include "test.h"

// This client (test4a) tests Wait() while the other client (test4b) tests Broadcast()
// 
// Run three clients (two test4a and one test4b) at the same time (within a one second window)
// Success if all three clients print that the test passed
//
// Usage:
//   $ ./test4a & ./test4a & ./test4b & wait ; wait ; wait
// 



int main(void){
    DLong l1;
    int iterations = 0;

    int ret = DLong::Map(l1, std::string("11"));
    EXPECT_EQ(ret, ERR_OK);

    l1.Lock();

    sleep(1);

    while (l1.Value() != 123456){
        iterations++;
        l1.Wait();
    }

    l1.Unlock();

    EXPECT_EQ(iterations, 1);
    printf("Client test4a (%d) passed the test! (%d iterations)\n", getpid(), iterations);
}




