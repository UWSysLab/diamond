#!/usr/bin/perl

die "usage: ./start-servers.pl config_prefix" unless @ARGV==1;
my $configPrefix = shift;
my $configFile = "${configPrefix}0.config";
my $tssConfigFile = "${configPrefix}.tss.config";

system("../platform/build/server -c $configFile -i 0 &");
system("../platform/build/server -c $configFile -i 1 &");
system("../platform/build/server -c $configFile -i 2 &");

system("../platform/build/tss -c $tssConfigFile -i 0 &");
system("../platform/build/tss -c $tssConfigFile -i 1 &");
system("../platform/build/tss -c $tssConfigFile -i 2 &");
