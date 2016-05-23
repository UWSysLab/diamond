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
WORKING_DIR = os.environ["DIAMOND_WORKING_DIR"]
BUILD_DIR = os.environ["DIAMOND_SRC_DIR"] + "/platform/build"

parser = argparse.ArgumentParser(description='Launch servers.')
parser.add_argument('action', choices=['start', 'kill'], help='the action to take')
parser.add_argument('config_prefix', help='the config file prefix')
parser.add_argument('--shards', type=int, help='number of backend shards')
parser.add_argument('--frontends', type=int, help='number of frontend servers')
parser.add_argument('--keys', help='a file containing keys to load')
parser.add_argument('--numkeys', type=int, help='number of keys to load from file')
parser.add_argument('--batch', type=int, default=1, help='batch size for backend servers')
parser.add_argument('--debug', action='store_true', help='enable debug logging')
args = parser.parse_args()

if args.keys == None and args.numkeys != None or args.keys != None and args.numkeys == None:
    parser.error('--keys and --numkeys must be given together')
    sys.exit()

if args.action == 'kill' and args.shards != None:
    parser.error('--shards option not allowed with action \'kill\'')
    sys.exit()

frontendExecutablePath = BUILD_DIR + "/frontserver"
backendExecutablePath = BUILD_DIR + "/storeserver"
tssConfigPath = args.config_prefix + ".tss.config"
tssExecutablePath = BUILD_DIR + "/tss"
keyPath = args.keys
numKeys = args.numkeys
numShards = args.shards
numFrontends = args.frontends

remoteFrontendExecutablePath = WORKING_DIR + "/frontserver"
remoteBackendExecutablePath = WORKING_DIR + "/storeserver"
remoteTssConfigPath = WORKING_DIR + "/diamond.tss.config"
remoteTssExecutablePath = WORKING_DIR + "/tss"
remoteKeyPath = None
if keyPath != None:
    remoteKeyPath = WORKING_DIR + "/keys.txt"

setVarsCmd = "LD_LIBRARY_PATH=" + repr(WORKING_DIR)
debugCmd = ""
if args.debug:
    debugCmd = "DEBUG=all"

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

print("Running command for %d shards (%d config files detected)" % (numShards, totalNumShards));
print("Running command for %d frontends (%d config files detected)" % (numFrontends, totalNumFrontends));
if args.action == 'start':
    if args.debug:
        print("Enabling debug output")
    print("Starting servers...")
elif args.action == 'kill':
    print("Killing servers...")

# launch frontend servers
for frontendNum in range(0, numFrontends):
    frontendConfigPath = args.config_prefix + ".frontend" + repr(frontendNum) + ".config"
    remoteFrontendConfigPath = WORKING_DIR + "/diamond.frontend" + repr(frontendNum) + ".config"
    frontendConfig = open(frontendConfigPath, 'r')
    for line in frontendConfig:
        match = re.match("replica\s+([\w\.-]+):(\d+)", line)
        if match:
            hostname = match.group(1)
            remoteFrontendOutputPath = WORKING_DIR + "/output.frontend." + repr(frontendNum) + ".txt"
            print("Handling frontend %d" % frontendNum)
            if args.action == 'start':
                os.system("rsync " + frontendConfigPath + " " + hostname + ":" + remoteFrontendConfigPath)
                os.system("rsync " + frontendExecutablePath + " " + hostname + ":" + remoteFrontendExecutablePath)
                os.system("rsync " + tssConfigPath + " " + hostname + ":" + remoteTssConfigPath)
                for shardNum in range(0, numShards):
                    backendConfigPath = args.config_prefix + repr(shardNum) + ".config"
                    remoteBackendConfigPath = WORKING_DIR + "/diamond" + repr(shardNum) + ".config"
                    os.system("rsync " + backendConfigPath + " " + hostname + ":" + remoteBackendConfigPath)
                remoteBackendConfigPrefix = WORKING_DIR + "/diamond"
                os.system("ssh -f " + hostname + " '" + debugCmd + " " + setVarsCmd + " " + remoteFrontendExecutablePath + " -c " + remoteFrontendConfigPath + " -b " + remoteBackendConfigPrefix + " -N " + repr(numShards) +  " > " + remoteFrontendOutputPath + " 2>&1'");
            elif args.action == 'kill':
                os.system("ssh " + hostname + " 'pkill -9 -f " + remoteFrontendConfigPath + "'");



# launch backend servers
for shardNum in range(0, numShards):
    backendConfigPath = args.config_prefix + repr(shardNum) + ".config"
    remoteBackendConfigPath = WORKING_DIR + "/diamond" + repr(shardNum) + ".config"
    backendConfig = open(backendConfigPath, 'r')
    replicaNum = 0
    for line in backendConfig:
        match = re.match("replica\s+([\w\.-]+):(\d+)", line)
        if match:
            hostname = match.group(1)
            remoteBackendOutputPath = WORKING_DIR + "/output.backend." + repr(shardNum) + "." + repr(replicaNum) + ".txt"
            print("Handling replica %d in shard %d" % (replicaNum, shardNum))
            if args.action == 'start':
                os.system("rsync " + backendConfigPath + " " + hostname + ":" + remoteBackendConfigPath)
                os.system("rsync " + backendExecutablePath + " " + hostname + ":" + remoteBackendExecutablePath)
                keyArgs = ""
                if keyPath != None:
                    os.system("rsync " + keyPath + " " + hostname + ":" + remoteKeyPath)
                    keyArgs = " -k " + repr(numKeys) + " -f " + remoteKeyPath
                os.system("ssh -f " + hostname + " '" + debugCmd + " " + setVarsCmd + " " + remoteBackendExecutablePath + " -c " + remoteBackendConfigPath + " -i " + repr(replicaNum) + " -B " + repr(args.batch) + keyArgs + " > " + remoteBackendOutputPath + " 2>&1'");
            elif args.action == 'kill':
                os.system("ssh " + hostname + " 'pkill -9 -f " + remoteBackendConfigPath + "'");
            replicaNum = replicaNum + 1
    backendConfig.close()

# launch tss servers
tssConfig = open(tssConfigPath, 'r')
replicaNum = 0
for line in tssConfig:
    match = re.match("replica\s+([\w\.-]+):(\d+)", line)
    if match:
        hostname = match.group(1)
        remoteTssOutputPath = WORKING_DIR + "/output.tss." + repr(replicaNum) + ".txt"
        print("Handling TSS replica %s" % replicaNum)
        if args.action == 'start':
            os.system("rsync " + tssConfigPath + " " + hostname + ":" + remoteTssConfigPath)
            os.system("rsync " + tssExecutablePath + " " + hostname + ":" + remoteTssExecutablePath)
            os.system("ssh -f " + hostname + " '" + debugCmd + " " + setVarsCmd + " " + remoteTssExecutablePath + " -c " + remoteTssConfigPath + " -i " + repr(replicaNum) + " > " + remoteTssOutputPath + " 2>&1'");
        elif args.action == 'kill':
            os.system("ssh " + hostname + " 'pkill -9 -f " + remoteTssConfigPath + "'");
        replicaNum = replicaNum + 1
tssConfig.close()
