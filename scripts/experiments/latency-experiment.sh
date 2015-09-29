#!/bin/bash

./desktop-chat-wrapper.sh fixed 1000 0 writer latencyroom 2> error1.log &
PID1=$!
./desktop-chat-wrapper.sh fixed 1000 1.0 reader latencyroom 2> error2.log &
PID2=$!
wait $PID1 $PID2
