#!/usr/bin/perl

use warnings;
use strict;

my $usage = "usage: ./plot-baseline.pl directory";
die $usage unless @ARGV == 1;

my $dir = shift;

my @modes = ("local");
my @zipfNums = (0.3, 0.8);

my @tempFiles;

for my $mode (@modes) {
    for my $zipf (@zipfNums) {
        my $linearizable = getMaxThroughput("$dir/diamond.$mode.linearizable.$zipf.txt");
        my $snapshot = getMaxThroughput("$dir/diamond.$mode.snapshot.$zipf.txt");
        my $eventual = getMaxThroughput("$dir/diamond.$mode.eventual.$zipf.txt");
        my $baseline = getMaxThroughput("$dir/baseline.$mode.$zipf.txt");

        my $outFile = "temp-$mode-$zipf.txt";
        push(@tempFiles, $outFile);
        open(OUT_FILE, "> $outFile");
        print(OUT_FILE "linearizable $linearizable\n");
        print(OUT_FILE "snapshot $snapshot\n");
        print(OUT_FILE "eventual $eventual\n");
        print(OUT_FILE "baseline $baseline\n");
        close(OUT_FILE);
    }
}

my $script = "temp-gnuplot-script.txt";
push(@tempFiles, $script);

my $local3 = "temp-local-0.3.txt";
my $local8 = "temp-local-0.8.txt";

open(SCRIPT, "> $script");
print(SCRIPT "set terminal pdfcairo\n");
print(SCRIPT "set output \"baseline-local-0.3.pdf\"\n");
print(SCRIPT "set ylabel \"Throughput (txn/s)\"\n");
print(SCRIPT "set yrange [0:*]\n");
print(SCRIPT "set boxwidth 0.5\n");
print(SCRIPT "set style fill solid\n");
print(SCRIPT "unset key\n");
print(SCRIPT "plot \"$local3\" using (column(0)):2:xtic(1) with boxes\n");
close(SCRIPT);

system("gnuplot $script");

open(SCRIPT, "> $script");
print(SCRIPT "set terminal pdfcairo\n");
print(SCRIPT "set output \"baseline-local-0.8.pdf\"\n");
print(SCRIPT "set ylabel \"Throughput (txn/s)\"\n");
print(SCRIPT "set yrange [0:*]\n");
print(SCRIPT "set boxwidth 0.5\n");
print(SCRIPT "set style fill solid\n");
print(SCRIPT "unset key\n");
print(SCRIPT "plot \"$local8\" using (column(0)):2:xtic(1) with boxes\n");
close(SCRIPT);

system("gnuplot $script");

#for my $file (@tempFiles) {
#    system("rm $file");
#}

sub getMaxThroughput {
    my $file = shift;
    open(IN_FILE, $file);
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
    return $maxThroughput;
}
