.TH hsdump l "14 February 1995"
.SH NAME
\fBhsdump\fP - hsdump lists the information contained in Hydrosweep DS data files.

.SH VERSION
Version 4.2

.SH SYNOPSIS
\fBhsdump\fP [\fB-F\fIformat \fB-I\fIinfile \fB-O\fIkind \fB-V -H\fP]

.SH DESCRIPTION
\fBhsdump\fP hsdump lists the information contained in data records on 
Hydrosweep DS data files, including survey, calibrate, water velocity 
and comment records. The default input stream is stdin.

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
.B \fB-F\fIformat\fP
Sets the format for the input and output multibeam data using 
\fBMBIO\fP integer format identifiers. 
This program is specific to Hydrosweep DS data, and cannot be used 
with data from any other multibeam system or with data in a Hydrosweep
format that does not include water velocity records. Currently, the two formats
which can be used are the raw Hydrosweep DS format (\fBMBIO\fP id 21)
and the L-DEO in-house binary Hydrosweep format (\fBMBIO\fP id 24).
Default: \fIformat\fP = 5.
.TP
.B \fB-H\fP
This "help" flag causes the program to print out a description
of its operation and then exit immediately.
.TP
.B \fB-I\fIinfile\fP
Data file from which the input data will be read. If
no input file is specified, the input will be read
from stdin. Default: \fIinfile\fP = stdin.
.TP
.B \fB-O\fIkind\fP
Sets the kind of data record which will be listed, where the choices
for kind include:
 	1:	Survey data record
 	2:	Comment record
 	3:	Calibrate data record
 	4:	Mean velocity data record
 	5:	Velocity profile data record
 	6:	Standby data record
 	7:	Navigation source data record
.br
This option can be given multiple times to set more than one
kind of data record for listing.
.TP
.B \fB-V\fP
Normally, \fBhsdump\fP lists only the contents of the data records 
which are specified with the \fB-O\fP option.  If the
\fB-V\fP flag is given, then \fBhsdump\fP works in a "verbose" mode and
outputs the program version being used, any comment records encountered
in the data stream, and all error status messages.

.SH EXAMPLES
Suppose one wishes to list out the water velocity records 
contained in the raw Hydrosweep (format 21) file hsraw.
The following will suffice:
 	hsdump -F21 -O4 -O5 -Ihsraw

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbinfo\fP(l), \fBmblist\fP(l)

.SH BUGS
You tell me.
