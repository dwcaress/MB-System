.TH mbareaclean l "10 March 2003" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbareaclean\fP - Tool to automatically flag bad beams in swath sonar bathymetry data.

.SH VERSION
Version 5.0

.SH SYNOPSIS
mbareaclean [-Fformat -Iinfile -Rwest/east/south/north-Sbinsize 
-B -G -M -Sbinsize]

.SH WARNING
\fBTHIS PROGRAM IS NOT YET FUNCTIONAL!!!\fP

.SH DESCRIPTION
\fBmbareaclean\fP identifies and flags artifacts in swath sonar bathymetry data.  
The edit events are output to edit save files which can be applied
to the data by the program \fBmbprocess\fP. These are the same edit save
files created and/or modified by \fBmbclean\fP and \fBmbedit\fP.
The input data are one swath file or a datalist referencing multiple
swath files. An area is specified in longitude and latitude bounds,
along with a bin size in meters. The area is divided into a grid with
square cells of the specified bin size. As the data are read, each of
the soundings that fall within one of the bins is stored. Once all of
data are read, one or more statistical tests are performed on the soundings
within each bin, providing there are a sufficient number of soundings.
The user may specify one or both of the following actions:
  1) Previously unflagged soundings that fail a test are flagged as bad.
  2) Previously flagged soundings that pass all tests are unflagged.
If a sounding's flag status is changed, that flagging action is output
to the edit save file of the swath file containing that sounding. This
program will create edit save files if necessary, or append to those that
already exist.

.SH AUTHORSHIP
David W. Caress (caress@mbari.org)
.br
  Monterey Bay Aquarium Research Institute
.br
Dale N. Chayes (dale@ldeo.columbia.edu)
.br
  Lamont-Doherty Earth Observatory
.br
Alberto Malinverno
.br
  Schlumberger-Doll

.SH OPTIONS
.TP
.TP
.B \-B
.br
This option cause \fBmbarealcean\fP to flag as bad any previously unflagged
soundings that fail one of the specified statistical tests. If neither \fB-B\fP
or \fB-G\fP are specified, then the program will by default use this option.
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
is given in the \fBMBIO\fP manual page. Default: \fIformat\fP = 11.
.TP
.B \-G
This option cause \fBmbarealcean\fP to unflag (set good) any previously flagged
soundings that pass all of the specified statistical tests. If neither \fB-B\fP
or \fB-G\fP are specified, then the program will by default apply the  \fB-B\fP
option.
.TP
.B \-H
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \-I
\fIinfile\fP
.br
Sets the input filename. If \fIformat\fP > 0 (set with the 
\fB-F\fP option) then the swath sonar data contained in \fIinfile\fP 
is read and processed. If \fIformat\fP < 0, then \fIinfile\fP
is assumed to be an ascii file containing a list of the input swath sonar
data files to be processed and their formats.  The program will read 
and process the data in each one of these files. Each input file will
have an associated output file with either the ".sga" or ".aga" suffix.
In the \fIinfile\fP file, each
data file should be followed by a data format identifier, e.g.:
 	datafile1 11
 	datafile2 24
.br
This program uses the \fBMBIO\fP library and will read or write any swath sonar
format supported by \fBMBIO\fP. A list of the swath sonar data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page. Default: \fIinfile\fP = "datalist.mb-1".
.TP
.B \-M
.br
Turns on use of a median filter test for the soundings. If no other test is
specified, the median filter test is used by default.
.TP
.B \-R
\fIwest/east/south/north\fP
.br
Sets the longitude and latitude bounds within which swath sonar 
data will be read, binned, and tested. Soundings lying outside these
bounds will be ignored. 
.br
Default: None. This is a mandatory argument.
.TP
.B \-S
\fIbinsize\fP
.br
Sets the size of the bins to be used in meters. The area specified
with the \fB-R\fP option will be broken into a grid with cells that
are roughly \fIbinsize\fP meters east-west and north-south.
.br
Default: None. This is a mandatory argument.
.TP
.B \-V
Normally, \fBmbareaclean\fP works "silently" without outputting
anything to the stderr stream.  If the
\fB-V\fP flag is given, then \fBmbareaclean\fP works in a "verbose" mode and
outputs the program version being used, all error status messages, 
and the number of beams flagged as bad.

.SH EXAMPLES

.SH SEE ALSO
\fBmbsystem\fP(l), \fBclean\fP(l), \fBmbedit\fP(l), 
\fBmbinfo\fP(l) \fBmbprocess\fP(l),

.SH BUGS
The algorithms implemented in \fBmbareaclean\fP simply 
don't detect all bathymetric artifacts that
are obvious to the eye on contour charts.  Although
the autofiltering tools \fBmbareaclean\fP and \fBmbclean\fP 
often do a credible first pass at
flagging obvious artifacts, we strongly recommend that
any swath bathymetry processing stream include 
interactive editing of the
bathymetry data (e.g. \fBmbedit\fP).
