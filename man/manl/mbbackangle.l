.TH mbbackangle l "17 September- 1997" "MB-System 4.5" "MB-System 4.5"
.SH NAME
\fBmbbackangle\fP - Generates a table of the average amplitude or 
sidescan values in swath sonar data as a function of 
the grazing angle with the seafloor.

.SH VERSION
Version 4.5

.SH SYNOPSIS
\fBmbbackangle\fP [\fB-A\fIkind\fP \fB-B\fIyr/mo/da/hr/mn/sc 
\fB-C\fP \fB-E\fIyr/mo/da/hr/mn/sc 
\fB-F\fIformat\fP \fB-I\fIfile\fP  \fB-N\fInangle/anglemax\fP
\fB-R\fIwest/east/south/north\fP \fB-S\fIspeed 
\fB-Z\fIdepth\fP \fB-V -H\fP]

.SH DESCRIPTION
The program \fBmbbackangle\fP reads a swath sonar data file 
containing beam amplitude and/or sidescan data and generates a table
of the average amplitude or sidescan values as a function of
the grazing angle with the seafloor. This table can be used
by the program \fBmbanglecorrect\fP to correct the amplitude or
sidescan data for the variation with grazing angle. No assumption
is made about the nature of the data or the sonar used to
collect it. If bathymetry is
not available, the seafloor is assumed to be flat with a depth
specified by the \fB-Z\fP option. The takeoff
angle for each beam or pixel arrival is projected to the seafloor;
no raytracing is done. The user specifies the angular width 
of the swath considered and the number of angular bins in that
swath; the average amplitude or sidescan value within each bin
form the output data. The results are dumped to stdout.

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
.B \-A
\fIkind\fP
.br
Determines whether beam amplitude (\fIkind\fP = 1) 
or sidescan (\fIkind\fP = 2) data will be processed. 
Default: \fIkind\fP = 2.
.TP
.B \-B
\fIyr/mo/da/hr/mn/sc\fP
.br
Sets the starting time for data allowed in the input data; pings
with times before the starting time will be ignored. 
Default: \fIyr/mo/da/hr/mn/sc\fP = 1962/2/21/10/30/0.
.TP
.B \-C
Normally, \fBmbbackangle\fP produces a table of amplitude vs
grazing angle values which is symmetric about the vertical
(zero angle) axis. If \fB-C\fP is specified, the amplitude
vs grazing angle function will not be forced to be symmetric.
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
Sets the data format used if the input is read from stdin
or from a file. If \fIformat\fP < 0, then the input file specified
with the \fB-I\fP option will actually contain a list of input swath sonar
data files. This program uses the \fBMBIO\fP library 
and will read or write any swath sonar
format supported by \fBMBIO\fP. A list of the swath sonar data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page. Default: \fIformat\fP = 11.
.TP
.B \-H
This "help" flag causes the program to print out a description
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
.B \-N
\fInangle/angle\fP
.br
The amplitude vs grazing angle table is calculated by binning the
amplitude values according to their grazing angles and averaging
the amplitudes within each bin. This option sets the number of
grazing angle bins (\fInangle\fP) and the maximum angle considered
(\fIangle\fP). The grazing angle function will be defined at
\fInangle\fP points spaced equally from -\fIangle\fP to 
+\fIangle\fP. The \fInangle\fP value should be an odd integer
so that the middle bin is centered on the angle 0.0.
Default: \fInangle\fP = 161, \fIangle\fP = 80.0.
.TP
.B \-R
\fIwest/east/south/north\fP
.br
Sets the longitude and latitude bounds within which swath sonar 
data will be read. Only the data which lies within these bounds will
be copied. 
Default: \fIwest\fP=-360, east\fI=360\fP, \fIsouth\fP=-90, \fInorth\fP=90.
.TP
.B \-S
\fIspeed\fP
.br
Sets the minimum speed in km/hr (5.5 kts ~ 10 km/hr) allowed in 
the input data; pings associated with a smaller ship speed will not be
copied. Default: \fIspeed\fP = 0.
.TP
.B \-V
Normally, \fBmbbackangle\fP works "silently" without outputting
anything to the stderr stream.  If the
\fB-V\fP flag is given, then \fBmbmask\fP works in a "verbose" mode and
outputs the program version being used and all error status messages.
.TP
.B \-Z
\fIdepth\fP
.br
This option specifies a default depth value in meters to be
used whenever bathymetry values are unavailable. A flat seafloor
(zero slope) will be assumed wherever the default depth needs
to be used (e.g. data files with sidescan but no bathymetry, or
the outer parts of swaths where the sidescan may extend further
than the bathymetry). If this option is not specified, any
amplitude or sidescan values without associated bathymetry
information will not be used in calculating the amplitude
vs grazing angle table.

.SH EXAMPLE
Suppose one has a SeaBeam 2100 data file called test.mb41
which contains bathymetry (121 beams in a 120 degree swath), 
beam amplitude (121 beams coincident with bathymetry),
and sidescan data (2000 pixels, roughly a 150 degree swath).
Plots of the raw sidescan are dominated by the high amplitude
specular region in the center of the swath and the gradual
fall-off of amplitudes in the outer swath. In order to obtain
a correction function which can be used by \fBmbanglecorrect\fP
to "flatten out" the sidescan values so that seafloor features
are more evident, the user can run \fBmbbackangle\fP as follows:
 	mbbackangle -F41 -Itest.mb41 -A2 -N161/80
.br
The output comes to stdout as a set of \fInangle\fP (161)
ACII lines each containing one pair of angle and average
amplitude values:
 -64.000000 727.933366
 -63.000000 765.391874
 -62.000000 762.552656
 -61.000000 711.655808
 -60.000000 631.966401
 -59.000000 563.588467
 -58.000000 555.110137
 -57.000000 540.660712
 -56.000000 605.508072
 -55.000000 658.763692
 -54.000000 726.006440
 -53.000000 851.389106
 -52.000000 911.463876
 -51.000000 927.908720
 -50.000000 971.749079
 -49.000000 1041.232836
 -48.000000 1033.725516
 -47.000000 1084.941435
 -46.000000 1173.430224
 -45.000000 1219.655876
 -44.000000 1263.997789
 -43.000000 1287.571182
 -42.000000 1248.881823
 -41.000000 1321.806504
 -40.000000 1416.306163
 -39.000000 1468.342740
 -38.000000 1511.206454
 -37.000000 1619.645511
 -36.000000 1601.959117
 -35.000000 1639.856949
 -34.000000 1820.206447
 -33.000000 1899.774419
 -32.000000 2039.100963
 -31.000000 2188.549047
 -30.000000 2198.595859
 -29.000000 2319.472727
 -28.000000 2542.527061
 -27.000000 2631.109418
 -26.000000 2522.161503
 -25.000000 2614.772907
 -24.000000 2861.066208
 -23.000000 2991.539474
 -22.000000 3122.064477
 -21.000000 3043.202609
 -20.000000 3147.861928
 -19.000000 3314.436011
 -18.000000 3507.669565
 -17.000000 3745.632558
 -16.000000 4052.080873
 -15.000000 4064.052135
 -14.000000 4486.166369
 -13.000000 4816.169209
 -12.000000 5318.674783
 -11.000000 5619.060352
 -10.000000 5837.487238
 -9.000000 6885.289048
 -8.000000 7426.885985
 -7.000000 9265.994437
 -6.000000 11172.203110
 -5.000000 12973.700872
 -4.000000 14764.825333
 -3.000000 15236.592260
 -2.000000 15315.271640
 -1.000000 17197.422379
 0.000000 20346.039283
 1.000000 17197.422379
 2.000000 15315.271640
 3.000000 15236.592260
 4.000000 14764.825333
 5.000000 12973.700872
 6.000000 11172.203110
 7.000000 9265.994437
 8.000000 7426.885985
 9.000000 6885.289048
 10.000000 5837.487238
 11.000000 5619.060352
 12.000000 5318.674783
 13.000000 4816.169209
 14.000000 4486.166369
 15.000000 4064.052135
 16.000000 4052.080873
 17.000000 3745.632558
 18.000000 3507.669565
 19.000000 3314.436011
 20.000000 3147.861928
 21.000000 3043.202609
 22.000000 3122.064477
 23.000000 2991.539474
 24.000000 2861.066208
 25.000000 2614.772907
 26.000000 2522.161503
 27.000000 2631.109418
 28.000000 2542.527061
 29.000000 2319.472727
 30.000000 2198.595859
 31.000000 2188.549047
 32.000000 2039.100963
 33.000000 1899.774419
 34.000000 1820.206447
 35.000000 1639.856949
 36.000000 1601.959117
 37.000000 1619.645511
 38.000000 1511.206454
 39.000000 1468.342740
 40.000000 1416.306163
 41.000000 1321.806504
 42.000000 1248.881823
 43.000000 1287.571182
 44.000000 1263.997789
 45.000000 1219.655876
 46.000000 1173.430224
 47.000000 1084.941435
 48.000000 1033.725516
 49.000000 1041.232836
 50.000000 971.749079
 51.000000 927.908720
 52.000000 911.463876
 53.000000 851.389106
 54.000000 726.006440
 55.000000 658.763692
 56.000000 605.508072
 57.000000 540.660712
 58.000000 555.110137
 59.000000 563.588467
 60.000000 631.966401
 61.000000 711.655808
 62.000000 762.552656
 63.000000 765.391874
 64.000000 727.933366
.br
The output data can be directed to a correction file to be used
as input for the program \fBmbanglecorrect\fP.

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbanglecorrect\fP(l), \fBmbfilter\fP(l)

.SH BUGS
None worth bragging about.
