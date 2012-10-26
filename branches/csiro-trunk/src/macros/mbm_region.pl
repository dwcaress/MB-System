eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
                    & eval 'exec perl -S $0 $argv:q'
                         if 0;
#--------------------------------------------------------------------
#    The MB-system:	mbm_region.perl	2012-07-27
#    $Id: mbm_grid.pl 1962 2012-06-22 05:40:52Z gkeith $
#
#    Copyright (c) 1999-2012 by
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
#   mbm_region
#
# Purpose:
#	Outputs the bounding region of a swath file.
#	The output in the form usable as an argument to -R.
#	This macro will read the .inf file if it exists or run mbinfo
#	if it doesn't.
#
# Basic Usage:
#   mbm_region {-Ifilelist | -Ifile -Fformat} 
#
# Additional Options:
#            [-H -Llonflip -Jprojection
#            -M -N -Ppings
#            -U{azimuth/factor | time}
#            -V -Wscale -Xextend
#            -Ypriority_file -Zbathdef]
#
# Author:
#	Gordon Keith
#	CSIRO Marine and Atmospheric Research
#	27 July 2012
#	Based on mbm_grid
#
# Version:
#   $Id: mbm_region.pl 1962 2012-06-22 05:40:52Z gkeith $
#
# Revisions:
$program_name = "mbm_region";

# Deal with command line arguments
$command_line = "@ARGV";
&MBGetopts('A:a:B:b:C:c:D:d:E:e:F:f:G:g:HhI:i:J:j:K:k:L:l:MmO:o:P:p:R:r:S:s:T:t:U:u:VvW:w:X:x:Y:y:Z:z:');

$format = 		($opt_F);
$help =    		($opt_H || $opt_h);
$file_data = 		($opt_I || $opt_i || "datalist.mb-1");
$projection = 		($opt_J || $opt_j);
$lonflip =    		($opt_L || $opt_l);
$more = 		($opt_M || $opt_m);
$pings = 		($opt_P || $opt_p);
$verbose = 		($opt_V || $opt_v);
$extend = 		($opt_X || $opt_x);

# print out help message if required
if ($help)
	{
	print "\n$program_name:\n";
	print "\nVersion: $Id: mbm_region.pl 1962 2012-06-22 05:40:52Z gkeith $\n";
        print "Macro to output the region containing a data file. \n";
        print "\nBasic Usage:\n\tmbm_region {-Ifilelist | -Fformat -IFile} -V  \n";
        print "\nAdditional Options:\n\t-Rwest/east/south/north \n";
        print "         [-Adatatype -Bborder -Cclip -Dxdim/ydim -Edx/dy/units \n";
        print "         -fpriority_range -Ggridkind -H -Llonflip -M -N -Ppings  \n";
        print "         -Sspeed -Ttension -U{azimuth/factor | time}  \n";
        print "         -V -Wscale -Xextend  \n";
        print "         -Ypriority_file -Zbathdef]  \n";
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

# tell the world we got started
if ($verbose)
	{
	print "\nRunning $program_name...\n";
	}

# get format if needed
if (!$format)
	{
	$line = `mbformat -I $file_data -L`;
	($format) = $line =~ /(\S+)/;
	if ($format == 0)
		{
		$format = -1;
		}
	}

# get limits of file using mbinfo
if ($format > 0)
	{
	push(@files_data, $file_data);
	push(@formats, $format);
	}
else
	{
	# we used to use this perl function
	# 	MBparsedatalist($file_data);
	# but now we use the program mbdatalist
	@mbdatalist = `mbdatalist -F-1 -I$file_data`;
	while (@mbdatalist)
		{
		$line = shift @mbdatalist;
		if ($line =~ /(\S+)\s+(\S+)/)
			{
			($file_mb,$format_mb) =
				$line =~ /(\S+)\s+(\S+)/;
			push(@files_data, $file_mb);
			push(@formats, $format_mb);
			}
		}
	}

$cnt = -1;
foreach $file_mb (@files_data)
	{
	# use .inf file if it exists and no time or space bounds applied
	$use_inf = 0;
	$file_inf = $file_mb . ".inf";
	@mbinfo = 0;
	if (-r $file_inf)
		{
		if ($verbose && !$bounds)
			{
			print "Reading mbinfo output from file $file_inf...\n";
			}
		if (open(FILEINF,"<$file_inf"))
			{
			while ($line = <FILEINF>)
				{
				push(@mbinfo, $line);
				if ($line =~ /^Number of Records:\s+(\S+)/)
					{
					($nrec_f) =
						$line =~ /^Number of Records:\s+(\S+)/;
					}
				if ($line =~ /^Minimum Longitude:\s+(\S+)\s+Maximum Longitude:\s+(\S+)/)
					{
					($xmin_f,$xmax_f) =
						$line =~ /^Minimum Longitude:\s+(\S+)\s+Maximum Longitude:\s+(\S+)/;
					}
				if ($line =~ /^Minimum Latitude:\s+(\S+)\s+Maximum Latitude:\s+(\S+)/)
					{
					($ymin_f,$ymax_f) =
						$line =~ /^Minimum Latitude:\s+(\S+)\s+Maximum Latitude:\s+(\S+)/;
					}
				}
			close FILEINF;

			if (!$bounds)
			    {
			    $use_inf = 1;
			    }
			elsif ($nrec_f > 0 &&
			    $xmin_f >= $xmin && $xmin_f <= $xmax
				&& $xmax_f >= $xmin && $xmax_f <= $xmax
			    && $ymin_f >= $ymin && $ymin_f <= $ymax
				&& $ymax_f >= $ymin && $ymax_f <= $ymax)
			    {
			    $use_inf = 1;
			    }
			elsif ($nrec_f > 0 &&
			    ($xmin_f < $xmin
				|| $xmax_f > $xmax
				|| $ymin_f < $ymin
				|| $ymax_f > $ymax))
				    {
			    $use_inf = 0;
			    }
			elsif ($nrec_f > 0)
			    {
			    $use_inf = 1;
			    }
			}
		}

	# if .inf file not accessible or suitable run mbinfo directly
	if (!$use_inf)
		{
		if ($verbose)
			{
			print "Running mbinfo on file $file_mb...\n";
			}
		$cnt++;
		if (!$bounds)
			{
			@mbinfo = `mbinfo -F$formats[$cnt] -I$file_mb -G`;
			}
		else
			{
			@mbinfo = `mbinfo -F$formats[$cnt] -I$file_mb -R$bounds -G`;
			}
		}

	# now parse the mbinfo output
	$nrec_f = 0;
	while (@mbinfo)
		{
		$line = shift @mbinfo;
		if ($line =~ /^Number of Records:\s+(\S+)/)
			{
			($nrec_f) =
				$line =~ /^Number of Records:\s+(\S+)/;
			}
		if ($line =~ /^Minimum Longitude:\s+(\S+)\s+Maximum Longitude:\s+(\S+)/)
			{
			($xmin_f,$xmax_f) =
				$line =~ /^Minimum Longitude:\s+(\S+)\s+Maximum Longitude:\s+(\S+)/;
			}
		if ($line =~ /^Minimum Latitude:\s+(\S+)\s+Maximum Latitude:\s+(\S+)/)
			{
			($ymin_f,$ymax_f) =
				$line =~ /^Minimum Latitude:\s+(\S+)\s+Maximum Latitude:\s+(\S+)/;
			}
		if ($line =~ /^Minimum Depth:\s+(\S+)\s+Maximum Depth:\s+(\S+)/)
			{
			($zmin_f,$zmax_f) =
			$line =~ /^Minimum Depth:\s+(\S+)\s+Maximum Depth:\s+(\S+)/;
			}
		}

	if (!$first_mb && $nrec_f > 0)
		{
		$first_mb = 1;
		$xmin_data = $xmin_f;
		$xmax_data = $xmax_f;
		$ymin_data = $ymin_f;
		$ymax_data = $ymax_f;
		$zmin_data = $zmin_f;
		$zmax_data = $zmax_f;
		}
	elsif ($nrec_f > 0)
		{
		$xmin_data = &min($xmin_data, $xmin_f);
		$xmax_data = &max($xmax_data, $xmax_f);
		$ymin_data = &min($ymin_data, $ymin_f);
		$ymax_data = &max($ymax_data, $ymax_f);
		$zmin_data = &min($zmin_data, $zmin_f);
		$zmax_data = &max($zmax_data, $zmax_f);
		}
	}

# check that there is data
if ($xmin_data >= $xmax_data || $ymin_data >= $ymax_data)
	{
	die "$xmin_data $xmax_data $ymin_data $ymax_data \nDoes not appear to be any data in the input!\n$program_name aborted.\n";
	}

if($extend)
{
	$delx = 0.05 * ($xmax_data - $xmin_data);
	$dely = 0.05 * ($ymax_data - $ymin_data);
	$xmin = $xmin_data - $delx;
	$xmax = $xmax_data + $delx;
	$ymin = $ymin_data - $dely;
	$ymax = $ymax_data + $dely;
	$dx = $xmax - $xmin;
	$dy = $ymax - $ymin;
	if ($dy/$dx > 10.0)
		{
		$delx = 0.5 * (0.1 * $dy - $dx);
		$xmin = $xmin - $delx;
		$xmax = $xmax + $delx;
		}
	elsif ($dx/$dy > 10.0)
		{
		$dely = 0.5 * (0.1 * $dx - $dy);
		$ymin = $ymin - $dely;
		$ymax = $ymax + $dely;
		}
	$bounds_grid = sprintf ("%1.8g/%1.8g/%1.8g/%1.8g",
		$xmin, $xmax, $ymin, $ymax);
}
else
{
	$bounds_grid = sprintf ("%1.8g/%1.8g/%1.8g/%1.8g",
		$xmin_data, $xmax_data, $ymin_data, $ymax_data);
	
}
print "$bounds_grid\n";

exit 0;

#-----------------------------------------------------------------------
sub min {

	# make local variables
	local ($min);

	# get the minimum of the arguments
	if ($_[0] < $_[1])
		{
		$min = $_[0];
		}
	else
		{
		$min = $_[1];
		}
	$min;
}
#-----------------------------------------------------------------------
sub max {

	# make local variables
	local ($max);

	# get the minimum of the arguments
	if ($_[0] > $_[1])
		{
		$max = $_[0];
		}
	else
		{
		$max = $_[1];
		}
	$max;
}
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
