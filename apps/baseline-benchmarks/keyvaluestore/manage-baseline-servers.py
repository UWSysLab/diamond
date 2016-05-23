#!/usr/bin/python

import argparse
import glob
import os
import re
import time
import sys

if "DIAMOND_WORKING_DIR" not in os.environ:
    print("Error: environment variable DIAMOND_WORKING_DIR is not set")
    print("(It should point to the working directory you want to use on the server host machines)")
    sys.exit()
if "DIAMOND_SRC_DIR" not in os.environ:
    print("Error: environment variable DIAMOND_SRC_DIR is not set")
    print("(It should point to the location of the diamond-src repo on this machine)")
    sys.exit()
if "REDIS_DIR" not in os.environ:
    print("Error: environment variable REDIS_DIR is not set")
    print("(It should point to a folder containing redis-server and redis-cli binaries)")
    sys.exit()
WORKING_DIR = os.environ["DIAMOND_WORKING_DIR"]
PROJECT_DIR = os.environ["DIAMOND_SRC_DIR"] + "/apps/baseline-benchmarks/keyvaluestore"
REDIS_DIR = os.environ["REDIS_DIR"]

parser = argparse.ArgumentParser(description='Launch servers.')
parser.add_argument('action', choices=['start', 'kill'], help='the action to take')
parser.add_argument('config_prefix', help='the config file prefix')
parser.add_argument('--frontends', type=int, help='number of frontend servers')
parser.add_argument('--shards', type=int, help='number of shards')
parser.add_argument('--keys', help='a file containing keys to load')
parser.add_argument('--numkeys', type=int, help='number of keys to load from file')
parser.add_argument('--zipf', type=float, default=0.3, help='zipf distribution coefficient')
args = parser.parse_args()

if args.keys == None and args.numkeys != None or args.keys != None and args.numkeys == None:
    parser.error('--keys and --numkeys must be given together')
    sys.exit()
if args.action == 'start' and args.keys == None:
    parser.error('--keys is required for action \'start\'')
    sys.exit()

serverJarPath = PROJECT_DIR + "/target/keyvaluestore-1.0-SNAPSHOT-jar-with-dependencies.jar"
redisPath = REDIS_DIR + "/redis-server"
redisClientPath = REDIS_DIR + "/redis-cli"
protocolScriptPath = PROJECT_DIR + "/generate-fill-protocol.pl"
keyPath = args.keys
numKeys = args.numkeys
numFrontends = args.frontends
numShards = args.shards

remoteServerJarPath = WORKING_DIR + "/keyvaluestore.jar"
remoteRedisPath = WORKING_DIR + "/redis-server"
remoteRedisClientPath = WORKING_DIR + "/redis-cli"
remoteProtocolScriptPath = WORKING_DIR + "/generate-fill-protocol.pl"
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

# find number of shards
files = glob.glob(args.config_prefix + "*.config")
maxShardNum = -1
for filename in files:
    match = re.match(args.config_prefix + "(\d+)\.config", filename)
    if match:
        shardNum = int(match.group(1))
        if maxShardNum < shardNum:
            maxShardNum = shardNum
totalNumShards = maxShardNum + 1

if numShards == None:
    numShards = totalNumShards
if numShards > totalNumShards:
    print("Error: missing config files for one or more shards")
    sys.exit()

if numFrontends == None:
    numFrontends = totalNumFrontends
if numFrontends > totalNumFrontends:
    print("Error: missing config files for one or more frontends")
    sys.exit()

print("Running command for %d frontends (%d config files detected)" % (numFrontends, totalNumFrontends));
print("Running command for %d frontends (%d config files detected)" % (numShards, totalNumShards));
if args.action == 'start':
    print("Starting servers...")
elif args.action == 'kill':
    print("Killing servers...")

# launch redis instances
for shardNum in range(0, numShards):
    backendConfigPath = args.config_prefix + repr(shardNum) + ".config"
    remoteBackendConfigPath = WORKING_DIR + "/diamond" + repr(shardNum) + ".config"
    backendConfig = open(backendConfigPath, 'r')
    replicaNum = 0
    isLeader = True
    leaderHostname = ""
    leaderPort = ""
    numFailures = 0
    for line in backendConfig:
        match = re.match("^replica\s+([\w\.-]+):(\d+)", line)
        if match:
            hostname = match.group(1)
            port = match.group(2)
            remoteBackendOutputPath = WORKING_DIR + "/output.redis." + repr(shardNum) + "." + repr(replicaNum) + ".txt"
            print("Handling redis server %d in shard %d" % (replicaNum, shardNum))
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
                if isLeader and keyPath != None:
                    os.system("rsync " + keyPath + " " + hostname + ":" + remoteKeyPath)
                    os.system("rsync " + redisClientPath + " " + hostname + ":" + remoteRedisClientPath)
                    os.system("rsync " + protocolScriptPath + " " + hostname + ":" + remoteProtocolScriptPath)
                    os.system("ssh " + hostname + " 'cat " + remoteKeyPath + " | head -n " + repr(numKeys) + " | " + remoteProtocolScriptPath +  " | " + remoteRedisClientPath + " -p " + repr(port) + " --pipe'")
            elif args.action == 'kill':
                os.system("ssh " + hostname + " 'pkill -9 -f " + port + "'");
            replicaNum = replicaNum + 1
            isLeader = False
        else:
            match = re.match("^f\s+(\d+)", line)
            if match:
                numFailures = int(match.group(1))
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
            print("Handling Jetty server %d" % frontendNum);
            if args.action == 'start':
                os.system("rsync " + serverJarPath + " " + hostname + ":" + remoteServerJarPath)
                for shardNum in range(0, numShards):
                    backendConfigPath = args.config_prefix + repr(shardNum) + ".config"
                    remoteBackendConfigPath = WORKING_DIR + "/diamond" + repr(shardNum) + ".config"
                    os.system("rsync " + backendConfigPath + " " + hostname + ":" + remoteBackendConfigPath)
                remoteBackendConfigPrefix = WORKING_DIR + "/diamond"
                serverCmd = "java -cp " + remoteServerJarPath + " edu.washington.cs.diamond.RetwisServer " + port + " " + remoteBackendConfigPrefix + " " + repr(numShards)
                if keyPath != None:
                    os.system("rsync " + keyPath + " " + hostname + ":" + remoteKeyPath)
                    serverCmd = serverCmd + " " + remoteKeyPath + " " + repr(numKeys)
                serverCmd = serverCmd + " " + repr(args.zipf)
                os.system("ssh -f " + hostname + " '" + serverCmd + " > " + remoteFrontendOutputPath + " 2>&1'");
            elif args.action == 'kill':
                os.system("ssh " + hostname + " 'pkill -9 -f " + remoteServerJarPath + "'");
