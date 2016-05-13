#!/usr/bin/python

import argparse
import os
import re
import redis
import sys

def getStatsForType(txnType, timeRangeSeconds, startTimes, endTimes, results, types):
    numTxns = 0
    numAborts = 0
    sumTimes = 0
    for i in range(0, len(startTimes)):
        if types[i] == txnType:
            numTxns += 1
            if results[i] == 1:
                time = (endTimes[i] - startTimes[i])
                sumTimes += time
            else:
                numAborts += 1

    sumTimesSeconds = sumTimes / 1000.0
    numSuccessful = numTxns - numAborts
    meanTime = 0
    abortRate = 1.0
    if numSuccessful > 0:
        meanTime = float(sumTimesSeconds) / numSuccessful
    if numTxns > 0:
        abortRate = float(numAborts) / numTxns
    meanThroughput = float(numSuccessful) / timeRangeSeconds
    return (meanThroughput, meanTime, abortRate, numTxns, numSuccessful)
    

parser = argparse.ArgumentParser(description='Parse retwis client results.')
parser.add_argument('--directory', '-d', help='parse data from output files in the given directory')
parser.add_argument('--redis', '-r', action='store_true', help='parse transaction data from redis')
args = parser.parse_args()

allStartTimes = []
allEndTimes = []
allResults = []
allTypes = []

if args.redis:
    r = redis.StrictRedis(host='localhost', port=6379)
    txns = r.lrange('txns', 0, -1)
    for txn in txns:
        txnDict = r.hgetall(txn)
        allStartTimes.append(int(txnDict['start-time']))
        allEndTimes.append(int(txnDict['end-time']))
        allResults.append(int(txnDict['committed']))
        allTypes.append(int(txnDict['type']))

elif args.directory != None:
    resultDir = args.directory
    files = os.listdir(resultDir)

    for filename in files:
        f = open(resultDir + "/" + filename, 'r')
        for line in f:
            match = re.match("^(\d+)\s+(\d+)\s+(\d)", line)
            if match:
                startTime = int(match.group(1))
                endTime = int(match.group(2))
                committed = int(match.group(3))
                txnType = int(match.group(4))

                allStartTimes.append(startTime)
                allEndTimes.append(endTime)
                allResults.append(committed)
                allTypes.append(txnType)

    if len(files) == 0:
        print("No files to parse!")
        sys.exit()

print("Results span " + repr((max(allEndTimes) - min(allStartTimes)) / 1000.0) + " seconds");

timeLowerBound = min(allStartTimes) + (max(allEndTimes) - min(allStartTimes)) * (25.0 / 100.0)
timeUpperBound = min(allStartTimes) + (max(allEndTimes) - min(allStartTimes)) * (75.0 / 100.0)

timeRangeSeconds = (timeUpperBound - timeLowerBound) / 1000.0

startTimes = []
endTimes = []
results = []
types = []

for i in range(0, len(allStartTimes)):
    if allStartTimes[i] >= timeLowerBound and allEndTimes[i] <= timeUpperBound:
        startTimes.append(allStartTimes[i])
        endTimes.append(allEndTimes[i])
        results.append(allResults[i])
        types.append(allTypes[i])

print("Selected " + repr(len(results)) + " of " + repr(len(allResults)) + " transactions over " + repr(timeRangeSeconds) + " seconds")

numTxns = 0
numAborts = 0
sumTimes = 0
for i in range(0, len(startTimes)):
    numTxns += 1
    if results[i] == 1:
        time = (endTimes[i] - startTimes[i])
        sumTimes += time
    else:
        numAborts += 1

sumTimesSeconds = sumTimes / 1000.0
numSuccessful = numTxns - numAborts
meanThroughput = float(numSuccessful) / timeRangeSeconds
meanTime = 0
abortRate = 1.0
if numSuccessful > 0:
    meanTime = float(sumTimesSeconds) / numSuccessful
if numTxns > 0:
    abortRate = float(numAborts) / numTxns

print("Type\tthroughput\tlatency\tabort-rate\tnum-txns\tnum-successful")
print("Overall\t%f\t%f\t%f\t%d\t%d" % (meanThroughput, meanTime, abortRate, numTxns, numSuccessful))

for i in range(1, 6):
    (meanThroughput, meanTime, abortRate, numTxns, numSuccessful) = getStatsForType(i, timeRangeSeconds, startTimes, endTimes, results, types)
    print("%d\t%f\t%f\t%f\t%d\t%d" % (i, meanThroughput, meanTime, abortRate, numTxns, numSuccessful))
