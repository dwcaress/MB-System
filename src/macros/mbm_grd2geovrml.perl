eval '(exit $?0)' && eval 'exec perl -S $0 ${1+"$@"}'
                    & eval 'exec perl -S $0 $argv:q'
                         if 0;
#--------------------------------------------------------------------
#    The MB-system:     mbm_grd2geovrml.perl              11/7/2001
$ver = '$Id: mbm_grd2geovrml.perl,v 5.0 2002-08-10 06:42:59 caress Exp $';
#
#    Copyright (c) 2001 by
#    Mike McCann (mccann@mbari.org)
#      Monterey Bay Aquarium Research Institute
#      Moss Landing, CA
#
#    See README file for copying and redistribution conditions.
#--------------------------------------------------------------------
#
# Command:
#   mbm_grd2geovrml
#
# Purpose:
#
# Script to convert GMT grid to TerraVision and geoVRML tiles
#
# Stuff this script does:
# - pad out the no data areas (by regridding a sample of original grid)
# - create tiff with black background from original data (unless exists)
# - store as TerraVision data sets (.dem & .oi)
# - convert and store as geoVRML tiles
#
# Requires:
# - GMT version 3.1 or higher
# - TsmApi version 2.2 or higher.  http://www.tsmapi.com
# - ImageMagick 3.7.7 or higher. http://www.imagemagick.org/
#
# Basic Usage: 
#  mbm_grd2geovrml <base_name> --tvdir <dir> --vrmldir <dir> 
#                  -olat <latitude> -olon <longitude>
#
#   Where <base_name> is the bathymetry grid filename without the .grd.gz
#    extensions.
#
#   E.G.:
#
#   mbm_grd2geovrml /hosts/menard/vol2/EM300_GRIDS/hawaii/PapauA_bath \
#   --tvdir ~/TileSets/Pyramids/hawaii \
#   --vrmldir ~/TileSets/geoVRML/hawaii \
#   --olat 21 --olon -157
#
# Additional Options:
#
# Author:
#   Mike McCann
#   Monterey Bay Aquarium Research Institute
#   Moss Landing, CA
#   November 7, 2001
#
# Version:
#   $Id: mbm_grd2geovrml.perl,v 5.0 2002-08-10 06:42:59 caress Exp $
#
# Revisions:
#
# $Log: not supported by cvs2svn $
# Revision 1.11  2002/08/08 21:09:36  mccann
# Change to using the make_geovrml in the search path, not mccann's.
#
# Revision 1.10  2002/08/08 21:05:37  mccann
# Changed name from mbm_grd2geovrml to mbm_grd2geovrml.perl
# Use mbstripNaN instead of removeNaN; mbstripNaN is now a part of MB-System.
#
# Revision 1.9  2002/04/04 18:36:50  mccann
# Added '-interlace none' to mogrify because make_oi wants chunky (RGBRGB...)
#
# Revision 1.8  2002/01/10 20:07:29  mccann
# Fixed bugs in grid clipping command setup.
#
# Revision 1.7  2002/01/09 21:14:18  mccann
# Removed customizations for MBARI data coverages (hawaii, cencal, etc.)
#
# Revision 1.6  2002/01/03 19:33:59  mccann
# *** empty log message ***
#
# Revision 1.5  2001/12/05 18:15:42  mccann
# Added comment for ImageMagick requirement.  Version sent to Dave for
# testing and initial inclusion in MB-System.
#
# Revision 1.4  2001/11/28 23:22:14  mccann
# Added options for color pallette and grid clipping.
#
# Revision 1.3  2001/11/14 17:42:40  mccann
# Full command line written to out file.  -wrz option added to make_geovrml.
#
# Revision 1.2  2001/11/07 20:01:16  mccann
# Added check in execute() for existence of commands in search path.
#
# Revision 1.1  2001/11/07 19:34:10  mccann
# Initial revision
#
#


use File::Basename;
use Getopt::Long;


#
# Constants. These may need to be changed to suit your needs
#
$| = 1; 			# unbuffered stdout
$tmpDir = $ENV{'TMPDIR'} || die "\nPlease set your TMPDIR environment variable.\n";
$scale = 1000;		# To Multiple for bitmap
$offset = 10000;	# To add for bitmap
$grdResampFac = 10;	# Subsampling factor
$radFrac = 1.0;		# Fraction of longest grid dimension for resampling
$make_geovrml = "make_geovrml";
##$make_geovrml = "/u/mccann/Pkgs/tsmApi/tsmApi-2.2a/irix6.5n32/tsmUtils/make_geovrml";

#
# Usage note
#
sub usage {
	print <<EOF;

    Usage: $0 <grd_file> --tvdir <tvdir> --vrmldir <vrmldir>
           -olat <lat> -olon <lon> 
           [ --nowrz --pallette <pal> --newimage --white 
             --zmax <maxclip> --zmin <minclip> 
             --elevscale <vert_exag> --noview --vrmlurl <base_addr> ]
    
    Where: <grd_file>:  the full name of the GMT grd file to be converted
           <tvdir>:     the location of TerraVision tile set to be created
           <vrmldir>:   the location of geoVRML tile set to be created
           <lat>:       latitude of geoOrigin in decimal degrees
           <lon>:       longitude of geoOrigin in decimal degrees
           (A directory that is the base name of the grd file will be created
            in <tvdir> and <vrmldir>.  <tvdir> and <vrmldir> will be created
            if they do not exist.)

    Optional parameters:
      --nowrz:      write geoVRML .wrl files instead of gzipped .wrz
      <pal>:        number of Color Lookup Table (1:Haxby,2:High,3:Low,4:Gray)
      --newimage:   force creation of new image, otherwise uses one in $TMPDIR
      --white:      create image with white background instead of black
      <maxclip>:    clip grid at this maximum elevation - adjusts CLT
      <minclip>:    clip grid at this minumum elevation - adjusts CLT

    Optional make_geovrml(1) parameters:
      <vert_exag>:  vertical exaggeration for geoVRML (default is 1)
      --noview:     do not include viewpoints for each tile in geoVRML
      <base_addr>:  explicit base web address for childurls & images

    E.G.:

    $0 /hosts/menard/vol2/EM300_GRIDS/hawaii/PapauA_bath \\
    --olat 21 --olon -157 \\
    --tvdir ~/TileSets/Pyramids/hawaii \\
    --vrmldir ~/TileSets/geoVRML/hawaii \\
    --vrmlurl http://menard.mbari.org/vrml/terrain/hawaii/PapauA_bath

EOF
	exit(1);
}

# -----------------------------------------------------------------------

#
# Get options and test for input/output directories
#
my $allArgs = join(" ", @ARGV);		# GetOptions slurps up, so save in advance

#
# Default options
#
$elevscale = 1;
$nowrzFlag = 0;
$noviewFlag = 0;
$newimageFlag = 0;
$whiteFlag = 0;
$pallette = 1;		# See mbm_grdtiff 1:Haxby, 2:High, 3: Low, 4:gray, ...
$vrmlurl = '';
if ( ! GetOptions ( 'tvdir=s' => \$tvdir, 'vrmldir=s' => \$vrmldir,
					'olat=f' => \$olat, 'olon=f' => \$olon,
                    'elevscale=f' => \$elevscale, 'nowrz' => \$nowrzFlag,
                    'noview' => \$noviewFlag, 'pallette=i' => \$pallette,
					'newimage' => \$newimageFlag, 'white' => \$whiteFlag,
					'zmax=f' => \$clipmax, 'zmin=f' => \$clipmin,
					'vrmlurl=s' => \$vrmlurl ) ) {
	print "\nFailed to read options.\n\n";
	usage;
}
if ( ! $tvdir ) {
	print "\n*** Must specify <tvdir>.\n\n";
	usage;
}
if ( ! -d $tvdir ) {
	print "\nCreating $tvdir \n\n";
	mkdir ($tvdir,0755) || die "Cannot mkdir $tvdir: $!\n";
}
if ( ! $vrmldir ) {
	print "\n*** Must specify <vrmldir>.\n\n";
	usage;
}
if ( ! -d $vrmldir ) {
	print "\nCreating $vrmldir \n\n";
	mkdir ($vrmldir,0755) || die "Cannot mkdir $vrmldir: $!\n";
}
if ( ! $olat ) {
	print "\n*** Must specify <olat> for geoOrigin.\n\n";
	usage;
}
if ( ! $olon ) {
	print "\n*** Must specify <olon> for geoOrigin.\n\n";
	usage;
}

#
# Check that $zmin < $zmax ( I wasted 40 minutes with this user mistake!)
#
if ( $clipmin > $clipmax ) {
	print "\n*** minclip must be less than maxclip.  Below sea level is < 0.\n\n";
	die;
}	

#
# Set up file names
#
($baseFile, $origPath, undef) = fileparse($ARGV[0]);
$baseFile =~ s/\.grd//;		# Remove '.grd' if supplied
$origPath =~ s#\/$##;		# Remove trailing slash

if ( $origPath eq "." ) {	# If short name then set to current dir
	$origPath = `pwd`;
	chop($origPath);
}

$gzGrdFile = "$baseFile.grd.gz";
$grdFile = $gzGrdFile;
$grdFile =~ s/\.gz$//;

#
# Temporary grid files (get removed upon successful completion)
#
$sampGrdFile = "Samp_$grdFile";
$tmpGrdFile = "Tmp_$grdFile";
$fillGrdFile = "Fill_$grdFile";
$fixGrdFile = "Fix_$grdFile";
@tmpFiles = ($grdFile, $sampGrdFile, 
			 $tmpGrdFile, $fillGrdFile, 
			 $fixGrdFile, "${baseFile}_tiff.cmd");

#
# Deal with grd file (either gzipped or not), make it local to $tmpDir
#
if ( ! -f "$origPath/$gzGrdFile" ) {
	print "\nFile $origPath/$gzGrdFile not found.\n\n";
	if ( ! -f "$origPath/$grdFile" ) {
		print "\nFile $origPath/$grdFile not found.\n\n";
    	usage;
	}
}

#
# Go to temp work area and start the work...
#
chdir($tmpDir) || die "Can't chdir to $tmpDir: $!\n";
print "Working in " . `pwd`;
unlink "$baseFile.out" || die "Can't remove $baseFile.out\n";

#
# Start output file, give a litle info
#
open(OUT, ">>$baseFile.out");
print OUT "\n#----------------------------------------------------------\n";
print OUT "# Starting $0 on " . scalar localtime() . "\n";
print OUT "# $ver\n";
print OUT "# \n";
print OUT "# Command line that generated this file and its results:\n";
print OUT "#   $0 $allArgs \n"; 
print OUT "# \n";
print OUT "# \n";
close OUT;


#
# Unzip grid file to local file, check that file may be unzipped already
#
if ( -f "$origPath/$gzGrdFile" ) {
	$cmd = "gunzip -c $origPath/$gzGrdFile > $grdFile";
	execute($cmd, "Unzip grid file to local file");
}
elsif ( -f "$origPath/$grdFile" ) {
	$cmd = "cp $origPath/$grdFile $grdFile";
	execute($cmd, "Copy file to local area");
}
else {
	die "Can't find $origPath/$gzGrdFile or $origPath/$grdFile\n";
}

#
# Clip the grid if specified on command line
#	
if ( defined $clipmax || defined $clipmin ) {
	$cmd = "mv $grdFile tmp_${grdFile}";
	execute($cmd, "Clipping grid file, rename original in prep for grdclip");
}
if ( defined $clipmax && ! defined $clipmin ) {
	$cmd = "grdclip tmp_${grdFile} -G$grdFile -Sa$clipmax/NaN";
}
elsif ( defined $clipmin && ! defined $clipmax ) {
	$cmd = "grdclip tmp_${grdFile} -G$grdFile -Sb$clipmin/NaN";
}
elsif ( defined $clipmax && defined $clipmin ) {
	$cmd = "grdclip tmp_${grdFile} -G$grdFile -Sa$clipmax/NaN -Sb$clipmin/NaN";
}
if ( defined $clipmax || defined $clipmin ) {
	execute($cmd, "Clip grid at min = $clipmin, max = $clipmax");
}

#
# Parse out grid information for use by make_dem & make_oi
#
execute("/bin/ls -l $grdFile", "Size of original grid file");
$grdInfo = `grdinfo $grdFile`;
foreach ( split('\n', $grdInfo) ) {
	if ( /x_min: (\S+)/ ) {
		$x_min = $1;
	}
	if ( /x_max: (\S+)/ ) {
		$x_max = $1;
	}
	if ( /x_inc: (\S+)/ ) {
		$x_inc = $1;
	}
	if ( /y_min: (\S+)/ ) {
		$y_min = $1;
	}
	if ( /y_max: (\S+)/ ) {
		$y_max = $1;
	}
	if ( /y_inc: (\S+)/ ) {
		$y_inc = $1;
	}
	if ( /nx: (\S+)/ ) {
		$nx = $1;
	}
	if ( /ny: (\S+)/ ) {
		$ny = $1;
	}
}
print "x_min= $x_min, x_max= $x_max, x_inc= $x_inc\n";
print "y_min= $y_min, y_max= $y_max, y_inc= $y_inc\n";
print "nx= $nx, ny= $ny\n";
die "*** Can't continue without grid dimensions." unless 
	($x_min && $x_max && $x_inc && $y_min && $y_max && $y_inc && $nx && $ny);
execute("grdinfo $grdFile", "Get grid dimensions & resolution");	# For the log output file

#
# Compute dimensions of grid for the course resampling
#
$dimen = ( $nx > $ny ) ? $nx : $ny; 		# Use larger dimension
$rad = $x_inc * $dimen * $radFrac; 				# Some fraction of grid dimension
#
# To avoid GMT ERROR: x_inc incompatible with (x_max-x_min), see grdedit -A.
# See http://op.gfz-potsdam.de/GMT-Help/Archive/msg03955.html
# Calculate my own $myXinc & $myYinc
#
$myXinc = ($x_max - $x_min) / ($nx - 1);
$myYinc = ($y_max - $y_min) / ($ny - 1);
$surfXinc = ($x_max - $x_min) / (100 - 1);
$surfYinc = ($y_max - $y_min) / (100 - 1);

#
# Subsample grid
#
$xSampInc = $grdResampFac * $myXinc;
$cmd = "grdsample $grdFile -I${xSampInc} -Q -G${sampGrdFile}";
execute($cmd, "Subsample grid");

#
# 1. Convert grid to xyz format for filling in blank areas 
# 2. preprocess w/blockmean
# 3. use surface to extrapolate to no data areas
#
$cmd = "grd2xyz -bo $sampGrdFile | ";
$cmd .= "blockmean -bi -bo -V -I${surfXinc}/${surfYinc} -R$x_min/$x_max/$y_min/$y_max | ";
$cmd .= "mbstripNaN | ";
$cmd .= "surface -bi -I${surfXinc}/${surfYinc} -S${rad} -T0.35 -G${tmpGrdFile} ";
$cmd .= "-V -R$x_min/$x_max/$y_min/$y_max ";
execute($cmd, "Convert grid to xyz format for filling in blank areas, preprocess w/blockmean, use surface to extrapolate to no data areas");


#
# Resample to original grid dimenensions
#
$cmd = "grdsample $tmpGrdFile -I${myXinc}/${myYinc} -Q -G${fillGrdFile}";
execute($cmd, "Resample to original grid dimenensions");


#
# Give proper units to Filled Grid file
#
$cmd = "grdedit ${fillGrdFile} -DLongitude/Latitude/\"Topography (m)\"";
$cmd .= "/1/0/\"Topography Grid\"";
execute($cmd, "Give proper units to Filled Grid file");

#
# Repair the dx/dy for the original grid
#
$cmd = "grdedit -A -R$x_min/$x_max/$y_min/$y_max ${grdFile} ";
execute($cmd, "Repair the dx/dy for the original grid");

#
#  1. Paste (ADD) original grid on top of this large-search-radius grid
#
#  2. Scale and offset the grid in preparation for make_geoVRML (preserves
#     accuracy to 1 mm in Z, also make_geovrml can't handle elevations < 0)
#
$cmd = "grdmath $grdFile $fillGrdFile AND $offset ADD $scale MUL = $fixGrdFile";
execute($cmd, "Paste (ADD) original grid on top of this large-search-radius grid, Scale and offset the grid in preparation for make_geoVRML");


#
# Extract bitmap from fixed grid for use by tsmApi,
#
$cmd = "grd2xyz -Zl $fixGrdFile > $baseFile.raw && ls -l $baseFile.raw";
execute($cmd, "Extract bitmap from fixed grid for use by tsmApi");


#
# Image generation takes a real long time, if .tif exists skip, unless flag
#
if ( -s "$baseFile.tif" && ! $newimageFlag ) {
	print "Found\n" . `/bin/ls -l "$baseFile.tif"` . "\n";
	print "Skipping image generation\n";
	goto TerraVision;
}

image:
#========================= Image generation ========================
#
# Create command file for creating shaded tiff from original grid file
# - edit in place the .cmd file to set black NAN
# - execute the script
#
##$cmd = "mbm_grdtiff -I $grdFile -O$baseFile -W bath.cpt -G2 -A0.25/280/15 -Q -V";
$cmd = "mbm_grdtiff -I $grdFile -O$baseFile -W1/${pallette} -G2 -A0.25/280/15 -Q -V";
execute($cmd, "Create script to create black NANed shaded tiff from original grid file");

if ( ! $whiteFlag ) {
	# Create black NANed shaded tiff from original grid file
	#
	# Used sed(1) to edit .cmd file
	#
	@ARGV = ("${baseFile}_tiff.cmd");
	$cmd = "mv ${baseFile}_tiff.cmd ${baseFile}_tiff.cmd.bak";
	execute($cmd, "Rename command file ");

	$cmd = "sed 's#gmtset COLOR_NAN 255/255/255#gmtset COLOR_NAN 0/0/0#' ${baseFile}_tiff.cmd.bak > ${baseFile}_tiff.cmd && chmod +x ${baseFile}_tiff.cmd && /bin/rm ${baseFile}_tiff.cmd.bak";
	execute($cmd, "Set COLOR_NAN to 0/0/0 in .cmd file for black background");
}

#
# Run the commands to create the tiff image
#
execute("csh ./${baseFile}_tiff.cmd && ls -l $baseFile.tif", "Run the script to create the .tiff");
unlink "${baseFile}.grd.int";

#
# Increase image size to get more geometry, needs lots of swap & time for this!
#
##execute("/bin/cp $baseFile.tif ${baseFile}_orig.tif", "Save a copy of original image");	# For debugging
execute("mogrify -interlace none -geometry 400% $baseFile.tif && identify $baseFile.tif", "Quadruple image size");

TerraVision:
#=================== TerraVision Pyramid generation ====================
#
# Make TerraVision DEM tile set
#
$cmd = "make_dem $tvdir/$baseFile $baseFile.raw ";
$cmd .= "-startres $x_inc -geoname $baseFile ";
$cmd .= "-width $nx -height $ny -ll_lat $y_min -ll_long $x_min ";
$cmd .= "-offset -$offset -scale " . 1/$scale;
execute("$cmd && /bin/rm -f $baseFile.raw", "Make TerraVision DEM tile set");

#
# Make TerraVision OI tile set from file in local directory
#
$cmd = "make_oi $tvdir/$baseFile $baseFile.tif ";
$cmd .= "-startres " . $x_inc / 4 . " -geoname $baseFile ";
$cmd .= "-width " . $nx * 4 . " -height " . $ny * 4 . " -ll_lat $y_min -ll_long $x_min ";
execute("$cmd", "Make TerraVision OI tile set");

geoVRML:
#======================= geoVRML generation ========================
#
# Create geoVRML parent dir for this coverage and make geoVRML tiles
#
$cmd = "$make_geovrml \\\n";
$cmd .= "-dem $tvdir/$baseFile.dem \\\n";
$cmd .= "-oi $tvdir/$baseFile.oi \\\n";
$cmd .= "-vrmldir $vrmldir/$baseFile \\\n";
$cmd .= "-olat $olat -olon $olon \\\n";
$cmd .= "-vrmlurl $vrmlurl \\\n";
$cmd .= "-touch 2,4,6 -hud -maxrange -onesided -numpolys 16 ";
$cmd .= "-elevscale $elevscale ";
$cmd .= "-wrz " unless $nowrzFlag;
$cmd .= "-whitebg " if  $whiteFlag;
$cmd .= "-noview " if $noviewFlag;	# Don't have viewpoints if using a basemap
execute($cmd, "Create geoVRML parent dir for this coverage and make geoVRML tiles");
execute("/bin/ls -l $vrmldir/$baseFile/trees", "All finished");

#
# Remove temp files
#
foreach ( @tmpFiles ) {
	print "removing $_\n";
	unlink $_;
}

#
# Save commands that created the geoVRML
#
execute("cp $baseFile.out $vrmldir/$baseFile", "Save commands that created all this");

#
# Parse geoVRML trees and collect data for database load
#
##parse_geoVRML();
##load_database();


exit(0);

# ----------------------------------------------------------------

sub execute {
	local ($cmd, $comment) = @_;

	#
	# Check that command is found in users search path
	#
	my @item = split('\s+', $cmd);
	my $which = `which $item[0]`;
	chop $which;
	die "\nCan't find $item[0] in your search path:\n$which" unless -f $which;
		
	#
	# Write comments & commands to output file
	#
	open(OUT, ">>$baseFile.out");
	print OUT "\n#----------------------------------------------------------\n";
	print OUT "# $comment\n";
	print OUT "# ", scalar localtime, ":\n";
	print OUT "$cmd\n\n";			# Record command to out file

	print "\n#---------------------------------------------------------------\n";
	print "# $comment\n";
	print "$cmd\n\n";

	##my $ret = system($cmd);
	# Capture output & put comments in front
	open (CMD, "$cmd 2>&1 |") or die "Can't run $cmd : $!\n";
	while (<CMD>) {
		print "# ", $_;
		print OUT "# ", $_;
	}
	close OUT;
	
	##$ret = $ret / 256 / 255;
	##die "\nReturn value > 0. See above for any error message.\n" unless $ret <= 0;
}

