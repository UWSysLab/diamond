#!/bin/bash

CONCURRENCY=transaction
#CONCURRENCY=atomic

SERVER=localhost

DIR="desktopchat-latency"

./desktop-chat-wrapper.sh fixed 1000 0 transaction verbose $SERVER writer latencyroom stale 100 > $DIR/transaction-writer.log 2> $DIR/transaction-writer.error &
PID1=$!
./desktop-chat-wrapper.sh fixed 1000 1.0 transaction verbose $SERVER reader latencyroom stale 100 > $DIR/transaction-reader.log 2> $DIR/transaction-reader.error &
PID2=$!
wait $PID1 $PID2

./desktop-chat-wrapper.sh fixed 1000 0 atomic verbose $SERVER writer latencyroom nostale 0 > $DIR/atomic-writer.log 2> $DIR/atomic-writer.error &
PID1=$!
./desktop-chat-wrapper.sh fixed 1000 1.0 atomic verbose $SERVER reader latencyroom nostale 0 > $DIR/atomic-reader.log 2> $DIR/atomic-reader.error &
PID2=$!
wait $PID1 $PID2

cat $DIR/transaction-writer.log | awk '$1 !~ /Summary/ { print $4 }' > $DIR/transaction-writer-latencies.txt
cat $DIR/transaction-reader.log | awk '$1 !~ /Summary/ { print $4 }' > $DIR/transaction-reader-latencies.txt
cat $DIR/atomic-writer.log | awk '$1 !~ /Summary/ { print $4 }' > $DIR/atomic-writer-latencies.txt
cat $DIR/atomic-reader.log | awk '$1 !~ /Summary/ { print $4 }' > $DIR/atomic-reader-latencies.txt
