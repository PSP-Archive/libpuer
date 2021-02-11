#!/usr/bin/perl
use strict;
use warnings;
use Digest::SHA::PurePerl;

my $pspsdk="/usr/local/pspdev/psp/sdk";
my $pspgcc="psp-gcc \"-I".$pspsdk."/include\" -c -x assembler-with-cpp - -o";
my $pspar="psp-ar crus ";

sub nid{
	#unpack once using little endian long.
	return sprintf("%08x",
		unpack("V1",
			Digest::SHA::PurePerl::sha1(shift)
		)
	);
}

sub genstub{
	my $arg=shift;
	my $objname=shift;
	open(my $fh,"|".$pspgcc.$objname);
	#print $arg."\n";
	print $fh <<EOM;
.set noreorder
#include "pspstub.s"
$arg
EOM
	close($fh);
}

###main
my $module="";
my $attr=0;
my @funcs=();
my @lines=();
my @objs=();
while(my $line=<>){ #will read $ARGV[0] or stdin.
	$line=~s/\n$//;
	$line=~s/\r$//;
	$line=~s/ //g;
	if($line=~/^PSP_EXPORT_START\((.+),0,0x(.+)\)$/){
		$module=$1;$attr=hex($2);
	}elsif($line=~/^PSP_EXPORT_START\((.+),0,([0-9]+)\)$/){
		$module=$1;$attr=$2;
	}elsif($line=~/^PSP_EXPORT_FUNC\((.+)\)$/ || $line=~/^PSP_EXPORT_FUNC_HASH\((.+)\)$/){
		foreach my $func(@funcs){if($func->[0] eq $1){goto func_duplicate;}}
		push(@funcs,[$1,nid($1)]);
func_duplicate:
	}elsif($line=~/^PSP_EXPORT_END$/){ #write!
		if($module ne "syslib"){
			my $latebind=8;
			push(@lines,"STUB_START \"".$module."\",0x".sprintf("%08x",($attr|$latebind)<<16).",0x".sprintf("%08x",(scalar(@funcs)<<16)|5));
			foreach my $func(@funcs){
				push(@lines,"STUB_FUNC 0x".$func->[1].",".$func->[0]);
			}
			push(@lines,"STUB_END");
			genstub(join("\n",@lines),$module.".o");push(@objs,$module.".o");
			#system($pspar." lib".$module.".a ".join(" ",@objs));
			#foreach my $obj(@objs){unlink($obj);}
		}
		@funcs=();
		@lines=();
		@objs=();
	}
}
