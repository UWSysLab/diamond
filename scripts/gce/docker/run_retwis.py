#!/usr/bin/python

import experiment_common
import os

NUM_KEYS = "1000"
NUM_SECONDS = "60"
OUTPUT_DEST = "scripts/experiments/retwis/"
NUM_CLIENTS = 10
USE_REDIS = False

def getCommandFunc(workingDir, configFile, keyFile):
    return workingDir + "/retwisClient -m diamond -c " + workingDir + configFile + " -f " + workingDir + keyFile + " -k " + NUM_KEYS + " -d " + NUM_SECONDS

def processOutputFunc(outputFile):
    if USE_REDIS:
        experiment_common.putDataInRedis(outputFile)
    else:
        experiment_common.copyToSrcHost(outputFile, OUTPUT_DEST)
        
experiment_common.copyFiles("retwisClient", "apps/tapir-benchmarks/build")
experiment_common.runProcesses(getCommandFunc, NUM_CLIENTS, "retwis-out")
experiment_common.processOutput(processOutputFunc)
