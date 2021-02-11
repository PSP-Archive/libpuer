#!/usr/bin/perl --
#Usage: wget -O - http://tfact.net/repos/psp/trunk/iso_tool/key.txt | prometheus.pl > key.txt
#Then copy latter part.

use strict;
use warnings;
while(my $line=<>){
	chomp($line);
	my @line=split(" ",$line);
	my $arg=shift(@line) || "";
	if($arg eq "tag"){print "tag ".lc($line[0])."\n";}
	elsif($arg eq "code"){print "code ".lc($line[0])."\n";}
	elsif($arg eq "key"){print "key ".lc(join("",@line))."\n";}
	elsif($arg eq "xor"){print "xor_key1 ".lc(join("",@line))."\n";}
	else{print $line."\n";}
}
