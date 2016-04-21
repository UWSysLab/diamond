#!/usr/bin/perl

use warnings;
use strict;

my $usage = "usage: ./baseline-experiment.pl user image";
die $usage unless @ARGV == 2;

my ($user, $image) = @ARGV;

my $GCE_IP = "104.196.38.63";
my $USE_REDIS = 1;

my $outputDir = "../../experiments/results/baseline";
my $gceOutputDir = "baseline";

# set up log
my $log = "baseline-log.txt";
system("rm -f $log; touch $log");

# experimental parameters
# modes: georeplicated, local
# isolation levels: linearizable, snapshot, eventual
# zipf: 0.3, 0.8 (maybe other numbers work too?)

my %configs;
$configs{"georeplicated"} = "gce";
$configs{"local"} = "gcelocaloneshard";

my %startDiamondCmd;
my %killDiamondCmd;
my %killBaselineCmd;
$startDiamondCmd{"georeplicated"} = "ssh -t $GCE_IP 'cd diamond-src/scripts; ./manage-servers.py start ../platform/test/gce --keys experiments/keys.txt --numkeys 100000 --shards 1' >> $log 2>&1";
$startDiamondCmd{"local"} = "ssh -t $GCE_IP 'cd diamond-src/scripts; ./manage-servers.py start ../platform/test/gcelocaloneshard --keys experiments/keys.txt --numkeys 100000 --shards 1' >> $log 2>&1";
$killDiamondCmd{"georeplicated"} = "ssh $GCE_IP 'cd diamond-src/scripts; ./manage-servers.py kill ../platform/test/gce' >> $log 2>&1";
$killDiamondCmd{"local"} = "ssh $GCE_IP 'cd diamond-src/scripts; ./manage-servers.py kill ../platform/test/gcelocaloneshard' >> $log 2>&1";
$killBaselineCmd{"georeplicated"} = "ssh $GCE_IP 'cd diamond-src/apps/baseline-benchmarks/keyvaluestore; ./manage-baseline-servers.py kill ~/diamond-src/platform/test/gce' >> $log 2>&1";
$killBaselineCmd{"local"} = "ssh $GCE_IP 'cd diamond-src/apps/baseline-benchmarks/keyvaluestore; ./manage-baseline-servers.py kill ~/diamond-src/platform/test/gcelocaloneshard' >> $log 2>&1";

my $startRedisCmd = "ssh -f $GCE_IP 'nohup redis-3.0.7/src/redis-server &' >> $log 2>&1";
my $killRedisCmd = "ssh $GCE_IP 'pkill redis-server'";

# Make sure no servers are running
system("./build-everything.pl $image $user $GCE_IP >> $log 2>&1");
system("./cleanup.pl $GCE_IP >> $log 2>&1");

# do sanity checks
system("$startRedisCmd");
my $checkResult = system("./sanity-checks.pl run_baseline.py $outputDir $gceOutputDir $GCE_IP $USE_REDIS >> $log 2>&1");
if ($checkResult != 0) {
    die("Error in sanity checks: see log file for details");
}
system("$killRedisCmd");

######## BASELINE ########

runBaseline("local", "0.3", 128, [3, 4, 5, 6, 7]);

####### DIAMOND #######

runDiamond("local", "linearizable", "0.3", 64, [4, 5, 6, 7, 8]);

sub getStartBaselineCmd {
    my ($mode, $zipf) = @_;
    my $configPrefix = "";
    if ($mode eq "georeplicated") {
        $configPrefix = "gce";
    }
    elsif ($mode eq "local") {
        $configPrefix = "gcelocaloneshard";
    }
    my $startBaselineCmd = "ssh -t $GCE_IP 'cd diamond-src/apps/baseline-benchmarks/keyvaluestore; ./manage-baseline-servers.py start ~/diamond-src/platform/test/$configPrefix --keys ~/diamond-src/scripts/experiments/keys.txt --numkeys 100000 --zipf $zipf' >> $log 2>&1";
    return $startBaselineCmd;
}

sub runBaseline {
    my ($mode, $zipf, $numClients, $instanceNumsRef) = @_;
    my @instanceNums = @{$instanceNumsRef};

    logPrint("Running Baseline in mode $mode with zipf $zipf\n");
    resetClient();
    system("$startRedisCmd");
    system(getStartBaselineCmd($mode, $zipf));

    my $outFile = "$outputDir/baseline.$mode.$zipf.txt";
    open(OUTFILE, "> $outFile");
    print(OUTFILE "clients\tthroughput\tlatency\tabortrate\tseconds\tinstances\n");
    for my $instances (@instanceNums) {
        logPrint("Running $instances instances...\n");

        # clear output location
        if ($USE_REDIS) {
            system("ssh $GCE_IP 'redis-3.0.7/src/redis-cli -p 6379 flushdb' >> $log 2>&1");
        }
        else {
            system("ssh $GCE_IP 'rm diamond-src/scripts/experiments/$gceOutputDir/*' >> $log 2>&1");
        }

        # run Kubernetes instances
        system("./run-kubernetes-job.pl baseline $image $user $instances run_baseline_retwis.py --config $configs{$mode} --numclients $numClients >> $log 2>&1");

        # parse output
        my $clientCmd = "ls diamond-src/scripts/experiments/baseline | wc | awk \"{ print \\\$1 }\"";
        my $resultCmd = "cd diamond-src/scripts/experiments; ./parse-scalability.py -d $gceOutputDir";
        if ($USE_REDIS) {
            $clientCmd = "redis-3.0.7/src/redis-cli -p 6379 get clients";
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
        print(OUTFILE "$clients\t$throughput\t$latency\t$abortrate\t$seconds\t$instances\n");
    }
    close(OUTFILE);
    my $time = time();
    system("cp $outFile $outFile.$time");

    # kill redis and baseline servers
    system("$killRedisCmd");
    system("$killBaselineCmd{$mode}");
    return;
}

sub runDiamond {
    my ($mode, $isolation, $zipf, $numClients, $instanceNumsRef) = @_;
    my @instanceNums = @{$instanceNumsRef};

    # reset client VM
    logPrint("Running Diamond in mode $mode with isolation $isolation and zipf $zipf\n");
    resetClient();

    my $outFile = "$outputDir/diamond.$mode.$isolation.$zipf.txt";
    open(OUTFILE, "> $outFile");
    print(OUTFILE "clients\tthroughput\tlatency\tabortrate\tseconds\tinstances\n");
    for my $instances (@instanceNums) {
        # start redis and Diamond servers
        system("$startRedisCmd");
        system("$startDiamondCmd{$mode}");
        logPrint("Running $instances instances...\n");

        # clear output location
        if ($USE_REDIS) {
            system("ssh $GCE_IP 'redis-3.0.7/src/redis-cli -p 6379 flushdb' >> $log 2>&1");
        }
        else {
            system("ssh $GCE_IP 'rm diamond-src/scripts/experiments/$gceOutputDir/*' >> $log 2>&1");
        }

        # run Kubernetes instances
        system("./run-kubernetes-job.pl baseline $image $user $instances run_retwis.py --config $configs{$mode} --numclients $numClients --zipf $zipf >> $log 2>&1");

        # parse output
        my $clientCmd = "ls diamond-src/scripts/experiments/baseline | wc | awk \"{ print \\\$1 }\"";
        my $resultCmd = "cd diamond-src/scripts/experiments; ./parse-scalability.py -d $gceOutputDir";
        if ($USE_REDIS) {
            $clientCmd = "redis-3.0.7/src/redis-cli -p 6379 get clients";
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
        print(OUTFILE "$clients\t$throughput\t$latency\t$abortrate\t$seconds\t$instances\n");

        # kill redis and Diamond servers
        system("$killRedisCmd");
        system("$killDiamondCmd{$mode}");
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
    system("gcloud compute instances reset diamond-client --zone us-central1-c >> $log 2>&1");
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
