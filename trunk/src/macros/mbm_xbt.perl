eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
                    & eval 'exec perl -S $0 $argv:q'
                         if 0;
#--------------------------------------------------------------------
#    The MB-system:	mbm_xbt.perl	6/18/93
#    $Id: mbm_xbt.perl,v 4.0 1994-10-21 11:47:31 caress Exp $
#
#    Copyright (c) 1993, 1994 by 
#    D. W. Caress (caress@lamont.ldgo.columbia.edu)
#    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
#    Lamont-Doherty Earth Observatory
#    Palisades, NY  10964
#
#    See README file for copying and redistribution conditions.
#--------------------------------------------------------------------
#
# Command:
#   mbm_xbt
#
# Purpose: 
#   Perl shellscript to translate Sparton XBT data or NBP MK12 XBT data
#   from depth and temperature into depth and sound velocity. Header lines are 
#   turned into comments beginning with '#' characters.
#   The output filename consists of the input filename with the
#   suffix ".sv".
#
# Usage:
#   mbm_xbt -Iinfile [-Ssalinity -Fformat -V -H]
#
# Known XBT formats:
#   Sparton  =  1
#   MK12     =  2
#
# Assumptions:
#   In the absence of salinity data, a salinity value of 35 ppt is
#   is assumed.  This will be significantly in error in some areas.
#   Latitude and depth (pressure) corrections are not applied.
#
#   Perl program to convert Sparton XBT data and MK12 XBT data into depth 
#   and Sound Velocity
# Dale Chayes
# 
# Authors:
#   Dale N. Chayes
#   David W. Caress
#   Lamont-Doherty Earth Observatory
#   Palisades, NY  10964
#
# Version:
# $Id: mbm_xbt.perl,v 4.0 1994-10-21 11:47:31 caress Exp $
#
# Revisions:
#   $Log: not supported by cvs2svn $
# Revision 4.1  1994/10/12  22:27:58  dale
# Add handling of Sippican MK12 XBT data files (by Dave Brock)
#
# Revision 4.0  1994/03/05  23:52:40  caress
# First cut at version 4.0
#
# Revision 4.1  1994/03/03  04:11:13  caress
# Fixed copyright message.
#
# Revision 4.0  1994/02/26  19:37:57  caress
# First cut at new version.
#
# Revision 1.7  1993/08/24  17:01:32  caress
# Fixed problems solved once earlier but not checked in.
#
# Revision 1.6  1993/08/17  16:58:36  caress
# Set location of perl to /usr/local/bin/perl
#
# Revision 1.5  1993/08/17  00:26:07  caress
# Version current as of 16 August 1993.
#
# Revision 1.4  1993/06/19  01:02:24  caress
# Combined functions of mbm_xbt and xbt2sv into a single
# Perl script.
#
#   Revision 1.3  1993/06/10  01:14:12  caress
#   Fixed bug in last improvement.
#
#   Revision 1.2  1993/06/10  01:08:40  caress
#   Now writes a more informative header.
#
#   Revision 1.1  1993/05/25  05:43:54  caress
#   Initial revision
#
#   Revision 3.1  1993/05/20  05:13:20  dale
#   Clean up the comments, fix the execution of xbt2sv
#
#   Revision 3.0  1993/05/20  03:49:35  dale
#   Initial version
#
#
$program_name = "mbm_xbt";

# indicates that we are still searching for end of header
$searchForMaker = "NO";

# indicates that we are still reading header records
$foundData = "NO";

# currently support formats
$SPARTON = 1;
$MK12    = 2;

# Deal with command line arguments
&Getopts('I:i:S:s:F:f:VvHh');
$file =     ($opt_I || $opt_i);
$salinity = ($opt_S || $opt_s);
$XBTtype =  ($opt_F || $opt_f || $SPARTON);
$help =     ($opt_H || $opt_h);
$verbose =  ($opt_V || $opt_v);

# print out help message if required
if ($help) {
   print "\n$program_name:\n";
   print "\nPerl shellscript to translate various XBT (with -F option) \n";
   print "data from depth and temperature into depth and sound velocity.\n";
   print "Header lines are turned into comments beginning with # characters.\n";
   print "The output filename consists of the input filename \n";
   print "with the suffix .sv\n";
   print "\nUsage: $program_name -Ifile [-Ssalinity -Fformat -V -H]\n";
   print "   Currently supported formats:\n";
   print "      XBT       Format Number\n";
   print "   --------------------------\n";
   print "    Sparton           1\n";
   print "    MK12              2\n\n";
   die "\n";
}

# set salinity if required
if (!$salinity)
	{
	$salinity = 35;
	}

# set degree conversion factor
$PI = 3.1415926;
$DTR = $PI/180.;

# Get filenames
$suffix = ".sv";
$OUTFILE = $file.$suffix;
$count=0;

# Open the input and output files
open(F,$file) || die "Cannot open input file: $file\n$program_name aborted.\n";
open(O,"+>".$OUTFILE) || die "Cannot open temporary file: $TMPFILE\n$program_name aborted.\n";

# Put leading comment in output file, depending on format selected
if ($XBTtype == $SPARTON) {
   print O "# Sparton XBT data processed using program $program_name\n";
} 
else {
   print O "# NBP XBT data processed using program $program_name\n";
}

# Loop over the input records
while (<F>) 
	{
        ++$count;
        # process the data set header depending on the format specified
        if ($XBTtype == $SPARTON) {
           &SpartonHeader();
        }
        else {
           &MK12header();
        }

	# once through the header fields, deal with the data
	if ( $foundData eq "YES")
		{
		# parse depth and temperature
		($depth, $temperature) = /(\S+)\s+(\S+)/;

		# Calculate water sound velocity using Medwin's (1975) equation
		# as referenced in Clay and Medwin's "Acoustical Oceanography",
		# page 88.  Temperature is in degrees centigrade, salinity in
		# parts per thousand (ppt), and depth in meters.
		$pressure = 0.016*depth;
		$velocity = 1449.2 + 4.6 * $temperature
			- 0.055 * $temperature * $temperature
			+ 0.00029 * $temperature * $temperature * $temperature
			+ 1.34 - (0.01 * $temperature * ($salinity - 35))
			+ 0.00000158 * $pressure;

		# get geoid latitude */
		$theta = $DTR*$latitude;
		$sine_theta = sin($theta);
		$sine_two_theta = sin(2.0*$theta);
		$geoid_latitude = 978.0309 
			+ 5.18552*$sine_theta*$sine_theta 
			 - 0.0057*$sine_two_theta*$sine_two_theta;

		# get depth for a given pressure 
		# as a function of latitude
		$d1 = (-3.434e-12*$depth[i] + 1.113e-07)*$depth[i] + 0.712953;
		$d2 = $d1*$depth[i] + 14190.7*log(1 + 1.83e-05*$depth[i]);
		$depth_pressure = 1000.*($d2/($geoid_latitude 
					+ 0.0001113*$depth[i]));
		if ($depth_pressure < 0.0)
			{
			$depth_pressure = -$depth_pressure;
			}
		if (($depth_pressure - $depth[i]) >= 0.5)
			{
			$pressure = $depth[i]*$depth[i]/$depth_pressure;
			}
		else
			{
			$pressure = $depth[i];
			}

		# Calculate water sound velocity using Wilson's October
		# 1960 formula
		$sm = $salinity - 35.0;
		$pkc = $pressure * 0.1019716 + 1.03323;
		$c00 = (((7.9851e-06 * temperature - 2.6054e-04)
			 * temperature - 0.044532)
			 * temperature + 4.5721)
			 * temperature + 1449.14;
		$c01 = (7.7711e-07 * temperature - 0.011244)
			 * temperature + 1.39799;
		$c10 = ((4.5283e-08 * temperature + 7.4812e-06)
			 * temperature - 1.8607e-04)
			 * temperature + 0.16027;
		$c11 = (1.579e-09 * temperature + 3.158e-08)
			 * temperature + 7.7016e-05;
		$c20 = (1.8563e-09 * temperature - 2.5294e-07)
			 * temperature + 1.0268e-05;
		$b0 =  (1.69202E-03 * sm + $c01) * sm + $c00;
		$b1 =  $c11 * sm + $c10;
		$b2 =  -1.2943E-07 * sm + c20;
		$b3 =  -1.9646E-10 * temperature + 3.5216E-09;
		$velocity2 =  (((-3.3603E-12 * $pkc + $b3) * $pkc + $b2)
			 * $pkc + $b1) * $pkc + $b0;



		# Output the result
		printf O "%.2f %.2f\n", $depth, $velocity;
		}
  
	}

print STDERR "Processed ", $count," lines of XBT data\n";

# Close the input and output files
close(F);
close(O);

# End it all
die "\nAll done!\n";

#-----------------------------------------------------------------------
# This should be loaded from the library but the shipboard installation
# of Perl is screwed up so....
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


#-----------------------------------------------------------------------
# SpartonHeader - deals with the Sparton Header, which is made up of
# 12 records 
#
# Usage:
#      SpartonHeader();  
#                       

sub SpartonHeader {

if (( $count == 1 ) && ( $_ ne "SOC BT/SV PROCESSOR\n"))
   {
   die "This is does not look like a SPARTON BT/SV data file $F: $!";
   }

   # Step one, insert comments in front of the gook.
if ( $count == 1 )
   {
   print O "# SOURCE:              ", $_;
   }
if ( $count == 2 )
   {
   print O "# DATE:                ", $_;
   }
if ( $count == 3 )
   {
   print O "# TIME:                ", $_;
   }
if ( $count == 4 )
   {
   print O "# UNKNOWN PARAMETER:   ", $_;
   }
if ( $count == 5 )
   {
   print O "# UNKNOWN PARAMETER:   ", $_;
   }
if ( $count == 6 )
   {
   print O "# SHIP:                ", $_;
   }
if ( $count == 7 )
   {
   print O "# CRUISE:              ", $_;
   }
if ( $count == 8 )
   {
   print O "# LATITUDE:            ", $_;
   }
if ( $count == 9 )
   {
   print O "# LONGITUDE:           ", $_;
   }
if ( $count == 10 )
   {
   print O "# DEPTH:               ", $_;
   }
if ( $count == 11 )
   {
   print O "# SPEED:               ", $_;
   }
if ( $count == 12 )
   {
   print O "# SURFACE TEMPERATURE: ", $_;
   }
if ( $count == 13 )
   {
   $foundData = "YES";
   }
}


#-----------------------------------------------------------------------
# MK12header - deals with the MK12 XBT Header, which is of variable
# length. Currently keying on the // depth field in the header record
#
# Usage:
#      MK12header();  
#                       
sub MK12header {

   # look for the "end of header" maker
#   if ( $_ =~ "// depth" || $searchForMaker eq "YES" ) {
   if ( $_ =~ " depth" || $searchForMaker eq "YES" ) {

      if ( $MKdataFound eq "NO" ) {
         # indicate that the end of header has been found
         # skip past the remaining header fields
         $_ = <F>;
         $_ = <F>;
      }

      $pattern = /(\d)/;
      if ( $pattern == 1 ) {
         print O "# ------------------------------------------\n";
         $foundData = "YES";
         $searchForMaker = "NO";
      }
      else {
         $searchForMaker = "YES";
      }
   }
   # if we have not reached the end of the header yet, write the
   # header info out as a comment to the output file
   elsif ( $foundData eq "NO" ) {
      print O "# ",$_;
   }
}
