#!/bin/bash

DIAMOND_SRC="/home/nl35/research/diamond-src"
PROJECT_DIR="$DIAMOND_SRC/apps/chat/BaselineChatClient"
JAVA_BINDINGS_DIR="$DIAMOND_SRC/backend/src/bindings/java"
JAVA_BINARY="/home/nl35/research/jdk1.8.0_60/jre/bin/java"

if [ ! -d $DIAMOND_SRC ]
then
    echo "Error: DIAMOND_SRC directory does not exist"
    exit
fi

if [ ! -e "$PROJECT_DIR/bin/BaselineChatClient.class" ]
then
    echo "Error: BaselineChatClient has not been compiled"
    exit
fi

classpath="$PROJECT_DIR/bin:$PROJECT_DIR/libs/gson-2.3.1.jar"

param1=$1
param2=$2
param3=$3
param4=$4
param5=$5
param6=$6
param7=$7

$JAVA_BINARY -cp $classpath BaselineChatClient $param1 $param2 $param3 $param4 $param5 $param6 $param7
