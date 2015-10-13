#!/usr/bin/perl

use warnings;
use strict;

my $endString = "average read latency";

open(OUT, "> android-twitter-log.txt");

while(<STDIN>) {
    print OUT $_;
    if ($_ =~ /$endString/) {
        last;
    }
}

close(OUT);
