#!/usr/bin/perl

# Generate config files for multiple frontend servers located on the same host with consecutive
# port numbers.

use warnings;
use strict;

my $usage = "usage: ./generate-frontend-configs.pl num-frontends prefix";
die $usage unless @ARGV == 2;

my @hosts = ("diamond-frontend-central-011u",
             "diamond-frontend-central-861o",
             "diamond-frontend-central-f1r3",
             "diamond-frontend-central-h166",
             "diamond-frontend-central-jvxz",
             "diamond-frontend-central-ogog",
             "diamond-frontend-central-xffz",
             "diamond-frontend-central-xt1j");

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
