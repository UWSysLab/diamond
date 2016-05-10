#!/usr/bin/perl

use warnings;
use strict;

my $usage = "usage: ./retwis-docc-experiment.pl user image";
die $usage unless @ARGV == 2;

my ($user, $image) = @ARGV;

my $GCE_IP = "104.196.38.63";
my $USE_REDIS = 1;

my $outputDir = "../../experiments/results/docc";
my $gceOutputDir = "docc";

# set up log
my $log = "docc-log.txt";
system("rm -f $log; touch $log");

my $startDiamondCmd = "ssh -t $GCE_IP 'cd diamond-src/scripts; ./manage-servers.py start ../platform/test/gcelocalfiveshards --keys experiments/keys.txt --numkeys 100000 --batch 64' >> $log 2>&1";
my $killDiamondCmd = "ssh $GCE_IP 'cd diamond-src/scripts; ./manage-servers.py kill ../platform/test/gcelocalfiveshards' >> $log 2>&1";
my $startRedisCmd = "ssh -f $GCE_IP 'nohup redis-3.0.7/src/redis-server &' >> $log 2>&1";
my $killRedisCmd = "ssh $GCE_IP 'pkill redis-server'";

# make sure Diamond and redis servers are not running
system("$killRedisCmd");
system("$killDiamondCmd");

# do sanity checks
system("$startRedisCmd");
my $checkResult = system("./sanity-checks.pl run_retwis.py $outputDir $gceOutputDir $GCE_IP $USE_REDIS >> $log 2>&1");
if ($checkResult != 0) {
    die("Error in sanity checks: see log file for details");
}
system("$killRedisCmd");

# run experiments
runDiamond("linearizabledocc", 128, [4, 6, 8, 10]);
runDiamond("linearizable", 128, [8, 10, 12, 14]);



sub runDiamond {
    my ($isolation, $numClients, $instanceNumsRef) = @_;
    my @instanceNums = @{$instanceNumsRef};

    # reset client VM
    logPrint("Running DOCC experiment with isolation $isolation\n");

    my $outFile = "$outputDir/diamond.$isolation.txt";
    open(OUTFILE, "> $outFile");
    print(OUTFILE "clients\tthroughput\tlatency\tabortrate\tseconds\tinstances\t");
    for (my $i = 1; $i <= 5; $i++) {
        print(OUTFILE "throughput-$i\tlatency-$i\tabortrate-$i\t");
    }
    print(OUTFILE "\n");
    for my $instances (@instanceNums) {
        logPrint("Running $instances instances...\n");

        # start redis and Diamond servers
        resetClient();
        system("$startRedisCmd");
        system("$startDiamondCmd");

        # clear output location
        if ($USE_REDIS) {
            system("ssh $GCE_IP 'redis-3.0.7/src/redis-cli -p 6379 flushdb' >> $log 2>&1");
        }
        else {
            system("ssh $GCE_IP 'rm diamond-src/scripts/experiments/$gceOutputDir/*' >> $log 2>&1");
        }

        # run Kubernetes instances
        system("./run-kubernetes-job.pl docc $image $user $instances run_retwis.py --config gcelocalfiveshards --numclients $numClients --isolation $isolation --zipf 0.8 >> $log 2>&1");

        # parse output
        my $clientCmd = "ls diamond-src/scripts/experiments/$gceOutputDir | wc | awk \"{ print \\\$1 }\"";
        my $resultCmd = "cd diamond-src/scripts/experiments; ./parse-retwis.py -d $gceOutputDir";
        if ($USE_REDIS) {
            $clientCmd = "redis-3.0.7/src/redis-cli -p 6379 get clients";
            $resultCmd = "diamond-src/scripts/experiments/parse-retwis.py -r";
        }
        my $clients = `ssh $GCE_IP '$clientCmd'`;
        chomp($clients);
        my @result = `ssh $GCE_IP '$resultCmd'`;

        my %throughput;
        my %latency;
        my %abortrate;
        my $seconds = "ERROR";
        for my $line (@result) {
            if ($line =~ /^(\S+)\s+([\d\.]+)\s+([\d\.]+)\s+([\d\.]+)\s+\d+\s+\d+$/) {
                my $type = $1;
                $throughput{$type} = $2;
                $latency{$type} = $3;
                $abortrate{$type} = $4;
            }
            elsif ($line =~ /over ([\d\.]+) seconds/) {
                $seconds = $1;
            }
        }
        print(OUTFILE "$clients\t$throughput{Overall}\t$latency{Overall}\t$abortrate{Overall}\t$seconds\t$instances\t");
        for (my $i = 1; $i <= 5; $i++) {
            print(OUTFILE "$throughput{$i}\t$latency{$i}\t$abortrate{$i}\t");
        }
        print(OUTFILE "\n");

        # kill redis and Diamond servers
        system("$killRedisCmd");
        system("$killDiamondCmd");
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

sub resetClient {
    system("gcloud compute instances reset diamond-client --zone us-east1-c >> $log 2>&1");
    my $done = 0;
    while (!$done) {
        sleep(5);
        my $test = `ssh $GCE_IP 'echo test' 2>&1`;
        if ($test =~ /test/) {
            $done = 1;
        }
        elsif ($test =~ /Connection refused/) {
            logPrint("Waiting for GCE client VM to restart...");
        }
        else {
            logPrint("Error: unknown response to GCE client VM check: $test");
            exit(1);
        }
    }
}
