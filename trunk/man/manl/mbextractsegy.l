.TH mbextractsegy l "3 June 2004" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbextractsegy\fP - extracts subbottom profiler, center beam reflection,
or seismic reflection data from swath data supported by MB-System and
rewrites it as a SEGY file.

.SH VERSION
Version 5.0

.SH SYNOPSIS
\fBmbextractsegy\fP [\fB-B\fIyr/mo/da/hr/mn/sc\fP \fB-C -E\fIyr/mo/da/hr/mn/sc\fP 
\fB-F\fIformat\fP \fB-G\fP \fB-I\fIfilename\fP 
\fB-L\fIlonflip\fP \fB-M\fIlondim/latdim\fP 
\fB-N\fP \fB-O\fP \fB-P\fIping\fP 
\fB-R\fIwest/east/south/norths\fP \fB-S\fIspeed\fP \fB-W -V -H\fP]
 
\fBmbextractsegy\fP [\fB-B\fIyr/mo/da/hr/mn/sc\fP -E\fIyr/mo/da/hr/mn/sc\fP 
\fB-F\fIformat\fP \fB-I\fIfile\fP \fB-O\fIsegyfile\fP 
\fB-S\fIsampleformat\fP \fB-V -H\fP]

.SH DESCRIPTION
\fBMBextractsegy\fP is a utility for reading a swath sonar data file 
or files, extracting subbottom profiler, center beam reflection,
or seismic reflection data, and outputting that selected data 
as a SEGY file. Parameters such as sonar depth, seafloor depth, and
position are embedded into the SEGY data. The SEGY files are
output in a form used by the seismic reflection processing package
\fBSIOSEIS\fP. A "sinf" file containing statistics about the content
of the SEGY file is created automatically by \fBMBextractsegy\fP; this
file is named by adding a ".sinf" suffix to the SEGY filename.

At present, the only swath data formats from which subbottom data
can be extracted are format 88 (Reson 7k format), format 132 
(Edgetech Jstar format), and format 133 (Edgetech Jstar format).

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
Sets the data format used if the input is read from stdin
or from a file. If \fIformat\fP < 0, then the input file specified
with the \fB-I\fP option will actually contain a list of input swath sonar
data files. This program uses the \fBMBIO\fP library 
and will read or write any swath sonar
format supported by \fBMBIO\fP. A list of the swath sonar data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page. The default format is set
using the program \fBmbdefaults\fP..
.TP
.B \-H
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \-I
\fIinfile\fP
.br
Sets the input file path. If \fIformat\fP > 0 (set with the 
\fB-f\fP option or \fBmbdefaults\fP) then the swath sonar data contained in \fIinfile\fP 
is read and processed. If \fIformat\fP < 0, then \fIinfile\fP
is assumed to be an ascii file containing a list of the input swath sonar
data files to be processed and their formats.  The program will read 
the data in each one of these files.
In the \fIinfile\fP file, each
data file should be followed by a data format identifier, e.g.:
 	datafile1 11
 	datafile2 24
.br
This program uses the \fBMBIO\fP library and will read or write any swath sonar
format supported by \fBMBIO\fP. A list of the swath sonar data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page. Default: \fIinfile\fP = "stdin".
.TP
.B \-O
.br
This option sets the output SEGY file path. If \fB-O\fP is not invoked,
the output SEGY filename (or files if a datalist is being processed) will
be the input swath filename with ".segy" appended.
.TP
.B \-S
\fIsampleformat\fP
.br
Sets the sample format desired for use in the output SEGY file. Options include:
 	MB_SEGY_SAMPLEFORMAT_NONE           1
 	MB_SEGY_SAMPLEFORMAT_TRACE          2
 	MB_SEGY_SAMPLEFORMAT_ENVELOPE       3
 	MB_SEGY_SAMPLEFORMAT_ANALYTIC       4
.br
MB_SEGY_SAMPLEFORMAT_TRACE implies that the seismic trace is in the original,
most raw form with both positive and negative values. 
MB_SEGY_SAMPLEFORMAT_ANALYTIC implies that the trace has been
been transformed from the original time series to an analytic signal containing
both real and imaginary parts. Both the real and imaginary signals 
are stored in the SEGY file. MB_SEGY_SAMPLEFORMAT_ENVELOPE implies that the
analytic signal has been reduced back to a single real, positive-only envelope
signal by taking the magnitude of the real and imaginary parts at every sample.
It is important to note that this option only sets the desired sample format, and
the program may find it necessary to store the data in an alternate form. For
instance, if the user specifies MB_SEGY_SAMPLEFORMAT_TRACE or 
MB_SEGY_SAMPLEFORMAT_ANALYTIC but the swath file only contains envelope data,
the data will be written in the envelope form. The default value of
MB_SEGY_SAMPLEFORMAT_NONE allows the program to choose the most appropriate
sample format.
.TP
.B \-V
This option increases the verbosity of \fBmbextractsegy\fP, causing it
to print out messages regarding its progress to stdout.

.SH EXAMPLES
Suppose that one has collected a set of Reson 7k datafiles incorporating 
multibeam sonar data, sidescan data, and subbottom profiler data, and
that the filenames are:
 	20040722_152111.s7k
 	20040722_154429.s7k
 	20040722_160809.s7k
.br

In order to extract the subbottom data into segy files, one can use
\fBmbextractsegy\fP on each file in turn, for example:
 	mbextractsegy -I 20040722_152111.s7k
.br
which yields the following output:
 	2527 records output to segy file 20040722_154429.s7k.segy
.br
In this command the 7k file format id of 88 is inferred from the 
".7k" filename suffix.

One may also input a datalist file referencing all of the 7k files. The
contents of a datalist file (typically named datalist.mb-1) would look like:
 	20040722_152111.s7k 88
 	20040722_154429.s7k 88
 	20040722_160809.s7k 88
.br
where the second column is the data format id (see the \fBmbsystem\fP man page
for a discussion of datalist files). To operate on all the files in
a single command, use:
 	mbextractsegy -I datalist.mb-1
 which yields the following output:
 	2527 records output to segy file 20040722_154429.s7k.segy
 	2568 records output to segy file 20040722_160809.s7k.segy
 	2569 records output to segy file 20040722_163148.s7k.segy
.br
In this command the datalist format id of -1 is inferred from the 
".mb-1" filename suffix.

Use of the \fB-V\fP option causes \fBmbextractsegy\fP to output information
about every tenth record written to a segy file. So:
 	mbextractsegy -I 20040722_152111.s7k -V
.br
which yields the following output:
 	2527 records output to segy file 20040722_154429.s7k.segy
.br
 	Outputting subbottom data to segy file 20040722_152111.s7k.segy
 	file:20040722_152111.s7k record:10 shot:65  2004/204 15:20:42.083 samples:8330 interval:64 usec  minmax: 0.000000 17487.244141
 	file:20040722_152111.s7k record:20 shot:75  2004/204 15:20:47.698 samples:8330 interval:64 usec  minmax: 0.000000 16525.875000
 	......... lines omitted ..........
 	file:20040722_152111.s7k record:2510 shot:2565  2004/204 15:44:05.893 samples:8330 interval:64 usec  minmax: 0.000000 17616.189453
 	file:20040722_152111.s7k record:2520 shot:2575  2004/204 15:44:11.507 samples:8330 interval:64 usec  minmax: 0.000000 31888.093750
 	2527 records output to segy file 20040722_152111.s7k.segy
.br
\fBMBextractsegy\fP also generates a "sinf" file containing statistics about the data in the segy
file 20040722_152111.s7k.segy. The contents of the sinf file 20040722_152111.s7k.segy.sinf are:
 	SEGY Data File:      20040722_152111.s7k.segy
 	
 	File Header Info:
 	  Channels:                          1
 	  Auxilliary Channels:               0
 	  Sample Interval (usec):           64
 	  Number of Samples in Trace:     8330
 	  Trace length (sec):         0.533120
 	  Data Format:                IEEE 32 bit integer
 	  CDP Fold:                          0
 	
 	Data Totals:
 	  Number of Traces:               2527
 	  Min Max Delta:
 	    Shot number:                    56     2582     2527
 	    Shot trace:                      1        1        1
 	    RP number:                      56     2582     2527
 	    RP trace:                        1        1        1
 	    Delay (sec):              0.000000 0.000000 0.000000
 	    Range (m):                0.000000 0.000000 0.000000
 	    Receiver Elevation (m):   -224.030000 -2.860000 -221.170000
 	    Source Elevation (m):     -224.030000 -2.860000 -221.170000
 	    Source Depth (m):         2.860000 224.030000 -221.170000
 	    Receiver Water Depth (m): 51.510000 487.670000 -436.160000
 	    Source Water Depth (m):   51.510000 487.670000 -436.160000
 	
 	Navigation Totals:
 	
 	  Start of Data:
 	    Time:  07 22 2004 15:20:37.029000  JD204
 	    Lon: -121.8573     Lat:   36.7755
 	
 	  End of Data:
 	    Time:  07 22 2004 15:44:15.438000  JD204
 	    Lon: -121.8572     Lat:   36.7952 
 	
 	Limits:
 	  Minimum Longitude:    -121.8574   Maximum Longitude:    -121.8572
 	  Minimum Latitude:       36.7755   Maximum Latitude:       36.7952



.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbsegyinfo\fP(l), \fBmbsegylist\fP(l), \fBmbsegygrid\fP(l),
\fBSIOSEIS\fP(http://sioseis.ucsd.edu/ )

.SH BUGS
Doesn't support all of the data formats containing subbottom or center beam
reflection data..
