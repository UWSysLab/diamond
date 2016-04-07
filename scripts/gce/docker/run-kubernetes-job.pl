#!/usr/bin/perl

# Start multiple Kubernetes jobs running the given script, spread across multiple clusters.
# Pulls cluster information from a 'clusters.txt' file located in the working directory.
#
# This script:
# 1) Generates a temporary job YAML file for each cluster that runs the
#    cluster's instances in parallel, where the single argument supplied to each
#    instance's container is the full path to the given script.
# 2) Runs 'kubectl create' for each cluster to start the job.
# 3) Repeatedly polls all clusters using 'kubectl get pods' to see if the jobs are done.
# 4) Deletes the jobs using 'kubectl delete job' once all pods in the job are finished
#    running.

use warnings;
use strict;

my $usage = "usage: ./run-kubernetes-job.pl job-name image script user instances";
if (@ARGV != 5) {
    print("$usage\n");
    exit(1);
}

my ($jobName, $image, $script, $user, $numInstances) = @ARGV;

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

my @instancesPerCluster;
for (my $c = 0; $c < @clusters; $c++) {
    $instancesPerCluster[$c] = 0;
}
for (my $i = 0; $i < $numInstances; $i++) {
    my $clusterIndex = $i % @clusters;
    $instancesPerCluster[$clusterIndex]++;
}

print("Spreading instances across clusters as follows:\n");
print("cluster\tinstances\n");
for (my $i = 0; $i < @clusters; $i++) {
    print("$clusters[$i]\t$instancesPerCluster[$i]\n");
}

my $tempJobFile = "temp-job.yaml";
for (my $i = 0; $i < @clusters; $i++) {

    open(JOB, "> $tempJobFile");
    print(JOB "apiVersion: batch/v1\n");
    print(JOB "kind: Job\n");
    print(JOB "metadata:\n");
    print(JOB "    name: $jobName\n");
    print(JOB "spec:\n");
    print(JOB "    template:\n");
    print(JOB "        metadata:\n");
    print(JOB "            name: $jobName\n");
    print(JOB "        spec:\n");
    print(JOB "            containers:\n");
    print(JOB "                - name: $jobName\n");
    print(JOB "                  image: us.gcr.io/diamond-1239/$image\n");
    print(JOB "                  args: [\"/home/$user/$script\"]\n");
    print(JOB "            restartPolicy: Never\n");
    print(JOB "    completions: $instancesPerCluster[$i]\n");
    print(JOB "    parallelism: $instancesPerCluster[$i]\n");
    close(JOB);

    system("gcloud config set container/cluster $clusters[$i] 2> /dev/null");
    system("gcloud container clusters get-credentials $clusters[$i] --zone $zones[$i] 2> /dev/null");
    system("kubectl create -f $tempJobFile");
}

# Wait for job to finish
print("Waiting for jobs to finish...\n");
my $done = 0;
my $totalSleepSeconds = 0;
while (!$done) {
    my $sleepSeconds = 5;
    sleep($sleepSeconds);
    $totalSleepSeconds += $sleepSeconds;
    $done = 1;
    for (my $i = 0; $i < @clusters; $i++) {
        system("gcloud config set container/cluster $clusters[$i] 2> /dev/null");
        system("gcloud container clusters get-credentials $clusters[$i] --zone $zones[$i] 2> /dev/null");
        my $pods = `kubectl get pods | wc | awk '{ print \$1 }'` - 1;
        if ($pods > 0) {
            $done = 0;
        }
    }
    if ($totalSleepSeconds % 60 == 0) {
        print("Still waiting ($totalSleepSeconds seconds have passed)\n");
    }
}

# Delete job
print("Deleting jobs...\n");
for (my $i = 0; $i < @clusters; $i++) {
    system("gcloud config set container/cluster $clusters[$i] 2> /dev/null");
    system("gcloud container clusters get-credentials $clusters[$i] --zone $zones[$i] 2> /dev/null");
    system("kubectl delete job $jobName");
}

# Cleanup
system("rm $tempJobFile");
