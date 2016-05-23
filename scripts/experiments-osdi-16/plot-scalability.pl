#!/usr/bin/perl

use warnings;
use strict;

my $usage = "usage: ./plot-scalability.pl directory";
die $usage unless @ARGV == 1;

my $dir = shift;

my @shards = (1, 2, 3, 4, 5);
my @levels = ("linearizable", "snapshot");

my @tempFiles;

for my $level (@levels) {
    my $levelFile = "temp-$level.txt";
    push(@tempFiles, $levelFile);
    open(LEVEL_FILE, "> $levelFile");
    for my $shard(@shards) {
        open(SHARD_FILE, "$dir/$level.$shard.txt");
        my $maxThroughput = 0;
        while(<SHARD_FILE>) {
            if ($_ =~ /^\d+\s+([\d\.]+)\s+[\d\.]+\s+[\d\.]+\s+[\d\.]+\s+\d+\s+\w+\s+\d+$/) {
                my $throughput = $1;
                if ($throughput > $maxThroughput) {
                    $maxThroughput = $throughput;
                }
            }
        }
        close(SHARD_FILE);
        print(LEVEL_FILE "$shard\t$maxThroughput\n");
    }
    close(LEVEL_FILE);
}

my $script = "temp-gnuplot-script.txt";
push(@tempFiles, $script);

my $linearizable = "temp-linearizable.txt";
my $snapshot = "temp-snapshot.txt";

open(SCRIPT, "> $script");
print(SCRIPT "set terminal pdfcairo\n");
print(SCRIPT "set output \"scalability.pdf\"\n");
print(SCRIPT "set xlabel \"Shards\"\n");
print(SCRIPT "set ylabel \"Throughput (txn/s)\"\n");
print(SCRIPT "set xtics 1\n");
print(SCRIPT "plot \"$linearizable\" using 1:2 with lines title 'linearizable',\\
                   \"$snapshot\" using 1:2 with lines title 'snapshot'\n");
close(SCRIPT);

system("gnuplot $script");

for my $file (@tempFiles) {
    system("rm $file");
}
