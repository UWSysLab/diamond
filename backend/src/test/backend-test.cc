// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 3 -*-
/***********************************************************************
 *
 * kvstore-test.cc:
 *   test cases for simple key-value store class
 *
 * Copyright 2015 Irene Zhang  <iyzhang@cs.washington.edu>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************************/

#include "storage/cloud.h"
#include "includes/data_types.h"
#include <semaphore.h>

#include <map>

#include <gtest/gtest.h>

using namespace diamond;

extern Cloud* cloudstore;

TEST(DString, Map)
{
    DString s1;
    DString s2;

    int ret = DString::Map(s1, std::string("10"));
    s1 = std::string("Hello");
    
    ret = DString::Map(s2, std::string("10"));

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(s1.Value(), s2.Value());

    s2 = std::string("Goodbye");

    EXPECT_EQ(s1.Value(), s2.Value());
}

TEST(DLong, Map) {
    DLong l1;
    DLong l2;

    int ret = DLong::Map(l1, std::string("11"));
    l1 = 42;

    ret = DLong::Map(l2, std::string("11"));

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(l1.Value(), l2.Value());

    l2 = 0;

    EXPECT_EQ(l1.Value(), l2.Value());
}

TEST(DCounter, Map) {
    DCounter c1;
    DCounter c2;

    int ret = DCounter::Map(c1, std::string("12"));
    ++c1;

    ret = DCounter::Map(c2, std::string("12"));

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(c1.Value(), c2.Value());

    ++c2;

    EXPECT_EQ(c1.Value(), c2.Value());
}

TEST(DSet, Map) {
    DSet set1;
    DSet set2;

    int ret = DSet::Map(set1, std::string("13"));
    set1.Clear();
    set1.Add(33);

    ret = DSet::Map(set2, std::string("13"));

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(set2.InSet(33), true);

    set2.Add(7);

    EXPECT_EQ(set1.InSet(7), true);

    set1.Remove(7);
    EXPECT_EQ(set2.InSet(7), false);

    set1.Clear();
    EXPECT_EQ(set2.Members().size(), 0);
}

TEST(DList, Map) {
    DList list1;
    DList list2;

    int ret = DList::Map(list1, std::string("14"));
    list1.Clear();
    list1.Append(4);
    list1.Append(9);

    ret = DList::Map(list2, std::string("14"));

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(list2.Index(9), 1);
    EXPECT_EQ(list2.Value(0), 4);

    list2.Append(9);
    list2.Append(25);

    EXPECT_EQ(list1.Index(25), 3);
    EXPECT_EQ(list1.Index(9), 1);

    list1.Insert(2, 16);

    EXPECT_EQ(list2.Index(16), 2);
    EXPECT_EQ(list2.Index(25), 4);
    EXPECT_EQ(list2.Value(3), 9);

    EXPECT_EQ(list2.Members().size(), list2.Size()); 
    EXPECT_EQ(list2.Members().at(1), list2.Value(1));
    EXPECT_EQ(list2.Members().at(2), list2.Value(2));

    list1.Erase(2);
    list1.Remove(9);
    EXPECT_EQ(list2.Index(25), 2);

    list1.Clear();
    EXPECT_EQ(list2.Size(), 0);
}

TEST(DStringList, Map) {
    DStringList list1;
    DStringList list2;

    int ret = DStringList::Map(list1, std::string("15"));
    list1.Clear();
    list1.Append("A");
    list1.Append("B");

    ret = DStringList::Map(list2, std::string("15"));

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(list2.Index("B"), 1);
    EXPECT_EQ(list2.Value(0), "A");

    list2.Append("B");
    list2.Append("D");

    EXPECT_EQ(list1.Index("D"), 3);
    EXPECT_EQ(list1.Index("B"), 1);

    list1.Insert(2, "C");

    EXPECT_EQ(list2.Index("C"), 2);
    EXPECT_EQ(list2.Index("D"), 4);
    EXPECT_EQ(list2.Value(3), "B");

    EXPECT_EQ(list2.Members().size(), list2.Size()); 
    EXPECT_EQ(list2.Members().at(1), list2.Value(1));
    EXPECT_EQ(list2.Members().at(2), list2.Value(2));

    list1.Erase(2);
    list1.Remove("B");
    EXPECT_EQ(list2.Index("D"), 2);

    list1.Clear();
    EXPECT_EQ(list2.Size(), 0);
}

TEST(DString, EmptyStrings) {
    DString s1, s2;

    int ret = DString::Map(s1, std::string("s"));
    s1 = "hi";

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(s1.Value(), "hi");

    ret = DString::Map(s2, std::string("s"));

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(s2.Value(), "hi");

    s1 = "";
    
    EXPECT_EQ(s1.Value(), "");
    EXPECT_EQ(s2.Value(), "");
}

TEST(DObject, MultiMap) {
    DString sa1, sa2;
    DString sb1, sb2;
    DLong long1, long2;
    DList list1, list2;

    std::vector<std::string> keys;
    keys.push_back("sa");
    keys.push_back("sb");
    keys.push_back("long");
    keys.push_back("list");
    std::vector<DObject *> objects1;
    objects1.push_back(&sa1);
    objects1.push_back(&sb1);
    objects1.push_back(&long1);
    objects1.push_back(&list1);

    int ret = DObject::MultiMap(keys, objects1);
    sa1 = "hello";
    sb1 = "world";
    long1 = 42;
    list1.Append(4);
    list1.Append(9);
    
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(sa1.Value(), "hello");
    EXPECT_EQ(sb1.Value(), "world");
    EXPECT_EQ(long1.Value(), 42);
    EXPECT_EQ(list1.Value(0), 4);
    EXPECT_EQ(list1.Value(1), 9);

    std::vector<DObject *> objects2;
    objects2.push_back(&sa2);
    objects2.push_back(&sb2);
    objects2.push_back(&long2);
    objects2.push_back(&list2);
    ret = DObject::MultiMap(keys, objects2);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(sa2.Value(), "hello");
    EXPECT_EQ(sb2.Value(), "world");
    EXPECT_EQ(long2.Value(), 42);
    EXPECT_EQ(list2.Value(0), 4);
    EXPECT_EQ(list2.Value(1), 9);

    sa1 = "";
    sb1 = "";
    list1.Clear();
    
    EXPECT_EQ(sa1.Value(), "");
    EXPECT_EQ(sb1.Value(), "");
    EXPECT_EQ(list1.Size(), 0);

    EXPECT_EQ(sa2.Value(), "");
    EXPECT_EQ(sb2.Value(), "");
    EXPECT_EQ(list2.Size(), 0);
}

// int dlong_wait_local = 0;
// 
// void* dlong_wait_client1_thread(void *v){
//     DLong l1;
//     sem_t *sem = (sem_t*) v;
// 
//     int ret = DLong::Map(l1, std::string("11"));
//     EXPECT_EQ(ret, ERR_OK);
// 
//     l1.Lock();
//     l1 = 10;    // Initial value
//     dlong_wait_local = 10;
// 
//     sem_post(sem);
//     sleep(1);
// 
//     l1 = 20;    // New value
//     dlong_wait_local = 20;
//     l1.Signal();
//     l1.Unlock();
// 
//     return 0;
// }
// 
// void* dlong_wait_client2_thread(void *v){
//     DLong l1;
//     sem_t *sem = (sem_t*) v;
// 
//     int ret = DLong::Map(l1, std::string("11"));
//     EXPECT_EQ(ret, ERR_OK);
// 
// 
//     sem_wait(sem);
//     l1.Lock();
//     EXPECT_EQ(dlong_wait_local, 10); // Expect initial value
//     EXPECT_EQ(l1.Value(), 10); // Expect initial value
// 
//     l1.Wait();
//     l1.Unlock();
// 
//     EXPECT_EQ(dlong_wait_local, 20) << "ERROR: Client 2 did not wait for the new value"; // Expect new value
//     EXPECT_EQ(l1.Value(), 20) << "ERROR: Client 2 did not wait for the new value"; // Expect new value
// 
//     return 0;
// }
// 
// 
// TEST(DLong, Wait) {
//     DLong l1;
//     DLong l2;
//     pthread_t thread1, thread2; 
//     void *status;
//     sem_t sem;
//     int ret;
// 
//     ret = sem_init(&sem, 0, 0);
//     EXPECT_EQ(ret, 0);
//  
//     ret = DLong::Map(l1, std::string("11"));
//     EXPECT_EQ(ret, ERR_OK);
// 
//     l1 = 42;
// 
//     pthread_create (&thread1, NULL,  &dlong_wait_client1_thread, (void *) &sem);
//     pthread_create (&thread2, NULL,  &dlong_wait_client2_thread, (void *) &sem);
// 
//     pthread_join(thread1, &status);
//     pthread_join(thread2, &status);
// }



// int dlong_lock_local = 0;
// void* dlong_lock_client1_thread(void *v){
//     DLong l1;
//     sem_t *sem = (sem_t*) v;
// 
//     int ret = DLong::Map(l1, std::string("11"));
//     EXPECT_EQ(ret, ERR_OK);
// 
//     l1.Lock();
//     l1 = 10;    // Initial value
//     dlong_lock_local = 10;
// 
//     sleep(1);
// 
//     EXPECT_EQ(dlong_lock_local, 10);
//     EXPECT_EQ(l1.Value(), 10);
//     l1.Unlock();
// 
//     return 0;
// }
// 
// void* dlong_lock_client2_thread(void *v){
//     DLong l1;
//     sem_t *sem = (sem_t*) v;
// 
//     int ret = DLong::Map(l1, std::string("11"));
//     EXPECT_EQ(ret, ERR_OK);
// 
//     l1.Lock();
//     l1 = 5; // Initial value
//     dlong_lock_local = 5;
// 
//     sleep(1);
// 
//     EXPECT_EQ(dlong_lock_local, 5);
//     EXPECT_EQ(l1.Value(), 5);
//     l1.Unlock();
// 
//     return 0;
// }
// 
// 
// TEST(DLong, Lock) {
// 
//     DLong l1;
//     DLong l2;
//     pthread_t thread1, thread2; 
//     void *status;
//     sem_t sem;
//     int ret;
// 
//     ret = sem_init(&sem, 0, 0);
//     EXPECT_EQ(ret, 0);
//     
//  //   ret = DLong::Map(l1, std::string("11"));
//  //   EXPECT_EQ(ret, ERR_OK);
//  //   l1 = 42;
// 
//     pthread_create (&thread1, NULL,  &dlong_lock_client1_thread, (void *) &sem);
//     pthread_create (&thread2, NULL,  &dlong_lock_client2_thread, (void *) &sem);
// 
//     pthread_join(thread1, &status);
//     pthread_join(thread2, &status);
// 
// 
// }
// 




