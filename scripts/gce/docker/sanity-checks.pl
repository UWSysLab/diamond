#!/usr/bin/perl

use warnings;
use strict;

my $usage = "usage: ./sanity-checks.pl image user script output-dir gce-ip use-redis";
die $usage unless @ARGV==6;

my ($image, $user, $script, $outputDir, $GCE_IP, $USE_REDIS) = @ARGV;

# make sure experiment script exists
my $scriptExists = `cat Dockerfile | grep $script | wc | awk '{ print \$1 }'`;
chomp($scriptExists);
if (!$scriptExists) {
    print(STDERR "Error: experiment is not included in Dockerfile\n");
    exit(1);
}

# make sure GCE client is up-to-date and has all required files
system("ssh $GCE_IP 'cd diamond-src/apps/benchmarks/build; git pull origin master; make -j8' 2>&1");
my $keyFileExists = `ssh $GCE_IP 'ls diamond-src/scripts/experiments/keys.txt 2>/dev/null | wc' | awk '{ print \$1 }'`;
chomp($keyFileExists);
if (!$keyFileExists) {
    print(STDERR "Error: keys.txt file missing from scripts/experiments on GCE client\n");
    exit(1);
}
my $frontendConfigsExist = `ssh $GCE_IP 'ls diamond-src/platform/test/gce.frontend*.config 2>/dev/null | wc' | awk '{ print \$1 }'`;
chomp($frontendConfigsExist);
if (!$frontendConfigsExist) {
    print(STDERR "Error: no frontend config files detected on GCE client\n");
    exit(1);
}

# make sure our chosen output method is set up properly
if ($USE_REDIS) {
    my $redisRunning = `ssh $GCE_IP './redis-3.0.7/src/redis-cli get test 2>&1'`;
    if ($redisRunning =~ /^Could not connect to Redis/) {
        print(STDERR "Error: redis not running on GCE client\n");
        exit(1);
    }
}
else {
    my $outputDirExists = `ssh $GCE_IP 'ls -d diamond-src/scripts/experiments/$outputDir 2>/dev/null | wc' | awk '{ print \$1 }'`;
    chomp($outputDirExists);
    if (!$outputDirExists) {
        print(STDERR "Error: scripts/experiments/$outputDir does not exist on GCE client\n");
        exit(1);
    }
}

# build Docker image
system("./build-kubernetes.pl $image $user 2>&1");
