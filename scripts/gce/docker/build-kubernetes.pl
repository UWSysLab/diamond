#!/usr/bin/perl

# Build and tag a Docker image and push it to the gcloud repository for Diamond

use warnings;
use strict;

my $usage = "usage: ./build-kubernetes.pl image-name username";

if (@ARGV != 2) {
    print("$usage\n");
    exit(1);
}

my ($image, $user) = @ARGV;

system("docker build -t $image --build-arg user=$user .");
system("docker tag $image us.gcr.io/diamond-1239/$image");
system("gcloud docker push us.gcr.io/diamond-1239/$image");
