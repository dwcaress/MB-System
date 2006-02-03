.TH mbm_grdinfo l "3 February 2006" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbm_makedatalist\fP - Macro to generate a datalist of the swath
files in a specified directory.

.SH VERSION
Version 5.0

.SH SYNOPSIS
\fBmbm_makedatalist\fP [\fB-I\fP\fIdirectory\fP \fB-O\fP\fIdatalist\fP \fB-H -V\fP]

.SH DESCRIPTION
\fBMbm_makedatalist\fP is a macro to generate an MB-System 
datalist file referencing all identifiable swath files in 
the specified target directory. 

The output datalist will be placed in the current working 
directory. The output datalist is named datalist.mb-1 
by default, but can be specified with the \fB-O\fP option.
If no target directory is specified with the \fB-I\fP
option, then the current working directory is used. In order 
for swath files to be recognized and included in the datalist,
they must conform to the MB-System swath file naming convention
(see the \fBmbsystem\fP manual page for details).

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
.B \-H
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \-I
\fIdirectory\fP
.br
Sets the path of the target directory. 
.TP
.B \-O
\fIdatalist\fP
.br
Sets the name of the output datalist file. Default: datalist.mb-1 
.TP
.B \-V
The \fB-V\fP option causes \fBmbm_makedatalist\fP to print out status messages.

.SH EXAMPLES
Suppose we have a set of swath files in a directory and we 
want to construct a datalist referring to those files. A simple
directory listing shows four files:
.br
 	20051019_154030.mb88
 	20051019_162540.mb88
 	20051019_171756.mb88
 	junk.txt
.br
of which three are identifiable as swath files because of the
".mb88" file suffix. Running \fBmbm_makedatalist\fP in this
directory with no arguments other than verbosity:
.br
 	mbm_makedatalist -V
.br
produces a datalist file named datalist.mb-1. The contents of
this file are:
.br
 	./20051019_154030.mb88 88
 	./20051019_162540.mb88 88
 	./20051019_171756.mb88 88
.br
where the file junk.txt has been ignored.

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbdatalist\fP(l), \fBmbm_grdinfo\fP(l)

.SH BUGS
This macro doesn't yet recognize when one swath file derives from
another (e.g. line1p.mb57 can be inferred to be a processed 
version of line1.mb57).


