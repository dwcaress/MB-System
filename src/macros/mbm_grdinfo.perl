eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
                    & eval 'exec perl -S $0 $argv:q'
                         if 0;
#--------------------------------------------------------------------
#    The MB-system:	mbm_grdinfo.perl	4/26/01
#    $Id: mbm_grdinfo.perl,v 1.6 2006-11-26 09:42:01 caress Exp $
#
#    Copyright (c) 2001, 2003 by 
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
#   mbm_grdinfo
#
# Purpose:
#   Macro to get information regarding a GMT grd file when the
#   region of interest is a subset of the area covered in the
#   input file. If no bounds are specified, the program grdinfo 
#   is called directly.  If longitude and latitude bounds are 
#   specified, then the specified region is cut from the input 
#   file using the macro mbm_grdcut, and the information is 
#   obtained from the subset temporary grd file using grdinfo.
#
# Basic Usage: 
#   mbm_grdinfo -Igrdfile -H -V -Rw/e/s/n 
#
# Author:
#   David W. Caress
#   Monterey Bay Aquarium Research Institute
#   Moss Landing, CA 95039
#   April 23, 2001
#   (at sea on the R/V Western Flyer about
#    10 km off the windward coast of Oahu)
##
# Version:
#   $Id: mbm_grdinfo.perl,v 1.6 2006-11-26 09:42:01 caress Exp $
#
# Revisions:
#   $Log: not supported by cvs2svn $
#   Revision 1.5  2006/09/11 18:55:52  caress
#   Changes during Western Flyer and Thomas Thompson cruises, August-September
#   2006.
#
#   Revision 1.4  2006/02/10 01:27:40  caress
#   Fixed parsing of grdinfo output to handle changes to GMT4.1.
#
#   Revision 1.3  2003/07/02 18:12:33  caress
#   Release 5.0.0
#
#   Revision 1.2  2003/04/17 20:42:48  caress
#   Release 5.0.beta30
#
#   Revision 1.1  2001/06/03 06:59:24  caress
#   Initial revision
#
#   Revision 5.1  2001/03/22 21:05:45  caress
#   Trying to make release 5.0.beta0
#
#
#
#
$program_name = "mbm_grdinfo";
 
# Determine the GMT version
@grdinfo = `grdinfo 2>&1`;
while (@grdinfo)
	{
	$line = shift @grdinfo;
	if ($line =~ 
		/^grdinfo\s+(\S+)\s+\S+/)
		{
		($gmt_version) = $line =~ 
			/^grdinfo\s+(\S+)\s+\S+/;
		}
	}

# Deal with command line arguments
$command_line = "@ARGV";
&MBGetopts('HhI:i:R:r:Vv');
$help =    		($opt_H || $opt_h);
$file_input =    	($opt_I || $opt_i);
$bounds = 		($opt_R || $opt_r);
$verbose = 		($opt_V || $opt_v);
 
# print out help message if required
if ($help)
	{
	print "\n$program_name:\n";
	print "\nMacro to get information regarding a GMT grd file when the\n";
	print "region of interest is a subset of the area covered in the\n";
	print "input file. If no bounds are specified, the program grdinfo \n";
	print "is called directly.  If longitude and latitude bounds are \n";
	print "specified, then the specified region is cut from the input \n";
	print "file using the macro mbm_grdcut, and the information is \n";
	print "obtained from the subset temporary grd file using grdinfo.\n";
	print "\nBasic Usage: \n";
	print "\t$program_name mbm_grdinfo -Igrdfile [-H -V -Rw/e/s/n]\n";
	exit 0;
	}

# check for input file
if (!$file_input)
	{
	print "\a";
	die "\nNo input file specified!\n$program_name aborted\n";
	}
elsif (! -r $file_input)
	{
	print "\a";
	die "\nSpecified input file $file_data cannot be opened!\n$program_name aborted\n";
	}

# Save old GMT default double format and set new format
$line = `gmtdefaults -L | grep D_FORMAT`;
($dformatsave) = $line =~ /D_FORMAT\s+=\s+(\S+)/;
`gmtset D_FORMAT %.15lg`;

# get limits of file using grdinfo
if ($bounds)
	{
	# get specified bounds
	($xmin_raw,$xmax_raw,$ymin_raw,$ymax_raw) = $bounds =~ 
			/(\S+)\/(\S+)\/(\S+)\/(\S+)/;
	$xmin = &GetDecimalDegrees($xmin_raw);
	$xmax = &GetDecimalDegrees($xmax_raw);
	$ymin = &GetDecimalDegrees($ymin_raw);
	$ymax = &GetDecimalDegrees($ymax_raw);

	# get bounds from file
	@rawgrdinfo = `grdinfo $file_input`;
	while (@rawgrdinfo)
		{
		$line = shift @rawgrdinfo;
		if ($line =~ 
			/\S+\s+x_min:\s+\S+\s+x_max:\s+\S+\s+x_inc:\s+\S+\s+units:\s+.+\s+nx:\s+\S+/)
			{
			($xmin_f,$xmax_f,$xinc_f,$xunits,$xnx_d) = $line =~ 
				/\S+\s+x_min:\s+(\S+)\s+x_max:\s+(\S+)\s+x_inc:\s+(\S+)\s+units:\s+(.+)\s+nx:\s+(\S+)/;
			}
		elsif ($line =~ 
			/\S+\s+x_min:\s+\S+\s+x_max:\s+\S+\s+x_inc:\s+\S+\s+name:\s+.+\s+nx:\s+\S+/)
			{
			($xmin_f,$xmax_f,$xinc_f,$xunits,$xnx_d) = $line =~ 
				/\S+\s+x_min:\s+(\S+)\s+x_max:\s+(\S+)\s+x_inc:\s+(\S+)\s+name:\s+(.+)\s+nx:\s+(\S+)/;
			}
		elsif ($line =~ 
			/\S+\s+x_min:\s+(\S+)\s+x_max:\s+(\S+)\s+x_inc:/)
			{
			($xmin_f,$xmax_f) = $line =~ 
				/\S+\s+x_min:\s+(\S+)\s+x_max:\s+(\S+)\s+x_inc:/;
			}
		if ($line =~ /\S+\s+y_min:\s+\S+\s+y_max:\s+\S+\s+y_inc:\s+\S+\s+units:\s+.+\s+ny:\s+\S+/)
			{
			($ymin_f,$ymax_f,$yinc_f,$yunits,$yny_d) = $line =~ 
				/\S+\s+y_min:\s+(\S+)\s+y_max:\s+(\S+)\s+y_inc:\s+(\S+)\s+units:\s+(.+)\s+ny:\s+(\S+)/;
			}
		elsif ($line =~ /\S+\s+y_min:\s+\S+\s+y_max:\s+\S+\s+y_inc:\s+\S+\s+name:\s+.+\s+ny:\s+\S+/)
			{
			($ymin_f,$ymax_f,$yinc_f,$yunits,$yny_d) = $line =~ 
				/\S+\s+y_min:\s+(\S+)\s+y_max:\s+(\S+)\s+y_inc:\s+(\S+)\s+name:\s+(.+)\s+ny:\s+(\S+)/;
			}
		elsif ($line =~ /\S+\s+y_min:\s+\S+\s+y_max:\s+\S+\s+y_inc:/)
			{
			($ymin_f,$ymax_f) = $line =~ 
				/\S+\s+y_min:\s+(\S+)\s+y_max:\s+(\S+)\s+y_inc:/;
			}
		if ($line =~ /\S+\s+zmin:\s+\S+\s+zmax:\s+\S+\s+units:\s+\S+/)
			{
			($zmin_f,$zmax_f) = $line =~ 
				/\S+\s+zmin:\s+(\S+)\s+zmax:\s+(\S+)\s+units:\s+\S+/;
			}
		elsif ($line =~ /\S+\s+zmin:\s+\S+\s+zmax:\s+\S+\s+name:\s+\S+/)
			{
			($zmin_f,$zmax_f) = $line =~ 
				/\S+\s+zmin:\s+(\S+)\s+zmax:\s+(\S+)\s+name:\s+\S+/;
			}
		if ($line =~ /\S+\s+z_min:\s+\S+\s+z_max:\s+\S+\s+units:/)
			{
			($zmin_f,$zmax_f,$zunits_s) = $line =~ 
				/\S+\s+z_min:\s+(\S+)\s+z_max:\s+(\S+)\s+units:\s+(.+)/;
			}
		elsif ($line =~ /\S+\s+z_min:\s+\S+\s+z_max:\s+\S+\s+name:/)
			{
			($zmin_f,$zmax_f,$zunits_s) = $line =~ 
				/\S+\s+z_min:\s+(\S+)\s+z_max:\s+(\S+)\s+name:\s+(.+)/;
			}
		}
 	
	# check file bounds vs specified bounds
	if ($x_min < $xmin_f)
		{
		$xmin = $xmin_f;
		}
	if ($xmax > $xmax_f)
		{
		$xmax = $xmax_f;
		}
	if ($ymin < $ymin_f)
		{
		$ymin = $ymin_f;
		}
	if ($ymax > $ymax_f)
		{
		$ymax = $ymax_f;
		}
	if ($xmin >= $xmin_f && $xmax <= $xmax_f
		&& $ymin >= $ymin_f && $ymax <= $ymax_f
		&& $xmax > $xmin && $ymax > $ymin)
		{
		# cut out desired file
		$pid = getppid();
		$file_grd = "$file_input" ."_grdcut_$pid";		
		if ($verbose > 0)
			{
			print "\nRunning mbm_grdcut...\n";
			print "\tmbm_grdcut -I$file_input -O$file_grd -R$xmin/$xmax/$ymin/$ymax -V\n";
			}
		@grdcut = `mbm_grdcut -I$file_input -O$file_grd -R$xmin/$xmax/$ymin/$ymax -V 2>&1`;
		
		if ($verbose > 0)
			{
			while (@grdcut)
				{
				$line = shift @grdcut;
				print "mbm_grdcut output: $line";
				}
			}
			
		@grdinfo = `grdinfo $file_grd 2>&1`;
		@rminfo = `rm -f $file_grd`;
		
		# if it doesn't work just grdinfo the whole file
		if (!@grdinfo)
			{
			@grdinfo = `grdinfo $file_input 2>&1`;
			}
		}
	else
		{
		@grdinfo = `grdinfo $file_input 2>&1`;
 		}

 	}
else
	{
	@grdinfo = `grdinfo $file_input 2>&1`;
 	}

# now spit out grdinfo results 
while (@grdinfo)
	{
	$line = shift @grdinfo;
	print $line;
	}

# reset the GMT default double format
`gmtset D_FORMAT $dformatsave`;

exit 0;

#-----------------------------------------------------------------------
sub GetDecimalDegrees {

	# make local variables
	local ($dec_degrees, $degrees, $minutes, $seconds);

	# deal with dd:mm:ss format
	if ($_[0] =~ /^\S+:\S+:\S+$/)
		{
		($degrees, $minutes, $seconds) 
			= $_[0] =~ /^(\S+):(\S+):(\S+)$/;
		if ($degrees =~ /^-\S+/)
			{
			$dec_degrees = $degrees 
				- $minutes / 60.0 
				- $seconds / 3600.0;
			}
		else
			{
			$dec_degrees = $degrees 
				+ $minutes / 60.0 
				+ $seconds / 3600.0;
			}
		}
	# deal with dd:mm format
	elsif ($_[0] =~ /^\S+:\S+$/)
		{
		($degrees, $minutes) 
			= $_[0] =~ /^(\S+):(\S+)$/;
		if ($degrees =~ /^-\S+/)
			{
			$dec_degrees = $degrees - $minutes / 60.0;
			}
		else
			{
			$dec_degrees = $degrees + $minutes / 60.0;
			}
		}

	# value already in decimal degrees
	else
		{
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

