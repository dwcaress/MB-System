.TH mbinfo l "31 October 1994"
.SH NAME
\fBmbinfo\fP - Output some basic statistics of multibeam bathymetry data files.

.SH VERSION
Version 4.1

.SH SYNOPSIS
\fBmbinfo\fP [\fB-B\fIyr/mo/da/hr/mn/sc \fB-C -E\fIyr/mo/da/hr/mn/sc 
\fB-F\fIformat \fB-I\fIfilename \fB-L\fIlonflip \fB-P\fIping 
\fB-R\fIwest/east/south/norths \fB-S\fIspeed \fB-V -H\fP]

.SH DESCRIPTION
\fBmbinfo\fP is an utility for reading a multibeam data file 
and outputting some basic statistics.  The data input may be averaged 
or windowed in time and space.  If pings are averaged (pings > 2) 
\fBmbinfo\fP estimates the variance for each of the multibeam beams 
and sidescan pixels
by reading a set number of pings (>2) and then finding the 
variance of the values for each beam.  The bathymetry values are
detrended before variances are calculated. The results are 
dumped to stdout.

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
.B \fB-B\fIyr/mo/da/hr/mn/sc\fP
Sets the starting time for data allowed in the input data; pings
with times before the starting time will be ignored. 
Default: \fIyr/mo/da/hr/mn/sc\fP = 1962/2/21/10/30/0.
.TP
.B \fB-C\fP
Normally, \fBmbinfo\fP ignores comments encountered in the
data file.  If the \fB-C\fP flag is given, all of the comments will be 
printed out.
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
.B \fB-I\fIfilename\fP
Data file from which the input data will be read. If
no input file is specified, the input will be read
from stdin. Default: \fIfilename\fP = stdin.
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
.B \fB-P\fIpings\fP
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
Normally, \fBmbinfo\fP only prints out the statistics obtained
by reading all of the data.  If the
\fB-V\fP flag is given, then \fBMBCOPY\fP works in a "verbose" mode and
outputs the program version being used and all read error status messages.
.TP
.B \fB-H\fP
This "help" flag cause the program to print out a description
of its operation and then exit immediately.

.SH EXAMPLES
Suppose one wishes to know something about the contents of 
a Hydrosweep file (format 24) called hstest. 
The following will suffice:
 	mbinfo -F24 -Ihstst

The following output is produced: 

 Multibeam Data File:  hstst
 MBIO Data Format ID:  24
 Format name:          MBF_HSLDEOIH
 Informal Description: L-DEO in-house binary Hydrosweep
 Attributes:           Hydrosweep DS, 59 beams, bathymetry and amplitude, 
                       binary, centered, L-DEO.

 Data Totals:
 Number of Records:             3052
 Bathymetry Data:
   Number of Beams:           180068
   Number of Good Beams:      163352
   Number of Zero Beams:       16440
   Number of Flagged Beams:      276
 Amplitude Data:
   Number of Beams:           180068
   Number of Good Beams:           0
   Number of Zero Beams:      180068
   Number of Flagged Beams:        0
 Sidescan Data:
   Number of Pixels:               0
   Number of Good Pixels:          0
   Number of Zero Pixels:          0
   Number of Flagged Pixels:       0

 Navigation Totals:
 Total Time:            10.1903 hours
 Total Track Length:   183.5527 km
 Average Speed:         18.0125 km/hr

 Start of Data:
 Time:  06 06 1993 13:48:22  JD157
 Lon:  -29.0000     Lat:   55.7848   Depth: 2980.00
 Speed: 16.2000  Heading:  61.4000

 End of Data:
 Time:  06 06 1993 23:59:47  JD157
 Lon:  -26.6236     Lat:   56.7536   Depth: 2675.00
 Speed: 19.0800  Heading:  45.3000

 Limits:
 Minimum Longitude:   -29.0000   Maximum Longitude:   -26.6236
 Minimum Latitude:     55.7848   Maximum Latitude:     56.7536
 Minimum Depth:      2604.0000   Maximum Depth:      3023.0000

Suppose we wanted to know how noisy the outer beams are relative
to the inner beams.  We might try:
 	mbinfo -F24 -P5 hstst

obtaining:

 Multibeam Data File:  hstst
 MBIO Data Format ID:  24
 Format name:          MBF_HSLDEOIH
 Informal Description: L-DEO in-house binary Hydrosweep
 Attributes:           Hydrosweep DS, 59 beams, bathymetry and amplitude, 
                       binary, centered, L-DEO.

 Data Totals:
 Number of Records:             3052
 Bathymetry Data:
   Number of Beams:           180068
   Number of Good Beams:      163352
   Number of Zero Beams:       16440
   Number of Flagged Beams:      276
 Amplitude Data:
   Number of Beams:           180068
   Number of Good Beams:           0
   Number of Zero Beams:      180068
   Number of Flagged Beams:        0
 Sidescan Data:
   Number of Pixels:               0
   Number of Good Pixels:          0
   Number of Zero Pixels:          0
   Number of Flagged Pixels:       0

 Navigation Totals:
 Total Time:            10.1903 hours
 Total Track Length:   183.5527 km
 Average Speed:         18.0125 km/hr

 Start of Data:
 Time:  06 06 1993 13:48:22  JD157
 Lon:  -29.0000     Lat:   55.7848   Depth: 2980.00
 Speed: 16.2000  Heading:  61.4000

 End of Data:
 Time:  06 06 1993 23:59:47  JD157
 Lon:  -26.6236     Lat:   56.7536   Depth: 2675.00
 Speed: 19.0800  Heading:  45.3000

 Limits:
 Minimum Longitude:   -29.0000   Maximum Longitude:   -26.6236
 Minimum Latitude:     55.7848   Maximum Latitude:     56.7536
 Minimum Depth:      2604.0000   Maximum Depth:      3023.0000

 Beam Bathymetry Variances:
 Pings Averaged: 5
  Beam     N      Mean     Variance    Sigma
  ----     -      ----     --------    -----
    0      0       0.00       0.00      0.00
    1      0       0.00       0.00      0.00
    2     65    2738.68      68.59      8.28
    3    375    2745.22      82.75      9.10
    4   1130    2755.22      71.21      8.44
    5   1875    2762.57      78.69      8.87
    6   2040    2765.62      77.00      8.77
    7   2690    2765.38      73.46      8.57
    8   2880    2766.23      60.10      7.75
    9   2990    2765.78      58.77      7.67
   10   3050    2766.01      52.71      7.26
   11   3050    2764.12      44.35      6.66
   12   3050    2764.52      40.27      6.35
   13   3050    2763.54      36.75      6.06
   14   3040    2762.47      34.58      5.88
   15   3040    2765.12      32.07      5.66
   16   3045    2764.02      28.08      5.30
   17   3045    2766.51      25.42      5.04
   18   3045    2762.24      21.18      4.60
   19   3045    2762.13      18.31      4.28
   20   3045    2761.27      15.48      3.93
   21   3045    2761.22      13.25      3.64
   22   3050    2760.35      13.27      3.64
   23   3050    2759.73      10.88      3.30
   24   3050    2759.22       8.74      2.96
   25   3050    2758.53       7.34      2.71
   26   3050    2757.69       5.54      2.35
   27   3050    2757.39       5.09      2.26
   28   3050    2755.59       4.41      2.10
   29   3050    2754.57       2.65      1.63
   30   3050    2754.41       2.34      1.53
   31   3050    2752.83       3.58      1.89
   32   3045    2752.97       4.85      2.20
   33   3050    2752.93       5.64      2.38
   34   3050    2753.11       5.62      2.37
   35   3050    2752.31       6.92      2.63
   36   3045    2752.51       9.24      3.04
   37   3050    2752.17      10.31      3.21
   38   3050    2751.34      10.17      3.19
   39   3050    2750.61      11.64      3.41
   40   3050    2750.11      15.57      3.95
   41   3045    2753.24      17.65      4.20
   42   3045    2750.49      21.48      4.63
   43   3045    2749.09      25.17      5.02
   44   3035    2746.72      26.82      5.18
   45   3025    2746.76      33.52      5.79
   46   3040    2745.77      32.09      5.66
   47   3040    2745.46      36.80      6.07
   48   3045    2744.89      37.94      6.16
   49   3040    2744.80      48.66      6.98
   50   3015    2744.34      51.85      7.20
   51   2975    2743.04      56.28      7.50
   52   2790    2742.27      61.90      7.87
   53   2010    2742.49      71.17      8.44
   54   2335    2739.53      71.38      8.45
   55   1610    2737.56      74.31      8.62
   56    745    2733.07      80.96      9.00
   57    155    2745.39      82.94      9.11
   58      0       0.00       0.00      0.00

.SH SEE ALSO
\fBmbsystem\fP(l)

.SH BUGS
No bugs, only features...
