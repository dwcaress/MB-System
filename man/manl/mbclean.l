.TH mbclean l "31 October 1994"
.SH NAME
\fBmbclean\fP - Identifies and flags bad beams in multibeam bathymetry data.

.SH VERSION
Version 4.1

.SH SYNOPSIS
\fBmbclean\fP [\fB-B\fIlow/high \fB-C\fIslope\fP 
\fB-D\fIdistance\fP \fB-F\fIformat\fP 
\fB-I\fIinfile\fP \fB-L\fIlonflip\fP \fB-O\fIoutfile\fP 
\fB-M\fImode\fP \fB-Q\fP \fB-X\fIzap_beams\fP \fB-V -H\fP]

.SH DESCRIPTION
\fBmbclean\fP identifies and flags artifacts in multibeam bathymetry data. 
Bad beams are primarily identified based on excessive bathymetric slopes. 
If desired, \fBmbclean\fP will also flag beams associated with "rails" where
outer beams have smaller crosstrack distances than more inner beams.
Low and high bounds on acceptable depth values can be set; depth values
outside the acceptable range will be flagged.
A set number of outer beams can also be flagged.
The default input and output streams 
are stdin and stdout.

.SH AUTHORSHIP
David W. Caress (caress@lamont.ldeo.columbia.edu)
.br
Dale N. Chayes (dale@lamont.ldeo.columbia.edu)
.br
Alberto Malinverno (now at Schlumberger)
.br
Lamont-Doherty Earth Observatory
.br
Palisades, NY 10964

.SH OPTIONS
.TP
.B \fB-B\fIlow/high\fP
This option sets the range of acceptable depth values.  If a depth
value is less than \fIlow\fP or more than \fIhigh\fP then it
will be flagged. No depth range checking is done if the \fB-B\fP option
is not used.
.TP
.B \fB-C\fIslope\fP
The value \fIslope\fP is the maximum acceptable slope. Beams associated
with excessive slopes will be flagged or removed according to the
operational mode specified using the \fB-M\fP option.
Default: \fIslope\fP = 1.0
.TP
.B \fB-D\fIdistance\fP
The navigation and heading of the ship are used to calculate the locations
of beams.  Ship turns often cause beams of adjacent pings to overlap, causing
the distances between these beams to become quite small.  This can, in turn,
magnify noise in the bathymetry data to produce slope estimates which
are excessively large.  The \fB-D\fP option sets the minimum distance 
(in meters) between beams required for an excessive slope to be used 
to flag bad beams.
Default: \fIdistance\fP = 10.
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
.B \fB-M\fImode\fP
Sets the manner in which bad beams identified by excessive slope
are handled.
 	\fImode\fP = 1:	Flags one beam associated with each outlier slope.
 			The flagged beam is the one furthest from the local
 			median depth.
 	\fImode\fP = 2:	Flags both beams associated with each outlier slope.
 	\fImode\fP = 3:	Zeros one beam associated with each outlier slope.
 			The zeroed beam is the one furthest from the local
 			median depth.
 	\fImode\fP = 4:	Zeros both beams associated with each outlier slope.
If the data format of the input file
prohibits storage of negative depths, an error message will be output
and the program will exit. Default: \fImode\fP = 1.
.TP
.B \fB-O\fIoutfile\fP
Data file to which the output data will be written. If
no output file is specified, the output will be written to
stdout. Default: \fIoutfile\fP = stdout.
.TP
.B \fB-Q\fP
This flag causes \fBmbclean\fP to search for bad "rails" in the
multibeam swath; the "rails" refer to groups of outer beams which
have crosstrack distances (and depths) much less than they should 
have.  These are identified when one or more outer beams lies
inboard of a more inner beam; all beams outboard of the first
offending beam are flagged.
.TP
.B \fB-X\fIzap_beams\fP
If this option is used, the outermost \fIzap_beams\fP at both ends
of the swath are flagged as bad; this is useful if the outer beams 
are known to be unreliable. Default: \fIzap_beams\fP = 0.
.TP
.B \fB-V\fP
Normally, \fBmbclean\fP works "silently" without outputting
anything to the stderr stream.  If the
\fB-V\fP flag is given, then \fBmbclean\fP works in a "verbose" mode and
outputs the program version being used, all error status messages, 
and the number of beams flagged as bad.

.SH EXAMPLES
Suppose one wishes to clean up a noisy raw Hydrosweep file in
L-DGO binary format (format 24) by flagging bad
depths as negative numbers. Use the following to delete
the outermost four beams at each end of the swath and to allow
a maximum slope of 45 degrees:

 	mbclean -F24 -C1.0 -M1 -X4 -Ihsnoisy -Ohsclean

The program will output some information:

 	input multibeam data file:  noisy.edmb
 	output multibeam data file: clean.edmb
 	multibeam format 24: L-DGO 59 ping edmb format
 	(binary, centered)

 	input records:  527
 	output records: 527
 	beams before deletion:  30378
 	beams deleted:          3672 (12%)
 	number of zapped pings: 4

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbunclean\fP(l), \fBmbedit\fP(l), 
\fBmbinfo\fP(l)

.SH BUGS
This algorithm simply doesn't detect all bathymetric artifacts that
are obvious to the eye on contour charts.  In particular, bad beams 
at the outer ends of the swaths are sometimes not detected because 
they are so far from any other samples that the slopes
turn out to be acceptable.
