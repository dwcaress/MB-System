.TH MBEDIT 1 "15 September 1994"
.SH NAME
\fBMBEDIT\fP - Interactive editor used to flag bad beams in multibeam bathymetry data.

.SH VERSION
Version 4.30

.SH SYNOPSIS
\fBmbedit\fP [\fB-F\fIformat \fB-I\fIinfile \fB-V -H\fP]

.SH DESCRIPTION
\fBMBEDIT\fP is an interactive editor used to identify and flag 
artifacts in multibeam bathymetry data. Once a file has been
read in, \fBMBEDIT\fP displays the bathymetry profiles
from several pings, allowing the user to identify and flag anomalous
beams.  The output filename is the input filename with ".ed" appended.


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
.B \fB-F\fIformat\fP
Sets the format at startup for the input and output multibeam data using 
\fBMBIO\fP integer format identifiers. This value can also be set
interactively when specifying the input file.
This program uses the \fBMBIO\fP library and will read or write any multibeam
format supported by \fBMBIO\fP. A list of the multibeam data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page.
Default: \fIformat\fP = 11.
.TP
.B \fB-H\fP
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \fB-I\fIinfile\fP
Sets the data file from which the input data will be read at startup.
This value can also be set interactively.  If the input file is named
using the \fBMB-System\fP convention of an ".mbXX" suffix (the XX
corresponds to the \fBMBIO\fP format id), then the output file name
will have an "e.mbXX" suffix. Otherwise, the output file will
be \fIinfile\fP with ".ed" appended.
.TP
.B \fB-V\fP
Normally, \fBMBEDIT\fP works "silently" without outputting
anything to the stderr stream.  If the
\fB-V\fP flag is given, then \fBMBEDIT\fP works in a "verbose" mode and
outputs the program version being used, all error status messages, 
and a large amount of other information.

.SH INTERACTIVE CONTROLS
.TP
.B \fBFile\fP
This button brings up a popup window which allows the user to
specify the input multibeam bathymetry data file and its \fBMBIO\fP
format id.  When a valid file is specified, as much data as will fit
into the data buffer is read and several pings are displayed as 
stacked bathymetry profiles.
.TP
.B \fBQuit\fP
This button causes the program to exit gracefully.  If a data file
has been read, all of the data will be written to the output file
before exiting.
.TP
.B \fBNext Buffer\fP
This button causes the program to write out the data from the
current buffer and then read in and display the next buffer.
If there is no more data to be read in after the current buffer
has been written out, then the input and output files are closed.
.TP
.B \fBDone\fP
This button causes the program to write out all of the data from the
input file and then close the input and output files.
.TP
.B \fBForward\fP
This button causes the set of displayed pings to step \fInstep\fP 
pings forward in the current buffer.  The right mouse button causes
the same action.
.TP
.B \fBReverse\fP
This button causes the set of displayed pings to step \fInstep\fP 
pings backward in the current buffer.  The middle mouse button causes
the same action.
.TP
.B \fBMode\fP
This choice item allows the user to specify the edit mode.  If mode
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
.B \fBX Scale\fP
This slider sets the cross track distance scale in m/100 pixels.
.TP
.B \fBVertical Exageration\fP
This slider sets the depth scale in terms of vertical exageration.
The depth scale will change as the cross track distance scale is
changed to maintain the same vertical exageration.
.TP
.B \fBNumber of pings shown\fP
This slider sets the number of pings shown at a time.
.TP
.B \fBNumber of pings to step\fP
This textitem sets the number of pings to step when the \fBForward\fP
or \fBReverse\fP buttons are pushed.
.TP
.B \fBGo To\fP
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
.B \fBX Axis Tick Interval\fP
This textitem sets the tick interval in m for the cross track distance axis.
.TP
.B \fBY Axis Tick Interval\fP
This textitem sets the tick interval in m for the depth axis.
.TP
.B \fBData Buffer Size\fP
This slider sets the number of data records which can be held 
in the data buffer.  Any change becomes effective the next time
that a data file is read in.
.TP
.B \fBBuffer Retain Size\fP
This slider sets the number of data records which are held over in
the buffer each time the old buffer is written out.

.SH MOUSE ACTIONS
.TP
.B \fBLeft Mouse Button\fP
The left mouse button is used to pick beams.  Good beams are
shown as filled squares and bad (flagged) beams as empty squares.  The 
result of picking a particular beam depends on the current edit mode,
as set by the \fBMode\fP button or keyboard macros defined below. The
last picked beam (and ping) is remembered for use with 
some of the keyboard actions described below.
.TP
.B \fBMiddle Mouse Button\fP
The middle mouse button causes the set of displayed pings to step \fInstep\fP 
pings backward in the current buffer.  The control button \fBReverse\fP causes
the same action.
.TP
.B \fBRight Mouse Button\fP
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

.SH BUGS
It might be better to define the cross track distance scale
in terms of the swath width that is desired rather than in m/100 pixels.
User defined default values such as the default data format id
are not used.
