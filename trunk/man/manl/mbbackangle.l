.TH mbbackangle l "7 September 2002" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbbackangle\fP - Generates tables of the average amplitude or 
sidescan values in swath sonar data as a function of 
the grazing angle with the seafloor.

.SH VERSION
Version 5.0

.SH SYNOPSIS
\fBmbbackangle\fP [\fB-A\fIkind\fP 
\fB-C\fP \fB-D\fP \fB-F\fIformat\fP \fB-I\fIfile\fP  
\fB-N\fInangle/anglemax\fP \fB-P\fIpings\fP
\fB-Z\fIaltitude\fP \fB-V -H\fP]

.SH DESCRIPTION
The program \fBmbbackangle\fP reads a swath sonar data file 
containing beam amplitude and/or sidescan data and generates tables
of the average amplitude and/or sidescan values as a function of
the grazing angle with the seafloor. These tables can be used
by the program \fBmbprocess\fP to correct the sidescan or
amplitude data for the variation with grazing angle. No assumption
is made about the nature of the data or the sonar used to
collect it. The user specifies the angular width 
of the swath considered and the number of angular bins in that
swath; the average amplitude or sidescan value within each bin
form the output data. The user also specifies the number of pings
to be read to generate each table The results are written to a file named by 
adding a ".sga" suffix to the swath filename for sidescan and
".aga" for amplitude. These files contain a series of amplitude vs.
grazing angle tables, each time tagged according to the average time
of the pings used to generate that table. The swath file's 
\fBmbprocess\fP parameter file is also modified (or created if necessary)
so that sidescan or amplitude correction is enabled. The correction
tables are read and interpolated according to ping time by mbprocess.
If bathymetry is
not available, the seafloor is assumed to be flat with a sonar altitude
specified by the \fB-Z\fP option. In this case the takeoff
angle for each beam or pixel arrival is projected to the seafloor; 
no raytracing is done. 

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
\fIkind\fP
.br
Determines if beam amplitude (\fIkind\fP = 1) 
and/or sidescan (\fIkind\fP = 2) data will be processed. 
Default: both amplitude and sidescan data are processed.
.TP
.B \-C
Normally, \fBmbbackangle\fP produces a table of amplitude vs
grazing angle values which is asymmetric about the vertical
(zero angle) axis. If \fB-C\fP is specified, the amplitude
vs grazing angle function will be forced to be symmetric.
Generally, it is better to not use this option in \fBmbbackangle\fP, 
and to instead use \fBmbset\fP to set symmetric or asymmetric
sidescan correction in the \fBmbprocess\fP parameter file.
.TP
.B \-D
This option causes \fBmbbackangle\fP to dump the output to
stdout rather than to ".sga" or ".aga" files..
.TP
.B \-F
\fIformat\fP
.br
Sets the \fBMBIO\fP integer format identifier 
for the input file specified with the
\fB-I\fP option. By default, \fBmbbackangle\fP
infers the format from the "*.mbXX" \fBMB-System\fP suffix
convention. 
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
Default: \fInangle\fP = 81, \fIangle\fP = 80.0.
.TP
.B \-P
\fIpings\fP
.br
Sets the number of pings to be read and averaged for each
amplitude vs grazing angle table. The output "*.sga" or ".aga" files
will generally contain many tables, each with a timestamp
corresponding to the average of the ping times used for
each table. A table may be output averaged over a lesser number
of beams if a time gap is encountered in the data, and also at
the end of the file. Default: \fIpings\fP = 25.
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
This option specifies a default sonar altitude value in meters to be
used whenever bathymetry values are unavailable. A flat seafloor
(zero slope) will be assumed wherever the default depth needs
to be used (e.g. data files with sidescan but no bathymetry, or
the outer parts of swaths where the sidescan may extend further
than the bathymetry). If this option is not specified, any
amplitude or sidescan values without associated bathymetry
information will not be used in calculating the amplitude
vs grazing angle table.

.SH EXAMPLE
Suppose one has a Simrad EM300 data file called
       0003_20020425_022926.mb57
which contains 131 pings. In order to correct the sidescan 
with mbprocess, first use \fBmbbackangle\fP to generate the
amplitude vs grazing angle tables:
 	mbbackangle -I 0003_20020425_022926.mb57 \
 			-N161/80.0 -P25 -V
Here the angle bins will be 1.0 degree wide, the table
will extend from -80.0 degees (port) to +80.0 (starboard),
and each table will represent the averages over up to 25
pings. The output looks like:

 	Program mbbackangle
 	Version $Id: mbbackangle.l,v 5.6 2002-09-19 00:28:12 caress Exp $
 	MB-system Version 5.0.beta20
 	
 	Pings to average:    25
 	Number of angle bins: 161
 	Maximum angle:         80.000000
 	Default altitude:      0.000000
        Working on beam amplitude data...
 	Working on sidescan data...
 	
 	processing swath file: 0003_20020425_022926.mb57 57
 	6 tables written to 0003_20020425_022926.mb57.aga
 	6 tables written to 0003_20020425_022926.mb57.sga
 	131 records processed
 	97128 data processed
 	
 	6 total tables written
 	131 total records processed
 	97128 total data processed

The \fBmbprocess\fP parameter file has also been either
created (if necessary) or modified to enable sidescan
correction using the ".sga" tables. The first of the six tables
in 0003_20020425_022926.mb57.sga looks like:
 	## Sidescan correction table files generated by program mbbackangle
 	## Version $Id: mbbackangle.l,v 5.6 2002-09-19 00:28:12 caress Exp $
 	## MB-system Version 5.0.beta20
 	## Table file format: 1.0.0
 	## Run by user <caress> on cpu <nbp-128-146.nbp.usap.gov> at <Sun Jul 21 17:01:11 2002>
 	## Input swath file:      0003_20020425_022926.mb57
 	## Input swath format:    57
 	## Output table file:     0003_20020425_022926.mb57.sga
 	## Pings to average:      25
 	## Number of angle bins:  161
 	## Maximum angle:         80.000000
 	## Default altitude:      0.000000
 	## Data type:             sidescan
 	# table: 0
 	# nping: 25
 	# time:  2002/04/25 02:30:49.674399    1019701849.674400
 	# nangles: 161
 	-80.0000       0.0000       0.0000
 	-79.0000       0.0000       0.0000
 	-78.0000       0.0000       0.0000
 	-77.0000       0.0000       0.0000
 	-76.0000       0.0000       0.0000
 	-75.0000       0.0000       0.0000
 	-74.0000       0.0000       0.0000
 	-73.0000       0.0000       0.0000
 	-72.0000      32.2739       5.1205
 	-71.0000      33.8169       4.5096
 	-70.0000      32.5683       4.1676
 	-69.0000      33.3233       4.5819
 	-68.0000      33.2733       4.3618
 	-67.0000      32.7095       4.5116
 	-66.0000      33.1693       4.4164
 	-65.0000      33.4931       4.2788
 	-64.0000      33.4072       4.2757
 	-63.0000      33.6430       4.5740
 	-62.0000      33.7174       4.4539
 	-61.0000      33.4760       4.2332
 	-60.0000      33.4768       4.3316
 	-59.0000      33.7259       4.5100
 	-58.0000      33.7861       4.2651
 	-57.0000      33.9738       4.4992
 	-56.0000      33.7753       4.6273
 	-55.0000      34.0484       4.4099
 	-54.0000      34.3366       4.5408
 	-53.0000      34.1053       4.2912
 	-52.0000      34.3885       4.2946
 	-51.0000      34.3664       4.4912
 	-50.0000      34.7203       4.2428
 	-49.0000      34.5826       4.3700
 	-48.0000      34.7410       4.6414
 	-47.0000      35.4736       4.1617
 	-46.0000      34.8456       4.7846
 	-45.0000      35.6649       4.6107
 	-44.0000      36.0851       5.3508
 	-43.0000      36.6807       4.3003
 	-42.0000      36.3087       4.2519
 	-41.0000      36.6893       4.2447
 	-40.0000      36.2372       4.3403
 	-39.0000      36.2227       4.3308
 	-38.0000      36.0223       4.1694
 	-37.0000      36.2182       4.3674
 	-36.0000      36.3618       4.4440
 	-35.0000      36.1489       4.2543
 	-34.0000      36.2703       4.4830
 	-33.0000      36.6548       4.9095
 	-32.0000      36.7752       4.8813
 	-31.0000      37.0692       4.7108
 	-30.0000      37.3190       4.5877
 	-29.0000      37.2481       4.8750
 	-28.0000      36.8861       5.6123
 	-27.0000      37.1561       5.0761
 	-26.0000      37.6518       5.3463
 	-25.0000      38.1557       5.5696
 	-24.0000      37.5547       5.8475
 	-23.0000      37.5606       5.6230
 	-22.0000      37.3771       5.3460
 	-21.0000      38.0293       5.5573
 	-20.0000      37.6278       5.4099
 	-19.0000      37.5382       5.1906
 	-18.0000      37.1919       6.3249
 	-17.0000      37.0054       6.1842
 	-16.0000      37.5196       5.8413
 	-15.0000      37.0851       5.6107
 	-14.0000      37.6514       5.3717
 	-13.0000      38.8210       5.1755
 	-12.0000      37.8904       5.2884
 	-11.0000      38.2412       5.9632
 	-10.0000      38.1898       5.8135
 	-9.0000      37.8771       7.7696
 	-8.0000      36.5629       7.1477
 	-7.0000      37.5029       5.9113
 	-6.0000      36.4109       6.0910
 	-5.0000      37.3796       6.4721
 	-4.0000      39.5517       4.8289
 	-3.0000      38.5109       6.5044
 	-2.0000      39.4625       5.2427
 	-1.0000      42.5517       4.8802
 	 0.0000      38.9211       5.7999
 	 1.0000      42.5517       4.8802
 	 2.0000      39.4625       5.2427
 	 3.0000      38.5109       6.5044
 	 4.0000      39.5517       4.8289
 	 5.0000      37.3796       6.4721
 	 6.0000      36.4109       6.0910
 	 7.0000      37.5029       5.9113
 	 8.0000      36.5629       7.1477
 	 9.0000      37.8771       7.7696
 	10.0000      38.1898       5.8135
 	11.0000      38.2412       5.9632
 	12.0000      37.8904       5.2884
 	13.0000      38.8210       5.1755
 	14.0000      37.6514       5.3717
 	15.0000      37.0851       5.6107
 	16.0000      37.5196       5.8413
 	17.0000      37.0054       6.1842
 	18.0000      37.1919       6.3249
 	19.0000      37.5382       5.1906
 	20.0000      37.6278       5.4099
 	21.0000      38.0293       5.5573
 	22.0000      37.3771       5.3460
 	23.0000      37.5606       5.6230
 	24.0000      37.5547       5.8475
 	25.0000      38.1557       5.5696
 	26.0000      37.6518       5.3463
 	27.0000      37.1561       5.0761
 	28.0000      36.8861       5.6123
 	29.0000      37.2481       4.8750
 	30.0000      37.3190       4.5877
 	31.0000      37.0692       4.7108
 	32.0000      36.7752       4.8813
 	33.0000      36.6548       4.9095
 	34.0000      36.2703       4.4830
 	35.0000      36.1489       4.2543
 	36.0000      36.3618       4.4440
 	37.0000      36.2182       4.3674
 	38.0000      36.0223       4.1694
 	39.0000      36.2227       4.3308
 	40.0000      36.2372       4.3403
 	41.0000      36.6893       4.2447
 	42.0000      36.3087       4.2519
 	43.0000      36.6807       4.3003
 	44.0000      36.0851       5.3508
 	45.0000      35.6649       4.6107
 	46.0000      34.8456       4.7846
 	47.0000      35.4736       4.1617
 	48.0000      34.7410       4.6414
 	49.0000      34.5826       4.3700
 	50.0000      34.7203       4.2428
 	51.0000      34.3664       4.4912
 	52.0000      34.3885       4.2946
 	53.0000      34.1053       4.2912
 	54.0000      34.3366       4.5408
 	55.0000      34.0484       4.4099
 	56.0000      33.7753       4.6273
 	57.0000      33.9738       4.4992
 	58.0000      33.7861       4.2651
 	59.0000      33.7259       4.5100
 	60.0000      33.4768       4.3316
 	61.0000      33.4760       4.2332
 	62.0000      33.7174       4.4539
 	63.0000      33.6430       4.5740
 	64.0000      33.4072       4.2757
 	65.0000      33.4931       4.2788
 	66.0000      33.1693       4.4164
 	67.0000      32.7095       4.5116
 	68.0000      33.2733       4.3618
 	69.0000      33.3233       4.5819
 	70.0000      32.5683       4.1676
 	71.0000      33.8169       4.5096
 	72.0000      32.2739       5.1205
 	73.0000       0.0000       0.0000
 	74.0000       0.0000       0.0000
 	75.0000       0.0000       0.0000
 	76.0000       0.0000       0.0000
 	77.0000       0.0000       0.0000
 	78.0000       0.0000       0.0000
 	79.0000       0.0000       0.0000
 	80.0000       0.0000       0.0000
 	#
 	#


.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbanglecorrect\fP(l), \fBmbfilter\fP(l)

.SH BUGS
None worth bragging about.
