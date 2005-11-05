.TH mbformat l "17 January 2003" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbformat\fP -  List information about swath sonar data formats
supported by the \fBMBIO\fP library.

.SH VERSION
Version 5.0

.SH SYNOPSIS\fBmbformat\fP [\fB -F\fP\fIformat\fP \fB-I\fP\fIfile\fP \fB-L -K -V -H\fP]

.SH DESCRIPTION
\fBmbformat\fP is a utility which identifies the swath sonar data formats
associated with \fBmbio\fP format id's.  If no format id or input filename
is specified, \fBmbformat\fP lists all of the currently supported formats 
along with short descriptions. If a format id is specified using the 
\fB-F\fP option, then \fBmbformat\fP prints a short description of that 
format. If a filename is specified using the \fB-I\fP option, then 
\fBmbformat\fP attempts to identify a valid format id for that file using 
swath data filename prefix and suffix conventions embedded in \fBMB-System\fP. 
The program also determines the filename "root", which is the filename minus 
whatever format-specific prefix or suffix has been recognized. In no prefix
or suffix is recognized, then the root is simply the entire filename.

Normally, \fBmbformat\fP
prints out a brief description of each identified format. If the \fB-L\fP
option is used, \fBmbformat\fP simply prints out the valid format id number,
or 0 if a file or format has been specified but no valid id can be identified.
If the \fB-K\fP option is used, \fBmbformat\fP will print out the filename
"root" followed by the format id number.

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
.B \-I
\fIfile\fP
.br
Sets the filename to be tested against known filename prefix and
suffix conventions in order to identify the format id.
.TP
.B \-K
Causes the fileroot and the format id to be output. The fileroot
is the filename with any format specific suffix removed.
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
.br
 	mbformat -F11
.br
and the result will be this output:
  MBIO data format id: 11
  Format name:          MBF_SBSIOMRG
  Informal Description: SIO merge Sea Beam
  Attributes:           Sea Beam, bathymetry, 16 beams, binary, uncentered,
                        SIO.

Suppose one wishes to list all of the currently supported data formats.
The following will suffice:
.br
 	mbformat

Suppose one wants to find out what format is associated with a raw
SeaBeam 2112 file named sb199811231045.rec:
.br
 	mbformat -I sb199811231045.rec
.br
This results in the output:
  MBIO data format id: 41
  Format name:          MBF_SB2100RW
  Informal Description: SeaBeam 2100 series vender format
  Attributes:           SeaBeam 2100, bathymetry, amplitude 
                        and sidescan, 151 beams and 2000 pixels, ascii 
                        with binary sidescan, SeaBeam Instruments.

Using the -K option:
.br
 	mbformat -K -I sb199811231045.rec
.br
will produce the following output:
.br
  sb199811231045 41
.br
where the format id has been identified as 41 because of the ".rec"
file suffix and the fileroot has been identified as "sb199811231045"
by removing the suffix. Use of the -L option:
.br
 	mbformat -L -I sb199811231045.rec
.br
will produce output with just format id:
.br
  41

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbio\fP(l)

.SH BUGS
No known bugs.
