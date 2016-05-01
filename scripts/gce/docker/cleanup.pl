#!/usr/bin/perl

use warnings;
use strict;

my $usage = "usage: ./cleanup.pl gce-ip";
die $usage unless @ARGV==1;

my $GCE_IP = shift;

system("ssh $GCE_IP 'cd diamond-src/scripts; ./manage-servers.py kill ../platform/test/gce'");
system("ssh $GCE_IP 'cd diamond-src/scripts; ./manage-servers.py kill ../platform/test/gcelocaloneshard'");
system("ssh $GCE_IP 'cd diamond-src/scripts; ./manage-servers.py kill ../platform/test/gcelocalfiveshards'");
system("ssh $GCE_IP 'cd diamond-src/apps/baseline-benchmarks/keyvaluestore; ./manage-baseline-servers.py kill ~/diamond-src/platform/test/gce'");
system("ssh $GCE_IP 'cd diamond-src/apps/baseline-benchmarks/keyvaluestore; ./manage-baseline-servers.py kill ~/diamond-src/platform/test/gcelocaloneshard'");
system("ssh $GCE_IP 'cd diamond-src/apps/baseline-benchmarks/keyvaluestore; ./manage-baseline-servers.py kill ~/diamond-src/platform/test/gcelocalfiveshards'");
