eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
                    & eval 'exec perl -S $0 $argv:q'
                         if 0;
#--------------------------------------------------------------------
#    The MB-system:	mbm_arc2grd.perl	4/23/01
#    $Id: mbm_grdcut.perl,v 5.2 2003-04-17 20:42:48 caress Exp $
#
#    Copyright (c) 2001, 2003 by
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
#   mbm_grdcut
#
# Purpose:
#   Macro to painlessly cut out a region from a GMT grd grid file.
#   The GMT program grdcut requires that one specify bounds which
#   exactly match grid cell boundaries. Frequently, one just wants
#   to extract an approximate region quickly, without calculating
#   grid cell sizes and boundary locations. This macro does the
#   the calculations and extracts the subregion closest to that
#   specified by the user.
#
# Basic Usage:
#   mbm_grdcut -Iarcfile -Ogrdfile [-H -V]
#
# Author:
#   David W. Caress
#   Monterey Bay Aquarium Research Institute
#   Moss Landing, CA 95039
#   April 23, 2001
#   (at sea on the R/V Western Flyer about
#    10 km off the Kohala coast of Hawaii)
#
# Version:
#   $Id: mbm_grdcut.perl,v 5.2 2003-04-17 20:42:48 caress Exp $
#
# Revisions:
#   $Log: not supported by cvs2svn $
#   Revision 5.1  2001/06/03 06:59:24  caress
#   Release 5.0.beta01
#
#   Revision 5.0  2001/04/24 23:29:35  caress
#   Initital revision.
#
#
#

$program_name = "mbm_grdcut";

# Determine the GMT version
@grdinfo = `grdinfo 2>&1`;
while (@grdinfo)
	{
	$line = shift @grdinfo;
	if ($line =~ 
		/^grdinfo\s+(\S+)\s+\S+/)
		{
		($gmt_version) = $line =~ 
			/^grdinfo\s+(\S+)\s+\S+/;
		}
	}

# Deal with command line arguments
&Getopts('I:i:O:o:R:r:VvHh');
$ifile =    ($opt_I || $opt_i);
$ofile =    ($opt_O || $opt_o);
$help =     ($opt_H || $opt_h);
$bounds =   ($opt_R || $opt_r);
$verbose =  ($opt_V || $opt_v);

#--------------------------------------------------------------------

# print out help message if required
if ($help)
	{
 	print "Macro to painlessly cut out a region from a GMT grd grid file.\n";
	print "The GMT program grdcut requires that one specify bounds which\n";
	print "exactly match grid cell boundaries. Frequently, one just wants\n";
	print "to extract an approximate region quickly, without calculating\n";
	print "grid cell sizes and boundary locations. This macro does the\n";
	print "the calculations and extracts the subregion closest to that\n";
	print "specified by the user.\n";
	print "\t$program_name -Iinputfile -Ooutputfile  -Rw/e/s/n [-H -V]\n";
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
if (!$ofile)
	{
	print "\a";
	die "No output file specified\nMacro $program_name aborted\n";
	}
if (!$bounds)
	{
	print "\a";
	die "No output bounds specified\nMacro $program_name aborted\n";
	}

# get specified output bounds
if ($bounds =~ /^\S+\/\S+\/\S+\/\S+$/)
	{
	($xmin_raw,$xmax_raw,$ymin_raw,$ymax_raw) = $bounds =~ 
			/(\S+)\/(\S+)\/(\S+)\/(\S+)/;
	$xminr = &GetDecimalDegrees($xmin_raw);
	$xmaxr = &GetDecimalDegrees($xmax_raw);
	$yminr = &GetDecimalDegrees($ymin_raw);
	$ymaxr = &GetDecimalDegrees($ymax_raw);
	}


# Save old GMT default double format and set new format
$line = `gmtdefaults -L | grep D_FORMAT`;
($dformatsave) = $line =~ /D_FORMAT\s+=\s+(\S+)/;
`gmtset D_FORMAT %.10lg`;

# get limits of files using grdinfo
@grdinfo = `grdinfo $ifile`;
while (@grdinfo)
	{
	$line = shift @grdinfo;
	if ($line =~ 
			/\S+\s+x_min:\s+(\S+)\s+x_max:\s+(\S+)\s+x_inc:/)
		{
		($xmin,$xmax,$xinc,$nx) = $line =~ 
				/\S+\s+x_min:\s+(\S+)\s+x_max:\s+(\S+)\s+x_inc:\s+(\S+)\s+units:.+nx:\s+(\S+)/;
		}
	if ($line =~ /\S+\s+y_min:\s+(\S+)\s+y_max:\s+(\S+)\s+y_inc:/)
		{
		($ymin,$ymax,$yinc,$ny) = $line =~ 
				/\S+\s+y_min:\s+(\S+)\s+y_max:\s+(\S+)\s+y_inc:\s+(\S+)\s+units:.+ny:\s+(\S+)/;
		}
	}

# reset the GMT default double format
`gmtset D_FORMAT $dformatsave`;

# calculate acceptable output bounds
$diffx = ($xminr - $xmin) / $xinc;
$diffxn = int(($xminr - $xmin) / $xinc);
print "diffx: $diffx $diffxn\n";
$xminout = $xmin + $xinc * int(($xminr - $xmin) / $xinc);
$xmaxout = $xmin + $xinc * int(($xmaxr - $xmin) / $xinc + 0.5);
$yminout = $ymin + $yinc * int(($yminr - $ymin) / $yinc);
$ymaxout = $ymin + $yinc * int(($ymaxr - $ymin) / $yinc + 0.5);
$nxout = int((($xmaxout - $xminout) / $xinc) + 0.5) + 1;
$nyout = int((($ymaxout - $yminout) / $yinc) + 0.5) + 1;

# tell the world we got started
if ($verbose) 
	{
	print "\nProgram $program_name status:\n";
	print "\tInput GRD file:         $ifile\n";
	print "\tOutput GRD file:        $ofile\n";
	print "\tInput Grid bounds:      $xmin $xmax  $ymin $ymax\n";
	print "\tInput grid dimensions:  $nx  $ny\n";
	print "\tGrid cell sizes:        $xinc  $yinc\n";
	print "\tRequested Grid bounds:  $xminr $xmaxr  $yminr $ymaxr\n";
	print "\tOutput Grid bounds:     $xminout $xmaxout  $yminout $ymaxout\n";
	print "\tOutput grid dimensions: $nxout  $nyout\n";
	}

# run grdcut
$cmd = "grdcut $ifile -G$ofile -R$xminout/$xmaxout/$yminout/$ymaxout -V";
if ($verbose)
	{
	print "\nRunning grdcut...\n$cmd\n";
	}
`$cmd`;

exit 0;

#-----------------------------------------------------------------------
sub GetDecimalDegrees {

	# make local variables
	local ($dec_degrees, $degrees, $minutes, $seconds);

	# deal with dd:mm:ss format
	if ($_[0] =~ /^\S+:\S+:\S+$/)
		{
		($degrees, $minutes, $seconds) 
			= $_[0] =~ /^(\S+):(\S+):(\S+)$/;
		if ($degrees =~ /^-\S+/)
			{
			$dec_degrees = $degrees 
				- $minutes / 60.0 
				- $seconds / 3600.0;
			}
		else
			{
			$dec_degrees = $degrees 
				+ $minutes / 60.0 
				+ $seconds / 3600.0;
			}
		}
	# deal with dd:mm format
	elsif ($_[0] =~ /^\S+:\S+$/)
		{
		($degrees, $minutes) 
			= $_[0] =~ /^(\S+):(\S+)$/;
		if ($degrees =~ /^-\S+/)
			{
			$dec_degrees = $degrees - $minutes / 60.0;
			}
		else
			{
			$dec_degrees = $degrees + $minutes / 60.0;
			}
		}

	# value already in decimal degrees
	else
		{
		$dec_degrees = $_[0];
		}

	# return decimal degrees;
	$dec_degrees;
}
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
#-----------------------------------------------------------------------
