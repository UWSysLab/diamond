#!/usr/bin/python

import argparse
import experiment_common
import os

parser = argparse.ArgumentParser(description='Run retwis client.')
parser.add_argument('--numclients', type=int, default=64, help='number of clients')
parser.add_argument('--isolation', choices=['linearizable', 'snapshot', 'eventual', 'linearizabledocc', 'snapshotdocc'],  default='linearizable', help='isolation level')
parser.add_argument('--zipf', type=float, default=0.3, help='zipf coefficient')
parser.add_argument('--time', type=int, default=60, help='number of seconds to run')
experiment_common.addCommonArgs(parser)
args = parser.parse_args()
experiment_common.handleCommonArgs(args)

def retwisCommandFunc(workingDir, configFile, keyFile, numKeys):
    return workingDir + "/retwisClient -m " + args.isolation + " -c " + workingDir + "/" + configFile + " -f " + workingDir + "/" + keyFile + " -k " + repr(numKeys) + " -d " + repr(args.time) + " -z " + repr(args.zipf)

experiment_common.copyCommonFiles()
experiment_common.copyIntoWorkingDir("apps/tapir-benchmarks/build/retwisClient")

experiment_common.runProcesses(retwisCommandFunc, args.numclients)
experiment_common.processOutput(experiment_common.putRetwisDataInRedis)
