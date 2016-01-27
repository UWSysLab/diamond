#!/usr/bin/python

import argparse
import os
import re
import time
import sys

USER_NAME = "nl35"
WORKING_DIR = "/biggerraid/users/" + USER_NAME + "/scratch"
BUILD_DIR = "../platform/build"

parser = argparse.ArgumentParser(description='Launch servers.')
parser.add_argument('action', choices=['start', 'kill'], help='the action to take')
parser.add_argument('config_prefix', help='the config file prefix')
args = parser.parse_args()

serverExecutablePath = BUILD_DIR + "/server"
tssConfigPath = args.config_prefix + ".tss.config"
tssExecutablePath = BUILD_DIR + "/tss"

remoteServerExecutablePath = WORKING_DIR + "/server"
remoteTssConfigPath = WORKING_DIR + "/diamond.tss.config"
remoteTssExecutablePath = WORKING_DIR + "/tss"

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

# launch servers
for shardNum in range(0, numShards):
    serverConfigPath = args.config_prefix + repr(shardNum) + ".config"
    remoteServerConfigPath = WORKING_DIR + "/diamond" + repr(shardNum) + ".config"
    serverConfig = open(serverConfigPath, 'r')
    replicaNum = 0
    for line in serverConfig:
        match = re.match("replica\s+([\w\.]+):(\d)", line)
        if match:
            hostname = match.group(1)
            if args.action == 'start':
                os.system("rsync " + serverConfigPath + " " + hostname + ":" + remoteServerConfigPath)
                os.system("rsync " + serverExecutablePath + " " + hostname + ":" + remoteServerExecutablePath)
                os.system("ssh -f " + hostname + " '" + remoteServerExecutablePath + " -c " + remoteServerConfigPath + " -i " + repr(replicaNum) + "'");
            elif args.action == 'kill':
                os.system("ssh " + hostname + " 'pkill -f " + remoteServerConfigPath + "'");
            replicaNum = replicaNum + 1
    serverConfig.close()

# launch tss servers
tssConfig = open(tssConfigPath, 'r')
replicaNum = 0
for line in tssConfig:
    match = re.match("replica\s+([\w\.]+):(\d)", line)
    if match:
        hostname = match.group(1)
        if args.action == 'start':
            os.system("rsync " + tssConfigPath + " " + hostname + ":" + remoteTssConfigPath)
            os.system("rsync " + tssExecutablePath + " " + hostname + ":" + remoteTssExecutablePath)
            os.system("ssh -f " + hostname + " '" + remoteTssExecutablePath + " -c " + remoteTssConfigPath + " -i " + repr(replicaNum) + "'");
        elif args.action == 'kill':
            os.system("ssh " + hostname + " 'pkill -f " + remoteTssConfigPath + "'");
        replicaNum = replicaNum + 1
tssConfig.close()
