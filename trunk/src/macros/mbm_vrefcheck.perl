#! /usr/local/bin/perl 
#--------------------------------------------------------------------
#    The MB-system:	mbm_vrefcheck.perl	6/18/93
#    $Id: mbm_vrefcheck.perl,v 4.0 1994-03-05 23:52:40 caress Exp $
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
#   mbm_vrefcheck
#
# Purpose:
#   Perl shellscript to generate plot of crosstrack seafloor slope
#   from a multibeam file.  The noise in this time series largely
#   reflects noise in the vertical reference used by the sonar.
#
# Usage:
#   mbm_vrefcheck -Fformat -Ifile [-Axmin -Bxmax -Sxscale -Xxaxis]
#
# Author:
#   David W. Caress
#   Lamont-Doherty Earth Observatory
#   Palisades, NY  10964
#   June 13, 1993
#
# Version:
#   $Id: mbm_vrefcheck.perl,v 4.0 1994-03-05 23:52:40 caress Exp $
#
# Revisions:
#   $Log: not supported by cvs2svn $
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
&Getopts('I:i:F:f:S:s:X:x:A:a:B:b:');
$file = ($opt_I || $opt_i);
$format = ($opt_F || $opt_f);
$xscale = ($opt_S || $opt_s);
$xaxis = ($opt_X || $opt_x);
$xmin = ($opt_A || $opt_a);
$xmax = ($opt_B || $opt_b);

# if needed set defaults
if (!$format) 
	{
	$format = "24";
	}
if (!$xscale) 
	{
	$xscale = "1.2";
	}
$file || die "Input file not specified...\nMBM_VREFCHECK aborted.\n";

# Get filenames in order...
$datfile = "vrefcheck_$$.dat";
$fltfile = "vrefcheck_$$.flt";
$resfile = "vrefcheck_$$.res";
$psfile = "vrefcheck_$$.ps";
$rasfile = "vrefcheck_$$.rast";
$rtlfile = "vrefcheck_$$.rtl";

# Get going.
print "\nMacro mbm_vrefcheck running...\n";
print "Running mblist...\n";
`mblist -F$format -I$file -OmA > $datfile`;

# Filter the seafloor slope data.
print "Running filter1d...\n";
`filter1d $datfile -Fm5 -E -V > $fltfile`;

# Read the data and filtered data files, 
# subtracting to get the high-passed signal.
print "Processing raw and filtered data to get residual...\n";
open(F1,"$datfile");
open(F2,"$fltfile");
open(FOUT,"> $resfile");
$min_in_hour = "60.0";
while ($line1 = <F1>)
	{
	($time1, $dat) = $line1 =~ /(\S+)\s(\S+)/;
#	print "time1:",$time1," dat:",$dat,"\n";
	$line2 = <F2>;
	($time2, $flt) = $line2 =~ /(\S+)\s(\S+)/;
	$res = $dat - $flt;
	$hour = $time1 / $min_in_hour;
#	print "time1:",$time1," dat:",$dat," time2:",$time2," flt:",$flt," res:",$res,"\n";
	print FOUT $hour,"\t",$res,"\n";
	}
close(F1);
close(F2);
close(FOUT);

# Get x limits of plot and page.
if (!$xmin || !$xmax)
	{
	print "Running minmax...\n";
	$minmax = `minmax $resfile`;
	($resfile, $n, $xmin, $xmax, $ymin, $ymax) = $minmax 
		=~ /(\S+): N = (\S+)\s<(\S+)\/(\S+)>\s<(\S+)\/(\S+)>/;
	}
if (!$xaxis)
	{
	$xaxis = ($xmax - $xmin) * $xscale;
	}
$pagex = $xaxis + 2.0;

# Generate the postscript plot.
print "Running psxy...\n";
`psxy $resfile -JX$xaxis/9 -R$xmin/$xmax/-2/2 -B1g0.25:"Time From Beginning of File (hours)":/0.25g0.25:"Apparent Vertical Reference Noise (degrees)"::."Multibeam Data File - $file": -P > $psfile`;
`rm -f $datfile $fltfile $resfile`;

# Translate the postscript plot into a Sun rasterfile.
print "Running ps2ras...\n";
`ps2ras -w $pagex -h 11.5 -d 1 -p 300 $psfile $rasfile`;
`rm -f $psfile`;

# Translate the rasterfile into an rtl file for the Novajet plotter.
print "Running image alchemy...\n";
`alchemy $rasfile $rtlfile --r10`;
`rm -f $rasfile`;

# Shove the mess at the plotter.
print "Sending image to plotter...";
`cat $rtlfile > /dev/bpp0`;
`rm -f $rtlfile`;

#`pageview $psfile &`;

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
