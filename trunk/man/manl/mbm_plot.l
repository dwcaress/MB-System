.TH mbm_plot l "14 February 1995"
.SH NAME
\fBmbm_plot\fP - Create an executable shellscript which will generate
a GMT plot of multibeam swath data.

.SH VERSION
Version 4.2

.SH SYNOPSIS
\fBmbm_plot\fP \fB-F\fP\fIformat\fP \fB-I\fP\fIfile\fP
[\fB-A\fImagnitude/azimuth\fP \fB-C\fP \fB-G\fImode\fP 
\fB-N\fP \fB-O\fIroot\fP \fB-R\fIw/e/s/n\fP -S
 \fB-X -V -H\fP]

.SH DESCRIPTION
\fBmbm_plot\fP is an utility which generates a shellscript for making
a Postscript plot of the multibeam data contained in a single file
or a set of data files specified in a list.  
A combination of \fBMB-System\fP and \fBGMT-System\fP utilities are used.
The plot will be scaled to fit on an 8.5 X 11 inch page.  
The plot may include bathymetry color fill (\fB-G\fP\fI1\fP), 
bathymetry color shaded relief (\fB-G\fP\fI2\fP), bathymetry 
shaded with amplitudes
(\fB-G\fP\fI3\fP), greyshade fill amplitude (\fB-G\fP\fI4\fP), or 
greyshade fill sidescan (\fB-G\fP\fI5\fP).
The plot may also include four color contoured bathymetry (\fB-C\fP).  
An annotated navigation track may also be plotted (\fB-N\fP).
The colorscale or grayscale used for plotting can either be 
created using linear steps from the minimum to maximum data values
(the default) or using histogram equalization to enhance the
contrast of the plot (the \fB-S\fP option).
The shellscript first creates the image and then displays it using 
pageview. The \fB-X\fP option will cause the shellscript to be executed 
immediately.  Once the basic plotting capability exists, the user
can edit the shellscript to obtain precisely the sort of plot desired.
This macro makes plotting a bit easier by providing a working template
so that the user does not always have to start from scratch.
In particular, the shellscript created by running \fBmbm_plot\fP
on a single file is a good starting point for plotting a survey
comprised of many data files.

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
Sets the data format for the input data. 
If \fIformat\fP < 0, then the input file specified
with the \fB-I\fP option will actually contain a list of input multibeam
data files. This program uses the \fBMBIO\fP library 
and will read or write any multibeam
format supported by \fBMBIO\fP. A list of the multibeam data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page. Default: \fIformat\fP = -1.
.TP
.B \fB-G\fImode\fP
Turns on color fill swath plot and sets the style of the plot.
 	\fImode\fP = 1:	Color fill of bathymetry data.
 	\fImode\fP = 2:	Color shaded relief bathymetry.
 	\fImode\fP = 3:	Bathymetry shaded using amplitude data.
 	\fImode\fP = 4:	Grayscale fill of amplitude data.
 	\fImode\fP = 5:	Grayscale fill of sidescan data.
.TP
.B \fB-I\fIfilename\fP
Sets the input filename. If \fIformat\fP > 0 (set with the 
\fB-f\fP option) then the multibeam data contained in \fIinfile\fP 
is read and processed. If \fIformat\fP < 0 (the default), 
then \fIinfile\fP
is assumed to be an ascii file containing a list of the input multibeam
data files to be processed and their formats.  The program will read 
the data in each one of these files.
In the \fIinfile\fP file, each
data file should be followed by a data format identifier, e.g.:
 	datafile1 11
 	datafile2 24
.br
This program uses the \fBMBIO\fP library and will read or write any multibeam
format supported by \fBMBIO\fP. A list of the multibeam data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page.
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
.B \fB-S\fP
Causes the color or grayscale table to be created using a
histogram equalization. This is particularly useful for
enhancing the contrast of sidescan and beam amplitude plots. 
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
	mbm_plot -F24 -Ihs_ew9302_157_bcen.mb24 -G1 -C -P -X
.br
In order to generate a shaded relief color map of the topography run:
.br
	mbm_plot -F24 -Ihs_ew9302_157_bcen.mb24 -G2 -P -X
.br
In order to generate a contour map of the topography:
.br
	mbm_plot -F24 -Ihs_ew9302_157_bcen.mb24 -C -X
.br
Suppose that one a set of SeaBeam 2100 data files comprising
a short survey and that one wants a first cut plot of them
together. Create an ASCII text file which has a list of the
filenames each followed by the appropriate \fBMBIO\fP 
format id number, e.g.:
 	test1.mb41 41
 	test2.mb41 41
 	test3.mb41 41
.br
If the name of the data file list is "datalist", then the 
following will generate a shellscript for creating a color
fill plot of the bathymetry in the datafiles, then execute
that shellscript, and then put the plot on the screen:
 	mbm_plot -F-1 -Idatalist -G1 -P -X -Obathtest
.br
The name of the shellscript will be bathtest.cmd and the
Postscript plot will be found in the file bathtest.ps.

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbcontour\fP(l), 
\fBmbswath\fP(l), \fBmbdefaults\fP(l), \fBmbm_grdplot\fP(l)

.SH BUGS
Uses only the standard mercator projection and is thus useless for
polar data.



