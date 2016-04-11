#!/usr/bin/perl

use warnings;
use strict;

my $usage = "usage: ./plot-docc.pl directory";
die $usage unless @ARGV == 1;

my $dir = shift;

my $docc = "$dir/docc.0.txt";
my $baseline = "$dir/baseline.0.txt";
my $doccReads = "$dir/docc.0.1.txt";
my $baselineReads = "$dir/baseline.0.1.txt";

my $script = "temp-gnuplot-script.txt";

open(SCRIPT, "> $script");

print(SCRIPT "set terminal pdfcairo\n");
print(SCRIPT "set output \"docc-throughput.pdf\"\n");
print(SCRIPT "set xlabel \"Clients\"\n");
print(SCRIPT "set ylabel \"Throughput (txn/s)\"\n");
print(SCRIPT "plot \"$docc\" using 1:2 with lines title 'DOCC',\\
                   \"$doccReads\" using 1:2 with lines title 'DOCC (90/10)',\\
                   \"$baseline\" using 1:2 with lines title 'baseline',\\
                   \"$baselineReads\" using 1:2 with lines title 'baseline (90/10)'\n");

print(SCRIPT "set output \"docc-abortrate.pdf\"\n");
print(SCRIPT "set xlabel \"Clients\"\n");
print(SCRIPT "set ylabel \"Abort rate\"\n");
print(SCRIPT "set yrange [0:1]\n");
print(SCRIPT "plot \"$doccReads\" using 1:4 with lines title 'DOCC (90/10)',\\
                   \"$baseline\" using 1:4 with lines title 'baseline',\\
                   \"$baselineReads\" using 1:4 with lines title 'baseline (90/10)'\n");

close(SCRIPT);

system("gnuplot $script");
system("rm $script");
