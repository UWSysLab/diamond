#!/usr/bin/perl

use warnings;
use strict;

my $writeSum = 0;
my $numWrites = 0;
open(CLIENT1, "client1.log");
while (<CLIENT1>) {
    if ($_ =~ /^(\d+)\s+\S+\s+\S+\s+\S+\s+(\d+)$/) {
        my ($roundNum, $time) = ($1, $2);
        if ($roundNum >= 200 && $roundNum <= 800) {
            $writeSum += $time;
            $numWrites += 1;
        }
    }
}
close(CLIENT1);
if ($numWrites == 0) {
    print("No valid writes\n");
}
else {
    my $writeAvg = $writeSum / $numWrites;
    print("Number of writes: $numWrites\n");
    print("Average write latency: $writeAvg\n");
}

my $readSum = 0;
my $numReads = 0;
open(CLIENT2, "client2.log");
while (<CLIENT2>) {
    if ($_ =~ /^(\d+)\s+\S+\s+\S+\s+\S+\s+(\d+)$/) {
        my ($roundNum, $time) = ($1, $2);
        if ($roundNum >= 200 && $roundNum <= 800) {
            $readSum += $time;
            $numReads += 1;
        }
    }
}
close(CLIENT2);
if ($numReads == 0) {
    print("No valid reads\n");
}
else {
    my $readAvg = $readSum / $numReads;
    print("Number of reads: $numReads\n");
    print("Average read latency: $readAvg\n");
}
