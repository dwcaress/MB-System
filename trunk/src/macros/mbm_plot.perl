#! /usr/local/bin/perl 
#--------------------------------------------------------------------
#    The MB-system:	mbm_plot.perl	3.00	6/18/93
#    $Id  $
#
#    Copyright (c) 1993 by 
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
#   inch page.  The plot may be color fill (-G), color shaded relief (-S), or
#   four color contoured (-C).  A gmt shellscript will be created which
#   generates a postscript image and then displays it using pageview. 
#   The -X option will cause the shellscript to be executed immediately.
#
# Usage:
#   mbm_plot -Fformat -Ifile [-Oroot -Rw/e/s/n -G -S -C -N -X -P -V -H]
#
# Author:
#   David W. Caress
#   Lamont-Doherty Earth Observatory
#   Palisades, NY  10964
#   June 17, 1993
#
# Version:
#   $Id: mbm_plot.perl,v 3.7 1993-11-27 18:31:54 caress Exp $
#
# Revisions:
#   $Log: not supported by cvs2svn $
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
&Getopts('F:f:I:i:O:o:R:r:GgCcSsNnA:a:XxPpVvHh');
$file =    ($opt_I || $opt_i);
$root =    ($opt_O || $opt_o);
$format =  ($opt_F || $opt_f);
$bounds =  ($opt_R || $opt_r);
$color =   ($opt_G || $opt_g);
$shade =   ($opt_S || $opt_s);
$contour = ($opt_C || $opt_c);
$navigation = ($opt_N || $opt_n);
$shadecontrol = ($opt_A || $opt_a);
$execute = ($opt_X || $opt_x);
$pageview = ($opt_P || $opt_p);
$help =    ($opt_H || $opt_h);
$verbose = ($opt_V || $opt_v);

# print out help message if required
if ($help)
	{
	print "\n$program_name:\n";
	print "\nPerl shellscript to generate a gmt plot of the multibeam data contained \nin the specified file.  The plot will be scaled to fit on an 8.5 X 11 \ninch page.  The plot may be color fill (-G), color shaded relief (-S), or \nfour color contoured (-C).  A gmt shellscript will be created which \ngenerates a postscript image and then displays it using pageview. \nThe -X option will cause the shellscript to be executed immediately.\n";
	print "\nUsage: $program_name -Fformat -Ifile [-Oroot -Rw/e/s/n -G -S -C -N -X -P -V -H]\n";
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
if (!$format) 
	{
	$format = "8";
	}
if (!$color && !$contour && !$shade && !$navigation)
	{
	$navigation = 1;
	}
if (!$shadecontrol)
	{
	$shadecontrol = "5/0";
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
	if ($line =~ /\s+Minimum Lon:\s+(\S+)\s+Maximum Lon:\s+(\S+)/)
		{
		print "$line";
		($xmin,$xmax) = 
			$line =~ /\s+Minimum Lon:\s+(\S+)\s+Maximum Lon:\s+(\S+)/;
		}
	if ($line =~ /\s+Minimum Lat:\s+(\S+)\s+Maximum Lat:\s+(\S+)/)
		{
		print "$line";
		($ymin,$ymax) = 
			$line =~ /\s+Minimum Lat:\s+(\S+)\s+Maximum Lat:\s+(\S+)/;
		}
	if ($line =~ /Minimum Depth:\s+(\S+)\s+Maximum Depth:\s+(\S+)/)
		{
		print "$line";
		($zmin,$zmax) = 
		$line =~ /Minimum Depth:\s+(\S+)\s+Maximum Depth:\s+(\S+)/;
		}
	}

# check that there is data
if ($xmin >= $xmax || $ymin >= $ymax || $zmin >= $zmax)
	{
	die "Does not appear to be any data in the input!\n$program_name aborted.\n"
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
if ($base_tick < 0.01667)
	{
	$base_tick = 0.01667;
	}
elsif ($base_tick < 0.03333)
	{
	$base_tick = 0.03333;
	}
elsif ($base_tick < 0.08333)
	{
	$base_tick = 0.08333;
	}
elsif ($base_tick < 0.16667)
	{
	$base_tick = 0.16667;
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


# figure out some reasonable contour intervals for swath contouring
$contour_int = 50;
$color_int = 250;
$tick_int = 250;

# come up with the filenames
$cmdfile = "$root.cmd";
$psfile = "$root.ps";
$listfile = "datalist\$\$";
$cptfile = "$root.cpt";
$navfile = "$root\$\$.nav";

# open the shellscript file
open(FCMD,">$cmdfile") || die "Cannot open output file $cmdfile\nMacro $program_name aborted.\n";

# write the shellscript header
print FCMD "#\n# Shellscript to create Postscript plot of multibeam data\n";
print FCMD "# Created by macro $program_name\n";

# generate datalist file
print FCMD "#\n# Make datalist file \n";
print FCMD "echo Making datalist file...\n";
print FCMD "echo $file $format > $listfile\n";

# generate color pallette table file if needed
if ($color || $shade)
	{
	print FCMD "#\n# Make color pallette table file\n";
	print FCMD "echo Making color pallette table file...\n";
	$ncpt = 15;
	@cptbr = (255, 255, 255, 255, 255, 240, 205, 138, 106,  87,  50,   0,  40,  21,  37);
	@cptbg = (255, 221, 186, 161, 189, 236, 255, 236, 235, 215, 190, 160, 127,  92,  57);
	@cptbb = (255, 171, 133,  68,  87, 121, 162, 174, 255, 255, 255, 255, 251, 236, 175);
	$dd = 1.1 * ($zmax - $zmin)/($ncpt - 1);
	$d1 = $zmin - 0.05*($zmax - $zmin);
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

# do swath plot if needed
if ($color || $shade)
	{
	print FCMD "#\n# Run mbswath\n";
	print FCMD "echo Running mbswath...\n";
	printf FCMD "mbswath -I%s -Jm%g", $listfile,$plotscale;
	printf FCMD " -R%.4f/%.4f/%.4f/%.4f", $xmin,$xmax,$ymin,$ymax;
	printf FCMD " -Ba%.4fg%.4f\":.Data File %s:\"", 
		$base_tick,$base_tick,$file;
	print FCMD " -C$cptfile -p1 -A1 -Q100";
	if ($shade)
		{
		print FCMD " -G$shadecontrol -Z2";
		}
	if ($portrait)
		{
		print FCMD " -P";
		}
	print FCMD " -X1 -Y1.75";
	print FCMD " -K -V > $psfile\n";
	}

# do contour plot if needed
if ($contour)
	{
	print FCMD "#\n# Run mbcontour\n";
	print FCMD "echo Running mbcontour...\n";
	printf FCMD "mbcontour -I%s -Jm%g", $listfile,$plotscale;
	printf FCMD " -R%.4f/%.4f/%.4f/%.4f", $xmin,$xmax,$ymin,$ymax;
	printf FCMD " -Ba%.4fg%.4f\":.Data File %s:\"", 
		$base_tick,$base_tick,$file;
	if ($color || $shade)
		{
		print FCMD " -C$cptfile";
		}
	else
		{
		printf FCMD " -A%.4f/%.4f/%.4f/%.4f/0.01/0.1",
			$contour_int,$color_int,$tick_int,$color_int;
		}
	print FCMD " -p1";
	if ($portrait)
		{
		print FCMD " -P";
		}
	if (!$color && !$shade)
		{
		print FCMD " -X1 -Y1 -K -V > $psfile\n";
		}
	elsif ($color || $shade)
		{
		print FCMD " -O -K -V >> $psfile\n";
		}
	elsif ($navigation)
		{
		print FCMD " -K -V >> $psfile\n";
		}
	else
		{
		print FCMD " -X1 -Y1 -V > $psfile\n";
		}
	}

# do navigation plot if needed
if ($navigation)
	{
	print FCMD "#\n# Run mblist\n";
	print FCMD "echo Running mblist...\n";
	printf FCMD "mblist -F%d -I%s -OXYU > %s\n", $format,$file,$navfile;
	print FCMD "#\n# Run pstrack\n";
	print FCMD "echo Running pstrack...\n";
	printf FCMD "pstrack %s -Jm%g", $navfile,$plotscale;
	printf FCMD " -R%.4f/%.4f/%.4f/%.4f", $xmin,$xmax,$ymin,$ymax;
	printf FCMD " -Ba%.4fg%.4f\":.Data File %s:\" -W2", 
		$base_tick,$base_tick,$file;
	printf FCMD " -Mt15ma1h";
	if ($portrait)
		{
		print FCMD " -P";
		}
	if ($color || $shade)
		{
		print FCMD " -O -K >> $psfile\n";
		}
	elsif ($contour)
		{
		print FCMD " -O >> $psfile\n";
		}
	else
		{
		print FCMD " -X1 -Y1 > $psfile\n";
		}
	}

# do scale plot if needed
if ($color || $shade)
	{
	print FCMD "#\n# Run psscale\n";
	print FCMD "echo Running psscale...\n";
	print FCMD "psscale  -C$cptfile";
	printf FCMD " -D%.4f/%.4f/%.4f/%.4fh", 
		$colorscale_offx,$colorscale_offy,
		$colorscale_width,$colorscale_height;
	print FCMD " -B\":.Depth (meters):\"";
	if ($portrait)
		{
		print FCMD " -P";
		}
	print FCMD " -O -V >> $psfile\n";
	}

# delete surplus files
print FCMD "#\n# Delete surplus files\n";
print FCMD "echo Deleting surplus files...\n";
print FCMD "rm -f $cptfile $listfile $navfile\n";

# display image on screen if desired
print FCMD "#\n# Run pageview\n";
if ($pageview)
	{
	print FCMD "echo Running pageview in background...\n";
	print FCMD "pageview $psfile &\n";
	}
else
	{
	print FCMD "#echo Running pageview in background...\n";
	print FCMD "#pageview $psfile &\n";
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
