#!/usr/bin/perl

use warnings;
use strict;

my $time = 5;
my $readFraction = 0.9;
my $prefix = "logthroughput";
my $concurrency = "transaction";

for (my $numClients = 1; $numClients < 20; $numClients++) {
    system("rm $prefix.*");

    for (my $i = 0; $i < $numClients; $i++) {
        system("./desktop-chat-wrapper.sh timed $time $readFraction $concurrency client$i throughputroom > $prefix.$i 2>error.$i &");
    }

    sleep($time + 5);

    my $totalNumActions = 0;

    for (my $i = 0; $i < $numClients; $i++) {
        my $lines = 0;
        open(LOG, "$prefix.$i");
        while(<LOG>) {
            chomp;
            my @lineSplit = split(/\s+/);
            my $numActions = $lineSplit[2];
            $totalNumActions += $numActions;
            $lines = $lines + 1;
        }
        close(LOG);
        if ($lines != 1) {
            die "Error: log file $i has $lines lines";
        }
    }

    my $throughput = $totalNumActions / $time;
    print("Num clients: $numClients\tThroughput: $throughput\tConcurrency: $concurrency\n");
}
