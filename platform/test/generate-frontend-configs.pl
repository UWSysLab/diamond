#!/usr/bin/perl

# Generate config files for multiple frontend servers located on the same host with consecutive
# port numbers.

use warnings;
use strict;

my $usage = "usage: ./generate-frontend-configs.pl num-frontends prefix";
die $usage unless @ARGV == 2;

my @hosts = ("10.128.0.17",
             "10.128.0.13",
             "10.128.0.15",
             "10.128.0.11",
             "10.128.0.10",
             "10.128.0.14",
             "10.128.0.12",
             "10.128.0.16");

my $startport = 12345;

my ($num, $prefix) = @ARGV;

for (my $i = 0; $i < $num; $i++) {
    my $hostname = $hosts[$i % scalar(@hosts)];
    my $port = $startport + int($i / scalar(@hosts));

    my $file = "> $prefix.frontend$i.config";
    open(FILE, $file);
    print(FILE "f 1\n");
    print(FILE "replica $hostname:$port\n");
    close(FILE);
}
