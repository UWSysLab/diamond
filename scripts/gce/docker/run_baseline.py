#!/usr/bin/python

import experiment_common
import os

NUM_KEYS = "1000"
NUM_SECONDS = "60"
OUTPUT_DEST = "scripts/experiments/baseline/"
NUM_CLIENTS = 350
USE_REDIS = True

def getCommandFunc(workingDir, configFile, keyFile):
    return "java -cp " + workingDir + "/keyvaluestore-1.0-SNAPSHOT-jar-with-dependencies.jar edu.washington.cs.diamond.Client " + workingDir + configFile + ".config" + " " + workingDir + keyFile + " " + NUM_KEYS + " " + NUM_SECONDS

def processOutputFunc(outputFile):
    if USE_REDIS:
        experiment_common.putDataInRedis(outputFile)
    else:
        experiment_common.copyToSrcHost(outputFile, OUTPUT_DEST)

#copy files
experiment_common.copyIntoWorkingDir("apps/baseline-benchmarks/keyvaluestore/target/keyvaluestore-1.0-SNAPSHOT-jar-with-dependencies.jar")
experiment_common.copyConfigFiles()

experiment_common.runProcesses(getCommandFunc, NUM_CLIENTS, "baseline-out")
experiment_common.processOutput(processOutputFunc)
