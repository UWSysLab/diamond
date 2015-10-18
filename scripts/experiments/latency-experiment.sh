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

DIAMOND_SRC="/home/nl35/research/diamond-src"
PROJECT_DIR="$DIAMOND_SRC/apps/chat/BaselineChatServer"
JAVA_BINARY="/home/nl35/research/jdk1.8.0_60/jre/bin/java"
classpath="$PROJECT_DIR/bin:$PROJECT_DIR/libs/gson-2.3.1.jar:$PROJECT_DIR/libs/commons-pool2-2.0.jar:$PROJECT_DIR/libs/jedis-2.4.2.jar"
$JAVA_BINARY -cp $classpath Main 2> $DIR/baseline-server.error &
PID_SERVER=$!
echo $PID_SERVER
sleep 1

./baseline-chat-client-wrapper.sh fixed 1000 0 verbose localhost writer > $DIR/baseline-writer.log 2> $DIR/baseline-writer.error &
PID1=$!
./baseline-chat-client-wrapper.sh fixed 1000 1.0 verbose localhost reader > $DIR/baseline-reader.log 2> $DIR/baseline-reader.error &
PID2=$!
wait $PID1 $PID2
kill $PID_SERVER
echo $PID_SERVER

cat $DIR/transaction-writer.log | awk '$1 !~ /Summary/ { print $4 }' > $DIR/transaction-writer-latencies.txt
cat $DIR/transaction-reader.log | awk '$1 !~ /Summary/ { print $4 }' > $DIR/transaction-reader-latencies.txt
cat $DIR/atomic-writer.log | awk '$1 !~ /Summary/ { print $4 }' > $DIR/atomic-writer-latencies.txt
cat $DIR/atomic-reader.log | awk '$1 !~ /Summary/ { print $4 }' > $DIR/atomic-reader-latencies.txt
cat $DIR/baseline-writer.log | awk '$1 !~ /Summary/ { print $3 }' > $DIR/baseline-writer-latencies.txt
cat $DIR/baseline-reader.log | awk '$1 !~ /Summary/ { print $3 }' > $DIR/baseline-reader-latencies.txt
