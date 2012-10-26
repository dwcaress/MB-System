eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}' 
					& eval 'exec perl -S $0 $argv:q'
					  if 0;

#--------------------------------------------------------------------
#    The MB-system:	mbm_datalist.perl	2010-08-11
#    $Id: mbm_datalist.pl 1792 2009-11-13 01:01:35Z gkeith $
#
#    Copyright (c) 2010 by
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
#   mbm_datalist
#
# Purpose:
#   Macro to select a subset of a datalist based on time and region.
#   Basically this adds the -B and -E options to mbdatalist.
#
# Basic Usage:
#   mbm_datalist -Ifilelist
#
# Additional Options:
#             [-Byyyy/mm/dd/hh/mm/ss -Eyyyy/mm/dd/hh/mm/ss
#             -Rwest/east/south/north -V]
#
# Author:
#   Gordon J Keith
#   CSIRO Marine and Atmospheric Research
#   Battery Point, TAS 7001
#   11 August, 2010
#
# Version:
#   $Id: mbm_datalist.pl 1792 2009-11-13 01:01:35Z gkeith $
#
# Revisions:
#   Initial version based on mbm_grid.pl
#
$program_name = "mbm_datalist";

# Deal with command line arguments
&MBGetopts('B:b:E:e:F:f:HhI:i:R:r:Vv');

$start     = ( $opt_B || $opt_b );
$fin       = ( $opt_E || $opt_e );
$format    = ( $opt_F || $opt_f || "-1" );
$help      = ( $opt_H || $opt_h );
$file_data = ( $opt_I || $opt_i || "datalist.mb-1" );
$bounds    = ( $opt_R || $opt_r );
$verbose   = ( $opt_V || $opt_v );

# print out help message if required
if ($help) {
	print "\n$program_name:\n";
	print
	  '\nVersion: $Id: mbm_datalist.pl 1792 2009-11-13 01:01:35Z gkeith $\n';
	print "Macro to select a subset of a datalist based on time and region.\n";
	print "\nBasic Usage:\n\t$program_name -Ifilelist  \n";
	print "\nAdditional Options:\n\t-Rwest/east/south/north \n";
	print "         -Byyyy/mm/dd/hh/mm/ss -Eyyyy/mm/dd/hh/mm/ss \n";
	exit 0;
}

# check for input file
if ( !$file_data ) {
	print "\a";
	die "No input file specified - $program_name aborted\n";
}
elsif ( !-e $file_data ) {
	print "\a";
	die
"Specified input file $file_data cannot be opened - $program_name aborted\n";
}

# tell the world we got started
if ($verbose) {
	warn "\nRunning $program_name...\n";
}

# set up user defined geographic limits
$xmin = -360;
$xmax = 360;
$ymin = -90;
$ymax = 90;

if ( $bounds =~ /^\S+\/\S+\/\S+\/\S+$/ ) {
	( $xmin_raw, $xmax_raw, $ymin_raw, $ymax_raw ) =
	  $bounds =~ /(\S+)\/(\S+)\/(\S+)\/(\S+)/;
	$xmin = &GetDecimalDegrees($xmin_raw);
	$xmax = &GetDecimalDegrees($xmax_raw);
	$ymin = &GetDecimalDegrees($ymin_raw);
	$ymax = &GetDecimalDegrees($ymax_raw);
}

$begin = $start || "0";
$end = $fin || "9";

checkFile( $file_data, $format );

#-----------------------------------------------------------------------
sub min {

	# make local variables
	local ($min);

	# get the minimum of the arguments
	if ( $_[0] < $_[1] ) {
		$min = $_[0];
	}
	else {
		$min = $_[1];
	}
	$min;
}

#-----------------------------------------------------------------------
sub max {

	# make local variables
	local ($max);

	# get the minimum of the arguments
	if ( $_[0] > $_[1] ) {
		$max = $_[0];
	}
	else {
		$max = $_[1];
	}
	$max;
}

#-----------------------------------------------------------------------
sub checkFile {
	my ( $file_mb, $file_format ) = @_;

	warn "checking " . $file_mb . " " . $file_format if ($verbose);

	# use .inf file if it exists and no time or space bounds applied
	my $use_file = 1;
	my $file_inf = $file_mb . ".inf";
	my @mbinfo   = 0;
	my $line;

	if ( -r $file_inf ) {
		if ( open( FILEINF, "<$file_inf" ) ) {
			@mbinfo = <FILEINF>;
			close FILEINF;
		}
		$use_file = checkInfo(@mbinfo);
	}

	if ( $use_file > 0 ) {
		if ( $file_format >= 0 ) {
			if ( $use_file == 1 ) {
				my $flags = "-G ";
				$flags .= "-R$bounds " if $bounds;
				$flags .= "-B$start " if $start;
				$flags .= "-E$fin " if $fin;
				warn "mbinfo -F$file_format -I$file_mb $flags" if ($verbose);
				@mbinfo = `mbinfo -F$file_format -I$file_mb $flags`;
				$use_file = checkInfo(@mbinfo);
				if ( $use_file > 0 ) {
					print "$file_mb \t$file_format\n";
				}

			}
			else {
				print "$file_mb \t$file_format\n";
			}
		}

		else {
			if ( open( DATALIST, "<$file_mb" ) ) {	
				my @datalist = <DATALIST>;
				close DATALIST; # This sub is recursive and I'm not sure of handle scope.
							
				while (@datalist) {
					my $line = shift @datalist;
					if ( $line =~ /^[^#\s]/ ) {
						my $file;
						my $format = 0;
						if ( $line =~ /^"([^"]*)"\s+(-?\d+)/ ) {
							$file   = $1;
							$format = $2;
						}
						elsif ( $line =~ /^(\S*)\s+(-?\d+)/ ) {
							$file   = $1;
							$format = $2;
						}
						else {
							($file) = $line =~ /(\S*)/;
						}
						if ( $file =~ /^[^\/]/ ) {
							my ($dir) = $file_mb =~ /^(.*)\/[^\/]*$/;
							$file = $dir . "/" . $file;
						}
						checkFile( $file, $format );
					}
				}
			}
			else {
				warn "Unable to open $file_mb";
			}
		}
	}
}

#-----------------------------------------------------------------------
sub checkInfo {
	my @mbinfo  = @_;
	my $retval  = 0;
	my $xmin_f  = "NaN";
	my $xmax_f  = "NaN";
	my $ymin_f  = "NaN";
	my $ymax_f  = "NaN";
	my $begin_f = "9";
	my $end_f   = "0";
	my $nrec_f  = 0;
	my $debug   = "-B$begin -E$end -R$xmin/$xmax/$ymin/$ymax";

	while (@mbinfo) {
		my $line = shift @mbinfo;
		if ( $line =~ /^Number of Records:\s+(\S+)/ ) {
			($nrec_f) = $line =~ /^Number of Records:\s+(\S+)/;
		}
		if ( $line =~
			/^Minimum Longitude:\s+(\S+)\s+Maximum Longitude:\s+(\S+)/ )
		{
			( $xmin_f, $xmax_f ) = $line =~
			  /^Minimum Longitude:\s+(\S+)\s+Maximum Longitude:\s+(\S+)/;
		}
		if ( $line =~ /^Minimum Latitude:\s+(\S+)\s+Maximum Latitude:\s+(\S+)/ )
		{
			( $ymin_f, $ymax_f ) =
			  $line =~ /^Minimum Latitude:\s+(\S+)\s+Maximum Latitude:\s+(\S+)/;
		}
		if ( $line =~ /^Start of Data:/ ) {
			$line = shift @mbinfo;
			if ( $line =~ /^Time:/ ) {
				my ( $month, $day, $year, $hour, $min, $sec ) =
				  $line =~ /^Time:\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+):(\d+):(\d+[.]?[\d]*)/;
				$begin_f =
				    $year . "/" 
				  . $month . "/" 
				  . $day . "/" 
				  . $hour . "/" 
				  . $min . "/"
				  . $sec;
			}
		}
		if ( $line =~ /^End of Data:/ ) {
			$line = shift @mbinfo;
			if ( $line =~ /^Time:/ ) {
				my ( $month, $day, $year, $hour, $min, $sec ) =
				  $line =~ /^Time:\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+):(\d+):(\d+[.]?[\d]*)/;
				$end_f =
				    $year . "/" 
				  . $month . "/" 
				  . $day . "/" 
				  . $hour . "/" 
				  . $min . "/"
				  . $sec;
			}
		}
	}

	if (   $nrec_f == 0
		|| $begin_f ge $end
		|| $end_f le $begin
		|| $xmin_f > $xmax
		|| $xmax_f < $xmin
		|| $ymin_f > $ymax
		|| $ymax_f < $ymin )
	{
		$use_file = 0;    # distinct
	}
	elsif ($begin_f lt $begin
		|| $end_f gt $end
		|| $xmin_f < $xmin
		|| $xmax_f > $xmax
		|| $ymin_f < $ymin
		|| $ymax_f > $ymax )
	{
		$use_file = 1;    # overlap
	}
	else {
		$use_file = 2;    # subset
	}

	warn $use_file if ($verbose);

	return $use_file;
}

#-----------------------------------------------------------------------
sub GetDecimalDegrees {

	# make local variables
	local ( $dec_degrees, $degrees, $minutes, $seconds );

	# deal with dd:mm:ss format
	if ( $_[0] =~ /^\S+:\S+:\S+$/ ) {
		( $degrees, $minutes, $seconds ) = $_[0] =~ /^(\S+):(\S+):(\S+)$/;
		if ( $degrees =~ /^-\S+/ ) {
			$dec_degrees = $degrees - $minutes / 60.0 - $seconds / 3600.0;
		}
		else {
			$dec_degrees = $degrees + $minutes / 60.0 + $seconds / 3600.0;
		}
	}

	# deal with dd:mm format
	elsif ( $_[0] =~ /^\S+:\S+$/ ) {
		( $degrees, $minutes ) = $_[0] =~ /^(\S+):(\S+)$/;
		if ( $degrees =~ /^-\S+/ ) {
			$dec_degrees = $degrees - $minutes / 60.0;
		}
		else {
			$dec_degrees = $degrees + $minutes / 60.0;
		}
	}

	# value already in decimal degrees
	else {
		$dec_degrees = $_[0];
	}

	# return decimal degrees;
	$dec_degrees;
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
	local ($argumentative) = @_;
	local ( @args, $_, $first, $rest );
	local ($errs) = 0;
	local ($[)    = 0;

	@args = split( / */, $argumentative );
	while ( @ARGV && ( $_ = $ARGV[0] ) =~ /^-(.)(.*)/ ) {
		( $first, $rest ) = ( $1, $2 );
		$pos = index( $argumentative, $first );
		if ( $pos >= $[ ) {
			if ( $args[ $pos + 1 ] eq ':' ) {
				shift(@ARGV);
				if ( $rest eq '' ) {
					++$errs unless @ARGV;
					$rest = shift(@ARGV);
				}
				eval "\$opt_$first = \$rest;";
				eval "\$flg_$first = 1;";
			}
			elsif ( $args[ $pos + 1 ] eq '+' ) {
				shift(@ARGV);
				if ( $rest eq '' ) {
					++$errs unless @ARGV;
					$rest = shift(@ARGV);
				}
				if ( eval "\$opt_$first" ) {
					eval "\$opt_$first = \$opt_$first 
				. \":\" . \$rest;";
				}
				else {
					eval "\$opt_$first = \$rest;";
				}
				eval "\$flg_$first = 1;";
			}
			elsif ( $args[ $pos + 1 ] eq '%' ) {
				shift(@ARGV);
				if ( $rest ne '' ) {
					eval "\$opt_$first = \$rest;";
				}
				else {
					$rest = $ARGV[0];
					($one) = $rest =~ /^-(.).*/;
					$pos = index( $argumentative, $one );
					if ( !$one || $pos < $[ ) {
						eval "\$opt_$first = \$rest;";
						shift(@ARGV);
					}
				}
				eval "\$flg_$first = 1;";
			}
			else {
				eval "\$opt_$first = 1";
				eval "\$flg_$first = 1;";
				if ( $rest eq '' ) {
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
			if ( $rest ne '' ) {
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
