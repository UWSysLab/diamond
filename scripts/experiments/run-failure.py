#!/usr/bin/python

import os
import subprocess
import time


os.system("cd ..; ./manage-servers.py start ../platform/test/niel --shards 1")

game1 = subprocess.Popen("../../apps/benchmarks/build/game --name player1 --config ../../platform/test/niel.frontend0 --keyprefix test > failure-output-1.txt 2>&1", shell=True);
game2 = subprocess.Popen("../../apps/benchmarks/build/game --name player2 --config ../../platform/test/niel.frontend0 --keyprefix test > failure-output-2.txt 2>&1", shell=True);

os.system("pkill -f 'diamond0.config -i 0'");

game1.wait()
game2.wait()

os.system("cd ..; ./manage-servers.py kill ../platform/test/niel")
