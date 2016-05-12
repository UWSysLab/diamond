#!/usr/bin/python

import os
import random
import re
import redis
import subprocess
import sys

DATA_REDIS_HOST = "moranis.cs.washington.edu"
DATA_REDIS_PORT = "6379"
SRC_HOST = "moranis.cs.washington.edu"
WORKING_DIR = "/scratch/nl35"

CONFIG_DIR = "platform/test/"
KEY_DIR = "scripts/experiments/"
KEY_FILE = "keys.txt"
NUM_FRONTENDS = 1

processes = []
outputFiles = []

def copyCommonFiles(configPrefix):
    copyIntoWorkingDir("platform/build/libdiamond.so");
    copyConfigFiles(configPrefix)

def copyConfigFiles(configPrefix):
    for i in range(0, NUM_FRONTENDS):
        configFile = configPrefix + ".frontend" + repr(i)
        copyIntoWorkingDir(CONFIG_DIR + configFile + ".config")
    copyIntoWorkingDir(KEY_DIR + KEY_FILE)

def copyIntoWorkingDir(filename):
    os.system("rsync " + SRC_HOST + ":diamond-src/" + filename + " " + WORKING_DIR)

# getCommandFunc should be a function with arguments (workingDir, configFile, keyFile) that
# returns the the shell command to execute the binary
def runProcesses(getCommandFunc, numClients, configPrefix, outputPrefix):
    sys.stderr.write("Running clients...\n")

    for i in range(0, numClients):
        outputFile = outputPrefix + "-" + repr(random.randint(0, sys.maxint))
        configFile = configPrefix + ".frontend" + repr(i % NUM_FRONTENDS)
        cmd = getCommandFunc(WORKING_DIR, configFile, KEY_FILE) + " > " + WORKING_DIR + "/" + outputFile
        processes.append(subprocess.Popen(cmd, shell=True))
        outputFiles.append(outputFile)

    for process in processes:
        process.wait()

    sys.stderr.write("Finished running clients\n")
    success = True
    for process in processes:
        if process.returncode:
            success = False
    if not success:
        sys.stderr.write("Error: some clients returned nonzero return codes\n")


def processOutput(processOutputFunc):
    for outputFileName in outputFiles:
        fullPath = os.path.expanduser(WORKING_DIR) + "/" + outputFileName
        processOutputFunc(fullPath)
        sys.stderr.write("Finished processing output file %s\n" % outputFileName)

def putDataInRedis(outputFileName):
    r = redis.StrictRedis(host=DATA_REDIS_HOST, port=DATA_REDIS_PORT)
    r.incr("clients")
    outputFile = open(outputFileName, 'r')
    for line in outputFile:
        match = re.match("^(\d+)\s+(\d+)\s+(\d+)", line)
        if match:
            startTime = match.group(1)
            endTime = match.group(2)
            committed = match.group(3)
            txnNum = random.randint(0, sys.maxint)
            txnKey = "txn-" + repr(txnNum)
            mapping = dict()
            mapping['start-time'] = startTime
            mapping['end-time'] = endTime
            mapping['committed'] = committed
            r.hmset(txnKey, mapping)
            r.lpush("txns", txnKey)

def putRetwisDataInRedis(outputFileName):
    r = redis.StrictRedis(host=DATA_REDIS_HOST, port=DATA_REDIS_PORT)
    r.incr("clients")
    outfile = open(outputFileName, 'r')
    for line in outfile:
        match = re.match("^\d+\s+([\d\.]+)\s+([\d\.]+)\s+\d+\s+(\d+)\s+(\d+)", line)
        if match:
            startTime = int(float(match.group(1)) * 1000)
            endTime = int(float(match.group(2)) * 1000)
            committed = int(match.group(3))
            txnType = int(match.group(4))
            txnNum = random.randint(0, sys.maxint)
            txnKey = "txn-" + repr(txnNum)
            mapping = dict()
            mapping['start-time'] = startTime
            mapping['end-time'] = endTime
            mapping['committed'] = committed
            mapping['type'] = txnType
            r.hmset(txnKey, mapping)
            r.lpush("txns", txnKey)
