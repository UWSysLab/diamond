#!/usr/bin/perl

# Delete jobs with the given job name on all of the Kubernetes clusters.
# Pulls cluster information from a 'clusters.txt' file located in the working directory.

use warnings;
use strict;

my $usage = "usage: ./kubectl-delete-job.pl job-name";
die $usage unless @ARGV==1;

my $job = shift;

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
    system("kubectl delete job $job");
}
