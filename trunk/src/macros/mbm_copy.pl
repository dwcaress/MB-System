eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
                    & eval 'exec perl -S $0 $argv:q'
                         if 0;
#--------------------------------------------------------------------
#    The MB-system:	mbm_plot.perl	1/17/2003
#    $Id$
#
#    Copyright (c) 2003-2009 by 
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
#   mbm_copy
#
# Purpose:
#   Macro to apply mbcopy to all files referenced through
#   a datalist, using the MB-System file suffix convention
#   to name the output files. 
#
# Basic Usage: 
#   mbm_copy -Foutputformat -Idatalist [-H -V -C -T]
#
# Author:
#   David W. Caress
#   Lamont-Doherty Earth Observatory
#   Palisades, NY  10964
#   January 17, 2003
#
# Version:
#   $Id$
#
# Revisions:
#   $Log: mbm_copy.perl,v $
#   Revision 5.2  2006/09/11 18:55:52  caress
#   Changes during Western Flyer and Thomas Thompson cruises, August-September
#   2006.
#
#   Revision 5.1  2005/11/05 01:34:20  caress
#   Much work over the past two months.
#
#   Revision 5.0  2003/03/10 19:54:43  caress
#   Initial version.
#
#
#
$program_name = "mbm_copy";

# Deal with command line arguments
$command_line = "@ARGV";
&MBGetopts('CcF:f:HhI:i:TtVv');
$oformat = 		($opt_F || $opt_f);
$help =    		($opt_H || $opt_h);
$datalist =    		($opt_I || $opt_i);
$check =    		($opt_C || $opt_c);
$test =    		($opt_T || $opt_t);
$verbose = 		($opt_V || $opt_v);

# print out help message if required
if ($help)
	{
	print "\n$program_name:\n";
	print "\nVersion: $Id$\n";
	print "\nMacro to apply mbcopy to all files referenced through\n";
	print "a datalist, using the MB-System file suffix convention\n";
	print "to name the output files.\n";
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
	print "\nGetting file list using mbdatalist...\n\n";
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

# copy each of the files using mbcopy
$cnt = 0;
foreach $file_mb (@files_data)
	{
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
	}

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
