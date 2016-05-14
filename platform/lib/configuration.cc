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
#include "lib/configuration.h"
#include "lib/message.h"

#include <iostream>
#include <fstream>
#include <string>
#include <string.h>

namespace transport {

HostAddress::HostAddress(const string &host, const string &port)
    : host(host), port(port)
{

}

bool
HostAddress::operator==(const HostAddress &other) const {
    return ((host == other.host) &&
            (port == other.port));
}

Configuration::Configuration(std::ifstream &file)
{
    f = -1;

    while (!file.eof()) {
        // Read a line
        string line;
        getline(file, line);;

        // Ignore comments
        if ((line.size() == 0) || (line[0] == '#')) {
            continue;
        }

        // Get the command
        // This is pretty horrible, but C++ does promise that &line[0]
        // is going to be a mutable contiguous buffer...
        char *cmd = strtok(&line[0], " \t");

        if (strcasecmp(cmd, "host") == 0) {
            char *arg = strtok(NULL, " \t");
            if (!arg) {
                Panic ("'host' configuration line requires an argument");
            }

            char *host = strtok(arg, ":");
            char *port = strtok(NULL, "");
            
            if (!host || !port) {
                Panic("Configuration line format: 'host host:port'");
            }

            hosts.push_back(HostAddress(string(host), string(port)));
        } else {
            Panic("Unknown configuration directive: %s", cmd);
        }
    }
}

Configuration::~Configuration()
{
}

HostAddress
Configuration::host(int idx) const
{
    return hosts[idx];
}

bool
Configuration::operator==(const Configuration &other) const
{
    return hosts == other.hosts;
    
}

} // namespace transport
