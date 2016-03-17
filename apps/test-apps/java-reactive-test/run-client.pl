#!/usr/bin/perl

use warnings;
use strict;

my $DIAMOND_SRC="../../..";
my $JAVA_BINDINGS_DIR="$DIAMOND_SRC/platform/bindings/java";

my $classpath=".:$JAVA_BINDINGS_DIR/libs/javacpp.jar:$JAVA_BINDINGS_DIR/target/diamond-1.0-SNAPSHOT.jar";
my $nativePath="$JAVA_BINDINGS_DIR/target/classes/x86-lib:$DIAMOND_SRC/platform/build";

$ENV{LD_LIBRARY_PATH}=$nativePath;

die "usage: ./run-client.pl config_file" unless @ARGV==1;
my $configFile = shift;
system("java -classpath $classpath -Djava.library.path=$nativePath Client $configFile");
