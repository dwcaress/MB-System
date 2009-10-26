.TH mbm_copy l "27 August 2006" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbm_copy\fP - Apply mbcopy to all files referenced through
a datalist, using the MB-System file suffix convention
to name the output files

.SH VERSION
Version 5.0

.SH SYNOPSIS
\fBmbm_copy\fP \fB-F\fP\fIoutputformat\fP \fB-I\fP\fIdatalist\fP 
[\fB-H\fP \fB-C -T\fP \fB-V\fP]

.SH DESCRIPTION

\fBmbm_copy\fP is a macro used to copy large numbers of swath data files
from one data format to another using the program \fBmbcopy\fP. 
The input is a datalist file referencing
all of the target swath files. The output swath file format is specified
with the \fB-F\fP option. If the output swath file format is not specified,
the output format will be the same as the input format.

The macro identifies a filename root for each input swath file. For files
with recognized filename suffixes (e.g. ".mb61" for a format 61 file), the
filename root is the part of the filename that comes before the suffix. For
files without a recognized suffix, the filename root is the entire filename.
So, for a format 57 data file called "0007_20020425_060531.mb57", the filename root
is "0007_20020425_060531". For a format 121 file named "009_1659", the filename
root "009_1659".

The output filenames are automatically constructed by adding the standard 
MB-System file suffix to the filename root. This suffix is ".mbXXX" where
XXX is the two or three digit format id number. In the event that the output
filename is the same as the input filename (likely only if the \fB-F\fP option
is not used to set the output format), a "c" will be placed at the beginning
of the output file suffix, making it "c.mbXXX".

The \fB-T\fP option causes this macro to print out the \fBmbcopy\fP commands
it will execute without actually executing them. This test option allows 
users to check the consequences of their command before actually running it.

.SH AUTHORSHIP
David W. Caress (caress@mbari.org)
.br
  Monterey Bay Aquarium Research Institute
.br
Dale N. Chayes (dale@ldeo.columbia.edu)
.br
  Lamont-Doherty Earth Observatory

.SH SIMPLE DESCRIPTION OF BASIC OPTIONS
.TP
.B \-H
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \-C
.br
This option causes the program to check if each output file
already exists, and to forgo copying those that do exist.
.TP
.B \-F
\fIoutputformat\fP
.br
This option sets the output swath format. All of the swath data
files referenced in the datalist specified with the \fB-I\fP option
will be copied to format \fIoutputformat\fP using \fBmbcopy\fP. If
the \fB-F\fP option is not given, then the input files will be copied
to their input formats.
.TP
.B \-I
\fIdatalist\fP
.br
Sets the input datalist filename. A datalist is an ascii 
file containing a list of the input swath sonar
data files to be copied and their formats.  
In the \fIdatalist\fP file, each
swath data file should be followed by a data format identifier, e.g.:
 	datafile1 11
 	datafile2 24
.br
As discussed in the \fBMB-System\fP manual page, datalists may reference
other datalists, allowing users to construct recursive datalist structures.
This program uses the \fBMBIO\fP library and will read or write any swath sonar
format supported by \fBMBIO\fP. A list of the swath sonar data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page.
.TP
.B \-T
.br
This option causes \fBmbm_copy\fP to print out the \fBmbcopy\fP commands
it will execute without actually executing them. This test option allows 
users to check the consequences of their command before actually running it.
.TP
.B \-V
Causes \fBmbm_copy\fP to operate in "verbose" mode 
so that it outputs
more information than usual.

.SH EXAMPLES
Suppose one has three raw (format 56) Simrad EM300 data files:
 	0001_20020424_212920_raw.all
 	0002_20020425_011607_raw.all
 	0003_20020425_022926_raw.all
.br
One can generate a datalist called datalistr.mb-1 referencing these
three swath files using a text editor or by using \fBawk\fP:
 	ls -1 *all | awk '{print $1" 56"}' > datalistr.mb-1
.br
In order to translate all three files in the processing format for
Simrad multibeam data (format 57), use the \fBmbm_copy\fP macro:
 	mbm_copy -I datalistr.mb-1 -F 57 -T -V
.br
The macro prints out the following as it translates the swath data:
  	Running: mbcopy -F56/57 
 		-I0001_20020424_212920_raw.all 
 		-O0001_20020424_212920.mb57
  	Running: mbcopy -F56/57 
 		-I0002_20020425_011607_raw.all 
 		-O0002_20020425_011607.mb57
  	Running: mbcopy -F56/57 
 		-I0003_20020425_022926_raw.all
 		-O0003_20020425_022926.mb57

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbcopy\fP(l), 
\fBmbdatalist\fP(l)

.SH BUGS
Perhaps. 


