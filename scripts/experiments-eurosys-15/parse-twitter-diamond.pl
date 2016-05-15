#!/usr/bin/perl

use warnings;
use strict;

die "usage: ./parse-twitter-diamond.pl log_file" unless @ARGV == 1;

my $logFile = shift;

my $dir = "twitter-latency";

my $prefetchFile = "$dir/prefetch.txt";
my $prefetchStaleFile = "$dir/prefetchstale.txt";
my $otherFile = "$dir/diamond.txt";

open(LOG, $logFile);
open(OTHER, "> $otherFile");
open(PREFETCH, "> $prefetchFile");
open(PREFETCHSTALE, "> $prefetchStaleFile");
while(<LOG>) {
    if ($_ =~ /Diamond timeline read time: (\d+\.*\d*)/) {
        my $time = $1;
        print OTHER "$time\n";
    }
    elsif ($_ =~ /Prefetch timeline read time: (\d+\.*\d*)/) {
        my $time = $1;
        print PREFETCH "$time\n";
    }
    elsif ($_ =~ /Prefetchstale timeline read time: (\d+\.*\d*)/) {
        my $time = $1;
        print PREFETCHSTALE "$time\n";
    }
}
close(LOG);
close(OTHER);
close(PREFETCH);
close(PREFETCHSTALE);
