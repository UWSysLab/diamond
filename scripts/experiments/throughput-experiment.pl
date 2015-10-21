#!/usr/bin/perl

use warnings;
use strict;

my $time = 5;
my $warmupTimeMs = 1000;
my $maxClients = 20;
my $readFraction = 0.9;

my $dir = "desktopchat-throughput";
my $prefix = "$dir/run";

my $server = "localhost";

my $concurrency = "transaction";
my $staleness = "nostale";
my $stalelimit = "0";
my $log = "$dir/nostale-log.txt";
my $throughputFile = "$dir/nostale-results.txt";
my $abortRateFile = "$dir/nostale-abortrate.txt";

doExperiment();
parseThroughputs();
parseAbortRates();

$staleness = "stale";
$stalelimit = "100";
$log = "$dir/stale-log.txt";
$throughputFile = "$dir/stale-results.txt";
$abortRateFile = "$dir/stale-abortrate.txt";

doExperiment();
parseThroughputs();
parseAbortRates();

$log = "$dir/baseline-log.txt";
$throughputFile = "$dir/baseline-results.txt";

doBaselineExperiment();
parseThroughputs();

sub parseThroughputs {
    system("cat $log | awk '
        BEGIN { print \"clients\tthroughput\" }
        { print \$3, \"\t\", \$5 }
        ' > $throughputFile");
}

sub parseAbortRates {
    system("cat $log | awk '
        BEGIN { print \"clients\tabortrate\" }
        { print \$3, \"\t\", \$11 }
        ' > $abortRateFile");
}


sub doExperiment {
    # fill chat log
    system("./desktop-chat-wrapper.sh fixed 200 0.0 transaction concise $server filler throughputroom nostale 0 0");

    open(FILE, "> $log");
    for (my $numClients = 1; $numClients < $maxClients; $numClients++) {
        print("Experiment: $log Clients: $numClients\n");

        system("rm $prefix.*");

        for (my $i = 0; $i < $numClients; $i++) {
            system("./desktop-chat-wrapper.sh timed $time $readFraction $concurrency concise $server client$i throughputroom $staleness $stalelimit $warmupTimeMs > $prefix.$i.log 2> $prefix.$i.error &");
        }

        sleep($time + ($warmupTimeMs / 1000) + 1);

        my $totalNumActions = 0;
        my $abortRateSum = 0;

        for (my $i = 0; $i < $numClients; $i++) {
            my $lines = 0;
            open(LOG, "$prefix.$i.log");
            while(<LOG>) {
                chomp;
                if ($_ =~ /Summary:/) {
                    my @lineSplit = split(/\s+/);
                    my $numActions = $lineSplit[3];
                    $totalNumActions += $numActions;
                    if ($concurrency eq "transaction") {
                        my $abortRate = $lineSplit[7];
                        $abortRateSum += $abortRate;
                    }
                }
                $lines = $lines + 1;
            }
            close(LOG);
            if ($lines != 1) {
                die "Error: log file $i has $lines lines";
            }
        }

        my $throughput = $totalNumActions / $time;
        print(FILE "Num clients: $numClients\tThroughput: $throughput\tConcurrency: $concurrency");
        if ($concurrency eq "transaction") {
            my $avgAbortRate = $abortRateSum / $numClients;
            print(FILE "\tAvg abort rate: $avgAbortRate");
        }
        print(FILE "\n");
    }
    close(FILE);
}

sub doBaselineExperiment {
    my $DIAMOND_SRC = "/home/nl35/research/diamond-src";
    my $PROJECT_DIR = "$DIAMOND_SRC/apps/chat/BaselineChatServer";
    my $JAVA_BINARY = "/home/nl35/research/jdk1.8.0_60/jre/bin/java";
    my $classpath = "$PROJECT_DIR/bin:$PROJECT_DIR/libs/gson-2.3.1.jar:$PROJECT_DIR/libs/commons-pool2-2.0.jar:$PROJECT_DIR/libs/jedis-2.4.2.jar";
    my $cmd = "$JAVA_BINARY -cp $classpath Main 2> $dir/baseline-server.error &";
    system("$cmd");
    sleep(1);
    my $serverPid = `ps aux | grep -v grep | grep BaselineChatServer | awk '{ print \$2 }'`;
    chomp($serverPid);

    # fill chat log
    system("./baseline-chat-client-wrapper.sh fixed 200 0.0 concise $server filler 0");

    open(FILE, "> $log");
    for (my $numClients = 1; $numClients < $maxClients; $numClients++) {
        print("Experiment: $log Clients: $numClients\n");

        system("rm $prefix.*");

        for (my $i = 0; $i < $numClients; $i++) {
            system("./baseline-chat-client-wrapper.sh timed $time $readFraction concise $server client$i $warmupTimeMs > $prefix.$i.log 2> $prefix.$i.error &");
        }

        sleep($time + ($warmupTimeMs / 1000) + 1);

        my $totalNumActions = 0;
        my $abortRateSum = 0;

        for (my $i = 0; $i < $numClients; $i++) {
            my $lines = 0;
            open(LOG, "$prefix.$i.log");
            while(<LOG>) {
                chomp;
                if ($_ =~ /Summary:/) {
                    my @lineSplit = split(/\s+/);
                    my $numActions = $lineSplit[2];
                    $totalNumActions += $numActions;
                }
                $lines = $lines + 1;
            }
            close(LOG);
            if ($lines != 1) {
                die "Error: log file $i has $lines lines";
            }
        }

        my $throughput = $totalNumActions / $time;
        print(FILE "Num clients: $numClients\tThroughput: $throughput\tConcurrency: $concurrency\n");
    }
    close(FILE);

    system("kill $serverPid");
}
