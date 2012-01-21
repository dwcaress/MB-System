eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
                    & eval 'exec perl -S $0 $argv:q'
                         if 0;
#--------------------------------------------------------------------
#    The MB-system:	mbm_grdtiff.perl	11/3/1999
#    $Id: mbm_grdtiff.pl 1891 2011-05-04 23:46:30Z caress $
#
#    Copyright (c) 1999-2011 by
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
#   mbm_grdtiff
#
# Purpose:
#   Macro to generate a shellscript which, when executed, will 
#   generate a geographically located TIFF image of gridded 
#   data. The primary purpose of this macro is to allow the simple, 
#   semi-automated production of a nice looking image with a few 
#   command line arguments. This image can be loaded into the 
#   ArcInfo and ArcView GIS packages as geographically located 
#   coverages. Several styles of images can be generated, including 
#   color fill and shaded relief maps. The available options mirror 
#   a subset of the options in mbm_grdplot, allowing users to easily 
#   generate equivalent Postscript plots and TIFF images of gridded 
#   data. 
#
# Basic Usage: 
#   mbm_grdtiff -Ifile [-Amagnitude[/azimuth/elevation] 
#            -Gcolor_mode -H -Kintensity_file -Oroot
#            -S[color/shade] -V 
#            -Wcolor_style[/pallette[/ncolors]] ]
#
# Additional Options:
#            [-Dflipcolor/flipshade -MGSscalefactor 
#            -Q -Rw/e/s/n -X -Y -Zmin/max]
#
# Author:
#   David W. Caress
#   Lamont-Doherty Earth Observatory
#   Palisades, NY  10964
#   October 19, 1994
#
# Version:
#   $Id: mbm_grdtiff.pl 1891 2011-05-04 23:46:30Z caress $
#
# Revisions:
#   $Log: mbm_grdtiff.perl,v $
#   Revision 5.5  2006/07/05 19:50:21  caress
#   Working towards 5.1.0beta
#
#   Revision 5.4  2006/02/10 01:27:40  caress
#   Fixed parsing of grdinfo output to handle changes to GMT4.1.
#
#   Revision 5.3  2003/04/17 20:42:48  caress
#   Release 5.0.beta30
#
#   Revision 5.2  2002/08/02 01:00:05  caress
#   Release 5.0.beta22
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
# Revision 4.0  1999/12/29  00:17:04  caress
# Initial revision.
#
#
#
#
$program_name = "mbm_grdtiff";

# set degree to radians conversion
$DTR = 3.1415926 / 180.0;

# set default number of colors
$ncpt = 11;

# define color pallettes

@color_pallette_names = (   "Haxby Colors", 
			    "High Intensity Colors", 
			    "Low Intensity Colors", 
			    "Grayscale", 
			    "Uniform Gray",
			    "Uniform Black",
			    "Uniform White");

# original Haxby color pallette
#	$ncolors = 15;
#	@cptbr = (255, 255, 255, 255, 255, 240, 205, 138, 106,  87,  50,   0,  40,  21,  37);
#	@cptbg = (255, 221, 186, 161, 189, 236, 255, 236, 235, 215, 190, 160, 127,  92,  57);
#	@cptbb = (255, 171, 133,  68,  87, 121, 162, 174, 255, 255, 255, 255, 251, 236, 175);
#                 use       use  use  use  use  use  use  use       use       use       use

# color pallette 1 - Haxby Color Table
	@cptbr1 = (255, 255, 255, 255, 240, 205, 138, 106,  50,  40,  37);
	@cptbg1 = (255, 186, 161, 189, 236, 255, 236, 235, 190, 127,  57);
	@cptbb1 = (255, 133,  68,  87, 121, 162, 174, 255, 255, 251, 175);

# color pallette 2 - High Intensity Colors
	@cptbr2 = (255, 255, 255, 255, 128,   0,   0,   0,   0, 128, 255);
	@cptbg2 = (  0,  64, 128, 255, 255, 255, 255, 128,   0,   0,   0);
	@cptbb2 = (  0,   0,   0,   0,   0,   0, 255, 255, 255, 255, 255);

# color pallette 3 - Low Intensity Colors
	@cptbr3 = (200, 194, 179, 141,  90,   0,   0,   0,   0,  90, 141);
	@cptbg3 = (  0,  49,  90, 141, 179, 200, 141,  90,   0,   0,   0);
	@cptbb3 = (  0,   0,   0,   0,   0,   0, 141, 179, 200, 179, 141);

# color pallette 4 - Grayscale
	@cptbr4 = (255, 230, 204, 179, 153, 128, 102,  77,  51,  26,   0);
	@cptbg4 = (255, 230, 204, 179, 153, 128, 102,  77,  51,  26,   0);
	@cptbb4 = (255, 230, 204, 179, 153, 128, 102,  77,  51,  26,   0);

# color pallette 5 - Uniform Grayscale
	@cptbr5 = (128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128);
	@cptbg5 = (128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128);
	@cptbb5 = (128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128);

# color pallette 6 - Uniform Black
	@cptbr6 = (  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0);
	@cptbg6 = (  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0);
	@cptbb6 = (  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0);

# color pallette 7 - Uniform White
	@cptbr7 = (255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255);
	@cptbg7 = (255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255);
	@cptbb7 = (255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255);

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

# Deal with command line arguments (accept but do not necessarily use any argument
# accepted by mbm_grdplot)
$command_line = "@ARGV";
&MBGetopts('A:a:D%d%G%g%HhI:i:J:j:K:k:L:l:M+m+O:o:P:p:QqR:r:S%s%TtU:u:VvW:w:XxYyZ:z:');
$shade_control = 	($opt_A || $opt_a);
$color_flip_mode = 	($flg_D || $flg_d);
$color_flip_control = 	($opt_D || $opt_d);
$color_mode =   	($opt_G || $opt_g || $flg_G || $flg_g);
$help =    		($opt_H || $opt_h);
$file_data =    	($opt_I || $opt_i);
$file_intensity =    	($opt_K || $opt_k);
$misc = 		($opt_M || $opt_m);
$root =    		($opt_O || $opt_o);
$no_view_tiff = 	($opt_Q || $opt_q);
$bounds = 		($opt_R || $opt_r);
$stretch_mode = 	($flg_S || $flg_s);
$stretch_control = 	($opt_S || $opt_s);
$verbose = 		($opt_V || $opt_v);
$color_control = 	($opt_W || $opt_w);
$execute = 		($opt_X || $opt_x);
$no_nice_color_int = 	($opt_Y || $opt_y);
$zbounds = 		($opt_Z || $opt_z);

# print out help message if required
if ($help)
	{
	print "\n$program_name:\n";
	print "\nVersion: $Id: mbm_grdtiff.pl 1891 2011-05-04 23:46:30Z caress $\n";
        print "\nMacro to generate a shellscript which, when executed, will \n";
        print "generate a geographically located TIFF image of gridded \n";
        print "data. The primary purpose of this macro is to allow the simple, \n";
        print "semi-automated production of a nice looking image with a few \n";
        print "command line arguments. This image can be loaded into the \n";
        print "ArcInfo and ArcView GIS packages as geographically located \n";
        print "coverages. Several styles of images can be generated, including \n";
        print "color fill and shaded relief maps. The available options mirror \n";
        print "a subset of the options in mbm_grdplot, allowing users to easily \n";
        print "generate equivalent Postscript plots and TIFF images of gridded \n";
        print "data. \n";
	print "\nBasic Usage: \n";
	print "\t$program_name -Ifile [-Amagnitude[/azimuth/elevation] \n";
	print "\t\t-Gcolor_mode -H -Kintensity_file -Oroot -S[color/shade]\n";
	print "\t\t-V -Wcolor_style[/pallette[/ncolors]] ]\n";
	print "Additional Options:\n";
	print "\t\t[-Dflipcolor/flipshade\n";
	print "\t\t-MGSscalefactor -Q -Rw/e/s/n -X -Y -Zmin/max]\n";
	exit 0;
	}

# check for input file
if (!$file_data)
	{
	print "\a";
	die "\nNo input file specified!\n$program_name aborted\n";
	}
elsif (! -r $file_data)
	{
	print "\a";
	die "\nSpecified input file $file_data cannot be opened!\n$program_name aborted\n";
	}
if ($file_intensity && ! -r $file_intensity)
	{
	print "\a";
	die "\nSpecified intensity input file $file_intensity cannot be opened!\n$program_name aborted\n";
	}
if ($color_mode == 3 && !$file_intensity)
	{
	print "\a";
	die "\nShading with intensity file set but no intensity input file specified!\n$program_name aborted\n";
	}

# parse misc commands
if ($misc)
	{
	@misc_cmd = split(/:/, $misc);
	foreach $cmd (@misc_cmd) {

		# deal with general options
		##############################

		# set GMT default values
		if ($cmd =~ /^[Gg][Dd]./)
			{
			($gmt_def) = $cmd =~ 
				/^[Gg][Dd](\S+)/;
			push(@gmt_defs, $gmt_def);
			}

		# set data scaling
		if ($cmd =~ /^[Gg][Ss]./)
			{
			($data_scale) = $cmd =~ 
				/^[Gg][Ss](\S+)/;
			}
		}
	}

# set plot mode
if (!$root)
	{
	if ($file_data =~ /.=./)
		{
		($root) = $file_data =~ /^(\S+)=./;
		}
	else
		{
		$root = $file_data;
		}
	}
if (!$color_mode)
	{
	$color_mode = 1;
	}
if ($shade_control)
	{
	if ($shade_control =~ /^(\S+)\/(\S+)\/(\S+)/)
		{
		($magnitude, $azimuth, $elevation) = 
			$shade_control =~ /^(\S+)\/(\S+)\/(\S+)/;
		}
	elsif ($shade_control =~ /^(\S+)\/(\S+)/)
		{
		($magnitude, $azimuth) = 
			$shade_control =~ /^(\S+)\/(\S+)/;
		}
	elsif ($shade_control =~ /^(\S+).*/)
		{
		($magnitude) = 
			$shade_control =~ /^(\S+).*/;
		}
	}
if ($file_intensity && !$color_mode)
	{
	$color_mode = 3;
	}
if ($color_mode == 2 && !$azimuth)
	{
	$azimuth = 0;
	}
if ($color_mode == 2 && !$magnitude)
	{
	$magnitude = 1.0;
	}
elsif ($color_mode == 3 && !$magnitude)
	{
	$magnitude = -0.4;
	}
if ($color_mode == 2 && !$elevation)
	{
	$elevation = 30.0;
	}
if ($color_mode >= 4 && !$magnitude)
	{
	$magnitude = 1.0;
	}
if ($color_control)
	{
	if (-e $color_control)
		{
		$file_cpt = $color_control;
		$color_style = 1;
		$color_pallette = 1;
		$ncolors = $ncpt;
		}
	elsif ($color_control =~ /\S+\/\S+\/\S+/)
		{
		($color_style, $color_pallette, $ncolors) 
			= $color_control =~  /(\S+)\/(\S+)\/(\S+)/;
		if ($color_pallette < 1 
			|| $color_pallette > 7)
			{
			$color_pallette = 1;
			}
		if ($ncolors < 2)
			{
			$ncolors = 2;
			}
		}
	elsif ($color_control =~ /\S+\/\S+/)
		{
		($color_style, $color_pallette) = $color_control
			=~  /(\S+)\/(\S+)/;
		if ($color_pallette < 1 
			|| $color_pallette > 7)
			{
			$color_pallette = 1;
			}
		$ncolors = $ncpt;
		}
	else
		{
		$color_style = $color_control;
		$color_pallette = 1;
		$ncolors = $ncpt;
		}
	}
else
	{
	$color_style = 1;
	$color_pallette = 1;
	$ncolors = $ncpt;
	}
if ($color_flip_control)
	{
	if ($color_flip_control =~ /^\S+\/\S+/)
		{
		($color_flip, $shade_flip) = $color_flip_control
			=~ /^(\S+)\/(\S+)/;
		}
	elsif ($color_flip_control =~ /^\S+/)
		{
		($color_flip) = $color_flip_control
			=~ /^(\S+)/;
		}
	}
elsif ($color_flip_mode)
	{
	$color_flip = 1;
	}
if ($color_mode && $shade_flip)
	{
	$magnitude = -1 * $magnitude;
	}
if ($stretch_control)
	{
	if ($stretch_control =~ /^\S+\/\S+/)
		{
		($stretch_color, $stretch_shade) = $stretch_control
			=~ /^(\S+)\/(\S+)/;
		}
	elsif ($stretch_control =~ /^\S+/)
		{
		($stretch_color) = $stretch_control
			=~ /^(\S+)/;
		}
	}
elsif ($stretch_mode)
	{
	$stretch_color = 1;
	}

# set page size
if (!$pagesize)
	{
	$pagesize = "a";
	}
else
	{
	$pagesize =~ tr/A-Z/a-z/;
	if (!$page_width_in{$pagesize})
		{
		$pagesize = "a";
		}
	}

# get postscript viewer
# check environment variable
if ($ENV{"MB_IMG_VIEWER"})
	{
	$tiff_viewer = $ENV{"MB_IMG_VIEWER"};
	}

# check for .mbio_defaults file
if (!$tiff_viewer)
	{
	$home = $ENV{"HOME"};
	$mbdef = "$home/.mbio_defaults";
	if (open(MBDEF,"<$mbdef"))
		{
		while (<MBDEF>)
			{
			if (/img viewer:\s+(\S+)/)
				{
				($img_viewer) = /img viewer:\s+(\S+)/;
				}
			}
		close MBDEF;
		}
	}
# else just set it to xv
if (!$img_viewer)
	{
	$img_viewer = "xv";
	}

# see if data file is a grd file or a list of grdfiles
@grdinfo = `grdinfo $file_data 2>&1`;
while (@grdinfo)
	{
	$line = shift @grdinfo;
	if ($line =~ /\S+\s+x_min:\s+(\S+)\s+x_max:\s+(\S+)\s+x_inc:/)
		{
		$file_data_no_list = 1;
		}
	}
if ($file_data_no_list)
	{
	push(@files_data, $file_data);
	}
else
	{
	if (open(FILEDATA,"<$file_data"))
		{
		while (<FILEDATA>)
			{
			chop($_);
			push(@files_data, $_);
			}
		close FILEDATA;
		}
	}

# see if intensity file is a grd file or a list of grdfiles
if ($file_intensity)
	{
	@grdinfo = `grdinfo $file_intensity 2>&1`;
	while (@grdinfo)
		{
		$line = shift @grdinfo;
		if ($line =~ /\S+\s+x_min:\s+(\S+)\s+x_max:\s+(\S+)\s+x_inc:/)
			{
			$file_intensity_no_list = 1;
			}
		}
	if ($file_intensity_no_list)
		{
		push(@files_intensity, $file_intensity);
		}
	else
		{
		if (open(FILEDATA,"<$file_intensity"))
			{
			while (<FILEDATA>)
				{
				chop($_);
				push(@files_intensity, $_);
				}
			close FILEDATA;
			}
		}
	}

# get limits of files using grdinfo
if (!$bounds || !$zbounds)
	{
	foreach $file_grd (@files_data) {

	if ($bounds)
		{
		@grdinfo = `mbm_grdinfo -I$file_grd -R$bounds`;
		}
	else
		{
		@grdinfo = `grdinfo $file_grd`;
		}
	while (@grdinfo)
		{
		$line = shift @grdinfo;
		if ($line =~ 
			/\s+Projection: UTM Zone \S+/)
			{
			($utm_zone) = $line =~ 
				/\s+Projection: UTM Zone (\S+)/;
			$gridprojected = 1;
			}
		if ($line =~ 
			/\s+Projection: Geographic/)
			{
			$gridprojected = 0;
			}
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
		}

	if (!$first_grd)
		{
		$first_grd = 1;
		$xmin = $xmin_f;
		$xmax = $xmax_f;
		$ymin = $ymin_f;
		$ymax = $ymax_f;
		$zmin = $zmin_f;
		$zmax = $zmax_f;
		}
	else
		{
		$xmin = &min($xmin, $xmin_f);
		$xmax = &max($xmax, $xmax_f);
		$ymin = &min($ymin, $ymin_f);
		$ymax = &max($ymax, $ymax_f);
		$zmin = &min($zmin, $zmin_f);
		$zmax = &max($zmax, $zmax_f);
		}

	# check that there is data
	if ($xmin_f >= $xmax_f || $ymin_f >= $ymax_f || $zmin_f >= $zmax_f)
		{
		print "\a";
		die "The program grdinfo does not appear to have worked \nproperly with file $file_grd!\n$program_name aborted.\n"
		}
	}
	}

# use user defined geographic limits
if ($bounds)
	{
	if ($bounds =~ /^\S+\/\S+\/\S+\/\S+r$/)
		{
		($xmin_raw,$ymin_raw,$xmax_raw,$ymax_raw) = $bounds =~ 
			/^(\S+)\/(\S+)\/(\S+)\/(\S+)r$/;
		$xmin = &GetDecimalDegrees($xmin_raw);
		$xmax = &GetDecimalDegrees($xmax_raw);
		$ymin = &GetDecimalDegrees($ymin_raw);
		$ymax = &GetDecimalDegrees($ymax_raw);
		$use_corner_points = 1;
		$bounds_plot = $bounds;
		}
	elsif ($bounds =~ /^\S+\/\S+\/\S+\/\S+$/)
		{
		($xmin_raw,$xmax_raw,$ymin_raw,$ymax_raw) = $bounds =~ 
			/(\S+)\/(\S+)\/(\S+)\/(\S+)/;
		$xmin = &GetDecimalDegrees($xmin_raw);
		$xmax = &GetDecimalDegrees($xmax_raw);
		$ymin = &GetDecimalDegrees($ymin_raw);
		$ymax = &GetDecimalDegrees($ymax_raw);
		$bounds_plot = $bounds;
		}
	elsif ($bounds =~ /^r$/)
		{
		$use_corner_points = 1;
		$xmin = $xmin;
		$xmax = $xmax;
		$ymin = $ymin;
		$ymax = $ymax;
		$bounds_plot = sprintf ("%1.8g/%1.8g/%1.8g/%1.8gr",
			$xmin, $ymin, $xmax, $ymax);
		}
	}

# set grid to projected if outside geographic bounds
if ($xmin < -360.0 || $xmax > 360.0
	|| $ymin < -90.0 || $ymax > 90.0)
	{
	$gridprojected = 1;
	}

# set bounds string for plotting if not already set
if (!$bounds_plot)
	{
	$bounds_plot = sprintf ("%1.8g/%1.8g/%1.8g/%1.8g",
		$xmin, $xmax, $ymin, $ymax);
	}

# use user defined data limits
if ($zbounds)
	{
	($zmin,$zmax) = $zbounds =~ /(\S+)\/(\S+)/;
	}

# check that there is data
if ($xmin >= $xmax || $ymin >= $ymax || $zmin >= $zmax)
	{
	print "\a";
	die "Improper data limits: x: $xmin $xmax  y: $ymin $ymax  z: $zmin $zmax\n$program_name aborted.\n"
	}

# apply rescaling to zmin and zmax if needed
if ($data_scale)
	{
	$zmin = $data_scale * $zmin;
	$zmax = $data_scale * $zmax;
	if ($zmin > $zmax)
		{
		$tmp = $zmin;
		$zmin = $zmax;
		$zmax = $tmp;
		}
	}

# figure out reasonable color intervals
$dzz = ($zmax - $zmin); 
$contour_int = 0.0;
if ($dzz > 0)
	{
	$base = int((log($dzz) / log(10.)) + 0.5);
	$contour_int = (10 ** $base) / 10.0;
	if ($dzz / $contour_int < 10)
		{
		$contour_int = $contour_int / 4;
		}
	elsif ($dzz / $contour_int < 20)
		{
		$contour_int = $contour_int / 2;
		}
	}
if ($color_mode && $color_style == 1)
	{
	$ncolors_use = $ncolors;
	}
elsif ($color_mode)
	{
	$ncolors_use = $ncolors + 1;
	}
if ($color_mode && !$no_nice_color_int && $dzz > 0)
	{
	$start_int = $contour_int / 2;
	$multiplier = int($dzz / ($ncolors_use - 1) / $start_int) + 1;
	$color_int = $multiplier * $start_int;
	if ($zmin < 0.0)
		{
		$color_start = $color_int * (int($zmin / $color_int) - 1);
		}
	else
		{
		$color_start = $color_int * int($zmin / $color_int);
		}
	$color_end = $color_start + $color_int * ($ncolors_use - 1);
	if ($color_end < $zmax)
		{
		$multiplier = $multiplier + 1;
		$color_int = $multiplier * $start_int;
		}
	if ($zmin < 0.0)
		{
		$color_start = $color_int * (int($zmin / $color_int) - 1);
		}
	else
		{
		$color_start = $color_int * int($zmin / $color_int);
		}
	$color_end = $color_start + $color_int * ($ncolors_use - 1);
	}
elsif ($color_mode)
	{
	$color_int = ($zmax - $zmin)/($ncolors_use - 1);
	$color_start = $zmin;
	$color_end = $color_start + $color_int * ($ncolors_use - 1);
	}

# get colors to use by interpolating defined color pallette
if ($color_mode)
	{
	# set selected color pallette
	eval "\@cptbr = \@cptbr$color_pallette;";
	eval "\@cptbg = \@cptbg$color_pallette;";
	eval "\@cptbb = \@cptbb$color_pallette;";

	# interpolate colors
	for ($i = 0; $i < $ncolors; $i++)
		{
		$xx = ($ncpt - 1) * $i / ($ncolors - 1);
		$i1 = int($xx);
		$i2 = $i1 + 1;
		$red = $cptbr[$i1] 
			+ ($cptbr[$i2] - $cptbr[$i1])
			* ($xx - $i1) / ($i2 - $i1);
		$green = $cptbg[$i1] 
			+ ($cptbg[$i2] - $cptbg[$i1])
			* ($xx - $i1) / ($i2 - $i1);
		$blue = $cptbb[$i1] 
			+ ($cptbb[$i2] - $cptbb[$i1])
			* ($xx - $i1) / ($i2 - $i1);
		push (@cptr, $red);
		push (@cptg, $green);
		push (@cptb, $blue);
		}
	}

# come up with the filenames
$cmdfile = "$root" . "_tiff.cmd";
$tiffile = "$root.tif";
if ($color_mode && $file_cpt)
	{
	$cptfile = $file_cpt;
	}
else
	{
	$cptfile = "$root.cpt";
	}
$gmtfile = "gmtdefaults\$\$";

# set macro gmt default settings
$gmt_def = "COLOR_BACKGROUND/0/0/0";
push(@gmt_macro_defs, $gmt_def);
$gmt_def = "COLOR_FOREGROUND/255/255/255";
push(@gmt_macro_defs, $gmt_def);
$gmt_def = "COLOR_NAN/255/255/255";
push(@gmt_macro_defs, $gmt_def);
if ($gmt_version eq "3.0"
	|| $gmt_version eq "3.1")
	{
	$gmt_def = "DEGREE_FORMAT/3";
	push(@gmt_macro_defs, $gmt_def);
	}
else
	{
	$gmt_def = "PLOT_DEGREE_FORMAT/ddd:mm";
	push(@gmt_macro_defs, $gmt_def);
	}

# open the shellscript file
if (!open(FCMD,">$cmdfile"))
	{
	print "\a";
	die "Cannot open output file $cmdfile\nMacro $program_name aborted.\n";
	}

# write the shellscript header
print FCMD "#! /bin/csh -f\n";
print FCMD "#\n# Shellscript to create TIFF image of data in grd file\n";
print FCMD "# Created by macro $program_name\n";
print FCMD "#\n# This shellscript created by following command line:\n";
print FCMD "# $program_name $command_line\n";

# Define shell variables
print FCMD "#\n# Define shell variables used in this script:\n";
print FCMD "set TIFF_FILE       = $tiffile\n";
print FCMD "set CPT_FILE        = $cptfile\n";
print FCMD "set MAP_REGION      = $bounds_plot\n";

# Reset GMT defaults, saving old defaults
print FCMD "#\n# Save existing GMT defaults\n";
print FCMD "echo Saving GMT defaults...\n";
print FCMD "gmtdefaults -L > $gmtfile\n";
print FCMD "#\n# Set new GMT defaults\n";
print FCMD "echo Setting new GMT defaults...\n";
foreach $gmt_def (@gmt_macro_defs) {
	($gmt_par, $gmt_var) = $gmt_def =~ /^([^\/]+)\/(.+)/;
	print FCMD "gmtset $gmt_par $gmt_var\n";
	}

# Reset GMT defaults as per user commands
if (@gmt_defs)
	{
	print FCMD "#\n# Set user defined GMT defaults\n";
	print FCMD "echo Setting user defined GMT defaults...\n";
	foreach $gmt_def (@gmt_defs) {
		($gmt_par, $gmt_var) = $gmt_def =~ /^([^\/]+)\/(.+)/;
		print FCMD "gmtset $gmt_par $gmt_var\n";
		}
	}

# generate color pallette table file if needed
if ($color_mode && !$file_cpt)
	{
	# set slope cpt
	if ($color_mode == 4)
		{
		foreach $i (0 .. $ncolors - 1) {
			$d1 = $magnitude * $i / ($ncolors - 1);
			push(@hist, $d1);
			}
		}

	# break data distribution up into equal size 
	# regions using grdhisteq on first data file
	if ($stretch_color)
		{
		if ($verbose) 
			{
			print "Running grdhisteq...\n";
			}
		$ncolors_minus = $ncolors - 1;
		@grdhisteq = `grdhisteq $files_data[0] -C$ncolors_minus -D`;
		foreach $d (@grdhisteq) {
			($d1, $d2) = $d =~ /(\S+)\s+(\S+).*/;
			push(@hist, $d1);
			}
		push(@hist, $d2);

		# rescale hist values if needed
		if ($data_scale)
			{
			foreach $i (0 .. $ncolors - 1) {
				$hist[$i] = $data_scale * $hist[$i];
				}
			}

		# make sure hist encompasses all data
		if ($zmin < $hist[0])
			{
			$hist[0] = $zmin;
			}
		if ($zmax > $hist[scalar(@hist)-1])
			{
			$hist[scalar(@hist)-1] = $zmax;
			}
		$hist[0] = $hist[0] - 0.01*$dzz;
		$hist[scalar(@hist)-1] = 
			$hist[scalar(@hist)-1] + 0.01*$dzz;
		}

	# generate cpt file
	print FCMD "#\n# Make color pallette table file\n";
	print FCMD "echo Making color pallette table file...\n";
	if ($color_mode == 4 || $stretch_color)
		{
		$d1 = shift @hist;
		}
	else
		{
		$d1 = $color_start;
		}
	if ($color_style == 1 && $color_flip)
		{
		foreach $i (0 .. $ncolors - 2)
			{
			if ($color_mode == 4 || $stretch_color)
				{
				$d2 = shift @hist;
				}
			else
				{
				$d2 = $d1 + $color_int;
				}
			printf FCMD "echo %6g %3d %3d %3d %6g %3d %3d %3d",
				$d1,@cptr[$i],@cptg[$i],@cptb[$i],
				$d2,@cptr[$i+1],@cptg[$i+1],@cptb[$i+1];
			if ($i == 0)
				{
				print FCMD " >";
				}
			else
				{
				print FCMD " >>";
				}
			print FCMD " \$CPT_FILE\n";
			$d1 = $d2
			}
		}
	elsif ($color_style == 1)
		{
		for ($i = $ncolors - 2; $i >= 0; $i--)
			{
			if ($color_mode == 4 || $stretch_color)
				{
				$d2 = shift @hist;
				}
			else
				{
				$d2 = $d1 + $color_int;
				}
			printf FCMD "echo %6g %3d %3d %3d %6g %3d %3d %3d",
				$d1,@cptr[$i+1],@cptg[$i+1],@cptb[$i+1],
				$d2,@cptr[$i],@cptg[$i],@cptb[$i];
			if ($i == ($ncolors - 2))
				{
				print FCMD " >";
				}
			else
				{
				print FCMD " >>";
				}
			print FCMD " \$CPT_FILE\n";
			$d1 = $d2
			}
		}
	elsif ($color_flip)
		{
		foreach $i (0 .. $ncolors - 1)
			{
			if ($color_mode == 4 || $stretch_color)
				{
				$d2 = shift @hist;
				}
			else
				{
				$d2 = $d1 + $color_int;
				}
			printf FCMD "echo %6g %3d %3d %3d %6g %3d %3d %3d",
				$d1,@cptr[$i],@cptg[$i],@cptb[$i],
				$d2,@cptr[$i],@cptg[$i],@cptb[$i];
			if ($i == 0)
				{
				print FCMD " >";
				}
			else
				{
				print FCMD " >>";
				}
			print FCMD " \$CPT_FILE\n";
			$d1 = $d2
			}
		}
	else
		{
		for ($i = $ncolors - 1; $i >= 0; $i--)
			{
			if ($color_mode == 4 || $stretch_color)
				{
				$d2 = shift @hist;
				}
			else
				{
				$d2 = $d1 + $color_int;
				}
			printf FCMD "echo %6g %3d %3d %3d %6g %3d %3d %3d",
				$d1,@cptr[$i],@cptg[$i],@cptb[$i],
				$d2,@cptr[$i],@cptg[$i],@cptb[$i];
			if ($i == ($ncolors - 2))
				{
				print FCMD " >";
				}
			else
				{
				print FCMD " >>";
				}
			print FCMD " \$CPT_FILE\n";
			$d1 = $d2
			}
		}
	}

# now loop over all of the grid files
foreach $file_grd (@files_data) {

# get intensity file
if (@files_intensity)
	{
	$file_int = shift @files_intensity;
	}

# Define shell variables
print FCMD "#\n# Define data files to be plotted:\n";
print FCMD "set DATA_FILE        = $file_grd\n";
print FCMD "set INTENSITY_FILE   = $file_int\n";

# scale data if needed
$file_use = "\$DATA_FILE";
if ($data_scale)
	{
	printf FCMD "#\n# Rescale data\n";
	printf FCMD "echo Rescaling data by $data_scale...\n";
	printf FCMD "echo Running grdmath...\n";
	printf FCMD "grdmath \$DATA_FILE $data_scale x = \$DATA_FILE.scale\n";
	$file_use = "\$DATA_FILE.scale";
	}

# get shading by illumination if needed
#if ($color_mode == 2)
#	{
#	printf FCMD "#\n# Get shading array\n";
#	printf FCMD "echo Getting shading array...\n";
#	printf FCMD "echo Running grdgradient...\n";
#	printf FCMD "grdgradient $file_use -A$azimuth -G\$DATA_FILE.grad -N";
#	if (!$gridprojected)
#		{
#		printf FCMD " -M";
#		}
#	printf FCMD "\n";
#	printf FCMD "echo Running grdhisteq...\n";
#	printf FCMD "grdhisteq \$DATA_FILE.grad -G\$DATA_FILE.eq -N\n";
#	printf FCMD "echo Running grdmath...\n";
#	printf FCMD "grdmath \$DATA_FILE.eq $magnitude x = \$DATA_FILE.int\n";
#	printf FCMD "/bin/rm -f \$DATA_FILE.grad \$DATA_FILE.eq\n";
#	$file_shade = "\$DATA_FILE.int";
#	}

# get shading by illumination if needed
if ($color_mode == 2)
	{
	# Compute lighting vector from sun azimuth and elevation
	$light_x = sin($DTR * $azimuth) * cos($DTR * $elevation);
	$light_y = cos($DTR * $azimuth) * cos($DTR * $elevation);
	$light_z = sin($DTR * $elevation);

	printf FCMD "#\n# Get shading array\n";
	printf FCMD "echo Getting shading array...\n";
	printf FCMD "echo Running grdgradient to get x component of the gradient...\n";
	printf FCMD "grdgradient $file_use -A90 -G\$DATA_FILE.drvx";
	if (!$gridprojected)
		{
		printf FCMD " -M";
		}
	printf FCMD "\n";
	printf FCMD "echo Running grdgradient to get y component of the gradient...\n";
	printf FCMD "grdgradient $file_use -A0 -G\$DATA_FILE.drvy";
	if (!$gridprojected)
		{
		printf FCMD " -M";
		}
	printf FCMD "\n";

	printf FCMD "echo Running grdmath to get adjusted x gradient...\n";
	printf FCMD "grdmath \$DATA_FILE.drvx $magnitude MUL 10 MUL = \$DATA_FILE.magx\n";
	printf FCMD "echo Running grdmath to get adjusted y gradient...\n";
	printf FCMD "grdmath \$DATA_FILE.drvy $magnitude MUL 10 MUL = \$DATA_FILE.magy\n";

	printf FCMD "echo Running grdmath to get normalization factor...\n";
	printf FCMD "grdmath \$DATA_FILE.magx 2.0 POW \\\n";
		printf FCMD "\t\$DATA_FILE.magy 2.0 POW ADD \\\n";
		printf FCMD "\t1.0 ADD SQRT = \$DATA_FILE.denom\n";

	printf FCMD "echo Running grdmath to get normalized x gradient...\n";
	printf FCMD "grdmath \$DATA_FILE.magx \$DATA_FILE.denom DIV = \$DATA_FILE.normx\n";
	printf FCMD "echo Running grdmath to get normalized y gradient...\n";
	printf FCMD "grdmath \$DATA_FILE.magy \$DATA_FILE.denom DIV = \$DATA_FILE.normy\n";
	printf FCMD "echo Running grdmath to get normalized z gradient...\n";
	printf FCMD "grdmath 1.0 \$DATA_FILE.denom DIV = \$DATA_FILE.normz\n";

	printf FCMD "echo Running grdmath to apply lighting vector to normalized gradient...\n";
	printf FCMD "grdmath \$DATA_FILE.normx $light_x MUL \\\n";
		printf FCMD "\t\$DATA_FILE.normy $light_y MUL ADD \\\n";
		printf FCMD "\t\$DATA_FILE.normz $light_z MUL ADD -0.5 ADD = \$DATA_FILE.int\n";

	printf FCMD "/bin/rm -f \$DATA_FILE.drvx \$DATA_FILE.drvy \\\n";
	printf FCMD "\t\$DATA_FILE.magx \$DATA_FILE.magy \\\n";
	printf FCMD "\t\$DATA_FILE.denom \$DATA_FILE.normx \\\n";
	printf FCMD "\t\$DATA_FILE.normy \$DATA_FILE.normz\n";
	$file_shade = "\$DATA_FILE.int";
	}

# get equalized shading by intensity file if needed
if ($color_mode == 3 && $file_int && $stretch_shade)
	{
	printf FCMD "#\n# Get shading array\n";
	printf FCMD "echo Getting shading array...\n";
	printf FCMD "echo Running grdhisteq...\n";
	printf FCMD "grdhisteq \$INTENSITY_FILE -G\$INTENSITY_FILE.eq -N\n";
	printf FCMD "echo Running grdmath...\n";
	printf FCMD "grdmath \$INTENSITY_FILE.eq $magnitude x = \$INTENSITY_FILE.int\n";
	printf FCMD "/bin/rm -f \$INTENSITY_FILE.eq\n";
	$file_shade = "\$INTENSITY_FILE.int";
	}

# get shading by unaltered intensity file
elsif ($color_mode == 3 && $file_int)
	{
	$file_shade = "\$INTENSITY_FILE";
	}

# get color by slope magnitude if needed
elsif ($color_mode >= 4)
	{
	printf FCMD "#\n# Get slope array\n";
	printf FCMD "echo Getting slope array...\n";
	printf FCMD "echo Running grdgradient to get x component of the gradient...\n";
	printf FCMD "grdgradient $file_use -A90 -G\$DATA_FILE.drvx";
	if (!$gridprojected)
		{
		printf FCMD " -M";
		}
	printf FCMD "\n";
	printf FCMD "echo Running grdgradient to get y component of the gradient...\n";
	printf FCMD "grdgradient $file_use -A0 -G\$DATA_FILE.drvy";
	if (!$gridprojected)
		{
		printf FCMD " -M";
		}
	printf FCMD "\n";

	printf FCMD "echo Running grdmath to get slope magnitude...\n";
	printf FCMD "grdmath \$DATA_FILE.drvx 2.0 POW \\\n";
	printf FCMD "\t\$DATA_FILE.drvy 2.0 POW ADD SQRT \\\n";
	if ($color_mode == 5)
		{
		printf FCMD "\t$magnitude MUL \\\n";
		}
	printf FCMD "\t= \$DATA_FILE.slope\n";

	printf FCMD "/bin/rm -f \$DATA_FILE.drvx \$DATA_FILE.drvy \n";
	$file_slope = "\$DATA_FILE.slope";
	}

# do grdtiff image
if ($color_mode)
	{
	printf FCMD "#\n# Make tiff image\n";
	printf FCMD "echo Running mbgrdtiff...\n";
	if ($color_mode == 4)
		{
		printf FCMD "mbgrdtiff -I $file_slope \\\n\t";
		}
	else
		{
		printf FCMD "mbgrdtiff -I $file_use \\\n\t";
		}
	printf FCMD "-O \$TIFF_FILE \\\n\t";
	printf FCMD "-R\$MAP_REGION \\\n\t";
	printf FCMD "-C \$CPT_FILE \\\n\t";
	if ($color_mode == 2 || $color_mode == 3)
		{
		printf FCMD "-K $file_shade \\\n\t";
		}
	elsif ($color_mode == 5)
		{
		printf FCMD "-K $file_slope \\\n\t";
		}
	printf FCMD "-V \n";
	}
} # end loop over grd data files

# delete surplus files
print FCMD "#\n# Delete surplus files\n";
print FCMD "echo Deleting surplus files...\n";
if (!$file_cpt)
	{
	print FCMD "/bin/rm -f \$CPT_FILE\n";
	}
if ($data_scale)
	{
	printf FCMD "/bin/rm -f $file_use\n";
	}

# reset GMT defaults
print FCMD "#\n# Reset GMT default fonts\n";
print FCMD "echo Resetting GMT fonts...\n";
print FCMD "/bin/mv $gmtfile .gmtdefaults\n";

# display image on screen if desired
print FCMD "#\n# Run $img_viewer\n";
if ($no_view_tiff)
	{
	print FCMD "#echo Running $img_viewer in background...\n";
	print FCMD "#$img_viewer $tiffile &\n";
	}
else
	{
	print FCMD "echo Running $img_viewer in background...\n";
	print FCMD "$img_viewer $tiffile &\n";
	}

# claim it's all over
print FCMD "#\n# All done!\n";
print FCMD "echo All done!\n";

# now close the shellscript and make it executable
close FCMD;
chmod 0775, $cmdfile;

# tell program status
if ($verbose)
	{
	print "\nProgram Status:\n";
	print "--------------\n";
	print "\n  GMT Version:\n";
	print "    Version $gmt_version\n";
	print "\n  Plot Style:\n";
	if ($color_mode == 1)
		{
		print "    Color Fill\n";
		}
	elsif ($color_mode == 2)
		{
		print "    Color Shaded Relief\n";
		}
	elsif ($color_mode == 3 && $file_intensity && $stretch_shade)
		{
		print "    Color Shaded by Equalized Intensity File\n";
		}
	elsif ($color_mode == 3 && $file_intensity)
		{
		print "    Color Shaded by Unaltered Intensity File\n";
		}
	elsif ($color_mode == 4)
		{
		print "    Color Fill of Slope Magnitude\n";
		}
	elsif ($color_mode == 5)
		{
		print "    Color Shaded by Slope Magnitude\n";
		}
	print "\n  Input Files:\n";
	if ($file_data_no_list)
		{
		print "    Data GRD File:            $file_data\n";
		}
	else
		{
		print "    Data GRD List File:       $file_data\n";
		foreach $file_data (@files_data) {
			print "    Data GRD File:            $file_data\n";
		}
		}
	if ($file_intensity_no_list)
		{
		print "    Intensity GRD File:       $file_intensity\n";
		}
	else
		{
		print "    Intensity GRD List File:   $file_intensity\n";
		foreach $file_int (@files_intensity) {
			print "    Data GRD File:            $file_int\n";
		}
		}
	print "\n  Output Files:\n";
	print "    Output plot name root:    $root\n";
	print "    Color pallette table:     $cptfile\n";
	print "    Plotting shellscript:     $cmdfile\n";
	print "    Image file:               $tiffile\n";
	print "\n  Image Attributes:\n";
	if ($color_mode)
		{
		print "    Number of colors:         $ncolors\n";
		print "    Color Pallette:           ", 
			"@color_pallette_names[$color_pallette - 1]\n";
		if ($color_flip && color_pallette < 4)
			{
			print "    Colors reversed\n";
			}
		elsif ($color_flip)
			{
			print "    Grayscale reversed\n";
			}
		if ($shade_flip)
			{
			print "    Shading reversed\n";
			}
		}
	print "\n  Grid Data Attributes:\n";
	if ($data_scale)
		{
		print "    Data scale factor:        $data_scale\n";
		}
	printf "    Longitude min max:        %9.4f  %9.4f\n", 
		$xmin, $xmax;
	printf "    Latitude min max:         %9.4f  %9.4f\n", 
		$ymin, $ymax;
	printf "    Data min max:             %9.4g  %9.4g\n", 
		$zmin, $zmax;
	print "\n  Primary Grid Plotting Controls:\n";
	if ($color_mode && $stretch_color)
		{
		printf "    Color start datum:        %f\n", $color_start;
		printf "    Color end datum:          %f\n", $color_end;
		printf "    Histogram stretch applied to color pallette\n";
		}
	elsif ($color_mode)
		{
		printf "    Color start datum:        %f\n", $color_start;
		printf "    Color end datum:          %f\n", $color_end;
		printf "    Color datum interval:     %f\n", $color_int;
		}
	if ($color_mode == 3 && !$file_intensity)
		{
		printf "    Shading Magnitude:        %f\n", $magnitude;
		}
	elsif ($color_mode == 2)
		{
		printf "    Illumination Azimuth:     %f\n", $azimuth;
		printf "    Illumination Elevation:   %f\n", $elevation;
		printf "    Illumination Magnitude:   %f\n", $magnitude;
		}
	elsif ($color_mode == 4)
		{
		printf "    Slope Magnitude Magnitude:%f\n", $magnitude;
		}
	print "\n  GMT Default Values Reset in Script:\n";
	foreach $gmt_def (@gmt_macro_defs) {
		($gmt_par, $gmt_var) = $gmt_def =~ /^([^\/]+)\/(.+)/;
		printf "    %-25s %s\n", $gmt_par, $gmt_var;
		}
	foreach $gmt_def (@gmt_defs) {
		($gmt_par, $gmt_var) = $gmt_def =~ /^([^\/]+)\/(.+)/;
		printf "    $gmt_par : $gmt_var\n";
		}
	print "\n--------------\n";
	}

# print out final notes
print "\nImage generation shellscript <$cmdfile> created.\n";
print "\nInstructions:\n";
print "  Execute <$cmdfile> to generate TIFF image <$tiffile>.\n";
if (!$no_view_ps)
	{
	print "  Executing <$cmdfile> also invokes $ps_viewer ";
	print "to view the plot on the screen.\n";
	}
if ($verbose)
	{
	print "\n--------------\n\n";
	}

# execute shellscript if desired
if ($execute)
	{
	if ($verbose)
		{
		print "Executing shellscript $cmdfile...\n";
		}
	system "$cmdfile &";
	}

exit 0;

#-----------------------------------------------------------------------
sub min {

	# make local variables
	local ($min);
	
	# get the minimum of the arguments
	if ($_[0] < $_[1])
		{
		$min = $_[0];
		}
	else
		{
		$min = $_[1];
		}
	$min;
}
#-----------------------------------------------------------------------
sub max {

	# make local variables
	local ($max);
	
	# get the minimum of the arguments
	if ($_[0] > $_[1])
		{
		$max = $_[0];
		}
	else
		{
		$max = $_[1];
		}
	$max;
}
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
sub GetPageSize {

# deal with location of color scale
if ($scale_loc eq "l")
	{
	$space_top =    1.50 * $page_height_in{$pagesize} 
			    / $page_height_in{"a"};
	$space_bottom = 0.75 * $page_height_in{$pagesize} 
			    / $page_height_in{"a"};
	$space_left =   2.50 * $page_height_in{$pagesize} 
			    / $page_height_in{"a"};
	$space_right =  1.00 * $page_height_in{$pagesize} 
			    / $page_height_in{"a"};
	}
elsif ($scale_loc eq "r")
	{
	$space_top =    1.50 * $page_height_in{$pagesize} 
			    / $page_height_in{"a"};
	$space_bottom = 0.75 * $page_height_in{$pagesize} 
			    / $page_height_in{"a"};
	$space_left =   1.00 * $page_height_in{$pagesize} 
			    / $page_height_in{"a"};
	$space_right =  2.50 * $page_height_in{$pagesize} 
			    / $page_height_in{"a"};
	}
elsif ($scale_loc eq "t")
	{
	$space_top =    2.75 * $page_height_in{$pagesize} 
			    / $page_height_in{"a"};
	$space_bottom = 0.75 * $page_height_in{$pagesize} 
			    / $page_height_in{"a"};
	$space_left =   1.00 * $page_height_in{$pagesize} 
			    / $page_height_in{"a"};
	$space_right =  1.00 * $page_height_in{$pagesize} 
			    / $page_height_in{"a"};
	}
else
	{
	$space_top =    1.50 * $page_height_in{$pagesize} 
			    / $page_height_in{"a"};
	$space_bottom = 2.00 * $page_height_in{$pagesize} 
			    / $page_height_in{"a"};
	$space_left =   1.00 * $page_height_in{$pagesize} 
			    / $page_height_in{"a"};
	$space_right =  1.00 * $page_height_in{$pagesize} 
			    / $page_height_in{"a"};
	}

# set the relevent page width and height
$width_max_landscape = $page_height_in{$pagesize}
		- $space_left - $space_right;
$height_max_landscape = $page_width_in{$pagesize}
		- $space_bottom - $space_top;
$width_max_portrait = $page_width_in{$pagesize}
		- $space_left - $space_right;
$height_max_portrait = $page_height_in{$pagesize}
		- $space_bottom - $space_top;
$frame_size = 0.075;
if (($frame_size / $height_max_portrait) > 0.01)
	{
	$frame_size = 0.01 * $height_max_portrait;
	}
$tick_size = 0.075;
if (($tick_size / $height_max_portrait) > 0.01)
	{
	$tick_size = 0.01 * $height_max_portrait;
	}
}
#-----------------------------------------------------------------------
sub GetProjection {

	# get the map projection flag
	($projection) = $map_scale =~ /^(\w)/;
	($projection_pars) = $map_scale =~ /^$projection(\S+)/;

	# see if plot scale or plot width defined 
	$use_scale = 0;
	$use_width = 0;
	$separator = "/";
	$trial_value = 1.0;

	# Cassini Projection
	if ($projection eq "c")
		{
		($plot_scale) = $map_scale =~ /^c\S+\/\S+\/(\S+)$/; 
		$use_scale = 1;
		}
	elsif ($projection eq "C")
		{
		($plot_width) = $map_scale =~ /^C\S+\/\S+\/(\S+)$/; 
		$use_width = 1;
		}

	# Mercator Projection
	elsif ($projection eq "m")
		{
		($plot_scale) = $map_scale =~ /^m(\S+)$/; 
		$use_scale = 1;
		$separator = "";
		}
	elsif ($projection eq "M")
		{
		($plot_width) = $map_scale =~ /^M(\S+)$/; 
		$use_width = 1;
		$separator = "";
		}

	# Oblique Mercator Projection
	elsif ($projection eq "o")
		{
		if ($map_scale =~ /^oa\S+/)
			{
			($plot_scale) = $map_scale =~ /^oa\S+\/\S+\/\S+\/(\S+)$/;
			}
		elsif ($map_scale =~ /^ob\S+/)
			{
			($plot_scale) = $map_scale =~ /^ob\S+\/\S+\/\S+\/\S+\/(\S+)$/;
			}
		elsif ($map_scale =~ /^oc\S+/)
			{
			($plot_scale) = $map_scale =~ /^oc\S+\/\S+\/\S+\/\S+\/(\S+)$/;
			}
		$use_scale = 1;
		}
	elsif ($projection eq "O")
		{
		if ($map_scale =~ /^Oa\S+/)
			{
			($plot_width) = $map_scale =~ /^Oa\S+\/\S+\/\S+\/(\S+)$/;
			}
		elsif ($map_scale =~ /^Ob\S+/)
			{
			($plot_width) = $map_scale =~ /^Ob\S+\/\S+\/\S+\/\S+\/(\S+)$/;
			}
		elsif ($map_scale =~ /^Oc\S+/)
			{
			($plot_width) = $map_scale =~ /^Oc\S+\/\S+\/\S+\/\S+\/(\S+)$/;
			}
		$use_width = 1;
		}

	# Equidistant Cylindrical Projection
	elsif ($projection eq "q")
		{
		($plot_scale) = $map_scale =~ /^q\S+\/(\S+)$/; 
		$use_scale = 1;
		}
	elsif ($projection eq "Q")
		{
		($plot_width) = $map_scale =~ /^Q\S+\/(\S+)$/; 
		$use_width = 1;
		}

	# Transverse Mercator Projection
	elsif ($projection eq "t")
		{
		($plot_scale) = $map_scale =~ /^t\S+\/(\S+)$/; 
		$use_scale = 1;
		}
	elsif ($projection eq "T")
		{
		($plot_width) = $map_scale =~ /^T\S+\/(\S+)$/; 
		$use_width = 1;
		}

	# Universal Transverse Mercator Projection
	elsif ($projection eq "u")
		{
		($plot_scale) = $map_scale =~ /^u\S+\/(\S+)$/; 
		$use_scale = 1;
		}
	elsif ($projection eq "U")
		{
		($plot_width) = $map_scale =~ /^U\S+\/(\S+)$/; 
		$use_width = 1;
		}

	# Basic Cylindrical Projection
	elsif ($projection eq "y")
		{
		($plot_scale) = $map_scale =~ /^y\S+\/\S+\/(\S+)$/; 
		$use_scale = 1;
		}
	elsif ($projection eq "Y")
		{
		($plot_width) = $map_scale =~ /^Y\S+\/\S+\/(\S+)$/; 
		$use_width = 1;
		}

	# Lambert Azimuthal Projection
	elsif ($projection eq "a")
		{
		($plot_scale) = $map_scale =~ /^a\S+\/\S+\/(\S+)$/; 
		$use_scale = 1;
		$trial_value = "1:1";
		$use_ratio = 1;
		}
	elsif ($projection eq "A")
		{
		($plot_width) = $map_scale =~ /^A\S+\/\S+\/(\S+)$/; 
		$use_width = 1;
		}

	# Equidistant Projection
	elsif ($projection eq "e")
		{
		($plot_scale) = $map_scale =~ /^e\S+\/\S+\/(\S+)$/; 
		$use_scale = 1;
		$trial_value = "1:1";
		$use_ratio = 1;
		}
	elsif ($projection eq "E")
		{
		($plot_width) = $map_scale =~ /^E\S+\/\S+\/(\S+)$/; 
		$use_width = 1;
		}

	# Orthographic Projection
	elsif ($projection eq "g")
		{
		($plot_scale) = $map_scale =~ /^g\S+\/\S+\/(\S+)$/; 
		$use_scale = 1;
		$trial_value = "1:1";
		$use_ratio = 1;
		}
	elsif ($projection eq "G")
		{
		($plot_width) = $map_scale =~ /^G\S+\/\S+\/(\S+)$/; 
		$use_width = 1;
		}

	# General Sterographic Projection
	elsif ($projection eq "s")
		{
		($plot_scale) = $map_scale =~ /^s\S+\/\S+\/(\S+)$/; 
		$use_scale = 1;
		$trial_value = "1:1";
		$use_ratio = 1;
		}
	elsif ($projection eq "S")
		{
		($plot_width) = $map_scale =~ /^S\S+\/\S+\/(\S+)$/; 
		$use_width = 1;
		}

	# Albers Projection
	elsif ($projection eq "b")
		{
		($plot_scale) = $map_scale =~ /^b\S+\/\S+\/\S+\/\S+\/(\S+)$/; 
		$use_scale = 1;
		}
	elsif ($projection eq "B")
		{
		($plot_width) = $map_scale =~ /^B\S+\/\S+\/\S+\/\S+\/(\S+)$/; 
		$use_width = 1;
		}

	# Lambert Conic Projection
	elsif ($projection eq "l")
		{
		($plot_scale) = $map_scale =~ /^l\S+\/\S+\/\S+\/\S+\/(\S+)$/; 
		$use_scale = 1;
		}
	elsif ($projection eq "L")
		{
		($plot_width) = $map_scale =~ /^L\S+\/\S+\/\S+\/\S+\/(\S+)$/; 
		$use_width = 1;
		}

	# Hammer Projection
	elsif ($projection eq "h")
		{
		($plot_scale) = $map_scale =~ /^h\S+\/(\S+)$/; 
		$use_scale = 1;
		}
	elsif ($projection eq "H")
		{
		($plot_width) = $map_scale =~ /^H\S+\/(\S+)$/; 
		$use_width = 1;
		}

	# Sinusoidal Projection
	elsif ($projection eq "i")
		{
		($plot_scale) = $map_scale =~ /^i\S+\/(\S+)$/; 
		$use_scale = 1;
		}
	elsif ($projection eq "I")
		{
		($plot_width) = $map_scale =~ /^I\S+\/(\S+)$/; 
		$use_width = 1;
		}

	# Eckert VI Projection
	elsif ($projection eq "k")
		{
		($plot_scale) = $map_scale =~ /^k\S+\/(\S+)$/; 
		$use_scale = 1;
		}
	elsif ($projection eq "K")
		{
		($plot_width) = $map_scale =~ /^K\S+\/(\S+)$/; 
		$use_width = 1;
		}

	# Robinson Projection
	elsif ($projection eq "n")
		{
		($plot_scale) = $map_scale =~ /^n\S+\/(\S+)$/; 
		$use_scale = 1;
		}
	elsif ($projection eq "N")
		{
		($plot_width) = $map_scale =~ /^N\S+\/(\S+)$/; 
		$use_width = 1;
		}

	# Winkel Tripel Projection
	elsif ($projection eq "r")
		{
		($plot_scale) = $map_scale =~ /^r\S+\/(\S+)$/; 
		$use_scale = 1;
		}
	elsif ($projection eq "R")
		{
		($plot_width) = $map_scale =~ /^R\S+\/(\S+)$/; 
		$use_width = 1;
		}

	# Mollweide Projection
	elsif ($projection eq "w")
		{
		($plot_scale) = $map_scale =~ /^w\S+\/(\S+)$/; 
		$use_scale = 1;
		}
	elsif ($projection eq "W")
		{
		($plot_width) = $map_scale =~ /^W\S+\/(\S+)$/; 
		$use_width = 1;
		}

	# Linear Polar Projection
	elsif ($projection eq "p")
		{
		($plot_scale) = $map_scale =~ /^p(\S+)$/; 
		$use_scale = 1;
		$separator = "";
		}
	elsif ($projection eq "P")
		{
		($plot_width) = $map_scale =~ /^P(\S+)$/; 
		$use_width = 1;
		$separator = "";
		}

	# Linear Projection
	elsif ($projection eq "x")
		{
		if ($map_scale =~ /^xd$/)
			{
			$linear = 1;
			chop($map_scale);
			}
		else
			{
			($plot_scale) = $map_scale =~ /^x(\S+)$/; 
			}
		$use_scale = 1;
		$separator = "";
		}
	elsif ($projection eq "X")
		{
		if ($map_scale =~ /^Xd$/)
			{
			$linear = 1;
			chop($map_scale);
			}
		else
			{
			($plot_width) = $map_scale =~ /^X(\S+)$/; 
			}
		$use_width = 1;
		$separator = "";
		}
}
#-----------------------------------------------------------------------
sub GetBaseTick {

	# figure out some reasonable tick intervals for the basemap
	$base_tick_x = ($xmax - $xmin) / 5;
	$base_tick_y = ($ymax - $ymin) / 5;
	$base_tick = &min($base_tick_x, $base_tick_y);
	if ($base_tick < 0.0002777777)
		{
		$base_tick = "1c";
		}
	elsif ($base_tick < 0.0005555555)
		{
		$base_tick = "2c";
		}
	elsif ($base_tick < 0.0013888889)
		{
		$base_tick = "5c";
		}
	elsif ($base_tick < 0.0027777778)
		{
		$base_tick = "10c";
		}
	elsif ($base_tick < 0.0041666667)
		{
		$base_tick = "15c";
		}
	elsif ($base_tick < 0.0083333333)
		{
		$base_tick = "30c";
		}
	elsif ($base_tick < 0.0166667)
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
	elsif ($base_tick < 10.0)
		{
		$base_tick = "10";
		}
	elsif ($base_tick < 15.0)
		{
		$base_tick = "15";
		}
	elsif ($base_tick < 30.0)
		{
		$base_tick = "30";
		}
	elsif ($base_tick < 30.0)
		{
		$base_tick = "30";
		}
	elsif ($base_tick < 360.0)
		{
		$base_tick = "60";
		}

}
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
