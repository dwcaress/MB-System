.TH mbclean l "8 August 2001" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbclean\fP - Tool to automatically flag bad beams in swath sonar bathymetry data.

.SH VERSION
Version 5.0

.SH SYNOPSIS
\fBmbclean\fP [\fB-A\fImax\fP \fB-B\fIlow/high\fP \fB-C\fIslope\fP 
\fB-D\fImin/max\fP \fB-F\fIformat\fP 
\fB-G\fIfraction_low/fraction_high\fP 
\fB-I\fIinfile\fP \fB-L\fIlonflip\fP \fB-O\fIoutfile\fP 
\fB-M\fImode\fP \fB-Q\fP \fB-U\fInmin\fP \fB-X\fIzap_beams\fP \fB-V -H\fP]

.SH DESCRIPTION
\fBmbclean\fP identifies and flags artifacts in swath sonar bathymetry data.
Several algorithms are available for identifying artifacts; multiple
algorithms can be applied in a single pass.
The most commonly used approach is to identify artifacts  
based on excessive bathymetric slopes. 
If desired, \fBmbclean\fP will also flag beams 
associated with "rails" where
outer beams have smaller acrosstrack distances 
than more inner beams (\fB-Q\fP option).
Low and high bounds on acceptable depth values can be set; depth values
outside the acceptable range will be flagged. The acceptable depth
ranges can either be absolute (\fB-B\fP option), relative to
the local median depth (\fB-A\fP option) or defined by low
and high fractions of the local median depth (\fB-G\fP option).
A set number of outer beams can also be flagged.

The order in which the flagging algorithms are applied is
as follows:
.br
 	1. Flag specified number of outer beams 
 		(\fB-X\fP option).
 	2. Flag soundings outside specified acceptable 
 		depth range (\fB-B\fP option).
 	3. Flag soundings outside acceptable depth range 
 		using fractions of local median depth 
 		(\fB-G\fP option).
 	4. Flag soundings outside acceptable depth range 
 		using deviation from local median depth 
 		(\fB-A\fP option).
 	5. Flag soundings associated with excessive slopes 
 		(\fB-C\fP option or default).
 	6. Zap "rails" (\fB-Q\fP option).
 	7. Flag all soundings in pings with too few 
 		good soundings (\fB-U\fP option).

This program flags beams by outputting the flags as 
edit events to an "edit save file", like that produced
by \fBmbedit\fP. If an "edit save file" (named by adding
a ".esf" suffix to the input swath filename) already
exists, the edits are read in and applied before the
\fBmbclean\fP flagging algorithms are used.
Once generated, the edit events can be applied 
to the data using the program \fBmbprocess\fP, which
outputs a processed swath data file. 
The \fBmbprocess\fP program is also used to merge
edited navigation, recalculate bathymetry, and apply
other corrections to swath bathymetry data. 

The program \fBmbclean\fP that was included in \fBMB-System\fP
version 4 distributions directly output a "cleaned"
swath file. This functionality is currently preserved
in the program \fBmbcleanold\fP.

.SH AUTHORSHIP
David W. Caress (caress@mbari.org)
.br
  Monterey Bay Aquarium Research Institute
.br
Dale N. Chayes (dale@ldeo.columbia.edu)
.br
  Lamont-Doherty Earth Observatory
.br
Alberto Malinverno
.br
  Schlumberger-Doll

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
Sets the input filename. If \fIformat\fP > 0 (set with the 
\fB-F\fP option) then the swath sonar data contained in \fIinfile\fP 
is read and processed. If \fIformat\fP < 0, then \fIinfile\fP
is assumed to be an ascii file containing a list of the input swath sonar
data files to be processed and their formats.  The program will read 
and process the data in each one of these files. Each input file will
have an associated output file with either the ".sga" or ".aga" suffix.
In the \fIinfile\fP file, each
data file should be followed by a data format identifier, e.g.:
 	datafile1 11
 	datafile2 24
.br
This program uses the \fBMBIO\fP library and will read or write any swath sonar
format supported by \fBMBIO\fP. A list of the swath sonar data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page. Default: \fIinfile\fP = "datalist.mb-1".
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
six Simrad EM300 files in
the processing format (format 57). A datalist referencing these
six files exists as the file datalist.mb-1 and has the contents:
 	0001_20020424_212920.mb57 57
 	0002_20020425_011607.mb57 57
 	0003_20020425_022926.mb57 57
 	0004_20020425_024336.mb57 57
 	0005_20020425_034057.mb57 57
 	0006_20020425_045013.mb57 57
.br
Use the following to flag any
beams which deviate by more than 20% from the local median
depth or which produce a slope greater than 3.5 (74 degrees):

 	mbclean -Idatalist.mb-1 \\
 		-M1 -C3.5 -D0.01/0.20 \\
 		-G0.80/1.20

The program will output flagging statistics for each file and
give totals at the end. If the \fB-V\fP option is specified, 
\fBmbclean\fP will also output information for each beam that
is flagged. Here is an example of the nonverbose output:
.br
.br
 	Processing 0001_20020424_212920.mb57
 	908 bathymetry data records processed
 	0 outer beams zapped
 	0 beams zapped for too few good beams in ping
 	0 beams out of acceptable depth range
 	64 beams out of acceptable fractional depth range
 	0 beams exceed acceptable deviation from median depth
 	0 bad rail beams identified
 	1601 excessive slopes identified
 	1665 beams flagged
 	0 beams unflagged
 	0 beams zeroed

 	Processing 0002_20020425_011607.mb57
 	259 bathymetry data records processed
 	0 outer beams zapped
 	0 beams zapped for too few good beams in ping
 	0 beams out of acceptable depth range
 	0 beams out of acceptable fractional depth range
 	0 beams exceed acceptable deviation from median depth
 	0 bad rail beams identified
 	242 excessive slopes identified
 	242 beams flagged
 	0 beams unflagged
 	0 beams zeroed

 	Processing 0003_20020425_022926.mb57
 	65 bathymetry data records processed
 	0 outer beams zapped
 	0 beams zapped for too few good beams in ping
 	0 beams out of acceptable depth range
 	9 beams out of acceptable fractional depth range
 	0 beams exceed acceptable deviation from median depth
 	0 bad rail beams identified
 	497 excessive slopes identified
 	506 beams flagged
 	0 beams unflagged
 	0 beams zeroed

 	Processing 0004_20020425_024336.mb57
 	410 bathymetry data records processed
 	0 outer beams zapped
 	0 beams zapped for too few good beams in ping
 	0 beams out of acceptable depth range
 	0 beams out of acceptable fractional depth range
 	0 beams exceed acceptable deviation from median depth
 	0 bad rail beams identified
 	148 excessive slopes identified
 	148 beams flagged
 	0 beams unflagged
 	0 beams zeroed

 	Processing 0005_20020425_034057.mb57
 	252 bathymetry data records processed
 	0 outer beams zapped
 	0 beams zapped for too few good beams in ping
 	0 beams out of acceptable depth range
 	0 beams out of acceptable fractional depth range
 	0 beams exceed acceptable deviation from median depth
 	0 bad rail beams identified
 	100 excessive slopes identified
 	100 beams flagged
 	0 beams unflagged
 	0 beams zeroed

 	Processing 0006_20020425_045013.mb57
 	562 bathymetry data records processed
 	0 outer beams zapped
 	0 beams zapped for too few good beams in ping
 	0 beams out of acceptable depth range
 	0 beams out of acceptable fractional depth range
 	0 beams exceed acceptable deviation from median depth
 	0 bad rail beams identified
 	41 excessive slopes identified
 	41 beams flagged
 	0 beams unflagged
 	0 beams zeroed

 	MBclean Processing Totals:
 	-------------------------
 	6 total swath data files processed
 	2456 total bathymetry data records processed
 	0 total beams flagged in old esf files
 	0 total beams unflagged in old esf files
 	0 total beams zeroed in old esf files
 	0 total outer beams zapped
 	0 total beams zapped for too few good beams in ping
 	0 total beams out of acceptable depth range
 	73 total beams out of acceptable fractional depth range
 	0 total beams exceed acceptable deviation from median depth
 	0 total bad rail beams identified
 	2629 total excessive slopes identified
 	2702 total beams flagged
 	0 total beams unflagged
 	0 total beams zeroed

.SH SEE ALSO
\fBmbsystem\fP(l), \fBcleanold\fP(l), \fBmbedit\fP(l), 
\fBmbinfo\fP(l) \fBmbprocess\fP(l),

.SH BUGS
The algorithms implemented in \fBmbclean\fP simply 
don't detect all bathymetric artifacts that
are obvious to the eye on contour charts.  Although
\fBmbclean\fP often does a credible first pass at
flagging obvious artifacts, we strongly recommend that
any swath bathymetry processing stream include 
interactive editing of the
bathymetry data (e.g. \fBmbedit\fP).
