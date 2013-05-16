eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
                    & eval 'exec perl -S $0 $argv:q'
                         if 0;
#--------------------------------------------------------------------
#    The MB-system:	mbm_multicopy.perl	7/17/2011
#   $Id: mbm_multi7kprocess.pl 19XX 2011-07-17 15:15:00Z ferreira $
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
#   mbm_multicopy
#
# Purpose:
#   Macro to apply mbcopy to all files referenced through
#   a datalist, using the MB-System file suffix convention
#   to name the output files.
#
# Basic Usage:
#   mbm_multicopy -Foutputformat -Idatalist [-H -V -C -T]
#
# Authors:
#   Christian Ferreira
#   MARUM - Center for Marine Environmental Sciences
#   Klagenfurter Str. 2, GEO Building
#   28359, Bremen, Germany
#   July 17, 2011
#
#   David W. Caress
#   Lamont-Doherty Earth Observatory
#   Palisades, NY  10964
#   January 17, 2003
#
# Version:
#   $Id: mbm_multiprocess.pl 19XX 2011-07-17 15:15:00Z ferreira $
#
# Revisions:
#
#  Revision 1.0		2011-07-17 15:15:00	cferreira
#  Initial version with full implementation of mbcopy flags
#

$program_name = "mbm_multicopy";

# Deal with command line arguments
$command_line = "@ARGV";
&MBGetopts('CcF:f:HhI:i:X:x:TtVv');
$oformat = 		($opt_F || $opt_f);
$help =    		($opt_H || $opt_h);
$datalist =    		($opt_I || $opt_i);
$check =    		($opt_C || $opt_c);
$test =    		($opt_T || $opt_t);
$verbose = 		($opt_V || $opt_v);
$nprocesses =        	($opt_X || $opt_x);

# print out help message if required
if ($help)
	{
	print "\n$program_name:\n";
	print "\nVersion: $Id: mbm_multicopy.pl 1891 2011-05-04 23:46:30Z caress $\n";
	print "\nMacro to apply mbcopy to all files referenced through\n";
	print "a datalist, using the MB-System file suffix convention\n";
	print "to name the output files, and using parallel processes.\n";
	print "\nBasic Usage: \n";
	print "\t$program_name -Foutputformat -Idatalist [-H -V -T]\n";
	exit 0;
	}

# check for input file
if (!$datalist)
	{
	print "\a";
	die "No input datalist specified - $program_name aborted\n";
	}
elsif (! -e $datalist)
	{
	print "\a";
	die "Specified input datalist cannot be opened - $program_name aborted\n";
	}

# tell the world we got started
if ($verbose)
	{
	print "\nRunning $program_name...\n";
	}

# loop over all files referenced in datalist
if ($verbose)
	{
	print "Getting file list using mbdatalist...\n";
	}
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
if ($verbose)
	{
	print "Initializing parallel processing with as many as $nprocesses processes\n\n";
	}
my $pm=new Parallel::ForkManager($nprocesses);

# copy each of the files using mbcopy
$cnt = 0;
foreach $file_mb (@files_data)
	{
	my $pid = $pm->start and next;

	# get file root
	$line = `mbformat -K -I$file_mb`;
	($root,$format_mb) = $line =~ /(\S+)\s+(\S+)/;
	$iformat = $formats[$cnt];
	if ($oformat)
		{
		$outformat = $oformat;
		}
	else
		{
		$outformat = $format_mb;
		}
	$ofile_mb = "$root.mb$outformat";
	if ($ofile_mb eq $file_mb)
		{
		$root = $root . "c";
		$ofile_mb = "$root.mb$oformat";
		}

	# copy to output format using mbcopy
	if (!$check || !(-e $ofile_mb))
		{
		$mbcopy = "mbcopy -F$iformat/$outformat -I$file_mb -O$root.mb$outformat";
		if (!$test)
			{
			print "Running: $mbcopy\n";
			`$mbcopy`;
			}
		else
			{
			print "Testing: $mbcopy\n";
			}
		}

	# increment counter
	$cnt++;
	$pm->finish;
	}

$pm->wait_all_children;

# exit
exit 0;

#-----------------------------------------------------------------------
# This version of Getopts has been augmented to support multiple
# calls to the same option. If an arg in argumentative is followed
# by "+" rather than ":",  then the corresponding scalar will
# be concatenated rather than overwritten by multiple calls to
# the same arg.
#
# Usage:
#      do Getopts('a:b+c'); # -a takes arg, -b concatenates args,
#			    # -c does not take arg. Sets opt_* as a
#                           # side effect.

sub MBGetopts {
    local($argumentative) = @_;
    local(@args,$_,$first,$rest);
    local($errs) = 0;

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
		eval "\$flg_$first = 1;";
	    }
	    elsif($args[$pos+1] eq '+') {
		shift(@ARGV);
		if($rest eq '') {
		    ++$errs unless @ARGV;
		    $rest = shift(@ARGV);
		}
		if (eval "\$opt_$first") {
		    eval "\$opt_$first = \$opt_$first
				. \":\" . \$rest;";
		}
		else {
		    eval "\$opt_$first = \$rest;";
		}
		eval "\$flg_$first = 1;";
	    }
	    elsif($args[$pos+1] eq '%') {
		shift(@ARGV);
		if($rest ne '') {
		    eval "\$opt_$first = \$rest;";
		}
		else {
		    $rest = $ARGV[0];
		    ($one) = $rest =~ /^-(.).*/;
		    $pos = index($argumentative,$one);
		    if(!$one || $pos < $[) {
			eval "\$opt_$first = \$rest;";
			shift(@ARGV);
		    }
		}
		eval "\$flg_$first = 1;";
	    }
	    else {
		eval "\$opt_$first = 1";
		eval "\$flg_$first = 1;";
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
#-----------------------------------------------------------------------
