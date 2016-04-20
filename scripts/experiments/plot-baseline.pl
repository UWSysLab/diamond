#!/usr/bin/perl

use warnings;
use strict;

my $usage = "usage: ./plot-baseline.pl directory";
die $usage unless @ARGV == 1;

my $dir = shift;

my @systems = ("diamond", "baseline");
my @modes = ("local", "georeplicated");

my @tempFiles;

for my $mode (@modes) {
    my $outFile = "temp-$mode.txt";
    push(@tempFiles, $outFile);
    open(OUT_FILE, "> $outFile");
    my $line = 0;
    for my $system (@systems) {
        open(IN_FILE, "$dir/$system.$mode.txt");
        my $maxThroughput = 0;
        while(<IN_FILE>) {
            if ($_ =~ /^\d+\s+([\d\.]+)\s+[\d\.]+\s+[\d\.]+\s+[\d\.]+\s+\d+$/) {
                my $throughput = $1;
                if ($throughput > $maxThroughput) {
                    $maxThroughput = $throughput;
                }
            }
        }
        close(IN_FILE);
        print(OUT_FILE "$system $maxThroughput\n");
    }
    close(OUT_FILE);
}

my $script = "temp-gnuplot-script.txt";
push(@tempFiles, $script);

my $georeplicated = "temp-georeplicated.txt";
my $local = "temp-local.txt";

open(SCRIPT, "> $script");
print(SCRIPT "set terminal pdfcairo\n");
print(SCRIPT "set output \"baseline-georeplicated.pdf\"\n");
print(SCRIPT "set ylabel \"Throughput (txn/s)\"\n");
print(SCRIPT "set yrange [0:*]\n");
print(SCRIPT "set boxwidth 0.5\n");
print(SCRIPT "set style fill solid\n");
print(SCRIPT "unset key\n");
print(SCRIPT "plot \"$georeplicated\" using (column(0)):2:xtic(1) with boxes\n");
close(SCRIPT);

system("gnuplot $script");

open(SCRIPT, "> $script");
print(SCRIPT "set terminal pdfcairo\n");
print(SCRIPT "set output \"baseline-local.pdf\"\n");
print(SCRIPT "set ylabel \"Throughput (txn/s)\"\n");
print(SCRIPT "set yrange [0:*]\n");
print(SCRIPT "set boxwidth 0.5\n");
print(SCRIPT "set style fill solid\n");
print(SCRIPT "unset key\n");
print(SCRIPT "plot \"$local\" using (column(0)):2:xtic(1) with boxes\n");
close(SCRIPT);

system("gnuplot $script");

for my $file (@tempFiles) {
    system("rm $file");
}
