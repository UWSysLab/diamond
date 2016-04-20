#!/usr/bin/python

import argparse
import experiment_common
import os

parser = argparse.ArgumentParser(description='Run baseline client.')
parser.add_argument('--numclients', type=int, default=12, help='number of clients')
parser.add_argument('--config', default="gce", help='config prefix')
args = parser.parse_args()

NUM_KEYS = "1000"
NUM_SECONDS = "60"
OUTPUT_DEST = "scripts/experiments/baseline/"
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
experiment_common.copyConfigFiles(args.config)

experiment_common.runProcesses(getCommandFunc, args.numclients, args.config, "baseline-out")
experiment_common.processOutput(processOutputFunc)
