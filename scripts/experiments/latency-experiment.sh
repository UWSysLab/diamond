#!/bin/bash

CONCURRENCY=transaction
#CONCURRENCY=atomic

SERVER=localhost

./desktop-chat-wrapper.sh fixed 1000 0 $CONCURRENCY $SERVER writer latencyroom 2> error1.log &
PID1=$!
./desktop-chat-wrapper.sh fixed 1000 1.0 $CONCURRENCY $SERVER reader latencyroom 2> error2.log &
PID2=$!
wait $PID1 $PID2
