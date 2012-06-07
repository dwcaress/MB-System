eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
                    & eval 'exec perl -S $0 $argv:q'
                         if 0;
#--------------------------------------------------------------------
#    The MB-system:	mbm_bpr.perl	5/13/2002
#    $Id$
#
#    Copyright (c) 2012-2012 by
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
#   mbm_bpr
#
# Purpose:
#   MB-System macro to process data from a Seabird SBE53 pressure
#   sensor into a tidal model for use by mbprocess
#
# Basic Usage:
#   mbm_bpr -Ifile -Zzone -Dellipsoid [-F -Q -H -V]
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
#   $Log: mbm_bpr.perl,v $
#   Initial revision.
#
use Getopt::Std;

my $ProgramName = "mbm_bpr";

# some default values
$tmode = 0;
$surfacepressure = 14.0;
$tidemin = 0.0;
$tidemax = 0.0;
$PI = 3.1415926;

# Deal with command line arguments using the "standard" perl library
if ( ! (getopts('D:d:HhI:i:O:o:R:r:TtVv'))) {
  print "$0: error on input\n";
  useage();
  exit 0;
}

my $help     = ($opt_H || $opt_h);
my $offset   = ($opt_D || $opt_d);
my $ifile    = ($opt_I || $opt_i);
my $ofile    = ($opt_O || $opt_o);
my $location = ($opt_R || $opt_r);
my $tmode    = ($opt_T || $opt_t);
my $verbose  = ($opt_V || $opt_v);

# print out help message if required
if ($help)
	{
	print "\n$ProgramName:\n";
	print "\nVersion: $Id$\n";
	print "\nMB-System macro to process data from a Seabird SBE53 pressure\n";
	print "sensor into a tidal model for use by mbprocess\n";
	print "\nUsage: \n";
	print "\t$Program_name -Ibprfile -Otidefile [-Doffset -T -H -V]\n";
	exit 0;
	}

# Get position if that has been specified
if ($location && $location =~ /^(\S+)\/(\S+)/) {
    ($longitude, $latitude) = $location =~ /^(\S+)\/(\S+)/;
}
else {
    $latitude = 0.0;
}

# Open the input and output files. Failure on either is fatal.
open(FI,$ifile) || die "Cannot open input file: $ifile\n$ProgramName aborted.\n";
open(FO,">".$ofile) || die "Cannot open output file: $ofile\n$ProgramName aborted.\n";

# Loop over the input records
$count = 0;
while ($line=<FI>) {		# reading from the input
  ($tcount, $month, $day, $year, $hour, $minute, $second, $pressure, $temperature)
  	= $line =~ /(\S+)\s+(\S+)\/(\S+)\/(\S+)\s+(\S+):(\S+):(\S+)\s+(\S+)\s+(\S+)/;

  # Use the first value as the surface pressure provided it is valid
  if ($tcount == 1 && $pressure < 99999.0) {
    $surfacepressure = $pressure;
  }

  # only use valid pressure values
  if ($pressure < 99999.0) {
    $time_d = `mbtime -T$year/$month/$day/$hour/$minute/$second`;
    chop($time_d);
    push (@time_ds, $time_d);

    push (@years, $year);
    push (@months, $month);
    push (@days, $day);
    push (@hours, $hour);
    push (@minutes, $minute);
    push (@seconds, $second);

    # Calculate the depth from pressure
    #
    # The following derives from Sea-Bird Electronics
    # Application Note number 69: Conversion of Pressure to Depth
    #	http://www.seabird.com/application_notes/AN69.htm
    #
    # Sea-Bird uses the formula in UNESCO Technical Papers in Marine Science No. 44.
    # This is an empirical formula that takes compressibility (that is, density) into account.
    # An ocean water column at 0 �C (t = 0) and 35 PSU (s = 35) is assumed.
    #
    # The gravity variation with latitude and pressure is computed as:
    #
    # g (m/sec2) = 9.780318 * [ 1.0 + ( 5.2788x10 -3  + 2.36x10 -5  * x) * x ] + 1.092x10 -6  * p
    #
    # where
    # x = [sin (latitude / 57.29578) ] 2
    # p = pressure (decibars)
    #
    # Then, depth is calculated from pressure:
    #
    # depth (meters) = [(((-1.82x10 -15  * p + 2.279x10 -10 ) * p - 2.2512x10 -5 ) * p + 9.72659) * p] / g
    #
    # where
    # p = pressure (decibars)
    # g = gravity (m/sec2)
    #
    # N. P. Fofonoff and R. C. Millard, Jr., ��Algorithms for computation of fundamental properties of seawater,�� Unesco Tech. Papers in Mar. Sci., No. 44 1983.
    #
    $pressuredbar = 0.6894757293 * ($pressure - $surfacepressure);
    $x = (sin($latitude * $PI / 180.0))**2;
    $g = 9.780318 * ( 1.0 + ( 5.2788e-3  + 2.36e-5  * $x) * $x ) + 1.092e-6  * $pressuredbar;
    $depth = ((((-1.82e-15  * $pressuredbar + 2.279e-10 ) * $pressuredbar - 2.2512e-5 ) * $pressuredbar + 9.72659) * $pressuredbar) / $g;

    push (@depths, $depth);
    $count++;
 }
}

close(FI);

# tell program status
if ($verbose) {
    print "\nProgram $program_name Status:\n";
    print "  $count pressure values read from $ifile\n";
    if ($offset) {
      print "  Depth offset: $offset m\n";
    }
    if ($location) {
      print "  Vertical reference to tidal model for position $longitude $latitude\n";
    } else {
      print "  Vertical reference will be the mean of the middle half of the data\n";
    }
    if ($tmode == 0) {
      print "  Tide will be output as <time_d tide> values\n";
      print "  A plot will be generated\n";
    }
   else {
      print "  Tide will be output as <yr mo da hr mi sc tide> values\n";
      print "  No plot will be generated\n";
    }
}

# If location specified then use mbotps to get a tidal model
# and then correct the bpr data to a tidal signal by
# subtracting the mean difference between the tidal model and
# the middle half of the data
if ($location && $count > 10) {
    # Execut mbotps to get tidal model for the specified location
    $i = $count / 4;
    $j = 3 * $count / 4;
    $interval = 20 * int(($time_ds[$j] - $time_ds[$i]) / ($j - $i));
    $start = "$years[0]/$months[0]/$days[0]/$hours[0]/$minutes[0]/$seconds[0]";
    $end = "$years[$count-1]/$months[$count-1]/$days[$count-1]/$hours[$count-1]/$minutes[$count-1]/$seconds[$count-1]";
    $modelfile = $ifile . "_tidemodel.txt";
    $cmd = "mbotps -A1 -D$interval -R$location -B$start -E$end -O$modelfile";
    if ($verbose) {
      print "  Executing: $cmd\n";
    }
    `$cmd`;

    $nmean = 0;
    $ntide = 0;
    $mean = 0.0;
    open(FI,$modelfile) || die "Cannot open temporary file: $modelfile\n$ProgramName aborted.\n";
    while ($line=<FI>) {		# reading from the input
      if ($line =~ /^\#.+/) {
      }
      else {
	($time_d, $tide) = $line =~ /(\S+)\s+(\S+)/;
	if ($ntide > $count / 80 && $ntide < 3 * $count / 80) {
	  $mean += $depths[$i + 20 * $nmean] - $tide;
	  $nmean++;
	}
	if ($tide < $tidemin) {
	  $tidemin = $tide;
	}
	if ($tide > $tidemax) {
	  $tidemax = $tide;
	}
	$ntide++;
      }
    }
    close(FI);
    $mean /= $nmean;
}

# Else if no location specified then correct to a tidal signal by
# subtracting the mean depth of the middle half of the data
# Calculate the mean depth for the middle half of the data
else {
    $nmean = 0;
    $mean = 0.0;
    for ($i = $count / 4; $i < 3 * $count / 4; $i++) {
	$mean += $depths[$i];
	$nmean++;
    }
    $mean /= $nmean;
}

# Output the tidal data corrected for the mean (or referenced to a model)
$ocount = 0;
for ($i = 0; $i < $count; $i++) {
  $tide = $depths[$i] - $mean + $offset;
  if (abs($tide) < 2.5) {
    if ($tmode == 0) {
      printf FO "$time_ds[$i] $tide\n";
    }
    else {
      printf FO "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d %f\n",
    		$years[$i], $months[$i], $days[$i], $hours[$i],
		$minutes[$i], $seconds[$i], $tide;
    }
  $ocount++;
  if ($tide < $tidemin) {
    $tidemin = $tide;
  }
  if ($tide > $tidemax) {
    $tidemax = $tide;
  }
  }
}

close(FO);

# tell program status
if ($verbose) {
    print "  $ocount pressure values output to $ofile\n";
    print "  Vertical reference: $mean m\n";
    }

# generate plot of tide observations (and model if location specified)
if ($tmode == 0) {
  $plotfile = $ofile . "_tideplot";
  if (abs($tidemin) > $tidemax) {
    $tidemax = abs($tidemin);
  }
  $tidemax *= 1.1;
  if ($ntide > 0) {
    $cmd = "mbm_xyplot -R$time_ds[0]/$time_ds[$count-1]/-$tidemax/$tidemax -IW0/0/0:$ofile -IW255/0/0:$modelfile -O$plotfile -L\"Tide Data from BPR <$ofile> (black) & Tide Model (red):Seconds:Tide (meters)\" -V";
  }
  else {
    $cmd = "mbm_xyplot -R$time_ds[0]/$time_ds[$count-1]/-$tidemax/$tidemax -IW0/0/0:$ofile -O$plotfile -L\"Tide Data from BPR <$ofile>:Seconds:Tide (meters)\" -V";
  }
  print "  Executing $cmd\n\n";
  $line = `$cmd`;
  print $line;
}

#-----------------------------------------------------------------------
