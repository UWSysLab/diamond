#!/usr/bin/perl

use warnings;
use strict;

my $time = 5;
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
    for (my $numClients = 1; $numClients < $maxClients; $numClients++) {
        print("Experiment: $log Clients: $numClients\n");

        system("rm $prefix.*");

        for (my $i = 0; $i < $numClients; $i++) {
            system("./desktop-chat-wrapper.sh timed $time $readFraction $concurrency concise $server client$i throughputroom $staleness $stalelimit > $prefix.$i.log 2> $prefix.$i.error &");
        }

        sleep($time + 1);

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
