#!/usr/bin/python

import argparse
import experiment_common
import os

NUM_KEYS = "1000000"
NUM_SECONDS = "120"
OUTPUT_DEST = "scripts/experiments/retwis/"
USE_REDIS = True

parser = argparse.ArgumentParser(description='Run retwis client.')
parser.add_argument('--numclients', type=int, default=64, help='number of clients')
parser.add_argument('--config', default="gce", help='config prefix')
parser.add_argument('--isolation', choices=['linearizable', 'snapshot', 'eventual', 'linearizabledocc', 'snapshotdocc'],  default='linearizable', help='isolation level')
parser.add_argument('--zipf', type=float, default=0.3, help='zipf coefficient')
args = parser.parse_args()

def getCommandFunc(workingDir, configFile, keyFile):
    return workingDir + "/retwisClient -m " + args.isolation + " -c " + workingDir + configFile + " -f " + workingDir + keyFile + " -k " + NUM_KEYS + " -d " + NUM_SECONDS + " -z " + repr(args.zipf)

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
                txnType = int(match.group(4))
                txnNum = random.randint(0, sys.maxint)
                txnKey = "txn-" + repr(txnNum)
                mapping = dict()
                mapping['start-time'] = startTime
                mapping['end-time'] = endTime
                mapping['committed'] = committed
                mapping['type'] = txnType
                r.hmset(txnKey, mapping)
                r.lpush("txns", txnKey)
    else:
        experiment_common.copyToSrcHost(outputFile, OUTPUT_DEST)
        
experiment_common.copyFiles("retwisClient", "apps/tapir-benchmarks/build", args.config)
experiment_common.runProcesses(getCommandFunc, args.numclients, args.config, "retwis-out")
experiment_common.processOutput(processOutputFunc)
