#!/usr/bin/python

#import argparse
import os

SRC_HOST = "diamond-client-a1o6"
WORKING_DIR = "~/"

COPY_CMD = "rsync " + SRC_HOST + ":diamond-src/platform/"
BINARY_DIR = "build/terminalclient/"
BINARY_FILE = "terminalclient"
CONFIG_DIR = "test/"
CONFIG_FILE = "gce.frontend"

dependencies = {} # map local full path to remote file name in working dir
dependencies["libprotobuf.so.9"] = "/usr/lib/x86_64-linux-gnu/libprotobuf.so.9"
dependencies["libevent_pthreads-2.0.so.5"] = "/usr/lib/x86_64-linux-gnu/libevent_pthreads-2.0.so.5"
dependencies["libevent_core-2.0.so.5"] = "/usr/lib/x86_64-linux-gnu/libevent_core-2.0.so.5"

#parser = argparse.ArgumentParser(description='Copy dependencies to GCE machines.')
#parser.add_argument('shards', metavar='n', type=int, help='the number of shards')
#args = parser.parse_args()

# Copy binary over
os.system(COPY_CMD + BINARY_DIR + BINARY_FILE + " " + WORKING_DIR)

# Copy dependencies
for dep in dependencies:
    os.system("rsync --copy-links "+ SRC_HOST + ":" + dependencies[dep] + " " + WORKING_DIR + "/" + dep)

# Copy config files
os.system(COPY_CMD + CONFIG_DIR + CONFIG_FILE + ".config" + " " + WORKING_DIR)
        
# run binary
os.system(WORKING_DIR + BINARY_FILE + " -c " + WORKING_DIR + CONFIG_FILE)
