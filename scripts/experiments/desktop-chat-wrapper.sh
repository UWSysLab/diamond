#!/bin/bash

DIAMOND_SRC="/home/nl35/research/diamond-src"
PROJECT_DIR="$DIAMOND_SRC/apps/chat/DesktopChat"
JAVA_BINDINGS_DIR="$DIAMOND_SRC/backend/src/bindings/java"
JAVA_BINARY="/home/nl35/research/jdk1.8.0_60/jre/bin/java"

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

export LD_LIBRARY_PATH=$nativePath

param1=$1
param2=$2
param3=$3
param4=$4
param5=$5
param6=$6
param7=$7
param8=$8
param9=$9

shift
param10=$9

shift
param11=$9

$JAVA_BINARY -cp $classpath -Djava.library.path=$nativePath Main $param1 $param2 $param3 $param4 $param5 $param6 $param7 $param8 $param9 $param10 $param11
