.TH mbswath l "3 October 1998" "MB-System 4.6" "MB-System 4.6"
.SH NAME
\fBmbswath\fP - GMT compatible utility for color fill or color shaded relief
swath plots of swath sonar data using Postscript.

.SH VERSION
Version 4.5

.SH SYNOPSIS
\fBmbswath\fP \fB-C\fIcptfile\fP \fB-J\fIparameters\fP 
\fB-R\fIwest/east/south/north\fP [\fB-A\fIfactor/mode/depth\fP 
\fB-B\fItickinfo\fP \fB-b\fIyr/mo/da/hr/mn/sc\fP 
\fB-c\fIcopies\fP \fB-D\fImode/scale/min/max\fP 
\fB-E\fIyr/mo/da/hr/mn/sc\fP 
\fB-f\fIformat\fP \fB-F\fIred/green/blue\fP 
\fB-G\fImagnitude/azimuth\fP \fB-I\fIdatalist\fP \fB-K\fP 
\fB-N\fIcptfile\fP \fB-O\fP \fB-P\fP \fB-p\fIpings \fB-Q\fIdpi\fP \fB-S\fIspeed\fP 
\fB-T\fItimegap\fP \fB-U\fP \fB-W\fP \fB-X\fIx-shift\fP \fB-Y\fIy-shift\fP \fB-Z\fImode\fP 
\fB-0 -1 -2\fP
\fB-V -H\fP]

.SH DESCRIPTION
\fBmbswath\fP is an utility for plotting swath sonar data in color fill
or color shaded relief using Postscript.  Like \fBmbcontour\fP, \fBmbswath\fP
is fully compatible with the GMT package version 3, including the use
of GMT style color pallete (cpt) files to control the color table. 
A 24-bit true color \fIPostScript\fP file is output using one of 
three methods. Unless individual polygon fills are used (\fB-1\fP option),
only one call to \fBmbswath\fP can be made for each plot because the
color image produced covers the entire plot space.
The program can read data from stdin (the default) or from one 
or more swath sonar files.

Before opening an input swath data file, \fBmbmbswath\fP checks for
an ascii file in the same directory having the same name except
that ".inf" is appended to the end. The program assumes that this
ascii file contains the output of the program \fBmbinfo\fP run on
the input data file. If the ".inf" file exists, \fBmbswath\fP reads
the minimum and maximum longitude and latitude bounds from the 
\fBmbinfo\fP output and compares those to the bounds
for the plot. If the ".inf" file indicates that none of the data
in the input file lies inside the plot bounds, that input
file is skipped. This allows users to maintain a single master list
of data files for use in all plotting without the performance penalty
of \fBmbswath\fP reading through all the data files, even those
with no relevent data. We recommend that users maintain a ".inf"
file for each swath data file used for gridding or plotting. The
programs \fBmbcontour\fP and \fBmbgrid\fP also use ".inf" files
in the same fashion.

.SH AUTHORSHIP
David W. Caress (caress@lamont.ldeo.columbia.edu)
.br
Dale N. Chayes (dale@lamont.ldeo.columbia.edu)
.br
Lamont-Doherty Earth Observatory
.br
Palisades, NY 10964

.SH OPTIONS
.TP
.B \-A
\fIfactor/mode/depth\fP
.br
This option determines how the along-track dimension of the
beam or pixel footprints is calculated. If \fImode\fP = 1,
then the fore-aft beam angle width of the sonar is used so that
the width increases towards the outer parts of the swath.
The fore-aft beam angle width (\fBMB-System\fP internally stores
a value for each format/sonar) is multiplied by the \fIfactor\fP
value; a \fIfactor\fP < 1.0 can be useful if the data highly
oversamples the seafloor and a \fIfactor\fP > 1.0 can fill in
plots of data which undersample the seafloor. If the data
stream does not include depth values (e.g. one is plotting
pure sidescan data), then the \fIdepth\fP value sets the
depth value in meters used in the footprint calculations.
If \fImode\fP = 2, then the along-track dimension of the beam
or pixel footprints is just the along-track distance between
pings multiplied by the \fIfactor\fP value.
If \fImode\fP = 3, then each data point is plotted as a point,
and the \fIfactor\fP and \fIdepth\fP parameters are ignored.
Default: \fIfactor\fP = 1.0, \fImode\fP = 1, \fIdepth\fP = 3000.0.
.TP
.B \-B
\fItickinfo\fP
.br
Sets map boundary tickmark intervals. See the \fBpsbasemap\fP
manual page for details.
.TP
.B \-b
\fIyr/mo/da/hr/mn/sc\fP
.br
Sets the starting time for data allowed in the input data; pings
with times before the starting time will be ignored. 
Default: \fIyr/mo/da/hr/mn/sc\fP = 1962/2/21/10/30/0.
.TP
.B \-C
Sets the color palette file which controls the color of the plot.
See documentation of the GMT package for a complete description
of cpt files.
.TP
.B \-c
\fIcopies\fP
.br
Specifies the number of plot copies (the default is 1).
.TP
.B \-D
\fImode/scale/min/max\fP
.br
Sets scaling of beam amplitude or sidescan pixel values which
can be applied before plotting. If \fImode\fP = 1 or 2, then
a linear scaling of the form:
 	scaled_value = scale * (value - min) / (max - min)
.br
is applied.  If \fImode\fP = 3 or 4, then a log10 scaling of
the form:
 	scaled_value = scale * (20 * log10(value) - min) / (max - min)
.br
is applied.  If \fImode\fP = 2 or 4, then the value (or 20*log10(value))
will be clipped to \fImin\fP if it is smaller than \fImin\fP or \fImax\fP
if it is greater than \fImax\fP; this clipping happens prior to the
multiplication by \fIscale\fP. Default: \fImode\fP = 1, \fIscale\fP = 1.0,
\fImin\fP = 0.0, \fImax\fP = 1.0.
.TP
.B \-E
\fIyr/mo/da/hr/mn/sc\fP
.br
Sets the ending time for data allowed in the input data; pings
with times after the ending time will be ignored. 
Default: \fIyr/mo/da/hr/mn/sc\fP = 2062/2/21/10/30/0.
.TP
.B \-F
\fIred/green/blue\fP
.br
Sets the color used for Frame and annotation. [Default is black]
.TP
.B \-f
\fIformat\fP
.br
Sets the data format used if the input is read from stdin
or from a file. If \fIformat\fP < 0, then the input file specified
with the \fB-I\fP option will actually contain a list of input swath sonar
data files. This program uses the \fBMBIO\fP library 
and will read or write any swath sonar
format supported by \fBMBIO\fP. A list of the swath sonar data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page. Default: \fIformat\fP = -1.
.TP
.B \-G
\fImagnitude/azimuth\fP or \fImagnitude/median\fP
.br
Sets the parameters which control how \fBmbswath\fP generates
simulated illumination of bathymetry, which can be either
shaded relief bathymetry or bathymetry draped with amplitude data.
If \fImode\fP is set to 2 (shaded relief bathymetry) using the
\fB-Z\fP option, then the value \fImagnitude\fP
is an effective vertical exageration which modulates the intensity of
the shading; typical values are in the 1-5 range.  The value \fIazimuth\fP
is the azimuth from which the bathymetry is illuminated.
If \fImode\fP is set to 3 (bathymetry shaded using amplitudes) using the
\fB-Z\fP option, then the value \fImagnitude\fP
modulates the intensity of the shading; the value \fImedian\fP sets the
amplitude value which serves as the zero or neutral level.  
.TP
.B \-H
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \-I
\fIinfile\fP
.br
Sets the input filename. If \fIformat\fP > 0 (set with the 
\fB-f\fP option) then the swath sonar data contained in \fIinfile\fP 
is read and processed. If \fIformat\fP < 0, then \fIinfile\fP
is assumed to be an ascii file containing a list of the input swath sonar
data files to be processed and their formats.  The program will read 
and plot the data in each one of these files.
In the \fIinfile\fP file, each
data file should be followed by a data format identifier, e.g.:
 	datafile1 11
 	datafile2 24
.br
This program uses the \fBMBIO\fP library and will read or write any swath sonar
format supported by \fBMBIO\fP. A list of the swath sonar data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page. Default: \fIinfile\fP = "stdin".
.TP
.B \-J
Selects the map projection. Scale is inch/degree, 1:xxxxx. 
or width in inches (upper case modifier).
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
.B \-K
More \fIPostScript\fP code will be appended later [Default terminates the plot system].
.TP
.B \-L
\fIlonflip\fP
.br
Sets the range of the longitude values returned by the swath sonar i/o routines.
If \fIlonflip\fP=-1 then the longitude values will be in
the range from -360 to 0 degrees. If \fIlonflip\fP=0 
then the longitude values will be in
the range from -180 to 180 degrees. If \fIlonflip\fP=1 
then the longitude values will be in
the range from 0 to 360 degrees.
Default: \fIlonflip\fP = 0.
.TP
.B \-N
\fIcptfile\fP
.br
Normally, shading of bathymetry with amplitudes (\fImode\fP = 3
as set with the \fB-Z\fP option) is accomplished
by linearly mapping the amplitudes to shade values. This option
specifies a grayscale cpt file used to map amplitude values
to grayscale shade values. 
.TP
.B \-O
Selects Overlay plot mode [Default initializes a new plot system].
.TP
.B \-P
Selects Portrait plotting mode [GMT Default is Landscape, see gmtdefaults to change this].
.TP
.B \-p
\fIpings\fP
.br
Sets the ping averaging of the input data. If \fIpings\fP = 1, then
no ping averaging is performed. If \fIpings\fP > 0, then
that number of input pings will be averaged to produce one output
ping.  If \fIpings\fP = 0, then the ping averaging will automatically
be done so that the along-track ping spacing is equal to the across-track
beam spacing. 
Default: \fIpings\fP = 1 (no ping averaging).
.TP
.B \-Q
\fIdpi\fP
.br
Sets the resolution of the color image output by \fBmbswath\fP
in pixels per inch (except when the \fB-1\fP option is used to force individual
polygon color fills). Default: \fIdpi\fP = 100.
.TP
.B \-R
\fIwest/east/south/north\fP
.br
Sets the longitude and latitude bounds within which swath sonar 
data will be read and plotted. Only the data which lies within 
these bounds will be read. 
Default: \fIwest\fP=-360, east\fI=360\fP, \fIsouth\fP=-90, \fInorth\fP=90.
.TP
.B \-S
\fIspeed\fP
.br
Sets the minimum speed in km/hr (5.5 kts ~ 10 km/hr) allowed in 
the input data; pings associated with a smaller ship speed will not be
processed. Default: \fIspeed\fP = 0.
.TP
.B \-T
\fItimegap\fP
.br
Sets the maximum time gap in minutes between adjacent pings allowed before
the data is considered to have a gap. Default: \fItimegap\fP = 1.
.TP
.B \-U
Draw Unix System time stamp on plot.  Optionally, append a label, or 'c' which will plot
the command string.
.TP
.B \-V
Selects verbose mode, which will send progress reports to stderr [Default runs "silently"].
.TP
.B \-W
Normally, \fBmbswath\fP works with bathymetry values in meters.  If the
\fB-W\fP flag is given, then \fBmbswath\fP will work with bathymetry
values in feet. The color pallette file used must conform to the
range of bathymetry values in feet.
.TP
.B \-X \-Y
Shift origin of plot by (\fIx-shift,y-shift\fP) inches  [Default is (1,1) for new plots, (0,0) for overlays].
.TP
.B \-Z
\fImode\fP
.br
Sets the style of the plot.
 	\fImode\fP = 1:	Color fill of bathymetry data.
 	\fImode\fP = 2:	Color shaded relief bathymetry.
 	\fImode\fP = 3:	Bathymetry shaded using amplitude data.
 	\fImode\fP = 4:	Color fill of amplitude data.
 	\fImode\fP = 5:	Color fill of sidescan data.
.br
Default: \fImode\fP = 1;
.TP
.B \-0
Create the image using the Adobe \fIPostScript\fP colorimage operator.  
This is the default.
.TP
.B \-1
Create the image by plotting individual color polygons.
.TP
.B \-2
Create the image by color separation using three calls to image for later
processing by \fBpsto24\fP.
.SH EXAMPLES
Suppose the user has a Hydrosweep data file in the L-DEO in-house
binary format (\fBMBIO\fP format id 24) called hs_ew9302_157_mn.mb24
which lies in the region w/s/e/n = -32.1874/-26.6236/54.6349/56.7536.
The following will suffice to generate a color fill plot:
 	mbswath -Idatalist -Jm1.46578 -R-32.1874/-26.6236/54.6349/56.7536 
 		-B1/1 -Ccolor.cpt -p1 -A1 -Q100 -Z2 
 		-X1 -Y1.75 -V > mbswath.ps
.br
where the file datalist contains:
 	hs_ew9302_157_mn.mb24 24
.br
A more complicated plot including a navigation track and a color scale
can be created using \fBmbswath\fP in conjunction with other GMT
and GMT-compatible utilities.  The following is an example of a shellscript
which generates such a plot and then displays it on the screen (assuming
you have a Sun workstation):

 #
 # Shellscript to create Postscript plot of swath sonar data
 # Created by macro mbm_plot
 #
 # Make datalist file 
 echo Making datalist file...
 echo hs_ew9302_157_bmn.mb24 24 > datalist
 #
 # Make color pallette table file
 echo Making color pallette table file...
 echo 2235 255 255 255 2372 255 221 171 > hs_ew9302_157_bmn.mb24.cpt
 echo 2372 255 221 171 2509 255 186 133 >> hs_ew9302_157_bmn.mb24.cpt
 echo 2509 255 186 133 2646 255 161 68 >> hs_ew9302_157_bmn.mb24.cpt
 echo 2646 255 161 68 2783 255 189 87 >> hs_ew9302_157_bmn.mb24.cpt
 echo 2783 255 189 87 2920 240 236 121 >> hs_ew9302_157_bmn.mb24.cpt
 echo 2920 240 236 121 3057 205 255 162 >> hs_ew9302_157_bmn.mb24.cpt
 echo 3057 205 255 162 3193 138 236 174 >> hs_ew9302_157_bmn.mb24.cpt
 echo 3193 138 236 174 3330 106 235 255 >> hs_ew9302_157_bmn.mb24.cpt
 echo 3330 106 235 255 3467 87 215 255 >> hs_ew9302_157_bmn.mb24.cpt
 echo 3467 87 215 255 3604 50 190 255 >> hs_ew9302_157_bmn.mb24.cpt
 echo 3604 50 190 255 3741 0 160 255 >> hs_ew9302_157_bmn.mb24.cpt
 echo 3741 0 160 255 3878 40 127 251 >> hs_ew9302_157_bmn.mb24.cpt
 echo 3878 40 127 251 4015 21 92 236 >> hs_ew9302_157_bmn.mb24.cpt
 echo 4015 21 92 236 4152 37 57 175 >> hs_ew9302_157_bmn.mb24.cpt
 #
 # Run mbswath
 echo Running mbswath...
 mbswath -Idatalist -Jm1.46578 -R-32.1874/-26.6236/54.6349/56.7536 \
 	-B1.1128/1.1128":.Data File hs_ew9302_157_bmn.mb24:" \
 	-Chs_ew9302_157_bmn.mb24.cpt -p1 -A1 -Q100 -G5/0 -Z2 -X1 \
 	-Y1.75 -K -V > hs_ew9302_157_bmn.mb24.ps
 #
 # Run mblist
 echo Running mblist...
 mblist -F24 -Ihs_ew9302_157_bmn.mb24 -OXYU > hs_ew9302_157_bmn.mb24.nav
 #
 # Run pstrack
 echo Running pstrack...
 pstrack hs_ew9302_157_bmn.mb24.nav -Jm1.46578 \
 	-R-32.1874/-26.6236/54.6349/56.7536 -B1.1128/1.1128":.Data File \
 	hs_ew9302_157_bmn.mb24:" -W1p -Mt15ma1h -O -K >> hs_ew9302_157_bmn.mb24.ps
 #
 # Run psscale
 echo Running psscale...
 psscale  -Chs_ew9302_157_bmn.mb24.cpt -D4.0777/-0.5000/6.5242/0.1500h 
 	-B":.Depth (meters):" -O -V >> hs_ew9302_157_bmn.mb24.ps
 #
 # Delete surplus files
 echo Deleting surplus files...
 rm -f hs_ew9302_157_bmn.mb24.cpt datalist hs_ew9302_157_bmn.mb24.nav
 #
 # Run pageview
 echo Running pageview in background...
 pageview hs_ew9302_157_bmn.mb24.ps &
 #
 # All done!
 echo All done!

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbm_plot\fP(l), \fBmbcontour\fP(l), 
\fBgmtsystem\fP(l), \fBpsbasemap\fP(l), \fBpsto24\fP(l)

.SH BUGS
Let us know.
