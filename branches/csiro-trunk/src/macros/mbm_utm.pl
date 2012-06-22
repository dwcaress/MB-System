eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
                    & eval 'exec perl -S $0 $argv:q'
                         if 0;
#--------------------------------------------------------------------
#    The MB-system:	mbm_utm.perl	5/13/2002
#    $Id$
#
#    Copyright (c) 2002-2012 by
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
#   mbm_utm
#
# Purpose:
#   MB-System macro to perform forward and inverse UTM projections
#   of ASCII data triples using the GMT program mapproject.
#   Forward projections translate from geographic data
#   (lon, lat, value) to UTM eastings and northings (x, y, value).
#   Inverse projections translate from (x, y, z) to (lon, lat, z).
#
# Basic Usage:
#   mbm_utm -Ifile -Zzone -Dellipsoid [-F -Q -H -V]
#
# Author:
#   David W. Caress
#   Monterey Bay Aquarium Research Institute
#   Moss Landing, CA 95039
#   May 13, 2002
#
# Version:
#   $Id$
#
# Revisions:
#   $Log: mbm_utm.perl,v $
#   Revision 5.4  2007/10/08 04:27:20  caress
#   Now handles comma delimited xyz data along with white space delimited data.
#
#   Revision 5.3  2003/04/17 20:42:48  caress
#   Release 5.0.beta30
#
#   Revision 5.2  2003/03/27 07:58:11  caress
#   This macro now sets MEASURE_UNIT to inches and then resets it to the original value.
#
#   Revision 5.1  2002/05/29 23:35:57  caress
#   Release 5.0.beta18
#
#   Revision 5.0  2002/05/14 18:53:03  caress
#   Initial revision.
#
#
#
$program_name = "mbm_utm";

# Deal with command line arguments
$command_line = "@ARGV";
&MBGetopts('HHE:e:FfI:i:L:l:O:o:QqZ:z:Vv');
$help =    		($opt_H || $opt_h);
$file_data =    	($opt_I || $opt_i);
$file_out =    		($opt_O || $opt_o);
$zone =    		($opt_Z || $opt_z || 10);
$ellipsoid =    	($opt_E || $opt_e || "WGS-84");
$usefeet = 		($opt_F || $opt_f);
$lonflip = 		($opt_L || $opt_l || 0);
$inverse = 		($opt_Q || $opt_q);
$verbose = 		($opt_V || $opt_v);

# print out help message if required
if ($help)
	{
	print "\n$program_name:\n";
	print "\nVersion: $Id$\n";
	print "\nMB-System macro to perform forward and inverse UTM projections";
	print "of ASCII data triples using the GMT program mapproject. ";
	print "Forward projections translate from geographic data ";
	print "(lon, lat, value) to UTM eastings and northings (x, y, value). ";
	print "Inverse projections translate from (x, y, z) to (lon, lat, z).";
	print "\nUsage: \n";
	print "\t$program_name -Eellipsoid -Ifile -Llonflip -Ooutput -Zzone [-F -Q -H -V]\n";
	exit 0;
	}

# check for input file
if (!$file_data)
	{
	print "\a";
	die "No input file specified - $program_name aborted\n";
	}
elsif (! -e $file_data)
	{
	print "\a";
	die "Specified input file cannot be opened - $program_name aborted\n";
	}

# check for ouput file
if (!$file_out)
	{
	print "\a";
	die "No output file specified - $program_name aborted\n";
	}

# get projection
$projection = "u" . "$zone" . "/1:1000000";

# now set the other projection parameters
$org_lon = -183.0 + $zone * 6.0;
$org_lat = 0.0;
$org_lon_plus1 = $org_lon + 1.0;
$org_lat_plus1 = $org_lat + 1.0;
$org_x_ft = 1640416.66667;
$org_x_m = 500000.0;
$org_y_ft = 0.0;
$org_y_m = 0.0;

$bounds = "$org_lon" . "/" . "$org_lat"
		. "/" . "$org_lon_plus1"
		. "/" . "$org_lat_plus1" . "r";

# set gmt defaults
($d_format) = `gmtdefaults -L | grep D_FORMAT` =~ /\S+\s+\S+\s+(\S+)/;
($ellipsoid) = `gmtdefaults -L | grep ELLIPSOID` =~ /\S+\s+\S+\s+(\S+)/;
($measure_unit) = `gmtdefaults -L | grep MEASURE_UNIT` =~ /\S+\s+\S+\s+(\S+)/;
if ($ellipsoid eq "Clarke-1866")
	{
	`gmtset D_FORMAT %.10lg ELLIPSOID Clarke-1866 MEASURE_UNIT inch`;
	}
else
	{
	`gmtset D_FORMAT %.10lg ELLIPSOID WGS-84 MEASURE_UNIT inch`;
	}

# set temporary file name
$filetmpa = "$file_data" . "$$" . "a.tmp";
$filetmpb = "$file_data" . "$$" . "b.tmp";

# Do forward projection from lon lat to UTM easting northing
if (!$inverse)
	{
	# open input file
	if (!open(INP1,"<$file_data"))
		{
		die "\nInput file $file_data cannot be opened!\n$program_name aborted\n";
		}

	# open temporary file
	if (!open(OUT1,">$filetmpa"))
		{
		die "\nTemporary file $filetmpa cannot be opened!\n$program_name aborted\n";
		}

	# read temporary file, looping over input lines
	if ($verbose)
		{
		print "Preprocessing input...\n";
		}
	$ndata = 0;
	while ($line = <INP1>)
		{
		$ok = 0;

		# replace any commas with tabs
		$line =~ s/,/\t/g;

		if ($line =~ /[^\s\d\-\.]/)
			{
			}
		elsif ($line =~ /(\S+)\s+(\S+)\s+(.*)/)
			{
			($xx,$yy,$val) = $line =~ /(\S+)\s+(\S+)\s+(.*)/;
			$ok = 1;
			}
		elsif ($line =~ /(\S+)\s+(\S+)/)
			{
			($xx,$yy) = $line =~ /(\S+)\s+(\S+)/;
			$val = "";
			$ok = 1;
			}

		# only use good lines
		if ($ok == 1)
			{
			# print out result
			print OUT1 "$xx\t$yy\t$val\n";

			$ndata = $ndata + 1;
			}
		}

	# close input file
	close INP1;

	# close temporary file
	close OUT1;

	# run mapproject
	if ($verbose)
		{
		print "Doing forward projection using mapproject...\n";
		}
	`mapproject $filetmpa -J$projection -R$bounds > $filetmpb`;

	# open temporary file
	if (!open(INP2,"<$filetmpb"))
		{
		die "\nTemporary file $filetmpb cannot be opened!\n$program_name aborted\n";
		}

	# open output file
	if (!open(OUT2,">$file_out"))
		{
		die "\nOutput file $file_out cannot be opened!\n$program_name aborted\n";
		}

	# read temporary file, looping over input lines
	if ($verbose)
		{
		print "Processing output from mapproject...\n";
		}
	$ndata = 0;
	while ($line = <INP2>)
		{
		if ($line =~ /(\S+)\s+(\S+)\s+(.*)/)
			{
			($xx,$yy,$val) = $line =~ /(\S+)\s+(\S+)\s+(.*)/;
			}
		else
			{
			($xx,$yy) = $line =~ /(\S+)\s+(\S+)/;
			$val = "";
			}

		# get projected values in desired units
		if ($usefeet)
			{
			$xx = 1000000 * $xx / 12.0 + $org_x_ft;
			$yy = 1000000 * $yy / 12.0 + $org_y_ft;
			}
		else
			{
			$xx = 1000000 * 0.0254 * $xx + $org_x_m;
			$yy = 1000000 * 0.0254 * $yy + $org_y_m;
			}

		# print out result
		print OUT2 "$xx\t$yy\t$val\n";

		$ndata = $ndata + 1;
		}
	if ($verbose)
		{
		print "Processed $ndata data points...\n";
		}

	# close temporary file
	close INP2;

	# close output file
	close OUT2;
	}

# Do inverse projection from UTM easting northing to lon lat
else
	{
	# open input file
	if (!open(INP1,"<$file_data"))
		{
		die "\nInput file $file_data cannot be opened!\n$program_name aborted\n";
		}

	# open temporary file
	if (!open(OUT1,">$filetmpa"))
		{
		die "\nTemporary file $filetmpa cannot be opened!\n$program_name aborted\n";
		}

	# read temporary file, looping over input lines
	if ($verbose)
		{
		print "Preprocessing input...\n";
		}
	$ndata = 0;
	while ($line = <INP1>)
		{
		$ok = 0;

		# replace any commas with tabs
		$line =~ s/,/\t/g;

		if ($line =~ /[^\s\d\-\.]/)
			{
			}
		elsif ($line =~ /(\S+)\s+(\S+)\s+(.*)/)
			{
			($xx,$yy,$val) = $line =~ /(\S+)\s+(\S+)\s+(.*)/;
			$ok = 1;
			}
		elsif ($line =~ /(\S+)\s+(\S+)/)
			{
			($xx,$yy) = $line =~ /(\S+)\s+(\S+)/;
			$val = "";
			$ok = 1;
			}

		# only use good lines
		if ($ok == 1)
			{
			# get projected values in desired units
			if ($usefeet)
				{
				$xx = 12.0 / 1000000 *($xx - $org_x_ft);
				$yy = 12.0 / 1000000 *($yy - $org_y_ft);
				}
			else
				{
				$xx = ($xx - $org_x_m) / (0.0254 * 1000000);
				$yy = ($yy - $org_y_m) / (0.0254 * 1000000);
				}

			# print out result
			print OUT1 "$xx\t$yy\t$val\n";

			$ndata = $ndata + 1;
			}
		}

	# close input file
	close INP1;

	# close temporary file
	close OUT1;

	# run mapproject
	if ($verbose)
		{
		print "Doing inverse projection using mapproject...\n";
		}
	`mapproject $filetmpa -J$projection -I -R$bounds > $filetmpb`;

	# open temporary file
	if (!open(INP2,"<$filetmpb"))
		{
		die "\nTemporary file $filetmpb cannot be opened!\n$program_name aborted\n";
		}

	# open output file
	if (!open(OUT2,">$file_out"))
		{
		die "\nOutput file $file_out cannot be opened!\n$program_name aborted\n";
		}

	# read temporary file, looping over input lines
	if ($verbose)
		{
		print "Processing output from mapproject...\n";
		}
	$ndata = 0;
	while ($line = <INP2>)
		{
		if ($line =~ /(\S+)\s+(\S+)\s+(.*)/)
			{
			($xx,$yy,$val) = $line =~ /(\S+)\s+(\S+)\s+(.*)/;
			}
		else
			{
			($xx,$yy) = $line =~ /(\S+)\s+(\S+)/;
			$val = "";
			}

		# apply lonflip
		if ($lonflip == -1)
			{
			if ($xx < -360.0)
				{
				$xx += 360.0;
				}
			elsif ($xx > 0.0)
				{
				$xx -= 360.0;
				}
			}
		elsif ($lonflip == 0)
			{
			if ($xx < -180.0)
				{
				$xx += 360.0;
				}
			elsif ($xx > 180.0)
				{
				$xx -= 360.0;
				}
			}
		elsif ($lonflip == 1)
			{
			if ($xx < 0.0)
				{
				$xx += 360.0;
				}
			elsif ($xx > 360.0)
				{
				$xx -= 360.0;
				}
			}

		# print out result
		print OUT2 "$xx\t$yy\t$val\n";

		$ndata = $ndata + 1;
		}
	if ($verbose)
		{
		print "Processed $ndata data points...\n";
		}

	# close temporary file
	close INP2;

	# close output file
	close OUT2;
	}

# remove temporary files
unlink($filetmpa);
unlink($filetmpb);

# reset the gmt defaults
`gmtset D_FORMAT $d_format ELLIPSOID $ellipsoid MEASURE_UNIT $measure_unit`;

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
