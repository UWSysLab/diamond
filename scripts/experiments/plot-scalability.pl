#!/usr/bin/perl

use warnings;
use strict;

my $usage = "usage: ./plot-scalability.pl scalability-data-file";
die $usage unless @ARGV == 1;

my $data = shift;

my $script = "temp-gnuplot-script.txt";

open(SCRIPT, "> $script");

print(SCRIPT "set terminal pdfcairo\n");
print(SCRIPT "set output \"scalability-latency.pdf\"\n");
print(SCRIPT "unset key\n");
print(SCRIPT "set xlabel \"clients\"\n");
print(SCRIPT "set ylabel \"latency(s)\"\n");
print(SCRIPT "plot \"$data\" using 1:3 with lines\n");

print(SCRIPT "set output \"scalability-throughput.pdf\"\n");
print(SCRIPT "set xlabel \"clients\"\n");
print(SCRIPT "set ylabel \"throughput(txn/s)\"\n");
print(SCRIPT "plot \"$data\" using 1:2 with lines\n");

print(SCRIPT "set output \"scalability-throughput-latency.pdf\"\n");
print(SCRIPT "set xlabel \"throughput(txn/s)\"\n");
print(SCRIPT "set ylabel \"latency(s)\"\n");
print(SCRIPT "plot \"$data\" using 2:3 with lines\n");

close(SCRIPT);

system("gnuplot $script");
system("rm $script");
