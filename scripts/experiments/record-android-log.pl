#!/usr/bin/perl

use warnings;
use strict;

my $ANDROID_DIR = "/home/nl35/research/android-sdk-linux/platform-tools";

die "usage: ./record-android-log.pl log_file" unless @ARGV==1;

my $logFile = shift;

system("NOW=\$(date +\"%m-%d %H:%M:%S.000\") ; $ANDROID_DIR/adb logcat -T \"\$NOW\" | ./record-android-log-helper.pl $logFile");
