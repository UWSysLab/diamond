#!/usr/bin/python

import argparse
import glob
import os
import re
import time
import sys

WORKING_DIR = "/scratch/nl35"
SERVER_DIR = "target"
REDIS_DIR = "../../../../redis-3.0.7/src"

parser = argparse.ArgumentParser(description='Launch servers.')
parser.add_argument('action', choices=['start', 'kill'], help='the action to take')
parser.add_argument('config_prefix', help='the config file prefix')
parser.add_argument('--frontends', type=int, help='number of frontend servers')
parser.add_argument('--keys', help='a file containing keys to load')
parser.add_argument('--numkeys', type=int, help='number of keys to load from file')
args = parser.parse_args()

if args.keys == None and args.numkeys != None or args.keys != None and args.numkeys == None:
    parser.error('--keys and --numkeys must be given together')
    sys.exit()

serverJarPath = SERVER_DIR + "/keyvaluestore-1.0-SNAPSHOT-jar-with-dependencies.jar"
redisPath = REDIS_DIR + "/redis-server"
keyPath = args.keys
numKeys = args.numkeys
numFrontends = args.frontends

remoteServerJarPath = WORKING_DIR + "/keyvaluestore.jar"
remoteRedisPath = WORKING_DIR + "/redis-server"
remoteKeyPath = None
if keyPath != None:
    remoteKeyPath = WORKING_DIR + "/keys.txt"

# find number of frontends
files = glob.glob(args.config_prefix + ".frontend*.config")
maxFrontendNum = -1
for filename in files:
    match = re.match(args.config_prefix + ".frontend(\d+)\.config", filename)
    if match:
        frontendNum = int(match.group(1))
        if maxFrontendNum < frontendNum:
            maxFrontendNum = frontendNum
totalNumFrontends = maxFrontendNum + 1

if numFrontends == None:
    numFrontends = totalNumFrontends
if numFrontends > totalNumFrontends:
    print("Error: missing config files for one or more frontends")
    sys.exit()

print("Running command for %d frontends (%d config files detected)" % (numFrontends, totalNumFrontends));
if args.action == 'start':
    print("Starting servers...")
elif args.action == 'kill':
    print("Killing servers...")

# launch redis instances
shardNum = 0
backendConfigPath = args.config_prefix + repr(shardNum) + ".config"
remoteBackendConfigPath = WORKING_DIR + "/diamond" + repr(shardNum) + ".config"
backendConfig = open(backendConfigPath, 'r')
replicaNum = 0
isLeader = True
leaderHostname = ""
leaderPort = ""
for line in backendConfig:
    match = re.match("^replica\s+([\w\.-]+):(\d+)", line)
    if match:
        hostname = match.group(1)
        port = match.group(2)
        remoteBackendOutputPath = WORKING_DIR + "/output.redis." + repr(shardNum) + "." + repr(replicaNum) + ".txt"
        if args.action == 'start':
            os.system("rsync " + backendConfigPath + " " + hostname + ":" + remoteBackendConfigPath)
            os.system("rsync " + redisPath + " " + hostname + ":" + remoteRedisPath)
            redisCmd = ""
            if isLeader:
                redisCmd = remoteRedisPath + " --port " + port
                leaderHostname = hostname
                leaderPort = port
            else:
                redisCmd = remoteRedisPath + " --port " + port + " --slaveof " + leaderHostname + " " + leaderPort
            os.system("ssh -f " + hostname + " '" + redisCmd + " > " + remoteBackendOutputPath + " 2>&1'");
        elif args.action == 'kill':
            os.system("ssh " + hostname + " 'pkill -9 -f " + port + "'");
        replicaNum = replicaNum + 1
        isLeader = False
backendConfig.close()

# launch frontend servers
for frontendNum in range(0, numFrontends):
    frontendConfigPath = args.config_prefix + ".frontend" + repr(frontendNum) + ".config"
    frontendConfig = open(frontendConfigPath, 'r')
    for line in frontendConfig:
        match = re.match("replica\s+([\w\.-]+):(\d+)", line)
        if match:
            hostname = match.group(1)
            port = match.group(2)
            remoteFrontendOutputPath = WORKING_DIR + "/output.keyvalueserver." + repr(frontendNum) + ".txt"
            if args.action == 'start':
                os.system("rsync " + serverJarPath + " " + hostname + ":" + remoteServerJarPath)
                serverCmd = "java -cp " + remoteServerJarPath + " edu.washington.cs.diamond.Server " + port + " " + leaderHostname + " " + leaderPort
                os.system("ssh -f " + hostname + " '" + serverCmd + " > " + remoteFrontendOutputPath + " 2>&1'");
            elif args.action == 'kill':
                os.system("ssh " + hostname + " 'pkill -9 -f " + remoteServerJarPath + "'");