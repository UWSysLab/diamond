#!/usr/bin/perl

use warnings;
use strict;

die "usage: ./parse-twitter-original.pl log_file" unless @ARGV == 1;

my $logFile = shift;

my $dir = "twitter-latency";

my $dataFile = "$dir/original.txt";

open(LOG, $logFile);
open(DATA, "> $dataFile");
while(<LOG>) {
    if ($_ =~ /OG twimight timeline read time: (\d+\.*\d*)/) {
        my $time = $1;
        print DATA "$time\n";
    }
}
close(LOG);
close(DATA);
