.TH MBVELOCITYTOOL 1 "15 September 1994"
.SH NAME
\fBMBVELOCITYTOOL\fP - Interactive water velocity profile editor.

.SH VERSION
Version 4.00

.SH SYNOPSIS
\fBmbvelocitytool\fP [\fBA\fIdangle\fP \fB-V -H\fP]

.SH DESCRIPTION
\fBMBVELOCITYTOOL\fP is an interactive water velocity profile editor
used to examine multiple water velocity profiles and to create
new water velocity profiles which can be used for the processing
of multibeam sonar data.  In general, this tool is used to examine
water velocity profiles obtained from XBTs, CTDs, or databases,
and to construct new profiles consistent with these various
sources of information.

.SH AUTHORSHIP
David W. Caress (caress@lamont.ldeo.columbia.edu)
.br
Dale N. Chayes (dale@lamont.ldeo.columbia.edu)
.br
Lamont-Doherty Earth Observatory
.br
Palisades, NY 10964
.br
.br
Rod McCabe (mccabe@iris-2.asa.org)
.br
David Brock (brockda@iris-2.asa.org)
.br
Antarctic Support Associates
.br
61 Inverness Drive East
.br
Englewood, CO

.SH OPTIONS
.TP
.B \fB-A\fIdangle\fP
This option allows the user to specify the beam angle separation
in degrees that will be used in the raytracing calculations.  This
is useful when MB-System does not know the proper beam
angles associated with a particular dataset (e.g. the actual beam angle
separation may vary among installations of a sonar on different ships,
or MB-System may have incorrect information built in regarding certain
sonars).  The default is to use the beam angle pattern that MB-System
stores internally for each supported multibeam sonar system.
.TP
.B \fB-H\fP
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \fB-V\fP
Normally, \fBMBVELOCITYTOOL\fP works "silently" without outputting
anything to the stderr stream.  If the
\fB-V\fP flag is given, then \fBMBVELOCITYTOOL\fP works in a "verbose" mode and
outputs the program version being used, all error status messages, 
and a large amount of other information.

.SH INTERACTIVE CONTROLS
.TP
.B \fBFile\fP
This button brings up a menu containing the commands listed below:
.TP
.B \fBFile->Open Display Profile\fP
Selecting this menu item brings up a popup window which allows the 
user to select an input water velocity profile file. The profile
will be displayed in a depth vs velocity plot along with other
display profiles and the edit velocity profile, if one has been
opened.  Up to ten display velocity profiles may be loaded; the
color of the display profiles cycles through red, green, blue, and
black in the order that they are loaded.
.TP
.B \fBFile->Open Editable Profile\fP
Selecting this menu item brings up a popup window which allows the 
user to select an input water velocity profile file to be edited. The profile
will be displayed in a depth vs velocity plot along with any
display profiles that have been opened.  This profile will be plotted
as black lines with small black squares at the velocity depth points.
The velocity depth points may be selected and dragged using the mouse
to change the editable velocity profile.
.TP
.B \fBFile->New Editable Profile\fP
Selecting this menu item creates a default editable velocity profile
consisting of 14 velocity-depth points with a velocity of 1490 m/s. 
Any previously defined editable velocity profile will disappear. The profile
will be displayed in a depth vs velocity plot along with any
display profiles that have been opened.  This profile will be plotted
as black lines with small black squares at the velocity depth points.
The velocity depth points may be selected and dragged using the mouse
to change the editable velocity profile.
.TP
.B \fBFile->Save Editable Profile\fP
Selecting this menu item brings up a popup window which allows the 
user to save the current editable velocity profile as a file.
.TP
.B \fBFile->Open Multibeam data\fP
Selecting this menu item brings up a popup window which allows the 
user to select an input multibeam data file to be processed
in conjunction with the current editable velocity profile.
The multibeam data file must be in a format which stores
travel time data for each beam. The first 1000 data records
of the data file will be read into a buffer for later processing.
.TP
.B \fBProcess Multibeam data\fP
Selecting this button causes the multibeam travel time data to be
processed into bathymetry using the current editable velocity profile.
The processing involves full raytracing throught the water velocity model.
No calculations are performed or results plotted if an editable
velocity profile has not been loaded.
The bathymetry of each ping is fit with a line, and bathymetry residuals
are calculated for each good beam relative to the linear fit.  The
average of the bathymetry residuals is displayed in the lower part of
the window as a function of beam number. Anomalously shallow
bathymetry maps into negative residuals and deep bathymetry into positive
residuals; the residuals are displayed so that shallow is up and deep
is down. If the seafloor is reasonably smooth so that
a linear fit is appropriate, then the residuals will accurately reflect
any problems with the water velocity profile.  If the water velocity profile
is correct, then the residual plot will be roughly flat.  If the water
velocity profile is significantly in error, then the outer beam depths
may anomalously shallow (edge curl up) or deep (edge curl down).  In practice,
the editable velocity profile is altered interactively until a reasonably
residual pattern is achieved; reference velocity profiles obtained from
CTD casts, XBT's, or data bases (see manual page for program \fBMBLEVITUS\fP)
may be read in as display profiles to guide the editing process.
.TP
.B \fBQuit\fP
This button causes the program to exit gracefully, but without asking
any questions.  If you haven't already saved you edited velocity profile
it will be lost when you quit.
.TP
.B \fBMaximum Depth\fP
This slider sets the maximum depth in meters of the velocity profile 
display; the minimum depth is always zero.
.TP
.B \fBVelocity Range\fP
This slider sets the minimum and maximum velocity values of the velocity
profile display.  The minimum value is 1490 m/s minus the velocity range;
the maximum value is 1490 m/s plus the velocity range.
.TP
.B \fBResidual Range\fP
This slider sets the minimum and maximum bathymetry residual values 
of the bathymetry residual display.  The residual plot has a minimum
value of -range and a maximum value of +range. 

.SH BUGS
Users cannot remove display sound velocity profiles from the screen
once they are read in.
User defined default values such as the default data format id
are not used.
