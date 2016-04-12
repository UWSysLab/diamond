#!/usr/bin/perl

use warnings;
use strict;

my $usage = "usage: ./scalability-experiment.pl user image";
die $usage unless @ARGV == 2;

my ($user, $image) = @ARGV;

my $GCE_IP = "8.35.196.178";
my $USE_REDIS = 1;

my $outputDir = "../../experiments/results/scalability";
my $script = "run_scalability.py";
my $gceOutputDir = "scalability";

# set up log
my $log = "scalability-log.txt";
system("rm -f $log; touch $log");

my $checkResult = system("./sanity-checks.pl $image $user $script $outputDir $gceOutputDir $GCE_IP $USE_REDIS >> $log 2>&1");
if ($checkResult != 0) {
    die("Error in sanity checks: see log file for details");
}

# run experiment
my @instanceNums = (1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
my @shardNums = (1, 2, 3, 4, 5);
my @isolations = ("linearizable", "snapshot", "eventual");

my $kill_command = "cd diamond-src/scripts; ./manage-servers.py kill ../platform/test/gce";
system("ssh $GCE_IP '$kill_command'");

for my $isolation (@isolations) {
    for my $shards (@shardNums) {
        system("ssh -t $GCE_IP 'cd diamond-src/scripts; ./manage-servers.py start ../platform/test/gce --keys experiments/keys.txt --numkeys 100000 --shards $shards'");

        my $outFile = "$outputDir/$isolation.$shards.txt";
        open(OUTFILE, "> $outFile");
        print(OUTFILE "clients\tthroughput\tlatency\tabortrate\tseconds\tinstances\tisolation\tshards\n");
        for my $instances (@instanceNums) {
            print("Running $instances instances...\n");
            if ($USE_REDIS) {
                system("ssh $GCE_IP 'redis-3.0.7/src/redis-cli flushdb' >> $log 2>&1");
            }
            else {
                system("ssh $GCE_IP 'rm diamond-src/scripts/experiments/$gceOutputDir/*' >> $log 2>&1");
            }
            system("./run-kubernetes-job.pl scalability $image $user $instances $script $isolation >> $log 2>&1");
            my $clientCmd = "ls diamond-src/scripts/experiments/scalability | wc | awk \"{ print \\\$1 }\"";
            my $resultCmd = "cd diamond-src/scripts/experiments; ./parse-scalability.py -d $gceOutputDir";
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
            print(OUTFILE "$clients\t$throughput\t$latency\t$abortrate\t$seconds\t$instances\t$isolation\t$shards\n");
        }
        close(OUTFILE);
        my $time = time();
        system("cp $outFile $outFile.$time");

        system("ssh $GCE_IP '$kill_command'");
    }
}
