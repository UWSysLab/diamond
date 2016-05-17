#!/usr/bin/python

import experiment_common
from experiment_common import logPrint
import re
import subprocess
import sys

CONFIG_PREFIX = "diamond-src/platform/test/niel"
BATCH_SIZE = 64

OUTPUT_DIR = "results/caching"
LOG = "caching-log.txt"

SRC_HOST = experiment_common.getSrcHost()

experiment_common.setLog(LOG)
experiment_common.init()

experiment_common.copyFromSrcHostToClients("diamond-src/scripts/experiments/run_game.py")
experiment_common.copyFromSrcHostToClients("diamond-src/scripts/experiments/run_redis_game.py")
experiment_common.copyFromSrcHostToClients("diamond-src/scripts/experiments/client_common.py")

def runBaseline(numPairsPerMachine, machineNums):
    logPrint("Running baseline")
    outFileName = OUTPUT_DIR + "/baseline.txt"
    outFile = open(outFileName, "w")

    startBaselineCmd = "ssh -t %s 'diamond-src/apps/baseline-benchmarks/100game/manage-baseline-100game-servers.py start %s' >> %s 2>&1" \
            % (SRC_HOST, CONFIG_PREFIX, LOG)
    killBaselineCmd = "ssh %s 'diamond-src/apps/baseline-benchmarks/100game/manage-baseline-100game-servers.py kill %s' >> %s 2>&1" % (SRC_HOST, CONFIG_PREFIX, LOG)

    # read hostname and port of Redis leader from config file
    hostname = ""
    port = ""
    config = subprocess.check_output("ssh %s 'cat %s0.config'" % (SRC_HOST, CONFIG_PREFIX), shell=True).split("\n")
    match = re.match("replica\s+([\d\w\.]+):(\d+)", config[1])
    if match:
        hostname = match.group(1)
        port = match.group(2)
    else:
        print("Error reading hostname and port of Redis leader from config file")
        sys.exit(1)

    experiment_common.startDataRedis()

    outFile.write("clients\tthroughput\tlatency\tseconds\tmachines\n")

    for numMachines in machineNums:
        logPrint("Running %d machines" % numMachines)
        subprocess.call(startBaselineCmd, shell=True)
        experiment_common.clearDataRedis()

        command = "./run_redis_game.py --hostname %s --port %s --numpairs %d" \
                % (hostname, port, numPairsPerMachine)
        experiment_common.runOnClientMachines(command, numMachines)

        clients = experiment_common.getClientCountFromDataRedis()
        result = subprocess.check_output("ssh %s 'diamond-src/scripts/experiments/parse-game.py -r'" % experiment_common.getSrcHost(), shell=True)
        for line in result.split("\n"):
            match = re.match("Avg\. turns\/second: ([\d\.]+)", line)
            if match:
                throughput = match.group(1)
            match = re.match("Avg\. time between turns \(s\): ([\d\.]+)", line)
            if match:
                latency = match.group(1)
            match = re.search("over ([\d\.]+) seconds", line)
            if match:
                seconds = match.group(1)
        outFile.write("%d\t%s\t%s\t%s\t%d\n" % (clients, throughput, latency, seconds, numMachines))

        subprocess.call(killBaselineCmd, shell=True)

def runDiamond(caching, numPairsPerMachine, machineNums):
    logPrint("Running Diamond with caching %s" % (caching))
    outFileName = ""
    if caching:
        outFileName = OUTPUT_DIR + "/diamond.caching.txt"
    else:
        outFileName = OUTPUT_DIR + "/diamond.nocaching.txt"
    outFile = open(outFileName, "w")

    experiment_common.startDataRedis()

    outFile.write("clients\tthroughput\tlatency\tseconds\tmachines\n")

    for numMachines in machineNums:
        logPrint("Running %d machines" % numMachines)
        experiment_common.startDiamond(CONFIG_PREFIX, batchSize=64)
        experiment_common.clearDataRedis()

        cachingArg = ""
        if not caching:
            cachingArg = "--nocaching"
        command = "./run_game.py --config %s --numpairs %d %s" % (CONFIG_PREFIX, numPairsPerMachine, cachingArg)
        experiment_common.runOnClientMachines(command, numMachines)

        clients = experiment_common.getClientCountFromDataRedis()
        result = subprocess.check_output("ssh %s 'diamond-src/scripts/experiments/parse-game.py -r'" % experiment_common.getSrcHost(), shell=True)
        for line in result.split("\n"):
            match = re.match("Avg\. turns\/second: ([\d\.]+)", line)
            if match:
                throughput = match.group(1)
            match = re.match("Avg\. time between turns \(s\): ([\d\.]+)", line)
            if match:
                latency = match.group(1)
            match = re.search("over ([\d\.]+) seconds", line)
            if match:
                seconds = match.group(1)
        outFile.write("%d\t%s\t%s\t%s\t%d\n" % (clients, throughput, latency, seconds, numMachines))

        experiment_common.killDiamond(CONFIG_PREFIX)

    outFile.close()

# run experiments
runDiamond(True, 10, [1])
runDiamond(False, 10, [1])
runBaseline(10, [1])
