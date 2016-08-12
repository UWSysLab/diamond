#!/usr/bin/perl

use warnings;
use strict;

my $usage = "usage: ./plot-caching.pl directory";
die $usage unless @ARGV == 1;

my $dir = shift;

my $caching = "$dir/diamond.caching.txt";
my $nocaching = "$dir/diamond.nocaching.txt";

my $script = "temp-gnuplot-script.txt";

open(SCRIPT, "> $script");

print(SCRIPT "set terminal pdfcairo\n");
print(SCRIPT "set output \"caching.pdf\"\n");
print(SCRIPT "set xlabel \"Clients\"\n");
print(SCRIPT "set ylabel \"Time between turns (s)\"\n");
print(SCRIPT "plot \"$caching\" using 1:3 with lines title 'w/ caching',\\
                   \"$nocaching\" using 1:3 with lines title 'w/o caching'\n");

close(SCRIPT);

system("gnuplot $script");
system("rm $script");
