.TH mbbath l "5 October 1998" "MB-System 4.6" "MB-System 4.6"
.SH NAME
\fBmbbath\fP - Recalculates bathymetry swath bathymetry data using
new sound velocity profile, roll bias, pitch bias and draft values.

.SH VERSION
Version 4.6

.SH SYNOPSIS
\fBmbbath\fP [\fB-B\fIroll_bias\fP 
\fB-C\fP \fB-D\fIdraft\fP 
\fB-F\fIformat\fP \fB-I\fIinfile\fP \fB-K\fIssv\fP 
\fB-O\fIoutfile \fB-P\fIpitch_bias\fP 
\fB-R\fIrollfile\fP \fB-S\fIstaticfile\fP 
\fB-U\fP \fB-W\fIvelfile\fP \fB-Z -V -H\fP]

.SH DESCRIPTION
\fBmbbath\fP is an utility for recalculating swath
bathymetry data using new values for some of the fundamental
sonar operational parameters. Users may apply changes to the
water sound velocity profile, roll bias, pitch bias, and draft
parameters. Static corrections can also be applied to the depth
values.

If a file containing a water sound velocity profile (SVP) is
specified with the \fB-W\fP option, then the bathymetry will
be recalculated from the raw travel times and arrival angles by
raytracing through the SVP. Raytracing can only be performed
using data formats that store both travel times and arrival angles
for each sounding.

The input SVP is specified in terms of velocity values
at specific depths with linear interpolation between these "nodes".
Because the velocity gradients are constant between nodes, the
geometrical raypaths take the form of circular arcs within each
layer, allowing analytical solutions for raypaths and travel times.
The initial position used in the raytracing is the transducer depth
as determined by the sum of the draft and heave values.

The surface sound velocity (SSV - the sound speed at the transducer arrays)
used by the sonar is a critical element in the depth calculation
because the SSV is used in the beamforming calculation by multibeam
sonars and in the phase calculation by interferometry sonars.
When recalculating bathymetry, users have two options:
.br
 	1) Assume that the SSV used by the sonar is 
 		correct (default).
 	2) Change the SSV, using the sound speed at
 		the transducer depth in the SVP
 		(the \fB-C\fP option).
.br
For the first option, in general the SSV will not be the same as
the initial sound speed used in the raytracing. This situation 
requires that the initial ray angles be adjusted for the sound
speed change using Snell's law (and assuming an horizontal interface
for the calculation).

The \fB-C\fP option should only be used if the original SSV is known to
be incorrect and the new SSV derived from the SVP is closer to 
correct. This situation requires that the initial ray angles be
adjusted to correct the errors in the beamforming or phase
calculations done by the sonar. If the transducer receiver arrays
are flat and horizontal, this correction is equivalent to the
Snell's law adjustment in the first option. If the transducer 
receiver arrays are tilted, then this correction is made in 
a reference frame rotated by the tilt angle. The \fBMBIO\fP i/o
library knows about the array geometries of the supported sonars
and provides the relevent information to \fBmbbath\fP.

In all cases, the SSV must be known for the raytracing to proceed. 
Unfortunately, in some data
formats the SSV is recorded only sporadically in special data
records, and thus users may encounter data files which contain
no information about the SSV. Normally, \fBmbbath\fP makes a
preliminary pass through an input data file looking for the first
SSV value and then uses this value from the beginning of the file
until overridden by other values. If no SSV values are found, a
default value (specified with the \fB-K\fP\fIssv\fP option) is used. 
This preliminary pass is inconsistent
with input from stdin (e.g. using \fBmbbath\fP in a pipe), and users
may turn off the preliminary pass 
with the \fB-Z\fP option.

The newly raytraced depths can be saved in corrected 
meters, where the depth values obtained by raytracing are not adjusted 
further, or in uncorrected meters, where the depth values are adjusted
to correspond to a vertical mean water velocity of 1500 m/s. 

If the raytracing recalculation is done, then any roll bias
(\fB-B\fP and \fB-R\fP options) and 
pitch bias (\fB-P\fP option) corrections specified
will be applied to the arrival angles prior to the raytracing.
If no raytracing is done, then the roll and pitch bias corrections
are performed by rigid rotations of the sounding positions.
Users should be aware that the roll and pitch bias corrections
are made in addition to the roll and pitch bias corrections
originally made by the sonar. Users therefore should NOT reenter
the original roll bias and pitch bias values!

If no SVP is specified, then any roll bias
(\fB-B\fP and \fB-R\fP options) and 
pitch bias (\fB-P\fP option) corrections specified 
are applied by performing rigid rotations of the
soundings in a ship-centered reference frame.

Static corrections to the bathymetry
as a function of beam number can be read from a file and applied
to the bathymetry regardless of the other options specified.  
Any bathymetry data flags set in the input data are preserved in
the output.

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
.B \-B
\fIroll_bias\fP
.br
Sets the change to the roll bias value (in degrees) 
to be used in calculating soundings. This value is
added to the roll bias used by the sonar, so users should
NOT reenter the original roll bias value.  If raytracing is 
done, the roll bias change is
added to the acrosstrack component of the beam takeoff angles prior to
the raytracing. If no raytracing is done, the roll bias change 
is applied via a rigid rotation of the soundings in a ship-centered
reference frame. Ship roll is measured with starboard negative and
port positive.
Default: \fIroll_bias\fP = 0.0
.TP
.B \-C
.br
Corrects initial ray angles for error in the original surface sound 
velocity (SSV) value used by the sonar. The new SSV value is
taken from the user-specified SVP at the transducer depth. The
correction incorporates the geometry of the receiving transducer
array.
Default: treats original SSV as correct, and uses Snell's law
to adjust the initial ray angles for the difference between the
SSV and the sound speed at the transducer depth in the SVP.
.TP
.B \-D
\fIdraft\fP
.br
Sets the depth in meters of the sonar used to 
collect the swath bathymetry data. For hull mounted sonars, this
value is equivalent to the draft of the vessel. This value is 
only used when recalculating bathymetry by raytracing, and
replaces any draft value in the input data stream
Default: \fIdraft\fP value found in data used.
.TP
.B \-F
\fIformat\fP
.br
Sets the format for the input and output swath sonar data using 
\fBMBIO\fP integer format identifiers. 
This program only works with data formats which store travel time
data for each beam.
.TP
.B \-H
This "help" flag causes the program to print out a description
of its operation and then exit immediately.
.TP
.B \-I
\fIinfile\fP
.br
Data file from which the input data will be read. If
no input file is specified, the input will be read
from stdin. Default: \fIinfile\fP = stdin.
.TP
.B \-K
\fIssv\fP
.br
Sets the initial SSV value used
from the beginning of the input data until a new SSV value is
encountered in the data stream. 
Default: \fIssv\fP = 1500 m/s.
.TP
.B \-O
\fIoutfile\fP
.br
Data file to which the output data will be written. If
no output file is specified, the output will be written to
stdout. Default: \fIoutfile\fP = stdout.
.TP
.B \-P
\fIpitch_bias\fP
.br
Sets the change to the pitch bias value (in degrees) 
to be used in calculating soundings. This value is
added to the pitch bias used by the sonar, so users should
NOT reenter the original pitch bias value.  If raytracing is 
done, the pitch bias change is
added to the fore-aft component of the beam takeoff angles prior to
the raytracing. If no raytracing is done, the pitch bias change 
is applied via a rigid rotation of the soundings in a ship-centered
reference frame. Ship pitch is measured with aft (bow down) negative and
fore (bow up) positive.
Default: \fIpitch_bias\fP = 0.0
.TP
.B \-R
\fIrollfile\fP
.br
Data file from which roll corrections as a function of time are 
read. The roll corrections should be in the form
.br
	time  correction
.br
where the time is in seconds since 1/1/71 00:00:0 (Unix time) and the
roll correction is in degrees with starboard negative and
port positive.
.TP
.B \-S
\fIstaticfile\fP
.br
Data file from which static depth corrections as a function of
beam number are read.  The depth corrections should be in the form
.br
	beam  correction
.br
where the beam number starts at 0 and the correction value should
be in meters.
.TP
.B \-U
This flag causes the program to adjust the depth values to correspond
to a vertical mean water velocity of 1500 m/s, effectively giving
depth values in the traditional "uncorrected meters" units.  Crosstrack
distances are not affected by this adjustment.  Normally, the program
outputs the depths calculated by raytracing without adjustment; these
values have units of "corrected meters" meters because they are
accurate with respect to a presumably realistic SVP.
Default: meters calculated in corrected meters.
.TP
.B \-V
Normally, \fBmbbath\fP works "silently" without outputting
anything to the stderr stream.  If the
\fB-V\fP flag is given, then \fBmbbath\fP works in a "verbose" mode and
outputs the program version being used and all error status messages.
.TP
.B \-W
\fIvelfile\fP
.br
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
.TP
.B \-Z
.br
Normally \fBmbbath\fP makes an initial pass through the input
data to find the first SSV value, and then uses this value
from the beginning of the file to the first change in the SSV.
If the \fB-Z\fP option is given, the preliminary pass is
skipped and a default value (see the \fB-K\fP\fIssv\fP option)
is used until the first SSV value is encountered in the data stream.

.SH EXAMPLES
Suppose one wishes to recalculate the bathymetry for Hydrosweep DS data
contained in the raw Hydrosweep (format 21) file hsturkey.mb21 using a water
velocity profile specified in the file dodo.svp. Further suppose that
the roll and pitch biases used in the sonar are known to be wrong by 
1.0 and -0.5 degrees, respectively.
The following will suffice:
 	mbbath -F21 -Wdodo.svp -R1.0 -P-0.5 \\
 		-Ihsturkey.mb21 -Ohsturkey_b.mb21
.br
The bathymetry in the output file hsturkey_b.mb21 will be completely
recalculated from the travel time and arrival angle 
data in the file hsturkey.mb21.

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmblevitus\fP(l), \fBmbvelocitytool\fP(l), 
\fBmbm_xbt\fP(l), \fBxbt\fP(l)

.SH BUGS
You tell me.
