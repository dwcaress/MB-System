.TH mbm_fmtvel l "4 January 2001" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbm_fmtvel\fP - Extract sound velocity info from Hydrosweep data
files and create output in tabular format.

.SH VERSION
Version 5.0

.SH SYNOPSIS
\fBmbm_fmtvel\fP [\fB-F\fIformat \fB-I\fIinfilename \fP]

.SH DESCRIPTION
\fBmbm_fmtvel\fP is a macro that scans a Hydrosweep swath sonar data file
using the program \fBhsdump\fP and generates a list in columnar
format with time, date, latitude, longitude, C-mean, and C-keel
entrys. Comment records (lines) at the top of the output begin with '#'.
The columns are tab-delimited. The only allowed formats are 21 (raw 
Hydrosweep data) and 24 (L-DEO in-house binary Hydrosweep data).

.SH AUTHORSHIP
David W. Caress (caress@mbari.org)
.br
  Monterey Bay Aquarium Research Institute
.br
Dale N. Chayes (dale@ldeo.columbia.edu)
.br
  Lamont-Doherty Earth Observatory
.br
Suzanne H. O'Hara (sohara@ldeo.columbia.edu)
.br
  Lamont-Doherty Earth Observatory

.SH OPTIONS
.TP
.B -F
\fIformat\fP
.br
Sets the format for the input swath sonar data using
\fBMBIO\fP integer format identifiers. The only allowed 
formats are 21 (raw 
Hydrosweep data) and 24 (L-DEO in-house binary Hydrosweep data).
.TP
.B -I
\fIinfilename\fP
.br
Data file from which the input data will be read. 

.SH SEE ALSO
\fBmbsystem\fP(l)

.SH BUGS
Only one output format is currently supported.


