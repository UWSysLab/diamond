#!/usr/bin/perl

use warnings;
use strict;

my $usage = "usage: ./docc-experiment.pl user image";
die $usage unless @ARGV == 2;

my ($user, $image) = @ARGV;

my $GCE_IP = "8.35.196.178";
my $USE_REDIS = 1;

my $outputDir = "../../experiments/results/docc";

# set up log
my $log = "docc-log.txt";
system("rm -f $log; touch $log");

my $checkResult = system("./sanity-checks.pl $image $user run_docc.py $outputDir docc $GCE_IP $USE_REDIS >> $log 2>&1");
if ($checkResult != 0) {
    die("Error in sanity checks: see log file for details");
}

# run experiment
my @instanceNums = (2, 4, 6, 8, 10);
my @optypes = ("docc", "baseline");

for my $optype (@optypes) {
    my $outFile = "$outputDir/$optype.txt";
    open(OUTFILE, "> $outFile");
    print(OUTFILE "clients\tthroughput\tlatency\tabortrate\tseconds\tinstances\toptype\n");
    for my $instances (@instanceNums) {
        print("Running $instances instances with optype $optype...\n");
        if ($USE_REDIS) {
            system("ssh $GCE_IP 'redis-3.0.7/src/redis-cli flushdb' >> $log 2>&1");
        }
        else {
            system("ssh $GCE_IP 'rm diamond-src/scripts/experiments/docc/*' >> $log 2>&1");
        }
        system("./run-kubernetes-job.pl docc $image $user $instances run_docc.py $optype >> $log 2>&1");
        my $clientCmd = "ls diamond-src/scripts/experiments/docc | wc | awk \"{ print \\\$1 }\"";
        my $resultCmd = "cd diamond-src/scripts/experiments; ./parse-scalability.py -d docc";
        if ($USE_REDIS) {
            $clientCmd = "redis-3.0.7/src/redis-cli get clients";
            $resultCmd = "diamond-src/scripts/experiments/parse-scalability.py -r";
        }
        my $clients = `ssh $GCE_IP '$clientCmd'`;
        chomp($clients);
        my @result = `ssh $GCE_IP '$resultCmd'`;

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
        print(OUTFILE "$clients\t$throughput\t$latency\t$abortrate\t$seconds\t$instances\t$optype\n");
    }
    close(OUTFILE);
    my $time = time();
    system("cp $outFile $outFile.$time");
}
