#!/bin/bash

if [[ $# -ne 12 ]]
then
    echo "Error: need 12 arguments"
    exit
fi

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

shift
DIR=$9

PROG_DIR="$DIR/diamond-client"

classpath="$PROG_DIR/bin:$PROG_DIR/libs/javacpp.jar:$PROG_DIR/libs/diamond-1.0-SNAPSHOT.jar"
nativePath="$PROG_DIR/libs"

export LD_LIBRARY_PATH=$nativePath

$DIR/jre/bin/java -cp $classpath -Djava.library.path=$nativePath DesktopChatClient $param1 $param2 $param3 $param4 $param5 $param6 $param7 $param8 $param9 $param10 $param11
