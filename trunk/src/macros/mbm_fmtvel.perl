#! /usr/local/bin/perl
#
# mbm_fmt_vel
# $Id: mbm_fmtvel.perl,v 1.1 1993-05-25 05:43:54 caress Exp $
# 
# Runs hs_veldump on the input file with output to a temp file
# Extracts and formats the velocity data into a table into tabular 
# format for display and/or processing.
#
# Language: perl
#
# Columns are tab delimeted (for now)
# 
# BUGS:
#   There is no argument parser: you have to jam the flag and the file name
#   together just like hsveldump wants to see it.
#
# Dale Chayes  5/19/93
# $Log: not supported by cvs2svn $
# Revision 1.1  1993/05/20  04:03:13  dale
# Initial revision
#
#
###################################################
$SUFFIX=".fmt";

$OUTFILE = "+>"."mbm_fmtvel.d";
print "Output going to ",$OUTFILE,"\n";
require "ctime.pl";

$count=0;

open (I, "/usr/local/bin/hsveldump  $ARGV[0] $ARGV[1] |") ||
       die "couldn't open hsveldump";
open(O,$OUTFILE) || die "Can not open output file ";

$me = `/usr/bin/id`;
@me = split(' ',$me);
$now = &ctime(time);

# print "Data into: ", $OUTFILE, "\n";

print O "#  Hydrosweep data extracted with:\n";
print O "# /usr/local/bin/hsveldump ",$ARGV[0]," ", $ARGV[1], "\n";
print O "# on: ",$now,"# using dmb_fmtvel by: ";
print O @me[0],"\t",@me[1], "\n";
print O "# \n";
print O "# Date\t\tTime\t\tLongitude\tLatitude\tC-mean\tC-keel\n";


while (<I> ) {
  ++$count;
  print $_;

      if (/Time: (.*)/)
	{ 
	  @result = split(' ',$_);
	  print O  @result[1],"\t", @result[2],"\t" ;
	}
      if (/Longitude: (.*)/)
	{ 
	  @result = split(' ',$_);
	  print O  @result[1],"\t" ;
	}
      if (/Latitude: (.*)/)
	{ 
	  @result = split(' ',$_);
	  print O  @result[1],"\t" ;
	}
      if (/Mean velocity: (.*)/)
	{ 
	  @result = split(' ',$_);
	  $s=sprintf("%6.1f", @result[2]);
	  print O  $s,"\t" ;
	}
      if (/Keel velocity: (.*)/)
	{ 
	  @result = split(' ',$_);
	  $s=sprintf("%6.1f", @result[2]);
	  print O  $s,"\n" ;
	}

}
close O;
close I;
