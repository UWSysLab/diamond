#!/usr/bin/python

import argparse
import os
import random
import sys

SRC_HOST = "diamond-client-a1o6"
WORKING_DIR = "~/"

COPY_CMD = "rsync " + SRC_HOST + ":diamond-src/"
CONFIG_DIR = "platform/test/"
CONFIG_FILE = "gce.frontend"
KEY_DIR = "scripts/experiments/"
KEY_FILE = "keys.txt"
NUM_KEYS = "1000"
OUTPUT_DEST = "scripts/experiments/scalability/"

os.system(COPY_CMD + "apps/benchmarks/build/scalability" + " " + WORKING_DIR)
os.system(COPY_CMD + "platform/build/libdiamond.so" + " " + WORKING_DIR)

# Copy config files
os.system(COPY_CMD + CONFIG_DIR + CONFIG_FILE + ".config" + " " + WORKING_DIR)
os.system(COPY_CMD + KEY_DIR + KEY_FILE + " " + WORKING_DIR)

# Run binary
OUTPUT_FILE = "scalability-out-" + repr(random.randint(0, sys.maxint))
os.system(WORKING_DIR + "scalability --config " + WORKING_DIR + CONFIG_FILE + " --keys " + WORKING_DIR + KEY_FILE + " --numkeys " + NUM_KEYS + " > " + WORKING_DIR + OUTPUT_FILE)

# Copy output back to client
os.system("rsync " + WORKING_DIR + OUTPUT_FILE + " " + SRC_HOST + ":diamond-src/" + OUTPUT_DEST)
