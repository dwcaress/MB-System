.TH mbm_route2mission l "12 July 2006" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbm_route2mission\fP - Macro to convert an \fBmbgrdviz\fP route file into an
MBARI AUV mission script. 

.SH VERSION
Version 5.0\fP

.SH SYNOPSIS
\fBmbm_route2mission\fP \fB-I\fP\fIroutefile\fP 
[\fB-A\fP\fIaltitudemin/altitudeabort[/altitudedepthcom]\fP 
\fB-B\fP\fIbehavior\fP \fB-C\fP\fImissiontime\fP \fB-D\fP\fIdepthmax/depthabort/depthdescent\fP 
\fB-F\fP\fIforwarddistance\fP \fB-G\fP\fIgpsmode\fP \fB-G\fP\fIapproachdepth\fP 
\fB-M\fP\fI[sonarlist]\fP \fB-N\fP \fB-O\fP\fImissionfile\fP 
\fB-P\fP\fI[startlon/startlat | startdistance]\fP 
\fB-T\fP\fIspeed\fP \fB-T\fP\fIstarttime\fP
\fB-W\fP\fIwaypointspacing\fP \fB-V\fP \fB-H\fP]

.SH DESCRIPTION
\fBmbm_route2mission\fP - Macro to convert an \fBmbgrdviz\fP route file into an
MBARI AUV mission script. 

MBARI Autonomous Underwater Vehicles, or AUVs, are controlled using a layered
behavior system that is specified using a mission script. This macro generates
an MBARI AUV mission script from an \fBMB-System\fP route file such as those 
generated interactively using \fBMBgrdviz\fP. The output MBARI AUV mission script
is named using a ".cfg" suffix. A Winfrog waypoint file (ending in ".pts") is also 
output which mirrors the waypoints in the input route file.

The output AUV mission script includes safety behaviors that operate throughout
a mission (e.g. missionTimer sets a mission timeout duration, and depthEnvelope
sets depth and altitude limits). The script also includes the sequential behaviors
required to take a vehicle from the start of a mission at the surface, obtaining
the initial position from GPS, descent to depth, flying to the starting waypoint,
running the desired survey lines with a desired depth or altitude profile,
ascent to the surface, and reacquiring positioning from GPS. For seafloor mapping
surveys, sonar settings and data logging are also controlled through the 
mission script.

The input route file specifies a survey mission as a set of sequential waypoints,
and also includes the depth profile between those waypoints. Waypoints may be
ordinary, the start of a survey line, or the end of a survey line (the start and
end line waypoints are generated using the survey route planning feature of 
\fBMBgrdviz\fP). The spacing  of the depth profile points between the waypoints 
derives from the bathymetric grid used to generate the route.

The output mission specifies a larger number of waypoints than the input route file. 
The mission waypoints are generated every \fIwaypointspacing\fP meters (specified 
with the \fB-W\fP option), and the AUV is commanded to fly at particular depths (or 
depth profiles) between each waypoint. By default, the AUV will be flown using a control 
behavior called WaypointDepth in which the commanded vehicle depth follows a linearly
interpolated profile between the depth at the starting waypoint and the depth
at the ending waypoint. The simpler alternative waypoint behavior specifies a
single vehicle depth while flying to the next mission; as it begins each segment
the vehicle will ascend or descend sharply to the desired depth and then fly
level until the next waypoint is reached.

The \fB-A\fP and \fB-D\fP options allows the user to set the basic vehicle depth and
altitude controls. The vehicle's vertical position will be simulatneously controlled
in terms of both altitude above the bottom and depth below the sea surface. The commanded
depth between each waypoint is calculated by \fBmbm_route2mission\fP (using the bathymetry
profiles in the input route file) to produce a vehicle altitude that is a\fIaltitudedepthcom\fP
meters above the shallowest depth looking \fIforwarddistance\fP meters ahead. The
forward looking feature allows the vehicle to be commanded to ascend prior to reaching
large scarps. As the mission commences, the AUV attempts to fly at the commanded depth 
unless that would result in an altitude less than \fIaltitudemin\fP meters. In that 
circumstance, the vehicle flies in an altitude-following mode. Thus, for consistent
bottom-following, a common approach is to set \fIaltitudemin\fP to the desired
altitude and to set \fIaltitudedepthcom\fP to a value slightly smaller than \fIaltitudemin\fP.
This strategy allows the look-ahead feature to bring the vehicle safely over steep rises
while otherwise maintaining a constant altitude.

The \fB-G\fP option allows the user to specify that the vehicle ascend to the surface
and reset positioning  with GPS at the beginning and/or end of survey lines.

The \fB-N\fP option allows the user to specify that the vehicle first move to the
initial survey waypoint, and then follow a spiral descent to the desired survey depth. 
The \fB-L\fP\fIapproachdepth\fP option sets the vehicle depth as it approaches the
first waypoint before the spiral descent. These options are used for deepwater missions
in which the vehicle navigation is updated by acoustic modem during the free
inertial descents.

The mission script must also allow sufficient time for the vehicle to transit from its
starting point to the first waypoint in the survey mission. The \fB-T\fP and \fB-P\fP
options allow the user specify the AUV starting position, the distance to the first
waypoint, or the time required to reach the first waypoint.

The MBARI Mapping AUV is equipped with a 200 kHz multibeam sonar, a 2-16 kHz chirp subbottom
profiler, a 110 kHz chirp sidescan, and a 410 kHz chirp sidescan. The \fB-M\fP option is
used to enable these mapping sonars in the AUV mission script.

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
\fIaltitudemin/altitudeabort[/altitudedepthcom]\fP
.br
Sets the AUV altitude control paramters. The missions generated by this 
script directly command the AUV's depth. However, the commanded depth 
will be overridden by the minimum altitude, or \fIaltitudemin\fP value; 
the vehicle will adjust it's depth so that it does not fly any closer 
to the bottom than \fIaltitudemin\fP meters. If the vehicle reaches an
altitude equal to or less than \fIaltitudeabort\fP meters, then the
mission will be aborted. The user may specify a desired altitude
(\fIaltitudedepthcom\fP) that is different than \fIaltitudemin\fP, so that
the commanded depths do not directly correspond to the minimum altitude.
.TP
.B \-B
\fIbehavior\fP
.br
Sets the AUV behavior used to fly the mission. If \fIbehavior\fP = 0, then
the AUV uses the WayPoint behavior in which a single vehicle depth is specified
for the approach to each waypoint. If \fIbehavior\fP = 1, then the AUV uses
the WayPointDepth behavior in which each mission segment has both a starting
and an ending depth specified, and the vehicle is commanded to fly a depth
profile that is a linear ramp between the two depths. Default: \fIbehavior\fP = 1;
.TP
.B \-C
\fImissiontime\fP
.br
Sets the maximum time allowed for the AUV mission. If the \fImissiontime\fP is
exceeded, the AUV will transition to it's internal abort plan. 
Default: mission abort time calculated from mission parameters.
.TP
.B \-D
\fIdepthmax/depthabort/depthdescent\fP
.br
Sets the maximum vehicle depth \fIdepthmax\fP in meters that the AUV will be 
commanded to fly. The \fIdepthabort\fP value sets the AUV depth at which a
mission will be aborted. The \fIdepthdescent\fP value is the depth to which
the vehicle descends before beginning to follow waypoints.
.TP
.B \-F
\fIforwarddistance\fP
.br
The program looks \fIforwarddistance\fP m ahead for the shallowest bottom
depth along the survey route, and then commands the vehicle to
fly at least \fIaltitudemin\fP above that depth.
.TP
.B \-G
\fIgpsmode\fP
.br
Sets the frequency with which the AUV will surface between survey lines to
get GPS fixes. By default, the vehicle only get GPS fixes at the surface at
the beginning and end of the mission, and not surface during the mission.
If \fIgpsmode\fP = 1, then the vehicle will ascend for gps fixes at
the start of each survey line. If \fIgpsmode\fP = 2, then the vehicle will 
ascend for gps fixes at the end of each survey line. If \fIgpsmode\fP = 3, 
then the vehicle will  ascend for gps fixes at the start and end of each survey line
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
.B \-L
\fIapproachdepth\fP
.br
Sets the vehicle depth during the approach to the first waypoint when a
spiral descent to depth at the first waypoint is specified  using the \fB-N\fP option.
.TP
.B \-M
\fI[sonarlist]\fP
This option enables control of the Mapping AUV's mapping sonars. By default,
the AUV mission is generated without turning on the mapping sonars. If \fB-M\fP
is given alone, the multibeam, subbottom profiler, and both low and high
frequency sidescan sonars will be enabled. The \fIsonarlist\fP parameter can
optionally specify which sonars are enabled in addition to the multibeam (the
multibeam serves as the ping timing master for all sonars, and thus must be
enabled if any mapping sonars are enabled). The \fIsonarlist\fP value is 
made up of one or more of the following characters:
 		S	subbottom profiler
 		L	low frequency sidescan
 		H	high frequency sidescan
.br
Thus, \fB-M\fP\fISL\fP will cause the multibeam, the subbottom profiler, and
the low frequency sidescan to be enabled while leaving the high frequency
sidescan off.
.TP
.B \-N
\fIapproachdepth\fP
.br
Sets the mission to include a shallow transit to the first waypoint followed by a
spiral descent to depth to start the survey.
.TP
.B \-O
\fImissionfile\fP
.br
Sets the filename of the output MBARI AUV mission script. 
.TP
.B \-P
\fIlon/lat\fP
.br
This sets the expected starting position (\fIlon\fP longitude 
and \fIlat\fP latitude) of the Mapping AUV so that the 
expected time to run to the first waypoint can be estimated. If neither 
the \fB-P\fP or \fB-T\fP options are specified, a distance of 500 meters 
to the first waypoint is assumed. 
.TP
.B \-S
\fIspeed\fP
.br
This sets the commanded AUV speed in meters/second. Default: \fIspeed\fP = 1.5 m/s. 
.TP
.B \-T
\fIstarttime\fP
.br
This sets the expected time required for the AUV to reach the first waypoint
in the survey mission. If neither 
the \fB-P\fP or \fB-T\fP options are specified, a distance of 500 meters 
to the first waypoint is assumed. 
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


