.TH mbsvplist l "4 January 2000" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbsvplist\fP - List water sound velocity profiles in swath sonar data files.

.SH VERSION
Version 5.0

.SH SYNOPSIS
\fBmbsvplist\fP [\fB-D \fB-F\fIformat \fB-H \fB-I\fIfile \fB-O -V\fP]

.SH DESCRIPTION
This program, \fBmbsvplist\fP, lists all water 
sound velocity profiles (SVPs) within swath data files. 
Swath bathymetry is calculated from raw angles and travel
times by raytracing through a model of the speed of sound
in water. Many swath data formats allow SVPs to be 
embedded in the data, and often the SVPs used to 
calculate the data will be included. 
By default, all unique SVPs encountered are 
listed to stdout. The SVPs may instead be 
written to individual files with names FILE_XXX.svp, 
where FILE is the swath data filename and XXX is the
SVP count within the file. Some files contain SVPs at
regular intervals, typically with the same SVP duplicated
many times. By default, \fBmbsvplist\fP ignores duplicate
SVPs. The \fB-D\fP option causes duplicate SVPs to be output. 
.SH AUTHORSHIP
David W. Caress (caress@mbari.org)
.br
  Monterey Bay Aquarium Research Institute
.br
Dale N. Chayes (dale@ldeo.columbia.edu)
.br
  Lamont-Doherty Earth Observatory
.br

.SH OPTIONS
.TP
.B \-D
.br
Causes \fBmbsvplist\fI to output duplicate SVPs.
.TP
.B \-F
\fIformat\fP
.br
Sets the format for the input swath sonar data using 
\fBMBIO\fP integer format identifiers. 
If \fIformat\fP < 0, then the input
file specified with the \fB-I\fP 
option will actually contain
a list of input swath sonar data files.
This program uses the \fBMBIO\fP library and will read any swath sonar
format supported by \fBMBIO\fP. A list of the swath sonar data formats
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
Sets the input filename. If \fIformat\fP > 0 (set with the 
\fB-F\fP option) then the swath sonar data contained in \fIfile\fP 
is read and processed. If \fIformat\fP < 0, then \fIfile\fP
is assumed to be an ascii file containing a list of the input swath sonar
data files to be processed and their formats.  The program will read 
the data in each one of these files.
In the \fIfile\fP file, each
data file should be followed by a data format identifier, e.g.:
 	datafile1 11
 	datafile2 24
.br
This program uses the \fBMBIO\fP library and will read or write any swath sonar
format supported by \fBMBIO\fP. A list of the swath sonar data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page. Default: \fIfile\fP = "stdin".
.TP
.B \-O
.br
By default, \fBmbsvplist\fP writes the SVP records to
stdout. This option causes the program to instead write
the SVPs to individual files with names FILE_XXX.svp, 
where FILE is the source swath data filename and XXX is the
SVP count within the file.
.TP
.B \-V
Normally, \fBmbsvplist\fP works "silently" without outputting
anything to the stderr stream.  If the
\fB-V\fP flag is given, then \fBmblist\fP works in a "verbose" mode and
outputs the program version being used and all error status messages.

.SH EXAMPLES
Suppose one wishes to obtain an SVP list from a Simrad EM300 data file
in the MBARI format (MBIO id 57) called mbari_1998_107_msn.mb57. To 
obtain a listing to stdout, the following will suffice:

 	mbsvplist -F57 -I mbari_1998_526_msn.mb57 -V

The output will be as follows:

 Program mbsvplist
 Version $Id: mbsvplist.l,v 5.0 2001-01-04 21:43:50 caress Exp $
 MB-system Version 5.0

 Searching mbari_1998_526_msn.mb57 for SVP records
 Outputting SVP to file: 
 ## Water Sound Velocity Profile (SVP)
 ## Output by Program mbsvplist
 ## Program Version $Id: mbsvplist.l,v 5.0 2001-01-04 21:43:50 caress Exp $
 ## MB-System Version 5.0.alpha01
 ## Run by user <caress> on cpu <menard> at <Thu Jan  4 13:36:44 2001>
 ## Swath File: mbari_1998_526_msn.mb57
 ## Start Time: 1998/05/02 01:33:08.300000
 ## SVP Count: 1
 ## Number of SVP Points: 15
     0.00        1503.50
     9.99        1502.70
    19.98        1501.80
    30.00        1501.40
    39.99        1501.40
    49.98        1499.90
    60.00        1499.80
    69.99        1497.50
    79.98        1496.90
    90.00        1496.90
    99.99        1495.10
   198.99        1486.60
   498.99        1480.60
  1000.98        1481.90
  1830.00        1488.70
 ## 
 ## 
 4 SVP records read
 1 SVP records written

Total 4 SVP records read
Total 1 SVP records written

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbprocess\fP(l)

.SH BUGS
Probably.
