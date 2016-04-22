#!/usr/bin/python

import argparse
import experiment_common
import os

NUM_KEYS = "1000"
NUM_SECONDS = "60"
OUTPUT_DEST = "scripts/experiments/caching/"
USE_REDIS = True

parser = argparse.ArgumentParser(description='Run caching client.')
parser.add_argument('--numclients', type=int, default=350, help='number of clients')
parser.add_argument('--config', default="gce", help='config prefix')
parser.add_argument('--caching', action="store_true", help='enable caching')
args = parser.parse_args()

def getCommandFunc(workingDir, configFile, keyFile):
    cmd = workingDir + "/caching --config " + workingDir + configFile + " --time " + NUM_SECONDS
    if args.caching:
        cmd = cmd + " --caching"
    return cmd

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
            match = re.match("(\d+)\s+(\d+)\s+(\d+)", line)
            if match:
                prevEndTime = int(match.group(1))
                endTime = int(match.group(2))
                committed = int(match.group(3))
                txnNum = random.randint(0, sys.maxint)
                txnKey = "txn-" + repr(txnNum)
                mapping = dict()
                mapping['prev-end-time'] = prevEndTime
                mapping['end-time'] = endTime
                mapping['committed'] = committed
                r.hmset(txnKey, mapping)
                r.lpush("txns", txnKey)
    else:
        experiment_common.copyToSrcHost(outputFile, OUTPUT_DEST)
        
experiment_common.copyFiles("caching", "apps/benchmarks/build", args.config)
experiment_common.runProcesses(getCommandFunc, args.numclients, args.config, "caching-out")
experiment_common.processOutput(processOutputFunc)
