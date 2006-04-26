.TH mbm_histplot l "18 April 2006" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbm_histplot\fP - Create an executable shellscript which will generate
a GMT histogram plot of a dataset.

.SH VERSION
Version 5.0

.SH SYNOPSIS
\fBmbm_histplot\fP \fB-I\fP\fIfile\fP 
[\fB-G\fP\fIfill\fP \fB-H\fP 
\fB-O\fP\fIroot\fP \fB-P\fP\fIpagesize\fP
\fB-U\fP\fIorientation\fP \fB-V\fP 
\fB-W\fP\fIpen\fP ]

Additional Options:
.br
[\fB-B\fP\fItickinfo\fP \fP
\fB-J\fP\fIprojection\fP[\fI/scale | width\fP]
\fB-L\fP\fItitle\fP[\fI:xlabel:ylabel\fP] \fB-M\fP\fImisc\fP 
\fB-Q\fP \fB-R\fP\fIw/e/s/n\fP \fB-X\fP \fB-Z\fP]

.SH DESCRIPTION
\fBmbm_histplot\fP is a macro to generate a shellscript of 
GMT commands which, when executed, will generate a Postscript 
histogram plot of a set of data values.
The plot will be scaled to fit on the specified page size 
or, if the scale is user defined, the page size will be 
chosen in accordance with the plot size. The primary purpose 
of this macro is to allow the simple, semi-automated
production of nice looking plots with a few command line
arguments. For users seeking more control over the plot
appearance, a number of additional optional arguments are
provided. Truly ambitious users may edit the plot shellscript 
to take advantage of GMT capabilites not supported by this 
macro.

.SH AUTHORSHIP
David W. Caress (caress@mbari.org)
.br
  Monterey Bay Aquarium Research Institute
.br
Dale N. Chayes (dale@ldeo.columbia.edu)
.br
  Lamont-Doherty Earth Observatory
.br
Suzanne H. O'Hara (sohara@ldeo.columbia.edu)
.br
  Lamont-Doherty Earth Observatory

.SH OPTIONS
.TP
.B \-B
\fItickinfo\fP
.br
Sets map boundary tickmark intervals. See the \fBpsbasemap\fP
manual page for details. By default the program chooses
basemap annotations based on the map boundaries.
.TP
.B \-C
\fIcellwidth\fP
.br
Sets the width of the cells into which the data are binned to form
the histogram. The default is use a cell width that is 1/10 the
data range.
.TP
.B \-G
\fIfill\fP 
.br
Select filling of the histogram.  
Set the shade (0\-255) or color 
(r/g/b) [Default is a light gray]. To reset no fill,
use \fIfill\fP = "N".
You may optionally specify 
\fB\-Gp\fP\fIicon_size/pattern\fP, where 
\fIpattern\fP gives the number of the
image pattern (1-32) OR the name of a 
icon-format file.  \fIicon_size\fP sets 
the unit size in inch.
To invert black and white pixels, use 
\fB\-GP\fP instead of \fB\-Gp\fP.  See 
\fBGMTs\fP Cookbook & Technical Reference
Appendix E for information on individual patterns.
.TP
.B \-H
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \-I
\fI[filepars:]xy_file\fP
.br

Specifies the file containing the data to be plotted in a
histogram. A single column of data is expected.
.TP
.B \-J
\fIprojection\fP[\fI/width/height\fP]
.br
Sets the plot scale. The projection is linear (\fB-J\fP\fIX), and
the user can optionally specify the width and height.
.TP
.B \-L
\fItitle[:xlabel[:ylabel]]\fP
.br
Sets the title and the labels for the x and y axes of
the plot. Note that a colon (:) rather than a slash (/) is
used to separate the labels. Colons cannot be used in the
labels themselves. If this option is not used, then a default title
and colorscale label are provided. If the title is supplied
alone, no x or y-axis labels will be provided.
.TP
.B \-O
\fIroot\fP
.br
Sets the root used to construct the filename of the output shellscript
(\fIroot\fP.cmd) and names of files created when the shellscript is 
run.  Normally the 
name of the input grid file or grid file list is 
used as the \fIroot\fP. 
.TP
.B \-P
\fIpagesize\fP
.br
This option sets the size of the page the plot will be centered
on. If the user does not set the plot scale, the plot will be
sized as large as will fit on the designated page. If the user
sets the plot scale such that the plot will not fit on the
designated page, a larger page will be used.
The supported page sizes are:

          American ANSI sizes:
          A     8.5 x 11.0 in.    ( 215.9 x  279.4 mm)
          B    11.0 x 17.0 in.    ( 279.4 x  431.8 mm)
          C    17.0 x 22.0 in.    ( 431.8 x  558.8 mm)
          D    22.0 x 34.0 in.    ( 558.8 x  863.6 mm)
          E    34.0 x 44.0 in.    ( 863.6 x 1117.6 mm)
          F    28.0 x 40.0 in.    ( 711.2 x 1016.0 mm)
          E1   44.0 x 68.0 in.    (1117.6 x 1727.2 mm)

          Metric ISO A sizes:
          A0   841.0 x 1189.0 mm  (33.11 x 46.81 in.)
          A1   594.0 x  841.0 mm  (23.39 x 33.11 in.)
          A2   420.0 x  594.0 mm  (16.54 x 23.39 in.)
          A3   297.0 x  420.0 mm  (11.69 x 16.54 in.)
          A4   210.0 x  297.0 mm  ( 8.27 x 11.69 in.)
          A5   148.0 x  210.0 mm  ( 5.83 x  8.27 in.)
          A6   105.0 x  148.0 mm  ( 4.13 x  5.83 in.)
          A7    74.0 x  105.0 mm  ( 2.91 x  4.13 in.)
          A8    52.0 x   74.0 mm  ( 2.05 x  2.91 in.)
          A9    37.0 x   52.0 mm  ( 1.46 x  2.05 in.)
          A10   26.0 x   37.0 mm  ( 1.02 x  1.46 in.)

          Metric ISO B sizes:
          B0   1000.0x 1414.0 mm  (39.37 x 55.67 in.)
          B1   707.0 x 1000.0 mm  (27.83 x 39.37 in.)
          B2   500.0 x  707.0 mm  (19.68 x 27.83 in.)
          B3   353.0 x  500.0 mm  (13.90 x 19.68 in.)
          B4   250.0 x  353.0 mm  ( 9.84 x 13.90 in.)
          B5   176.0 x  250.0 mm  ( 6.93 x  9.84 in.)
          B6   125.0 x  176.0 mm  ( 4.92 x  6.93 in.)
          B7    88.0 x  125.0 mm  ( 3.46 x  4.92 in.)
          B8    62.0 x   88.0 mm  ( 2.44 x  3.46 in.)
          B9    44.0 x   62.0 mm  ( 1.73 x  2.44 in.)
          B10   31.0 x   44.0 mm  ( 1.22 x  1.73 in.)

          Metric ISO C sizes:
          C0   914.4 x 1300.5 mm  (36.00 x 51.20 in.)
          C1   650.2 x  914.4 mm  (25.60 x 36.00 in.)
          C2   457.2 x  650.2 mm  (18.00 x 25.60 in.)
          C3   325.1 x  457.2 mm  (12.80 x 18.00 in.)
          C4   228.6 x  325.1 mm  ( 9.00 x 12.80 in.)
          C5   162.6 x  228.6 mm  ( 6.40 x  9.00 in.)
          C6   114.3 x  162.6 mm  ( 4.50 x  6.40 in.)
          C7    81.3 x  114.3 mm  ( 3.20 x  4.50 in.)

The default page size is A.
.TP
.B \-Q
Normally, the output plot generation shellscript 
includes lines which execute
a program to display the Postscript image on the screen.  
This option causes those lines to be commented out so
that executing the shellscript produces a Postscript plot
but does not attempt to display it on the screen.
The program
to be used to display the Postscript is set 
using \fBmbdefaults\fP;
the default value can be overridden by setting the environment
variable $MB_PS_VIEWER. 
.TP
.B \-R
\fIwest/east/south/north\fP
.br
\fIwest, east, south,\fP and \fInorth\fP specify the Region of interest. [Default is
the range of the data and the histogram distribution].
.TP
.B \-U
\fIorientation\fP
.br
Normally the orientation of the plot (portrait or landscape)
is selected automatically so as to maximize the plot scale.
The \fB-U\fP option allows the user to set the plot orientation. If
\fIorientation\fP = 1, a portrait plot will be produced; if
\fIorientation\fP = 2, a landscape plot will be produced.
.TP
.B \-V
Causes \fBmbm_histplot\fP to operate in "verbose" mode 
so that it outputs
more information than usual.
.TP
.B \-W
\fIpen\fP
.br
Set pen attributes for plotting. See chapter 4.12 in the
GMT Technical reference for a discussion of GMT pen values.  
[Defaults: width = 1, color = 0/0/0, 
texture = solid]. 
.TP
.B \-X
Normally, \fBmbm_histplot\fP creates an executable shellscript and
then exits.  This option will cause the shellscript to be executed
in the background before \fBmbm_histplot\fP exits.

.SH EXAMPLES
Suppose we have a single column ascii file of time lag estimates called
timelag.dat. In order to generate a 6 inch wide and 4 inches high
simple histogram plot of these data, use:
.br
	mbm_histplot -Itimelag.dat -Otimelaghist -JX6/4 -V
to generate a GMT plot shellscript called timelaghist.cmd.
 
As an example, the contents of the plotting shellscript 
"timelaghist.cmd" are:

#! /bin/csh -f
#
# Shellscript to create Postscript plot of data in grd file
# Created by macro mbm_histplot
#
# This shellscript created by following command line:
# mbm_histplot -Itimelag.dat -Otimelaghist -JX6/4 -V
#
# Define shell variables used in this script:
set PS_FILE         = timelaghist.ps
set CPT_FILE        = 
set MAP_PROJECTION  = X
set MAP_SCALE       = 6/4
set MAP_REGION      = -0.57/0.12/0/50
set X_OFFSET        = 2.5
set Y_OFFSET        = 2.375
#
# Save existing GMT defaults
echo Saving GMT defaults...
gmtdefaults -L >! gmtdefaults$$
#
# Set new GMT defaults
echo Setting new GMT defaults...
gmtset MEASURE_UNIT inch
gmtset PAPER_MEDIA archA+
gmtset ANOT_FONT Helvetica
gmtset LABEL_FONT Helvetica
gmtset HEADER_FONT Helvetica
gmtset ANOT_FONT_SIZE 8
gmtset LABEL_FONT_SIZE 8
gmtset HEADER_FONT_SIZE 10
gmtset FRAME_WIDTH 0.075
gmtset TICK_LENGTH 0.075
gmtset PAGE_ORIENTATION LANDSCAPE
gmtset COLOR_BACKGROUND 0/0/0
gmtset COLOR_FOREGROUND 255/255/255
gmtset COLOR_NAN 255/255/255
gmtset DEGREE_FORMAT 3
#
# Make histogram
echo Running pshistogram...
pshistogram timelag.dat \
	-J$MAP_PROJECTION$MAP_SCALE \
	-R$MAP_REGION \
	-B0.05:" ":/5:"Frequency %"::."Frequency Histogram of timelag.dat": \
	-W0.069 -Z1 \
	-L1p \
	-Ggray \
	-X$X_OFFSET -Y$Y_OFFSET -V >! $PS_FILE
#
# Delete surplus files
echo Deleting surplus files...
/bin/rm -f $CPT_FILE
#
# Reset GMT default fonts
echo Resetting GMT fonts...
/bin/mv gmtdefaults$$ .gmtdefaults
#
# Run gv
echo Running gv in background...
gv --orientation=landscape --media=BBox $PS_FILE &
#
# All done!
echo All done!

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbdefaults\fP(l), 
\fBmbm_grdplot\fP(l), \fBmbm_grd3dplot\fP(l),
\fBmbm_plot\fP(l)

.SH BUGS
Please let us know.




