.TH mblevitus l "14 August 1995" "MB-System 4.3" "MB-System 4.3"
.SH NAME
\fBmblevitus\fP - Create a water velocity profile which is representative
of the mean annual water column for a specified 1 degree by 1 degree
region.

.SH VERSION
Version 4.3

.SH SYNOPSIS
\fBmblevitus\fP [\fB-R\fIlon/lat \fB-O\fIoutfile \fB-V -H\fP]

.SH DESCRIPTION
\fBmblevitus\fP generates a mean water velocity profile for a 
specified location from the Levitus temperature and salinity database.
The water velocity profile is representative of the mean annual
water column structure for a specified 1 degree by 1 degree region.
The profile is output to a specified file which can be read and
used by programs (e.g. \fBHSBATH\fP) which calculate multibeam sonar
bathymetry from travel times by raytracing through a water velocity
model.  The depth to which the Levitus database defines the water
column reflects regional water depths which may be exceeded in
places.  \fBmblevitus\fP extrapolates water velocity profiles
to a depth of 12000 meters using a velocity gradient obtained from
the deepest two velocities defined by the Levitus database.
The water velocity values are calculated using standard physical
oceanography corrections for pressure, temperature, and salinity.
The C code which makes these calculations has been translated
directly from Fortran code obtained by L-DEO from Bob Tyce's group
at the University
of Rhode Island (some of this code obviously originated with the 
Shipboard Computer Group at the Scripps Institution of Oceanography
back in the IBM 1800 days of yore).  If the specified location is
subaerial, then no water velocity profile is calculated.

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
.B \fB-H\fP
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \fB-O\fIoutfile\fP
Sets the output file for the water velocity profile. 
Default: \fIoutfile\fP = "velocity".
.TP
.B \fB-R\fIlon/lat\fP
Sets the longitude and latitude of the location of the water
velocity profile.  Whole degrees should be used, as the database
has a 1 degree by 1 degree resolution.
Default: \fIlon\fP=0, \fIlat\fP=0.
.TP
.B \fB-V\fP
Normally, \fBmblevitus\fP outputs only a minor bit of information 
to the stdout stream.  If the
\fB-V\fP flag is given, then \fBmblevitus\fP works in a "verbose" mode and
also outputs the program version being used and the complete
water column structure used to calculate the water velocity profile.

.SH EXAMPLES
Suppose that one wishes to obtain a mean annual water velocity profile
for processing Hydrosweep DS bathymetry data collected on the 
Cocos-Nazca spreading center at 95W longitude and 2N latitude.
The following will suffice:
.br
 	mblevitus -R-95/2 -Ovelocity_profile -V
.br
The output will be:

 	Program mblevitus
 	Version 4.00
 	MB-system Version 4.00

 	Location for mean annual water velocity profile:
 	  Requested:  -95.0000 longitude   2.0000 latitude
 	  Used:       -95.0000 longitude   2.0000 latitude
 	Velocity points defined by Levitus database: 27
 	Velocity points extrapolated to depth:       19
 	Velocity points written:                     46
 	Output file: velocity_profile

 	Mean annual water column profile:
 	     Depth Temperature Salinity   Velocity
 	    0.0000   25.6560   33.8830   1535.1179
 	   10.0000   25.4080   33.8230   1534.6371
 	   20.0000   24.5410   34.0190   1532.9641
 	   30.0000   23.3290   34.2400   1530.4181
 	   50.0000   19.7940   34.7450   1522.0782
 	   75.0000   15.9700   34.9210   1511.5450
 	  100.0000   14.5630   34.9400   1507.5586
 	  125.0000   13.9500   34.9320   1505.9731
 	  150.0000   13.5920   34.9270   1505.2008
 	  200.0000   13.0430   34.9100   1504.1730
 	  250.0000   12.4510   34.8780   1502.9521
 	  300.0000   11.4720   34.8190   1500.3179
 	  400.0000    9.4060   34.6990   1494.3768
 	  500.0000    8.0400   34.6390   1490.8097
 	  600.0000    7.0330   34.5950   1488.5027
 	  700.0000    6.2200   34.5740   1486.9109
 	  800.0000    5.5640   34.5620   1485.9110
 	  900.0000    5.0280   34.5620   1485.3845
 	 1000.0000    4.5840   34.5650   1485.2213
 	 1100.0000    4.1950   34.5730   1485.2808
 	 1200.0000    3.8570   34.5820   1485.5466
 	 1300.0000    3.5860   34.5970   1486.0980
 	 1400.0000    3.3320   34.6090   1486.7150
 	 1500.0000    3.1020   34.6140   1487.4231
 	 1750.0000    2.6290   34.6280   1489.6320
 	 2000.0000    2.3010   34.6440   1492.4771
 	 2500.0000    1.8770   34.6670   1499.2097
 	 3000.0000    0.0000    0.0000   1505.9424
 	 3500.0000    0.0000    0.0000   1512.6750
 	 4000.0000    0.0000    0.0000   1519.4077
 	 4500.0000    0.0000    0.0000   1526.1404
 	 5000.0000    0.0000    0.0000   1532.8730
 	 5500.0000    0.0000    0.0000   1539.6057
 	 6000.0000    0.0000    0.0000   1546.3384
 	 6500.0000    0.0000    0.0000   1553.0710
 	 7000.0000    0.0000    0.0000   1559.8037
 	 7500.0000    0.0000    0.0000   1566.5364
 	 8000.0000    0.0000    0.0000   1573.2690
 	 8500.0000    0.0000    0.0000   1580.0017
 	 9000.0000    0.0000    0.0000   1586.7344
 	 9500.0000    0.0000    0.0000   1593.4670
 	10000.0000    0.0000    0.0000   1600.1997
 	10500.0000    0.0000    0.0000   1606.9324
 	11000.0000    0.0000    0.0000   1613.6650
 	11500.0000    0.0000    0.0000   1620.3977
 	12000.0000    0.0000    0.0000   1627.1304

The contents of the output file velocity_profile are:

 	# Water velocity profile created by program mblevitus version 3.00
 	# MB-system Version 4.00
 	# Run by user <caress> on cpu <menard> at <Tue Mar 1 12:12:39 1994>
 	# Water velocity profile derived from Levitus
 	# temperature and salinity database.  This profile
 	# represents the annual average water velocity
 	# structure for a 1 degree X 1 degree area centered
 	# at 265.0000 longitude and 2.0000 latitude.
 	# This water velocity profile is in the form
 	# of discrete (depth, velocity) points where
 	# the depth is in meters and the velocity in
 	# meters/second.
 	# The first 27 velocity values are defined using the
 	# Levitus database; the remaining 19 points are
 	# extrapolated using a constant velocity gradient
 	# obtained from the two deepest known points.
 	0.000000 1535.117920
 	10.000000 1534.637085
 	20.000000 1532.964111
 	30.000000 1530.418091
 	50.000000 1522.078247
 	75.000000 1511.545044
 	100.000000 1507.558594
 	125.000000 1505.973145
 	150.000000 1505.200806
 	200.000000 1504.172974
 	250.000000 1502.952148
 	300.000000 1500.317871
 	400.000000 1494.376831
 	500.000000 1490.809692
 	600.000000 1488.502686
 	700.000000 1486.910889
 	800.000000 1485.911011
 	900.000000 1485.384521
 	1000.000000 1485.221313
 	1100.000000 1485.280762
 	1200.000000 1485.546631
 	1300.000000 1486.098022
 	1400.000000 1486.714966
 	1500.000000 1487.423096
 	1750.000000 1489.631958
 	2000.000000 1492.477051
 	2500.000000 1499.209717
 	3000.000000 1505.942383
 	3500.000000 1512.675049
 	4000.000000 1519.407715
 	4500.000000 1526.140381
 	5000.000000 1532.873047
 	5500.000000 1539.605713
 	6000.000000 1546.338379
 	6500.000000 1553.071045
 	7000.000000 1559.803711
 	7500.000000 1566.536377
 	8000.000000 1573.269043
 	8500.000000 1580.001709
 	9000.000000 1586.734375
 	9500.000000 1593.467041
 	10000.000000 1600.199707
 	10500.000000 1606.932373
 	11000.000000 1613.665039
 	11500.000000 1620.397705
 	12000.000000 1627.130371

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbvelocitytool\fP(l), \fBmbbath\fP(l), 
\fBmbm_xbt\fP(l), \fBxbt\fP(l)

.SH BUGS
None known.
