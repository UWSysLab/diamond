#!/usr/bin/perl

use warnings;
use strict;

my $usage = "usage: ./run-experiment.pl user image experiment";
die $usage unless @ARGV == 3;

my ($user, $image, $experiment) = @ARGV;

my $log = "$experiment-log.txt";
system("rm -f $log; touch $log");

# make sure GCE client is up-to-date and has all required files
system("ssh 104.154.73.35 'cd diamond-src/apps/benchmarks/build; git pull origin master; make -j8' >> $log 2>&1");
my $keyFileExists = `ssh 104.154.73.35 'ls diamond-src/scripts/experiments/keys.txt 2>/dev/null | wc' | awk '{ print \$1 }'`;
chomp($keyFileExists);
if (!$keyFileExists) {
    print(STDERR "Error: keys.txt file missing from scripts/experiments on GCE client\n");
    exit(1);
}
my $outputDirExists = `ssh 104.154.73.35 'ls -d diamond-src/scripts/experiments/$experiment 2>/dev/null | wc' | awk '{ print \$1 }'`;
chomp($outputDirExists);
if (!$outputDirExists) {
    print(STDERR "Error: scripts/experiments/$experiment does not exist on GCE client\n");
    exit(1);
}
my $frontendConfigsExist = `ssh 104.154.73.35 'ls diamond-src/platform/test/gce.frontend*.config 2>/dev/null | wc' | awk '{ print \$1 }'`;
chomp($frontendConfigsExist);
if (!$frontendConfigsExist) {
    print(STDERR "Error: no frontend config files detected on GCE client\n");
    exit(1);
}

# build Docker image
system("./build-kubernetes.pl $image $user >> $log 2>&1");

# run experiment
my @instanceNums = (1, 2, 3, 4, 5, 6, 7, 8, 9, 10);

print("clients\tthroughput\tlatency\tabortrate\tseconds\tinstances\n");

for my $instances (@instanceNums) {
    system("ssh 104.154.73.35 'rm diamond-src/scripts/experiments/$experiment/*' >> $log 2>&1");
    system("./run-kubernetes-job.pl $experiment $image run_$experiment.py $user $instances >> $log 2>&1");
    my $clients = `ssh 104.154.73.35 'ls diamond-src/scripts/experiments/$experiment | wc' | awk '{ print \$1 }'`;
    chomp($clients);
    my @result = `ssh 104.154.73.35 'cd diamond-src/scripts/experiments; ./parse-scalability.py $experiment'`;

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
