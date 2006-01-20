.TH mbbackangle l "28 September 2004" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbbackangle\fP - Generates tables of the average amplitude or 
sidescan values in swath sonar data as a function of 
the grazing angle with the seafloor.

.SH VERSION
Version 5.0

.SH SYNOPSIS
\fBmbbackangle\fP [\fB-A\fIkind\fP 
\fB-C\fP \fB-D\fP \fB-F\fIformat\fP  
\fB-G\fIkind/angle/max/nx/ny\fP
\fB-I\fIfile\fP  
\fB-N\fInangle/anglemax\fP \fB-P\fIpings\fP \fB-Q\fP
\fB-Z\fIaltitude\fP \fB-V -H\fP]

.SH DESCRIPTION
The program \fBmbbackangle\fP reads a swath sonar data file 
containing beam amplitude and/or sidescan data and generates tables
of the average amplitude and/or sidescan values as a function of
the grazing angle with the seafloor. These tables can be used
by the program \fBmbprocess\fP to correct the sidescan or
amplitude data for the variation with grazing angle. No assumption
is made about the nature of the data or the sonar used to
collect it. 

The user specifies the angular width 
of the swath considered and the number of angular bins in that
swath; the average amplitude or sidescan value within each bin
form the output data. By default, the grazing angle is calculated simply using the
acrosstrack distance and depth at the beam or pixel location. 
If bathymetry is not available, the seafloor is assumed to be flat 
with a sonar altitude specified by the \fB-Z\fP option.
If the \fB-Q\fP option is used, then the acrosstrack bottom slope
at the beam or pixel location is factored into the grazing angle
values. For bins outside the nadir region (angles > 15.0 degrees
from vertical), the averaging extends over the neighboring bins (three bins
averaged for each value). The user also specifies the number of pings
to be read to generate each table; the default is 50 pings. 

The results are written to a file named by 
adding a ".sga" suffix to the swath filename for sidescan and
".aga" for amplitude. These files contain a series of amplitude vs.
grazing angle tables, each time tagged according to the average time
of the pings used to generate that table. The swath file's 
\fBmbprocess\fP parameter file is also modified (or created if necessary)
so that sidescan or amplitude correction is enabled. The correction
tables are read and interpolated according to ping time by \fBmbprocess\fP.

If the \fB-G\fP option is used, then \fBmbbackangle\fP also generates
gridded histograms of the amplitude versus grazing angle values for
each swath data file processed. The user specifies the bounds and dimensions
of the grids; the \fB-G\fP option must be given twice to produce grids
of both amplitude and sidescan histograms. In addition to outputting the
grids, \fBmbbackangle\fP uses \fBmbm_grdplot\fP to generate \fBGMT\fP
shellscripts that, when executed, will generate plots of the gridded
histograms overlain with the amplitude versus grazing angle tables in
the ".aga" and ".sga" files.

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
.B \-G
\fIkind/angle/max/nx/ny\fP
.br
This option causes \fBmbbackangle\fP to output gridded histograms
of the amplitude versus grazing angle data for each swath file
processed. The program also generates a shellscript to produce a
first-cut \fBGMT\fP postscript plot of the histogram overlain by
the amplitude versus grazing angle tables used by \fBmbprocess\fP.
The \fIkind\fP parameter indicates whether an amplitude (\fIkind\fP = 1)
or sidescan (\fIkind\fP = 2) histogram is desired; the \fB-G\fP command
must be given twice (once with \fIkind\fP = 1 and once with \fIkind\fP = 2)
to generate both amplitude and sidescan histograms. The histogram grid
consists of \fInx\fP bins extending from -\fIangle\fP to +\fIangle\fP degrees in the
x-dimension, and \fIny\fP bins extending from 0.0 to \fImax\fP in the
y-dimension. The values of the histogram are normalized so that they sum
to 1000.0 within each angle bin.
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
This program uses the \fBMBIO\fP library and will read any swath sonar
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
the end of the file. Default: \fIpings\fP = 50.
.TP
.B \-Q
By default, \fBmbbackangle\fP does not use seafloor slopes in
calculating the grazing angles for each amplitude beam and sidescan
pixel. This option causes the program to calculate acrosstrack slopes
from swath bathymetry, if available, and to factor these slopes
into the grazing angle calculation for each data point. In general,
this option does not work well.
slopes 
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
which contains 131 pings. In order to correct the beam amplitude and sidescan 
with mbprocess, first use \fBmbbackangle\fP to generate the
amplitude vs grazing angle tables:
 	mbbackangle -I mbari_1998_988_msn.mb57 \\
 			-N81/80.0 -P50 \\
 			-G1/80/60/81/121 \\
 			-G2/80/60/81/121 \\
 			-V
.br
Here the angle bins will be 2.0 degree wide, the table
will extend from -80.0 degees (port) to +80.0 (starboard),
and each table will represent the averages over up to 50
pings. The output looks like:

 	Program mbbackangle
 	Version $Id: mbbackangle.l,v 5.9 2006-01-20 15:55:39 caress Exp $
 	MB-system Version 5.0.4
 	
 	Pings to average:    50
 	Number of angle bins: 81
 	Maximum angle:         80.000000
 	Default altitude:      0.000000
 	Working on beam amplitude data...
 	Working on sidescan data...
 	Using flat bottom assumption in calculating correction tables...
 	Outputting gridded histograms of beam amplitude vs grazing angle...
 	Outputting gridded histograms of sidescan amplitude vs grazing angle...
 	
 	processing swath file: mbari_1998_988_msn.mb57 57
 	
 	7274 records processed
 	805564 amplitude data processed
 	146 tables written to mbari_1998_988_msn.mb57.aga
 	5371412 sidescan data processed
 	146 tables written to mbari_1998_988_msn.mb57.sga

 	Plot generation shellscript <mbari_1998_988_msn.mb57_aga.grd.cmd> created.
 	Plot generation shellscript <mbari_1998_988_msn.mb57_sga.grd.cmd> created.
 	
 	7274 total records processed
 	805564 total amplitude data processed
 	146 total aga tables written
 	5371412 total sidescan data processed
 	146 total sga tables written

The output files include the amplitude versus grazing angle tables in
mbari_1998_988_msn.mb57.aga and mbari_1998_988_msn.mb57.sga, the
gridded histogram files mbari_1998_988_msn.mb57_aga.grd and 
mbari_1998_988_msn.mb57_sga.grd, and the plotting shellscripts 
mbari_1998_988_msn.mb57_aga.grd.cmd and 
mbari_1998_988_msn.mb57_sga.grd.cmd. The \fBmbprocess\fP parameter file 
mbari_1998_988_msn.mb57.par has also been either
created (if necessary) or modified to enable sidescan
correction using the ".sga" tables. The first of the six tables
in mbari_1998_988_msn.mb57.sga looks like:
 	## Sidescan correction table files generated by program mbbackangle
 	## Version $Id: mbbackangle.l,v 5.9 2006-01-20 15:55:39 caress Exp $
 	## MB-system Version 5.0.4
 	## Table file format: 1.0.0
 	## Run by user <caress> on cpu <hess> at <Fri Oct  1 11:18:30 2004>
 	## Input swath file:      mbari_1998_988_msn.mb57
 	## Input swath format:    57
 	## Output table file:     mbari_1998_988_msn.mb57.sga
 	## Pings to average:      50
 	## Number of angle bins:  81
 	## Maximum angle:         80.000000
 	## Default altitude:      0.000000
 	## Data type:             sidescan
 	# table: 0
 	# nping: 50
 	# time:  1998/06/10 14:55:24.979779    897490524.979780
 	# nangles: 81
 	-80.0000       0.0000       0.0000
 	-78.0000       0.0000       0.0000
 	-76.0000       0.0000       0.0000
 	-74.0000       0.0000       0.0000
 	-72.0000       0.0000       0.0000
 	-70.0000       0.0000       0.0000
 	-68.0000       0.0000       0.0000
 	-66.0000       0.0000       0.0000
 	-64.0000      33.4828       7.5915
 	-62.0000      32.4068       5.4016
 	-60.0000      31.9667       5.3320
 	-58.0000      31.7880       5.1880
 	-56.0000      31.6249       5.0681
 	-54.0000      31.6362       4.9521
 	-52.0000      31.5590       5.0482
 	-50.0000      31.3955       5.1242
 	-48.0000      31.0372       5.2364
 	-46.0000      30.9569       5.1092
 	-44.0000      30.8126       4.9848
 	-42.0000      30.3497       4.9611
 	-40.0000      29.7660       4.9479
 	-38.0000      29.2746       4.9727
 	-36.0000      29.1929       4.9734
 	-34.0000      29.1729       4.9893
 	-32.0000      28.7375       5.0295
 	-30.0000      28.6560       5.0031
 	-28.0000      28.0609       5.1757
 	-26.0000      27.3652       5.2692
 	-24.0000      26.2536       5.1193
 	-22.0000      25.2113       4.8437
 	-20.0000      24.9033       4.5717
 	-18.0000      24.4118       4.5952
 	-16.0000      24.4341       4.5824
 	-14.0000      24.3251       4.7864
 	-12.0000      23.6965       4.1409
 	-10.0000      24.1326       3.7653
 	-8.0000      26.1912       4.2246
 	-6.0000      27.9516       4.5724
 	-4.0000      31.1992       5.6401
 	-2.0000      29.2624       5.8076
 	 0.0000      27.4466       4.3310
 	 2.0000      24.9780       4.1345
 	 4.0000      24.2328       4.0980
 	 6.0000      23.3917       4.6856
 	 8.0000      24.0867       4.5882
 	10.0000      23.2293       4.6383
 	12.0000      24.1375       4.5005
 	14.0000      25.1402       4.6843
 	16.0000      25.1948       4.9131
 	18.0000      25.0986       5.0503
 	20.0000      25.0798       5.0769
 	22.0000      25.0582       5.1801
 	24.0000      25.6108       5.1108
 	26.0000      25.8393       5.0068
 	28.0000      26.3232       4.9469
 	30.0000      26.4346       5.0686
 	32.0000      26.6474       5.1690
 	34.0000      27.0092       5.2098
 	36.0000      27.1987       5.1664
 	38.0000      27.3710       5.2504
 	40.0000      27.1160       5.1788
 	42.0000      27.0691       5.1418
 	44.0000      27.2547       4.8907
 	46.0000      27.7406       4.9654
 	48.0000      28.1568       4.9277
 	50.0000      28.4015       4.9687
 	52.0000      28.4042       4.9034
 	54.0000      28.3741       4.9500
 	56.0000      28.5195       4.9799
 	58.0000      28.6455       5.0142
 	60.0000      28.9284       4.9876
 	62.0000      28.9028       5.0380
 	64.0000       0.0000       0.0000
 	66.0000       0.0000       0.0000
 	68.0000       0.0000       0.0000
 	70.0000       0.0000       0.0000
 	72.0000       0.0000       0.0000
 	74.0000       0.0000       0.0000
 	76.0000       0.0000       0.0000
 	78.0000       0.0000       0.0000
 	80.0000       0.0000       0.0000
 	#
 	#

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbanglecorrect\fP(l), \fBmbfilter\fP(l)

.SH BUGS
None worth bragging about.
