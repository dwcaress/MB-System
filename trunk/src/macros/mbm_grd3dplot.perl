eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
                    & eval 'exec perl -S $0 $argv:q'
                         if 0;
#--------------------------------------------------------------------
#    The MB-system:	mbm_grd3dplot.perl	8/6/95
#    $Id: mbm_grd3dplot.perl,v 5.3 2001-10-10 23:56:01 dcaress Exp $
#
#    Copyright (c) 1993, 1994, 1995, 2000 by 
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
#   mbm_grd3dplot
#
# Purpose:
#   Macro to generate a shellscript of GMT commands which, when
#   executed, will generate a 3D perspective Postscript plot of 
#   gridded data.  Several styles of plots can be generated,
#   including color fill views, color shaded relief views, mesh 
#   plot views, and text labels. Five different color schemes are
#   included. The plot will be scaled to fit on the specified page 
#   size or, if the scale is user defined, the page size will be 
#   chosen in accordance with the plot size. The primary purpose 
#   of this macro is to allow the simple, semi-automated
#   production of nice looking plots with a few command line
#   arguments. For users seeking more control over the plot
#   appearance, a number of additional optional arguments are
#   provided. Truly ambitious users may edit the plot shellscript 
#   to take advantage of GMT capabilites not supported by this 
#   macro.
#
# Basic Usage: 
#   mbm_grd3dplot -Ifile [-A[magnitude/azimuth/elevation] 
#            -C[contour_control] -Dflipcolor/flipshade 
#            -Eview_az/view_el
#            -Gcolor_mode -H -Kintensity_file 
#            -Ndrape_file -Oroot -Ppagesize 
#            -S[color/shade] -Uorientation -V 
#            -Wcolor_style[/pallette] ]
#
# Additional Options:
#            [-Btickinfo 
#            -Jprojection[/scale | width] -Ltitle[:scale_label] 
#            -Mmisc -Q -Rw/e/s/n -X -Y -Zmin/max]
#
# Miscellaneous Options:
#            [-MGDgmtdef/value -FMGscale_loc 
#            -MGL[f][x]lon0/lat0/slat/length[m]
#            -MGQdpi -MGSscalefactor -MGTx/y/size/angle/font/just/text
#            -MGU[/dx/dy/][label] -MVMmesh_pen -MVNnull -MVZzlevel ]
#
# Author:
#   David W. Caress
#   Lamont-Doherty Earth Observatory
#   Palisades, NY  10964
#   August 8, 1994
#
# Version:
#   $Id: mbm_grd3dplot.perl,v 5.3 2001-10-10 23:56:01 dcaress Exp $
#
# Revisions:
#   $Log: not supported by cvs2svn $
#   Revision 5.2  2001-06-02 23:59:24-07  caress
#   Release 5.0.beta01
#
#   Revision 5.1  2001/03/22 21:05:45  caress
#   Trying to make release 5.0.beta0
#
# Revision 5.0  2000/12/01  22:58:01  caress
# First cut at Version 5.0.
#
# Revision 4.14  2000/10/03  21:42:17  caress
# Snapshot for Dale.
#
# Revision 4.13  1999/12/29  00:17:55  caress
# Release 4.6.8
#
# Revision 4.12  1999/06/25  17:55:47  caress
# I must have changed something!
#
# Revision 4.11  1999/05/06  23:46:32  caress
# Release 4.6a
#
# Revision 4.10  1999/04/16  01:25:51  caress
# Version 4.6 final release?
#
# Revision 4.9  1999/04/15  19:28:52  caress
# Fixed sprintf statements.
#
# Revision 4.8  1999/02/04  23:39:54  caress
# MB-System version 4.6beta7
#
# Revision 4.7  1999/01/26  19:46:58  caress
# Fixed parsing of grdinfo output from GMT 3.1.
#
# Revision 4.6  1998/10/05  17:00:15  caress
# MB-System version 4.6beta
#
# Revision 4.5  1997/09/15  19:05:23  caress
# Real Version 4.5
#
# Revision 4.4  1997/04/21  16:54:41  caress
# MB-System 4.5 Beta Release.
#
# Revision 4.4  1997/04/17  15:06:49  caress
# MB-System 4.5 Beta Release
#
# Revision 4.3  1995/11/22  22:46:40  caress
# Check in during general flail.
#
# Revision 4.2  1995/09/28  19:52:25  caress
# Fixed handling of null plane specification.
#
# Revision 4.1  1995/09/28  18:05:43  caress
# Various bug fixes working toward release 4.3.
#
# Revision 4.0  1995/08/17  14:51:59  caress
# Revision for release 4.3.
#
#
#
#
$program_name = "mbm_grd3dplot";

# set degree to radians conversion
$DTR = 3.1415926 / 180.0;

# set page size database
@page_size_names = (  
	"a", "b", "c", "d", "e", "f", "e1",
	"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9", "a10", 
	"b0", "b1", "b2", "b3", "b4", "b5", "b6", "b7", "b8", "b9", "b10", 
	"c0", "c1", "c2", "c3", "c4", "c5", "c6", "c7");
%page_width_in = (  
	"a",     8.50,   "b",    11.00,   "c",    17.00,   "d",    22.00, 
	"e",    34.00,   "f",    28.00,   "e1",   44.00,   "a0",   33.11,
	"a1",   23.39,   "a2",   16.54,   "a3",   11.69,   "a4",    8.27,
	"a5",    5.83,   "a6",    4.13,   "a7",    2.91,   "a8",    2.05,
	"a9",    1.46,   "a10",   1.02,   "b0",   39.37,   "b1",   27.83,
	"b2",   19.68,   "b3",   13.90,   "b4",    9.84,   "b5",    6.93,
	"b6",    4.92,   "b7",    3.46,   "b8",    2.44,   "b9",    1.73,
	"b10",   1.22,   "c0",   36.00,   "c1",   25.60,   "c2",   18.00,
	"c3",   12.80,   "c4",    9.00,   "c5",    6.40,   "c6",    4.50,
	"c7",    3.20);
%page_height_in = ( 
	"a",    11.00,   "b",    17.00,   "c",    22.00,   "d",    34.00, 
	"e",    44.00,   "f",    40.00,   "e1",   68.00,   "a0",   46.81,
	"a1",   33.11,   "a2",   23.39,   "a3",   16.54,   "a4",   11.69,
	"a5",    8.27,   "a6",    5.83,   "a7",    4.13,   "a8",    2.91,
	"a9",    2.05,   "a10",   1.46,   "b0",   56.67,   "b1",   39.37,
	"b2",   27.83,   "b3",   19.68,   "b4",   13.90,   "b5",    9.84,
	"b6",    6.93,   "b7",    4.92,   "b8",    3.46,   "b9",    2.44,
	"b10",   1.73,   "c0",   51.20,   "c1",   36.00,   "c2",   25.60,
	"c3",   18.00,   "c4",   12.80,   "c5",    9.00,   "c6",    6.40,
	"c7",    4.50);
%page_anot_font = ( 
	"a",     8,   "b",    12,   "c",    16,   "d",    24,
	"e",    32,   "f",    32,   "e1",   32,   "a0",   32,
	"a1",   24,   "a2",   16,   "a3",   12,   "a4",    8,
	"a5",    6,   "a6",    6,   "a7",    6,   "a8",    4,
	"a9",    4,   "a10",   4,   "b0",   32,   "b1",   24,
	"b2",   16,   "b3",   16,   "b4",   12,   "b5",    8,
	"b6",    6,   "b7",    4,   "b8",    4,   "b9",    4,
	"b10",   4,   "c0",   32,   "c1",   24,   "c2",   16,
	"c3",   12,   "c4",    8,   "c5",    6,   "c6",    6,
	"c7",    6);
%page_header_font =(
	"a",    10,   "b",    15,   "c",    20,   "d",    30,
	"e",    40,   "f",    40,   "e1",   40,   "a0",   40,
	"a1",   30,   "a2",   20,   "a3",   15,   "a4",   10,
	"a5",    8,   "a6",    8,   "a7",    8,   "a8",    5,
	"a9",    5,   "a10",   5,   "b0",   40,   "b1",   30,
	"b2",   20,   "b3",   20,   "b4",   15,   "b5",   10,
	"b6",    8,   "b7",    5,   "b8",    5,   "b9",    5,
	"b10",   5,   "c0",   40,   "c1",   30,   "c2",   20,
	"c3",   15,   "c4",   10,   "c5",    8,   "c6",    8,
	"c7",    8);
%page_gmt_name =     (
	"a",     "archA",   "b",     "archB",   "c",     "archC",   "d",     "archD", 
	"e",     "archE",   "f",     "archE",   "e1",    "archE",   "a0",    "A0",
	"a1",    "A1",      "a2",    "A2",      "a3",    "A3",      "a4",    "A4",
	"a5",    "A5",      "a6",    "A6",      "a7",    "A7",      "a8",    "A8",
	"a9",    "A9",      "a10",   "A10",     "b0",    "B0",      "b1",    "B1",
	"b2",    "B2",      "b3",    "B3",      "b4",    "B4",      "b5",    "B5",
	"b6",    "A6",      "b7",    "A7",      "b8",    "A8",      "b9",    "A9",
	"b10",   "A10",     "c0",    "B0",      "c1",    "B1",      "c2",    "B2",
	"c3",    "B3",      "c4",    "B4",      "c5",    "B5",      "c6",    "B6",
	"c7",    "B7");
%xpsview_mem =     (
	"a",     "4m",   "b",     "6m",   "c",     "8m",   "d",    "12m", 
	"e",    "16m",   "f",    "16m",   "e1",   "16m",   "a0",   "16m",
	"a1",   "12m",   "a2",    "8m",   "a3",    "6m",   "a4",    "4m",
	"a5",    "4m",   "a6",    "4m",   "a7",    "4m",   "a8",    "4m",
	"a9",    "4m",   "a10",   "4m",   "b0",   "16m",   "b1",   "12m",
	"b2",    "8m",   "b3",    "8m",   "b4",    "6m",   "b5",    "4m",
	"b6",    "4m",   "b7",    "4m",   "b8",    "4m",   "b9",    "4m",
	"b10",   "4m",   "c0",   "16m",   "c1",   "12m",   "c2",    "8m",
	"c3",    "6m",   "c4",    "4m",   "c5",    "4m",   "c6",    "4m",
	"c7",    "4m");

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

# define degrees to radians conversion
$PI = 3.1415926;
$DTR = $PI / 180.0;

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
$command_line = "@ARGV";
&MBGetopts('A:a:B:b:C%c%D%d%E:e:e:G%g%HhI:i:J+j+K:k:L:l:M+m+N:n:O:o:P:p:QqR:r:S%s%T%t%U:u:VvW:w:XxYyZ:z:');
$shade_control = 	($opt_A || $opt_a);
$tick_info = 		($opt_B || $opt_b);
$contour_mode = 	($flg_C || $flg_c);
$contour_control = 	($opt_C || $opt_c);
$color_flip_mode = 	($flg_D || $flg_d);
$color_flip_control = 	($opt_D || $opt_d);
$view_control = 	($opt_E || $opt_e);
$color_mode =   	($opt_G || $opt_g || $flg_G || $flg_g);
$help =    		($opt_H || $opt_h);
$file_data =    	($opt_I || $opt_i);
$map_scale =    	($opt_J || $opt_j);
$file_intensity =    	($opt_K || $opt_k);
$labels =    		($opt_L || $opt_l);
$misc = 		($opt_M || $opt_m);
$file_drape =    	($opt_N || $opt_n);
$root =    		($opt_O || $opt_o);
$pagesize = 		($opt_P || $opt_p);
$no_view_ps = 		($opt_Q || $opt_q);
$bounds = 		($opt_R || $opt_r);
$stretch_mode = 	($flg_S || $flg_s);
$stretch_control = 	($opt_S || $opt_s);
$coastline_control = 	($opt_T || $opt_t);
$coastline_mode = 	($flg_T || $flg_t);
$orientation = 		($opt_U || $opt_u);
$verbose = 		($opt_V || $opt_v);
$color_control = 	($opt_W || $opt_w);
$execute = 		($opt_X || $opt_x);
$no_nice_color_int = 	($opt_Y || $opt_y);
$zbounds = 		($opt_Z || $opt_z);

# print out help message if required
if ($help)
	{
	print "\n$program_name:\n";
	print "\nMacro to generate a shellscript of GMT commands which, when \n";
	print "executed, will generate a 3D perspective Postscript plot of  \n";
	print "gridded data.  Several styles of plots can be generated, \n";
	print "including color fill views, color shaded relief views, mesh  \n";
	print "plot views, and text labels. Five different color schemes are \n";
	print "included. The plot will be scaled to fit on the specified page  \n";
	print "size or, if the scale is user defined, the page size will be  \n";
	print "chosen in accordance with the plot size. The primary purpose \n";
	print "of this macro is to allow the simple, semi-automated \n";
	print "production of nice looking maps with a few command line \n";
	print "arguments. For users seeking more control over the plot \n";
	print "appearance, a number of additional optional arguments are \n";
	print "provided. Truly ambitious users may edit the plot shellscript \n";
	print "to take advantage of GMT capabilites not supported by this \n";
	print "macro.\n";
	print "\nBasic Usage: \n";
	print "\t$program_name -Ifile [-Amagnitude[/azimuth/elevation] \n";
	print "\t\t-C[contour_control] -Eview_az/view_el \n";
	print "\t\t-Gcolor_mode -H -Kintensity_file \n";
	print "\t\t-Ndrape_file -Oroot -Ppagesize \n";
	print "\t\t-S[color/shade] -Uorientation -V\n";
	print "\t\t-Wcolor_style[/pallette] ]\n";
	print "Additional Options:\n";
	print "\t\t[-Btickinfo -Dflipcolor/flipshade\n";
	print "\t\t-Jprojection[/scale | width] -Ltitle[:scale_label] \n";
	print "\t\t-Mmisc -Q -Rw/e/s/n -X -Y -Zmin/max]\n";
	print "Miscellaneous Options:\n";
	print "\t\t[-MGDgmtdef/value -MGFscale_loc\n";
	print "\t\t-MGL[f][x]lon0/lat0/slat/length[m]\n";
	print "\t\t-MGQdpi -MGSscalefactor -MGTx/y/size/angle/font/just/text\n";
	print "\t\t-MGU[/dx/dy/][label] -MVMmesh_pen -MVNnull -MVZzlevel ]\n";
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
if ($file_drape && ! -r $file_drape)
	{
	print "\a";
	die "\nSpecified drape input file $file_drape cannot be opened!\n$program_name aborted\n";
	}

# parse misc commands
if ($misc)
	{
	@misc_cmd = split(/:::::::/, $misc);
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

		# set color scale location
		if ($cmd =~ /^[Gg][Ff]./)
			{
			($scale_loc) = $cmd =~ 
				/^[Gg][Ff](\S+)/;
			}

		# set map scale
		if ($cmd =~ /^[Gg][Ll]./)
			{
			($length_scale) = $cmd =~ 
				/^[Gg][Ll](\S+)/;
			}

		# set dpi for color image output
		if ($cmd =~ /^[GG][Qq]./)
			{
			($dpi) = $cmd =~ /^[GG][Qq](.+)/;
			}

		# set data scaling
		if ($cmd =~ /^[Gg][Ss]./)
			{
			($data_scale) = $cmd =~ 
				/^[Gg][Ss](\S+)/;
			}

		# set text labels
		if ($cmd =~ /^[Gg][Tt]./)
			{
			($tx, $ty, $tsize, $tangle, $font, $just, $txt) 
			    = $cmd
			    =~ /^[Gg][Pp](\S+)\/(\S+)\/(\S+)\/(\S+)\/(\S+)\/(\S+)\/(.+)/;
			if ($txt)
			    {
			    ($text_info) = $cmd =~ 
				/^[Gg][Tt](.*)/;
			    push(@text, $text_info);
			    }
			else
			    {
			    print "\nInvalid text label ignored: $cmd\n";
			    }
			}

		# set unix time stamp
		if ($cmd =~ /^[Gg][Uu]./)
			{
			($unix_stamp) = $cmd =~ /^[Gg][Uu](\S+)/;
			$unix_stamp_on = 1;
			}
		elsif ($cmd =~ /^[Gg][Uu]/)
			{
			$unix_stamp_on = 1;
			}

		# deal with grdview options
		##############################

		# set grdview mesh pen attributes
		if ($cmd =~ /^[Vv][Mm]./)
			{
			($grdview_mesh_pen) = $cmd =~ 
				/^[Vv][Mm](\S+)/;
			}

		# set grdview null plane level and color
		if ($cmd =~ /^[Vv][Nn]./)
			{
			if ($cmd =~ /^[Vv][Nn]\S+/)
				{
				($grdview_null) = $cmd =~ 
					/^[Vv][Nn](\S+)/;
				}
			}
		elsif ($cmd =~ /^[Vv][Nn]/)
			{
			$grdview_null_set = 1;
			}

		# set grdview contour pen attributes
		if ($cmd =~ /^[Vv][Ww]./)
			{
			($grdview_contour_pen) = $cmd =~ 
				/^[Vv][Ww](\S+)/;
			}

		# set grdview z level
		if ($cmd =~ /^[Vv][Zz]./)
			{
			($grdview_zlevel) = $cmd =~ 
				/^[Vv][Zz](\S+)/;
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
if (!$color_mode && !$contour_mode)
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
	$magnitude = 0.2;
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
if (!$view_control)
	{
	$view_control = "240/30";
	($view_azimuth, $view_elevation) = $view_control
		=~ /(\S+)\/(\S+)/;
	$view_azimuth = "240";
	$view_elevation = "30";
	}
else
	{
	($view_azimuth, $view_elevation) = $view_control
		=~ /(\S+)\/(\S+)/;
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
		close MBDEF;
		}
	}
# else just set it to ghostview
if (!$ps_viewer)
	{
	$ps_viewer = "ghostview";
	}

# get limits of data file using grdinfo
if (!$bounds || !$zbounds)
	{
	if ($bounds)
		{
		@grdinfo = `mbm_grdinfo -I$file_data -R$bounds`;
		}
	else
		{
		@grdinfo = `grdinfo $file_data`;
		}
	while (@grdinfo)
		{
		$line = shift @grdinfo;
		if ($line =~ 
			/\S+\s+x_min:\s+(\S+)\s+x_max:\s+(\S+)\s+x_inc:/)
			{
			($xmin,$xmax) = $line =~ 
				/\S+\s+x_min:\s+(\S+)\s+x_max:\s+(\S+)\s+x_inc:/;
			}
		if ($line =~ /\S+\s+y_min:\s+(\S+)\s+y_max:\s+(\S+)\s+y_inc:/)
			{
			($ymin,$ymax) = $line =~ 
				/\S+\s+y_min:\s+(\S+)\s+y_max:\s+(\S+)\s+y_inc:/;
			}
		if ($line =~ /\S+\s+zmin:\s+(\S+)\s+zmax:\s+(\S+)\s+units:/)
			{
			($zmin,$zmax) = $line =~ 
				/\S+\s+zmin:\s+(\S+)\s+zmax:\s+(\S+)\s+units:/;
			}
		if ($line =~ /\S+\s+z_min:\s+(\S+)\s+z_max:\s+(\S+)\s+units:/)
			{
			($zmin,$zmax) = $line =~ 
				/\S+\s+z_min:\s+(\S+)\s+z_max:\s+(\S+)\s+units:/;
			}
		}

	# check that there is data
	if ($xmin >= $xmax || $ymin >= $ymax || $zmin >= $zmax)
		{
		print "\a";
		die "The program grdinfo does not appear to have worked properly!\n$program_name aborted.\n"
		}
	}

# get limits of drape data file using grdinfo
if ($file_drape && !$zbounds)
	{
	@grdinfo = `grdinfo $file_drape`;
	while (@grdinfo)
		{
		$line = shift @grdinfo;
		if ($line =~ /\S+\s+zmin:\s+(\S+)\s+zmax:\s+(\S+)\s+units:/)
			{
			($zmin,$zmax) = $line =~ 
				/\S+\s+zmin:\s+(\S+)\s+zmax:\s+(\S+)\s+units:/;
			}
		if ($line =~ /\S+\s+z_min:\s+(\S+)\s+z_max:\s+(\S+)\s+units:/)
			{
			($zmin,$zmax) = $line =~ 
				/\S+\s+z_min:\s+(\S+)\s+z_max:\s+(\S+)\s+units:/;
			}
		}

	# check that there is data
	if ($zmin >= $zmax)
		{
		print "\a";
		die "The program grdinfo does not appear to have worked properly!\n$program_name aborted.\n"
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
		$bounds_plot = sprintf ("%1.8g/%1.8g/%1.8g/%1.8gr",
			$xmin, $ymin, $xmax, $ymax);
		}
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
if ((!$use_corner_points && ($xmin >= $xmax || $ymin >= $ymax)) 
	|| $zmin >= $zmax)
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

# set the relevent page width and height
&GetPageSize;

# get user constraints on map scale 
if ($map_scale)
	{
	# break up map_scale into -Jz and -Jproj
	@J = split(/:::::::/, $map_scale);
	if (scalar(@J) == 2 && $J[0] =~ /^z.*/)
		{
		($map_zscale) = $J[0] =~ /^z.(\S+)/;
		$map_scale = $J[1];

		# sets $plot_scale or $plot_width if possible
		&GetProjection;
		}
	elsif (scalar(@J) == 2)
		{
		($map_zscale) = $J[1] =~ /^z.(\S+)/;
		$map_scale = $J[0];

		# sets $plot_scale or $plot_width if possible
		&GetProjection;
		}
	elsif (scalar(@J) == 1 && $J[0] =~ /^z.*/)
		{
		($map_zscale) = $J[0] =~ /^z.(\S+)/;
		}
	else
		{
		# sets $plot_scale or $plot_width if possible
		&GetProjection;
		}
	}

# set up for mapproject
if (($use_scale && $plot_scale) || ($use_width && $plot_width))
	{
	($projection) = $map_scale =~ /^(\w)/;
	($projection_pars) = $map_scale =~ /^$projection(\S+)/;
	}
elsif ($use_scale || $use_width)
	{
	($projection) = $map_scale =~ /^(\w)/;
	($projection_pars) = $map_scale =~ /^$projection(\S+)/;
	$projection_pars = "$projection_pars" . "$separator" . "$trial_value";
	}
else
	{
	$projection = "m";
	$projection_pars = "1.0";
	$use_scale = 1;
	}

# now find out the apparent size of the plot
`echo $xmin $ymin > tmp$$.dat`;
`echo $xmax $ymin >> tmp$$.dat`;
`echo $xmax $ymax >> tmp$$.dat`;
`echo $xmin $ymax >> tmp$$.dat`;
@projected = `mapproject tmp$$.dat -J$projection$projection_pars -R$bounds_plot 2>&1 `;
`/bin/rm -f tmp$$.dat`;
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

# check for valid scaling
if ($dxx == 0.0 && $dyy == 0.0)
	{
	print "\a";
	die "Invalid projection specified - $program_name aborted\n";
	}

# figure out scaling issues
if (($use_scale && $plot_scale) || ($use_width && $plot_width))
	{
	# set axis lengths
	$plot_width_xaxis = $dxx;
	$plot_height_yaxis = $dyy;

	# adjust plotscale for 3D
	$x_axis_rot = &abs($dxx * cos($DTR * $view_azimuth));
	$y_axis_rot = &abs($dyy * sin($DTR * $view_azimuth));
	$plot_width_factor = ($x_axis_rot + $y_axis_rot) / $dxx;
	$plot_width = $plot_width_factor * $plot_width_xaxis;
	$plot_height = $plot_width_factor * $plot_height_yaxis;
	if (!$map_zscale)
		{
		$map_zscale = 2.0/($zmax - $zmin);
		}

	# decide which plot orientation to use
	if ($orientation == 1)
		{
		$portrait = 1;
		$width = $page_width_in{$pagesize};
		$height = $page_height_in{$pagesize};
		$width_max = $width_max_portrait;
		$height_max = $height_max_portrait;
		}
	elsif ($orientation == 2)
		{
		$landscape = 1;
		$width = $page_height_in{$pagesize};
		$height = $page_width_in{$pagesize};
		$width_max = $width_max_landscape;
		$height_max = $height_max_landscape;
		}
	elsif ($dxx > $dyy)
		{
		$landscape = 1;
		$width = $page_height_in{$pagesize};
		$height = $page_width_in{$pagesize};
		$width_max = $width_max_landscape;
		$height_max = $height_max_landscape;
		}
	else
		{
		$portrait = 1;
		$width = $page_width_in{$pagesize};
		$height = $page_height_in{$pagesize};
		$width_max = $width_max_portrait;
		$height_max = $height_max_portrait;
		}

	# check if plot fits on page
	if ($plot_width > $width_max || $plot_height > $height_max)
		{
		# try to find a sufficiently large pagesize
		$pagesize_save = $pagesize;
		foreach $elem (@page_size_names) {
			$pagesize = "$elem";
			&GetPageSize;
			if ($portrait)
				{
				$width_max = $width_max_portrait;
				$height_max = $height_max_portrait;
				}
			else
				{
				$width_max = $width_max_landscape;
				$height_max = $height_max_landscape;
				}
			if (!$good_page && 
				$plot_width <= $width_max 
				&& $plot_height <= $height_max)
				{
				$good_page = 1;
				$pagesize_save = $pagesize;
				}
			}

		# print out warning
		if ($pagesize eq $pagesize_save)
			{
			print "\nWarning: Unable to fit plot on any available page size!\n";
			print "\tThis plot will not be particularly useful!\n";
			print "\tTry using a different scale or allow the program to set the scale!\n";
			}

		# reset the page size
		$pagesize = $pagesize_save;
		&GetPageSize;
		if ($portrait)
			{
			$width = $page_width_in{$pagesize};
			$height = $page_height_in{$pagesize};
			}
		else
			{
			$width = $page_height_in{$pagesize};
			$height = $page_width_in{$pagesize};
			}
		}
	}
elsif ($use_scale)
	{
	# get landscape and portrait scales
	$plot_scale_landscape = $width_max_landscape/$dxx;
	if ($plot_scale_landscape*$dyy > $height_max_landscape)
		{
		$plot_scale_landscape = $height_max_landscape/$dyy;
		}
	$plot_scale_portrait = $width_max_portrait/$dxx;
	if ($plot_scale_portrait*$dyy > $height_max_portrait)
		{
		$plot_scale_portrait = $height_max_portrait/$dyy;
		}

	# decide which plot orientation to use
	if ($orientation == 1)
		{
		$portrait = 1;
		$plot_scale = $plot_scale_portrait;
		$width = $page_width_in{$pagesize};
		$height = $page_height_in{$pagesize};
		}
	elsif ($orientation == 2)
		{
		$landscape = 1;
		$plot_scale = $plot_scale_landscape;
		$width = $page_height_in{$pagesize};
		$height = $page_width_in{$pagesize};
		}
	elsif ($plot_scale_landscape > $plot_scale_portrait)
		{
		$landscape = 1;
		$plot_scale = $plot_scale_landscape;
		$width = $page_height_in{$pagesize};
		$height = $page_width_in{$pagesize};
		}
	else
		{
		$portrait = 1;
		$plot_scale = $plot_scale_portrait;
		$width = $page_width_in{$pagesize};
		$height = $page_height_in{$pagesize};
		}

	# set raw plot width
	$plot_width = $dxx * $plot_scale;
	$plot_height = $dyy * $plot_scale;

	# adjust plotscale for 3D
	$x_axis_rot = &abs($dxx * cos($DTR * $view_azimuth));
	$y_axis_rot = &abs($dyy * sin($DTR * $view_azimuth));
	$plot_scale_factor = $dxx / ($x_axis_rot + $y_axis_rot);
	$plot_scale = $plot_scale_factor * $plot_scale;
	if (!$map_zscale)
		{
		$map_zscale = 2.0/($zmax - $zmin);
		}

	# set plot width
	$plot_width_xaxis = $plot_scale_factor * $plot_width;
	$plot_height_yaxis = $plot_scale_factor * $plot_height;

	# reset plot_scale if ratio required
	if ($use_ratio)
		{
		$top = int(1 / $plot_scale);
		$plot_scale = "1:" . "$top";
		}

	# construct plot scale parameters
	($projection_pars) = $map_scale =~ /^$projection(\S+)/;
	$projection_pars = sprintf ("$projection_pars$separator%1.5g", $plot_scale);

	# handle special case for linear projections
	if ($geographic)
		{
		$projection_pars = "$projection_pars" . "d";
		}
	}
elsif ($use_width)
	{
	# get landscape and portrait heights
	$plot_width_landscape = $height_max_landscape * $dxx / $dyy;
	if ($plot_width_landscape > $width_max_landscape)
		{
		$plot_width_landscape = $width_max_landscape;
		}
	$plot_width_portrait = $height_max_portrait * $dxx / $dyy;
	if ($plot_width_portrait > $width_max_portrait)
		{
		$plot_width_portrait = $width_max_portrait;
		}

	# decide which plot orientation to use
	if ($orientation == 1)
		{
		$portrait = 1;
		$plot_width = $plot_width_portrait;
		$plot_height = $plot_width * $dyy / $dxx;
		$width = $page_width_in{$pagesize};
		$height = $page_height_in{$pagesize};
		}
	elsif ($orientation == 2)
		{
		$landscape = 1;
		$plot_width = $plot_width_landscape;
		$plot_height = $plot_width * $dyy / $dxx;
		$width = $page_height_in{$pagesize};
		$height = $page_width_in{$pagesize};
		}
	elsif ($plot_width_landscape > $plot_width_portrait)
		{
		$landscape = 1;
		$plot_width = $plot_width_landscape;
		$plot_height = $plot_width * $dyy / $dxx;
		$width = $page_height_in{$pagesize};
		$height = $page_width_in{$pagesize};
		}
	else
		{
		$portrait = 1;
		$plot_width = $plot_width_portrait;
		$plot_height = $plot_width * $dyy / $dxx;
		$width = $page_width_in{$pagesize};
		$height = $page_height_in{$pagesize};
		}

	# adjust plotwidth for 3D (kluge)
	$x_axis_rot = &abs($dxx * cos($DTR * $view_azimuth));
	$y_axis_rot = &abs($dyy * sin($DTR * $view_azimuth));
	$plots_width_factor = $dxx / ($x_axis_rot + $y_axis_rot);
	$plot_width_xaxis = $plot_width_factor * $plot_width;
	$plot_height_yaxis = $plot_width_factor * $plot_height;
	if (!$map_zscale)
		{
		$map_zscale = 2.0/($zmax - $zmin);
		}

	# construct plot scale parameters
	($projection_pars) = $map_scale =~ /^$projection(\S+)/;
	$projection_pars = sprintf ("$projection_pars$separator%1.5g", $plot_width_xaxis);

	# handle special case for linear projections
	if ($geographic)
		{
		$projection_pars = "$projection_pars" . "d";
		}
	}

# place the origin so plot is more or less centered
$xoffset = ($width - $plot_width 
	- $space_left - $space_right) / 2 + $space_left;
$yoffset = ($height - $plot_height 
	- $space_bottom - $space_top) / 2 + $space_bottom;

# figure out where to place the color scale
$scale_loc =~ tr/A-Z/a-z/;
if ($scale_loc eq "l")
	{
	$colorscale_length = $plot_height;
	$colorscale_thick = 0.013636364 * $page_height_in{$pagesize};
	$colorscale_offx = -0.13636 * $page_height_in{$pagesize};
	$colorscale_offy = 0.5*$plot_height;
	$colorscale_vh = "v";
	}
elsif ($scale_loc eq "r")
	{
	$colorscale_length = $plot_height;
	$colorscale_thick = 0.013636364 * $page_height_in{$pagesize};
	$colorscale_offx = $plot_width 
		+ 0.0909 * $page_height_in{$pagesize};
	$colorscale_offy = 0.5*$plot_height;
	$colorscale_vh = "v";
	}
elsif ($scale_loc eq "t")
	{
	$colorscale_length = $plot_width;
	$colorscale_thick = 0.013636364 * $page_height_in{$pagesize};
	$colorscale_offx = 0.5*$plot_width;
	$colorscale_offy = $plot_height
		+ 0.15 * $page_height_in{$pagesize};
	$colorscale_vh = "h";
	}
else
	{
	$colorscale_length = $plot_width;
	$colorscale_thick = 0.013636364 * $page_height_in{$pagesize};
	$colorscale_offx = 0.5*$plot_width;
	$colorscale_offy = -0.045454545 * $page_height_in{$pagesize};
	$colorscale_vh = "h";
	}

# figure out reasonable color and contour intervals
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
	$color_int = 1.02 * ($zmax - $zmin)/($ncolors_use - 1);
	$color_start = $zmin - 0.01*($zmax - $zmin);
	$color_end = $color_start + $color_int * ($ncolors_use - 1);
	}

# get null plane level
if (!$grdview_null && $grdview_null_set)
	{
	$grdview_null = "$zmin/200/200/200";
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

# set contour control
if (!$contour_control && $contour_mode)
	{
	$contour_control = $contour_int;
	}

# come up with the filenames
$cmdfile = "$root.cmd";
$psfile = "$root.ps";
if ($color_mode && $file_cpt)
	{
	$cptfile = $file_cpt;
	}
else
	{
	$cptfile = "$root.cpt";
	}
$gmtfile = "gmtdefaults\$\$";

# set some gmtisms
$first_gmt = 1;
$first = "-X\$X_OFFSET -Y\$Y_OFFSET -K -V >! \$PS_FILE";
$middle = "-K -O -V >> \$PS_FILE";
$end = "-O -V >> \$PS_FILE";

# set macro gmt default settings
$gmt_def = "MEASURE_UNIT/inch";
push(@gmt_macro_defs, $gmt_def);
if ($gmt_version eq "3.0"
	|| $gmt_version eq "3.1")
	{
	$gmt_def = "PAPER_WIDTH/$page_width_in{$pagesize}";
	push(@gmt_macro_defs, $gmt_def);
	}
else
	{
	$gmt_def = "PAPER_MEDIA/$page_gmt_name{$pagesize}+";
	push(@gmt_macro_defs, $gmt_def);
	}
$gmt_def = "ANOT_FONT/Helvetica";
push(@gmt_macro_defs, $gmt_def);
$gmt_def = "LABEL_FONT/Helvetica";
push(@gmt_macro_defs, $gmt_def);
$gmt_def = "HEADER_FONT/Helvetica";
push(@gmt_macro_defs, $gmt_def);
$gmt_def = "ANOT_FONT_SIZE/$page_anot_font{$pagesize}";
push(@gmt_macro_defs, $gmt_def);
$gmt_def = "LABEL_FONT_SIZE/$page_anot_font{$pagesize}";
push(@gmt_macro_defs, $gmt_def);
$gmt_def = "HEADER_FONT_SIZE/$page_header_font{$pagesize}";
push(@gmt_macro_defs, $gmt_def);
$gmt_def = "FRAME_WIDTH/$frame_size";
push(@gmt_macro_defs, $gmt_def);
$gmt_def = "TICK_LENGTH/$tick_size";
push(@gmt_macro_defs, $gmt_def);
$gmt_def = "PAGE_ORIENTATION/LANDSCAPE";
push(@gmt_macro_defs, $gmt_def);
$gmt_def = "COLOR_BACKGROUND/0/0/0";
push(@gmt_macro_defs, $gmt_def);
$gmt_def = "COLOR_FOREGROUND/255/255/255";
push(@gmt_macro_defs, $gmt_def);
$gmt_def = "COLOR_NAN/255/255/255";
push(@gmt_macro_defs, $gmt_def);
$gmt_def = "DEGREE_FORMAT/3";
push(@gmt_macro_defs, $gmt_def);

# open the shellscript file
if (!open(FCMD,">$cmdfile"))
	{
	print "\a";
	die "Cannot open output file $cmdfile\nMacro $program_name aborted.\n";
	}

# write the shellscript header
print FCMD "#! /bin/csh -f\n";
print FCMD "#\n# Shellscript to create Postscript plot of data in grd file\n";
print FCMD "# Created by macro $program_name\n";
print FCMD "#\n# This shellscript created by following command line:\n";
print FCMD "# $program_name $command_line\n";

# Define shell variables
print FCMD "#\n# Define shell variables used in this script:\n";
print FCMD "set PS_FILE         = $psfile\n";
print FCMD "set CPT_FILE        = $cptfile\n";
print FCMD "set MAP_PROJECTION  = $projection\n";
print FCMD "set MAP_SCALE       = $projection_pars\n";
printf FCMD "set MAP_ZSCALE      = %1.5g\n", $map_zscale;
print FCMD "set MAP_REGION      = $bounds_plot/$zmin/$zmax\n";
printf FCMD "set X_OFFSET        = %1.5g\n", $xoffset;
printf FCMD "set Y_OFFSET        = %1.5g\n", $yoffset;
print FCMD "set DATA_FILE       = $file_data\n";
print FCMD "set INTENSITY_FILE  = $file_intensity\n";

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
# if ($color_mode == 2)
# 	{
# 	printf FCMD "#\n# Get shading array\n";
# 	printf FCMD "echo Getting shading array...\n";
# 	printf FCMD "echo Running grdgradient...\n";
# 	printf FCMD "grdgradient $file_use -A$azimuth -G\$DATA_FILE.grad -N -M\n";
# 	printf FCMD "echo Running grdhisteq...\n";
# 	printf FCMD "grdhisteq \$DATA_FILE.grad -G\$DATA_FILE.eq -N\n";
# 	printf FCMD "echo Running grdmath...\n";
# 	printf FCMD "grdmath \$DATA_FILE.eq $magnitude x = \$DATA_FILE.int\n";
# 	printf FCMD "/bin/rm -f \$DATA_FILE.grad \$DATA_FILE.eq\n";
# 	$file_shade = "\$DATA_FILE.int";
# 	}

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
	printf FCMD "grdgradient $file_use -A90 -G\$DATA_FILE.drvx -M\n";
	printf FCMD "echo Running grdgradient to get y component of the gradient...\n";
	printf FCMD "grdgradient $file_use -A0 -G\$DATA_FILE.drvy -M\n";

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
if ($color_mode == 3 && $file_intensity && $stretch_shade)
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
elsif ($color_mode == 3 && $file_intensity)
	{
	$file_shade = "\$INTENSITY_FILE";
	}

# get color by slope magnitude if needed
elsif ($color_mode >= 4)
	{
	printf FCMD "#\n# Get slope array\n";
	printf FCMD "echo Getting slope array...\n";
	printf FCMD "echo Running grdgradient to get x component of the gradient...\n";
	printf FCMD "grdgradient $file_use -A90 -G\$DATA_FILE.drvx -M\n";
	printf FCMD "echo Running grdgradient to get y component of the gradient...\n";
	printf FCMD "grdgradient $file_use -A0 -G\$DATA_FILE.drvy -M\n";

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

# figure out labels
$nlabels = 0;
if ($labels)
	{
	@labels_split = split(/:/, $labels);
	$nlabels = @labels_split;
	if ($nlabels > 0)
		{
		$tlabel = shift(@labels_split);
		$tlabel =~ s/\\/\//g;
		}
	if ($nlabels > 1)
		{
		$slabel = shift(@labels_split);
		$slabel =~ s/\\/\//g;
		}
	}
if ($nlabels < 1)
	{
	$tlabel = "Data File \$DATA_FILE";
	}
if ($nlabels < 2)
	{
	$slabel = "Data Values";
	}

# set basemap axes annotation
if ($tick_info)
	{
	$axes = $tick_info;
	if (!($tick_info =~ /.*\..*/))
		{
		$axes = "$axes:.\"$tlabel\":";
		}
	}
else
	{
	# figure out some reasonable tick intervals for the basemap
	&GetBaseTick;
	if ($view_azimuth >= 0.0 && $view_azimuth < 90.0)
		{
		$axes = "$base_tick/$base_tick/$base_tick_z:.\"$tlabel\":NEZ";
		}
	elsif ($view_azimuth >= 90.0 && $view_azimuth < 180.0)
		{
		$axes = "$base_tick/$base_tick/$base_tick_z:.\"$tlabel\":SEZ";
		}
	elsif ($view_azimuth >= 180.0 && $view_azimuth < 270.0)
		{
		$axes = "$base_tick/$base_tick/$base_tick_z:.\"$tlabel\":WSZ";
		}
	elsif ($view_azimuth >= 270.0 && $view_azimuth < 360.0)
		{
		$axes = "$base_tick/$base_tick/$base_tick_z:.\"$tlabel\":WNZ";
		}
	}

# do grdview plot
if ($color_mode)
	{
	printf FCMD "#\n# Make 3D view\n";
	printf FCMD "echo Running grdview...\n";
	if ($color_mode == 4)
		{
		printf FCMD "grdview $file_slope \\\n\t";
		}
	else
		{
		printf FCMD "grdview $file_use \\\n\t";
		}
	printf FCMD "-J\$MAP_PROJECTION\$MAP_SCALE -Jz\$MAP_ZSCALE \\\n\t";
	printf FCMD "-E$view_control \\\n\t";
	printf FCMD "-R\$MAP_REGION \\\n\t";
	if ($color_mode != 6)
		{
		printf FCMD "-C\$CPT_FILE \\\n\t";
		}
	if ($grdview_null)
		{
		printf FCMD "-N$grdview_null \\\n\t";
		}
	if ($grdview_zlevel)
		{
		printf FCMD "-Z$grdview_zlevel \\\n\t";
		}
	if ($color_mode == 2 || $color_mode == 3)
		{
		printf FCMD "-I$file_shade \\\n\t";
		}
	elsif ($color_mode == 5)
		{
		printf FCMD "-I$file_slope \\\n\t";
		}
	if ($file_drape)
		{
		printf FCMD "-G$file_drape \\\n\t";
		}
	if ($color_mode <= 5)
		{
		printf FCMD "-Qi$dpi \\\n\t";
		}
	elsif ($color_mode >= 6)
		{
		printf FCMD "-Qm \\\n\t";
		}
	if ($color_mode == 7)
		{
		printf FCMD "-W$grdview_contour_pen \\\n\t";
		}
	if ($portrait)
	    {
	    printf FCMD "-P ";
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

# do psscale plot
if ($color_mode && $color_mode < 7 && $color_pallette < 5)
	{
	printf FCMD "#\n# Make color scale\n";
	printf FCMD "echo Running psscale...\n";
	printf FCMD "psscale -C\$CPT_FILE \\\n\t";
	printf FCMD "-D%.4f/%.4f/%.4f/%.4f%s \\\n\t", 
		$colorscale_offx,$colorscale_offy,
		$colorscale_length,$colorscale_thick, 
		$colorscale_vh;
	if ($gmt_version eq "3.0")
		{
		print FCMD "-B\":.$slabel:\" \\\n\t";
		}
	else
		{
		print FCMD "-B\":$slabel:\" \\\n\t";
		}
	if ($stretch_color)
		{
		print FCMD "-L \\\n\t";
		}
	if ($portrait)
		{
		printf FCMD "-P ";
		}
	printf FCMD "$middle\n";
	}

# do pstext plot
if (@text)
	{
	printf FCMD "#\n# Make text labels\n";
	printf FCMD "echo Running pstext...\n";
	printf FCMD "pstext -J\$MAP_PROJECTION\$MAP_SCALE -Jz\$MAP_ZSCALE \\\n\t";
	printf FCMD "-E$view_control \\\n\t";
	printf FCMD "-R\$MAP_REGION \\\n\t";
	printf FCMD "$middle <<EOT\n";
	foreach $text_info (@text) {
	    ($tx, $ty, $tsize, $tangle, $font, $just, $txt) = $text_info
		=~ /^(\S+)\/(\S+)\/(\S+)\/(\S+)\/(\S+)\/(\S+)\/(.+)/;
	    print "text_info:$text_info\n";
	    print "txt:$txt\n";
	    print FCMD "$tx $ty $tsize $tangle $font $just $txt\n";
	    }
	print FCMD "EOT\n";
	}

# do psbasemap plot
printf FCMD "#\n# Make basemap\n";
printf FCMD "echo Running psbasemap...\n";
printf FCMD "psbasemap -J\$MAP_PROJECTION\$MAP_SCALE -Jz\$MAP_ZSCALE \\\n\t";
	printf FCMD "-E$view_control \\\n\t";
printf FCMD "-R\$MAP_REGION \\\n\t";
printf FCMD "-B$axes \\\n\t";
if ($grdview_zlevel)
	{
	printf FCMD "-Z$grdview_zlevel \\\n\t";
	}
if ($length_scale)
	{
	printf FCMD "-L$length_scale \\\n\t";
	}
if ($unix_stamp_on && $unix_stamp)
	{
	printf FCMD "-U$unix_stamp \\\n\t";
	}
elsif ($unix_stamp_on)
	{
	printf FCMD "-U \\\n\t";
	}
if ($portrait)
	{
	printf FCMD "-P ";
	}
printf FCMD "$end\n";

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
print FCMD "#\n# Run $ps_viewer\n";
if ($ps_viewer eq "xpsview" && $portrait)
	{
	$view_pageflag = "-ps $pagesize -maxp $xpsview_mem{$pagesize}";
	}
elsif ($ps_viewer eq "xpsview" && $landscape)
	{
	$view_pageflag = "-ps $pagesize -or landscape -maxp $xpsview_mem{$pagesize}";
	}
elsif ($ps_viewer eq "pageview" && $portrait)
	{
	$view_pageflag = "-w $page_width_in{$pagesize} -h $page_height_in{$pagesize}";
	}
elsif ($ps_viewer eq "pageview" && $landscape)
	{
	$view_pageflag = "-w $page_height_in{$pagesize} -h $page_width_in{$pagesize}";
	}
if ($no_view_ps)
	{
	print FCMD "#echo Running $ps_viewer in background...\n";
	print FCMD "#$ps_viewer $view_pageflag \$PS_FILE &\n";
	}
else
	{
	print FCMD "echo Running $ps_viewer in background...\n";
	print FCMD "$ps_viewer $view_pageflag \$PS_FILE &\n";
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
		print "    3D Color Fill\n";
		}
	elsif ($color_mode == 2)
		{
		print "    3D Color Shaded Relief\n";
		}
	elsif ($color_mode == 3 && $file_intensity
		&& $stretch_shade)
		{
		print "    3D Color Shaded by Equalized Intensity File\n";
		}
	elsif ($color_mode == 3 && $file_intensity)
		{
		print "    3D Color Shaded by Unaltered Intensity File\n";
		}
	elsif ($color_mode == 4)
		{
		print "    3D Color Fill of Slope Magnitude\n";
		}
	elsif ($color_mode == 5)
		{
		print "    3D Color Shaded by Slope Magnitude\n";
		}
	elsif ($color_mode == 6)
		{
		print "    3D Mesh\n";
		}
	elsif ($color_mode == 7)
		{
		print "    3D Mesh with Contours\n";
		}
	if ($contour_mode)
		{
		print "    Contour Plot\n";
		}
	if ($color_mode && $color_pallette < 5)
		{
		if ($colorscale_vh eq "v")
			{
			print "    Vertical Color Scale\n";
			} 
		else
			{
			print "    Horizontal Color Scale\n";
			} 
		}
	if (@text)
		{
		print "    ", scalar(@text), " Text labels\n";
		}
	if ($length_scale)
		{
		print "    Map distance scale\n";
		}
	if ($unix_stamp_on && $unix_stamp)
		{
		print "    Unix time stamp: $unix_stamp\n";
		}
	elsif ($unix_stamp_on)
		{
		print "    Unix time stamp\n";
		}
	print "\n  Input Files:\n";
	print "    Data GRD File:            $file_data\n";
	if ($file_intensity)
		{
		print "    Intensity GRD File:       $file_intensity\n";
		}
	if ($file_drape)
		{
		print "    Drape GRD File:           $file_drape\n";
		}
	print "\n  Output Files:\n";
	print "    Output plot name root:    $root\n";
	print "    Color pallette table:     $cptfile\n";
	print "    Plotting shellscript:     $cmdfile\n";
	print "    Plot file:                $psfile\n";
	print "\n  Plot Attributes:\n";
	printf "    Plot width:               %.4f\n", $plot_width;
	printf "    Plot height:              %.4f\n", $plot_height;
	printf "    X-axis length:            %.4f\n", $plot_width_xaxis;
	printf "    Y-axis length:            %.4f\n", $plot_height_yaxis;
	print "    Page size:                $pagesize\n";
	print "    Page width:               $width\n";
	print "    Page height:              $height\n";
	print "    Projection:               -J$projection$projection_pars\n";
	print "    Z Projection:             -Jz$map_zscale\n";
	print "    Axes annotation:          $axes\n";
	if ($portrait)
		{
		print "    Orientation:              portrait\n";
		}
	else
		{
		print "    Orientation:              landscape\n";
		}
	if ($color_mode)
		{
		print "    Number of colors:         $ncolors\n";
		print "    Color Pallette:           ", 
			"@color_pallette_names[$color_pallette - 1]\n";
		}
	print "\n  Grid Data Attributes:\n";
	if ($data_down)
		{
		print "    Data treated as positive downward\n";
		}
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
	if ($contour_mode)
		{
		print "    Contour control:          $contour_control\n";
		}
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
	if ($dpi && $color_mode)
		{
		printf "    Image dots-per-inch:      $dpi\n";
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
	if ($length_scale || $grdview_null
		|| $grdview_mesh_pen || $grdview_contour_pen)
		{
		print "\n  Miscellaneous Plotting Controls:\n";
		}
	if ($length_scale)
		{
		printf "    Length scale:             $length_scale\n";
		}
	if ($grdview_null)
		{
		printf "    Null plane level:         $grdview_null\n";
		}
	if ($grdview_mesh_pen)
		{
		printf "    Mesh pen attributes:      $grdview_mesh_pen\n";
		}
	if ($grdview_contour_pen)
		{
		printf "    Contour pen attributes:   $grdview_contour_pen\n";
		}
	if ($grdview_zlevel)
		{
		printf "    Z level:                  $grdview_zlevel\n";
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
print "\nPlot generation shellscript <$cmdfile> created.\n";
print "\nInstructions:\n";
print "  Execute <$cmdfile> to generate Postscript plot <$psfile>.\n";
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
sub abs {

	# make local variables
	local ($abs);
	
	# get the absolute value of the argument
	if ($_[0] <= 0.0)
		{
		$abs = -$_[0];
		}
	else
		{
		$abs = $_[0];
		}
}

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
			$geographic = 1;
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
			$geographic = 1;
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

	# figure out some reasonable tick intervals for the basemap z axis
	$base_tick_z = ($zmax - $zmin) / 5;
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
				. \":::::::\" . \$rest;";
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
