#!/usr/bin/perl

use warnings;
use strict;

my $time = 5;
my $readFraction = 0.9;
my $prefix = "throughputlog";
my $errPrefix = "throughput";

my $server = "localhost";

my $concurrency = "transaction";
my $log = "desktopchat-throughput-transaction-log.txt";
my $throughputFile = "desktopchat-throughput-transaction.txt";
my $abortRateFile = "desktopchat-throughput-transaction-abortrates.txt";

doExperiment();
parseThroughputs();
parseAbortRates();

$concurrency = "atomic";
$log = "desktopchat-throughput-atomic-log.txt";
$throughputFile = "desktopchat-throughput-atomic.txt";

doExperiment();
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
    open(FILE, "> $log");
    for (my $numClients = 1; $numClients < 20; $numClients++) {
        print("Experiment: $log Clients: $numClients\n");

        system("rm $prefix.*");
        system("rm $errPrefix.*");

        for (my $i = 0; $i < $numClients; $i++) {
            system("./desktop-chat-wrapper.sh timed $time $readFraction $concurrency concise $server client$i throughputroom > $prefix.$i.txt 2> $errPrefix.$i.error &");
        }

        sleep($time + 5);

        my $totalNumActions = 0;
        my $abortRateSum = 0;

        for (my $i = 0; $i < $numClients; $i++) {
            my $lines = 0;
            open(LOG, "$prefix.$i.txt");
            while(<LOG>) {
                chomp;
                my @lineSplit = split(/\s+/);
                my $numActions = $lineSplit[3];
                $totalNumActions += $numActions;
                if ($concurrency eq "transaction") {
                    my $abortRate = $lineSplit[6];
                    $abortRateSum += $abortRate;
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
