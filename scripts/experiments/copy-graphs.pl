#!/usr/bin/perl

my $dir = "/home/nl35/research/paper-diamond/figures";

system("cp android-twitter-latency-plots.pdf $dir");
system("cp desktop-chat-latency-plots.pdf $dir");
system("cp desktop-chat-throughput-plots.pdf $dir");
system("cp android-chat-latency-plots.pdf $dir");
