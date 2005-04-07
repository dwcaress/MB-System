eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
                    & eval 'exec perl -S $0 $argv:q'
                         if 0;
#--------------------------------------------------------------------
#    The MB-system: mbm_route2mission.perl   7/18/2004
#    $Id: mbm_route2mission.perl,v 5.3 2005-04-07 04:14:12 caress Exp $
#
#    Copyright (c) 2004 by 
#    D. W. Caress (caress@mbari.org)
#      Monterey Bay Aquarium Research Institute
#      Moss Landing, CA
#    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
#      Lamont-Doherty Earth Observatory
#      Palisades, NY  10964
#
#    See README file for copying and redistribution conditions.
#--------------------------------------------------------------------
#
# Command:
#   mbm_route2mission
#
# Purpose: 
#   Perl shellscript to translate survey route file derived from 
#   MBgrdviz into an MBARI AUV mission script. Developed for use
#   in survey planning for the MBARI Mapping AUV.
#
# Usage:
#   mbm_route2mission -Iroutefile [-Aaltitudemin -Ddepthmax -Fforwarddistance -Omissionfile -Wwaypointspacing -V -H]
#
# 
# Author:
#   David W. Caress
#      Monterey Bay Aquarium Research Institute
#      Moss Landing, CA
#
# Version:
# $Id: mbm_route2mission.perl,v 5.3 2005-04-07 04:14:12 caress Exp $
#
# Revisions:
#   $Log: not supported by cvs2svn $
#   Revision 5.2  2004/12/02 06:27:45  caress
#   Fixes to support MBARI AUV program.
#
#   Revision 5.1  2004/09/16 19:29:11  caress
#   Development to support for MBARI Mapping AUV.
#
#   Revision 5.0  2004/07/27 19:56:43  caress
#   Macro to translate MBgrdviz route file into MBARI AUV mission file.
#
#
$program_name = "mbm_route2mission";

# Definition of waypoint values:
#     0 = NONE (not a waypoint, just an interpolated point along a line between waypoints)
#     1 = SIMPLE
#     2 = TRANSIT
#     3 = STARTLINE
#     4 = ENDLINE

# Definition of gps modes:
#     0 = Do GPS fix at surface at start and end of missions only
#     1 = Also do GPS fix at surface at each line start (waypoint = 3)
#     2 = Also do GPS fix at surface at each line end (waypoint = 4)
#     3 = Also do GPS fix at surface at each line start and end (waypoint = 3 || 4)

# set defaults

# mission
$altitudemin = 18.0;
$altitudeabort = 10.0;
$mission_speed = 1.5;

# behavior gps
$gpsminhits = 60;
$gpsduration = 600;

# behavior setpoint
$setpointtime = 30;

# behavior descend
$descendpitch = -15;
$descentdepth = 3.0;
$descendtime = 300;

# behavior ascend
$ascendrudder = 10;
$ascendpitch = 20;
$ascendenddepth = 2;

# behavior reson
$resonduration = 6;

# behavior waypoint
$depthmax = 130.0;
$depthabort = 150.0;

# assumed ascent and descent rate
$ascendrate = 0.5; # m/s

$forwarddist = 400.0;
$waypointdist = 200.0;

# Deal with command line arguments
&Getopts('A:a:D:d:F:f:G:g:HhI:i:MmO:o:P:p:S:s:T:t:W:w:V*v*Zz');
$altitudearg =		($opt_A || $opt_a);
$deptharg =		($opt_D || $opt_d);
$forwarddist =		($opt_F || $opt_f || $forwarddist);
$gpsmode =		($opt_G || $opt_g);
$help =			($opt_H || $opt_h);
$routefile =		($opt_I || $opt_i);
$mappingsonar =		($opt_M || $opt_m);
$missionfile =		($opt_O || $opt_o);
$startposition =	($opt_P || $opt_p);
$mission_speed =	($opt_S || $opt_s || $mission_speed);
$starttime =		($opt_T || $opt_t);
$waypointdist =		($opt_W || $opt_w || $waypointdist);
$verbose =		($opt_V || $opt_v);
$outputoff =		($opt_Z || $opt_z);

# print out help message if required
if ($help) {
    print "\r\n$program_name:\r\n";    
    print "\r\nPerl shellscript to translate survey route file derived from \r\n";
    print "MBgrdviz into an MBARI AUV mission script. Developed for use\r\n";
    print "in survey planning for the MBARI Mapping AUV.\r\n";
    print "Usage: mbm_route2mission -Iroutefile \r\n";
    print "\t\t[-Aaltitudemin/altitudeabort -Ddepthmax/depthabort[/depthdescent] -Fforwarddistance \r\n";
    print "\t\t-Ggpsmode -Omissionfile -Pstartlon/startlat -Sstarttime -Wwaypointspacing -Z -V -H]\r\n";
    exit 0;
}

# set debug mode
if ($verbose > 1)
	{
	$debug = 1;
	}

# get user info
$user = `whoami`;
$date = `date`;
$host = `hostname -s`;
chop $user;
chop $date;
chop $host;

# deal with command line values
if ($altitudearg)
	{
	($altitudemin, $altitudeabort) 
		= $altitudearg =~ /^(\S+)\/(\S+)/;
	}
if ($deptharg
	&& $deptharg =~ /^(\S+)\/(\S+)\/(\S+)/)
	{
	($depthmax, $depthabort, $descentdepth) 
		= $deptharg =~ /^(\S+)\/(\S+)\/(\S+)/;
	}
elsif ($deptharg
	&& $deptharg =~ /^(\S+)\/(\S+)/)
	{
	($depthmax, $depthabort) 
		= $deptharg =~ /^(\S+)\/(\S+)/;
	}
if ($startposition)
	{
	($startlon, $startlat) 
		= $startposition =~ /^(\S+)\/(\S+)/;
	}

# Open the input file
open(RFILE,"$routefile") || die "Cannot open input route file: $routefile\r\n$program_name aborted.\r\n";
if ($routefile =~ /\S+.rte/)
	{
	($root) = $routefile =~ /(\S+).rte/;
	}
else
	{
	$root = $routefile;
	}
	
# Read in the initial route data
$cnt = 0;
while ($line = <RFILE>) 
	{
	if ($line =~ /^## /)
		{
#		printf "Comment: $line";
		}
	elsif ($line =~ /^> ## STARTROUTE/)
		{
#		printf "Route: $line";
		}
	elsif ($line =~ /^> ## ENDROUTE/)
		{
#		printf "Route: $line";
		}
	elsif ($line =~ /(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)/)
		{
		#extract route data
		($lon, $lat, $topo, $waypoint, $bearing, $distance, $distonbottom, $slope) 
			= $line 
			=~ /(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)/;
		push(@lons, $lon);
		push(@lats, $lat);
		push(@topos, $topo);
		push(@waypoints, $waypoint);
		push(@bearings, $bearing);
		push(@distances, $distance);
		push(@distonbottoms, $distonbottom);
		push(@slopes, $slope);

		# increment counter
		$cnt++;
		}
	}
$npoints = $cnt;
close(RFILE);
	
# Count waypoints and gps events
$cnt = 0;
$nwaypoints = 0;
$ngpsevents = 0;
foreach $lon (@lons)
	{
		
	# count waypoints
	if ($waypoints[$cnt] != 0)
		{
		$nwaypoints++;
		}

	# count surface GPS events
	if (($cnt == 0 || $cnt == $npoints - 1)
		|| ($waypoints[$cnt] == 3 && $gpsmode == 1)
		|| ($waypoints[$cnt] == 4 && $gpsmode == 2)
		|| ($waypoints[$cnt] >= 3 && $gpsmode == 3))
		{
		$ngpsevents++;
		}

	# increment counter
	$cnt++;
	}
	
# Write out the input route data
if ($debug)
	{
	$cnt = 0;
	foreach $lon (@lons)
		{
		# print route data
		print "$waypoints[$cnt] $lons[$cnt] $lats[$cnt] $topos[$cnt] $bearings[$cnt] $distances[$cnt] $distonbottoms[$cnt] $slopes[$cnt]\r\n";

		# increment counter
		$cnt++;
		}
	}
	
# Process the route data to generate AUV waypoints that keep the 
# vehicle a safe distance above the bottom.
$distancelastmpoint = 0.0;
$nmissionpoints = 0;
$ilast = 0;
for ($i = 0; $i < $npoints; $i++)
	{
	# first figure out if this is a point to be output
	$output = 0;
	if ($waypoints[$i] != 0)
		{
		$output = 1;
		}
	elsif (($distances[$i] - $distancelastmpoint) >= $waypointdist)
		{
		$output = 1;
		}
		
	# process points of interest
	if ($output == 1)
		{
		# insert points into arrays so they can be printed out last to first
		# to follow the MBARI AUV mission file convention
		push(@mwaypoints, $waypoints[$i]);
		push(@mlons, $lons[$i]);
		push(@mlats, $lats[$i]);
		push(@mtopos, $topos[$i]);
		push(@mbearings, $bearings[$i]);
		push(@mdistances, $distances[$i]);
		$nmissionpoints++;
		
		# print it out
		if ($debug)
			{
			print "$i $waypoints[$i] $lons[$i] $lats[$i] $topos[$i] $topomax $distances[$i] $bearings[$i]\r\n";
			}
print "$nmissionpoints $waypoints[$i] $lons[$i] $lats[$i] $topos[$i] $topomax $distances[$i] $bearings[$i]\r\n";

		# reset distance from last mission point
		$distancelastmpoint = $distances[$i];
		$ilast = $i;
		}
	}
	
# get safe depth for each mission segment
for ($i = 0; $i < $nmissionpoints; $i++)
	{
	$topomax = $mtopos[$i];
	for ($j = 0; $j < $npoints; $j++)
		{
		$ii = $i - 1;
		if ($ii < 0)
			{
			$ii = 0;
			}
		$ddist = $distances[$j] - $mdistances[$ii];
		if ($ddist > 0.0 
			&& $ddist <= $forwarddist
			&& $topos[$j] > $topomax)
			{
			$topomax = $topos[$j];
			}
		}
	push(@mtopomaxs, $topomax);
	push(@mmissiondepths, $topomax);
	$ascendtime = 1.2 * $topomax / $ascendrate;
	if ($ascendtime < 300)
		{
		$ascendtime = 300;
		}
	push(@ascendtimes, $ascendtime);
	}

# Calculate time to first waypoint
if ($startposition)
	{
	# get local scaling of lon lat to distance
	$C1 = 111412.84;
	$C2 = -93.5;
	$C3 = 0.118;
	$C4 = 111132.92;
	$C5 = -559.82;
	$C6 = 1.175;
	$C7 = 0.0023;
	$DTR = 3.14159265358979323846 / 180.0;
	$radlat = $mlats[0] * DTR;
	$mtodeglat = 1./abs($C4 + $C5*cos(2*$radlat) 
				+ $C6*cos(4*$radlat) + $C7*cos(6*$radlat));
	$mtodeglon = 1./abs($C1*cos($radlat) + $C2*cos(3*$radlat) 
				+ $C3*cos(5*$radlat));
				
	# calculate distance to first point
	$dx = ($mlons[0] - $startlon) / $mtodeglon;
	$dy = ($mlats[0] - $startlat) / $mtodeglat;
	$startdistance = sqrt($dx * $dx + $dy * $dy);
	}
elsif ($starttime)
	{
	$startdistance = $starttime * $mission_speed;
	}
else
	{
	$startdistance = 500.0;
	}

# Calculate estimated time of mission
$missiontime = 0.0;

# time from running lines
$missiontime += $distancelastmpoint / $mission_speed;

# add time to get to first waypoint
$missiontime += $startdistance / $mission_speed;

# add time for initial gps and descent
$missiontime += $gpsduration + $descendtime;

# add time for each ascent, gps, descent event
for ($i = 1; $i < $nmissionpoints - 1; $i++)
	{
	if (($gpsmode == 1 && $mwaypoints[$i] == 3) 
			|| ($gpsmode == 2 && $mwaypoints[$i] == 4) 
			|| ($gpsmode == 3 && $mwaypoints[$i] >= 3))
		{
		$missiontime += (-$mmissiondepths[$i] / $ascendrate) + $gpsduration + $descendtime;
		}
	}

# add time for final ascent and gps
$missiontime += (-$mmissiondepths[$nmissionpoints - 1] / $ascendrate) 
		+ $gpsduration;
		
# calculate abort time using safety factor of 1.2
$aborttime = 1.2 * $missiontime;
	
# Print out program info
if ($outputoff)
	{
	printf "Route: $routefile\r\n";
	printf "Distance: %.0f (m)\r\n", $distancelastmpoint;
	printf "Time: %d (s)\r\n", $missiontime;
	}
elsif ($verbose)
	{
	printf "MB-System program $program_name run by\r\n";
	printf "user <$user> on cpu <$host> at <$date>\r\n";
	printf "\r\n";
	printf "Mission Summary:\r\n";
	printf "    Route File:               $routefile\r\n";
	printf "    Mission File:             $missionfile\r\n";
	printf "    Distance:                 %.0f (m)  %.3f (km)\r\n", $distancelastmpoint, 0.001 * $distancelastmpoint;
	printf "    Estimated Time:           %d (s)  %.3f (hr)\r\n", $missiontime, $missiontime / 3600.0;
	printf "    Abort Time:               %d (s)\r\n", $aborttime;
	printf "    Way Points:               $nwaypoints\r\n";
	printf "    Route Points:             $nmissionpoints\r\n";
	if ($mappingsonars)
		{
		printf "    Mapping sonar control enabled:          \r\n";
		}
	else
		{
		printf "    Mapping sonar control disabled:          \r\n";
		}
	printf "\r\n";
	printf "Mission Parameters:\r\n";
	printf "    Vehicle Speed:            %f (m/s) %f (knots)\r\n", $mission_speed, 1.943846 * $mission_speed;
	printf "    Minimum Vehicle Altitude: $altitudemin (m)\r\n";
	printf "    Abort Vehicle Altitude:   $altitudeabort (m)\r\n";
	printf "    Maximum Vehicle Depth:    $depthmax (m)\r\n";
	printf "    Abort Vehicle Depth:      $depthabort (m)\r\n";
	printf "    Descent Vehicle Depth:    $descentdepth (m)\r\n";
	printf "    Forward Looking Distance: $forwarddist (m)\r\n";
	printf "    Waypoint Spacing:         $waypointdist (m)\r\n";
	if ($starttime)
		{
		printf "    Time to First Waypoint:   %d (s)\r\n", $starttime;
		}
	if ($startposition)
		{
		printf "    Start Longitude:          $startlon (deg)\r\n";
		printf "    Start Latitude:           $startlat (deg)\r\n";
		}
	printf "    GPS Duration:             %d (s)\r\n", $gpsduration;
	printf "    Ascend/Descend Rate:      $ascendrate (m/s)\r\n";
	printf "    Descend Duration:         %d (s)\r\n", $descendtime;
	printf "    Setpoint Duration:        %d (s)\r\n", $setpointtime;
	printf "\r\n";
	printf "The primary waypoints from the route file are:\r\n";
	printf "  <number> <longitude (deg)> <latitude (deg)> <topography (m)> <distance (m)> <type>\r\n";
	$cnt = 0;
	for ($i = 0; $i < $npoints; $i++)
 		{
		if ($waypoints[$i] != 0)
			{
 			printf "  $cnt $lons[$i] $lats[$i] $topos[$i] $distances[$i] $waypoints[$i]\r\n";
			$cnt++;
			}
 		}
	}
else
	{
	printf "MB-System program $program_name run by\r\n";
	printf "user <$user> on cpu <$host> at <$date>\r\n";
	printf "\r\n";
	printf "Mission Summary:\r\n";
	printf "    Route File:               $routefile\r\n";
	printf "    Mission File:             $missionfile\r\n";
	printf "    Distance:                 %.0f (m)  %.3f (km) \r\n", $distancelastmpoint, 0.001 * $distancelastmpoint;
	printf "    Estimated Time:           %d (s)  %.3f (hr) \r\n", $missiontime, $missiontime / 3600.0;
	printf "    Abort Time:               %d (s)\r\n", $aborttime;
	printf "    Way Points:               $nwaypoints\r\n";
	printf "    Route Points:             $nmissionpoints\r\n";
	}

# output mission file unless outputoff option selected
if (!$outputoff)
	{
	# open the output file 
	if (!$missionfile)
		{
		$missionfile = "$root.cfg";
		}
	open(MFILE,">$missionfile") || die "Cannot open output mission file: $missionfile\r\n$program_name aborted.\r\n";

	# output mission file comments
	printf MFILE "# This MBARI Mapping AUV mission file has been generated\r\n";
	printf MFILE "# by the MB-System program $program_name run by\r\n";
	printf MFILE "# user <$user> on cpu <$host> at <$date>\r\n";
	printf MFILE "# \r\n";
	printf MFILE "# Mission Summary:\r\n";
	printf MFILE "#     Route File:               $routefile\r\n";
	printf MFILE "#     Mission File:             $missionfile\r\n";
	printf MFILE "#     Distance:                 $distancelastmpoint (m)\r\n";
	printf MFILE "#     Estimated Time:           %d (s)  %.3f (hr)\r\n", $missiontime, $missiontime / 3600.0;
	printf MFILE "#     Abort Time:               %d (s)\r\n", $aborttime;
	printf MFILE "#     Way Points:               $nwaypoints\r\n";
	printf MFILE "#     Route Points:             $nmissionpoints\r\n";
	if ($mappingsonars)
		{
		printf "MFILE "#     Mapping sonar control enabled:          \r\n";
		}
	else
		{
		printf "MFILE "#     Mapping sonar control disabled:          \r\n";
		}
	printf MFILE "# \r\n";
	printf MFILE "# Mission Parameters:\r\n";
	printf MFILE "#     Minimum Vehicle Altitude: $altitudemin (m)\r\n";
	printf MFILE "#     Abort Vehicle Altitude:   $altitudeabort (m)\r\n";
	printf MFILE "#     Maximum Vehicle Depth:    $depthmax (m)\r\n";
	printf MFILE "#     Abort Vehicle Depth:      $depthabort (m)\r\n";
	printf MFILE "#     Descent Vehicle Depth:    $descentdepth (m)\r\n";
	printf MFILE "#     Forward Looking Distance: $forwarddist (m)\r\n";
	printf MFILE "#     Waypoint Spacing:         $waypointdist (m)\r\n";
	if ($starttime)
		{
		printf MFILE "#     Time to First Waypoint:   %d (s)\r\n", $starttime;
		}
	if ($startposition)
		{
		printf MFILE "#     Start Longitude:          $startlon (deg)\r\n";
		printf MFILE "#     Start Latitude:           $startlat (deg)\r\n";
		}
	printf MFILE "#     GPS Duration:             %d (s)\r\n", $gpsduration;
	printf MFILE "#     Ascend Rate:              $ascendrate (m/s)\r\n";
	printf MFILE "#     Descend Duration:         %d (s)\r\n", $descendtime;
	printf MFILE "#     Setpoint Duration:        %d (s)\r\n", $setpointtime;
	printf MFILE "# \r\n";
	printf MFILE "# The primary waypoints from the route file are:\r\n";
	printf MFILE "#   <number> <longitude (deg)> <latitude (deg)> <topography (m)> <distance (m)> <type>\r\n";
	$cnt = 0;
	for ($i = 0; $i < $npoints; $i++)
 		{
		if ($waypoints[$i] != 0)
			{
 			printf MFILE "#   $cnt $lons[$i] $lats[$i] $topos[$i] $distances[$i] $waypoints[$i]\r\n";
			$cnt++;
			}
 		}
	printf MFILE "# \r\n";
	printf MFILE "# A total of %d mission points have been defined.\r\n", $nmissionpoints;
	printf MFILE "# \r\n";
	printf MFILE "# Define Mission parameters:\r\n";
	printf MFILE "#define MISSION_SPEED     %f\r\n", $mission_speed;
	printf MFILE "#define MISSION_DISTANCE  %f\r\n", $distancelastmpoint;
	printf MFILE "#define MISSION_TIME      %d\r\n", $missiontime;
	printf MFILE "#define MISSION_TIMEOUT   %d\r\n", $aborttime;
	printf MFILE "#define DEPTH_MAX         %f\r\n", $depthmax;
	printf MFILE "#define DEPTH_ABORT       %f\r\n", $depthabort;
	printf MFILE "#define ALTITUDE_MIN      %f\r\n", $altitudemin;
	printf MFILE "#define ALTITUDE_ABORT    %f\r\n", $altitudeabort;
	printf MFILE "#define GPS_DURATION      %d\r\n", $gpsduration;
	printf MFILE "#define DESCENT_DEPTH     %f\r\n", $descentdepth;
	printf MFILE "#define DESCEND_DURATION  %d\r\n", $descendtime;
	printf MFILE "#define SETPOINT_DURATION %d\r\n", $setpointtime;
	printf MFILE "#define GPSMINHITS        %d\r\n", $gpsminhits;
	printf MFILE "#define ASCENDRUDDER      %f\r\n", $ascendrudder;
	printf MFILE "#define ASCENDPITCH       %f\r\n", $ascendpitch;
	printf MFILE "#define ASCENDENDDEPTH    %f\r\n", $ascendenddepth;
	printf MFILE "#define DESCENDPITCH      %f\r\n", $descendpitch;
	printf MFILE "#define RESON_DURATION    %d\r\n", $resonduration;
	print MFILE "# \r\n";
	print MFILE "#######################################################\r\n";
	print MFILE "# Set Mission Behaviors\r\n";
	print MFILE "# \r\n";
	print MFILE "# mission timer set to 120% of estimated time of mission\r\n";
	print MFILE "behavior missionTimer \r\n";
	print MFILE "{\r\n";
	print MFILE "timeOut = MISSION_TIMEOUT;\r\n";
	print MFILE "}\r\n";
	print MFILE "# \r\n";
	print MFILE "# depth envelope\r\n";
	print MFILE "behavior depthEnvelope \r\n";
	print MFILE "{\r\n";
	print MFILE "minDepth = 0;\r\n";
	print MFILE "maxDepth     = DEPTH_MAX;\r\n";
	print MFILE "abortDepth   = DEPTH_ABORT;\r\n";
	print MFILE "minAltitude  = ALTITUDE_MIN;\r\n";
	print MFILE "abortAltitude = ALTITUDE_ABORT;\r\n";
	print MFILE "}\r\n";
	print MFILE "#######################################################\r\n";
	print MFILE "# Set End-of-Mission Behaviors\r\n";
	print MFILE "# \r\n";
	print MFILE "# \r\n";
	print MFILE "# acquire gps fix \r\n";
	print MFILE "behavior getgps  \r\n";
	print MFILE "{ \r\n";
	print MFILE "duration = GPS_DURATION; \r\n";
	print MFILE "minHits = GPSMINHITS; \r\n";
	print MFILE "abortOnTimeout = False; \r\n";
	print MFILE "} \r\n";
	print MFILE "# \r\n";
	print MFILE "# ascend behavior \r\n";
	print MFILE "behavior ascend  \r\n";
	print MFILE "{ \r\n";
	$i = $nmissionpoints - 1;
	print MFILE "duration  = $ascendtimes[$i]; \r\n";
	print MFILE "horizontalMode   = rudder; \r\n";
	print MFILE "horizontal       = ASCENDRUDDER; \r\n";
	print MFILE "pitch            = ASCENDPITCH; \r\n";
	print MFILE "speed            = MISSION_SPEED; \r\n";
	print MFILE "endDepth         = ASCENDENDDEPTH; \r\n";
	print MFILE "} \r\n";
	print MFILE "# \r\n";
	if ($mappingsonars)
		{
		print MFILE "# Turn off power to sonars and stop logging on the PLC \r\n";
		print MFILE "# by setting the value of the mode attribute to 0 (used to be False). \r\n";
		print MFILE "behavior reson \r\n";
		print MFILE "{ \r\n";
		print MFILE "duration  = RESON_DURATION; \r\n";
		print MFILE "SBP_Mode  = 0; \r\n";
		print MFILE "LoSS_Mode = 0; \r\n";
		print MFILE "HiSS_Mode = 0; \r\n";
		print MFILE "Log_Mode  = 0; \r\n";
		print MFILE "} \r\n";
		}

	# output mission points in reverse order
	$iwaypoint = $nwaypoints;
	for ($i = $nmissionpoints - 1; $i >= 0; $i--)
		{
		# figure out which waypoint this is
		if ($mwaypoints[$i] != 0)
			{
			$iwaypoint--;
			}

		# put comment break in at waypoint
		if ($mwaypoints[$i] != 0)
			{
			print MFILE "#######################################################\r\n";
			}

		# calculate distance and depth
		if ($i == 0)
			{
			$distance = $startdistance;
			}
		else
			{
			$distance = $mdistances[$i] - $mdistances[$i-1];
			}
		$mmissiondepths[$i] = -$mtopomaxs[$i] - $altitudemin;
		$maxdepthapplied = 0;
		if ($mmissiondepths[$i] > $depthmax)
			{
			$mmissiondepths[$i] = $depthmax;
			$maxdepthapplied = 1;
			}
		elsif ($mmissiondepths[$i] < $altitudemin)
			{
			$mmissiondepths[$i] = $altitudemin;
			$maxdepthapplied = 0;
			}

		# do ascend, gps, descend at line starts and ends if specified
		if (($iwaypoint != $nwaypoints - 1) 
			&& (($gpsmode == 1 && $mwaypoints[$i] == 3) 
				|| ($gpsmode == 2 && $mwaypoints[$i] == 4) 
				|| ($gpsmode == 3 && $mwaypoints[$i] >= 3)))
			{
			print MFILE "# \r\n";
			printf MFILE "# Ascend, gps, descend after reaching waypoint %d at end of line %d\r\n", $iwaypoint, $iwaypoint;
			print MFILE "# \r\n";
			print MFILE "# Descend behavior \r\n";
			print MFILE "behavior descend  \r\n";
			print MFILE "{ \r\n";
			print MFILE "horizontalMode   = heading; \r\n";
			printf MFILE "horizontal   = %f; \r\n", $mbearings[$i];
			print MFILE "pitch        = DESCENDPITCH; \r\n";
			print MFILE "speed        = MISSION_SPEED; \r\n";
			print MFILE "maxDepth     = DESCENT_DEPTH; \r\n";
			print MFILE "minAltitude  = ALTITUDE_MIN; \r\n";
			print MFILE "duration     = DESCEND_DURATION; \r\n";
			print MFILE "} \r\n";
			print MFILE "# \r\n";
			print MFILE "# setpoint on surface to gather momentum \r\n";
			print MFILE "behavior setpoint \r\n";
			print MFILE "{ \r\n";
			print MFILE "duration     = SETPOINT_DURATION; \r\n";
			printf MFILE "heading      = %f; \r\n", $mbearings[$i];
			print MFILE "speed        = MISSION_SPEED; \r\n";
			print MFILE "verticalMode = pitch; \r\n";
			print MFILE "pitch        = 0; \r\n";
			print MFILE "} \r\n";
			print MFILE "# \r\n";
			print MFILE "# acquire gps fix \r\n";
			print MFILE "behavior getgps  \r\n";
			print MFILE "{ \r\n";
			print MFILE "duration = GPS_DURATION; \r\n";
			print MFILE "minHits = GPSMINHITS; \r\n";
			print MFILE "abortOnTimeout = False; \r\n";
			print MFILE "} \r\n";
			print MFILE "# \r\n";
			print MFILE "# ascend behavior \r\n";
			print MFILE "behavior ascend  \r\n";
			print MFILE "{ \r\n";
			printf MFILE "duration  = %d; \r\n", $ascendtimes[$i];
			print MFILE "horizontalMode   = rudder; \r\n";
			print MFILE "horizontal       = ASCENDRUDDER; \r\n";
			print MFILE "pitch            = ASCENDPITCH; \r\n";
			print MFILE "speed            = MISSION_SPEED; \r\n";
			print MFILE "endDepth         = ASCENDENDDEPTH; \r\n";
			print MFILE "} \r\n";
			print MFILE "# \r\n";
			}

		# output mission point behavior
		print MFILE "# \r\n";
		if ($mwaypoints[$i] != 0 && $iwaypoint == 0)
			{
			print MFILE "# Waypoint behavior to get to start of line 1\r\n";
			}
		elsif ($mwaypoints[$i] != 0)
			{
			printf MFILE "# Waypoint behavior to get to end of line %d\r\n", $iwaypoint;
			}
		else
			{
			printf MFILE "# Waypoint behavior during line %d\r\n", $iwaypoint;
			}
		printf MFILE "#   Segment length %f meters\r\n", $distance;
		printf MFILE "#   Minimum depth: %f meters looking forward %f meters along route\r\n", -$mtopomaxs[$i], $forwarddist;
		printf MFILE "#   Maximum vehicle depth: %f meters\r\n", $depthmax;
		printf MFILE "#   Minimum vehicle altitude: %f meters\r\n", $altitudemin;
		if ($maxdepthapplied == 0)
			{
			printf MFILE "#   Behavior depth of %f meters set by local depth and minimum altitude\r\n", $mmissiondepths[$i];
			}
		else
			{
			printf MFILE "#   Behavior depth of %f meters set to maximum vehicle depth\r\n", $mmissiondepths[$i];
			}
		print MFILE "behavior waypoint  \r\n";
		print MFILE "{ \r\n";
		printf MFILE "latitude     = %f; \r\n", $mlats[$i];
		printf MFILE "longitude    = %f; \r\n", $mlons[$i];
		print MFILE "captureRadius = 10; \r\n";
		printf MFILE "duration     = %d; \r\n", (1.2 * $distance / $mission_speed);
		printf MFILE "depth        = %f; \r\n", $mmissiondepths[$i];
		print MFILE "speed        = MISSION_SPEED; \r\n";
		print MFILE "} \r\n";

		# put comment break in at waypoint
		if ($mwaypoints[$i] != 0)
			{
			print MFILE "#######################################################\r\n";
			}
		}

	# output beginning of mission
	print MFILE "#######################################################\r\n";
	print MFILE "# \r\n";
	if ($mappingsonars)
		{
		print MFILE "# Set power levels and high-freq rate \r\n";
		print MFILE "# \r\n";
		print MFILE "behavior reson \r\n";
		print MFILE "{ \r\n";
		print MFILE "duration  = RESON_DURATION; \r\n";	
		print MFILE "Reson_Power = 100.0; \r\n";
		print MFILE "Reson_Range = 450.0; \r\n";
		print MFILE "Reson_Rate = 1.0; \r\n";
		print MFILE "SBP_Power = 100.0; \r\n";
		print MFILE "SBP_Range = 450.0; \r\n";
		print MFILE "LoSS_Power = 100.0; \r\n";
		print MFILE "LoSS_Range = 450.0; \r\n";
		print MFILE "LoSS_Rate  = 1.0; \r\n";
		print MFILE "HiSS_Power = 100.0; \r\n";
		print MFILE "HiSS_Range = 450.0; \r\n";
		print MFILE "HiSS_Rate  = 1.0; \r\n";
		print MFILE "} \r\n";
		print MFILE "# \r\n";
		print MFILE "# Turn on power to sonars and start logging on the PLC \r\n";
		print MFILE "# by setting the value of the mode attribute to 1 (used to be True). \r\n";
		print MFILE "# \r\n";
		print MFILE "behavior reson \r\n";
		print MFILE "{ \r\n";
		print MFILE "duration  = RESON_DURATION; \r\n";
		print MFILE "Reson_Mode  = 1; \r\n";
		print MFILE "SBP_Mode  = 1; \r\n";
		print MFILE "LoSS_Mode = 1; \r\n";
		print MFILE "HiSS_Mode = 1; \r\n";
		print MFILE "Log_Mode  = 1; \r\n";
		print MFILE "} \r\n";
		}
	print MFILE "# \r\n";
	print MFILE "# Descend behavior \r\n";
	print MFILE "behavior descend  \r\n";
	print MFILE "{ \r\n";
	print MFILE "horizontalMode   = heading; \r\n";
	printf MFILE "horizontal   = %f; \r\n", $mbearings[0];
	print MFILE "pitch        = DESCENDPITCH; \r\n";
	print MFILE "speed        = MISSION_SPEED; \r\n";
	print MFILE "maxDepth     = DESCENT_DEPTH; \r\n";
	print MFILE "minAltitude  = ALTITUDE_MIN; \r\n";
	print MFILE "duration     = DESCEND_DURATION; \r\n";
	print MFILE "} \r\n";
	print MFILE "# \r\n";
	print MFILE "# setpoint on surface to gather momentum \r\n";
	print MFILE "behavior setpoint \r\n";
	print MFILE "{ \r\n";
	print MFILE "duration     = SETPOINT_DURATION; \r\n";
	printf MFILE "heading      = %f; \r\n", $mbearings[0];
	print MFILE "speed        = MISSION_SPEED; \r\n";
	print MFILE "verticalMode = pitch; \r\n";
	print MFILE "pitch        = 0; \r\n";
	print MFILE "} \r\n";
	print MFILE "# \r\n";
	print MFILE "# acquire gps fix \r\n";
	print MFILE "behavior getgps  \r\n";
	print MFILE "{ \r\n";
	print MFILE "duration = GPS_DURATION; \r\n";
	print MFILE "minHits = GPSMINHITS; \r\n";
	print MFILE "abortOnTimeout = False; \r\n";
	print MFILE "} \r\n";
	print MFILE "#######################################################\r\n";

	# Close the output file
	close(MFILE);

	# generate data for plots
	$topodatafile = "$root" . "_topo.xy";
	open(TFILE,">$topodatafile") || die "Cannot open output distance vs topo file: $topodatafile\r\n$program_name aborted.\r\n";
	for ($i = 0; $i < $npoints; $i++)
		{
		printf TFILE "%f %f\r\n", $distances[$i], $topos[$i];
		}
	close(TFILE);
	$topodatafile = "$root" . "_mission.xy";
	open(TFILE,">$topodatafile") || die "Cannot open output distance vs topo file: $topodatafile\r\n$program_name aborted.\r\n";
	for ($i = 0; $i < $nmissionpoints-1; $i++)
		{
		printf TFILE "%f %f\r\n", $mdistances[$i], -$mmissiondepths[$i];
		printf TFILE "%f %f\r\n", $mdistances[$i+1], -$mmissiondepths[$i];
		}
	close(TFILE);
	}

# End it all
if ($verbose)
	{
	print "\r\nAll done!\r\n";
	}
exit 0;

#-----------------------------------------------------------------------
# This should be loaded from the library but its safer to
# just include it....
#
;# getopts.pl - a better getopt.pl

;# Usage:
;#      do Getopts('a:b*c');  # -a takes arg. -b & -c not. Sets opt_* as a
;#                           #  side effect.

sub Getopts {
    local($argumentative) = @_;
    local(@args,$_,$first,$rest);
    local($errs) = 0;
    local($[) = 0;

    @args = split( / */, $argumentative );
    while(@ARGV && ($_ = $ARGV[0]) =~ /^-(.)(.*)/) {
     ($first,$rest) = ($1,$2);
     $pos = index($argumentative,$first);
     if($pos >= $[) {
         if($args[$pos+1] eq ':') {
          shift(@ARGV);
          if($rest eq '') {
              ++$errs unless @ARGV;
              $rest = shift(@ARGV);
          }
          eval "\$opt_$first = \$rest;";
         }
         elsif($args[$pos+1] eq '*') {
          eval "\$opt_$first += 1";
          if($rest eq '') {
              shift(@ARGV);
          }
          else {
              $ARGV[0] = "-$rest";
          }
         }
	 else {
          eval "\$opt_$first = 1";
          if($rest eq '') {
              shift(@ARGV);
          }
          else {
              $ARGV[0] = "-$rest";
          }
         }
     }
     else {
         print STDERR "Unknown option: $first\r\n";
         ++$errs;
         if($rest ne '') {
          $ARGV[0] = "-$rest";
         }
         else {
          shift(@ARGV);
         }
     }
    }
    $errs == 0;
}


#-----------------------------------------------------------------------
