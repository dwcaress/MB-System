eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
                    & eval 'exec perl -S $0 $argv:q'
                         if 0;
#--------------------------------------------------------------------
#    The MB-system:	mbm_arc2grd.perl	4/21/01
#    $Id: mbm_arc2grd.perl,v 5.0 2001-04-23 21:22:45 caress Exp $
#
#    Copyright (c) 2001 by
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
#   mbm_arc2grd
#
# Purpose:
#   Macro to convert a ArcView ASCII grid to an GMT grid file in the 
#   GMT NetCDF grid format. This allows users to import the grid
#   into GMT. The grid will have the same grid interval in both 
#   longitude and latitude.
#
# Basic Usage:
#   mbm_arc2grd -Iarcfile -Ogrdfile [-H -V]
#
# Author:
#   David W. Caress
#   Monterey Bay Aquarium Research Institute
#   Moss Landing, CA 95039
#   April 21, 2001
#   (at sea on the R/V Western Flyer about
#    10 km off the windward side of Oahu)
#
# Version:
#   $Id: mbm_arc2grd.perl,v 5.0 2001-04-23 21:22:45 caress Exp $
#
# Revisions:
#   $Log: not supported by cvs2svn $
#
#
#

$program_name = "mbm_arc2grd";

# Deal with command line arguments
&Getopts('I:i:N:n:O:o:VvHh');
$ifile =    ($opt_I || $opt_i);
$ofile =    ($opt_O || $opt_o);
$help =     ($opt_H || $opt_h);
$verbose =  ($opt_V || $opt_v);

#--------------------------------------------------------------------

# print out help message if required
if ($help)
	{
	print "\n$program_name:\n";
	print "\nMacro to convert a ArcView ASCII grid to an GMT grid file in the \n";
	print "GMT NetCDF grid format. This allows users to import the grid\n";
	print "into GMT. The grid will have the same grid interval in both \n";
	print "longitude and latitude.\n";
	print "\nBasic Usage: \n";
	print "\t$program_name -Igrdfile -Oarcfile [-H -Nnodata -V]\n";
	exit 0;
	}

# check for input file
if (!$ifile)
	{
	print "\a";
	die "No input file specified\nMacro $program_name aborted\n";
	}
elsif (!open(INP,"<$ifile"))
	{
	print "\a";
	die "Specified input file $ifile cannot be opened\nMacro $program_name aborted\n";
	}
$tmpfile = $program_name . "_tmp_" . "$PID";
if (!open(TMP,">$tmpfile"))
	{
	print "\a";
	die "Cannot open temporary file $tmpfile\nMacro $program_name aborted.\n";
	}
if (!$ofile)
	{
	print "\a";
	die "No output file specified\nMacro $program_name aborted\n";
	}

# read header of ascii Arc grid file
$cnt = 0;
while (<INP>) {
	$cnt++;
	if ($cnt == 1) {
		($ncols) = $_ =~ /ncols\s+(\S+)/;
	}
	elsif ($cnt == 2) {
		($nrows) = $_ =~ /nrows\s+(\S+)/;
	}
	elsif ($cnt == 3) {
		($xllcorner) = $_ =~ /xllcorner\s+(\S+)/;
	}
	elsif ($cnt == 4) {
		($yllcorner) = $_ =~ /yllcorner\s+(\S+)/;
	}
	elsif ($cnt == 5) {
		($cellsize) = $_ =~ /cellsize\s+(\S+)/;
	}
	elsif ($cnt == 6) {
		($nodata) = $_ =~ /nodata_value\s+(\S+)/;
		$xmin_f = $xllcorner;
		$xmax_f = $xllcorner + $cellsize * ($ncols - 1);
		$ymin_f = $yllcorner;
		$ymax_f = $yllcorner + $cellsize * ($nrows - 1);
	}
	elsif ($cnt > 6) {
		chop($_);
		@values = split(/ /, $_);
		foreach $value (@values) {
			$ndata++;
			print TMP "$value\n";
		}
	}
}
close(INP);
close(TMP);

# tell the world we got started
if ($verbose) 
	{
	print "\nProgram $program_name status:\n";
	print "\tInput ArcView ASCII  file: $ifile\n";
	print "\tOutput GRD file:           $ofile\n";
	print "\tGrid dimensions:  $ncols  $nrows\n";
	print "\tGrid cell sizes:  $cellsize  $cellsize\n";
	print "\tGrid bounds:      $xmin_f  $xmax_f    $ymin_f  $ymax_f\n";
	}

# run xyz2grd
$cmd = "xyz2grd $tmpfile -G$ofile -H6  -I$cellsize/$cellsize -R$xmin_f/$xmax_f/$ymin_f/$ymax_f -N$nodata -ZTLa -V";
if ($verbose)
	{
	print "\nRunning xyz2grd...\n$cmd\n";
	}
`$cmd`;
`rm -f $tmpfile`;

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
