.TH mbbath l "31 October 1994"
.SH NAME
\fBmbbath\fP - Generates bathymetry from travel times in multibeam data.

.SH VERSION
Version 4.1

.SH SYNOPSIS
\fBmbbath\fP [\fB-A\fIangle\fP \fB-B\fIroll_bias\fP \fB-D\fIdraught\fP 
\fB-F\fIformat\fP \fB-I\fIinfile\fP \fB-O\fIoutfile \fB-P\fIpitch_bias\fP 
\fB-R\fIrollfile\fP \fB-S\fIstaticfile\fP 
\fB-U\fP \fB-W\fIvelfile\fP \fB-V -H\fP]

.SH DESCRIPTION
\fBmbbath\fP is an utility for calculating bathymetry from travel times
in multibeam data.  The depths and crosstrack distances are calculated 
from the travel times by raytracing through a water velocity profile 
specified by the user.  This program only works with data formats
that store travel time data for each beam.

The input velocity model is specified in terms of velocity values
at specific depths.  The raytracing is done using a "layercake"
model consisting of layers with uniform velocities.  The input
"nodal" model is converted to a "layercake" model by taking each
interval between velocity nodes as a layer and using the average
of the velocity values from the top and bottom of the layer as
the velocity within the layer.

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
for the appropriate multibeam system is internal to MB-System,
but could be different on different installations of the
same sonar. Also, some sonars may allow variable beam angle
spacing.
.TP
.B \fB-B\fIroll_bias\fP
Sets the roll bias used in calculating crosstrack distances and depths
for the beams in survey (athwartships) pings.  The roll bias is
added to the takeoff angles for each of the survey beams prior to
the raytracing.  Ship roll is measured with starboard negative and
port positive.
Default: \fIroll_bias\fP = 0.0
.TP
.B \fB-D\fIdraught\fP
Sets the draught in meters of the ship used to collect the multibeam data.
This value will be added to the depth values determined by raytracing.
Default: \fIdraught\fP = 5.5, the standard value for the R/V Ewing.
.TP
.B \fB-F\fIformat\fP
Sets the format for the input and output multibeam data using 
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
Sets the pitch bias used in calculating crosstrack distances and depths
for the beams in calibrate (fore-aft) pings.  The pitch bias is
added to the takeoff angles for each of the calibrate beams prior to
the raytracing.  Ship pitch is measured with aft negative and
fore positive.
Default: \fIpitch_bias\fP = 0.0
.TP
.B \fB-R\fIrollfile\fP
Data file from which roll corrections as a function of time are 
read. The roll corrections should be in the form
.br
	time  correction
.br
where the time is in minutes since 1/1/71 00:00:0 and the
roll correction is in degrees with starboard negative and
port positive. This correction file can be created using the
macro mbm_rollerror.
.TP
.B \fB-S\fIstaticfile\fP
Data file from which static depth corrections as a function of
beam number are read.  The depth corrections should be in the form
.br
	beam  correction
.br
where the beam number is 1-59 and the correction value should
be in meters.
.TP
.B \fB-U\fP
This flag causes the program to adjust the depth values to correspond
to a vertical mean water velocity of 1500 m/s, effectively giving
depth values in the traditional "uncorrected meters" units.  Crosstrack
distances are not affected by this adjustment.  Normally, the program
outputs the depths calculated by raytracing without adjustment; these
values have units of "corrected meters" meters because they are
accurate with respect to a presumably realistic water velocity model.
Default: meters calculated in corrected meters.
.TP
.B \fB-V\fP
Normally, \fBmbbath\fP works "silently" without outputting
anything to the stderr stream.  If the
\fB-V\fP flag is given, then \fBmbbath\fP works in a "verbose" mode and
outputs the program version being used and all error status messages.
.TP
.B \fB-W\fIvelfile\fP
Data file from which the water velocity profile will be read. The 
water velocity profile must be specified as a set of
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
\fBmbbath\fP converts the velocity
profile to a model consisting of homogeneous layers by using the
average velocity between each pair of adjacent depth/velocity pairs
as the velocity of a homogeneous layer between those two depth
points. If the velocity profile does not extend
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
.SH BUGS
You tell me.
