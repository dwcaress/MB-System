eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
                    & eval 'exec perl -S $0 $argv:q'
                         if 0;
#--------------------------------------------------------------------
#    The MB-system:	mbm_plot.perl	6/18/93
#    $Id: mbm_plot.perl,v 4.8 1995-05-12 17:43:23 caress Exp $
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
#   mbm_plot
#
# Purpose:
#   Perl shellscript to generate a gmt plot of the multibeam data contained 
#   in the specified file.  The plot will be scaled to fit on an 8.5 X 11
#   inch page.  The plot may include bathymetry color fill (-G1), 
#   bathymetry color shaded relief (-G2), bathymetry shaded with amplitudes
#   (-G3), greyshade fill amplitude (-G4), or greyshade fill sidescan (-G5).
#   The plot may also include four color contoured bathymetry (-C).  
#   A gmt shellscript will be created which generates 
#   a postscript image and then displays it using pageview. 
#   The -X option will cause the shellscript to be executed immediately.
#
# Usage:
#   mbm_plot -Fformat -Ifile [-Amagnitude/azimuth
#            -C -Gmode -H -N -Oroot -P -Rw/e/s/n -S -V -X]
#
# Author:
#   David W. Caress
#   Lamont-Doherty Earth Observatory
#   Palisades, NY  10964
#   June 17, 1993
#
# Version:
#   $Id: mbm_plot.perl,v 4.8 1995-05-12 17:43:23 caress Exp $
#
# Revisions:
#   $Log: not supported by cvs2svn $
# Revision 4.7  1995/05/05  17:00:23  caress
# Fixed typo.
#
# Revision 4.6  1995/05/03  17:30:42  caress
# Fixed some typos noted by Mike Realander <miker@ocean.washington.edu>
#
# Revision 4.5  1995/03/20  16:42:45  caress
# Fixed synopsis.
#
# Revision 4.4  1995/02/14  19:50:31  caress
# Version 4.2
#
# Revision 4.3  1994/10/21  13:54:57  caress
# Release V4.0
#
# Revision 4.2  1994/10/21  11:36:58  caress
# Release V4.0
#
# Revision 4.1  1994/05/02  00:19:44  caress
# Added 2 significant digits to basemap tick intervals.
#
# Revision 4.0  1994/03/05  23:52:40  caress
# First cut at version 4.0
#
# Revision 4.3  1994/03/05  03:01:33  caress
# Added capability to handle amplitude and sidescan plots.
#
# Revision 4.2  1994/03/03  04:11:13  caress
# Fixed copyright message.
#
# Revision 4.1  1994/02/26  21:15:42  caress
# Altered parsing of mbinfo output to be consistent
# with changes in version 4 mbinfo.
#
# Revision 4.0  1994/02/26  19:37:57  caress
# First cut at new version.
#
# Revision 3.7  1993/11/27  18:31:54  caress
# Fixed syntax error in last fix.
#
# Revision 3.6  1993/11/27  18:24:07  caress
# Made choice of tick interval more rational.
#
# Revision 3.5  1993/08/17  16:58:36  caress
# Set location of perl to /usr/local/bin/perl
#
# Revision 3.4  1993/08/16  23:03:59  caress
# I'm not sure what the changes are - I'm just checking in the
# current version.
#
# Revision 3.3  1993/07/03  02:08:27  caress
# Changed /usr/local/bin/perl to /usr/lib/perl for L-DEO installation.
#
# Revision 3.2  1993/06/19  15:06:54  caress
# Fixed handling of command line argument "-O".
#
# Revision 3.1  1993/06/19  11:50:30  caress
# Improved autoscaling so that a reasonable box including
# all of the data is always (I hope) used.  Also added -P
# option to turn on the invocation of pageview; pageview
# is now put in the output shellscript only if requested.
#
# Revision 3.0  1993/06/19  01:00:18  caress
# Initial version.
#
#
$program_name = "mbm_plot";

# Deal with command line arguments
&Getopts('A:a:CcF:f:G:g:HhI:i:NnO:o:PpR:r:SsVvXx');
$file =    ($opt_I || $opt_i);
$root =    ($opt_O || $opt_o);
$format =  ($opt_F || $opt_f);
$bounds =  ($opt_R || $opt_r);
$color = ($opt_G || $opt_g);
$contour = ($opt_C || $opt_c);
$navigation = ($opt_N || $opt_n);
$shadecontrol = ($opt_A || $opt_a);
$execute = ($opt_X || $opt_x);
$stretch = ($opt_S || $opt_s);
$view_ps = ($opt_P || $opt_p);
$help =    ($opt_H || $opt_h);
$verbose = ($opt_V || $opt_v);

# print out help message if required
if ($help)
	{
	print "\n$program_name:\n";
	print "\nPerl shellscript to generate a gmt plot of the multibeam data contained \nin the specified file.  The plot will be scaled to fit on an 8.5 X 11 \ninch page.  The plot may include bathymetry color fill (-G1),\nbathymetry color shaded relief (-G2), bathymetry shaded with amplitudes\n(-G3), greyshade fill amplitude (-G4), or greyshade fill sidescan (-G5).\nThe plot may also include four color contoured bathymetry (-C).  A gmt shellscript will be created which \ngenerates a postscript image and then displays it using pageview. \nThe -X option will cause the shellscript to be executed immediately.\n";
	print "\nUsage: $program_name -Fformat -Ifile [-Oroot -Rw/e/s/n -Gmode -Amagnitude/azimuth -C -N -X -P -S -V -H]\n";
	exit 0;
	}

# check for defined parameters
if (!$file)
	{
	die "No input file specified - $program_name aborted\n";
	}
if (!$root)
	{
	$root = $file;
	}
if (!$format) 
	{
	$format = "-1";
	}
if (!$color && !$contour && !$navigation)
	{
	$navigation = 1;
	}
if ($color && $color == 0)
	{
	$color = 1;
	}

# get postscript viewer
# check environment variable
if ($ENV{"MB_PS_VIEWER"})
	{
	$ps_viewer = $ENV{"MB_PS_VIEWER"};
	}
# check for .mbio_defaults file
if (!$ps_viewer)
	{
	$home = $ENV{"HOME"};
	$mbdef = "$home/.mbio_defaults";
	if (open(MBDEF,"<$mbdef"))
		{
		while (<MBDEF>)
			{
			if (/ps viewer:\s+(\S+)/)
				{
				($ps_viewer) = /ps viewer:\s+(\S+)/;
				}
			}
		}
	}
# just set it to ghostview
if (!$ps_viewer)
	{
	$ps_viewer = "ghostview";
	}

# deal with specified bounds
if ($bounds)
	{
	($rxmin,$rxmax,$rymin,$rymax) = $bounds =~ /(\S+)\/(\S+)\/(\S+)\/(\S+)/;
	$rbounds = "-R$bounds";
	}

# get limits of file using mbinfo
if ($verbose) 
	{
	print "\nRunning $program_name...\nRunning mbinfo...\n";
	}
@mbinfo = `mbinfo -F$format -I$file $rbounds`;
while (@mbinfo)
	{
	$line = shift @mbinfo;
	if ($line =~ /Minimum Longitude:\s+(\S+)\s+Maximum Longitude:\s+(\S+)/)
		{
		print "$line";
		($xmin,$xmax) = 
			$line =~ /Minimum Longitude:\s+(\S+)\s+Maximum Longitude:\s+(\S+)/;
		}
	if ($line =~ /Minimum Latitude:\s+(\S+)\s+Maximum Latitude:\s+(\S+)/)
		{
		print "$line";
		($ymin,$ymax) = 
			$line =~ /Minimum Latitude:\s+(\S+)\s+Maximum Latitude:\s+(\S+)/;
		}
	if ($line =~ /Minimum Depth:\s+(\S+)\s+Maximum Depth:\s+(\S+)/)
		{
		print "$line";
		($zmin,$zmax) = 
		$line =~ /Minimum Depth:\s+(\S+)\s+Maximum Depth:\s+(\S+)/;
		}
	if ($line =~ /Minimum Amplitude:\s+(\S+)\s+Maximum Amplitude:\s+(\S+)/)
		{
		print "$line";
		($amin,$amax) = 
		$line =~ /Minimum Amplitude:\s+(\S+)\s+Maximum Amplitude:\s+(\S+)/;
		}
	if ($line =~ /Minimum Sidescan:\s+(\S+)\s+Maximum Sidescan:\s+(\S+)/)
		{
		print "$line";
		($smin,$smax) = 
		$line =~ /Minimum Sidescan:\s+(\S+)\s+Maximum Sidescan:\s+(\S+)/;
		}
	}

# check that there is data
if ($xmin >= $xmax || $ymin >= $ymax)
	{
	die "Does not appear to be any data in the input!\n$program_name aborted.\n";
	}
if (($color == 1 || $color == 2 || $color == 3) && ($zmin >= $zmax))
	{
	die "Does not appear to be any bathymetry data in the input!\n$program_name aborted.\n";
	}
if (($color == 3 || $color == 4) && ($amin >= $amax))
	{
	die "Does not appear to be any amplitude data in the input!\n$program_name aborted.\n";
	}
if (($color == 5) && ($smin >= $smax))
	{
	die "Does not appear to be any sidescan data in the input!\n$program_name aborted.\n";
	}

# either use specified bounds
if ($bounds)
	{
	$xmin = $rxmin;
	$xmax = $rxmax;
	$ymin = $rymin;
	$ymax = $rymax;
	if ($verbose)
		{
		print "\nUsing specified plot bounds:\n";
		printf "  Minimum Lon:%11.4f     Maximum Lon:%11.4f\n", 
			$xmin,$xmax;
		printf "  Minimum Lat:%11.4f     Maximum Lat:%11.4f\n", 
			$ymin,$ymax;
		}
	}
# or expand the data limits a bit and ensure a reasonable aspect ratio
else
	{
	$delx = 0.05 * ($xmax - $xmin);
	$dely = 0.05 * ($ymax - $ymin);
	$xmin = $xmin - $delx;
	$xmax = $xmax + $delx;
	$ymin = $ymin - $dely;
	$ymax = $ymax + $dely;
	$dx = $xmax - $xmin;
	$dy = $ymax - $ymin;
	if ($dy/$dx > 2.0)
		{
		$delx = 0.5 * (0.5 * $dy - $dx);
		$xmin = $xmin - $delx;
		$xmax = $xmax + $delx;
		}
	elsif ($dx/$dy > 2.0)
		{
		$dely = 0.5 * (0.5 * $dx - $dy);
		$ymin = $ymin - $dely;
		$ymax = $ymax + $dely;
		}
	if ($verbose)
		{
		print "\nUsing calculated plot bounds:\n";
		printf "  Minimum Lon:%11.4f     Maximum Lon:%11.4f\n", 
			$xmin,$xmax;
		printf "  Minimum Lat:%11.4f     Maximum Lat:%11.4f\n", 
			$ymin,$ymax;
		}
	}

# now come up with a reasonable plotscale
print "\nRunning mapproject...\n";
`echo $xmin $ymin > tmp$$.dat`;
`echo $xmax $ymin >> tmp$$.dat`;
`echo $xmax $ymax >> tmp$$.dat`;
`echo $xmin $ymax >> tmp$$.dat`;
@projected = `mapproject tmp$$.dat -Jm1.0 -R$xmin/$xmax/$ymin/$ymax `;
`rm -f tmp$$.dat`;
while (@projected)
	{
	$line = shift @projected;
	if (!$xxmin)
		{
		($xxmin,$yymin) = $line =~ /(\S+)\s+(\S+)/;
		$xxmax = $xxmin;
		$yymax = $yymin;
		}
	else
		{
		($xx,$yy) = $line =~ /(\S+)\s+(\S+)/;
		$xxmin = ($xx < $xxmin ? $xx : $xxmin);
		$xxmax = ($xx > $xxmax ? $xx : $xxmax);
		$yymin = ($yy < $yymin ? $yy : $yymin);
		$yymax = ($yy > $yymax ? $yy : $yymax);
		}
	}
$dxx = $xxmax - $xxmin;
$dyy = $yymax - $yymin;
$plotscale_landscape = 9.0/$dxx;
if ($plotscale_landscape*$dyy > 5.5)
	{
	$plotscale_landscape = 5.5/$dyy;
	}
$plotscale_portrait = 6.5/$dxx;
if ($plotscale_portrait*$dyy > 8.0)
	{
	$plotscale_portrait = 8.0/$dyy;
	}
if ($plotscale_landscape > $plotscale_portrait)
	{
	$landscape = 1;
	$plotscale = $plotscale_landscape;
	}
else
	{
	$portrait = 1;
	$plotscale = $plotscale_portrait;
	}
if ($verbose)
	{
	printf "\nPlot scale: %.3f inches/degree longitude\n", $plotscale;
	if ($portrait)
		{
		print "Using portrait orientation\n";
		}
	else
		{
		print "Using landscape orientation\n";
		}
	}

# figure out where to place the color scale
$colorscale_width = 0.8*$dxx*$plotscale;
$colorscale_height = 0.15;
$colorscale_offx = 0.5*$dxx*$plotscale;
$colorscale_offy = -0.5;

# figure out some reasonable tick intervals for the basemap
$base_tick = $dxx/5;
if ($base_tick < 0.0166667)
	{
	$base_tick = "1m";
	}
elsif ($base_tick < 0.0333333)
	{
	$base_tick = "2m";
	}
elsif ($base_tick < 0.0833333)
	{
	$base_tick = "5m";
	}
elsif ($base_tick < 0.1666667)
	{
	$base_tick = "10m";
	}
elsif ($base_tick < 0.25)
	{
	$base_tick = "15m";
	}
elsif ($base_tick < 0.5)
	{
	$base_tick = "30m";
	}
elsif ($base_tick < 1.0)
	{
	$base_tick = "1";
	}
elsif ($base_tick < 2.0)
	{
	$base_tick = "2";
	}
elsif ($base_tick < 5.0)
	{
	$base_tick = "5";
	}


# figure out some reasonable contour intervals for swath contouring
$contour_int = 50;
$color_int = 250;
$tick_int = 250;

# figure out some reasonable time tick intervals for navigation
$time_tick = 0.25;
$time_annot = 1;
$date_annot = 4;

# come up with the filenames
$cmdfile = "$root.cmd";
$psfile = "$root.ps";
if ($format <= -1)
	{
	$listfile = $file;
	}
else
	{
	$listfile = "datalist\$\$";
	}
$cptfile = "$root.cpt";
$navfile = "$root\$\$.nav";
$gmtfile = "gmtdefaults\$\$";

# open the shellscript file
open(FCMD,">$cmdfile") || die "Cannot open output file $cmdfile\nMacro $program_name aborted.\n";

# write the shellscript header
print FCMD "#\n# Shellscript to create Postscript plot of multibeam data\n";
print FCMD "# Created by macro $program_name\n";

# Reset GMT fonts, saving old defaults
print FCMD "#\n# Save existing GMT defaults\n";
print FCMD "echo Saving GMT defaults...\n";
print FCMD "gmtdefaults -D > $gmtfile\n";
print FCMD "#\n# Set new GMT fonts\n";
print FCMD "echo Setting new GMT fonts...\n";
print FCMD "gmtset ANOT_FONT Helvetica ANOT_FONT_SIZE 8 \\\n";
print FCMD "\tHEADER_FONT Helvetica HEADER_FONT_SIZE 10 \\\n";
print FCMD "\tLABEL_FONT Helvetica LABEL_FONT_SIZE 8\n";

# generate datalist file if needed
if ($format != -1)
	{
	print FCMD "#\n# Make datalist file \n";
	print FCMD "echo Making datalist file...\n";
	print FCMD "echo $file $format > $listfile\n";
	}

# generate color pallette table file if needed
if ($color == 1 || $color == 2 || $color == 3)
	{
	print FCMD "#\n# Make color pallette table file\n";
	print FCMD "echo Making color pallette table file...\n";
	$ncpt = 15;
	@cptbr = (255, 255, 255, 255, 255, 240, 205, 138, 106,  87,  50,   0,  40,  21,  37);
	@cptbg = (255, 221, 186, 161, 189, 236, 255, 236, 235, 215, 190, 160, 127,  92,  57);
	@cptbb = (255, 171, 133,  68,  87, 121, 162, 174, 255, 255, 255, 255, 251, 236, 175);

	# break data distribution up into equal size 
	# regions using mbhistogram
	if ($stretch)
		{
		if ($verbose) 
			{
			print "\nRunning mbhistogram...\n";
			}
		@mbhistogram = `mbhistogram -F$format -I$file -A0 -D$zmin/$zmax -M$ncpt -N1000`;
		$d1 = shift @mbhistogram;
		}

	# get spacing the old way
	else
		{
		$dd = 1.1 * ($zmax - $zmin)/($ncpt - 1);
		$d1 = $zmin - 0.05*($zmax - $zmin);
		}

	# print out the cpt
	foreach $i (0 .. $ncpt - 2)
		{
		if ($stretch)
			{
			$d2 = shift @mbhistogram;
			}
		else
			{
			$d2 = $d1 + $dd;
			}
		printf FCMD "echo %.0f %d %d %d %.0f %d %d %d",
			$d1,@cptbr[$i],@cptbg[$i],@cptbb[$i],
			$d2,@cptbr[$i+1],@cptbg[$i+1],@cptbb[$i+1];
		if ($i == 0)
			{
			print FCMD " >";
			}
		else
			{
			print FCMD " >>";
			}
		print FCMD " $cptfile\n";
		$d1 = $d2;
		}
	}

# generate greyscale pallette table file if needed
if ($color == 4 || $color == 5)
	{

	print FCMD "#\n# Make greyshade pallette table file\n";
	print FCMD "echo Making greyshade pallette table file...\n";
	$ncpt = 15;
#	@cptbr = (0, 17, 35, 53, 71, 89, 107, 125, 143,  161,  179,   197,  215,  233,  255);
#	@cptbg = (0, 17, 35, 53, 71, 89, 107, 125, 143,  161,  179,   197,  215,  233,  255);
#	@cptbb = (0, 17, 35, 53, 71, 89, 107, 125, 143,  161,  179,   197,  215,  233,  255);
	@cptbr = (255, 233, 215, 197, 179, 161, 143, 125, 107,  89,  71,   53,  35,  17,  0);
	@cptbg = (255, 233, 215, 197, 179, 161, 143, 125, 107,  89,  71,   53,  35,  17,  0);
	@cptbb = (255, 233, 215, 197, 179, 161, 143, 125, 107,  89,  71,   53,  35,  17,  0);

	# break data distribution up into equal size 
	# regions using mbhistogram
	if ($stretch)
		{
		if ($verbose) 
			{
			print "\nRunning mbhistogram...\n";
			}
		if ($color == 4)
			{
			@mbhistogram = `mbhistogram -F$format -I$file -A1 -D$amin/$amax -M$ncpt -N1000`;
			}
		elsif ($color == 5)
			{
			@mbhistogram = `mbhistogram -F$format -I$file -A2 -D$smin/$smax -M$ncpt -N1000`;
			}
		$d1 = shift @mbhistogram;
		}

	# get spacing the old way
	else
		{
		if ($color == 4)
			{
			$dd = 1.1 * ($amax - $amin)/($ncpt - 1);
			$d1 = $amin - 0.05*($amax - $amin);
			}
		if ($color == 5 && $format == 41)
			{
			$smin = 20 * log($smin) / log(10);
			$smax = 20 * log($smax) / log(10);
			$dd = 1.01 * ($smax - $smin)/($ncpt - 1);
			$d1 = $smin - 0.01*($smax - $smin);
			}
		elsif ($color == 5)
			{
			$dd = 1.05 * ($smax - $smin)/($ncpt - 1);
			$d1 = $smin - 0.05*($smax - $smin);
			}
		}

	# print out the cpt
	foreach $i (0 .. $ncpt - 2)
		{
		if ($stretch)
			{
			$d2 = shift @mbhistogram;
			}
		else
			{
			$d2 = $d1 + $dd;
			}
		printf FCMD "echo %.0f %d %d %d %.0f %d %d %d",
			$d1,@cptbr[$i],@cptbg[$i],@cptbb[$i],
			$d2,@cptbr[$i+1],@cptbg[$i+1],@cptbb[$i+1];
		if ($i == 0)
			{
			print FCMD " >";
			}
		else
			{
			print FCMD " >>";
			}
		print FCMD " $cptfile\n";
		$d1 = $d2;
		}
	}

# set shade control if not set by user
if (!$shadecontrol && $color == 3)
	{
	$shademagnitude = -2.0/($amax - $amin);
	$shadenull = $amin + 0.3 * ($amax - $amin);
	$shadecontrol = sprintf("%.4g/%.4g",$shademagnitude,$shadenull);
	}
elsif (!$shadecontrol)
	{
	$shadecontrol = "2.5/0";
	}

# do swath plot if needed
if ($color)
	{
	print FCMD "#\n# Run mbswath\n";
	print FCMD "echo Running mbswath...\n";
	printf FCMD "mbswath -I%s -Jm%g \\\n", $listfile,$plotscale;
	printf FCMD "\t-R%.4f/%.4f/%.4f/%.4f \\\n", $xmin,$xmax,$ymin,$ymax;
	printf FCMD "\t-Ba%sg%sf\":.Data File %s:\" \\\n", 
		$base_tick,$base_tick,$file;
	print FCMD "\t-C$cptfile -p1 -A1 -Q100 -Z$color";
	if ($color == 2 || $color == 3)
		{
		print FCMD " -G$shadecontrol";
		}
#	if ($color == 5 && $format == 41)
#		{
#		print FCMD " -D3/1/0/1";
#		}
	print FCMD " \\\n\t";
	if ($portrait)
		{
		print FCMD " -P";
		}
	print FCMD " -X1 -Y1.75";
	print FCMD " -K -V > $psfile\n";
	}

# do contour plot if needed
if ($contour || $navigation)
	{
	print FCMD "#\n# Run mbcontour\n";
	print FCMD "echo Running mbcontour...\n";
	printf FCMD "mbcontour -I%s -Jm%g \\\n\t", $listfile,$plotscale;
	printf FCMD "-R%.4f/%.4f/%.4f/%.4f \\\n\t", $xmin,$xmax,$ymin,$ymax;
	printf FCMD "-Ba%sg%s\":.Data File %s:\" \\\n\t", 
		$base_tick,$base_tick,$file;
	if ($color && $contour)
		{
		print FCMD "-C$cptfile";
		}
	elsif ($contour)
		{
		printf FCMD "-A%.4f/%.4f/%.4f/%.4f/0.01/0.1",
			$contour_int,$color_int,$tick_int,$color_int;
		}
	if ($contour && $format == 41)
		{
		print FCMD " -Z1";
		}
	if ($contour)
		{
		print FCMD " \\\n\t";
		}
	if ($navigation)
		{
		printf FCMD "-D%.4f/%.4f/%.4f/0.15 ",
			$time_tick,$time_annot,$date_annot;
		}
	print FCMD "-p1 \\\n\t";
	if ($portrait)
		{
		print FCMD "-P";
		}
	if ($color)
		{
		print FCMD " -O -K -V >> $psfile\n";
		}
	else
		{
		print FCMD " -X1 -Y1 -V > $psfile\n";
		}
	}

# do scale plot if needed
if ($color)
	{
	print FCMD "#\n# Run psscale\n";
	print FCMD "echo Running psscale...\n";
	print FCMD "psscale  -C$cptfile";
	printf FCMD " -D%.4f/%.4f/%.4f/%.4fh \\\n\t", 
		$colorscale_offx,$colorscale_offy,
		$colorscale_width,$colorscale_height;
	if ($color == 5)
		{
		print FCMD " -B\":.Sidescan Pixel Values:\"";
		}
	elsif ($color == 4)
		{
		print FCMD " -B\":.Beam Amplitude Values:\"";
		}
	else
		{
		print FCMD " -B\":.Depth (meters):\"";
		}
	if ($portrait)
		{
		print FCMD " -P";
		}
	print FCMD " -O -V >> $psfile\n";
	}

# delete surplus files
print FCMD "#\n# Delete surplus files\n";
print FCMD "echo Deleting surplus files...\n";
print FCMD "rm -f $cptfile\n";
if ($format > -1)
	{
	print FCMD "rm -f $listfile\n";
	}

# reset GMT defaults
print FCMD "#\n# Reset GMT default fonts\n";
print FCMD "echo Resetting GMT fonts...\n";
print FCMD "mv $gmtfile .gmtdefaults\n";

# display image on screen if desired
print FCMD "#\n# Run $ps_viewer\n";
if ($view_ps)
	{
	print FCMD "echo Running $ps_viewer in background...\n";
	print FCMD "$ps_viewer $psfile &\n";
	}
else
	{
	print FCMD "#echo Running $ps_viewer in background...\n";
	print FCMD "#$ps_viewer $psfile &\n";
	}

# claim it's all over
print FCMD "#\n# All done!\n";
print FCMD "echo All done!\n";

# now close the shellscript and make it executable
close FCMD;
chmod 0775, $cmdfile;

# execute shellscript if desired
if ($execute)
	{
	if ($verbose)
		{
		print "Executing shellscript $cmdfile...\n";
		}
	if (verbose)
		{
		print `$cmdfile`;
		}
	else
		{
		`$cmdfile`;
		}
	}

print "\nAll done!\n";
exit 0;

#-----------------------------------------------------------------------
# This should be loaded from the library but Perl installations
# are often screwed up so....
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
