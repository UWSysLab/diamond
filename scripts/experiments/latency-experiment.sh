#!/bin/bash

DIAMOND_SRC="/home/nl35/research/diamond-src"
PROJECT_DIR="$DIAMOND_SRC/apps/chat/DesktopChat"
JAVA_BINDINGS_DIR="$DIAMOND_SRC/backend/src/bindings/java"

if [ ! -d $DIAMOND_SRC ]
then
    echo "Error: DIAMOND_SRC directory does not exist"
    exit
fi

if [ ! -e "$PROJECT_DIR/bin/Main.class" ]
then
    echo "Error: DesktopChat has not been compiled"
    exit
fi

classpath="$PROJECT_DIR/bin:$JAVA_BINDINGS_DIR/libs/javacpp.jar:$JAVA_BINDINGS_DIR/target/diamond-1.0-SNAPSHOT.jar"
nativePath="$JAVA_BINDINGS_DIR/target/classes/x86-lib:$DIAMOND_SRC/backend/build"

rm client1.log
rm client2.log
rm error1.log
rm error2.log

export LD_LIBRARY_PATH=$nativePath
java -cp $classpath -Djava.library.path=$nativePath Main fixed 1000 0 client1 latencyroom > client1.log 2> error1.log &
PID1=$!
java -cp $classpath -Djava.library.path=$nativePath Main fixed 1000 1.0 client2 latencyroom > client2.log 2> error2.log &
PID2=$!
wait $PID1 $PID2
$DIAMOND_SRC/scripts/experiments/latency-measurements.pl
