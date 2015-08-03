// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
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

#include "client/client.h"
#include "includes/data_types.h"

#include <gtest/gtest.h>

using namespace diamond;

extern Client diamondclient;

TEST(DString, Map)
{
    DString s1;
    DString s2;

    //diamondclient.Connect("seymour");
    int ret = DString::Map(s1, std::string("10"));
    s1 = std::string("Hello");
    
    ret = DString::Map(s2, std::string("10"));

    EXPECT_EQ(ret, RPC_OK);
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

    EXPECT_EQ(ret, RPC_OK);
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

    EXPECT_EQ(ret, RPC_OK);
    EXPECT_EQ(c1.Value(), c2.Value());

    ++c2;

    EXPECT_EQ(c1.Value(), c2.Value());
}

TEST(DSet, Map) {
    DSet set1;
    DSet set2;

    int ret = DSet::Map(set1, std::string("13"));
    set1.Add(33);

    ret = DSet::Map(set2, std::string("13"));

    EXPECT_EQ(ret, RPC_OK);
    EXPECT_EQ(set2.InSet(33), true);

    set2.Add(7);

    EXPECT_EQ(set1.InSet(7), true);

    set1.Remove(7);
    EXPECT_EQ(set2.InSet(7), false);
}
