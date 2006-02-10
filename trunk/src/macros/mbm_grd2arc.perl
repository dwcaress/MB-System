eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
                    & eval 'exec perl -S $0 $argv:q'
                         if 0;
#--------------------------------------------------------------------
#    The MB-system:	mbm_grd2arc.perl	6/11/99
#    $Id: mbm_grd2arc.perl,v 5.3 2006-02-10 01:27:40 caress Exp $
#
#    Copyright (c) 1999, 2000, 2003 by
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
#   mbm_grd2arc
#
# Purpose:
#   Macro to convert a GMT grid file in the GMT NetCDF grid format 
#   to an ArcView ASCII grid. This allows users to import the grid
#   into Arc/Info and ArcView. The grids must have the same grid 
#   interval in both longitude and latitude. If the grid was created
#   using mbgrid or mbmosaic, the -E option must have been used 
#   with both dy = 0 and "!" appended (see the mbgrid and mbmosaic
#   manual pages).
#
# Basic Usage:
#   mbm_grd2arc -Igrdfile -Oarcfile [-H -Nnodata -V]
#
# Author:
#   David W. Caress
#   Monterey Bay Aquarium Research Institute
#   Moss Landing, CA 95039
#   October 5, 1999
#
# Version:
#   $Id: mbm_grd2arc.perl,v 5.3 2006-02-10 01:27:40 caress Exp $
#
# Revisions:
#   $Log: not supported by cvs2svn $
#   Revision 5.2  2003/04/17 20:42:48  caress
#   Release 5.0.beta30
#
#   Revision 5.1  2001/06/03 06:59:24  caress
#   Release 5.0.beta01
#
#   Revision 5.0  2000/12/01 22:58:01  caress
#   First cut at Version 5.0.
#
# Revision 4.1  2000/10/03  21:42:17  caress
# Snapshot for Dale.
#
# Revision 4.0  1999/10/06  20:43:21  caress
# Initial version.
#
#
#

$program_name = "mbm_grd2arc";

# Deal with command line arguments
&Getopts('I:i:N:n:O:o:VvHh');
$ifile =    ($opt_I || $opt_i);
$ofile =    ($opt_O || $opt_o);
$nodata =   ($opt_N || $opt_n || -99999);
$help =     ($opt_H || $opt_h);
$verbose =  ($opt_V || $opt_v);

#--------------------------------------------------------------------

# print out help message if required
if ($help)
	{
	print "\n$program_name:\n";
	print "\nMacro to convert a GMT grid file in the GMT NetCDF grid format \n";
	print "to an ArcView ASCII grid. This allows users to import the grid\n";
	print "into Arc/Info and ArcView. The grids must have the same grid \n";
	print "interval in both longitude and latitude. If the grid was created\n";
	print "using mbgrid or mbmosaic, the -E option must have been used \n";
	print "with both dy = 0 and \"!\" appended (see the mbgrid and mbmosaic\n";
	print "manual pages).\n";
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
elsif (! -e $ifile)
	{
	print "\a";
	die "Specified input file $ifile cannot be opened\nMacro $program_name aborted\n";
	}
if (!$ofile)
	{
	print "\a";
	die "No output file specified\nMacro $program_name aborted\n";
	}
elsif (!open(OUT,">$ofile"))
	{
	print "\a";
	die "Cannot open output file $ofile\nMacro $program_name aborted.\n";
	}

# Save old GMT default double format and set new format
$line = `gmtdefaults -L | grep D_FORMAT`;
($dformatsave) = $line =~ /D_FORMAT\s+=\s+(\S+)/;
`gmtset D_FORMAT %.10lg`;

# Get info using grdinfo
@grdinfo = `grdinfo $ifile`;
while (@grdinfo)
	{
	$line = shift @grdinfo;
	if ($line =~ 
		/\S+\s+x_min:\s+\S+\s+x_max:\s+\S+\s+x_inc:\s+\S+\s+units:\s+.+\s+nx:\s+\S+/)
		{
		($xmin_f,$xmax_f,$xinc_f,$xunits,$xnx_d) = $line =~ 
			/\S+\s+x_min:\s+(\S+)\s+x_max:\s+(\S+)\s+x_inc:\s+(\S+)\s+units:\s+(.+)\s+nx:\s+(\S+)/;
		}
	elsif ($line =~ 
		/\S+\s+x_min:\s+\S+\s+x_max:\s+\S+\s+x_inc:\s+\S+\s+name:\s+.+\s+nx:\s+\S+/)
		{
		($xmin_f,$xmax_f,$xinc_f,$xunits,$xnx_d) = $line =~ 
			/\S+\s+x_min:\s+(\S+)\s+x_max:\s+(\S+)\s+x_inc:\s+(\S+)\s+name:\s+(.+)\s+nx:\s+(\S+)/;
		}
	elsif ($line =~ 
		/\S+\s+x_min:\s+(\S+)\s+x_max:\s+(\S+)\s+x_inc:/)
		{
		($xmin_f,$xmax_f) = $line =~ 
			/\S+\s+x_min:\s+(\S+)\s+x_max:\s+(\S+)\s+x_inc:/;
		}
	if ($line =~ /\S+\s+y_min:\s+\S+\s+y_max:\s+\S+\s+y_inc:\s+\S+\s+units:\s+.+\s+ny:\s+\S+/)
		{
		($ymin_f,$ymax_f,$yinc_f,$yunits,$yny_d) = $line =~ 
			/\S+\s+y_min:\s+(\S+)\s+y_max:\s+(\S+)\s+y_inc:\s+(\S+)\s+units:\s+(.+)\s+ny:\s+(\S+)/;
		}
	elsif ($line =~ /\S+\s+y_min:\s+\S+\s+y_max:\s+\S+\s+y_inc:\s+\S+\s+name:\s+.+\s+ny:\s+\S+/)
		{
		($ymin_f,$ymax_f,$yinc_f,$yunits,$yny_d) = $line =~ 
			/\S+\s+y_min:\s+(\S+)\s+y_max:\s+(\S+)\s+y_inc:\s+(\S+)\s+name:\s+(.+)\s+ny:\s+(\S+)/;
		}
	elsif ($line =~ /\S+\s+y_min:\s+\S+\s+y_max:\s+\S+\s+y_inc:/)
		{
		($ymin_f,$ymax_f) = $line =~ 
			/\S+\s+y_min:\s+(\S+)\s+y_max:\s+(\S+)\s+y_inc:/;
		}
	if ($line =~ /\S+\s+zmin:\s+\S+\s+zmax:\s+\S+\s+units:\s+\S+/)
		{
		($zmin_f,$zmax_f) = $line =~ 
			/\S+\s+zmin:\s+(\S+)\s+zmax:\s+(\S+)\s+units:\s+\S+/;
		}
	elsif ($line =~ /\S+\s+zmin:\s+\S+\s+zmax:\s+\S+\s+name:\s+\S+/)
		{
		($zmin_f,$zmax_f) = $line =~ 
			/\S+\s+zmin:\s+(\S+)\s+zmax:\s+(\S+)\s+name:\s+\S+/;
		}
	if ($line =~ /\S+\s+z_min:\s+\S+\s+z_max:\s+\S+\s+units:/)
		{
		($zmin_f,$zmax_f,$zunits_s) = $line =~ 
			/\S+\s+z_min:\s+(\S+)\s+z_max:\s+(\S+)\s+units:\s+(.+)/;
		}
	elsif ($line =~ /\S+\s+z_min:\s+\S+\s+z_max:\s+\S+\s+name:/)
		{
		($zmin_f,$zmax_f,$zunits_s) = $line =~ 
			/\S+\s+z_min:\s+(\S+)\s+z_max:\s+(\S+)\s+name:\s+(.+)/;
		}
	elsif ($line =~ /\S+\s+Normal node registration used/)
		{
		$node_normal = 1;
		}
	}

# reset the GMT default double format
`gmtset D_FORMAT $dformatsave`;

# deal with translating grid bounds
if ($node_normal)
	{
	$xmin_a = $xmin_f - 0.5 * $xinc_f;
	$xmax_a = $xmax_f + 0.5 * $xinc_f;
	$ymin_a = $ymin_f - 0.5 * $yinc_f;
	$ymax_a = $ymax_f + 0.5 * $yinc_f;
	}
else
	{
	$xmin_a = $xmin_f;
	$xmax_a = $xmax_f;
	$ymin_a = $ymin_f;
	$ymax_a = $ymax_f;
	}

# tell the world we got started
if ($verbose) 
	{
	print "\nProgram $program_name status:\n";
	print "\tInput GRD file:            $ifile\n";
	print "\tOutput ArcView ASCII file: $ofile\n";
	print "\tGrid dimensions:    $xnx_d  $yny_d\n";
	print "\tGrid cell sizes:    $xinc_f  $yinc_f\n";
	print "\tGrid bounds (GMT):  $xmin_f  $xmax_f    $ymin_f  $ymax_f\n";
	print "\tGrid bounds (Arc):  $xmin_a  $xmax_a    $ymin_a  $ymax_a\n";
	}

# Check for equal dx and dy
if ($xinc_f != $yinc_f)
	{
	print "\a";
	die "Grid x and y cell sizes differ: $x_inc $y_inc\nMacro $program_name aborted\n";
	}

# output header of Arcview ascii file
printf OUT "ncols $xnx_d\n";
printf OUT "nrows $yny_d\n";
printf OUT "xllcorner $xmin_a\n";
printf OUT "yllcorner $ymin_a\n";
printf OUT "cellsize $xinc_f\n";
printf OUT "nodata_value $nodata\n";

# Get data using grd2xyz
if ($verbose > 0)
	{
	print "\nGenerating temporary file...\n";
	}
$tmpfile = $program_name . "_tmp_" . "$PID";
@grd2xyz = `grd2xyz $ifile -ZTLa > $tmpfile`;
if (!open(TMP,"<$tmpfile"))
	{
	print "\a";
	die "Cannot open temporary file $tmpfilefile\nMacro $program_name aborted.\n";
	}
$cnt = 0;
$cnttot = 0;
$target = 0.1;
if ($verbose > 0)
	{
	print "Parsing temporary file...\n";
	}
while (<TMP>)
	{
	$line = $_;
	if ($line =~ /NaN/)
		{
		$cnt = $cnt + 1;
		$cnttot = $cnttot + 1;
		if ($cnt == $xnx_d)
			{
			print OUT "$nodata\n";
			$cnt = 0;
			}
		else
			{
			print OUT "$nodata ";
			}
		}
	elsif ($line =~ /(\S+)/)
		{
		$cnt = $cnt + 1;
		$cnttot = $cnttot + 1;
		($value) = $line =~ /(\S+)/;
		if ($cnt == $xnx_d)
			{
			print OUT "$value\n";
			$cnt = 0;
			}
		else
			{
			print OUT "$value ";
			}
		}
	if ($verbose > 0 && $cnttot / $xnx_d / $yny_d > $target)
		{
		$value = 100.0 * $target;
		print "$value% complete\n";
		$target = $target + 0.1;
		}
	}
close OUT;
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
