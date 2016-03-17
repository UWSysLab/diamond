#!/usr/bin/perl

use warnings;
use strict;

my $DIAMOND_SRC="/home/nl35/research/diamond-src";
my $JAVA_BINDINGS_DIR="$DIAMOND_SRC/platform/bindings/java";

my $classpath=".:$JAVA_BINDINGS_DIR/libs/javacpp.jar:$JAVA_BINDINGS_DIR/target/diamond-1.0-SNAPSHOT.jar";
my $nativePath="$JAVA_BINDINGS_DIR/target/classes/x86-lib:$DIAMOND_SRC/platform/build";

$ENV{LD_LIBRARY_PATH}=$nativePath;

die "usage: ./run-client.pl config_file [message]" unless @ARGV==1;
my $configFile = $ARGV[0];
my $message = "";
if (@ARGV==2) {
    $message = $ARGV[1];
}
system("java -classpath $classpath -Djava.library.path=$nativePath Publisher $configFile $message");
