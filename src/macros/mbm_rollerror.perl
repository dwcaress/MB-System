eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
                    & eval 'exec perl -S $0 $argv:q'
                         if 0;
#--------------------------------------------------------------------
#    The MB-system:	mbm_rollover.perl	6/18/93
#    $Id: mbm_rollerror.perl,v 5.2 2003-04-17 20:42:48 caress Exp $
#
#    Copyright (c) 1993, 1994, 2000, 2003 by 
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
#   mbm_rollover
#
# Purpose:
#   Perl shellscript to read a multibeam bathymetry file, calculate the
#   noise in the vertical reference used by the sonar, and generate
#   a file containing roll corrections which can be applied to the
#   data.
#
# Usage:
#   mbm_rollover -Fformat -Wfilterwidth -Ifile
#
# Author:
#   David W. Caress
#   Lamont-Doherty Earth Observatory
#   Palisades, NY  10964
#   June 13, 1993
#
# Version:
#   $Id: mbm_rollerror.perl,v 5.2 2003-04-17 20:42:48 caress Exp $
#
# Revisions:
#   $Log: not supported by cvs2svn $
#   Revision 5.1  2001/03/22 21:05:45  caress
#   Trying to make release 5.0.beta0
#
# Revision 5.0  2000/12/01  22:58:01  caress
# First cut at Version 5.0.
#
# Revision 4.6  2000/10/03  21:42:17  caress
# Snapshot for Dale.
#
# Revision 4.5  1997/04/21  16:54:41  caress
# MB-System 4.5 Beta Release.
#
# Revision 4.4  1995/09/28  18:05:43  caress
# Various bug fixes working toward release 4.3.
#
# Revision 4.4  1995/09/28  18:05:43  caress
# Various bug fixes working toward release 4.3.
#
# Revision 4.3  1995/08/17  14:52:53  caress
# Revision for release 4.3.
#
# Revision 4.2  1995/05/12  17:43:23  caress
# Made exit status values consistent with Unix convention.
# 0: ok  nonzero: error
#
# Revision 4.1  1994/10/21  11:36:58  caress
# Release V4.0
#
# Revision 4.0  1994/03/05  23:52:40  caress
# First cut at version 4.0
#
# Revision 4.1  1994/03/03  04:11:13  caress
# Fixed copyright message.
#
# Revision 4.0  1994/02/26  19:37:57  caress
# First cut at new version.
#
# Revision 3.3  1993/08/17  16:58:36  caress
# Set location of perl to /usr/local/bin/perl
#
# Revision 3.2  1993/08/17  01:52:17  caress
# Added MB-system header.
#
#
# Deal with command line arguments
&Getopts('I:i:F:f:W:w:');
$file =        ($opt_I || $opt_i);
$format =      ($opt_F || $opt_f);
$filterwidth = ($opt_W || $opt_w);

# get format if needed
if (!$format) 
	{
	$line = `mbformat -I $file -L`;
	($format) = $line =~ /(\S+)/;
	if ($format == 0)
		{
		$format = -1;
		}
	}

# set filterwidth if needed
if (!$filterwidth)
	{
	$filterwidth = 60;
	}

print "file: ",$file,"\n";
print "format: ",$format,"\n";
$file || die "Input file not specified...\nMBM_ROLLERROR aborted.\n";

# Get filenames in order...
$datfile = "mbm_rollerror$$.dat";
$fltfile = "mbm_rollerror$$.flt";
$corfile = "$file.cor";

# Get going.
print "Macro mbm_rollerror running...\n\n";
print "Running mblist...\n";
`mblist -F$format -I$file -OMA > $datfile`;

# Filter the seafloor slope data.
print "Running filter1d...\n";
`filter1d $datfile -Fc$filterwidth -E -V > $fltfile`;

# Read the data and filtered data files, 
# subtracting to get the high-passed signal.
print "Processing raw and filtered data to get residual...\n";
open(F1,"$datfile");
open(F2,"$fltfile");
open(FOUT,"> $corfile");
while ($line1 = <F1>)
	{
	($time1, $dat) = $line1 =~ /(\S+)\s(\S+)/;
	$line2 = <F2>;
	($time2, $flt) = $line2 =~ /(\S+)\s(\S+)/;
	$res = $dat - $flt;
	print FOUT $time1,"\t",$res,"\n";
	}
close(F1);
close(F2);
close(FOUT);

# remove excess files
`rm -f $datfile $fltfile`;

# Announce success whether it is deserved or not.
print "All done!\n";
exit 0;

#-----------------------------------------------------------------------
# This should be loaded from the library but the shipboard installation
# of Perl is screwed up so....
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

1;
