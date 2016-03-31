#!/usr/bin/python

import argparse
import os
import re

WORKING_DIR = "/home/nl35"

dependencies = {} # map local full path to remote file name in working dir
dependencies["libprotobuf.so.9"] = "/usr/lib/x86_64-linux-gnu/libprotobuf.so.9"
dependencies["libevent_pthreads-2.0.so.5"] = "/usr/lib/x86_64-linux-gnu/libevent_pthreads-2.0.so.5"
dependencies["libevent_core-2.0.so.5"] = "/usr/lib/x86_64-linux-gnu/libevent_core-2.0.so.5"

parser = argparse.ArgumentParser(description='Copy dependencies to GCE machines.')
parser.add_argument('config_prefix', help='the config file prefix')
args = parser.parse_args()

frontendConfigPath = args.config_prefix + ".frontend.config"
tssConfigPath = args.config_prefix + ".tss.config"

# find number of shards
configPath = args.config_prefix + "0.config"
config = open(configPath, 'r')
line = config.readline()
match = re.match("f\s+(\d+)", line)
if match:
    numShards = int(match.group(1))
else:
    print "Error: could not get number of shards"
    sys.exit

servers = []

frontendConfig = open(frontendConfigPath, 'r')
for line in frontendConfig:
    match = re.match("replica\s+([\w\.-]+):(\d)", line)
    if match:
        hostname = match.group(1)
        servers.append(hostname)
frontendConfig.close()

for shardNum in range(0, numShards):
    backendConfigPath = args.config_prefix + repr(shardNum) + ".config"
    backendConfig = open(backendConfigPath, 'r')
    replicaNum = 0
    for line in backendConfig:
        match = re.match("replica\s+([\w\.-]+):(\d)", line)
        if match:
            hostname = match.group(1)
            servers.append(hostname)
    backendConfig.close()

tssConfig = open(tssConfigPath, 'r')
for line in tssConfig:
    match = re.match("replica\s+([\w\.-]+):(\d)", line)
    if match:
        hostname = match.group(1)
        servers.append(hostname)
tssConfig.close()

for hostname in servers:
    for dep in dependencies:
        os.system("rsync --copy-links " + dependencies[dep] + " " + hostname + ":" + WORKING_DIR + "/" + dep)
