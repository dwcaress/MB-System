.TH mbrolltimelag l "9 January 2008" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbrolltimelag\fP - Calculates cross correlation between the apparent
bottom slope in swath bathymetry data and the roll time series used by
the sonar in order to assess attitude time lag problems.

.SH VERSION
Version 5.0

.SH SYNOPSIS
\fBmbrolltimelag\fP \fB-I\fP\fIswathdata\fP [\fB-F\fP\fIformat\fP 
\fB-N\fP\fInping\fP \fB-O\fP\fIoutroot\fP \fB-S\fP\fInavchannel\fP 
\fB-T\fP\fInlag/lagmin/lagmax\fP \fB-V -H\fP ]
.SH DESCRIPTION
\fBMBrolltimelag\fP is an utility used to assess relative time errors,
or time lag, between the ping timestamps of swath bathymetry data
and the sonar attitude (roll and pitch) used in calculating 
the bathymetry. For any sonar ping, a roll error will cause 
the acrosstrack bathymetry profile to rotate vertically so that 
it is too shallow on one side of the swath and too deep on the other. 
Consequently, if no correction is made for the roll of the sonar, then the 
bathymetry data will appear to be ruffled in the outer swath. 
If the timestamps of the roll and ping data are consistent, then
correcting for roll will produce bathymetry without these pronounced
across-track artifacts. If, however, there is a lag between the two
sets of time stamps, then the roll values used for correction will
be incorrect, and the ruffles will still occur.

In order to determine the amount of time lag, if any, associated
with a swath bathymetry dataset, this program calculates the 
cross correlation between the roll signal and the apparent bottom
slope in the case of no roll correction (in practice this latter
value is found by subtracting the applied roll value from the
apparent acrosstrack bottom slope). If the seafloor is planar so 
that the apparent slope signal is dominated by roll error, then
there will be a distinct peak in the cross correlation function
at the actual time lag.

\fBMBrolltimelag\fP reads one or more swath sonar data files 
containing bathymetry data and calculates many cross correlation
functions. The number of pings used for each calculation is
specified with the \fB-N\fP\fInping\fP option. The range of time
lags used and the number of lag values are all specified using
the \fB-T\fP\fInlag/lagmin/lagmax\fP option. The cross correlation
functions are output to a file named "swathdata_xcorr.txt", where
\fIswathdata\fP is the input file or datalist specified with the 
\fB-I\fP\fIswathdata\fP option. The macro \fBmbm_xyplot\fP is also
called to generate a plot shellscript that, if run, will generate
a postscript plot of the correlation functions.

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
.B \-F
\fIformat\fP
.br
Sets the \fBMBIO\fP integer format identifier 
for the input file specified with the
\fB-I\fP option. By default, \fBmbrolltimelag\fP
infers the format from the "*.mbXX" \fBMB-System\fP suffix
convention. 
.TP
.B \-H
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \-I
\fIfilename\fP
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
This program uses the \fBMBIO\fP library and will read any swath sonar
format supported by \fBMBIO\fP. A list of the swath sonar data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page. Default: \fIinfile\fP = "datalist.mb-1".
.TP
.B \-N
\fInping\fP
.br
Sets the number of pings to be read for each cross correlation calculation.
Default: \fIlonflip\fP = 0.
.TP
.B \-O
\fIoutroot\fP
.br
Sets the root of the output filenames.
Default: output root is is the input filename minus any \fBMB-System\fP
filename suffixe (e.g. the root derived from "datalistp.mb-1" will be 
"datalist").
.TP
.B \-S
\fInavchannel\fP
.br
Sets \fBMBrolltimelag\fP to read the roll data from navigation records rather
than the record type listed as the "navigation source" for a particular
data format. Swath data formats may contain synchronous navigation
(navigation is included in survey data records), asynchronous navigation
(navigation comes in records separate from and at different times
than the survey data records), or both (survey records contain merged
navigation while the original asynchronous navigation is preserved in
separate navigation records). Some data formats provide for multiple
navigation sources, so that there are more than one channel of
asynchronous data records. In this case one channel is always designated
as the primary or active navigation channel. The \fB-S\fP option allows
the user to specify that \fBMBrolltimelag\fP uses roll from
the primary asynchronous navigation (\fB-N\fP\fI0\fP) or from one
of three auxilliary navigation channels 
(\fB-N\fP\fI1\fP, \fB-N\fP\fI2\fP, or \fB-N\fP\fI3\fP). If \fB-N\fP is
used but the data file
does not contain asynchronous navigation or the specified auxilliary
navigation channels, then \fBmbnavlist\fP will output nothing. To
determine if a datafile contains asynchronous navigation and/or auxilliary
navigation, run \fBmbinfo\fP using its \fB-N\fP option to get counts
of all data record types in the file.
Default: \fInavchannel\fP = 1.
.TP
.B \-T
\fInlag/lagmin/lagmax\fP
.br
Sets the time lag values used for the cross correlation calculations. A total
of \fInlag\fP  cross correlation values will be calculated evenly spaced from
\fIlagmin\fP seconds to \fIlagmax\fP seconds.
Default: \fInlag\fP = 41, \fIlagmin\fP = -2.0 seconds, \fIlagmax\fP = 2.0 seconds.
.TP
.B \-V
The \fB-V\fP option causes the program to be verbose.
.SH EXAMPLE
Suppose you want to evaluate the possible roll time lag in a collection
of Reson 7125 files referenced by a datalist called datalistp.mb-1. The
sonar was pinging at 2Hz (twice a second), and roll is available at 10 Hz
in asynchronous navigation records that can be accessed with 
\fInavchannel\fP = 2. We choose to calculate the cross correlation with
time lags separated by 0.1 seconds from -2.0 to 2.0 seconds. The following
will suffice:

 	mbrolltimelag -Idatalistp.mb-1 -N41/-2.0/2.0 -S2 -V

.SH SEE ALSO
\fBmbsystem\fP(l)

.SH BUGS
Da.
