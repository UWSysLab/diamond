#!/usr/bin/perl

use warnings;
use strict;

#my $usage = "usage: ./run-desktop-client.pl job_type job_number read_fraction [client_name] [chatroom_name]";
#die $usage unless @ARGV == 5;

my $jobType = shift;
my $jobNumber = shift;
my $readFraction = shift;
my $clientName = shift;
my $chatroomName = shift;

my $DIAMOND_SRC = "/home/nl35/research/diamond-src";
my $PROJECT_DIR = "$DIAMOND_SRC/apps/chat/DesktopChat";
my $JAVA_BINDINGS_DIR = "$DIAMOND_SRC/backend/src/bindings/java";

my $classpath = "$PROJECT_DIR/bin:$JAVA_BINDINGS_DIR/libs/javacpp.jar:$JAVA_BINDINGS_DIR/target/diamond-1.0-SNAPSHOT.jar";
my $nativePath = "$JAVA_BINDINGS_DIR/target/classes/x86-lib:$DIAMOND_SRC/backend/build";

$ENV{"LD_LIBRARY_PATH"} = $nativePath;
system("java -cp $classpath -Djava.library.path=$nativePath Main $jobType $jobNumber $readFraction $clientName $chatroomName");
