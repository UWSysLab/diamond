#!/usr/bin/python

import argparse
import experiment_common
import os

NUM_KEYS = "100000"
NUM_SECONDS = "60"
OUTPUT_DEST = "scripts/experiments/retwis/"

parser = argparse.ArgumentParser(description='Run retwis client.')
parser.add_argument('--numclients', type=int, default=64, help='number of clients')
parser.add_argument('--config', default="local", help='config prefix')
parser.add_argument('--isolation', choices=['linearizable', 'snapshot', 'eventual', 'linearizabledocc', 'snapshotdocc'],  default='linearizable', help='isolation level')
parser.add_argument('--zipf', type=float, default=0.3, help='zipf coefficient')
args = parser.parse_args()

def getCommandFunc(workingDir, configFile, keyFile):
    return workingDir + "/retwisClient -m " + args.isolation + " -c " + workingDir + "/" + configFile + " -f " + workingDir + "/" + keyFile + " -k " + NUM_KEYS + " -d " + NUM_SECONDS + " -z " + repr(args.zipf)

experiment_common.copyCommonFiles(args.config)
experiment_common.copyIntoWorkingDir("apps/tapir-benchmarks/build/retwisClient")

experiment_common.runProcesses(getCommandFunc, args.numclients, args.config, "retwis-out")
experiment_common.processOutput(experiment_common.putRetwisDataInRedis)
