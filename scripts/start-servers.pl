#!/usr/bin/perl

die "./start-servers.pl config_file" unless @ARGV==1;
my $config = shift;

system("../platform/build/server -c $config -i 0 &");
system("../platform/build/server -c $config -i 1 &");
system("../platform/build/server -c $config -i 2 &");
