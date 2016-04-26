#!/usr/bin/python

import argparse
import experiment_common
import os
import random
import subprocess
import sys

def processOutputFunc(outputFile):
    if USE_REDIS:
        import random
        import redis
        import re
        import sys
        r = redis.StrictRedis(host=experiment_common.SRC_HOST, port=6379)
        r.incr("clients")
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
    else:
        experiment_common.copyToSrcHost(outputFile, OUTPUT_DEST)

WORKING_DIR = experiment_common.WORKING_DIR
NUM_FRONTENDS = experiment_common.NUM_FRONTENDS
OUTPUT_DEST = "scripts/experiments/caching/"
USE_REDIS = True

parser = argparse.ArgumentParser(description='Run 100 game benchmark.')
parser.add_argument('--numpairs', type=int, default=50, help='number of client pairs')
parser.add_argument('--config', default="gce", help='config prefix')
parser.add_argument('--nocaching', action="store_true", help='disable caching')
args = parser.parse_args()
        
experiment_common.copyFiles("game", "apps/benchmarks/build", args.config)

sys.stderr.write("Running clients...\n")

processes = []
outputFiles = []

for i in range(0, args.numpairs):
    gameKeyPrefix = repr(random.randint(0, sys.maxint))
    for j in range(0, 2):
        outputFile = "game-" + gameKeyPrefix + "-" + repr(j)
        configFile = args.config + ".frontend" + repr(i % NUM_FRONTENDS)
        cmd = WORKING_DIR + "/game --config " + WORKING_DIR + configFile + " --name player" + repr(j) + " --keyprefix " + gameKeyPrefix
        if args.nocaching:
            cmd = cmd + " --nocaching"
        cmd = cmd + " > " + WORKING_DIR + outputFile
        processes.append(subprocess.Popen(cmd, shell=True))
        outputFiles.append(outputFile)

for process in processes:
    process.wait()

sys.stderr.write("Finished running clients\n")

for outputFileName in outputFiles:
    fullPath = os.path.expanduser(WORKING_DIR) + outputFileName
    processOutputFunc(fullPath)
    sys.stderr.write("Finished processing output file %s\n" % outputFileName)
