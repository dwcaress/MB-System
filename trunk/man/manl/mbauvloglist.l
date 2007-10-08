.TH mbauvloglist l "24 November 2006" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbauvloglist\fP - Lists table data from an MBARI AUV mission log file.

.SH VERSION
Version 5.0

.SH SYNOPSIS
\fBmbauvloglist\fP \fB-I\fP\fIfile\fP [\fB-L\fP\fIlonflip\fP
\fB-O\fP\fIlist\fP \fB-V -H\fP ]

.SH DESCRIPTION
MBauvloglist reads an MBARI AUV mission log file and lists specified
fields in tab-delimited text output. MBARI AUV mission logs  are binary
files with an ascii header describing the units, name, and binary size 
of each value in a single record. An example of a log file header is:

# binary fastcatlog
.br
# timeTag time %8.8e ,time ,UNKNOWN 
.br
# double conductivity %8.8e ,Calculated conductivity ,Siemens/meter 
.br
# double temperature %8.8e ,Calculated temperature ,Celsius 
.br
# double pressure %8.8e ,Calculated pressure ,Decibars 
.br
# double calculated_salinity %8.8e ,Calculated salinity ,Volts 
.br
# double cond_frequency %8.8e ,Raw Conductivity frequency ,Hertz 
.br
# double temp_counts %8.8e ,Raw Pressure A/D counts ,Unitless 
.br
# double pressure_counts %8.8e ,pressure_counts ,Unitless 
.br
# double pressure_temp_comp_voltage_reading %8.8e ,Raw pressure temp compensation voltage reading ,Volts 
.br
# double calculated_sound_velocity %8.8e ,Calculated sound velocity ,M/s 
.br
# begin

The values found in the various MBARI AUV mission logs include:
 	"time"
 	"mPos_x"
 	"mPos_y"
 	"mDepth"
 	"mGpsNorth"
 	"mGpsEast"
 	"mGpsValid"
 	"mPhi"
 	"mTheta"
 	"mPsi"
 	"mOmega_x"
 	"mOmega_y"
 	"mOmega_z"
 	"mPsaRange"
 	"mAltitude"
 	"mDvlAltitude"
 	"mWaterSpeed"
 	"mDvlValid"
 	"mDvlNewData"
 	"mDeltaT"
 	"nfix"
 	"efix"
 	"filter_north"
 	"filter_east"
 	"filter_depth"
 	"north_current"
 	"east_current"
 	"speed_bias"
 	"heading_bias"
 	"latitude"
 	"longitude"
 	"conductivity"
 	"temperature"
 	"pressure"
 	"calculated_salinity"
 	"cond_frequency"
 	"temp_counts"
 	"pressure_counts"
 	"pressure_temp_comp_voltage_reading"
 	"calculated_sound_velocity"
 	"depth"
 	"temp"
 	"pressure"
 	"temp_period"
 	"pres_period"
.br
Users can extract arbitrary text lists of values in the AUV log files by using the \fB-O\fP
and \fB-F\fP options to specify the values and formats in the order desired.

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
.B \-F
\fIprintformat\fP
.br
Sets the printing format (in fprintf() style) of the next value specified using
the \fB-O\fP option.By default, \fBmbauvloglist\fP uses the print format specified
in the log file header.
format 
.TP
.B \-I
\fIifile\fP
.br
Sets the input MBARI AUV mission log  filename. 
.B \-L
\fIlonflip\fP
.br
Sets the range of the longitude values output.
If \fIlonflip\fP=-1 then the longitude values will be in
the range from -360 to 0 degrees. If \fIlonflip\fP=0 
then the longitude values will be in
the range from -180 to 180 degrees. If \fIlonflip\fP=1 
then the longitude values will be in
the range from 0 to 360 degrees.
Default: \fBmbauvloglist\fP uses the user default \fIlonflip\fP set using
\fBmbdefaults\fP.
.TP
.B \-O
\fIvaluename\fP
.br
Causes \fBmbauvloglist\fP to output the specified value. This option can be 
given multiple times, and the values will be output in the order specified.
.TP
.B \-V
The \fB-V\fP option causes the program to be verbose.
.SH EXAMPLE
To extract a list of pressure and water sound speed from the CTD log of an
AUV mission, use:
 	mbauvloglist -I mvc_logs/2006.224.00/fastcatlog.log \\ 
 		-F%f -O pressure -O calculated_sound_velocity 
.br
This simple list is ordered as the data were collected, following the ups
and downs of the AUV mission. In order to construct a model of the water
sound speed as a function of depth, one can sort the output of \fBmbauvloglist\fP
(using the program \fBsort\fP) to be ordered with increasing pressure (depth), 
and then resampled into uniform increments of pressure using the \fBGMT\fP
program \fBsample1d\fP.
 	mbauvloglist -I mvc_logs/2006.224.00/fastcatlog.log \\ 
 		-F%f -O pressure -O calculated_sound_velocity \\ 
 		| sort -n -u > auv_ssv.txt
 	sample1d auv_ssv.txt -Fl -H1 -S0.0 -I10.0 > auv_ssv_bin.txt

.SH SEE ALSO
\fBmbsystem\fP(l), \fBsample1d\fP, \fBsort\fP.

.SH BUGS
Indubitably.
