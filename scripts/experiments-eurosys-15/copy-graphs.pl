#!/usr/bin/perl

my $dir = "/home/nl35/research/paper-diamond/figures";

system("pdfcrop android-twitter-latency-plots.pdf");
system("mv android-twitter-latency-plots-crop.pdf $dir/android-twitter-latency-plots.pdf");

system("pdfcrop desktop-chat-throughput-plots.pdf");
system("mv desktop-chat-throughput-plots-crop.pdf $dir/desktop-chat-throughput-plots.pdf");

system("cp android-chat-latency-readers.pdf $dir");
system("cp android-chat-latency-writers.pdf $dir");
