// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
/***********************************************************************
 *
 * configuration.cc:
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

#include "lib/assert.h"
#include "replication/common/configuration.h"
#include "lib/message.h"

#include <iostream>
#include <fstream>
#include <string>
#include <string.h>

namespace replication {

ReplicaConfig::ReplicaConfig(const ReplicaConfig &c)
    : Configuration(c.hosts), f(c.f)
{
}
    
ReplicaConfig::ReplicaConfig(int n, int f,
                             std::vector<transport::HostAddress> hosts)
    : Configuration(hosts), f(f)
{
}

ReplicaConfig::ReplicaConfig(std::ifstream &file)
    : Configuration(file)
{
    if (n == 0) {
        Panic("ReplicaConfig did not specify any hosts");
    }
    f = (n-1)/2;
}

ReplicaConfig::~ReplicaConfig()
{
}

int
ReplicaConfig::GetLeaderIndex(view_t view) const
{
    return (view % this->n);
}

int
ReplicaConfig::QuorumSize() const
{
    return f+1;
}

int
ReplicaConfig::FastQuorumSize() const
{
    return f + (f+1)/2 + 1;
}


} // namespace replication
