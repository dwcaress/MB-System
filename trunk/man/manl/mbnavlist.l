.TH mbnavlist l "21 November 2004" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbnavlist\fP - List navigation data in swath sonar data files.

.SH VERSION
Version 5.0

.SH SYNOPSIS
\fBmbnavlist\fP [\fB-B\fP\fIyr/mo/da/hr/mn/sc \fP
\fB-E\fP\fIyr/mo/da/hr/mn/sc\fP \fB-F\fP\fIformat\fP \fB-H -I\fP\fIfile \fP
\fB-L\fP\fIlonflip\fP \fB-N\fP\fInavchannel\fP
\fB-O\fP\fIoutput_format\fP \fB-R\fP\fIwest/east/south/north\fP 
\fB-S\fP\fIspeed\fP \fB-T\fP\fItimegap\fP \fB-V\fP \fB-Z\fP\fIsegment\fP]

.SH DESCRIPTION
\fBmbnavlist\fP prints the specified contents of navigation records
in a swath sonar data file to stdout. By default, \fBmbnavlist\fP 
produces ASCII files in
spreadsheet style, with data columns separated by tabs. Alternatively,
the output can be binary, with each field represented 
as a double precision float (\fB-A\fP option). The output of
\fBmbnavlist\fP can be piped to plotting and data analysis programs. The
option \fB-O\fP\fIoutput_format\fP can be 
used to control the data types that
are sent to stdout. 
By default, \fBmbnavlist\fP extracts navigation from the 
data record type listed as the "navigation source" for a particular
data format. Swath data formats may contain synchronous navigation
(navigation is included in survey data records), asynchronous navigation
(navigation comes in records separate from and at different times
than the survey data records), or both (survey records contain merged
navigation while the original asynchronous navigation is preserved in
separate navigation records). Some data formats provide for multiple
navigation sources, so that there are more than one channel of
asynchronous data records. In this case one channel is always designated
as the primary or active navigation channel. The \fB-N\fP option allows
the user to specify that \fBmbnavlist\fP outputs navigation from
the primary asynchronous navigation (\fB-N\fP\fI0\fP) or from one
of three auxilliary navigation channels 
(\fB-N\fP\fI1\fP, \fB-N\fP\fI2\fP, or \fB-N\fP\fI3\fP). If the data
format only uses asynchronous navigation, then the default
navigation source will already be the primary asynchronous navigation
and \fB-N\fP need not be used. If \fB-N\fP is used but the data file
does not contain asynchronous navigation or the specified auxilliary
navigation channels, then \fBmbnavlist\fP will output nothing. To
determine if a datafile contains asynchronous navigation and/or auxilliary
navigation, run \fBmbinfo\fP using its \fB-N\fP option to get counts
of all data record types in the file.

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
.B \-B
\fIyr/mo/da/hr/mn/sc\fP
.br
This option sets the starting time for data allowed in the input data.
The \fB-E\fP option sets the ending time for data. If the 
starting time is before the ending time, then any data
with a time stamp before the starting time or after the
ending time is ignored. If instead the starting time is
after the ending time, then any data between the ending
and starting time will be ignored. This scheme allows time
windowing both inside and outside a specified interval.
Default: \fIyr/mo/da/hr/mn/sc\fP = 1962/2/21/10/30/0.
.TP
.B \-E
\fIyr/mo/da/hr/mn/sc\fP
.br
This option sets the ending time for data allowed in the input data.
The \fB-B\fP option sets the starting time for data. If the 
starting time is before the ending time, then any data
with a time stamp before the starting time or after the
ending time is ignored. If instead the starting time is
after the ending time, then any data between the ending
and starting time will be ignored. This scheme allows time
windowing both inside and outside a specified interval.
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
is given in the \fBMBIO\fP manual page. 
Default: \fIfile\fP = "datalist.mb-1".
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
.B \-N
\fInavchannel\fP
.br
Sets \fBmbnavlist\fP to look for navigation in navigation records rather
than the record type listed as the "navigation source" for a particular
data format. Swath data formats may contain synchronous navigation
(navigation is included in survey data records), asynchronous navigation
(navigation comes in records separate from and at different times
than the survey data records), or both (survey records contain merged
navigation while the original asynchronous navigation is preserved in
separate navigation records). Some data formats provide for multiple
navigation sources, so that there are more than one channel of
asynchronous data records. In this case one channel is always designated
as the primary or active navigation channel. The \fB-N\fP option allows
the user to specify that \fBmbnavlist\fP outputs navigation from
the primary asynchronous navigation (\fB-N\fP\fI0\fP) or from one
of three auxilliary navigation channels 
(\fB-N\fP\fI1\fP, \fB-N\fP\fI2\fP, or \fB-N\fP\fI3\fP). If \fB-N\fP is
used but the data file
does not contain asynchronous navigation or the specified auxilliary
navigation channels, then \fBmbnavlist\fP will output nothing. To
determine if a datafile contains asynchronous navigation and/or auxilliary
navigation, run \fBmbinfo\fP using its \fB-N\fP option to get counts
of all data record types in the file.
.TP
.B \-O
\fIoutput_format\fP
.br
Determines the form of the output. \fIOutput_format\fP is a string composed
of one or more of the following characters:

 	\fBc\fP  for sonar depth (meters)
 	\fBH\fP  for heading (degrees)
 	\fBh\fP  for course made good (degrees)
  	\fBJ\fP  for a time string (yyyy jd hh mm ss.ssssss) 
                 where jd is the julian day of the year
  	\fBj\fP  for a time string (yyyy jd dm ss.ssssss) 
                 where jd is the julian day of the year
                 and dm is the minute of the day
 	\fBL\fP  for cumulative along-track distance (km)
 	\fBl\fP  for cumulative along-track distance (m)
 	\fBM\fP  for unix time in decimal seconds since 1/1/70 00:00:00
 	\fBm\fP  for time in decimal seconds since first record
 	\fBN\fP  for ping count
 	\fBP\fP  for pitch in degrees
 	\fBp\fP  for draft in meters
 	\fBR\fP  for roll in degrees
 	\fBr\fP  for heave in meters
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
Normally, \fBmbnavlist\fP works "silently" without outputting
anything to the stderr stream.  If the
\fB-V\fP flag is given, then \fBmbnavlist\fP works in a "verbose" mode and
outputs the program version being used and all error status messages.
.TP
.B \-Z
\fIsegment\fP
.br
Causes the ascii output of different input swath files
(e.g. when a datalist is specified with the \fB-I\fP option)
to be separated by lines with \fIsegment\fP. If \fIsegment\fP
is a single character, then the output is a multiple segment
file of the sort accepted by the \fBGMT\fP program \fBpsxy\fP.
This option only works with ascii output, and is thus disabled
when the \fB-A\fP option is specified. The most common usage
is \fB-Z\fP\I>\fP.

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
