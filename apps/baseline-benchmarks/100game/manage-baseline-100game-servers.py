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
if "REDIS_DIR" not in os.environ:
    print("Error: environment variable REDIS_DIR is not set")
    print("(It should point to a folder containing redis-server and redis-cli binaries)")
    sys.exit()
WORKING_DIR = os.environ["DIAMOND_WORKING_DIR"]
REDIS_DIR = os.environ["REDIS_DIR"]

redisPath = REDIS_DIR + "/redis-server"
redisCliPath = REDIS_DIR + "/redis-cli"

remoteRedisPath = WORKING_DIR + "/redis-server"

def startRedis(hostname, port, leaderHostname, leaderPort):
    isLeader = (hostname == leaderHostname and port == leaderPort)
    os.system("rsync " + redisPath + " " + hostname + ":" + remoteRedisPath)
    if isLeader:
        redisCmd = remoteRedisPath + " --port " + repr(port)
    else:
        redisCmd = remoteRedisPath + " --port " + repr(port) + " --slaveof " + leaderHostname + " " + repr(leaderPort)
    redisCmd = redisCmd + " > output.redis." + repr(port) + ".txt 2>&1"
    os.system("ssh -f " + hostname + " '" + redisCmd + "'");

def killRedis(hostname, port):
    os.system("ssh " + hostname + " 'pkill -9 -f " + repr(port) + "'");

parser = argparse.ArgumentParser(description='Launch Redis servers.')
parser.add_argument('action', choices=['start', 'kill'], help='the action to take')
parser.add_argument('config_prefix', help='the config file prefix')
args = parser.parse_args()

backendConfigPath = args.config_prefix + "0.config"
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
        print("Handling Redis replica %d" % replicaNum)
        if isLeader:
            leaderHostname = hostname
            leaderPort = port
        if args.action == 'start':
            startRedis(hostname, port, leaderHostname, leaderPort)
        elif args.action == 'kill':
            killRedis(hostname, port)
        replicaNum = replicaNum + 1
        isLeader = False

if args.action == 'start':
    print("Calling FLUSHDB...")
    os.system(redisCliPath + " -h %s -p %s FLUSHDB" % (leaderHostname, leaderPort))
