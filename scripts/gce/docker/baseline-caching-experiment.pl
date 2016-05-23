#!/usr/bin/perl

use warnings;
use strict;

my $usage = "usage: ./baseline-caching-experiment.pl user image";
die $usage unless @ARGV == 2;

my ($user, $image) = @ARGV;

my $GCE_IP = "104.196.38.63";
my $USE_REDIS = 1;

my $outputDir = "../../experiments/results/caching";
my $gceOutputDir = "caching";

# set up log
my $log = "caching-log.txt";
system("rm -f $log; touch $log");

my $startBaselineCmd = "ssh -t $GCE_IP 'cd diamond-src/apps/baseline-benchmarks/100game; ./manage-baseline-100game-servers.py start' >> $log 2>&1";
my $killBaselineCmd = "ssh $GCE_IP 'cd diamond-src/apps/baseline-benchmarks/100game; ./manage-baseline-100game-servers.py kill' >> $log 2>&1";
my $startRedisCmd = "ssh -f $GCE_IP 'nohup redis-3.0.7/src/redis-server &' >> $log 2>&1";
my $killRedisCmd = "ssh $GCE_IP 'pkill redis-server'";

# make sure baseline servers are not running
system("$killBaselineCmd");

# do sanity checks
system("$startRedisCmd");
my $checkResult = system("./sanity-checks.pl run_baseline.py $outputDir $gceOutputDir $GCE_IP $USE_REDIS >> $log 2>&1");
if ($checkResult != 0) {
    die("Error in sanity checks: see log file for details");
}

# run experiments
#runBaseline(200, [2, 4, 6, 8, 10, 12, 14]);
runBaseline(200, [10, 12, 14]);



sub runBaseline {
    my ($numPairs, $instanceNumsRef) = @_;
    my @instanceNums = @{$instanceNumsRef};

    logPrint("Running baseline notification experiment\n");

    my $outFile = "$outputDir/baseline.txt";
    open(OUTFILE, "> $outFile");
    print(OUTFILE "clients\tthroughput\tlatency\tseconds\tinstances\n");
    for my $instances (@instanceNums) {
        logPrint("Running $instances instances...\n");

        # start baseline servers
        system("$startBaselineCmd");

        # clear output location
        if ($USE_REDIS) {
            system("ssh $GCE_IP 'redis-3.0.7/src/redis-cli -p 6379 flushdb' >> $log 2>&1");
        }
        else {
            system("ssh $GCE_IP 'rm diamond-src/scripts/experiments/$gceOutputDir/*' >> $log 2>&1");
        }

        # run Kubernetes instances
        system("./run-kubernetes-job.pl caching $image $user $instances run_redis_game.py --numpairs $numPairs >> $log 2>&1");

        # parse output
        my $clientCmd = "ls diamond-src/scripts/experiments/baseline | wc | awk \"{ print \\\$1 }\"";
        my $resultCmd = "cd diamond-src/scripts/experiments; ./parse-game.py -d $gceOutputDir";
        if ($USE_REDIS) {
            $clientCmd = "redis-3.0.7/src/redis-cli -p 6379 get clients";
            $resultCmd = "diamond-src/scripts/experiments/parse-game.py -r";
        }
        my $clients = `ssh $GCE_IP '$clientCmd'`;
        chomp($clients);
        my @result = `ssh $GCE_IP '$resultCmd'`;

        my $throughput = "ERROR";
        my $latency = "ERROR";
        my $abortrate = "ERROR";
        my $seconds = "ERROR";
        for my $line (@result) {
            if ($line =~ /^Avg\. turns\/second: ([\d\.]+)$/) {
                $throughput = $1;
            }
            elsif ($line =~ /^Avg\. time between turns \(s\): ([\d\.]+)$/) {
                $latency = $1;
            }
            elsif ($line =~ /over ([\d\.]+) seconds/) {
                $seconds = $1;
            }
        }
        print(OUTFILE "$clients\t$throughput\t$latency\t$seconds\t$instances\n");

        # kill baseline servers
        system("$killBaselineCmd");
    }
    close(OUTFILE);
    my $time = time();
    system("cp $outFile $outFile.$time");

    return;
}

sub logPrint {
    my ($str) = @_;
    chomp($str);
    system("echo $str | tee -a $log");
    return;
}
