#!/usr/bin/python

import experiment_common
import os

NUM_KEYS = "1000"
NUM_SECONDS = "120"
OUTPUT_DEST = "scripts/experiments/scalability/"
NUM_CLIENTS = 10
USE_REDIS = True

def getCommandFunc(workingDir, configFile, keyFile):
    return workingDir + "/scalability --config " + workingDir + configFile + " --keys " + workingDir + keyFile + " --numkeys " + NUM_KEYS + " --time " + NUM_SECONDS

def processOutputFunc(outputFile):
    if USE_REDIS:
        experiment_common.putDataInRedis(outputFile)
    else:
        experiment_common.copyToSrcHost(outputFile, OUTPUT_DEST)
        
experiment_common.copyFiles("scalability", "apps/benchmarks/build")
experiment_common.runProcesses(getCommandFunc, NUM_CLIENTS, "scalability-out")
experiment_common.processOutput(processOutputFunc)
