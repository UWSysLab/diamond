#!/bin/bash

if [[ $# -ne 9 ]]
then
    echo "Error: need 9 arguments"
    exit
fi

DIR=$9

PROG_DIR=$DIR/baseline-client
classpath="$PROG_DIR/bin:$PROG_DIR/libs/gson-2.3.1.jar"

param1=$1
param2=$2
param3=$3
param4=$4
param5=$5
param6=$6
param7=$7
param8=$8

$DIR/jre/bin/java -cp $classpath BaselineChatClient $param1 $param2 $param3 $param4 $param5 $param6 $param7 $param8
