// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * store/common/backend/comstore.cc:
 *   Versioned store with support for commutative operations
 *
 * Copyright 2015 Irene Zhang <iyzhang@cs.washington.edu>
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

#include "commstore.h"

using namespace std;

void
CommutativeStore::Increment(const string &key, const int inc, const Timestamp &t)
{
    set<Version>::iterator it;
    if (getValue(key, t, it)) {
        int total = atoi(it->GetValue().c_str()) + inc;
        char totalBuf[100];
        sprintf(totalBuf, "%d", total);
        string totalStr(totalBuf);
        store[key].insert(it, Version(t, totalStr, INCREMENT));
        while (++it != store[key].end()) {
            int total = atoi(it->GetValue().c_str()) + inc;
            char totalBuf[100];
            sprintf(totalBuf, "%d", total);
            string totalStr(totalBuf);
            Version v = *it;
            store[key].erase(it);
            v.SetValue(totalStr);
            store[key].insert(v);
        }
    } else {
        char incBuf[100];
        sprintf(incBuf, "%d", inc);
        string incStr(incBuf);
        store[key].insert(Version(t, incStr, INCREMENT));
    }
}
