#!/usr/bin/python

import argparse
import experiment_common
import os
import sys

NUM_KEYS = "100"
NUM_SECONDS = "60"
OUTPUT_DEST = "scripts/experiments/docc/"
USE_REDIS = True

parser = argparse.ArgumentParser(description='Run DOCC client.')
parser.add_argument('optype', choices=['docc', 'baseline'], help='choose between DOCC increment or read/write')
parser.add_argument('readfrac', type=float, help='fraction of workload that is reads')
parser.add_argument('--numclients', type=int, default=300, help='number of clients')
parser.add_argument('--config', default="gce", help='config prefix')
args = parser.parse_args()

def getCommandFunc(workingDir, configFile, keyFile):
    cmd = workingDir + "/docc --config " + workingDir + configFile + " --keys " + workingDir + keyFile + " --numkeys " + NUM_KEYS + " --time " + NUM_SECONDS
    if args.optype == 'docc':
        cmd = cmd + " --increment"
    cmd = cmd + " --readfrac " + repr(args.readfrac)
    return cmd

def processOutputFunc(outputFile):
    if USE_REDIS:
        experiment_common.putDataInRedis(outputFile)
    else:
        experiment_common.copyToSrcHost(outputFile, OUTPUT_DEST)

experiment_common.copyFiles("docc", "apps/benchmarks/build", args.config)
experiment_common.runProcesses(getCommandFunc, args.numclients, args.config, "docc-out")
experiment_common.processOutput(processOutputFunc)
