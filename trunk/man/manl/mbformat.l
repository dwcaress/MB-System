.TH mbformat l "17 December 1998" "MB-System 4.6" "MB-System 4.6"
.SH NAME
\fBmbformat\fP -  List information about swath sonar data formats
supported by the \fBMBIO\fP library.

.SH VERSION
Version 4.6

.SH SYNOPSIS
\fBmbformat\fP [\fB -A -F\fIformat \fB -L -V -H\fP]

.SH DESCRIPTION
\fBmbformat\fP is an utility which identifies the swath sonar data formats
associated with \fBmbio\fP format id's.  If no format id is specified,
\fBmbformat\fP lists all of the currently supported formats.
By default, the listing includes a brief description of each format.

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
Causes formats which are expected to be supported
in the future to be
listed in addition to the formats which are fully supported.
.TP
.B \-F
\fIformat\fP
.br
Sets the format id to be listed.  If no format is given, all of the 
currently supported formats will be listed. A list of the swath sonar data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page.
.TP
.B \-H
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \-L
Causes the format descriptions to be omitted 
so that only the \fBMBIO\fP format id numbers are listed.
.TP
.B \-V
Normally, \fBmbformat\fP only prints out format descriptions.  If the
\fB-V\fP flag is given, then \fBmbformat\fP works in a "verbose" mode and
also outputs the program version being used.

.SH EXAMPLES
Suppose one wishes to identify the swath sonar data format associated
with a format id of 11.  The following will suffice:
	mbformat -F11

Suppose one wishes to list all of the currently supported data formats:
The following will suffice:
	mbformat

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbio\fP(l)

.SH BUGS
No known bugs.
