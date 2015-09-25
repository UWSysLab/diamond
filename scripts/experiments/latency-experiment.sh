#!/bin/bash

DIAMOND_SRC="/home/nl35/research/diamond-src"
PROJECT_DIR="$DIAMOND_SRC/apps/chat/DesktopChat"
JAVA_BINDINGS_DIR="$DIAMOND_SRC/backend/src/bindings/java"

classpath="$PROJECT_DIR/bin:$JAVA_BINDINGS_DIR/libs/javacpp.jar:$JAVA_BINDINGS_DIR/target/diamond-1.0-SNAPSHOT.jar"
nativePath="$JAVA_BINDINGS_DIR/target/classes/x86-lib:$DIAMOND_SRC/backend/build"

export LD_LIBRARY_PATH=$nativePath
#java -cp $classpath -Djava.library.path=$nativePath Main 0 client1 > client1.log 2> error1.log &
#java -cp $classpath -Djava.library.path=$nativePath Main 1.0 client2 > client2.log 2> error2.log &
java -cp $classpath -Djava.library.path=$nativePath Main 0 client1 > client1.log 2> error1.log
sleep 1
#system("./latency-measurements.pl");
