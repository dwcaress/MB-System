#! /usr/local/bin/perl 
#--------------------------------------------------------------------
#    The MB-system:	mbm_calcomp_plot.perl	8/21/93
#    $Id: mbm_calcomp_plot.perl,v 4.0 1994-03-06 00:09:08 caress Exp $
#
#    Copyright (c) 1993, 1994 by 
#    D. W. Caress (caress@lamont.ldgo.columbia.edu)
#    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
#    Lamont-Doherty Earth Observatory
#    Palisades, NY  10964
#
#    See README file for copying and redistribution conditions.
#--------------------------------------------------------------------
#
# Command:
#   mbm_calcomp_plot
#
# Purpose:
#   Perl shellscript to generate "realtime" contour plot of Hydrosweep
#   bathymetry on Calcomp 965 pen plotters used on the R/V Maurice
#   Ewing.  This shellscript ties together a number of different
#   utilities to create a primitive, if servicable, realtime
#   plotting capability on the Ewing.
#
# Usage:
#   mbm_calcomp_plot -Ifile -Rwest/east/south/north -Sscale
#                    [-Byear/month/day/hour/min/sec
#                     -Eyear/month/day/hour/min/sec
#                     -N -Q -Fformat -Ttitle -Pport -H]
#
# Author:
#   David W. Caress
#   Lamont-Doherty Earth Observatory
#   Palisades, NY  10964
#   June 13, 1993
#
# A Bit of History:
#   The first L-DGO realtime Hydrosweep plotting capability
#   (independent of the VAX-based software operated by URI
#   on the ship through 1992) was kludged together in 48 hours
#   during EW9103 (May 1991) by David Caress from an early version of
#   the MB-System code.  As part of the implementation of MB-System 
#   Version 3 on the Ewing during EW9302 (June 1993), Suzanne O'Hara built
#   a reasonable interface for the realtime plotting using a csh 
#   script called mbm_realtime_plot.csh.  During EW9305 (August-September
#   1993), David Caress rewrote the basic realtime plotting programs
#   in order to incorporate the current version of MBIO and to
#   allow the code to plot shiptracks as well as contoured bathymetry.
#   Some minor bugs in the interface shellscript were also noted,
#   consequently David Caress also rewrote the interface shellscript 
#   in Perl (this script).
#
# Version:
#   $Id: mbm_calcomp_plot.perl,v 4.0 1994-03-06 00:09:08 caress Exp $
#
# Revisions:
#   $Log: not supported by cvs2svn $
# Revision 4.0  1994/03/01  20:54:10  caress
# First cut at new version.
#
# Revision 3.1  1993/08/27  22:04:49  caress
# Added contour and shiptrack annotation control.
#
# Revision 3.0  1993/08/26  01:09:11  caress
# Initial version.
#
#
# Deal with command line arguments
&Getopts('A:a:B:b:C:c:E:e:F:f:I:i:NnP:p:QqR:r:S:s:T:t:Hh');
$annotate = ($opt_A || $opt_a || "0.25/1.0/4.0");
$begin_time = ($opt_B || $opt_b || "1962/2/21/0/0/0");
$contour = ($opt_C || $opt_c || "50/200/200/200/.02/.07");
$end_time = ($opt_E || $opt_e || "2062/2/21/0/0/0");
$format = ($opt_F || $opt_f || 5);
$help = ($opt_H || $opt_h);
$file = ($opt_I || $opt_i);
$restart = ($opt_N || $opt_n);
$port = ($opt_P || $opt_p || "/dev/ttyb");
$norealtime = ($opt_Q || $opt_q);
$bounds = ($opt_R || $opt_r);
$scale = ($opt_S || $opt_s || 4.0);
$title = ($opt_T || $opt_t);
#
# Do help
if ($help) 
	{
	print STDERR "\nmbm_calcomp_plot:\n";
	print STDERR "  Perl shellscript to generate realtime \n";
	print STDERR "  contour plot of Hydrosweep bathymetry on\n";
	print STDERR "  Calcomp 965 pen plotters used on the\n";
	print STDERR "  R/V Maurice Ewing.  This shellscript ties\n";
	print STDERR "  together a number of different utilities\n";
	print STDERR "  to create a primitive, if servicable,\n";
	print STDERR "  realtime plotting capability on the Ewing.\n\n";
	print STDERR "Usage:\n";
	print STDERR "  mbm_calcomp_plot -Ifile -Rwest/east/south/north\n";
	print STDERR "        -Sscale [-Fformat -Ttitle -Pport -N -Q -H]\n";
	die "\n";
	}
#
# Figure out how to spew data to mbcontourfilter 
# (if realtime use tail +0f, if not realtime use cat)
if ($norealtime)
	{
	$source = "cat";
	}
else
	{
	$source = "tail +0f";
	}
#
# Extract raw bounds values
($west,$east,$south,$north) = $bounds =~ /(\S+)\/(\S+)\/(\S+)\/(\S+)/;
($westd,$westm,$eastd,$eastm,$southd,$southm,$northd,$northm) 
	= $bounds =~ /(\S+):(\S+)\/(\S+):(\S+)\/(\S+):(\S+)\/(\S+):(\S+)/;

# if bounds defined with decimal degrees 
# convert to whole degrees and minutes
if (!($westd && $westm && $eastd && $eastm 
	&& $southd && $southm && $northd && $northm))
	{
	if ($west < 0)
		{
		$westd = int($west);
		$westm = int($west*60. - $westd*60. - 0.5);
		$westm = -$westm;
		}
	else
		{
		$westd = int($west);
		$westm = int($west*60. - $westd*60. + 0.5);
		}
	if ($east < 0)
		{
		$eastd = int($east);
		$eastm = int($east*60. - $eastd*60. - 0.5);
		$eastm = -$eastm;
		}
	else
		{
		$eastd = int($east);
		$eastm = int($east*60. - $eastd*60. + 0.5);
		}
	if ($south < 0)
		{
		$southd = int($south);
		$southm = int($south*60. - $southd*60. - 0.5);
		$southm = -$southm;
		}
	else
		{
		$southd = int($south);
		$southm = int($south*60. - $southd*60. + 0.5);
		}
	if ($north < 0)
		{
		$northd = int($north);
		$northm = int($north*60. - $northd*60. - 0.5);
		$northm = -$northm;
		}
	else
		{
		$northd = int($north);
		$northm = int($north*60. - $northd*60. + 0.5);
		}
	}

# Now convert whole degrees and minutes to decimal degrees
# so the two measures of the bounds are truly equivalent
if ($westd < 0)
	{
	$west = $westd - $westm/60.;
	}
else
	{
	$west = $westd + $westm/60.;
	}
if ($eastd < 0)
	{
	$east = $eastd - $eastm/60.;
	}
else
	{
	$east = $eastd + $eastm/60.;
	}
if ($southd < 0)
	{
	$south = $southd - $southm/60.;
	}
else
	{
	$south = $southd + $southm/60.;
	}
if ($northd < 0)
	{
	$north = $northd - $northm/60.;
	}
else
	{
	$north = $northd + $northm/60.;
	}
$bounds = sprintf("%.4f/%.4f/%.4f/%.4f",$west,$east,$south,$north);

# Figure out size of plot and how it should be placed on the Calcomp
$PI = 3.14.15926;
$DTR = $PI/180.;
$C1 = 111412.84;
$C2 = -93.5;
$C3 = 0.118;
$C4 = 111132.92;
$C5 = -559.82;
$C6 = 1.175;
$C7 = 0.0023;
$center_lat = 0.5*($south + $north);
if ($center_lat < 0.0)
	{
	$center_lat = -$center_lat;
	}
$km_per_deg_lon = $C1*cos($DTR*$center_lat)
	+ $C2*cos(3.0*$DTR*$center_lat)
	+ $C3*cos(5.0*$DTR*$center_lat);
$km_per_deg_lat = $C4 
	+ $C5*cos(2.0*$DTR*$center_lat) 
	+ $C6*cos(4.0*$DTR*$center_lat)
	+ $C7*cos(6.0*$DTR*$center_lat);
$width = sprintf("%.4f",($scale*($east - $west)));
$inch_lon = $scale*($east - $west) + 4.0;
$inch_lat = $scale*($north - $south)*$km_per_deg_lat/$km_per_deg_lon 
		+ 4.0;

print "\n";
print "MB-System Macro MBM_CALCOMP_PLOT:\n";
print "Real-time pen plots of Hydrosweep bathymetry on the R/V Ewing\n\n";
print "The desired plot has the following bounds:\n";
printf "  Longitude: %10.5f to %10.5f  or  %4d:%2.2d to %4d:%2.2d\n",
	$west, $east, $westd, $westm, $eastd, $eastm;
printf "  Longitude: %10.5f to %10.5f  or  %4d:%2.2d to %4d:%2.2d\n",
	$south, $north, $southd, $southm, $northd, $northm;
print "\nThe size of this plot is:\n";
printf "  Longitude: %8.3f inches\n", $inch_lon;
printf "  Latitude: %8.3f inches\n\n", $inch_lat;

if ($restart)
	{
	print "Restarting or continuing an existing plot.\n";
	print "To set up the plotter, please do the following:\n";
	print "\t1) Press <MANUAL>\n\t2) Press <NEW PLOT>, <TO>,\n";
	print "\t\tthen <NEW PLOT>, <ENTER>\n";
	print "\t3) Then put plotter on <AUTO>\n\n";
	}

elsif (($inch_lon > 34.0 && $inch_lat > 34.0) 
	|| $inch_lon > 55.0 || $inch_lat > 55.0)
	{
	print "WARNING:  The largest plot which will fit on the Ewings\n";
	print "Calcomp 965 plotters is 34 inches by 55 inches.\n";
	print "This plot will not fit on these plotters.\n\n";
	die "MB System Macro MBM_CALCOMP_PLOT terminated\n\n";
	}
elsif ($inch_lon > 34.0)
	{
	print "The longitude length is too long for the plot width.\n";
	print "The latitude will be along the width.  To set up the\n";
	print "plotter, please do the following:\n";
	print "\t1) Press <MANUAL>\n\t2) Put paper on the plotter\n";
	print "\t3) Move cartridge to the lower right-hand side of\n";
	print "\t\tthe paper, leaving room for annotations.\n";
	print "\t4) Press <PLOTTER SETUP>, then <1>,<ENTER>,<1>,<ENTER>,<0>,<ENTER>,\n";
	print "\t\tthen <NEW PLOT>, <ENTER>\n";
	print "\t5) Then put plotter on <AUTO>\n\n";
	}
else
	{
	print "The longitude will be along the width.  To set up the\n";
	print "plotter, please do the following:\n";
	print "\t1) Press <MANUAL>\n\t2) Put paper on the plotter\n";
	print "\t3) Move cartridge to the lower left-hand side of\n";
	print "\t\tthe paper, leaving room for annotations.\n";
	print "\t4) Press PLOTTER SETUP, then <1>,<ENTER>,<1>,<ENTER>,<270>,<ENTER>,\n";
	print "\t\tthen <NEW PLOT>, <ENTER>\n";
	print "\t5) Then put plotter on <AUTO>\n\n";
	}
print "Hit <Return> to continue or <q> to quit\n";
$answer = <STDIN>;
if ($answer =~ /^q(.*)/ || $answer =~ /^Q(.*)/)
	{
	print "You killed me, you merciless thug...\n";
	die "\n";
	}

# Generate the restart shellscript
if (!$restart)
	{
	print "Creating restart script restart_calcomp_plot.cmd - use\n";
	print "this script to restart plot if necessary.\n\n";
	$restartfile = "restart_calcomp_plot.cmd";
	open(FILE,"> $restartfile");
	$command = sprintf("/net/heezen/packages/MB-System3/src/realtime/mbm_calcomp_plot.perl -F%s -I%s -R%s -B%s -E%s -S%s -C%s -A%s -P%s -N",
		$format,$file,$bounds,$begin_time,$end_time,$scale,
		$contour,$annotate,$port);

	print FILE "#\n";
	print FILE "#\n# Name: restart_calcomp_plot.cmd\n";
	print FILE "# This script was created by mbm_calcomp_plot\n";
	print FILE "$command\n";
	close FILE;
	chmod 0775, $restartfile;
	}

# Plot the axes if needed
if (!$restart)
	{
	print "Drawing axes...\n";
	$command = sprintf("p_sheet -g %d,%d %d,%d %d,%d %d,%d -gd 1,1,0 -gm 10,1,10 -s %s -t \"%s\" | ccgr -d %s",$westd,$westm,$eastd,$eastm,
		$southd,$southm,$northd,$northm,$scale,$title,$port);
	print "$command\n";
	`$command`;
	print "\nTo reset the plotter, please do the following:\n";
	print "\t1) Press <MANUAL>\n\t2) Press <NEW PLOT>, <TO>,\n";
	print "\t\tthen <NEW PLOT>, <ENTER>\n";
	print "\t3) Then put plotter on <AUTO>\n\n";
	print "Hit <Return> to continue or <q> to quit\n";
	$answer = <STDIN>;
	if ($answer =~ /^q(.*)/ || $answer =~ /^Q(.*)/)
		{
		print "You killed me, you murderous pathogen...\n";
		die "\n";
		}
	}

# Now, finally, plot the contours...
print "\nDrawing contours...\n";
$command = sprintf("%s %s | mbcontourfilter -L0 -F%s -R%s -B%s -E%s -V -J%s -C%s -A%s | mbplotfilter -R%s -V -W%s | ccgr -d %s",
	$source, $file, $format, $bounds, $begin_time, $end_time,
	$scale, $contour, $annotate, $bounds, $width, $port);
	print "$command\n";
	`$command`;

# Announce success whether it is deserved or not.
print "All done!\n";


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
