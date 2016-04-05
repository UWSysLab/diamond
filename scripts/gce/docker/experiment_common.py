#!/usr/bin/python

import random
import re
import redis
import sys

def putDataInRedis(outputFileName, redisHost, redisPort=6379):
    r = redis.StrictRedis(host=redisHost, port=redisPort)
    r.incr("clients")
    outputFile = open(outputFileName, 'r')
    for line in outputFile:
        match = re.match("^(\d+)\s+(\d+)\s+(\d+)", line)
        if match:
            startTime = match.group(1)
            endTime = match.group(2)
            committed = match.group(3)
            txnNum = random.randint(0, sys.maxint)
            txnKey = "txn-" + repr(txnNum)
            mapping = dict()
            mapping['start-time'] = startTime
            mapping['end-time'] = endTime
            mapping['committed'] = committed
            r.hmset(txnKey, mapping)
            r.lpush("txns", txnKey)
