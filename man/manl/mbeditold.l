.TH mbedit l "5 October 1998" "MB-System 4.6" "MB-System 4.6"
.SH NAME
\fBmbedit\fP - Interactive editor used to flag bad bathymetry values in swath sonar bathymetry data.

.SH VERSION
Version 4.6

.SH SYNOPSIS
\fBmbedit\fP [\fB-B\fIyr/mo/da/hr/mn/sc\fP 
\fB-D\fP \fB-E\fIyr/mo/da/hr/mn/sc\fP 
\fB-F\fIformat\fP \fB -G -I\fIinfile\fP \fB-O\fIoutfile\fP \fB-S -V -H\fP]

.SH DESCRIPTION
\fBMBedit\fP is an interactive editor used to identify and flag 
artifacts in swath sonar bathymetry data. Once a file has been
read in, \fBMBedit\fP displays the bathymetry profiles
from several pings, allowing the user to identify and flag anomalous
beams. Flagging is handled internally by setting depth values
negative, so that no information is lost. In extreme circumstances,
pings may be zeroed (see "KEYBOARD ACTIONS"). 
The edited bathymetry is usually output to a file, but the program
can be operated in a "browse" mode where no data are output.

The display consists of several bathymetry profiles plotted within
a box with annotation showing the scaling of the x 
(acrosstrack distance) and y (depth) axes. The number of pings displayed,
and the width, vertical exageration, and annotation of the plot 
are all set by the user. Unflagged bathymetry points
are shown as small black rectangles, and flagged bathymtetry points
are shown as small red rectangles. The unflagged or good bathymetry
points are connected by black line segments to show the acrosstrack
bathymetry profiles for each ping. If the \fBShow Flagged Profile\fP
toggle is set on, then red line segments connect the flagged depth
values in the acrosstrack bathymetry profile.
Each of the displayed pings has a label giving the record number
in the data file, the ping time, and the center beam depth. 

The editing is driven by the left mouse button and 
involves three basic modes. In pick
mode each mouse pick toggles the nearest bathymetry point between
flagged and unflagged states. In erase mode the left mouse button
is held down as the cursor is dragged over the data; all bathymetry
points touched by the mouse are flagged. Restore mode is just like
erase mode, except that the affected bathymetry values are unflagged.
A few keyboard macros described below add additional flexibility
to the editing process.

If the ping includes flagged depths outside the plotting box, the ping label
is underlain by a green box. If
the ping includes unflagged depths outside the plotting box, the label
is underlain by a red box, and a small black box appears to the left
of the label. These colored labels help users identify
bad bathymetry points which lie outside the box defined by the current
vertical exageration and plot width values. In the case where unflagged
depths lie outside the plotting box (red label), clicking on the small
black box to the left of the ping label automatically flags all of the
depth values outside the plotting box. Alternatively, 
users can decrease the
vertical exageration and/or increase the plot width to bring the
offending bathymetry points into view. Users should be aware that
extreme bathymetric slopes or the use of high vertical exagerations
may cause good depth values to lie outside the plotting box for
the first and last pings in view.

The middle and right mouse buttons allow the user to step forward
and backward, respectively, through the data file. The \fBForward\fP
and \fBReverse\fP buttons 
provide a duplicate stepping capability. 

\fBMBedit\fP can hold up to 5000 data records at a time in memory.
Users on memory limited machines may find it necessary to set the
maximum buffer size to a smaller number using the
\fBData Buffer Size\fP slider. Many data formats include data
records in addition to those containing bathymetry data (e.g.
comment, sonar parameter, or navigation records). Thus, the number
of bathymetry records to be edited at any time may be less than
the total number of records loaded. Also, the record id numbers
shown on the ping labels may not be sequential. In the case that
a data file contains more data records than can be held in memory,
users will find it necessary to step through multiple buffers of
data. The \fBNext Buffer\fP button will cause \fBMBedit\fP to
dump the current buffer contents and read in a new set of data.
The \fBBuffer Retain Size\fP slider sets the number of data records
retained from the old buffer when more data are loaded. Clicking on
the \fBDone\fP button rather than the \fBNext Buffer\fP button will
cause \fBMBedit\fP to dump the rest of the input data file directly
to the output file without making the bathymetry available for editing.
The output data file always contains all of the data records in the
input file, regardless of whether all the data was actually edited.

The user can bring up
a "Go To" dialog by clicking on the \fBGo To\fP button. This dialog
allows the user to specify the time of a particular ping to be viewed.
The first ping with a time tag later than or equal to the specified time
is then displayed, providing such a ping is available. If an appropriate
ping is not available in the current buffer, \fBMBedit\fP will dump
and load buffers of data until such a ping is found or the end of the
file is reached. Thus, specifying an incorrect "Go To" time may cause
\fBMBedit\fP to close the file. Caution is advised in using this feature.

\fBMBedit\fP creates "edit save" files containing a list of each
edit command executed during an editing session. These files are
given names consisting of the input filename appended with ".mbesf".
In the event that a computer or program crash interrupts an edit,
the edit save files contain all information required to recover the
potentially lost work. If a user seeks to open a swath sonar data 
file and an associated edit save file already exists, the user is
given an option to apply the saved edits to the data as it is loaded.
If the saved edits are to be applied, \fBMBedit\fP copies the edit
save file to a file named "mbedit_tmp.mbesf" and reads the saved
edits from that second file.
If the user chooses not to apply the saved edits, those edits will
be lost as a new edit save file is created.

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
.B \-B
\fIyr/mo/da/hr/mn/sc\fP
.br
Sets the starting time for data allowed in the input data; pings
with times before the starting time will be ignored. 
Default: \fIyr/mo/da/hr/mn/sc\fP = 1962/2/21/10/30/0.
.TP
.B \-D
Starts up the program in "browse" mode. If a file is opened
in browse mode (either at startup or later), none
of the edited data will be output to a file. The default
is to output the edited data to a file.
.TP
.B \-E
\fIyr/mo/da/hr/mn/sc\fP
.br
Sets the ending time for data allowed in the input data; pings
with times after the ending time will be ignored. 
Default: \fIyr/mo/da/hr/mn/sc\fP = 2062/2/21/10/30/0.
.TP
.B \-F
\fIformat\fP
.br
Sets the format at startup for the input and output swath sonar data using 
\fBMBIO\fP integer format identifiers. This value can also be set
interactively when specifying the input file.
This program uses the \fBMBIO\fP library and will read or write any swath sonar
format supported by \fBMBIO\fP. A list of the swath sonar data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page.
Default: \fIformat\fP = 11.
.TP
.B \-G
This flag causes the program to treat the \fBDone\fP button as
equivalent to the \fBQuit\fP button. This option is used when
\fBMBedit\fP is started automatically by some other process and
only a single file is to edited.
.TP
.B \-H
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \-I
\fIinfile\fP
.br
Sets the data file from which the input data will be read at startup.
This option is usually used only when \fBMBedit\fP is started
automatically from some other process. The \fB-F\fP option should
also be used to set the data format id. If the \fB-B\fP option is
not used to specify browse mode and the \fB-O\fP option is
not used to set the output filename, then the output filename is
automatically set as follows:  If the input file is named
using the \fBMB-System\fP convention of an ".mbXX" suffix (the XX
corresponds to the \fBMBIO\fP format id), then the output file name
will have an "e.mbXX" suffix. Otherwise, the output file will
be \fIinfile\fP with ".ed" appended.
.TP
.B \-O
\fIoutfile\fP
.br
Sets the output data file to be used at startup. 
This option is usually used only when \fBMBedit\fP is started
automatically from some other process and the \fB-I\fP option is
used to specify a startup input data file.
.TP
.B \-S
This flag modifies how the program handles an input data file
read at startup, as specified with the \fB-I\fP option. If the
\fB-S\fP flag is given and an edit save file exists for the
startup input data file, the edit save file will be used. 
.TP
.B \-V
Normally, \fBMBedit\fP outputs information to the stderr stream
regarding the number of records loaded and dumped.  If the
\fB-V\fP flag is given, then \fBMBedit\fP works in a "verbose" mode and
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
automatically use the appropriate format id and the default
output filename will have an "e.mbXX" suffix; otherwise the
format must be set by the user and the default output filename
will be \fIinfile\fP with ".ed" appended.
The popup window also allows the output mode to be set to "browse"
so that no data are output. 
When a valid file is specified and the \fBOK\fP button
is clicked, as much data as will fit
into the data buffer is read and several pings are displayed as 
stacked bathymetry profiles.
.TP
.B Quit
This button causes the program to exit gracefully.  If a data file
has been read, all of the data will be written to the output file
before exiting.
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
.B Mode
This toggle allows the user to specify the edit mode.  If mode
is set to \fIPick\fP, then clicking the left mouse button will cause
the nearest beam to toggle between flagged and unflagged.  If mode
is set to \fIErase\fP, then the cursor will change to an erasor and
any beam with the cursor while the left mouse button is held down
will be flagged.  If mode is set to \fIRestore\fP, the behavior will
be the same as for \fIErase\fP except that the affected beams will be
unflagged instead of flagged.  The edit mode can also be set using
key macros (see the keyboard action section):  
 	Pick:		'Q', 'q', 'U', 'u' 
 	Erase:		'W', 'w', 'I', 'i' 
 	Restore:		'E', 'e', 'O', 'o' 
.TP
.B X Scale
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
.B Number of pings shown
This slider sets the number of pings shown at a time.
.TP
.B Number of pings to step
This slider sets the number of pings to step when the \fBForward\fP
or \fBReverse\fP buttons are pushed.
.TP
.B Go To
This button brings up a popup window which allows the user to
specify the time of a particular ping to be displayed.  Once the
year, month, day, hour, minute, and second values are entered, clicking
the \fBApply\fP button causes \fBmbedit\fP to seek the specified
target time.  If the current data buffer begins after the target
time, an error is returned.  If the target time is later than the
end of the current data buffer, then \fBmbedit\fP will dump and
load buffers until the target time is reached or the data file ends.
If the end of the file is reached during the search, the file will
be closed.
.TP
.B X Axis Tick Interval
This slider sets the tick interval in m for the cross track 
distance axis.
If a particular value is desired which cannot be
obtained by dragging the slider, the slider
can be changed by increments of 1 by clicking with the left button
inside the slider area, but not on the slider itself.
.TP
.B Y Axis Tick Interval
This slider sets the tick interval in m for the depth axis.
If a particular value is desired which cannot be
obtained by dragging the slider, the slider
can be changed by increments of 1 by clicking with the left button
inside the slider area, but not on the slider itself.
.TP
.B Data Buffer Size
This slider sets the number of data records which can be held 
in the data buffer.  Any change becomes effective the next time
that a data file is read in.
.TP
.B Buffer Retain Size
This slider sets the number of data records which are held over in
the buffer each time the old buffer is written out.
.TP
.B Show Flagged Profile
This toggle allows the user to specify whether the acrosstrack
bathymetry profile includes only the unflagged or "good" bathymetry
(toggle set to "On") or
whether the profile also includes the flagged or "bad" bathymetry
(toggle set to "Off"). In
the latter case, red line segments show the portion of the profile
associated with the flagged depth points.

.SH MOUSE ACTIONS
.TP
.B Left Mouse Button
The left mouse button is used to pick beams.  Good beams are
shown as filled squares and bad (flagged) beams as empty squares.  The 
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
.B Pick Mode:
Pressing one of these keys sets the edit mode to "pick" so that
clicking the left mouse button will cause
the nearest beam to toggle between flagged and unflagged.  
The edit mode can also be set using the \fBMode\fP button.
.TP
.B 'W', 'w', 'I', or 'i'
.B Erase Mode:
Pressing one of these keys sets the edit mode to "erase" so that
clicking the left mouse button will cause
any beam under the cursor while the left mouse button is held down
to be flagged.
The edit mode can also be set using the \fBMode\fP button.
.TP
.B 'E', 'e', 'O', or 'o'
.B Restore Mode:
Pressing one of these keys sets the edit mode to "restore" so that
clicking the left mouse button will cause any beam under the cursor 
while the left mouse button is held down to be unflagged.
The edit mode can also be set using the \fBMode\fP button.

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbclean\fP(l), 
\fBmbunclean\fP(l), \fBmbinfo\fP(l)

.SH BUGS
This program is only fun when the data are good.
