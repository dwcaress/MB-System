.TH mbm_plot l "31 October 1994"
.SH NAME
\fBmbm_plot\fP - Create an executable shellscript which will generate
a GMT plot of multibeam swath data.

.SH VERSION
Version 4.1

.SH SYNOPSIS
\fBmbm_plot\fP \fB-F\fIformat \fB-I\fIfile \fP[\fB-O\fIroot \fB-R\fIw/e/s/n \fB-Gmode -C -N -A\fImagnitude/azimuth\fB -X -V -H\fP]

.SH DESCRIPTION
\fBmbm_plot\fP is an utility which generates a shellscript for making
a Postscript plot of the multibeam data contained in the specified file.  
A combination of MB-system and GMT-system utilities are used.
The plot will be scaled to fit on an 8.5 X 11 \ninch page.  
The plot may include bathymetry color fill (-G1), 
bathymetry color shaded relief (-G2), bathymetry shaded with amplitudes
(-G3), greyshade fill amplitude (-G4), or greyshade fill sidescan (-G5).
The plot may also include four color contoured bathymetry (-C).  
An annotated navigation track may also be plotted (-N).
The shellscript first creates the image and then displays it using 
pageview. The -X option will cause the shellscript to be executed 
immediately.  Once the basic plotting capability exists, the user
can edit the shellscript to obtain precisely the sort of plot desired.
This macro makes plotting a bit easier by providing a working template
so that the user does not always have to start from scratch.

.SH AUTHORSHIP
David W. Caress (caress@lamont.ldeo.columbia.edu)
.br
Dale N. Chayes (dale@lamont.ldeo.columbia.edu)
.br
Suzanne H. O'Hara (sohara@lamont.ldeo.columbia.edu)
.br
Lamont-Doherty Earth Observatory
.br
Palisades, NY 10964

.SH OPTIONS
.TP
.B \fB-A\fImagnitude/azimuth\fP
.B \fB-A\fImagnitude/median\fP
Sets the parameters which control how \fBmbswath\fP generates
simulated illumination of bathymetry, which can be either
shaded relief bathymetry or bathymetry draped with amplitude data.
If \fImode\fP is set to 2 (shaded relief bathymetry) using the
\fB-G\fP option, then the value \fImagnitude\fP
is an effective vertical exageration which modulates the intensity of
the shading; typical values are in the 1-5 range.  The value \fIazimuth\fP
is the azimuth from which the bathymetry is illuminated.
If \fImode\fP is set to 3 (bathymetry shaded using amplitudes) using the
\fB-G\fP option, then the value \fImagnitude\fP
modulates the intensity of the shading; the value \fImedian\fP sets the
amplitude value which serves as the zero or neutral level.  
.TP
.B \fB-C\fP
Sets flag indicating that a contour map of the data is desired.
.TP
.B \fB-F\fIformat\fP
Sets the format for the input multibeam data using 
\fBMBIO\fP integer format identifiers. 
This macro uses the \fBMBIO\fP library and will read any multibeam
format supported by \fBMBIO\fP. A list of the multibeam data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page.
Default: \fIformat\fP = 24.
.TP
.B \fB-G\fImode\fP
Turns on color fill swath plot and sets the style of the plot.
 	\fImode\fP = 1:	Color fill of bathymetry data.
 	\fImode\fP = 2:	Color shaded relief bathymetry.
 	\fImode\fP = 3:	Bathymetry shaded using amplitude data.
 	\fImode\fP = 4:	Color fill of amplitude data.
 	\fImode\fP = 5:	Color fill of sidescan data.
.TP
.B \fB-I\fIfile\fP
Data file from which the input data will be read.
.TP
.B \fB-H\fP
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \fB-N\fP
Sets flag indicating that a navigation track plot of the data is desired.
.TP
.B \fB-O\fIroot\fP
Sets the root used to construct the filename of the output shellscript
(\fIroot\fP.cmd) and names of files created when the shellscript is 
run.  Normally the 
name of the input GRD file is used as the \fIroot\fP. 
.TP
.B \fB-P\fP
This option will cause the shellscript to include lines which execute
a program to display the Postscript image on the screen.  The program
to be used to display the Postscript is set using \fBmbdefaults\fP;
the default value can be overridden by setting the environment
variable $MB_PS_VIEWER.
.TP
.B \fB-R\fIwest/east/south/north\fP
Sets the longitude and latitude bounds within which multibeam 
data will be read. Normally the bounds are automatically chosen
to include all of the data in the input file. 
.TP
.B \fB-V\fP
Causes \fBmbm_plot\fP to operate in "verbose" mode so that it outputs
more information than usual.
.TP
.B \fB-X\fP
Normally, \fBmbm_plot\fP creates an executable shellscript and
then exits.  This option will cause the shellscript to be executed
before \fBmbm_plot\fP exits.

.SH EXAMPLES
Suppose one has obtained a Hydrosweep DS multibeam data file 
called hs_ew9302_157_bcen.mb24 in the
L-DEO binary in-house format (format id 24). In order to generate a 
color map of the bathymetry overlaid with
contours run:
.br
	mbm_plot -F24 -Ihs_ew9302_157_bcen.mb24 -G -C -X
.br
In order to generate a shaded relief color map of the topography run:
.br
	mbm_plot -F24 -Ihs_ew9302_157_bcen.mb24 -S -X
.br
In order to generate a contour map of the topography:
.br
	mbm_plot -F24 -Ihs_ew9302_157_bcen.mb24 -C -X

.SH BUGS
Uses only the standard mercator projection and is thus useless for
polar data.  Doesn't do a particularly good job of choosing tick
and contour intervals.



