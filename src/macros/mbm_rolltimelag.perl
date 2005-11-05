eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
                    & eval 'exec perl -S $0 $argv:q'
                         if 0;
#--------------------------------------------------------------------
#    The MB-system:	mbm_rollover.perl	6/18/93
#    $Id: mbm_rolltimelag.perl,v 5.0 2005-11-05 01:35:38 caress Exp $
#
#    Copyright (c) 1993, 1994, 2000, 2003 by 
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
#   mbm_rollover
#
# Purpose:
#   Perl shellscript to read a multibeam bathymetry file, calculate the
#   noise in the vertical reference used by the sonar, and generate
#   a file containing roll corrections which can be applied to the
#   data.
#
# Usage:
#   mbm_rollover -Fformat -Wfilterwidth -Ifile
#
# Author:
#   David W. Caress
#   Lamont-Doherty Earth Observatory
#   Palisades, NY  10964
#   June 13, 1993
#
# Version:
#   $Id: mbm_rolltimelag.perl,v 5.0 2005-11-05 01:35:38 caress Exp $
#
# Revisions:
#   $Log: not supported by cvs2svn $
#
#
#
$program_name = "mbm_rolltimelag";
#
# Deal with command line arguments
&Getopts('I:i:F:f:T:t:W:w:Vv');
$file =        ($opt_I || $opt_i);
$format =      ($opt_F || $opt_f);
$timelagstep = ($opt_S || $opt_s || 0.05);
$timelagmax =  ($opt_T || $opt_t || 5.00);
$npings =      ($opt_N || $opt_n || 100);
$verbose =     ($opt_V || $opt_v || 0);

# get format if needed
if (!$format) 
	{
	$line = `mbformat -I $file -L`;
	($format) = $line =~ /(\S+)/;
	if ($format == 0)
		{
		$format = -1;
		}
	}

# check for input file
if (!$file)
	{
	print "\a";
	die "No input file specified - $program_name aborted\n";
	}
elsif (! -e $file)
	{
	print "\a";
	die "Specified input file $file cannot be opened - $program_name aborted\n";
	}

# tell the world we got started
if ($verbose) 
	{
	print "\nRunning $program_name...\n";
	}

# loop over all files referenced in datalist
if ($verbose)
	{
	print "Getting file list using mbdatalist...\n";
	}
@mbdatalist = `mbdatalist -F$format -I$file`;
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

# set filterwidth if needed
if (!$windowwidth)
	{
	$windowwidth = 5;
	}

# loop over all files
$cnt = 0;
foreach $file_mb (@files_data)
{
# increment counter
$cnt++;

# get output files for cross correlations
$xcorrfile = "$file_mb" . "_xcorr.txt";
$xcorrcmd = "$file_mb" . "_xcorr.cmd";
if (!open(XFIL,">$xcorrfile"))
	{
	print "\a";
	die "Cannot open output file $xcorrfile\nMacro $program_name aborted.\n";
	}

# Run mblist to get apparent slope time series
print "\nCalculating roll/slope cross correlation for $file_mb ...\n";
print "Running mblist...\n";
#print "Running mblist...\nmblist -F$formats[$cnt-1] -I$file_mb -OMA\n";
@mblist = `mblist -F$formats[$cnt-1] -I$file_mb -OMA >& 1`;
$nslope = 0;
while (@mblist)
	{
	$line = shift @mblist;
	if ($line =~ /(\S+)\s+(\S+)/)
		{
		($time_d, $slope) = $line =~ /(\S+)\s+(\S+)/;
		if ($nslope == 0 || $time_d > @slope_time_d[$nslope-1])
			{
			push(@slope_time_d, $time_d);
			push(@slope, $slope);
			$nslope++;
			}
		}
 	}
print "Read $nslope slope data...\n";

# Run mbnavlist to get roll time series
print "Running mbnavlist...\n";
#print "Running mbnavlist...\nmbnavlist -F$formats[$cnt-1] -I$file_mb -N2 -OMR\n";
@mbnavlist = `mbnavlist -F$formats[$cnt-1] -I$file_mb -N2 -OMR >& 1`;
$nroll = 0;
while (@mbnavlist)
	{
	$line = shift @mbnavlist;
	if ($line =~ /(\S+)\s+(\S+)/)
		{
		($time_d, $roll) = $line =~ /(\S+)\s+(\S+)/;
		if ($nroll == 0 || $time_d > @roll_time_d[$nroll-1])
			{
			push(@roll_time_d, $time_d);
			push(@roll, $roll);
			$nroll++;
			}
		}
 	}
print "Read $nroll roll data...\n";

# Print out what's been read
# for ($i = 0; $i < $nslope; $i++)
# 	{
# 	print "Slope: @slope_time_d[$i] @slope[$i]\n";
# 	}
# for ($i = 0; $i < $nroll; $i++)
# 	{
# 	print "Roll: @roll_time_d[$i] @roll[$i]\n";
# 	}

# set up cross correlation calculations 
$ntimelag = 2 * int($timelagmax / $timelagstep) + 1;

# loop over the pings in $npings chunks
$ncalcs = int($nslope / $npings);
for ($i = 0; $i < $nslope / $npings; $i++)
	{
	# get ping range in this chunk
	$j0 = $i * $npings;
	$j1 = $j0 + $npings - 1;
	
	# get mean slope in this chunk
	$slopemean = 0.0;
	for ($j = $j0; $j <= $j1; $j++)
		{
		$slopemean += @slope[$j];
		}
	$slopemean /= $npings;
	
	# get mean roll in this chunk
	$rollmean = 0.0;
	$nrollmean = 0;
	for ($j = 0; $j < $nroll; $j++)
		{
		if ((@roll_time_d[$j] >= @slope_time_d[$j0] - $timelagmax)
			&& (@roll_time_d[$j] <= @slope_time_d[$j1] + $timelagmax))
			{
			$rollmean += @roll[$j];
			$nrollmean++;
			}
		}
	if ($nrollmean > 0)
		{
		$rollmean /= $nrollmean;
		}
#print "chunk $i pings: $j0 $j1   slopemean: $slopemean $npings rollmean: $rollmean $nrollmean\n";
	
	# calculate cross correlation for the specified time lags
	if ($nrollmean > 0)
		{
		print XFIL ">\n";
		for ($k = 0; $k < $ntimelag; $k++)
			{
			$timelag = -$timelagmax + $k * $timelagstep;
			$sumsloperoll = 0.0;
			$sumslopesq = 0.0;
			$sumrollsq = 0.0;

			for ($j = $j0; $j <= $j1; $j++)
				{
				# interpolate lagged roll value
				$nr = -1;
				$time_d = @slope_time_d[$j] - $timelag;
				for ($l = 0; $l < $nroll - 1 && $nr < 0; $l++)
					{
					if ($time_d >= @roll_time_d[$l] 
						&& $time_d <= @roll_time_d[$l+1])
						{
						$nr = $l;
						}
					}
				if ($nr == -1 && $time_d < @roll_time_d[0])
					{
					$rollint = @roll[0];
					}
				elsif ($nr == -1 && $time_d > @roll_time_d[$nroll - 1])
					{
					$rollint = @roll[$nroll - 1];
					}
				else
					{
					$rollint = @roll[$nr] + (@roll[$nr+1] - @roll[$nr])
								* ($time_d - @roll_time_d[$nr])
								/ (@roll_time_d[$nr+1] - @roll_time_d[$nr]);
					}
				
				# add to sums
				$slopeminusmean = (@slope[$j] - $slopemean);
				$rollminusmean = ($rollint - $rollmean);
				$sumslopesq += $slopeminusmean * $slopeminusmean;
				$sumrollsq += $rollminusmean * $rollminusmean;
				$sumsloperoll += $slopeminusmean * $rollminusmean;
				}
				
			$r = $sumsloperoll / sqrt($sumslopesq) / sqrt($sumrollsq);
			push(@r, $r);
#printf "%3d %4.2f %4.2f \n", $k, $timelag, $r;
			printf XFIL "%4.2f %4.2f \n", $timelag, $r;
			}
			
		# get max and closest peak cross correlations
		$maxr = 0.0;
		$peakr = 0.0;
		$peaktimelag = 0.0;
		for ($k = 0; $k < $ntimelag; $k++)
			{
			$timelag = -$timelagmax + $k * $timelagstep;
			if (@r[$k] > $maxr)
				{
				$maxr = @r[$k];
				$maxtimelag = $timelag;
				}
			if ($k == 0)
				{
				$peakr = @r[$k];
				$peaktimelag = $timelag;
				}
			elsif ($k < $ntimelag - 1
				&& @r[$k] > 0.0
				&& @r[$k] > @r[$k-1]
				&& @r[$k] > @r[$k+1]
				&& abs($timelag) < abs($peaktimelag))
				{
				$peakr = @r[$k];
				$peaktimelag = $timelag;
				}
			elsif ($k == $ntimelag - 1
				&& $peaktimelag == -$timelagmax
				&& @r[$k] > $peakr)
				{
				$peakr = @r[$k];
				$peaktimelag = $timelag;
				}
			}
		@r = ();
					
		# print out max and closest peak cross correlations
		if ($verbose > 0)
			{
printf "cross correlation pings %5d - %5d: max: %5.2f %4.2f  peak: %5.2f %4.2f\n",
$j0, $j1, $maxtimelag, $maxr, $peaktimelag, $peakr;
			}
		
		
		}
		
	}
	
# generate plot of cross correlation data
'mbm_xyplot -I$xcorrfile -M';

# clear arrays 
$nslope = 0;
@slope = ();
@slope_time_d = ();
$nroll = 0;
@roll = ();
@roll_time_d = ();

# close output files
close XFIL;

}

# Announce success whether it is deserved or not.
print "All done!\n";
exit 0;

#-----------------------------------------------------------------------
# This is included because not every perl installation has the library
# correctly installed
#
;# getopts.pl - a better getopt.pl

;# Usage:
;#      do Getopts('a:bc');  # -a takes arg. -b & -c not. Sets opt_* as a
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

1;
