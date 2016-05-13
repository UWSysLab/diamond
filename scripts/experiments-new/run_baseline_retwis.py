#!/usr/bin/python

import argparse
import experiment_common
import os

parser = argparse.ArgumentParser(description='Run baseline client.')
parser.add_argument('--numclients', type=int, default=128, help='number of clients')
parser.add_argument('--config', default="local", help='config prefix')
args = parser.parse_args()

NUM_KEYS = "100000"
NUM_SECONDS = "60"
OUTPUT_DEST = "scripts/experiments/baseline/"

def getCommandFunc(workingDir, configFile, keyFile):
    return "java -cp " + workingDir + "/keyvaluestore-1.0-SNAPSHOT-jar-with-dependencies.jar edu.washington.cs.diamond.RetwisClient " + workingDir + "/" + configFile + ".config" + " " + workingDir + "/" + keyFile + " " + NUM_KEYS + " " + NUM_SECONDS

#copy files
experiment_common.copyIntoWorkingDir("apps/baseline-benchmarks/keyvaluestore/target/keyvaluestore-1.0-SNAPSHOT-jar-with-dependencies.jar")
experiment_common.copyCommonFiles(args.config)

experiment_common.runProcesses(getCommandFunc, args.numclients, args.config, "baseline-out")
experiment_common.processOutput(experiment_common.putRetwisDataInRedis)
