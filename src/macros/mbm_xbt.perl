eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
                    & eval 'exec perl -S $0 $argv:q'
                         if 0;
#--------------------------------------------------------------------
#    The MB-system: mbm_xbt.perl   6/18/93
#    $Id: mbm_xbt.perl,v 5.2 2004-09-16 19:14:49 caress Exp $
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
#   mbm_xbt
#
# Purpose: 
#   Perl shellscript to translate Sparton XBT data or NBP MK12 XBT data
#   from depth and temperature into depth and sound velocity. 
#   Sound Velocity is computed according to DelGrosso's equations.
#
#   We use the DelGrosso equation because of the results presented in 
#   Dusha, Brian D. Worcester, Peter F., Cornuelle, Bruce D., Howe, Bruce. M.
#   "On equations for the speed of sound in seawater", J. Acoust. Soc. Am. 
#   Vol 93, No 1, January 1993, pp 255-275.
#
#   Header lines are 
#   turned into comments beginning with '#' characters.
#   The output filename consists of the input filename with the
#   suffix ".sv".
#
# Usage:
#   mbm_xbt -Iinfile [-Fformat -Llatitude -Ssalinity -V -H]
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
# 
# Authors:
#   Dale N. Chayes
#   David W. Caress
#   Lamont-Doherty Earth Observatory
#   Palisades, NY  10964
#
#   David Brock of ASA added the Sippican MK12 header parsing code and switch
#
# Version:
# $Id: mbm_xbt.perl,v 5.2 2004-09-16 19:14:49 caress Exp $
#
# Revisions:
#   $Log: not supported by cvs2svn $
#   Revision 5.1  2003/04/17 20:42:48  caress
#   Release 5.0.beta30
#
#   Revision 5.0  2000/12/01 22:58:01  caress
#   First cut at Version 5.0.
#
# Revision 4.4  2000/10/03  21:42:17  caress
# Snapshot for Dale.
#
# Revision 4.3  1997/04/21  16:54:41  caress
# MB-System 4.5 Beta Release.
#
# Revision 4.3  1997/03/26  17:50:15  caress
# Fixed handling of MK12 xbt file when the first temperature
# value is less than zero.
#
# Revision 4.2  1995/05/12  17:43:23  caress
# Made exit status values consistent with Unix convention.
# 0: ok  nonzero: error
#
# Revision 4.1  1995/02/14  19:50:31  caress
# Version 4.2
#
# Revision 4.3  1995/01/23  10:20:14  dale
#    Move the latitude calculation out of the loop, we only need to
# do it once. Its now in the if/else test for the existance of a user
# supplied latitude. If a negative lat is provided, its converted to be
# positive.
#
# Revision 4.2  1995/01/23  10:16:10  dale
#    Corrected the output statement to only put out one velocity
# (instead of both.)
#
# Revision 4.1  1995/01/23  10:14:54  dale
#    Implemented SV calculation with DelGrosso's equation. Added
# argument for Latitude input. Buried code for Lovett's equation. Canned
# the old and not correct SV calculation code.
#
#    The DelGrosso code is the same as used in the Thermoalinograph
# code (r_tsg) on the Thompson and is based upon code from the Scripp's
# Ocean Data Facility (ODF).
#
#    This code does latitude density correction. The conversion of
# depth in meters to pressure (decibars) is done according to the
# equation presented in Coates.
#
# Revision 4.0  1995/01/23  03:21:58  dale
# This was the version that was distrubuted with MB 4.1
# It is known to have a number of problems and does not produce t
# either at the surface or at depth.
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

# minimum plausible seawater temperature - used for sanity check before output
$MINIMUMTEMPERATURE = -2.0;

# Deal with command line arguments
&Getopts('I:i:S:s:F:f:L:l:VvHh');
$file =     ($opt_I || $opt_i);
$sal = ($opt_S || $opt_s);
$XBTtype =  ($opt_F || $opt_f || $SPARTON);
$help =     ($opt_H || $opt_h);
$verbose =  ($opt_V || $opt_v);
$latitude = ($opt_L || $opt_l);    # pick up the Latitude for pressure conversion

# print out help message if required
if ($help) {
    print "\n$program_name:\n";    
    print "\nPerl shellscript to translate various XBT (with -F option) \n";
    print "data from depth and temperature into depth and sound velocity.\n";
    print "Header lines are turned into comments beginning with # characters.\n";
     print "The output filename consists of the input filename \n";
    print "with the suffix .sv\n";
    print "\nUsage: $program_name -Ifile [-Ssalinity -Fformat -Llatitude -V -H]\n";
    print "   Currently supported formats:\n";
    print "      XBT       Format Number\n";
    print "   --------------------------\n";
    print "    Sparton           1\n";
    print "    MK12              2\n\n";
    exit 0;
}                   # 
                    # 


if (!$sal) {                    # set salinity if it was defaulted 
    $sal = 35;
}

if (!$latitude) {        # if Lat not provided, use the equator
    $latitude = 0;
}
else {
    $latitude = $latitude * $DTR; # Lat comes in in degrees, but we need
}                   # radians for the sin() function.

if ($latitude < 0 ) {         # make sure the latitude is positive
    $latitude = $latitude * -1.;
}

# set degree conversion factor
$PI = 3.1415926;
$DTR = $PI/180.;

# Get filenames
$suffix = ".sv";
$OUTFILE = $file.$suffix;
$count=0;

# Open the input and output files
open(F,$file) || die "Cannot open input file: $file\n$program_name 
aborted.\n";
open(O,"+>".$OUTFILE) || die "Cannot open temporary file: 
$TMPFILE\n$program_name aborted.\n";

# Put leading comment in output file, depending on format selected
if ($XBTtype == $SPARTON) {
   print O "# Sparton XBT data processed using program $program_name\n";
} 
else {
   print O "# NBP XBT data processed using program $program_name\n";
}

# Loop over the input records
while (<F>) {
    ++$count;            
    # process the data set header depending on the format specified
                    # The headder reading processes return when
                    # they have consumed all the headder data
    if ($XBTtype == $SPARTON) {    
     &SpartonHeader();
    }
    else {
     &MK12header();
    }

     # once through the header fields, deal with the data
     if ( $foundData eq "YES") {
      # parse depth and temperature
      ($depth, $temp) = /(\S+)\s+(\S+)/;
 
# First calculate the Pressure term (P) in decibars from depth in meters
# This conversion is from Coates, 1989, Page 4.

     $P=1.0052405 * ( 1+ 5.28E-3 * ((sin ($latitude)) 
                    * (sin($latitude))))
               * $depth + 2.36E-6 * ($depth * $depth);

# Then calculate SV according to DelGrosso

      $P1      = $P * 0.1019716;   # to pressure in kg/cm**2 gauge 
      $c0      = 1402.392;         

      $dltact  = $temp*( 0.501109398873E1 + $temp*(-0.550946843172E-1+ $temp
                 *  0.221535969240E-3));

      $dltacs  = $sal*(0.132952290781E1 
                 + $sal* 0.128955756844E-3);
      
      $dltacp  = $P1*( 0.156059257041E0 +  $P1*( 0.24499868841E-4 
                  + $P1*(-0.883392332513E-8)));
     $dcstp   =  $temp*(-0.127562783426E-1*$sal      + 
             $P1*( 0.635191613389E-2        +
             $P1*( 0.265484716608E-7*$temp      +
                -0.159349479045E-5        +
                 0.522116437235E-9*$P1)     +
               (-0.438031096213E-6)*$temp*$temp)) +
               $sal*((-0.161674495909E-8)*$sal*$P1*$P1 +
             $temp*( 0.968403156410E-4*$temp      +
             $P1*( 0.485639620015E-5*$sal      +
               (-0.340597039004E-3))));

      $velocity = $c0 + $dltact + $dltacs + $dltacp + $dcstp;

# Lovett:  This commented out section of code does the calculation per
# Lovett:  Lovett: Lovett, J. R. Merged Sea-Water Sound Speed Equations
# Lovett:  J. Acoust. Soc. Am. Vol 63, No. 6, 1978, pp. 1713-1718.
# Lovett:  as presented by Coates on Page 4.
# Lovett:
# Lovett: # Set up the coeficients for the calculation
# Lovett: 
# Lovett:     $K1  =  5.01132;
# Lovett:     $K2  = -1.26628E-2;
# Lovett:     $K3  =  2.062107E-8;
# Lovett: 
# Lovett:     $K4  = -1.052396E-8;
# Lovett:     $K5  = -5.513036E-2;
# Lovett:     $K6  =  9.543664E-5;
# Lovett: 
# Lovett:     $K7  =  2.221008E-4;
# Lovett:     $K8  =  1.332947;
# Lovett:     $K9  =  1.605336E-2;
# Lovett:  
# Lovett:     $K10 =  2.12448E-7;
# Lovett:     $K11 =  2.183988E-13;
# Lovett:     $K12 = -2.253828E-13;
# Lovett: 
# Lovett: # Calculate SV according to Lovett per Coates, page 4.
# Lovett:                      
# Lovett:      $velocity2 = 1402.394 +
# Lovett:          $temp * ($K1 + $sal *  ($K2 + $K3 * $sal * $P)
# Lovett:           + $K4 * ($P * $P)   
# Lovett:               + $temp *(($K5 +$K6 * $sal) +$K7 *temp) )
# Lovett:                + $K8 * $sal   
# Lovett:                    + $P * ($K9 
# Lovett:               +$P * ($K10+$P*($K11*$temp + $K12 * $sal)));
# Lovett:      
     
     # Output the result
     if ($temp > $MINIMUMTEMPERATURE)
	    {
	    printf O "%.2f %.2f\n", $depth, $velocity;
	    }
     else
            {
            printf STDERR "Error: data (%.2f %.2f) ignored because temperature %.2f < minimum %.2f\n",
	    			$depth, $velocity, $temp, $MINIMUMTEMPERATURE;
	    }
    }
    
}
# 
print STDERR "Processed ", $count," lines of XBT data\n";

# Close the input and output files
close(F);
close(O);

# End it all
print "\nAll done!\n";
exit 0;

#-----------------------------------------------------------------------
# This should be loaded from the library but its safer to
# just include it....
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
# length. This routine parses the incomming data and watches for a
# numeric pattern of ##### ###### to indicate the start of data.
#
# Usage:
#      MK12header();  
#                       
sub MK12header {

   # parse out the depth and temperature values from the input
   ($depth, $temperature) = /(\S+)\s+(\S+)/;

   # get the numeric ASCII value of the first character in the depth
   # and temperature variables
   $depAscii = ord($depth);
   $tempAscii = ord($temperature);

   # check to see if these numeric ASCII values are numerals (ASCII 0
   # is 48, and ASCII 9 is 57). The both these values are numbers,
   # then we have found the start of the header
   if ( ($depAscii > 47 && $depAscii < 58) && 
        (($tempAscii > 47 && $tempAscii < 58) || $tempAscii == 45) ) {
      $foundData = "YES";
   }

}

