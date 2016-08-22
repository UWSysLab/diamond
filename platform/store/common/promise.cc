// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
// vim: set ts=4 sw=4:
/***********************************************************************
 *
 * store/common/promise.h
 *   Simple promise implementation.
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

#include "promise.h"

using namespace std;

Promise::Promise()
{ 
    done = false;
    reply = 0;
    timeout = 1000;
}

Promise::Promise(int timeoutMS) 
{ 
    done = false;
    reply = 0;
    timeout = timeoutMS;
}

Promise::~Promise() { }

// Get configured timeout, return after this period
int
Promise::GetTimeout()
{
    return timeout;
}

// Functions for replying to the promise
void
Promise::ReplyInternal(int r)
{
    done = true;
    reply = r;
    cv.notify_all();
}

void
Promise::Reply(int r)
{
    lock_guard<mutex> l(lock);
    ReplyInternal(r);
}

void
Promise::Reply(int r, Timestamp t)
{
    lock_guard<mutex> l(lock);
    timestamp = t;
    ReplyInternal(r);
}

void
Promise::Reply(int r,
               const string &k,
               const Version &v)
{
    lock_guard<mutex> l(lock);
    values.insert(make_pair(k, v));
    ReplyInternal(r);
}

void
Promise::Reply(int r,
               map<string, Version> &v)
{
    lock_guard<mutex> l(lock);
    values = v;
    ReplyInternal(r);
}

void
Promise::Reply(int r,
               Timestamp t,
               map<string, Version> &v)
{
    lock_guard<mutex> l(lock);
    timestamp = t;
    values = v;
    ReplyInternal(r);
}

void
Promise::Reply(int r, Timestamp t,
               map<string,
               Version> &v,
               uint64_t rid)
{
    lock_guard<mutex> l(lock);
    values = v;
    timestamp = t;
    reactive_id = rid;
    ReplyInternal(r);
}

// Function for checking status
bool
Promise::GetDone()
{
    lock_guard<mutex> l(lock);
    return done;
}


// Functions for getting a reply from the promise
int
Promise::GetReply()
{
    unique_lock<mutex> l(lock);
    while(!done) {
        cv.wait(l);
    }
    return reply;
}

const Timestamp
Promise::GetTimestamp()
{
    unique_lock<mutex> l(lock);
    while(!done) {
        cv.wait(l);
    }
    return timestamp;
}

const Version &
Promise::GetValue(const string &key)
{
    unique_lock<mutex> l(lock);
    while(!done) {
        cv.wait(l);
    }
    return values[key];
}

const map<string, Version> &
Promise::GetValues()
{
    unique_lock<mutex> l(lock);
    while(!done) {
        cv.wait(l);
    }
    return values;
}

const uint64_t
Promise::GetReactiveId()
{
    unique_lock<mutex> l(lock);
    while(!done) {
        cv.wait(l);
    }
    return reactive_id;
}
