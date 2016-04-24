#!/usr/bin/python

import os
import subprocess
import time


os.system("cd ..; ./manage-servers.py start ../platform/test/gce --shards 1 --batch 1 --frontends 1")

game1 = subprocess.Popen("../../apps/benchmarks/build/game --name player1 --config ../../platform/test/gce.frontend0 --keyprefix test > failure/failure-output-1.txt 2>&1", shell=True)
game2 = subprocess.Popen("../../apps/benchmarks/build/game --name player2 --config ../../platform/test/gce.frontend0 --keyprefix test > failure/failure-output-2.txt 2>&1", shell=True)

time.sleep(15)

print("Killing backend leader...")
os.system("ssh diamond-backend-central-4clv 'pkill -9 -f diamond0.config'")

game1.wait()
game2.wait()

os.system("cd ..; ./manage-servers.py kill ../platform/test/gce")
