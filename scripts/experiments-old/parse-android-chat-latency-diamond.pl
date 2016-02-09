#!/usr/bin/perl

use warnings;
use strict;

die "usage: ./android-chat-latency-diamond.pl log_file" unless @ARGV==1;

my $logFile = shift;

my $dir = "android-chat-latency";

open(LOG, $logFile);
open(ATOMIC_READS, "> $dir/atomic-reads.txt");
open(ATOMIC_WRITES, "> $dir/atomic-writes.txt");
open(TRANS_READS, "> $dir/transaction-reads.txt");
open(TRANS_WRITES, "> $dir/transaction-writes.txt");
open(STALE_READS, "> $dir/stale-reads.txt");
open(STALE_WRITES, "> $dir/stale-writes.txt");
while(<LOG>) {
    chomp($_);
    if ($_ =~ /data:\s+(\w+)\s+(\w+)\s+(\d+\.*\d*)/) {
        my $action = $1;
        my $dataset = $2;
        my $time = $3;
        if ($action eq "write") {
            if ($dataset eq "transaction") {
                print TRANS_WRITES "$time\n";
            }
            elsif ($dataset eq "stale") {
                print STALE_WRITES "$time\n";
            }
            elsif ($dataset eq "atomic") {
                print ATOMIC_WRITES "$time\n";
            }
        }
        elsif ($action eq "read") {
            if ($dataset eq "transaction") {
                print TRANS_READS "$time\n";
            }
            elsif ($dataset eq "stale") {
                print STALE_READS "$time\n";
            }
            elsif ($dataset eq "atomic") {
                print ATOMIC_READS "$time\n";
            }
        }
    }
}
close(LOG);
close(ATOMIC_READS);
close(ATOMIC_WRITES);
close(TRANS_READS);
close(TRANS_WRITES);
close(STALE_READS);
close(STALE_WRITES);
