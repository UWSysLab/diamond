// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
/***********************************************************************
 *
 * transport-common.h:
 *   template support for implementing transports
 *
 * Copyright 2013-2015 Irene Zhang <iyzhang@cs.washington.edu>
 *                     Naveen Kr. Sharma <nksharma@cs.washington.edu>
 *                     Dan R. K. Ports  <drkp@cs.washington.edu>
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

#ifndef _LIB_TRANSPORTCOMMON_H_
#define _LIB_TRANSPORTCOMMON_H_

#include "lib/assert.h"
#include "lib/configuration.h"
#include "lib/transport.h"

#include <map>
#include <unordered_map>

template <typename ADDR>
class TransportCommon : public Transport
{
    
public:
    TransportCommon() 
    {
        hostAddressesInitialized = false;
    }

    virtual
    ~TransportCommon()
    {
        for (auto &kv : canonicalConfigs) {
            delete kv.second;
        }
    }
    
    virtual bool
    SendMessage(TransportReceiver *src, const TransportAddress &dst,
                const Message &m)
    {
        const ADDR &dstAddr = dynamic_cast<const ADDR &>(dst);
        return SendMessageInternal(src, dstAddr, m);
    }

    virtual bool
    SendMessageToHost(TransportReceiver *src, int hostIdx,
                         const Message &m)
    {
        const transport::Configuration *cfg = configurations[src];
        ASSERT(cfg != NULL);

        if (!hostAddressesInitialized) {
            LookupAddresses();
        }
        
        auto kv = hostAddresses[cfg].find(hostIdx);
        ASSERT(kv != hostAddresses[cfg].end());
        
        return SendMessageInternal(src, kv->second, m);
    }
    virtual int Timer(uint64_t ms, timer_callback_t cb) = 0;
    virtual bool CancelTimer(int id) = 0;
    virtual void CancelAllTimers() = 0;

protected:
    virtual bool SendMessageInternal(TransportReceiver *src,
                                     const ADDR &dst,
                                     const Message &m) = 0;
    virtual ADDR LookupAddress(const transport::Configuration &cfg,
                               int hostIdx) = 0;
    
    std::unordered_map<transport::Configuration,
                       transport::Configuration *> canonicalConfigs;
    std::map<TransportReceiver *,
             transport::Configuration *> configurations;
    std::map<const transport::Configuration *,
             std::map<int, ADDR> > hostAddresses;
    std::map<const transport::Configuration *,
             std::map<int, TransportReceiver *> > hostReceivers;
    bool hostAddressesInitialized;

    virtual transport::Configuration *
    RegisterConfiguration(TransportReceiver *receiver,
                          const transport::Configuration &config,
                          int hostIdx)
    {
        ASSERT(receiver != NULL);

        // Have we seen this configuration before? If so, get a
        // pointer to the canonical copy; if not, create one. This
        // allows us to use that pointer as a key in various
        // structures. 
        transport::Configuration *canonical = canonicalConfigs[config];
        if (canonical == NULL) {
            canonical = new transport::Configuration(config);
            canonicalConfigs[config] = canonical;
        }

        // Record configuration
        configurations.insert(std::make_pair(receiver, canonical));

        // If this is a host, record the receiver
        if (hostIdx != -1) {
            hostReceivers[canonical].insert(std::make_pair(hostIdx,
                                                             receiver));
        }

        // Mark hostAddreses as uninitalized so we'll look up
        // host addresses again the next time we send a message.
        hostAddressesInitialized = false;

        return canonical;
    }

    virtual void
    LookupAddresses()
    {
        // Clear any existing list of addresses
        hostAddresses.clear();
        
        // For every configuration, look up all addresses and cache
        // them.
        for (auto &kv : canonicalConfigs) {
            transport::Configuration *cfg = kv.second;

            for (int i = 0; i < cfg->n; i++) {
                const ADDR addr = LookupAddress(*cfg, i);
                hostAddresses[cfg].insert(std::make_pair(i, addr));
            }
        }
        
        hostAddressesInitialized = true;
    }

};

#endif // _LIB_TRANSPORTCOMMON_H_
