.TH mbtide l "10 July 1995"
.SH NAME
\fBmbtide\fP - MBTIDE corrects multibeam bathymetry data for tides.

.SH VERSION
Version 4.2

.SH SYNOPSIS
\fBmbtide\fP [\fB-F\fIformat \fB-I\fIinfile \fB-O\fIoutfile 
\fB-M\fItideformat\fP \fB-T\fItidefile\fP \fB-V -H\fP]

.SH DESCRIPTION
\fBmbtide\fP is an utility for correcting swath bathymetry for
tides.  The input tide data can be in one of several ascii table
formats. A simple spline interpolation of the tide values is
used to calculate the tide correction for each ping. The tide
values are interpreted in meters above zero tide, where zero
tide is also referred to as the "chart datum" on hydrographic
charts. The effect of the tide correction is to make the
bathymetry values correspond to the "chart datum" reference.
The default input and output streams are stdin and stdout.

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
.B \fB-F\fIformat\fP
Sets the format for the input and output multibeam data using 
\fBMBIO\fP integer format identifiers. 
This program uses the \fBMBIO\fP library and will read or write any multibeam
format supported by \fBMBIO\fP. A list of the multibeam data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page.
Default: \fIformat\fP = 11.
.TP
.B \fB-H\fP
This "help" flag causes the program to print out a description
of its operation and then exit immediately.
.TP
.B \fB-I\fIinfile\fP
Data file from which the input data will be read. If
no input file is specified, the input will be read
from stdin. Default: \fIinfile\fP = stdin.
.TP
.B \fB-M\fItideformat\fP
Defines the format of the tide data to be read.  Currently,
four formats are allowed, all consisting of ascii records:

\fItideformat\fP = 1:  
.br
This format consists for lines of the form:
 	time_d tide
.br
where time_d is decimal unix time in seconds since
1/1/1970 00:00:00.000 and tide values are in meters
above zero tide.
Some representative lines are:
 804164520.0	1.25
 804164940.0	2.25
 804165660.0	3.25
 804166800.0	4.00

\fItideformat\fP = 2:  
.br
This format consists for lines of the form:
 	year mon day hour min sec tide
.br
where tide is in meters above zero tide.
Some representative lines are:
 1995 06 26 11 04 00 2.91
 1995 06 26 11 11 00 2.98
 1995 06 26 11 23 00 3.21
 1995 06 26 11 42 00 3.59

\fItideformat\fP = 3:  
.br
This format consists for lines of the form:
 	year jday hour min sec lon lat
.br
where jday is the julian day of the year and 
lon and lat are in decimal degrees.
Some representative lines are:
 1995 177 11 04 00 2.91
 1995 177 11 11 00 2.98
 1995 177 11 23 00 3.21
 1995 177 11 42 00 3.59

\fItideformat\fP = 4:  
.br
This format consists for lines of the form:
 	year jday daymin sec lon lat
.br
where jday is the julian day of the year, daymin is the minute
of the day, and lon and lat are in decimal degrees.
Some representative lines are:
 1995 177 664 00 2.91
 1995 177 671 00 2.98
 1995 177 683 00 3.21
 1995 177 702 00 3.59

Default: \fItideformat\fP = 2.
.TP
.B \fB-O\fIoutfile\fP
Data file to which the output data will be written. If
no output file is specified, the output will be written to
stdout. Default: \fIoutfile\fP = stdout.
.TP
.B \fB-N\fItidefile\fP
Data file from which the tide data will be read.
.TP
.B \fB-V\fP
Normally, \fBmbtide\fP works "silently" without outputting
anything to the stderr stream.  If the
\fB-V\fP flag is given, then \fBmbtide\fP works in a "verbose" mode and
outputs the program version being used and all error status messages.

.SH EXAMPLES
Suppose one wishes to correct the bathymetry in the Reson SeaBat 9002
data file ref2001.mb81 (format 81) for the tide variations 
contained in the file tide_950626.dat. The following will
suffice:
 	mbtide -F81 -M2 -Ttide_950626.dat 
 		-Iref2001.mb81 -Oref2001_t.mb81

.SH BUGS
Bathymetry data collected in intertidal zones can end up
negative (above zero tide or chart datum) after correction
for tides. This conflicts with the MB-System convention of
flagging bad data with negative depth values. There is no
graceful way to handle this problem at present (other than
to introduce a static shift in the tide values, which is
hardly graceful).
Please let us know if you find any other problems.
