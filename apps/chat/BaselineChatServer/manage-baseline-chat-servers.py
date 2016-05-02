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
jarPath = "baselinechatserver.jar"
dependencies = ["commons-pool2-2.0.jar", "gson-2.3.1.jar", "jedis-2.4.2.jar"]
localDependenciesDir = "libs"

remoteRedisPath = WORKING_DIR + "/redis-server"

def startJavaServer(hostname, port, redisHostname, redisPort):
    os.system("rsync " + jarPath + " " + hostname + ":" + WORKING_DIR)
    for dep in dependencies:
        os.system("rsync " + localDependenciesDir + "/" + dep + " " + hostname + ":" + WORKING_DIR)
    os.system("ssh -f " + hostname + " 'java -classpath commons-pool2-2.0.jar:gson-2.3.1.jar:jedis-2.4.2.jar:baselinechatserver.jar Main " + repr(port) + " " + redisHostname + " " + repr(redisPort) + "'")

def killJavaServer(hostname, port):
    os.system("ssh " + hostname + " 'pkill -9 -f " + repr(port) + "'");

def startRedis(hostname, port, leaderHostname, leaderPort):
    isLeader = (hostname == leaderHostname and port == leaderPort)
    os.system("rsync " + redisPath + " " + hostname + ":" + remoteRedisPath)
    if isLeader:
        redisCmd = remoteRedisPath + " --port " + repr(port)
    else:
        redisCmd = remoteRedisPath + " --port " + repr(port) + " --slaveof " + leaderHostname + " " + repr(leaderPort)
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
    startJavaServer("diamond-frontend-central-011u", 8000, "diamond-backend-central-a-6cores", 8001)
elif args.action == 'kill':
    print("Killing servers...")
    killJavaServer("diamond-frontend-central-011u", 8000)
    killRedis("diamond-backend-central-a-6cores", 8001)
    killRedis("diamond-backend-central-b-6cores", 8001)
    killRedis("diamond-backend-central-c-6cores", 8001)
