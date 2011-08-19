eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
                    & eval 'exec perl -S $0 $argv:q'
                         if 0;
#--------------------------------------------------------------------
#    The MB-system: mbm_route2mission.perl   7/18/2004
#    $Id$
#
#    Copyright (c) 2004-2011 by 
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
#   mbm_route2mission -Iroutefile [-Aaltitudemin/altitudeabort[/altitudedesired] 
#                     -Bbehavior -Ddepthmax/depthabort[/depthdescent]
#                     -Fforwarddistance -Ggpsmode -M[sensors] -Omissionfile 
#                     -P[startdistance | startlon/startlat] -Sspeed -Tstarttime -Wwaypointspacing -V -H]
#
# 
# Author:
#   David W. Caress
#      Monterey Bay Aquarium Research Institute
#      Moss Landing, CA
#
# Version:
# $Id$
#
# Revisions:
#   $Log: mbm_route2mission.perl,v $
#   Revision 5.21  2008/12/05 17:32:51  caress
#   Check-in mods 5 December 2008 including contributions from Gordon Keith.
#
#   Revision 5.20  2008/11/16 21:51:18  caress
#   Updating all recent changes, including time lag analysis using mbeditviz and improvements to the mbgrid footprint gridding algorithm.
#
#   Revision 5.19  2008/10/17 07:52:44  caress
#   Check in on October 17, 2008.
#
#   Revision 5.18  2008/09/27 03:27:10  caress
#   Working towards release 5.1.1beta24
#
#   Revision 5.17  2008/09/11 20:06:46  caress
#   Checking in updates made during cruise AT15-36.
#
#   Revision 5.16  2008/05/16 22:36:21  caress
#   Release 5.1.1beta18
#
#   Revision 5.15  2008/02/12 02:51:51  caress
#   Changes in preparation for 2008 MBARI Mapping AUV operations.
#
#   Revision 5.14  2007/10/08 05:45:46  caress
#   Added zero point setpoint to descent sequences and can output WPL files.
#
#   Revision 5.13  2007/05/14 17:10:12  caress
#   Mods during May 2007 Lucia Canyon cruise.
#
#   Revision 5.12  2007/03/02 20:33:37  caress
#   Fixed informational output.
#
#   Revision 5.11  2006/11/26 09:42:01  caress
#   Making distribution 5.1.0.
#
#   Revision 5.10  2006/11/10 22:36:04  caress
#   Working towards release 5.1.0
#
#   Revision 5.9  2006/09/11 18:55:52  caress
#   Changes during Western Flyer and Thomas Thompson cruises, August-September
#   2006.
#
#   Revision 5.8  2006/07/27 18:42:51  caress
#   Working towards 5.1.0
#
#   Revision 5.7  2006/06/16 19:30:58  caress
#   Check in after the Santa Monica Basin Mapping AUV Expedition.
#
#   Revision 5.6  2006/04/11 19:12:53  caress
#   Updates during Mapping AUV test operations.
#
#   Revision 5.5  2005/11/05 01:34:20  caress
#   Much work over the past two months.
#
#   Revision 5.4  2005/06/04 04:28:43  caress
#   Added support for new waypoint_depth AUV behavior.
#
#   Revision 5.3  2005/04/07 04:14:12  caress
#   Added mission speed and mapping sonar options.
#
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
#     5 = STARTLINE2
#     6 = ENDLINE2
#     7 = STARTLINE3
#     8 = ENDLINE3
#     9 = STARTLINE4
#     10 = ENDLINE4

# Definition of gps modes:
#     0 = Do GPS fix at surface at start and end of missions only
#     1 = Also do GPS fix at surface at each line start (waypoint = 3)
#     2 = Also do GPS fix at surface at each line end (waypoint = 4)
#     3 = Also do GPS fix at surface at each line start and end (waypoint = 3 || 4)

# Derivation of timeout duration values
#     Use 3.0 * expected duration for waypoint behaviors
#     Use 1.3 * expected duration the mission as a whole
#     Abort time must not be greater than battery life of 8 hours = 28800 seconds
$durationfactorwaypoint = 3.0;
$durationfactormission = 1.3;
$durationmax = 28800;
$batterylife = 64800; # 18 hours
$safetymargin = 1800;

# set defaults

# route name
$routename = "Survey";

# altitude controls
$altitudemin = 50.0;
$altitudeabort = 25.0;
$altitudedesired = $altitudemin;
$altitudedesired2 = $altitudemin;
$deltadepthrestart = $altitudemin - $altitudeabort;

# constant depth controls
$depthconstant = 50.0;
$depthconstant2 = 50.0;

# speed
$ascentdescent_speed = 1.5;
$survey_speed = 1.5;
$transit_speed = 1.5;

# behavior gps
$gpsminhits = 10; # used to be 30, but vehicle started to time out and abort 3/20/2008
$gpsduration = 600;
$gpsmode = 0;

# behavior setpoint
$setpointtime = 30;

# behavior descend
$descendpitch = -35;
$descendrudder = 3;
$descentdepth = 3.0;
$initialdescendtime = 300;

# behavior ascend
$ascendrudder = 3;
$ascendpitch = 45;
$ascendenddepth = 2;

# behavior reson
$resonduration = 6;
$sonaraltitudemax = 100.0;
$mb_pingrate = 3.0;
$mb_transmitgain = 220.0;
$mb_receivegain = 75.0;
$mb_minrangefraction = 0.2;
$mb_pulsewidth = 0.000060;
$resongainsetcount = 0;
$mb_snippetmode = 1;

# behavior waypoint and waypoint_depth
$behaviorWaypointID = 0;
$behaviorWaypointDepthID = 1;
$depthmax = 5900.0;
$depthabort = 6000.0;
$maxclimbslope = 0.5734;

# default waypoint behavior to use
$behavior = $behaviorWaypointID;
$behaviorarg = -1;

# spiral descent approach depth
$approachdepth = 50.0;

# assumed ascent and descent rate
$ascendrate = 1.0; # m/s
$descendrate = 0.417; # m/s

#$forwarddist = 0;
$waypointdist = 200.0;
$maxslopedeg = 25;
$DTR  = 3.1415926 / 180.0;
$maxslope = sin($DTR * $maxslopedeg) / cos($DTR * $maxslopedeg);

# Deal with command line arguments
&MBGetopts('A:a:B:b:C:c:D:d:F:f:G:g:HhI:i:J:j:L%l%M%mN%n%O:o:P:p:R:r:S:s:T:t:W:w:V*v*Zz');
$altitudearg =		($opt_A || $opt_a);
$behaviorarg =		($opt_B || $opt_b);
$aborttime = 		($opt_C || $opt_c);
$deptharg =		($opt_D || $opt_d);
$forwarddist =		($opt_F || $opt_f);
$gpsmode =		($opt_G || $opt_g || $gpsmode);
$help =			($opt_H || $opt_h);
$routefile =		($opt_I || $opt_i);
$depthprofilefile =	($opt_J || $opt_j);
$approachdepth =	($opt_L || $opt_l || $approachdepth);
$sensor =		($flg_M || $flg_m);
$sensorarg =		($opt_M || $opt_m);
$spiraldescent =	($flg_N || $flg_n);
$spiraldescentarg =	($opt_N || $opt_n);
$missionfile =		($opt_O || $opt_o);
$startposition =	($opt_P || $opt_p);
$multibeamsettings =	($opt_R || $opt_r);
$speedarg =		($opt_S || $opt_s);
$starttime =		($opt_T || $opt_t);
$waypointdist =		($opt_W || $opt_w || $waypointdist);
$verbose =		($opt_V || $opt_v);
$outputoff =		($opt_Z || $opt_z);

# print out help message if required
if ($help) {
    print "\r\n$program_name:\r\n";    
    print "\nVersion: $Id$\n";
    print "\r\nPerl shellscript to translate survey route file derived from \r\n";
    print "MBgrdviz into an MBARI AUV mission script. Developed for use\r\n";
    print "in survey planning for the MBARI Mapping AUV.\r\n";
    print "Usage: mbm_route2mission -Iroutefile \r\n";
    print "\t\t[-Aaltitudemin/altitudeabort[/altitudedesired] -Abehavior -Caborttime \r\n";
    print "-Ddepthconstant[/depthconstant2] -Fforwarddistance -Ggpsmode \r\n";
    print "\t\t-Jdepthprofilefile -Lapproachdepth -M[sonarlist] -N \r\n";
    print "-Omissionfile \r\n\t\t-P[startlon/startlat | startdistance] \r\n";
    print "\t\t-Rtransmitpower/receivegain[/rangeminfraction] \r\n";
    print "\t\t-Tstarttime -Wwaypointspacing -Z -V -H]\r\n";
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
if ($behaviorarg != -1)
	{
	if ($behaviorarg =~ /^\S+/)
		{
		($behavior) = $behaviorarg =~ /^(\S+)/;
		}
	}
if ($altitudearg)
	{
	if ($altitudearg =~ /^\S+\/\S+\/\S+\/\S+/)
		{
		($altitudemin, $altitudeabort, $altitudedesired, $altitudedesired2) 
			= $altitudearg =~ /^(\S+)\/(\S+)\/(\S+)\/(\S+)\/(\S+)\/(\S+)/;
		$deltadepthrestart = $altitudemin - $altitudeabort;
		}
	elsif ($altitudearg =~ /^\S+\/\S+\/\S+/)
		{
		($altitudemin, $altitudeabort, $altitudedesired) 
			= $altitudearg =~ /^(\S+)\/(\S+)\/(\S+)/;
		$deltadepthrestart = $altitudemin - $altitudeabort;
		$altitudedesired2 = $altitudedesired;
		}
	elsif ($altitudearg =~ /^\S+\/\S+/)
		{
		($altitudemin, $altitudeabort) 
			= $altitudearg =~ /^(\S+)\/(\S+)/;
		$altitudedesired = $altitudemin;
		$deltadepthrestart = $altitudemin - $altitudeabort;
		$altitudedesired2 = $altitudedesired;
		}
	else
		{
		($altitudemin) 
			= $altitudearg =~ /^(\S+)/;
		$altitudeabort = $altitudemin / 2;
		$altitudedesired = $altitudemin;
		$deltadepthrestart = $altitudemin - $altitudeabort;
		$altitudedesired2 = $altitudedesired;
		}
	}
if ($speedarg)
	{
	if ($speedarg =~ /^\S+\/\S+\/\S+/)
		{
		($survey_speed, $ascentdescent_speed, $transit_speed) 
			= $speedarg =~ /^(\S+)\/(\S+)\/(\S+)/;
		}
	elsif ($speedarg =~ /^\S+\/\S+/)
		{
		($survey_speed, $ascentdescent_speed) 
			= $speedarg =~ /^(\S+)\/(\S+)/;
		}
	else
		{
		($survey_speed) 
			= $speedarg =~ /^(\S+)/;
		}
	}

if ($deptharg)
	{
	if ($deptharg =~ /^(\S+)\/(\S+)/)
		{
		($depthconstant, $depthconstant2) 
			= $deptharg =~ /^(\S+)\/(\S+)/;
		}
	elsif ($deptharg =~ /^(\S+)/)
		{
		($depthconstant) 
			= $deptharg =~ /^(\S+)/;
		}
	}

if ($startposition && $startposition =~ /^(\S+)\/(\S+)/)
	{
	($startlon, $startlat) 
		= $startposition =~ /^(\S+)\/(\S+)/;
	}
elsif ($startposition)
	{
	($startdistance) = $startposition;
	}
	
# Set mapping sonar status
if ($sensorarg =~ /\S*M\S*/)
	{
	$mappingsonar = 1;
	}
if ($sensorarg =~ /\S*S\S*/)
	{
	$mappingsonar = 1;
	$subbottom = 1;
	}
if ($sensorarg =~ /\S*L\S*/)
	{
	$mappingsonar = 1;
	$sidescanlo = 1;
	}
if ($sensorarg =~ /\S*H\S*/)
	{
	$mappingsonar = 1;
	$sidescanhi = 1;
	}
if ($sensorarg =~ /\S*C\S*/)
	{
	$camera = 1;
	}
if ($sensorarg =~ /\S*B\S*/)
	{
	$mappingsonar = 1;
	$beamdata = 1;
	}
if ($sensor && !$sensorarg)
	{
	$mappingsonar = 1;
	$subbottom = 1;
	$sidescanlo = 1;
	$sidescanhi = 1;
	}
if ($spiraldescentarg =~ /\S+/)
	{
	($spiraldescentaltitude) = $spiraldescentarg =~ /(\S+)/;
	}
elsif ($spiraldescent)
	{
	$spiraldescentaltitude = $altitudedesired;
	}
if ($multibeamsettings =~ /\S+\/\S+\/\S+/)
	{
	($mb_transmitgain,$mb_receivegain,$mb_minrangefraction) = $multibeamsettings =~ /(\S+)\/(\S+)\/(\S+)/;
	}
elsif ($multibeamsettings =~ /\S+\/\S+/)
	{
	($mb_transmitgain,$mb_receivegain) = $multibeamsettings =~ /(\S+)\/(\S+)/;
	$mb_minrangefraction = 0.2;
	}
if ($mappingsonar && $beamdata)
	{
	$mb_snippetmode = 0;
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
if (!$missionfile)
	{
	$missionfile = "$root.cfg";
	}
$waypointfile = "$root" . "_wpt.ste";

# Read in the initial route data
$cnt = 0;
while ($line = <RFILE>) 
	{
	if ($line =~ /^## ROUTENAME/)
		{
		($routename) = $line 
			=~ /^## ROUTENAME\s+(\S+)/;
		}
	elsif ($line =~ /^## /)
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
		$output = 0;
		$length = 0;
		push(@lons, $lon);
		push(@lats, $lat);
		push(@topos, $topo);
		push(@waypoints, $waypoint);
		push(@bearings, $bearing);
		push(@distances, $distance);
		push(@lengths, $length);
		push(@distonbottoms, $distonbottom);
		push(@slopes, $slope);
		push(@outputs, $output);

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
	
# calculate the spiral descent depth from first waypoint and desired altitude if needed
if ($spiraldescent)
	{
	$spiraldescentdepth = -$topos[0] - $spiraldescentaltitude - 10.0;
	}
	
# Process the route data to generate AUV waypoints that have
# the desired spacing.
$distancelastmpoint = 0.0;
$ilast = 0;
for ($i = 0; $i < $npoints; $i++)
	{
	# first figure out if this is a point to be output
	$outputs[$i] = 0;
	if ($waypoints[$i] != 0)
		{
		$outputs[$i] = 1;
		$lengths[$i] = $distances[$i] - $distancelastmpoint;
		
		if ($i > 0 && $waypoints[$ilast] == 0 
			&& ($distances[$i] - $distancelastmpoint) < 0.75 * $waypointdist)
			{
			$outputs[$ilast] = 0;
			$lengths[$i] += $lengths[$ilast];
			$lengths[$ilast] = 0.0;
			}
		}
	elsif (($distances[$i] - $distancelastmpoint) >= $waypointdist)
		{
		$outputs[$i] = 1;
		$lengths[$i] = $distances[$i] - $distancelastmpoint;
		}
		
	# process points of interest
	if ($outputs[$i] == 1)
		{

		# reset distance from last mission point
		$distancelastmpoint = $distances[$i];
		$ilast = $i;
		}
	}
	
# Write out the intermediate waypoint data
if ($debug)
	{
	print "\nWaypoint List:\n";
	printf "%d %10.2f %d %d\n",0,$distances[0],$waypoints[0],$outputs[0];
	$ilast = 0;
	for ($i = 1; $i < $npoints; $i++)
		{
		#print "$i $distances[$i] $waypoints[$i] $outputs[$i]\n";
		printf "%d %10.2f %d %d  %10.2f %10.2f %10.2f",$i,$distances[$i],$waypoints[$i],$outputs[$i],
		$distances[$i]-$distances[$i-1],$distances[$i]-$distances[$ilast],$lengths[$i];
		if ($outputs[$i] == 1)
			{
			$ilast = $i;
			print " *****\n";
			}
		else
			{
			print "\n";
			}
		}
	}	

# Process the route data to generate AUV waypoints that keep the 
# vehicle a safe distance above the bottom.
$nmissionpoints = 0;
$lastmode = 0;
for ($i = 0; $i < $npoints; $i++)
	{
	# process points of interest
	if ($outputs[$i] == 1)
		{
		# insert points into arrays so they can be printed out last to first
		# to follow the MBARI AUV mission file convention
		push(@mwaypoints, $waypoints[$i]);
		if ($waypoints[$i] == 3 || $waypoints[$i] == 5 || $waypoints[$i] == 7 || $waypoints[$i] == 9)
			{
			push(@mstartstops, 1);
			}
		elsif ($waypoints[$i] == 4 || $waypoints[$i] == 6 || $waypoints[$i] == 8 || $waypoints[$i] == 10)
			{
			push(@mstartstops, 2);
			}
		else
			{
			push(@mstartstops, 0);
			}
		if ($waypoints[$i] == 3 || $waypoints[$i] == 4)
			{
			$lastmode = 1;
			}
		elsif ($waypoints[$i] == 5 || $waypoints[$i] == 6)
			{
			$lastmode = 2;
			}
		elsif ($waypoints[$i] == 7 || $waypoints[$i] == 8)
			{
			$lastmode = 3;
			}
		elsif ($waypoints[$i] == 9 || $waypoints[$i] == 10)
			{
			$lastmode = 4;
			}
		push(@mmodes, $lastmode);
		
		push(@mlons, $lons[$i]);
		push(@mlats, $lats[$i]);
		push(@mtopos, $topos[$i]);
		push(@mbearings, $bearings[$i]);
		push(@mdistances, $distances[$i]);
		push(@mlengths, $lengths[$i]);
		$nmissionpoints++;
		
		# print it out
		if ($debug)
			{
			print "$i $waypoints[$i] $mstartstops[$i] $mmodes[$i] $lons[$i] $lats[$i] $topos[$i] $distances[$i] $bearings[$i]\r\n";
			}
#print "$nmissionpoints $waypoints[$i] $lons[$i] $lats[$i] $topos[$i] $distances[$i] $bearings[$i]\r\n";
		}
	}

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

# read depth profile data if specified
$ndppoints = 0;
if ($depthprofilefile)
	{
	# Open the input file
	open(RFILE,"$depthprofilefile") || die "Cannot open input depth profile file: $depthprofilefile\r\n$program_name aborted.\r\n";

	# Read in the depth profile data
	$line = <RFILE>;
	$line = <RFILE>;
	while ($line = <RFILE>) 
		{
		if ($line =~ /(\S+)\s+(\S+)\s+(\S+)\s+(\S+)/)
			{
			#extract route data
			($depth, $altitude, $lon, $lat, $distance) 
				= $line 
				=~ /(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)/;
			push(@dpdeps, $depth);
			push(@dpalts, $altitude);
			push(@dplons, $lon);
			push(@dplats, $lat);
			push(@dpdsts, $distance);
#printf "DEPTH PROFILE: $ndppoints $dplons[$ndppoints] $dplats[$ndppoints] $dpdeps[$ndppoints] $dpalts[$ndppoints] $dpdsts[$ndppoints]\n";

			# increment counter
			$ndppoints++;
			}
		}
	close(RFILE);
	}
	
# get safe depth for each mission segment
for ($i = 0; $i < $nmissionpoints; $i++)
	{
	# find shallowest point in look ahead distance
	$topomax = $mtopos[$i];
	$topomin = $mtopos[$i];
	if ($forwarddist)
		{
		for ($j = 0; $j < $npoints; $j++)
			{
			$ii = $i - 1;
			if ($ii < 0)
				{
				$ii = 0;
				}
			$ddist = $distances[$j] - $mdistances[$ii];
			if ($ddist > 0.0 
				&& $ddist <= $forwarddist)
				{
				if ($topos[$j] > $topomax)
					{
					$topomax = $topos[$j];
					}
				if ($topos[$j] < $topomin)
					{
					$topomin = $topos[$j];
					}
				}
			}
		}
	push(@mtopomaxs, $topomax);
	push(@mtopomins, $topomin);

	# if optimal depth profile is available get value
	if ($ndppoints > 0)
		{
		$optimaldepth = 0.0;
		for ($j = 0; $j < $ndppoints; $j++)
			{
			if ($dpdsts[$j] <= $mdistances[$i])
				{
				$optimaldepth = $dpdeps[$j];
				}
			}
		push(@moptimaldepths, $optimaldepth);
		}

	push(@mmissiondepths, $topomax);
	$ascendtime = -1.2 * $topomax / $ascendrate;
	$descendtime = -1.2 * $topomax / $descendrate;
	if ($ascendtime < 300)
		{
		$ascendtime = 300;
		}
	if ($descendtime < 300)
		{
		$descendtime = 300;
		}
	push(@ascendtimes, $ascendtime);
	push(@descendtimes, $descendtime);
	}

# Calculate time to first waypoint
if ($startlon && $startlat)
	{
	# calculate distance to first point
	$dx = ($mlons[0] - $startlon) / $mtodeglon;
	$dy = ($mlats[0] - $startlat) / $mtodeglat;
	$startdistance = sqrt($dx * $dx + $dy * $dy);
	}
elsif ($starttime)
	{
	$startdistance = $starttime * $ascentdescent_speed;
	}
else
	{
	$startdistance = 500.0;
	}
	
# calculate first cut depths and abort depth
$depthabort = 0.0;
for ($i = 0; $i < $nmissionpoints; $i++)
	{
	if ($ndppoints > 0)
		{
		$mmissiondepths[$i] = $moptimaldepths[$i];
#printf "DEPTHS optimal:%f regular:%f\n", $moptimaldepths[$i], -$mtopomaxs[$i] - $altitudedesired;
		}
	elsif ($mmodes[$i] == 1)
		{
		$mmissiondepths[$i] = -$mtopomaxs[$i] - $altitudedesired;
#printf "DEPTHS 1 %f\n", $mmissiondepths[$i];
		}
	elsif ($mmodes[$i] == 2)
		{
		$mmissiondepths[$i] = -$mtopomaxs[$i] - $altitudedesired2;
#printf "DEPTHS 2 %f\n", $mmissiondepths[$i];
		}
	elsif ($mmodes[$i] == 3)
		{
		$mmissiondepths[$i] = $depthconstant;
#printf "DEPTHS 3 %f\n", $mmissiondepths[$i];
		}
	elsif ($mmodes[$i] == 4)
		{
		$mmissiondepths[$i] = $depthconstant2;
#printf "DEPTHS 4 %f\n", $mmissiondepths[$i];
		}
	else
		{
		$mmissiondepths[$i] = -$mtopomaxs[$i] - $altitudedesired;
#printf "DEPTHS 0 %f\n", $mmissiondepths[$i];
		}
	if ($mmissiondepths[$i] < $descentdepth)
		{
		$mmissiondepths[$i] = $descentdepth;
		if (-$mtopomaxs[$i] - $mmissiondepths[$i] <= $altitudeabort)
			{
			die "Mission will fail due to altitude abort at mission point $i\r\n\tTopo:$mtopomaxs[$i]\r\nAbort Altitude:$altitudeabort\r\n";
			}
		}
	if ($mmissiondepths[$i] > $depthabort)
		{
		$depthabort = $mmissiondepths[$i];
		}
	}
$depthabort = $depthabort + $altitudedesired;
$depthmax = $depthabort - 0.5 * $altitudemin;
	
# calculate slopes
for ($i = $nmissionpoints-1; $i > 0; $i--)
	{
	if ($i == 0)
		{
		$distance = $startdistance;
		$slope = 0.0;
		}
	else
		{
		$distance = $mdistances[$i] - $mdistances[$i-1];
		$slope = -($mmissiondepths[$i] - $mmissiondepths[$i-1])/$distance;
		}
	if ($slope > $maxslope)
		{
print "EXCESSIVE SLOPE: $i $slope Change vehicle depth from $mmissiondepths[$i-1] to ";
		$mmissiondepths[$i-1] = $mmissiondepths[$i] + $maxslope * $distance;
print "$mmissiondepths[$i-1]\n";
		$slope = -($mmissiondepths[$i] - $mmissiondepths[$i-1])/$distance;
		}
	}

# Calculate estimated time of mission
$missiontime = 0.0;

# time from running lines
$missiontime += $distancelastmpoint / $survey_speed;
#printf "MISSIONTIME: distance:%f speed:%f time:%f  tottime:%f\n",
#$distancelastmpoint,$survey_speed,$distancelastmpoint / $survey_speed, $missiontime;

# add time to get to first waypoint
$missiontime += $startdistance / $ascentdescent_speed;
#printf "MISSIONTIME: first waypoint distance:%f speed:%f time:%f  tottime:%f\n",
#$startdistance,$ascentdescent_speed,$startdistance / $ascentdescent_speed, $missiontime;

# add time for initial gps and descent
#$missiontime += $gpsduration + $initialdescendtime;
#printf "MISSIONTIME: descent tinme:%f  tottime:%f\n",$initialdescendtime, $missiontime;

# add time for each ascent, gps, descent event
for ($i = 1; $i < $nmissionpoints - 1; $i++)
	{
	if (($gpsmode == 1 && ($mstartstops[$i] == 1)) 
			|| ($gpsmode == 2 && ($mstartstops[$i] == 2)) 
			|| ($gpsmode == 3 && $mstartstops[$i] > 0))
		{
		$missiontime += (-$mmissiondepths[$i] / $ascendrate) 
				+ (-$mmissiondepths[$i] / $descendrate) 
				+ $gpsduration + $initialdescendtime;
		}
	}

# add time for final ascent and gps
$missiontime += ($mmissiondepths[$nmissionpoints - 1] / $ascendrate) 
		+ $gpsduration;
#printf "MISSIONTIME: ascent time:%f  tottime:%f\n",
#($mmissiondepths[$nmissionpoints - 1] / $ascendrate)+ $gpsduration, $missiontime;
		
# if not specified calculate abort time using safety factor of $durationfactormission
if (!$aborttime)
	{
	$aborttime = $durationfactormission * $missiontime;
	}
	
# shorten abort time if unsafe
$ascendtime = $ascendtimes[$nmissionpoints - 1];
if ($aborttime > $batterylife - $safetymargin - $ascendtime)
	{
	$aborttime = $batterylife - $safetymargin - $ascendtime;
	}

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
	if ($depthprofilefile)
		{
		printf "    Depth Profile File:       $depthprofilefile\r\n";
		}
	printf "    Mission File:             $missionfile\r\n";
	printf "    Distance:                 %.0f (m)  %.3f (km)\r\n", $distancelastmpoint, 0.001 * $distancelastmpoint;
	printf "    Estimated Time:           %d (s)  %.3f (hr)\r\n", $missiontime, $missiontime / 3600.0;
	printf "    Abort Time:               %d (s)\r\n", $aborttime;
	printf "    Max battery life:         %d (s)\r\n", $batterylife;
	printf "    Safety margin:            %d (s)\r\n", $safetymargin;
	printf "    Ascend time:              %d (s)\r\n", $ascendtime;
	printf "    Way Points:               $nwaypoints\r\n";
	printf "    Route Points:             $nmissionpoints\r\n";
	if ($behavior == $behaviorWaypointDepthID)
		{
		printf "    Survey behavior:          WaypointDepth\r\n";
		}
	else
		{
		printf "    Survey behavior:          Waypoint\r\n";
		}
	if ($spiraldescent)
		{
		printf "    Descent style:            Spiral descent\r\n";
		}
	else
		{
		printf "    Descent style:            Waypoint descent\r\n";
		}
	if ($mappingsonar)
		{
		printf "    Mapping sonar control enabled:          \r\n";
		printf "                               Multibeam enabled\r\n";
		printf "                                 Multibeam receive gain:           $mb_receivegain\r\n";
		printf "                                 Multibeam transmit gain:          $mb_transmitgain\r\n";
		printf "                                 Multibeam minimum range fraction: $mb_minrangefraction\r\n";
		if ($beamdata)
			{
			printf "                                 Multibeam beam data collection enabled (100 m range)\r\n";
			}
		if ($subbottom)
			{
			printf "                               Subbottom enabled\r\n";
			}
		else
			{
			printf "                               Subbottom disabled\r\n";
			}
		if ($sidescanlo)
			{
			printf "                               Low sidescan enabled\r\n";
			}
		else
			{
			printf "                               Low sidescan disabled\r\n";
			}
		if ($sidescanhi)
			{
			printf "                               High sidescan enabled\r\n";
			}
		else
			{
			printf "                               High sidescan disabled\r\n";
			}
		}
	else
		{
		printf "    Mapping sonar control disabled:          \r\n";
		}
	if ($camera)
		{
		printf "    Benthic imaging camera control enabled:  \r\n";
		}
	else
		{
		printf "    Benthic imaging camera control disabled: \r\n";
		}
	printf "\r\n";
	printf "Mission Parameters:\r\n";
	printf "    Vehicle Survey Speed:       %f (m/s) %f (knots)\r\n", $survey_speed, 1.943846 * $survey_speed;
	printf "    Vehicle Ascent Speed:       %f (m/s) %f (knots)\r\n", $ascentdescent_speed, 1.943846 * $ascentdescent_speed;
	printf "    Vehicle Transit Speed:      %f (m/s) %f (knots)\r\n", $transit_speed, 1.943846 * $transit_speed;
	printf "    Desired Vehicle Altitude 1: $altitudedesired (m)\r\n";
	printf "    Minimum Vehicle Altitude 1: $altitudemin (m)\r\n";
	printf "    Abort Vehicle Altitude 1:   $altitudeabort (m)\r\n";
	printf "    Delta Depth Restart 1:      $deltadepthrestart (m)\r\n";
	printf "    Desired Vehicle Altitude 2: $altitudedesired2 (m)\r\n";
	printf "    Constant Vehicle Depth 1:   $depthconstant (m)\r\n";
	printf "    Constant Vehicle Depth 2:   $depthconstant2 (m)\r\n";
	printf "    Maximum Vehicle Depth:      $depthmax (m)\r\n";
	printf "    Abort Vehicle Depth:        $depthabort (m)\r\n";
	printf "    Descent Vehicle Depth:      $descentdepth (m)\r\n";
	if ($spiraldescent)
		{
		printf "    Spiral descent depth:     $spiraldescentdepth m\r\n";
		printf "    Spiral descent altitude:  $spiraldescentaltitude m\r\n";
		}
	if ($forwarddist)
		{
		printf "    Forward Looking Distance: $forwarddist (m)\r\n";
		}
	printf "    Waypoint Spacing:         $waypointdist (m)\r\n";
	printf "    Maximum upward slope:     %f degrees  %f ratio\r\n", $maxslopedeg, $maxslope;
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
	printf "    Descend Rate:             $descendrate (m/s)\r\n";
	printf "    Ascend Rate:              $ascendrate (m/s)\r\n";
	printf "    Initial descend Duration: %d (s)\r\n", $initialdescendtime;
	printf "    Setpoint Duration:        %d (s)\r\n", $setpointtime;
	printf "\r\n";
	printf "The primary waypoints and selected section points from the route file are:\r\n";
	printf "  <number> <longitude (deg)> <latitude (deg)> <topography (m)> <distance (m)> <length (m)> <type>\r\n";
	$cnt = 0;
	for ($i = 0; $i < $nmissionpoints; $i++)
 		{
 		printf "  %4d %11.6f %10.6f %8.3f %8.2f %8.2f %d\r\n",
 			$i,$mlons[$i],$mlats[$i],$mtopos[$i],$mdistances[$i],$mlengths[$i],$mwaypoints[$i];
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
	printf "    Max battery life:         %d (s)\r\n", $batterylife;
	printf "    Safety margin:            %d (s)\r\n", $safetymargin;
	printf "    Ascend time:              %d (s)\r\n", $ascendtime;
	printf "    Way Points:               $nwaypoints\r\n";
	printf "    Route Points:             $nmissionpoints\r\n";
	}

# output mission file unless outputoff option selected
if (!$outputoff)
	{
	# open the output file 
	open(MFILE,">$missionfile") || die "Cannot open output mission file: $missionfile\r\n$program_name aborted.\r\n";
	open(WFILE,">$waypointfile") || die "Cannot open output waypoint file: $waypointfile\r\n$program_name aborted.\r\n";

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
	printf MFILE "#     Max battery life:         %d (s)\r\n", $batterylife;
	printf MFILE "#     Safety margin:            %d (s)\r\n", $safetymargin;
	printf MFILE "#     Ascend time:              %d (s)\r\n", $ascendtime;
	printf MFILE "#     Way Points:               $nwaypoints\r\n";
	printf MFILE "#     Route Points:             $nmissionpoints\r\n";
	if ($behavior == $behaviorWaypointDepthID)
		{
		printf MFILE "#     Survey behavior:          WaypointDepth\r\n";
		}
	else
		{
		printf MFILE "#     Survey behavior:          Waypoint\r\n";
		}
	if ($spiraldescent)
		{
		printf MFILE "#     Descent style:            Spiral descent\r\n";
		}
	else
		{
		printf MFILE "#     Descent style:            Waypoint descent\r\n";
		}
	if ($mappingsonar)
		{
		printf MFILE "#     Mapping sonar control enabled:          \r\n";
		printf MFILE "#                               Multibeam enabled\r\n";
		printf MFILE "#                                 Multibeam receive gain:           $mb_receivegain\r\n";
		printf MFILE "#                                 Multibeam transmit gain:          $mb_transmitgain\r\n";
		printf MFILE "#                                 Multibeam minimum range fraction: $mb_minrangefraction\r\n";
		if ($beamdata)
			{
			printf MFILE "#                                 Multibeam beam data collection enabled (100 m range)\r\n";
			}
		if ($subbottom)
			{
			printf MFILE "#                               Subbottom enabled\r\n";
			}
		else
			{
			printf MFILE "#                               Subbottom disabled\r\n";
			}
		if ($sidescanlo)
			{
			printf MFILE "#                               Low sidescan enabled\r\n";
			}
		else
			{
			printf MFILE "#                               Low sidescan disabled\r\n";
			}
		if ($sidescanhi)
			{
			printf MFILE "#                               High sidescan enabled\r\n";
			}
		else
			{
			printf MFILE "#                               High sidescan disabled\r\n";
			}
		}
	else
		{
		printf MFILE "#     Mapping sonar control disabled:          \r\n";
		}
	if ($camera)
		{
		printf MFILE "#     Benthic imaging camera control enabled:  \r\n";
		}
	else
		{
		printf MFILE "#     Benthic imaging camera control disabled: \r\n";
		}
	printf MFILE "# \r\n";
	printf MFILE "# Mission Parameters:\r\n";
	printf MFILE "#     Vehicle Survey Speed:       %f (m/s) %f (knots)\r\n", $survey_speed, 1.943846 * $survey_speed;
	printf MFILE "#     Vehicle Ascent Speed:       %f (m/s) %f (knots)\r\n", $ascentdescent_speed, 1.943846 * $ascentdescent_speed;
	printf MFILE "#     Vehicle Transit Speed:      %f (m/s) %f (knots)\r\n", $transit_speed, 1.943846 * $transit_speed;
	printf MFILE "#     Desired Vehicle Altitude 1: $altitudedesired (m)\r\n";
	printf MFILE "#     Minimum Vehicle Altitude 1: $altitudemin (m)\r\n";
	printf MFILE "#     Abort Vehicle Altitude 1:   $altitudeabort (m)\r\n";
	printf MFILE "#     Delta Depth Restart 1:      $deltadepthrestart (m)\r\n";
	printf MFILE "#     Desired Vehicle Altitude 2: $altitudedesired2 (m)\r\n";
	printf MFILE "#     Constant Vehicle Depth 1:   $depthconstant (m)\r\n";
	printf MFILE "#     Constant Vehicle Depth 2:   $depthconstant2 (m)\r\n";
	printf MFILE "#     Maximum Vehicle Depth:      $depthmax (m)\r\n";
	printf MFILE "#     Abort Vehicle Depth:        $depthabort (m)\r\n";
	printf MFILE "#     Descent Vehicle Depth:      $descentdepth (m)\r\n";
	if ($spiraldescent)
		{
		printf MFILE "#     Spiral descent depth:       $spiraldescentdepth m\r\n";
		printf MFILE "#     Spiral descent altitude:    $spiraldescentaltitude m\r\n";
		}
	if ($forwarddist)
		{
		printf MFILE "#     Forward Looking Distance:   $forwarddist (m)\r\n";
		}
	printf MFILE "#     Waypoint Spacing:           $waypointdist (m)\r\n";
	if ($starttime)
		{
		printf MFILE "#     Time to First Waypoint:     %d (s)\r\n", $starttime;
		}
	if ($startposition)
		{
		printf MFILE "#     Start Longitude:            $startlon (deg)\r\n";
		printf MFILE "#     Start Latitude:             $startlat (deg)\r\n";
		}
	printf MFILE "#     GPS Duration:               %d (s)\r\n", $gpsduration;
	printf MFILE "#     Descend Rate:               $descendrate (m/s)\r\n";
	printf MFILE "#     Ascend Rate:                $ascendrate (m/s)\r\n";
	printf MFILE "#     Initial descend Duration:   %d (s)\r\n", $initialdescendtime;
	printf MFILE "#     Setpoint Duration:          %d (s)\r\n", $setpointtime;
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
	printf MFILE "#define SURVEY_SPEED               %f\r\n", $survey_speed;
	printf MFILE "#define ASCENTDESCENT_SPEED        %f\r\n", $ascentdescent_speed;
	printf MFILE "#define TRANSIT_SPEED              %f\r\n", $transit_speed;
	printf MFILE "#define MISSION_DISTANCE           %f\r\n", $distancelastmpoint;
	printf MFILE "#define MISSION_TIME               %d\r\n", $missiontime;
	printf MFILE "#define MISSION_TIMEOUT            %d\r\n", $aborttime;
	printf MFILE "#define DEPTH_MAX                  %f\r\n", $depthmax;
	printf MFILE "#define DEPTH_ABORT                %f\r\n", $depthabort;
	printf MFILE "#define ALTITUDE_DESIRED           %f\r\n", $altitudedesired;
	printf MFILE "#define ALTITUDE_MIN               %f\r\n", $altitudemin;
	printf MFILE "#define ALTITUDE_ABORT             %f\r\n", $altitudeabort;
	printf MFILE "#define DELTA_DEPTH_RESTART        %f\r\n", $deltadepthrestart;
	printf MFILE "#define ALTITUDE_DESIRED2          %f\r\n", $altitudedesired2;
	printf MFILE "#define DEPTH_CONSTANT             %f\r\n", $depthconstant;
	printf MFILE "#define DEPTH_CONSTANT2            %f\r\n", $depthconstant2;
	printf MFILE "#define GPS_DURATION               %d\r\n", $gpsduration;
	printf MFILE "#define DESCENT_DEPTH              %f\r\n", $descentdepth;
	printf MFILE "#define SPIRAL_DESCENT_DEPTH       %f\r\n", $spiraldescentdepth;
	printf MFILE "#define SPIRAL_DESCENT_ALTITUDE    %f\r\n", $spiraldescentaltitude;
	printf MFILE "#define DESCEND_DURATION           %d\r\n", $initialdescendtime;
	printf MFILE "#define SETPOINT_DURATION          %d\r\n", $setpointtime;
	printf MFILE "#define GPSMINHITS                 %d\r\n", $gpsminhits;
	printf MFILE "#define ASCENDRUDDER               %f\r\n", $ascendrudder;
	printf MFILE "#define ASCENDPITCH                %f\r\n", $ascendpitch;
	printf MFILE "#define ASCENDENDDEPTH             %f\r\n", $ascendenddepth;
	printf MFILE "#define DESCENDRUDDER              %f\r\n", $descendrudder;
	printf MFILE "#define DESCENDPITCH               %f\r\n", $descendpitch;
	printf MFILE "#define RESON_DURATION             %d\r\n", $resonduration;
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
	print MFILE "minDepth      = 0;\r\n";
	print MFILE "maxDepth      = DEPTH_MAX;\r\n";
	print MFILE "abortDepth    = DEPTH_ABORT;\r\n";
	print MFILE "minAltitude   = ALTITUDE_MIN;\r\n";
	print MFILE "abortAltitude = ALTITUDE_ABORT;\r\n";
	print MFILE "deltaDepthRestart = DELTA_DEPTH_RESTART;\r\n";
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
	print MFILE "abortOnTimeout = True; \r\n";
	print MFILE "} \r\n";
print "Behavior: gps\n";
	print MFILE "# \r\n";
	print MFILE "# ascend behavior \r\n";
	print MFILE "behavior ascend  \r\n";
	print MFILE "{ \r\n";
	$i = $nmissionpoints - 1;
	printf MFILE "duration  = %d; \r\n", $ascendtimes[$i];
	print MFILE "horizontalMode   = rudder; \r\n";
	print MFILE "horizontal       = ASCENDRUDDER; \r\n";
	print MFILE "pitch            = ASCENDPITCH; \r\n";
	print MFILE "speed            = ASCENTDESCENT_SPEED; \r\n";
	print MFILE "endDepth         = ASCENDENDDEPTH; \r\n";
	print MFILE "} \r\n";
print "Behavior: ascend\n";
	print MFILE "# \r\n";
	print MFILE "# Acoustic update 1 - sent status ping after end of survey \r\n";
	print MFILE "# \r\n";
	print MFILE "behavior acousticUpdate \r\n";
	print MFILE "{ \r\n";
	print MFILE "duration  = 2; \r\n";	
	print MFILE "dummy  = 1; \r\n";	
	print MFILE "} \r\n";
	print MFILE "# \r\n";
print "Behavior: acousticUpdate\n";
	if ($mappingsonar)
		{
		print MFILE "# Turn off power to sonars and stop logging on the PLC \r\n";
		print MFILE "# by setting the value of the mode attribute to 0 (used to be False). \r\n";
		print MFILE "behavior reson \r\n";
		print MFILE "{ \r\n";
		print MFILE "duration  = RESON_DURATION; \r\n";
		print MFILE "MB_Power = 0; \r\n";
		print MFILE "SBP_Mode  = 0; \r\n";
		print MFILE "LoSS_Mode = 0; \r\n";
		print MFILE "HiSS_Mode = 0; \r\n";
		print MFILE "Log_Mode  = 0; \r\n";
		print MFILE "} \r\n";
print "Behavior: reson (stop, Log_Mode = 0)\n";
		}
	if ($camera)
		{
		$cameraenddistance = $mdistances[$nmissionpoints-1];
		print MFILE "# Turn off camera imaging \r\n";
		print MFILE "#   Distance along survey: $mdistances[$nmissionpoints-1]\r\n";
		print MFILE "behavior stopCamera \r\n";
		print MFILE "{ \r\n";
		print MFILE "duration  = 1; \r\n";
		print MFILE "} \r\n";
printf "Behavior: stopCamera (distance:%.2f m\n",$mdistances[$nmissionpoints-1];
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

		# check depth
		$maxdepthapplied = 0;
		if ($mmissiondepths[$i] > $depthmax)
			{
			$mmissiondepths[$i] = $depthmax;
			$maxdepthapplied = 1;
			}
		elsif ($mmissiondepths[$i] < $descentdepth)
			{
			$mmissiondepths[$i] = $descentdepth;
			$maxdepthapplied = 0;
			}
			
		# get altitude for mapping sonar parameters
		$sonaraltitude = -$mmissiondepths[$i] - $mtopos[$i];
		if ($i < $nmissionpoints - 1
			&& (-$mmissiondepths[$i+1] - $mtopos[$i+1]) > $sonaraltitude)
			{
			$sonaraltitude = -$mmissiondepths[$i+1] - $mtopos[$i+1];
			}
		if ($sonaraltitude > $sonaraltitudemax)
			{
			$sonaraltitudeuse = $sonaraltitudemax;
			}
		elsif ($sonaraltitude < $altitudemin)
			{
			$sonaraltitudeuse = $altitudemin;
			}
		else
			{
			$sonaraltitudeuse = $sonaraltitude;
			}
		
		# sonar range allows for 150 degree swath on flat bottom
		# sonar range cut off at 100 m for recorded full beamformed data
		$mb_range = 3.5 * $sonaraltitudeuse;
		if ($mb_range > 200.0)
			{
			$mb_range = 200.0;
			}
		if ($beamdata && $mb_range > 100.0)
			{
			$mb_range = 100.0;
			}
		$mb_minrange = $mb_minrangefraction * $sonaraltitudeuse;
		$mb_maxrange = $mb_range;
		$mb_mindepth = 0.0;
		$mb_maxdepth = $mb_range;
		$sslo_range = 0.9 * 750.0 / $mb_pingrate;
		if ($sslo_range > $mb_range)
			{
			$sslo_range = $mb_range;
			}
		$sbp_duration = 1000.0 * 0.6 / $mb_pingrate;
		if ($sbp_duration > 200.0)
			{
			$sbp_duration = 200.0;
			}

		# do ascend, gps, descend at line starts and ends if specified
		if (($iwaypoint != $nwaypoints - 1) 
			&& (($gpsmode == 1 && ($mstartstops[$i] == 1)) 
				|| ($gpsmode == 2 && ($mstartstops[$i] == 2)) 
				|| ($gpsmode == 3 && ($mstartstops[$i] > 0))))
			{
			print MFILE "# \r\n";
			printf MFILE "# Ascend, gps, descend after reaching waypoint %d at end of line %d\r\n", $iwaypoint, $iwaypoint;
			if ($mappingsonar)
				{
				print MFILE "#######################################################\r\n";
				print MFILE "# Turn on power to sonars and restart logging on the PLC \r\n";
				print MFILE "behavior reson \r\n";
				print MFILE "{ \r\n";
				print MFILE "Log_Mode  = 1; \r\n";
				print MFILE "duration  = RESON_DURATION; \r\n";
				print MFILE "MB_Power = $mb_transmitgain; \r\n";
				if ($subbottom && $mstartstops[$i] == 2)
					{
					print MFILE "SBP_Mode = 0; \r\n";
					print MFILE "SBP_Power = 0.0; \r\n";
					}
				elsif ($subbottom && $mstartstops[$i] == 1)
					{
					print MFILE "SBP_Mode = 1; \r\n";
					print MFILE "SBP_Power = 100.0; \r\n";
					}
				if ($sidescanlo)
					{
					print MFILE "LoSS_Power = 100.0; \r\n";
					}
				if ($sidescanhi)
					{
					print MFILE "HiSS_Power = 100.0; \r\n";
					}
				print MFILE "} \r\n";
				print MFILE "#######################################################\r\n";
print "Behavior: reson (start, Log_Mode = 1)\n";
				}
			if ($camera)
				{
				if ($mstartstops[$i] == 1)
					{
					$camerarunlength = $cameraenddistance - $mdistances[$i];
					# $nphotos = int (1.1 * $camerarunlength / 10.0);
					$nphotos = 0;
					print MFILE "#######################################################\r\n";
					print MFILE "# Start taking pictures with camera.\r\n";
					print MFILE "#   Distance along survey: $mdistances[$i-1]\r\n";
					print MFILE "#   Length of camera run: $camerarunlength\r\n";
					print MFILE "#   Number of photos: $nphotos\r\n";
					print MFILE "behavior startCamera \r\n";
					print MFILE "{ \r\n";
					print MFILE "duration  = 1; \r\n";
					print MFILE "nPhotos = $nphotos; \r\n";
					print MFILE "nSamplePeriods = 9; \r\n";
					print MFILE "} \r\n";
					print MFILE "# \r\n";
					print MFILE "#######################################################\r\n";
printf "Behavior: startCamera (distance:%.2f m, run length:%.2f m, nphotos:%d)\n", 
		$mdistances[$i],$camerarunlength,$nphotos;
					}
				else
					{
					$cameraenddistance = $mdistances[$i];
					print MFILE "# Turn off camera imaging \r\n";
					print MFILE "#   Distance along survey: $mdistances[$i]\r\n";
					print MFILE "behavior stopCamera \r\n";
					print MFILE "{ \r\n";
					print MFILE "duration  = 1; \r\n";
					print MFILE "} \r\n";
printf "Behavior: stopCamera (distance:%.2f m\n",$mdistances[$i];
					}
				}
			print MFILE "# Acoustic update 2 - sent status ping before resuming logging \r\n";
			print MFILE "# \r\n";
			print MFILE "behavior acousticUpdate \r\n";
			print MFILE "{ \r\n";
			print MFILE "duration  = 2; \r\n";	
			print MFILE "dummy  = 1; \r\n";	
			print MFILE "} \r\n";
			print MFILE "# \r\n";
print "Behavior: acousticUpdate\n";
			print MFILE "# \r\n";
			print MFILE "# Descend behavior \r\n";
			print MFILE "behavior descend  \r\n";
			print MFILE "{ \r\n";
			print MFILE "horizontalMode = heading; \r\n";
			printf MFILE "horizontal      = %f; \r\n", $mbearings[$i];
			print MFILE "pitch           = DESCENDPITCH; \r\n";
			print MFILE "speed           = ASCENTDESCENT_SPEED; \r\n";
			print MFILE "maxDepth        = DESCENT_DEPTH; \r\n";
			print MFILE "minAltitude     = ALTITUDE_MIN; \r\n";
			print MFILE "duration        = DESCEND_DURATION; \r\n";
			print MFILE "} \r\n";
print "Behavior: descend\n";
			print MFILE "# \r\n";
			print MFILE "# setpoint on surface to gather momentum \r\n";
			print MFILE "behavior setpoint \r\n";
			print MFILE "{ \r\n";
			print MFILE "duration        = SETPOINT_DURATION; \r\n";
			printf MFILE "heading         = %f; \r\n", $mbearings[$i];
			print MFILE "speed           = ASCENTDESCENT_SPEED; \r\n";
			print MFILE "verticalMode    = pitch; \r\n";
			print MFILE "pitch           = 0; \r\n";
			print MFILE "} \r\n";
print "Behavior: setpoint\n";
			print MFILE "# \r\n";
			print MFILE "# acquire gps fix \r\n";
			print MFILE "behavior getgps  \r\n";
			print MFILE "{ \r\n";
			print MFILE "duration = GPS_DURATION; \r\n";
			print MFILE "minHits = GPSMINHITS; \r\n";
			print MFILE "abortOnTimeout = True; \r\n";
			print MFILE "} \r\n";
print "Behavior: gps\n";
			print MFILE "# \r\n";
			print MFILE "# ascend behavior \r\n";
			print MFILE "behavior ascend  \r\n";
			print MFILE "{ \r\n";
			printf MFILE "duration  = %d; \r\n", $ascendtimes[$i];
			print MFILE "horizontalMode   = rudder; \r\n";
			print MFILE "horizontal       = ASCENDRUDDER; \r\n";
			print MFILE "pitch            = ASCENDPITCH; \r\n";
			print MFILE "speed            = ASCENTDESCENT_SPEED; \r\n";
			print MFILE "endDepth         = ASCENDENDDEPTH; \r\n";
			print MFILE "} \r\n";
print "Behavior: ascend\n";
			print MFILE "# Acoustic update 3 - sent status ping before ascent \r\n";
			print MFILE "# \r\n";
			print MFILE "behavior acousticUpdate \r\n";
			print MFILE "{ \r\n";
			print MFILE "duration  = 2; \r\n";	
			print MFILE "dummy  = 1; \r\n";	
			print MFILE "} \r\n";
			print MFILE "# \r\n";
print "Behavior: acousticUpdate\n";

			print MFILE "# \r\n";
			if ($mappingsonar)
				{
				print MFILE "#######################################################\r\n";
				print MFILE "# Turn off power to sonars and stop logging on the PLC \r\n";
				print MFILE "# by setting the value of the mode attribute to 0 (used to be False). \r\n";
				print MFILE "behavior reson \r\n";
				print MFILE "{ \r\n";
				print MFILE "duration  = RESON_DURATION; \r\n";
				print MFILE "Log_Mode  = 0; \r\n";
				print MFILE "MB_Power = 0.0; \r\n";
				if ($subbottom)
					{
					print MFILE "SBP_Power = 0.0; \r\n";
					}
				if ($sidescanlo)
					{
					print MFILE "LoSS_Power = 0.0; \r\n";
					}
				if ($sidescanhi)
					{
					print MFILE "HiSS_Power = 0.0; \r\n";
					}
				print MFILE "} \r\n";
				print MFILE "#######################################################\r\n";
print "Behavior: reson (stop, Log_Mode = 0)\n";
				}
			if ($camera)
				{
				$cameraenddistance = $mdistances[$i];
				print MFILE "# Turn off camera imaging \r\n";
				print MFILE "#   Distance along survey: $mdistances[$i]\r\n";
				print MFILE "behavior stopCamera \r\n";
				print MFILE "{ \r\n";
				print MFILE "duration  = 1; \r\n";
				print MFILE "} \r\n";
printf "Behavior: stopCamera (distance:%.2f m\n",$mdistances[$i];
				}
			}

		# reset sonar parameters and start logging
		print MFILE "# \r\n";
if ($mappingsonar)
{
print "mappingsonar:$mappingsonar i:$i mwaypoints[$i]:$mwaypoints[$i] iwaypoint:$iwaypoint\n";
}
		if ($mappingsonar && $mwaypoints[$i] != 0 && $iwaypoint == 0)
			{
			print MFILE "#######################################################\r\n";
			print MFILE "# Turn Mapping sonars on and start logging.\r\n";
			print MFILE "# \r\n";
			print MFILE "behavior reson \r\n";
			print MFILE "{ \r\n";
			print MFILE "duration  = RESON_DURATION; \r\n";
			print MFILE "Log_Mode  = 1; \r\n";
			print MFILE "} \r\n";
print "Behavior: reson (startup, Log_Mode = 1)\n";
			print MFILE "# Set sonar parameters \r\n";
			print MFILE "#   Waypoint type:            $mwaypoints[$i]\n";
			print MFILE "#   Commanded altitude:       $sonaraltitude\n";
			print MFILE "#   Sonar parameter altitude: $sonaraltitudeuse\n";
			print MFILE "#   Commanded vehicle depth:  $mmissiondepths[$i] \n";
			print MFILE "#   Seafloor depth:           $mtopos[$i]\n";
			print MFILE "# \r\n";
			print MFILE "behavior reson \r\n";
			print MFILE "{ \r\n";
			print MFILE "duration  = RESON_DURATION; \r\n";	
			if ($subbottom && $mstartstops[$i] == 2)
				{
				print MFILE "SBP_Mode = 0; \r\n";
				print MFILE "SBP_Power = 0.0; \r\n";
print "Behavior: reson (reset, Log_Mode = 0, line  = $iwaypoint, waypoint($i) type = $mwaypoints[$i], SBP off, MBrange:$mb_range MBaltitude:$sonaraltitudeuse)\n";
				}
			elsif ($subbottom && $mstartstops[$i] == 1)
				{
				print MFILE "SBP_Mode = 1; \r\n";
				print MFILE "SBP_Power = 100.0; \r\n";
print "Behavior: reson (reset, Log_Mode = 0, line  = $iwaypoint, waypoint($i) type = $mwaypoints[$i], SBP on, MBrange:$mb_range MBaltitude:$sonaraltitudeuse)\n";
				}
			if ($subbottom)
				{
				print MFILE "SBP_Gain = 128.0; \r\n";
				printf MFILE "SBP_Duration = %.3f; \r\n", $sbp_duration;
				}
			if ($sidescanlo)
				{
				print MFILE "LoSS_Mode = 1; \r\n";
				print MFILE "LoSS_Power = 100.0; \r\n";
				printf MFILE "LoSS_Range = %.2f; \r\n", $sslo_range;
				}
			if ($sidescanhi)
				{
				print MFILE "HiSS_Mode = 1; \r\n";
				print MFILE "HiSS_Power = 100.0; \r\n";
				print MFILE "HiSS_Range = $sslo_range; \r\n";
				}
			print MFILE "MB_Power = $mb_transmitgain; \r\n";
			printf MFILE "MB_Range = %.2f; \r\n", $mb_range;
			print MFILE "MB_Rate = $mb_pingrate; \r\n";
			print MFILE "MB_Gain = $mb_receivegain; \r\n";
			print MFILE "MB_Sound_Velocity = 0.0; \r\n";
			printf MFILE "MB_Pulse_Width = %f; \r\n", $mb_pulsewidth;
			printf MFILE "MB_Bottom_Detect_Filter_Min_Range = %.2f; \r\n", $mb_minrange;
			printf MFILE "MB_Bottom_Detect_Filter_Max_Range = %.2f; \r\n", $mb_maxrange;
			printf MFILE "MB_Bottom_Detect_Filter_Min_Depth = %.2f; \r\n", $mb_mindepth;
			printf MFILE "MB_Bottom_Detect_Filter_Max_Depth = %.2f; \r\n", $mb_maxdepth;
			print MFILE "MB_Bottom_Detect_Range_Mode = 1; \r\n";
			print MFILE "MB_Bottom_Detect_Depth_Mode = 0; \r\n";
			print MFILE "Snippet_Mode = $mb_snippetmode; \r\n";
			print MFILE "Window_Size = 200; \r\n";
			print MFILE "Log_Mode = 0; \r\n";
			print MFILE "} \r\n";
			print MFILE "# \r\n";
#			print MFILE "# Acoustic update - sent status ping before start of logging \r\n";
#			print MFILE "# \r\n";
#			print MFILE "behavior acousticUpdate \r\n";
#			print MFILE "{ \r\n";
#			print MFILE "duration  = 2; \r\n";	
#			print MFILE "dummy  = 1; \r\n";	
#			print MFILE "} \r\n";
#			print MFILE "# \r\n";
#print "Behavior: acousticUpdate\n";
			print MFILE "#######################################################\r\n";
			}

		# special case of spiral descent at first waypoint
		if ($spiraldescent && $i == 0)
			{
			print MFILE "#######################################################\r\n";
			print MFILE "# \r\n";
			print MFILE "# \r\n";
			print MFILE "# Waypoint behavior to get back to start of line 1\r\n";
			printf MFILE "#   Segment length %f meters\r\n", $distance;
			printf MFILE "#   Minimum depth: %f meters looking forward %f meters along route\r\n", -$mtopomaxs[$i], $forwarddist;
			printf MFILE "#   Maximum depth: %f meters looking forward %f meters along route\r\n", -$mtopomins[$i], $forwarddist;
			printf MFILE "#   Vehicle depth: %f meters\r\n", $mmissiondepths[$i];
			print MFILE "behavior waypoint  \r\n";
			print MFILE "{ \r\n";
			printf MFILE "latitude     = %f; \r\n", $mlats[$i];
			printf MFILE "longitude    = %f; \r\n", $mlons[$i];
			printf MFILE "duration     = %d; \r\n", ($durationfactorwaypoint * (0.5 * $mmissiondepths[$i]) / $ascentdescent_speed);
			printf MFILE "depth        = %f; \r\n", $mmissiondepths[$i];
			print MFILE "speed        = ASCENTDESCENT_SPEED; \r\n";
			print MFILE "} \r\n";
print "Behavior: waypoint\n";
			print MFILE "# \r\n";
			print MFILE "# Zero speed hang to allow final nav updates over acoustic modem\r\n";
			print MFILE "# - must get start survey command over acoustic modem or mission aborts\r\n";
			print MFILE "behavior StartSurvey  \r\n";
			print MFILE "{ \r\n";
			print MFILE "duration     = 300; \r\n";
			print MFILE "} \r\n";
print "Behavior: startsurvey\n";
			print MFILE "# \r\n";
			print MFILE "# Acoustic update 4 - sent status ping at beginning of StartSurvey behavior \r\n";
			print MFILE "# \r\n";
			print MFILE "behavior acousticUpdate \r\n";
			print MFILE "{ \r\n";
			print MFILE "duration  = 2; \r\n";	
			print MFILE "dummy  = 1; \r\n";	
			print MFILE "} \r\n";
print "Behavior: acousticUpdate\n";
			print MFILE "# \r\n";
			print MFILE "# Spiral descend behavior to get to proper depth at start of line 1 \r\n";
			if ($maxdepthapplied == 0)
				{
				printf MFILE "#   Behavior depth of %f meters set by local depth and desired altitude\r\n", $mmissiondepths[$i];
				}
			else
				{
				printf MFILE "#   Behavior depth of %f meters set to maximum vehicle depth\r\n", $mmissiondepths[$i];
				}
			print MFILE "behavior descend  \r\n";
			print MFILE "{ \r\n";
			printf MFILE "duration        = %d; \r\n", $descendtimes[$i];
			print MFILE "horizontalMode   = rudder; \r\n";
			print MFILE "horizontal       = DESCENDRUDDER; \r\n";
			print MFILE "pitch            = DESCENDPITCH; \r\n";
			print MFILE "speed            = ASCENTDESCENT_SPEED; \r\n";
			print MFILE "maxDepth         = SPIRAL_DESCENT_DEPTH; \r\n";
			print MFILE "minAltitude      = SPIRAL_DESCENT_ALTITUDE; \r\n";
			print MFILE "} \r\n";
			print MFILE "# \r\n";
print "Behavior: spiral descend\n";
			print MFILE "# Acoustic update 5 - sent status ping after end of line \r\n";
			print MFILE "# \r\n";
			print MFILE "behavior acousticUpdate \r\n";
			print MFILE "{ \r\n";
			print MFILE "duration  = 2; \r\n";	
			print MFILE "dummy  = 1; \r\n";	
			print MFILE "} \r\n";
print "Behavior: acousticUpdate\n";
			print MFILE "# \r\n";
			print MFILE "# Waypoint behavior to get to start of line 1\r\n";
			printf MFILE "#   Segment length %f meters\r\n", $distance;
			printf MFILE "#   Minimum depth: %f meters looking forward %f meters along route\r\n", -$mtopomaxs[$i], $forwarddist;
			printf MFILE "#   Maximum depth: %f meters looking forward %f meters along route\r\n", -$mtopomins[$i], $forwarddist;
			printf MFILE "#   Vehicle depth: %f meters\r\n", $approachdepth;
			print MFILE "behavior waypoint  \r\n";
			print MFILE "{ \r\n";
			printf MFILE "latitude     = %f; \r\n", $mlats[$i];
			printf MFILE "longitude    = %f; \r\n", $mlons[$i];
			printf MFILE "duration     = %d; \r\n", ($durationfactorwaypoint * 500 / $ascentdescent_speed);
			printf MFILE "depth        = %f; \r\n", $approachdepth;
			print MFILE "speed        = ASCENTDESCENT_SPEED; \r\n";
			print MFILE "} \r\n";
print "Behavior: waypoint\n";
			print WFILE "$mlons[$i] $mlats[$i]\n";
			}
			
		# output $behaviorWaypointID mission point behavior
		elsif ($behavior == $behaviorWaypointID)
			{
			print MFILE "# \r\n";
			if ($mwaypoints[$i] != 0 && $iwaypoint == 0)
				{
				print MFILE "# Waypoint behavior to get to start of line 1\r\n";
print "Behavior: waypoint (to start line 1) ";
				}
			elsif ($mwaypoints[$i] != 0)
				{
				printf MFILE "# Waypoint behavior to get to end of line %d\r\n", $iwaypoint;
print "Behavior: waypoint (to end line $iwaypoint) ";
				}
			else
				{
				printf MFILE "# Waypoint behavior during line %d\r\n", $iwaypoint;
print "Behavior: waypoint (during line $iwaypoint) ";
				}
			printf MFILE "#   Segment length %f meters\r\n", $mlengths[$i];
			printf MFILE "#   Minimum depth: %f meters looking forward %f meters along route\r\n", -$mtopomaxs[$i], $forwarddist;
			printf MFILE "#   Maximum depth: %f meters looking forward %f meters along route\r\n", -$mtopomins[$i], $forwarddist;
			printf MFILE "#   Maximum vehicle depth: %f meters\r\n", $depthmax;
			printf MFILE "#   Desired vehicle altitude: %f meters\r\n", $altitudedesired;
			printf MFILE "#   Minimum vehicle altitude: %f meters\r\n", $altitudemin;
			if ($maxdepthapplied == 0)
				{
				printf MFILE "#   Behavior depth of %f meters set by local depth and desired altitude\r\n", $mmissiondepths[$i];
				}
			else
				{
				printf MFILE "#   Behavior depth of %f meters set to maximum vehicle depth\r\n", $mmissiondepths[$i];
				}
			if ($i > 0 && $mstartstops[$i-1] == 2)
				{
				printf MFILE "#   Vehicle transit speed:    %f m/s\r\n", $transit_speed;
				}
			else
				{
				printf MFILE "#   Vehicle survey speed:     %f m/s\r\n", $survey_speed;
				}
			print MFILE "behavior waypoint  \r\n";
			print MFILE "{ \r\n";
			printf MFILE "latitude     = %f; \r\n", $mlats[$i];
			printf MFILE "longitude    = %f; \r\n", $mlons[$i];
			if ($mwaypoints[$i] != 0 && $iwaypoint == 0)
				{
				printf MFILE "duration           = %d; \r\n", 600;
				}
			else
				{
				printf MFILE "duration           = %d; \r\n", ($durationfactorwaypoint * $mlengths[$i] / $survey_speed);
				}
			if ($mwaypoints[$i] != 0 && $iwaypoint != 0)
				{
				printf MFILE "# abortOnTimeout    = True; \r\n";
				}
			printf MFILE "depth        = %f; \r\n", $mmissiondepths[$i];
			if ($i > 0 && $mmodels[$i-1] == 2)
				{
				print MFILE "speed              = TRANSIT_SPEED; \r\n";
				}
			else
				{
				print MFILE "speed              = SURVEY_SPEED; \r\n";
				}
			print MFILE "} \r\n";
print " Depth: $mmissiondepths[$i]\n";
			print WFILE "$mlons[$i] $mlats[$i]\n";
			}
			

		# output $behaviorWaypointDepthID mission point behavior
		elsif ($behavior == $behaviorWaypointDepthID)
			{
			print MFILE "# \r\n";
			if ($mwaypoints[$i] != 0 && $iwaypoint == 0)
				{
				print MFILE "# Waypoint_depth behavior to get to start of line 1\r\n";
print "Behavior: waypoint_depth (to start line 1) ";
				}
			elsif ($mwaypoints[$i] != 0)
				{
				printf MFILE "# Waypoint_depth behavior to get to end of line %d\r\n", $iwaypoint;
print "Behavior: waypoint_depth (to end line $iwaypoint) ";
				}
			else
				{
				printf MFILE "# Waypoint_depth behavior during line %d\r\n", $iwaypoint;
print "Behavior: waypoint_depth (during line $iwaypoint) ";
				}
printf " Segment length: %.2f m ",$mlengths[$i];
			printf MFILE "#   Segment length %f meters\r\n", $mlengths[$i];
			printf MFILE "#   Minimum depth: %f meters looking forward %f meters along route\r\n", -$mtopomaxs[$i], $forwarddist;
			printf MFILE "#   Maximum depth: %f meters looking forward %f meters along route\r\n", -$mtopomins[$i], $forwarddist;
			printf MFILE "#   Maximum vehicle depth: %f meters\r\n", $depthmax;
			printf MFILE "#   Desired vehicle altitude: %f meters\r\n", $altitudedesired;
			printf MFILE "#   Minimum vehicle altitude: %f meters\r\n", $altitudemin;
			if ($maxdepthapplied == 0)
				{
				printf MFILE "#   Behavior depth of %f meters set by local depth and desired altitude\r\n", $mmissiondepths[$i];
				}
			else
				{
				printf MFILE "#   Behavior depth of %f meters set to maximum vehicle depth\r\n", $mmissiondepths[$i];
				}
			if ($i > 0 && $mstartstops[$i-1] == 2)
				{
				printf MFILE "#   Vehicle transit speed:    %f m/s\r\n", $transit_speed;
				}
			else
				{
				printf MFILE "#   Vehicle survey speed:     %f m/s\r\n", $survey_speed;
				}
			print MFILE "behavior waypoint_depth  \r\n";
			print MFILE "{ \r\n";
			printf MFILE "latitude           = %f; \r\n", $mlats[$i];
			printf MFILE "longitude          = %f; \r\n", $mlons[$i];
			if ($mwaypoints[$i] != 0 && $iwaypoint == 0)
				{
				printf MFILE "duration           = %d; \r\n", 600;
				}
			else
				{
				printf MFILE "duration           = %d; \r\n", ($durationfactorwaypoint * $mlengths[$i] / $survey_speed);
				}
			if ($mwaypoints[$i] != 0 && $iwaypoint != 0)
				{
				printf MFILE "# abortOnTimeout    = True; \r\n";
				}
print " Depths: ";
			if ($i > 0)
				{
				printf MFILE "initialDepth       = %f; \r\n", $mmissiondepths[$i-1];
printf " %.2f",$mmissiondepths[$i-1];
				}
			else
				{
				printf MFILE "initialDepth       = %f; \r\n", $mmissiondepths[$i];
printf " %.2f",$mmissiondepths[$i];
				}
			printf MFILE "finalDepth         = %f; \r\n", $mmissiondepths[$i];
printf " %.2f m\n",$mmissiondepths[$i];
			if ($i > 0 && $mstartstops[$i-1] == 2)
				{
				print MFILE "speed              = TRANSIT_SPEED; \r\n";
				}
			else
				{
				print MFILE "speed              = SURVEY_SPEED; \r\n";
				}
			print MFILE "} \r\n";
			print WFILE "$mlons[$i] $mlats[$i]\n";
			}
			
		# insert acoustic update after end of line
		if ($mstartstops[$i-1] == 2)
			{
			print MFILE "#######################################################\r\n";
			print MFILE "# Acoustic update 6 - sent status ping after end of line \r\n";
			print MFILE "# \r\n";
			print MFILE "behavior acousticUpdate \r\n";
			print MFILE "{ \r\n";
			print MFILE "duration  = 2; \r\n";	
			print MFILE "dummy  = 1; \r\n";	
			print MFILE "} \r\n";
			print MFILE "# \r\n";
print "Behavior: acousticUpdate\n";
			}

		# reset sonar parameters
		if ($mappingsonar && $iwaypoint > 0)
			{
			print MFILE "#######################################################\r\n";
			print MFILE "# Reset sonar parameters \r\n";
			print MFILE "#   Waypoint type:            $mwaypoints[$i-1]\n";
			print MFILE "#   Commanded altitude:       $sonaraltitude\n";
			print MFILE "#   Actual altitude:          $sonaraltitudeuse\n";
			print MFILE "#   Commanded vehicle depth:  $mmissiondepths[$i] \n";
			print MFILE "#   Seafloor depth:           $mtopos[$i]\n";
			print MFILE "# \r\n";
			print MFILE "behavior reson \r\n";
			print MFILE "{ \r\n";
			print MFILE "duration  = RESON_DURATION; \r\n";	
			if ($subbottom && $mstartstops[$i-1] == 2)
				{
				print MFILE "SBP_Mode = 0; \r\n";
				print MFILE "SBP_Power = 0.0; \r\n";
print "Behavior: reson (reset, Log_Mode = 1, line  = $iwaypoint, waypoint($i-1) type = $mwaypoints[$i-1] mstartstops = $mstartstops[$i-1], SBP off, MBrange:$mb_range MBaltitude:$sonaraltitudeuse)\n";
				}
			elsif ($subbottom && $mstartstops[$i-1] == 1)
				{
				print MFILE "SBP_Mode = 1; \r\n";
				print MFILE "SBP_Power = 100.0; \r\n";
print "Behavior: reson (reset, Log_Mode = 1, line  = $iwaypoint, waypoint($i-1) type = $mwaypoints[$i-1] mstartstops = $mstartstops[$i-1], SBP on, MBrange:$mb_range MBaltitude:$sonaraltitudeuse)\n";
				}
			elsif ($subbottom)
				{
print "Behavior: reson (reset, Log_Mode = 1, line  = $iwaypoint, waypoint($i-1) type = $mwaypoints[$i-1] mstartstops = $mstartstops[$i-1], SBP no change, MBrange:$mb_range MBaltitude:$sonaraltitudeuse)\n";
				}
			if ($subbottom)
				{
				printf MFILE "SBP_Duration = %.3f; \r\n", $sbp_duration;
				}
			if ($sidescanlo)
				{
				printf MFILE "LoSS_Range = %.2f; \r\n", $sslo_range;
				}
			if ($sidescanhi)
				{
				print MFILE "HiSS_Range = $sslo_range; \r\n";
				}
			printf MFILE "MB_Range = %.2f; \r\n", $mb_range;
			print MFILE "MB_Rate = $mb_pingrate; \r\n";

# hack to provide a gain sweep for testing	
# $mb_receivegain = 83 - int($resongainsetcount / 4);
# if ($mb_receivegain < 40)
# {
# $mb_receivegain = 60;
# }
# $resongainsetcount++;
			
			print MFILE "MB_Gain = $mb_receivegain; \r\n";
			print MFILE "MB_Sound_Velocity = 0.0; \r\n";
			printf MFILE "MB_Pulse_Width = %f; \r\n", $mb_pulsewidth;
			printf MFILE "MB_Bottom_Detect_Filter_Min_Range = %.2f; \r\n", $mb_minrange;
			printf MFILE "MB_Bottom_Detect_Filter_Max_Range = %.2f; \r\n", $mb_maxrange;
			printf MFILE "MB_Bottom_Detect_Filter_Min_Depth = %.2f; \r\n", $mb_mindepth;
			printf MFILE "MB_Bottom_Detect_Filter_Max_Depth = %.2f; \r\n", $mb_maxdepth;
			print MFILE "MB_Bottom_Detect_Range_Mode = 1; \r\n";
			print MFILE "MB_Bottom_Detect_Depth_Mode = 0; \r\n";
			print MFILE "} \r\n";
			print MFILE "# \r\n";
			print MFILE "#######################################################\r\n";
			}

		# turn on or off camera
		print MFILE "# \r\n";
		if ($camera && $i > 0 && $mstartstops[$i-1] == 1)
			{
			$camerarunlength = $cameraenddistance - $mdistances[$i-1];
			$nphotos = int (1.1 * $camerarunlength / 10.0);
			# $nphotos = 0;
			print MFILE "#######################################################\r\n";
			print MFILE "# Start taking pictures with camera.\r\n";
			print MFILE "#   Distance along survey: $mdistances[$i-1]\r\n";
			print MFILE "#   Length of camera run: $camerarunlength\r\n";
			print MFILE "#   Number of photos: $nphotos\r\n";
			print MFILE "behavior startCamera \r\n";
			print MFILE "{ \r\n";
			print MFILE "duration  = 1; \r\n";
			#print MFILE "nPhotos = $nphotos; \r\n";
			print MFILE "nSamplePeriods = 9; \r\n";
			print MFILE "} \r\n";
			print MFILE "# \r\n";
			print MFILE "#######################################################\r\n";
printf "Behavior: startCamera (distance:%.2f m, run length:%.2f m, nphotos:%d)\n", 
		$mdistances[$i-1],$camerarunlength,$nphotos;
			}
		elsif ($camera && $i > 0 && $mstartstops[$i-1] == 2)
			{
			$cameraenddistance = $mdistances[$i-1];
			print MFILE "#######################################################\r\n";
			print MFILE "# Stop taking pictures with camera.\r\n";
			print MFILE "#   Distance along survey: $mdistances[$i-1]\r\n";
			print MFILE "behavior stopCamera \r\n";
			print MFILE "{ \r\n";
			print MFILE "duration  = 1; \r\n";
			print MFILE "} \r\n";
			print MFILE "# \r\n";
			print MFILE "#######################################################\r\n";
printf "Behavior: stopCamera (distance:%.2f m\n",$mdistances[$i-1];
			}
			
		# insert acoustic update before start of line
		if ($mstartstops[$i-1] == 1)
			{
			print MFILE "# Acoustic update 7 - sent status ping before start of line \r\n";
			print MFILE "# \r\n";
			print MFILE "behavior acousticUpdate \r\n";
			print MFILE "{ \r\n";
			print MFILE "duration  = 2; \r\n";	
			print MFILE "dummy  = 1; \r\n";	
			print MFILE "} \r\n";
			print MFILE "# \r\n";
			print MFILE "#######################################################\r\n";
print "Behavior: acousticUpdate\n";
			}

		# put comment break in at waypoint
		if ($mwaypoints[$i] != 0)
			{
			print MFILE "#######################################################\r\n";
			}
		}

	# output beginning of mission
	print MFILE "#######################################################\r\n";
	print MFILE "# \r\n";
#	print MFILE "# \r\n";
#	print MFILE "# Acoustic update - sent status ping before start of line \r\n";
#	print MFILE "# \r\n";
#	print MFILE "behavior acousticUpdate \r\n";
#	print MFILE "{ \r\n";
#	print MFILE "duration  = 2; \r\n";	
#	print MFILE "dummy  = 1; \r\n";	
#	print MFILE "} \r\n";
#print "Behavior: acousticUpdate\n";
	print MFILE "# \r\n";
	print MFILE "# Descend behavior \r\n";
	print MFILE "behavior descend  \r\n";
	print MFILE "{ \r\n";
	print MFILE "horizontalMode   = heading; \r\n";
	printf MFILE "horizontal   = %f; \r\n", $mbearings[0];
	print MFILE "pitch        = DESCENDPITCH; \r\n";
	print MFILE "speed        = ASCENTDESCENT_SPEED; \r\n";
	print MFILE "maxDepth     = DESCENT_DEPTH; \r\n";
	print MFILE "minAltitude  = ALTITUDE_MIN; \r\n";
	print MFILE "duration     = DESCEND_DURATION; \r\n";
	print MFILE "} \r\n";
print "Behavior: descend\n";
	print MFILE "# \r\n";
	print MFILE "# setpoint on surface to gather momentum \r\n";
	print MFILE "behavior setpoint \r\n";
	print MFILE "{ \r\n";
	print MFILE "duration     = SETPOINT_DURATION; \r\n";
	printf MFILE "heading      = %f; \r\n", $mbearings[0];
	print MFILE "speed        = ASCENTDESCENT_SPEED; \r\n";
	print MFILE "verticalMode = pitch; \r\n";
	print MFILE "pitch        = 0; \r\n";
	print MFILE "} \r\n";
print "Behavior: setpoint\n";
	print MFILE "# \r\n";
	print MFILE "# acquire gps fix \r\n";
	print MFILE "behavior getgps  \r\n";
	print MFILE "{ \r\n";
	print MFILE "duration = GPS_DURATION; \r\n";
	print MFILE "minHits = GPSMINHITS; \r\n";
	print MFILE "abortOnTimeout = True; \r\n";
	print MFILE "} \r\n";
print "Behavior: gps\n";

	# output starting sonar parameters
	if ($mappingsonar)
		{
		# get starting sonar parameters assuming $altitudedesired altitude
		#  $sslo_range & $sbp_duration calculated near start of program
		# from ping rate
		$sonaraltitudeuse = $altitudedesired;
		$mb_range = 4.0 * $sonaraltitudeuse;
		if ($mb_range > 350.0)
			{
			$mb_range = 350.0;
			}
		if ($beamdata && $mb_range > 100.0)
			{
			$mb_range = 100.0;
			}
		$mb_minrange = $mb_minrangefraction * $sonaraltitudeuse;
		$mb_maxrange = $mb_range;
		$mb_mindepth = 0.0;
		$mb_maxdepth = $mb_range;

		print MFILE "#######################################################\r\n";
		print MFILE "# Set sonar parameters, turn pinging on, power zero and logging off \r\n";
		print MFILE "# \r\n";
		print MFILE "behavior reson \r\n";
		print MFILE "{ \r\n";
		print MFILE "duration  = RESON_DURATION; \r\n";	
		print MFILE "MB_Mode  = 1; \r\n";	
		print MFILE "Log_Mode  = 0; \r\n";	
		print MFILE "SBP_Mode = 0; \r\n";
		print MFILE "SBP_Power = 0.0; \r\n";
		print MFILE "SBP_Gain = 128.0; \r\n";
		printf MFILE "SBP_Duration = %.3f; \r\n", $sbp_duration;
		print MFILE "LoSS_Mode = 0; \r\n";
		print MFILE "LoSS_Power = 0.0; \r\n";
		printf MFILE "LoSS_Range = %.2f; \r\n", $sslo_range;
		print MFILE "HiSS_Mode = 0; \r\n";
		print MFILE "HiSS_Power = 0.0; \r\n";
		print MFILE "HiSS_Range = $sslo_range; \r\n";
		print MFILE "MB_Power = 0; \r\n";
		printf MFILE "MB_Range = %.2f; \r\n", $mb_range;
		print MFILE "MB_Rate = $mb_pingrate; \r\n";
		print MFILE "MB_Gain = $mb_receivegain; \r\n";
		print MFILE "MB_Sound_Velocity = 0.0; \r\n";
		printf MFILE "MB_Pulse_Width = %f; \r\n", $mb_pulsewidth;
		printf MFILE "MB_Bottom_Detect_Filter_Min_Range = %.2f; \r\n", $mb_minrange;
		printf MFILE "MB_Bottom_Detect_Filter_Max_Range = %.2f; \r\n", $mb_maxrange;
		printf MFILE "MB_Bottom_Detect_Filter_Min_Depth = %.2f; \r\n", $mb_mindepth;
		printf MFILE "MB_Bottom_Detect_Filter_Max_Depth = %.2f; \r\n", $mb_maxdepth;
		print MFILE "MB_Bottom_Detect_Range_Mode = 1; \r\n";
		print MFILE "MB_Bottom_Detect_Depth_Mode = 0; \r\n";
		print MFILE "Snippet_Mode = $mb_snippetmode; \r\n";
		print MFILE "Window_Size = 200; \r\n";
		print MFILE "} \r\n";
		print MFILE "# \r\n";
		print MFILE "#######################################################\r\n";
print "Behavior: reson (start, reset, Log_Mode = 0)\n";
		print MFILE "#######################################################\r\n";
		}

	# Close the output file
	print MFILE "#######################################################\r\n";
	print MFILE "#######################################################\r\n";
	close(MFILE);
	close(WFILE);

	# output winfrog waypoint file unless outputoff option selected
	$winfrogfile = "$root.pts";
	open(WFILE,">$winfrogfile") || die "Cannot open output winfrog file: $winfrogfile\r\n$program_name aborted.\r\n";
	printf WFILE "0,$routename,0,0.000,0.000,1,2,65280,0,0.200,0,0,1.000\r\n";
	for ($i = 0; $i < $npoints; $i++)
 		{
		if ($waypoints[$i] != 0)
			{
 			printf WFILE "1,%.10f,%.10f,0.00m,0.00m,0.00,0.00,%.3f\r\n", $lats[$i], $lons[$i], $distances[$i];
			}
 		}
	close(WFILE);

# 	# output Capn Voyager WPL waypoint file unless outputoff option selected
# 	$winfrogfile = "$root" . "_capnwpt.rut";
# 	open(WFILE,">$winfrogfile") || die "Cannot open output Capn Voyager file: $winfrogfile\r\n$program_name aborted.\r\n";
# 	$cnt = 0;
# 	for ($i = 0; $i < $npoints; $i++)
#  		{
# 		if ($waypoints[$i] != 0)
# 			{
# 			$cnt++;
# 			if ($lats[$i] > 0.0)
# 				{
# 				$NorS = "N";
# 				$latdeg = int($lats[$i]);
# 				$latmin = ($lats[$i] - $latdeg) * 60.0;
# 				$latminb = int($latmin);
# 				$latmins = int(($latmin - $latminb) * 1000 + 0.5);
# 				}
# 			else
# 				{
# 				$NorS = 'S';
# 				$latdeg = int(-$lats[$i]);
# 				$latmin = (-$lats[$i] - $latdeg) * 60.0;
# 				$latminb = int($latmin);
# 				$latmins = int(($latmin - $latminb) * 1000 + 0.5);
# 				}
# 			if ($lons[$i] > 0.0)
# 				{
# 				$EorW = "E";
# 				$londeg = int($lons[$i]);
# 				$lonmin = ($lons[$i] - $londeg) * 60.0;
# 				$lonminb = int($lonmin);
# 				$lonmins = int(($lonmin - $lonminb) * 1000 + 0.5);
# 				}
# 			else
# 				{
# 				$EorW = 'W';
# 				$londeg = int(-$lons[$i]);
# 				$lonmin = (-$lons[$i] - $londeg) * 60.0;
# 				$lonminb = int($lonmin);
# 				$lonmins = int(($lonmin - $lonminb) * 1000 + 0.5);
# 				}
#  			printf WFILE "\$IIWPL,%2.2d%2.2d.%3.3d,%s,%3.3d%2.2d.%3.3d,%s,AUV %d\r\n", $latdeg, $latminb, $latmins, $NorS,, $londeg, $lonminb, $lonmins, $EorW, $cnt;
# 			}
#  		}
# 	close(WFILE);

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
#-----------------------------------------------------------------------
# This version of Getopts has been augmented to support multiple
# calls to the same option. If an arg in argumentative is followed
# by "+" rather than ":",  then the corresponding scalar will
# be concatenated rather than overwritten by multiple calls to
# the same arg.
#
# Usage:
#      do Getopts('a:b+c'); # -a takes arg, -b concatenates args,  
#			    # -c does not take arg. Sets opt_* as a
#                           # side effect.

sub MBGetopts {
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
		eval "\$flg_$first = 1;";
	    }
	    elsif($args[$pos+1] eq '+') {
		shift(@ARGV);
		if($rest eq '') {
		    ++$errs unless @ARGV;
		    $rest = shift(@ARGV);
		}
		if (eval "\$opt_$first") {
		    eval "\$opt_$first = \$opt_$first 
				. \":\" . \$rest;";
		}
		else {
		    eval "\$opt_$first = \$rest;";
		}
		eval "\$flg_$first = 1;";
	    }
	    elsif($args[$pos+1] eq '%') {
		shift(@ARGV);
		if($rest ne '') {
		    eval "\$opt_$first = \$rest;";
		}
		else {
		    $rest = $ARGV[0];
		    ($one) = $rest =~ /^-(.).*/;
		    $pos = index($argumentative,$one);
		    if(!$one || $pos < $[) {
			eval "\$opt_$first = \$rest;";
			shift(@ARGV);
		    }
		}
		eval "\$flg_$first = 1;";
	    }
	    else {
		eval "\$opt_$first = 1";
		eval "\$flg_$first = 1;";
		if($rest eq '') {
		    shift(@ARGV);
		}
		else {
		    $ARGV[0] = "-$rest";
		}
	    }
	}
	else {
	    print STDERR "Unknown option: $first\n";
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
