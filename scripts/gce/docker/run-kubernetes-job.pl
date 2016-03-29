#!/usr/bin/perl

use warnings;
use strict;

my $usage = "usage: ./run-kubernetes-job.pl job-name image script user instances";
if (@ARGV != 5) {
    print("$usage\n");
    exit(1);
}

my ($jobName, $image, $script, $user, $numInstances) = @ARGV;

my $tempJobFile = "temp-job.yaml";

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
print(JOB "                  image: $image\n");
print(JOB "                  args: [\"/home/$user/$script\"]\n");
print(JOB "            restartPolicy: Never\n");
print(JOB "    completions: $numInstances\n");
print(JOB "    parallelism: $numInstances\n");
close(JOB);

system("kubectl create -f $tempJobFile");

# Wait for job to finish
my $done = 0;
while (!$done) {
    sleep(1);
    my $pods = `kubectl get pods | wc | awk '{ print \$1 }'` - 1;
    if ($pods == 0) {
        $done = 1;
    }
}

# Delete job
system("kubectl delete job $jobName");

# Cleanup
system("rm $tempJobFile");
