.TH mbsegylist l "24 September 2004" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbsegylist\fP - List selected header values in segy format seismic data files.

.SH VERSION
Version 5.0

.SH SYNOPSIS
\fBmbsegylist\fP [\fB-A\fP \fB-I\fP\fIinfilename\fP 
\fB-L\fP\fIlonflip\fP \fB-O\fP\fIoutput_format\fP 
\fB-U\fP\fIcheck\fP \fB-H\fP \fB-V\fP]

.SH DESCRIPTION
\fBMBsegylist\fP is a utility to output specified header values
from segy seismic data files to stdout. The
option \fB-O\fP\fIoutput_format\fP is 
used to control the type and order of values output. 
By default, \fBmbsegylist\fP  produces ASCII files in
spreadsheet style, with data columns separated by tabs. Alternatively,
the output can be binary, with each field represented 
as a double precision float (\fB-A\fP option).  

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
.B \-A
.br
Causes the output to be binary (native double precision floating
point) rather than ASCII. Some
output options cannot be represented as single binary floats (e.g.
time strings and longitude or latitude broken into degrees
and minutes. These values are output as multiple fields as
appropriate.
Default: ASCII output with fields separated by tabs.
.TP
.B \-H
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \-I
\fIfilename\fP
.br
Sets the filename of the input segy seismic data file.
.TP
.B \-L
\fIlonflip\fP
.br
Sets the range of the longitude values returned.
If \fIlonflip\fP=-1 then the longitude values will be in
the range from -360 to 0 degrees. If \fIlonflip\fP=0 
then the longitude values will be in
the range from -180 to 180 degrees. If \fIlonflip\fP=1 
then the longitude values will be in
the range from 0 to 360 degrees.
Default: \fIlonflip\fP = 0.
.TP
.B \-O
\fIoutput_format\fP
.br
Determines the form of the output; \fIoutput_format\fP is a string composed
of one or more of the following characters:
.IP
\fB/\fP 
special character: this causes the value 
indicated by the next character to be  inverted. This applies only to simple
numeric values such as depth and 
heading and not to values like time  
strings orpositions with hemisphere 
characters.
.IP
\fB-\fP  
special character: this causes the value 
indicated by the next character to be 
multiplied by -1. This applies only 
to simple numeric values such as  
depth and heading and not to values   
like time strings or positions with 
hemisphere characters.
.IP
\fBC\fP
for CMP (or RP or CDP) gather number of a trace
.IP
\fBc\fP 
for trace number within a CMP (or RP or CDP) gather
.IP
\fBD\fP
for trace start delay (seconds)
.IP
\fBI\fP
for sample length (seconds)
.IP
\fBi\fP
for time interval since last trace (seconds)
.IP
\fBJ\fP
for a time string (yyyy jd hh mm ss.ssssss) 
where jd is the day of the year
.IP
\fBj\fP 
for a time string (yyyy jd dm ss.ssssss) 
where jd is the day of the year
and dm is the minute of the day
.IP
\fBL\fP 
for trace length (seconds)
.IP
\fBM\fP 
for unix (epoch) time in decimal seconds since 1/1/70 00:00:00
.IP
\fBm\fP 
for time in decimal seconds since first record
.IP
\fBN\fP 
for number of samples in the trace
.IP
\fBn\fP 
for trace counter (starts from 0)
.IP
\fBR\fP  for range from source to receiver (m)
.IP
\fBS\fP  for shot number
.IP
\fBs\fP  for trace number within a shot gather
.IP
\fBT\fP  for a time string (yyyy/mm/dd/hh/mm/ss)
.IP
\fBt\fP  for a time string (yyyy mm dd hh mm ss)
.IP
\fBU\fP  for unix time in integer seconds since 1/1/70 00:00:00
.IP
\fBu\fP  for time in integer seconds since first record
.IP
\fBV\fP  for shot or ping interval (decimal seconds)
.IP
\fBX\fP  for longitude (decimal degrees)
.IP
\fBx\fP  for longitude (degrees + decimal minutes + E/W)
.IP
\fBY\fP  for latitude (decimal degrees)
.IP
\fBy\fP  for latitude (degrees + decimal minutes + N/S)
.IP
\fBZ\fP  for source or sonar depth (positive downwards) (m)
.IP
.\fBz\fP  for depth (positive downwards) (m)
.br

Default \fIoutput_format\fP = \fBYXLZ\fP (latitude, longitude, cumulative
along-track distance, and depth).
Default \fIoutput_format\fP = \fBTiXYSsCcDINL\fP 
(time, time interval, lon, lat, shot, shot trace #, cmp, cmp trace #, 
delay, sample length, number samples, trace length)
.TP
.B \-V
Normally, \fBmbsegylist\fP works "silently" without outputting
anything to the stderr stream.  If the
\fB-V\fP flag is given, then \fBmbsegylist\fP works in a "verbose" mode and
outputs the program version being used and all error status messages.

.SH EXAMPLES
Suppose one has a subbottom profiler segy data file called 
20040722_152111.s7k.segy.

In order to obtain a listing of the shot number, time tag, and position
of the traces in the file, use \fBmbsegylist\fP as follows:
 	mbsegylist -I 20040722_152111.s7k.segy -OSTXY

The output will be as follows:

    56  2004/07/22/15/20/37.029000      -121.857289       36.775508
    57  2004/07/22/15/20/37.590000      -121.857289       36.775514
    58  2004/07/22/15/20/38.152000      -121.857289       36.775519
    59  2004/07/22/15/20/38.713000      -121.857289       36.775525
    60  2004/07/22/15/20/39.275000      -121.857289       36.775531
    61  2004/07/22/15/20/39.837000      -121.857289       36.775536
    62  2004/07/22/15/20/40.398000      -121.857289       36.775542
    63  2004/07/22/15/20/40.960000      -121.857289       36.775547
    64  2004/07/22/15/20/41.521000      -121.857289       36.775553
  	.....

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbextractsegy\fP(l), \fBmbsegyinfo\fP(l), \fBmbsegygrid\fP(l),
\fBSIOSEIS\fP(http://sioseis.ucsd.edu/ )

.SH BUGS
Si. Oui. Ya. Da. Yes. Mess.
