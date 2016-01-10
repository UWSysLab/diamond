// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 3 -*-

#include "storage/cloud.h"
#include "includes/data_types.h"
#include "test.h"

// Tests the transactions
// Uses one thread to test the commit and the rollback
// 
//
// Usage:
//   $ ./test9
// 

#define TEST_NAME "Test 9"


int main(void){
    DiamondInit();

    DLong l1, l1_2, l2;

    DLong::Map(l1, std::string("11"));
    DLong::Map(l1_2, std::string("11"));
    DLong::Map(l2, std::string("12"));
    l1 = 0;
    l2 = 0;


    // Transaction 1
    DObject::TransactionBegin();

    l1 = 10;    // Initial value
    EXPECT_EQ(l1.Value(), 10);
    EXPECT_EQ(l1_2.Value(), 10);

    l1 = 11;
    EXPECT_EQ(l1.Value(), 11);

    l2 = 7;
    EXPECT_EQ(l2.Value(), 7);

    DObject::TransactionRollback();

    EXPECT_EQ(l1.Value(), 0);
    EXPECT_EQ(l1_2.Value(), 0);
    EXPECT_EQ(l2.Value(), 0);


    // Write outside of TX should be atomic
    l1 = 5;    
    EXPECT_EQ(l1.Value(), 5);


    // Transaction 2
    DObject::TransactionBegin();

    l1 = 10;    // Initial value
    EXPECT_EQ(l1.Value(), 10);
    EXPECT_EQ(l1_2.Value(), 10);

    l1 = 11;
    EXPECT_EQ(l1.Value(), 11);

    l2 = 7;
    EXPECT_EQ(l2.Value(), 7);


    int committed = DObject::TransactionCommit();
    EXPECT_EQ(committed, true);

    EXPECT_EQ(l1.Value(), 11);
    EXPECT_EQ(l1_2.Value(), 11);
    EXPECT_EQ(l2.Value(), 7);



    printf("%s passed the test!\n", TEST_NAME);
}





