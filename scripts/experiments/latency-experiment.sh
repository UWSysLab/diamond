#!/bin/bash

CONCURRENCY=transaction
#CONCURRENCY=atomic

SERVER=localhost

./desktop-chat-wrapper.sh fixed 1000 0 transaction verbose $SERVER writer latencyroom > desktopchat-transaction-writer.log 2> desktopchat-transaction-writer.error &
PID1=$!
./desktop-chat-wrapper.sh fixed 1000 1.0 transaction verbose $SERVER reader latencyroom > desktopchat-transaction-reader.log 2> desktopchat-transaction-reader.error &
PID2=$!
wait $PID1 $PID2

./desktop-chat-wrapper.sh fixed 1000 0 atomic verbose $SERVER writer latencyroom > desktopchat-atomic-writer.log 2> desktopchat-atomic-writer.error &
PID1=$!
./desktop-chat-wrapper.sh fixed 1000 1.0 atomic verbose $SERVER reader latencyroom > desktopchat-atomic-reader.log 2> desktopchat-atomic-reader.error &
PID2=$!
wait $PID1 $PID2

cat desktopchat-transaction-writer.log | awk '$1 !~ /Summary/ { print $4 }' > desktopchat-transaction-writer-latencies.txt
cat desktopchat-transaction-reader.log | awk '$1 !~ /Summary/ { print $4 }' > desktopchat-transaction-reader-latencies.txt
cat desktopchat-atomic-writer.log | awk '$1 !~ /Summary/ { print $4 }' > desktopchat-atomic-writer-latencies.txt
cat desktopchat-atomic-reader.log | awk '$1 !~ /Summary/ { print $4 }' > desktopchat-atomic-reader-latencies.txt
