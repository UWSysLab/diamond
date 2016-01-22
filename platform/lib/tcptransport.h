// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
/***********************************************************************
 *
 * tcptransport.h:
 *   message-passing network interface that uses TCP message delivery
 *   and libasync
 *
 * Copyright 2013 Dan R. K. Ports  <drkp@cs.washington.edu>
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

#ifndef _LIB_TCPTRANSPORT_H_
#define _LIB_TCPTRANSPORT_H_

#include "common/configuration.h"
#include "lib/transport.h"

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include <map>
#include <list>
#include <netinet/in.h>

class TCPtransportAddress : public TransportAddress
{
public:
    TCPtransportAddress * clone() const;
private:
    TCPtransportAddress(const sockaddr_in &addr);
    sockaddr_in addr;
    friend class TCPtransport;
    friend bool operator==(const TCPtransportAddress &a,
                           const TCPtransportAddress &b);
    friend bool operator!=(const TCPtransportAddress &a,
                           const TCPtransportAddress &b);
};

class TCPtransport : public Transport
{
public:
    TCPtransport(const transport::Configuration &config);
    virtual ~TCPtransport();
    void Register(TransportReceiver *receiver);
    bool SendMessage(TransportReceiver *src, const TransportAddress &dst,
                     const Message &m);
    void Run();
    int Timer(uint64_t ms, timer_callback_t cb);
    bool CancelTimer(int id);
    void CancelAllTimers();
    
private:
    struct TCPtransportTimerInfo
    {
        TCPtransport *transport;
        timer_callback_t cb;
        event *ev;
        int id;
    };
    struct TCPtransportTCPListener
    {
        TCPtransport *transport;
        TransportReceiver *receiver;
        int acceptFd;
        int replicaIdx;
        event *acceptEvent;
        std::list<struct bufferevent *> connectionEvents;
    };
    specpaxos::Configuration config;
    event_base *libeventBase;
    std::vector<event *> listenerEvents;
    std::vector<event *> signalEvents;
    std::map<int, TransportReceiver*> receivers; // fd -> receiver
    int multicastFd;
    std::map<TransportReceiver*, int> fds; // receiver -> fd
    std::map<int, TCPtransportAddress> replicaAddresses; // idx -> addr
    TCPtransportAddress *multicastAddress;
    std::map<int, int> replicaFds;                       // idx -> fd
    int lastTimerId;
    std::map<int, TCPtransportTimerInfo *> timers;
    std::list<TCPtransportTCPListener *> tcpListeners;
    struct bufferevent * tcpOutgoing;

    TCPtransportAddress LookupAddress(const transport::ReplicaAddress &addr,
                                      int replicaIdx, bool tcp = false);
    void ConnectTCP(int replicaIdx);
    bool SendMessageTCP(TransportReceiver *src, int replicaIdx,
                        const Message &m);
    void OnReadable(int fd);
    void OnTimer(TCPtransportTimerInfo *info);
    static void SocketCallback(evutil_socket_t fd,
                               short what, void *arg);
    static void TimerCallback(evutil_socket_t fd,
                              short what, void *arg);
    static void LogCallback(int severity, const char *msg);
    static void FatalCallback(int err);
    static void SignalCallback(evutil_socket_t fd,
                               short what, void *arg);
    static void TCPAcceptCallback(evutil_socket_t fd, short what,
                                  void *arg);
    static void TCPReadableCallback(struct bufferevent *bev,
                                    void *arg);
    static void TCPEventCallback(struct bufferevent *bev,
                                 short what, void *arg);
    static void TCPIncomingEventCallback(struct bufferevent *bev,
                                         short what, void *arg);
    static void TCPOutgoingEventCallback(struct bufferevent *bev,
                                         short what, void *arg);
};

#endif  // _LIB_TCPTRANSPORT_H_
