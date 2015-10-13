#!/usr/bin/perl

use warnings;
use strict;

my $time = 5;
my $readFraction = 0.9;
my $prefix = "throughputlog";
my $errPrefix = "throughputerror";
my $concurrency = "transaction";
my $server = "localhost";

for (my $numClients = 1; $numClients < 20; $numClients++) {
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
    print("Num clients: $numClients\tThroughput: $throughput\tConcurrency: $concurrency");
    if ($concurrency eq "transaction") {
        my $avgAbortRate = $abortRateSum / $numClients;
        print("\tAvg abort rate: $avgAbortRate");
    }
    print("\n");
}
