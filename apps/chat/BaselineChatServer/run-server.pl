#!/usr/bin/perl

use warnings;
use strict;

my $usage = "./run-server.pl port redis-hostname redis-port [java-dir]";
die $usage unless @ARGV>=3;

my $port = $ARGV[0];
my $redisHostname = $ARGV[1];
my $redisPort = $ARGV[2];

my $java = "java";

if (@ARGV >= 4) {
    my $javaDir = $ARGV[3];
    $java = "$javaDir/jre/bin/java";
}

system("$java -classpath libs/commons-pool2-2.0.jar:libs/gson-2.3.1.jar:libs/jedis-2.4.2.jar:libs/jetty-all-9.3.7.v20160115-uber.jar:baselinechatserver.jar BaselineChatServer $port $redisHostname $redisPort");
