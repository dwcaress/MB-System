.TH mbsimradmakess l "4 January 2001" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbsimradmakess\fP - Regenerates sidescan
imagery from the raw amplitude samples contained in data from 
Simrad EM300 and EM3000 multibeam sonars.

.SH VERSION
Version 5.0

.SH SYNOPSIS
\fBmbsimradmakess\fP [\fB-F\fIformat \fB-I\fIinfile \fB-O\fIoutfile 
\fB-P\fIpixel_size\fP \fB-S\fIswath_width\fP \fB-T\fIpixel_int\fP 
\fB-V -H\fP]

.SH DESCRIPTION
The program \fBmbsimradmakess\fP is an utility for regenerating sidescan
imagery from the raw amplitude samples contained in data from 
Simrad EM300 and EM3000 multibeam sonars. This program ignores
amplitude data associated with flagged (bad) bathymetry data, 
thus removing one important source of noise in the
sidescan data.

The number of raw samples provided by the sonar for 
each ping ranges from a few hundred to
as much as ten thousand, depending on the water depth, swath width,
and the raw digitization sampling interval. Each bathymetry
sounding (or beam) is associated with some number of raw
amplitude samples. The acrosstrack locations of the samples are
found using the raw sample size (determined by the digitization
rate within the sonar) and the beam locations. This program 
regenerates sidescan by binning, averaging, and interpolating 
the amplitudes into a 1024 pixel array.

The most important control is the output pixel size. The user
can set this value to a constant using the \fB-P\fP command. By
default, the program adjusts the pixel size for each ping to maintain
reasonable swath coverage with 1024 pixels. The desired swath
width (in angular terms) can be set by the user using 
the \fB-S\fP command; by default
the swath width actually achieved by the sonar is used. Regardless
of the swath width setting, the pixel size is not allowed to change
by more than 5% for any ping.

Once the pizel size for a particular ping is set, all of the
amplitude samples associated with unflagged (presumably good)
bathymetry are binned into the 1024 pixel array, and then averaged.
The removal of amplitudes from bad beams may leave gaps in
the sidescan; these gaps can be filled by a simple linear
interpolation. The \fB-T\fP command sets the maximum size of the
gaps (in number of pixels) which can be interpolated. The default
is for no interpolation. 

The default input and output streams are stdin and stdout.

.SH AUTHORSHIP
David W. Caress (caress@mbari.org)
.br
  Monterey Bay Aquarium Research Institute
.br
Dale N. Chayes (dale@ldeo.columbia.edu)
.br
  Lamont-Doherty Earth Observatory

.SH OPTIONS
.TP
.B \-F
\fIformat\fP
.br
Sets the format for the input and output swath sonar data using 
\fBMBIO\fP integer format identifiers. 
This program uses the \fBMBIO\fP library, but works only with
Simrad EM300/EM3000 data in the MBF_EM300MBA processing 
format (MBIO id = 57). 
Default: \fIformat\fP = 57.
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
.B \-O
\fIoutfile\fP
.br
Data file to which the output data will be written. If
no output file is specified, the output will be written to
stdout. Default: \fIoutfile\fP = stdout.
.TP
.B \-P
\fIpixel_size\fP
.br
Sets the pixel size (in meters) of the sidescan data to a constant.
The default is for the pixel size to be calculated for every ping
so as to fit the swath width into the 1024 sidescan pixel array.
If the pixel size is recalculated for each ping, the value is not
allowed to change more than 5% in a single ping.
.TP
.B \-S
\fIswath_width\fP
.br
Sets the swath width (in degrees) of the sidescan data to a
constant. This swath width is used to set the pixel size (unless
the \fB-P\fP command is used to set that value constant).
The default is for the swath width to conform to whatever
the sonar actually achieves.
.TP
.B \-T
\fIpixel_int\fP
.br
Sets the maximum size (in numbers of pixels) of sidescan data gaps
that are filled by linear interpolation. The default is for no
interpolation, or \fIpixel_int\fP = 0.
.TP
.B \-V
Normally, \fBmbsimradmakess\fP works "silently" without outputting
anything to the stderr stream.  If the
\fB-V\fP flag is given, then \fBmbsimradmakess\fP works in a "verbose" mode and
outputs the program version being used and all error status messages.

.SH EXAMPLES
Suppose one wishes to work with the sidescan imagery in EM300
data. The data generally starts in the Simrad vendor format
(\fBMBIO\fP format ID 56) which does not support flagging of bathymetry
data and does not contain processed sidescan imagery. An initial
version of the 1024 pixel sidescan is created on the 
fly whenever \fBMB-System\fP
programs are used to read the format 56 data, but all of the raw
amplitude samples are included. Use the program \fBmbcopy\fP to
translate the data into a processing format (\fBMBIO\fP 
format ID 57) which supports 
bathymetry flagging and processed sidescan:
 	mbcopy -F56/57 -I 605.0.em300 \
 		-O 605.mb57 -V
.br
If some of the bathymetry soundings are erroneous, then the
raw amplitudes associated with those bad beams will also be
erroneous and corrupt the sidescan imagery.
Suppose that editing the bathymetry with \fBmbedit\fP results
in a data file called 605_e.mb57 in which all bad soundings have
been identified and flagged. Then the sidescan imagery can be
regenerated without the inclusion of the "bad" amplitudes by
doing the following:
 	mbsimradmakess -I 605_e.mb57 \
 		-O 605_es.mb57 \
 		-T10 -V
.br
Here we allow the program to determine the appropriate pixel
size on a ping by ping basis, and we allow interpolation of
sidescan gaps up to 10 pixels in size.

.SH BUGS
This program could also work with data from the older
Simrad sonars (e.g. EM950, EM1000, EM12), 
provided we devise and support appropriate 
new processing formats.
