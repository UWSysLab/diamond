#!/usr/bin/perl

use warnings;
use strict;

my $log = "scalability-log.txt";

system("ssh 104.154.73.35 'cd diamond-src/apps/benchmarks/build; git pull origin master; make -j8' > $log 2>&1");
system("./build-kubernetes.pl nielgce nl35 > $log 2>&1");

print("instances\tthroughput\tlatency\n");

my @instanceNums = (1, 5, 10, 20, 50, 100);

for my $instances (@instanceNums) {
    system("ssh 104.154.73.35 'rm diamond-src/scripts/experiments/scalability/*' > $log 2>&1");
    system("./run-kubernetes-job.pl scaling nielgce run_scalability.py nl35 $instances > $log 2>&1");
    my @result = `ssh 104.154.73.35 'cd diamond-src/scripts/experiments; ./parse-scalability.py scalability'`;

    my $throughput = "ERROR";
    my $latency = "ERROR";
    for my $line (@result) {
        if ($line =~ /^Avg\. throughput \(txn\/s\): ([\d\.]+)$/) {
            $throughput = $1;
        }
        elsif ($line =~ /^Avg\. latency \(s\): ([\d\.]+)$/) {
            $latency = $1;
        }
    }
    print("$instances\t$throughput\t$latency\n");
}
