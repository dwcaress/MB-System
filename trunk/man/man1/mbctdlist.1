.TH mbctdlist l "19 September 2008" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbctdlist\fP - List CTD data in swath sonar data files.

.SH VERSION
Version 5.0

.SH SYNOPSIS
\fBmbctdlist\fP [\fB-A\fP \fB-D\fP\fIdecimate\fP \fB-F\fP\fIformat\fP 
\fB-G\fP\fIdelimeter\fP \fB-H\fP \fB-I\fP\fIfile \fP
\fB-O\fP\fIoutput_format\fP \fB-V\fP \fB-Z\fP\fIsegment\fP]

.SH DESCRIPTION
\fBMBctdlist\fP prints the specified contents of CTD records
in a swath sonar data file to stdout. CTD stands for conductivity,
temperature, and depth (pressure). Some seafoor mapping sonar platforms 
(e.g. ROVs or AUVs) carry CTDs. The CTD data may be included in the 
sonar data stream to enable calculation of water sound speed for use
in sonar settings and/or data processing. In addition to the raw
conductivity (S/m), temperature (degree C), and pressure (dBar) data,
salinity (ppu), depth (m), and water sound speed (m/s) are typically
available by calculation.\fBMBctdlist\fP can also output navigation
and time stamp values. By default, \fBmbctdlist\fP  produces ASCII files in
spreadsheet style, with data columns separated by tabs. Alternatively,
other column delimiters can be used (\fB-G\fP option), or
the output can be binary, with each field represented 
as a double precision float (\fB-A\fP option). The
option \fB-O\fP\fIoutput_format\fP is 
used to control the data types that are sent to stdout. The output stream can
be decimated using the \fB-D\fP option. The output of
\fBmbctdlist\fP can be piped to plotting and data analysis programs.  

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
.B \-D
\fIdecimate\fP
.br
Sets the decimation of the output data. By default (i.e. \fIdecimate\fP=1), 
every available data record is output. If \fIdecimate\fP>1, then only 
every "\fIdecimate\fP"th record will be output. Default: \fIdecimate\fP=1.
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
.B \-G
\fIdelimiter\fP
.br
Sets the character(s) used to separate output fields when ascii
columns are output. Default: tabs are used as delimiters.
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
 	datafile.mb88 88
 	datafile.mb88 88
.br
This program uses the \fBMBIO\fP library and will read any swath sonar
format supported by \fBMBIO\fP. However, only files in formats supporting
CTD data will contain relevant data and produce an output.
A list of the swath sonar data formats
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
.B \-O
\fIoutput_format\fP
.br
Determines the form of the output. \fIOutput_format\fP is a string composed
of one or more of the following characters:

 	\fBC\fP  for conductivity (S/m)
 	\fB^C\fP for sonar altitude (meters)
 	\fBc\fP  for temperature (degree C)
 	\fB^c\fP for sonar depth (meters)
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
  	\fBS\fP  for salinity (ppu)
  	\fB^S\fP for speed (km/hr)
  	\fBs\fP  for water sound speed (m/s)
  	\fB^s\fP for speed made good (km/hr)
  	\fBT\fP  for a time string (yyyy/mm/dd/hh/mm/ss)
 	\fBt\fP  for a time string (yyyy mm dd hh mm ss)
 	\fBU\fP  for unix time in integer seconds since 1/1/70 00:00:00
 	\fBu\fP  for time in integer seconds since first record
 	\fBV\fP  for ping interval (decimal seconds)
 	\fBX\fP  for longitude (decimal degrees)
 	\fBx\fP  for longitude (degrees + decimal minutes + E/W)
 	\fBY\fP  for latitude (decimal degrees)
 	\fBy\fP  for latitude (degrees + decimal minutes + N/S)

Default \fIoutput_format\fP = \fBTXYCc^cSs\fP (Time, lon, lat, conductivity, 
temperature, depth, salinity, sound speed).
.TP
.B \-V
Normally, \fBmbctdlist\fP works "silently" without outputting
anything to the stderr stream.  If the
\fB-V\fP flag is given, then \fBmbctdlist\fP works in a "verbose" mode and
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
Suppose one wishes to obtain a ctd list from a Reson 7125 data file
in the 7k format (MBIO id 88) called 20080904_231809p.mb88. To 
obtain a listing with time in unix second forms followed
by longitude, latitude, and salinity, the following will suffice:

 	mbctdlist -i 20080904_231809p.mb88 -OMXYS | more

The output will be as follows:

 	1220570288.486000       -129.066699       47.997246     34.305
 	1220570288.685999       -129.066702       47.997245     34.304
 	1220570288.885999       -129.066706       47.997244     34.305
 	1220570289.086000       -129.066710       47.997243     34.305
 	1220570289.286000       -129.066713       47.997242     34.304
 	.....

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbinfo\fP(l)

.SH BUGS
\fBmbctdlist\fP initially only works with data in the MBF_RESON7KR format (MBIO format id 88)
supporting Reson 7k series multibeam sonars.
