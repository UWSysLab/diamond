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
SERVER_DIR = "target"
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
args = parser.parse_args()

if args.action == 'start':
    print("Starting servers...")
    startRedis("moranis.cs.washington.edu", 8001, "moranis.cs.washington.edu", 8001)
    startRedis("moranis.cs.washington.edu", 8002, "moranis.cs.washington.edu", 8001)
    startRedis("moranis.cs.washington.edu", 8003, "moranis.cs.washington.edu", 8001)
    os.system(redisCliPath + " -h moranis.cs.washington.edu -p 8001 FLUSHDB")
elif args.action == 'kill':
    print("Killing servers...")
    killRedis("moranis.cs.washington.edu", 8001)
    killRedis("moranis.cs.washington.edu", 8002)
    killRedis("moranis.cs.washington.edu", 8003)
