#!/usr/bin/python

import argparse
import experiment_common
import os

NUM_KEYS = "1000"
NUM_SECONDS = "60"
OUTPUT_DEST = "scripts/experiments/scalability/"
USE_REDIS = True

parser = argparse.ArgumentParser(description='Run scalability client.')
parser.add_argument('isolation', choices=['linearizable', 'snapshot', 'eventual'], help='isolation level')
parser.add_argument('--numclients', type=int, default=350, help='number of clients')
args = parser.parse_args()

def getCommandFunc(workingDir, configFile, keyFile):
    return workingDir + "/scalability --config " + workingDir + configFile + " --keys " + workingDir + keyFile + " --numkeys " + NUM_KEYS + " --time " + NUM_SECONDS + " --isolation " + args.isolation

def processOutputFunc(outputFile):
    if USE_REDIS:
        experiment_common.putDataInRedis(outputFile)
    else:
        experiment_common.copyToSrcHost(outputFile, OUTPUT_DEST)
        
experiment_common.copyFiles("scalability", "apps/benchmarks/build")
experiment_common.runProcesses(getCommandFunc, args.numclients, "scalability-out")
experiment_common.processOutput(processOutputFunc)
