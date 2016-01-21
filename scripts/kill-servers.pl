#!/usr/bin/perl

die "usage: ./kill-servers.pl config_prefix" unless @ARGV==1;

my $configPrefix = shift;

system("pkill -f $configPrefix");
