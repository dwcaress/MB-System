.TH mbnavlist l "4 January 2001" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbnavlist\fP - List navigation data in swath sonar data files.

.SH VERSION
Version 5.0

.SH SYNOPSIS
\fBmbnavlist\fP [\fB-B\fIyr/mo/da/hr/mn/sc 
\fB-E\fIyr/mo/da/hr/mn/sc \fB-F\fIformat \fB-H \fB-I\fIfile 
\fB-L\fIlonflip
\fB-O\fIoutput_format \fB-R\fIwest/east/south/north 
\fB-S\fIspeed \fB-T\fItimegap \fB-V\fP]

.SH DESCRIPTION
\fBmbnavlist\fP prints the specified contents of navigation records
in a swath sonar data file to stdout. The form of the 
output is quite flexible; mbnavlist is tailored to produce 
ascii files in spreadsheet style with data columns separated by tabs.

.SH AUTHORSHIP
David W. Caress (caress@mbari.org)
.br
  Monterey Bay Aquarium Research Institute
.br
Dale N. Chayes (dale@ldeo.columbia.edu)
.br
  Lamont-Doherty Earth Observatory
.br

.SH OPTIONS
.TP
.B \-B
\fIyr/mo/da/hr/mn/sc\fP
.br
Sets the starting time for data allowed in the input data; pings
with times before the starting time will be ignored. 
Default: \fIyr/mo/da/hr/mn/sc\fP = 1962/2/21/10/30/0.
.TP
.B \-E
\fIyr/mo/da/hr/mn/sc\fP
.br
Sets the ending time for data allowed in the input data; pings
with times after the ending time will be ignored. 
Default: \fIyr/mo/da/hr/mn/sc\fP = 2062/2/21/10/30/0.
.TP
.B \-F
\fIformat\fP
.br
Sets the format for the input swath sonar data using 
\fBMBIO\fP integer format identifiers. 
This program uses the \fBMBIO\fP library and will read any swath sonar
format supported by \fBMBIO\fP. A list of the swath sonar data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page. Default: \fIformat\fP = 11.
.TP
.B \-H
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \-I
\fIfile\fP
.br
Sets the input filename. If \fIformat\fP > 0 (set with the 
\fB-F\fP option) then the swath sonar data contained in \fIfile\fP 
is read and processed. If \fIformat\fP < 0, then \fIfile\fP
is assumed to be an ascii file containing a list of the input swath sonar
data files to be processed and their formats.  The program will read 
the data in each one of these files.
In the \fIfile\fP file, each
data file should be followed by a data format identifier, e.g.:
 	datafile1 11
 	datafile2 24
.br
This program uses the \fBMBIO\fP library and will read or write any swath sonar
format supported by \fBMBIO\fP. A list of the swath sonar data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page. Default: \fIfile\fP = "stdin".
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
Determines the form of the output. \fIOutput_format\fP is a string composed
of one or more of the following characters:

 	\fBH\fP  for heading (degrees)
 	\fBh\fP  for course made good (degrees)
  	\fBJ\fP  for a time string (yyyy jd hh mm ss.ssssss) 
                 where jd is the julian day of the year
  	\fBj\fP  for a time string (yyyy jd dm ss.ssssss) 
                 where jd is the julian day of the year
                 and dm is the minute of the day
 	\fBL\fP  for cumulative along-track distance (km)
 	\fBM\fP  for unix time in decimal seconds since 1/1/70 00:00:00
 	\fBm\fP  for time in decimal seconds since first record
 	\fBN\fP  for ping count
  	\fBS\fP  for speed (km/hr)
  	\fBs\fP  for speed made good (km/hr)
  	\fBT\fP  for a time string (yyyy/mm/dd/hh/mm/ss)
 	\fBt\fP  for a time string (yyyy mm dd hh mm ss)
 	\fBU\fP  for unix time in integer seconds since 1/1/70 00:00:00
 	\fBu\fP  for time in integer seconds since first record
 	\fBV\fP  for ping interval (decimal seconds)
 	\fBX\fP  for longitude (decimal degrees)
 	\fBx\fP  for longitude (degrees + decimal minutes + E/W)
 	\fBY\fP  for latitude (decimal degrees)
 	\fBy\fP  for latitude (degrees + decimal minutes + N/S)

Default \fIoutput_format\fP = \fBtMXYHs\fP (time, unix time, latitude, 
longitude, heading, speed).
.TP
.B \-R
\fIwest/east/south/north\fP
.br
Sets the longitude and latitude bounds within which swath sonar 
data will be read. Only the data which lies within these bounds will
be read. 
Default: \fIwest\fP=-360, east\fI=360\fP, \fIsouth\fP=-90, \fInorth\fP=90.
.TP
.B \-S
\fIspeed\fP
.br
Sets the minimum speed in km/hr (5.5 kts ~ 10 km/hr) allowed in 
the input data; pings associated with a smaller ship speed will not be
copied. Default: \fIspeed\fP = 0.
.TP
.B \-T
\fItimegap\fP
.br
Sets the maximum time gap in minutes between adjacent pings allowed before
the data is considered to have a gap. Default: \fItimegap\fP = 1.
.TP
.B \-V
Normally, \fBmblist\fP works "silently" without outputting
anything to the stderr stream.  If the
\fB-V\fP flag is given, then \fBmblist\fP works in a "verbose" mode and
outputs the program version being used and all error status messages.

.SH EXAMPLES
Suppose one wishes to obtain a navigation list from a Simrad EM300 data file
in the MBARI format (MBIO id 57) called mbari_1998_107_msn.mb57. To 
obtain a listing with time in unix second forms followed
by longitude and latitude, the following will suffice:

 	mbnavlist -F57 -i mbari_1998_107_msn.mb57 -OMXY | more

The output will be as follows:

 	889125106.792000        -155.898471       19.979325
 	889125108.148000        -155.898586       19.979400
 	889125109.496000        -155.898738       19.979454
 	889125110.852000        -155.898876       19.979504
 	889125112.207000        -155.899020       19.979544
 	889125113.571000        -155.899204       19.979591
 	889125114.921000        -155.899479       19.979485
 	.....

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbinfo\fP(l)

.SH BUGS
\fBmbnavlist\fP is not able to list all of the navigation information 
available in some swath data formats.
