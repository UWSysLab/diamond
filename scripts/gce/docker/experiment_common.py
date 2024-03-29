#!/usr/bin/python

import os
import random
import re
import redis
import subprocess
import sys

SRC_HOST = "diamond-client"
WORKING_DIR = "~/"

COPY_CMD = "rsync " + SRC_HOST + ":diamond-src/"
CONFIG_DIR = "platform/test/"
KEY_DIR = "scripts/experiments/"
KEY_FILE = "keys.txt"
NUM_FRONTENDS = 16
USE_REDIS = True

processes = []
outputFiles = []

def copyFiles(binary, binaryDir, configPrefix):
    copyBinary(binary, binaryDir)
    #copyConfigFiles(configPrefix)

def copyBinary(binary, binaryDir):
    os.system(COPY_CMD + binaryDir + "/" + binary + " " + WORKING_DIR)
    os.system(COPY_CMD + "platform/build/libdiamond.so" + " " + WORKING_DIR)
    sys.stderr.write("Finished copying binaries\n")

def copyConfigFiles(configPrefix):
    for i in range(0, NUM_FRONTENDS):
        configFile = configPrefix + ".frontend" + repr(i)
        os.system(COPY_CMD + CONFIG_DIR + configFile + ".config" + " " + WORKING_DIR)
        sys.stderr.write("Finished syncing config file %d\n" % i)
    os.system(COPY_CMD + KEY_DIR + KEY_FILE + " " + WORKING_DIR)
    sys.stderr.write("Finished syncing key file\n")

def copyIntoWorkingDir(filename):
    os.system(COPY_CMD + filename + " " + WORKING_DIR)

# getCommandFunc should be a function with arguments (workingDir, configFile, keyFile) that
# returns the the shell command to execute the binary
def runProcesses(getCommandFunc, numClients, configPrefix, outputPrefix):
    sys.stderr.write("Running clients...\n")

    for i in range(0, numClients):
        outputFile = outputPrefix + "-" + repr(random.randint(0, sys.maxint))
        configFile = configPrefix + ".frontend" + repr(i % NUM_FRONTENDS)
        cmd = getCommandFunc(WORKING_DIR, configFile, KEY_FILE) + " > " + WORKING_DIR + outputFile
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
        import time
        while True:
            time.sleep(1) # make sure we notice that clients failed


def processOutput(processOutputFunc):
    for outputFileName in outputFiles:
        fullPath = os.path.expanduser(WORKING_DIR) + outputFileName
        processOutputFunc(fullPath)
        sys.stderr.write("Finished processing output file %s\n" % outputFileName)

def putDataInRedis(outputFileName, redisHost=SRC_HOST, redisPort=6379):
    r = redis.StrictRedis(host=redisHost, port=redisPort)
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

def copyToSrcHost(fileName, dest):
    os.system("rsync " + fileName + " " + SRC_HOST + ":diamond-src/" + dest)
