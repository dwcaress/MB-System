.TH MBFORMAT 1 "21 February 1994"
.SH NAME
\fBmbformat\fP -  List information about multibeam bathymetry data formats
supported by the \fBMBIO\fP library.

.SH VERSION
Version 4.00

.SH SYNOPSIS
\fBmbformat\fP [\fB-F\fIformat \fB-V -H\fP]

.SH DESCRIPTION
\fBMBFORMAT\fP is an utility which identifies the multibeam data formats
associated with \fBmbio\fP format id's.  If no format id is specified,
\fBMBFORMAT\fP lists all of the currently supported formats.

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
Sets the format id to be listed.  If no format is given, all of the 
currently supported formats will be listed. A list of the multibeam data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page.
.TP
.B \fB-H\fP
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \fB-V\fP
Normally, \fBMBFORMAT\fP only prints out format descriptions.  If the
\fB-V\fP flag is given, then \fBMBFORMAT\fP works in a "verbose" mode and
also outputs the program version being used.

.SH EXAMPLES
Suppose one wishes to identify the multibeam data format associated
with a format id of 11.  The following will suffice:
	mbformat -F11

Suppose one wishes to list all of the currently supported data formats:
The following will suffice:
	mbformat

.SH BUGS
No known bugs.
