eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
                    & eval 'exec perl -S $0 $argv:q'
                         if 0;
#--------------------------------------------------------------------
#    The MB-system:	mbm_plot.perl	6/18/93
#    $Id: mbm_plot.perl,v 5.5 2001-11-02 21:07:40 caress Exp $
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
#   mbm_plot
#
# Purpose:
#   Macro to generate a shellscript of MB-System and GMT commands 
#   which, when executed, will generate a Postscript plot of the 
#   specified swath sonar data. The plot may include bathymetry color 
#   fill (-G1), bathymetry color shaded relief (-G2), bathymetry 
#   shaded with amplitudes (-G3), greyshade fill amplitude (-G4), 
#   greyshade fill sidescan (-G5), contoured bathymetry (-C),
#   or annotated navigation. The plot may also include text
#   labels and xy data in lines or symbols. Five different color 
#   schemes are included. The plot will be scaled to fit on 
#   the specified page size or, if the scale is user defined, 
#   the page size will be chosen in accordance with the plot 
#   size. The primary purpose of this macro is to allow the 
#   simple, semi-automated production of nice looking maps with 
#   a few command line arguments. For users seeking more control 
#   over the plot appearance, a number of additional optional 
#   arguments are provided. Truly ambitious users may edit the 
#   plot shellscript to take advantage of MB-System and GMT 
#   capabilites not supported by this macro.
#
# Basic Usage: 
#   mbm_plot -Fformat -Ifile [-Amagnitude[/azimuth | zero_level] 
#            -C[cont_int/col_int/tic_int/lab_int/tic_len/lab_hgt] 
#            -Gcolor_mode -H 
#            -N[time_tick/time_annot/date_annot/time_tick_len]
#            -Oroot -Ppagesize -S[color/shade] -T -Uorientation -V 
#            -Wcolor_style[/pallette] ]
#
# Additional Options:
#            [-Btickinfo -Dflipcolor/flipshade 
#            -Jprojection[/scale | width] -Ltitle[:scale_label] 
#            -Mmisc -Q -Rw/e/s/n -X -Y -Zmin/max\]n
#
# Miscellaneous Options:
#            [-MGDgmtdef/value -MGFscale_loc 
#            -MGL[f][x]lon0/lat0/slat/length[m]
#            -MGTx/y/size/angle/font/just/text
#            -MGQdpi -MGU[/dx/dy/][label]
#            -MMAfactor/mode/depth -MMByr/mo/da/hr/mn/sc 
#            -MMDmode/scale[/min/max] -MMEyr/mo/da/hr/mn/sc 
#            -MMNnplot -MMPpings -MMSspeedmin 
#            -MMTtimegap -MMZalgorithm 
#            -MTCfill -MTDresolution -MTGfill -MTIriver[/pen] 
#            -MTNborder[/pen] -MTSfill -MTWpen
#            -MXGfill -MXIxy_file -MXSsymbol/size -MXWpen]
#
# Author:
#   David W. Caress
#   Lamont-Doherty Earth Observatory
#   Palisades, NY  10964
#   June 17, 1993
#
# Version:
#   $Id: mbm_plot.perl,v 5.5 2001-11-02 21:07:40 caress Exp $
#
# Revisions:
#   $Log: not supported by cvs2svn $
# Revision 5.4  2001/10/29  20:07:39  caress
# Now checks that the number of records is > 0 before
# using the min max data values in mbinfo output.
#
# Revision 5.3  2001/10/11  01:32:11  caress
# Replaced use of perl function MBparsedatalist
# with call to new program mbdatalist.
#
# Revision 5.2  2001/10/10  23:56:01  dcaress
# Regrettably, I don't remember what I changed...
#
#   Revision 5.1  2001-03-22 13:05:45-08  caress
#   Trying to make release 5.0.beta0
#
# Revision 5.0  2000/12/01  22:58:01  caress
# First cut at Version 5.0.
#
# Revision 4.28  2000/10/03  21:42:17  caress
# Snapshot for Dale.
#
# Revision 4.27  2000/09/11  21:54:49  caress
# Added support for recursive and commented datalists.
#
# Revision 4.26  2000/01/13  22:48:11  caress
# Fixed usage of .inf files
#
# Revision 4.25  1999/12/29  00:17:55  caress
# Release 4.6.8
#
# Revision 4.24  1999/08/08  04:17:04  caress
# Added coastline plots.
#
# Revision 4.23  1999/06/25  17:55:47  caress
# I must have changed something!
#
# Revision 4.22  1999/05/06  23:46:32  caress
# Release 4.6a
#
# Revision 4.21  1999/04/16  01:25:51  caress
# Version 4.6 final release?
#
# Revision 4.20  1999/04/15  19:28:52  caress
# Fixed sprintf statements.
#
# Revision 4.19  1999/03/31  18:09:36  caress
# MB-System 4.6beta7
#
# Revision 4.18  1999/02/04  23:39:54  caress
# MB-System version 4.6beta7
#
# Revision 4.17  1998/10/13  18:08:37  caress
# Fixed typos.
#
# Revision 4.16  1998/10/05  17:00:15  caress
# MB-System version 4.6beta
#
# Revision 4.15  1997/04/21  16:54:41  caress
# MB-System 4.5 Beta Release.
#
# Revision 4.16  1997/04/17  15:06:49  caress
# MB-System 4.5 Beta Release
#
# Revision 4.15  1997/03/26  15:37:14  caress
# Comment changes.
#
# Revision 4.14  1997/01/28  18:34:03  caress
# Added printout of actual data lon lat bounds in addition
# to stretched bounds for plot.
#
# Revision 4.13  1996/03/12  17:28:19  caress
# Check-in after flail with format 63.
#
# Revision 4.12  1995/11/22  22:46:40  caress
# Check in during general flail.
#
# Revision 4.11  1995/09/28  18:05:43  caress
# Various bug fixes working toward release 4.3.
#
# Revision 4.10  1995/08/17  14:52:53  caress
# Revision for release 4.3.
#
# Revision 4.9  1995/07/18  17:24:57  caress
# Now uses -G option of mbinfo.
#
# Revision 4.8  1995/05/12  17:43:23  caress
# Made exit status values consistent with Unix convention.
# 0: ok  nonzero: error
#
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
&MBGetopts('A:a:B:b:C%c%D%d%F:f:G%g%HhI:i:J:j:L:l:M+m+N%n%O:o:P:p:QqR:r:S%s%TtU:u:VvW:w:XxYyZ:z:');
$shade_control = 	($opt_A || $opt_a);
$tick_info = 		($opt_B || $opt_b);
$contour_mode = 	($flg_C || $flg_c);
$contour_control = 	($opt_C || $opt_c);
$color_flip_mode = 	($flg_D || $flg_d);
$color_flip_control = 	($opt_D || $opt_d);
$format = 		($opt_F || $opt_f);
$color_mode =   	($opt_G || $opt_g || $flg_G || $flg_g);
$help =    		($opt_H || $opt_h);
$file_data =    	($opt_I || $opt_i);
$map_scale =    	($opt_J || $opt_j);
$labels =    		($opt_L || $opt_l);
$misc = 		($opt_M || $opt_m);
$navigation_mode = 	($flg_N || $flg_n);
$navigation_control = 	($opt_N || $opt_n);
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
	print "\nMacro to generate a shellscript of MB-System and GMT commands \n";
	print "which, when executed, will generate a Postscript plot of the \n";
	print "specified swath sonar data. The plot may include bathymetry color \n";
	print "fill (-G1), bathymetry color shaded relief (-G2), bathymetry \n";
	print "shaded with amplitudes (-G3), greyshade fill amplitude (-G4), \n";
	print "greyshade fill sidescan (-G5), contoured bathymetry (-C),\n";
	print "or annotated navigation. The plot may also include text\n";
	print "labels and xy data in lines or symbols. Five different color \n";
	print "schemes are included. The plot will be scaled to fit on \n";
	print "the specified page size or, if the scale is user defined, \n";
	print "the page size will be chosen in accordance with the plot \n";
	print "size. The primary purpose of this macro is to allow the \n";
	print "simple, semi-automated production of nice looking maps with \n";
	print "a few command line arguments. For users seeking more control \n";
	print "over the plot appearance, a number of additional optional \n";
	print "arguments are provided. Truly ambitious users may edit the \n";
	print "plot shellscript to take advantage of MB-System and GMT \n";
	print "capabilites not supported by this macro.\n";
	print "\nBasic Usage: \n";
	print "\t$program_name -Fformat -Ifile [-Amagnitude[/azimuth | zero_level]\n";
	print "\t\t-C[cont_int/col_int/tic_int/lab_int/tic_len/lab_hgt]\n";
	print "\t\t-Gcolor_mode -H\n";
	print "\t\t-N[time_tick/time_annot/date_annot/time_tick_len]\n";
	print "\t\t-Oroot -Ppagesize -S[color/shade] -T -Uorientation -V \n";
	print "\t\t-Wcolor_style[/pallette] ]\n";
	print "Additional Options:\n";
	print "\t\t[-Btickinfo -Dflipcolor/flipshade\n";
	print "\t\t-Jprojection[/scale | width] -Ltitle[:scale_label] \n";
	print "\t\t-Mmisc -Q -Rw/e/s/n -X -Y -Zmin/max\]n";
	print "Miscellaneous Options:\n";
	print "\t\t[-MGDgmtdef/value -MGFscale_loc\n";
	print "\t\t-MGL[f][x]lon0/lat0/slat/length[m]\n";
	print "\t\t-MGTx/y/size/angle/font/just/text\n";
	print "\t\t-MGQdpi -MGU[/dx/dy/][label]\n";
	print "\t\t-MMAfactor/mode/depth -MMByr/mo/da/hr/mn/sc\n";
	print "\t\t-MMDmode/scale[/min/max] -MMEyr/mo/da/hr/mn/sc\n";
	print "\t\t-MMNnplot -MMPpings -MMSspeedmin\n";
	print "\t\t-MMTtimegap -MMZalgorithm\n";
	print "\t\t-MTCfill -MTDresolution -MTGfill -MTIriver[/pen]\n";
	print "\t\t-MTNborder[/pen] -MTSfill -MTWpen\n";
	print "\t\t-MXGfill -MXIxy_file -MXM -MXSsymbol/size -MXWpen]\n";
	exit 0;
	}

# check for input file
if (!$file_data)
	{
	print "\a";
	die "No input file specified - $program_name aborted\n";
	}
elsif (! -e $file_data)
	{
	print "\a";
	die "Specified input file cannot be opened - $program_name aborted\n";
	}

# tell the world we got started
if ($verbose) 
	{
	print "\nRunning $program_name...\n";
	}

# set ping averaging
$mb_pings = 1;

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
		if ($cmd =~ /^[Gg][Qq]./)
			{
			($dpi) = $cmd =~ /^[Gg][Qq](.+)/;
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

		# deal with MB-System options
		##############################

		# set beam footprint parameters
		if ($cmd =~ /^[Mm][Aa]./)
			{
			($swath_footprint) = $cmd =~ /^[Mm][Aa](.+)/;
			}

		# set begin time for acceptable swath sonar data
		if ($cmd =~ /^[Mm][Bb]./)
			{
			($mb_btime) = $cmd =~ /^[Mm][Bb](.+)/;
			}

		# set swath sonar data scaling parameters
		if ($cmd =~ /^[Mm][Dd]./)
			{
			($swath_scale) = $cmd =~ /^[Mm][Dd](.+)/;
			}

		# set end time for acceptable swath sonar data
		if ($cmd =~ /^[Mm][Ee]./)
			{
			($mb_etime) = $cmd =~ /^[Mm][Ee](.+)/;
			}

		# set number of pings to be contoured at a time
		if ($cmd =~ /^[Mm][Nn]./)
			{
			($contour_nplot) = $cmd =~ /^[Mm][Nn](.+)/;
			}

		# set ping averaging
		if ($cmd =~ /^[Mm][Pp]./)
			{
			($mb_pings) = $cmd =~ /^[Mm][Pp](.+)/;
			}

		# set minimum ship speed
		if ($cmd =~ /^[Mm][Ss]./)
			{
			($mb_speedmin) = $cmd =~ /^[Mm][Ss](.+)/;
			}

		# set time gap threshold
		if ($cmd =~ /^[Mm][Tt]./)
			{
			($mb_timegap) = $cmd =~ /^[Mm][Tt](.+)/;
			}

		# set contour algorithm
		if ($cmd =~ /^[Mm][Zz]./)
			{
			($contour_algorithm) = $cmd =~ /^[Mm][Zz](.+)/;
			}

		# deal with pscoast options
		##############################

		# set pscoast lake fill
		if ($cmd =~ /^[Tt][Cc]./)
			{
			($coast_lakefill) = $cmd =~ /^[Tt][Cc](.+)/;
			}

		# set pscoast resolution
		if ($cmd =~ /^[Tt][Dd]./)
			{
			($coast_resolution) = $cmd =~ /^[Tt][Dd](.+)/;
			}

		# set pscoast dry fill
		if ($cmd =~ /^[Tt][Gg]./)
			{
			($coast_dryfill) = $cmd =~ /^[Tt][Gg](.+)/;
			}

		# set pscoast rivers
		if ($cmd =~ /^[Tt][Ii]./)
			{
			($coast_river) = $cmd =~ /^[Tt][Ii](.+)/;
			}

		# set pscoast national boundaries
		if ($cmd =~ /^[Tt][Nn]./)
			{
			($coast_boundary) = $cmd =~ /^[Tt][Nn](.+)/;
			push(@coast_boundaries, $coast_boundary);
			}

		# set pscoast wet fill
		if ($cmd =~ /^[Tt][Ss]./)
			{
			($coast_wetfill) = $cmd =~ /^[Tt][Ss](.+)/;
			}

		# set pscoast coastline pen
		if ($cmd =~ /^[Tt][Ww]./)
			{
			($coast_pen) = $cmd =~ /^[Tt][Ww](.+)/;
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
			if (!$xypen)
				{
				$xypen = "N";
				}
			push(@xysymbols, $xysymbol);
			push(@xyfills, $xyfill);
			push(@xysegments, $xysegment);
			push(@xypens, $xypen);
			}

		# set xy segment
		if ($cmd =~ /^[Xx][Mm]/)
			{
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

# set root name if needed
if (!$root)
	{
	$root = $file_data;
	}

# get format if needed
if (!$format) 
	{
	$line = `mbformat -I $file_data -L`;
	($format) = $line =~ /(\S+)/;
	if ($format == 0)
		{
		$format = -1;
		}
	}

# set plot mode if needed
if (!$color_mode && !$contour_mode && !$navigation_mode)
	{
	$navigation_mode = 1;
	}
if ($shade_control && $color_mode == 3)
	{
	($magnitude, $zero_level) = $shade_control =~ /^(\S+)\/(\S+)/;
	}
elsif ($shade_control)
	{
	($magnitude, $azimuth) = $shade_control =~ /^(\S+)\/(\S+)/;
	}
if ($color_control)
	{
	if (-e $color_control)
		{
		$file_cpt = $color_control;
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
	$ncolors = $ncpt;
	if ($color_mode == 4 || $color_mode == 5)
		{
		$color_pallette = 4;
		}
	else
		{
		$color_pallette = 1;
		}
	}
if ($navigation_control && $navigation_control =~ /\S+\/\S+\/\S+\/\S+/)
	{
	$navigation_mode = 1;
	}
elsif ($navigation_control)
	{
	if ($navigation_control =~ /\S+\/\S+\/\S+/)
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
if ($color_mode == 2 && $shade_flip)
	{
	if ($azimuth > 180)
		{
		$azimuth = $azimuth - 180;
		}
	else
		{
		$azimuth = $azimuth + 180;
		}
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
		}
	}
# just set it to ghostview
if (!$ps_viewer)
	{
	$ps_viewer = "ghostview";
	}

# get limits of file using mbinfo
if ($format >= 0)
	{
	push(@files_data, $file_data);
	push(@formats, $format);
	}
else
	{
	# we used to use this perl function
	# 	MBparsedatalist($file_data);
	# but now we use the program mbdatalist
	@mbdatalist = `mbdatalist -F-1 -I$file_data`;
	while (@mbdatalist)
		{
		$line = shift @mbdatalist;
		if ($line =~ /(\S+)\s+(\S+)/)
			{
			($file_mb,$format_mb) = 
				$line =~ /(\S+)\s+(\S+)/;
			push(@files_data, $file_mb);
			push(@formats, $format_mb);
			}
		}
	}
if ($bounds)
	{
	$bounds_info = "-R$bounds";
	}
$cnt = 0;
foreach $file_mb (@files_data)
	{
	# use .inf file if it exists and no time or space bounds applied
	$use_inf = 0;
	$file_inf = $file_mb . ".inf";
	if (-r $file_inf && !$mb_btime && !$mb_etime && !$bounds_info)
		{
		if ($verbose) 
			{
			print "Reading mbinfo output from file $file_inf...\n";
			}
		if (open(FILEINF,"<$file_inf"))
			{
			while (<FILEINF>)
				{
				push(@mbinfo, $_);
				}
			close FILEINF;
			$use_inf = 1;
			}
		}

	# if .inf file not accessible or suitable run mbinfo directly 
	if (!$use_inf)
		{
		if ($verbose) 
			{
			print "Running mbinfo on file $file_mb...\n";
			}
		if ($mb_btime)
			{
			$time_info = "-B$mb_btime";
			}
		if ($mb_etime)
			{
			$time_info = $time_info . "-E$mb_etime";
			}
print"mbinfo -F$formats[$cnt] -I$file_mb $time_info $bounds_info -G\n";
		@mbinfo = `mbinfo -F$formats[$cnt] -I$file_mb $time_info $bounds_info -G`;		}

	# now parse the mbinfo input 
	$nrec_f = 0;
	while (@mbinfo)
		{
		$line = shift @mbinfo;
		if ($line =~ /Number of Records:\s+(\S+)/)
			{
			($nrec_f) = 
				$line =~ /Number of Records:\s+(\S+)/;
			}
		if ($line =~ /Minimum Longitude:\s+(\S+)\s+Maximum Longitude:\s+(\S+)/)
			{
			($xmin_f,$xmax_f) = 
				$line =~ /Minimum Longitude:\s+(\S+)\s+Maximum Longitude:\s+(\S+)/;
			}
		if ($line =~ /Minimum Latitude:\s+(\S+)\s+Maximum Latitude:\s+(\S+)/)
			{
			($ymin_f,$ymax_f) = 
				$line =~ /Minimum Latitude:\s+(\S+)\s+Maximum Latitude:\s+(\S+)/;
			}
		if ($line =~ /Minimum Depth:\s+(\S+)\s+Maximum Depth:\s+(\S+)/)
			{
			($zmin_f,$zmax_f) = 
			$line =~ /Minimum Depth:\s+(\S+)\s+Maximum Depth:\s+(\S+)/;
			}
		if ($line =~ /Minimum Amplitude:\s+(\S+)\s+Maximum Amplitude:\s+(\S+)/)
			{
			($amin_f,$amax_f) = 
			$line =~ /Minimum Amplitude:\s+(\S+)\s+Maximum Amplitude:\s+(\S+)/;
			}
		if ($line =~ /Minimum Sidescan:\s+(\S+)\s+Maximum Sidescan:\s+(\S+)/)
			{
			($smin_f,$smax_f) = 
			$line =~ /Minimum Sidescan:\s+(\S+)\s+Maximum Sidescan:\s+(\S+)/;
			}
		}

	if (!$first_mb && $nrec_f > 0)
		{
		$first_mb = 1;
		$xmin_data = $xmin_f;
		$xmax_data = $xmax_f;
		$ymin_data = $ymin_f;
		$ymax_data = $ymax_f;
		$zmin_data = $zmin_f;
		$zmax_data = $zmax_f;
		$amin_data = $amin_f;
		$amax_data = $amax_f;
		$smin_data = $smin_f;
		$smax_data = $smax_f;
		}
	elsif ($nrec_f > 0)
		{
		$xmin_data = &min($xmin_data, $xmin_f);
		$xmax_data = &max($xmax_data, $xmax_f);
		$ymin_data = &min($ymin_data, $ymin_f);
		$ymax_data = &max($ymax_data, $ymax_f);
		$zmin_data = &min($zmin_data, $zmin_f);
		$zmax_data = &max($zmax_data, $zmax_f);
		$amin_data = &min($amin_data, $amin_f);
		$amax_data = &max($amax_data, $amax_f);
		$smin_data = &min($smin_data, $smin_f);
		$smax_data = &max($smax_data, $smax_f);
		}
	$cnt++;
	}

# use user defined data limits for bathymetry if supplied
if ($zbounds)
	{
	($zmin,$zmax) = $zbounds =~ /(\S+)\/(\S+)/;
	}
else
	{
	$zmin = $zmin_data;
	$zmax = $zmax_data;
	}

# check that there is data
if ($xmin_data >= $xmax_data || $ymin_data >= $ymax_data)
	{
	die "$xmin_data $xmax_data $ymin_data $ymax_data \nDoes not appear to be any data in the input!\n$program_name aborted.\n";
	}
if (($color == 1 || $color == 2 || $color == 3) && ($zmin_data >= $zmax_data))
	{
	die "Does not appear to be any bathymetry data in the input!\n$program_name aborted.\n";
	}
if (($color == 3 || $color == 4) && ($amin_data >= $amax_data))
	{
	die "Does not appear to be any amplitude data in the input!\n$program_name aborted.\n";
	}
if (($color == 5) && ($smin_data >= $smax_data))
	{
	die "Does not appear to be any sidescan data in the input!\n$program_name aborted.\n";
	}

# either use user defined geographic limits
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

# or expand the data limits a bit and ensure a reasonable aspect ratio
else
	{
	$delx = 0.05 * ($xmax_data - $xmin_data);
	$dely = 0.05 * ($ymax_data - $ymin_data);
	$xmin = $xmin_data - $delx;
	$xmax = $xmax_data + $delx;
	$ymin = $ymin_data - $dely;
	$ymax = $ymax_data + $dely;
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
	$bounds_plot = sprintf ("%1.8g/%1.8g/%1.8g/%1.8g",
		$xmin, $xmax, $ymin, $ymax);
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

	# construct plot scale parameters
	($projection_pars) = $map_scale =~ /^$projection(\S+)/;
	$projection_pars = sprintf ("$projection_pars$separator%1.5g", $plot_width);

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

# figure out reasonable bathymetry contour intervals
$dmin = $zmin;
$dmax = $zmax;
$dd = ($dmax - $dmin); 
$contour_int = 0.0;
if ($dd > 0)
	{
	$base = int((log($dd) / log(10.)) + 0.5);
	$contour_int = (10 ** $base) / 10.0;
	if ($dd / $contour_int < 10)
		{
		$contour_int = $contour_int / 4;
		}
	elsif ($dd / $contour_int < 20)
		{
		$contour_int = $contour_int / 2;
		}
	}

# figure out reasonable color intervals
if ($color_mode >= 1 && $color_mode <= 3)
	{
	$dmin = $zmin;
	$dmax = $zmax;
	}
elsif ($color_mode == 4)
	{
	$dmin = $amin_data;
	$dmax = $amax_data;
	}
elsif ($color_mode == 5)
	{
	$dmin = $smin_data;
	$dmax = $smax_data;
	}
$dd = ($dmax - $dmin); 
$color_int = 0.0;
if ($dd > 0)
	{
	$base = int((log($dd) / log(10.)) + 0.5);
	$color_int = (10 ** $base) / 10.0;
	if ($dd / $color_int < 10)
		{
		$color_int = $color_int / 4;
		}
	elsif ($dd / $color_int < 20)
		{
		$color_int = $color_int / 2;
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
if ($color_mode && !$no_nice_color_int && $dd > 0)
	{
	$start_int = $color_int / 2;
	$multiplier = int($dd / ($ncolors_use - 1) / $start_int) + 1;
	$color_int = $multiplier * $start_int;
	if (($color_int * int($dmin / $color_int) 
		+ $color_int * ($ncolors_use - 1))
		< $zmax)
		{
		$multiplier = $multiplier + 1;
		$color_int = $multiplier * $start_int;
		}
	$color_start = $color_int * int($dmin / $color_int);
	$color_end = $color_start + $color_int * ($ncolors_use - 1);
	}
elsif ($color_mode)
	{
	$color_int = 1.02 * ($dmax - $dmin)/($ncolors_use - 1);
	$color_start = $dmin - 0.01*($dmax - $dmin);
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

# set contour control
if (!$contour_control && $contour_mode)
	{
	if ($color_mode)
		{
		$contour_color_int = 100000;
		}
	else
		{
		$contour_color_int = 4 * $contour_int;
		}
	$contour_tick_int = $contour_color_int;
	$contour_label_int = $contour_tick_int;
	$contour_tick_len = 0.001 * $page_height_in{$pagesize};
	$contour_label_hgt = 0.01 * $page_height_in{$pagesize};
	if ($contour_tick_len < 0.01 || $contour_label_hgt < 0.1)
		{
		$contour_tick_len = 0.01;
		$contour_label_hgt = 0.1;
		}
	}
elsif ($contour_control =~ /\S+\/\S+\/\S+\/\S+\/\S+\/\S+/)
	{
	($contour_int, $contour_color_int, $contour_tick_int, 
		$contour_label_int, $contour_tick_len, 
		$contour_label_hgt) 
		= $contour_control
		=~ /(\S+)\/(\S+)\/(\S+)\/(\S+)\/(\S+)\/(\S+)/;
	$contour_label_hgt = 0.1;
	}
elsif ($contour_control =~ /\S+\/\S+\/\S+\/\S+\/\S+/)
	{
	($contour_int, $contour_color_int, $contour_tick_int, 
		$contour_label_int, $contour_tick_len) 
		= $contour_control
		=~ /(\S+)\/(\S+)\/(\S+)\/(\S+)\/(\S+)/;
	$contour_label_hgt = 0.1;
	}
elsif ($contour_control =~ /\S+\/\S+\/\S+\/\S+/)
	{
	($contour_int, $contour_color_int, $contour_tick_int, 
		$contour_label_int) 
		= $contour_control 
		=~ /(\S+)\/(\S+)\/(\S+)\/(\S+)/;
	$contour_tick_len = 0.01;
	$contour_label_hgt = 0.1;
	}
elsif ($contour_control =~ /\S+\/\S+\/\S+/)
	{
	($contour_int, $contour_color_int, $contour_tick_int) 
		= $contour_control =~ /(\S+)\/(\S+)\/(\S+)/;
	$contour_label_int = $contour_color_int;
	$contour_tick_len = 0.01;
	$contour_label_hgt = 0.1;
	}
elsif ($contour_control =~ /\S+\/\S+/)
	{
	($contour_int, $contour_color_int) 
		 = $contour_control =~ /(\S+)\/(\S+)/;
	$contour_tick_int = $contour_color_int;
	$contour_label_int = $contour_color_int;
	$contour_tick_len = 0.01;
	$contour_label_hgt = 0.1;
	}
elsif ($contour_control =~ /\S+/)
	{
	($contour_int) = $contour_control =~ /(\S+)/;
	if ($color_mode)
		{
		$contour_color_int = 100000;
		}
	else
		{
		$contour_color_int = 4*$contour_int;
		}
	$contour_tick_int = $contour_color_int;
	$contour_label_int = $contour_color_int;
	$contour_tick_len = 0.01;
	$contour_label_hgt = 0.1;
	}
if ($contour_mode)
	{
	$contour_control = "$contour_int" . "/" . "$contour_color_int"
			. "/" . "$contour_tick_int" . "/" . "$contour_label_int"
			. "/" . "$contour_tick_len" . "/" . "$contour_label_hgt";
	}

# set pscoast control
if ($coast_control
	&& !$coast_wetfill
	&& !$coast_dryfill
	&& !$coast_coast
	&& !$coast_boundary
	&& !$coast_river)
	{
	$coast_dryfill = "200";
	$coast_pen = "1p";
	}
if ($coast_control
	&& !$coast_resolution)
	{
	$coast_resolution = "f";
	}

# come up with the filenames
$cmdfile = "$root.cmd";
$psfile = "$root.ps";
if ($format == -1)
	{
	$file_list = "\$INPUT_FILE";
	}
else
	{
	$file_list = "datalist\$\$";
	}
$cptfile = "$root.cpt";
$cptshadefile = "$root.shade.cpt";
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

# set shade control if not set by user
if (!$shade_control && $color_mode == 3 && !stretch_shade)
	{
	$magnitude = 2.0/($amax_data - $amin_data);
	$zero_level = $amin_data + 0.2 * ($amax_data - $amin_data);
	$shade_control = sprintf("%.4g/%.4g",$magnitude,$zero_level);
	}
elsif (!$shade_control && $color_mode == 3)
	{
	$magnitude = 1;
	$zero_level = 191;
	$shade_control = sprintf("%.4g/%.4g",$magnitude,$zero_level);
	}
elsif (!$shade_control)
	{
	$azimuth = 0;
	$magnitude = 2.5;
	$shade_control = "2.5/0";
	}
elsif ($color_mode == 3)
	{
	$shade_control = "$magnitude/$zero_level";
	}
else
	{
	$shade_control = "$magnitude/$azimuth";
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
if ($nlabels < 1 && $format < 0)
	{
	$tlabel = "Data List File $file_list";
	}
elsif ($nlabels < 1)
	{
	$tlabel = "Data File \$INPUT_FILE";
	}
if ($nlabels < 2)
	{
	if ($color_mode == 5)
		{
		$slabel = "Sidescan Pixel Values";
		}
	elsif ($color_mode == 4)
		{
		$slabel = "Beam Amplitude Values";
		}
	else
		{
		$slabel = "Depth (meters)";
		}
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
	$axes = "$base_tick/$base_tick:.\"$tlabel\":";
	}

# open the shellscript file
if (!open(FCMD,">$cmdfile"))
	{
	print "\a";
	die "Cannot open output file $cmdfile\nMacro $program_name aborted.\n";
	}

# write the shellscript header
print FCMD "#! /bin/csh -f\n";
print FCMD "#\n# Shellscript to create Postscript plot of swath sonar data\n";
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
print FCMD "set INPUT_FILE      = $file_data\n";
print FCMD "set INPUT_FORMAT    = $format\n";

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

# generate datalist file if needed
if ($format != -1)
	{
	print FCMD "#\n# Make datalist file \n";
	print FCMD "echo Making datalist file...\n";
	print FCMD "echo \$INPUT_FILE \$INPUT_FORMAT >! $file_list\n";
	}

# generate color pallette table file if needed
if ($color_mode && $file_cpt)
	{
	$cptfile = $file_cpt;
	}
elsif ($color_mode)
	{
	# break data distribution up into equal size 
	# regions using mbhistogram
	if ($stretch_color)
		{
		if ($verbose) 
			{
			print "Running mbhistogram...\n";
			}
		if ($color_mode == 4)
			{
			$data_type = 1;
			}
		elsif ($color_mode == 5)
			{
			$data_type = 2;
			}
		else
			{
			$data_type = 0;
			}
		@mbhistogram = `mbhistogram -F$format -I$file_data -A$data_type -D$dmin/$dmax -M$ncolors -N1000`;
		}

	# generate cpt file for primary data
	print FCMD "#\n# Make color pallette table file\n";
	print FCMD "echo Making color pallette table file...\n";
	if ($stretch_color)
		{
		$d1 = shift @mbhistogram;
		}
	else
		{
		$d1 = $color_start;
		}
	if ($color_style == 1 && !$color_flip)
		{
		foreach $i (0 .. $ncolors - 2)
			{
			if ($stretch_color)
				{
				$d2 = shift @mbhistogram;
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
				print FCMD " >!";
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
			if ($stretch_color)
				{
				$d2 = shift @mbhistogram;
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
				print FCMD " >!";
				}
			else
				{
				print FCMD " >>";
				}
			print FCMD " \$CPT_FILE\n";
			$d1 = $d2
			}
		}
	elsif (!$color_flip)
		{
		foreach $i (0 .. $ncolors - 1)
			{
			if ($stretch_color)
				{
				$d2 = shift @mbhistogram;
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
				print FCMD " >!";
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
			if ($stretch_color)
				{
				$d2 = shift @mbhistogram;
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
				print FCMD " >!";
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

# generate cpt file for amplitude shading
if ($color_mode == 3 && $stretch_shade)
	{
	# break data distribution up into equal size 
	# regions of Gaussian distribution using mbhistogram
	if ($verbose) 
		{
		print "Running mbhistogram...\n";
		}
	$data_type = 1;
	@mbhistogram = `mbhistogram -F$format -I$file_data -A$data_type -D$amin_data/$amax_data -M$ncpt -N1000 -G`;

	print FCMD "#\n# Make shading control table file\n";
	print FCMD "echo Making shading control table file...\n";
	$d1 = shift @mbhistogram;
	if (!$shade_flip)
		{
		foreach $i (0 .. $ncolors - 2)
			{
			$d2 = shift @mbhistogram;
			printf FCMD "echo %6g %3d %3d %3d %6g %3d %3d %3d",
				$d1,@cptbr4[$i],@cptbg4[$i],@cptbb4[$i],
				$d2,@cptbr4[$i+1],@cptbg4[$i+1],@cptbb4[$i+1];
			if ($i == 0)
				{
				print FCMD " >";
				}
			else
				{
				print FCMD " >>";
				}
			print FCMD " $cptshadefile\n";
			$d1 = $d2
			}
		}
	else
		{
		for ($i = $ncolors - 2; $i >= 0; $i--)
			{
			$d2 = shift @mbhistogram;
			printf FCMD "echo %6g %3d %3d %3d %6g %3d %3d %3d",
				$d1,@cptbr4[$i+1],@cptbg4[$i+1],@cptbb4[$i+1],
				$d2,@cptbr4[$i],@cptbg4[$i],@cptbb4[$i];
			if ($i == ($ncolors - 2))
				{
				print FCMD " >";
				}
			else
				{
				print FCMD " >>";
				}
			print FCMD " $cptshadefile\n";
			$d1 = $d2
			}
		}
	} # end of making amplitude shading file

# do swath plot if needed
if ($color_mode)
	{
	print FCMD "#\n# Run mbswath\n";
	print FCMD "echo Running mbswath...\n";
	printf FCMD "mbswath -f-1 -I$file_list \\\n\t";
	printf FCMD "-J\$MAP_PROJECTION\$MAP_SCALE \\\n\t";
	printf FCMD "-R\$MAP_REGION \\\n\t";
	printf FCMD "-C\$CPT_FILE \\\n\t";
	print FCMD "-Z$color_mode \\\n\t";
	if ($swath_footprint)
		{
		printf FCMD "-A$swath_footprint \\\n\t";
		}
	else
		{
		printf FCMD "-A1 \\\n\t";
		}
	if ($color_mode == 2 || $color_mode == 3)
		{
		print FCMD "-G$shade_control \\\n\t";
		}
	if ($color_mode == 3 && $stretch_shade)
		{
		print FCMD "-N$cptshadefile \\\n\t";
		}
	if ($dpi)
		{
		printf FCMD "-Q$dpi \\\n\t";
		}
	if ($mb_btime)
		{
		printf FCMD "-b$mb_btime \\\n\t";
		}
	if ($mb_etime)
		{
		printf FCMD "-E$mb_etime \\\n\t";
		}
	printf FCMD "-p$mb_pings \\\n\t";
	if ($mb_speedmin)
		{
		printf FCMD "-S$mb_speedmin \\\n\t";
		}
	if ($mb_timegap)
		{
		printf FCMD "-T$mb_timegap \\\n\t";
		}
	if ($swath_scale)
		{
		printf FCMD "-D$swath_scale \\\n\t";
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

# do contour plot if needed
if ($contour_mode || $navigation_mode)
	{
	print FCMD "#\n# Run mbcontour\n";
	print FCMD "echo Running mbcontour...\n";
	printf FCMD "mbcontour -f-1 -I$file_list \\\n\t";
	printf FCMD "-J\$MAP_PROJECTION\$MAP_SCALE \\\n\t";
	printf FCMD "-R\$MAP_REGION \\\n\t";
	if ($contour_mode && $contour_algorithm)
		{
		printf FCMD "-Z$contour_algorithm \\\n\t";
		}
	elsif ($contour_mode && 
		($format == 41 || $format == 101
		|| $format == 102))
		{
		print FCMD "-Z1 ";
		}
	if ($contour_mode)
		{
		printf FCMD "-A$contour_control \\\n\t";
		}
	if ($navigation_mode)
		{
		printf FCMD "-D$navigation_control \\\n\t";
		}
	if ($mb_btime)
		{
		printf FCMD "-b$mb_btime \\\n\t";
		}
	if ($mb_etime)
		{
		printf FCMD "-E$mb_etime \\\n\t";
		}
	printf FCMD "-p$mb_pings \\\n\t";
	if ($mb_speedmin)
		{
		printf FCMD "-S$mb_speedmin \\\n\t";
		}
	if ($mb_timegap)
		{
		printf FCMD "-T$mb_timegap \\\n\t";
		}
	if ($contour_nplot)
		{
		printf FCMD "-N$contour_nplot \\\n\t";
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
		printf FCMD "-M \\\n\t";
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

# do psscale plot
if ($color_mode && $color_pallette < 5)
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
print FCMD "/bin/rm -f \$CPT_FILE\n";
if ($format > -1)
	{
	print FCMD "/bin/rm -f $file_list\n";
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
		print "    Color Fill Bathymetry\n";
		}
	elsif ($color_mode == 2)
		{
		print "    Color Shaded Relief Bathymetry\n";
		}
	elsif ($color_mode == 3)
		{
		print "    Color Bathymetry Shaded with Amplitude\n";
		}
	elsif ($color_mode == 4)
		{
		print "    Grayscale Amplitude\n";
		}
	elsif ($color_mode == 5)
		{
		print "    Grayscale Sidescan\n";
		}
	if ($contour_mode)
		{
		print "    Contoured Bathymetry\n";
		}
	if ($navigation_mode)
		{
		print "    Navigation\n";
		}
	if ($coast_control)
		{
		print "    Coastline\n";
		}
	if (@xyfiles)
		{
		print "    XY Plots of ", scalar(@xyfiles), " Datasets\n";
		}
	if ($color_mode && $color_pallette != 5)
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
	if ($format < 0)
		{
		print "    Data list file:           $file_data\n";
		}
	else
		{
		print "    Multibeam data file:      $file_data\n";
		print "    Data format id:           $format\n";
		}
	foreach $xyfile (@xyfiles) {
		print "    XY Data File:             $xyfile\n";
	}
	print "\n  Output Files:\n";
	print "    Output plot name root:    $root\n";
	print "    Color pallette table:     $cptfile\n";
	if ($color_mode == 3 && $stretch_shade)
		{
		print "    Shade control table:      $cptshadefile\n";
		}
	print "    Plotting shellscript:     $cmdfile\n";
	print "    Plot file:                $psfile\n";
	print "\n  Plot Attributes:\n";
	printf "    Plot width:               %.4f\n", $plot_width;
	printf "    Plot height:              %.4f\n", $plot_height;
	print "    Page size:                $pagesize\n";
	print "    Page width:               $width\n";
	print "    Page height:              $height\n";
	print "    Projection:               -J$projection$projection_pars\n";
	printf "    Longitude min max:        %9.4f  %9.4f\n", 
		$xmin, $xmax;
	printf "    Latitude min max:         %9.4f  %9.4f\n", 
		$ymin, $ymax;
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
		print "    Color pallette:           ", 
			"@color_pallette_names[$color_pallette - 1]\n";
		if ($color_style == 1)
			{
			print "    Color style:              continuous\n";
			}
		else
			{
			print "    Color style:              stepped\n";
			}
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
	print "\n  Sonar Data Attributes:\n";
	if ($data_scale)
		{
		print "    Data scale factor:        $data_scale\n";
		}
	printf "    Longitude min max:        %9.4f  %9.4f\n", 
		$xmin_data, $xmax_data;
	printf "    Latitude min max:         %9.4f  %9.4f\n", 
		$ymin_data, $ymax_data;
	printf "    Bathymetry min max:       %9.4g  %9.4g\n", 
		$zmin_data, $zmax_data;
	printf "    Amplitude min max:        %9.4g  %9.4g\n", 
		$amin_data, $amax_data;
	printf "    Sidescan min max:         %9.4g  %9.4g\n", 
		$smin_data, $smax_data;
	print "\n  Primary Plotting Controls:\n";
	if ($contour_mode)
		{
		print "    Contour control:          $contour_control\n";
		}
	if ($navigation_mode)
		{
		print "    Navigation control:       $navigation_control\n";
		}
	if ($color_mode)
		{
		printf "    Color start datum:        %f\n", $color_start;
		printf "    Color end datum:          %f\n", $color_end;
		}
	if ($color_mode && !$stretch_color)
		{
		printf "    Color datum interval:     %f\n", $color_int;
		}
	elsif ($color_mode && $stretch_color)
		{
		printf "    Histogram stretch applied to color pallette\n";
		}
	if ($dpi && $color_mode)
		{
		printf "    Image dots-per-inch:      $dpi\n";
		}
	if ($color_mode == 2)
		{
		printf "    Illumination Azimuth:     %f\n", $azimuth;
		printf "    Illumination Magnitude:   %f\n", $magnitude;
		}
	elsif ($color_mode == 3)
		{
		printf "    Amplitude zero level:     %f\n", $zero_level;
		printf "    Amplitude shade magnitude:%f\n", $magnitude;
		}
	if ($color_mode == 3  && $stretch_shade)
		{
		printf "    Histogram stretch applied to amplitude shading\n";
		}
	if ($mb_btime || $mb_etime 
		|| $mb_pings != 1 || $mb_speedmin
		|| $mb_timegap)
		{
		print "\n  General MB-System Controls:\n";
		}
	if ($mb_btime)
		{
		print "    Begin time:               $mb_btime\n";
		}
	if ($mb_etime)
		{
		print "    End time:                 $mb_etime\n";
		}
	if ($mb_pings != 1)
		{
		print "    Ping averaging:           $mb_pings\n";
		}
	if ($mb_speedmin)
		{
		print "    Minimum speed:            $mb_speedmin\n";
		}
	if ($mb_timegap)
		{
		print "    Time gap:                 $mb_timegap\n";
		}
	if ($swath_footprint || $swath_scale 
		|| $contour_nplot || $contour_algorithm)
		{
		print "\n  Miscellaneous Plotting Controls:\n";
		}
	if ($swath_footprint)
		{
		print "    Beam footprint control:   $swath_footprint\n";
		}
	if ($swath_scale)
		{
		print "    Multibeam data scaling:   $swath_scale\n";
		}
	if ($contour_nplot)
		{
		print "    Pings to contour:         $contour_nplot\n";
		}
	if ($contour_mode && $contour_algorithm == 0)
		{
		print "    Contour algorithm:        ping-to-ping\n";
		}
	elsif ($contour_algorithm == 1)
		{
		print "    Contour algorithm:        triangle network\n";
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
	if (@xyfiles)
		{
		print "\n  Primary XY Plotting Controls:\n";
		printf "    symbol     pen        fill      segment      file\n";
		printf "    ------     ---        ----      -------      ----\n";
		for ($i = 0; $i < scalar(@xyfiles); $i++) 
			{
			printf "    %-10s %-10s %-10s %-10s %s\n", 
				$xysymbols[$i], $xypens[$i], 
				$xyfills[$i], $xysegments[$i], 
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
sub MBparsedatalist {
	local ($FILEDATA, $line, $file_tmp, $format_tmp, @datalists);

 	if (open(FILEDATA,"<$_[0]"))
        	{
        	while ($line = <FILEDATA>)
        		{
			if (!($line =~ /^#/)
			    && $line =~ /\S+\s+\S+/)
			    {
        		    ($file_tmp, $format_tmp) = $line =~ /(\S+)\s+(\S+)/;
        		    if ($file_tmp && $format_tmp >= 0)
        		 	{
        			push(@files_data, $file_tmp);
        			push(@formats, $format_tmp);
        			}
			    elsif ($file_tmp && $format_tmp == -1)
				{
        			push(@datalists, $file_tmp);
				}
			    }
        		}
        	close FILEDATA;
        	}

	# loop over datalists 
	foreach $datalist (@datalists)
		{
		MBparsedatalist($datalist);
		}

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
