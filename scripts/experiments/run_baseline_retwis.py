#!/usr/bin/python

import argparse
import experiment_common
import os

parser = argparse.ArgumentParser(description='Run baseline client.')
parser.add_argument('--numclients', type=int, default=128, help='number of clients')
parser.add_argument('--time', type=int, default=60, help='number of seconds to run')
experiment_common.addCommonArgs(parser)
args = parser.parse_args()
experiment_common.handleCommonArgs(args)

def getCommandFunc(workingDir, configFile, keyFile, numKeys):
    return "java -cp " + workingDir + "/keyvaluestore-1.0-SNAPSHOT-jar-with-dependencies.jar edu.washington.cs.diamond.RetwisClient " + workingDir + "/" + configFile + ".config" + " " + workingDir + "/" + keyFile + " " + repr(numKeys) + " " + repr(args.time)

#copy files
experiment_common.copyIntoWorkingDir("apps/baseline-benchmarks/keyvaluestore/target/keyvaluestore-1.0-SNAPSHOT-jar-with-dependencies.jar")
experiment_common.copyCommonFiles()

experiment_common.runProcesses(getCommandFunc, args.numclients)
experiment_common.processOutput(experiment_common.putRetwisDataInRedis)
