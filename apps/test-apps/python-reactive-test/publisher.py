#!/usr/bin/python

import argparse
import sys
sys.path.append("../../../platform/build/bindings/python/")
sys.path.append("../../../platform/bindings/python/")
from libpydiamond import *
import ReactiveManager

def main():
    parser = argparse.ArgumentParser(description='Reactive test client.')
    parser.add_argument('config_prefix', help='frontend config file prefix')
    args = parser.parse_args()

    DiamondInit(args.config_prefix, 0, 1);
    ReactiveManager.start();

    dstr = DString()
    DString.Map(dstr, "pythonreactivetest:str")

    DObject.TransactionBegin()
    dstr.Set("Testing in Python!")
    DObject.TransactionCommit()

if __name__ == "__main__": main()
