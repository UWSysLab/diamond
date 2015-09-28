#!/usr/bin/perl

use warnings;
use strict;

my $time = 5;
my $readFraction = 0.9;
my $file = "logthroughput.txt";


for (my $numClients = 1; $numClients < 20; $numClients++) {
    system("rm $file");

    for (my $i = 0; $i < $numClients; $i++) {
        system("./desktop-chat-wrapper.sh timed $time $readFraction client$i throughputroom >> $file 2>/dev/null &");
    }

    sleep($time + 2);

    my $totalNumActions = 0;

    open(LOG, $file);
    while(<LOG>) {
        chomp;
        my @lineSplit = split(/\s+/);
        my $numActions = $lineSplit[2];
        $totalNumActions += $numActions;
    }
    close(LOG);

    my $throughput = $totalNumActions / $time;
    print("Num clients: $numClients\tThroughput: $throughput\n");
}
