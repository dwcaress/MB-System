.TH mbrollbias l "31 October 1994"
.SH NAME
\fBmbrollbias\fP - Assess roll bias of multibeam sonar systems.

.SH VERSION
Version 4.1

.SH SYNOPSIS
\fBmbrollbias\fP \fB-D\fIxdim/ydim \fB-R\fIwest/east/south/north \fB-L\fIlonflip \fB-I\fIfile1 \fB-J\fIfile2 [\fB-H -V\fI]

.SH DESCRIPTION
\fBmbrollbias\fP is an utility used to assess roll bias of multibeam 
sonar systems using bathymetry data from two swaths covering the same  
seafloor in opposite directions. The program takes two input  
files and calculates best fitting planes for each dataset.   
The roll bias is calculated by solving for a common roll bias
factor which explains the difference between the seafloor
slopes observed on the two swaths.  This approach assumes that 
pitch bias is not a factor; this assumption is most correct when
the heading of the two shiptracks are exactly opposite. The area is
divided into a number of rectangular regions and calculations are done  
in each region containing a sufficient number of data from both 
swaths.  The data contained in the two datafiles should be processed to
ensure that no artifacts will bias the roll bias calculations.
Typically, this means using \fBMBCLEAN\fP and or \fBMBEDIT\fP to
remove noisy outer beams.
A positive roll bias value means that the the vertical reference 
used by the multibeam system is biased to starboard, 
giving rise to shallow bathymetry to port and deep bathymetry 
to starboard.  Results are written to stdout.

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
.B \fB-F\fIformat1/format2\fP
Sets the \fBMBIO\fP format identifiers for the first and
second data files, respectively.
This program uses the \fBMBIO\fP library and will read or write any multibeam
format supported by \fBMBIO\fP. A list of the multibeam data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page.
.TP
.B \fB-D\fIxdim/ydim\fP
The area specified using the \fB-R\fP option is divided up into
a grid of rectangular regions; roll bias calculations are done
in each of these regions providing sufficient data is available
from both data files.  This option sets the dimensions of the
region grid.  Use \fIxdim\fP = \fIydim\fP = 1 to make one
calculation for the entire area.
Default:  \fIxdim\fP = \fIydim\fP = 1.
.TP
.B \fB-H\fP
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \fB-I\fIfile1\fP
Sets the filename of the first data file to be analyzed.
.TP
.B \fB-J\fIfile2\fP
Sets the filename of the second data file to be analyzed.
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
.B \fB-R\fIwest/east/south/north\fP
Sets the longitude and latitude bounds of the area of the roll bias test.
.TP
.B \fB-V\fP
The \fB-V\fP option causes the program to print out the program name
and version during execution.
.SH EXAMPLE
Suppose you want to calculate roll bias using two coincident swaths
of Hydrosweep DS data collected in nearly opposite directions.
Run \fBmbrollbias\fP as follows:

 	mbrollbias -R-51.07/-50.98/9.0166/9.117 -D1/1 -F22/22 
 		-Ihs_leg1_z.mb22 -Jhs_leg2_z.mb22 -V

The output looks like:

 Program mbrollbias
 Version $Id: mbrollbias.l,v 4.1 1994-10-31 21:54:07 caress Exp $
 MB-system Version 4.00

 mbrollbias Parameters:
 Input file 1:     hs_leg1_z.mb22
 Input file 2:     hs_leg2_z.mb22
 Region grid bounds:
   Longitude:  -51.0700  -50.9800
   Latitude:     9.0166    9.1170
 Region grid dimensions: 1 1
 Longitude interval: 0.090000 degrees or 9.894392 km
 Latitude interval:  0.100400 degrees or 11.104426 km
 Longitude flipping:   0
 
 4840 depth points counted in hs_leg1_z.mb22
 1733 depth points counted in hs_leg2_z.mb22
 4840 depth points read from hs_leg1_z.mb22
 1733 depth points read from hs_leg2_z.mb22
 
 Region 0 (0 0) bounds:
     Longitude:  -51.0700  -50.9800
     Latitude:     9.0166    9.1170
 First data file:    hs_leg1_z.mb22
     Number of data: 4840
     Mean heading:   107.845062
     Plane fit:      4.759709 -0.002327 -0.004623
 Second data file:   hs_leg2_z.mb22
     Number of data: 1733
     Mean heading:   302.022793
     Plane fit:      4.640304 0.008438 0.008494
 Roll bias:   -0.007288 (-0.417598 degrees)
 Roll bias is positive to port, negative to starboard.
 A postive roll bias means the ship rolls to port, 
     giving rise to shallow bathymetry to port and 
     deep bathymetry to starboard.

.SH BUGS
Probably.
