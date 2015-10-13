#!/bin/bash

CONCURRENCY=transaction
#CONCURRENCY=atomic

SERVER=localhost

rm desktop-latency-error*

./desktop-chat-wrapper.sh fixed 1000 0 transaction verbose $SERVER writer latencyroom > desktopchat-transaction-writer.txt 2> desktop-latency-error1.txt &
PID1=$!
./desktop-chat-wrapper.sh fixed 1000 1.0 transaction verbose $SERVER reader latencyroom > desktopchat-transaction-reader.txt 2> desktop-latency-error2.txt &
PID2=$!
wait $PID1 $PID2

./desktop-chat-wrapper.sh fixed 1000 0 atomic verbose $SERVER writer latencyroom > desktopchat-atomic-writer.txt 2> desktop-latency-error3.txt &
PID1=$!
./desktop-chat-wrapper.sh fixed 1000 1.0 atomic verbose $SERVER reader latencyroom > desktopchat-atomic-reader.txt 2> desktop-latency-error4.txt &
PID2=$!
wait $PID1 $PID2
