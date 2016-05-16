#!/usr/bin/python

import argparse
import client_common
import os
import random
import subprocess
import sys

parser = argparse.ArgumentParser(description='Run 100 game benchmark.')
parser.add_argument('--numpairs', type=int, default=50, help='number of client pairs')
parser.add_argument('--nocaching', action="store_true", help='disable caching')
client_common.addCommonArgs(parser)
args = parser.parse_args()
client_common.handleCommonArgs(args)

WORKING_DIR = client_common.getWorkingDir()
NUM_FRONTENDS = client_common.getNumFrontends()
        
client_common.copyCommonFiles()
client_common.copyIntoWorkingDir("apps/benchmarks/build/game")

sys.stderr.write("Running clients...\n")

processes = []
outputFiles = []

for i in range(0, args.numpairs):
    gameKeyPrefix = repr(random.randint(0, sys.maxint))
    outputFile1 = "game-" + gameKeyPrefix + "-1"
    outputFile2 = "game-" + gameKeyPrefix + "-2"
    configFile = "diamond.frontend" + repr(i % NUM_FRONTENDS)
    noCachingArg = ""
    if args.nocaching:
        noCachingArg = " --nocaching "
    cmd1 = WORKING_DIR + "/game --config " + WORKING_DIR + "/" + configFile + " --name player1 --keyprefix " + gameKeyPrefix + noCachingArg + " > " + WORKING_DIR + "/" + outputFile1
    cmd2 = WORKING_DIR + "/game --config " + WORKING_DIR + "/" + configFile + " --name player2 --keyprefix " + gameKeyPrefix + noCachingArg + " > " + WORKING_DIR + "/" + outputFile2
    processes.append(subprocess.Popen(cmd1, shell=True))
    processes.append(subprocess.Popen(cmd2, shell=True))
    outputFiles.append(outputFile1)
    outputFiles.append(outputFile2)

for process in processes:
    process.wait()

sys.stderr.write("Finished running clients\n")

for outputFileName in outputFiles:
    fullPath = os.path.expanduser(WORKING_DIR) + "/" + outputFileName
    client_common.putGameDataInRedis(fullPath)
    subprocess.call(["rm", fullPath]);
    sys.stderr.write("Finished processing output file %s\n" % outputFileName)
