.TH mbhistogram l "14 February 1995"
.SH NAME
\fBmbhistogram\fP - Generate a histogram of bathymetry, amplitude, or sidescan values.

.SH VERSION
Version 4.2

.SH SYNOPSIS
\fBmbhistogram\fP [\fB-A\fIkind\fP \fB-B\fIyr/mo/da/hr/mn/sc\fP 
\fB-C\fP \fB-D\fImin/max\fP \fB-E\fIyr/mo/da/hr/mn/sc\fP 
\fB-F\fIformat\fP \fB-I\fIfilename\fP \fB-L\fIlonflip\fP 
\fB-M\fInintervals\fP \fB-N\fInbins\fP \fB-P\fIping\fP 
\fB-R\fIwest/east/south/north\fP \fB-S\fIspeed\fP \fB-V -H\fP]

.SH DESCRIPTION
\fBmbhistogram\fP reads a multibeam data file and generates a histogram
of the bathymetry,  amplitude,  or sidescan values. Alternatively, 
mbhistogram can output a list of values which break up the
distribution into equal sized regions. 
The results are dumped to stdout.

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
.B \fB-A\fIkind\fP
Sets the kind of data to be processed, where 
 	\fIkind\fP = 0 : histogram of bathymetry data
 	\fIkind\fP = 1 : histogram of beam amplitude data
 	\fIkind\fP = 2 : histogram of sidescan data
Default: \fIkind\fP = 2.
.TP
.B \fB-B\fIyr/mo/da/hr/mn/sc\fP
Sets the starting time for data allowed in the input data; pings
with times before the starting time will be ignored. 
Default: \fIyr/mo/da/hr/mn/sc\fP = 1962/2/21/10/30/0.
.TP
.B \fB-D\fImin/max\fP
Specifies the minimum and maximum data values used for
constructing the histogram. 
The width of the bins is determined by (\fImax\fP - \fImin\fP)
divided by \fInbins\fP (see the \fB-N\fP option).
The \fImin\fP value is the
center of the first histogram bin, and the \fImax\fP value is the
center of the last histogram bin.
Data values falling outside the range of the bins are ignored.
.TP
.B \fB-E\fIyr/mo/da/hr/mn/sc\fP
Sets the ending time for data allowed in the input data; pings
with times after the ending time will be ignored. 
Default: \fIyr/mo/da/hr/mn/sc\fP = 2062/2/21/10/30/0.
.TP
.B \fB-F\fIformat\fP
Sets the data format used if the input is read from stdin
or from a file. If \fIformat\fP < 0, then the input file specified
with the \fB-I\fP option will actually contain a list of input multibeam
data files. This program uses the \fBMBIO\fP library 
and will read or write any multibeam
format supported by \fBMBIO\fP. A list of the multibeam data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page. Default: \fIformat\fP = 11.
.TP
.B \fB-I\fIfilename\fP
Sets the input filename. If \fIformat\fP > 0 (set with the 
\fB-f\fP option) then the multibeam data contained in \fIinfile\fP 
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
.B \fB-M\fInintervals\fP
If \fInintervals\fP > 0 then \fBmbhistogram\fP will 
calculate and output a list of values which break up
the data distribution into equal sized regions.  This can
be useful for constructing color tables which accentuate
features in amplitude and sidescan data (the macro \fBmbm_plot\fP
makes use of this feature). If this option is enabled, then the
raw histogram is not output.
.TP
.B \fB-N\fInbins\fP
Sets the number of bins used to calculate the histogram.
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
Normally, \fBmbhistogram\fP only prints out the statistics obtained
by reading all of the data.  If the
\fB-V\fP flag is given, then \fBMBCOPY\fP works in a "verbose" mode and
outputs the program version being used and all read error status messages.
.TP
.B \fB-H\fP
This "help" flag cause the program to print out a description
of its operation and then exit immediately.

.SH EXAMPLES
Suppose one wishes to obtain a histogram of the sidescan data
in a file called mbexample.mb41 containing raw data from a SeaBeam
2112 sonar (format 41). The following will suffice:
 mbhistogram -F41 -I sb199411211212.rec -D0.0/45000 -N25
.break
to yield the following histogram:
 0.000000 183814
 1875.000000 30845
 3750.000000 5365
 5625.000000 1918
 7500.000000 951
 9375.000000 591
 11250.000000 403
 13125.000000 232
 15000.000000 189
 16875.000000 127
 18750.000000 108
 20625.000000 75
 22500.000000 85
 24375.000000 48
 26250.000000 30
 28125.000000 31
 30000.000000 28
 31875.000000 21
 33750.000000 10
 35625.000000 9
 37500.000000 4
 39375.000000 2
 41250.000000 2
 43125.000000 2
 45000.000000 1
.break
In order to obtain 16 values breaking up the distribution into
equal sized regions, use the \fB-M\fP option:
 mbhistogram -F41 -I sb199411211212.rec -D0.0/45000 -N45000 -M25
.break
giving:
  -0.500011
 110.657221
 137.446811
 165.916284
 199.635469
 239.900007
 287.726270
 345.615610
 418.142114
 513.487776
 639.986551
 824.475879
 1140.372280
 1994.280032
 44803.495633

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbm_plot\fP(l)

.SH BUGS
You find em, we fix 'em...
