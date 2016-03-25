#!/usr/bin/python

#import argparse
import os

SRC_HOST = "diamond-client-a1o6"
WORKING_DIR = "~/"

COPY_CMD = "rsync " + SRC_HOST + ":diamond-src/"
CONFIG_DIR = "test/"
CONFIG_FILE = "gce.frontend"

os.system(COPY_CMD + "apps/benchmarks/build/benchmarkclient" + " " + WORKING_DIR)
os.system(COPY_CMD + "platform/build/libdiamond.so" + " " + WORKING_DIR)

# Copy config files
os.system(COPY_CMD + CONFIG_DIR + CONFIG_FILE + ".config" + " " + WORKING_DIR)

# run binary
os.system(WORKING_DIR + "benchmarkclient -c " + WORKING_DIR + CONFIG_FILE)
