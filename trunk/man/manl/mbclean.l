.TH mbclean l "17 September 1997" "MB-System 4.5" "MB-System 4.5"
.SH NAME
\fBmbclean\fP - Identifies and flags bad beams in swath sonar bathymetry data.

.SH VERSION
Version 4.5

.SH SYNOPSIS
\fBmbclean\fP [\fB-A\fImax\fP \fB-B\fIlow/high\fP \fB-C\fIslope\fP 
\fB-D\fImin/max\fP \fB-F\fIformat\fP 
\fB-G\fIfraction_low/fraction_high\fP 
\fB-I\fIinfile\fP \fB-L\fIlonflip\fP \fB-N\fIbuffersize\fP \fB-O\fIoutfile\fP 
\fB-M\fImode\fP \fB-Q\fP \fB-U\fInmin\fP \fB-X\fIzap_beams\fP \fB-V -H\fP]

.SH DESCRIPTION
\fBmbclean\fP identifies and flags artifacts in swath sonar bathymetry data.
Several algorithms are available for identifying artifacts; multiple
algorithms can be applied in a single pass.
The most commonly used approach is to identify artifacts  
based on excessive bathymetric slopes. 
If desired, \fBmbclean\fP will also flag beams associated with "rails" where
outer beams have smaller crosstrack distances than more inner beams.
Low and high bounds on acceptable depth values can be set; depth values
outside the acceptable range will be flagged. The acceptable depth
ranges can either be absolute (\fB-B\fP option), relative to
the local median depth (\fB-A\fP option) or defined by low
and high fractions of the local median depth (\fB-G\fP option).
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
.B \-A
\fImax\fP
.br
This option sets the range of acceptable depth values relative to
the local median depth. The median depth is obtained from the
current ping and the pings immediately before and after that
ping.  If a depth value deviates from the median depth by more
than \fImax\fP, then it
will be flagged. No deviation from the median depth checking is 
done if the \fB-A\fP option
is not used.
.TP
.B \-B
\fIlow/high\fP
.br
This option sets the range of acceptable depth values.  If a depth
value is less than \fIlow\fP or more than \fIhigh\fP then it
will be flagged. No depth range checking is done if the \fB-B\fP option
is not used.
.TP
.B \-C
\fIslope\fP
.br
The value \fIslope\fP is the maximum acceptable slope. Beams associated
with excessive slopes will be flagged or removed according to the
operational mode specified using the \fB-M\fP option. This method will
be used if no other algorithms are specified; if other algorithms are
specified but \fB-C\fP is not used then no slope checking will occur.
Default: \fIslope\fP = 1.0
.TP
.B \-D
\fImin/max\fP
.br
Sets the minimum and maximum allowed distances between beams used for
some of the flagging algorithms.  Both values are expressed in terms
of fractions of the local median depth. Thus, \fB-D\fP\fI0.01/0.25\fP
will translate, if the local median depth is 1000 meters, to a minimum
distance of 10 meters and a maximum distance of 250 meters.
The \fImin\fP value sets the minimum distance 
between beams required for an excessive slope to be used 
to flag bad beams.
The navigation and heading of the ship are used to calculate the locations
of beams.  Ship turns often cause beams of adjacent pings to overlap, causing
the distances between these beams to become quite small.  This can, in turn,
magnify noise in the bathymetry data to produce slope estimates which
are excessively large. The \fImax\fP value sets the maximum distance
between the current beam and other beams for those beams to be used
in evaluating the current beam. For instance, only beams within the
maximum distance are used to calculate the local median depth, and only
beams within the maximum distance are used to check for excessive slopes.
Default: \fImin/max\fP = 0.01/0.25.
.TP
.B \-F
\fIformat\fP
.br
Sets the format for the input and output swath sonar data using 
\fBMBIO\fP integer format identifiers. 
This program uses the \fBMBIO\fP library and will read or write any swath sonar
format supported by \fBMBIO\fP. A list of the swath sonar data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page.
Default: \fIformat\fP = 11.
.TP
.B \-G
\fIfraction_low/fraction_high\fP
.br
This option sets the range of acceptable depth values relative to
low and high fractions of the local median depth. 
The median depth is obtained from the
current ping and the pings immediately before and after that
ping.  If a depth
value is less than \fIfraction_low\fP times the median depth
(e.g. \fIfraction_low\fP = 0.5 means one half the median
depth) or more than \fIfraction_high\fP times the median depth then it
will be flagged. No fractional depth range checking is 
done if the \fB-G\fP option
is not used.
.TP
.B \-H
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \-I
\fIinfile\fP
.br
Data file from which the input data will be read. If
no input file is specified, the input will be read
from stdin. Default: \fIinfile\fP = stdin.
.TP
.B \-L
\fIlonflip\fP
.br
Sets the range of the longitude values used.
If \fIlonflip\fP=-1 then the longitude values will be in
the range from -360 to 0 degrees. If \fIlonflip\fP=0 
then the longitude values will be in
the range from -180 to 180 degrees. If \fIlonflip\fP=1 
then the longitude values will be in
the range from 0 to 360 degrees.
Default: \fIlonflip\fP = 0.
.TP
.B \-M
\fImode\fP
.br
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
.B \-N
\fIbuffersize\fP
.br
Sets the maximum number of data records which can be
read into the buffer. In general, data records may be
of several different types (e.g. parameter, position, 
comment) in addition to survey data records. Many data 
formats include many more position data records than
survey data records. Thus, a large buffer may be required
to access a reasonable number of survey data records. 
However, on memory limited machines large buffer sizes
can lead to poor performance due to memory swapping. 
The default value of \fIbuffersize\fP = 500 is appropriate 
for most cases, but users can set the buffer size as required.
The absolute maximum buffer size is 5000. 
Default: \fIbuffersize\fP = 500.
.TP
.B \-O
\fIoutfile\fP
.br
Data file to which the output data will be written. If
no output file is specified, the output will be written to
stdout. Default: \fIoutfile\fP = stdout.
.TP
.B \-Q
This flag causes \fBmbclean\fP to search for bad "rails" in the
swath sonar swath; the "rails" refer to groups of outer beams which
have crosstrack distances (and depths) much less than they should 
have.  These are identified when one or more outer beams lies
inboard of a more inner beam; all beams outboard of the first
offending beam are flagged.
.TP
.B \-U
\fInmin\fP
.br
This flag causes \fBmbclean\fP to search for port or starboard
halves of pings which contain fewer than \fInmin\fP good bathymetry
values. All bathymetry values in the affected half-pings are
flagged.
.TP
.B \-X
\fIzap_beams\fP
.br
If this option is used, the outermost \fIzap_beams\fP at both ends
of the swath are flagged as bad; this is useful if the outer beams 
are known to be unreliable. Default: \fIzap_beams\fP = 0.
.TP
.B \-V
Normally, \fBmbclean\fP works "silently" without outputting
anything to the stderr stream.  If the
\fB-V\fP flag is given, then \fBmbclean\fP works in a "verbose" mode and
outputs the program version being used, all error status messages, 
and the number of beams flagged as bad.

.SH EXAMPLES
Suppose one wishes to do a first pass edit of 
a raw SeaBeam 2112 file in
the vendor format (format 41). Use the following to flag any
beams which deviate by more than 1% from the local median
depth or which produce a slope greater than 3.5 (74 degrees):

 	mbclean -F41 -M1 -C3.5 -D0.01/0.20 \
 		-G0.99/1.01 \
 		-I sb199507130318.rec \
 		-O sb199507130318_c.mb41

The program will output some information because of the
\fB-V\fP option:
.br
 	Program MBCLEAN
 	Version $Id: mbclean.l,v 4.11 1997-09-17 19:22:06 caress Exp $
 	MB-system Version 4.3
.br
 	132 records loaded into buffer
.br
 	f: 1995  9  2 04:50:44.413000    75  1243.50  1263.80
 	f: 1995  9  2 04:50:44.413000    76  1244.80  1263.80
 	f: 1995  9  2 04:50:44.413000    99  1244.20  1265.20
 	f: 1995  9  2 04:50:48.795000    14  1242.90  1257.30
 	......... (lines deleted) .........
 	s: 1995  9  2 04:59:25.189000    45  1784.00  1231.20   3.71 148.80
 	s: 1995  9  2 04:59:31.779000    46  1777.20  1230.90   3.63 149.80
 	s: 1995  9  2 04:59:31.779000    47  1530.50  1231.20   4.27  68.50
 	......... (lines deleted) .........
.br
 	82 records dumped from buffer
 	0 records loaded into buffer
 	50 records dumped from buffer
 	179 bathymetry data records processed
.br
 	0 outer beams zapped
 	0 beams out of acceptable depth range
 	96 beams out of acceptable fractional depth range
 	0 beams exceed acceptable deviation from median depth
 	0 bad rail beams identified
 	14 excessive slopes identified
 	110 beams flagged
 	0 beams zeroed

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbunclean\fP(l), \fBmbedit\fP(l), 
\fBmbinfo\fP(l)

.SH BUGS
The algorithms implemented in \fBmbclean\fP simply 
don't detect all bathymetric artifacts that
are obvious to the eye on contour charts.  Although
\fBmbclean\fP often does a credible first pass at
flagging obvious artifacts, we strongly recommend that
any swath bathymetry processing stream include 
interactive editing of the
bathymetry data (e.g. \fBmbedit\fP).
