#!/usr/bin/python

import argparse
import os
import re
import time
from pyrem.host import RemoteHost

USER_NAME = "nl35"
WORKING_DIR = "/biggerraid/users/" + USER_NAME + "/scratch"
BUILD_DIR = "../platform/build"

parser = argparse.ArgumentParser(description='Launch servers.')
parser.add_argument('action', choices=['start', 'kill'], help='the action to take')
parser.add_argument('config_prefix', help='the config file prefix')
args = parser.parse_args()

serverConfigPath = args.config_prefix + '0.config'
serverExecutablePath = BUILD_DIR + "/server"

remoteServerConfigPath = WORKING_DIR + "/diamond.0.config"
remoteServerExecutablePath = WORKING_DIR + "/server"

serverConfig = open(serverConfigPath, 'r')
i = 0
for line in serverConfig:
    match = re.match("replica\s+([\w\.]+):(\d)", line)
    if match:
        hostname = match.group(1)
        host = RemoteHost(hostname)
        if args.action == 'start':
            sendConfigFileTask = host.send_file(serverConfigPath, remoteServerConfigPath)
            sendConfigFileTask.start()
            sendConfigFileTask.wait()
            sendExecutableTask = host.send_file(serverExecutablePath, remoteServerExecutablePath)
            sendExecutableTask.start()
            sendExecutableTask.wait()
            command = remoteServerExecutablePath + " -c " + remoteServerConfigPath + " -i " + repr(i)
            startServerTask = host.run([command], kill_remote=False)
            startServerTask.start()
        elif args.action == 'kill':
            command = "pkill " + remoteServerConfigPath
            killServerTask = host.run([command])
            killServerTask.start()
            killServerTask.wait()
        i = i + 1
