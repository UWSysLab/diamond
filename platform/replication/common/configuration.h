// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
/***********************************************************************
 *
 * configuration.h:
 *   Representation of a replica group configuration, i.e. the number
 *   and list of replicas in the group
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

#ifndef _LIB_CONFIGURATION_H_
#define _LIB_CONFIGURATION_H_

#include "replication/common/viewstamp.h"

#include <fstream>
#include <stdbool.h>
#include <string>
#include <vector>

using std::string;

namespace replication {

class Configuration : transport::Configuration
{
public:
    Configuration(const Configuration &c);
    Configuration(int n, int f, std::vector<HostAddress> replicas,
                  HostAddress *multicastAddress = nullptr) : hosts(replicas) { };
    Configuration(std::ifstream &file);
    virtual ~Configuration();
    HostAddress replica(int idx) const;
    const HostAddress *multicast() const;
    int GetLeaderIndex(view_t view) const;
    int QuorumSize() const;
    int FastQuorumSize() const;
    bool operator==(const Configuration &other) const;
    inline bool operator!= (const Configuration &other) const {
        return !(*this == other);
    }
    
public:
    int n;                      // number of replicas
    int f;                      // number of failures tolerated
private:
    HostAddress *multicastAddress;
    bool hasMulticast = false;
};

}      // namespace replication

namespace std {
template <> struct hash<transport::HostAddress>
{
    size_t operator()(const transport::HostAddress & x) const
        {
            return hash<string>()(x.host) * 37 + hash<string>()(x.port);
        }
};
}

namespace std {
template <> struct hash<transport::Configuration>
{
    size_t operator()(const transport::Configuration & x) const
        {
            size_t out = 0;
            out = x.n * 37 + x.f;
            for (int i = 0; i < x.n; i++) {
                out *= 37;
                out += hash<transport::HostAddress>()(x.replica(i));
            }
            return out;
        }
};
}


#endif  /* _LIB_CONFIGURATION_H_ */
