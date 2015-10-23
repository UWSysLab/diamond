#!/usr/bin/perl

use warnings;
use strict;

my $time = 5;
my $warmupTimeMs = 5000;
my $startingNumClients = 5;
my $maxClients = 40;
my $numClientsStep = 5;
my $readFraction = 0.9;

my $dir = "desktopchat-throughput";
my $prefix = "$dir/run";

my $diamondServer = "moranis.cs.washington.edu";

my @baselineServers = ("spyhunter.cs.washington.edu", "breakout.cs.washington.edu", "tradewars.cs.washington.edu", "zork.cs.washington.edu");
my @clientMachines = ("qbert.cs.washington.edu", "pitfall.cs.washington.edu",
                      "tetris.cs.washington.edu", "charlottetown.cs.washington.edu", "nethack.cs.washington.edu");
my $SERVERS_PER_MACHINE = 8;

my $concurrency = "transaction";
my $staleness = "nostale";
my $stalelimit = "0";
my $log = "$dir/nostale-log.txt";
my $throughputFile = "$dir/nostale-results.txt";
my $abortRateFile = "$dir/nostale-abortrate.txt";

checkBaselineServers();

doExperiment();
parseThroughputs();
parseAbortRates();

$staleness = "stale";
$stalelimit = "100";
$log = "$dir/stale-log.txt";
$throughputFile = "$dir/stale-results.txt";
$abortRateFile = "$dir/stale-abortrate.txt";

doExperiment();
parseThroughputs();
parseAbortRates();

$log = "$dir/baseline-log.txt";
$throughputFile = "$dir/baseline-results.txt";

doBaselineExperiment();
parseThroughputs();

sub checkBaselineServers {
    my $numServers = 0;
    for my $server (@baselineServers) {
        my $pids = `ssh $server ps aux | grep baseline-server | grep java | awk '{print \$2}'`;
        my @pids = split(/\s+/, $pids);
        $numServers = $numServers + scalar(@pids);
    }
    if ($numServers > 0) {
        die "Error: some baseline chat servers from the last round are still alive";
    }
}

sub getNumDiamondClients {
    my $total = 0;
    for my $machine (@clientMachines) {
        my $pids = `ssh $machine ps aux | grep -v grep | grep DesktopChatClient | awk '{ print \$2 }'`;
        my @pids = split(/\s+/, $pids);
        $total = $total + @pids;
    }
    return $total;
}

sub getNumBaselineClients {
    my $total = 0;
    for my $machine (@clientMachines) {
        my $pids = `ssh $machine ps aux | grep -v grep | grep BaselineChatClient | awk '{ print \$2 }'`;
        my @pids = split(/\s+/, $pids);
        $total = $total + @pids;
    }
    return $total;
}

sub killBaselineServers {
    for my $server (@baselineServers) {
        my $pids = `ssh $server ps aux | grep baseline-server | grep java | awk '{print \$2}'`;
        my @pids = split(/\s+/, $pids);
        for my $pid (@pids) {
            system("ssh $server kill $pid");
        }
    }
}

sub parseThroughputs {
    system("cat $log | awk '
        BEGIN { print \"clients\tthroughput\" }
        { print \$3, \"\t\", \$5 }
        ' > $throughputFile");
}

sub parseAbortRates {
    system("cat $log | awk '
        BEGIN { print \"clients\tabortrate\" }
        { print \$3, \"\t\", \$11 }
        ' > $abortRateFile");
}


sub doExperiment {
    # fill chat log
    system("ssh $clientMachines[0] research/chat-program-package/diamond-client/diamond-client-package-wrapper.sh fixed 200 0.0 transaction concise $diamondServer filler throughputroom nostale 0 0 research/chat-program-package");

    open(FILE, "> $log");
    for (my $numClients = $startingNumClients; $numClients <= $maxClients; $numClients += $numClientsStep) {
        print("Experiment: $log Clients: $numClients\n");

        system("rm $prefix.*");

        for (my $i = 0; $i < $numClients; $i++) {
            my $clientMachine = @clientMachines[$i % scalar(@clientMachines)];
            system("ssh $clientMachine \"research/chat-program-package/diamond-client/diamond-client-package-wrapper.sh timed $time $readFraction $concurrency concise $diamondServer client$i throughputroom $staleness $stalelimit $warmupTimeMs research/chat-program-package\" > $prefix.$i.log 2> $prefix.$i.error &");
        }

        sleep(1);

        my $done = 0;
        my $startTime = time();
        while (!$done) {
            if (getNumDiamondClients() == 0) {
                $done = 1;
            }
            sleep(1);
        }
        my $endTime = time();
        my $waitTime = $endTime - $startTime;
        print("Waited for $waitTime seconds\n");

        my $totalNumActions = 0;
        my $abortRateSum = 0;

        for (my $i = 0; $i < $numClients; $i++) {
            my $lines = 0;
            open(LOG, "$prefix.$i.log");
            while(<LOG>) {
                chomp;
                if ($_ =~ /Summary:/) {
                    my @lineSplit = split(/\s+/);
                    my $numActions = $lineSplit[3];
                    $totalNumActions += $numActions;
                    if ($concurrency eq "transaction") {
                        my $abortRate = $lineSplit[7];
                        $abortRateSum += $abortRate;
                    }
                }
                $lines = $lines + 1;
            }
            close(LOG);
            if ($lines != 1) {
                die "Error: log file $i has $lines lines";
            }
        }

        my $throughput = $totalNumActions / $time;
        print(FILE "Num clients: $numClients\tThroughput: $throughput\tConcurrency: $concurrency");
        if ($concurrency eq "transaction") {
            my $avgAbortRate = $abortRateSum / $numClients;
            print(FILE "\tAvg abort rate: $avgAbortRate");
        }
        print(FILE "\n");
    }
    close(FILE);
}

sub doBaselineExperiment {
    print("Starting baseline servers\n");
    for (my $i = 0; $i < @baselineServers; $i++) {
        for (my $j = 0; $j < $SERVERS_PER_MACHINE; $j++) {
            my $server = $baselineServers[$i];
            my $port = 9000 + $j;
            system("ssh $server /homes/sys/nl35/research/chat-program-package/baseline-server/baseline-server-package-wrapper.sh $port /homes/sys/nl35/research/chat-program-package &");
        }
    }

    sleep(1);

    # fill chat log
    system("ssh $clientMachines[0] research/chat-program-package/baseline-client/baseline-client-package-wrapper.sh fixed 200 0.0 concise $baselineServers[0] 9000 filler 0 research/chat-program-package");

    open(FILE, "> $log");
    for (my $numClients = $startingNumClients; $numClients <= $maxClients; $numClients += $numClientsStep) {
        print("Experiment: $log Clients: $numClients\n");

        system("rm $prefix.*");

        for (my $i = 0; $i < $numClients; $i++) {
            my $serverNum = $i % @baselineServers;
            my $portOffset = ($i / @baselineServers) % $SERVERS_PER_MACHINE;
            my $server = $baselineServers[$serverNum];
            my $port = 9000 + $portOffset;
            my $clientMachine = @clientMachines[$i % scalar(@clientMachines)];
            system("ssh $clientMachine \"research/chat-program-package/baseline-client/baseline-client-package-wrapper.sh timed $time $readFraction concise $server $port client$i $warmupTimeMs research/chat-program-package\" > $prefix.$i.log 2> $prefix.$i.error &");
        }

        sleep(1);

        my $done = 0;
        my $startTime = time();
        while (!$done) {
            if (getNumBaselineClients() == 0) {
                $done = 1;
            }
            sleep(1);
        }
        my $endTime = time();
        my $waitTime = $endTime - $startTime;
        print("Waited for $waitTime seconds\n");

        my $totalNumActions = 0;
        my $abortRateSum = 0;

        for (my $i = 0; $i < $numClients; $i++) {
            my $lines = 0;
            open(LOG, "$prefix.$i.log");
            while(<LOG>) {
                chomp;
                if ($_ =~ /Summary:/) {
                    my @lineSplit = split(/\s+/);
                    my $numActions = $lineSplit[2];
                    $totalNumActions += $numActions;
                }
                $lines = $lines + 1;
            }
            close(LOG);
            if ($lines != 1) {
                killBaselineServers();
                die "Error: log file $i has $lines lines";
            }
        }

        my $throughput = $totalNumActions / $time;
        print(FILE "Num clients: $numClients\tThroughput: $throughput\tConcurrency: $concurrency\n");
    }
    close(FILE);

    killBaselineServers();
}
