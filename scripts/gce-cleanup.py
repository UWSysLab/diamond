#!/usr/bin/python

import argparse
import glob
import os
import re

WORKING_DIR = "/home/nl35"

parser = argparse.ArgumentParser(description='Delete output files from GCE machines.')
parser.add_argument('config_prefix', help='the config file prefix')
args = parser.parse_args()

tssConfigPath = args.config_prefix + ".tss.config"

# find number of shards
files = glob.glob(args.config_prefix + "*.config")
maxShardNum = -1
for filename in files:
    match = re.match(args.config_prefix + "(\d+)\.config", filename)
    if match:
        shardNum = int(match.group(1))
        if maxShardNum < shardNum:
            maxShardNum = shardNum
numShards = maxShardNum + 1

# find number of frontends
files = glob.glob(args.config_prefix + ".frontend*.config")
maxFrontendNum = -1
for filename in files:
    match = re.match(args.config_prefix + ".frontend(\d+)\.config", filename)
    if match:
        frontendNum = int(match.group(1))
        if maxFrontendNum < frontendNum:
            maxFrontendNum = frontendNum
numFrontends = maxFrontendNum + 1

print("Deleting output files for %d shards, %d frontends" % (numShards, numFrontends))

servers = []

for frontendNum in range(0, numFrontends):
    frontendConfigPath = args.config_prefix + ".frontend" + repr(frontendNum) + ".config"
    frontendConfig = open(frontendConfigPath, 'r')
    for line in frontendConfig:
        match = re.match("replica\s+([\w\.-]+):(\d)", line)
        if match:
            hostname = match.group(1)
            os.system("ssh " + hostname + " 'rm output.frontend." + repr(frontendNum) + ".txt'")
    frontendConfig.close()

for shardNum in range(0, numShards):
    backendConfigPath = args.config_prefix + repr(shardNum) + ".config"
    backendConfig = open(backendConfigPath, 'r')
    replicaNum = 0
    for line in backendConfig:
        match = re.match("replica\s+([\w\.-]+):(\d)", line)
        if match:
            hostname = match.group(1)
            os.system("ssh " + hostname + " 'rm output.backend." + repr(shardNum) + ".*.txt'")
    backendConfig.close()

tssConfig = open(tssConfigPath, 'r')
for line in tssConfig:
    match = re.match("replica\s+([\w\.-]+):(\d)", line)
    if match:
        hostname = match.group(1)
        os.system("ssh " + hostname + " 'rm output.tss.*.txt'")
tssConfig.close()
