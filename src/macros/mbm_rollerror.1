.TH mbm_rollerror 1 "26 February 1994"
.SH NAME
\fBmbm_rollerror\fP - Macro to read a multibeam bathymetry file, calculate the noise in the vertical reference used by the sonar, and generate a file containing roll corrections which can be applied to the data.

.SH VERSION
Version 4.00

.SH SYNOPSIS
\fBmbm_rollerror\fP \fB-F\fIformat \fB-I\fIfile \fP[\fB-S\fIscale \fB-X\fIxaxis \fB-A\fIxmin \fB-B\fIxmax\fP]

.SH DESCRIPTION
\fBmbm_rollerror\fP is a macro which generates crosstrack slope
corrections from the apparent vertical reference noise of multibeam
data.  The crosstrack slopes are obtained by running
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
The corrections can be applied to Hydrosweep data during the calculation
of bathymetry from travel times using \fBhsbath\fP.

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

.SH EXAMPLES
Suppose one has obtained a Hydrosweep DS multibeam data file 
called hs_ew9302_157_bcen.mb24 in the
L-DEO binary in-house format (format id 24). In order to generate a 
slope correction file:
.br
	mbm_rollerror -F24 -Ihs_ew9302_157_bcen.mb24 
.br
The output will be in a file called hs_ew9302_157_bcen.mb24.cor

.SH BUGS
The roll corrections don't seem to do much good, so this utility
isn't very useful.  Why don't we call it "experimental".




