#!/usr/bin/perl

use warnings;
use strict;

my $user = "nl35";
my $image = "nielgce";
my $log = "scalability-log.txt";

system("rm -f $log; touch $log");

# make sure GCE client is up-to-date and has all required files
system("ssh 104.154.73.35 'cd diamond-src/apps/benchmarks/build; git pull origin master; make -j8' >> $log 2>&1");
my $keyFileExists = `ssh 104.154.73.35 'ls diamond-src/scripts/experiments/keys.txt 2>/dev/null | wc' | awk '{ print \$1 }'`;
chomp($keyFileExists);
if (!$keyFileExists) {
    print(STDERR "Error: keys.txt file missing from scripts/experiments on GCE client\n");
    exit(1);
}
my $outputDirExists = `ssh 104.154.73.35 'ls -d diamond-src/scripts/experiments/scalability 2>/dev/null | wc' | awk '{ print \$1 }'`;
chomp($outputDirExists);
if (!$outputDirExists) {
    print(STDERR "Error: scripts/experiments/scalability does not exist on GCE client\n");
    exit(1);
}

# build Docker image
system("./build-kubernetes.pl $image $user >> $log 2>&1");

# run experiment
my @instanceNums = (1, 2, 3, 4, 5, 6, 7, 8, 9, 10);

print("clients\tthroughput\tlatency\tabortrate\tseconds\tinstances\n");

for my $instances (@instanceNums) {
    system("ssh 104.154.73.35 'rm diamond-src/scripts/experiments/scalability/*' >> $log 2>&1");
    system("./run-kubernetes-job.pl scaling $image run_scalability.py $user $instances >> $log 2>&1");
    my $clients = `ssh 104.154.73.35 'ls diamond-src/scripts/experiments/scalability | wc' | awk '{ print \$1 }'`;
    chomp($clients);
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
    print("$clients\t$throughput\t$latency\t$abortrate\t$seconds\t$instances\n");
}
