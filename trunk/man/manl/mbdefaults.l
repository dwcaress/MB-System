.TH mbdefaults l "20 August 1996" "MB-System 4.4" "MB-System 4.4"
.SH NAME
\fBmbdefaults\fP - Set or list default mbio parameters for reading and writing multibeam bathymetry data

.SH VERSION
Version 4.4

.SH SYNOPSIS
\fBmbdefaults\fP [\fB-B\fIyr/mo/da/hr/mn/sc \fB-D\fIpsdisplay \fB-E\fIyr/mo/da/hr/mn/sc 
\fB-F\fIformat \fB-L\fIlonflip \fB-P\fIpings \fB-R\fIwest/east/south/north 
\fB-S\fIspeed \fB-T\fItimegap \fB-W\fIproject \fB-V -H\fP]

.SH DESCRIPTION
\fBmbdefaults\fP is an utility for setting or listing the default read
parameters used by some \fBMBIO\fP utilities such as \fBMBCOPY\fP.  
If a user wishes to set default
parameters different from those set by the \fBMBIO\fP library, then
these specialized default values must be stored
in the file .mbio_defaults in the users home directory.  If any option
flag is given in invoking \fBmbdefaults\fP, then a new .mbio_defaults
file will be written incorporating the newly set parameters along with
any old parameters which have not been reset. If no option flag
is given, then \fBmbdefaults\fP will list the current default values
(set by \fBmbio\fP if no .mbio_defaults file exists).

.SH AUTHORSHIP
David W. Caress (caress@lamont.ldeo.columbia.edu)
.br
Dale N. Chayes (dale@lamont.ldeo.columbia.edu)
.br
Lamont-Doherty Earth Observatory
.br
Palisades, NY 10964

.SH OPTIONS
.TP
.B \fB-B\fIyr/mo/da/hr/mn/sc\fP
Sets the starting time for data allowed in the input data; pings
with times before the starting time will be ignored. 
Default: \fIyr/mo/da/hr/mn/sc\fP = 1962/2/21/10/30/0.
.B \fB-D\fIpsdisplay\fP
Sets the program used for displaying Postscript graphics files
on a screen; this facility is used by shellscripts created through
macros like \fBmbm_plot\fP. The default Postscript display
utility is \fBghostview\fP. Other common Postscript display utilities
are \fBpageview\fP (for Sun workstations) and \fBxpsview\fP 
(for Silicon Graphics workstations.
.TP
.B \fB-E\fIyr/mo/da/hr/mn/sc\fP
Sets the ending time for data allowed in the input data; pings
with times after the ending time will be ignored. 
Default: \fIyr/mo/da/hr/mn/sc\fP = 2062/2/21/10/30/0.
.TP
.B \fB-F\fIformat\fP
Sets the format for reading multibeam data using the 
\fBMBIO\fP integer format identifiers.  A list of the multibeam data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page.
Default = 11.
.TP
.B \fB-H\fP
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \fB-L\fIlonflip\fP
Sets the range of the longitude values returned.
If \fIlonflip\fP=-1 then the longitude values will be in
the range from -360 to 0 degrees. If \fIlonflip\fP=0 
then the longitude values will be in
the range from -180 to 180 degrees. If \fIlonflip\fP=1 
then the longitude values will be in
the range from 0 to 360 degrees.
Default: \fIlonflip\fP = 0.
.TP
.B \fB-P\fIpings\fP
Sets the ping averaging of the input data. If \fIpings\fP = 1, then
no ping averaging is performed. If \fIpings\fP > 0, then
that number of input pings will be averaged to produce one output
ping.  If \fIpings\fP = 0, then the ping averaging will automatically
be done so that the along-track ping spacing is equal to the across-track
beam spacing. If ping averaging is used (\fIpings\fP != 1), then
only a partial data stream can be copied (see the \fB-F\fP option).
Default: \fIpings\fP = 1 (no ping averaging).
.TP
.B \fB-R\fIwest/east/south/north\fP
Sets the longitude and latitude bounds within which multibeam bathymetry 
data will be read. If no data within the input satisfies the bounds 
then an initialization error will be returned. 
Default: west=-360, east=360, south=-90, north=90.
.TP
.B \fB-S\fIspeed\fP
Sets the minimum speed in km/hr (5.5 kts ~ 10 km/hr) allowed in 
the input data; pings associated with a smaller ship speed will not be
read. Default: \fIspeed\fP = 0.
.TP
.B \fB-T\fItimegap\fP
Sets the maximum time gap in minutes between adjacent pings allowed before
the data is considered to have a gap. Default: \fItimegap\fP = 1.
.TP
.B \fB-V\fP
Normally, \fBmbdefaults\fP outputs only a list of the default values 
to the stdout stream.  If the
\fB-V\fP flag is given, then \fBmbdefaults\fP works in a "verbose" mode and
also outputs the program version being used.
.TP
.B \fB-W\fIproject\fP
Sets the name of the default project. This will be used by
MB-System database management utilities yet to be created.
Default: \fIproject\fP = "none".

.SH EXAMPLES
Suppose that one wishes the default \fBMBFORMAT\fP data format to
be 8 rather than 1. The following will suffice to create (or rewrite)
a .mbio_defaults file in the users home directory including a 
default data format of 41:
	mbdefaults -F41

The output will be:

 New MBIO Default Control Parameters:
 format:   41
 pings:    1
 lonflip:  0
 speed:    0.000000
 timegap:  1.000000
 bounds: -360.000000 360.000000 -90.000000 90.000000
 begin time: 1962 2 21 10 30 0
 end time:   2062 2 21 10 30 0
 ps viewer:  ghostview
 project:    none

Suppose that one just wishes to see what the current default
parameters are.  The following will suffice:
	mbdefaults

The output will be

 Current MBIO Default Control Parameters:
 format:   5
 pings:    1
 lonflip:  0
 speed:    0.000000
 timegap:  1.000000
 bounds: -360.000000 360.000000 -90.000000 90.000000
 begin time: 1962 2 21 10 30 0
 end time:   2062 2 21 10 30 0
 ps viewer:  ghostview
 project:    none

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbio\fP(l), \fBmbcontour\fP(l), 
\fBmbswath\fP(l), \fBmbm_plot\fP(l), \fBmbm_grdplot\fP(l)

.SH BUGS
Too simple to have any really interesting bugs.
