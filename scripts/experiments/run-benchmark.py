#!/usr/bin/python

import argparse
from pyrem.host import RemoteHost

WORKING_DIR = "/scratch/nl35/benchmark"
DIAMOND_SRC = "../.." 
OUTPUT_DIR = "."

HOST1 = RemoteHost('moranis.cs.washington.edu')
#HOST2 = RemoteHost('charlottetown.cs.washington.edu')

#hosts = [HOST1, HOST2]
hosts = [HOST1]

parser = argparse.ArgumentParser(description='Run benchmark.')
parser.add_argument('num_clients', type=int, help='the number of benchmark clients')
parser.add_argument('frontend_config_prefix', help='the frontend config file prefix')
args = parser.parse_args()

numClients = args.num_clients

frontendConfigPath = args.frontend_config_prefix + ".config"
executablePath = DIAMOND_SRC + "/apps/benchmarks/build/benchmarkclient"
outputPrefix = OUTPUT_DIR + "/output"

remoteFrontendConfigPrefix = WORKING_DIR + "/benchmark.frontend"
remoteFrontendConfigPath = remoteFrontendConfigPrefix + ".config"
remoteExecutablePath = WORKING_DIR + "/benchmarkclient"
remoteOutputPrefix = WORKING_DIR + "/output"

for host in hosts:
    host.send_file(frontendConfigPath, remoteFrontendConfigPath).start(wait=True)
    host.send_file(executablePath, remoteExecutablePath).start(wait=True)

tasks = []
taskHosts = [] # maps each task number to the host holding that task

hostNum = 0
for i in range(0, numClients):
    host = hosts[hostNum]
    remoteOutputPath = remoteOutputPrefix + "." + repr(i) + ".txt"
    client = host.run([remoteExecutablePath, '--config', remoteFrontendConfigPrefix, '> ', remoteOutputPath])
    taskHosts.append(host)
    tasks.append(client)
    hostNum = hostNum + 1
    if hostNum >= len(hosts):
        hostNum = 0

for task in tasks:
    task.start()

for task in tasks:
    task.wait()

for i in range(0, numClients):
    host = taskHosts[i]
    remoteOutputPath = remoteOutputPrefix + "." + repr(i) + ".txt"
    outputPath = outputPrefix + "." + repr(i) + ".txt"
    host.get_file(remoteOutputPath, outputPath).start(wait=True)
