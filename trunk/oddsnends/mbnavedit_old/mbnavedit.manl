.TH mbnavedit l "17 December 1998" "MB-System 4.6" "MB-System 4.6"
.SH NAME
\fBmbnavedit\fP - Interactive navigation editor for swath sonar data.

.SH VERSION
Version 4.6

.SH SYNOPSIS
\fBmbnavedit\fP [\fB-B\fIyr/mo/da/hr/mn/sc\fP 
\fB-D\fP \fB-E\fIyr/mo/da/hr/mn/sc\fP 
\fB-F\fIformat \fB-I\fIinfile \fB-V -H\fP]

.SH DESCRIPTION
\fBmbnavedit\fP is an interactive editor used to identify and fix 
problems with the navigation of swath sonar data. Once a file has been
read in, \fBmbnavedit\fP displays autoscaled plots of the longitude,
latitude, speed, and heading time series. The user may select
outliers and interpolate over them or, in the case of heading or
speed data, replace the erroneous values with estimates derived 
from "course-made-good" or "speed-made-good", respectively.
Data can only be selected and altered in a single 
plot at a time (this avoids
confusion and prevents mistakes).
The user can also inspect plots of roll, pitch, and heaved
data to determine if the ship's vertical reference sensors
were working properly.
The edited data is usually output to a file, but the program
can be operated in a "browse" mode where no data is output.

.SH AUTHORSHIP
David W. Caress (caress@mbari.org)
.br
  Monterey Bay Aquarium Research Institute
.br
Dale N. Chayes (dale@ldeo.columbia.edu)
.br
  Lamont-Doherty Earth Observatory

.SH OPTIONS
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
.B \-H
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \-I
\fIinfile\fP
.br
Sets the data file from which the input data will be read at startup.
This value can also be set interactively.  If the input file is named
using the \fBMB-System\fP convention of an ".mbXX" suffix (the XX
corresponds to the \fBMBIO\fP format id), then the output file name
will have an "e.mbXX" suffix. Otherwise, the output file will
be \fIinfile\fP with ".ed" appended.
.TP
.B \-I
\fIoutfile\fP
.br
Sets the output data file, overriding the file naming conventions
discussed above in the \fB-I\fP option.
.TP
.B \-V
Normally, \fBmbnavedit\fP outputs information to the stderr stream
regarding the number of records loaded and dumped.  If the
\fB-V\fP flag is given, then \fBmbnavedit\fP works in a "verbose" mode and
outputs the program version being used, all error status messages, 
and a large amount of other information including all of the
beams flagged or zeroed.

.SH INTERACTIVE CONTROLS
.TP
.B File
This menu brings up a popup window which allows the user to
specify the input swath sonar data file, its \fBMBIO\fP
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
so that no data is output. 
When a valid file is specified and the \fBOK\fP button
is clicked, as much data as will fit
into the data buffer is read and the navigition times series
plots are displayed.
.TP
.B Next Buffer
This button causes the program to write out the data from the
current buffer and then read in and display the next buffer.
If there is no more data to be read in after the current buffer
has been written out, then the input and output files are closed.
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
.B Done
This button causes the program to write out all of the data from the
input file and then close the input and output files.
.TP
.B Quit
This button causes the program to exit gracefully.  If a data file
has been read, all of the data will be written to the output file
before exiting.
.TP
.B Pick
Clicking on this toggle button sets the edit mode to "pick". In
this case, clicking the left mouse button will cause the
nearest data value to toggle between selected (red) and
unselected. 
.TP
.B Select
Clicking on this toggle button sets the edit mode to "select". In
this case, clicking and dragging the left mouse button will 
cause any data value touched by the cursor to become selected. 
.TP
.B Deselect
Clicking on this toggle button sets the edit mode to "deselect". In
this case, clicking and dragging the left mouse button will 
cause any data value touched by the cursor to become unselected. 
.TP
.B Select All
Clicking on this toggle button sets the edit mode to "select all". In
this case, clicking in any of the editable plots will cause all
of the data in that plot to be selected. 
.TP
.B Deselect All
Clicking on this toggle button sets the edit mode to "select all". In
this case, clicking in any of the editable plots will cause all
of the data in that plot to be unselected. 
.TP
.B Time Span Shown
This slider sets the number of seconds shown at a time.
.TP
.B Time Step
This slider sets the number of seconds to step when the \fBForward\fP
or \fBReverse\fP buttons are pushed.
.TP
.B Interpolate Selection
This button causes the selected data to be replaced by linear interpolation
of the surrounding unselected data. If the selection extends to the
edge of the data, the selected data will be replaced by the first
unselected datum on the other side.
.TP
.B Revert Selection
This button causes the selected data to revert to their
original values.
.TP
.B Show Entire Buffer
This button causes the plots to expand to show all of the data
in the current buffer.
.TP
.B Pick Time Interval
This button allows users to focus the plots on a particular
time interval. Once the button is pushed, the left mouse button
is used to select the left edge of the time selection. Similarly,
the middle mouse button selects the right edge of the time selection.
Both ends of the time selection can be adjusted multiple times.
Once the time interval of interest is selected to the users
satisfaction, pressing the right mouse button will cause the
plots to be redrawn with the selected beginning and ending
times.
.TP
.B Longitude Plot
This toggle button turns the longitude plot on and off.
.TP
.B Show Original Data (Longitude Plot)
When this toggle button is on, the longitude plot includes a
green line representing the original longitude values.
.TP
.B Latitude Plot
This toggle button turns the latitude plot on and off.
.TP
.B Show Original Data (Latitude Plot)
When this toggle button is on, the latitude plot includes a
green line representing the original latitude values.
.TP
.B Speed Plot
This toggle button turns the speed plot on and off.
.TP
.B Show Original Data (Speed Plot)
When this toggle button is on, the speed plot includes a
green line representing the original speed values.
.TP
.B Show Speed-Made-Good (Speed Plot)
When this toggle button is on, the speed plot includes a
blue line representing the speed-made-good values derived
from the longitude and latitude time series.
.TP
.B Use Speed-Made-Good (Speed Plot)
When this button is pushed, any selected data in the speed
plot will be replaced by the current estimates of speed-made-good.
.TP
.B Heading Plot
This toggle button turns the heading plot on and off.
.TP
.B Show Original Data (Heading Plot)
When this toggle button is on, the speed plot includes a
green line representing the original speed values.
.TP
.B Show Course-Made-Good (Heading Plot)
When this toggle button is on, the heading plot includes a
blue line representing the course-made-good values derived
from the longitude and latitude time series.
.TP
.B Use Course-Made-Good (Heading Plot)
When this button is pushed, any selected data in the heading
plot will be replaced by the current estimates of course-made-good.
.TP
.B Roll, Pitch, and Heave Plots
This toggle button turns the roll, pitch, and heave plots on and off.
Unlike the other plots, the data in these plots is not editable.
These data are shown purely to allow users to determine if the
vertical reference sensors were working properly at the time the
data was collected (note: many swath data formats do not contain roll,
pitch, and heave data).

.SH MOUSE ACTIONS
.TP
.B Left Mouse Button
The left mouse button is used to pick data values.  Unselected data
values are shown as filled black squares and selected values 
as empty red squares.  The manner in which data are selected or
unselected is controlled by the edit mode, as set by the 
\fBPick\fP, \fBSelect\fP, \fBDeselect\fP, \fBSelect All\fP, and
\fBDeselect All\fP buttons.
.TP
.B Middle Mouse Button
The middle mouse button causes the set of displayed data to 
step backward in the current buffer by the amount of time
set on the \fBTime Step\fP slider.  
The control button \fBReverse\fP causes
the same action.
.TP
.B Right Mouse Button
The right mouse button causes the set of displayed data to 
step forward in the current buffer by the amount of time
set on the \fBTime Step\fP slider.  
The control button \fBForward\fP causes
the same action.

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbmerge\fP(l), 
\fBmbedit\fP(l), \fBmbinfo\fP(l)

.SH BUGS
Please let us know.
