#!/usr/bin/python

# This script is pretty similar to run-baseline.py, except it doesn't have any of the code to
# run the baseline Redis/Jetty system, and it uses parse-retwis.py when parsing the results
# to break out the numbers by transaction type.

import experiment_common
from experiment_common import logPrint
import re
import subprocess

SRC_HOST = "moranis.cs.washington.edu"
DATA_REDIS_PORT = 6379
REDIS_DIR = "redis-3.0.7/src";
WORKING_DIR = "/scratch/nl35";

CONFIG_PREFIX = "platform/test/niel"
KEY_FILE = "scripts/experiments/keys.txt"
NUM_KEYS = 100000
BATCH_SIZE = 64

CLIENTS_FILE = "clients.txt"

OUTPUT_DIR = "results/docc"
LOG = "docc-log.txt"

machines = experiment_common.readClients(CLIENTS_FILE)

startDiamondCmd = "ssh -t %s 'cd diamond-src/scripts; ./manage-servers.py start ../%s --keys ../%s --numkeys %d --batch %d' >> %s 2>&1" % (SRC_HOST, CONFIG_PREFIX, KEY_FILE, NUM_KEYS, BATCH_SIZE, LOG)
killDiamondCmd = "ssh %s 'cd diamond-src/scripts; ./manage-servers.py kill ../%s' >> %s 2>&1" % (SRC_HOST, CONFIG_PREFIX, LOG)
startRedisCmd = "ssh -f %s 'nohup %s/redis-server &' >> %s 2>&1" % (SRC_HOST, REDIS_DIR, LOG)
killRedisCmd = "ssh %s 'pkill -f %d'" % (SRC_HOST, DATA_REDIS_PORT)
clearRedisCmd = "ssh %s '%s/redis-cli -p %d flushdb' >> %s 2>&1" % (SRC_HOST, REDIS_DIR, DATA_REDIS_PORT, LOG)

experiment_common.setLog(LOG)

def runDiamond(isolation, zipf, numClientsPerMachine, machineNums):
    logPrint("Running Diamond with isolation %s and zipf %f" % (isolation, zipf))
    outFileName = OUTPUT_DIR + "/diamond." + isolation + "." + repr(zipf) + ".txt"
    outFile = open(outFileName, "w")

    subprocess.call(startRedisCmd, shell=True)

    outFile.write("clients\tthroughput\tlatency\tabortrate\tseconds\tmachines\t")
    for i in range(1, 6):
        outFile.write("throughput-%d\tlatency-%d\tabortrate-%d\t" % (i, i, i))
    outFile.write("\n")

    for numMachines in machineNums:
        logPrint("Running %d machines" % numMachines)
        subprocess.call(startDiamondCmd, shell=True)
        subprocess.call(clearRedisCmd, shell=True)

        processes = []
        for i in range(0, numMachines):
            machine = machines[i]
            subprocess.call("ssh %s 'rsync %s:diamond-src/scripts/experiments/run_retwis.py %s'" % (machine, SRC_HOST, WORKING_DIR), shell=True)
            subprocess.call("ssh %s 'rsync %s:diamond-src/scripts/experiments/client_common.py %s'" % (machine, SRC_HOST, WORKING_DIR), shell=True)

        for i in range(0, numMachines):
            machine = machines[i]
            p = subprocess.Popen("ssh %s 'cd %s; ./run_retwis.py --config %s --numclients %d --keys %s --numkeys %d --isolation %s --zipf %f' >> %s 2>&1" \
                    % (machine, WORKING_DIR, CONFIG_PREFIX, numClientsPerMachine, KEY_FILE, NUM_KEYS, isolation, zipf, LOG), shell=True)
            processes.append(p)

        for p in processes:
            p.wait()

        clients = int(subprocess.check_output("ssh %s '%s/redis-cli -p %d get clients'" % (SRC_HOST, REDIS_DIR, DATA_REDIS_PORT), shell=True))
        result = subprocess.check_output("ssh %s 'diamond-src/scripts/experiments/parse-retwis.py -r'" % SRC_HOST, shell=True)
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

        subprocess.call(killDiamondCmd, shell=True)

    outFile.close()

# clear log file
subprocess.call("rm %s" % LOG, shell=True)

# run experiments
runDiamond("linearizabledocc", 0.8, 10, [1])
runDiamond("linearizable", 0.8, 10, [1])
