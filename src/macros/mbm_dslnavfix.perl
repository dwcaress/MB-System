eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
                    & eval 'exec perl -S $0 $argv:q'
                         if 0;
#--------------------------------------------------------------------
#    The MB-system:	mbm_dslnavfix.perl	8/9/96
#    $Id: mbm_dslnavfix.perl,v 4.0 1996-08-12 21:14:19 caress Exp $
#
#    Copyright (c) 1996 by 
#    D. W. Caress (caress@lamont.ldgo.columbia.edu)
#    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
#    Lamont-Doherty Earth Observatory
#    Palisades, NY  10964
#
#    See README file for copying and redistribution conditions.
#--------------------------------------------------------------------
#
# Command:
#   mbm_dslnavfix
#
# Purpose:
#   Macro to take WHOI DSL AMS-120 processed navigation
#   in UTM projected eastings and northings and produce
#   navigation in longitude and latitude. The output navigation
#   can be merged with the DSL AMS-120 bathymetry and sidescan
#   data using mbmerge (use the -M2 option of mbmerge).
#
# Basic Usage: 
#   mbm_dslnavfix -Iinfile -Ooutfile -Jutm_zone [-V -H]
#
# Author:
#   David W. Caress
#   Lamont-Doherty Earth Observatory
#   Palisades, NY  10964
#   August 9, 1996
#
# Version:
#   $Id: mbm_dslnavfix.perl,v 4.0 1996-08-12 21:14:19 caress Exp $
#
# Revisions:
#   $Log: not supported by cvs2svn $
#
#
$program_name = "mbm_dslnavfix";

# Deal with command line arguments
$command_line = "@ARGV";
&MBGetopts('HhI:i:J:j:O:o:Vv');
$help =    		($opt_H || $opt_h);
$infile =    		($opt_I || $opt_i);
$map_scale =    	($opt_J || $opt_j);
$outfile =    		($opt_O || $opt_o);
$verbose =    		($opt_V || $opt_v);

# print out help message if required
if ($help)
	{
	print "\n$program_name:\n";
	print "\nMacro to take WHOI DSL AMS-120 processed navigation \n";
	print "in UTM projected eastings and northings and produce \n";
	print "navigation in longitude and latitude. The output navigation \n";
	print "can be merged with the DSL AMS-120 bathymetry and sidescan \n";
	print "data using mbmerge (use the -M2 option of mbmerge). \n";
        print "\nBasic Usage:  \n";
        print "mbm_dslnavfix -Iinfile -Ooutfile -Jutm_zone [-V -H] \n\n";
	exit 0;
	}

# check for input and output files
if (!$infile)
	{
	print "\a";
	die "No input file specified - $program_name aborted\n";
	}
elsif (! -e $infile)
	{
	print "\a";
	die "Specified input file cannot be opened - $program_name aborted\n";
	}
if (!$outfile)
	{
	print "\a";
	die "No output file specified - $program_name aborted\n";
	}

# check for projection
if (!$map_scale)
	{
	print "\a";
	die "No UTM zone specified - $program_name aborted\n";
	}

# tell the world we got started
if ($verbose) 
	{
	print "\nRunning $program_name...\n";
	}

# make the temporary filenames
$tmp_proj_navfile = "tmp_proj_$$.nav";
$tmp_geo_navfile = "tmp_geo_$$.nav";

# get projection
$projection = "u" . "$map_scale" . "/1:1000000";
$projection_id = "UTM Zone $map_scale";

# set units of eastings and northings 
$projection_units = "M";
$units_meters = 1;

# now set the other projection parameters 
if ($map_scale > 0)
	{
	$org_lon_n = -182 + 6 * $map_scale;
	}
else
	{
	$org_lon_n = -182 - 6 * $map_scale;
	}
$org_lon = "$org_lon_n:0";
$org_lat = "0:0";
$org_lon_n = $org_lon_n + 1;
$org_lon_plus1 = "$org_lon_n:0";
$org_lat_plus1 = "1:0";
if ($map_scale > 0)
	{
	$org_x_ft = 1640416.66667;
	$org_x_m = 500000.0;
	$org_y_ft = 0.0;
	$org_y_m = 0.0;
	}
else
	{
	$org_x_ft = 1640416.66667;
	$org_x_m = 500000.0;
	$org_y_ft = 32808399.0;
	$org_y_m = 100000000.0;
	}

$bounds = "$org_lon" . "/" . "$org_lat" 
		. "/" . "$org_lon_plus1" 
		. "/" . "$org_lat_plus1" . "r";

# set gmt defaults
($d_format) = `gmtdefaults -L | grep D_FORMAT` =~ /\S+\s+\S+\s+(\S+)/;
($ellipsoid) = `gmtdefaults -L | grep ELLIPSOID` =~ /\S+\s+\S+\s+(\S+)/;
`gmtset D_FORMAT %.10lg ELLIPSOID Clarke-1866`;

# print out info
if ($verbose)
	{
	print "\nProgram Status:\n";
	print "--------------\n\n";
	print "  Input DSL navigation file:     $infile\n";
	print "  Output navigation file:        $outfile\n";
	print "  Temporary projected nav file:  $tmp_proj_navfile\n";
	print "  Temporary geographic nav file: $tmp_geo_navfile\n";
	print "\n  ------------\n\n";
	print "  Projection:                    $projection_id\n";
	if ($units_feet == 1)
		{
		print "  Projection units:              feet\n";
		}
	else
		{
		print "  Projection units:              meters\n";
		}
	print "\n  ------------\n\n";
	}
	
# now open the raw DSL navigation data
if (!open(INP,"<$infile"))
	{
	die "\nInput file $infile cannot be opened!\n$program_name aborted\n";
	}
	
# now open the tmp projected navigation file
if (!open(OTMP,">$tmp_proj_navfile"))
	{
	die "\nTmp file $tmp_proj_navfile cannot be opened!\n$program_name aborted\n";
	}
	
# now read the raw DSL navigation data
while ($line = <INP>)
	{
	if (($date, $time, $easting, $northing) 
		= $line =~ /^\S+\s+(\S+)\s+(\S+)\s+\S+\s+\S+\s+\S+\s+(\S+)\s+(\S+)/)
		{
		# get time stamp
		($year, $month, $day)
			= $date =~ /^(.{2}).{1}(.{2}).{1}(.{2})/;
		($hour, $minute, $second)
			= $time =~ /^(.{2}).{1}(.{2}).{1}(.{5})/;
		$year = $year + 1900;
		push(@nyear, $year);
		push(@nmonth, $month);
		push(@nday, $day);
		push(@nhour, $hour);
		push(@nminute, $minute);
		push(@nsecond, $second);

		# get projected values in desired units 
		if ($units_feet)
			{
			$xx = 12.0 / 1000000 *($easting - $org_x_ft);
			$yy = 12.0 / 1000000 *($northing - $org_y_ft);
			}
		elsif ($units_meters)
			{
			$xx = ($easting - $org_x_m) / (0.0254 * 1000000);
			$yy = ($northing - $org_y_m) / (0.0254 * 1000000);
			}
			
		#print "$year $month $day $hour:$minute:$second $easting $northing\n";
		print OTMP "$xx $yy\n";
		}
	else
		{
		die "\nUnable to parse navigation from line:\n$line\n$program_name aborted\n";
		}
	}

# close files
close INP;
close OTMP;

# get number of time stamps
$ntime = @nyear;

# print out info
if ($verbose)
	{
	print "\n$ntime navigation records read...\n";
	}

# unproject the eastings and northings into longitude and latitude
if ($verbose)
	{
	print "\nRunning mapproject...\n";
	print "mapproject $tmp_proj_navfile -J$projection -I -R$bounds > $tmp_geo_navfile\n";
	}
@line = `mapproject $tmp_proj_navfile -J$projection -I -R$bounds > $tmp_geo_navfile`;
if ($verbose)
	{
	foreach $line (@line) {
		print "$line";
		}
	}

# reset the gmt defaults
`gmtset D_FORMAT $d_format ELLIPSOID $ellipsoid`;
	
# now open the unprojected navigation data
if (!open(INP,"<$tmp_geo_navfile"))
	{
	die "\nTmp file $tmp_geo_navfile cannot be opened!\n$program_name aborted\n";
	}
	
# now read the unprojected navigation data
while ($line = <INP>)
	{
	if (($lon, $lat) 
		= $line =~ /^(\S+)\s+(\S+)/)
		{
		# put lon in -180 to 180 range
		if ($lon < -180.0)
			{
			$lon = $lon + 360.0;
			}
		elsif ($lon > 180.0)
			{
			$lon = $lon - 360.0;
			}
			
		# add to list
		push(@nlon, $lon);
		push(@nlat, $lat);
		}
	else
		{
		die "\nUnable to parse navigation from line:\n$line\n$program_name aborted\n";
		}
	}

# close file
close INP;

# get number of unprojected nav points
$nnav = @nlon;

# check that numbers of data match
if ($ntime != $nnav)
	{
	die "\nNumber of time stamps ($ntime) disagrees with number of nav points ($nnav)!\n$program_name aborted\n";
	}
	
# now open the final navigation file
if (!open(OTMP,">$outfile"))
	{
	die "\nTmp file $outfile cannot be opened!\n$program_name aborted\n";
	}

# now write the final navigation to the file
for ($i = 0; $i < $ntime; $i ++)
	{
	print OTMP "$nyear[$i] $nmonth[$i] $nday[$i] $nhour[$i] $nminute[$i] $nsecond[$i] $nlon[$i] $nlat[$i]\n";
	}

# close file
close OTMP;

# remove the temporary files
if ($verbose)
	{
	print "\nDeleting $tmp_proj_navfile, $tmp_geo_navfile...\n";
	}
`rm -f $tmp_proj_navfile $tmp_geo_navfile`;

# done
if ($verbose)
	{
	print "\nDone...\n";
	}

exit 0;


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
