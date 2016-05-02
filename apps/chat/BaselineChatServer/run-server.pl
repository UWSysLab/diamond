#!/usr/bin/perl

use warnings;
use strict;

my $usage = "./run-server.pl port redis-hostname redis-port";
die $usage unless @ARGV==3;

my ($port, $redisHostname, $redisPort) = @ARGV;

system("java -classpath libs/commons-pool2-2.0.jar:libs/gson-2.3.1.jar:libs/jedis-2.4.2.jar:src Main $port $redisHostname $redisPort");
