.TH mbm_xbt 1 "26 Februrary 1994"
.SH NAME
\fBmbm_xbt\fP - Calculate sound velocity from XBT data.

.SH VERSION
Version 4.00

.SH SYNOPSIS
\fBmbm_xbt\fP infilename -Ssalinity -Fformat -V -H

.SH DESCRIPTION
\fBmbm_xbt\fP is a perl shellscript to translate various XBT data sets
from depth and temperature into depth and sound velocity.  Header 
lines are  turned into comments beginning with '#' characters. The 
output filename consists of the input filename with the suffix ".sv".
The conversion is done with Medwin's (1975) equation as referenced 
in "Acoustical Oceanography" by Clay and Medwin, page 88. 
Latitude and depth (pressure) corrections are not applied in this
conversion.

.SH AUTHORSHIP
David W. Caress (caress@lamont.ldeo.columbia.edu)
.br
Dale N. Chayes (dale@lamont.ldeo.columbia.edu)
.br
Suzanne H. O'Hara (sohara@lamont.ldeo.columbia.edu)
.br
 Lamont-Doherty Earth Observatory
.br
Palisades, NY 10964
.br
David Brock (brockda.asa@ass.org)
.br
 Antarctic Support Associates
.br
Denver, CO 

.SH OPTIONS
.TP
.B \fB-H\fP
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \fB-I\fIfile\fP
Data file from which the input data will be read.
.TP
.B \fB-S\fIsalinity\fP
Sets the salinity used to calculate velocity from temperature
and depth. The default value of 35 ppt may be significantly
in error in some areas.  Default: \fIsalinity\fP = 35.
.TP
.B \fB-F\fIformat\fP
Allows the user to select the type of XBT data. Currently supported
formats are: 1 = Sparton XBT, 2 = MK12 XBT. Default: \fIformat\fp = 1.
.TP
.B \fB-V\fP
Causes \fBmbm_plot\fP to operate in "verbose" mode so that it outputs
more information than usual.

.SH BUGS
Perhaps the output should be sent to written to stdout. Suggestions?
