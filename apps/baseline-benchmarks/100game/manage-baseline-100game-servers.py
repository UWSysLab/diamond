#!/usr/bin/python

import argparse
import glob
import os
import re
import time
import sys

WORKING_DIR = "/home/nl35"
SERVER_DIR = "target"
REDIS_DIR = "/home/nl35/redis-3.0.7/src"

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
    startRedis("diamond-backend-central-a-6cores", 8001, "diamond-backend-central-a-6cores", 8001)
    startRedis("diamond-backend-central-b-6cores", 8001, "diamond-backend-central-a-6cores", 8001)
    startRedis("diamond-backend-central-c-6cores", 8001, "diamond-backend-central-a-6cores", 8001)
    os.system(redisCliPath + " -h diamond-backend-central-a-6cores -p 8001 FLUSHDB")
elif args.action == 'kill':
    print("Killing servers...")
    killRedis("diamond-backend-central-a-6cores", 8001)
    killRedis("diamond-backend-central-b-6cores", 8001)
    killRedis("diamond-backend-central-c-6cores", 8001)
