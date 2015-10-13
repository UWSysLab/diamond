#!/usr/bin/perl

use warnings;
use strict;

my $ANDROID_DIR = "/home/nl35/research/android-sdk-linux/platform-tools";

system("mv android-twitter-log.txt android-twitter-log-old.txt");
#system("$ANDROID_DIR/adb logcat -c");
#system("$ANDROID_DIR/adb logcat | ./android-twitter-helper.pl");
system("NOW=\$(date +\"%m-%d %H:%M:%S.000\") ; $ANDROID_DIR/adb logcat -T \"\$NOW\" | ./android-twitter-helper.pl");

open(LOG, "android-twitter.txt");
while(<LOG>) {
    if ($_ =~ /timeline read time: (\d+\.*\d*)/) {
        my $time = $1;
        print "$time\n";
    }
}
