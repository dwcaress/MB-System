.TH mbbath l "25 March 1997" "MB-System 4.5" "MB-System 4.5"
.SH NAME
\fBmbbath\fP - Generates bathymetry from travel times in swath sonar data.

.SH VERSION
Version 4.5

.SH SYNOPSIS
\fBmbbath\fP [\fB-A\fIangle\fP \fB-B\fIroll_bias\fP \fB-D\fIdraught\fP 
\fB-F\fIformat\fP \fB-I\fIinfile\fP \fB-O\fIoutfile \fB-P\fIpitch_bias\fP 
\fB-R\fIrollfile\fP \fB-S\fIstaticfile\fP 
\fB-U\fP \fB-W\fIvelfile\fP \fB-V -H\fP]

.SH DESCRIPTION
\fBmbbath\fP is an utility for calculating bathymetry from travel times
in swath sonar data.  The depths and crosstrack distances are calculated 
from the travel times by raytracing through a water sound velocity profile 
(SVP) specified by the user.  This program only works with data formats
that store travel time data for each beam.

The input SVP is specified in terms of velocity values
at specific depths with linear interpolation between these "nodes".
Because the velocity gradients are constant between nodes, the
geometrical raypaths take the form of circular arcs within each
layer, allowing analytical solutions for raypaths and travel times.

If the previous
version of the bathymetry includes beams that have been flagged as bad
by having the depths set negative, the corresponding new depths will
also be set negative to preserve the existing data editing.
The depths can be saved in corrected 
meters, where the depth values obtained by raytracing are not adjusted 
further, or in uncorrected meters, where the depth values are adjusted
to correspond to a vertical mean water velocity of 1500 m/s.  Roll bias
and pitch bias values can be input; the roll bias affects the calculation
of survey (athwartships) pings and the pitch bias affects the calculation
of calibration (fore-aft) pings.  Static corrections to the bathymetry
as a function of beam number can be read from a file and applied.  
Roll corrections as a function of time can be read from a file and
applied as the bathymetry is generated.

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
.B \fB-A\fIangle\fP
Sets the angle interval of adjacent beams.  The default value
for the appropriate swath sonar system is internal to MB-System,
but could be different on different installations of the
same sonar.
.TP
.B \fB-B\fIroll_bias\fP
Sets the roll bias (in degrees) used in calculating 
beam locations.  The roll bias is
added to the acrosstrack component of the beam takeoff angles prior to
the raytracing.  Ship roll is measured with starboard negative and
port positive.
Default: \fIroll_bias\fP = 0.0
.TP
.B \fB-D\fIdraught\fP
Sets the \fIdraught\fP in meters of the ship used to collect the swath sonar data.
This value will be added to the depth values determined by raytracing.
Note that if the data stream includes a \fIdraught\fP value, it will
automatically be included in the bathymetry calculations. 
Thus, this option should only be used when the \fIdraught\fP value is
not included in the data stream or is in error.
Default: \fIdraught\fP = 0.0.
.TP
.B \fB-F\fIformat\fP
Sets the format for the input and output swath sonar data using 
\fBMBIO\fP integer format identifiers. 
This program only works with data formats which store travel time
data for each beam.
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
.B \fB-O\fIoutfile\fP
Data file to which the output data will be written. If
no output file is specified, the output will be written to
stdout. Default: \fIoutfile\fP = stdout.
.TP
.B \fB-P\fIpitch_bias\fP
Sets the pitch bias (in degrees) used in calculating beam 
locations.  The pitch bias is
added to the fore-aft component of the beam takeoff angles prior to
the raytracing.  Ship pitch is measured with aft (bow down) negative and
fore (bow up) positive.
Default: \fIpitch_bias\fP = 0.0
.TP
.B \fB-R\fIrollfile\fP
Data file from which roll corrections as a function of time are 
read. The roll corrections should be in the form
.br
	time  correction
.br
where the time is in seconds since 1/1/71 00:00:0 (Unix time) and the
roll correction is in degrees with starboard negative and
port positive.
.TP
.B \fB-S\fIstaticfile\fP
Data file from which static depth corrections as a function of
beam number are read.  The depth corrections should be in the form
.br
	beam  correction
.br
where the beam number starts at 0 and the correction value should
be in meters.
.TP
.B \fB-U\fP
This flag causes the program to adjust the depth values to correspond
to a vertical mean water velocity of 1500 m/s, effectively giving
depth values in the traditional "uncorrected meters" units.  Crosstrack
distances are not affected by this adjustment.  Normally, the program
outputs the depths calculated by raytracing without adjustment; these
values have units of "corrected meters" meters because they are
accurate with respect to a presumably realistic SVP.
Default: meters calculated in corrected meters.
.TP
.B \fB-V\fP
Normally, \fBmbbath\fP works "silently" without outputting
anything to the stderr stream.  If the
\fB-V\fP flag is given, then \fBmbbath\fP works in a "verbose" mode and
outputs the program version being used and all error status messages.
.TP
.B \fB-W\fIvelfile\fP
Data file from which the water SVP will be read. The 
SVP must be specified as a set of
depth-velocity pairs which can be read in free format.  An example
would be:
.br
 # this is a water velocity profile
 0.000000 1522.337158
 10.000000 1522.323364
 20.000000 1522.285889
 30.000000 1521.955200
 50.000000 1520.708252
 75.000000 1518.300903
 100.000000 1516.424316
 125.000000 1515.150024
.br
where the first column is depth in meters and the second column
is velocity in meters/second.  If the first character of a line is
'\#', then the line is treated as a comment and ignored.
If the velocity profile does not extend
to depths of 12000 m (deeper than the deepest known seafloor),
then \fBmbbath\fP extends the profile to that depth using the
velocity of the deepest layer.  The default water velocity
profile is simply a halfspace with a velocity of 1500 m/s.

.SH EXAMPLES
Suppose one wishes to recalculate the bathymetry for Hydrosweep DS data
contained in the raw Hydrosweep (format 21) file hsraw using a water
velocity profile specified in the file velprof. Further suppose that
the roll and pitch biases are known to be 1.0 and -0.5 degrees,
respectively.
The following will suffice:
 	mbbath -F21 -Wnavhs -R1.0 -P-0.5 -Ihsraw -Ohsraytrace
.br
The bathymetry in the output file hsraytrace will be completely
recalculated from the travel time data in the file hsraw.

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmblevitus\fP(l), \fBmbvelocitytool\fP(l), 
\fBmbm_xbt\fP(l), \fBxbt\fP(l)

.SH BUGS
You tell me.
