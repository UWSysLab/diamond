#!/usr/bin/python

import argparse
import os
import re
from pyrem.host import RemoteHost

USER_NAME = "nl35"
BUILD_DIR = "/homes/sys/nl35/research/diamond-src/platform/build"

WORKING_DIR = "/biggerraid/users/" + USER_NAME + "/scratch"

parser = argparse.ArgumentParser(description='Launch servers.')
parser.add_argument('config_prefix', help='the config file prefix')
args = parser.parse_args()

serverConfigName = args.config_prefix + '0.config'
serverConfig = open(serverConfigName, 'r')

remoteServerConfigName = WORKING_DIR + "/diamond.0.config"

i = 0

for line in serverConfig:
    match = re.match("replica\s+([\w\.]+):(\d)", line)
    if match:
        hostname = match.group(1)

        os.system("scp " + serverConfigName + " " + USER_NAME + "@" + hostname + ":" + remoteServerConfigName)

        host = RemoteHost(hostname)
        command = BUILD_DIR + "/server -c " + remoteServerConfigName + " -i " + repr(i)
        server = host.run([command])
        server.start()

        i = i + 1
