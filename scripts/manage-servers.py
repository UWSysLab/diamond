#!/usr/bin/python

import argparse
import os
import re
import time

USER_NAME = "nl35"
WORKING_DIR = "/biggerraid/users/" + USER_NAME + "/scratch"
BUILD_DIR = "../platform/build"

parser = argparse.ArgumentParser(description='Launch servers.')
parser.add_argument('action', choices=['start', 'kill'], help='the action to take')
parser.add_argument('config_prefix', help='the config file prefix')
args = parser.parse_args()

serverConfigPath = args.config_prefix + '0.config'
serverExecutablePath = BUILD_DIR + "/server"
tssConfigPath = args.config_prefix + '.tss.config'
tssExecutablePath = BUILD_DIR + "/tss"

remoteServerConfigPath = WORKING_DIR + "/diamond0.config"
remoteServerExecutablePath = WORKING_DIR + "/server"
remoteTssConfigPath = WORKING_DIR + "/diamond.tss.config"
remoteTssExecutablePath = WORKING_DIR + "/tss"

serverConfig = open(serverConfigPath, 'r')
i = 0
for line in serverConfig:
    match = re.match("replica\s+([\w\.]+):(\d)", line)
    if match:
        hostname = match.group(1)
        if args.action == 'start':
            os.system("scp " + serverConfigPath + " " + hostname + ":" + remoteServerConfigPath)
            os.system("scp " + serverExecutablePath + " " + hostname + ":" + remoteServerExecutablePath)
            os.system("ssh -f " + hostname + " '" + remoteServerExecutablePath + " -c " + remoteServerConfigPath + " -i " + repr(i) + "'");
        elif args.action == 'kill':
            os.system("ssh " + hostname + " 'pkill " + remoteServerConfigPath + "'");
        i = i + 1

tssConfig = open(tssConfigPath, 'r')
i = 0
for line in tssConfig:
    match = re.match("replica\s+([\w\.]+):(\d)", line)
    if match:
        hostname = match.group(1)
        if args.action == 'start':
            os.system("scp " + tssConfigPath + " " + hostname + ":" + remoteTssConfigPath)
            os.system("scp " + tssExecutablePath + " " + hostname + ":" + remoteTssExecutablePath)
            os.system("ssh -f " + hostname + " '" + remoteTssExecutablePath + " -c " + remoteTssConfigPath + " -i " + repr(i) + "'");
        elif args.action == 'kill':
            os.system("ssh " + hostname + " 'pkill " + remoteTssConfigPath + "'");
        i = i + 1
