#!/usr/bin/python

import re
import subprocess

def logPrint(msg):
    subprocess.call("echo %s | tee -a %s" % (msg, LOG), shell=True)

SRC_HOST = "moranis.cs.washington.edu"
DATA_REDIS_PORT = 6379
REDIS_DIR = "redis-3.0.7/src";
WORKING_DIR = "/scratch/nl35";

CONFIG_PREFIX = "platform/test/niel"
BATCH_SIZE = 64

OUTPUT_DIR = "results/caching"
LOG = "caching-log.txt"

machines = ["charlottetown.cs.washington.edu"]

startDiamondCmd = "ssh -t %s 'cd diamond-src/scripts; ./manage-servers.py start ../%s --batch %d' >> %s 2>&1" % (SRC_HOST, CONFIG_PREFIX, BATCH_SIZE, LOG)
killDiamondCmd = "ssh %s 'cd diamond-src/scripts; ./manage-servers.py kill ../%s' >> %s 2>&1" % (SRC_HOST, CONFIG_PREFIX, LOG)
startRedisCmd = "ssh -f %s 'nohup %s/redis-server &' >> %s 2>&1" % (SRC_HOST, REDIS_DIR, LOG)
killRedisCmd = "ssh %s 'pkill -f %d'" % (SRC_HOST, DATA_REDIS_PORT)
clearRedisCmd = "ssh %s '%s/redis-cli -p %d flushdb' >> %s 2>&1" % (SRC_HOST, REDIS_DIR, DATA_REDIS_PORT, LOG)

def runDiamond(caching, numPairsPerMachine, machineNums):
    logPrint("Running Diamond with caching %s" % (caching))
    outFileName = ""
    if caching:
        outFileName = OUTPUT_DIR + "/diamond.caching.txt"
    else:
        outFileName = OUTPUT_DIR + "/diamond.nocaching.txt"
    outFile = open(outFileName, "w")

    subprocess.call(startRedisCmd, shell=True)

    outFile.write("clients\tthroughput\tlatency\tseconds\tmachines\n")

    for numMachines in machineNums:
        logPrint("Running %d machines" % numMachines)
        subprocess.call(startDiamondCmd, shell=True)
        subprocess.call(clearRedisCmd, shell=True)

        processes = []
        for i in range(0, numMachines):
            machine = machines[i]
            subprocess.call("ssh %s 'rsync %s:diamond-src/scripts/experiments/run_game.py %s'" % (machine, SRC_HOST, WORKING_DIR), shell=True)
            subprocess.call("ssh %s 'rsync %s:diamond-src/scripts/experiments/client_common.py %s'" % (machine, SRC_HOST, WORKING_DIR), shell=True)

        cachingArg = ""
        if not caching:
            cachingArg = "--nocaching"
        for i in range(0, numMachines):
            machine = machines[i]
            p = subprocess.Popen("ssh %s 'cd %s; ./run_game.py --config %s --numpairs %d %s' >> %s 2>&1" \
                    % (machine, WORKING_DIR, CONFIG_PREFIX, numPairsPerMachine, cachingArg, LOG), shell=True)
            processes.append(p)

        for p in processes:
            p.wait()

        clients = int(subprocess.check_output("ssh %s '%s/redis-cli -p %d get clients'" % (SRC_HOST, REDIS_DIR, DATA_REDIS_PORT), shell=True))
        result = subprocess.check_output("ssh %s 'diamond-src/scripts/experiments/parse-game.py -r'" % SRC_HOST, shell=True)
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

        subprocess.call(killDiamondCmd, shell=True)

    outFile.close()

# clear log file
subprocess.call("rm %s" % LOG, shell=True)

# run experiments
runDiamond(True, 10, [1])
runDiamond(False, 10, [1])
