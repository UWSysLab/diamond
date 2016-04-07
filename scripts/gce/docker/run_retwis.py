#!/usr/bin/python

import experiment_common
import os

NUM_KEYS = "1000"
NUM_SECONDS = "60"
OUTPUT_DEST = "scripts/experiments/retwis/"
NUM_CLIENTS = 10
USE_REDIS = True

def getCommandFunc(workingDir, configFile, keyFile):
    return workingDir + "/retwisClient -m diamond -c " + workingDir + configFile + " -f " + workingDir + keyFile + " -k " + NUM_KEYS + " -d " + NUM_SECONDS

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
        
experiment_common.copyFiles("retwisClient", "apps/tapir-benchmarks/build")
experiment_common.runProcesses(getCommandFunc, NUM_CLIENTS, "retwis-out")
experiment_common.processOutput(processOutputFunc)
