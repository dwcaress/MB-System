.TH mblist l "25 March 1997" "MB-System 4.5" "MB-System 4.5"
.SH NAME
\fBmblist\fP - List data in multibeam bathymetry data files.

.SH VERSION
Version 4.5

.SH SYNOPSIS
\fBmblist\fP [\fB-B\fIyr/mo/da/hr/mn/sc \fB-D\fIdumpmode 
\fB-E\fIyr/mo/da/hr/mn/sc \fB-F\fIformat \fB-H \fB-I\fIinfilename 
\fB-L\fIlonflip \fB-M\fIstart_beam/end_beam \fB-N\fIstart_pixel/end_pixel 
\fB-O\fIoutput_format \fB-P\fIpings \fB-Q\fP \fB-R\fIwest/east/south/north 
\fB-S\fIspeed \fB-T\fItimegap \fB-V\fP \fB-W\fP]

.SH DESCRIPTION
\fBmblist\fP is a utility to list the contents of a multibeam 
data file or files to stdout. \fBmblist\fP is tailored to 
produce ASCII files in
spreadsheet style, with data columns separated by tabs. The output of
\fBmblist\fP can be piped to plotting and data analysis programs. The
option \fB-O\fP\fIoutput_format\fP can be used to control the data types that
are sent to stdout. The data input may be averaged or windowed in time
and space before it is listed.  Complete dumps of bathymetry,
amplitude, or sidescan data are possible as well.

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
.B \fB-B\fIyr/mo/da/hr/mn/sc\fP
Sets the starting time for data allowed in the input data; pings
with times before the starting time will be ignored. 
Default: \fIyr/mo/da/hr/mn/sc\fP = 1962/2/21/10/30/0.
.TP
.B \fB-D\fIdumpmode\fP
Normally, the output format is controlled by the \fB-O\fP option and
the number of beams or pixels which are output is controlled by
the \fB-M\fP and \fB-N\fP options.  The \fB-D\fP option provides
a short cut for producing complete dumps of the longtitude and
latitude locations of all beams or pixels along with the
associated bathymetry, topography, amplitude, or sidescan values.
The "lon lat value" triples are often useful for input into
gridding programs (e.g. the \fBGMT\fP program \fBsurface\fP) or
other utilities. All valid (positive) values will be output, unless
the \fB-Q\fP option is used to disable value checking.
The \fIdumpmode\fP options are:
        \fIdumpmode\fP = 1:  format controlled by \fB-O\fP option
        \fIdumpmode\fP = 2:  longitude latitude depth
        \fIdumpmode\fP = 3:  longitude latitude topography
        \fIdumpmode\fP = 4:  longitude latitude amplitude
        \fIdumpmode\fP = 5:  longitude latitude sidescan
.br
Use of the \fB-D\fP option supercedes the \fB-O\fP, \fB-M\fP,
and \fB-N\fP options.
Default: \fImode\fP = 1.
.TP
.B \fB-E\fIyr/mo/da/hr/mn/sc\fP
Sets the ending time for data allowed in the input data; pings
with times after the ending time will be ignored. 
Default: \fIyr/mo/da/hr/mn/sc\fP = 2062/2/21/10/30/0.
.TP
.B \fB-F\fIformat\fP
Sets the format for the input multibeam data using 
\fBMBIO\fP integer format identifiers. 
This program uses the \fBMBIO\fP library and will read any multibeam
format supported by \fBMBIO\fP. A list of the multibeam data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page. Default: \fIformat\fP = 11.
.TP
.B \fB-H\fP
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \fB-I\fIfilename\fP
Sets the input filename. If \fIformat\fP > 0 (set with the 
\fB-F\fP option) then the multibeam data contained in \fIinfile\fP 
is read and processed. If \fIformat\fP < 0, then \fIinfile\fP
is assumed to be an ascii file containing a list of the input multibeam
data files to be processed and their formats.  The program will read 
the data in each one of these files.
In the \fIinfile\fP file, each
data file should be followed by a data format identifier, e.g.:
 	datafile1 11
 	datafile2 24
.br
This program uses the \fBMBIO\fP library and will read or write any multibeam
format supported by \fBMBIO\fP. A list of the multibeam data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page. Default: \fIinfile\fP = "stdin".
.TP
.B \fB-L\fIlonflip\fP
Sets the range of the longitude values returned.
If \fIlonflip\fP=-1 then the longitude values will be in
the range from -360 to 0 degrees. If \fIlonflip\fP=0 
then the longitude values will be in
the range from -180 to 180 degrees. If \fIlonflip\fP=1 
then the longitude values will be in
the range from 0 to 360 degrees.
Default: \fIlonflip\fP = 0.
.TP
.B \fB-M\fIstart_beam/end_beam\fP
Determines the range of bathymetry beams for which records will be output.
Beam numbers start with zero on the port side. The default is 
to output a single record for each ping corresponding to the
center beam.
.TP
.B \fB-N\fIstart_pixel/end_pixel\fP
Determines the range of sidescan pixels for which records will be output.
Pixel numbers start with zero on the port side. The default is 
to not output records associated with sidescan pixels. Instead, by
default a single record is outputted for each ping corresponding to the
center bathymetry beam; 
.TP
.B \fB-O\fIoutput_format\fP
Determines the form of the output. \fIOutput_format\fP is a string composed
of one or more of the following characters:

 	\fBA\fP  for apparent seafloor crosstrack slope 
  		 (degrees from horizontal with positive
 		 slopes dipping toward port) - calculated 
 		 by fitting a line to the the
 		 bathymetry data of each ping.
 	\fBB\fP  for amplitude
 	\fBb\fP  for sidescan
 	\fBD\fP  for bathymetry acrosstrack distance (m)
 	\fBd\fP  for sidescan acrosstrack distance (m)
 	\fBE\fP  for bathymetry alongtrack distance (m)
 	\fBe\fP  for sidescan alongtrack distance (m)
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
 	\fBZ\fP  for topography (positive upwards) (m)
 	\fBz\fP  for depth (positive downwards) (m)
 	\fB#\fP  for beam or pixel number

Default \fIoutput_format\fP = \fBYXLZ\fP (latitude, longitude, cumulative
along-track distance, and depth).
.TP
.B \fB-P\fIpings\fP
Sets the ping averaging of the input data. If \fIpings\fP = 1, then
no ping averaging is performed. If \fIpings\fP > 0, then
that number of input pings will be averaged to produce one output
ping.  If \fIpings\fP = 0, then the ping averaging will automatically
be done so that the along-track ping spacing is equal to the across-track
beam spacing.
Default: \fIpings\fP = 1 (no ping averaging).
.TP
.B \fB-Q\fP
Disables value checking for validity (only positive bathymetry,
amplitude, and sidescan values are valid).  This allows dumps
of all of the data, including zero or flagged beams and pixels.
.TP
.B \fB-R\fIwest/east/south/north\fP
Sets the longitude and latitude bounds within which multibeam 
data will be read. Only the data which lies within these bounds will
be read. 
Default: \fIwest\fP=-360, east\fI=360\fP, \fIsouth\fP=-90, \fInorth\fP=90.
.TP
.B \fB-S\fIspeed\fP
Sets the minimum speed in km/hr (5.5 kts ~ 10 km/hr) allowed in 
the input data; pings associated with a smaller ship speed will not be
copied. Default: \fIspeed\fP = 0.
.TP
.B \fB-T\fItimegap\fP
Sets the maximum time gap in minutes between adjacent pings allowed before
the data is considered to have a gap. Default: \fItimegap\fP = 1.
.TP
.B \fB-V\fP
Normally, \fBmblist\fP works "silently" without outputting
anything to the stderr stream.  If the
\fB-V\fP flag is given, then \fBmblist\fP works in a "verbose" mode and
outputs the program version being used and all error status messages.
.TP
.B \fB-W\fP
Normally, \fBmblist\fP outputs bathymetry and across and along
track distances in meters.  If the
\fB-W\fP flag is given, then \fBmblist\fP outputs these values
in feet.

.SH EXAMPLES
Suppose one wishes to obtain a centerbeam profile from a raw Hydrosweep file
(format 21) in a region between 105W and 103W 
longitude and between 10S and 8S latitude. The following will suffice:

	mblist -Iinfile -F21 -R-105/-103/-10/-8 -OLz

The output will be as follows:

 	0.000  4378
 	0.085  4370
 	0.166  4370
 	0.247  4351
 	0.330  4353
 	0.407  4337
 	0.492  4334
 	0.571  4323
 	0.651  4316
 	0.737  4307
 	.....

Suppose one wishes instead to obtain time, heading and speed data
in the same file from 8AM to 9AM on August 10 1991. The following
is appropriate:

 	mblist -Iinfile -F21 -B1991/8/10/8/0/0
 	-E1991/8/10/9/0/0 -OTHS

The output will be as follows:

 	1991/08/10/08/00/05     283.9   41.29
 	1991/08/10/08/00/19     283.4   20.36
 	1991/08/10/08/00/33     285.1   20.36
 	1991/08/10/08/00/48     286.7   20.09
 	1991/08/10/08/01/02     284.9   20.08
 	1991/08/10/08/01/16     285.2   20.02
 	1991/08/10/08/01/44     284.2   20.20
 	1991/08/10/08/02/12     283.7   20.50
 	1991/08/10/08/02/41     283.6   20.75
 	1991/08/10/08/03/09     285.1   21.19
 	.....

Suppose one wishes a data series with along-track distance,
topography and across-track distance of beam number 15 for the same file
and time limits as above:

 	mblist -Iinfile -F21 -B1991/8/10/7/0/0
 	-E1991/8/10/9/0/0 -OLZD -M15/15

The output will be as follows:

 	  0.000  4510   -1704
 	  0.172  4494   -1692
 	  0.260  4486   -1689
 	  0.343  4471   -1683
 	  0.427  4491   -1691
 	  0.506  4490   -1690
 	  0.591  4478   -1686
 	  0.676  4505   -1697
 	  0.763  4488   -1695
 	  0.849  4495   -1699
 	  .....

Supppose one wishes to obtain longitude, latitude, and
depth at the centerbeam as x-y-z data for the same region as in the
first example:

 	mblist -Iinfile -F21 -R-105/-103/-10/-8 -OXYz

The output will be as follows:

 	-103.000236      -9.577439       4378
 	-103.000943      -9.577229       4370
 	-103.001651      -9.577020       4370
 	-103.002372      -9.576794       4351
 	-103.003041      -9.576584       4353
 	-103.003771      -9.576338       4337
 	-103.004456      -9.576105       4334
 	-103.005153      -9.575895       4323
 	-103.005903      -9.575679       4316
 	-103.006586      -9.575449       4307
 	.....

Suppose one wishes to obtain a dump of longitude, latitude, and
depth for all good beams in a Hydrosweep data file. There are two ways to
obtain this output. One can explicitly specify the output format as
\fB-O\fP\fIXYz\fP and the output beams as \fB-M\fP\fI0/58\fP:

 	mblist -Iinfile -F21 -OXYz -M0/58

or one can use the equivalent \fB-D\fP\fI2\fP shortcut:

	mblist -Iinfile -F21 -D2

Either way, the output is as follows:

 	-49.296454	  12.180552	  4866
 	-49.296695	  12.178668	  4858
 	-49.296923	  12.176893	  4855
 	-49.297123	  12.175341	  4877
 	-49.297319	  12.173808	  4895
 	-49.297536	  12.172122	  4879
 	-49.297744	  12.170498	  4865
 	-49.297909	  12.169216	  4904
 	-49.298100	  12.167727	  4899
 	-49.298299	  12.166175	  4871
 	-49.298476	  12.164803	  4873
 	-49.298639	  12.163530	  4891
 	.....

Suppose one wishes to obtain a dump of longitude, latitude, and
depth for all beams, valid or not, in a Hydrosweep data file. 
The approach is the same as the preceding example, except that
the \fB-Q\fP option is used to disable validity checking of
beam values. One can explicitly specify the output format as
\fB-O\fP\fIXYz\fP and the output beams as \fB-M\fP\fI0/58\fP:

 	mblist -Iinfile -F21 -OXYz -M0/58 -Q

or one can use the equivalent \fB-D\fP\fI2\fP shortcut:

	mblist -Iinfile -F21 -D2 -Q

Either way, the output includes both zero beams (no data) and
beams with negative depths (flagged as bad data):

 	-49.301094	  12.144409	     0
 	-49.301094	  12.144409	     0
 	-49.296454	  12.180552	  4866
 	-49.296695	  12.178668	  4858
 	-49.296923	  12.176893	  4855
 	-49.297123	  12.175341	  4877
 	-49.297319	  12.173808	  4895
 	-49.297536	  12.172122	  4879
 	-49.297744	  12.170498	  4865
 	-49.297909	  12.169216	  4904
 	-49.298100	  12.167727	  4899
 	-49.298100	  12.167727	 -4144
 	-49.298299	  12.166175	  4871
 	-49.298476	  12.164803	  4873
 	-49.298639	  12.163530	  4891
 	.....

Finally, suppose one wishes to obtain a dump of longitude, latitude, and
amplitude for all good beams in a Hydrosweep data file. There are two ways to
obtain this output. One can explicitly specify the output format as
\fB-O\fP\fIXYB\fP and the output beams as \fB-M\fP\fI0/58\fP:

 	mblist -Iinfile -F21 -OXYB -M0/58

or one can use the equivalent \fB-D\fP\fI4\fP shortcut:

	mblist -Iinfile -F21 -D4

Either way, the output is as follows:

 	-49.296454	  12.180552	    13
 	-49.296695	  12.178668	    17
 	-49.296923	  12.176893	    16
 	-49.297123	  12.175341	    14
 	-49.297319	  12.173808	    17
 	-49.297536	  12.172122	     9
 	-49.297744	  12.170498	    14
 	-49.297909	  12.169216	    15
 	-49.298100	  12.167727	    12
 	-49.298299	  12.166175	    12
 	-49.298476	  12.164803	    28
 	-49.298639	  12.163530	    14
 	.....

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbinfo\fP(l)

.SH BUGS
\fBmblist\fP is not able to list all of the information available in some
multibeam formats.
