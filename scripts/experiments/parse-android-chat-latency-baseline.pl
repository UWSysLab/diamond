#!/usr/bin/perl

use warnings;
use strict;

die "usage: ./parse-android-chat-latency-baseline.pl log_file" unless @ARGV==1;

my $logFile = shift;

my $dir = "android-chat-latency";

open(LOG, $logFile);
open(BASELINE_READS, "> $dir/baseline-reads.txt");
open(BASELINE_WRITES, "> $dir/baseline-writes.txt");
while(<LOG>) {
    chomp($_);
    if ($_ =~ /data:\s+(\w+)\s+(\d+\.*\d*)/) {
        my $action = $1;
        my $time = $2;
        if ($action eq "write") {
            print BASELINE_WRITES "$time\n";
        }
        elsif ($action eq "read") {
            print BASELINE_READS "$time\n";
        }
    }
    elsif ($_ =~ /Error:/) {
        die "Error detected: $_";
    }
}
close(LOG);
close(BASELINE_READS);
close(BASELINE_WRITES);
