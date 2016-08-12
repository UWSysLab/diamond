#!/usr/bin/perl

use warnings;
use strict;

my $END_STR = "Done with Diamond experiment";

die "usage: ./record-android-log.pl log_file" unless @ARGV == 1;

my $log = shift;

open(OUT, "> $log");

while(<STDIN>) {
    print OUT $_;
    if ($_ =~ /$END_STR/) {
        last;
    }
}

close(OUT);
