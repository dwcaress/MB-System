.TH mbmask 1 "24 February 1994"
.SH NAME
\fBmbmask\fP, \fBmbgetmask\fP - Extract multibeam data flags from one
file and apply to another.

.SH VERSION
Version 4.00

.SH SYNOPSIS
\fBmbgetmask\fP [\fB-F\fPformat \fB-B\fPyr/mo/da/hr/mn/sc \fB-E\fPyr/mo/da/hr/mn/sc \fB-S\fPspeed \fB-I\fPinfile \fB-V -H\fP]
.br
\fBmbmask\fP [\fB-F\fIformat \fB-M\fImaskfile\fP \fB-I\fIinfile \fB-O\fIoutfile \fB-V -H\fP]

.SH DESCRIPTION
Multibeam bathymetry data is typically edited either with interactive
tools such as \fBmbedit\fP or automatic filters such as \fBmbclean\fP.
The beams which are deemed "bad" are flagged by having the bathymetry
value set negative.  Occasionally, the need arises to translate data
flagging information from one file to another which represents a
different version of the same dataset.  In particular, it may happen
that editing has been done on a data format which does not support
all of the original data stream (e.g. the edmb Hydrosweep format 22,
which does not contain travel times or amplitudes), but the user now
wishes to work with the complete data stream without having to redo
the editing process. 

The two utilities \fBmbgetmask\fP and \fBmbmask\fP 
allow the user to extract the flagging information in the form of a
"mask" file from the edited file and then to apply the flagging mask
to another file containing a different version of the same data.
The first utility, \fBmbgetmask\fP, is used to extract the mask from
the edited multibeam file; the output is written to stdout.  The
second utility, \fBmbmask\fP, reads in the mask file and the unedited
multibeam data file and flags all beams indicated by the mask.
The mask is keyed to the times of the individual pings; only exactly
corresponding pings will be affected by the mask.

.SH AUTHORSHIP
David W. Caress (caress@lamont.ldeo.columbia.edu)
.br
Dale N. Chayes (dale@lamont.ldeo.columbia.edu)
.br
Lamont-Doherty Earth Observatory
.br
Palisades, NY 10964

.SH MBGETMASK OPTIONS
.TP
.B \fB-B\fIyr/mo/da/hr/mn/sc\fP
Sets the starting time for data allowed in the input data; pings
with times before the starting time will be ignored. 
Default: \fIyr/mo/da/hr/mn/sc\fP = 1962/2/21/10/30/0.
.TP
.B \fB-E\fIyr/mo/da/hr/mn/sc\fP
Sets the ending time for data allowed in the input data; pings
with times after the ending time will be ignored. 
Default: \fIyr/mo/da/hr/mn/sc\fP = 2062/2/21/10/30/0.
.TP
.B \fB-F\fIformat\fP
Sets the format for the input and output multibeam data using 
\fBMBIO\fP integer format identifiers. 
These utilities uses the \fBMBIO\fP library and will read any multibeam
format supported by \fBMBIO\fP. A list of the multibeam data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page.
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
.B \fB-S\fIspeed\fP
Sets the minimum speed in km/hr (5.5 kts ~ 10 km/hr) allowed in 
the input data; pings associated with a smaller ship speed will not be
copied. Default: \fIspeed\fP = 0.
.TP
.B \fB-V\fP
Normally, \fBmbgetmask\fP works "silently" without outputting
anything to the stderr stream.  If the
\fB-V\fP flag is given, then \fBmbmask\fP works in a "verbose" mode and
outputs the program version being used and all error status messages.

.SH MBMASK OPTIONS
.TP
.B \fB-H\fP
This "help" flag causes the program to print out a description
of its operation and then exit immediately.
.TP
.B \fB-F\fIformat\fP
Sets the format for the input and output multibeam data using 
\fBMBIO\fP integer format identifiers. 
These utilities uses the \fBMBIO\fP library and will read any multibeam
format supported by \fBMBIO\fP. A list of the multibeam data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page.
.TP
.B \fB-I\fIinfile\fP
Data file from which the input data will be read. If
no input file is specified, the input will be read
from stdin. Default: \fIinfile\fP = stdin.
.TP
.B \fB-I\fImaskfile\fP
Input file containing a data flagging mask created by \fBmbgetmask\fP.
.TP
.B \fB-I\fIoutfile\fP
Data file to which the output data will be written. If
no output file is specified, the output will be written to
stdout. Default: \fIoutfile\fP = stdout.
.TP
.B \fB-V\fP
Normally, \fBmbmask\fP works "silently" without outputting
anything to the stderr stream.  If the
\fB-V\fP flag is given, then \fBmbmask\fP works in a "verbose" mode and
outputs the program version being used and all error status messages.

.SH EXAMPLES
Suppose one has an edmb format (\fBMBIO\fP format id 22) Hydrosweep
file called hs_ew9103_143.mb22 which has been edited until the
data is just right. Further suppose that one's thesis advisor
suggests that the bathymetry would be better if it were recalculated
from travel times using a new water velocity model, requiring that
the raw Hydrosweep data (originally collected in format 5 but
recently translated to format 24) in the file hs_ew9103_143.mb24
be used. First extract the editing information in the form of
a flagging mask file from the edited edmb file:
 	mbgetmask -F22 -Ihs_ew9103_143.mb22 > hs_ew9103_143.mask
.br
Then, apply the mask to the raw file as follows:
 	mbmask -F24 -Mhs_ew9103_143.mask -Ihs_ew9103_143.mb24 -Ohs_ew9103_143_m.mb24
.SH BUGS
The program \fBmbmask\fP occasionally seems to hang on the last 
data record.  We will probably fix this the next time we have to use it.
