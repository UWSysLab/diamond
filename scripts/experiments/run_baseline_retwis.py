#!/usr/bin/python

import argparse
import client_common
import os

parser = argparse.ArgumentParser(description='Run baseline client.')
parser.add_argument('--numclients', type=int, default=128, help='number of clients')
parser.add_argument('--time', type=int, default=60, help='number of seconds to run')
client_common.addCommonArgs(parser)
args = parser.parse_args()
client_common.handleCommonArgs(args)

def getCommandFunc(workingDir, configFile, keyFile, numKeys):
    return "java -cp " + workingDir + "/keyvaluestore-1.0-SNAPSHOT-jar-with-dependencies.jar edu.washington.cs.diamond.RetwisClient " + workingDir + "/" + configFile + ".config" + " " + workingDir + "/" + keyFile + " " + repr(numKeys) + " " + repr(args.time)

#copy files
client_common.copyFromSrcHost("diamond-src/apps/baseline-benchmarks/keyvaluestore/target/keyvaluestore-1.0-SNAPSHOT-jar-with-dependencies.jar")
client_common.copyCommonFiles()

client_common.runProcesses(getCommandFunc, args.numclients)
client_common.processOutput(client_common.putRetwisDataInRedis)
