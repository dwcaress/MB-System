.TH mbareaclean l "28 November 2003" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbareaclean\fP - Tool to automatically flag bad beams in swath 
sonar bathymetry data within a specified area.

.SH VERSION
Version 5.0

.SH SYNOPSIS
\fBmbareaclean\fP [\fB-F\fP\fIformat\fP \fB-I\fP\fIinfile\fP \fB-R\fP\fIwest/east/south/north\fP 
\fB-B -G -H -M\fP\fIthreshold\fP[\fI/nmin\fP] \fB-S\fP\fIbinsize\fP -V\fP]

.SH DESCRIPTION
\fBmbareaclean\fP identifies and flags artifacts in swath sonar 
bathymetry data within a specified area of interest. 
The input data are one swath file or a datalist referencing multiple
swath files. An area is specified in longitude and latitude bounds,
along with a bin size in meters. The area is divided into a grid with
square cells of the specified bin size. As the data are read, each of
the soundings that fall within one of the bins is stored. Once all of
data are read, one or more statistical tests are performed on the soundings
within each bin, providing there are a sufficient number of soundings.

The user may specify one or both of the following actions:
  1) Previously unflagged soundings that fail a test are 
flagged as bad
  2) Previously flagged soundings that pass all tests are 
unflagged (the \fB-G\fP option)..
  
The edit events are output to edit save files which can be applied
to the data by the program \fBmbprocess\fP. These are the same edit save
files created and/or modified by \fBmbclean\fP and \fBmbedit\fP.
If a sounding's flag status is changed, that flagging action is output
to the edit save file of the swath file containing that sounding. This
program will create edit save files if necessary, or append to those that
already exist.

At present only one algorithm for identifying good and bad beams is
implemented. This is a simple median filter controlled by the 
\fB-M\fP\fIthreshold/nmin\fP option. Soundings that differ from the
median depth by a value greater than \fIthreshold\fP times the sonar 
altitude will be considered "bad". So, if \fIthreshold\fP = 0.05, then
any sounding that is 5% greater or less than the median depth will be
considered bad. The \fInmin\fP parameter sets the minimum number of 
soundings required to use the median filter, The default values are 
\fIthreshold\fP = 0.25 and \fInmin\fP = 10.

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
.TP
.B \-B
.br
This option causes \fBmbareaclean\fP to flag as bad any previously unflagged
soundings that fail one of the specified statistical tests. If neither \fB-B\fP
or \fB-G\fP are specified, then the program will by default use this option.
.TP
.B \-F
\fIformat\fP
.br
Sets the data format used to read \fIinfile\fP. 
If \fIformat\fP < 0, then the input file specified
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
\fIthreshold\fP[\fI/nmin\fP]
.br
Turns on use of a median filter test for the soundings. If no other test is
specified, the median filter test is used by default. Soundings that differ from the
median depth by a value greater than \fIthreshold\fP times the sonar 
altitude will be considered "bad". So, if \fIthreshold\fP = 0.05, then
any sounding that is 5% greater or less than the median depth will be
considered bad. The \fInmin\fP parameter sets the minimum number of 
soundings required to use the median filter, The default values are 
\fIthreshold\fP = 0.25 and \fInmin\fP = 10.
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
Suppose we are working with a set of 5 Reson 8101 multibeam data files comprising a
shallow water survey. One of these files has previously been edited with \fBmbedit\fP,
so an esf file exists and contains a number of pre-existing edits. 
If we know that this survey is contained in the area specified
by -R-122.42556/-122.41974/47.67111/47.67529, then we can invoke \fBmbareaclean\fP
using:
 	mbareaclean -Idatalist.mb-1 \\
  		-B -M0.1/10 -S2.5 \\
 		-R-122.42556/-122.41974/47.67111/47.67529  \\
 		-V
.br
where the bin size is 2.5 meters, the median filter threshold is 0.1 (or 10%)
of the sonar altitude, and the minimum number of sounding required for
filtering is 10. The results look like:

 Program MBAREACLEAN
 Version $Id: mbareaclean.l,v 5.3 2004-05-21 23:10:32 caress Exp $
 MB-system Version 5.0.beta29
 Area of interest:
      Minimum Longitude: -122.425560 Maximum Longitude: -122.419740
      Minimum Latitude:  47.671110 Maximum Latitude:  47.675290
      Bin Size:   2.500000
      Dimensions: 175 186
 Cleaning algorithms:
      Median filter: ON
      Plane fit:     OFF
 Output:
      Flag unflagged soundings identified as bad:  ON
      Unflag flagged soundings identified as good: OFF
 
 Processing 001_1730.fbt
 Sorting 41580 old edits...
 10000 of 41580 old edits sorted...
 20000 of 41580 old edits sorted...
 30000 of 41580 old edits sorted...
 40000 of 41580 old edits sorted...
 pings:1169  beams:   20233 good   41686 flagged   56150 null 
 
 Processing 003_1733.fbt
 pings: 991  beams:   52439 good     159 flagged   47493 null 
 
 Processing 005_1736.fbt
 pings:1011  beams:   53080 good     588 flagged   48443 null 
 
 Processing 007_1739.fbt
 pings: 922  beams:   48854 good     212 flagged   44056 null 
 
 Processing 009_1741.fbt
 pings:1017  beams:   53416 good     586 flagged   48715 null 
 
 MBareaclean Processing Totals:
 -------------------------
 5 total swath data files processed
 5110 total pings processed
 271253 total soundings processed
 -------------------------
   0 soundings:  61919 flagged:      0 unflagged:      0  file:001_1730
   1 soundings:  52598 flagged:      0 unflagged:      0  file:003_1733
   2 soundings:  53668 flagged:      0 unflagged:      0  file:005_1736
   3 soundings:  49066 flagged:      0 unflagged:      0  file:007_1739
   4 soundings:  54002 flagged:      0 unflagged:      0  file:009_1741
 

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbclean\fP(l), \fBmbedit\fP(l), 
\fBmbinfo\fP(l) \fBmbprocess\fP(l),

.SH BUGS
A simple median filter is the only statistical test
implemented at present. Any suggestions?

The algorithms implemented in \fBmbareaclean\fP simply 
don't detect all bathymetric artifacts that
are obvious to the eye on contour charts.  Although
the autofiltering tools \fBmbareaclean\fP and \fBmbclean\fP 
often do a credible first pass at
flagging obvious artifacts, we strongly recommend that
any swath bathymetry processing stream include 
interactive editing of the
bathymetry data (e.g. \fBmbedit\fP).
