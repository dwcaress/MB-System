.TH mbm_vrefcheck l "31 October 1994"
.SH NAME
\fBmbm_vrefcheck\fP - Macro to generate plot of crosstrack seafloor slope
from a multibeam file.  The noise in this time series largely reflects
noise in the vertical reference used by the sonar.

.SH VERSION
Version 4.1

.SH SYNOPSIS
\fBmbm_vrefcheck\fP \fB-F\fIformat \fB-I\fIfile \fP[\fB-S\fIscale \fB-X\fIxaxis \fB-A\fIxmin \fB-B\fIxmax\fP]

.SH DESCRIPTION
\fBmbm_vrefcheck\fP is a macro which generates a plot of high pass
filtered apparent crosstrack seafloor slope.  The noise in this signal
appears to be dominated by noise in the vertical reference data 
used by the sonar system.  The crosstrack slopes are obtained by running
\fBmblist\fP. The resulting time series is filtered using the \fBGMT\fP
utility \fBfilter1d\fP, which uses a time domain median filter with
a time width of 5 minutes.  The filtered signal contains little of
the actual bathymetric signal, and thus represents a combination of
noise in the seafloor reflection travel times and the vertical 
reference data stream.  Close inspection of Hydrosweep data collected 
on the R/V Ewing with an interactive editor (\fBmbedit\fP) reveals
clear variability in the observed crosstrack slope, even when
surveying essentially flat seafloor.  This suggests that the filtered
signal is probably dominated by vertical reference noise on the Ewing.
The plot is automatically rasterized and sent to a Novajet plotter
(assuming one is on the R/V Ewing).

.SH AUTHORSHIP
David W. Caress (caress@lamont.ldeo.columbia.edu)
.br
Dale N. Chayes (dale@lamont.ldeo.columbia.edu)
.br
Suzanne H. O'Hara (sohara@lamont.ldeo.columbia.edu)
.br
Lamont-Doherty Earth Observatory
.br
Palisades, NY 10964

.SH OPTIONS
.TP
.B \fB-A\fIxmin\fP
Sets the start time of the plot in hours from the beginning of the
input file. If not specified, the beginning of the input file
will be used as \fIxmin\fP.
.TP
.B \fB-B\fIxmax\fP
Sets the end time of the plot in hours from the beginning of the
input file. If not specified, the end of the input file
will be used as \fIxmax\fP.
.TP
.B \fB-F\fIformat\fP
Sets the format for the input multibeam data using 
\fBMBIO\fP integer format identifiers. 
This macro uses the \fBMBIO\fP library and will read any multibeam
format supported by \fBMBIO\fP. A list of the multibeam data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page.
Default: \fIformat\fP = 24.
.TP
.B \fB-I\fIfile\fP
Data file from which the input data will be read.
.TP
.B \fB-S\fIxscale\fP
Sets the x-axis scale of the plot in inches/hour.  
Default: \fIxscale\fP = 1.2.
.TP
.B \fB-X\fIxaxis\fP
Sets the length of the x-axis in inches.  

.SH EXAMPLES
Suppose one has obtained a Hydrosweep DS multibeam data file 
called hs_ew9302_157_bcen.mb24 in the
L-DEO binary in-house format (format id 24). In order to generate a 
vertical reference noise plot:
.br
	mbm_vrefcheck -F24 -Ihs_ew9302_157_bcen.mb24

.SH BUGS
This script is hard wired for the R/V Ewing installation. This will
have to be fixed for it to be useful elsewhere.




