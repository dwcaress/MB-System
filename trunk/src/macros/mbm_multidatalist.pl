eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
                    & eval 'exec perl -S $0 $argv:q'
                         if 0;
#--------------------------------------------------------------------
#    The MB-system:	mbm_multidatalist.perl	7/18/2011
#   $Id: mbm_multidatalisz.pl 19XX 2011-07-18 15:40:00Z ferreira $
#
#    Copyright (c) 2011-2012 by 
#    D. W. Caress (caress@mbari.org)
#      Monterey Bay Aquarium Research Institute
#      Moss Landing, CA
#    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
#      Lamont-Doherty Earth Observatory
#      Palisades, NY  10964
#
#    See README file for copying and redistribution conditions.
#--------------------------------------------------------------------
#
# Command:
#   mbm_multidatalist
#
# Purpose:
#   Perl shellscript to generate standard ancilliary files using 
#   parallel mbdatalist -O processes
#
# Usage:
#   mbm_multidatalist -Xcpu -Idatalist
#
# Author:
#   Christian Ferreira
#   MARUM - Center for Marine Environmental Sciences
#   Klagenfurter Str. 2, GEO Building
#   28359, Bremen, Germany
#   July 17, 2011
#
# Version:
#   $Id: mbm_multidatalist.pl 19XX 2011-07-18 15:40:00Z ferreira $
#
# Revisions:
#
#  Revision 1.0		2011-07-18 15:40:00	cferreira
#  Initial version with partial implementation of mbdatalist flags
#

$program_name = "mbm_multidatalist";

# Deal with command line arguments
$command_line = "@ARGV";
&Getopts('I:i:NnX:x:HhVv');
$help =    		($opt_H || $opt_h);
$datalist =    		($opt_I || $opt_i || "datalist.mb-1");
$regenerate =    	($opt_N || $opt_n);
$verbose = 		($opt_V || $opt_v);
$cpu =        		($opt_X || $opt_x || 1);

# print out help message if required
if ($help)
	{
	print "\n$program_name:\n";
	print "\nVersion: mbm_multidatalist.pl 19XX 2011-07-18 15:40:00Z ferreira\n";
	print "\nMacro to use mbdatalist with the -O option to generate standard ancilliary files, using parallel processes\n";
	print "\n";
	print "\nBasic Usage: \n\n";
	print "\t$program_name -Idatalist -Xnprocesses [-N -H -V]\n\n";
	exit 0;
	}

# tell the world we got started
if ($verbose) 
	{
	print "\nRunning $program_name...\n";
	}

# exiting if CPU number is lower than 1
if ($cpu < 1) 
	{
	print "\nExiting $program_name, CPU(s) number(s) should be\n equal or bigger than one...\n\n";
	exit 0;
	}
	
# set the mbdatalist control option
if ($regenerate)
	{
	$option = "-N";
	}
else
	{
	$option = "-O";
	}

# loop over all files referenced in datalist
@mbdatalist = `mbdatalist -F-1 -I$datalist`;
while (@mbdatalist)
	{
	$line = shift @mbdatalist;
	if ($line =~ /(\S+)\s+(\S+)/)
		{
		($file_mb,$format_mb) = 
			$line =~ /(\S+)\s+(\S+)/;
		push(@files_data, $file_mb);
		push(@formats, $format_mb);
		}
	}

# start parellel computing
use Parallel::ForkManager;
use LWP::Simple;
my $pm=new Parallel::ForkManager($cpu);
  
foreach $file_mb (@files_data)
	{
	my $pid = $pm->start and next;
	if ($verbose) 
		{
		$command = "mbdatalist -I$file_mb $option -V";
		print "Executing $command\n";
		}
	else
		{
		$command = "mbdatalist -I$file_mb $option";
		}
	system $command;
	$pm->finish;
	}

$pm->wait_all_children;

exit 0;

#-----------------------------------------------------------------------
# This should be loaded from the library but its safer to
# just include it....
#
;# getopts.pl - a better getopt.pl

;# Usage:
;#      do Getopts('a:bc');  # -a takes arg. -b & -c not. Sets opt_* as a
;#                           #  side effect.

sub Getopts {
    local($argumentative) = @_;
    local(@args,$_,$first,$rest);
    local($errs) = 0;
    local($[) = 0;

    @args = split( / */, $argumentative );
    while(@ARGV && ($_ = $ARGV[0]) =~ /^-(.)(.*)/) {
     ($first,$rest) = ($1,$2);
     $pos = index($argumentative,$first);
     if($pos >= $[) {
         if($args[$pos+1] eq ':') {
          shift(@ARGV);
          if($rest eq '') {
              ++$errs unless @ARGV;
              $rest = shift(@ARGV);
          }
          eval "\$opt_$first = \$rest;";
         }
         else {
          eval "\$opt_$first = 1";
          if($rest eq '') {
              shift(@ARGV);
          }
          else {
              $ARGV[0] = "-$rest";
          }
         }
     }
     else {
         print STDERR "Unknown option: $first\n";
         ++$errs;
         if($rest ne '') {
          $ARGV[0] = "-$rest";
         }
         else {
          shift(@ARGV);
         }
     }
    }
    $errs == 0;
}
