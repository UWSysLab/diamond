#!/usr/bin/python

import experiment_common
import os

NUM_KEYS = "100"
NUM_SECONDS = "120"
OUTPUT_DEST = "scripts/experiments/docc-noincrement/"
NUM_CLIENTS = 10
USE_REDIS = True

def getCommandFunc(workingDir, configFile, keyFile):
    return workingDir + "/docc --config " + workingDir + configFile + " --keys " + workingDir + keyFile + " --numkeys " + NUM_KEYS + " --time " + NUM_SECONDS

def processOutputFunc(outputFile):
    if USE_REDIS:
        experiment_common.putDataInRedis(outputFile)
    else:
        experiment_common.copyToSrcHost(outputFile, OUTPUT_DEST)

experiment_common.copyFiles("docc", "apps/benchmarks/build")
experiment_common.runProcesses(getCommandFunc, NUM_CLIENTS)
experiment_common.processOutput(processOutputFunc)
