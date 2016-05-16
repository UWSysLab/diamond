#!/usr/bin/python

import experiment_common
from experiment_common import logPrint
import re
import subprocess

CONFIG_PREFIX = "platform/test/niel"
KEY_FILE = "scripts/experiments/keys.txt"
NUM_KEYS = 100000
BATCH_SIZE = 64

OUTPUT_DIR = "results/baseline"
LOG = "baseline-log.txt"

SRC_HOST = experiment_common.getSrcHost()

experiment_common.setLog(LOG)
experiment_common.init()

experiment_common.copyFromSrcHostToClients("scripts/experiments/run_retwis.py")
experiment_common.copyFromSrcHostToClients("scripts/experiments/run_baseline_retwis.py")
experiment_common.copyFromSrcHostToClients("scripts/experiments/client_common.py")

def runBaseline(zipf, numClientsPerMachine, machineNums):
    logPrint("Running baseline with zipf %f" % zipf)
    outFileName = OUTPUT_DIR + "/baseline." + repr(zipf) + ".txt"
    outFile = open(outFileName, "w")

    startBaselineCmd = "ssh -t %s 'cd diamond-src/apps/baseline-benchmarks/keyvaluestore; ./manage-baseline-servers.py start ../../../%s --keys ../../../%s --numkeys %d --zipf %f' >> %s 2>&1" \
            % (SRC_HOST, CONFIG_PREFIX, KEY_FILE, NUM_KEYS, zipf, LOG)
    killBaselineCmd = "ssh %s 'cd diamond-src/apps/baseline-benchmarks/keyvaluestore; ./manage-baseline-servers.py kill ../../../%s' >> %s 2>&1" % (SRC_HOST, CONFIG_PREFIX, LOG)

    experiment_common.startDataRedis()

    outFile.write("clients\tthroughput\tlatency\tabortrate\tseconds\tmachines\n")
    for numMachines in machineNums:
        logPrint("Running %d machines" % numMachines)

        subprocess.call(startBaselineCmd, shell=True)
        experiment_common.clearDataRedis()

        command = "./run_baseline_retwis.py --config %s --numclients %d --keys %s --numkeys %d" \
                % (CONFIG_PREFIX, numClientsPerMachine, KEY_FILE, NUM_KEYS)
        experiment_common.runOnClientMachines(command, numMachines)

        clients = experiment_common.getClientCountFromDataRedis()
        result = subprocess.check_output("ssh %s 'diamond-src/scripts/experiments/parse-scalability.py -r'" % SRC_HOST, shell=True)
        throughput = "ERROR"
        latency = "ERROR"
        abortRate = "ERROR"
        seconds = "ERROR"
        for line in result.split("\n"):
            match = re.match("Avg\. throughput \(txn\/s\): ([\d\.]+)", line)
            if match:
                throughput = match.group(1)
            match = re.match("Avg\. latency \(s\): ([\d\.]+)", line)
            if match:
                latency = match.group(1)
            match = re.match("Abort rate: ([\d\.]+)", line)
            if match:
                abortRate = match.group(1)
            match = re.search("over ([\d\.]+) seconds", line)
            if match:
                seconds = match.group(1)
        outFile.write("%d\t%s\t%s\t%s\t%s\t%d\n" % (clients, throughput, latency, abortRate, seconds, numMachines))

        subprocess.call(killBaselineCmd, shell=True)

    outFile.close()

def runDiamond(isolation, zipf, numClientsPerMachine, machineNums):
    logPrint("Running Diamond with isolation %s and zipf %f" % (isolation, zipf))
    outFileName = OUTPUT_DIR + "/diamond." + isolation + "." + repr(zipf) + ".txt"
    outFile = open(outFileName, "w")

    experiment_common.startDataRedis()

    outFile.write("clients\tthroughput\tlatency\tabortrate\tseconds\tmachines\n")
    for numMachines in machineNums:
        logPrint("Running %d machines" % numMachines)

        experiment_common.startDiamond(CONFIG_PREFIX, keys=KEY_FILE, numKeys=NUM_KEYS, batchSize=64)
        experiment_common.clearDataRedis()

        command = "./run_retwis.py --config %s --numclients %d --keys %s --numkeys %d --isolation %s --zipf %f" \
                % (CONFIG_PREFIX, numClientsPerMachine, KEY_FILE, NUM_KEYS, isolation, zipf)
        experiment_common.runOnClientMachines(command, numMachines)

        clients = experiment_common.getClientCountFromDataRedis()
        result = subprocess.check_output("ssh %s 'diamond-src/scripts/experiments/parse-scalability.py -r'" % SRC_HOST, shell=True)
        throughput = "ERROR"
        latency = "ERROR"
        abortRate = "ERROR"
        seconds = "ERROR"
        for line in result.split("\n"):
            match = re.match("Avg\. throughput \(txn\/s\): ([\d\.]+)", line)
            if match:
                throughput = match.group(1)
            match = re.match("Avg\. latency \(s\): ([\d\.]+)", line)
            if match:
                latency = match.group(1)
            match = re.match("Abort rate: ([\d\.]+)", line)
            if match:
                abortRate = match.group(1)
            match = re.search("over ([\d\.]+) seconds", line)
            if match:
                seconds = match.group(1)
        outFile.write("%d\t%s\t%s\t%s\t%s\t%d\n" % (clients, throughput, latency, abortRate, seconds, numMachines))

        experiment_common.killDiamond(CONFIG_PREFIX)

    outFile.close()

# run experiments
runDiamond("linearizable", 0.8, 10, [1, 2])
runBaseline(0.8, 10, [1, 2])
