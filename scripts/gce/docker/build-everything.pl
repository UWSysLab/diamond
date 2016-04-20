#!/usr/bin/perl

use warnings;
use strict;

my $usage = "usage: ./build-everything.pl image user gce-ip";
die $usage unless @ARGV==3;

my ($image, $user, $GCE_IP) = @ARGV;

system("ssh $GCE_IP 'cd diamond-src/apps/benchmarks/build; git pull origin master; make -j8'");
system("ssh $GCE_IP 'cd diamond-src/apps/baseline-benchmarks/keyvaluestore; mvn package'");
system("./build-kubernetes.pl $image $user");
