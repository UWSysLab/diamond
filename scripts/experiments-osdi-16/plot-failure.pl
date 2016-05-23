#!/usr/bin/perl

use warnings;
use strict;

my $usage = "usage: ./plot-failure.pl directory";
die $usage unless @ARGV==1;

my $i = 0;

my $tempFile = "temp-failure.txt";
open(TEMP, "> $tempFile");

my $directory = shift;
open(FILE, "$directory/failure-output-1.txt");
while(<FILE>) {
    if ($_ =~ /^(\d+)\s+(\d+)$/) {
        my $latency = $2 - $1;
        print(TEMP "$i\t$latency\n");
        $i += 1;
    }
}
close(FILE);

close(TEMP);

my $script = "temp-gnuplot-script.txt";
open(SCRIPT, "> $script");
print(SCRIPT "set terminal pdfcairo\n");
print(SCRIPT "set output \"failure.pdf\"\n");
print(SCRIPT "set xlabel \"Turn\"\n");
print(SCRIPT "set ylabel \"Latency (ms)\"\n");
print(SCRIPT "unset key\n");
print(SCRIPT "plot \"$tempFile\" using 1:2 with lines");
close(SCRIPT);

system("gnuplot $script");

system("rm $tempFile");
system("rm $script");
