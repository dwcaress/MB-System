.TH mbinfo l "17 September 1997" "MB-System 4.5" "MB-System 4.5"
.SH NAME
\fBmbinfo\fP - Output some basic statistics of swath sonar data files.

.SH VERSION
Version 4.5

.SH SYNOPSIS
\fBmbinfo\fP [\fB-B\fIyr/mo/da/hr/mn/sc\fP \fB-C -E\fIyr/mo/da/hr/mn/sc\fP 
\fB-F\fIformat\fP \fB-G\fP \fB-I\fIfilename\fP \fB-L\fIlonflip\fP \fB-P\fIping\fP 
\fB-R\fIwest/east/south/norths\fP \fB-S\fIspeed\fP \fB-W -V -H\fP]

.SH DESCRIPTION
\fBMBinfo\fP is an utility for reading a swath sonar data file 
or files and outputting some basic statistics.  The table generated
shows the filename, the data format id, a brief description of
the format, data counts, navigation totals, time and navigation
of the first and last data records, and minimum and maximum
data values. The data counts include the total number of bathymetry,
amplitude, and sidescan values read. These totals may be less
than the maximum possible (number of data records times the
maximum number of data supported for the particular data format)
for formats supporting variable numbers of beams or pixels.
Also reported are the numbers and percentages of good, zero,
and flagged data values (good values are those which are neither
zero nor flagged as bad).

The data input may be averaged over several pings
and/or windowed in time and space.  Comments in the data 
stream may also be printed out.
The results are dumped to stdout.

If pings are averaged (\fIpings\fP > 2) 
\fBmbinfo\fP estimates the variance for each of the bathymetry beams 
and sidescan pixels
by reading a set number of pings and then finding the 
variance of the values for each beam.  The bathymetry values are
detrended before variances are calculated. 

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
.B \-B
\fIyr/mo/da/hr/mn/sc\fP
.br
Sets the starting time for data allowed in the input data; pings
with times before the starting time will be ignored. 
Default: \fIyr/mo/da/hr/mn/sc\fP = 1962/2/21/10/30/0.
.TP
.B \-C
Normally, \fBmbinfo\fP ignores comments encountered in the
data file.  If the \fB-C\fP flag is given, all of the comments will be 
printed out.
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
.B \-G
Enables checking for reasonable navigation to be used in
calculating the minimum and maximum longitude and latitude values.
This mode excludes longitude and latitude values of 0.0 or
those values associated with very large apparent speeds 
between pings (calculated using distance and time differences
between adjacent pings). This option is particularly useful 
when one is trying to obtain reasonable bounds for a plot.
.TP
.B \-H
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \-I
\fIfilename\fP
.br
Sets the input filename. If \fIformat\fP > 0 (set with the 
\fB-f\fP option) then the swath sonar data contained in \fIinfile\fP 
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
.B \-P
\fIpings\fP
.br
Turns on variance calculations for the bathymetry, amplitude,
and sidescan data (as available in the data stream).  If 
\fIpings\fP = 1, then no variance calculations are made.  If
\fIpings\fP > 1, then variances are calculated for each beam
and pixel using groups of \fIpings\fP values.  The bathymetry
values are detrended before the variances are calculated; 
the amplitude and sidescan values are not detrended.  The
variance calculations can provide crude measures of noise
and/or signal as a function of beam and pixel number.
Default: \fIpings\fP = 1 (no variance calculations).
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
Normally, \fBmbinfo\fP only prints out the statistics obtained
by reading all of the data.  If the
\fB-V\fP flag is given, then \fBmbinfo\fP works in a "verbose" mode and
outputs the program version being used and all read error status messages.
.TP
.B \-W
Normally, \fBmbinfo\fP reports depth values in meters. If the
\fB-W\fP flag is given, then \fBmbinfo\fP reports these values
in feet.

.SH EXAMPLES
Suppose one wishes to know something about the contents of 
a Hydrosweep file (format 24) called example_hs.mb24. 
The following will suffice:
 	mbinfo -F24 -Iexample_hs.mb24

The following output is produced: 

 Multibeam Data File:  example_hs.mb24
 MBIO Data Format ID:  24
 Format name:          MBF_HSLDEOIH
 Informal Description: L-DEO in-house binary Hydrosweep
 Attributes:           Hydrosweep DS, 59 beams, bathymetry and amplitude, 
                       binary, centered, L-DEO.
 
 Data Totals:
 Number of Records:              263
 Bathymetry Data (59 beams):
   Number of Beams:            15517
   Number of Good Beams:       13661     88.04%
   Number of Zero Beams:         868      5.59%
   Number of Flagged Beams:      988      6.37%
 Amplitude Data (59 beams):
   Number of Beams:            15517
   Number of Good Beams:           0      0.00%
   Number of Zero Beams:       15517     100.00%
   Number of Flagged Beams:        0      0.00%
 Sidescan Data (0 pixels):
   Number of Pixels:               0
   Number of Good Pixels:          0      0.00%
   Number of Zero Pixels:          0      0.00%
   Number of Flagged Pixels:       0      0.00%
 
 Navigation Totals:
 Total Time:             1.2425 hours
 Total Track Length:    20.9421 km
 Average Speed:         16.8548 km/hr ( 9.1107 knots)
 
 Start of Data:
 Time:  08 14 1993 18:00:25.000000  JD226
 Lon:  -49.3011     Lat:   12.1444     Depth:  4920.0000 meters
 Speed: 18.3600 km/hr ( 9.9243 knots)  Heading:  97.2000 degrees
 
 End of Data:
 Time:  08 14 1993 19:14:58.000000  JD226
 Lon:  -49.1111     Lat:   12.1149     Depth:  5021.0000 meters
 Speed: 17.2800 km/hr ( 9.3405 knots)  Heading:  97.0000 degrees
 
 Limits:
 Minimum Longitude:   -49.3061   Maximum Longitude:   -49.1064
 Minimum Latitude:     12.0750   Maximum Latitude:     12.1806
 Minimum Depth:      3726.0000   Maximum Depth:      5190.0000

Suppose we wanted to know how noisy the outer beams are relative
to the inner beams.  We might try:
 	mbinfo -F24 -P5 -Iexample_hs.mb24

obtaining:

 Multibeam Data File:  example_hs.mb24
 MBIO Data Format ID:  24
 Format name:          MBF_HSLDEOIH
 Informal Description: L-DEO in-house binary Hydrosweep
 Attributes:           Hydrosweep DS, 59 beams, bathymetry and amplitude, 
                       binary, centered, L-DEO.
 
 Data Totals:
 Number of Records:              263
 Bathymetry Data (59 beams):
   Number of Beams:            15517
   Number of Good Beams:       13661     88.04%
   Number of Zero Beams:         868      5.59%
   Number of Flagged Beams:      988      6.37%
 Amplitude Data (59 beams):
   Number of Beams:            15517
   Number of Good Beams:           0      0.00%
   Number of Zero Beams:       15517     100.00%
   Number of Flagged Beams:        0      0.00%
 Sidescan Data (0 pixels):
   Number of Pixels:               0
   Number of Good Pixels:          0      0.00%
   Number of Zero Pixels:          0      0.00%
   Number of Flagged Pixels:       0      0.00%
 
 Navigation Totals:
 Total Time:             1.2425 hours
 Total Track Length:    20.9421 km
 Average Speed:         16.8548 km/hr ( 9.1107 knots)
 
 Start of Data:
 Time:  08 14 1993 18:00:25.000000  JD226
 Lon:  -49.3011     Lat:   12.1444     Depth:  4920.0000 meters
 Speed: 18.3600 km/hr ( 9.9243 knots)  Heading:  97.2000 degrees
 
 End of Data:
 Time:  08 14 1993 19:14:58.000000  JD226
 Lon:  -49.1111     Lat:   12.1149     Depth:  5021.0000 meters
 Speed: 17.2800 km/hr ( 9.3405 knots)  Heading:  97.0000 degrees
 
 Limits:
 Minimum Longitude:   -49.3061   Maximum Longitude:   -49.1064
 Minimum Latitude:     12.0750   Maximum Latitude:     12.1806
 Minimum Depth:      3726.0000   Maximum Depth:      5190.0000

 Beam Bathymetry Variances:
 Pings Averaged: 5
  Beam     N      Mean     Variance    Sigma
  ----     -      ----     --------    -----
    0      0       0.00       0.00      0.00
    1      0       0.00       0.00      0.00
    2    110    4719.59     342.69     18.51
    3    105    4779.49     399.15     19.98
    4    155    4748.81     280.18     16.74
    5    155    4817.12     194.62     13.95
    6    150    4826.44     197.76     14.06
    7    160    4863.82     155.50     12.47
    8    215    4806.08     229.11     15.14
    9    235    4807.09     220.23     14.84
   10    240    4766.29     158.83     12.60
   11    250    4764.34     221.09     14.87
   12    245    4765.35     146.24     12.09
   13    250    4782.02     167.34     12.94
   14    240    4798.38      92.98      9.64
   15    245    4775.16      98.27      9.91
   16    225    4782.35     136.30     11.67
   17    210    4820.37      80.70      8.98
   18    215    4821.15      80.97      9.00
   19    215    4827.71      76.20      8.73
   20    195    4842.65      84.22      9.18
   21    190    4843.02     155.87     12.48
   22    185    4884.28      73.69      8.58
   23    175    4885.21      69.88      8.36
   24    175    4871.47      52.01      7.21
   25    180    4871.92      34.71      5.89
   26    200    4830.80      36.83      6.07
   27    205    4835.16      33.47      5.79
   28    210    4809.96      43.07      6.56
   29    190    4850.77      40.97      6.40
   30    240    4768.69      64.23      8.01
   31    240    4772.90      74.44      8.63
   32    245    4760.11      57.97      7.61
   33    255    4734.01      81.72      9.04
   34    255    4728.19      82.21      9.07
   35    260    4722.94      83.45      9.14
   36    260    4721.95     102.02     10.10
   37    260    4713.48      83.85      9.16
   38    250    4715.40     101.33     10.07
   39    255    4722.56     118.20     10.87
   40    250    4727.48     109.13     10.45
   41    255    4734.96     127.97     11.31
   42    255    4724.53     124.06     11.14
   43    230    4744.74     122.96     11.09
   44    225    4752.16      98.22      9.91
   45    230    4692.27     107.96     10.39
   46    240    4696.93      95.93      9.79
   47    230    4699.80     129.08     11.36
   48    225    4696.32     145.20     12.05
   49    220    4681.50     140.29     11.84
   50    210    4676.16     103.35     10.17
   51    180    4627.31     105.22     10.26
   52    200    4654.55     207.85     14.42
   53    130    4665.82     250.97     15.84
   54    185    4704.29     300.80     17.34
   55    135    4731.13     218.16     14.77
   56    150    4736.29     178.16     13.35
   57    115    4691.45     217.31     14.74
   58      0       0.00       0.00      0.00

.SH SEE ALSO
\fBmbsystem\fP(l)

.SH BUGS
No bugs, only features...
