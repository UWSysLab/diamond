#!/usr/bin/perl

use warnings;
use strict;

my $usage = "usage: ./plot-docc.pl docc-increment-file docc-noincrement-file";
die $usage unless @ARGV == 2;

my ($incrData, $noincrData) = @ARGV;

my $script = "temp-gnuplot-script.txt";

open(SCRIPT, "> $script");

print(SCRIPT "set terminal pdfcairo\n");
print(SCRIPT "set output \"docc-throughput.pdf\"\n");
print(SCRIPT "set xlabel \"clients\"\n");
print(SCRIPT "set ylabel \"throughput(txn/s)\"\n");
print(SCRIPT "plot \"$incrData\" using 1:2 with lines title 'DOCC', \"$noincrData\" using 1:2 with lines title 'baseline'\n");

print(SCRIPT "set output \"docc-abortrate.pdf\"\n");
print(SCRIPT "set xlabel \"clients\"\n");
print(SCRIPT "set ylabel \"% aborted txns\"\n");
print(SCRIPT "plot \"$incrData\" using 1:4 with lines title 'DOCC', \"$noincrData\" using 1:4 with lines title 'baseline'\n");

close(SCRIPT);

system("gnuplot $script");
system("rm $script");
