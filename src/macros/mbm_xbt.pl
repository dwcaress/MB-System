eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
                    & eval 'exec perl -S $0 $argv:q'
                         if 0;
#--------------------------------------------------------------------
#    The MB-system: mbm_xbt.perl   6/18/93
#    $Id$
#
#    Copyright (c) 1993-2012 by 
#    D. W. Caress (caress@mbari.org)
#      Monterey Bay Aquarium Research Institute
#      Moss Landing, CA
#    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
#      Lamont-Doherty Earth Observatory of Columbia University
#      Palisades, NY  10964
#
#    See README file in the top level of the MB-System distribution 
#    directory for copying and redistribution conditions.
#--------------------------------------------------------------------
#
# Command:
#   mbm_xbt
#
# Purpose: 
#   Perl code to translate Sparton XBT data or Sippican (MK12 or MK21)
#   XBT data from depth and temperature into depth and sound speed.
#   Sound speed is computed according to DelGrosso's equations.
#
#   We use the DelGrosso equation because of the results presented in 
#   Dusha, Brian D. Worcester, Peter F., Cornuelle, Bruce D., Howe, Bruce. M.
#   "On equations for the speed of sound in seawater", J. Acoust. Soc. Am.
#   Vol 93, No 1, January 1993, pp 255-275.
#
#   Header lines are turned into comments beginning with '#' characters.
#   The output filename consists of the input filename with the
#   suffix ".sv".
#
# Usage:
#   mbm_xbt -Iinfile [-Fformat -Llatitude -Ssalinity -V -H -C]
#
# Known XBT formats:
#   Sparton        =  1
#   Sippican MK12  =  2
#   Sippican MK21  = 3
#
# Assumptions:
#   In the absence of salinity data, a salinity value of 35 ppt is
#   is assumed.  This will be significantly in error in some areas.
#   Latitude and depth (pressure) corrections are not applied.
#
#   The first column (after the header) is water depth (meters)
#   The second column (after the header) is temperature (Celcius)
#
#   Perl program to convert Sparton XBT data and MK12 XBT data into depth 
#   and Sound Speed
# 
# Authors:
#   Dale N. Chayes <dale@ldeo.columbia.edu>
#   David W. Caress
#   Lamont-Doherty Earth Observatory of Columbia University
#   Palisades, NY  10964
#
#   David Brock while working for Antarctic Support Associates (ASA)
#    when ASA was the contractor for the US Antarctic Research Program
#    added the Sippican MK12 header parsing code and switch at the time
#    (1993 or so?) when the SeaBeam 2112 was instaled on the Nathaniel
#    B. Palmer.
#
# Notes:
#    Major re-write by Dale started in early August,2003, completed in 
#    September 2004 including:
#      - adding explicit filehandle passing for header parsing
#      - restructure for clearer header parsing
#      - explicit input variable naming
#      - clean up formatting
#      - improve test for below zero (C) temperature handling
#      - add support for Sippican MK21 header format
#      - expanded mbm_xbt header info
#      - copy all source header info into the output file
#      - convert to use "standard" perl getopts
#      - move useage message to function
#      
#
# Version:
# $Id$
#
# Revisions:
#   $Log: mbm_xbt.perl,v $
#   Revision 5.5  2004/09/18 00:36:10  dale
#   Update copyright date, add CU to LDEO and change one more instance of velocity.
#
#   Revision 5.3  2004/09/17 19:47:44  dale
#   This is a major revision to mbm_xbt. It adds new functionality for Sippican MK21 file format, addresses sub-zero temperatures better and is significantly cleaned up. See the comments.
#
#   Revision 1.1.1.1  2003/07/27 17:17:35  dale
#   mb import from 5b31
#
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

# use strict;
use Getopt::Std;		# break from the old MB-System included version



my $ProgramName = "mbm_xbt";
my $Version ="\$Id\$";

my $PI = 3.1415926;	 # (probably) good enough definition fo Pi
my $DTR = $PI/180.;     # Degree/radians conversion factor

my $ColdLimit = -2.0;		# Normal water can't get colder without freezing

# indicates that we are still searching for end of header
my $searchForMaker = "NO";

# indicates that we are still reading header records
my $foundData = "NO";

# Currently supported input file formats
my $SPARTON = 1;
my $MK12    = 2;
my $MK21    = 3;


# Deal with command line arguments using the "standard" perl library

if ( ! (getopts('I:i:S:s:F:f:L:l:VvHh'))) {
  print "$0: error on input\n";
  useage();
  exit 0;
}

my $file =     ($opt_I || $opt_i);
my $sal = ($opt_S || $opt_s);
my $XBTtype =  ($opt_F || $opt_f || $MK12);
my $help =     ($opt_H || $opt_h);
my $verbose =  ($opt_V || $opt_v);
my $latitude = ($opt_L || $opt_l); # Latitude for pressure conversion


# print out help message if requested
if ($help) {
  useage();
  exit 0;
}

if (!$sal) {	# set salinity if it was defaulted 
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

# Filenames: there are two: input and output
my $suffix = ".sv";
my $OUTFILE = $file.$suffix;
my $count=0;
my $temp;		 # Temperature (Celcius)
my $depth;		 # Water depth (meters)

# Open the input and output files. Failure on either is fatal.
open(F,$file) || die "Cannot open input file: $file\n$ProgramName 
aborted.\n";

open(Out,"+>".$OUTFILE) || die "Cannot open temporary file: 
$TMPFILE\n$ProgramName aborted.\n";

# Put leading comment in output file, depending on format selected
if ($XBTtype == $SPARTON) {
    print Out "# Sparton XBT data processed using program $ProgramName\n";
} 
elsif ($XBTtype == $MK12) {
    print Out "# Sippican MK12  XBT data processed using program $ProgramName\n";
}

elsif ($XBTtype == $MK21) {
  print Out "# Sippican MK21 XBT data processed using program $ProgramName\n";
}

else {
    printf("mbm_xbt: failed on unknown input file format.\n");
    printf("\t If you have a new kind of XBT file, please let us know.\n");
    exit;
}

# Add some content to the output header
#

printf (Out "# %s version: %s\n",$ProgramName, $Version );
printf (Out "# %s assumed %.2f PSU salinity \n",$ProgramName, $sal);
printf (Out "# --end of mbm_xbt header info -----\n");

# Deal with the data set header depending on the format 
# specified on the command line.  The headder reading processes returns when
# it has consumed all the header data. All of the input header info is
# inserted into the output file as comments.

if ($XBTtype == $SPARTON) {    	# We've gotten rid of _@ passing!
    SpartonHeader(F, Out);
}
elsif ($XBTtype == $MK21) {
    printf ("Invoking MK21header\n");
    MK21header(F, Out);
}
else {				# make into an elseif and add a failure case
    MK12header(F, Out);
}

# Now that the header stuff is out of the way, loop over the input records
while ($In=<F>) {		# reading from the input
  ++$count;
  if (length($In) >= 7) {
                  # Some folks create "edf" files with additional columns so
                  # we just take the first two and assume that depth is in
                  # the first column and temperature is in the second,
                  # ignore all after that.

    ($depth, $temp, $junk) = split (" ", $In, 3);

#	if ($XBTtype == $MK21) {
#	    ($depth, $temp) = split (/\s+/, $In, 2);
#	}
#	elsif ($XBTtype == $MK12) {
#	    ($depth, $temp) = split (" ", $In, 2);
#	}
	
 
# First calculate the Pressure term (P) in decibars from depth in meters
# This conversion is from Coates, 1989, Page 4.
	
    my $P=1.0052405 
      * ( 1+ 5.28E-3 * ((sin ($latitude)) * (sin($latitude))))
	* $depth + 2.36E-6 * ($depth * $depth);
	
# Then calculate SV according to DelGrosso

    my $P1      = $P * 0.1019716;   # to pressure in kg/cm**2 gauge
    my $c0      = 1402.392;	
    my $dltact  = $temp
      *( 0.501109398873E1 + $temp*(-0.550946843172E-1+ $temp
				   *  0.221535969240E-3));

    my $dltacs  = $sal*(0.132952290781E1 
			+ $sal* 0.128955756844E-3);
    my $dltacp  = $P1 
      *( 0.156059257041E0 +  $P1*( 0.24499868841E-4 
				   + $P1*(-0.883392332513E-8)));

    my $dcstp   =  $temp*(-0.127562783426E-1*$sal
			  + $P1*( 0.635191613389E-2 
				  + $P1*( 0.265484716608E-7*$temp +
					  -0.159349479045E-5        +
					  0.522116437235E-9*$P1)     +
				  (-0.438031096213E-6)*$temp*$temp)) +
				    $sal*((-0.161674495909E-8)*$sal*$P1*$P1 +
					  $temp*( 0.968403156410E-4*$temp +
						  $P1*( 0.485639620015E-5*$sal +
							(-0.340597039004E-3))));
	
    my $velocity = $c0 + $dltact + $dltacs + $dltacp + $dcstp;

				# Output the result if it's "okay"
    if ($temp > $ColdLimit)   {
      printf Out "%.2f %.2f\n", $depth, $velocity;
    }
  }				# end of (too) simple input length check
}

#
print STDERR "Processed ", $count," lines of XBT data\n";

# Close the input and output files
close(F);
close(Out);

# End it all
print "\nAll done!\n";
exit 0;


#----------------------------
# useage();
sub useage {

 
    print "\nUsage: $ProgramName -Ifile \n";
    print "\nVersion: $Id$\n";
    print "\t[-C -Ssalinity -Fformat -Llatitude -V -H -C]\n\n";

    print "\tPerl shellscript to translate various XBT (with -F option) \n";
    print "\tdata from depth and temperature into depth and sound speed.\n";
    print "\tHeader lines are turned into comments (#) characters.\n";
    print "\tThe output filename consists of the input filename \n";
    print "\twith the suffix .sv\n";
    print "\tIF you have a decent AVERAGE saliity, use -S salinity...\n\n";
    print "   Currently supported input formats:\n";
    print "      XBT       Format Number\n";
    print "   --------------------------------------------------\n";
    print "    Sparton                                       1\n";
    print "    Sippican MK12 export data format              2\n";
    print "    Sippican MK21 (edf)                           3\n";
    print "\n";
}				# End of useage()



#-----------------------------------------------------------------------
# SpartonHeader - deals with the Sparton Header, which is made up of
# 12 records
#
# Usage:
#      SpartonHeader(FileHandleIn, FileHandleOUt);
#

sub SpartonHeader (*$*$;) {
   my $InFileHandle = shift;	# get the input file handle
   my $Out = shift;	#  and the output file handle
   my $Done=0;
   my $count=0;
   while  ( $Done == 0) {
       $In=<$InFileHandle>;
       chomp($In);			# dump the end of line stuff
       $count++;
	
       if (( $count == 1 ) && ( $In !=~ "SOC BT/SV PROCESSOR")) {
	   die "This is does not look like a SPARTON BT/SV data file $F: $!";
       }

   # Step one, insert comments in front of the output data
       if ( $count == 1 ) {
	   printf ( $Out "# SOURCE:   %s\n", $In);
       }
       if ( $count == 2 ) {
	   printf ($Out "# DATE:      %s\n", $In);
       }
       if ( $count == 3 ) {
	   printf ($Out "# TIME:      %s\n ", $In);
       }
       if ( $count == 4 ) {
	   printf ($Out "# UNKNOWN:   %s\n ", $In);
       }
       if ( $count == 5 ) {
	   printf ($Out "# UNKNOWN:   %s\n", $In);
       }
       if ( $count == 6 ) {
	   printf ($Out "# SHIP:      %s\n", $In);
       }
       if ( $count == 7 ) {
	   printf ($Out "# CRUISE:    %s\n", $In);
       }
       if ( $count == 8 ) {
	   printf ($Out "# LATITUDE:  %s\n", $In);
       }
       if ( $count == 9 ) {
	   printf ($Out "# LONGITUDE: %s\n", $In);
       }
       if ( $count == 10 ) {
	   printf ($Out "# DEPTH:     %s\n ", $In);
       }
       if ( $count == 11 ) {
	   printf ($Out "# SPEED:     %s\n", $In);
       }
       if ( $count == 12 ) {
	   printf ($Out "# SURF TEMP: %s\n", $In);
       }
       if ( $count == 13 )  {
	   $Done=1;
       }
   }


#-----------------------------------------------------------------------
# MK12header - deals with the MK12 XBT Header, which is of variable
# length. This routine parses the incomming data and watches for a
# numeric pattern of ##### ###### to indicate the start of data.
#
# Usage:
#      MK12header(InFileHandler, OutFileHandler);
#
sub MK12header(*$*$;) {
  my $InFileHandle = shift;	# get the input file handle
  my $OutFileHandle = shift;	#  and the output file handle
  my $Done=0;
  while  ( $Done == 0) {
    $In=<$InFileHandle>;
    chomp($In);			# dump the end of line stuff
    printf $OutFileHandle "# %s\n", $In; # copy header info to output file
	
    if (substr($In,0,1) eq "\t" ) {
      $Done=1;
    }
  }
}			 # End of MK12header()


#---------------------------------------------------------------
# MK21header
# Code to parse a Sippican Mark 21 headder of and EDF file. This headder
# seems to be a bit more organized than the MK12 version.....
# It assumes that:
#     1) the last line of the header starts with "Depth"
#     2) the first column of data is depth in meters
#     3) the next column is temperature in degress
#     4) There may be other columns
#
# 
sub MK21header(*$*$;) {		# Two HANDLES required
    my $InFileHandle = shift;	# get the input file handle
    my $OutFileHandle = shift;	#  and the output file handle
    my $Done=0;
    while  ( $Done == 0) {
      $In=<$InFileHandle>;	# get one line from the file
      chomp($In);		# clean up the end of line

                                # copy header info to output file as comment
      printf $OutFileHandle "# %s\n", $In;

	if ($In =~ 'Depth \(m\)') {
#	if ($In =~ 'Depth (m)') {
#	    printf("End of header\n\n");
	    $Done=1;
	}
    }
}		# end of MK21header

}				# End of this file

