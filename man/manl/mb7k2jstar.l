.TH mb7k2jstar l "7 June 2005" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmb7k2jstar\fP - extracts subbottom profiler and/or sidescan sonar
data from Reson 7k format data files into Edgetech Jstar format data files.

.SH VERSION
Version 5.0

.SH SYNOPSIS
\fBmb7k2jstar\fP [\fB-I\fIfile \fB-A\fItype\fP \fB-C\fP 
\fB-F\fIformat\fP \fB-O\fIoutfile\fP \fB-H -V\fP]

.SH DESCRIPTION
\fBMB7k2jstar\fP is a utility that extracts Edgetech subbottom 
profiler and sidescan data from Reson 7k format (MBIO format 88) data and 
outputs in the Edgetech Jstar format (MBIO formats 132 and 133).
By default, \fBmb7k2jstar\fP extracts all channels of
sonar data from the Reson 7k file to the Edgetech Jstar file.
Edgetech sonar systems may include dual frequency sidescans and a
subbottom profiler. Users may use the \fB-A\fP option to specify
which channels are extracted. \fBMB7k2jstar\fP operates on single
input files or on datalists. By default there will be a separate 
output file for every input file, but if the \fB-O\fP option
is used to specify an output file, data from all input files will
be directed to that single output file.

When the user specifies that all Edgetech sidescan and subbottom
data be extracted but does not specify the output file name, the
output files will have the "*.jsf" suffix recognized by Edgetech
software. If the \fB-A\fP\fI1\fP option is used to specify that only
low frequency sidescan be extracted, the output files will have
an "*.mb132" suffix. If the \fB-A\fP\fI1\fP option is used to specify that only
high frequency sidescan be extracted, the output files will have
an "*.mb133" suffix.

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
\fImode\fP
.br
This option sets the types of data to be extracted and output.
If \fB-A\fP\fI1\fP is given, then any low frequency sidescan
in the input will be extracted to the output. If \fB-A\fP\fI2\fP
is given, then any high frequency sidescan records are extracted.
The \fB-A\fP\fI3\fP option causes subbottom profiler records to
be extracted. Default: All Edgetech sidescan and subbottom
profiler records are extracted.
.B \-C
.br
This option causes  \fBmb7k2jstar\fP to print out any comment records
encountered in the input Reson 7k data.
.TP
.B \-F
\fIformat\fP
.br
Sets the data format used if the input is read from stdin
or from a file. If \fIformat\fP < 0, then the input file specified
with the \fB-I\fP option will actually contain a list of input swath sonar
data files. This program only reads Reson 7k format data files (\fBMBIO\fP
format 88).
.TP
.B \-H
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \-I
\fIinfile\fP
.br
Sets the input file path. If \fIformat\fP > 0 (set with the 
\fB-f\fP option or \fBmbdefaults\fP) then the swath sonar data contained in \fIinfile\fP 
is read and processed. If \fIformat\fP < 0, then \fIinfile\fP
is assumed to be an ascii file containing a list of the input swath sonar
data files to be processed and their formats.  The program will read 
the data in each one of these files.
In the \fIinfile\fP file, each
data file should be followed by a data format identifier, e.g.:
 	datafile1 88
 	datafile2 88
.br
This program only reads Reson 7k format data files (\fBMBIO\fP
format 88).
.TP
.B \-O
.br
This option sets the output Edgetech Jstar (\fBMBIO\fP format 132/133) format file path. 
If \fB-O\fP is not invoked,
the output jstar filename (or files if a datalist is being processed) will
be the input swath filename with the ".s7k" or ".mb88" suffix replaced by a new suffix.
An ".jsf" suffix is used when all Edgetech record types are extracted
to the output, ".mb132" is used when the low frequency sidescan records are extracted,
and ".mb133" is used when the high frequency sidescan records are extracted. The record
types to be extracted are set using the \fB-A\fP option.
.TP
.B \-V
This option increases the verbosity of \fBmb7k2jstar\fP, causing it
to print out messages regarding its progress to stdout.

.SH EXAMPLES
Suppose that one has collected a Reson 7k datafile incorporating 
multibeam sonar data, sidescan data, and subbottom profiler data, and
that the filename is:
 	20050418_150155.s7k
.br

In order to extract all of the Edgetech sonar data into a Jstar file, one can use
\fBmb7k2jstar\fP with no special arguments:
 	mb7k2jstar -I 20040722_152111.s7k
.br
which yields the following output:

 	Data records read from: 20050418_150155.s7k
 	     Survey:        990
 	     File Header:   1
 	     Bluefin CTD:   989
 	     Bluefin Nav:   760
 	     Subbottom:     989
 	     Low Sidescan:  990
 	     High Sidescan: 989
 	Data records written to: 20050418_150155.jsf
 	     Subbottom:     989
 	     Low Sidescan:  990
 	     High Sidescan: 989

One may also extract just the low frequency sidescan by using the \fB-A\fP\fI1\fP option:
 	mb7k2jstar -I 20040722_152111.s7k -A1
.br
which yields the following output:

 	Data records read from: 20050418_150155.s7k
 	     Survey:        990
 	     File Header:   1
 	     Bluefin CTD:   989
 	     Bluefin Nav:   760
 	     Subbottom:     989
 	     Low Sidescan:  990
 	     High Sidescan: 989
 	Data records written to: 20050418_150155.mb132
 	     Subbottom:     0
 	     Low Sidescan:  990
 	     High Sidescan: 0

One may also extract just the high frequency sidescan by using the \fB-A\fP\fI2\fP option:
 	mb7k2jstar -I 20040722_152111.s7k -A2
.br
which yields the following output:

 	Data records read from: 20050418_150155.s7k
 	     Survey:        990
 	     File Header:   1
 	     Bluefin CTD:   989
 	     Bluefin Nav:   760
 	     Subbottom:     989
 	     Low Sidescan:  990
 	     High Sidescan: 989
 	Data records written to: 20050418_150155.mb132
 	     Subbottom:     0
 	     Low Sidescan:  990
 	     High Sidescan: 0

 	2527 records output to segy file 20040722_154429.s7k.segy
.br

Users may process multiple Reson 7k files by inputting a datalist, or list of
swath data files. All of the extracted data records can be output to a single
file by specifying the output file with the \fB-O\fP option. For example, if 
one has a datalist file called datalistp.mb-1 that references three Reson 7k files:
 	20050418_150155p.mb88 88
 	20050418_151812p.mb88 88
.br
and one wants all the low frequency sidescan data collated into a single
Jstar data file called 20050418_sslow.mb132, then use the following command:
 	mb7k2jstar -A1 -I datalistp.mb-1 -O 20050418_sslow.mb132
.br
which yields the following output:
 	Data records to extract:
 	     Low Sidescan
 	
 	Data records read from: 20050418_150155p.mb88
 	     Survey:        990
 	     File Header:   2
 	     Bluefin CTD:   989
 	     Bluefin Nav:   760
 	     Subbottom:     989
 	     Low Sidescan:  990
 	     High Sidescan: 989
 	Data records written to: 20050418_sslow.mb132
 	     Subbottom:     0
 	     Low Sidescan:  990
 	     High Sidescan: 0
 	
 	Data records read from: 20050418_151812p.mb88
 	     Survey:        741
 	     File Header:   2
 	     Bluefin CTD:   741
 	     Bluefin Nav:   563
 	     Subbottom:     741
 	     Low Sidescan:  742
 	     High Sidescan: 741
 	Data records written to: 20050418_sslow.mb132
 	     Subbottom:     0
 	     Low Sidescan:  742
 	     High Sidescan: 0
 	
 	Total data records read from: 20050418_151812p.mb88
 	     Survey:        1731
 	     File Header:   4
 	     Bluefin CTD:   1730
 	     Bluefin Nav:   1323
 	     Subbottom:     1730
 	     Low Sidescan:  1732
 	     High Sidescan: 1730
 	Total data records written to: 20050418_sslow.mb132
 	     Subbottom:     0
 	     Low Sidescan:  1732
 	     High Sidescan: 0

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbformat\fP(l), \fBmbinfo\fP(l)

.SH BUGS
No doubt.
