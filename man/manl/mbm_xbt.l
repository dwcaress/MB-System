.TH mbm_xbt l "5 October 1998" "MB-System 4.6" "MB-System 4.6"
.SH NAME
\fBmbm_xbt\fP - Calculate sound velocity from XBT data.

.SH VERSION
Version 4.5

.SH SYNOPSIS
\fBmbm_xbt\fP \fB-I\fIinfile\fP [\fB-F\fI\fIformat\fP 
\fB-L\fIlatitude\fP \fB-S\fIsalinity\fP \fB-V -H\fP]

.SH DESCRIPTION
\fBmbm_xbt\fP is a perl shellscript to translate various XBT data sets
from depth and temperature into depth and sound velocity.  Header 
lines are  turned into comments beginning with '#' characters. The 
output filename consists of the input filename with the suffix ".sv".
The conversion is done using the DelGrosso equation as referenced 
in Dusha et al. [1993] (reference below).

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

.SH OPTIONS
.TP
.B \-F
\fIformat\fP
.br
Allows the user to select the type of XBT data. Currently supported
formats are: 1 = Sparton XBT, 2 = MK12 XBT. Default: \fIformat\fP = 1.
.TP
.B \-H
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \-I
\fIfile\fP
.br
Data file from which the input data will be read.
.TP
.B \-L
\fIlatitude\fP
.br
Latitude of the XBT cast.
.TP
.B \-S
\fIsalinity\fP
.br
Sets the salinity used to calculate velocity from temperature
and depth. The default value of 35 ppt may be significantly
in error in some areas.  Default: \fIsalinity\fP = 35.
.TP
.B \-V
Causes \fBmbm_xbt\fP to operate in "verbose" mode so that it outputs
more information than usual.

.SH SEE ALSO
\fBmbsystem\fP(l), \fBxbt\fP(l), 
\fBmbvelocitytool\fP(l), \fBmblevitus\fP(l)

.SH REFERENCES
Dusha, B. D., P. F. Worcester, B. D. Cornuelle, B. M. Howe, 
On equations for the speed of sound in seawater, J. Acoust. Soc. Am., 
93, 255-275, 1993.

.SH BUGS
Let us know.
