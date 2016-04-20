#!/usr/bin/python

import argparse
import experiment_common
import os

parser = argparse.ArgumentParser(description='Run baseline client.')
parser.add_argument('--numclients', type=int, default=10, help='number of clients')
parser.add_argument('--config', default="gce", help='config prefix')
args = parser.parse_args()

NUM_KEYS = "1000"
NUM_SECONDS = "60"
OUTPUT_DEST = "scripts/experiments/baseline/"
USE_REDIS = True

def getCommandFunc(workingDir, configFile, keyFile):
    return "java -cp " + workingDir + "/keyvaluestore-1.0-SNAPSHOT-jar-with-dependencies.jar edu.washington.cs.diamond.RetwisClient " + workingDir + configFile + ".config" + " " + workingDir + keyFile + " " + NUM_KEYS + " " + NUM_SECONDS

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
            match = re.match("^\d+\s+([\d\.]+)\s+([\d\.]+)\s+\d+\s+(\d+)\s+(\d+)", line)
            if match:
                startTime = int(float(match.group(1)) * 1000)
                endTime = int(float(match.group(2)) * 1000)
                committed = int(match.group(3))
                type = int(match.group(4))
                txnNum = random.randint(0, sys.maxint)
                txnKey = "txn-" + repr(txnNum)
                mapping = dict()
                mapping['start-time'] = startTime
                mapping['end-time'] = endTime
                mapping['committed'] = committed
                r.hmset(txnKey, mapping)
                r.lpush("txns", txnKey)
    else:
        experiment_common.copyToSrcHost(outputFile, OUTPUT_DEST)

#copy files
experiment_common.copyIntoWorkingDir("apps/baseline-benchmarks/keyvaluestore/target/keyvaluestore-1.0-SNAPSHOT-jar-with-dependencies.jar")
experiment_common.copyConfigFiles(args.config)

experiment_common.runProcesses(getCommandFunc, args.numclients, args.config, "baseline-out")
experiment_common.processOutput(processOutputFunc)
