#!/usr/bin/python

# This script is pretty similar to run-baseline.py, except it doesn't have any of the code to
# run the baseline Redis/Jetty system, and it uses parse-retwis.py when parsing the results
# to break out the numbers by transaction type.

import experiment_common
from experiment_common import logPrint
import re
import subprocess

CONFIG_PREFIX = "platform/test/niel"
KEY_FILE = "scripts/experiments/keys.txt"
NUM_KEYS = 100000
BATCH_SIZE = 64

OUTPUT_DIR = "results/docc"
LOG = "docc-log.txt"

experiment_common.setLog(LOG)
experiment_common.init()

experiment_common.copyFromSrcHostToClients("scripts/experiments/run_retwis.py")
experiment_common.copyFromSrcHostToClients("scripts/experiments/client_common.py")

def runDiamond(isolation, zipf, numClientsPerMachine, machineNums):
    logPrint("Running Diamond with isolation %s and zipf %f" % (isolation, zipf))
    outFileName = OUTPUT_DIR + "/diamond." + isolation + "." + repr(zipf) + ".txt"
    outFile = open(outFileName, "w")

    experiment_common.startDataRedis()

    outFile.write("clients\tthroughput\tlatency\tabortrate\tseconds\tmachines\t")
    for i in range(1, 6):
        outFile.write("throughput-%d\tlatency-%d\tabortrate-%d\t" % (i, i, i))
    outFile.write("\n")

    for numMachines in machineNums:
        logPrint("Running %d machines" % numMachines)
        experiment_common.startDiamond(CONFIG_PREFIX, keys=KEY_FILE, numKeys=NUM_KEYS, batchSize=BATCH_SIZE)
        experiment_common.clearDataRedis()

        command = "./run_retwis.py --config %s --numclients %d --keys %s --numkeys %d --isolation %s --zipf %f" \
                % (CONFIG_PREFIX, numClientsPerMachine, KEY_FILE, NUM_KEYS, isolation, zipf)
        experiment_common.runOnClientMachines(command, numMachines)

        clients = experiment_common.getClientCountFromDataRedis()
        result = subprocess.check_output("ssh %s 'diamond-src/scripts/experiments/parse-retwis.py -r'" % experiment_common.getSrcHost(), shell=True)
        throughput = dict()
        latency = dict()
        abortRate = dict()
        seconds = "ERROR"
        for line in result.split("\n"):
            match = re.match("(\S+)\s+([\d\.]+)\s+([\d\.]+)\s+([\d\.]+)\s+\d+\s+\d+", line)
            if match:
                txnType = match.group(1)
                throughput[txnType] = match.group(2)
                latency[txnType] = match.group(3)
                abortRate[txnType] = match.group(4)
            match = re.search("over ([\d\.]+) seconds", line)
            if match:
                seconds = match.group(1)
        outFile.write("%d\t%s\t%s\t%s\t%s\t%d\t" % (clients, throughput["Overall"], latency["Overall"], abortRate["Overall"], seconds, numMachines))
        for i in range(1, 6):
            outFile.write("%s\t%s\t%s\t" % (throughput[repr(i)], latency[repr(i)], abortRate[repr(i)]))
        outFile.write("\n")

        experiment_common.killDiamond(CONFIG_PREFIX)

    outFile.close()

# run experiments
runDiamond("linearizabledocc", 0.8, 10, [1])
runDiamond("linearizable", 0.8, 10, [1])
