#!/usr/bin/python

import argparse
import os
import re
import redis
import sys

parser = argparse.ArgumentParser(description='Parse game client results.')
parser.add_argument('--directory', '-d', help='parse data from output files in the given directory')
parser.add_argument('--redis', '-r', action='store_true', help='parse transaction data from redis')
args = parser.parse_args()

allPrevEndTimes = []
allEndTimes = []

if args.redis:
    r = redis.StrictRedis(host='localhost', port=6379)
    txns = r.lrange('txns', 0, -1)
    for txn in txns:
        txnDict = r.hgetall(txn)
        allPrevEndTimes.append(int(txnDict['prev-end-time']))
        allEndTimes.append(int(txnDict['end-time']))

elif args.directory != None:
    resultDir = args.directory
    files = os.listdir(resultDir)

    for filename in files:
        f = open(resultDir + "/" + filename, 'r')
        for line in f:
            match = re.match("^(\d+)\s+(\d+)", line)
            if match:
                prevEndTime = int(match.group(1))
                endTime = int(match.group(2))

                allPrevEndTimes.append(prevEndTime)
                allEndTimes.append(endTime)

    if len(files) == 0:
        print("No files to parse!")
        sys.exit()

print("Results span " + repr((max(allEndTimes) - min(allPrevEndTimes)) / 1000.0) + " seconds");

timeLowerBound = min(allPrevEndTimes) + (max(allEndTimes) - min(allPrevEndTimes)) * (25.0 / 100.0)
timeUpperBound = min(allPrevEndTimes) + (max(allEndTimes) - min(allPrevEndTimes)) * (75.0 / 100.0)

timeRangeSeconds = (timeUpperBound - timeLowerBound) / 1000.0

prevEndTimes = []
endTimes = []

for i in range(0, len(allPrevEndTimes)):
    if allPrevEndTimes[i] >= timeLowerBound and allEndTimes[i] <= timeUpperBound:
        prevEndTimes.append(allPrevEndTimes[i])
        endTimes.append(allEndTimes[i])

print("Selected " + repr(len(prevEndTimes)) + " of " + repr(len(allPrevEndTimes)) + " transactions over " + repr(timeRangeSeconds) + " seconds")

numTxns = 0
sumTimes = 0
times = []
for i in range(0, len(prevEndTimes)):
    numTxns += 1
    time = (endTimes[i] - prevEndTimes[i])
    sumTimes += time
    times.append(time)

sumTimesSeconds = sumTimes / 1000.0
meanTime = float(sumTimesSeconds) / numTxns
meanThroughput = float(numTxns) / timeRangeSeconds

print("Num transactions: " + repr(numTxns));
print("Avg. time between turns (s): " + repr(meanTime))
print("Avg. turns/second: " + repr(meanThroughput))

minLatency = min(times) / 1000.0
maxLatency = max(times) / 1000.0
print("Min latency: " + repr(minLatency))
print("Max latency: " + repr(maxLatency))
