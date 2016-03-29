#!/usr/bin/perl

use warnings;
use strict;

my $log = "scalability-log.txt";

system("rm -f $log; touch $log");

system("ssh 104.154.73.35 'cd diamond-src/apps/benchmarks/build; git pull origin master; make -j8' >> $log 2>&1");
system("./build-kubernetes.pl nielgce nl35 >> $log 2>&1");

print("instances\tthroughput\tlatency\tabortrate\tseconds\tcount\n");

my @instanceNums = (1, 5, 10, 20, 50, 100);

for my $instances (@instanceNums) {
    system("ssh 104.154.73.35 'rm diamond-src/scripts/experiments/scalability/*' >> $log 2>&1");
    system("./run-kubernetes-job.pl scaling nielgce run_scalability.py nl35 $instances >> $log 2>&1");
    my $count = `ssh 104.154.73.35 'ls diamond-src/scripts/experiments/scalability | wc' | awk '{ print \$1 }'`;
    chomp($count);
    my @result = `ssh 104.154.73.35 'cd diamond-src/scripts/experiments; ./parse-scalability.py scalability'`;

    my $throughput = "ERROR";
    my $latency = "ERROR";
    my $abortrate = "ERROR";
    my $seconds = "ERROR";
    for my $line (@result) {
        if ($line =~ /^Avg\. throughput \(txn\/s\): ([\d\.]+)$/) {
            $throughput = $1;
        }
        elsif ($line =~ /^Avg\. latency \(s\): ([\d\.]+)$/) {
            $latency = $1;
        }
        elsif ($line =~ /^Abort rate: ([\d\.]+)$/) {
            $abortrate = $1;
        }
        elsif ($line =~ /over ([\d\.]+) seconds/) {
            $seconds = $1;
        }
    }
    print("$instances\t$throughput\t$latency\t$abortrate\t$seconds\t$count\n");
}
