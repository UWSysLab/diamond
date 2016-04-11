#!/usr/bin/python

import argparse
import experiment_common
import os
import sys

NUM_KEYS = "100"
NUM_SECONDS = "60"
OUTPUT_DEST = "scripts/experiments/docc/"
NUM_CLIENTS = 300
USE_REDIS = True

parser = argparse.ArgumentParser(description='Run DOCC client.')
parser.add_argument('optype', choices=['docc', 'baseline'], help='choose between DOCC increment or read/write')
parser.add_argument('readfrac', type=float, help='choose between DOCC increment or read/write')
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

experiment_common.copyFiles("docc", "apps/benchmarks/build")
experiment_common.runProcesses(getCommandFunc, NUM_CLIENTS, "docc-out")
experiment_common.processOutput(processOutputFunc)
