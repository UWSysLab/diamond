#!/bin/bash

CONCURRENCY=transaction
#CONCURRENCY=atomic

SERVER=moranis.cs.washington.edu

./desktop-chat-wrapper.sh fixed 1000 0 transaction verbose $SERVER writer latencyroom > desktopchat-transaction-writer-log.txt 2> desktop-latency-error1.txt &
PID1=$!
./desktop-chat-wrapper.sh fixed 1000 1.0 transaction verbose $SERVER reader latencyroom > desktopchat-transaction-reader-log.txt 2> desktop-latency-error2.txt &
PID2=$!
wait $PID1 $PID2

./desktop-chat-wrapper.sh fixed 1000 0 atomic verbose $SERVER writer latencyroom > desktopchat-atomic-writer-log.txt 2> desktop-latency-error3.txt &
PID1=$!
./desktop-chat-wrapper.sh fixed 1000 1.0 atomic verbose $SERVER reader latencyroom > desktopchat-atomic-reader-log.txt 2> desktop-latency-error4.txt &
PID2=$!
wait $PID1 $PID2

cat desktopchat-transaction-writer-log.txt | awk '$1 !~ /Summary/ { print $4 }' > desktopchat-transaction-writer-latencies.txt
cat desktopchat-transaction-reader-log.txt | awk '$1 !~ /Summary/ { print $4 }' > desktopchat-transaction-reader-latencies.txt
cat desktopchat-atomic-writer-log.txt | awk '$1 !~ /Summary/ { print $4 }' > desktopchat-atomic-writer-latencies.txt
cat desktopchat-atomic-reader-log.txt | awk '$1 !~ /Summary/ { print $4 }' > desktopchat-atomic-reader-latencies.txt
