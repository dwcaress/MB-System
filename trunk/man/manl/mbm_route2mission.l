.TH mbm_arc2grd l "12 January 2006" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbm_route2mission\fP - Macro to convert an \fBmbgrdviz\fP route file into an
MBARI AUV mission script. 

.SH VERSION
Version 5.0\fP

.SH SYNOPSIS
\fBmbm_route2mission\fP \fB-I\fP\fIroutefile\fP [\fB-A\fP\fIaltitudemin\fP \fB-D\fP\fIdepthmax\fP 
\fB-F\fP\fIforwarddistance\fP \fB-O\fP\fImissionfile\fP \fB-W\fP\fIwaypointspacing\fP \fB-V\fP \fB-H\fP]

.SH DESCRIPTION
\fBmbm_route2mission\fP - Macro to convert an \fBmbgrdviz\fP route file into an
MBARI AUV mission script. 

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
.B \-A
\fIaltitudemin\fP
.br
Sets the minimum altitude that the AUV can be commanded to fly.
.TP
.B \-A
\fIdepthmax\fP
.br
Sets the maximum vehicle depth that the AUV can be commanded to fly.
.TP
.B \-F
\fIforwarddistance\fP
.br
The program looks \fIforwarddistance\fP m ahead for the shallowest bottom
depth along the survey route, and then commands the vehicle to
fly at least \fIaltitudemin\fP above that depth.
.TP
.B \-H
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \-I
\fIroutefile\fP
.br
Sets the filename of the input \fBmbgrdviz\fP route file. 
A route file specifies a desired survey route as a set of
lines between waypoints. In addition to the waypoints, the file
also includes the depth profiles along those lines.
.TP
.B \-O
\fImissionfile\fP
.br
Sets the filename of the output MBARI AUV mission script. 
.TP
.B \-W
\fIwaypointspacing\fP
.br
Sets the spacing in meters of the waypoints output to the mission script. 
.TP
.B \-V
The \fB-V\fP option causes \fBmbm_route2mission\fP to print out status messages.

.SH EXAMPLES
Soon...

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbgrdviz\fP(l)

.SH BUGS
Perhaps.


