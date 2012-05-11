eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
                    & eval 'exec perl -S $0 $argv:q'
                         if 0;
#--------------------------------------------------------------------
#    The MB-system:	mbm_grdplot.perl	8/6/95
#    $Id$
#
#    Copyright (c) 1993-2012 by
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
#   mbm_grdplot
#
# Purpose:
#   Macro to generate a shellscript of GMT commands which,
#   when executed, will generate a Postscript plot of gridded
#   data.  Several styles of plots can be generated, including
#   color fill maps, contour maps, color fill maps overlaid with
#   contours, shaded relief color maps, slope magnitude maps,
#   coastline maps, text labels, and xy data
#   in lines or symbols. Five different color schemes are included.
#   The plot will be scaled to fit on the specified page size
#   or, if the scale is user defined, the page size will be
#   chosen in accordance with the plot size. The primary purpose
#   of this macro is to allow the simple, semi-automated
#   production of nice looking maps with a few command line
#   arguments. For users seeking more control over the plot
#   appearance, a number of additional optional arguments are
#   provided. Truly ambitious users may edit the plot shellscript
#   to take advantage of GMT capabilites not supported by this
#   macro.
#
# Basic Usage:
#   mbm_grdplot -Ifile [-Amagnitude[/azimuth/elevation] -C[contour_control]
#            -Gcolor_mode -H -Kintensity_file -Oroot -Ppagesize
#            -S[color/shade] -T -Uorientation -V
#            -Wcolor_style[/pallette[/ncolors]] ]
#
# Additional Options:
#            [-Btickinfo -Dflipcolor/flipshade -Fcontour_file
#            -Jprojection[/scale | width] -Ltitle[:scale_label]
#            -Mmisc -Q -Rw/e/s/n -X -Y -Zmin/max]
#
# Miscellaneous Options:
#            [-MGDgmtdef/value -MGFscale_loc";
#            -MGL[f][x]lon0/lat0/slat/length[m]
#            -MGQdpi -MGSscalefactor -MGTx/y/size/angle/font/just/text
#            -MGU[/dx/dy/][label] -MCAanot_int/[ffont_size][aangle][/r/g/b][o]]
#            -MCGgap/width -MCQcut -MCT[+|-][gap/length][:LH] -MCWtype[pen]
#            -MNFformat -MNIdatalist
#            -MMM[pingnumber_tick/pingnumber_annot/pingnumber_tick_len]
#            -MNN[time_tick/time_annot/date_annot/time_tick_len[/name_hgt] | F | FP]
#            -MTCfill -MTDresolution -MTGfill -MTIriver[/pen]
#            -MTNborder[/pen] -MTSfill -MTWpen
#            -MXGfill -MXIxy_file -MXSsymbol/size -MXWpen]
#
# Author:
#   David W. Caress
#   Lamont-Doherty Earth Observatory
#   Palisades, NY  10964
#   October 19, 1994
#
# Version:
#   $Id$
#
# Revisions:
#   $Log: mbm_grdplot.perl,v $
#   Revision 5.33  2008/09/11 20:06:45  caress
#   Checking in updates made during cruise AT15-36.
#
#   Revision 5.32  2008/07/10 06:43:40  caress
#   Preparing for 5.1.1beta20
#
#   Revision 5.31  2008/05/16 22:36:21  caress
#   Release 5.1.1beta18
#
#   Revision 5.30  2008/02/12 02:52:50  caress
#   Working on adding sealevel colormap, but haven't finished it at this point.
#
#   Revision 5.29  2008/01/14 17:47:41  caress
#   Fixed typo.
#
#   Revision 5.28  2007/10/08 04:30:55  caress
#   Added some large page definitions.
#
#   Revision 5.27  2007/03/02 20:34:26  caress
#   Fixed plotting of ping/shot number annotation along navigation tracks.
#
#   Revision 5.26  2006/12/15 21:42:49  caress
#   Incremental CVS update.
#
#   Revision 5.25  2006/09/11 18:55:52  caress
#   Changes during Western Flyer and Thomas Thompson cruises, August-September
#   2006.
#
#   Revision 5.24  2006/08/09 22:36:23  caress
#   The macro mbm_grdplot now handles calls for a linear plot with
#   decreasing x values (e.g. using -Jx-0.01/20 to specify the projection).
#
#   Revision 5.23  2006/08/04 03:56:41  caress
#   Working towards 5.1.0 release.
#
#   Revision 5.22  2006/07/05 19:50:21  caress
#   Working towards 5.1.0beta
#
#   Revision 5.21  2006/06/22 04:45:42  caress
#   Working towards 5.1.0
#
#   Revision 5.20  2006/06/16 19:30:58  caress
#   Check in after the Santa Monica Basin Mapping AUV Expedition.
#
#   Revision 5.19  2006/01/20 17:39:15  caress
#   Working towards 5.0.8
#
#   Revision 5.18  2006/01/18 15:09:27  caress
#   Now parses grdinfo output from GMT 4.1.
#
#   Revision 5.17  2006/01/06 18:26:26  caress
#   Working towards 5.0.8
#
#   Revision 5.16  2005/11/05 01:34:20  caress
#   Much work over the past two months.
#
#   Revision 5.15  2005/04/07 04:12:39  caress
#   Implemented feature to generate contours from a separate grid file. Contributed by Gordon Keith.
#
#   Revision 5.14  2005/03/25 04:05:39  caress
#   Fixed handling of tickinfo string.
#   For mbm_plot only, added control on filename annotation direction.
#
#   Revision 5.13  2004/12/18 01:31:26  caress
#   Working towards release 5.0.6.
#
#   Revision 5.12  2004/10/06 18:56:11  caress
#   Release 5.0.5 update.
#
#   Revision 5.11  2004/09/16 19:11:48  caress
#   Supports postscript viewer ggv.
#
#   Revision 5.10  2004/06/18 04:16:55  caress
#   Adding support for segy i/o and working on support for Reson 7k format 88.
#
#   Revision 5.9  2003/04/17 20:42:48  caress
#   Release 5.0.beta30
#
#   Revision 5.8  2002/11/14 03:50:19  caress
#   Release 5.0.beta27
#
#   Revision 5.7  2002/08/02 01:00:05  caress
#   Release 5.0.beta22
#
#   Revision 5.6  2002/04/06 02:51:54  caress
#   Release 5.0.beta16
#
#   Revision 5.5  2001/12/18 04:26:12  caress
#   Version 5.0.beta11.
#
# Revision 5.4  2001/11/02  21:07:40  caress
# Adjusted handling of segmented xy files.
#
# Revision 5.3  2001/10/10  23:56:01  dcaress
# Regrettably, I don't remember what I changed...
#
#   Revision 5.2  2001-06-02 23:59:24-07  caress
#   Release 5.0.beta01
#
#   Revision 5.1  2001/03/22 21:05:45  caress
#   Trying to make release 5.0.beta0
#
# Revision 5.0  2000/12/01  22:58:01  caress
# First cut at Version 5.0.
#
# Revision 4.16  2000/10/03  21:42:17  caress
# Snapshot for Dale.
#
# Revision 4.15  1999/12/29  00:17:55  caress
# Release 4.6.8
#
# Revision 4.14  1999/08/08  04:17:04  caress
# Added coastline plots.
#
# Revision 4.13  1999/06/25  17:55:47  caress
# I must have changed something!
#
# Revision 4.12  1999/05/06  23:46:32  caress
# Release 4.6a
#
# Revision 4.11  1999/04/16  01:25:51  caress
# Version 4.6 final release?
#
# Revision 4.10  1999/04/15  19:28:52  caress
# Fixed sprintf statements.
#
# Revision 4.9  1999/02/04  23:39:54  caress
# MB-System version 4.6beta7
#
# Revision 4.8  1999/01/26  19:46:58  caress
# Fixed parsing of grdinfo output from GMT 3.1.
#
# Revision 4.7  1998/10/05  17:00:15  caress
# MB-System version 4.6beta
#
# Revision 4.6  1997/04/21  16:54:41  caress
# MB-System 4.5 Beta Release.
#
# Revision 4.6  1997/04/17  15:06:49  caress
# MB-System 4.5 Beta Release
#
# Revision 4.5  1995/11/22  22:46:40  caress
# Check in during general flail.
#
# Revision 4.4  1995/09/28  18:05:43  caress
# Various bug fixes working toward release 4.3.
#
# Revision 4.3  1995/08/17  14:52:53  caress
# Revision for release 4.3.
#
# Revision 4.2  1995/05/12  17:43:23  caress
# Made exit status values consistent with Unix convention.
# 0: ok  nonzero: error
#
# Revision 4.1  1995/02/14  19:50:31  caress
# Version 4.2
#
# Revision 4.0  1994/10/21  11:49:39  caress
# Release V4.0
#
# Revision 1.1  1994/10/21  11:36:58  caress
# Initial revision
#
#
#
$program_name = "mbm_grdplot";

# use the Posix module
use POSIX;

# set degree to radians conversion
$DTR = 3.1415926 / 180.0;

# set page size database
@page_size_names = (
	"a", "b", "c", "d", "e", "f", "e1",
	"a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9", "a10",
	"b0", "b1", "b2", "b3", "b4", "b5", "b6", "b7", "b8", "b9", "b10",
	"c0", "c1", "c2", "c3", "c4", "c5", "c6", "c7",
	"m1", "m2", "m3", "m4", "m5", "m6");
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
	"c7",    3.20,   "m1",   54.00,   "m2",   54.00,   "m3",   54.00,
	"m4",   60.00,   "m5",   60.00,   "m6",   60.00);
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
	"c7",    4.50,   "m1",   72.00,   "m2",   84.00,   "m3",   96.00,
	"m4",   72.00,   "m5",   84.00,   "m6",   96.00);
%page_anot_font = (
	"a",     8,   "b",    12,   "c",    16,   "d",    24,
	"e",    24,   "f",    24,   "e1",   24,   "a0",   24,
	"a1",   24,   "a2",   16,   "a3",   12,   "a4",    8,
	"a5",    6,   "a6",    6,   "a7",    6,   "a8",    4,
	"a9",    4,   "a10",   4,   "b0",   24,   "b1",   24,
	"b2",   16,   "b3",   16,   "b4",   12,   "b5",    8,
	"b6",    6,   "b7",    4,   "b8",    4,   "b9",    4,
	"b10",   4,   "c0",   24,   "c1",   24,   "c2",   16,
	"c3",   12,   "c4",    8,   "c5",    6,   "c6",    6,
	"c7",    6,   "m1",   24,   "m2",   24,   "m3",   24,
	"m4",   24,   "m5",   24,   "m6",   24);
%page_header_font =(
	"a",    10,   "b",    15,   "c",    20,   "d",    30,
	"e",    30,   "f",    30,   "e1",   30,   "a0",   30,
	"a1",   30,   "a2",   20,   "a3",   15,   "a4",   10,
	"a5",    8,   "a6",    8,   "a7",    8,   "a8",    5,
	"a9",    5,   "a10",   5,   "b0",   30,   "b1",   30,
	"b2",   20,   "b3",   20,   "b4",   15,   "b5",   10,
	"b6",    8,   "b7",    5,   "b8",    5,   "b9",    5,
	"b10",   5,   "c0",   30,   "c1",   30,   "c2",   20,
	"c3",   15,   "c4",   10,   "c5",    8,   "c6",    8,
	"c7",    8,   "m1",   30,   "m2",   30,   "m3",   30,
	"m4",   30,   "m5",   30,   "m6",   30);
%page_gmt_name =     (
	"a",     "archA",   "b",     "archB",   "c",     "archC",   "d",     "archD",
	"e",     "archE",   "f",     "B0",      "e1",    "B0",      "a0",    "A0",
	"a1",    "A1",      "a2",    "A2",      "a3",    "A3",      "a4",    "A4",
	"a5",    "A5",      "a6",    "A6",      "a7",    "A7",      "a8",    "A8",
	"a9",    "A9",      "a10",   "A10",     "b0",    "B0",      "b1",    "B1",
	"b2",    "B2",      "b3",    "B3",      "b4",    "B4",      "b5",    "B5",
	"b6",    "A6",      "b7",    "A7",      "b8",    "A8",      "b9",    "A9",
	"b10",   "A10",     "c0",    "B0",      "c1",    "B1",      "c2",    "B2",
	"c3",    "B3",      "c4",    "B4",      "c5",    "B5",      "c6",    "B6",
	"c7",    "B7",      "m1", "Custom_4241x5655",    "m2",   "Custom_4241x6578",
	"m3",   "Custom_4241x7540",  "m4",   "Custom_4712x5655",
	"m5",   "Custom_4712x6578",  "m6",   "Custom_4712x7540");
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
	"c7",    "4m",   "m1",   "16m",   "m2",   "16m",   "m3",   "16m",
	"m4",   "16m",   "m5",   "16m",   "m6",   "16m");

# set default number of colors
$ncpt = 11;

# define color pallettes

@color_pallette_names = (   "Haxby Colors",
			    "High Intensity Colors",
			    "Low Intensity Colors",
			    "Grayscale",
			    "Uniform Gray",
			    "Uniform Black",
			    "Uniform White",
			    "Sealevel");

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

# color pallette 8 - Sealevel (above sealevel - Haxby colors used below sealevel
	@cptbr8 = (250, 245, 240, 235, 230, 221, 212, 211, 210, 205, 200);
	@cptbg8 = (250, 240, 230, 221, 212, 201, 190, 180, 170, 160, 150);
	@cptbb8 = (120, 112, 104,  96,  88,  80,  72,  64,  56,  48,  40);

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
&MBGetopts('A:a:B:b:C%c%D%d%F:f:G%g%HhI:i:J:j:K:k:L:l:M+m+O:o:P:p:QqR:r:S%s%TtU:u:VvW:w:XxYyZ:z:');
$shade_control = 	($opt_A || $opt_a);
$tick_info = 		($opt_B || $opt_b);
$contour_mode = 	($flg_C || $flg_c);
$contour_control = 	($opt_C || $opt_c);
$color_flip_mode = 	($flg_D || $flg_d);
$color_flip_control = 	($opt_D || $opt_d);
$contour_file=		($opt_F || $opt_f);
$color_mode =   	($opt_G || $opt_g || $flg_G || $flg_g);
$help =    		($opt_H || $opt_h);
$file_data =    	($opt_I || $opt_i);
$map_scale =    	($opt_J || $opt_j);
$file_intensity =    	($opt_K || $opt_k);
$labels =    		($opt_L || $opt_l);
$misc = 		($opt_M || $opt_m);
$root =    		($opt_O || $opt_o);
$pagesize = 		($opt_P || $opt_p);
$no_view_ps = 		($opt_Q || $opt_q);
$bounds = 		($opt_R || $opt_r);
$stretch_mode = 	($flg_S || $flg_s);
$stretch_control = 	($opt_S || $opt_s);
$coast_control = 	($opt_T || $opt_t);
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
	print "\nVersion: $Id$\n";
	print "\nMacro to generate a shellscript of GMT commands which, \n";
	print "when executed, will generate a Postscript plot of gridded \n";
	print "data.  Several styles of plots can be generated, including \n";
	print "color fill maps, contour maps, color fill maps overlaid with\n";
	print "contours, shaded relief color maps, text labels, and xy data\n";
	print "in lines or symbols. Five different color schemes are included.\n";
	print "The plot will be scaled to fit on the specified page size \n";
	print "or, if the scale is user defined, the page size will be \n";
	print "chosen in accordance with the plot size. The primary purpose \n";
	print "of this macro is to allow the simple, semi-automated \n";
	print "production of nice looking maps with a few command line \n";
	print "arguments. For users seeking more control over the plot \n";
	print "appearance, a number of additional optional arguments are \n";
	print "provided. Truly ambitious users may edit the plot shellscript \n";
	print "to take advantage of GMT capabilites not supported by this \n";
	print "macro.\n";
	print "\nBasic Usage: \n";
	print "\t$program_name -Ifile [-Amagnitude[/azimuth/elevation] -C[contour_control] \n";
	print "\t\t-Gcolor_mode -H -Kintensity_file -Oroot -Ppagesize -S[color/shade] -T\n";
	print "\t\t-Uorientation -V -Wcolor_style[/pallette[/ncolors]] ]\n";
	print "Additional Options:\n";
	print "\t\t[-Btickinfo -Dflipcolor/flipshade -Fcontour_file\n";
	print "\t\t-Jprojection[/scale | width] -Ltitle[:scale_label]\n";
	print "\t\t-Mmisc -Q -Rw/e/s/n -X -Y -Zmin/max[/mode] ]\n";
	print "Miscellaneous Options:\n";
	print "\t\t[-MGDgmtdef/value  -MGFscale_loc\n";
	print "\t\t-MGL[f][x]lon0/lat0/slat/length[m]\n";
	print "\t\t-MGQdpi -MGSscalefactor -MGTx/y/size/angle/font/just/text\n";
	print "\t\t-MGU[/dx/dy/][label] -MCAanot_int/[ffont_size][aangle][/r/g/b][o]]\n";
	print "\t\t-MCGgap/width -MCQcut -MCT[+|-][gap/length][:LH] -MCWtype[pen]\n";
	print "\t\t-MNA[name_hgt[/P] | P] -MNFformat -MNIdatalist\n";
	print "\t\t-MNN[time_tick/time_annot/date_annot/time_tick_len[/name_hgt] | F | FP]\n";
	print "\t\t-MNP[pingnumber_tick/pingnumber_annot/pingnumber_tick_len]\n";
	print "\t\t-MTCfill -MTDresolution -MTGfill -MTIriver[/pen]\n";
	print "\t\t-MTNborder[/pen] -MTSfill -MTWpen\n";
	print "\t\t-MXGfill -MXIxy_file -MXM -MXSsymbol/size -MXWpen]\n";
	exit 0;
	}

# check for input file
if (!$file_data)
	{
	print "\a";
	die "\nNo input file specified!\n$program_name aborted\n";
	}
else
	{
	if ($file_data =~ /\S+=.+/)
		{
		($file_check) = $file_data =~ /(\S+)=.+/;
		}
	else
		{
		$file_check = $file_data;
		}
	if (! -r $file_check)
		{
		print "\a";
		die "\nSpecified input file $file_check cannot be opened!\n$program_name aborted\n";
		}
	}
if ($file_intensity)
	{
	if ($file_intensity =~ /\S+=.+/)
		{
		($file_check) = $file_intensity =~ /(\S+)=.+/;
		}
	else
		{
		$file_check = $file_intensity;
		}
	if (! -r $file_check)
		{
		print "\a";
		die "\nSpecified intensity input file $file_check cannot be opened!\n$program_name aborted\n";
		}
	}
if ($color_mode == 3 && !$file_intensity)
	{
	print "\a";
	die "\nShading with intensity file set but no intensity input file specified!\n$program_name aborted\n";
	}
if ($contour_file)
        {
        if ($contour_file =~ /\S+=.+/)
                {
                ($file_check) = $contour_file =~ /(\S+)=.+/;
                }
        else
                {
                $file_check = $contour_file;
                }
        if (! -r $file_check)
                {
                print "\a";
                die "\nSpecified contour input file $file_check cannot be opened!\n$program_name aborted\n";
                }
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
			    =~ /^[Gg][Tt](\S+)\/(\S+)\/(\S+)\/(\S+)\/(\S+)\/(\S+)\/(.+)/;
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

		# deal with grdcontour options
		##############################

		# set contour annotation interval
		if ($cmd =~ /^[Cc][Aa]./)
			{
			($contour_anot_int) = $cmd =~
				/^[Cc][Aa](\S+)/;
			}

		# set contour gap specs
		if ($cmd =~ /^[Cc][Gg]./)
			{
			($contour_gap) = $cmd =~
				/^[Cc][Gg](\S+)/;
			}

		# set contour point lower limit
		if ($cmd =~ /^[Cc][Qq]./)
			{
			($contour_cut) = $cmd =~
				/^[Cc][Qq](\S+)/;
			}

		# set contour ticks
		if ($cmd =~ /^[Cc][Tt]/)
			{
			$contour_tick_on = 1;
			}
		elsif ($cmd =~ /^[Cc][Tt]./)
			{
			($contour_tick) = $cmd =~
				/^[Cc][Tt](\S+)/;
			$contour_tick_on = 1;
			}

		# set contour pen attributes
		if ($cmd =~ /^[Cc][Ww]./)
			{
			($contour_pen) = $cmd =~
				/^[Cc][Ww](\S+)/;
			}

		# deal with swath navigation options
		##############################

		# set filename annotation
		if ($cmd =~ /^[Nn][Aa]\S+\/P/)
			{
			($nav_name_hgt) = $cmd =~ /^[Nn][Aa](\S+)\/P/;
			$name_mode = 1;
			$name_perp = 1;
			}
		elsif ($cmd =~ /^[Nn][Aa]P/)
			{
			$name_mode = 1;
			$name_perp = 1;
			$nav_name_hgt = 0.15;
			}
		elsif ($cmd =~ /^[Nn][Aa]\S+/)
			{
			($nav_name_hgt) = $cmd =~ /^[Nn][Aa](\S+)/;
			$name_mode = 1;
			$name_perp = 0;
			}
		elsif ($cmd =~ /^[Nn][Aa]/)
			{
			$name_mode = 1;
			$name_perp = 0;
			$nav_name_hgt = 0.15;
			}

		# set swath file format id
		if ($cmd =~ /^[Nn][Ff]./)
			{
			($swathformat) = $cmd =~
				/^[Nn][Ff](\S+)/;
			}

		# set swath file datalist of which to plot navigation
		if ($cmd =~ /^[Nn][Ii]./)
			{
			($swathnavdatalist) = $cmd =~
				/^[Nn][Ii](\S+)/;
			}

		# set swath navigation parameters
		if ($cmd =~ /^[Nn][Nn]./)
			{
			($navigation_control) = $cmd =~
				/^[Nn][Nn](\S+)/;
			$navigation_mode = 1;
			}
		elsif ($cmd =~ /^[Nn][Nn]/)
			{
			$navigation_mode = 1;
			}

		# set pingnumber annotation
		if ($cmd =~ /^[Nn][Pp]./)
			{
			($pingnumber_control) = $cmd =~ /^[Nn][Pp](.+)/;
			$pingnumber_mode = 1;
			if ($pingnumber_control =~ /\S+\/\S+\/\S+/)
				{
				($pingnumber_tick, $pingnumber_annot, $pingnumber_tick_len)
						= $pingnumber_control =~  /(\S+)\/(\S+)\/(\S+)/;
				}
			elsif ($pingnumber_control =~ /\S+\/\S+/)
				{
				($pingnumber_tick, $pingnumber_annot)
						= $pingnumber_control =~  /(\S+)\/(\S+)/;
				$pingnumber_tick_len = 0.10;
				}
			elsif ($pingnumber_control =~ /\S+/)
				{
				($pingnumber_tick) = $pingnumber_control =~  /(\S+)/;
				$pingnumber_tick_len = 0.10;
				$pingnumber_annot = 100;
				}
			}
		elsif ($cmd =~ /^[Nn][Pp]/)
			{
			$pingnumber_mode = 1;
			$pingnumber_tick_len = 0.10;
			$pingnumber_annot = 100;
			$pingnumber_tick = 50;
			}

		# deal with pscoast options
		##############################

		# set pscoast lake fill
		if ($cmd =~ /^[Tt][Cc]./)
			{
			($coast_lakefill) = $cmd =~ /^[Tt][Cc](.+)/;
			$coast_control = 1;
			}

		# set pscoast resolution
		if ($cmd =~ /^[Tt][Dd]./)
			{
			($coast_resolution) = $cmd =~ /^[Tt][Dd](.+)/;
			$coast_control = 1;
			}

		# set pscoast dry fill
		if ($cmd =~ /^[Tt][Gg]./)
			{
			($coast_dryfill) = $cmd =~ /^[Tt][Gg](.+)/;
			$coast_control = 1;
			}

		# set pscoast rivers
		if ($cmd =~ /^[Tt][Ii]./)
			{
			($coast_river) = $cmd =~ /^[Tt][Ii](.+)/;
			$coast_control = 1;
			}

		# set pscoast national boundaries
		if ($cmd =~ /^[Tt][Nn]./)
			{
			($coast_boundary) = $cmd =~ /^[Tt][Nn](.+)/;
			push(@coast_boundaries, $coast_boundary);
			$coast_control = 1;
			}

		# set pscoast wet fill
		if ($cmd =~ /^[Tt][Ss]./)
			{
			($coast_wetfill) = $cmd =~ /^[Tt][Ss](.+)/;
			$coast_control = 1;
			}

		# set pscoast coastline pen
		if ($cmd =~ /^[Tt][Ww]./)
			{
			($coast_pen) = $cmd =~ /^[Tt][Ww](.+)/;
			$coast_control = 1;
			}

		# deal with psxy options
		##############################

		# set xy symbol fill
		if ($cmd =~ /^[Xx][Gg]./)
			{
			($xyfill) = $cmd =~ /^[Xx][Gg](.+)/;
			}

		# set xy data to be plotted
		if ($cmd =~ /^[Xx][Ii]./)
			{
			($xyfile) = $cmd =~ /^[Xx][Ii](.+)/;
			push(@xyfiles, $xyfile);
			if (!$xysymbol)
				{
				$xysymbol = "N";
				}
			if (!$xyfill)
				{
				$xyfill = "N";
				}
			if (!$xysegment)
				{
				$xysegment = "N";
				}
			if (!$xysegchar)
				{
				$xysegchar = ">";
				}
			if (!$xypen)
				{
				$xypen = "N";
				}
			push(@xysymbols, $xysymbol);
			push(@xyfills, $xyfill);
			push(@xysegments, $xysegment);
			push(@xysegchars, $xysegchar);
			push(@xypens, $xypen);
			}

		# set xy segment
		if ($cmd =~ /^[Xx][Mm]/)
			{
			if ($cmd =~ /^[Xx][Mm]\S/)
				{
				($xysegchar) = $cmd =~ /^[Xx][Mm](\S)/
				}
			else
				{
				$xysegchar = ">";
				}
			if (!$xysegment)
				{
				$xysegment = "Y";
				}
			elsif ($xysegment ne "N")
				{
				$xysegment = "N";
				}
			else
				{
				$xysegment = "Y";
				}
			}

		# set xy symbol
		if ($cmd =~ /^[Xx][Ss]./)
			{
			($xysymbol) = $cmd =~ /^[Xx][Ss](.+)/;
			}

		# set xy pen
		if ($cmd =~ /^[Xx][Ww]./)
			{
			($xypen) = $cmd =~ /^[Xx][Ww](.+)/;
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
			|| $color_pallette > 8)
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
			|| $color_pallette > 8)
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
if ($contour_file && !$contour_mode)
	{
	$contour_mode = 1;
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

# see if data file is a grd file or a list of grdfiles
@grdinfo = `grdinfo $file_data 2>&1`;
if ($? != 0)
	{
	print "ERROR: 'grdinfo $file_data' failed.\n$program_name aborted\n";
	exit 1;
	}
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

# check for zmode == 1 in zbounds
if ($zbounds =~ /(\S+)\/(\S+)\/(\S+)/)
	{
	($zmin,$zmax,$zmode) = $zbounds =~ /(\S+)\/(\S+)\/(\S+)/;
	}
else
	{
	$zmode = 0;
	}

# get limits of files using grdinfo
if (!$bounds || !$zbounds || $zmode == 1)
	{
	if ($verbose > 0)
		{
		print "\nRunning grdinfo to get file bounds and min max...\n";
		}

	foreach $file_grd (@files_data) {

	if ($bounds)
		{
		if ($verbose > 0)
			{
			print "\tmbm_grdinfo -I$file_grd -R$bounds -V\n";
			}
		if ($verbose > 0)
			{
			@grdinfo = `mbm_grdinfo -I$file_grd -R$bounds -V`;
			}
		else
			{
			@grdinfo = `mbm_grdinfo -I$file_grd -R$bounds`;
			}
		}
	else
		{
		@grdinfo = `grdinfo $file_grd 2>&1`;
		if ($verbose > 0)
			{
			print "\tgrdinfo -I$file_grd\n";
			}
		}
	while (@grdinfo)
		{
		$line = shift @grdinfo;

		if ($verbose > 0)
			{
			print "\tgrdinfo output: $line";
			}

		if ($line =~
			/\s+Projection: UTM Zone \S+/)
			{
			($utm_zone) = $line =~
				/\s+Projection: UTM Zone (\S+)/;
			$gridprojected = 1;
			}
		if ($line =~
			/\s+Projection: SeismicProfile/)
			{
			$gridprojected = 2;
			}
		if ($line =~
			/\s+Projection: GenericLinear/)
			{
			$gridprojected = 3;
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
		$zmin_t = $zmin_f;
		$zmax_t = $zmax_f;
		}
	else
		{
		$xmin = &min($xmin, $xmin_f);
		$xmax = &max($xmax, $xmax_f);
		$ymin = &min($ymin, $ymin_f);
		$ymax = &max($ymax, $ymax_f);
		$zmin_t = &min($zmin_t, $zmin_f);
		$zmax_t = &max($zmax_t, $zmax_f);
		}

	# check that there are data
	if ($xmin_f >= $xmax_f || $ymin_f >= $ymax_f
		|| ($zmin_f >= $zmax_f && !$zbounds))
		{
		print "\a";
		print "$xmin_f $xmax_f $ymin_f $ymax_f $zmin_f $zmax_f\n";
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
		$bounds_plot = sprintf ("%1.8g/%1.8g/%1.8g/%1.8gr",
			$xmin, $ymin, $xmax, $ymax);
		}
	}

# set grid to projected if outside geographic bounds
if ($gridprojected == 0
	&& ($xmin < -360.0 || $xmax > 360.0
		|| $ymin < -90.0 || $ymax > 90.0))
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
	if ($zbounds =~ /(\S+)\/(\S+)\/(\S+)/)
		{
		($zmin,$zmax,$zmode) = $zbounds =~ /(\S+)\/(\S+)\/(\S+)/;
		}
	elsif ($zbounds =~ /(\S+)\/(\S+)/)
		{
		($zmin,$zmax) = $zbounds =~ /(\S+)\/(\S+)/;
		$zmode = 0;
		}
	}
else
	{
	$zmode = 0;
	$zmin = $zmin_t;
	$zmax = $zmax_t;
	}

# check that there are data
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
	$zmin_t = $data_scale * $zmin_t;
	$zmax_t = $data_scale * $zmax_t;
	if ($zmin > $zmax)
		{
		$tmp = $zmin;
		$zmin = $zmax;
		$zmax = $tmp;
		}
	if ($zmin_t > $zmax_t)
		{
		$tmp = $zmin_t;
		$zmin_t = $zmax_t;
		$zmax_t = $tmp;
		}
	}

# set the relevent page width and height
&GetPageSize;

# get user constraints on map scale
if ($map_scale)
	{
	# sets $plot_scale or $plot_width if possible
	&GetProjection;
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
elsif ($gridprojected > 0)
	{
	$projection = "x";
	$projection_pars = "1.0";
	$use_scale = 1;
	$linear = 1;
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
@projected = `mapproject tmp$$.dat -J$projection$projection_pars -R$bounds_plot 2>&1`;
#
if ($verbose > 0)
	{
	print "\nCalling GMT program mapproject...\n\tmapproject tmp$$.dat -J$projection$projection_pars -R$bounds_plot \n";
	}
`/bin/rm -f tmp$$.dat`;
$cnt = 0;
while (@projected)
	{
	$line = shift @projected;
	$cnt++;

	if ($verbose > 0)
		{
		print "mapproject output: $line";
		}

	if ($cnt == 1)
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
	print "GMT program mapproject failed...\n";
	print "\tMapproject invocation:\n";
	print "\t\tmapproject tmp$$.dat -J$projection$projection_pars -R$bounds_plot\n";
	print "\tInput bounds:\n";
	print "\t\txmin:$xmin xmax:$xmax ymax:$ymin ymax:$ymax\n";
	if ($cnt < 4)
		{
		print "\tmapproject seems to have failed to generate output\n";
		}
	else
		{
		print "\tOutput bounds:\n";
		print "\t\txmin:$xxmin xmax:$xxmax ymax:$yymin ymax:$yymax\n";
		}
	die "Program $program_name aborted\n";
	}

# figure out scaling issues
if (($use_scale && $plot_scale) || ($use_width && $plot_width))
	{
	$plot_width = $dxx;
	$plot_height = $dyy;

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
		$good_page = 0;
		foreach $elem (@page_size_names) {
			if (!$good_page)
				{
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
				if ($plot_width <= $width_max
					&& $plot_height <= $height_max)
					{
					$good_page = 1;
					$pagesize_save = $pagesize;
					}
				}
			}

		# print out warning
		if (!$good_page)
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

	# set plot width
	$plot_width = $dxx * $plot_scale;
	$plot_height = $dyy * $plot_scale;

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
	if ($linear && $gridprojected == 0)
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

	# construct plot scale parameters
	($projection_pars) = $map_scale =~ /^$projection(\S+)/;
	$projection_pars = sprintf ("$projection_pars$separator%1.5g", $plot_width);

	# handle special case for linear projections
	if ($linear && $gridprojected == 0)
		{
		$projection_pars = "$projection_pars" . "d";
		}
	}

# place the origin so plot is more or less centered
$xoffset = ($width - $plot_width
	- $space_left - $space_right) / 2 + $space_left;
$yoffset = ($height - $plot_height
	- $space_bottom - $space_top) / 2 + $space_bottom;

# get plot degree annotation for geographic maps
$degree_format = "ddd:mm";
if (!$gridprojected)
	{
	$xsize = $xmax - $xmin;
	$ysize = $ymax - $ymin;
	if ($xsize < $ysize)
		{
		$size = $xsize;
		}
	else
		{
		$size = $ysize;
		}
	if ($size > 4.0)
		{
		$degree_format = "ddd";
		}
	elsif ($size > (1.0 / 60.0))
		{
		$degree_format = "ddd:mm";
		}
	else
		{
		$degree_format = "ddd:mm:ss";
		}
	}

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
if ($colorscale_length < 3.0)
	{
	$colorscale_length = 3.0;
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
if ($color_mode && $zbounds)
	{
	$color_int = ($zmax - $zmin)/($ncolors_use - 1);
	$color_start = $zmin;
	$color_end = $color_start + $color_int * ($ncolors_use - 1);
	}
elsif ($color_mode && !$no_nice_color_int && $dzz > 0)
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
	# deal with single colormap (not sealevel)
	if ($color_pallette != 8)
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

	# if doing sealevel colors (different colormaps above and below sealevel
	# then generate first Haxby colormap below sealevel and second brown
	# colormap above sealevel
	else
		{
		# Find zero break
		$izero = int(-$color_start / $color_int);

		# interpolate colors
		for ($i = 0; $i < $ncolors - $izero; $i++)
			{
			$xx = ($ncpt - 1) * $i / ($ncolors - 1);
			$i1 = int($xx);
			$i2 = $i1 + 1;
			$red = $cptbr8[$i1]
				+ ($cptbr8[$i2] - $cptbr8[$i1])
				* ($xx - $i1) / ($i2 - $i1);
			$green = $cptbg8[$i1]
				+ ($cptbg8[$i2] - $cptbg8[$i1])
				* ($xx - $i1) / ($i2 - $i1);
			$blue = $cptbb8[$i1]
				+ ($cptbb8[$i2] - $cptbb8[$i1])
				* ($xx - $i1) / ($i2 - $i1);
			push (@cptr, $red);
			push (@cptg, $green);
			push (@cptb, $blue);
			}

		# interpolate colors
		for ($i = $izero; $i < $ncolors; $i++)
			{
			$xx = ($ncpt - 1) * $i / ($ncolors - 1);
			$i1 = int($xx);
			$i2 = $i1 + 1;
			$red = $cptbr1[$i1]
				+ ($cptbr1[$i2] - $cptbr1[$i1])
				* ($xx - $i1) / ($i2 - $i1);
			$green = $cptbg1[$i1]
				+ ($cptbg1[$i2] - $cptbg1[$i1])
				* ($xx - $i1) / ($i2 - $i1);
			$blue = $cptbb1[$i1]
				+ ($cptbb1[$i2] - $cptbb1[$i1])
				* ($xx - $i1) / ($i2 - $i1);
			push (@cptr, $red);
			push (@cptg, $green);
			push (@cptb, $blue);
			}
		}
	}

# get info from contour_file
if ($contour_file)
	{
	if ($bounds)
		{
		@grdinfo = `mbm_grdinfo -I$contour_file -R$bounds 2>&1`;
		}
	else
		{
		@grdinfo = `grdinfo $contour_file 2>&1`;
		}
	while (@grdinfo)
		{
		$line = shift @grdinfo;
		if ($line =~ /\S+\s+zmin:\s+(\S+)\s+zmax:\s+(\S+)\s+units:/)
			{
			($czmin,$czmax) = $line =~
				/\S+\s+zmin:\s+(\S+)\s+zmax:\s+(\S+)\s+units:/;
			}
		if ($line =~ /\S+\s+z_min:\s+(\S+)\s+z_max:\s+(\S+)\s+units:/)
			{
			($czmin,$czmax,$czunits) = $line =~
				/\S+\s+z_min:\s+(\S+)\s+z_max:\s+(\S+)\s+units:\s+(\S+)/;
			}
		}
	$cdzz = ($czmax - $czmin);
	$contour_int = 0.0;
		if ($cdzz > 0)
		{
		$cbase = int((log($cdzz) / log(10.)) + 0.5);
		$contour_int = (10 ** $cbase) / 10.0;
		if ($cdzz / $contour_int < 10)
			{
			$contour_int = $contour_int / 4;
			}
		elsif ($cdzz / $contour_int < 20)
			{
			$contour_int = $contour_int / 2;
			}
		}
	}
else
	{
	$czmin = $zmin;
	$czmax = $zmax;
	}

# set contour control
if (!$contour_control && $contour_mode)
	{
	$contour_control = $contour_int;
	}

# set pscoast control
if ($coast_control
	&& !$coast_wetfill
	&& !$coast_dryfill
	&& !$coast_pen
	&& !$coast_boundary
	&& !$coast_river)
	{
	$coast_pen = "1";
	}
if ($coast_control
	&& !$coast_resolution)
	{
	$coast_resolution = "f";
	}

# set swath navigation controls
if ($swathnavdatalist && !$navigation_control && !$navigation_mode)
	{
	$navigation_mode = 1;
	$navigation_control = "100000/100000/100000/0.15";
	}

if ($navigation_control && $navigation_control =~ /\S+\/\S+\/\S+\/\S+/)
	{
	$navigation_mode = 1;
	}
elsif ($navigation_control && ($navigation_control =~ /FP/
				|| $navigation_control =~ /fp/))
	{
	$navigation_mode = 1;
	$name_mode = 1;
	$name_perp = 1;
	$navigation_control = "0.25/1/4/0.15";
	$nav_name_hgt = "0.15";
	}
elsif ($navigation_control && ($navigation_control =~ /F/
				|| $navigation_control =~ /f/))
	{
	$navigation_mode = 1;
	$name_mode = 1;
	$name_perp = 0;
	$navigation_control = "0.25/1/4/0.15";
	$nav_name_hgt = "0.15";
	}
elsif ($navigation_control)
	{
	if ($navigation_control =~ /\S+\/\S+\/\S+\/\S+\/\S+\/\S+/)
		{
		$navigation_mode = 1;
		$name_mode = 1;
		($nav_time_tick, $nav_time_annot,
				$nav_date_annot, $nav_tick_size, $nav_name_hgt, $name_perp)
				= $navigation_control =~  /(\S+)\/(\S+)\/(\S+)\/(\S+)\/(\S+)/;
		}
	elsif ($navigation_control =~ /\S+\/\S+\/\S+\/\S+\/\S+/)
		{
		$navigation_mode = 1;
		$name_mode = 1;
		$name_perp = 0;
		($nav_time_tick, $nav_time_annot,
				$nav_date_annot, $nav_tick_size, $nav_name_hgt)
				= $navigation_control =~  /(\S+)\/(\S+)\/(\S+)\/(\S+)\/(\S+)/;
		}
	elsif ($navigation_control =~ /\S+\/\S+\/\S+/)
		{
		($nav_time_tick, $nav_time_annot,
			$nav_date_annot)
			= $navigation_control =~  /(\S+)\/(\S+)\/(\S+)/;
		$nav_tick_size = 0.15;
		}
	elsif ($navigation_control =~ /\S+\/\S+/)
		{
		($nav_time_tick, $nav_time_annot)
			= $navigation_control =~  /(\S+)\/(\S+)/;
		$nav_date_annot = 100000;
		$nav_tick_size = 0.15;
		}
	else
		{
		$nav_time_tick = $navigation_control;
		$nav_time_annot = 100000;
		$nav_date_annot = 100000;
		$nav_tick_size = 0.15;
		}
	$navigation_mode = 1;
	$navigation_control = "$nav_time_tick"
			. "/" . "$nav_time_annot"
			. "/" . "$nav_date_annot"
			. "/" . "$nav_tick_size";
	}
elsif ($navigation_mode)
	{
	$navigation_control = "0.25/1/4/0.15";
	}
if ($pingnumber_mode)
	{
	$pingnumber_control = "$pingnumber_tick"
			. "/" . "$pingnumber_annot"
			. "/" . "$pingnumber_tick_len";
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
$gmtfile = "gmtdefaults4\$\$";

# set some gmtisms
$first_gmt = 1;
$first = "-X\$X_OFFSET -Y\$Y_OFFSET -K -V >! \$PS_FILE";
$middle = "-K -O -V >> \$PS_FILE";
$end = "-O -V >> \$PS_FILE";

# set macro gmt default settings
$gmt_def = "MEASURE_UNIT/inch";
push(@gmt_macro_defs, $gmt_def);
# If + is added then the media is EPS
$gmt_def = "PAPER_MEDIA/$page_gmt_name{$pagesize}+";
push(@gmt_macro_defs, $gmt_def);
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
$gmt_def = "PLOT_DEGREE_FORMAT/$degree_format";
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
print FCMD "set MAP_REGION      = $bounds_plot\n";
printf FCMD "set X_OFFSET        = %1.5g\n", $xoffset;
printf FCMD "set Y_OFFSET        = %1.5g\n", $yoffset;

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
	elsif ($stretch_color)
		{
		if ($verbose)
			{
			print "Running grdhisteq...\n";
			}
		$ncolors_minus = $ncolors - 1;
		@grdhisteq = `grdhisteq $files_data[0] -C$ncolors_minus -D`;
		foreach $d (@grdhisteq) {
			($d1, $d2) = $d =~ /(\S+)\s+(\S+).*/;
			if ($d2 > $d1)
				{
				push(@hist, $d1);
				}
			}
		if ($d2 > $d1)
			{
			push(@hist, $d2);
			}

		# reset number of colors if grdhisteq returned fewer intervals
		if (scalar(@hist) < $ncolors)
			{
			$ncolors = scalar(@hist);
			}

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
		if ($zmax > $hist[$ncolors-1])
			{
			$hist[$ncolors-1] = $zmax;
			}
		$hist[0] = $hist[0] - 0.01*$dzz;
		$hist[$ncolors-1] =
			$hist[$ncolors-1] + 0.01*$dzz;
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
			if ($zmode == 1)
				{
				if ($i == 0)
					{
					if ($color_mode == 4 && $d1 > 0.0)
						{
						$d1 = 0.0;
						}
					elsif ($color_mode != 4 && $zmin_t < $d1)
						{
						$d1 = $zmin_t;
						}
					}
				if ($i == $ncolors - 2 && $zmax_t > $d2)
					{
					$d2 = $zmax_t;
					}
				}
			printf FCMD "echo %6g %3d %3d %3d %6g %3d %3d %3d",
				$d1,@cptr[$i],@cptg[$i],@cptb[$i],
				$d2,@cptr[$i+1],@cptg[$i+1],@cptb[$i+1];
			if ($zmode == 1 && $i == 0)
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
			if ($zmode == 1)
				{
				if ($i == $ncolors - 2)
					{
					if ($color_mode == 4 && $d1 > 0.0)
						{
						$d1 = 0.0;
						}
					elsif ($color_mode != 4 && $zmin_t < $d1)
						{
						$d1 = $zmin_t;
						}
					}
				if ($i == 0 && $zmax_t > $d2)
					{
					$d2 = $zmax_t;
					}
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
			if ($zmode == 1)
				{
				if ($i == 0)
					{
					if ($color_mode == 4 && $d1 > 0.0)
						{
						$d1 = 0.0;
						}
					elsif ($color_mode != 4 && $zmin_t < $d1)
						{
						$d1 = $zmin_t;
						}
					}
				if ($i == $ncolors - 1 && $zmax_t > $d2)
					{
					$d2 = $zmax_t;
					}
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
			if ($zmode == 1)
				{
				if ($i == $ncolors - 1)
					{
					if ($color_mode == 4 && $d1 > 0.0)
						{
						$d1 = 0.0;
						}
					elsif ($color_mode != 4 && $zmin_t < $d1)
						{
						$d1 = $zmin_t;
						}
					}
				if ($i == 0 && $zmax_t > $d2)
					{
					$d2 = $zmax_t;
					}
				}
			printf FCMD "echo %6g %3d %3d %3d %6g %3d %3d %3d",
				$d1,@cptr[$i],@cptg[$i],@cptb[$i],
				$d2,@cptr[$i],@cptg[$i],@cptb[$i];
			if ($i == ($ncolors - 1))
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
#	if ($gridprojected == 0)
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
	if ($gridprojected == 0)
		{
		printf FCMD " -M";
		}
	printf FCMD "\n";
	printf FCMD "echo Running grdgradient to get y component of the gradient...\n";
	printf FCMD "grdgradient $file_use -A0 -G\$DATA_FILE.drvy";
	if ($gridprojected == 0)
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
elsif ($color_mode == 3 && $file_int && $stretch_shade)
	{
	printf FCMD "#\n# Get shading array\n";
	printf FCMD "echo Getting shading array...\n";
	printf FCMD "echo Running grdhisteq...\n";
	printf FCMD "grdhisteq \$INTENSITY_FILE -G\$INTENSITY_FILE.eq -N\n";
	printf FCMD "echo Running grdmath...\n";
	printf FCMD "grdmath \$INTENSITY_FILE.eq $magnitude MUL = \$INTENSITY_FILE.int\n";
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
	if ($gridprojected == 0)
		{
		printf FCMD " -M";
		}
	printf FCMD "\n";
	printf FCMD "echo Running grdgradient to get y component of the gradient...\n";
	printf FCMD "grdgradient $file_use -A0 -G\$DATA_FILE.drvy";
	if ($gridprojected == 0)
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

# do grdimage plot
if ($color_mode)
	{
	printf FCMD "#\n# Make color image\n";
	printf FCMD "echo Running grdimage...\n";
	if ($color_mode == 4)
		{
		printf FCMD "grdimage $file_slope -J\$MAP_PROJECTION\$MAP_SCALE \\\n\t";
		}
	else
		{
		printf FCMD "grdimage $file_use -J\$MAP_PROJECTION\$MAP_SCALE \\\n\t";
		}
	printf FCMD "-R\$MAP_REGION -C\$CPT_FILE \\\n\t";
	if ($color_mode == 2 || $color_mode == 3)
		{
		printf FCMD "-I$file_shade \\\n\t";
		}
	elsif ($color_mode == 5)
		{
		printf FCMD "-I$file_slope \\\n\t";
		}
	if ($dpi)
		{
		printf FCMD "-E$dpi \\\n\t";
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

# do grdcontour plot
if ($contour_mode)
	{
	if (!$contour_file)
		{
		$contour_file = $file_use
		}
	printf FCMD "#\n# Make contour plot\n";
	printf FCMD "echo Running grdcontour...\n";
	printf FCMD "grdcontour $contour_file -J\$MAP_PROJECTION\$MAP_SCALE \\\n\t";
	printf FCMD "-R\$MAP_REGION \\\n\t";
	printf FCMD "-C$contour_control \\\n\t";
	if (!$contour_pen)
		{
		$contour_pen = "c1p";
		}
	printf FCMD "-L$czmin/$czmax -W$contour_pen\\\n\t";
	if ($contour_anot_int)
		{
		printf FCMD "-A$contour_anot_int \\\n\t";
		}
	if ($contour_gap)
		{
		printf FCMD "-G$contour_gap \\\n\t";
		}
	if ($contour_cut)
		{
		printf FCMD "-Q$contour_cut \\\n\t";
		}
	if ($contour_tick_on && $contour_tick)
		{
		printf FCMD "-T$contour_tick \\\n\t";
		}
	elsif ($contour_tick_on)
		{
		printf FCMD "-T \\\n\t";
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
} # end loop over grd data files

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
	$tlabel = "Data File $file_data";
	}
if ($nlabels < 2 && $zunits_s)
	{
	$slabel = "$zunits_s";
	}
elsif ($nlabels < 2)
	{
	$slabel = "Data Values";
	}

# set basemap axes annotation
if ($gridprojected == 2 && $tick_info && $tick_info =~ /\S+\/\S+/)
	{
	($base_tick_x, $base_tick_y) = $tick_info =~ /(\S+)\/(\S+)/;
	$base_tick_x = "$base_tick_x" . "\":$xunits:\"";
	$base_tick_y = "$base_tick_y" . "\":$yunits:\"";
	$axes = "$base_tick_x/$base_tick_y:.\"$tlabel\":WESN";
	}
elsif ($tick_info)
	{
	$axes = $tick_info;
	if (!($tick_info =~ /.*:\..*/))
		{
		$axes = "$axes:.\"$tlabel\":";
		}
	}
else
	{
	# figure out some reasonable tick intervals for the basemap
	&GetBaseTick;
	$axes = "$base_tick_x/$base_tick_y:.\"$tlabel\":";
	}

# do coastline plots
if ($coast_control)
	{
	printf FCMD "#\n# Make coastline data plot\n";
	printf FCMD "echo Running pscoast...\n";
	printf FCMD "pscoast \\\n\t";
	printf FCMD "-J\$MAP_PROJECTION\$MAP_SCALE \\\n\t";
	printf FCMD "-R\$MAP_REGION \\\n\t";
	if ($coast_lakefill)
		{
		printf FCMD "-C$coast_lakefill \\\n\t";
		}
	if ($coast_resolution)
		{
		printf FCMD "-D$coast_resolution \\\n\t";
		}
	if ($coast_dryfill)
		{
		printf FCMD "-G$coast_dryfill \\\n\t";
		}
	if ($coast_river)
		{
		printf FCMD "-I$coast_river \\\n\t";
		}
	for ($i = 0; $i < scalar(@coast_boundaries); $i++)
		{
		printf FCMD "-N$coast_boundaries[$i] \\\n\t";
		}
	if ($coast_wetfill)
		{
		printf FCMD "-S$coast_wetfill \\\n\t";
		}
	if ($coast_pen)
		{
		printf FCMD "-W$coast_pen \\\n\t";
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

# do xy plots
for ($i = 0; $i < scalar(@xyfiles); $i++)
	{
	printf FCMD "#\n# Make xy data plot\n";
	printf FCMD "echo Running psxy...\n";
	printf FCMD "psxy $xyfiles[$i] \\\n\t";
	printf FCMD "-J\$MAP_PROJECTION\$MAP_SCALE \\\n\t";
	printf FCMD "-R\$MAP_REGION \\\n\t";
	if ($xyfills[$i] ne "N")
		{
		printf FCMD "-G$xyfills[$i] \\\n\t";
		}
	if ($xysymbols[$i] ne "N")
		{
		printf FCMD "-S$xysymbols[$i] \\\n\t";
		}
	if ($xysegments[$i] ne "N")
		{
		if ($xysegchars[$i] eq "#")
			{
			printf FCMD "-M\\# \\\n\t";
			}
		else
			{
			printf FCMD "-M\'$xysegchars[$i]\' \\\n\t";
			}
		}
	if ($xypens[$i] ne "N")
		{
		printf FCMD "-W$xypens[$i] \\\n\t";
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

# do swath nav plots
if ($swathnavdatalist)
	{
	if (!$swathformat)
		{
		$swathformatline = `mbformat -I $swathnavdatalist -L`;
		($swathformat) = $swathformatline =~ /(\S+)/;
		if ($swathformat == 0)
			{
			$swathformat = -1;
			}
		}

	printf FCMD "#\n# Make swath nav plot\n";
	printf FCMD "echo Running mbcontour...\n";
	printf FCMD "mbcontour -F$swathformat -I $swathnavdatalist \\\n\t";
	printf FCMD "-J\$MAP_PROJECTION\$MAP_SCALE \\\n\t";
	printf FCMD "-R\$MAP_REGION \\\n\t";
	if ($navigation_mode)
		{
		printf FCMD "-D$navigation_control \\\n\t";
		}
	if ($name_mode && $name_perp != 0)
		{
		printf FCMD "-G$nav_name_hgt/1 \\\n\t";
		}
	elsif ($name_mode)
		{
		printf FCMD "-G$nav_name_hgt \\\n\t";
		}
	if ($pingnumber_mode)
		{
		printf FCMD "-M$pingnumber_control \\\n\t";
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
if ($color_mode && ($color_pallette < 5 || $color_pallette == 8) && $gridprojected != 2)
	{
	printf FCMD "#\n# Make color scale\n";
	printf FCMD "echo Running psscale...\n";
	printf FCMD "psscale -C\$CPT_FILE \\\n\t";
	printf FCMD "-D%.4f/%.4f/%.4f/%.4f%s \\\n\t",
		$colorscale_offx,$colorscale_offy,
		$colorscale_length,$colorscale_thick,
		$colorscale_vh;
	print FCMD "-B\":$slabel:\" \\\n\t";
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
	printf FCMD "pstext -J\$MAP_PROJECTION\$MAP_SCALE \\\n\t";
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
printf FCMD "psbasemap -J\$MAP_PROJECTION\$MAP_SCALE \\\n\t";
printf FCMD "-R\$MAP_REGION \\\n\t";
printf FCMD "-B$axes \\\n\t";
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
print FCMD "/bin/mv $gmtfile .gmtdefaults4\n";

# display image on screen if desired
print FCMD "#\n# Run $ps_viewer\n";
if ($ps_viewer eq "xpsview")
	{
	if ($portrait)
		{
		$view_pageflag = "-ps $pagesize -maxp $xpsview_mem{$pagesize}";
		}
	elsif ($landscape)
		{
		$view_pageflag = "-ps $pagesize -or landscape -maxp $xpsview_mem{$pagesize}";
		}
	}
elsif ($ps_viewer eq "pageview")
	{
	if ($portrait)
		{
		$view_pageflag = "-w $page_width_in{$pagesize} -h $page_height_in{$pagesize}";
		}
	elsif ($landscape)
		{
		$view_pageflag = "-w $page_height_in{$pagesize} -h $page_width_in{$pagesize}";
		}
	}
elsif ($ps_viewer eq "ghostview")
	{
	if ($portrait)
		{
		$view_pageflag = "-portrait -media BBox";
		}
	elsif ($landscape)
		{
		$view_pageflag = "-landscape -media BBox";
		}
	}
elsif ($ps_viewer eq "gv")
	{
	if ($portrait)
		{
		$pagescale = 11.0 / $page_height_in{$pagesize};
		if ($pagescale > 1.0)
			{
			$pagescale = 1.0;
			}
		$view_pageflag = "--orientation=portrait --media=BBox -scale=$pagescale -page=$psfile";
		}
	elsif ($landscape)
		{
		$pagescale = 11.0 / $page_width_in{$pagesize};
		if ($pagescale > 1.0)
			{
			$pagescale = 1.0;
			}
		$view_pageflag = "--orientation=landscape --media=BBox -scale=$pagescale -page=$psfile";
		}
	}
elsif ($ps_viewer eq "ggv")
	{
	if ($portrait)
		{
		$view_pageflag = "--geometry=portrait";
		}
	elsif ($landscape)
		{
		$view_pageflag = "--geometry=landscape";
		}
	}
if ($no_view_ps)
	{
	print FCMD "#echo Running $ps_viewer in background...\n";
	print FCMD "#$ps_viewer $view_pageflag $psfile &\n";
	}
else
	{
	print FCMD "echo Running $ps_viewer in background...\n";
	print FCMD "$ps_viewer $view_pageflag $psfile &\n";
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
	if ($contour_mode)
		{
		print "    Contours\n";
		}
	if ($coast_control)
		{
		print "    Coastline\n";
		}
	if (@xyfiles)
		{
		print "    XY Plots of ", scalar(@xyfiles), " Datasets\n";
		}
	if ($color_mode && ($color_pallette < 5 || $color_pallette == 8))
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
	if ($swathnavdatalist)
		{
		print "    Swath Nav Datalist:        $swathnavdatalist\n";
		}
	foreach $xyfile (@xyfiles) {
		print "    XY Data File:             $xyfile\n";
	}
	print "\n  Output Files:\n";
	print "    Output plot name root:    $root\n";
	print "    Color pallette table:     $cptfile\n";
	print "    Plotting shellscript:     $cmdfile\n";
	print "    Plot file:                $psfile\n";
	print "\n  Plot Attributes:\n";
	printf "    Plot width:               %.4f\n", $plot_width;
	printf "    Plot height:              %.4f\n", $plot_height;
	print "    Page size:                $pagesize\n";
	print "    Page width:               $width\n";
	print "    Page height:              $height\n";
	print "    Projection:               -J$projection$projection_pars\n";
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
	if ($coast_control)
		{
		print "\n  Coastline Plotting Controls:\n";
		}
	if ($coast_control && $coast_resolution)
		{
		printf "    Coastline resolution:     $coast_resolution\n";
		}
	if ($coast_control && $coast_pen)
		{
		printf "    Coastline pen:            $coast_pen\n";
		}
	if ($coast_control && $coast_wetfill)
		{
		printf "    Ocean fill:               $coast_wetfill\n";
		}
	if ($coast_control && $coast_lakefill)
		{
		printf "    Lake fill:                $coast_lakefill\n";
		}
	if ($coast_control && $coast_dryfill)
		{
		printf "    Land fill:                $coast_dryfill\n";
		}
	if ($coast_control && $coast_river)
		{
		printf "    Rivers:                   $coast_river\n";
		}

	if ($coast_control && $coast_boundaries)
		{
		for ($i = 0; $i < scalar(@coast_boundaries); $i++)
			{
			printf "    National Boundaries:      $coast_boundaries[$i]\n";
			}
		}
	if ($swathnavdatalist)
		{
		print "\n  Swath Navigation Plotting Controls:\n";
		if ($navigation_mode)
			{
			print "    Navigation control:       $navigation_control\n";
			}
		if ($pingnumber_mode)
			{
			print "    Ping annotation control:  $pingnumber_control\n";
			}
		if ($name_mode)
			{
			print "    Name annotation height:   $nav_name_hgt\n";
			if ($name_perp)
				{
				print "    Name annotation style:    Perpendicular\n";
				}
			else
				{
				print "    Name annotation style:    Parallel\n";
				}
			}
		}
	if (@xyfiles)
		{
		print "\n  Primary XY Plotting Controls:\n";
		printf "    symbol     pen        fill      segment      file\n";
		printf "    ------     ---        ----      -------      ----\n";
		for ($i = 0; $i < scalar(@xyfiles); $i++)
			{
			printf "    %-10s %-10s %-10s %s%-9s %s\n",
				$xysymbols[$i], $xypens[$i],
				$xyfills[$i], $xysegments[$i], $xysegchars[$i],
				$xyfiles[$i];
			}
		}
	if ($length_scale || $contour_anot_int
		|| $contour_anot_int || $contour_cut
		|| $contour_gap || $contour_tick
		|| $contour_pen)
		{
		print "\n  Miscellaneous Plotting Controls:\n";
		}
	if ($length_scale)
		{
		printf "    Length scale:             $length_scale\n";
		}
	if ($contour_anot_int)
		{
		printf "    Contour annotation:       $contour_anot_int\n";
		}
	if ($contour_anot_int)
		{
		printf "    Contour Annotation:       $contour_anot_int\n";
		}
	if ($contour_cut)
		{
		printf "    Contour cut threshold:    $contour_cut\n";
		}
	if ($contour_gap)
		{
		printf "    Contour gap control:      $contour_gap\n";
		}
	if ($contour_tick)
		{
		printf "    Contour tick control:     $contour_tick\n";
		}
	if ($contour_pen)
		{
		printf "    Contour pen attributes:   $contour_pen\n";
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
	if ($space_top > 4.50)
		{
		$space_top = 4.50;
		}
	if ($space_bottom > 2.25)
		{
		$space_bottom = 2.25;
		}
	if ($space_left > 7.50)
		{
		$space_left = 7.50;
		}
	if ($space_right > 3.00)
		{
		$space_right = 3.00;
		}
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
	if ($space_top > 4.50)
		{
		$space_top = 4.50;
		}
	if ($space_bottom > 2.25)
		{
		$space_bottom = 2.25;
		}
	if ($space_left > 3.00)
		{
		$space_left = 3.00;
		}
	if ($space_right > 7.50)
		{
		$space_right = 7.50;
		}
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
	if ($space_top > 8.25)
		{
		$space_top = 8.25;
		}
	if ($space_bottom > 2.25)
		{
		$space_bottom = 2.25;
		}
	if ($space_left > 3.00)
		{
		$space_left = 3.00;
		}
	if ($space_right > 3.00)
		{
		$space_right = 3.00;
		}
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
	if ($space_top > 4.50)
		{
		$space_top = 4.50;
		}
	if ($space_bottom > 6.00)
		{
		$space_bottom = 6.00;
		}
	if ($space_left > 3.00)
		{
		$space_left = 3.00;
		}
	if ($space_right > 3.00)
		{
		$space_right = 3.00;
		}
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
			$linear = 1;
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
			$linear = 1;
			($plot_width) = $map_scale =~ /^X(\S+)$/;
			}
		$use_width = 1;
		$separator = "";
		}
}
#-----------------------------------------------------------------------
sub GetBaseTick {

	# figure out some reasonable tick intervals for the basemap
	if ($gridprojected == 2)
		{
		if ($xmax - $xmin > 200)
			{
			$base_tick_x = 200;
			}
		else
			{
			$base_tick_x = $xmax - $xmin;
			}
		$base_tick_y = 10.0**(floor(log(abs($ymax - $ymin) / 5) / log (10)));
		}
	else
		{
		$base_tick_x = ($xmax - $xmin) / 5;
		$base_tick_y = ($ymax - $ymin) / 5;
		}

	# deal with seismic grids (time vs trace number)
	if ($gridprojected == 2)
		{
		$base_gridline_y = $base_tick_y / 2;
		$base_tick_x = "$base_tick_x" . "\":Trace Number:\"";
		$base_tick_y = "a$base_tick_y" . "g$base_gridline_y" . "\":Time (sec):\"";
		}

	# deal with generic linear grids
	elsif ($gridprojected == 3)
		{
		$base_tick_x = "$base_tick_x" . "\":$xunits:\"";
		$base_tick_y = "$base_tick_y" . "\":$yunits:\"";
		}

	# deal with linear plot of projected grid
	elsif ($gridprojected == 1)
		{
		$base_tick = &min($base_tick_x, $base_tick_y);
		$base_tick_x = $base_tick;
		$base_tick_y = $base_tick;
		}

	# deal with geographic grid
	elsif ($gridprojected == 0)
		{
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
		$base_tick_x = $base_tick;
		$base_tick_y = $base_tick;
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
