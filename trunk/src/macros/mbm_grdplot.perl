eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
                    & eval 'exec perl -S $0 $argv:q'
                         if 0;
#--------------------------------------------------------------------
#    The MB-system:	mbm_grdplot.perl	10/19/94
#    $Id: mbm_grdplot.perl,v 4.0 1994-10-21 11:49:39 caress Exp $
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
#   mbm_grdplot
#
# Purpose:
#   Perl shellscript to generate a gmt plot of the data contained 
#   in the specified grd file.  The plot will be scaled to fit on 
#   an 8.5 X 11 inch page.  The plot may include color fill, 
#   color shaded relief, contours, or 3D shaded relief
#   views. A gmt shellscript will be created which generates 
#   a postscript image and then displays it using pageview. 
#   The -X option will cause the shellscript to be executed immediately.
#
# Usage:
#   mbm_grdplot -Ifile [-Aazimuth -C -D -Eazimuth/elevation -G -H -Oroot 
#            -P -S -T -V -X]
#
# Author:
#   David W. Caress
#   Lamont-Doherty Earth Observatory
#   Palisades, NY  10964
#   October 19, 1994
#
# Version:
#   $Id: mbm_grdplot.perl,v 4.0 1994-10-21 11:49:39 caress Exp $
#
# Revisions:
#   $Log: not supported by cvs2svn $
# Revision 1.1  1994/10/21  11:36:58  caress
# Initial revision
#
#
#
$program_name = "mbm_grdplot";

# Deal with command line arguments
&Getopts('A:a:CcDdE:e:GgHhI:i:O:o:PpSsTtVvXx');
$file =    ($opt_I || $opt_i);
$root =    ($opt_O || $opt_o);
$color = ($opt_G || $opt_g);
$contour = ($opt_C || $opt_c);
$shade = ($opt_S || $opt_s);
$threed = ($opt_T || $opt_t);
$shadecontrol = ($opt_A || $opt_a);
$viewcontrol = ($opt_E || $opt_e);
$data_down = ($opt_D || $opt_d);
$execute = ($opt_X || $opt_x);
$view_ps = ($opt_P || $opt_p);
$help =    ($opt_H || $opt_h);
$verbose = ($opt_V || $opt_v);

# print out help message if required
if ($help)
	{
	print "\n$program_name:\n";
	print "\nPerl shellscript to generate a gmt plot of the multibeam data contained \nin the specified file.  The plot will be scaled to fit on an 8.5 X 11 \ninch page.  The plot may include bathymetry color fill (-G1),\nbathymetry color shaded relief (-G2), bathymetry shaded with amplitudes\n(-G3), greyshade fill amplitude (-G4), or greyshade fill sidescan (-G5).\nThe plot may also include four color contoured bathymetry (-C).  A gmt shellscript will be created which \ngenerates a postscript image and then displays it using pageview. \nThe -X option will cause the shellscript to be executed immediately.\n";
	print "\nUsage: $program_name -Ifile [-Aazimuth -C -D -Eelevation/azimuth -G -H -Oroot -P -S -T -V -X]\n";
	die "\n";
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
if (!$color && !$contour)
	{
	$color = 1;
	}
if ($shade)
	{
	$color = 1;
	}
if ($shade && !$shadecontrol)
	{
	$shadecontrol = "90.0";
	}
if ($threed)
	{
	$contour = 0;
	$color = 1;
	}
if ($threed && !$viewcontrol)
	{
	$viewcontrol = "200/40";
	}
($view_az, $view_el) = $viewcontrol =~ /(\S+)\/(\S+)/;

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

# get limits of file using grdinfo
if ($verbose) 
	{
	print "\nRunning $program_name...\nRunning grdinfo...\n";
	}
@grdinfo = `grdinfo $file`;
while (@grdinfo)
	{
	$line = shift @grdinfo;
	if ($line =~ /\S+\s+x_min:\s+(\S+)\s+x_max:\s+(\S+)\s+x_inc:/)
		{
		($xmin,$xmax) = 
			$line =~ /\S+\s+x_min:\s+(\S+)\s+x_max:\s+(\S+)\s+x_inc:/;
		}
	if ($line =~ /\S+\s+y_min:\s+(\S+)\s+y_max:\s+(\S+)\s+y_inc:/)
		{
		($ymin,$ymax) = 
			$line =~  /\S+\s+y_min:\s+(\S+)\s+y_max:\s+(\S+)\s+y_inc:/;
		}
	if ($line =~ /\S+\s+zmin:\s+(\S+)\s+zmax:\s+(\S+)\s+units:/)
		{
		($zmin,$zmax) = 
			$line =~  /\S+\s+zmin:\s+(\S+)\s+zmax:\s+(\S+)\s+units:/;
		}
	}

# check that there is data
if ($xmin >= $xmax || $ymin >= $ymax || $zmin >= $zmax)
	{
	die "The program grdinfo does not appear to have worked properly!\n$program_name aborted.\n"
	}

# use file bounds for plot
if ($verbose)
	{
	print "\nPlot bounds:\n";
	printf "  Minimum Lon:%11.4f     Maximum Lon:%11.4f\n", 
		$xmin,$xmax;
	printf "  Minimum Lat:%11.4f     Maximum Lat:%11.4f\n", 
		$ymin,$ymax;
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
$zscale = 3.0/($zmax - $zmin);

# figure out where to place the color scale
$colorscale_width = 0.8*$dxx*$plotscale;
$colorscale_height = 0.15;
$colorscale_offx = 0.5*$dxx*$plotscale;
$colorscale_offy = -0.5;

# figure out some reasonable tick intervals for the basemap
$base_tick = $dxx/5;
if ($base_tick < 0.0166667)
	{
	$base_tick = 0.0166667;
	}
elsif ($base_tick < 0.0333333)
	{
	$base_tick = 0.0333333;
	}
elsif ($base_tick < 0.0833333)
	{
	$base_tick = 0.0833333;
	}
elsif ($base_tick < 0.1666667)
	{
	$base_tick = 0.1666667;
	}
elsif ($base_tick < 0.25)
	{
	$base_tick = 0.25;
	}
elsif ($base_tick < 0.5)
	{
	$base_tick = 0.5;
	}
elsif ($base_tick < 1.0)
	{
	$base_tick = 1.0;
	}
elsif ($base_tick < 2.0)
	{
	$base_tick = 2.0;
	}
elsif ($base_tick < 5.0)
	{
	$base_tick = 5.0;
	}


# figure out a reasonable contour intervals
$contour_int = ($zmax - $zmin)/20; 

# come up with the filenames
$cmdfile = "$root.cmd";
$psfile = "$root.ps";
$cptfile = "$root.cpt";

# set some gmtisms
$first_gmt = 1;
$first = " -X1.0 -Y1.5 -K -V > $psfile";
$middle = " -K -O -V >> $psfile";
$end = " -O -V >> $psfile";

# open the shellscript file
open(FCMD,">$cmdfile") || die "Cannot open output file $cmdfile\nMacro $program_name aborted.\n";

# write the shellscript header
print FCMD "#\n# Shellscript to create Postscript plot of data in grd file\n";
print FCMD "# Created by macro $program_name\n";

# generate color pallette table file if needed
if ($color)
	{
	print FCMD "#\n# Make color pallette table file\n";
	print FCMD "echo Making color pallette table file...\n";
	$ncpt = 15;
	@cptbr = (255, 255, 255, 255, 255, 240, 205, 138, 106,  87,  50,   0,  40,  21,  37);
	@cptbg = (255, 221, 186, 161, 189, 236, 255, 236, 235, 215, 190, 160, 127,  92,  57);
	@cptbb = (255, 171, 133,  68,  87, 121, 162, 174, 255, 255, 255, 255, 251, 236, 175);
	$dd = 1.1 * ($zmax - $zmin)/($ncpt - 1);
	$d1 = $zmin - 0.05*($zmax - $zmin);
	if (!$data_down)
		{
		foreach $i (0 .. $ncpt - 2)
			{
			$d2 = $d1 + $dd;
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
			$d1 = $d2
			}
		}
	else
		{
		for ($i = $ncpt - 2; $i >= 0; $i--)
			{
			$d2 = $d1 + $dd;
			printf FCMD "echo %.0f %d %d %d %.0f %d %d %d",
				$d1,@cptbr[$i+1],@cptbg[$i+1],@cptbb[$i+1],
				$d2,@cptbr[$i],@cptbg[$i],@cptbb[$i];
			if ($i == ($ncpt - 2))
				{
				print FCMD " >";
				}
			else
				{
				print FCMD " >>";
				}
			print FCMD " $cptfile\n";
			$d1 = $d2
			}
		}
	}

# get shading if needed
if ($shade)
	{
	printf FCMD "#\n# Get shading array\n";
	printf FCMD "echo Getting shading array...\n";
	printf FCMD "echo Running grdgradient...\n";
	printf FCMD "grdgradient $file -A$shadecontrol -G$file.grad -N -M\n";
	printf FCMD "echo Running grdhisteq...\n";
	printf FCMD "grdhisteq $file.grad -G$file.eq -N\n";
	printf FCMD "echo Running grdmath...\n";
	printf FCMD "grdmath $file.eq / 6.0 = $file.int\n";
	printf FCMD "rm -f $file.grad $file.eq\n";
	}

# set axes
if (!$threed)
	{
	$axes = "$base_tick/$base_tick";
	}
else
	{
	if ($view_az >= 0.0 && $view_az < 90.0)
		{
		$axes = "$base_tick/$base_tick::NEZ";
		}
	elsif ($view_az >= 90.0 && $view_az < 180.0)
		{
		$axes = "$base_tick/$base_tick::SEZ";
		}
	elsif ($view_az >= 180.0 && $view_az < 270.0)
		{
		$axes = "$base_tick/$base_tick::WSZ";
		}
	elsif ($view_az >= 270.0 && $view_az < 360.0)
		{
		$axes = "$base_tick/$base_tick::WNZ";
		}
	}

# do 2D plot case
if (!$threed)
	{
	if ($color)
		{
		printf FCMD "#\n# Make color image\n";
		printf FCMD "echo Running grdimage...\n";
		printf FCMD "grdimage $file -Jm$plotscale";
		printf FCMD " -R$xmin/$xmax/$ymin/$ymax -C$cptfile";
		if ($shade)
			{
			printf FCMD " -I$file.int";
			}
		if ($portrait)
		    {
		    printf FCMD " -P";
		    }
		if ($first_gmt == 1)
			{
			$first_gmt = 0;
			printf FCMD "$first\n";
			}
		else
			{
			printf FCMD "$middle\n";
			}
		}

	if ($contour)
		{
		printf FCMD "#\n# Make contour plot\n";
		printf FCMD "echo Running grdcontour...\n";
		printf FCMD "grdcontour $file -Jm$plotscale";
		printf FCMD " -R$xmin/$xmax/$ymin/$ymax";
		if ($color)
			{
			printf FCMD " -C$cptfile";
			}
		else
			{
			printf FCMD " -A$contour_int";
			}
		printf FCMD " -L$zmin/$zmax -Wc1p";
		if ($portrait)
		    {
		    printf FCMD " -P";
		    }
		if ($first_gmt == 1)
			{
			$first_gmt = 0;
			printf FCMD "$first\n";
			}
		else
			{
			printf FCMD "$middle\n";
			}
		}

	if ($color)
		{
		printf FCMD "#\n# Make color scale\n";
		printf FCMD "echo Running psscale...\n";
		printf FCMD "psscale  -C$cptfile";
		printf FCMD " -D$colorscale_offx/$colorscale_offy/$colorscale_width/$colorscale_height";
		printf FCMD "h";
		if ($portrait)
			{
			printf FCMD " -P";
			}
		printf FCMD "$middle\n";
		}

	printf FCMD "#\n# Make basemap\n";
	printf FCMD "echo Running psbasemap...\n";
	printf FCMD "psbasemap -Jm$plotscale";
	printf FCMD " -R$xmin/$xmax/$ymin/$ymax";
	printf FCMD " -B$axes -U";
	if ($portrait)
		{
		printf FCMD " -P";
		}
 	printf FCMD "$end\n";
	}

# do 3D plot case
if ($threed)
	{
	printf FCMD "#\n# Make 3D color image\n";
	printf FCMD "echo Running grdview...\n";
	printf FCMD "grdview $file -Jm$plotscale -Jz$zscale -E$viewcontrol";
	printf FCMD " -R$xmin/$xmax/$ymin/$ymax -C$cptfile";
	printf FCMD " -N$zmin/200/200/200 -Qi100";
	if ($shade)
		{
		printf FCMD " -I$file.int";
		}
	if ($portrait)
	    {
	    printf FCMD " -P";
	    }
	if ($first_gmt == 1)
		{
		$first_gmt = 0;
		printf FCMD "$first\n";
		}
	else
		{
		printf FCMD "$middle\n";
		}

	printf FCMD "#\n# Make color scale\n";
	printf FCMD "echo Running psscale...\n";
	printf FCMD "psscale  -C$cptfile";
	printf FCMD " -D$colorscale_offx/$colorscale_offy/$colorscale_width/$colorscale_height";
	printf FCMD "h";
	if ($portrait)
		{
		printf FCMD " -P";
		}
	printf FCMD "$middle\n";

	printf FCMD "#\n# Make basemap\n";
	printf FCMD "echo Running psbasemap...\n";
	printf FCMD "psbasemap -Jm$plotscale -Jz$zscale -E$viewcontrol";
	printf FCMD " -R$xmin/$xmax/$ymin/$ymax";
	printf FCMD " -B$axes -U";
	if ($portrait)
		{
		printf FCMD " -P";
		}
 	printf FCMD "$end\n";
	}

# delete surplus files
print FCMD "#\n# Delete surplus files\n";
print FCMD "echo Deleting surplus files...\n";
print FCMD "rm -f $cptfile\n";

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

die "\nAll done!\n";

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
