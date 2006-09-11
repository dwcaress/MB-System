.TH mb7kpreprocess l "13 August 2006" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmb7kpreprocess\fP - performs preprocessing of Reson 7k multibeam data 
in the 7k format (\fBMBIO\fP format 88).

.SH VERSION
Version 5.0

.SH SYNOPSIS
\fBmb7kpreprocess\fP [\fB-A\fP \fB-B\fP \fB-D\fP\fIoffx/offy\fP \fB-F\fP\fIformat\fP 
\fB-I\fIfile \fB-L\fP \fB-N\fP\fIinsfile\fP \fB-O\fP\fIoutfile\fP 
\fB-T\fP\fItimelag\fP \fB-H -V\fP]

.SH DESCRIPTION
\fBmb7kpreprocess\fP reads a Reson 7k (format 88) file, interpolates the
asynchronous navigation and attitude onto the multibeam data,  
and writes a new 7k file with that information correctly embedded
in the multibeam data. This program can also fix various problems
with 7k data (early generations of the 6046 datalogger failed to
to meet the data format specification exactly).

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
This option causes the program to output multibeam records occuring before the first
available navigation and attitude data. Normally, these records are ignored.
.TP
.B \-B
.br
This option corrects the bathymetry timetags so they agree with the subbottom profiler timetags. 
.TP
.B \-D
\fIoffx/offz\fP
.br
This option sets horizontal and vertical.distance offsets between the vehicle's 
depth sensor and the multibeam receive head. If specified, these values are used
in a lever-arm calculation to determine the sonar depth change  associated with
vehicle pitch.
.TP
.B \-F
\fIformat\fP
.br
Sets the data format used if the input is read from stdin
or from a file. If \fIformat\fP < 0, then the input file specified
with the \fB-I\fP option will actually contain a list of input swath sonar
data files. This program only reads Reson 7k format data files (\fBMBIO\fP
format 88).
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
 	datafile1 88
 	datafile2 88
.br
This program only reads Reson 7k format data files (\fBMBIO\fP
format 88).
.TP
.B \-L
.br
This option causes the program to list the timetags of bathymetry,
sidescan, subbottom, navigation, and attitude data records.
.TP
.B \-N
\fIinsfile\fP
.br
The program will read navigation and attitude from the MBARI AUV navigation
log file \fIinsfile\fP and merge those data with the multibeam, sidescan,
and subbtottom data in the 7k file.If a timelag model is specified using
\fB-T\fP\fItimelag\fP, the time lag will be applied to the navigation and
attitude data before merging with the sonar data. The navigation and attitude
data from \fIinsfile\fP will also be embedded in asynchronous data records
in the output 7k format data. Any pre-existing navigation data records will
be replaced.
.TP
.B \-T
\fItimelag\fP
.br
This option specifies a \fItimelag\fP value in seconds to be applied 
to the navigation and attitude data prior to it being merged with the 
bathymetry. If \fItimelag\fP is a valid path to a file containing time
stamp and time lag pairs, then these data will be read and the time lag
applied to particular records will be interpolated by time.

.SH EXAMPLES
Suppose that one has collected a Reson 7k datafile incorporating 
multibeam sonar data, sidescan data, and subbottom profiler data, and
that the filename is:
 	20050418_150155.s7k
.br

In order to preprocess the data in a 7k data file, one can use
\fBmb7kpreprocess\fP with no special arguments:
 	mb7kpreprocess -I 20040722_152111.s7k

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbformat\fP(l), \fBmbinfo\fP(l)

.SH BUGS
Oh yeah.
