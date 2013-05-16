eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
                    & eval 'exec perl -S $0 $argv:q'
                         if 0;
#--------------------------------------------------------------------
#    The MB-system:	mbm_makedatalist.perl	27/1/2006
#    $Id$
#
#    Copyright (c) 2006-2012 by
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
#   mbm_makedatalist
#
# Purpose:
#   Macro to generate an MB-System datalist file referencing all
#   identifiable swath files in the specified directory. If no directory
#   is specified with the -I option, then the current directory is used.
#   The resulting datalist will be named datalist.mb-1 by default.
#
# Basic Usage:
#   mbm_makedatalist [-Idirectory -Odatalist -H -V]
#
# Author:
#   David W. Caress
#   Monterey Bay Aquarium Research Institute
#   Moss Landing, CA 95039
#   January 27, 2006
#
# Version:
#   $Id$
#
# Revisions:
#   $Log: mbm_makedatalist.perl,v $
#   Revision 5.1  2006/04/11 19:12:06  caress
#   Macro now handles data in remote directories.
#
#   Revision 5.0  2006/02/03 21:06:18  caress
#   Initial version of macro to generate a datalist of all recognizable swath files in a directory.
#
#
#
$program_name = "mbm_makedatalist";

# Deal with command line arguments
$command_line = "@ARGV";
&MBGetopts('HhI:i:O:o:Vv');

$help =    		($opt_H || $opt_h);
$directory = 		($opt_I || $opt_i || ".");
$datalist =    		($opt_O || $opt_o || "datalist.mb-1");
$verbose = 		($opt_V || $opt_v);

# print out help message if required
if ($help)
	{
	print "\n$program_name:\n";
	print "\nVersion: $Id$\n";
        print "Macro to generate an MB-System datalist file referencing all\n";
        print "identifiable swath files in the specified directory. If no directory\n";
        print "is specified with the -I option, then the current directory is used.\n";
        print "The resulting datalist will be named datalist.mb-1 by default.\n";

        print "\nBasic Usage:\n\tmbm_makedatalist [-Idirectory -Odatalist -H -V]\n";
	exit 0;
	}

# tell the world we got started
if ($verbose)
	{
	print "\nRunning $program_name...\n";
	}

# get list of files in specified directory
@filelist = `ls -1 $directory`;

# loop over the list of files counting identifiable swath files
$count = 0;
foreach $file (@filelist)
	{
	chop $file;
	$format = `mbformat -L -I $file`;
	chop $format;
	if ($format != 0 && ($file ne $datalist || $directory ne "."))
		{
		$count++;
		if ($directory ne ".")
			{
			$path = "$directory/$file";
			}
		else
			{
			$path = $file;
			}
		push(@outfilelist, $path);
		push(@outformatlist, $format);
		}
	}

# open output datalist
if ($count > 0)
	{
	if (!open(DATALIST,">$datalist"))
		{
		die "\n$program_name:\nUnable to output output datalist file $datalist\nExiting...\n";
		}

	# loop over the list of files
	for ($i = 0; $i < scalar(@outfilelist); $i++)
		{
		print DATALIST "$outfilelist[$i] $outformatlist[$i]\n";
		$count++;
		}

	# close datalist
	close DATALIST;
	}

# done
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
