.TH MBPS l "17 September 1997" "MB-System 4.5" "MB-System 4.5"
.SH NAME
\fBmbps\fP - Generates a PostScript perspective plot of a piece of swath sonar data.

.SH VERSION
Version 4.5

.SH SYNOPSIS
\fBmbps\fP [\fB-B\fIyr/mo/da/hr/mn/sc \fB-E\fIyr/mo/da/hr/mn/sc 
\fB-F\fIformat \fB-I\fIfilename \fB-D\fIview_direction \fB-A\fIalpha  
\fB-K\fIeta \fB-X\fIve \fB-T\fI"title" \fB-G\fIgap \fB-S\fIspeedmin
\fB-W\fImeters_per_inch \fB-Y\fIdisplay_stats \fB-Z\fIdisplay_scales 
\fB-V -H\fP]

.SH DESCRIPTION
\fBmbps\fP is a utility to generate an (almost correct) perspective view
of a piece of swath sonar data. It is especially useful to get a detailed
view of the quality of the data (which is not very well assessed in a contour
plot) and to make pretty pictures of features that fit within a swath.
The output is PostScript code, that can be piped to a PostScript printer, or to a file.
The options are a bit hard to interpret without a picture. Consider a
frame of reference tied to the ship, with z upward, x across the track
(port being negative, starboard positive) and y along the track. Using
the \fB-D\fP option, the data can be viewed from port, starboard,
or from the back. In the first two cases, the y-axis is horizontal in
the plot; in the third case, the x-axis is horizontal.  The angle \fIalpha\fP
is the angle between the x-axis and the y-axis in the final view; \fIeta\fP is the angle of elevation of the observing point (90 degrees being the zenith).  The
data points are plotted as triangular polygons with the vertices at three good (ie. unflagged) adjacent data points. A given point is adjacent to 8 points in the surrounding swath (top, bottom, 2 sides, and 4 corners).  Now that you are thoroughly
confused, just try plotting a half-hour or so of data from your favorite
place. The defaults are such that a decent plot should come out.
The plot will be scaled automatically to fit on a page, and will contain vertical and horizontal scales, an arrow pointing in the direction of the ship's motion, coordinate axes, and miscellaneous
information identifying the piece of data being plotted.
Now change some of the options, and things should get clearer.

.SH AUTHORSHIP
Russ T. Alexander
.br
Formerly at University of California at Santa Barbara
.br
Santa Barbara, Ca 93106
.br
Alberto Malinverno (now at Schlumberger)
.br
Lamont-Doherty Earth Observatory
.br
Palisades, NY 10964

.SH OPTIONS
.TP
.B \-A
\fIalpha\fP
.br
Sets the angle alpha (degrees) between the y-axis (shiptrack direction) and
the x-axis (across-track). Default = \fIalpha = 70.
.TP
.B \-B
\fIyr/mo/da/hr/mn/sc\fP
.br
Sets the starting time for data allowed in the input data; pings
with times before the starting time will be ignored. 
Default: \fIyr/mo/da/hr/mn/sc\fP = 1962/2/21/10/30/0.
.TP
.B \-D
\fIview_direction\fP
.br
Sets the viewing direction; \fIviewdir\fP is a single character, either
P/p (view from the port side), S/s (starboard), or B/b (back). Default
\fIview_direction = S.
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
Sets the format for the input swath sonar data using 
\fBMBIO\fP integer format identifiers. 
This program uses the \fBMBIO\fP library and will read any swath sonar
format supported by \fBMBIO\fP. A list of the swath sonar data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page. Default: \fIformat\fP = 11.
.TP
.B \-G
\fIgap\fP
.br
Sets the manner in which data gaps are displayed.  If \fIgap \fP= 1 then 
gaps are filled with black.  If \fIgap \fP= 0 then gaps are not filled and appear the same as the background color of the paper. 
Default: \fIgap \fP= 1.
.TP
.B \-H
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \-I
\fIfilename\fP
.br
File from which the input data will be read. If
no input file is specified, the input will be read
from stdin. Default: \fIfilename\fP = stdin.
.TP
.B \-K
\fIeta\fP
.br
Sets the angle eta (degrees) that is the elevation of the observer over
the plane defined by the mean depth. Default \fIeta = 45.
.TP
.B \-S
\fIspeedmin\fP
.br
Sets the minimum speed in km/hr (5.5 kts ~ 10 km/hr) allowed in 
the input data; pings associated with a smaller ship speed will not be used. Default: \fIspeed\fP = 0.
.TP
.B \-T 
\fI"title"\fP
.br
Sets the title printed at the top of the plot.  The title should be enclosed within double quotes.  Default: no title.
.TP
.B \-V
If the \fB-V\fP flag is given, then \fBmbps\fP works in a "verbose" mode and
outputs the program version being used and all read error status messages.
.TP
.B \-W
\fImeters_per_inch\fP
.br
Sets the horizontal plotting scale.  Default: automatically calculates
a scale such that the plot fits on a page.
.TP
.B \-X
\fIve\fP
.br
Sets the vertical exaggeration. Default \fIve\fP = 5.
.TP
.B \-Y
\fIdisplay_stats\fP
.br
Sets whether the final plot includes info such as mean latitude, mean longitude, mean heading, \fIalpha\fP, \fIeta\fP, \fIve\fP, scale, track length, begin time, and end time. if \fIdisplay_stats \fP= 1 then the above info is plotted; if \fIdispaly_stats \fP= 0 then it is not plotted.  Default: \fIdisplay_stats  \fP= 1.
.TP
.B \-Z
\fIdisplay_scales\fP
.br
Sets whether the final plot includes a horizontal scale, a vertical scale, an arrow pointing in the ship heading direction, and coordinate axes.  if \fIdisplay_scales \fP= 1 then the above info is plotted; if \fIdispaly_scales \fP= 0 then it is not plotted.  Default: \fIdisplay_scales  \fP= 1.

.SH EXAMPLES
Suppose one wishes to take a look at data from a Hydrosweep file in
L-DGO edmb format (data.edmb), starting at 09:00 and ending at
10:30 on September 3, 1991. The following will output PostScript to a file called fred.

 	mbps -F22 -Idata.edmb -B1991/9/3/9/0/0 -E1991/9/3/10/30/0 > fred

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbcontour\fP(l), \fBmbswath\fP(l)

.SH BUGS
\fBmbps\fP assumes that the ship was moving on a straight line. Weird
plots will result if this is not the case.  \fBmbps \fPonly draws polygons bewteen 3 adjacent good points; if isolated points without 2 neighbors exist they will not be apparent in the final plot.  The filling of bad polygons is not perfect and gaps in the black fill, especially at the edges of swaths, may occur.
