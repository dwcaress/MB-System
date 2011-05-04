eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
                    & eval 'exec perl -S $0 $argv:q'
                         if 0;
#--------------------------------------------------------------------
#    The MB-system:	mbm_stat.perl	8/14/93
#    $Id$
#
#    Copyright (c) 1993-2011 by 
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
#   mbm_stat
#
# Purpose:
#   Perl shellscript to extract beam statistics from mbinfo output.
#
# Usage:
#   mbm_stat -Ifile [-V -H]
#
# Author:
#   David W. Caress
#   Lamont-Doherty Earth Observatory
#   Palisades, NY  10964
#   August 14, 1993
#
# Version:
#   $Id$
#
# Revisions:
#   $Log: mbm_stat.perl,v $
#   Revision 5.1  2003/04/17 20:42:48  caress
#   Release 5.0.beta30
#
#   Revision 5.0  2000/12/01 22:58:01  caress
#   First cut at Version 5.0.
#
# Revision 4.3  2000/10/03  21:42:17  caress
# Snapshot for Dale.
#
# Revision 4.2  1997/04/21  16:54:41  caress
# MB-System 4.5 Beta Release.
#
# Revision 4.1  1995/05/12  17:43:23  caress
# Made exit status values consistent with Unix convention.
# 0: ok  nonzero: error
#
# Revision 4.1  1995/05/12  17:43:23  caress
# Made exit status values consistent with Unix convention.
# 0: ok  nonzero: error
#
# Revision 4.0  1994/10/21  11:47:31  caress
# Release V4.0
#
# Revision 1.1  1993/08/19  16:39:20  dale
# Initial revision
#
#
#
$program_name = "mbm_stat";

# Deal with command line arguments
&Getopts('I:i:VvHh');
$file =    ($opt_I || $opt_i);
$help =    ($opt_H || $opt_h);
$verbose = ($opt_V || $opt_v);

# print out help message if required
if ($help)
	{
	print "\n$program_name:\n";
	print "\nVersion: $Id$\n";
	print "\nPerl shellscript to extract beam statistics from the ouput of mbinfo.\n";
	print "\nUsage: $program_name -Ifile [-V -H]\n";
	exit 0;
	}

# check for defined parameters
if (!$file)
	{
	die "No input file specified - $program_name aborted\n";
	}

# Read and deal with the data
open(F1,"$file") || die "$program_name: Can't find input file $file \n";
$count = 0;

if ($verbose)
	{
	print "\nProgram ",$program_name,"\n";
	print "\nInput File: ",$file,"\n";
	}
printf("File\t\t\tHours\t  Drops\t  Flags\n");
$done = 0;

while (<F1>)
	{
		$count++;
		if (/^Multibeam Data File:  (.*)/) {
			$data_file = $1;
			@parts=split(/\//,$data_file);
			$file = @parts[scalar(@parts)-1];
			$done = 0;
			}


		elsif (/^  Number of Beams:(.*)/) {
			if ( $done == 0 ) {
				$beams = $1;
				}
			}	

		elsif (/^  Number of Zero Beams:(.*)/) {
			if ( $done == 0)
				{
				$zeros = $1;
				}
			}
		elsif (/^  Number of flagged beams:(.*)/) {
			if ( $done == 0)
				{
				$flags = $1;
				$done = 1;
				}
			}
		elsif (/^Total Time: (.*)/) {
				$duration = $1;
				printf("%s\t%7.4f\t%6.2f\t%6.2f\n", $file, 
				$duration, $zeros/$beams*100., 
				$flags/$beams*100.);
			}	

#	if ($done == 1) {
#		printf("%s\t%7.4f\t%6.2f\t%6.2f\n", $file, $duration, 
#			$zeros/$beams*100., $flags/$beams*100.);
#	$done = 0;
#	}	

	if ( $verbose )
		{
		printf("%4d\tData file:\t%s\n",$count,$data_file);
		printf("%6d\t%6d\t%6d\n", $beams, $zeros, $flags);
		}

}


close(F1);

# Announce success whether it is deserved or not.
print "\nAll done\n";
exit 0;

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

1;
