#!/usr/bin/perl

# Generate config files for multiple frontend servers located on the same host with consecutive
# port numbers.

use warnings;
use strict;

my $usage = "usage: ./generate-frontend-configs.pl num-frontends prefix hostname startport";
die $usage unless @ARGV == 4;

my ($num, $prefix, $hostname, $startport) = @ARGV;

for (my $i = 0; $i < $num; $i++) {
    my $file = "> $prefix.frontend$i.config";
    my $port = $startport + $i;
    open(FILE, $file);
    print(FILE "f 1\n");
    print(FILE "replica $hostname:$port\n");
    close(FILE);
}
