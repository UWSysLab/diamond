#!/usr/bin/perl

use warnings;
use strict;

my $ANDROID_DIR = "/home/nl35/research/android-sdk-linux/platform-tools";

die "usage: ./android-twitter.pl data_file log_file" unless @ARGV == 2;

my ($dataFile, $logFile) = @ARGV;

#system("$ANDROID_DIR/adb logcat -c");
#system("$ANDROID_DIR/adb logcat | ./android-twitter-helper.pl");
system("NOW=\$(date +\"%m-%d %H:%M:%S.000\") ; $ANDROID_DIR/adb logcat -T \"\$NOW\" | ./android-twitter-helper.pl $logFile");

open(LOG, $logFile);
open(DATA, "> $dataFile");
while(<LOG>) {
    if ($_ =~ /timeline read time: (\d+\.*\d*)/) {
        my $time = $1;
        print DATA "$time\n";
    }
}
close(LOG);
close(DATA)
