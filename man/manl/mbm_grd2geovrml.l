.TH mbm_grd2geovrml l "11 January 2002" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbm_grd2geovrml\fP - Create and execute commands which generate
a TerraVision tileset and GeoVRML set of files that can be combined 
with other data and viewed in a web browser.

.SH VERSION
Version 5.0

.SH SYNOPSIS
\fBmbm_grd2geovrml\fP \fIbath_file\fP 
\fB-tvdir\fP \fItvparent\fP 
\fB-vrmldir\fP \fIvrmlparent\fP 
\fB-olat\fP \fIlat\fP 
\fB-olon\fP \fIlon\fP 


Additional Options:

\fB-elevscale\fP \fIvert_exag\fP
\fB-nowrz\fP 
\fB-noview\fP 
\fB-pallette\fP \fIpal\fP 
\fB-newimage\fP 
\fB-white\fP
\fB-zmax\fP \fImaxclip\fP 
\fB-zmin\fP \fIminclip\fP
\fB-vrmlurl\fP \fIurl\fP 

.SH DESCRIPTION
\fBmbm_grd2geovrml\fP is a macro that takes as input a GMT
geographic-coordinate bathymetry grid file (\fIbath_file\fP) and from it generates a shaded image
which is paired with the elevation data in the grid file to
create a TerraVision tileset. This tileset is then used to
generate a GeoVRML quadtree multi-resolution set of files.  The resulting
set of files will be created in \fIvrmlparent\fP/<basename>, where
<basename> is the same as \fIbath_file\fP without the usual '.grd' extension.
The index.html file produced in this directory can be 
loaded into Netscape 4.7 on a PC (properly configured with CosmoPlayer
and GeoVRML) for interactive visualization. The resulting
terrain can be used as a 3D basemap for use in visualizing
other georeferenced data that has also been placed in GeoVRML.

GeoVRML (and TerraVision) expects rectangular gridded elevation data
and browsers render all terrain data within the rectangle.  This presents a
problem for ocean bathymetric data which often have irregular (not rectangular)
boundaries.  Before converting a grid file to TerraVision format 
\fBmbm_grd2geovrml\fP replaces all no-data elevation values with a coarse
resolution spline-filled extrapolation. This results in a rectangular grid 
with smooth transitions from areas of elevation data to areas of no data.  
In the resulting GeoVRML the no data areas are colored black 
(or white if \fB-white\fP is specified).

.SH AUTHORSHIP
Mike McCann (mccann@mbari.org)
.br
  Monterey Bay Aquarium Research Institute
.br

.SH SIMPLE DESCRIPTION OF BASIC OPTIONS
.TP
.B \ 
\fIbath_file\fP
.br
A GMT gridded bathymetry file in geographic (lat/lon) coordinates. 
The file can be expressed with just the base name without the .grd 
extension, in which case the macro will append '.grd.gz' to \fIbath_file\fP 
and uncompress the data upon input. If the compressed data file does not
exist then '.grd' will be appended to \fIbath_file\fP and it will be 
used, if it exists. 

.TP
.B \-tvdir 
\fItvparent\fP

The TerraVision tileset destination directory.
A subdirectory named \fIbath_file\fP (without the .grd extension)
will be created here.
.TP
.B \-vrmldir
\fIvrmlparent\fP
.br
The GeoVRML tileset destination directory.
A subdirectory named \fIbath_file\fP (without the .grd extension)
will be created here.

.TP
.B \-olat 
\fIlat\fP
.TP
.B \-olon 
\fIlon\fP

.br
The geoOrigin latitude and longitude in decimal degrees. Because of single 
precision arithmetic in VRML97 it is necessary to define
a geoOrigin from which offsets are computed before coordinates
are passed into the visualization pipeline.  The geoOrigin should
be within 5 degrees of the extent of the grid file in order to
have submeter precision in the location of objects within the
resulting GeoVRML output.  See the GeoVRML spec for more details.
.br

.br
The GeoOrigin of the terrain must be the same as the GeoOrigin
of any other GeoVRML content (e.g. navigation tracks, sample
locations) that you wish to combine with the terrain.

.SH DESCRIPTION OF OPTIONAL OPTIONS

.TP
.B \-elevscale
\fIvert_exag\fP
.br
Factor with which to multiply elevations resulting in a vertical
exaggeration of the terrain.  The default value is 1.0. Factors
greater than 1.0 increase vertical exaggeration.

.TP
.B \-nowrz
.br
By default vrml output files are gzipped and saved with the .wrz
extension.  Specify -nowrz to override this and save the files 
uncompressed with the .wrl extension.

.TP
.B \-noview
.br
By default a GeoViewpoint node with description 'home' is included 
with each GeoVRML tile file.  These viewpoints are helpful for
creating terrain-only visualizations, but they also quickly fill up
the viewpoint list when many tiles are loaded.  If you are 
creating GeoVRML tilesets
to combine with other GeoVRML content that contain viewpoints 
then you may want to not have these 'home' viewpoints.
Specify -noview to not include a GeoViewpoint with each tile.

.TP
.B \-newimage
.br
The orthorectified image for the bathymetry is created by running
mbm_grdtiff(1) and then quadrupling the image size using ImageMagick
mogrify(1). (The quadrupling together with the '-numpixels 16' 
argument in the make_geovrml(1) command results in identical resolution
of the highest level GeoVRML tiles and the original bathymetric
data.)  Making this image can be a time consuming process.  By default,
if a file with '.tif' appended to \fIbath_file\fP (without the .grd 
extension) exists in $TMPDIR then 
a new image is not created.  To override this either remove the .tif 
file or specify the -newimage option.

.TP
.B \-white
.br
By default a black background is generated for the orthorectified 
image.  If a white background is desired for printing purposes
then use this option.

.TP
.B \-zmax
\fImaxclip\fP
.TP
.B \-zmin
\fIminclip\fP

Maxclip and minclip may be used clip the elevation data at set
values.  For instance, to clip the elevation data at sealevel
and tile only bathymetry use \fB-zmax 0\fP.  To force a color map
of the standard Haxby colors between 4000 and 3000 meters
depth use \fB-zmin -4000 -zmax -3000\fP.

.TP
.B \-vrmlurl
\fIurl\fP
.br
All default URLs that are placed in the VRML files will be relative. 
However, you may use the \fB-vrmlurl\fP option to specify an absolute URL 
for the set of tiles.
Then all of the URLs that are used in the VRML files (image
texture links and childUrls in the GeoLOD node) will use this as 
their base URL.
This is generally not desirable because hardcoding absolute URLs 
into your datasets means that the dataset is nonportable to another 
server, or
different location on the same server. However, it may be necessary 
to write absolute URLs in your tile files, especially if you have 
many levels because
some VRML browsers have problems with relative links, resulting 
in tiles and images that will not load.


.SH ENVIRONMENT VARIABLE AND LOG FILE

The environment variable TMPDIR must be defined.  It specifies the
directory in which mbm_grd2geovrml does its work.  $TMPDIR should
have sufficient space to hold the temporary grid and image
files that get created.  The resulting 4 times magnified tiff
images are left in $TMPDIR.  These large image files are reused unless
the \fB-newimage\fP option is specified.

A log file is saved at the end of processing in the \fIvrmlparent\fP
directory.  It includes all the commands and their output.  This
file may be edited and executed again in order to produce customized
output.
 
.SH EXAMPLES
This example creates a GeoVRML set of terrain tiles from the bathymetry
data in file PapauA_bath.grd.  The geometry
files will not be compressed and viewpoints will be included with each 
tile.
.br

.br
  mbm_grd2geovrml \\
    /hosts/menard/vol2/EM300_GRIDS/hawaii/PapauA_bath \\
    --olat 21 --olon -157 \\
    --tvdir ~/TileSets/Pyramids/hawaii \\
    --vrmldir ~/TileSets/geoVRML/hawaii \\
    --vrmlurl \\
    http://menard.mbari.org/vrml/terrain/hawaii/PapauA_bath
.br

.br

.br
This example creates a grid file of the Northeast Pacific
at one minute resolution and creates the GeoVRML tiles of it
with the elevations clipped at sea level.
.br

.br
  # Extract 1 minute data for Northeastern Pacific Ocean
  # and convert it to GeoVRML terrain
  #
  grdraster 4 -R-165/-105/15/50\\
        -GNEPacific.grd \\
        -I1m -V

  mbm_grd2geovrml/mbm_grd2geovrml NEPacific.grd \\
    -tvdir /u/mccann/TileSets/Pyramids/pacific \\
    -vrmldir /u/mccann/TileSets/geoVRML/pacific  \\
    -zmax 0 -olat 35 -olon -135 -newimage



.SH SEE ALSO

This macro is built upon some pretty strong shoulders.  It uses
all of these programs which must be installed on your system.

GMT programs:
\fBgrdclip\fP(1), 
\fBgrdsample\fP(1), 
\fBgrd2xyz\fP(1), 
\fBblockmean\fP(1), 
\fBsurface\fP(1), 
\fBgrdedit\fP(1), 
\fBgrdmath\fP(1), 
\fBgrdclip\fP(1), 

mb-system macro:
\fBmbm_grdtiff\fP(1)

mb-system utility:
\fBmbstripNaN\fP(1)

tsmApi-2.3 (http://www.tsmapi.com) programs:
\fBmake_dem\fP(1), 
\fBmake_oi\fP(1)
\fBmake_geovrml\fP(1)

Note that after you've created the TerraVision tile sets with this
macro you may rerun make_geovrml(1) with different options, for
instance to make files for a different server or with a different
vertical exaggeration or background color.  This can save a lot
of processing time if you don't need to generate a new image
from the bathymetric data.

ImageMagick (http://www.imagemagick.org) program:
\fBmogrify\fP(1)

.br
Please see http://www.mbari.org/~mccann/vrml/ROVDataVis
for example content and more information on the project that developed this
macro.
.SH BUGS

You need to manually clean up the leftover files that get created in $TMPDIR.
.br

.br
The GeoVRML content produced by this macro may be 
viewed using Internet Explorer 
with the Cortona 4.0 VRML plugin or in Netscape 4.7 with the CosmoPlayer 
plugin.  Both browsers have problems with loading a second world after 
you have viewed one.  



