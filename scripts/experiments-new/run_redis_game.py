#!/usr/bin/python

import argparse
import experiment_common
import os
import random
import redis
import re
import subprocess
import sys

def processOutputFunc(outputFile):
    r = redis.StrictRedis(host=experiment_common.DATA_REDIS_HOST, port=experiment_common.DATA_REDIS_PORT)
    r.incr("clients")
    turnCount = 0
    outfile = open(outputFile, 'r')
    for line in outfile:
        match = re.match("(\d+)\s+(\d+)", line)
        if match:
            prevEndTime = int(match.group(1))
            endTime = int(match.group(2))
            txnNum = random.randint(0, sys.maxint)
            txnKey = "txn-" + repr(txnNum)
            mapping = dict()
            mapping['prev-end-time'] = prevEndTime
            mapping['end-time'] = endTime
            r.hmset(txnKey, mapping)
            r.lpush("txns", txnKey)
            turnCount += 1
    if turnCount >= 50:
        r.incr("activeplayers")

WORKING_DIR = experiment_common.WORKING_DIR
NUM_FRONTENDS = experiment_common.NUM_FRONTENDS

parser = argparse.ArgumentParser(description='Run Redis version of the 100 game benchmark.')
parser.add_argument('--numpairs', type=int, default=50, help='number of client pairs')
parser.add_argument('--hostname', default="moranis.cs.washington.edu", help='Redis hostname')
parser.add_argument('--port', default="8001", help='Redis port')
args = parser.parse_args()
        
experiment_common.copyIntoWorkingDir("apps/baseline-benchmarks/100game/build/game-redis")
os.system("rsync " + experiment_common.SRC_HOST + ":diamond-src/apps/baseline-benchmarks/redox/build/libredox.so.0.3.0 " + experiment_common.WORKING_DIR + "/libredox.so.0")

sys.stderr.write("Running clients...\n")

processes = []
outputFiles = []

for i in range(0, args.numpairs):
    gameKeyPrefix = repr(random.randint(0, sys.maxint))
    outputFile1 = "game-redis-" + gameKeyPrefix + "-1"
    outputFile2 = "game-redis-" + gameKeyPrefix + "-2"
    cmd1 = WORKING_DIR + "/game-redis --host " + args.hostname + " --port " + args.port + " --name player1 --keyprefix " + gameKeyPrefix + " > " + WORKING_DIR + "/" + outputFile1
    cmd2 = WORKING_DIR + "/game-redis --host " + args.hostname + " --port " + args.port + " --name player2 --keyprefix " + gameKeyPrefix + " > " + WORKING_DIR + "/" + outputFile2
    processes.append(subprocess.Popen(cmd1, shell=True))
    processes.append(subprocess.Popen(cmd2, shell=True))
    outputFiles.append(outputFile1)
    outputFiles.append(outputFile2)

for process in processes:
    process.wait()

sys.stderr.write("Finished running clients\n")

success = True
for process in processes:
    if process.returncode:
        success = False
if not success:
    sys.stderr.write("Error: some clients returned nonzero return codes\n")

for outputFileName in outputFiles:
    fullPath = os.path.expanduser(WORKING_DIR) + "/" + outputFileName
    processOutputFunc(fullPath)
    sys.stderr.write("Finished processing output file %s\n" % outputFileName)
