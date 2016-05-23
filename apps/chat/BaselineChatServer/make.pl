#!/usr/bin/perl

use warnings;
use strict;

my $usage = "./make.pl [java-dir]";

my $javac = "javac";
my $jar = "jar";

if (@ARGV >= 1) {
    my $javaDir = $ARGV[0];
    $javac = "$javaDir/bin/javac";
    $jar = "$javaDir/bin/jar";
}

system("$javac -classpath libs/commons-pool2-2.0.jar:libs/gson-2.3.1.jar:libs/jedis-2.4.2.jar:libs/jetty-all-9.3.7.v20160115-uber.jar src/BaselineChatServer.java");
system("$jar cfm baselinechatserver.jar src/MANIFEST.MF -C src/ .");
