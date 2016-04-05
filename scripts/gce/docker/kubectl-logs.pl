#!/usr/bin/perl

# View the result of 'kubectl logs' for the specified pod on the specified cluster

use warnings;
use strict;

my $usage = "usage: ./kubectl-logs.pl cluster pod";
die $usage unless @ARGV==2;

my ($cluster, $pod) = @ARGV;

my @clusters;
my @zones;
my $clusterFileInfo = `ls clusters.txt 2>&1`;
if ($clusterFileInfo =~ /cannot access/) {
    print(STDERR "Error: no clusters.txt file in current directory\n");
    exit(1);
}
open(CLUSTERS, "clusters.txt");
while(<CLUSTERS>) {
    if ($_ =~ /^(\S+)\s+(\S+)$/) {
        push(@clusters, $1);
        push(@zones, $2);
    }
}
close(CLUSTERS);

my $index = -1;
for (my $i = 0; $i < @clusters; $i++) {
    if ($clusters[$i] eq $cluster) {
        $index = $i;
    }
}

system("gcloud config set container/cluster $clusters[$index] 2> /dev/null");
system("gcloud container clusters get-credentials $clusters[$index] --zone $zones[$index] 2> /dev/null");
system("kubectl logs $pod");
