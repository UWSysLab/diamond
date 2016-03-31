#!/usr/bin/python

#import argparse
import os

SRC_HOST = "diamond-client-a1o6"
WORKING_DIR = "~/"

COPY_CMD = "rsync " + SRC_HOST + ":diamond-src/platform/"
CONFIG_DIR = "test/"
CONFIG_FILE = "gce.frontend"

os.system(COPY_CMD + "build/terminalclient/terminalclient" + " " + WORKING_DIR)
os.system(COPY_CMD + "build/libdiamond.so" + " " + WORKING_DIR)

# Copy config files
os.system(COPY_CMD + CONFIG_DIR + CONFIG_FILE + ".config" + " " + WORKING_DIR)

# run binary
os.system(WORKING_DIR + "terminalclient -c " + WORKING_DIR + CONFIG_FILE)
