.TH mbm_xyplot l "4 January 2001" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbm_xyplot\fP - Create an executable shellscript which will generate
a GMT plot of xy data.

.SH VERSION
Version 5.0

.SH SYNOPSIS
\fBmbm_xyplot\fP \fB-I\fP\fI[filepars:]file\fP 
[\fB-G\fP\fIfill\fP \fB-H\fP 
\fB-O\fP\fIroot\fP \fB-P\fP\fIpagesize\fP
\fB-S\fP\fIsymbol/size\fP
\fB-U\fP\fIorientation\fP \fB-V\fP 
\fB-W\fP\fIpen\fP ]

Additional Options:
.br
[\fB-B\fP\fItickinfo\fP \fP
\fB-J\fP\fIprojection\fP[\fI/scale | width\fP]
\fB-L\fP\fItitle\fP[\fI:xlabel:ylabel\fP] \fB-M\fP\fImisc\fP 
\fB-Q\fP \fB-R\fP\fIw/e/s/n\fP \fB-X\fP ]

Miscellaneous Options:
.br
[\fB-MGD\fP\fIgmtdef/value\fP
\fB-MGL\fP[\fIf\fP][\fIx\fP]\fIlon0/lat0/slat/length\fP[\fIm\fP]
\fB-MGT\fP\fIx/y/size/angle/font/just/text\fP
\fB-MGU\fP\fP[\fI/dx/dy/\fP][\fIlabel\fP] ]

.SH DESCRIPTION
\fBmbm_xyplot\fP is a macro to generate a shellscript of GMT commands which, 
when executed, will generate a Postscript plot of xy data.
Axes may be linear, log,  or any of several geographic 
projections. Data may be plotted as symbols or lines.
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

.SH SIMPLE DESCRIPTION OF BASIC OPTIONS
.TP
.B \-G
\fP\fIfill\fP 
.br
Select filling of symbols for xy plotting.  
Set the shade (0\-255) or color 
(r/g/b) [Default is no fill]. To reset no fill,
use \fIfill\fP = "N".
For polygons, you may optionally specify 
\fB-Gp\fP\fIicon_size/pattern\fP, where 
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
\fI[filepars:]xy_file \fP
.br
Specifies a file containing (x,y) pairs to be plotted
as lines or symbols. The line and symbol characteristics
are set using the last \fB-G\fP, \fB-S\fP, and \fB-W\fP
options used. In order to plot different files
using different line or symbol characteristics, 
\fB-G\fP, \fB-S\fP, and \fB-W\fP commands can be
strung together in front of \fIxy_file\fP; the "-"
is omitted and the commands are separated by ":".
For example, the command 
\fB-IG\fP\fI255/0/0:\fP\fBS\fP\fIa/0.1:xy.dat\fP
will plot the (x,y) data pairs in the file xy.dat
as 0.1 inch diameter red stars.
[Default is a solid black line].
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
The supported page sizes include ANSI A, B, C, D, E,
F, and E1, as well as most metric page sizes. See the
COMPLETE DESCRIPTION OF OPTIONS section 
below for a complete list of 
the supported page sizes. The default page size is A.
.TP
.B \-S
\fIsymbol/size\fP 
.br
Selects symbol to be used for plotting the next xy data
file. Setting \fIsymbol\fP = "N" causes line plotting.
The list of available symbols is given in the
COMPLETE DESCRIPTION OF OPTIONS section below.
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
Causes \fBmbm_grdplot\fP to operate in "verbose" mode 
so that it outputs
more information than usual.
.TP
.B \-W
\fIpen\fP
.br
Set pen attributes for xy plotting. See chapter 4.12 in the
GMT Technical reference for a discussion of GMT pen values.  
[Defaults: width = 1, color = 0/0/0, 
texture = solid]. 

.SH COMPLETE DESCRIPTION OF OPTIONS
.TP
.B \-B
\fItickinfo\fP
.br
Sets map boundary tickmark intervals. See the \fBpsbasemap\fP
manual page for details. By default the program chooses
basemap annotations based on the map boundaries.
.TP
.B \-G
\fIfill\fP 
.br
Select filling of symbols for xy plotting.  
Set the shade (0\-255) or color 
(r/g/b) [Default is no fill]. To reset no fill,
use \fIfill\fP = "N".
For polygons, you may optionally specify 
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
\fI[filepars:]xy_file \fP
.br
Specifies a file containing (x,y) pairs to be plotted
as lines or symbols. The line and symbol characteristics
are set using the last \fB-G\fP, \fB-S\fP, and \fB-W\fP
options used. In order to plot different files
using different line or symbol characteristics, 
\fB-G\fP, \fB-S\fP, and \fB-W\fP commands can be
strung together in front of \fIxy_file\fP; the "-"
is omitted and the commands are separated by ":".
For example, the command 
\fB-IG\fP\fI255/0/0:\fP\fBS\fP\fIa/0.1:xy.dat\fP
will plot the (x,y) data pairs in the file xy.dat
as 0.1 inch diameter red stars.
[Default is a solid black line].
.TP
.B \-J
\fIprojection\fP[\fI/scale | /width\fP]
.br
Selects the map projection. By default the map projection is
Mercator and the plot scale is chosen to fit on the selected
page size (see \fB-P\fP option). The user may specify a
different projection to be used, in which case the plot scale
is still automatically chosen to fit the page. The user may
also specify both the projection and the plot scale. If
the projection specifying character is upper case, a plot
width rather than a plot scale is used.
The scale values are specified in inch/degree or in 1:xxxxx
ratios. Plot widths are specified in inches. If the user
specifies a plot scale such that the plot will not fit
on the default A size page, a appropriately larger page
size will be chosen.
.br
.sp
\fBCYLINDRICAL PROJECTIONS:\fP
.br
.sp
\fB\-Jc\fP\fIlon0/lat0/scale\fP (Cassini)
.br
\fB\-Jm\fP\fIscale\fP (Mercator)
.br
\fB\-Joa\fP\fIlon0/lat0/azimuth/scale\fP (Oblique Mercator - point and azimuth)
.br
\fB\-Job\fP\fIlon0/lat0/lon1/lat1/scale\fP (Oblique Mercator - two points)
.br
\fB\-Joc\fP\fIlon0/lat0/lonp/latp/scale\fP (Oblique Mercator - point and pole)
.br
\fB\-Jq\fP\fIlon0/scale\fP (Equidistant Cylindrical Projection (Plate Carree))
.br
\fB\-Jt\fP\fIlon0/scale\fP (TM - Transverse Mercator)
.br
\fB\-Ju\fP\fIzone/scale\fP (UTM - Universal Transverse Mercator)
.br
\fB\-Jy\fP\fIlon0/lats/scale\fP (Basic Cylindrical Projection) 
.br
.sp
\fBAZIMUTHAL PROJECTIONS:\fP
.br
.sp
\fB\-Ja\fP\fIlon0/lat0/scale\fP (Lambert).
.br
\fB\-Je\fP\fIlon0/lat0/scale\fP (Equidistant).
.br
\fB\-Jg\fP\fIlon0/lat0/scale\fP (Orthographic).
.br
\fB\-Js\fP\fIlon0/lat0/scale\fP (General Stereographic)
.br
.sp
\fBCONIC PROJECTIONS:\fP
.br
.sp
\fB\-Jb\fP\fIlon0/lat0/lat1/lat2/scale\fP (Albers)
.br
\fB\-Jl\fP\fIlon0/lat0/lat1/lat2/scale\fP (Lambert)
.br
.sp
\fBMISCELLANEOUS PROJECTIONS:\fP
.br
.sp
\fB\-Jh\fP\fIlon0/scale\fP (Hammer)
.br
\fB\-Ji\fP\fIlon0/scale\fP (Sinusoidal)
.br
\fB\-Jk\fP\fIlon0/scale\fP (Eckert VI)
.br
\fB\-Jn\fP\fIlon0/scale\fP (Robinson)
.br
\fB\-Jr\fP\fIlon0/scale\fP (Winkel Tripel)
.br
\fB\-Jw\fP\fIlon0/scale\fP (Mollweide)
.br
.sp
\fBNON-GEOGRAPHICAL PROJECTIONS:\fP
.br
.sp
\fB\-Jp\fP\fIscale\fP (Linear projection for polar (theta,r) coordinates)
.br
\fB\-Jx\fP\fIx-scale\fP[\fBl|p\fP\fIpow\fP][\fI/y-scale\fP[\fBl|p\fP\fIpow\fP]] (Linear, log, and power scaling)
.br
More details can be found in the \fBpsbasemap\fP manpages.
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
.B \-M
A series of "miscellaneous" options are provided which are
given as \fB-M\fP followed by a two character identifier, followed
by any other parameters associated with that option.
The \fB-M\fP options may be strung together separated by
colons, e.g. "-MGQ100:GU:CA200/10", which is equivalent to
"-MGQ -MGU -MCA200/10". 
.TP
.B \-MGD
\fIgmtdef/value\fP
.br
Allows the user to set the \fBGMT\fP default values used as
the plot is constructed. This command may be given repeatedly
to set as many \fBGMT\fP defaults as required. For example, to
set the basemap annotation font to Courier, use 
"-MGDANOT_FONT/Courier".
.TP
.B \-MGT
\fIx/y/size/angle/font/just/text\fP
.br
Causes a text label to plotted on the map.
\fIsize\fP is text size in points, \fIangle\fP is 
measured in degrees counter-clockwise from horizontal, 
\fIfontno\fP sets the font type, \fIjustify\fP sets the alignment. 
If \fIfontno\fP starts with a leading hyphen, then 
the remainder of \fIfontno\fP is taken to be a 
textstring with the desired fontname.  See the 
\fBgmtdefaults\fP man page for names and numbers of available fonts 
(or run \fBpstext \-L\fP).  The alignment 
number refers to the part of the textstring that will be mapped 
onto the (\fIx,y\fP) point: 1 = Lower Left corner, 
2 = Lower Center, 3 = Lower Right, 5 = Mid Left, 6 = Mid Center, 
7 = Mid Right, 9 = Upper Left, 10 = Upper Center, 
11 = Upper Right. This option may be given as many times as
needed.
.TP
.B \-MGU
\fP[\fI/dx/dy/\fP][\fIlabel\fP]
.br
Draw Unix System time stamp on plot.  User may specify where the lower left corner
of the stamp should fall on the page relative to lower left corner of plot in inch [Default is (-0.75,-0.75)].  Optionally,
append a label, or \fBc\fP (which will plot the command string.)
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
\fIwest, east, south,\fP and \fInorth\fP specify the Region of interest.  To specify boundaries
in degrees and minutes [and seconds], use the dd:mm[:ss] format.  Append \fBr\fP if lower left and upper right
map coordinates are given instead of wesn.
You may ask for a larger \fIw/e/s/n\fP region to have more room between the image and the axes.
A smaller region than specified in the grdfile will result in a subset of the grid [Default is
region given by the grdfile].
.TP
.B \-S
\fIsymbol/size\fP 
.br
Selects symbol to be used for plotting the next xy data
file. Setting \fIsymbol\fP = "N" causes line plotting.
Choose between:
.TP
.B \-Sa
st\fBa\fPr.  \fIsize\fP is radius of circumscribing circle.
.TP
.B \-Sb
\fBb\fPar extending from \fIbase\fP to y.  \fIsize\fP is bar width.  By default,
\fIbase\fP = 0.  Append /\fIbase\fP to change this value.  Append \fBu\fP if \fIsize\fP
is in x-units [Default is inch].
.TP
.B \-Sc
\fBc\fPircle.  \fIsize\fP is diameter of circle.
.TP
.B \-Sd
\fBd\fPiamond.  \fIsize\fP is side of diamond.
.TP
.B \-Se
\fBe\fPllipse.  Direction (in degrees counterclockwise from horizontal), major_axis (in inch), and minor_axis (in inch) must be found in columns 3, 4, and 5.
.TP
.B \-Sf
\fBf\fPault.  Give distance gap between ticks and ticklength in inch.  If gap is
negative, it is interpreted to mean number of ticks instead.  Append \fBl\fP or \fBr\fP to draw tick on
the left or right side of line [Default is centered].  Upper case \fBL\fP or \fBR\fP draws a triangle
instead of line segment.
.TP
.B \-Sh
\fBh\fPexagon.  Give side in inch.
.TP
.B \-Si
\fBi\fPnverted triangle.  Give side in inch.
.TP
.B \-Sl
\fBl\fPetter or text string.  Give size in inch, and append /\fIstring\fP after the size.  Note that the size is only approximate; no individual scaling
is done for different characters.  Remember to escape special characters like *.
.TP
.B \-Sp
\fBp\fPoint.  No size needs to be specified (1 pixel is used).
.TP
.B \-Ss
\fBs\fPquare.  Give side in inch.
.TP
.B \-St
\fBt\fPriangle.  Give side in inch.
.TP
.B \-Sv
\fBv\fPector.  Direction (in degrees counterclockwise from horizontal) and length (in inch) must be found in columns 3 and 4.  \fIsize\fP,
if present, will be interpreted as arrowwidth/headlength/headwidth (in inch) [Default is 0.03/0.12/0.1 inch].
By default arrow attributes remains invariant to the length
of the arrow.  To have the size of the vector scale down with decreasing size,
append n\fPnorm\fP, where vectors shorter than \fInorm\fP will have their
attributes scaled by length/\fInorm\fP.
.TP
.B \-SV
Same as \fB\-Sv\fP, except azimuth (in degrees east of north) should be given instead of direction.  The azimuth will
be mapped into an angle based on the chosen map projection (\fB\-Sv\fP leaves the directions
unchanged.)
.TP
.B \-Sx
cross.  Give length in inch.
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
Causes \fBmbm_xyplot\fP to operate in "verbose" mode 
so that it outputs
more information than usual.
.TP
.B \-W
\fIpen\fP
.br
Set pen attributes for xy plotting. See chapter 4.12 in the
GMT Technical reference for a discussion of GMT pen values.  
[Defaults: width = 1, color = 0/0/0, 
texture = solid]. 
.TP
.B \-X
Normally, \fBmbm_xyplot\fP creates an executable shellscript and
then exits.  This option will cause the shellscript to be executed
in the background before \fBmbm_xyplot\fP exits.

.SH EXAMPLES
Suppose we have obtained a swath sonar data file called
sb2112_example.mb41 collected using a SeaBeam 2112 sonar.
In order to obtain an xy plot of the center beam depth
versus time, we first extract the time-depth xy doubles
from the swath sonar file using \fBmblist\fP:
 
 	mblist -F41 -Isb2112_example.mb41 -OmZ > mz.dat
 
Here time is in seconds from the start of the file and the
depths are in meters, positive upward (topography rather
than bathymetry).
Now, we use \fBmbm_xyplot\fP to generate
shellscripts which in turn generate plots when executed. 
First, we generate a simple black line plot:
 
 	mbm_xyplot -Imz.dat -Omz 
 
The above command generates an executable shellscript
mz_line.cmd; executing this shellscript will generate a
Postscript plot and display it on the screen.
 
Suppose we also have a sparse set of depth estimates 
in a file called sr.dat obtained
by picking the seafloor on a seismic reflection record, and
we wish to compare the two sets of depths. We can plot the
swath sonar derived depths as a black line and the seismic
derived depths as red stars as follows:
 
 	mbm_xyplot -Omz_sr -Imz.dat \\
 		-IG255/0/0:Sa/0.1:sr.dat
 
As an example, the contents of the plotting shellscript 
"mz.cmd" are:
 
 #
 # Shellscript to create Postscript plot of data in grd file
 # Created by macro mbm_xyplot
 #
 # This shellscript created by following command line:
 # mbm_xyplot -Imz.dat -Omz
 #
 # Save existing GMT defaults
 echo Saving GMT defaults...
 gmtdefaults -L > gmtdefaults$$
 #
 # Set new GMT defaults
 echo Setting new GMT defaults...
 gmtset ANOT_FONT Helvetica
 gmtset LABEL_FONT Helvetica
 gmtset HEADER_FONT Helvetica
 gmtset ANOT_FONT_SIZE 8
 gmtset LABEL_FONT_SIZE 8
 gmtset HEADER_FONT_SIZE 10
 gmtset FRAME_WIDTH 0.074999999999999997
 gmtset TICK_LENGTH 0.074999999999999997
 gmtset PAGE_ORIENTATION LANDSCAPE
 gmtset COLOR_BACKGROUND 0/0/0
 gmtset COLOR_FOREGROUND 255/255/255
 gmtset COLOR_NAN 255/255/255
 #
 # Make xy data plot
 echo Running psxy...
 psxy mz.dat \\
 	-Jx0.0011071486125582637/0.0062732342007434947 \\
 	-R0/8128.99/-4382/-3306 \\
 	-X1 -Y0.5 -K -V > mz.ps
 #
# Make basemap
 echo Running psbasemap...
 psbasemap -Jx0.0011071486125582637/0.0062732342007434947 \\
 	-R0/8128.99/-4382/-3306 \\
 	-B500/100:."Data File mz.dat": \\
 	-O -V >> mz.ps
 #
 # Delete surplus files
 echo Deleting surplus files...
 rm -f 
 #
 # Reset GMT default fonts
 echo Resetting GMT fonts...
 mv gmtdefaults$$ .gmtdefaults
 #
 # Run xpsview
 echo Running xpsview in background...
 xpsview -ps a -or landscape -maxp 4m mz.ps &
 #
 # All done!
 echo All done!

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbcontour\fP(l), 
\fBmbswath\fP(l), \fBmbdefaults\fP(l), 
\fBmbm_grdplot\fP(l), \fBmbm_grd3dplot\fP(l),
\fBmbm_plot\fP(l)

.SH BUGS
Please let us know.




