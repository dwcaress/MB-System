.TH mbswath l "26 January 1995"
.SH NAME
\fBmbswath\fP - GMT compatible utility for color fill or color shaded relief
swath plots of multibeam data using Postscript.

.SH VERSION
Version 4.2

.SH SYNOPSIS
\fBmbswath\fP \fB-C\fIcptfile \fB-J\fIparameters 
\fB-R\fIwest/east/south/north \fP[\fB-A\fIfactor/mode/depth 
\fB-B\fItickinfo \fB-b\fIyr/mo/da/hr/mn/sc 
\fB-D\fImode/scale/min/max \fB-E\fIyr/mo/da/hr/mn/sc 
\fB-f\fIformat \fB-F\fIred/green/blue 
\fB-G\fImagnitude/azimuth \fB-I\fIdatalist \fB-K\fI 
\fB-M\fI \fB-O\fI \fB-P\fI \fB-p\fIpings \fB-Q\fIdpi \fB-S\fIspeed 
\fB-U\fI \fB-X\fIx-shift \fB-Y\fIy-shift \fB-Z\fImode 
\fB-#\fIcopies \fB-V -H\fP]

.SH DESCRIPTION
\fBmbswath\fP is an utility for plotting multibeam swath data in color fill
or color shaded relief using Postscript.  Like \fBmbcontour\fP, \fBmbswath\fP
is fully compatible with the GMT package version 2.1, including the use
of GMT style color pallete (cpt) files to control the color table. The program
can read data from stdin (the default) or from one or more multibeam files. 

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
.B \fB-A\fIfactor/mode/depth\fP
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
Default: \fIfactor\fP = 1.0, \fImode\fP = 1, \fIdepth\fP = 3000.0.
.TP
.B \fB-B\fItickinfo\fP
Sets map boundary tickmark intervals. See \fBpsbasemap\fP for details.
.TP
.B \fB-b\fIyr/mo/da/hr/mn/sc\fP
Sets the starting time for data allowed in the input data; pings
with times before the starting time will be ignored. 
Default: \fIyr/mo/da/hr/mn/sc\fP = 1962/2/21/10/30/0.
.TP
.B \fB-C\fP
Sets the color palette file which controls the color of the plot.
See documentation of the GMT package for a complete description
of cpt files.
.TP
.B \fB-D\fImode/scale/min/max\fP
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
.B \fB-E\fIyr/mo/da/hr/mn/sc\fP
Sets the ending time for data allowed in the input data; pings
with times after the ending time will be ignored. 
Default: \fIyr/mo/da/hr/mn/sc\fP = 2062/2/21/10/30/0.
.TP
.B \fB-F\fIred/green/blue\fP
Sets the color used for Frame and annotation. [Default is black]
.TP
.B \fB-f\fIformat\fP
Sets the data format used if the input is read from stdin
or from a file. If \fIformat\fP < 0, then the input file specified
with the \fB-I\fP option will actually contain a list of input multibeam
data files. This program uses the \fBMBIO\fP library 
and will read or write any multibeam
format supported by \fBMBIO\fP. A list of the multibeam data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page. Default: \fIformat\fP = -1.
.TP
.B \fB-G\fImagnitude/azimuth\fP
.B \fB-G\fImagnitude/median\fP
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
.B \fB-H\fP
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \fB-I\fIinfile\fP
Sets the input filename. If \fIformat\fP > 0 (set with the 
\fB-f\fP option) then the multibeam data contained in \fIinfile\fP 
is read and processed. If \fIformat\fP < 0, then \fIinfile\fP
is assumed to be an ascii file containing a list of the input multibeam
data files to be processed and their formats.  The program will read 
and plot the data in each one of these files.
In the \fIinfile\fP file, each
data file should be followed by a data format identifier, e.g.:
 	datafile1 11
 	datafile2 24
.br
This program uses the \fBMBIO\fP library and will read or write any multibeam
format supported by \fBMBIO\fP. A list of the multibeam data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page. Default: \fIinfile\fP = "stdin".
.TP
.B \fB-J\fP
Selects the map projection.  Choose one of the following:
.br
\fB\-Ja\fP\fIlon0/lat0/scale[r]\fP (Lambert Azimuthal Equal-Area).
.br
\fB\-Jb\fP\fIlon0/lat0/lat1/lat2/scale[r]\fP (Albers Conic Equal-Area).
.br
\fB\-Jc\fP\fIlon0/lat0/scale[r]\fP (Cassini).
.br
\fB\-Je\fP\fIlon0/lat0/scale[r]\fP (Azimuthal Equidistant).
.br
\fB\-Jg\fP\fIlon0/lat0/scale[r]\fP (Orthographic).
.br
\fB\-Jh\fP\fIlongitude/radius\fP (Hammer Equal-Area)
.br
\fB\-Ji\fP\fIwidth\fP (Interrupted Sinusoidal)
.br
\fB\-Jl\fP\fIlon0/lat0/lat1/lat2/scale\fP (Lambert Conic Conformal)
.br
\fB\-Jm\fP\fIscale\fP (Mercator)
.br
\fB\-Joa\fP\fIorigin_x/origin_y/azimuth/scale[r]\fP (Oblique Mercator - point and azimuth)
.br
\fB\-Job\fP\fIorigin_x/origin_y/point_x/point_y/scale[r]\fP (Oblique Mercator - two points)
.br
\fB\-Joc\fP\fIorigin_x/origin_y/pole_x/pole_y/scale[r]\fP (Oblique Mercator - point and pole)
.br
\fB\-Jr\fP\fIlongitude/radius\fP (Winkel Tripel)
.br
\fB\-Js\fP\fIlon0/lat0/scale[r]\fP (Stereographic)
.br
\fB\-Jt\fP\fIlongitude/scale\fP (TM - Transverse Mercator)
.br
\fB\-Ju\fP\fIzone/scale\fP (UTM - Universal Transverse Mercator)
.br
\fB\-Jw\fP\fIlongitude/radius\fP (Mollweide Equal-Area)
.br
\fB\-Jp\fP\fIradius\fP (Linear scaling of polar (theta,r) coordinates)
.br
\fB\-Jx\fP\fIxscale[/yscale]\fP (Linear scaling of cartesian coordinates)
.br
More details can be found in the \fBpsbasemap\fP man pages.
.TP
.B \-K
More \fIPostScript\fP code will be appended later [Default terminates the plot system].
.TP
.B \fB-L\fIlonflip\fP
Sets the range of the longitude values returned by the multibeam i/o routines.
If \fIlonflip\fP=-1 then the longitude values will be in
the range from -360 to 0 degrees. If \fIlonflip\fP=0 
then the longitude values will be in
the range from -180 to 180 degrees. If \fIlonflip\fP=1 
then the longitude values will be in
the range from 0 to 360 degrees.
Default: \fIlonflip\fP = 0.
.TP
.B \-M
Force conversion to monochrome image using the [television] YIQ transformation.
.TP
.B \-O
Selects Overlay plot mode [Default initializes a new plot system].
.TP
.B \-P
Selects Portrait plotting mode [GMT Default is Landscape, see gmtdefaults to change this].
.TP
.B \fB-p\fIpings\fP
Sets the ping averaging of the input data. If \fIpings\fP = 1, then
no ping averaging is performed. If \fIpings\fP > 0, then
that number of input pings will be averaged to produce one output
ping.  If \fIpings\fP = 0, then the ping averaging will automatically
be done so that the along-track ping spacing is equal to the across-track
beam spacing. 
Default: \fIpings\fP = 1 (no ping averaging).
.TP
.B \fB-Q\fIdpi\fP
Causes \fBmbswath\fP to generate the plot as an image rather than as
a series of polygon color fills (the default). The resolution of the 
image is set by \fIdpi\fP in pixels per inch.
.TP
.B \fB-R\fIwest/east/south/north\fP
Sets the longitude and latitude bounds within which multibeam 
data will be read and plotted. Only the data which lies within 
these bounds will be read. 
Default: \fIwest\fP=-360, east\fI=360\fP, \fIsouth\fP=-90, \fInorth\fP=90.
.TP
.B \fB-S\fIspeed\fP
Sets the minimum speed in km/hr (5.5 kts ~ 10 km/hr) allowed in 
the input data; pings associated with a smaller ship speed will not be
processed. Default: \fIspeed\fP = 0.
.TP
.B \fB-T\fItimegap\fP
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
.B \-X \-Y
Shift origin of plot by (\fIx-shift,y-shift\fP) inches  [Default is (1,1) for new plots, (0,0) for overlays].
.TP
.B \fB-Z\fImode\fP
Sets the style of the plot.
 	\fImode\fP = 1:	Color fill of bathymetry data.
 	\fImode\fP = 2:	Color shaded relief bathymetry.
 	\fImode\fP = 3:	Bathymetry shaded using amplitude data.
 	\fImode\fP = 4:	Color fill of amplitude data.
 	\fImode\fP = 5:	Color fill of sidescan data.
.br
Default: \fImode\fP = 1;
.TP
.B \-#
Specifies the number of plot copies. [Default is 1]
.TP
.B \-0
Create the image using the Adobe \fIPostScript\fP colorimage operator.  This is the
GMT-SYSTEM default (which can be modified in the .gmtdefaults file).
.TP
.B \-1
Create the image using the old Sun NeWS 1.0 image operator extension.
.TP
.B \-2
Create the image by plotting individually colored tiles.
.TP
.B \-3
Create the image by color separation using three calls to image for later
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
 # Shellscript to create Postscript plot of multibeam data
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
\fBmbsystem\fP(l), \fBmbm_plot\fP(l), \fBmbcontour\fP(l)

.SH BUGS
Yes.
