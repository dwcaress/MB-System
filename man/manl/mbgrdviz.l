.TH mbgrdviz l "9 February 2005" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbgrdviz\fP - Simple interactive 2D/3D visualization of GMT grids..

.SH VERSION
Version 5.0

.SH SYNOPSIS
\fBmbgrdviz\fP [-I\fIgridfile\fP \fB-T -V -H\fP]

.SH DESCRIPTION
\fBMBgrdviz\fP is a simple, interactive 2D/3D vizualization tool
for GMT grid files. Essentially, this program is a demonstration and
test platform for a vizualization API called \fBMBview\fI that will be used
in planned (version 6) \fBMB-System\fI processing tools.

This man page does not yet include either a description of the 
program's capabilities or documentation of the interface. The user's success
will therefore depend on some combination of the author's success in 
providing an intutitive interface and the user's cleverness and/or 
determination.

Good luck.

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
\fIgridfile\fP
.br
Sets a GMT grid file to be read and displayed at startup.
This option is usually used only when \fBMBgrdviz\fP is started
automatically from some other process. 
.TP
.B \-T
Sets a test grid to be calculated and displayed at startup.This
allows the program to be tested even when a grid file is
unavailable. 
.TP
.B \-V
Normally, \fBMBgrdviz\fP outputs limited information to the stderr stream.  
If the \fB-V\fP flag is given, then \fBMBgrdviz\fP works in a "verbose" mode and
outputs the program version being used, all error status messages, 
and a large amount of other information including all of the
beams flagged or zeroed.

.SH INTERACTIVE CONTROLS
.TP
.B File
This button brings up a popup window which allows the user to
specify the input swath sonar bathymetry data file, its \fBMBIO\fP
format id, the output mode, and the output filename. 
This program uses the \fBMBIO\fP library and 
will read or write any swath sonar
format supported by \fBMBIO\fP. A list of the swath sonar data
formats currently supported by \fBMBIO\fP and their
identifier values is given in the \fBMBIO\fP manual page. 
If the swath sonar data file is named using the \fBMB-System\fP
suffix convention (format 11 files end with ".mb11", format
41 files end with ".mb41", etc.), then the program will
automatically use the appropriate format id; otherwise the
format must be set by the user.
The popup window also allows the output mode to be set to "browse"
so that no edit events are output. 
When a valid file is specified and the \fBOK\fP button
is clicked, as much data as will fit
into the data buffer are read and several pings are displayed as 
stacked bathymetry profiles.
.TP
.B View
This button accesses a pulldown menu with several toggle buttons: 
\fBShow Flagged Profile\fP, 
\fBShow Bottom Detect Algorithms\fP, 
\fBWide Bathymetry Profiles\fP, 
\fBPlot Time Stamps\fP, 
\fBPlot Ping Interval\fP, 
\fBPlot Longitude\fP, 
\fBPlot Latitude\fP, 
\fBPlot Heading\fP, 
\fBPlot Speed\fP, 
\fBPlot Center Beam Depth\fP, 
\fBPlot Sonar Altitude\fP, 
\fBPlot Sonar Depth\fP, 
\fBPlot Roll\fP, 
\fBPlot Pitch\fP, and
\fBPlot Heave\fP.
The first two toggle buttons set simple display options
and may be set or unset individually.
The remaining toggle buttons control the display plot
modes; only one mode and therefore one toggle button may
be set at any time.
.TP
.B View->Show Flagged Profile
This toggle allows the user to specify whether the acrosstrack
bathymetry profile includes only the unflagged or "good" bathymetry
(toggle set to "Off") or
whether the profile also includes the flagged or "bad" bathymetry
(toggle set to "On"). In
the latter case, red line segments show the portion of the profile
associated with the flagged depth points.
.TP
.B View->Show Bottom Detect Algorithms
This toggle allows the user to specify whether the beams are
colored according to their flagging status (toggle set to "Off"
or according to the bottom detect algorithm used by the sonar 
(toggle set to "On"). The default is to color each sounding
according to its flagging status - unflagged beams are black,
manually flagged beams are red, and filter flagged beams are
green. If the beams are colored according to bottom detect
algorithm, then amplitude detects are shown in black, phase
detects are shown in red, and beams for which the bottom
detection algorithm is unknown are shown in green. Users should
be aware that many swath data formats do not include bottom
detection algorithm information.
.TP
.B View->Wide Bathymetry Profiles
This toggle, when set, causes \fBMBgrdviz\fP to display the widest
possible plots of the bathymetry profiles. All of the other options
discussed immediately below narrow the primary plot in order to
present additional information on the left side of the window.
.TP
.B View->Plot Time Stamps
This toggle, when set, causes \fBMBgrdviz\fP to display time stamp
information to the left of the bathymetry profiles. 
.TP
.B View->Plot Ping Interval
This toggle, when set, causes \fBMBgrdviz\fP to display an
automatically scaled vertical time series plot of the time between pings.
 This plot appears to the left of the bathymetry profiles.
.TP
.B View->Plot Longitude
This toggle, when set, causes \fBMBgrdviz\fP to display an
automatically scaled vertical time series plot of the 
navigation longitude associated with each ping.
 This plot appears to the left of the bathymetry profiles.
.TP
.B View->Plot Latitude
This toggle, when set, causes \fBMBgrdviz\fP to display an
automatically scaled vertical time series plot of the 
navigation latitude associated with each ping.
 This plot appears to the left of the bathymetry profiles.
.TP
.B View->Plot Heading
This toggle, when set, causes \fBMBgrdviz\fP to display an
automatically scaled vertical time series plot of the 
heading associated with each ping.
 This plot appears to the left of the bathymetry profiles.
.TP
.B View->Plot Speed
This toggle, when set, causes \fBMBgrdviz\fP to display an
automatically scaled vertical time series plot of the 
speed associated with each ping.
 This plot appears to the left of the bathymetry profiles.
.TP
.B View->Plot Center Beam Depth
This toggle, when set, causes \fBMBgrdviz\fP to display an
automatically scaled vertical time series plot of the 
speed associated with each ping.
 This plot appears to the left of the bathymetry profiles.
.TP
.B View->Plot Sonar Depth
This toggle, when set, causes \fBMBgrdviz\fP to display an
automatically scaled vertical time series plot of the 
sonar depth associated with each ping.
 This plot appears to the left of the bathymetry profiles.
.TP
.B View->Plot Sonar Altitude
This toggle, when set, causes \fBMBgrdviz\fP to display an
automatically scaled vertical time series plot of the 
sonar altitude associated with each ping.
 This plot appears to the left of the bathymetry profiles.
.TP
.B View->Plot Roll
This toggle, when set, causes \fBMBgrdviz\fP to display an
automatically scaled vertical time series plot of the 
roll associated with each ping.
 This plot appears to the left of the bathymetry profiles.
.TP
.B View->Plot Pitch
This toggle, when set, causes \fBMBgrdviz\fP to display an
automatically scaled vertical time series plot of the 
pitch associated with each ping.
 This plot appears to the left of the bathymetry profiles.
.TP
.B View->Plot Heave
This toggle, when set, causes \fBMBgrdviz\fP to display an
automatically scaled vertical time series plot of the 
heave associated with each ping.
 This plot appears to the left of the bathymetry profiles.
.TP
.B Controls
This button accesses a pulldown menu with six items: 
\fBGo To Specified Time...\fP, \fBBuffer Controls...\fP, 
\fBAnnotation...\fP, \fBFilters...\fP, 
\fBReverse Right/Left Key Macros\fP, and
\fBReverse Mouse Buttons\fP.
The first (top) four items bring up a dialog of
the same name. These dialogs are discussed below.
The last two items are toggle buttons which set mouse button
and key macro behaviors.
.TP
.B Controls->Go To Specified Time...
This menu item brings up a dialog which allows the user to
specify the time of a particular ping to be displayed.  Once the
year, month, day, hour, minute, and second values are entered, clicking
the \fBApply\fP button causes \fBMBgrdviz\fP to seek the specified
target time.  If the current data buffer begins after the target
time, an error is returned.  If the target time is later than the
end of the current data buffer, then \fBMBgrdviz\fP will dump and
load buffers until the target time is reached or the data file ends.
If the end of the file is reached during the search, the file will
be closed.
.TP
.B Controls->Buffer Controls...
This menu item brings up a dialog which allows the user
to set the data buffer handling
through two sliders discussed immediately below.
.TP
.B Controls->Buffer Controls->Data Buffer Size
This slider on the \fBBuffer Controls\fP dialog
sets the number of data records which can be held
in the data buffer.  Any change becomes effective the next time
that a data file is read in.
.TP
.B Controls->Buffer Controls->Buffer Retain Size
This slider on the \fBBuffer Controls\fP dialog
sets the number of data records which are held over in
the buffer each time the old buffer is written out.
.TP
.B Controls->Annotation...
This menu item brings up a dialog which
allows the user to set the annotation intervals for
the across track distance and depth axes through the two sliders
discussed immediately below.
.TP
.B Controls->Annotation->X Axis Tick Interval
This slider on the \fBAnnotation\fP dialog
sets the tick interval in m for the across track
distance axis.
If a particular value is desired which cannot be
obtained by dragging the slider, the slider
can be changed by increments of 1 by clicking with the left button
inside the slider area, but not on the slider itself.
.TP
.B Controls->Annotation->Y Axis Tick Interval
This slider on the \fBAnnotation\fP dialog
sets the tick interval in m for the depth axis.
If a particular value is desired which cannot be
obtained by dragging the slider, the slider
can be changed by increments of 1 by clicking with the left button
inside the slider area, but not on the slider itself.
.TP
.B Controls->Filters
This menu item brings up a dialog which
allows the user to turn automatic bathymetry filtering algorithms
on and off and to set the filter parameters 
through the widgets discussed immediately below.
Clicking on the \fBApply\fP button first removes any previous
automatic filter flags and then applies the current
filter settings to the swath bathymetry data. These actions
are applied only from the current position 
to the end of the data currently
in memory. The filtering is not applied (or unapplied) to
pings earlier in the data file than the current position.
The \fBReset\fP button restores the filter settings that
were applied last.
.TP
.B Controls->Filters->Median Spike Fllter
This toggle button turns automatic median filtering of
bathymetry profiles on and off.
When median filtering is enabled, the median depth is
calculated for each bathymetry ping. Each sounding in the
ping is compared with the median value; any sounding that
differs from the median by a percentage 
greater than a threshold percentage
is flagged as bad data.
.TP
.B Controls->Filters->% Median Depth Threshold
This slider sets the threshold of the median depth
filter in units of percentage of the median depth.
.TP
.B Controls->Filters->Wrong Side Fllter
This toggle button turns automatic wrong side filtering of
bathymetry profiles on and off. This filter expects
bathymetry beams to be numbered from port to 
starboard. Any beam numbered less than the 
center beam with a positive
(starboard) acrosstrack distance is considered to
be on the "wrong side", and any beam numbered higher than
the center beam with a negative (port) acrosstrack distance
is similarly regarded. For most swath data, the center
beam occurs near the midway point in the beam list. This
algorithm ignores apparent wrong side beams close to
the midway beam, where close is defined as being within
a threshold number of the midway beam.
.TP
.B Controls->Filters->Beams from Center Threshold
This slider sets the threshold of beam locations
checked by the wrong side filter. 
.TP
.B Controls->Filters->Flag by Beam Number
This toggle button turns automatic beam flagging by
beam number on and off. The flagging is controlled by
start and end beam number values (set by the two
sliders discussed immediately below). If the start beam
number is less than or equal to the end beam number, then
all beams between and including the start and end are
flagged, producing a flagged zone within the swath. If
the start beam number is greater than the end beam number,
then all beams before and including the end beam are flagged,
and all beams including and after the start beam are flagged.
This produces flagging of both swath edges.
.TP
.B Controls->Filters->Start Flagging Beam Number
This slider sets the beam number at which flagging by beam number starts. 
.TP
.B Controls->Filters->End Flagging Beam Number
This slider sets the beam number at which flagging by beam number ends. 
.TP
.B Controls->Filters->Flag by Distance
This toggle button turns automatic beam flagging by
acrosstrack distance on and off. The flagging is controlled by
start and end distance values (set by the two
sliders discussed immediately below). If the start distance
is less than or equal to the end distance, then
all beams with acrosstrack distances between 
the start and end distances are
flagged, producing a flagged zone within the swath. If
the start distance is greater than the end distance,
then all beams with acrosstrack distance less than 
the end distance are flagged,
and all beams with acrosstrack distance greater than 
the start distance are flagged.
This produces flagging of both swath edges.
.TP
.B Controls->Filters->Start Flagging Distance
This slider sets the beam number at which flagging 
by acrosstrack distance starts. 
.TP
.B Controls->Filters->End Flagging Distance
This slider sets the beam number at which flagging 
by acrosstrack distance ends. 
.TP
.B Controls->Reverse Right/Left Key Macros
This toggle button, when set, reverses the
key macros associated with flagging all beams
to the left or right of the last picked beam. This
means that the 'A', 'a', 'J', and 'j' keys will 
flag to the right rather than to the left. Similarly,
the  'D', 'd', 'L', and 'l' keys will flag to the
left rather than to the right. This option allows users
to conveniently handle swath bathymetry in which the
beams are mistakenly ordered starboard to port instead
of the usual port to starboard.
.TP
.B Controls->Reverse Mouse Buttons
This toggle button, when set, reverses the meaning of
the right and left mouse buttons. This option is for
the convenience of left-handed users.
.TP
.B Next Buffer
This button causes the program to write out the data from the
current buffer and then read in and display the next buffer.
If there is no more data to be read in after the current buffer
has been written out, then the input and output files are closed.
.TP
.B Done
This button causes the program to write out all of the data from the
input file and then close the input and output files.
.TP
.B Forward
This button causes the set of displayed pings to step \fInstep\fP 
pings forward in the current buffer.  The right mouse button causes
the same action.
.TP
.B Reverse
This button causes the set of displayed pings to step \fInstep\fP 
pings backward in the current buffer.  The middle mouse button causes
the same action.
.TP
.B Quit
This button causes the program to exit gracefully.  If a data file
has been read, all of the data will be written to the output file
before exiting.
.TP
.B About
This button causes the program to bring up a dialog showing the
program's name, version, and authors.
.TP
.B Acrosstrack Width
This slider sets the width of the plot in meters; in general this
value should be slightly larger than the swath width of the data
being edited. If a particular value is desired which cannot be
obtained by dragging the slider (e.g., the user wants a plot
width of 10 meters but the slider jumps from 1 to 47), the slider
can be changed by increments of 1 by clicking with the left button
inside the slider area, but not on the slider itself.
.TP
.B Vertical Exageration
This slider sets the depth scale in terms of vertical exageration.
The depth scale will change as the cross track distance scale is
changed to maintain the same vertical exageration.
If a particular value is desired which cannot be
obtained by dragging the slider, the slider
can be changed by increments of 0.01 by clicking with the left button
inside the slider area, but not on the slider itself.
.TP
.B Mode
This toggle allows the user to specify the edit mode.  If mode
is set to \fIToggle\fP, then clicking the left mouse button will cause
the nearest beam to toggle between flagged and unflagged.  If mode
is set to \fIPick\fP, then clicking the left mouse button will cause
the nearest unflagged beam to be flagged.  If mode
is set to \fIErase\fP, then the cursor will change to an erasor and
any beam with the cursor while the left mouse button is held down
will be flagged.  If mode is set to \fIRestore\fP, the behavior will
be the same as for \fIErase\fP except that the affected beams will be
unflagged instead of flagged. Info mode acts like pick mode, 
except that instead of flagging
the nearest sounding, the program displays information about the
selected ping and sounding above the bathymetry profile box. 
The edit mode can also be set using
key macros (see the keyboard action section):  
 	Toggle:		'Q', 'q', 'U', 'u' 
 	Pick:		'W', 'w', 'I', 'i' 
 	Erase:		'E', 'e', 'O', 'o' 
 	Restore:	'R', 'r', 'P', 'p' 
 	Info:		'T', 't', '{', '[' 
.TP
.B Unflag View
This button flags all unflagged beams among the currently
displayed pings. Pings in the buffer before or after the current 
display are unaffected.
.TP
.B Unflag View
This button unflags all flagged beams among the currently
displayed pings. Pings in the buffer before or after the current 
display are unaffected.
.TP
.B Unflag Forward
This button  unflags all flagged beams among all pings from
the start of the current display to the end of the current
data buffer. Pings before the start of the current display are
unaffected.
.TP
.B Number of pings shown
This slider sets the number of pings shown at a time.
.TP
.B Number of pings to step
This slider sets the number of pings to step when the \fBForward\fP
or \fBReverse\fP buttons are pushed.

.SH MOUSE ACTIONS
.TP
.B Left Mouse Button
The left mouse button is used to pick beams.  Good beams are
shown as filled black squares and bad (flagged) beams as unfilled red
or green squares.  The 
result of picking a particular beam depends on the current edit mode,
as set by the \fBMode\fP button or keyboard macros defined below. The
last picked beam (and ping) is remembered for use with 
some of the keyboard actions described below.
.TP
.B Middle Mouse Button
The middle mouse button causes the set of displayed pings to step \fInstep\fP 
pings backward in the current buffer.  The control button \fBReverse\fP causes
the same action.
.TP
.B Right Mouse Button
The right mouse button causes the set of displayed pings to step \fInstep\fP 
pings forward in the current buffer.  The control button \fBForward\fP causes
the same action.

.SH KEYBOARD ACTIONS
.TP
.B 'Z', 'z', 'M', or 'm'
.B Bad Ping:
Pressing one of these keys causes all of the beams in the last picked
ping to be flagged as bad.
.TP
.B 'X', 'x', '<', or ','
.B Right:
Pressing one of these keys causes all of the unflagged beams 
in the current display to to be flagged as bad. This is equivalent
to the CFlag View\fP button.
.TP
.B 'C', 'c', '>', or '.'
.B Right:
Pressing one of these keys causes all of the flagged beams 
in the current display to to be unflagged. This is equivalent
to the \fBUnflag View\fP button.
.TP
.B 'S', 's', 'K', or 'k'
.B Good Ping:
Pressing one of these keys causes all of the beams in the last picked
ping to be unflagged as good.
.TP
.B 'A', 'a', 'J', or 'j'
.B Left:
Pressing one of these keys causes all of the beams including and to 
the left of the last picked beam to be flagged as bad.
.TP
.B 'D', 'd', 'L', or 'l'
.B Right:
Pressing one of these keys causes all of the beams including and to 
the right of the last picked beam to be flagged as bad.
.TP
.B '!'
.B Zero Ping:
Pressing this key causes all of the beams in the ping 
associated with the last picked beam to be zeroed. This
should be used only for completely ridiculous values, as
the values are not recoverable.
.TP
.B 'Q', 'q', 'U', or 'u'
.B Toggle Mode:
Pressing one of these keys sets the edit mode to "toggle" so that
clicking the left mouse button will cause
the nearest beam to toggle between flagged and unflagged.  
The edit mode can also be set using the \fBMode\fP toggle buttons.
.TP
.B 'W', 'w', 'I', or 'i'
.B Pick Mode:
Pressing one of these keys sets the edit mode to "pick" so that
clicking the left mouse button will cause
the nearest unflagged beam to be flagged.  
The edit mode can also be set using the \fBMode\fP toggle buttons.
.TP
.B 'E', 'e', 'O', or 'o'
.B Erase Mode:
Pressing one of these keys sets the edit mode to "erase" so that
clicking the left mouse button will cause
any beam under the cursor while the left mouse button is held down
to be flagged.
The edit mode can also be set using the \fBMode\fP toggle buttons.
.TP
.B 'R', 'r', 'P', or 'p'
.B Restore Mode:
Pressing one of these keys sets the edit mode to "restore" so that
clicking the left mouse button will cause any beam under the cursor 
while the left mouse button is held down to be unflagged.
The edit mode can also be set using the \fBMode\fP toggle buttons.
.TP
.B 'T', 't', '{', or '['
.B Info Mode:
Pressing one of these keys sets the edit mode to "info" so that
clicking the left mouse button will cause information about the
nearest beam to be displayed above the bathymetry profile box.
The edit mode can also be set using the \fBMode\fP toggle buttons.

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbprocess\fP(l), \fBmbset\fP(l), 
\fBmbclean\fP(l), \fBmbunclean\fP(l), \fBmbinfo\fP(l)

.SH BUGS
This program is only fun when the data are good. The main
window for \fBMBgrdviz\fP is too large for use on screens
smaller than 1024 X 768 pixels.
