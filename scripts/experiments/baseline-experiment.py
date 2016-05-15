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
KEY_FILE = "scripts/experiments/keys.txt"
NUM_KEYS = 100000
BATCH_SIZE = 64

OUTPUT_DIR = "results/baseline"
LOG = "baseline-log.txt"

machines = ["charlottetown.cs.washington.edu"]

startDiamondCmd = "ssh -t %s 'cd diamond-src/scripts; ./manage-servers.py start ../%s --keys ../%s --numkeys %d --batch %d' >> %s 2>&1" % (SRC_HOST, CONFIG_PREFIX, KEY_FILE, NUM_KEYS, BATCH_SIZE, LOG)
killDiamondCmd = "ssh %s 'cd diamond-src/scripts; ./manage-servers.py kill ../%s' >> %s 2>&1" % (SRC_HOST, CONFIG_PREFIX, LOG)
killBaselineCmd = "ssh %s 'cd diamond-src/apps/baseline-benchmarks/keyvaluestore; ./manage-baseline-servers.py kill ../../../%s' >> %s 2>&1" % (SRC_HOST, CONFIG_PREFIX, LOG)
startRedisCmd = "ssh -f %s 'nohup %s/redis-server &' >> %s 2>&1" % (SRC_HOST, REDIS_DIR, LOG)
killRedisCmd = "ssh %s 'pkill -f %d'" % (SRC_HOST, DATA_REDIS_PORT)
clearRedisCmd = "ssh %s '%s/redis-cli -p %d flushdb' >> %s 2>&1" % (SRC_HOST, REDIS_DIR, DATA_REDIS_PORT, LOG)

def runBaseline(zipf, numClientsPerMachine, machineNums):
    logPrint("Running baseline with zipf %f" % zipf)
    outFileName = OUTPUT_DIR + "/baseline." + repr(zipf) + ".txt"
    outFile = open(outFileName, "w")

    startBaselineCmd = "ssh -t %s 'cd diamond-src/apps/baseline-benchmarks/keyvaluestore; ./manage-baseline-servers.py start ../../../%s --keys ../../../%s --numkeys %d --zipf %f' >> %s 2>&1" \
            % (SRC_HOST, CONFIG_PREFIX, KEY_FILE, NUM_KEYS, zipf, LOG)

    subprocess.call(startRedisCmd, shell=True)

    outFile.write("clients\tthroughput\tlatency\tabortrate\tseconds\tmachines\n")
    for numMachines in machineNums:
        logPrint("Running %d machines" % numMachines)
        subprocess.call(startBaselineCmd, shell=True)
        subprocess.call(clearRedisCmd, shell=True)

        processes = []
        for i in range(0, numMachines):
            machine = machines[i]
            subprocess.call("ssh %s 'rsync %s:diamond-src/scripts/experiments/run_baseline_retwis.py %s'" % (machine, SRC_HOST, WORKING_DIR), shell=True)
            subprocess.call("ssh %s 'rsync %s:diamond-src/scripts/experiments/experiment_common.py %s'" % (machine, SRC_HOST, WORKING_DIR), shell=True)

        for i in range(0, numMachines):
            machine = machines[i]
            p = subprocess.Popen("ssh %s 'cd %s; ./run_baseline_retwis.py --config %s --numclients %d --keys %s --numkeys %d' >> %s 2>&1" \
                    % (machine, WORKING_DIR, CONFIG_PREFIX, numClientsPerMachine, KEY_FILE, NUM_KEYS, LOG), shell=True)
            processes.append(p)

        for p in processes:
            p.wait()

        clients = int(subprocess.check_output("ssh %s '%s/redis-cli -p %d get clients'" % (SRC_HOST, REDIS_DIR, DATA_REDIS_PORT), shell=True))
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

    subprocess.call(startRedisCmd, shell=True)

    outFile.write("clients\tthroughput\tlatency\tabortrate\tseconds\tmachines\n")
    for numMachines in machineNums:
        subprocess.call(startDiamondCmd, shell=True)
        subprocess.call(clearRedisCmd, shell=True)

        processes = []
        for i in range(0, numMachines):
            machine = machines[i]
            subprocess.call("ssh %s 'rsync %s:diamond-src/scripts/experiments/run_retwis.py %s'" % (machine, SRC_HOST, WORKING_DIR), shell=True)
            subprocess.call("ssh %s 'rsync %s:diamond-src/scripts/experiments/experiment_common.py %s'" % (machine, SRC_HOST, WORKING_DIR), shell=True)

        for i in range(0, numMachines):
            machine = machines[i]
            p = subprocess.Popen("ssh %s 'cd %s; ./run_retwis.py --config %s --numclients %d --keys %s --numkeys %d --isolation %s --zipf %f' >> %s 2>&1" \
                    % (machine, WORKING_DIR, CONFIG_PREFIX, numClientsPerMachine, KEY_FILE, NUM_KEYS, isolation, zipf, LOG), shell=True)
            processes.append(p)

        for p in processes:
            p.wait()

        clients = int(subprocess.check_output("ssh %s '%s/redis-cli -p %d get clients'" % (SRC_HOST, REDIS_DIR, DATA_REDIS_PORT), shell=True))
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

        subprocess.call(killDiamondCmd, shell=True)

    outFile.close()

# clear log file
subprocess.call("rm %s" % LOG, shell=True)

# run experiments
runDiamond("linearizable", 0.8, 10, [1])
runBaseline(0.8, 10, [1])
