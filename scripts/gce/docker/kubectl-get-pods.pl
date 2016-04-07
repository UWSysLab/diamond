#!/usr/bin/perl

# View the result of 'kubectl get pods' on all of the Kubernetes clusters.
# Pulls cluster information from a 'clusters.txt' file located in the working directory.

use warnings;
use strict;

my $usage = "usage: ./kubectl-get-pods.pl";

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

for (my $i = 0; $i < @clusters; $i++) {
    system("gcloud config set container/cluster $clusters[$i] 2> /dev/null");
    system("gcloud container clusters get-credentials $clusters[$i] --zone $zones[$i] 2> /dev/null");
    print("==== Pods for $clusters[$i] ====\n");
    system("kubectl get pods");
}
