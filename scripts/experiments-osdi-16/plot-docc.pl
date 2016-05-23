#!/usr/bin/perl

# Gnuplot script based on answer in http://stackoverflow.com/questions/5130517/clustered-bar-plot-in-gnuplot

use warnings;
use strict;

my $usage = "usage: ./plot-docc.pl directory";
die $usage unless @ARGV == 1;

my $dir = shift;

my @tempFiles;

my $outFile = "temp-docc.dat";
push(@tempFiles, $outFile);
open(OUT_FILE, "> $outFile");
my %nonDocc;
my %docc;
print(OUT_FILE "Type\tnon-DOCC\tDOCC\n");
for (my $i = 1; $i <= 5; $i++) {
    $nonDocc{$i} = getMaxThroughputByType($i, "$dir/diamond.linearizable.txt");
    $docc{$i} = getMaxThroughputByType($i, "$dir/diamond.docc.txt");
    print(OUT_FILE "$i\t$nonDocc{$i}\t$docc{$i}\n");
}
close(OUT_FILE);

my $script = "temp-gnuplot-script.txt";
push(@tempFiles, $script);

open(SCRIPT, "> $script");
print(SCRIPT "set terminal pdfcairo\n");
print(SCRIPT "set output \"docc.pdf\"\n");
print(SCRIPT "set style data histogram\n");
print(SCRIPT "set style histogram cluster gap 1\n");
print(SCRIPT "set style fill solid border rgb \"black\"\n");
print(SCRIPT "set auto x\n");
print(SCRIPT "set yrange [0:*]\n");
print(SCRIPT "set ylabel \"Throughput (txns/s)\"\n");
print(SCRIPT "set xlabel \"Retwis transaction type\"\n");
print(SCRIPT "set yrange [0:*]\n");
print(SCRIPT "plot \"$outFile\" using 2:xtic(1) title col, \\\n");
print(SCRIPT "'' using 3:xtic(1) title col\n");
close(SCRIPT);

system("gnuplot $script");

for my $file (@tempFiles) {
    system("rm $file");
}

sub getMaxOverallThroughput {
    my $file = shift;
    open(IN_FILE, $file);
    my $maxThroughput = 0;
    while(<IN_FILE>) {
        my @lineSplit = split(/\s+/, $_);
        if (!($lineSplit[0] eq "clients")) {
            my $throughput = $lineSplit[1];
            if ($throughput > $maxThroughput) {
                $maxThroughput = $throughput;
            }
        }
    }
    close(IN_FILE);
    return $maxThroughput;
}

sub getMaxThroughputByType {
    my ($type, $file) = @_;
    my $maxThroughput = 0;
    my $typeIndex = ($type - 1) * 3 + 5;
    open(IN_FILE, $file);
    while(<IN_FILE>) {
        my @lineSplit = split(/\s+/, $_);
        if (!($lineSplit[0] eq "clients")) {
            my $throughput = $lineSplit[$typeIndex];
            if ($throughput > $maxThroughput) {
                $maxThroughput = $throughput;
            }
        }
    }
    close(IN_FILE);
    return $maxThroughput;
}
