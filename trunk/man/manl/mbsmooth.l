.TH mbsmooth l "31 October 1994"
.SH NAME
\fBmbsmooth\fP - Smooths multibeam bathymetry data.

.SH VERSION
Version 4.1

.SH SYNOPSIS
\fBmbsmooth\fP [\fB-F\fIformat\fP \fB-G\fIwidth\fP \fB-I\fIinfile\fP \fB-L\fIlonflip\fP \fB-O\fIoutfile\fP \fB-W\fIfilterfile\fP \fB-V -H\fP]

.SH DESCRIPTION
\fBmbsmooth\fP applies a spatial domain gaussian filter 
to multibeam bathymetry data in order to smooth out noise 
in the data.  The width of the filter can be varied as a
function of beam number.
The default input and output streams are stdin and stdout.

.SH AUTHORSHIP
David W. Caress (caress@lamont.ldeo.columbia.edu)
.br
Dale N. Chayes (dale@lamont.ldeo.columbia.edu)
.br
Lamont-Doherty Earth Observatory
.br
Palisades, NY 10964

.SH ACKNOWLEDGEMENT
This program is inspired to a large extent by the mb-system
version 1 program mbsmooth by Alberto Malinverno (formerly 
at L-DEO, now at Schlumberger).  The smoothing mechanism
employed in the current version is significantly different
than the nearest-neighbor scheme used in Alberto's program,
so Alberto should not be held responsible for any shortcomings
in the current version.

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
.B \fB-G\fIwidth\fP
Sets the width in meters of the default Gaussian filter.
This value is used for all beams if a filter widths file
is not specified with the \fB-W\fP option.
Default: \fIformat\fP = 250.
.TP
.B \fB-H\fP
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \fB-I\fIinfile\fP
Data file from which the input data will be read. If
no input file is specified, the input will be read
from stdin. Default: \fIinfile\fP = stdin.
.TP
.B \fB-L\fIlonflip\fP
Sets the range of the longitude values used.
If \fIlonflip\fP=-1 then the longitude values will be in
the range from -360 to 0 degrees. If \fIlonflip\fP=0 
then the longitude values will be in
the range from -180 to 180 degrees. If \fIlonflip\fP=1 
then the longitude values will be in
the range from 0 to 360 degrees.
Default: \fIlonflip\fP = 0.
.TP
.B \fB-O\fIoutfile\fP
Data file to which the output data will be written. If
no output file is specified, the output will be written to
stdout. Default: \fIoutfile\fP = stdout.
.TP
.B \fB-W\fIfilterfile\fP
Data file from which filter widths as a function of beam number
are read.  The filter widths should be in the form
.br
	beam  width
.br
where the width is in meters and corresponds the the "w"
value in the Gaussian filter:
.br
	f(r) = exp(-r**2/(w/2)**2) 
.br
Thus, "w" is the full width of the Gaussian filter.
Use of this operation overrides the use of the default filter
width set with the \fB-G\fP option.
.TP
.B \fB-V\fP
Normally, \fBmbsmooth\fP works "silently" without outputting
anything to the stderr stream.  If the
\fB-V\fP flag is given, then \fBmbsmooth\fP works in a "verbose" mode and
outputs the program version being used and all error status messages.

.SH EXAMPLES
Suppose one wishes to smooth a noisy Hydrosweep file in
L-DGO binary format (format 24) using a Gaussian spatial domain
filter with a width of 250 meters for all of the beams. 

 	mbsmooth -F24 -Ihsnoisy -Ohssmooth -G250

.SH SEE ALSO
\fBmbsystem\fP(l)

.SH BUGS
It would be good to include other smoothing schemes as options
in this program.  In particular, we should re-implement 
Alberto Malinverno's nearest neighbor averaging algorithm as
well as a median filter algorithm.
