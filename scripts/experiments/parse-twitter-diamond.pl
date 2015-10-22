#!/usr/bin/perl

use warnings;
use strict;

die "usage: ./parse-twitter-diamond.pl log_file" unless @ARGV == 1;

my $logFile = shift;

my $dir = "twitter-latency";

my $prefetchFile = "$dir/prefetch.txt";
my $otherFile = "$dir/diamond.txt";

open(LOG, $logFile);
open(OTHER, "> $otherFile");
open(PREFETCH, "> $prefetchFile");
while(<LOG>) {
    if ($_ =~ /Diamond timeline read time: (\d+\.*\d*)/) {
        my $time = $1;
        print OTHER "$time\n";
    }
    elsif ($_ =~ /Prefetch timeline read time: (\d+\.*\d*)/) {
        my $time = $1;
        print PREFETCH "$time\n";
    }
}
close(LOG);
close(OTHER);
close(PREFETCH);
