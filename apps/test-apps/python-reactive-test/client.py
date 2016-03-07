#!/usr/bin/python

import argparse
import time
import sys
sys.path.append("../../../platform/build/bindings/python/")
sys.path.append("../../../platform/bindings/python/")
from libpydiamond import *
import ReactiveManager

def react(dstr):
    print "String value: " + dstr.Value()

def main():
    parser = argparse.ArgumentParser(description='Reactive test client.')
    parser.add_argument('config_prefix', help='frontend config file prefix')
    args = parser.parse_args()

    DiamondInit(args.config_prefix, 0, 1);
    ReactiveManager.start();

    dstr = DString()
    DString.Map(dstr, "pythonreactivetest:str")

    ReactiveManager.add(react, dstr);

    while True:
        time.sleep(1)

if __name__ == "__main__": main()
