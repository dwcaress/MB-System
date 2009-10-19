.TH mbgrid l "4 December 2008" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbgrid\fP - Grid bathymetry, amplitude, or sidescan data 
from swath sonar data files.

.SH VERSION
Version 5.0

.SH SYNOPSIS
\fBmbgrid\fP \fB-I\fIfilelist\fP \fB-O\fIroot\fP 
\fB-R\fIwest/east/south/north\fP [\fB-A\fIdatatype\fP 
\fB-B\fIborder\fP -C\fIclip\fP[\fI/mode\fP] \fB-D\fIxdim/ydim\fP 
\fB-E\fIdx/dy/units[!]\fP \fB-F\fImode\fP 
\fB-G\fIgridkind\fP \fB-J\fIprojection\fP \fB-K\fIbackground\fP
\fB-L\fIlonflip\fP \fB-M -N -P\fIpings\fP \fB-Q\fP
\fB-S\fIspeed\fP \fB-T\fItension\fP \fB-U\fItime\fP 
\fB-V\fP -W\fIscale\fP \fB-X\fIextend\fP]

.SH DESCRIPTION
\fBmbgrid\fP is a utility used to grid bathymetry, amplitude, or sidescan 
data contained in a set of swath sonar data files. This program uses 
one of four algorithms to grid regions covered by swath sonar 
swaths and then can fill in gaps between the swaths (to the degree 
specified by the user) using a thin plate spline interpolation.
The gridding algorithms include gaussian weighted average,
median filter, minimum filter (use minimum value in grid bin), and
maximum filter (use maximum value in grid bin).

The user must specify a file containing a list of the data files to 
be used and their data formats (\fB-I\fP), and a character string 
to be used as the root of the output filenames (\fB-O\fP). The user 
may specify the bounds of the region to be gridded (\fB-R\fP), and 
either the dimensions (\fB-D\fP) or node spacing \fB-E\fP of the grid, 
If the bounds and grid dimensions (or spacing) are not specified, the
program will select the region encompassing all of the data in the
input files and a grid spacing equivalent to 0.02 times the maximum
sonar altitude. The user can also specify the type of the
input data (\fB-A\fP), the width of the gaussian filter used for 
weighted average gridding (\fB-W\fP), the maximum distance from 
data points that the spline interpolation is used (\fB-C\fP), the 
format of the output files, and other parameters.

By default, \fBmbgrid\fP generates grids in Geographic coordinates,
meaning that position is defined in longitude and latitude using
the WGS84 horizontal datum. The \fB-J\fP option can be used to specify 
an alternate, projected coordinate system (PCS). When a PCS is used,
position will be defined in eastings and northings (meters) relative
to the origin of the particular PCS. Universal Transverse Mercator
is the most commonly used PCS in the oceanographic community, but
\fBmbgrid\fP supports a large number of other PCS as well. A list of
the supported PCS's is provided at the end of this manual page.

Before opening an input swath data file, \fBmbgrid\fP checks for
an ascii file in the same directory having the same name except
that ".inf" is appended to the end. The program assumes that this
ascii file contains the output of the program \fBmbinfo\fP run on
the input data file. If the ".inf" file exists, \fBmbgrid\fP reads
the minimum and maximum longitude and latitude bounds from the 
\fBmbinfo\fP output and compares those to the working bounds
for the grid. If the ".inf" file indicates that none of the data
in the input file lies inside the working grid bounds, that input
file is skipped. This allows users to maintain a single master list
of data files for use in all gridding without the performance penalty
of \fBmbgrid\fP reading through all the data files, even those
with no relevent data. We recommend that users maintain a ".inf"
file for each swath data file used for gridding or plotting. The
programs \fBmbswath\fP and \fBmbcontour\fP also use ".inf" files
in the same fashion.

Usually, the internal working grid has the same boundaries as the
output grid. However, the \fB-X\fP option
allows the size of the internal grid to be increased so that data outside
the grid can be used to guide the spline interpolation of data gaps which
happen to lie at the the edge of the grid.  This is particularly important
when adjacent grids are created which should match along the edges.
The data input bounds are set to a region three times as large as
the working grid in both longitude and latitude.  The program reads all
pings which lie within the data input bounds, and accepts all beam
values with locations within the working grid. In addition to swath sonar
data in formats supported by \fBMB-System\fP (see the \fBMB-System\fP manual
page), \fBmbgrid\fP can also read data from ASCII text files in
longitude, latitude, value triples. This allows one to incorporate
conventional echosounder bathymetry data into the gridding.

If the default weighted average gridding scheme is being used, 
each data point's contribution to a Gaussian weighted
average for each nearby grid cell is calculated as the point is read
and added to
the grid cell sums.  The weighting function is given by:
 	W(r) = Aexp(-r**2/a**2)
.br
where r is the distance from the center of the grid point to the data point,
a is the distance at which the weighting function falls to 1/e of its
maximum value, and A is a normalizing factor set so that the sum of all
the weights adds to a value of 1.  Normally, the distance a is set to be
half the average grid point spacing, but this can be varied using the \fB-W\fP
option. 

If the \fB-F\fP\fI5\fP option is used, the gridding is performed using
beam footprints rather than a weighting function tied to the grid
cell spacing. In this algorithm, the beam footprints are calculated
using the angular beamwidths and the sonar altitude above the seafloor.
A weighting value is calculated for each grid cell which fully or
parially lies within the beam footprint; these weighting values 
represent the fraction of the beam contained within the cell. Each 
footprint is tilted according to the local slope; the slope is estimated
from a low resolution, first pass grid created by simple mean filtering 
the soundings. The slope grid is generated using a cell size twice that
of the final grid. This approach allows one to sensibly grid data using 
a resolution greater than that of the raw data. Thus, if one has data 
with a large depth variation, one can generate a grid with a cell spacing
appropriate for the high resolution, shallow data and still get
sensible results in deep regions where the grid cells may be
much smaller than the beam footprints.

If the \fB-F\fP\fI2\fP option is used, the gridding is performed with a
median filter scheme instead of a Gaussian weighted average. In this
approach, all of the values for each bin are held in memory until
all of the data has been read. Then, the median value for each bin
is assigned as the gridded value for that bin.  The advantage of a
median filter approach is that it is relatively insensitive to 
isolated artifacts in the data, provided that several samples
exist for each bin.  The disadvantage to the median filter is
that in the absence of artifacts, the weighted average scheme
does a better job of representing the gridded field, particularly
if the spectral characteristics of the gridded field are important.
The median filter approach also requires much more memory than
a weighted average. In general, edited bathymetry should be gridded
using the Gaussian weighted average, while unedited bathymetry,
beam amplitude, and sidescan data should be gridded using the
median filter.

The minimum filter (\fB-F\fP\fI3\fP) and maximum filter
(\fB-F\fP\fI4\fP) gridding schemes work like the median filter,
except that the minimum or maximum bin values are reported
instead of the median. These algorithms can be useful for 
producing grids which strongly reflect outliers in the
data. Hydrographers often prefer to grid bathymetry using a minimum
depth scheme because they are most interested in the
shallowest (most dangerous) soundings in their data.

Normally, all of the data which falls into the region of interest
is used to construct the gridded data set. This means that the
data from overlapping swaths will be "averaged" in the region of
overlap. Averaging bathymetry data from overlapping swaths is 
usually fine, but averaging imagery data (beam amplitude or sidescan) 
derived from different swaths is usually undesirable. The \fB-U\fP
option allows the user to force \fBmbgrid\fP to ignore data which
overlies regions already covered by previous data (as defined by
a time lag criterea). Alternatively, the user can force the program
to use only the last data in a region, again as defined by a time
lag. This option works best with the median filter scheme. 

The gridding can be augmented by interpolation using a 2D thin plate
spline algorithm with optional tension. The use of interpolation is
set with the \fB-C\fP\fIclip[/mode]\fP option, and is only used to fill
in grid cells left undefined after all of the swath data have been
processed. The \fIclip\fP value sets the distance from swath data to
which the interpolation is applied (this distance is specified as
an integer number of grid cells, so the physical distance is
\fIclip\fP times the grid cell interval). If 
\fImode\fP = 1 (the default), undefined cells will be filled with interpolation
only if cells filled with swath data are found within 
\fIclip\fP cells in two opposite directions 
(e.g east and west, or northeast and southwest). This approach serves
to fill in data gaps while avoiding adding an interpolated band around
the edges of a survey. A caution: a large value of \fIclip\fP 
combined with \fImode\fP = 1 will be VERY slow. If \fImode\fP = 2, then
undefined cells will be filled with the interpolation if they are within
\fIclip\fP cells of swath data in any direction. This approach is faster,
but can make a survey look larger and more complete than in reality.
If \fImode\fP = 3 or \fIclip\fP is 
set to a value greater than both dimensions of the output grid, then
all grid cells not set by swath data will be filled by interpolation.
The \fB-T\fP\fItension\fP option modulates the thin plate spline algorithm.
Using the default \fItension\fP = 0.0 corresponds to a minimum curvature,
pure Laplacian solution. If \fItension\fP is made large, the solution
tends toward a thin plate spline and is effectively flattened.

The \fB-K\fP\fIbackground\fP option is used to underlay a bathymetry or topography
grid with a global or regional topography model. The background data
model can be read from a GMT grid file or from a database accessed by the \fBGMT\fP program \fBgrdraster\fP.
In the former case \fIbackground\fP is just the file path for the 
background grid. In the latter case \fIbackground\fP is an 
identifier number used to specify which
dataset to extract using \fBgrdraster\fP. These identifiers are user
defined and vary with \fBGMT\fP installations. When the \fB-K\fP\fIbackground\fP
option is invoked, \fBgrd2xyz\fP or \fBgrdraster\fP is used to extract all of the longitude,
latitude, and topography values within the specified database that lie
within the desired grid. These values are interpolated onto the desired
grid locations using the thin plate spline algorithm, and then mapped onto
the grid wherever the values are undefined by either swath data or the
spline interpolation invoked with the \fB-C\fP option.

For magnetic inversions it is useful to have a bathymetry grid which
tails off to a constant value at the border.  To facilitate this, the
\fB-B\fP option allows the user to set the border of a smoothly
interpolated grid to a constant value wherever no data are present.  
This is useful only if the data are confined to the central region 
of the grid and a smooth interpolation is done with a large \fIclip\fP
so that the entire grid is filled.

The output grid will by default contain values of 99999.9 at cells
containing no data; if the \fB-N\fP option is used then the flagging
value used is NaN, or not-a-number.

The names of the output files are based on the root character string
specified using the \fB-O\fP option. A number of grid formats are 
supported, including all of the grid formats supported by \fBGMT\fP.
See the \fB-G\fP option below for a list of the available formats.
If the grid is output in any of the \fBGMT\fP grid formats,
then its filename is "root.grd", and a shellscript
which will allow the contents of the grid to be viewed using \fBGMT\fP programs
is also output with the filename "root.grd.cmd".  If the
\fB-G\fP\fI1\fP option is used to specify an ascii format grid, then the
output grid filename will be "root.asc", but no plotting shellscript 
will be created. 

A datalist file containing references to all of the swath files actually 
contibuting to the grid is also created. This file is named by adding 
a ".mb-1" suffix to the root string.

The \fB-M\fP option causes \fBmbgrid\fP to output two additional
grids, the first ("root_num.grd") being the number of data 
points located within each bin, and the second ("root_sd.grd") 
being the standard deviation of the
data points located within each bin.  Plotting shellscripts
called "root_num.grd.cmd" and "root_sd.grd.cmd" are also output
if the grids are in a \fBGMT\fP grid format. The \fB-M\fP
option is ignored when the minimum or maximum filter gridding
algorithms are used.

.SH AUTHORSHIP
David W. Caress (caress@mbari.org)
.br
  Monterey Bay Aquarium Research Institute
.br
Dale N. Chayes (dale@ldeo.columbia.edu)
.br
  Lamont-Doherty Earth Observatory

.SH OPTIONS
.TP
.B \-A
\fIdatatype\fP
.br
Sets the type of data to be read and gridded. If \fIdatatype\fP = 1,
bathymetry data will be gridded (positive downwards). If \fIdatatype\fP = 2,
bathymetry data will be gridded as topography (positive upwards).  
If \fIdatatype\fP = 3,
amplitude data will be gridded.  If \fIdatatype\fP = 4,
sidescan data will be gridded. Default: \fIdatatype\fP = 1 (bathymetry).
.TP
.B \-K
\fIbackground\fP
.br
Enables filling in all undefined grid cells with bathymetry or topography
from a global or regional database accessed with the \fBGMT\fP program
\fBgrdraster\fP. The \fIbackground\fP specifies which locally defined
database is accessed with \fBgrdraster\fP.
.TP
.B \-B
\fIborder\fP
.br
Sets the border of a smoothly interpolated grid to the value \fIborder\fP
wherever no data exist, provided \fIborder\fP > 0.0.
Default: \fIborder\fP = 0.0
.TP
.B \-C
\fIclip/mode\fP
.br
Controls the use of spline interpolation to fill grid cells not
filled by swath data. The \fIclip\fP value sets the distance from
swath data (in grid cells) that the spline interpolation may be 
applied. If \fIclip\fP=0 no spline interpolation will be done. 
If \fImode\fP=1 (the default), then the interpolation will fill
data gaps up to two times \fIclip\fP grid cells in size. If \fImode\fP=2 
then the spline interpolation will fill all undefined cells within
a distance of \fIclip\fP cells from data. If \fImode\fP = 3 or \fIclip\fP is 
set to a value greater than both dimensions of the output grid, then
all grid cells not set by swath data will be filled by interpolation.
Default: \fIclip\fP = 0 and \fImode\fP = 1.
.TP
.B \-D
\fIxdim/ydim\fP
.br
Sets the dimensions of the output grid.  This option is superceded
if the user specifies the grid spacing with the \fB-E\fP option.
Default:  \fIxdim\fP = \fIydim\fP = 101.
.TP
.B \-E
\fIdx/dy/units[!]\fP
.br
Sets the grid cell spacing to \fIdx\fP in longitude and \fIdy\fP
in latitude. If \fIunits\fP is not specified, the \fIdx\fP 
and \fIdy\fP values are assumed to be in meters. Valid values
for \fIunits\fP include "km", "meters", and "degrees".
If not in degrees, the grid cell spacing values are converted to
degrees using the central latitude of the grid. If 
\fIdy\fP = 0.0, then the latitude cell spacing will be set
equal to the longitude cell spacing (after conversion to
degrees, if necessary).
By default, the grid spacing is calculated from 
the grid bounds and the grid dimensions. When the user
uses the \fB-E\fP option to set the cell spacings, the
grid dimensions are calculated using the grid bounds and
grid cell spacings. However, slight adjustments to the
grid cell spacings are usually required to keep
the grid bounds as specified. Appending an ! to the end of 
the \fB-E\fP arguments forces \fBmbgrid\fP to use the
exact grid cell spacing values specified by adjusting the 
grid bounds.
Default: If neither the \fB-E\fP or \fB-D\P options are specified,
the program sets the grid cell spacing to be 0.02 times the maximum 
sonar altitude in the input files.
.TP
.B \-F
\fImode\fP
.br
Sets the gridding algorithm to be used.
 	\fImode\fP = 1:         Gaussian Weighted Mean
 	\fImode\fP = 2:         Median Filter
 	\fImode\fP = 3:         Minimum Filter
 	\fImode\fP = 4:         Maximum Filter
 	\fImode\fP = 5:         Weighted Sonar Footprint
The default is \fImode\fP = 1 (Gaussian Weighted Mean).
.TP
.B \-G
\fIgridkind\fP
.br
This option sets the format of the output grid file.
The default is to output a GMT netCDF 4-byte float grid.
The user may specify the grid format either by using an
identifier number, or by using the \fBGMT\fP grid format
syntax, which allows for scaling and offsets of the data.

First, the simple grid format identifiers are:
 	\fIgridkind\fP = 1:	ASCII table
 	\fIgridkind\fP = 2:	binary file (\fBGMT\fP version 1 GRD file)
 	\fIgridkind\fP = 3:	netCDF file (\fBGMT\fP version 2 GRD file)
 	\fIgridkind\fP = 4:	Arc/Info and ArcView ASCII grid
 	\fIgridkind\fP = 100:	GMT netCDF 4-byte float format [Default]
 	\fIgridkind\fP = 101:	Native binary single precision floats in scanlines with leading grd header
 	\fIgridkind\fP = 102:	Native binary short integers in scanlines with leading grd header
 	\fIgridkind\fP = 103:	8-bit standard Sun rasterfile (colormap ignored)
 	\fIgridkind\fP = 104:	Native binary unsigned char in scanlines with leading grd header
 	\fIgridkind\fP = 105:	Native binary bits in scanlines with leading grd header
 	\fIgridkind\fP = 106:	Native binary ``surfer'' grid files
 	\fIgridkind\fP = 107:	netCDF 1-byte byte format
 	\fIgridkind\fP = 108:	netCDF 1-byte char format
 	\fIgridkind\fP = 109:	netCDF 2-byte int format
 	\fIgridkind\fP = 110:	netCDF 4-byte int format
 	\fIgridkind\fP = 111:	netCDF 8-byte double format
.br

Second, a \fBGMT\fP grid format string has the form:
.br
 	=id[/scale/offset[/nan]]
.br
where id is the \fBGMT\fP format id, and the other values
are optional. If \fIscale\fP and \fIoffset\fP are given, the
data will be multiplied by \fIscale\fP and offset by \fIoffset\fP
prior to being output. The \fInan\fP value sets the value used for
"not-a-number". The \fBGMT\fP format id's for these format strings are:
 	\fIid\fP = 0:	GMT netCDF 4-byte float format [Default]
 	\fIid\fP = 1:	Native binary single precision floats in scanlines with leading grd header
 	\fIid\fP = 2:	Native binary short integers in scanlines with leading grd header
 	\fIid\fP = 3:	8-bit standard Sun rasterfile (colormap ignored)
 	\fIid\fP = 4:	Native binary unsigned char in scanlines with leading grd header
 	\fIid\fP = 5:	Native binary bits in scanlines with leading grd header
 	\fIid\fP = 6:	Native binary ``surfer'' grid files
 	\fIid\fP = 7:	netCDF 1-byte byte format
 	\fIid\fP = 8:	netCDF 1-byte char format
 	\fIid\fP = 9:	netCDF 2-byte int format
 	\fIid\fP = 10:	netCDF 4-byte int format
 	\fIid\fP = 11:	netCDF 8-byte double format
.br
Note that the following arguments are equivalent because they
all produce a standard \fBGMT\fP netCDF 4-byte float grid:
 	\fB-G\fP\fI3\fP
 	\fB-G\fP\fI100\fP
 	\fB-G\fP\fI=0\fP
.br
Should the user wish to produce a grid in native binary floats,
then the following two arguments will work:
 	\fB-G\fP\fI101\fP
 	\fB-G\fP\fI=1\fP
.br
Should the user wish to produce a grid in native short int format
with a scaling factor of 10, an offset of 32000, and a NaN value
of 32767, then the following arguments will suffice:
.br
 	\fB-G\fP\fI=2/10/32000/32767\fP
 	
If \fIgridkind\fP = 3 or \fIgridkind\fP >= 100, 
\fBmbgrid\fP also outputs shellscripts 
which run \fBGMT\fP programs to provide preliminary color
fill maps of the gridded data. These shellscripts are 
generated using the \fBmbm_grdplot\fP macro.

If \fIgridkind\fP = 4, the output grids will be in the Arc/Info ASCII 
grid format.  Arc/Info ASCII grids use "square" bins,
meaning that the longitude and latitude grid cell spacings must
be identical. Thus, whenever the \fIgridkind\fP = 4 option is
used, the \fB-E\fP option must also be used in a way which
ensures equal grid cell spacings (see the \fB-E\fP documentation
above).
Default: \fIgridkind\fP = 100.
.TP
.B \-H
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \-I
\fIdatalist\fP
.br
Sets the filename of the file containing a list of the input swath sonar
data files and their formats.  In the \fIdatalist\fP file, each
data file should be followed by a data format identifier, e.g.:
 	datafile1 11
 	datafile2 24
.br
This program uses the \fBMBIO\fP library and will read any swath sonar
format supported by \fBMBIO\fP. A list of the swath sonar data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page.  A format identifier of 0 indicates
that the file contains lines of (lon, lat, depth) triples which can be read
in free format.
.br
An input datafile may be accompanied by a "fast bathymetry" or "fbt" file. 
An "fbt" file contains only swath bathymetry information in a compact format (format 71),
and is thus quick to read. The "fbt" fine naming convention is to add the 
".fbt" suffix to the original swath
data filename. In the
event that a bathymetry or topography grid is being generated, \fBmbgrid\fP
will attempt to read an "fbt" file in lieu of the original data.
Default: \fIdatalist\fP = datalist.mb-1
.TP
.B \-J
\fIprojection\fP
By default, \fBmbgrid\fP generates grids in Geographic coordinates,
meaning that position is defined in longitude and latitude using
the WGS84 geographic coordinate system. 
The \fB-J\fP option can be used to specify 
an alternate, projected coordinate system (PCS). When a PCS is used,
position will be defined in eastings and northings (meters) relative
to the origin of the particular PCS. Universal Transverse Mercator
is the most commonly used PCS in the oceanographic community, but
\fBmbgrid\fP supports a large number of other PCS's as well. 
The underlying projection functions derive from the \fBPROJ.4\fP library
written by Gerald Evenden, then of the U.S. Geological Survey.

The \fIprojection\fP argument for the \fB-J\fP option can be either
a PCS identifier from the projection definition list provided at the 
end of this manual page, or simply \fB-J\fP\fIU\fP to specify using
UTM in whatever zone is appropriate for the grid bounds specified
with the \fB-R\fP option.

For instance, to fully specify a particular northern UTM 
zone, set \fIprojection\fP = UTMXXN where XX gives
the UTM zone (defined from 01 to 60). As an example, a northern UTM
zone 12 projection can be specified using \fB-J\fP\fIUTM12N\fP.
Southern UTM zones are specified as UTMXXS. The European Petroleum
Survey Group (EPSG) has defined a large number of PCS's used worldwide
and assigned number id's to each; one can also specify the northern
UTM zone 12 projection using its EPSG designation, 
or \fB-J\fP\fIepsg32612\fP.
When the projected coordinate system is fully specified
by the \fB-J\fP option, then the grid bounds may be specified using
\fB-R\fP in either longitude and latitude or in eastings and northings.

Alternatively, one may indicate a UTM projection without specifying the
zone by using \fB-J\fP\fIU\fP. In this case, the 
UTM zone will be inferred from the midpoint of the 
specified longitude and latitude bounds, and then the 
longitude and latitude bounds given with the 
\fB-fR\fP option are translated to UTM eastings and northings.

All grids and mosaics produced by \fBMB-System\fP 
programs contain identifiers that are recognized
by the plotting macros \fBmbm_grdplot\fP, 
\fBmbm_grd3dplot\fP, and \fBmbm_grdtiff\fP.
These plotting macros automatically use a 
linear map projection whenever they
encounter grids and mosaics that are already 
in a projected coordinate system. Also, the program \fBmbgrdtiff\fP
automatically inserts the appropriate projection information into
the GeoTIFF images it generates. As a result, images generated by
\fBmbgrdtiff\fP will be properly georeferenced when they are imported
into GIS software.
 
.TP
.B \-K
\fIbackground\fP
.br
Enables filling in all undefined grid cells with bathymetry or topography
from a global or regional database. If raster data have been locally made 
available through the \fBGMT\fP program \fBgrdraster\fP, these can be 
accessed by specifying \fIbackground\fP as the \fBgrdraster\fP dataset id number
(e.g. \fB-K\fP\fI4\fP - note that \fBgrdraster\fP 
datasets are numbered starting at 1).
Users can also use data in GMT grids as a background (including grids
generated by \fBmbgrid\fP) by specifying \fIbackground\fP as the file path 
to the desired grid (e.g. \fB-K\fP\fI/usr/local/share/globaltopo.grd\fP).
 
.TP
.B \-L
\fIlonflip\fP
.br
Sets the range of the longitude values returned.
If \fIlonflip\fP=-1 then the longitude values will be in
the range from -360 to 0 degrees. If \fIlonflip\fP=0 
then the longitude values will be in
the range from -180 to 180 degrees. If \fIlonflip\fP=1 
then the longitude values will be in
the range from 0 to 360 degrees.
Default: \fIlonflip\fP = 0.
.TP
.B \-M
Causes two additional grids to be output. One is a grid containing
the standard deviation of the data within each grid cell relative
to the grid value, the other contains the number of data 
points in each grid cell. This option is ignored when the minimum
or maximum filter gridding algorithms are used (see the \fP-F\fP option).
.TP
.B \-N
Causes grid cells with no data and no interpolation to be set to a
value of NaN instead of the default value of 99999.9.  The NaN value
is expected by \fBGMT\fP programs such \fBgrdview\fP.
.TP
.B \-O
\fIroot\fP
.br
Sets the character string to be used as the root of the
output filenames. For example, if the grid is output as a \fBGMT\fP 
version 2 GRD format (netCDF) file (the default), 
then its filename is "root.grd". If the
\fB-G\fP\fI1\fP option is used to specify an ascii format grid, then the
output grid filename will be "root.asc". If the
\fB-G\fP\fI2\fP option is used to specify a version 1 GRD 
format (binary) grid, then the
output grid filename will be "root.grd1". If the output grid is
in the \fBGMT\fP version 2 GRD format, a shellscript
which will allow the contents of the grid to viewed using \fBGMT\fP programs
is also output with the filename "root.grd.cmd".
.TP
.B \-P
\fIpings\fP
Sets the ping averaging of the input data. If \fIpings\fP > 0, then
that number of input pings will be averaged to produce one output
ping.  If \fIpings\fP = 0, then the ping averaging will automatically
be done so that the along-track ping spacing is equal to the across-track
beam spacing. Default: \fIpings\fP = 1.
.TP
.B \-Q
Normally, bathymetry or topography data is gridded in meters. If
this option is used, bathymetry or topography data is gridded
in feet.
.TP
.B \-R
\fIwest/east/south/north\fP
.br
Sets the longitude and latitude bounds of the output grid. If the user
uses the \fB-E\fP option to set the grid spacing, then the dimensions
will be calculated from the grid bounds and spacing. In these
circumstances rounding errors will usually require that the eastern
and northern bounds be adjusted to fit exactly with the 
grid dimensions and spacing.
Default: If the \fB-R\fP option is not specified, the program will set
the grid bounds to be the area encompassing all of the data in the
input files.
.TP
.B \-S
\fIspeed\fP
.br
Sets the minimum speed in km/hr (5.5 kts ~ 10 km/hr) allowed in 
the input data; pings associated with a smaller ship speed will not be
output. Default: \fIspeed\fP = 0.
.TP
.B \-T
\fItension\fP
.br
Sets the \fItension\fP value used in the thin plate spline interpolation.
 
A \fItension\fP of 0 gives a minimum curvature surface with free edges;
this is a pure Laplacian solution. A nonzero \fItension\fP tends to 
suppress spurious oscillations and flatten
the interpolation toward the edges; a \fItension\fP of infinity
yields a pure thin plate spline solution. The \fItension\fP must be zero or
greater.  
Default: \fItension\fP = 0.0 (minimum curvature solution).
.TP
.B \-U
\fItime\fP
.br
Forces \fBmbgrid\fP to avoid averaging overlapping swaths by
ignoring the data from later swaths. "Later" data is identified
using the \fItime\fP value. The time of the first data point
is saved for each bin in the grid; any other data points which
are more than \fItime\fP minutes before or after the initial
data point in the relevent bin are ignored. If \fItime\fP is
negative, the last data in a bin (within the time lag criterea)
will be saved and used instead of the first data.
.TP
.B \-V-
Normally, \fBmbgrid\fP prints out information regarding its
controlling parameters during execution; the 
\fB-V\fP option causes the program to also print out statements
indicating its progress.
.TP
.B \-W
\fIscale\fP
.br
Sets the width of the gaussian weighting function in terms of
the grid spacing.  The distance to the 1/e point of the weighting
function is given by half of the grid spacing times \fIscale\fP.
Default: \fIscale\fP = 1.0
.TP
.B \-X
\fIextend\fP
.br
Extends the size of the internal grid so that the output grid is a
subset from the center of a larger grid.   This allows data outside
the output grid to guide the spline interpolation of data gaps which
happen to lie at the the edge of the output grid.  The amount of 
extension is \fIextend\fP times the grid width/height to each side.
Thus, if \fIextend\fP=1.0, then the internal grid will have dimensions
three times the output grid.
Default: \fIextend\fP = 0.0
.SH EXAMPLES
Suppose you want to grid some Hydrosweep data in six data files over
a region with longitude bounds of 139.9W to 139.65W and latitude bounds
of 9.7S to 9.45S.  To get a 110 m grid spacing, you need a grid dimensions
of 251 (x or longitude) and 251 (y or latitude). First,
create a datalist file using a text editor which contains the
data filenames followed by the appropriate format identifier:

 	d123e.mb24 24
 	d126e.mb24 24
 	d128e.mb24 24
 	d129e.mb24 24
 	d130e.mb24 24
 	d131e.mb24 24

Then, run \fBmbgrid\fP as follows:
 	mbgrid -R220.1/220.35/-9.7/-9.45 -D251/251 \\
 		-L1 -C251  -N \\
 		-Idatalist -Ourville_int -V

By specifying a clipping dimension of 251 we insure that the grid
will be filled in through spine interpolation even in the areas
not covered by data. The output looks like:
 
 Program MBGRID
 Version $Id$
 MB-system Version 4.5
 
 MBGRID Parameters:
 List of input files: datalist
 Output fileroot:     urville_int
 Input Data Type:     Bathymetry
 Gridding algorithm:  Gaussian Weighted Mean
 Grid dimensions: 251 251
 Grid bounds:
   Longitude:  220.1000  220.3500
   Latitude:    -9.7000   -9.4500
 Working grid dimensions: 251 251
 Working Grid bounds:
   Longitude:  220.1000  220.3500
   Latitude:    -9.7000   -9.4500
 Input data bounds:
   Longitude:  219.8500  220.6000
   Latitude:    -9.9500   -9.2000
 Longitude interval: 0.001000 degrees or 109.778801 m
 Latitude interval:  0.001000 degrees or 110.605002 m
 Gaussian filter 1/e length: 0.055096 km
 Spline interpolation applied with clipping dimension: 251
 Spline tension (range 0.0 to 1.0): 10000000000.000000
 Grid format 3:  GMT version 2 grd (netCDF)
 NaN values used to flag regions with no data
 MBIO parameters:
   Ping averaging:       1
   Longitude flipping:   1
   Speed minimum:       0.0 km/hr
  
 29075 data points processed in d123e.mb8
 0 data points processed in d125e.mb8
 98175 data points processed in d126e.mb8
 68637 data points processed in d128e.mb8
 20703 data points processed in d129e.mb8
 80372 data points processed in d130e.mb8
 55620 data points processed in d131e.mb8
 
 352582 total data points processed
 
 Making raw grid...
 
 Doing spline interpolation with 53513 data points...
 
 Total number of bins:            63001
 Bins set using data:             53513
 Bins set using interpolation:    9488
 Bins not set:                    0
 Maximum number of data in a bin: 65
 Minimum value:     504.93   Maximum value:    3405.75
 Minimum sigma:    0.14060   Maximum sigma:  275.53399
 
 Outputting results...
 
 executing mbm_grdplot...
 mbm_grdplot -Iurville_int.grd -G1 -C -D -V -L"File urville_int.grd - Bathymetry Grid:Depth (m)"
 
 Program Status:
 --------------
 
   Plot Style:
     Color Fill
     Contours
     Horizontal Color Scale
 
   Input Files:
     Data GRD File:            urville_int.grd
     Intensity GRD List File:   
 
   Output Files:
     Output plot name root:    urville_int.grd
     Color pallette table:     urville_int.grd.cpt
     Plotting shellscript:     urville_int.grd.cmd
     Plot file:                urville_int.grd.ps
 
   Plot Attributes:
     Plot width:               6.5000
     Plot height:              6.5489
     Page size:                a
     Page width:               8.5
     Page height:              11
     Projection:           -Jm26
     Axes annotation:          5m/5m:."File urville_int.grd - Bathymetry Grid":
     Orientation:              portrait
      Number of colors:         11
     Color Pallette:           Haxby Colors
     Colors reversed
 
   Grid Data Attributes:
     Longitude min max:         220.1000   220.3500
     Latitude min max:       -9.7000    -9.4500
     Data min max:                 504.9       3406
 
   Primary Grid Plotting Controls:
     Contour control:          100
     Color start datum:        350.000000
     Color end datum:          3850.000000
     Color datum interval:     350.000000
 
   GMT Default Values Reset in Script:
     PAPER_WIDTH               8.5
     ANOT_FONT                 Helvetica
     LABEL_FONT                Helvetica
     HEADER_FONT               Helvetica
     ANOT_FONT_SIZE            8
     LABEL_FONT_SIZE           8
     HEADER_FONT_SIZE          10
     FRAME_WIDTH               0.074999999999999997
     TICK_LENGTH               0.074999999999999997
     PAGE_ORIENTATION          LANDSCAPE
     COLOR_BACKGROUND          0/0/0
     COLOR_FOREGROUND          255/255/255
     COLOR_NAN                 255/255/255
 
 --------------
 
 Plot generation shellscript <urville_int.grd.cmd> created.
 
 Instructions:
   Execute <urville_int.grd.cmd> to generate Postscript plot <urville_int.grd.ps>.
   Executing <urville_int.grd.cmd> also invokes xpsview to view the plot on the screen.
 
 --------------
 
 
 Done.

The names of the output files are based on the root character string
specified using the \fB-O\fP option.  Since the grid is output as a netCDF
GRD format file, its filename is "urville_int.grd"; a shellscript
which will allow the contents of the grid to viewed using \fBGMT\fP programs
is also output with the filename "urville_int.grd.cmd".

Suppose that one wants to grid the same data considered above using
the median filtering scheme instead of the default Gaussian weighted
mean scheme and also outputting grids of the data density and 
standard deviation values. The following will suffice:
 	mbgrid -R220.1/220.35/-9.7/-9.45 -D251/251 \\
 		-L1 -C251 -N \\
 		-Idatalist -Ourville_int -F2 -M -V

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbmosaic\fP(l), \fBmbm_grid\fP(l), \fBmbm_grd2arc\fP(l)

.SH BUGS
The options for this program have grown a bit complicated.
If you have other problems, please let us know.

.SH APPENDIX 1: PROJECTED COORDINATE SYSTEM IDENTIFIERS
The following is a list of the projected coordinate systems (PCS's)
that are supported by MB-System. The full PCS definitions are
found in the file mbsystem/share/Projections.dat. These definitions
are in the \fBPROJ.4\fP format and derive from the \fBPROJ.4\fP
4.6.1 distribution obtained from http://trac.osgeo.org/proj/ 
in September 2008. The proj library source code has been 
incorporated unchanged into the MB-System package.

The first item on each line is the PCS identifier inside brackets, such
as <UTM10N> or <epsg32749>. To specify using one of these PCS's, use
the \fB-J\fP option, e.g. \fB-J\fP\fIUTM10N\fP or \fB-J\fP\fIepsg32749\fP.

 ---------------------------------------------------------
 Standard Universal Transverse Mercator (UTM)
     and Universal Polar Stereographic (UPS)
     projected coordinate systems for MB-System
 ---------------------------------------------------------

 <UTM01N> : WGS 84 / UTM zone 1N
 <UTM02N> : WGS 84 / UTM zone 2N
 <UTM03N> : WGS 84 / UTM zone 3N
 <UTM04N> : WGS 84 / UTM zone 4N
 <UTM05N> : WGS 84 / UTM zone 5N
 <UTM06N> : WGS 84 / UTM zone 6N
 <UTM07N> : WGS 84 / UTM zone 7N
 <UTM08N> : WGS 84 / UTM zone 8N
 <UTM09N> : WGS 84 / UTM zone 9N
 <UTM10N> : WGS 84 / UTM zone 10N
 <UTM11N> : WGS 84 / UTM zone 11N
 <UTM12N> : WGS 84 / UTM zone 12N
 <UTM13N> : WGS 84 / UTM zone 13N
 <UTM14N> : WGS 84 / UTM zone 14N
 <UTM15N> : WGS 84 / UTM zone 15N
 <UTM16N> : WGS 84 / UTM zone 16N
 <UTM17N> : WGS 84 / UTM zone 17N
 <UTM18N> : WGS 84 / UTM zone 18N
 <UTM19N> : WGS 84 / UTM zone 19N
 <UTM20N> : WGS 84 / UTM zone 20N
 <UTM21N> : WGS 84 / UTM zone 21N
 <UTM22N> : WGS 84 / UTM zone 22N
 <UTM23N> : WGS 84 / UTM zone 23N
 <UTM24N> : WGS 84 / UTM zone 24N
 <UTM25N> : WGS 84 / UTM zone 25N
 <UTM26N> : WGS 84 / UTM zone 26N
 <UTM27N> : WGS 84 / UTM zone 27N
 <UTM28N> : WGS 84 / UTM zone 28N
 <UTM29N> : WGS 84 / UTM zone 29N
 <UTM30N> : WGS 84 / UTM zone 30N
 <UTM31N> : WGS 84 / UTM zone 31N
 <UTM32N> : WGS 84 / UTM zone 32N
 <UTM33N> : WGS 84 / UTM zone 33N
 <UTM34N> : WGS 84 / UTM zone 34N
 <UTM35N> : WGS 84 / UTM zone 35N
 <UTM36N> : WGS 84 / UTM zone 36N
 <UTM37N> : WGS 84 / UTM zone 37N
 <UTM38N> : WGS 84 / UTM zone 38N
 <UTM39N> : WGS 84 / UTM zone 39N
 <UTM40N> : WGS 84 / UTM zone 40N
 <UTM41N> : WGS 84 / UTM zone 41N
 <UTM42N> : WGS 84 / UTM zone 42N
 <UTM43N> : WGS 84 / UTM zone 43N
 <UTM44N> : WGS 84 / UTM zone 44N
 <UTM45N> : WGS 84 / UTM zone 45N
 <UTM46N> : WGS 84 / UTM zone 46N
 <UTM47N> : WGS 84 / UTM zone 47N
 <UTM48N> : WGS 84 / UTM zone 48N
 <UTM49N> : WGS 84 / UTM zone 49N
 <UTM50N> : WGS 84 / UTM zone 50N
 <UTM51N> : WGS 84 / UTM zone 51N
 <UTM52N> : WGS 84 / UTM zone 52N
 <UTM53N> : WGS 84 / UTM zone 53N
 <UTM54N> : WGS 84 / UTM zone 54N
 <UTM55N> : WGS 84 / UTM zone 55N
 <UTM56N> : WGS 84 / UTM zone 56N
 <UTM57N> : WGS 84 / UTM zone 57N
 <UTM58N> : WGS 84 / UTM zone 58N
 <UTM59N> : WGS 84 / UTM zone 59N
 <UTM60N> : WGS 84 / UTM zone 60N
 <UTM01S> : WGS 84 / UTM zone 1S
 <UTM02S> : WGS 84 / UTM zone 2S
 <UTM03S> : WGS 84 / UTM zone 3S
 <UTM04S> : WGS 84 / UTM zone 4S
 <UTM05S> : WGS 84 / UTM zone 5S
 <UTM06S> : WGS 84 / UTM zone 6S
 <UTM07S> : WGS 84 / UTM zone 7S
 <UTM08S> : WGS 84 / UTM zone 8S
 <UTM09S> : WGS 84 / UTM zone 9S
 <UTM10S> : WGS 84 / UTM zone 10S
 <UTM11S> : WGS 84 / UTM zone 11S
 <UTM12S> : WGS 84 / UTM zone 12S
 <UTM13S> : WGS 84 / UTM zone 13S
 <UTM14S> : WGS 84 / UTM zone 14S
 <UTM15S> : WGS 84 / UTM zone 15S
 <UTM16S> : WGS 84 / UTM zone 16S
 <UTM17S> : WGS 84 / UTM zone 17S
 <UTM18S> : WGS 84 / UTM zone 18S
 <UTM19S> : WGS 84 / UTM zone 19S
 <UTM20S> : WGS 84 / UTM zone 20S
 <UTM21S> : WGS 84 / UTM zone 21S
 <UTM22S> : WGS 84 / UTM zone 22S
 <UTM23S> : WGS 84 / UTM zone 23S
 <UTM24S> : WGS 84 / UTM zone 24S
 <UTM25S> : WGS 84 / UTM zone 25S
 <UTM26S> : WGS 84 / UTM zone 26S
 <UTM27S> : WGS 84 / UTM zone 27S
 <UTM28S> : WGS 84 / UTM zone 28S
 <UTM29S> : WGS 84 / UTM zone 29S
 <UTM30S> : WGS 84 / UTM zone 30S
 <UTM31S> : WGS 84 / UTM zone 31S
 <UTM32S> : WGS 84 / UTM zone 32S
 <UTM33S> : WGS 84 / UTM zone 33S
 <UTM34S> : WGS 84 / UTM zone 34S
 <UTM35S> : WGS 84 / UTM zone 35S
 <UTM36S> : WGS 84 / UTM zone 36S
 <UTM37S> : WGS 84 / UTM zone 37S
 <UTM38S> : WGS 84 / UTM zone 38S
 <UTM39S> : WGS 84 / UTM zone 39S
 <UTM40S> : WGS 84 / UTM zone 40S
 <UTM41S> : WGS 84 / UTM zone 41S
 <UTM42S> : WGS 84 / UTM zone 42S
 <UTM43S> : WGS 84 / UTM zone 43S
 <UTM44S> : WGS 84 / UTM zone 44S
 <UTM45S> : WGS 84 / UTM zone 45S
 <UTM46S> : WGS 84 / UTM zone 46S
 <UTM47S> : WGS 84 / UTM zone 47S
 <UTM48S> : WGS 84 / UTM zone 48S
 <UTM49S> : WGS 84 / UTM zone 49S
 <UTM50S> : WGS 84 / UTM zone 50S
 <UTM51S> : WGS 84 / UTM zone 51S
 <UTM52S> : WGS 84 / UTM zone 52S
 <UTM53S> : WGS 84 / UTM zone 53S
 <UTM54S> : WGS 84 / UTM zone 54S
 <UTM55S> : WGS 84 / UTM zone 55S
 <UTM56S> : WGS 84 / UTM zone 56S
 <UTM57S> : WGS 84 / UTM zone 57S
 <UTM58S> : WGS 84 / UTM zone 58S
 <UTM59S> : WGS 84 / UTM zone 59S
 <UTM60S> : WGS 84 / UTM zone 60S
 <UPSN> : WGS 84 / UPS North
 <UPSS> : WGS 84 / UPS South

 ---------------------------------------------------------
 Listing of State Plane North American Datum Zones
 ---------------------------------------------------------
   MB-System projection ids are the zone number
   prefixed by either "nad27sp" or "nad83sp"
 ----------------------------------------------
                                NGS zone number
 State and zone                  1927     1983
 ----------------------------------------------
 Alabama east ..................  101      101
 Alabama west ..................  102      102
 Alaska zone no. 1 ............. 5001     5001
 Alaska zone no. 2 ............. 5002     5002
 Alaska zone no. 3 ............. 5003     5003
 Alaska zone no. 4 ............. 5004     5004
 Alaska zone no. 5 ............. 5005     5005
 Alaska zone no. 6 ............. 5006     5006
 Alaska zone no. 7 ............. 5007     5007
 Alaska zone no. 8 ............. 5008     5008
 Alaska zone no. 9 ............. 5009     5009
 Alaska zone no. 10 ............ 5010     5010
 American Samoa ................ 5300
 Arizona central ...............  202      202
 Arizona east ..................  201      201
 Arizona west ..................  203      203
 Arkansas north ................  301      301
 Arkansas south ................  302      302
 California I ..................  401      401
 California II .................  402      402
 California III ................  403      403
 California IV .................  404      404
 California V ..................  405      405
 California VI .................  406      406
 California VII ................  407
 Colorado central ..............  502      502
 Colorado north ................  501      501
 Colorado south ................  503      503
 Connecticut ...................  600      600
 Delaware ......................  700      700
 Florida east ..................  901      901
 Florida north .................  903      903
 Florida west ..................  902      902
 Georgia east .................. 1001     1001
 Georgia west .................. 1002     1002
 Guam Island ................... 5400
 Hawaii 1 ...................... 5101     5101
 Hawaii 2 ...................... 5102     5102
 Hawaii 3 ...................... 5103     5103
 Hawaii 4 ...................... 5104     5104
 Hawaii 5 ...................... 5105     5105
 Idaho central ................. 1102     1102
 Idaho east .................... 1101     1101
 Idaho west .................... 1103     1103
 Illinois east ................. 1201     1201
 Illinois west ................. 1202     1202
 Indiana east .................. 1301     1301
 Indiana west .................. 1302     1302
 Iowa north .................... 1401     1401
 Iowa south .................... 1402     1402
 Kansas north .................. 1501     1501
 Kansas south .................. 1502     1502
 Kentucky north ................ 1601     1601
 Kentucky south ................ 1602     1602
 Louisiana north ............... 1701     1701
 Louisiana offshore ............ 1703     1703
 Louisiana south ............... 1702     1702
 Maine east .................... 1801     1801
 Maine west .................... 1802     1802
 Maryland ...................... 1900     1900
 Massachusetts island .......... 2002     2002
 Massachusetts mainland ........ 2001     2001
 Michigan central/l ............ 2112     2112 current
 Michigan central/m ............ 2102          old
 Michigan east ................. 2101          old
 Michigan north ................ 2111     2111 current
 Michigan south ................ 2113     2113 current
 Michigan west ................. 2103          old
 Minnesota central ............. 2202     2202
 Minnesota north ............... 2201     2201
 Minnesota south ............... 2203     2203
 Mississippi east .............. 2301     2301
 Mississippi west .............. 2302     2302
 Missouri central .............. 2402     2402
 Missouri east ................. 2401     2401
 Missouri west ................. 2403     2403
 Montana .......................          2500
 Montana central ............... 2502
 Montana north ................. 2501
 Montana south ................. 2503
 Nebraska ......................          2600
 Nebraska north ................ 2601
 Nebraska south ................ 2602
 Nevada central ................ 2702     2702
 Nevada east ................... 2701     2701
 Nevada west ................... 2703     2703
 New hampshire ................. 2800     2800
 New jersey .................... 2900     2900
 New mexico central ............ 3002     3002
 New mexico east ............... 3001     3001
 New mexico west ............... 3003     3003
 New york central .............. 3102     3102
 New york east ................. 3101     3101
 New york long island .......... 3104     3104
 New york west ................. 3103     3103
 North carolina ................ 3200     3200
 North dakota north ............ 3301     3301
 North dakota south ............ 3302     3302
 Ohio north .................... 3401     3401
 Ohio south .................... 3402     3402
 Oklahoma north ................ 3501     3501
 Oklahoma south ................ 3502     3502
 Oregon north .................. 3601     3601
 Oregon south .................. 3602     3602
 Pennsylvania north ............ 3701     3701
 Pennsylvania south ............ 3702     3702
 Puerto Rico, Virgin Islands ... 5201     5200
 Rhode Island .................. 3800     3800
 South Carolina ................          3900
 South Carolina north .......... 3901
 South Carolina south .......... 3902
 South Dakota north ............ 4001     4001
 South Dakota south ............ 4002     4002
 Tennessee ..................... 4100     4100
 Texas central ................. 4203     4203
 Texas north ................... 4201     4201
 Texas north central ........... 4202     4202
 Texas south ................... 4205     4205
 Texas south central ........... 4204     4204
 Utah central .................. 4302     4302
 Utah north .................... 4301     4301
 Utah south .................... 4303     4303
 Vermont ....................... 4400     4400
 Virgin Islands, St. Croix ..... 5202
 Virginia north ................ 4501     4501
 Virginia south ................ 4502     4502
 Washington north .............. 4601     4601
 Washington south .............. 4602     4602
 West Virginia north ........... 4701     4701
 West Virginia south ........... 4702     4702
 Wisconsin central ............. 4802     4802
 Wisconsin north ............... 4801     4801
 Wisconsin south ............... 4803     4803
 Wyoming east .................. 4901     4901
 Wyoming east central .......... 4902     4902
 Wyoming west .................. 4904     4904
 Wyoming west central .......... 4903     4903
 ----------------------------------------------

 ---------------------------------------------------------
 State Plane Coordinate Systems
 North American Datum 1927
 ---------------------------------------------------------
 <nad27sp101> : alabama east> : nad27sp
 <nad27sp102> : alabama west> : nad27sp
 <nad27sp5010> : alaska zone no. 10> : nad27sp
 <nad27sp5300> : american samoa> : nad27sp
 <nad27sp201> : arizona east> : nad27sp
 <nad27sp202> : arizona central> : nad27sp
 <nad27sp203> : arizona west> : nad27sp
 <nad27sp301> : arkansas north> : nad27sp
 <nad27sp302> : arkansas south> : nad27sp
 <nad27sp401> : california i> : nad27sp
 <nad27sp402> : california ii> : nad27sp
 <nad27sp403> : california iii> : nad27sp
 <nad27sp404> : california iv> : nad27sp
 <nad27sp405> : california v> : nad27sp
 <nad27sp406> : california vi> : nad27sp
 <nad27sp407> : california vii> : nad27sp
 <nad27sp501> : colorado north> : nad27sp
 <nad27sp502> : colorado central> : nad27sp
 <nad27sp503> : colorado south> : nad27sp
 <nad27sp600> : connecticut ---> : nad27sp
 <nad27sp700> : delaware ---> : nad27sp
 <nad27sp901> : florida east> : nad27sp
 <nad27sp902> : florida west> : nad27sp
 <nad27sp903> : florida north> : nad27sp
 <nad27sp1001> : georgia east> : nad27sp
 <nad27sp1002> : georgia west> : nad27sp
 <nad27sp5101> : hawaii 1> : nad27sp
 <nad27sp5102> : hawaii 2> : nad27sp
 <nad27sp5103> : hawaii 3> : nad27sp
 <nad27sp5104> : hawaii 4> : nad27sp
 <nad27sp5105> : hawaii 5> : nad27sp
 <nad27sp1101> : idaho east> : nad27sp
 <nad27sp1102> : idaho central> : nad27sp
 <nad27sp1103> : idaho west> : nad27sp
 <nad27sp1201> : illinois east> : nad27sp
 <nad27sp1202> : illinois west> : nad27sp
 <nad27sp1301> : indiana east> : nad27sp
 <nad27sp1302> : indiana west> : nad27sp
 <nad27sp1401> : iowa north> : nad27sp
 <nad27sp1402> : iowa south> : nad27sp
 <nad27sp1501> : kansas north> : nad27sp
 <nad27sp1502> : kansas south> : nad27sp
 <nad27sp1601> : kentucky north> : nad27sp
 <nad27sp1602> : kentucky south> : nad27sp
 <nad27sp1701> : louisiana north> : nad27sp
 <nad27sp1702> : louisiana south> : nad27sp
 <nad27sp1703> : louisiana offshore> : nad27sp
 <nad27sp1801> : maine east> : nad27sp
 <nad27sp1802> : maine west> : nad27sp
 <nad27sp1900> : maryland ---> : nad27sp
 <nad27sp2001> : massachusetts mainland> : nad27sp
 <nad27sp2002> : massachusetts island> : nad27sp
 <nad27sp2101> : michigan east> : nad27sp
 <nad27sp2102> : michigan central/m> : nad27sp
 <nad27sp2103> : michigan west> : nad27sp
 <nad27sp2111> : michigan north> : nad27sp
 <nad27sp2112> : michigan central/l> : nad27sp
 <nad27sp2113> : michigan south> : nad27sp
 <nad27sp2201> : minnesota north> : nad27sp
 <nad27sp2202> : minnesota central> : nad27sp
 <nad27sp2203> : minnesota south> : nad27sp
 <nad27sp2301> : mississippi east> : nad27sp
 <nad27sp2302> : mississippi west> : nad27sp
 <nad27sp2401> : missouri east> : nad27sp
 <nad27sp2402> : missouri central> : nad27sp
 <nad27sp2403> : missouri west> : nad27sp
 <nad27sp2501> : montana north> : nad27sp
 <nad27sp2502> : montana central> : nad27sp
 <nad27sp2503> : montana south> : nad27sp
 <nad27sp2601> : nebraska north> : nad27sp
 <nad27sp2602> : nebraska south> : nad27sp
 <nad27sp2701> : nevada east> : nad27sp
 <nad27sp2702> : nevada central> : nad27sp
 <nad27sp2703> : nevada west> : nad27sp
 <nad27sp2800> : new hampshire ---> : nad27sp
 <nad27sp2900> : new jersey ---> : nad27sp
 <nad27sp3001> : new mexico east> : nad27sp
 <nad27sp3002> : new mexico central> : nad27sp
 <nad27sp3003> : new mexico west> : nad27sp
 <nad27sp3101> : new york east> : nad27sp
 <nad27sp3102> : new york central> : nad27sp
 <nad27sp3103> : new york west> : nad27sp
 <nad27sp3104> : new york long island> : nad27sp
 <nad27sp3200> : north carolina ---> : nad27sp
 <nad27sp3301> : north dakota north> : nad27sp
 <nad27sp3302> : north dakota south> : nad27sp
 <nad27sp3401> : ohio north> : nad27sp
 <nad27sp3402> : ohio south> : nad27sp
 <nad27sp3501> : oklahoma north> : nad27sp
 <nad27sp3502> : oklahoma south> : nad27sp
 <nad27sp3601> : oregon north> : nad27sp
 <nad27sp3602> : oregon south> : nad27sp
 <nad27sp3701> : pennsylvania north> : nad27sp
 <nad27sp3702> : pennsylvania south> : nad27sp
 <nad27sp3800> : rhode island ---> : nad27sp
 <nad27sp3901> : south carolina north> : nad27sp
 <nad27sp3902> : south carolina south> : nad27sp
 <nad27sp4001> : south dakota north> : nad27sp
 <nad27sp4002> : south dakota south> : nad27sp
 <nad27sp4100> : tennessee ---> : nad27sp
 <nad27sp4201> : texas north> : nad27sp
 <nad27sp4202> : texas north central> : nad27sp
 <nad27sp4203> : texas central> : nad27sp
 <nad27sp4204> : texas south central> : nad27sp
 <nad27sp4205> : texas south> : nad27sp
 <nad27sp4301> : utah north> : nad27sp
 <nad27sp4302> : utah central> : nad27sp
 <nad27sp4303> : utah south> : nad27sp
 <nad27sp4400> : vermont ---> : nad27sp
 <nad27sp4501> : virginia north> : nad27sp
 <nad27sp4502> : virginia south> : nad27sp
 <nad27sp4601> : washington north> : nad27sp
 <nad27sp4602> : washington south> : nad27sp
 <nad27sp4701> : west virginia north> : nad27sp
 <nad27sp4702> : west virginia south> : nad27sp
 <nad27sp4801> : wisconsin north> : nad27sp
 <nad27sp4802> : wisconsin central> : nad27sp
 <nad27sp4803> : wisconsin south> : nad27sp
 <nad27sp4901> : wyoming east> : nad27sp
 <nad27sp4902> : wyoming east central> : nad27sp
 <nad27sp4903> : wyoming west central> : nad27sp
 <nad27sp4904> : wyoming west> : nad27sp
 <nad27sp5001> : alaska zone no. 1> : nad27sp
 <nad27sp5002> : alaska zone no. 2> : nad27sp
 <nad27sp5003> : alaska zone no. 3> : nad27sp
 <nad27sp5004> : alaska zone no. 4> : nad27sp
 <nad27sp5005> : alaska zone no. 5> : nad27sp
 <nad27sp5006> : alaska zone no. 6> : nad27sp
 <nad27sp5007> : alaska zone no. 7> : nad27sp
 <nad27sp5008> : alaska zone no. 8> : nad27sp
 <nad27sp5009> : alaska zone no. 9> : nad27sp
 <nad27sp5201> : puerto rico and virgin islands> : nad27sp
 <nad27sp5202> : virgin islands st. croix> : nad27sp
 <nad27sp5400> : guam island> : nad27sp

 ---------------------------------------------------------
 State Plane Coordinate Systems
 North American Datum 1983
 ---------------------------------------------------------
 <nad83sp101> : alabama east> : nad83
 <nad83sp102> : alabama west> : nad83
 <nad83sp5010> : alaska zone no. 10> : nad83
 <nad83sp201> : arizona east> : nad83
 <nad83sp202> : arizona central> : nad83
 <nad83sp203> : arizona west> : nad83
 <nad83sp301> : arkansas north> : nad83
 <nad83sp302> : arkansas south> : nad83
 <nad83sp401> : california i> : nad83
 <nad83sp402> : california ii> : nad83
 <nad83sp403> : california iii> : nad83
 <nad83sp404> : california iv> : nad83
 <nad83sp405> : california v> : nad83
 <nad83sp406> : california vi> : nad83
 <nad83sp501> : colorado north> : nad83
 <nad83sp502> : colorado central> : nad83
 <nad83sp503> : colorado south> : nad83
 <nad83sp600> : connecticut ---> : nad83
 <nad83sp700> : delaware ---> : nad83
 <nad83sp901> : florida east> : nad83
 <nad83sp902> : florida west> : nad83
 <nad83sp903> : florida north> : nad83
 <nad83sp1001> : georgia east> : nad83
 <nad83sp1002> : georgia west> : nad83
 <nad83sp5101> : hawaii 1> : nad83
 <nad83sp5102> : hawaii 2> : nad83
 <nad83sp5103> : hawaii 3> : nad83
 <nad83sp5104> : hawaii 4> : nad83
 <nad83sp5105> : hawaii 5> : nad83
 <nad83sp1101> : idaho east> : nad83
 <nad83sp1102> : idaho central> : nad83
 <nad83sp1103> : idaho west> : nad83
 <nad83sp1201> : illinois east> : nad83
 <nad83sp1202> : illinois west> : nad83
 <nad83sp1301> : indiana east> : nad83
 <nad83sp1302> : indiana west> : nad83
 <nad83sp1401> : iowa north> : nad83
 <nad83sp1402> : iowa south> : nad83
 <nad83sp1501> : kansas north> : nad83
 <nad83sp1502> : kansas south> : nad83
 <nad83sp1601> : kentucky north> : nad83
 <nad83sp1602> : kentucky south> : nad83
 <nad83sp1701> : louisiana north> : nad83
 <nad83sp1702> : louisiana south> : nad83
 <nad83sp1703> : louisiana offshore> : nad83
 <nad83sp1801> : maine east> : nad83
 <nad83sp1802> : maine west> : nad83
 <nad83sp1900> : maryland ---> : nad83
 <nad83sp2001> : massachusetts mainland> : nad83
 <nad83sp2002> : massachusetts island> : nad83
 <nad83sp2111> : michigan north> : nad83
 <nad83sp2112> : michigan central/l> : nad83
 <nad83sp2113> : michigan south> : nad83
 <nad83sp2201> : minnesota north> : nad83
 <nad83sp2202> : minnesota central> : nad83
 <nad83sp2203> : minnesota south> : nad83
 <nad83sp2301> : mississippi east> : nad83
 <nad83sp2302> : mississippi west> : nad83
 <nad83sp2401> : missouri east> : nad83
 <nad83sp2402> : missouri central> : nad83
 <nad83sp2403> : missouri west> : nad83
 <nad83sp2500> : montana> : nad83
 <nad83sp2600> : nebraska> : nad83
 <nad83sp2701> : nevada east> : nad83
 <nad83sp2702> : nevada central> : nad83
 <nad83sp2703> : nevada west> : nad83
 <nad83sp2800> : new hampshire ---> : nad83
 <nad83sp2900> : new jersey ---> : nad83
 <nad83sp3001> : new mexico east> : nad83
 <nad83sp3002> : new mexico central> : nad83
 <nad83sp3003> : new mexico west> : nad83
 <nad83sp3101> : new york east> : nad83
 <nad83sp3102> : new york central> : nad83
 <nad83sp3103> : new york west> : nad83
 <nad83sp3104> : new york long island> : nad83
 <nad83sp3200> : north carolina ---> : nad83
 <nad83sp3301> : north dakota north> : nad83
 <nad83sp3302> : north dakota south> : nad83
 <nad83sp3401> : ohio north> : nad83
 <nad83sp3402> : ohio south> : nad83
 <nad83sp3501> : oklahoma north> : nad83
 <nad83sp3502> : oklahoma south> : nad83
 <nad83sp3601> : oregon north> : nad83
 <nad83sp3602> : oregon south> : nad83
 <nad83sp3701> : pennsylvania north> : nad83
 <nad83sp3702> : pennsylvania south> : nad83
 <nad83sp3800> : rhode island ---> : nad83
 <nad83sp3900> : south carolina> : nad83
 <nad83sp4001> : south dakota north> : nad83
 <nad83sp4002> : south dakota south> : nad83
 <nad83sp4100> : tennessee ---> : nad83
 <nad83sp4201> : texas north> : nad83
 <nad83sp4202> : texas north central> : nad83
 <nad83sp4203> : texas central> : nad83
 <nad83sp4204> : texas south central> : nad83
 <nad83sp4205> : texas south> : nad83
 <nad83sp4301> : utah north> : nad83
 <nad83sp4302> : utah central> : nad83
 <nad83sp4303> : utah south> : nad83
 <nad83sp4400> : vermont ---> : nad83
 <nad83sp4501> : virginia north> : nad83
 <nad83sp4502> : virginia south> : nad83
 <nad83sp4601> : washington north> : nad83
 <nad83sp4602> : washington south> : nad83
 <nad83sp4701> : west virginia north> : nad83
 <nad83sp4702> : west virginia south> : nad83
 <nad83sp4801> : wisconsin north> : nad83
 <nad83sp4802> : wisconsin central> : nad83
 <nad83sp4803> : wisconsin south> : nad83
 <nad83sp4901> : wyoming east> : nad83
 <nad83sp4902> : wyoming east central> : nad83
 <nad83sp4903> : wyoming west central> : nad83
 <nad83sp4904> : wyoming west> : nad83
 <nad83sp5001> : alaska zone no. 1> : nad83
 <nad83sp5002> : alaska zone no. 2> : nad83
 <nad83sp5003> : alaska zone no. 3> : nad83
 <nad83sp5004> : alaska zone no. 4> : nad83
 <nad83sp5005> : alaska zone no. 5> : nad83
 <nad83sp5006> : alaska zone no. 6> : nad83
 <nad83sp5007> : alaska zone no. 7> : nad83
 <nad83sp5008> : alaska zone no. 8> : nad83
 <nad83sp5009> : alaska zone no. 9> : nad83
 <nad83sp5200> : puerto rico and virgin islands> : nad83

 ---------------------------------------------------------
 Great Lakes Grids using Clarke 1866 ellipsoid
 ---------------------------------------------------------
 <erie-etal> : Lake Erie, Ontario and St. Lawrence River.
 <huron> : Lake Huron
 <michigan> : Lake Michigan
 <superior> : Lake Superior, Lake of the Woods

 ---------------------------------------------------------
 EPSG projection definitions
 ---------------------------------------------------------
 <epsg4001> : Unknown datum based upon the Airy 1830 ellipsoid
 <epsg4002> : Unknown datum based upon the Airy Modified 1849 ellipsoid
 <epsg4003> : Unknown datum based upon the Australian National Spheroid
 <epsg4004> : Unknown datum based upon the Bessel 1841 ellipsoid
 <epsg4005> : Unknown datum based upon the Bessel Modified ellipsoid
 <epsg4006> : Unknown datum based upon the Bessel Namibia ellipsoid
 <epsg4007> : Unknown datum based upon the Clarke 1858 ellipsoid
 <epsg4008> : Unknown datum based upon the Clarke 1866 ellipsoid
 <epsg4009> : Unknown datum based upon the Clarke 1866 Michigan ellipsoid
 <epsg4010> : Unknown datum based upon the Clarke 1880 (Benoit) ellipsoid
 <epsg4011> : Unknown datum based upon the Clarke 1880 (IGN) ellipsoid
 <epsg4012> : Unknown datum based upon the Clarke 1880 (RGS) ellipsoid
 <epsg4013> : Unknown datum based upon the Clarke 1880 (Arc) ellipsoid
 <epsg4014> : Unknown datum based upon the Clarke 1880 (SGA 1922) ellipsoid
 <epsg4015> : Unknown datum based upon the Everest 1830 (1937 Adjustment) ellipsoid
 <epsg4016> : Unknown datum based upon the Everest 1830 (1967 Definition) ellipsoid
 <epsg4018> : Unknown datum based upon the Everest 1830 Modified ellipsoid
 <epsg4019> : Unknown datum based upon the GRS 1980 ellipsoid
 <epsg4020> : Unknown datum based upon the Helmert 1906 ellipsoid
 <epsg4021> : Unknown datum based upon the Indonesian National Spheroid
 <epsg4022> : Unknown datum based upon the International 1924 ellipsoid
 <epsg4024> : Unknown datum based upon the Krassowsky 1940 ellipsoid
 <epsg4025> : Unknown datum based upon the NWL 9D ellipsoid
 <epsg4027> : Unknown datum based upon the Plessis 1817 ellipsoid
 <epsg4028> : Unknown datum based upon the Struve 1860 ellipsoid
 <epsg4029> : Unknown datum based upon the War Office ellipsoid
 <epsg4030> : Unknown datum based upon the WGS 84 ellipsoid
 <epsg4031> : Unknown datum based upon the GEM 10C ellipsoid
 <epsg4032> : Unknown datum based upon the OSU86F ellipsoid
 <epsg4033> : Unknown datum based upon the OSU91A ellipsoid
 <epsg4034> : Unknown datum based upon the Clarke 1880 ellipsoid
 <epsg4035> : Unknown datum based upon the Authalic Sphere
 <epsg4036> : Unknown datum based upon the GRS 1967 ellipsoid
 <epsg4041> : Unknown datum based upon the Average Terrestrial System 1977 ellipsoid
 <epsg4042> : Unknown datum based upon the Everest (1830 Definition) ellipsoid
 <epsg4043> : Unknown datum based upon the WGS 72 ellipsoid
 <epsg4044> : Unknown datum based upon the Everest 1830 (1962 Definition) ellipsoid
 <epsg4045> : Unknown datum based upon the Everest 1830 (1975 Definition) ellipsoid
 <epsg4047> : Unspecified datum based upon the GRS 1980 Authalic Sphere
 <epsg4052> : Unspecified datum based upon the Clarke 1866 Authalic Sphere
 <epsg4053> : Unspecified datum based upon the International 1924 Authalic Sphere
 <epsg4054> : Unspecified datum based upon the Hughes 1980 ellipsoid
 <epsg4120> : Greek
 <epsg4121> : GGRS87
 <epsg4122> : ATS77
 <epsg4123> : KKJ
 <epsg4124> : RT90
 <epsg4125> : Samboja
 <epsg4126> : LKS94 (ETRS89)
 <epsg4127> : Tete
 <epsg4128> : Madzansua
 <epsg4129> : Observatario
 <epsg4130> : Moznet
 <epsg4131> : Indian 1960
 <epsg4132> : FD58
 <epsg4133> : EST92
 <epsg4134> : PDO Survey Datum 1993
 <epsg4135> : Old Hawaiian
 <epsg4136> : St. Lawrence Island
 <epsg4137> : St. Paul Island
 <epsg4138> : St. George Island
 <epsg4139> : Puerto Rico
 <epsg4140> : NAD83(CSRS98)
 <epsg4141> : Israel
 <epsg4142> : Locodjo 1965
 <epsg4143> : Abidjan 1987
 <epsg4144> : Kalianpur 1937
 <epsg4145> : Kalianpur 1962
 <epsg4146> : Kalianpur 1975
 <epsg4147> : Hanoi 1972
 <epsg4148> : Hartebeesthoek94
 <epsg4149> : CH1903
 <epsg4150> : CH1903+
 <epsg4151> : CHTRF95
 <epsg4152> : NAD83(HARN)
 <epsg4153> : Rassadiran
 <epsg4154> : ED50(ED77)
 <epsg4155> : Dabola 1981
 <epsg4156> : S-JTSK
 <epsg4157> : Mount Dillon
 <epsg4158> : Naparima 1955
 <epsg4159> : ELD79
 <epsg4160> : Chos Malal 1914
 <epsg4161> : Pampa del Castillo
 <epsg4162> : Korean 1985
 <epsg4163> : Yemen NGN96
 <epsg4164> : South Yemen
 <epsg4165> : Bissau
 <epsg4166> : Korean 1995
 <epsg4167> : NZGD2000
 <epsg4168> : Accra
 <epsg4169> : American Samoa 1962
 <epsg4170> : SIRGAS
 <epsg4171> : RGF93
 <epsg4172> : POSGAR
 <epsg4173> : IRENET95
 <epsg4174> : Sierra Leone 1924
 <epsg4175> : Sierra Leone 1968
 <epsg4176> : Australian Antarctic
 <epsg4178> : Pulkovo 1942(83)
 <epsg4179> : Pulkovo 1942(58)
 <epsg4180> : EST97
 <epsg4181> : Luxembourg 1930
 <epsg4182> : Azores Occidental 1939
 <epsg4183> : Azores Central 1948
 <epsg4184> : Azores Oriental 1940
 <epsg4185> : Madeira 1936
 <epsg4188> : OSNI 1952
 <epsg4189> : REGVEN
 <epsg4190> : POSGAR 98
 <epsg4191> : Albanian 1987
 <epsg4192> : Douala 1948
 <epsg4193> : Manoca 1962
 <epsg4194> : Qornoq 1927
 <epsg4195> : Scoresbysund 1952
 <epsg4196> : Ammassalik 1958
 <epsg4197> : Garoua
 <epsg4198> : Kousseri
 <epsg4199> : Egypt 1930
 <epsg4200> : Pulkovo 1995
 <epsg4201> : Adindan
 <epsg4202> : AGD66
 <epsg4203> : AGD84
 <epsg4204> : Ain el Abd
 <epsg4205> : Afgooye
 <epsg4206> : Agadez
 <epsg4207> : Lisbon
 <epsg4208> : Aratu
 <epsg4209> : Arc 1950
 <epsg4210> : Arc 1960
 <epsg4211> : Batavia
 <epsg4212> : Barbados 1938
 <epsg4213> : Beduaram
 <epsg4214> : Beijing 1954
 <epsg4215> : Belge 1950
 <epsg4216> : Bermuda 1957
 <epsg4218> : Bogota 1975
 <epsg4219> : Bukit Rimpah
 <epsg4220> : Camacupa
 <epsg4221> : Campo Inchauspe
 <epsg4222> : Cape
 <epsg4223> : Carthage
 <epsg4224> : Chua
 <epsg4225> : Corrego Alegre
 <epsg4226> : Cote d'Ivoire
 <epsg4227> : Deir ez Zor
 <epsg4228> : Douala
 <epsg4229> : Egypt 1907
 <epsg4230> : ED50
 <epsg4231> : ED87
 <epsg4232> : Fahud
 <epsg4233> : Gandajika 1970
 <epsg4234> : Garoua
 <epsg4235> : Guyane Francaise
 <epsg4236> : Hu Tzu Shan
 <epsg4237> : HD72
 <epsg4238> : ID74
 <epsg4239> : Indian 1954
 <epsg4240> : Indian 1975
 <epsg4241> : Jamaica 1875
 <epsg4242> : JAD69
 <epsg4243> : Kalianpur 1880
 <epsg4244> : Kandawala
 <epsg4245> : Kertau 1968
 <epsg4246> : KOC
 <epsg4247> : La Canoa
 <epsg4248> : PSAD56
 <epsg4249> : Lake
 <epsg4250> : Leigon
 <epsg4251> : Liberia 1964
 <epsg4252> : Lome
 <epsg4253> : Luzon 1911
 <epsg4254> : Hito XVIII 1963
 <epsg4255> : Herat North
 <epsg4256> : Mahe 1971
 <epsg4257> : Makassar
 <epsg4258> : ETRS89
 <epsg4259> : Malongo 1987
 <epsg4260> : Manoca
 <epsg4261> : Merchich
 <epsg4262> : Massawa
 <epsg4263> : Minna
 <epsg4264> : Mhast
 <epsg4265> : Monte Mario
 <epsg4266> : M'poraloko
 <epsg4267> : NAD27
 <epsg4268> : NAD27 Michigan
 <epsg4269> : NAD83
 <epsg4270> : Nahrwan 1967
 <epsg4271> : Naparima 1972
 <epsg4272> : NZGD49
 <epsg4273> : NGO 1948
 <epsg4274> : Datum 73
 <epsg4275> : NTF
 <epsg4276> : NSWC 9Z-2
 <epsg4277> : OSGB 1936
 <epsg4278> : OSGB70
 <epsg4279> : OS(SN)80
 <epsg4280> : Padang
 <epsg4281> : Palestine 1923
 <epsg4282> : Pointe Noire
 <epsg4283> : GDA94
 <epsg4284> : Pulkovo 1942
 <epsg4285> : Qatar 1974
 <epsg4286> : Qatar 1948
 <epsg4287> : Qornoq
 <epsg4288> : Loma Quintana
 <epsg4289> : Amersfoort
 <epsg4291> : SAD69
 <epsg4292> : Sapper Hill 1943
 <epsg4293> : Schwarzeck
 <epsg4294> : Segora
 <epsg4295> : Serindung
 <epsg4296> : Sudan
 <epsg4297> : Tananarive
 <epsg4298> : Timbalai 1948
 <epsg4299> : TM65
 <epsg4300> : TM75
 <epsg4301> : Tokyo
 <epsg4302> : Trinidad 1903
 <epsg4303> : TC(1948)
 <epsg4304> : Voirol 1875
 <epsg4306> : Bern 1938
 <epsg4307> : Nord Sahara 1959
 <epsg4308> : RT38
 <epsg4309> : Yacare
 <epsg4310> : Yoff
 <epsg4311> : Zanderij
 <epsg4312> : MGI
 <epsg4313> : Belge 1972
 <epsg4314> : DHDN
 <epsg4315> : Conakry 1905
 <epsg4316> : Dealul Piscului 1933
 <epsg4317> : Dealul Piscului 1970
 <epsg4318> : NGN
 <epsg4319> : KUDAMS
 <epsg4322> : WGS 72
 <epsg4324> : WGS 72BE
 <epsg4326> : WGS 84
 <epsg4600> : Anguilla 1957
 <epsg4601> : Antigua 1943
 <epsg4602> : Dominica 1945
 <epsg4603> : Grenada 1953
 <epsg4604> : Montserrat 1958
 <epsg4605> : St. Kitts 1955
 <epsg4606> : St. Lucia 1955
 <epsg4607> : St. Vincent 1945
 <epsg4608> : NAD27(76)
 <epsg4609> : NAD27(CGQ77)
 <epsg4610> : Xian 1980
 <epsg4611> : Hong Kong 1980
 <epsg4612> : JGD2000
 <epsg4613> : Segara
 <epsg4614> : QND95
 <epsg4615> : Porto Santo
 <epsg4616> : Selvagem Grande
 <epsg4617> : NAD83(CSRS)
 <epsg4618> : SAD69
 <epsg4619> : SWEREF99
 <epsg4620> : Point 58
 <epsg4621> : Fort Marigot
 <epsg4622> : Guadeloupe 1948
 <epsg4623> : CSG67
 <epsg4624> : RGFG95
 <epsg4625> : Martinique 1938
 <epsg4626> : Reunion 1947
 <epsg4627> : RGR92
 <epsg4628> : Tahiti 52
 <epsg4629> : Tahaa 54
 <epsg4630> : IGN72 Nuku Hiva
 <epsg4631> : K0 1949
 <epsg4632> : Combani 1950
 <epsg4633> : IGN56 Lifou
 <epsg4634> : IGN72 Grand Terre
 <epsg4635> : ST87 Ouvea
 <epsg4636> : Petrels 1972
 <epsg4637> : Perroud 1950
 <epsg4638> : Saint Pierre et Miquelon 1950
 <epsg4639> : MOP78
 <epsg4640> : RRAF 1991
 <epsg4641> : IGN53 Mare
 <epsg4642> : ST84 Ile des Pins
 <epsg4643> : ST71 Belep
 <epsg4644> : NEA74 Noumea
 <epsg4645> : RGNC 1991
 <epsg4646> : Grand Comoros
 <epsg4657> : Reykjavik 1900
 <epsg4658> : Hjorsey 1955
 <epsg4659> : ISN93
 <epsg4660> : Helle 1954
 <epsg4661> : LKS92
 <epsg4662> : IGN72 Grande Terre
 <epsg4663> : Porto Santo 1995
 <epsg4664> : Azores Oriental 1995
 <epsg4665> : Azores Central 1995
 <epsg4666> : Lisbon 1890
 <epsg4667> : IKBD-92
 <epsg4668> : ED79
 <epsg4669> : LKS94
 <epsg4670> : IGM95
 <epsg4671> : Voirol 1879
 <epsg4672> : CI1971
 <epsg4673> : CI1979
 <epsg4674> : SIRGAS 2000
 <epsg4675> : Guam 1963
 <epsg4676> : Vientiane 1982
 <epsg4677> : Lao 1993
 <epsg4678> : Lao 1997
 <epsg4679> : Jouik 1961
 <epsg4680> : Nouakchott 1965
 <epsg4681> : Mauritania 1999
 <epsg4682> : Gulshan 303
 <epsg4683> : PRS92
 <epsg4684> : Gan 1970
 <epsg4685> : Gandajika
 <epsg4686> : MAGNA-SIRGAS
 <epsg4687> : RGPF
 <epsg4688> : Fatu Iva 72
 <epsg4689> : IGN63 Hiva Oa
 <epsg4690> : Tahiti 79
 <epsg4691> : Moorea 87
 <epsg4692> : Maupiti 83
 <epsg4693> : Nakhl-e Ghanem
 <epsg4694> : POSGAR 94
 <epsg4695> : Katanga 1955
 <epsg4696> : Kasai 1953
 <epsg4697> : IGC 1962 6th Parallel South
 <epsg4698> : IGN 1962 Kerguelen
 <epsg4699> : Le Pouce 1934
 <epsg4700> : IGN Astro 1960
 <epsg4701> : IGCB 1955
 <epsg4702> : Mauritania 1999
 <epsg4703> : Mhast 1951
 <epsg4704> : Mhast (onshore)
 <epsg4705> : Mhast (offshore)
 <epsg4706> : Egypt Gulf of Suez S-650 TL
 <epsg4707> : Tern Island 1961
 <epsg4708> : Cocos Islands 1965
 <epsg4709> : Iwo Jima 1945
 <epsg4710> : St. Helena 1971
 <epsg4711> : Marcus Island 1952
 <epsg4712> : Ascension Island 1958
 <epsg4713> : Ayabelle Lighthouse
 <epsg4714> : Bellevue
 <epsg4715> : Camp Area Astro
 <epsg4716> : Phoenix Islands 1966
 <epsg4717> : Cape Canaveral
 <epsg4718> : Solomon 1968
 <epsg4719> : Easter Island 1967
 <epsg4720> : Fiji 1986
 <epsg4721> : Fiji 1956
 <epsg4722> : South Georgia 1968
 <epsg4723> : Grand Cayman 1959
 <epsg4724> : Diego Garcia 1969
 <epsg4725> : Johnston Island 1961
 <epsg4726> : Little Cayman 1961
 <epsg4727> : Midway 1961
 <epsg4728> : Pico de la Nieves
 <epsg4729> : Pitcairn 1967
 <epsg4730> : Santo 1965
 <epsg4731> : Viti Levu 1916
 <epsg4732> : Marshall Islands 1960
 <epsg4733> : Wake Island 1952
 <epsg4734> : Tristan 1968
 <epsg4735> : Kusaie 1951
 <epsg4736> : Deception Island
 <epsg4737> : Korea 2000
 <epsg4738> : Hong Kong 1963
 <epsg4739> : Hong Kong 1963(67)
 <epsg4740> : PZ-90
 <epsg4741> : FD54
 <epsg4742> : GDM2000
 <epsg4743> : Karbala 1979 (Polservice)
 <epsg4744> : Nahrwan 1934
 <epsg4745> : RD/83
 <epsg4746> : PD/83
 <epsg4747> : GR96
 <epsg4748> : Vanua Levu 1915
 <epsg4749> : RGNC91-93
 <epsg4750> : ST87 Ouvea
 <epsg4751> : Kertau (RSO)
 <epsg4752> : Viti Levu 1912
 <epsg4753> : fk89
 <epsg4754> : LGD2006
 <epsg4755> : DGN95
 <epsg4756> : VN-2000
 <epsg4757> : SVY21
 <epsg4758> : JAD2001
 <epsg4759> : NAD83(NSRS2007)
 <epsg4760> : WGS 66
 <epsg4801> : Bern 1898 (Bern)
 <epsg4802> : Bogota 1975 (Bogota)
 <epsg4803> : Lisbon (Lisbon)
 <epsg4804> : Makassar (Jakarta)
 <epsg4805> : MGI (Ferro)
 <epsg4806> : Monte Mario (Rome)
 <epsg4807> : NTF (Paris)
 <epsg4808> : Padang (Jakarta)
 <epsg4809> : Belge 1950 (Brussels)
 <epsg4810> : Tananarive (Paris)
 <epsg4811> : Voirol 1875 (Paris)
 <epsg4813> : Batavia (Jakarta)
 <epsg4814> : RT38 (Stockholm)
 <epsg4815> : Greek (Athens)
 <epsg4816> : Carthage (Paris)
 <epsg4817> : NGO 1948 (Oslo)
 <epsg4818> : S-JTSK (Ferro)
 <epsg4819> : Nord Sahara 1959 (Paris)
 <epsg4820> : Segara (Jakarta)
 <epsg4821> : Voirol 1879 (Paris)
 <epsg4901> : ATF (Paris)
 <epsg4902> : NDG (Paris)
 <epsg4903> : Madrid 1870 (Madrid)
 <epsg4904> : Lisbon 1890 (Lisbon)
 <epsg2000> : Anguilla 1957 / British West Indies Grid
 <epsg2001> : Antigua 1943 / British West Indies Grid
 <epsg2002> : Dominica 1945 / British West Indies Grid
 <epsg2003> : Grenada 1953 / British West Indies Grid
 <epsg2004> : Montserrat 1958 / British West Indies Grid
 <epsg2005> : St. Kitts 1955 / British West Indies Grid
 <epsg2006> : St. Lucia 1955 / British West Indies Grid
 <epsg2007> : St. Vincent 45 / British West Indies Grid
 <epsg2008> : NAD27(CGQ77) / SCoPQ zone 2
 <epsg2009> : NAD27(CGQ77) / SCoPQ zone 3
 <epsg2010> : NAD27(CGQ77) / SCoPQ zone 4
 <epsg2011> : NAD27(CGQ77) / SCoPQ zone 5
 <epsg2012> : NAD27(CGQ77) / SCoPQ zone 6
 <epsg2013> : NAD27(CGQ77) / SCoPQ zone 7
 <epsg2014> : NAD27(CGQ77) / SCoPQ zone 8
 <epsg2015> : NAD27(CGQ77) / SCoPQ zone 9
 <epsg2016> : NAD27(CGQ77) / SCoPQ zone 10
 <epsg2017> : NAD27(76) / MTM zone 8
 <epsg2018> : NAD27(76) / MTM zone 9
 <epsg2019> : NAD27(76) / MTM zone 10
 <epsg2020> : NAD27(76) / MTM zone 11
 <epsg2021> : NAD27(76) / MTM zone 12
 <epsg2022> : NAD27(76) / MTM zone 13
 <epsg2023> : NAD27(76) / MTM zone 14
 <epsg2024> : NAD27(76) / MTM zone 15
 <epsg2025> : NAD27(76) / MTM zone 16
 <epsg2026> : NAD27(76) / MTM zone 17
 <epsg2027> : NAD27(76) / UTM zone 15N
 <epsg2028> : NAD27(76) / UTM zone 16N
 <epsg2029> : NAD27(76) / UTM zone 17N
 <epsg2030> : NAD27(76) / UTM zone 18N
 <epsg2031> : NAD27(CGQ77) / UTM zone 17N
 <epsg2032> : NAD27(CGQ77) / UTM zone 18N
 <epsg2033> : NAD27(CGQ77) / UTM zone 19N
 <epsg2034> : NAD27(CGQ77) / UTM zone 20N
 <epsg2035> : NAD27(CGQ77) / UTM zone 21N
 <epsg2036> : NAD83(CSRS98) / New Brunswick Stereo (deprecated)
 <epsg2037> : NAD83(CSRS98) / UTM zone 19N (deprecated)
 <epsg2038> : NAD83(CSRS98) / UTM zone 20N (deprecated)
 <epsg2039> : Israel / Israeli TM Grid
 <epsg2040> : Locodjo 1965 / UTM zone 30N
 <epsg2041> : Abidjan 1987 / UTM zone 30N
 <epsg2042> : Locodjo 1965 / UTM zone 29N
 <epsg2043> : Abidjan 1987 / UTM zone 29N
 <epsg2044> : Hanoi 1972 / Gauss-Kruger zone 18
 <epsg2045> : Hanoi 1972 / Gauss-Kruger zone 19
 <epsg2056> : CH1903+ / LV95
 <epsg2057> : Rassadiran / Nakhl e Taqi
 <epsg2058> : ED50(ED77) / UTM zone 38N
 <epsg2059> : ED50(ED77) / UTM zone 39N
 <epsg2060> : ED50(ED77) / UTM zone 40N
 <epsg2061> : ED50(ED77) / UTM zone 41N
 <epsg2062> : Madrid 1870 (Madrid) / Spain
 <epsg2063> : Dabola 1981 / UTM zone 28N (deprecated)
 <epsg2064> : Dabola 1981 / UTM zone 29N (deprecated)
 <epsg2065> : S-JTSK (Ferro) / Krovak
 <epsg2066> : Mount Dillon / Tobago Grid
 <epsg2067> : Naparima 1955 / UTM zone 20N
 <epsg2068> : ELD79 / Libya zone 5
 <epsg2069> : ELD79 / Libya zone 6
 <epsg2070> : ELD79 / Libya zone 7
 <epsg2071> : ELD79 / Libya zone 8
 <epsg2072> : ELD79 / Libya zone 9
 <epsg2073> : ELD79 / Libya zone 10
 <epsg2074> : ELD79 / Libya zone 11
 <epsg2075> : ELD79 / Libya zone 12
 <epsg2076> : ELD79 / Libya zone 13
 <epsg2077> : ELD79 / UTM zone 32N
 <epsg2078> : ELD79 / UTM zone 33N
 <epsg2079> : ELD79 / UTM zone 34N
 <epsg2080> : ELD79 / UTM zone 35N
 <epsg2081> : Chos Malal 1914 / Argentina zone 2
 <epsg2082> : Pampa del Castillo / Argentina zone 2
 <epsg2083> : Hito XVIII 1963 / Argentina zone 2
 <epsg2084> : Hito XVIII 1963 / UTM zone 19S
 <epsg2085> : NAD27 / Cuba Norte
 <epsg2086> : NAD27 / Cuba Sur
 <epsg2087> : ELD79 / TM 12 NE
 <epsg2088> : Carthage / TM 11 NE
 <epsg2089> : Yemen NGN96 / UTM zone 38N
 <epsg2090> : Yemen NGN96 / UTM zone 39N
 <epsg2091> : South Yemen / Gauss Kruger zone 8 (deprecated)
 <epsg2092> : South Yemen / Gauss Kruger zone 9 (deprecated)
 <epsg2093> : Hanoi 1972 / GK 106 NE
 <epsg2094> : WGS 72BE / TM 106 NE
 <epsg2095> : Bissau / UTM zone 28N
 <epsg2096> : Korean 1985 / Korea East Belt
 <epsg2097> : Korean 1985 / Korea Central Belt
 <epsg2098> : Korean 1985 / Korea West Belt
 <epsg2099> : Qatar 1948 / Qatar Grid
 <epsg2100> : GGRS87 / Greek Grid
 <epsg2101> : Lake / Maracaibo Grid M1
 <epsg2102> : Lake / Maracaibo Grid
 <epsg2103> : Lake / Maracaibo Grid M3
 <epsg2104> : Lake / Maracaibo La Rosa Grid
 <epsg2105> : NZGD2000 / Mount Eden Circuit 2000
 <epsg2106> : NZGD2000 / Bay of Plenty Circuit 2000
 <epsg2107> : NZGD2000 / Poverty Bay Circuit 2000
 <epsg2108> : NZGD2000 / Hawkes Bay Circuit 2000
 <epsg2109> : NZGD2000 / Taranaki Circuit 2000
 <epsg2110> : NZGD2000 / Tuhirangi Circuit 2000
 <epsg2111> : NZGD2000 / Wanganui Circuit 2000
 <epsg2112> : NZGD2000 / Wairarapa Circuit 2000
 <epsg2113> : NZGD2000 / Wellington Circuit 2000
 <epsg2114> : NZGD2000 / Collingwood Circuit 2000
 <epsg2115> : NZGD2000 / Nelson Circuit 2000
 <epsg2116> : NZGD2000 / Karamea Circuit 2000
 <epsg2117> : NZGD2000 / Buller Circuit 2000
 <epsg2118> : NZGD2000 / Grey Circuit 2000
 <epsg2119> : NZGD2000 / Amuri Circuit 2000
 <epsg2120> : NZGD2000 / Marlborough Circuit 2000
 <epsg2121> : NZGD2000 / Hokitika Circuit 2000
 <epsg2122> : NZGD2000 / Okarito Circuit 2000
 <epsg2123> : NZGD2000 / Jacksons Bay Circuit 2000
 <epsg2124> : NZGD2000 / Mount Pleasant Circuit 2000
 <epsg2125> : NZGD2000 / Gawler Circuit 2000
 <epsg2126> : NZGD2000 / Timaru Circuit 2000
 <epsg2127> : NZGD2000 / Lindis Peak Circuit 2000
 <epsg2128> : NZGD2000 / Mount Nicholas Circuit 2000
 <epsg2129> : NZGD2000 / Mount York Circuit 2000
 <epsg2130> : NZGD2000 / Observation Point Circuit 2000
 <epsg2131> : NZGD2000 / North Taieri Circuit 2000
 <epsg2132> : NZGD2000 / Bluff Circuit 2000
 <epsg2133> : NZGD2000 / UTM zone 58S
 <epsg2134> : NZGD2000 / UTM zone 59S
 <epsg2135> : NZGD2000 / UTM zone 60S
 <epsg2136> : Accra / Ghana National Grid
 <epsg2137> : Accra / TM 1 NW
 <epsg2138> : NAD27(CGQ77) / Quebec Lambert
 <epsg2139> : NAD83(CSRS98) / SCoPQ zone 2 (deprecated)
 <epsg2140> : NAD83(CSRS98) / MTM zone 3 (deprecated)
 <epsg2141> : NAD83(CSRS98) / MTM zone 4 (deprecated)
 <epsg2142> : NAD83(CSRS98) / MTM zone 5 (deprecated)
 <epsg2143> : NAD83(CSRS98) / MTM zone 6 (deprecated)
 <epsg2144> : NAD83(CSRS98) / MTM zone 7 (deprecated)
 <epsg2145> : NAD83(CSRS98) / MTM zone 8 (deprecated)
 <epsg2146> : NAD83(CSRS98) / MTM zone 9 (deprecated)
 <epsg2147> : NAD83(CSRS98) / MTM zone 10 (deprecated)
 <epsg2148> : NAD83(CSRS98) / UTM zone 21N (deprecated)
 <epsg2149> : NAD83(CSRS98) / UTM zone 18N (deprecated)
 <epsg2150> : NAD83(CSRS98) / UTM zone 17N (deprecated)
 <epsg2151> : NAD83(CSRS98) / UTM zone 13N (deprecated)
 <epsg2152> : NAD83(CSRS98) / UTM zone 12N (deprecated)
 <epsg2153> : NAD83(CSRS98) / UTM zone 11N (deprecated)
 <epsg2154> : RGF93 / Lambert-93
 <epsg2155> : American Samoa 1962 / American Samoa Lambert (deprecated)
 <epsg2156> : NAD83(HARN) / UTM zone 59S (deprecated)
 <epsg2157> : IRENET95 / Irish Transverse Mercator
 <epsg2158> : IRENET95 / UTM zone 29N
 <epsg2159> : Sierra Leone 1924 / New Colony Grid
 <epsg2160> : Sierra Leone 1924 / New War Office Grid
 <epsg2161> : Sierra Leone 1968 / UTM zone 28N
 <epsg2162> : Sierra Leone 1968 / UTM zone 29N
 <epsg2163> : US National Atlas Equal Area
 <epsg2164> : Locodjo 1965 / TM 5 NW
 <epsg2165> : Abidjan 1987 / TM 5 NW
 <epsg2166> : Pulkovo 1942(83) / Gauss Kruger zone 3 (deprecated)
 <epsg2167> : Pulkovo 1942(83) / Gauss Kruger zone 4 (deprecated)
 <epsg2168> : Pulkovo 1942(83) / Gauss Kruger zone 5 (deprecated)
 <epsg2169> : Luxembourg 1930 / Gauss
 <epsg2170> : MGI / Slovenia Grid
 <epsg2171> : Pulkovo 1942(58) / Poland zone I (deprecated)
 <epsg2172> : Pulkovo 1942(58) / Poland zone II
 <epsg2173> : Pulkovo 1942(58) / Poland zone III
 <epsg2174> : Pulkovo 1942(58) / Poland zone IV
 <epsg2175> : Pulkovo 1942(58) / Poland zone V
 <epsg2176> : ETRS89 / Poland CS2000 zone 5
 <epsg2177> : ETRS89 / Poland CS2000 zone 6
 <epsg2178> : ETRS89 / Poland CS2000 zone 7
 <epsg2179> : ETRS89 / Poland CS2000 zone 8
 <epsg2180> : ETRS89 / Poland CS92
 <epsg2188> : Azores Occidental 1939 / UTM zone 25N
 <epsg2189> : Azores Central 1948 / UTM zone 26N
 <epsg2190> : Azores Oriental 1940 / UTM zone 26N
 <epsg2191> : Madeira 1936 / UTM zone 28N (deprecated)
 <epsg2192> : ED50 / France EuroLambert
 <epsg2193> : NZGD2000 / New Zealand Transverse Mercator
 <epsg2194> : American Samoa 1962 / American Samoa Lambert (deprecated)
 <epsg2195> : NAD83(HARN) / UTM zone 2S
 <epsg2196> : ETRS89 / Kp2000 Jutland
 <epsg2197> : ETRS89 / Kp2000 Zealand
 <epsg2198> : ETRS89 / Kp2000 Bornholm
 <epsg2199> : Albanian 1987 / Gauss Kruger zone 4 (deprecated)
 <epsg2200> : ATS77 / New Brunswick Stereographic (ATS77)
 <epsg2201> : REGVEN / UTM zone 18N
 <epsg2202> : REGVEN / UTM zone 19N
 <epsg2203> : REGVEN / UTM zone 20N
 <epsg2204> : NAD27 / Tennessee
 <epsg2205> : NAD83 / Kentucky North
 <epsg2206> : ED50 / 3-degree Gauss-Kruger zone 9
 <epsg2207> : ED50 / 3-degree Gauss-Kruger zone 10
 <epsg2208> : ED50 / 3-degree Gauss-Kruger zone 11
 <epsg2209> : ED50 / 3-degree Gauss-Kruger zone 12
 <epsg2210> : ED50 / 3-degree Gauss-Kruger zone 13
 <epsg2211> : ED50 / 3-degree Gauss-Kruger zone 14
 <epsg2212> : ED50 / 3-degree Gauss-Kruger zone 15
 <epsg2213> : ETRS89 / TM 30 NE
 <epsg2214> : Douala 1948 / AOF west (deprecated)
 <epsg2215> : Manoca 1962 / UTM zone 32N
 <epsg2216> : Qornoq 1927 / UTM zone 22N
 <epsg2217> : Qornoq 1927 / UTM zone 23N
 <epsg2219> : ATS77 / UTM zone 19N
 <epsg2220> : ATS77 / UTM zone 20N
 <epsg2222> : NAD83 / Arizona East (ft)
 <epsg2223> : NAD83 / Arizona Central (ft)
 <epsg2224> : NAD83 / Arizona West (ft)
 <epsg2225> : NAD83 / California zone 1 (ftUS)
 <epsg2226> : NAD83 / California zone 2 (ftUS)
 <epsg2227> : NAD83 / California zone 3 (ftUS)
 <epsg2228> : NAD83 / California zone 4 (ftUS)
 <epsg2229> : NAD83 / California zone 5 (ftUS)
 <epsg2230> : NAD83 / California zone 6 (ftUS)
 <epsg2231> : NAD83 / Colorado North (ftUS)
 <epsg2232> : NAD83 / Colorado Central (ftUS)
 <epsg2233> : NAD83 / Colorado South (ftUS)
 <epsg2234> : NAD83 / Connecticut (ftUS)
 <epsg2235> : NAD83 / Delaware (ftUS)
 <epsg2236> : NAD83 / Florida East (ftUS)
 <epsg2237> : NAD83 / Florida West (ftUS)
 <epsg2238> : NAD83 / Florida North (ftUS)
 <epsg2239> : NAD83 / Georgia East (ftUS)
 <epsg2240> : NAD83 / Georgia West (ftUS)
 <epsg2241> : NAD83 / Idaho East (ftUS)
 <epsg2242> : NAD83 / Idaho Central (ftUS)
 <epsg2243> : NAD83 / Idaho West (ftUS)
 <epsg2244> : NAD83 / Indiana East (ftUS) (deprecated)
 <epsg2245> : NAD83 / Indiana West (ftUS) (deprecated)
 <epsg2246> : NAD83 / Kentucky North (ftUS)
 <epsg2247> : NAD83 / Kentucky South (ftUS)
 <epsg2248> : NAD83 / Maryland (ftUS)
 <epsg2249> : NAD83 / Massachusetts Mainland (ftUS)
 <epsg2250> : NAD83 / Massachusetts Island (ftUS)
 <epsg2251> : NAD83 / Michigan North (ft)
 <epsg2252> : NAD83 / Michigan Central (ft)
 <epsg2253> : NAD83 / Michigan South (ft)
 <epsg2254> : NAD83 / Mississippi East (ftUS)
 <epsg2255> : NAD83 / Mississippi West (ftUS)
 <epsg2256> : NAD83 / Montana (ft)
 <epsg2257> : NAD83 / New Mexico East (ftUS)
 <epsg2258> : NAD83 / New Mexico Central (ftUS)
 <epsg2259> : NAD83 / New Mexico West (ftUS)
 <epsg2260> : NAD83 / New York East (ftUS)
 <epsg2261> : NAD83 / New York Central (ftUS)
 <epsg2262> : NAD83 / New York West (ftUS)
 <epsg2263> : NAD83 / New York Long Island (ftUS)
 <epsg2264> : NAD83 / North Carolina (ftUS)
 <epsg2265> : NAD83 / North Dakota North (ft)
 <epsg2266> : NAD83 / North Dakota South (ft)
 <epsg2267> : NAD83 / Oklahoma North (ftUS)
 <epsg2268> : NAD83 / Oklahoma South (ftUS)
 <epsg2269> : NAD83 / Oregon North (ft)
 <epsg2270> : NAD83 / Oregon South (ft)
 <epsg2271> : NAD83 / Pennsylvania North (ftUS)
 <epsg2272> : NAD83 / Pennsylvania South (ftUS)
 <epsg2273> : NAD83 / South Carolina (ft)
 <epsg2274> : NAD83 / Tennessee (ftUS)
 <epsg2275> : NAD83 / Texas North (ftUS)
 <epsg2276> : NAD83 / Texas North Central (ftUS)
 <epsg2277> : NAD83 / Texas Central (ftUS)
 <epsg2278> : NAD83 / Texas South Central (ftUS)
 <epsg2279> : NAD83 / Texas South (ftUS)
 <epsg2280> : NAD83 / Utah North (ft)
 <epsg2281> : NAD83 / Utah Central (ft)
 <epsg2282> : NAD83 / Utah South (ft)
 <epsg2283> : NAD83 / Virginia North (ftUS)
 <epsg2284> : NAD83 / Virginia South (ftUS)
 <epsg2285> : NAD83 / Washington North (ftUS)
 <epsg2286> : NAD83 / Washington South (ftUS)
 <epsg2287> : NAD83 / Wisconsin North (ftUS)
 <epsg2288> : NAD83 / Wisconsin Central (ftUS)
 <epsg2289> : NAD83 / Wisconsin South (ftUS)
 <epsg2290> : ATS77 / Prince Edward Isl. Stereographic (ATS77)
 <epsg2291> : NAD83(CSRS98) / Prince Edward Isl. Stereographic (NAD83) (deprecated)
 <epsg2292> : NAD83(CSRS98) / Prince Edward Isl. Stereographic (NAD83) (deprecated)
 <epsg2294> : ATS77 / MTM Nova Scotia zone 4
 <epsg2295> : ATS77 / MTM Nova Scotia zone 5
 <epsg2308> : Batavia / TM 109 SE
 <epsg2309> : WGS 84 / TM 116 SE
 <epsg2310> : WGS 84 / TM 132 SE
 <epsg2311> : WGS 84 / TM 6 NE
 <epsg2312> : Garoua / UTM zone 33N
 <epsg2313> : Kousseri / UTM zone 33N
 <epsg2314> : Trinidad 1903 / Trinidad Grid (ftCla)
 <epsg2315> : Campo Inchauspe / UTM zone 19S
 <epsg2316> : Campo Inchauspe / UTM zone 20S
 <epsg2317> : PSAD56 / ICN Regional
 <epsg2318> : Ain el Abd / Aramco Lambert
 <epsg2319> : ED50 / TM27
 <epsg2320> : ED50 / TM30
 <epsg2321> : ED50 / TM33
 <epsg2322> : ED50 / TM36
 <epsg2323> : ED50 / TM39
 <epsg2324> : ED50 / TM42
 <epsg2325> : ED50 / TM45
 <epsg2326> : Hong Kong 1980 Grid System
 <epsg2327> : Xian 1980 / Gauss-Kruger zone 13
 <epsg2328> : Xian 1980 / Gauss-Kruger zone 14
 <epsg2329> : Xian 1980 / Gauss-Kruger zone 15
 <epsg2330> : Xian 1980 / Gauss-Kruger zone 16
 <epsg2331> : Xian 1980 / Gauss-Kruger zone 17
 <epsg2332> : Xian 1980 / Gauss-Kruger zone 18
 <epsg2333> : Xian 1980 / Gauss-Kruger zone 19
 <epsg2334> : Xian 1980 / Gauss-Kruger zone 20
 <epsg2335> : Xian 1980 / Gauss-Kruger zone 21
 <epsg2336> : Xian 1980 / Gauss-Kruger zone 22
 <epsg2337> : Xian 1980 / Gauss-Kruger zone 23
 <epsg2338> : Xian 1980 / Gauss-Kruger CM 75E
 <epsg2339> : Xian 1980 / Gauss-Kruger CM 81E
 <epsg2340> : Xian 1980 / Gauss-Kruger CM 87E
 <epsg2341> : Xian 1980 / Gauss-Kruger CM 93E
 <epsg2342> : Xian 1980 / Gauss-Kruger CM 99E
 <epsg2343> : Xian 1980 / Gauss-Kruger CM 105E
 <epsg2344> : Xian 1980 / Gauss-Kruger CM 111E
 <epsg2345> : Xian 1980 / Gauss-Kruger CM 117E
 <epsg2346> : Xian 1980 / Gauss-Kruger CM 123E
 <epsg2347> : Xian 1980 / Gauss-Kruger CM 129E
 <epsg2348> : Xian 1980 / Gauss-Kruger CM 135E
 <epsg2349> : Xian 1980 / 3-degree Gauss-Kruger zone 25
 <epsg2350> : Xian 1980 / 3-degree Gauss-Kruger zone 26
 <epsg2351> : Xian 1980 / 3-degree Gauss-Kruger zone 27
 <epsg2352> : Xian 1980 / 3-degree Gauss-Kruger zone 28
 <epsg2353> : Xian 1980 / 3-degree Gauss-Kruger zone 29
 <epsg2354> : Xian 1980 / 3-degree Gauss-Kruger zone 30
 <epsg2355> : Xian 1980 / 3-degree Gauss-Kruger zone 31
 <epsg2356> : Xian 1980 / 3-degree Gauss-Kruger zone 32
 <epsg2357> : Xian 1980 / 3-degree Gauss-Kruger zone 33
 <epsg2358> : Xian 1980 / 3-degree Gauss-Kruger zone 34
 <epsg2359> : Xian 1980 / 3-degree Gauss-Kruger zone 35
 <epsg2360> : Xian 1980 / 3-degree Gauss-Kruger zone 36
 <epsg2361> : Xian 1980 / 3-degree Gauss-Kruger zone 37
 <epsg2362> : Xian 1980 / 3-degree Gauss-Kruger zone 38
 <epsg2363> : Xian 1980 / 3-degree Gauss-Kruger zone 39
 <epsg2364> : Xian 1980 / 3-degree Gauss-Kruger zone 40
 <epsg2365> : Xian 1980 / 3-degree Gauss-Kruger zone 41
 <epsg2366> : Xian 1980 / 3-degree Gauss-Kruger zone 42
 <epsg2367> : Xian 1980 / 3-degree Gauss-Kruger zone 43
 <epsg2368> : Xian 1980 / 3-degree Gauss-Kruger zone 44
 <epsg2369> : Xian 1980 / 3-degree Gauss-Kruger zone 45
 <epsg2370> : Xian 1980 / 3-degree Gauss-Kruger CM 75E
 <epsg2371> : Xian 1980 / 3-degree Gauss-Kruger CM 78E
 <epsg2372> : Xian 1980 / 3-degree Gauss-Kruger CM 81E
 <epsg2373> : Xian 1980 / 3-degree Gauss-Kruger CM 84E
 <epsg2374> : Xian 1980 / 3-degree Gauss-Kruger CM 87E
 <epsg2375> : Xian 1980 / 3-degree Gauss-Kruger CM 90E
 <epsg2376> : Xian 1980 / 3-degree Gauss-Kruger CM 93E
 <epsg2377> : Xian 1980 / 3-degree Gauss-Kruger CM 96E
 <epsg2378> : Xian 1980 / 3-degree Gauss-Kruger CM 99E
 <epsg2379> : Xian 1980 / 3-degree Gauss-Kruger CM 102E
 <epsg2380> : Xian 1980 / 3-degree Gauss-Kruger CM 105E
 <epsg2381> : Xian 1980 / 3-degree Gauss-Kruger CM 108E
 <epsg2382> : Xian 1980 / 3-degree Gauss-Kruger CM 111E
 <epsg2383> : Xian 1980 / 3-degree Gauss-Kruger CM 114E
 <epsg2384> : Xian 1980 / 3-degree Gauss-Kruger CM 117E
 <epsg2385> : Xian 1980 / 3-degree Gauss-Kruger CM 120E
 <epsg2386> : Xian 1980 / 3-degree Gauss-Kruger CM 123E
 <epsg2387> : Xian 1980 / 3-degree Gauss-Kruger CM 126E
 <epsg2388> : Xian 1980 / 3-degree Gauss-Kruger CM 129E
 <epsg2389> : Xian 1980 / 3-degree Gauss-Kruger CM 132E
 <epsg2390> : Xian 1980 / 3-degree Gauss-Kruger CM 135E
 <epsg2391> : KKJ / Finland zone 1
 <epsg2392> : KKJ / Finland zone 2
 <epsg2393> : KKJ / Finland Uniform Coordinate System
 <epsg2394> : KKJ / Finland zone 4
 <epsg2395> : South Yemen / Gauss-Kruger zone 8
 <epsg2396> : South Yemen / Gauss-Kruger zone 9
 <epsg2397> : Pulkovo 1942(83) / Gauss-Kruger zone 3
 <epsg2398> : Pulkovo 1942(83) / Gauss-Kruger zone 4
 <epsg2399> : Pulkovo 1942(83) / Gauss-Kruger zone 5
 <epsg2400> : RT90 2.5 gon W (deprecated)
 <epsg2401> : Beijing 1954 / 3-degree Gauss-Kruger zone 25
 <epsg2402> : Beijing 1954 / 3-degree Gauss-Kruger zone 26
 <epsg2403> : Beijing 1954 / 3-degree Gauss-Kruger zone 27
 <epsg2404> : Beijing 1954 / 3-degree Gauss-Kruger zone 28
 <epsg2405> : Beijing 1954 / 3-degree Gauss-Kruger zone 29
 <epsg2406> : Beijing 1954 / 3-degree Gauss-Kruger zone 30
 <epsg2407> : Beijing 1954 / 3-degree Gauss-Kruger zone 31
 <epsg2408> : Beijing 1954 / 3-degree Gauss-Kruger zone 32
 <epsg2409> : Beijing 1954 / 3-degree Gauss-Kruger zone 33
 <epsg2410> : Beijing 1954 / 3-degree Gauss-Kruger zone 34
 <epsg2411> : Beijing 1954 / 3-degree Gauss-Kruger zone 35
 <epsg2412> : Beijing 1954 / 3-degree Gauss-Kruger zone 36
 <epsg2413> : Beijing 1954 / 3-degree Gauss-Kruger zone 37
 <epsg2414> : Beijing 1954 / 3-degree Gauss-Kruger zone 38
 <epsg2415> : Beijing 1954 / 3-degree Gauss-Kruger zone 39
 <epsg2416> : Beijing 1954 / 3-degree Gauss-Kruger zone 40
 <epsg2417> : Beijing 1954 / 3-degree Gauss-Kruger zone 41
 <epsg2418> : Beijing 1954 / 3-degree Gauss-Kruger zone 42
 <epsg2419> : Beijing 1954 / 3-degree Gauss-Kruger zone 43
 <epsg2420> : Beijing 1954 / 3-degree Gauss-Kruger zone 44
 <epsg2421> : Beijing 1954 / 3-degree Gauss-Kruger zone 45
 <epsg2422> : Beijing 1954 / 3-degree Gauss-Kruger CM 75E
 <epsg2423> : Beijing 1954 / 3-degree Gauss-Kruger CM 78E
 <epsg2424> : Beijing 1954 / 3-degree Gauss-Kruger CM 81E
 <epsg2425> : Beijing 1954 / 3-degree Gauss-Kruger CM 84E
 <epsg2426> : Beijing 1954 / 3-degree Gauss-Kruger CM 87E
 <epsg2427> : Beijing 1954 / 3-degree Gauss-Kruger CM 90E
 <epsg2428> : Beijing 1954 / 3-degree Gauss-Kruger CM 93E
 <epsg2429> : Beijing 1954 / 3-degree Gauss-Kruger CM 96E
 <epsg2430> : Beijing 1954 / 3-degree Gauss-Kruger CM 99E
 <epsg2431> : Beijing 1954 / 3-degree Gauss-Kruger CM 102E
 <epsg2432> : Beijing 1954 / 3-degree Gauss-Kruger CM 105E
 <epsg2433> : Beijing 1954 / 3-degree Gauss-Kruger CM 108E
 <epsg2434> : Beijing 1954 / 3-degree Gauss-Kruger CM 111E
 <epsg2435> : Beijing 1954 / 3-degree Gauss-Kruger CM 114E
 <epsg2436> : Beijing 1954 / 3-degree Gauss-Kruger CM 117E
 <epsg2437> : Beijing 1954 / 3-degree Gauss-Kruger CM 120E
 <epsg2438> : Beijing 1954 / 3-degree Gauss-Kruger CM 123E
 <epsg2439> : Beijing 1954 / 3-degree Gauss-Kruger CM 126E
 <epsg2440> : Beijing 1954 / 3-degree Gauss-Kruger CM 129E
 <epsg2441> : Beijing 1954 / 3-degree Gauss-Kruger CM 132E
 <epsg2442> : Beijing 1954 / 3-degree Gauss-Kruger CM 135E
 <epsg2443> : JGD2000 / Japan Plane Rectangular CS I
 <epsg2444> : JGD2000 / Japan Plane Rectangular CS II
 <epsg2445> : JGD2000 / Japan Plane Rectangular CS III
 <epsg2446> : JGD2000 / Japan Plane Rectangular CS IV
 <epsg2447> : JGD2000 / Japan Plane Rectangular CS V
 <epsg2448> : JGD2000 / Japan Plane Rectangular CS VI
 <epsg2449> : JGD2000 / Japan Plane Rectangular CS VII
 <epsg2450> : JGD2000 / Japan Plane Rectangular CS VIII
 <epsg2451> : JGD2000 / Japan Plane Rectangular CS IX
 <epsg2452> : JGD2000 / Japan Plane Rectangular CS X
 <epsg2453> : JGD2000 / Japan Plane Rectangular CS XI
 <epsg2454> : JGD2000 / Japan Plane Rectangular CS XII
 <epsg2455> : JGD2000 / Japan Plane Rectangular CS XIII
 <epsg2456> : JGD2000 / Japan Plane Rectangular CS XIV
 <epsg2457> : JGD2000 / Japan Plane Rectangular CS XV
 <epsg2458> : JGD2000 / Japan Plane Rectangular CS XVI
 <epsg2459> : JGD2000 / Japan Plane Rectangular CS XVII
 <epsg2460> : JGD2000 / Japan Plane Rectangular CS XVIII
 <epsg2461> : JGD2000 / Japan Plane Rectangular CS XIX
 <epsg2462> : Albanian 1987 / Gauss-Kruger zone 4
 <epsg2463> : Pulkovo 1995 / Gauss-Kruger CM 21E
 <epsg2464> : Pulkovo 1995 / Gauss-Kruger CM 27E
 <epsg2465> : Pulkovo 1995 / Gauss-Kruger CM 33E
 <epsg2466> : Pulkovo 1995 / Gauss-Kruger CM 39E
 <epsg2467> : Pulkovo 1995 / Gauss-Kruger CM 45E
 <epsg2468> : Pulkovo 1995 / Gauss-Kruger CM 51E
 <epsg2469> : Pulkovo 1995 / Gauss-Kruger CM 57E
 <epsg2470> : Pulkovo 1995 / Gauss-Kruger CM 63E
 <epsg2471> : Pulkovo 1995 / Gauss-Kruger CM 69E
 <epsg2472> : Pulkovo 1995 / Gauss-Kruger CM 75E
 <epsg2473> : Pulkovo 1995 / Gauss-Kruger CM 81E
 <epsg2474> : Pulkovo 1995 / Gauss-Kruger CM 87E
 <epsg2475> : Pulkovo 1995 / Gauss-Kruger CM 93E
 <epsg2476> : Pulkovo 1995 / Gauss-Kruger CM 99E
 <epsg2477> : Pulkovo 1995 / Gauss-Kruger CM 105E
 <epsg2478> : Pulkovo 1995 / Gauss-Kruger CM 111E
 <epsg2479> : Pulkovo 1995 / Gauss-Kruger CM 117E
 <epsg2480> : Pulkovo 1995 / Gauss-Kruger CM 123E
 <epsg2481> : Pulkovo 1995 / Gauss-Kruger CM 129E
 <epsg2482> : Pulkovo 1995 / Gauss-Kruger CM 135E
 <epsg2483> : Pulkovo 1995 / Gauss-Kruger CM 141E
 <epsg2484> : Pulkovo 1995 / Gauss-Kruger CM 147E
 <epsg2485> : Pulkovo 1995 / Gauss-Kruger CM 153E
 <epsg2486> : Pulkovo 1995 / Gauss-Kruger CM 159E
 <epsg2487> : Pulkovo 1995 / Gauss-Kruger CM 165E
 <epsg2488> : Pulkovo 1995 / Gauss-Kruger CM 171E
 <epsg2489> : Pulkovo 1995 / Gauss-Kruger CM 177E
 <epsg2490> : Pulkovo 1995 / Gauss-Kruger CM 177W
 <epsg2491> : Pulkovo 1995 / Gauss-Kruger CM 171W
 <epsg2492> : Pulkovo 1942 / Gauss-Kruger CM 9E
 <epsg2493> : Pulkovo 1942 / Gauss-Kruger CM 15E
 <epsg2494> : Pulkovo 1942 / Gauss-Kruger CM 21E
 <epsg2495> : Pulkovo 1942 / Gauss-Kruger CM 27E
 <epsg2496> : Pulkovo 1942 / Gauss-Kruger CM 33E
 <epsg2497> : Pulkovo 1942 / Gauss-Kruger CM 39E
 <epsg2498> : Pulkovo 1942 / Gauss-Kruger CM 45E
 <epsg2499> : Pulkovo 1942 / Gauss-Kruger CM 51E
 <epsg2500> : Pulkovo 1942 / Gauss-Kruger CM 57E
 <epsg2501> : Pulkovo 1942 / Gauss-Kruger CM 63E
 <epsg2502> : Pulkovo 1942 / Gauss-Kruger CM 69E
 <epsg2503> : Pulkovo 1942 / Gauss-Kruger CM 75E
 <epsg2504> : Pulkovo 1942 / Gauss-Kruger CM 81E
 <epsg2505> : Pulkovo 1942 / Gauss-Kruger CM 87E
 <epsg2506> : Pulkovo 1942 / Gauss-Kruger CM 93E
 <epsg2507> : Pulkovo 1942 / Gauss-Kruger CM 99E
 <epsg2508> : Pulkovo 1942 / Gauss-Kruger CM 105E
 <epsg2509> : Pulkovo 1942 / Gauss-Kruger CM 111E
 <epsg2510> : Pulkovo 1942 / Gauss-Kruger CM 117E
 <epsg2511> : Pulkovo 1942 / Gauss-Kruger CM 123E
 <epsg2512> : Pulkovo 1942 / Gauss-Kruger CM 129E
 <epsg2513> : Pulkovo 1942 / Gauss-Kruger CM 135E
 <epsg2514> : Pulkovo 1942 / Gauss-Kruger CM 141E
 <epsg2515> : Pulkovo 1942 / Gauss-Kruger CM 147E
 <epsg2516> : Pulkovo 1942 / Gauss-Kruger CM 153E
 <epsg2517> : Pulkovo 1942 / Gauss-Kruger CM 159E
 <epsg2518> : Pulkovo 1942 / Gauss-Kruger CM 165E
 <epsg2519> : Pulkovo 1942 / Gauss-Kruger CM 171E
 <epsg2520> : Pulkovo 1942 / Gauss-Kruger CM 177E
 <epsg2521> : Pulkovo 1942 / Gauss-Kruger CM 177W
 <epsg2522> : Pulkovo 1942 / Gauss-Kruger CM 171W
 <epsg2523> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 7
 <epsg2524> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 8
 <epsg2525> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 9
 <epsg2526> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 10
 <epsg2527> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 11
 <epsg2528> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 12
 <epsg2529> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 13
 <epsg2530> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 14
 <epsg2531> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 15
 <epsg2532> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 16
 <epsg2533> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 17
 <epsg2534> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 18
 <epsg2535> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 19
 <epsg2536> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 20
 <epsg2537> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 21
 <epsg2538> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 22
 <epsg2539> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 23
 <epsg2540> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 24
 <epsg2541> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 25
 <epsg2542> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 26
 <epsg2543> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 27
 <epsg2544> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 28
 <epsg2545> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 29
 <epsg2546> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 30
 <epsg2547> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 31
 <epsg2548> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 32
 <epsg2549> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 33
 <epsg2550> : Samboja / UTM zone 50S (deprecated)
 <epsg2551> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 34
 <epsg2552> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 35
 <epsg2553> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 36
 <epsg2554> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 37
 <epsg2555> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 38
 <epsg2556> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 39
 <epsg2557> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 40
 <epsg2558> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 41
 <epsg2559> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 42
 <epsg2560> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 43
 <epsg2561> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 44
 <epsg2562> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 45
 <epsg2563> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 46
 <epsg2564> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 47
 <epsg2565> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 48
 <epsg2566> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 49
 <epsg2567> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 50
 <epsg2568> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 51
 <epsg2569> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 52
 <epsg2570> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 53
 <epsg2571> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 54
 <epsg2572> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 55
 <epsg2573> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 56
 <epsg2574> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 57
 <epsg2575> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 58
 <epsg2576> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 59
 <epsg2577> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 60 (deprecated)
 <epsg2578> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 61
 <epsg2579> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 62
 <epsg2580> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 63
 <epsg2581> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 64
 <epsg2582> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 21E
 <epsg2583> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 24E
 <epsg2584> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 27E
 <epsg2585> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 30E
 <epsg2586> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 33E
 <epsg2587> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 36E
 <epsg2588> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 39E
 <epsg2589> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 42E
 <epsg2590> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 45E
 <epsg2591> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 48E
 <epsg2592> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 51E
 <epsg2593> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 54E
 <epsg2594> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 57E
 <epsg2595> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 60E
 <epsg2596> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 63E
 <epsg2597> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 66E
 <epsg2598> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 69E
 <epsg2599> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 72E
 <epsg2600> : Lietuvos Koordinoei Sistema 1994 (deprecated)
 <epsg2601> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 75E
 <epsg2602> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 78E
 <epsg2603> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 81E
 <epsg2604> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 84E
 <epsg2605> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 87E
 <epsg2606> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 90E
 <epsg2607> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 93E
 <epsg2608> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 96E
 <epsg2609> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 99E
 <epsg2610> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 102E
 <epsg2611> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 105E
 <epsg2612> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 108E
 <epsg2613> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 111E
 <epsg2614> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 114E
 <epsg2615> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 117E
 <epsg2616> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 120E
 <epsg2617> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 123E
 <epsg2618> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 126E
 <epsg2619> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 129E
 <epsg2620> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 132E
 <epsg2621> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 135E
 <epsg2622> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 138E
 <epsg2623> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 141E
 <epsg2624> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 144E
 <epsg2625> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 147E
 <epsg2626> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 150E
 <epsg2627> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 153E
 <epsg2628> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 156E
 <epsg2629> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 159E
 <epsg2630> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 162E
 <epsg2631> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 165E
 <epsg2632> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 168E
 <epsg2633> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 171E
 <epsg2634> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 174E
 <epsg2635> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 177E
 <epsg2636> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 180E
 <epsg2637> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 177W
 <epsg2638> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 174W
 <epsg2639> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 171W
 <epsg2640> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 168W
 <epsg2641> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 7
 <epsg2642> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 8
 <epsg2643> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 9
 <epsg2644> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 10
 <epsg2645> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 11
 <epsg2646> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 12
 <epsg2647> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 13
 <epsg2648> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 14
 <epsg2649> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 15
 <epsg2650> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 16
 <epsg2651> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 17
 <epsg2652> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 18
 <epsg2653> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 19
 <epsg2654> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 20
 <epsg2655> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 21
 <epsg2656> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 22
 <epsg2657> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 23
 <epsg2658> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 24
 <epsg2659> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 25
 <epsg2660> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 26
 <epsg2661> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 27
 <epsg2662> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 28
 <epsg2663> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 29
 <epsg2664> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 30
 <epsg2665> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 31
 <epsg2666> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 32
 <epsg2667> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 33
 <epsg2668> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 34
 <epsg2669> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 35
 <epsg2670> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 36
 <epsg2671> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 37
 <epsg2672> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 38
 <epsg2673> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 39
 <epsg2674> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 40
 <epsg2675> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 41
 <epsg2676> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 42
 <epsg2677> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 43
 <epsg2678> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 44
 <epsg2679> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 45
 <epsg2680> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 46
 <epsg2681> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 47
 <epsg2682> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 48
 <epsg2683> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 49
 <epsg2684> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 50
 <epsg2685> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 51
 <epsg2686> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 52
 <epsg2687> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 53
 <epsg2688> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 54
 <epsg2689> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 55
 <epsg2690> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 56
 <epsg2691> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 57
 <epsg2692> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 58
 <epsg2693> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 59
 <epsg2694> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 60 (deprecated)
 <epsg2695> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 61
 <epsg2696> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 62
 <epsg2697> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 63
 <epsg2698> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 64
 <epsg2699> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 21E
 <epsg2700> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 24E
 <epsg2701> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 27E
 <epsg2702> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 30E
 <epsg2703> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 33E
 <epsg2704> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 36E
 <epsg2705> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 39E
 <epsg2706> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 42E
 <epsg2707> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 45E
 <epsg2708> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 48E
 <epsg2709> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 51E
 <epsg2710> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 54E
 <epsg2711> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 57E
 <epsg2712> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 60E
 <epsg2713> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 63E
 <epsg2714> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 66E
 <epsg2715> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 69E
 <epsg2716> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 72E
 <epsg2717> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 75E
 <epsg2718> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 78E
 <epsg2719> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 81E
 <epsg2720> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 84E
 <epsg2721> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 87E
 <epsg2722> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 90E
 <epsg2723> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 93E
 <epsg2724> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 96E
 <epsg2725> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 99E
 <epsg2726> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 102E
 <epsg2727> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 105E
 <epsg2728> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 108E
 <epsg2729> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 111E
 <epsg2730> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 114E
 <epsg2731> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 117E
 <epsg2732> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 120E
 <epsg2733> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 123E
 <epsg2734> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 126E
 <epsg2735> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 129E
 <epsg2736> : Tete / UTM zone 36S
 <epsg2737> : Tete / UTM zone 37S
 <epsg2738> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 132E
 <epsg2739> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 135E
 <epsg2740> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 138E
 <epsg2741> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 141E
 <epsg2742> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 144E
 <epsg2743> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 147E
 <epsg2744> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 150E
 <epsg2745> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 153E
 <epsg2746> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 156E
 <epsg2747> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 159E
 <epsg2748> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 162E
 <epsg2749> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 165E
 <epsg2750> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 168E
 <epsg2751> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 171E
 <epsg2752> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 174E
 <epsg2753> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 177E
 <epsg2754> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 180E
 <epsg2755> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 177W
 <epsg2756> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 174W
 <epsg2757> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 171W
 <epsg2758> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 168W
 <epsg2759> : NAD83(HARN) / Alabama East
 <epsg2760> : NAD83(HARN) / Alabama West
 <epsg2761> : NAD83(HARN) / Arizona East
 <epsg2762> : NAD83(HARN) / Arizona Central
 <epsg2763> : NAD83(HARN) / Arizona West
 <epsg2764> : NAD83(HARN) / Arkansas North
 <epsg2765> : NAD83(HARN) / Arkansas South
 <epsg2766> : NAD83(HARN) / California zone 1
 <epsg2767> : NAD83(HARN) / California zone 2
 <epsg2768> : NAD83(HARN) / California zone 3
 <epsg2769> : NAD83(HARN) / California zone 4
 <epsg2770> : NAD83(HARN) / California zone 5
 <epsg2771> : NAD83(HARN) / California zone 6
 <epsg2772> : NAD83(HARN) / Colorado North
 <epsg2773> : NAD83(HARN) / Colorado Central
 <epsg2774> : NAD83(HARN) / Colorado South
 <epsg2775> : NAD83(HARN) / Connecticut
 <epsg2776> : NAD83(HARN) / Delaware
 <epsg2777> : NAD83(HARN) / Florida East
 <epsg2778> : NAD83(HARN) / Florida West
 <epsg2779> : NAD83(HARN) / Florida North
 <epsg2780> : NAD83(HARN) / Georgia East
 <epsg2781> : NAD83(HARN) / Georgia West
 <epsg2782> : NAD83(HARN) / Hawaii zone 1
 <epsg2783> : NAD83(HARN) / Hawaii zone 2
 <epsg2784> : NAD83(HARN) / Hawaii zone 3
 <epsg2785> : NAD83(HARN) / Hawaii zone 4
 <epsg2786> : NAD83(HARN) / Hawaii zone 5
 <epsg2787> : NAD83(HARN) / Idaho East
 <epsg2788> : NAD83(HARN) / Idaho Central
 <epsg2789> : NAD83(HARN) / Idaho West
 <epsg2790> : NAD83(HARN) / Illinois East
 <epsg2791> : NAD83(HARN) / Illinois West
 <epsg2792> : NAD83(HARN) / Indiana East
 <epsg2793> : NAD83(HARN) / Indiana West
 <epsg2794> : NAD83(HARN) / Iowa North
 <epsg2795> : NAD83(HARN) / Iowa South
 <epsg2796> : NAD83(HARN) / Kansas North
 <epsg2797> : NAD83(HARN) / Kansas South
 <epsg2798> : NAD83(HARN) / Kentucky North
 <epsg2799> : NAD83(HARN) / Kentucky South
 <epsg2800> : NAD83(HARN) / Louisiana North
 <epsg2801> : NAD83(HARN) / Louisiana South
 <epsg2802> : NAD83(HARN) / Maine East
 <epsg2803> : NAD83(HARN) / Maine West
 <epsg2804> : NAD83(HARN) / Maryland
 <epsg2805> : NAD83(HARN) / Massachusetts Mainland
 <epsg2806> : NAD83(HARN) / Massachusetts Island
 <epsg2807> : NAD83(HARN) / Michigan North
 <epsg2808> : NAD83(HARN) / Michigan Central
 <epsg2809> : NAD83(HARN) / Michigan South
 <epsg2810> : NAD83(HARN) / Minnesota North
 <epsg2811> : NAD83(HARN) / Minnesota Central
 <epsg2812> : NAD83(HARN) / Minnesota South
 <epsg2813> : NAD83(HARN) / Mississippi East
 <epsg2814> : NAD83(HARN) / Mississippi West
 <epsg2815> : NAD83(HARN) / Missouri East
 <epsg2816> : NAD83(HARN) / Missouri Central
 <epsg2817> : NAD83(HARN) / Missouri West
 <epsg2818> : NAD83(HARN) / Montana
 <epsg2819> : NAD83(HARN) / Nebraska
 <epsg2820> : NAD83(HARN) / Nevada East
 <epsg2821> : NAD83(HARN) / Nevada Central
 <epsg2822> : NAD83(HARN) / Nevada West
 <epsg2823> : NAD83(HARN) / New Hampshire
 <epsg2824> : NAD83(HARN) / New Jersey
 <epsg2825> : NAD83(HARN) / New Mexico East
 <epsg2826> : NAD83(HARN) / New Mexico Central
 <epsg2827> : NAD83(HARN) / New Mexico West
 <epsg2828> : NAD83(HARN) / New York East
 <epsg2829> : NAD83(HARN) / New York Central
 <epsg2830> : NAD83(HARN) / New York West
 <epsg2831> : NAD83(HARN) / New York Long Island
 <epsg2832> : NAD83(HARN) / North Dakota North
 <epsg2833> : NAD83(HARN) / North Dakota South
 <epsg2834> : NAD83(HARN) / Ohio North
 <epsg2835> : NAD83(HARN) / Ohio South
 <epsg2836> : NAD83(HARN) / Oklahoma North
 <epsg2837> : NAD83(HARN) / Oklahoma South
 <epsg2838> : NAD83(HARN) / Oregon North
 <epsg2839> : NAD83(HARN) / Oregon South
 <epsg2840> : NAD83(HARN) / Rhode Island
 <epsg2841> : NAD83(HARN) / South Dakota North
 <epsg2842> : NAD83(HARN) / South Dakota South
 <epsg2843> : NAD83(HARN) / Tennessee
 <epsg2844> : NAD83(HARN) / Texas North
 <epsg2845> : NAD83(HARN) / Texas North Central
 <epsg2846> : NAD83(HARN) / Texas Central
 <epsg2847> : NAD83(HARN) / Texas South Central
 <epsg2848> : NAD83(HARN) / Texas South
 <epsg2849> : NAD83(HARN) / Utah North
 <epsg2850> : NAD83(HARN) / Utah Central
 <epsg2851> : NAD83(HARN) / Utah South
 <epsg2852> : NAD83(HARN) / Vermont
 <epsg2853> : NAD83(HARN) / Virginia North
 <epsg2854> : NAD83(HARN) / Virginia South
 <epsg2855> : NAD83(HARN) / Washington North
 <epsg2856> : NAD83(HARN) / Washington South
 <epsg2857> : NAD83(HARN) / West Virginia North
 <epsg2858> : NAD83(HARN) / West Virginia South
 <epsg2859> : NAD83(HARN) / Wisconsin North
 <epsg2860> : NAD83(HARN) / Wisconsin Central
 <epsg2861> : NAD83(HARN) / Wisconsin South
 <epsg2862> : NAD83(HARN) / Wyoming East
 <epsg2863> : NAD83(HARN) / Wyoming East Central
 <epsg2864> : NAD83(HARN) / Wyoming West Central
 <epsg2865> : NAD83(HARN) / Wyoming West
 <epsg2866> : NAD83(HARN) / Puerto Rico & Virgin Is.
 <epsg2867> : NAD83(HARN) / Arizona East (ft)
 <epsg2868> : NAD83(HARN) / Arizona Central (ft)
 <epsg2869> : NAD83(HARN) / Arizona West (ft)
 <epsg2870> : NAD83(HARN) / California zone 1 (ftUS)
 <epsg2871> : NAD83(HARN) / California zone 2 (ftUS)
 <epsg2872> : NAD83(HARN) / California zone 3 (ftUS)
 <epsg2873> : NAD83(HARN) / California zone 4 (ftUS)
 <epsg2874> : NAD83(HARN) / California zone 5 (ftUS)
 <epsg2875> : NAD83(HARN) / California zone 6 (ftUS)
 <epsg2876> : NAD83(HARN) / Colorado North (ftUS)
 <epsg2877> : NAD83(HARN) / Colorado Central (ftUS)
 <epsg2878> : NAD83(HARN) / Colorado South (ftUS)
 <epsg2879> : NAD83(HARN) / Connecticut (ftUS)
 <epsg2880> : NAD83(HARN) / Delaware (ftUS)
 <epsg2881> : NAD83(HARN) / Florida East (ftUS)
 <epsg2882> : NAD83(HARN) / Florida West (ftUS)
 <epsg2883> : NAD83(HARN) / Florida North (ftUS)
 <epsg2884> : NAD83(HARN) / Georgia East (ftUS)
 <epsg2885> : NAD83(HARN) / Georgia West (ftUS)
 <epsg2886> : NAD83(HARN) / Idaho East (ftUS)
 <epsg2887> : NAD83(HARN) / Idaho Central (ftUS)
 <epsg2888> : NAD83(HARN) / Idaho West (ftUS)
 <epsg2889> : NAD83(HARN) / Indiana East (ftUS) (deprecated)
 <epsg2890> : NAD83(HARN) / Indiana West (ftUS) (deprecated)
 <epsg2891> : NAD83(HARN) / Kentucky North (ftUS)
 <epsg2892> : NAD83(HARN) / Kentucky South (ftUS)
 <epsg2893> : NAD83(HARN) / Maryland (ftUS)
 <epsg2894> : NAD83(HARN) / Massachusetts Mainland (ftUS)
 <epsg2895> : NAD83(HARN) / Massachusetts Island (ftUS)
 <epsg2896> : NAD83(HARN) / Michigan North (ft)
 <epsg2897> : NAD83(HARN) / Michigan Central (ft)
 <epsg2898> : NAD83(HARN) / Michigan South (ft)
 <epsg2899> : NAD83(HARN) / Mississippi East (ftUS)
 <epsg2900> : NAD83(HARN) / Mississippi West (ftUS)
 <epsg2901> : NAD83(HARN) / Montana (ft)
 <epsg2902> : NAD83(HARN) / New Mexico East (ftUS)
 <epsg2903> : NAD83(HARN) / New Mexico Central (ftUS)
 <epsg2904> : NAD83(HARN) / New Mexico West (ftUS)
 <epsg2905> : NAD83(HARN) / New York East (ftUS)
 <epsg2906> : NAD83(HARN) / New York Central (ftUS)
 <epsg2907> : NAD83(HARN) / New York West (ftUS)
 <epsg2908> : NAD83(HARN) / New York Long Island (ftUS)
 <epsg2909> : NAD83(HARN) / North Dakota North (ft)
 <epsg2910> : NAD83(HARN) / North Dakota South (ft)
 <epsg2911> : NAD83(HARN) / Oklahoma North (ftUS)
 <epsg2912> : NAD83(HARN) / Oklahoma South (ftUS)
 <epsg2913> : NAD83(HARN) / Oregon North (ft)
 <epsg2914> : NAD83(HARN) / Oregon South (ft)
 <epsg2915> : NAD83(HARN) / Tennessee (ftUS)
 <epsg2916> : NAD83(HARN) / Texas North (ftUS)
 <epsg2917> : NAD83(HARN) / Texas North Central (ftUS)
 <epsg2918> : NAD83(HARN) / Texas Central (ftUS)
 <epsg2919> : NAD83(HARN) / Texas South Central (ftUS)
 <epsg2920> : NAD83(HARN) / Texas South (ftUS)
 <epsg2921> : NAD83(HARN) / Utah North (ft)
 <epsg2922> : NAD83(HARN) / Utah Central (ft)
 <epsg2923> : NAD83(HARN) / Utah South (ft)
 <epsg2924> : NAD83(HARN) / Virginia North (ftUS)
 <epsg2925> : NAD83(HARN) / Virginia South (ftUS)
 <epsg2926> : NAD83(HARN) / Washington North (ftUS)
 <epsg2927> : NAD83(HARN) / Washington South (ftUS)
 <epsg2928> : NAD83(HARN) / Wisconsin North (ftUS)
 <epsg2929> : NAD83(HARN) / Wisconsin Central (ftUS)
 <epsg2930> : NAD83(HARN) / Wisconsin South (ftUS)
 <epsg2931> : Beduaram / TM 13 NE
 <epsg2932> : QND95 / Qatar National Grid
 <epsg2933> : Segara / UTM zone 50S
 <epsg2934> : Segara (Jakarta) / NEIEZ (deprecated)
 <epsg2935> : Pulkovo 1942 / CS63 zone A1
 <epsg2936> : Pulkovo 1942 / CS63 zone A2
 <epsg2937> : Pulkovo 1942 / CS63 zone A3
 <epsg2938> : Pulkovo 1942 / CS63 zone A4
 <epsg2939> : Pulkovo 1942 / CS63 zone K2
 <epsg2940> : Pulkovo 1942 / CS63 zone K3
 <epsg2941> : Pulkovo 1942 / CS63 zone K4
 <epsg2942> : Porto Santo / UTM zone 28N
 <epsg2943> : Selvagem Grande / UTM zone 28N
 <epsg2944> : NAD83(CSRS) / SCoPQ zone 2
 <epsg2945> : NAD83(CSRS) / MTM zone 3
 <epsg2946> : NAD83(CSRS) / MTM zone 4
 <epsg2947> : NAD83(CSRS) / MTM zone 5
 <epsg2948> : NAD83(CSRS) / MTM zone 6
 <epsg2949> : NAD83(CSRS) / MTM zone 7
 <epsg2950> : NAD83(CSRS) / MTM zone 8
 <epsg2951> : NAD83(CSRS) / MTM zone 9
 <epsg2952> : NAD83(CSRS) / MTM zone 10
 <epsg2953> : NAD83(CSRS) / New Brunswick Stereo
 <epsg2954> : NAD83(CSRS) / Prince Edward Isl. Stereographic (NAD83)
 <epsg2955> : NAD83(CSRS) / UTM zone 11N
 <epsg2956> : NAD83(CSRS) / UTM zone 12N
 <epsg2957> : NAD83(CSRS) / UTM zone 13N
 <epsg2958> : NAD83(CSRS) / UTM zone 17N
 <epsg2959> : NAD83(CSRS) / UTM zone 18N
 <epsg2960> : NAD83(CSRS) / UTM zone 19N
 <epsg2961> : NAD83(CSRS) / UTM zone 20N
 <epsg2962> : NAD83(CSRS) / UTM zone 21N
 <epsg2964> : NAD27 / Alaska Albers
 <epsg2965> : NAD83 / Indiana East (ftUS)
 <epsg2966> : NAD83 / Indiana West (ftUS)
 <epsg2967> : NAD83(HARN) / Indiana East (ftUS)
 <epsg2968> : NAD83(HARN) / Indiana West (ftUS)
 <epsg2969> : Fort Marigot / UTM zone 20N
 <epsg2970> : Guadeloupe 1948 / UTM zone 20N
 <epsg2971> : CSG67 / UTM zone 22N
 <epsg2972> : RGFG95 / UTM zone 22N
 <epsg2973> : Martinique 1938 / UTM zone 20N
 <epsg2975> : RGR92 / UTM zone 40S
 <epsg2976> : Tahiti 52 / UTM zone 6S
 <epsg2977> : Tahaa 54 / UTM zone 5S
 <epsg2978> : IGN72 Nuku Hiva / UTM zone 7S
 <epsg2979> : K0 1949 / UTM zone 42S (deprecated)
 <epsg2980> : Combani 1950 / UTM zone 38S
 <epsg2981> : IGN56 Lifou / UTM zone 58S
 <epsg2982> : IGN72 Grand Terre / UTM zone 58S (deprecated)
 <epsg2983> : ST87 Ouvea / UTM zone 58S (deprecated)
 <epsg2984> : RGNC 1991 / Lambert New Caledonia (deprecated)
 <epsg2987> : Saint Pierre et Miquelon 1950 / UTM zone 21N
 <epsg2988> : MOP78 / UTM zone 1S
 <epsg2989> : RRAF 1991 / UTM zone 20N
 <epsg2990> : Reunion 1947 / TM Reunion (deprecated)
 <epsg2991> : NAD83 / Oregon Lambert
 <epsg2992> : NAD83 / Oregon Lambert (ft)
 <epsg2993> : NAD83(HARN) / Oregon Lambert
 <epsg2994> : NAD83(HARN) / Oregon Lambert (ft)
 <epsg2995> : IGN53 Mare / UTM zone 58S
 <epsg2996> : ST84 Ile des Pins / UTM zone 58S
 <epsg2997> : ST71 Belep / UTM zone 58S
 <epsg2998> : NEA74 Noumea / UTM zone 58S
 <epsg2999> : Grand Comoros / UTM zone 38S
 <epsg3000> : Segara / NEIEZ
 <epsg3001> : Batavia / NEIEZ
 <epsg3002> : Makassar / NEIEZ
 <epsg3003> : Monte Mario / Italy zone 1
 <epsg3004> : Monte Mario / Italy zone 2
 <epsg3005> : NAD83 / BC Albers
 <epsg3006> : SWEREF99 TM
 <epsg3007> : SWEREF99 12 00
 <epsg3008> : SWEREF99 13 30
 <epsg3009> : SWEREF99 15 00
 <epsg3010> : SWEREF99 16 30
 <epsg3011> : SWEREF99 18 00
 <epsg3012> : SWEREF99 14 15
 <epsg3013> : SWEREF99 15 45
 <epsg3014> : SWEREF99 17 15
 <epsg3015> : SWEREF99 18 45
 <epsg3016> : SWEREF99 20 15
 <epsg3017> : SWEREF99 21 45
 <epsg3018> : SWEREF99 23 15
 <epsg3019> : RT90 7.5 gon V
 <epsg3020> : RT90 5 gon V
 <epsg3021> : RT90 2.5 gon V
 <epsg3022> : RT90 0 gon
 <epsg3023> : RT90 2.5 gon O
 <epsg3024> : RT90 5 gon O
 <epsg3025> : RT38 7.5 gon V
 <epsg3026> : RT38 5 gon V
 <epsg3027> : RT38 2.5 gon V
 <epsg3028> : RT38 0 gon
 <epsg3029> : RT38 2.5 gon O
 <epsg3030> : RT38 5 gon O
 <epsg3031> : WGS 84 / Antarctic Polar Stereographic
 <epsg3032> : WGS 84 / Australian Antarctic Polar Stereographic
 <epsg3033> : WGS 84 / Australian Antarctic Lambert
 <epsg3034> : ETRS89 / ETRS-LCC
 <epsg3035> : ETRS89 / ETRS-LAEA
 <epsg3036> : Moznet / UTM zone 36S
 <epsg3037> : Moznet / UTM zone 37S
 <epsg3038> : ETRS89 / ETRS-TM26
 <epsg3039> : ETRS89 / ETRS-TM27
 <epsg3040> : ETRS89 / ETRS-TM28
 <epsg3041> : ETRS89 / ETRS-TM29
 <epsg3042> : ETRS89 / ETRS-TM30
 <epsg3043> : ETRS89 / ETRS-TM31
 <epsg3044> : ETRS89 / ETRS-TM32
 <epsg3045> : ETRS89 / ETRS-TM33
 <epsg3046> : ETRS89 / ETRS-TM34
 <epsg3047> : ETRS89 / ETRS-TM35
 <epsg3048> : ETRS89 / ETRS-TM36
 <epsg3049> : ETRS89 / ETRS-TM37
 <epsg3050> : ETRS89 / ETRS-TM38
 <epsg3051> : ETRS89 / ETRS-TM39
 <epsg3054> : Hjorsey 1955 / UTM zone 26N
 <epsg3055> : Hjorsey 1955 / UTM zone 27N
 <epsg3056> : Hjorsey 1955 / UTM zone 28N
 <epsg3057> : ISN93 / Lambert 1993
 <epsg3058> : Helle 1954 / Jan Mayen Grid
 <epsg3059> : LKS92 / Latvia TM
 <epsg3060> : IGN72 Grande Terre / UTM zone 58S
 <epsg3061> : Porto Santo 1995 / UTM zone 28N
 <epsg3062> : Azores Oriental 1995 / UTM zone 26N
 <epsg3063> : Azores Central 1995 / UTM zone 26N
 <epsg3064> : IGM95 / UTM zone 32N
 <epsg3065> : IGM95 / UTM zone 33N
 <epsg3066> : ED50 / Jordan TM
 <epsg3067> : ETRS89 / ETRS-TM35FIN
 <epsg3068> : DHDN / Soldner Berlin
 <epsg3069> : NAD27 / Wisconsin Transverse Mercator
 <epsg3070> : NAD83 / Wisconsin Transverse Mercator
 <epsg3071> : NAD83(HARN) / Wisconsin Transverse Mercator
 <epsg3072> : NAD83 / Maine CS2000 East
 <epsg3073> : NAD83 / Maine CS2000 Central (deprecated)
 <epsg3074> : NAD83 / Maine CS2000 West
 <epsg3075> : NAD83(HARN) / Maine CS2000 East
 <epsg3076> : NAD83(HARN) / Maine CS2000 Central (deprecated)
 <epsg3077> : NAD83(HARN) / Maine CS2000 West
 <epsg3078> : NAD83 / Michigan Oblique Mercator
 <epsg3079> : NAD83(HARN) / Michigan Oblique Mercator
 <epsg3080> : NAD27 / Shackleford
 <epsg3081> : NAD83 / Texas State Mapping System
 <epsg3082> : NAD83 / Texas Centric Lambert Conformal
 <epsg3083> : NAD83 / Texas Centric Albers Equal Area
 <epsg3084> : NAD83(HARN) / Texas Centric Lambert Conformal
 <epsg3085> : NAD83(HARN) / Texas Centric Albers Equal Area
 <epsg3086> : NAD83 / Florida GDL Albers
 <epsg3087> : NAD83(HARN) / Florida GDL Albers
 <epsg3088> : NAD83 / Kentucky Single Zone
 <epsg3089> : NAD83 / Kentucky Single Zone (ftUS)
 <epsg3090> : NAD83(HARN) / Kentucky Single Zone
 <epsg3091> : NAD83(HARN) / Kentucky Single Zone (ftUS)
 <epsg3092> : Tokyo / UTM zone 51N
 <epsg3093> : Tokyo / UTM zone 52N
 <epsg3094> : Tokyo / UTM zone 53N
 <epsg3095> : Tokyo / UTM zone 54N
 <epsg3096> : Tokyo / UTM zone 55N
 <epsg3097> : JGD2000 / UTM zone 51N
 <epsg3098> : JGD2000 / UTM zone 52N
 <epsg3099> : JGD2000 / UTM zone 53N
 <epsg3100> : JGD2000 / UTM zone 54N
 <epsg3101> : JGD2000 / UTM zone 55N
 <epsg3102> : American Samoa 1962 / American Samoa Lambert
 <epsg3103> : Mauritania 1999 / UTM zone 28N (deprecated)
 <epsg3104> : Mauritania 1999 / UTM zone 29N (deprecated)
 <epsg3105> : Mauritania 1999 / UTM zone 30N (deprecated)
 <epsg3106> : Gulshan 303 / Bangladesh Transverse Mercator
 <epsg3107> : GDA94 / SA Lambert
 <epsg3108> : ETRS89 / Guernsey Grid
 <epsg3109> : ETRS89 / Jersey Transverse Mercator
 <epsg3110> : AGD66 / Vicgrid66
 <epsg3111> : GDA94 / Vicgrid94
 <epsg3112> : GDA94 / Geoscience Australia Lambert
 <epsg3113> : GDA94 / BCSG02
 <epsg3114> : MAGNA-SIRGAS / Colombia Far West zone
 <epsg3115> : MAGNA-SIRGAS / Colombia West zone
 <epsg3116> : MAGNA-SIRGAS / Colombia Bogota zone
 <epsg3117> : MAGNA-SIRGAS / Colombia East Central zone
 <epsg3118> : MAGNA-SIRGAS / Colombia East zone
 <epsg3119> : Douala 1948 / AEF west
 <epsg3120> : Pulkovo 1942(58) / Poland zone I
 <epsg3121> : PRS92 / Philippines zone 1
 <epsg3122> : PRS92 / Philippines zone 2
 <epsg3123> : PRS92 / Philippines zone 3
 <epsg3124> : PRS92 / Philippines zone 4
 <epsg3125> : PRS92 / Philippines zone 5
 <epsg3126> : ETRS89 / ETRS-GK19FIN
 <epsg3127> : ETRS89 / ETRS-GK20FIN
 <epsg3128> : ETRS89 / ETRS-GK21FIN
 <epsg3129> : ETRS89 / ETRS-GK22FIN
 <epsg3130> : ETRS89 / ETRS-GK23FIN
 <epsg3131> : ETRS89 / ETRS-GK24FIN
 <epsg3132> : ETRS89 / ETRS-GK25FIN
 <epsg3133> : ETRS89 / ETRS-GK26FIN
 <epsg3134> : ETRS89 / ETRS-GK27FIN
 <epsg3135> : ETRS89 / ETRS-GK28FIN
 <epsg3136> : ETRS89 / ETRS-GK29FIN
 <epsg3137> : ETRS89 / ETRS-GK30FIN
 <epsg3138> : ETRS89 / ETRS-GK31FIN
 <epsg3140> : Viti Levu 1912 / Viti Levu Grid
 <epsg3141> : Fiji 1956 / UTM zone 60S
 <epsg3142> : Fiji 1956 / UTM zone 1S
 <epsg3143> : Fiji 1986 / Fiji Map Grid (deprecated)
 <epsg3146> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 6
 <epsg3147> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 18E
 <epsg3148> : Indian 1960 / UTM zone 48N
 <epsg3149> : Indian 1960 / UTM zone 49N
 <epsg3150> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 6
 <epsg3151> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 18E
 <epsg3152> : ST74
 <epsg3153> : NAD83(CSRS) / BC Albers
 <epsg3154> : NAD83(CSRS) / UTM zone 7N
 <epsg3155> : NAD83(CSRS) / UTM zone 8N
 <epsg3156> : NAD83(CSRS) / UTM zone 9N
 <epsg3157> : NAD83(CSRS) / UTM zone 10N
 <epsg3158> : NAD83(CSRS) / UTM zone 14N
 <epsg3159> : NAD83(CSRS) / UTM zone 15N
 <epsg3160> : NAD83(CSRS) / UTM zone 16N
 <epsg3161> : NAD83 / Ontario MNR Lambert
 <epsg3162> : NAD83(CSRS) / Ontario MNR Lambert
 <epsg3163> : RGNC91-93 / Lambert New Caledonia
 <epsg3164> : ST87 Ouvea / UTM zone 58S
 <epsg3165> : NEA74 Noumea / Noumea Lambert
 <epsg3166> : NEA74 Noumea / Noumea Lambert 2
 <epsg3167> : Kertau (RSO) / RSO Malaya (ch)
 <epsg3168> : Kertau (RSO) / RSO Malaya (m)
 <epsg3169> : RGNC91-93 / UTM zone 57S
 <epsg3170> : RGNC91-93 / UTM zone 58S
 <epsg3171> : RGNC91-93 / UTM zone 59S
 <epsg3172> : IGN53 Mare / UTM zone 59S
 <epsg3174> : NAD83 / Great Lakes Albers
 <epsg3175> : NAD83 / Great Lakes and St Lawrence Albers
 <epsg3176> : Indian 1960 / TM 106 NE
 <epsg3177> : LGD2006 / Libya TM
 <epsg3178> : GR96 / UTM zone 18N
 <epsg3179> : GR96 / UTM zone 19N
 <epsg3180> : GR96 / UTM zone 20N
 <epsg3181> : GR96 / UTM zone 21N
 <epsg3182> : GR96 / UTM zone 22N
 <epsg3183> : GR96 / UTM zone 23N
 <epsg3184> : GR96 / UTM zone 24N
 <epsg3185> : GR96 / UTM zone 25N
 <epsg3186> : GR96 / UTM zone 26N
 <epsg3187> : GR96 / UTM zone 27N
 <epsg3188> : GR96 / UTM zone 28N
 <epsg3189> : GR96 / UTM zone 29N
 <epsg3190> : LGD2006 / Libya TM zone 5
 <epsg3191> : LGD2006 / Libya TM zone 6
 <epsg3192> : LGD2006 / Libya TM zone 7
 <epsg3193> : LGD2006 / Libya TM zone 8
 <epsg3194> : LGD2006 / Libya TM zone 9
 <epsg3195> : LGD2006 / Libya TM zone 10
 <epsg3196> : LGD2006 / Libya TM zone 11
 <epsg3197> : LGD2006 / Libya TM zone 12
 <epsg3198> : LGD2006 / Libya TM zone 13
 <epsg3199> : LGD2006 / UTM zone 32N
 <epsg3200> : FD58 / Iraq zone
 <epsg3201> : LGD2006 / UTM zone 33N
 <epsg3202> : LGD2006 / UTM zone 34N
 <epsg3203> : LGD2006 / UTM zone 35N
 <epsg3204> : WGS 84 / SCAR IMW SP19-20
 <epsg3205> : WGS 84 / SCAR IMW SP21-22
 <epsg3206> : WGS 84 / SCAR IMW SP23-24
 <epsg3207> : WGS 84 / SCAR IMW SQ01-02
 <epsg3208> : WGS 84 / SCAR IMW SQ19-20
 <epsg3209> : WGS 84 / SCAR IMW SQ21-22
 <epsg3210> : WGS 84 / SCAR IMW SQ37-38
 <epsg3211> : WGS 84 / SCAR IMW SQ39-40
 <epsg3212> : WGS 84 / SCAR IMW SQ41-42
 <epsg3213> : WGS 84 / SCAR IMW SQ43-44
 <epsg3214> : WGS 84 / SCAR IMW SQ45-46
 <epsg3215> : WGS 84 / SCAR IMW SQ47-48
 <epsg3216> : WGS 84 / SCAR IMW SQ49-50
 <epsg3217> : WGS 84 / SCAR IMW SQ51-52
 <epsg3218> : WGS 84 / SCAR IMW SQ53-54
 <epsg3219> : WGS 84 / SCAR IMW SQ55-56
 <epsg3220> : WGS 84 / SCAR IMW SQ57-58
 <epsg3221> : WGS 84 / SCAR IMW SR13-14
 <epsg3222> : WGS 84 / SCAR IMW SR15-16
 <epsg3223> : WGS 84 / SCAR IMW SR17-18
 <epsg3224> : WGS 84 / SCAR IMW SR19-20
 <epsg3225> : WGS 84 / SCAR IMW SR27-28
 <epsg3226> : WGS 84 / SCAR IMW SR29-30
 <epsg3227> : WGS 84 / SCAR IMW SR31-32
 <epsg3228> : WGS 84 / SCAR IMW SR33-34
 <epsg3229> : WGS 84 / SCAR IMW SR35-36
 <epsg3230> : WGS 84 / SCAR IMW SR37-38
 <epsg3231> : WGS 84 / SCAR IMW SR39-40
 <epsg3232> : WGS 84 / SCAR IMW SR41-42
 <epsg3233> : WGS 84 / SCAR IMW SR43-44
 <epsg3234> : WGS 84 / SCAR IMW SR45-46
 <epsg3235> : WGS 84 / SCAR IMW SR47-48
 <epsg3236> : WGS 84 / SCAR IMW SR49-50
 <epsg3237> : WGS 84 / SCAR IMW SR51-52
 <epsg3238> : WGS 84 / SCAR IMW SR53-54
 <epsg3239> : WGS 84 / SCAR IMW SR55-56
 <epsg3240> : WGS 84 / SCAR IMW SR57-58
 <epsg3241> : WGS 84 / SCAR IMW SR59-60
 <epsg3242> : WGS 84 / SCAR IMW SS04-06
 <epsg3243> : WGS 84 / SCAR IMW SS07-09
 <epsg3244> : WGS 84 / SCAR IMW SS10-12
 <epsg3245> : WGS 84 / SCAR IMW SS13-15
 <epsg3246> : WGS 84 / SCAR IMW SS16-18
 <epsg3247> : WGS 84 / SCAR IMW SS19-21
 <epsg3248> : WGS 84 / SCAR IMW SS25-27
 <epsg3249> : WGS 84 / SCAR IMW SS28-30
 <epsg3250> : WGS 84 / SCAR IMW SS31-33
 <epsg3251> : WGS 84 / SCAR IMW SS34-36
 <epsg3252> : WGS 84 / SCAR IMW SS37-39
 <epsg3253> : WGS 84 / SCAR IMW SS40-42
 <epsg3254> : WGS 84 / SCAR IMW SS43-45
 <epsg3255> : WGS 84 / SCAR IMW SS46-48
 <epsg3256> : WGS 84 / SCAR IMW SS49-51
 <epsg3257> : WGS 84 / SCAR IMW SS52-54
 <epsg3258> : WGS 84 / SCAR IMW SS55-57
 <epsg3259> : WGS 84 / SCAR IMW SS58-60
 <epsg3260> : WGS 84 / SCAR IMW ST01-04
 <epsg3261> : WGS 84 / SCAR IMW ST05-08
 <epsg3262> : WGS 84 / SCAR IMW ST09-12
 <epsg3263> : WGS 84 / SCAR IMW ST13-16
 <epsg3264> : WGS 84 / SCAR IMW ST17-20
 <epsg3265> : WGS 84 / SCAR IMW ST21-24
 <epsg3266> : WGS 84 / SCAR IMW ST25-28
 <epsg3267> : WGS 84 / SCAR IMW ST29-32
 <epsg3268> : WGS 84 / SCAR IMW ST33-36
 <epsg3269> : WGS 84 / SCAR IMW ST37-40
 <epsg3270> : WGS 84 / SCAR IMW ST41-44
 <epsg3271> : WGS 84 / SCAR IMW ST45-48
 <epsg3272> : WGS 84 / SCAR IMW ST49-52
 <epsg3273> : WGS 84 / SCAR IMW ST53-56
 <epsg3274> : WGS 84 / SCAR IMW ST57-60
 <epsg3275> : WGS 84 / SCAR IMW SU01-05
 <epsg3276> : WGS 84 / SCAR IMW SU06-10
 <epsg3277> : WGS 84 / SCAR IMW SU11-15
 <epsg3278> : WGS 84 / SCAR IMW SU16-20
 <epsg3279> : WGS 84 / SCAR IMW SU21-25
 <epsg3280> : WGS 84 / SCAR IMW SU26-30
 <epsg3281> : WGS 84 / SCAR IMW SU31-35
 <epsg3282> : WGS 84 / SCAR IMW SU36-40
 <epsg3283> : WGS 84 / SCAR IMW SU41-45
 <epsg3284> : WGS 84 / SCAR IMW SU46-50
 <epsg3285> : WGS 84 / SCAR IMW SU51-55
 <epsg3286> : WGS 84 / SCAR IMW SU56-60
 <epsg3287> : WGS 84 / SCAR IMW SV01-10
 <epsg3288> : WGS 84 / SCAR IMW SV11-20
 <epsg3289> : WGS 84 / SCAR IMW SV21-30
 <epsg3290> : WGS 84 / SCAR IMW SV31-40
 <epsg3291> : WGS 84 / SCAR IMW SV41-50
 <epsg3292> : WGS 84 / SCAR IMW SV51-60
 <epsg3293> : WGS 84 / SCAR IMW SW01-60
 <epsg3294> : WGS 84 / USGS Transantarctic Mountains
 <epsg3296> : RGPF / UTM zone 5S
 <epsg3297> : RGPF / UTM zone 6S
 <epsg3298> : RGPF / UTM zone 7S
 <epsg3299> : RGPF / UTM zone 8S
 <epsg3300> : Estonian Coordinate System of 1992
 <epsg3301> : Estonian Coordinate System of 1997
 <epsg3302> : IGN63 Hiva Oa / UTM zone 7S
 <epsg3303> : Fatu Iva 72 / UTM zone 7S
 <epsg3304> : Tahiti 79 / UTM zone 6S
 <epsg3305> : Moorea 87 / UTM zone 6S
 <epsg3306> : Maupiti 83 / UTM zone 5S
 <epsg3307> : Nakhl-e Ghanem / UTM zone 39N
 <epsg3308> : GDA94 / NSW Lambert
 <epsg3309> : NAD27 / California Albers
 <epsg3310> : NAD83 / California Albers
 <epsg3311> : NAD83(HARN) / California Albers
 <epsg3312> : CSG67 / UTM zone 21N
 <epsg3313> : RGFG95 / UTM zone 21N
 <epsg3314> : Katanga 1955 / Katanga Lambert
 <epsg3315> : Katanga 1955 / Katanga TM
 <epsg3316> : Kasai 1953 / Congo TM zone 22
 <epsg3317> : Kasai 1953 / Congo TM zone 24
 <epsg3318> : IGC 1962 / Congo TM zone 12
 <epsg3319> : IGC 1962 / Congo TM zone 14
 <epsg3320> : IGC 1962 / Congo TM zone 16
 <epsg3321> : IGC 1962 / Congo TM zone 18
 <epsg3322> : IGC 1962 / Congo TM zone 20
 <epsg3323> : IGC 1962 / Congo TM zone 22
 <epsg3324> : IGC 1962 / Congo TM zone 24
 <epsg3325> : IGC 1962 / Congo TM zone 26
 <epsg3326> : IGC 1962 / Congo TM zone 28
 <epsg3327> : IGC 1962 / Congo TM zone 30
 <epsg3328> : Pulkovo 1942(58) / GUGiK-80
 <epsg3329> : Pulkovo 1942(58) / 3-degree Gauss-Kruger zone 5
 <epsg3330> : Pulkovo 1942(58) / 3-degree Gauss-Kruger zone 6
 <epsg3331> : Pulkovo 1942(58) / 3-degree Gauss-Kruger zone 7
 <epsg3332> : Pulkovo 1942(58) / 3-degree Gauss-Kruger zone 8
 <epsg3333> : Pulkovo 1942(58) / Gauss-Kruger zone 3
 <epsg3334> : Pulkovo 1942(58) / Gauss-Kruger zone 4
 <epsg3335> : Pulkovo 1942(58) / Gauss-Kruger zone 5
 <epsg3336> : IGN 1962 Kerguelen / UTM zone 42S
 <epsg3337> : Le Pouce 1934 / Mauritius Grid
 <epsg3338> : NAD83 / Alaska Albers
 <epsg3339> : IGCB 1955 / Congo TM zone 12
 <epsg3340> : IGCB 1955 / Congo TM zone 14
 <epsg3341> : IGCB 1955 / Congo TM zone 16
 <epsg3342> : IGCB 1955 / UTM zone 33S
 <epsg3343> : Mauritania 1999 / UTM zone 28N
 <epsg3344> : Mauritania 1999 / UTM zone 29N
 <epsg3345> : Mauritania 1999 / UTM zone 30N
 <epsg3346> : LKS94 / Lithuania TM
 <epsg3347> : NAD83 / Statistics Canada Lambert
 <epsg3348> : NAD83(CSRS) / Statistics Canada Lambert
 <epsg3349> : WGS 84 / PDC Mercator
 <epsg3350> : Pulkovo 1942 / CS63 zone C0
 <epsg3351> : Pulkovo 1942 / CS63 zone C1
 <epsg3352> : Pulkovo 1942 / CS63 zone C2
 <epsg3353> : Mhast (onshore) / UTM zone 32S
 <epsg3354> : Mhast (offshore) / UTM zone 32S
 <epsg3355> : Egypt Gulf of Suez S-650 TL / Red Belt
 <epsg3356> : Grand Cayman 1959 / UTM zone 17N
 <epsg3357> : Little Cayman 1961 / UTM zone 17N
 <epsg3358> : NAD83(HARN) / North Carolina
 <epsg3359> : NAD83(HARN) / North Carolina (ftUS) (deprecated)
 <epsg3360> : NAD83(HARN) / South Carolina
 <epsg3361> : NAD83(HARN) / South Carolina (ft)
 <epsg3362> : NAD83(HARN) / Pennsylvania North
 <epsg3363> : NAD83(HARN) / Pennsylvania North (ftUS)
 <epsg3364> : NAD83(HARN) / Pennsylvania South
 <epsg3365> : NAD83(HARN) / Pennsylvania South (ftUS)
 <epsg3366> : Hong Kong 1963 Grid System (deprecated)
 <epsg3367> : IGN Astro 1960 / UTM zone 28N
 <epsg3368> : IGN Astro 1960 / UTM zone 29N
 <epsg3369> : IGN Astro 1960 / UTM zone 30N
 <epsg3370> : NAD27 / UTM zone 59N
 <epsg3371> : NAD27 / UTM zone 60N
 <epsg3372> : NAD83 / UTM zone 59N
 <epsg3373> : NAD83 / UTM zone 60N
 <epsg3374> : FD54 / UTM zone 29N
 <epsg3375> : GDM2000 / Peninsula RSO
 <epsg3376> : GDM2000 / East Malaysia BRSO
 <epsg3377> : GDM2000 / Johor Grid
 <epsg3378> : GDM2000 / Sembilan and Melaka Grid
 <epsg3379> : GDM2000 / PahangGrid
 <epsg3380> : GDM2000 / Selangor Grid
 <epsg3381> : GDM2000 / Terengganu Grid
 <epsg3382> : GDM2000 / Pinang Grid
 <epsg3383> : GDM2000 / Kedah and Perlis Grid
 <epsg3384> : GDM2000 / Perak Grid
 <epsg3385> : GDM2000 / Kelantan Grid
 <epsg3386> : KKJ / Finland zone 0
 <epsg3387> : KKJ / Finland zone 5
 <epsg3388> : Pulkovo 1942 / Caspian Sea Mercator
 <epsg3389> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 60
 <epsg3390> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 60
 <epsg3391> : Karbala 1979 (Polservice) / UTM zone 37N
 <epsg3392> : Karbala 1979 (Polservice) / UTM zone 38N
 <epsg3393> : Karbala 1979 (Polservice) / UTM zone 39N
 <epsg3394> : Nahrwan 1934 / Iraq zone
 <epsg3395> : WGS 84 / World Mercator
 <epsg3396> : PD/83 / Gauss-Kruger zone 3
 <epsg3397> : PD/83 / Gauss-Kruger zone 4
 <epsg3398> : RD/83 / Gauss-Kruger zone 4
 <epsg3399> : RD/83 / Gauss-Kruger zone 5
 <epsg3400> : NAD83 / Alberta 10-TM (Forest)
 <epsg3401> : NAD83 / Alberta 10-TM (Resource)
 <epsg3402> : NAD83(CSRS) / Alberta 10-TM (Forest)
 <epsg3403> : NAD83(CSRS) / Alberta 10-TM (Resource)
 <epsg3404> : NAD83(HARN) / North Carolina (ftUS)
 <epsg3405> : VN-2000 / UTM zone 48N
 <epsg3406> : VN-2000 / UTM zone 49N
 <epsg3407> : Hong Kong 1963 Grid System
 <epsg3408> : NSIDC EASE-Grid North
 <epsg3409> : NSIDC EASE-Grid South
 <epsg3411> : NSIDC Sea Ice Polar Stereographic North
 <epsg3412> : NSIDC Sea Ice Polar Stereographic South
 <epsg3413> : WGS 84 / NSIDC Sea Ice Polar Stereographic North
 <epsg3414> : SVY21 / Singapore TM
 <epsg3415> : WGS 72BE / South China Sea Lambert
 <epsg3416> : ETRS89 / Austria Lambert
 <epsg3417> : NAD83 / Iowa North (ft US)
 <epsg3418> : NAD83 / Iowa South (ft US)
 <epsg3419> : NAD83 / Kansas North (ft US)
 <epsg3420> : NAD83 / Kansas South (ft US)
 <epsg3421> : NAD83 / Nevada East (ft US)
 <epsg3422> : NAD83 / Nevada Central (ft US)
 <epsg3423> : NAD83 / Nevada West (ft US)
 <epsg3424> : NAD83 / New Jersey (ft US)
 <epsg3425> : NAD83(HARN) / Iowa North (ft US)
 <epsg3426> : NAD83(HARN) / Iowa South (ft US)
 <epsg3427> : NAD83(HARN) / Kansas North (ft US)
 <epsg3428> : NAD83(HARN) / Kansas South (ft US)
 <epsg3429> : NAD83(HARN) / Nevada East (ft US)
 <epsg3430> : NAD83(HARN) / Nevada Central (ft US)
 <epsg3431> : NAD83(HARN) / Nevada West (ft US)
 <epsg3432> : NAD83(HARN) / New Jersey (ft US)
 <epsg3433> : NAD83 / Arkansas North (ftUS)
 <epsg3434> : NAD83 / Arkansas South (ftUS)
 <epsg3435> : NAD83 / Illinois East (ftUS)
 <epsg3436> : NAD83 / Illinois West (ftUS)
 <epsg3437> : NAD83 / New Hampshire (ftUS)
 <epsg3438> : NAD83 / Rhode Island (ftUS)
 <epsg3439> : PSD93 / UTM zone 39N
 <epsg3440> : PSD93 / UTM zone 40N
 <epsg3441> : NAD83(HARN) / Arkansas North (ftUS)
 <epsg3442> : NAD83(HARN) / Arkansas South (ftUS)
 <epsg3443> : NAD83(HARN) / Illinois East (ftUS)
 <epsg3444> : NAD83(HARN) / Illinois West (ftUS)
 <epsg3445> : NAD83(HARN) / New Hampshire (ftUS)
 <epsg3446> : NAD83(HARN) / Rhode Island (ftUS)
 <epsg3447> : ETRS89 / Belgian Lambert 2005
 <epsg3448> : JAD2001 / Jamaica Metric Grid
 <epsg3449> : JAD2001 / UTM zone 17N
 <epsg3450> : JAD2001 / UTM zone 18N
 <epsg3451> : NAD83 / Louisiana North (ftUS)
 <epsg3452> : NAD83 / Louisiana South (ftUS)
 <epsg3453> : NAD83 / Louisiana Offshore (ftUS)
 <epsg3454> : NAD83 / South Dakota North (ftUS)
 <epsg3455> : NAD83 / South Dakota South (ftUS)
 <epsg3456> : NAD83(HARN) / Louisiana North (ftUS)
 <epsg3457> : NAD83(HARN) / Louisiana South (ftUS)
 <epsg3458> : NAD83(HARN) / South Dakota North (ftUS)
 <epsg3459> : NAD83(HARN) / South Dakota South (ftUS)
 <epsg3460> : Fiji 1986 / Fiji Map Grid
 <epsg3461> : Dabola 1981 / UTM zone 28N
 <epsg3462> : Dabola 1981 / UTM zone 29N
 <epsg3463> : NAD83 / Maine CS2000 Central
 <epsg3464> : NAD83(HARN) / Maine CS2000 Central
 <epsg3465> : NAD83(NSRS2007) / Alabama East
 <epsg3466> : NAD83(NSRS2007) / Alabama West
 <epsg3467> : NAD83(NSRS2007) / Alaska Albers
 <epsg3468> : NAD83(NSRS2007) / Alaska zone 1
 <epsg3469> : NAD83(NSRS2007) / Alaska zone 2
 <epsg3470> : NAD83(NSRS2007) / Alaska zone 3
 <epsg3471> : NAD83(NSRS2007) / Alaska zone 4
 <epsg3472> : NAD83(NSRS2007) / Alaska zone 5
 <epsg3473> : NAD83(NSRS2007) / Alaska zone 6
 <epsg3474> : NAD83(NSRS2007) / Alaska zone 7
 <epsg3475> : NAD83(NSRS2007) / Alaska zone 8
 <epsg3476> : NAD83(NSRS2007) / Alaska zone 9
 <epsg3477> : NAD83(NSRS2007) / Alaska zone 10
 <epsg3478> : NAD83(NSRS2007) / Arizona Central
 <epsg3479> : NAD83(NSRS2007) / Arizona Central (ft)
 <epsg3480> : NAD83(NSRS2007) / Arizona East
 <epsg3481> : NAD83(NSRS2007) / Arizona East (ft)
 <epsg3482> : NAD83(NSRS2007) / Arizona West
 <epsg3483> : NAD83(NSRS2007) / Arizona West (ft)
 <epsg3484> : NAD83(NSRS2007) / Arkansas North
 <epsg3485> : NAD83(NSRS2007) / Arkansas North (ftUS)
 <epsg3486> : NAD83(NSRS2007) / Arkansas South
 <epsg3487> : NAD83(NSRS2007) / Arkansas South (ftUS)
 <epsg3488> : NAD83(NSRS2007) / California Albers
 <epsg3489> : NAD83(NSRS2007) / California zone 1
 <epsg3490> : NAD83(NSRS2007) / California zone 1 (ftUS)
 <epsg3491> : NAD83(NSRS2007) / California zone 2
 <epsg3492> : NAD83(NSRS2007) / California zone 2 (ftUS)
 <epsg3493> : NAD83(NSRS2007) / California zone 3
 <epsg3494> : NAD83(NSRS2007) / California zone 3 (ftUS)
 <epsg3495> : NAD83(NSRS2007) / California zone 4
 <epsg3496> : NAD83(NSRS2007) / California zone 4 (ftUS)
 <epsg3497> : NAD83(NSRS2007) / California zone 5
 <epsg3498> : NAD83(NSRS2007) / California zone 5 (ftUS)
 <epsg3499> : NAD83(NSRS2007) / California zone 6
 <epsg3500> : NAD83(NSRS2007) / California zone 6 (ftUS)
 <epsg3501> : NAD83(NSRS2007) / Colorado Central
 <epsg3502> : NAD83(NSRS2007) / Colorado Central (ftUS)
 <epsg3503> : NAD83(NSRS2007) / Colorado North
 <epsg3504> : NAD83(NSRS2007) / Colorado North (ftUS)
 <epsg3505> : NAD83(NSRS2007) / Colorado South
 <epsg3506> : NAD83(NSRS2007) / Colorado South (ftUS)
 <epsg3507> : NAD83(NSRS2007) / Connecticut
 <epsg3508> : NAD83(NSRS2007) / Connecticut (ftUS)
 <epsg3509> : NAD83(NSRS2007) / Delaware
 <epsg3510> : NAD83(NSRS2007) / Delaware (ftUS)
 <epsg3511> : NAD83(NSRS2007) / Florida East
 <epsg3512> : NAD83(NSRS2007) / Florida East (ftUS)
 <epsg3513> : NAD83(NSRS2007) / Florida GDL Albers
 <epsg3514> : NAD83(NSRS2007) / Florida North
 <epsg3515> : NAD83(NSRS2007) / Florida North (ftUS)
 <epsg3516> : NAD83(NSRS2007) / Florida West
 <epsg3517> : NAD83(NSRS2007) / Florida West (ftUS)
 <epsg3518> : NAD83(NSRS2007) / Georgia East
 <epsg3519> : NAD83(NSRS2007) / Georgia East (ftUS)
 <epsg3520> : NAD83(NSRS2007) / Georgia West
 <epsg3521> : NAD83(NSRS2007) / Georgia West (ftUS)
 <epsg3522> : NAD83(NSRS2007) / Idaho Central
 <epsg3523> : NAD83(NSRS2007) / Idaho Central (ftUS)
 <epsg3524> : NAD83(NSRS2007) / Idaho East
 <epsg3525> : NAD83(NSRS2007) / Idaho East (ftUS)
 <epsg3526> : NAD83(NSRS2007) / Idaho West
 <epsg3527> : NAD83(NSRS2007) / Idaho West (ftUS)
 <epsg3528> : NAD83(NSRS2007) / Illinois East
 <epsg3529> : NAD83(NSRS2007) / Illinois East (ftUS)
 <epsg3530> : NAD83(NSRS2007) / Illinois West
 <epsg3531> : NAD83(NSRS2007) / Illinois West (ftUS)
 <epsg3532> : NAD83(NSRS2007) / Indiana East
 <epsg3533> : NAD83(NSRS2007) / Indiana East (ftUS)
 <epsg3534> : NAD83(NSRS2007) / Indiana West
 <epsg3535> : NAD83(NSRS2007) / Indiana West (ftUS)
 <epsg3536> : NAD83(NSRS2007) / Iowa North
 <epsg3537> : NAD83(NSRS2007) / Iowa North (ft US)
 <epsg3538> : NAD83(NSRS2007) / Iowa South
 <epsg3539> : NAD83(NSRS2007) / Iowa South (ft US)
 <epsg3540> : NAD83(NSRS2007) / Kansas North
 <epsg3541> : NAD83(NSRS2007) / Kansas North (ft US)
 <epsg3542> : NAD83(NSRS2007) / Kansas South
 <epsg3543> : NAD83(NSRS2007) / Kansas South (ft US)
 <epsg3544> : NAD83(NSRS2007) / Kentucky North
 <epsg3545> : NAD83(NSRS2007) / Kentucky North (ftUS)
 <epsg3546> : NAD83(NSRS2007) / Kentucky Single Zone
 <epsg3547> : NAD83(NSRS2007) / Kentucky Single Zone (ftUS)
 <epsg3548> : NAD83(NSRS2007) / Kentucky South
 <epsg3549> : NAD83(NSRS2007) / Kentucky South (ftUS)
 <epsg3550> : NAD83(NSRS2007) / Louisiana North
 <epsg3551> : NAD83(NSRS2007) / Louisiana North (ftUS)
 <epsg3552> : NAD83(NSRS2007) / Louisiana South
 <epsg3553> : NAD83(NSRS2007) / Louisiana South (ftUS)
 <epsg3554> : NAD83(NSRS2007) / Maine CS2000 Central
 <epsg3555> : NAD83(NSRS2007) / Maine CS2000 East
 <epsg3556> : NAD83(NSRS2007) / Maine CS2000 West
 <epsg3557> : NAD83(NSRS2007) / Maine East
 <epsg3558> : NAD83(NSRS2007) / Maine West
 <epsg3559> : NAD83(NSRS2007) / Maryland
 <epsg3560> : NAD83 / Utah North (ftUS)
 <epsg3561> : Old Hawaiian / Hawaii zone 1
 <epsg3562> : Old Hawaiian / Hawaii zone 2
 <epsg3563> : Old Hawaiian / Hawaii zone 3
 <epsg3564> : Old Hawaiian / Hawaii zone 4
 <epsg3565> : Old Hawaiian / Hawaii zone 5
 <epsg3566> : NAD83 / Utah Central (ftUS)
 <epsg3567> : NAD83 / Utah South (ftUS)
 <epsg3568> : NAD83(HARN) / Utah North (ftUS)
 <epsg3569> : NAD83(HARN) / Utah Central (ftUS)
 <epsg3570> : NAD83(HARN) / Utah South (ftUS)
 <epsg3571> : WGS 84 / North Pole LAEA Bering Sea
 <epsg3572> : WGS 84 / North Pole LAEA Alaska
 <epsg3573> : WGS 84 / North Pole LAEA Canada
 <epsg3574> : WGS 84 / North Pole LAEA Atlantic
 <epsg3575> : WGS 84 / North Pole LAEA Europe
 <epsg3576> : WGS 84 / North Pole LAEA Russia
 <epsg3577> : GDA94 / Australian Albers
 <epsg3578> : NAD83 / Yukon Albers
 <epsg3579> : NAD83(CSRS) / Yukon Albers
 <epsg3580> : NAD83 / NWT Lambert
 <epsg3581> : NAD83(CSRS) / NWT Lambert
 <epsg3582> : NAD83(NSRS2007) / Maryland (ftUS)
 <epsg3583> : NAD83(NSRS2007) / Massachusetts Island
 <epsg3584> : NAD83(NSRS2007) / Massachusetts Island (ftUS)
 <epsg3585> : NAD83(NSRS2007) / Massachusetts Mainland
 <epsg3586> : NAD83(NSRS2007) / Massachusetts Mainland (ftUS)
 <epsg3587> : NAD83(NSRS2007) / Michigan Central
 <epsg3588> : NAD83(NSRS2007) / Michigan Central (ft)
 <epsg3589> : NAD83(NSRS2007) / Michigan North
 <epsg3590> : NAD83(NSRS2007) / Michigan North (ft)
 <epsg3591> : NAD83(NSRS2007) / Michigan Oblique Mercator
 <epsg3592> : NAD83(NSRS2007) / Michigan South
 <epsg3593> : NAD83(NSRS2007) / Michigan South (ft)
 <epsg3594> : NAD83(NSRS2007) / Minnesota Central
 <epsg3595> : NAD83(NSRS2007) / Minnesota North
 <epsg3596> : NAD83(NSRS2007) / Minnesota South
 <epsg3597> : NAD83(NSRS2007) / Mississippi East
 <epsg3598> : NAD83(NSRS2007) / Mississippi East (ftUS)
 <epsg3599> : NAD83(NSRS2007) / Mississippi West
 <epsg3600> : NAD83(NSRS2007) / Mississippi West (ftUS)
 <epsg3601> : NAD83(NSRS2007) / Missouri Central
 <epsg3602> : NAD83(NSRS2007) / Missouri East
 <epsg3603> : NAD83(NSRS2007) / Missouri West
 <epsg3604> : NAD83(NSRS2007) / Montana
 <epsg3605> : NAD83(NSRS2007) / Montana (ft)
 <epsg3606> : NAD83(NSRS2007) / Nebraska
 <epsg3607> : NAD83(NSRS2007) / Nevada Central
 <epsg3608> : NAD83(NSRS2007) / Nevada Central (ft US)
 <epsg3609> : NAD83(NSRS2007) / Nevada East
 <epsg3610> : NAD83(NSRS2007) / Nevada East (ft US)
 <epsg3611> : NAD83(NSRS2007) / Nevada West
 <epsg3612> : NAD83(NSRS2007) / Nevada West (ft US)
 <epsg3613> : NAD83(NSRS2007) / New Hampshire
 <epsg3614> : NAD83(NSRS2007) / New Hampshire (ftUS)
 <epsg3615> : NAD83(NSRS2007) / New Jersey
 <epsg3616> : NAD83(NSRS2007) / New Jersey (ft US)
 <epsg3617> : NAD83(NSRS2007) / New Mexico Central
 <epsg3618> : NAD83(NSRS2007) / New Mexico Central (ftUS)
 <epsg3619> : NAD83(NSRS2007) / New Mexico East
 <epsg3620> : NAD83(NSRS2007) / New Mexico East (ftUS)
 <epsg3621> : NAD83(NSRS2007) / New Mexico West
 <epsg3622> : NAD83(NSRS2007) / New Mexico West (ftUS)
 <epsg3623> : NAD83(NSRS2007) / New York Central
 <epsg3624> : NAD83(NSRS2007) / New York Central (ftUS)
 <epsg3625> : NAD83(NSRS2007) / New York East
 <epsg3626> : NAD83(NSRS2007) / New York East (ftUS)
 <epsg3627> : NAD83(NSRS2007) / New York Long Island
 <epsg3628> : NAD83(NSRS2007) / New York Long Island (ftUS)
 <epsg3629> : NAD83(NSRS2007) / New York West
 <epsg3630> : NAD83(NSRS2007) / New York West (ftUS)
 <epsg3631> : NAD83(NSRS2007) / North Carolina
 <epsg3632> : NAD83(NSRS2007) / North Carolina (ftUS)
 <epsg3633> : NAD83(NSRS2007) / North Dakota North
 <epsg3634> : NAD83(NSRS2007) / North Dakota North (ft)
 <epsg3635> : NAD83(NSRS2007) / North Dakota South
 <epsg3636> : NAD83(NSRS2007) / North Dakota South (ft)
 <epsg3637> : NAD83(NSRS2007) / Ohio North
 <epsg3638> : NAD83(NSRS2007) / Ohio South
 <epsg3639> : NAD83(NSRS2007) / Oklahoma North
 <epsg3640> : NAD83(NSRS2007) / Oklahoma North (ftUS)
 <epsg3641> : NAD83(NSRS2007) / Oklahoma South
 <epsg3642> : NAD83(NSRS2007) / Oklahoma South (ftUS)
 <epsg3643> : NAD83(NSRS2007) / Oregon Lambert
 <epsg3644> : NAD83(NSRS2007) / Oregon Lambert (ft)
 <epsg3645> : NAD83(NSRS2007) / Oregon North
 <epsg3646> : NAD83(NSRS2007) / Oregon North (ft)
 <epsg3647> : NAD83(NSRS2007) / Oregon South
 <epsg3648> : NAD83(NSRS2007) / Oregon South (ft)
 <epsg3649> : NAD83(NSRS2007) / Pennsylvania North
 <epsg3650> : NAD83(NSRS2007) / Pennsylvania North (ftUS)
 <epsg3651> : NAD83(NSRS2007) / Pennsylvania South
 <epsg3652> : NAD83(NSRS2007) / Pennsylvania South (ftUS)
 <epsg3653> : NAD83(NSRS2007) / Rhode Island
 <epsg3654> : NAD83(NSRS2007) / Rhode Island (ftUS)
 <epsg3655> : NAD83(NSRS2007) / South Carolina
 <epsg3656> : NAD83(NSRS2007) / South Carolina (ft)
 <epsg3657> : NAD83(NSRS2007) / South Dakota North
 <epsg3658> : NAD83(NSRS2007) / South Dakota North (ftUS)
 <epsg3659> : NAD83(NSRS2007) / South Dakota South
 <epsg3660> : NAD83(NSRS2007) / South Dakota South (ftUS)
 <epsg3661> : NAD83(NSRS2007) / Tennessee
 <epsg3662> : NAD83(NSRS2007) / Tennessee (ftUS)
 <epsg3663> : NAD83(NSRS2007) / Texas Central
 <epsg3664> : NAD83(NSRS2007) / Texas Central (ftUS)
 <epsg3665> : NAD83(NSRS2007) / Texas Centric Albers Equal Area
 <epsg3666> : NAD83(NSRS2007) / Texas Centric Lambert Conformal
 <epsg3667> : NAD83(NSRS2007) / Texas North
 <epsg3668> : NAD83(NSRS2007) / Texas North (ftUS)
 <epsg3669> : NAD83(NSRS2007) / Texas North Central
 <epsg3670> : NAD83(NSRS2007) / Texas North Central (ftUS)
 <epsg3671> : NAD83(NSRS2007) / Texas South
 <epsg3672> : NAD83(NSRS2007) / Texas South (ftUS)
 <epsg3673> : NAD83(NSRS2007) / Texas South Central
 <epsg3674> : NAD83(NSRS2007) / Texas South Central (ftUS)
 <epsg3675> : NAD83(NSRS2007) / Utah Central
 <epsg3676> : NAD83(NSRS2007) / Utah Central (ft)
 <epsg3677> : NAD83(NSRS2007) / Utah Central (ftUS)
 <epsg3678> : NAD83(NSRS2007) / Utah North
 <epsg3679> : NAD83(NSRS2007) / Utah North (ft)
 <epsg3680> : NAD83(NSRS2007) / Utah North (ftUS)
 <epsg3681> : NAD83(NSRS2007) / Utah South
 <epsg3682> : NAD83(NSRS2007) / Utah South (ft)
 <epsg3683> : NAD83(NSRS2007) / Utah South (ftUS)
 <epsg3684> : NAD83(NSRS2007) / Vermont
 <epsg3685> : NAD83(NSRS2007) / Virginia North
 <epsg3686> : NAD83(NSRS2007) / Virginia North (ftUS)
 <epsg3687> : NAD83(NSRS2007) / Virginia South
 <epsg3688> : NAD83(NSRS2007) / Virginia South (ftUS)
 <epsg3689> : NAD83(NSRS2007) / Washington North
 <epsg3690> : NAD83(NSRS2007) / Washington North (ftUS)
 <epsg3691> : NAD83(NSRS2007) / Washington South
 <epsg3692> : NAD83(NSRS2007) / Washington South (ftUS)
 <epsg3693> : NAD83(NSRS2007) / West Virginia North
 <epsg3694> : NAD83(NSRS2007) / West Virginia South
 <epsg3695> : NAD83(NSRS2007) / Wisconsin Central
 <epsg3696> : NAD83(NSRS2007) / Wisconsin Central (ftUS)
 <epsg3697> : NAD83(NSRS2007) / Wisconsin North
 <epsg3698> : NAD83(NSRS2007) / Wisconsin North (ftUS)
 <epsg3699> : NAD83(NSRS2007) / Wisconsin South
 <epsg3700> : NAD83(NSRS2007) / Wisconsin South (ftUS)
 <epsg3701> : NAD83(NSRS2007) / Wisconsin Transverse Mercator
 <epsg3702> : NAD83(NSRS2007) / Wyoming East
 <epsg3703> : NAD83(NSRS2007) / Wyoming East Central
 <epsg3704> : NAD83(NSRS2007) / Wyoming West Central
 <epsg3705> : NAD83(NSRS2007) / Wyoming West
 <epsg3706> : NAD83(NSRS2007) / UTM zone 59N
 <epsg3707> : NAD83(NSRS2007) / UTM zone 60N
 <epsg3708> : NAD83(NSRS2007) / UTM zone 1N
 <epsg3709> : NAD83(NSRS2007) / UTM zone 2N
 <epsg3710> : NAD83(NSRS2007) / UTM zone 3N
 <epsg3711> : NAD83(NSRS2007) / UTM zone 4N
 <epsg3712> : NAD83(NSRS2007) / UTM zone 5N
 <epsg3713> : NAD83(NSRS2007) / UTM zone 6N
 <epsg3714> : NAD83(NSRS2007) / UTM zone 7N
 <epsg3715> : NAD83(NSRS2007) / UTM zone 8N
 <epsg3716> : NAD83(NSRS2007) / UTM zone 9N
 <epsg3717> : NAD83(NSRS2007) / UTM zone 10N
 <epsg3718> : NAD83(NSRS2007) / UTM zone 11N
 <epsg3719> : NAD83(NSRS2007) / UTM zone 12N
 <epsg3720> : NAD83(NSRS2007) / UTM zone 13N
 <epsg3721> : NAD83(NSRS2007) / UTM zone 14N
 <epsg3722> : NAD83(NSRS2007) / UTM zone 15N
 <epsg3723> : NAD83(NSRS2007) / UTM zone 16N
 <epsg3724> : NAD83(NSRS2007) / UTM zone 17N
 <epsg3725> : NAD83(NSRS2007) / UTM zone 18N
 <epsg3726> : NAD83(NSRS2007) / UTM zone 19N
 <epsg3727> : Reunion 1947 / TM Reunion
 <epsg3728> : NAD83(NSRS2007) / Ohio North (ftUS)
 <epsg3729> : NAD83(NSRS2007) / Ohio South (ftUS)
 <epsg3730> : NAD83(NSRS2007) / Wyoming East (ftUS)
 <epsg3731> : NAD83(NSRS2007) / Wyoming East Central (ftUS)
 <epsg3732> : NAD83(NSRS2007) / Wyoming West Central (ftUS)
 <epsg3733> : NAD83(NSRS2007) / Wyoming West (ftUS)
 <epsg3734> : NAD83 / Ohio North (ftUS)
 <epsg3735> : NAD83 / Ohio South (ftUS)
 <epsg3736> : NAD83 / Wyoming East (ftUS)
 <epsg3737> : NAD83 / Wyoming East Central (ftUS)
 <epsg3738> : NAD83 / Wyoming West Central (ftUS)
 <epsg3739> : NAD83 / Wyoming West (ftUS)
 <epsg3740> : NAD83(HARN) / UTM zone 10N
 <epsg3741> : NAD83(HARN) / UTM zone 11N
 <epsg3742> : NAD83(HARN) / UTM zone 12N
 <epsg3743> : NAD83(HARN) / UTM zone 13N
 <epsg3744> : NAD83(HARN) / UTM zone 14N
 <epsg3745> : NAD83(HARN) / UTM zone 15N
 <epsg3746> : NAD83(HARN) / UTM zone 16N
 <epsg3747> : NAD83(HARN) / UTM zone 17N
 <epsg3748> : NAD83(HARN) / UTM zone 18N
 <epsg3749> : NAD83(HARN) / UTM zone 19N
 <epsg3750> : NAD83(HARN) / UTM zone 4N
 <epsg3751> : NAD83(HARN) / UTM zone 5N
 <epsg3752> : WGS 84 / Mercator 41
 <epsg3753> : NAD83(HARN) / Ohio North (ftUS)
 <epsg3754> : NAD83(HARN) / Ohio South (ftUS)
 <epsg3755> : NAD83(HARN) / Wyoming East (ftUS)
 <epsg3756> : NAD83(HARN) / Wyoming East Central (ftUS)
 <epsg3757> : NAD83(HARN) / Wyoming West Central (ftUS)
 <epsg3758> : NAD83(HARN) / Wyoming West (ftUS)
 <epsg3759> : NAD83 / Hawaii zone 3 (ftUS)
 <epsg3760> : NAD83(HARN) / Hawaii zone 3 (ftUS)
 <epsg3761> : NAD83(CSRS) / UTM zone 22N
 <epsg3762> : WGS 84 / South Georgia Lambert
 <epsg3920> : Puerto Rico / UTM zone 20N
 <epsg3991> : Puerto Rico State Plane CS of 1927
 <epsg3992> : Puerto Rico / St. Croix
 <epsg20004> : Pulkovo 1995 / Gauss-Kruger zone 4
 <epsg20005> : Pulkovo 1995 / Gauss-Kruger zone 5
 <epsg20006> : Pulkovo 1995 / Gauss-Kruger zone 6
 <epsg20007> : Pulkovo 1995 / Gauss-Kruger zone 7
 <epsg20008> : Pulkovo 1995 / Gauss-Kruger zone 8
 <epsg20009> : Pulkovo 1995 / Gauss-Kruger zone 9
 <epsg20010> : Pulkovo 1995 / Gauss-Kruger zone 10
 <epsg20011> : Pulkovo 1995 / Gauss-Kruger zone 11
 <epsg20012> : Pulkovo 1995 / Gauss-Kruger zone 12
 <epsg20013> : Pulkovo 1995 / Gauss-Kruger zone 13
 <epsg20014> : Pulkovo 1995 / Gauss-Kruger zone 14
 <epsg20015> : Pulkovo 1995 / Gauss-Kruger zone 15
 <epsg20016> : Pulkovo 1995 / Gauss-Kruger zone 16
 <epsg20017> : Pulkovo 1995 / Gauss-Kruger zone 17
 <epsg20018> : Pulkovo 1995 / Gauss-Kruger zone 18
 <epsg20019> : Pulkovo 1995 / Gauss-Kruger zone 19
 <epsg20020> : Pulkovo 1995 / Gauss-Kruger zone 20
 <epsg20021> : Pulkovo 1995 / Gauss-Kruger zone 21
 <epsg20022> : Pulkovo 1995 / Gauss-Kruger zone 22
 <epsg20023> : Pulkovo 1995 / Gauss-Kruger zone 23
 <epsg20024> : Pulkovo 1995 / Gauss-Kruger zone 24
 <epsg20025> : Pulkovo 1995 / Gauss-Kruger zone 25
 <epsg20026> : Pulkovo 1995 / Gauss-Kruger zone 26
 <epsg20027> : Pulkovo 1995 / Gauss-Kruger zone 27
 <epsg20028> : Pulkovo 1995 / Gauss-Kruger zone 28
 <epsg20029> : Pulkovo 1995 / Gauss-Kruger zone 29
 <epsg20030> : Pulkovo 1995 / Gauss-Kruger zone 30
 <epsg20031> : Pulkovo 1995 / Gauss-Kruger zone 31
 <epsg20032> : Pulkovo 1995 / Gauss-Kruger zone 32
 <epsg20064> : Pulkovo 1995 / Gauss-Kruger 4N (deprecated)
 <epsg20065> : Pulkovo 1995 / Gauss-Kruger 5N (deprecated)
 <epsg20066> : Pulkovo 1995 / Gauss-Kruger 6N (deprecated)
 <epsg20067> : Pulkovo 1995 / Gauss-Kruger 7N (deprecated)
 <epsg20068> : Pulkovo 1995 / Gauss-Kruger 8N (deprecated)
 <epsg20069> : Pulkovo 1995 / Gauss-Kruger 9N (deprecated)
 <epsg20070> : Pulkovo 1995 / Gauss-Kruger 10N (deprecated)
 <epsg20071> : Pulkovo 1995 / Gauss-Kruger 11N (deprecated)
 <epsg20072> : Pulkovo 1995 / Gauss-Kruger 12N (deprecated)
 <epsg20073> : Pulkovo 1995 / Gauss-Kruger 13N (deprecated)
 <epsg20074> : Pulkovo 1995 / Gauss-Kruger 14N (deprecated)
 <epsg20075> : Pulkovo 1995 / Gauss-Kruger 15N (deprecated)
 <epsg20076> : Pulkovo 1995 / Gauss-Kruger 16N (deprecated)
 <epsg20077> : Pulkovo 1995 / Gauss-Kruger 17N (deprecated)
 <epsg20078> : Pulkovo 1995 / Gauss-Kruger 18N (deprecated)
 <epsg20079> : Pulkovo 1995 / Gauss-Kruger 19N (deprecated)
 <epsg20080> : Pulkovo 1995 / Gauss-Kruger 20N (deprecated)
 <epsg20081> : Pulkovo 1995 / Gauss-Kruger 21N (deprecated)
 <epsg20082> : Pulkovo 1995 / Gauss-Kruger 22N (deprecated)
 <epsg20083> : Pulkovo 1995 / Gauss-Kruger 23N (deprecated)
 <epsg20084> : Pulkovo 1995 / Gauss-Kruger 24N (deprecated)
 <epsg20085> : Pulkovo 1995 / Gauss-Kruger 25N (deprecated)
 <epsg20086> : Pulkovo 1995 / Gauss-Kruger 26N (deprecated)
 <epsg20087> : Pulkovo 1995 / Gauss-Kruger 27N (deprecated)
 <epsg20088> : Pulkovo 1995 / Gauss-Kruger 28N (deprecated)
 <epsg20089> : Pulkovo 1995 / Gauss-Kruger 29N (deprecated)
 <epsg20090> : Pulkovo 1995 / Gauss-Kruger 30N (deprecated)
 <epsg20091> : Pulkovo 1995 / Gauss-Kruger 31N (deprecated)
 <epsg20092> : Pulkovo 1995 / Gauss-Kruger 32N (deprecated)
 <epsg20135> : Adindan / UTM zone 35N
 <epsg20136> : Adindan / UTM zone 36N
 <epsg20137> : Adindan / UTM zone 37N
 <epsg20138> : Adindan / UTM zone 38N
 <epsg20248> : AGD66 / AMG zone 48
 <epsg20249> : AGD66 / AMG zone 49
 <epsg20250> : AGD66 / AMG zone 50
 <epsg20251> : AGD66 / AMG zone 51
 <epsg20252> : AGD66 / AMG zone 52
 <epsg20253> : AGD66 / AMG zone 53
 <epsg20254> : AGD66 / AMG zone 54
 <epsg20255> : AGD66 / AMG zone 55
 <epsg20256> : AGD66 / AMG zone 56
 <epsg20257> : AGD66 / AMG zone 57
 <epsg20258> : AGD66 / AMG zone 58
 <epsg20348> : AGD84 / AMG zone 48
 <epsg20349> : AGD84 / AMG zone 49
 <epsg20350> : AGD84 / AMG zone 50
 <epsg20351> : AGD84 / AMG zone 51
 <epsg20352> : AGD84 / AMG zone 52
 <epsg20353> : AGD84 / AMG zone 53
 <epsg20354> : AGD84 / AMG zone 54
 <epsg20355> : AGD84 / AMG zone 55
 <epsg20356> : AGD84 / AMG zone 56
 <epsg20357> : AGD84 / AMG zone 57
 <epsg20358> : AGD84 / AMG zone 58
 <epsg20436> : Ain el Abd / UTM zone 36N
 <epsg20437> : Ain el Abd / UTM zone 37N
 <epsg20438> : Ain el Abd / UTM zone 38N
 <epsg20439> : Ain el Abd / UTM zone 39N
 <epsg20440> : Ain el Abd / UTM zone 40N
 <epsg20499> : Ain el Abd / Bahrain Grid
 <epsg20538> : Afgooye / UTM zone 38N
 <epsg20539> : Afgooye / UTM zone 39N
 <epsg20790> : Lisbon (Lisbon)/Portuguese National Grid
 <epsg20791> : Lisbon (Lisbon)/Portuguese Grid
 <epsg20822> : Aratu / UTM zone 22S
 <epsg20823> : Aratu / UTM zone 23S
 <epsg20824> : Aratu / UTM zone 24S
 <epsg20934> : Arc 1950 / UTM zone 34S
 <epsg20935> : Arc 1950 / UTM zone 35S
 <epsg20936> : Arc 1950 / UTM zone 36S
 <epsg21035> : Arc 1960 / UTM zone 35S
 <epsg21036> : Arc 1960 / UTM zone 36S
 <epsg21037> : Arc 1960 / UTM zone 37S
 <epsg21095> : Arc 1960 / UTM zone 35N
 <epsg21096> : Arc 1960 / UTM zone 36N
 <epsg21097> : Arc 1960 / UTM zone 37N
 <epsg21100> : Batavia (Jakarta) / NEIEZ (deprecated)
 <epsg21148> : Batavia / UTM zone 48S
 <epsg21149> : Batavia / UTM zone 49S
 <epsg21150> : Batavia / UTM zone 50S
 <epsg21291> : Barbados 1938 / British West Indies Grid
 <epsg21292> : Barbados 1938 / Barbados National Grid
 <epsg21413> : Beijing 1954 / Gauss-Kruger zone 13
 <epsg21414> : Beijing 1954 / Gauss-Kruger zone 14
 <epsg21415> : Beijing 1954 / Gauss-Kruger zone 15
 <epsg21416> : Beijing 1954 / Gauss-Kruger zone 16
 <epsg21417> : Beijing 1954 / Gauss-Kruger zone 17
 <epsg21418> : Beijing 1954 / Gauss-Kruger zone 18
 <epsg21419> : Beijing 1954 / Gauss-Kruger zone 19
 <epsg21420> : Beijing 1954 / Gauss-Kruger zone 20
 <epsg21421> : Beijing 1954 / Gauss-Kruger zone 21
 <epsg21422> : Beijing 1954 / Gauss-Kruger zone 22
 <epsg21423> : Beijing 1954 / Gauss-Kruger zone 23
 <epsg21453> : Beijing 1954 / Gauss-Kruger CM 75E
 <epsg21454> : Beijing 1954 / Gauss-Kruger CM 81E
 <epsg21455> : Beijing 1954 / Gauss-Kruger CM 87E
 <epsg21456> : Beijing 1954 / Gauss-Kruger CM 93E
 <epsg21457> : Beijing 1954 / Gauss-Kruger CM 99E
 <epsg21458> : Beijing 1954 / Gauss-Kruger CM 105E
 <epsg21459> : Beijing 1954 / Gauss-Kruger CM 111E
 <epsg21460> : Beijing 1954 / Gauss-Kruger CM 117E
 <epsg21461> : Beijing 1954 / Gauss-Kruger CM 123E
 <epsg21462> : Beijing 1954 / Gauss-Kruger CM 129E
 <epsg21463> : Beijing 1954 / Gauss-Kruger CM 135E
 <epsg21473> : Beijing 1954 / Gauss-Kruger 13N (deprecated)
 <epsg21474> : Beijing 1954 / Gauss-Kruger 14N (deprecated)
 <epsg21475> : Beijing 1954 / Gauss-Kruger 15N (deprecated)
 <epsg21476> : Beijing 1954 / Gauss-Kruger 16N (deprecated)
 <epsg21477> : Beijing 1954 / Gauss-Kruger 17N (deprecated)
 <epsg21478> : Beijing 1954 / Gauss-Kruger 18N (deprecated)
 <epsg21479> : Beijing 1954 / Gauss-Kruger 19N (deprecated)
 <epsg21480> : Beijing 1954 / Gauss-Kruger 20N (deprecated)
 <epsg21481> : Beijing 1954 / Gauss-Kruger 21N (deprecated)
 <epsg21482> : Beijing 1954 / Gauss-Kruger 22N (deprecated)
 <epsg21483> : Beijing 1954 / Gauss-Kruger 23N (deprecated)
 <epsg21500> : Belge 1950 (Brussels) / Belge Lambert 50
 <epsg21780> : Bern 1898 (Bern) / LV03C
 <epsg21781> : CH1903 / LV03
 <epsg21817> : Bogota 1975 / UTM zone 17N (deprecated)
 <epsg21818> : Bogota 1975 / UTM zone 18N
 <epsg21891> : Bogota 1975 / Colombia West zone (deprecated)
 <epsg21892> : Bogota 1975 / Colombia Bogota zone (deprecated)
 <epsg21893> : Bogota 1975 / Colombia East Central zone (deprecated)
 <epsg21894> : Bogota 1975 / Colombia East (deprecated)
 <epsg21896> : Bogota 1975 / Colombia West zone
 <epsg21897> : Bogota 1975 / Colombia Bogota zone
 <epsg21898> : Bogota 1975 / Colombia East Central zone
 <epsg21899> : Bogota 1975 / Colombia East
 <epsg22032> : Camacupa / UTM zone 32S
 <epsg22033> : Camacupa / UTM zone 33S
 <epsg22091> : Camacupa / TM 11.30 SE
 <epsg22092> : Camacupa / TM 12 SE
 <epsg22171> : POSGAR 98 / Argentina 1
 <epsg22172> : POSGAR 98 / Argentina 2
 <epsg22173> : POSGAR 98 / Argentina 3
 <epsg22174> : POSGAR 98 / Argentina 4
 <epsg22175> : POSGAR 98 / Argentina 5
 <epsg22176> : POSGAR 98 / Argentina 6
 <epsg22177> : POSGAR 98 / Argentina 7
 <epsg22181> : POSGAR 94 / Argentina 1
 <epsg22182> : POSGAR 94 / Argentina 2
 <epsg22183> : POSGAR 94 / Argentina 3
 <epsg22184> : POSGAR 94 / Argentina 4
 <epsg22185> : POSGAR 94 / Argentina 5
 <epsg22186> : POSGAR 94 / Argentina 6
 <epsg22187> : POSGAR 94 / Argentina 7
 <epsg22191> : Campo Inchauspe / Argentina 1
 <epsg22192> : Campo Inchauspe / Argentina 2
 <epsg22193> : Campo Inchauspe / Argentina 3
 <epsg22194> : Campo Inchauspe / Argentina 4
 <epsg22195> : Campo Inchauspe / Argentina 5
 <epsg22196> : Campo Inchauspe / Argentina 6
 <epsg22197> : Campo Inchauspe / Argentina 7
 <epsg22234> : Cape / UTM zone 34S
 <epsg22235> : Cape / UTM zone 35S
 <epsg22236> : Cape / UTM zone 36S
 <epsg22332> : Carthage / UTM zone 32N
 <epsg22391> : Carthage / Nord Tunisie
 <epsg22392> : Carthage / Sud Tunisie
 <epsg22521> : Corrego Alegre / UTM zone 21S
 <epsg22522> : Corrego Alegre / UTM zone 22S
 <epsg22523> : Corrego Alegre / UTM zone 23S
 <epsg22524> : Corrego Alegre / UTM zone 24S
 <epsg22525> : Corrego Alegre / UTM zone 25S
 <epsg22700> : Deir ez Zor / Levant Zone
 <epsg22770> : Deir ez Zor / Syria Lambert
 <epsg22780> : Deir ez Zor / Levant Stereographic
 <epsg22832> : Douala / UTM zone 32N (deprecated)
 <epsg22991> : Egypt 1907 / Blue Belt
 <epsg22992> : Egypt 1907 / Red Belt
 <epsg22993> : Egypt 1907 / Purple Belt
 <epsg22994> : Egypt 1907 / Extended Purple Belt
 <epsg23028> : ED50 / UTM zone 28N
 <epsg23029> : ED50 / UTM zone 29N
 <epsg23030> : ED50 / UTM zone 30N
 <epsg23031> : ED50 / UTM zone 31N
 <epsg23032> : ED50 / UTM zone 32N
 <epsg23033> : ED50 / UTM zone 33N
 <epsg23034> : ED50 / UTM zone 34N
 <epsg23035> : ED50 / UTM zone 35N
 <epsg23036> : ED50 / UTM zone 36N
 <epsg23037> : ED50 / UTM zone 37N
 <epsg23038> : ED50 / UTM zone 38N
 <epsg23090> : ED50 / TM 0 N
 <epsg23095> : ED50 / TM 5 NE
 <epsg23239> : Fahud / UTM zone 39N
 <epsg23240> : Fahud / UTM zone 40N
 <epsg23433> : Garoua / UTM zone 33N (deprecated)
 <epsg23700> : HD72 / EOV
 <epsg23830> : DGN95 / Indonesia TM-3 zone 46.2
 <epsg23831> : DGN95 / Indonesia TM-3 zone 47.1
 <epsg23832> : DGN95 / Indonesia TM-3 zone 47.2
 <epsg23833> : DGN95 / Indonesia TM-3 zone 48.1
 <epsg23834> : DGN95 / Indonesia TM-3 zone 48.2
 <epsg23835> : DGN95 / Indonesia TM-3 zone 49.1
 <epsg23836> : DGN95 / Indonesia TM-3 zone 49.2
 <epsg23837> : DGN95 / Indonesia TM-3 zone 50.1
 <epsg23838> : DGN95 / Indonesia TM-3 zone 50.2
 <epsg23839> : DGN95 / Indonesia TM-3 zone 51.1
 <epsg23840> : DGN95 / Indonesia TM-3 zone 51.2
 <epsg23841> : DGN95 / Indonesia TM-3 zone 52.1
 <epsg23842> : DGN95 / Indonesia TM-3 zone 52.2
 <epsg23843> : DGN95 / Indonesia TM-3 zone 53.1
 <epsg23844> : DGN95 / Indonesia TM-3 zone 53.2
 <epsg23845> : DGN95 / Indonesia TM-3 zone 54.1
 <epsg23846> : ID74 / UTM zone 46N
 <epsg23847> : ID74 / UTM zone 47N
 <epsg23848> : ID74 / UTM zone 48N
 <epsg23849> : ID74 / UTM zone 49N
 <epsg23850> : ID74 / UTM zone 50N
 <epsg23851> : ID74 / UTM zone 51N
 <epsg23852> : ID74 / UTM zone 52N
 <epsg23853> : ID74 / UTM zone 53N (deprecated)
 <epsg23866> : DGN95 / UTM zone 46N
 <epsg23867> : DGN95 / UTM zone 47N
 <epsg23868> : DGN95 / UTM zone 48N
 <epsg23869> : DGN95 / UTM zone 49N
 <epsg23870> : DGN95 / UTM zone 50N
 <epsg23871> : DGN95 / UTM zone 51N
 <epsg23872> : DGN95 / UTM zone 52N
 <epsg23877> : DGN95 / UTM zone 47S
 <epsg23878> : DGN95 / UTM zone 48S
 <epsg23879> : DGN95 / UTM zone 49S
 <epsg23880> : DGN95 / UTM zone 50S
 <epsg23881> : DGN95 / UTM zone 51S
 <epsg23882> : DGN95 / UTM zone 52S
 <epsg23883> : DGN95 / UTM zone 53S
 <epsg23884> : DGN95 / UTM zone 54S
 <epsg23886> : ID74 / UTM zone 46S (deprecated)
 <epsg23887> : ID74 / UTM zone 47S
 <epsg23888> : ID74 / UTM zone 48S
 <epsg23889> : ID74 / UTM zone 49S
 <epsg23890> : ID74 / UTM zone 50S
 <epsg23891> : ID74 / UTM zone 51S
 <epsg23892> : ID74 / UTM zone 52S
 <epsg23893> : ID74 / UTM zone 53S
 <epsg23894> : ID74 / UTM zone 54S
 <epsg23946> : Indian 1954 / UTM zone 46N
 <epsg23947> : Indian 1954 / UTM zone 47N
 <epsg23948> : Indian 1954 / UTM zone 48N
 <epsg24047> : Indian 1975 / UTM zone 47N
 <epsg24048> : Indian 1975 / UTM zone 48N
 <epsg24100> : Jamaica 1875 / Jamaica (Old Grid)
 <epsg24200> : JAD69 / Jamaica National Grid
 <epsg24305> : Kalianpur 1937 / UTM zone 45N
 <epsg24306> : Kalianpur 1937 / UTM zone 46N
 <epsg24311> : Kalianpur 1962 / UTM zone 41N
 <epsg24312> : Kalianpur 1962 / UTM zone 42N
 <epsg24313> : Kalianpur 1962 / UTM zone 43N
 <epsg24342> : Kalianpur 1975 / UTM zone 42N
 <epsg24343> : Kalianpur 1975 / UTM zone 43N
 <epsg24344> : Kalianpur 1975 / UTM zone 44N
 <epsg24345> : Kalianpur 1975 / UTM zone 45N
 <epsg24346> : Kalianpur 1975 / UTM zone 46N
 <epsg24347> : Kalianpur 1975 / UTM zone 47N
 <epsg24370> : Kalianpur 1880 / India zone 0
 <epsg24371> : Kalianpur 1880 / India zone I
 <epsg24372> : Kalianpur 1880 / India zone IIa
 <epsg24373> : Kalianpur 1880 / India zone III
 <epsg24374> : Kalianpur 1880 / India zone IV
 <epsg24375> : Kalianpur 1937 / India zone IIb
 <epsg24376> : Kalianpur 1962 / India zone I
 <epsg24377> : Kalianpur 1962 / India zone IIa
 <epsg24378> : Kalianpur 1975 / India zone I
 <epsg24379> : Kalianpur 1975 / India zone IIa
 <epsg24380> : Kalianpur 1975 / India zone IIb
 <epsg24381> : Kalianpur 1975 / India zone III
 <epsg24382> : Kalianpur 1880 / India zone IIb
 <epsg24383> : Kalianpur 1975 / India zone IV
 <epsg24500> : Kertau 1968 / Singapore Grid
 <epsg24547> : Kertau 1968 / UTM zone 47N
 <epsg24548> : Kertau 1968 / UTM zone 48N
 <epsg24571> : Kertau / R.S.O. Malaya (ch) (deprecated)
 <epsg24600> : KOC Lambert
 <epsg24718> : La Canoa / UTM zone 18N
 <epsg24719> : La Canoa / UTM zone 19N
 <epsg24720> : La Canoa / UTM zone 20N
 <epsg24817> : PSAD56 / UTM zone 17N
 <epsg24818> : PSAD56 / UTM zone 18N
 <epsg24819> : PSAD56 / UTM zone 19N
 <epsg24820> : PSAD56 / UTM zone 20N
 <epsg24821> : PSAD56 / UTM zone 21N
 <epsg24877> : PSAD56 / UTM zone 17S
 <epsg24878> : PSAD56 / UTM zone 18S
 <epsg24879> : PSAD56 / UTM zone 19S
 <epsg24880> : PSAD56 / UTM zone 20S
 <epsg24881> : PSAD56 / UTM zone 21S
 <epsg24882> : PSAD56 / UTM zone 22S
 <epsg24891> : PSAD56 / Peru west zone
 <epsg24892> : PSAD56 / Peru central zone
 <epsg24893> : PSAD56 / Peru east zone
 <epsg25000> : Leigon / Ghana Metre Grid
 <epsg25231> : Lome / UTM zone 31N
 <epsg25391> : Luzon 1911 / Philippines zone I
 <epsg25392> : Luzon 1911 / Philippines zone II
 <epsg25393> : Luzon 1911 / Philippines zone III
 <epsg25394> : Luzon 1911 / Philippines zone IV
 <epsg25395> : Luzon 1911 / Philippines zone V
 <epsg25700> : Makassar (Jakarta) / NEIEZ (deprecated)
 <epsg25828> : ETRS89 / UTM zone 28N
 <epsg25829> : ETRS89 / UTM zone 29N
 <epsg25830> : ETRS89 / UTM zone 30N
 <epsg25831> : ETRS89 / UTM zone 31N
 <epsg25832> : ETRS89 / UTM zone 32N
 <epsg25833> : ETRS89 / UTM zone 33N
 <epsg25834> : ETRS89 / UTM zone 34N
 <epsg25835> : ETRS89 / UTM zone 35N
 <epsg25836> : ETRS89 / UTM zone 36N
 <epsg25837> : ETRS89 / UTM zone 37N
 <epsg25838> : ETRS89 / UTM zone 38N
 <epsg25884> : ETRS89 / TM Baltic93
 <epsg25932> : Malongo 1987 / UTM zone 32S
 <epsg26191> : Merchich / Nord Maroc
 <epsg26192> : Merchich / Sud Maroc
 <epsg26193> : Merchich / Sahara (deprecated)
 <epsg26194> : Merchich / Sahara Nord
 <epsg26195> : Merchich / Sahara Sud
 <epsg26237> : Massawa / UTM zone 37N
 <epsg26331> : Minna / UTM zone 31N
 <epsg26332> : Minna / UTM zone 32N
 <epsg26391> : Minna / Nigeria West Belt
 <epsg26392> : Minna / Nigeria Mid Belt
 <epsg26393> : Minna / Nigeria East Belt
 <epsg26432> : Mhast / UTM zone 32S (deprecated)
 <epsg26591> : Monte Mario (Rome) / Italy zone 1 (deprecated)
 <epsg26592> : Monte Mario (Rome) / Italy zone 2 (deprecated)
 <epsg26632> : M'poraloko / UTM zone 32N
 <epsg26692> : M'poraloko / UTM zone 32S
 <epsg26701> : NAD27 / UTM zone 1N
 <epsg26702> : NAD27 / UTM zone 2N
 <epsg26703> : NAD27 / UTM zone 3N
 <epsg26704> : NAD27 / UTM zone 4N
 <epsg26705> : NAD27 / UTM zone 5N
 <epsg26706> : NAD27 / UTM zone 6N
 <epsg26707> : NAD27 / UTM zone 7N
 <epsg26708> : NAD27 / UTM zone 8N
 <epsg26709> : NAD27 / UTM zone 9N
 <epsg26710> : NAD27 / UTM zone 10N
 <epsg26711> : NAD27 / UTM zone 11N
 <epsg26712> : NAD27 / UTM zone 12N
 <epsg26713> : NAD27 / UTM zone 13N
 <epsg26714> : NAD27 / UTM zone 14N
 <epsg26715> : NAD27 / UTM zone 15N
 <epsg26716> : NAD27 / UTM zone 16N
 <epsg26717> : NAD27 / UTM zone 17N
 <epsg26718> : NAD27 / UTM zone 18N
 <epsg26719> : NAD27 / UTM zone 19N
 <epsg26720> : NAD27 / UTM zone 20N
 <epsg26721> : NAD27 / UTM zone 21N
 <epsg26722> : NAD27 / UTM zone 22N
 <epsg26729> : NAD27 / Alabama East
 <epsg26730> : NAD27 / Alabama West
 <epsg26731> : NAD27 / Alaska zone 1
 <epsg26732> : NAD27 / Alaska zone 2
 <epsg26733> : NAD27 / Alaska zone 3
 <epsg26734> : NAD27 / Alaska zone 4
 <epsg26735> : NAD27 / Alaska zone 5
 <epsg26736> : NAD27 / Alaska zone 6
 <epsg26737> : NAD27 / Alaska zone 7
 <epsg26738> : NAD27 / Alaska zone 8
 <epsg26739> : NAD27 / Alaska zone 9
 <epsg26740> : NAD27 / Alaska zone 10
 <epsg26741> : NAD27 / California zone I
 <epsg26742> : NAD27 / California zone II
 <epsg26743> : NAD27 / California zone III
 <epsg26744> : NAD27 / California zone IV
 <epsg26745> : NAD27 / California zone V
 <epsg26746> : NAD27 / California zone VI
 <epsg26747> : NAD27 / California zone VII (deprecated)
 <epsg26748> : NAD27 / Arizona East
 <epsg26749> : NAD27 / Arizona Central
 <epsg26750> : NAD27 / Arizona West
 <epsg26751> : NAD27 / Arkansas North
 <epsg26752> : NAD27 / Arkansas South
 <epsg26753> : NAD27 / Colorado North
 <epsg26754> : NAD27 / Colorado Central
 <epsg26755> : NAD27 / Colorado South
 <epsg26756> : NAD27 / Connecticut
 <epsg26757> : NAD27 / Delaware
 <epsg26758> : NAD27 / Florida East
 <epsg26759> : NAD27 / Florida West
 <epsg26760> : NAD27 / Florida North
 <epsg26766> : NAD27 / Georgia East
 <epsg26767> : NAD27 / Georgia West
 <epsg26768> : NAD27 / Idaho East
 <epsg26769> : NAD27 / Idaho Central
 <epsg26770> : NAD27 / Idaho West
 <epsg26771> : NAD27 / Illinois East
 <epsg26772> : NAD27 / Illinois West
 <epsg26773> : NAD27 / Indiana East
 <epsg26774> : NAD27 / Indiana West
 <epsg26775> : NAD27 / Iowa North
 <epsg26776> : NAD27 / Iowa South
 <epsg26777> : NAD27 / Kansas North
 <epsg26778> : NAD27 / Kansas South
 <epsg26779> : NAD27 / Kentucky North
 <epsg26780> : NAD27 / Kentucky South
 <epsg26781> : NAD27 / Louisiana North
 <epsg26782> : NAD27 / Louisiana South
 <epsg26783> : NAD27 / Maine East
 <epsg26784> : NAD27 / Maine West
 <epsg26785> : NAD27 / Maryland
 <epsg26786> : NAD27 / Massachusetts Mainland
 <epsg26787> : NAD27 / Massachusetts Island
 <epsg26791> : NAD27 / Minnesota North
 <epsg26792> : NAD27 / Minnesota Central
 <epsg26793> : NAD27 / Minnesota South
 <epsg26794> : NAD27 / Mississippi East
 <epsg26795> : NAD27 / Mississippi West
 <epsg26796> : NAD27 / Missouri East
 <epsg26797> : NAD27 / Missouri Central
 <epsg26798> : NAD27 / Missouri West
 <epsg26799> : NAD27 / California zone VII
 <epsg26801> : NAD Michigan / Michigan East
 <epsg26802> : NAD Michigan / Michigan Old Central
 <epsg26803> : NAD Michigan / Michigan West
 <epsg26811> : NAD Michigan / Michigan North
 <epsg26812> : NAD Michigan / Michigan Central
 <epsg26813> : NAD Michigan / Michigan South
 <epsg26901> : NAD83 / UTM zone 1N
 <epsg26902> : NAD83 / UTM zone 2N
 <epsg26903> : NAD83 / UTM zone 3N
 <epsg26904> : NAD83 / UTM zone 4N
 <epsg26905> : NAD83 / UTM zone 5N
 <epsg26906> : NAD83 / UTM zone 6N
 <epsg26907> : NAD83 / UTM zone 7N
 <epsg26908> : NAD83 / UTM zone 8N
 <epsg26909> : NAD83 / UTM zone 9N
 <epsg26910> : NAD83 / UTM zone 10N
 <epsg26911> : NAD83 / UTM zone 11N
 <epsg26912> : NAD83 / UTM zone 12N
 <epsg26913> : NAD83 / UTM zone 13N
 <epsg26914> : NAD83 / UTM zone 14N
 <epsg26915> : NAD83 / UTM zone 15N
 <epsg26916> : NAD83 / UTM zone 16N
 <epsg26917> : NAD83 / UTM zone 17N
 <epsg26918> : NAD83 / UTM zone 18N
 <epsg26919> : NAD83 / UTM zone 19N
 <epsg26920> : NAD83 / UTM zone 20N
 <epsg26921> : NAD83 / UTM zone 21N
 <epsg26922> : NAD83 / UTM zone 22N
 <epsg26923> : NAD83 / UTM zone 23N
 <epsg26929> : NAD83 / Alabama East
 <epsg26930> : NAD83 / Alabama West
 <epsg26931> : NAD83 / Alaska zone 1
 <epsg26932> : NAD83 / Alaska zone 2
 <epsg26933> : NAD83 / Alaska zone 3
 <epsg26934> : NAD83 / Alaska zone 4
 <epsg26935> : NAD83 / Alaska zone 5
 <epsg26936> : NAD83 / Alaska zone 6
 <epsg26937> : NAD83 / Alaska zone 7
 <epsg26938> : NAD83 / Alaska zone 8
 <epsg26939> : NAD83 / Alaska zone 9
 <epsg26940> : NAD83 / Alaska zone 10
 <epsg26941> : NAD83 / California zone 1
 <epsg26942> : NAD83 / California zone 2
 <epsg26943> : NAD83 / California zone 3
 <epsg26944> : NAD83 / California zone 4
 <epsg26945> : NAD83 / California zone 5
 <epsg26946> : NAD83 / California zone 6
 <epsg26948> : NAD83 / Arizona East
 <epsg26949> : NAD83 / Arizona Central
 <epsg26950> : NAD83 / Arizona West
 <epsg26951> : NAD83 / Arkansas North
 <epsg26952> : NAD83 / Arkansas South
 <epsg26953> : NAD83 / Colorado North
 <epsg26954> : NAD83 / Colorado Central
 <epsg26955> : NAD83 / Colorado South
 <epsg26956> : NAD83 / Connecticut
 <epsg26957> : NAD83 / Delaware
 <epsg26958> : NAD83 / Florida East
 <epsg26959> : NAD83 / Florida West
 <epsg26960> : NAD83 / Florida North
 <epsg26961> : NAD83 / Hawaii zone 1
 <epsg26962> : NAD83 / Hawaii zone 2
 <epsg26963> : NAD83 / Hawaii zone 3
 <epsg26964> : NAD83 / Hawaii zone 4
 <epsg26965> : NAD83 / Hawaii zone 5
 <epsg26966> : NAD83 / Georgia East
 <epsg26967> : NAD83 / Georgia West
 <epsg26968> : NAD83 / Idaho East
 <epsg26969> : NAD83 / Idaho Central
 <epsg26970> : NAD83 / Idaho West
 <epsg26971> : NAD83 / Illinois East
 <epsg26972> : NAD83 / Illinois West
 <epsg26973> : NAD83 / Indiana East
 <epsg26974> : NAD83 / Indiana West
 <epsg26975> : NAD83 / Iowa North
 <epsg26976> : NAD83 / Iowa South
 <epsg26977> : NAD83 / Kansas North
 <epsg26978> : NAD83 / Kansas South
 <epsg26979> : NAD83 / Kentucky North (deprecated)
 <epsg26980> : NAD83 / Kentucky South
 <epsg26981> : NAD83 / Louisiana North
 <epsg26982> : NAD83 / Louisiana South
 <epsg26983> : NAD83 / Maine East
 <epsg26984> : NAD83 / Maine West
 <epsg26985> : NAD83 / Maryland
 <epsg26986> : NAD83 / Massachusetts Mainland
 <epsg26987> : NAD83 / Massachusetts Island
 <epsg26988> : NAD83 / Michigan North
 <epsg26989> : NAD83 / Michigan Central
 <epsg26990> : NAD83 / Michigan South
 <epsg26991> : NAD83 / Minnesota North
 <epsg26992> : NAD83 / Minnesota Central
 <epsg26993> : NAD83 / Minnesota South
 <epsg26994> : NAD83 / Mississippi East
 <epsg26995> : NAD83 / Mississippi West
 <epsg26996> : NAD83 / Missouri East
 <epsg26997> : NAD83 / Missouri Central
 <epsg26998> : NAD83 / Missouri West
 <epsg27037> : Nahrwan 1967 / UTM zone 37N
 <epsg27038> : Nahrwan 1967 / UTM zone 38N
 <epsg27039> : Nahrwan 1967 / UTM zone 39N
 <epsg27040> : Nahrwan 1967 / UTM zone 40N
 <epsg27120> : Naparima 1972 / UTM zone 20N
 <epsg27200> : NZGD49 / New Zealand Map Grid
 <epsg27205> : NZGD49 / Mount Eden Circuit
 <epsg27206> : NZGD49 / Bay of Plenty Circuit
 <epsg27207> : NZGD49 / Poverty Bay Circuit
 <epsg27208> : NZGD49 / Hawkes Bay Circuit
 <epsg27209> : NZGD49 / Taranaki Circuit
 <epsg27210> : NZGD49 / Tuhirangi Circuit
 <epsg27211> : NZGD49 / Wanganui Circuit
 <epsg27212> : NZGD49 / Wairarapa Circuit
 <epsg27213> : NZGD49 / Wellington Circuit
 <epsg27214> : NZGD49 / Collingwood Circuit
 <epsg27215> : NZGD49 / Nelson Circuit
 <epsg27216> : NZGD49 / Karamea Circuit
 <epsg27217> : NZGD49 / Buller Circuit
 <epsg27218> : NZGD49 / Grey Circuit
 <epsg27219> : NZGD49 / Amuri Circuit
 <epsg27220> : NZGD49 / Marlborough Circuit
 <epsg27221> : NZGD49 / Hokitika Circuit
 <epsg27222> : NZGD49 / Okarito Circuit
 <epsg27223> : NZGD49 / Jacksons Bay Circuit
 <epsg27224> : NZGD49 / Mount Pleasant Circuit
 <epsg27225> : NZGD49 / Gawler Circuit
 <epsg27226> : NZGD49 / Timaru Circuit
 <epsg27227> : NZGD49 / Lindis Peak Circuit
 <epsg27228> : NZGD49 / Mount Nicholas Circuit
 <epsg27229> : NZGD49 / Mount York Circuit
 <epsg27230> : NZGD49 / Observation Point Circuit
 <epsg27231> : NZGD49 / North Taieri Circuit
 <epsg27232> : NZGD49 / Bluff Circuit
 <epsg27258> : NZGD49 / UTM zone 58S
 <epsg27259> : NZGD49 / UTM zone 59S
 <epsg27260> : NZGD49 / UTM zone 60S
 <epsg27291> : NZGD49 / North Island Grid
 <epsg27292> : NZGD49 / South Island Grid
 <epsg27391> : NGO 1948 (Oslo) / NGO zone I
 <epsg27392> : NGO 1948 (Oslo) / NGO zone II
 <epsg27393> : NGO 1948 (Oslo) / NGO zone III
 <epsg27394> : NGO 1948 (Oslo) / NGO zone IV
 <epsg27395> : NGO 1948 (Oslo) / NGO zone V
 <epsg27396> : NGO 1948 (Oslo) / NGO zone VI
 <epsg27397> : NGO 1948 (Oslo) / NGO zone VII
 <epsg27398> : NGO 1948 (Oslo) / NGO zone VIII
 <epsg27429> : Datum 73 / UTM zone 29N
 <epsg27492> : Datum 73 / Modified Portuguese Grid
 <epsg27500> : ATF (Paris) / Nord de Guerre
 <epsg27561> : NTF (Paris) / Lambert Nord France
 <epsg27562> : NTF (Paris) / Lambert Centre France
 <epsg27563> : NTF (Paris) / Lambert Sud France
 <epsg27564> : NTF (Paris) / Lambert Corse
 <epsg27571> : NTF (Paris) / Lambert zone I
 <epsg27572> : NTF (Paris) / Lambert zone II
 <epsg27573> : NTF (Paris) / Lambert zone III
 <epsg27574> : NTF (Paris) / Lambert zone IV
 <epsg27581> : NTF (Paris) / France I (deprecated)
 <epsg27582> : NTF (Paris) / France II (deprecated)
 <epsg27583> : NTF (Paris) / France III (deprecated)
 <epsg27584> : NTF (Paris) / France IV (deprecated)
 <epsg27591> : NTF (Paris) / Nord France (deprecated)
 <epsg27592> : NTF (Paris) / Centre France (deprecated)
 <epsg27593> : NTF (Paris) / Sud France (deprecated)
 <epsg27594> : NTF (Paris) / Corse (deprecated)
 <epsg27700> : OSGB 1936 / British National Grid
 <epsg28191> : Palestine 1923 / Palestine Grid
 <epsg28192> : Palestine 1923 / Palestine Belt
 <epsg28193> : Palestine 1923 / Israeli CS Grid
 <epsg28232> : Pointe Noire / UTM zone 32S
 <epsg28348> : GDA94 / MGA zone 48
 <epsg28349> : GDA94 / MGA zone 49
 <epsg28350> : GDA94 / MGA zone 50
 <epsg28351> : GDA94 / MGA zone 51
 <epsg28352> : GDA94 / MGA zone 52
 <epsg28353> : GDA94 / MGA zone 53
 <epsg28354> : GDA94 / MGA zone 54
 <epsg28355> : GDA94 / MGA zone 55
 <epsg28356> : GDA94 / MGA zone 56
 <epsg28357> : GDA94 / MGA zone 57
 <epsg28358> : GDA94 / MGA zone 58
 <epsg28402> : Pulkovo 1942 / Gauss-Kruger zone 2
 <epsg28403> : Pulkovo 1942 / Gauss-Kruger zone 3
 <epsg28404> : Pulkovo 1942 / Gauss-Kruger zone 4
 <epsg28405> : Pulkovo 1942 / Gauss-Kruger zone 5
 <epsg28406> : Pulkovo 1942 / Gauss-Kruger zone 6
 <epsg28407> : Pulkovo 1942 / Gauss-Kruger zone 7
 <epsg28408> : Pulkovo 1942 / Gauss-Kruger zone 8
 <epsg28409> : Pulkovo 1942 / Gauss-Kruger zone 9
 <epsg28410> : Pulkovo 1942 / Gauss-Kruger zone 10
 <epsg28411> : Pulkovo 1942 / Gauss-Kruger zone 11
 <epsg28412> : Pulkovo 1942 / Gauss-Kruger zone 12
 <epsg28413> : Pulkovo 1942 / Gauss-Kruger zone 13
 <epsg28414> : Pulkovo 1942 / Gauss-Kruger zone 14
 <epsg28415> : Pulkovo 1942 / Gauss-Kruger zone 15
 <epsg28416> : Pulkovo 1942 / Gauss-Kruger zone 16
 <epsg28417> : Pulkovo 1942 / Gauss-Kruger zone 17
 <epsg28418> : Pulkovo 1942 / Gauss-Kruger zone 18
 <epsg28419> : Pulkovo 1942 / Gauss-Kruger zone 19
 <epsg28420> : Pulkovo 1942 / Gauss-Kruger zone 20
 <epsg28421> : Pulkovo 1942 / Gauss-Kruger zone 21
 <epsg28422> : Pulkovo 1942 / Gauss-Kruger zone 22
 <epsg28423> : Pulkovo 1942 / Gauss-Kruger zone 23
 <epsg28424> : Pulkovo 1942 / Gauss-Kruger zone 24
 <epsg28425> : Pulkovo 1942 / Gauss-Kruger zone 25
 <epsg28426> : Pulkovo 1942 / Gauss-Kruger zone 26
 <epsg28427> : Pulkovo 1942 / Gauss-Kruger zone 27
 <epsg28428> : Pulkovo 1942 / Gauss-Kruger zone 28
 <epsg28429> : Pulkovo 1942 / Gauss-Kruger zone 29
 <epsg28430> : Pulkovo 1942 / Gauss-Kruger zone 30
 <epsg28431> : Pulkovo 1942 / Gauss-Kruger zone 31
 <epsg28432> : Pulkovo 1942 / Gauss-Kruger zone 32
 <epsg28462> : Pulkovo 1942 / Gauss-Kruger 2N (deprecated)
 <epsg28463> : Pulkovo 1942 / Gauss-Kruger 3N (deprecated)
 <epsg28464> : Pulkovo 1942 / Gauss-Kruger 4N (deprecated)
 <epsg28465> : Pulkovo 1942 / Gauss-Kruger 5N (deprecated)
 <epsg28466> : Pulkovo 1942 / Gauss-Kruger 6N (deprecated)
 <epsg28467> : Pulkovo 1942 / Gauss-Kruger 7N (deprecated)
 <epsg28468> : Pulkovo 1942 / Gauss-Kruger 8N (deprecated)
 <epsg28469> : Pulkovo 1942 / Gauss-Kruger 9N (deprecated)
 <epsg28470> : Pulkovo 1942 / Gauss-Kruger 10N (deprecated)
 <epsg28471> : Pulkovo 1942 / Gauss-Kruger 11N (deprecated)
 <epsg28472> : Pulkovo 1942 / Gauss-Kruger 12N (deprecated)
 <epsg28473> : Pulkovo 1942 / Gauss-Kruger 13N (deprecated)
 <epsg28474> : Pulkovo 1942 / Gauss-Kruger 14N (deprecated)
 <epsg28475> : Pulkovo 1942 / Gauss-Kruger 15N (deprecated)
 <epsg28476> : Pulkovo 1942 / Gauss-Kruger 16N (deprecated)
 <epsg28477> : Pulkovo 1942 / Gauss-Kruger 17N (deprecated)
 <epsg28478> : Pulkovo 1942 / Gauss-Kruger 18N (deprecated)
 <epsg28479> : Pulkovo 1942 / Gauss-Kruger 19N (deprecated)
 <epsg28480> : Pulkovo 1942 / Gauss-Kruger 20N (deprecated)
 <epsg28481> : Pulkovo 1942 / Gauss-Kruger 21N (deprecated)
 <epsg28482> : Pulkovo 1942 / Gauss-Kruger 22N (deprecated)
 <epsg28483> : Pulkovo 1942 / Gauss-Kruger 23N (deprecated)
 <epsg28484> : Pulkovo 1942 / Gauss-Kruger 24N (deprecated)
 <epsg28485> : Pulkovo 1942 / Gauss-Kruger 25N (deprecated)
 <epsg28486> : Pulkovo 1942 / Gauss-Kruger 26N (deprecated)
 <epsg28487> : Pulkovo 1942 / Gauss-Kruger 27N (deprecated)
 <epsg28488> : Pulkovo 1942 / Gauss-Kruger 28N (deprecated)
 <epsg28489> : Pulkovo 1942 / Gauss-Kruger 29N (deprecated)
 <epsg28490> : Pulkovo 1942 / Gauss-Kruger 30N (deprecated)
 <epsg28491> : Pulkovo 1942 / Gauss-Kruger 31N (deprecated)
 <epsg28492> : Pulkovo 1942 / Gauss-Kruger 32N (deprecated)
 <epsg28600> : Qatar 1974 / Qatar National Grid
 <epsg28991> : Amersfoort / RD Old
 <epsg28992> : Amersfoort / RD New
 <epsg29100> : SAD69 / Brazil Polyconic (deprecated)
 <epsg29101> : SAD69 / Brazil Polyconic
 <epsg29118> : SAD69 / UTM zone 18N (deprecated)
 <epsg29119> : SAD69 / UTM zone 19N (deprecated)
 <epsg29120> : SAD69 / UTM zone 20N (deprecated)
 <epsg29121> : SAD69 / UTM zone 21N (deprecated)
 <epsg29122> : SAD69 / UTM zone 22N (deprecated)
 <epsg29168> : SAD69 / UTM zone 18N
 <epsg29169> : SAD69 / UTM zone 19N
 <epsg29170> : SAD69 / UTM zone 20N
 <epsg29171> : SAD69 / UTM zone 21N
 <epsg29172> : SAD69 / UTM zone 22N
 <epsg29177> : SAD69 / UTM zone 17S (deprecated)
 <epsg29178> : SAD69 / UTM zone 18S (deprecated)
 <epsg29179> : SAD69 / UTM zone 19S (deprecated)
 <epsg29180> : SAD69 / UTM zone 20S (deprecated)
 <epsg29181> : SAD69 / UTM zone 21S (deprecated)
 <epsg29182> : SAD69 / UTM zone 22S (deprecated)
 <epsg29183> : SAD69 / UTM zone 23S (deprecated)
 <epsg29184> : SAD69 / UTM zone 24S (deprecated)
 <epsg29185> : SAD69 / UTM zone 25S (deprecated)
 <epsg29187> : SAD69 / UTM zone 17S
 <epsg29188> : SAD69 / UTM zone 18S
 <epsg29189> : SAD69 / UTM zone 19S
 <epsg29190> : SAD69 / UTM zone 20S
 <epsg29191> : SAD69 / UTM zone 21S
 <epsg29192> : SAD69 / UTM zone 22S
 <epsg29193> : SAD69 / UTM zone 23S
 <epsg29194> : SAD69 / UTM zone 24S
 <epsg29195> : SAD69 / UTM zone 25S
 <epsg29220> : Sapper Hill 1943 / UTM zone 20S
 <epsg29221> : Sapper Hill 1943 / UTM zone 21S
 <epsg29333> : Schwarzeck / UTM zone 33S
 <epsg29635> : Sudan / UTM zone 35N (deprecated)
 <epsg29636> : Sudan / UTM zone 36N (deprecated)
 <epsg29700> : Tananarive (Paris) / Laborde Grid (deprecated)
 <epsg29702> : Tananarive (Paris) / Laborde Grid approximation
 <epsg29738> : Tananarive / UTM zone 38S
 <epsg29739> : Tananarive / UTM zone 39S
 <epsg29849> : Timbalai 1948 / UTM zone 49N
 <epsg29850> : Timbalai 1948 / UTM zone 50N
 <epsg29871> : Timbalai 1948 / RSO Borneo (ch)
 <epsg29872> : Timbalai 1948 / RSO Borneo (ft)
 <epsg29873> : Timbalai 1948 / RSO Borneo (m)
 <epsg29900> : TM65 / Irish National Grid (deprecated)
 <epsg29901> : OSNI 1952 / Irish National Grid
 <epsg29902> : TM65 / Irish Grid
 <epsg29903> : TM75 / Irish Grid
 <epsg30161> : Tokyo / Japan Plane Rectangular CS I
 <epsg30162> : Tokyo / Japan Plane Rectangular CS II
 <epsg30163> : Tokyo / Japan Plane Rectangular CS III
 <epsg30164> : Tokyo / Japan Plane Rectangular CS IV
 <epsg30165> : Tokyo / Japan Plane Rectangular CS V
 <epsg30166> : Tokyo / Japan Plane Rectangular CS VI
 <epsg30167> : Tokyo / Japan Plane Rectangular CS VII
 <epsg30168> : Tokyo / Japan Plane Rectangular CS VIII
 <epsg30169> : Tokyo / Japan Plane Rectangular CS IX
 <epsg30170> : Tokyo / Japan Plane Rectangular CS X
 <epsg30171> : Tokyo / Japan Plane Rectangular CS XI
 <epsg30172> : Tokyo / Japan Plane Rectangular CS XII
 <epsg30173> : Tokyo / Japan Plane Rectangular CS XIII
 <epsg30174> : Tokyo / Japan Plane Rectangular CS XIV
 <epsg30175> : Tokyo / Japan Plane Rectangular CS XV
 <epsg30176> : Tokyo / Japan Plane Rectangular CS XVI
 <epsg30177> : Tokyo / Japan Plane Rectangular CS XVII
 <epsg30178> : Tokyo / Japan Plane Rectangular CS XVIII
 <epsg30179> : Tokyo / Japan Plane Rectangular CS XIX
 <epsg30200> : Trinidad 1903 / Trinidad Grid
 <epsg30339> : TC(1948) / UTM zone 39N
 <epsg30340> : TC(1948) / UTM zone 40N
 <epsg30491> : Voirol 1875 / Nord Algerie (ancienne)
 <epsg30492> : Voirol 1875 / Sud Algerie (ancienne)
 <epsg30493> : Voirol 1879 / Nord Algerie (ancienne)
 <epsg30494> : Voirol 1879 / Sud Algerie (ancienne)
 <epsg30729> : Nord Sahara 1959 / UTM zone 29N
 <epsg30730> : Nord Sahara 1959 / UTM zone 30N
 <epsg30731> : Nord Sahara 1959 / UTM zone 31N
 <epsg30732> : Nord Sahara 1959 / UTM zone 32N
 <epsg30791> : Nord Sahara 1959 / Voirol Unifie Nord
 <epsg30792> : Nord Sahara 1959 / Voirol Unifie Sud
 <epsg30800> : RT38 2.5 gon W (deprecated)
 <epsg31028> : Yoff / UTM zone 28N
 <epsg31121> : Zanderij / UTM zone 21N
 <epsg31154> : Zanderij / TM 54 NW
 <epsg31170> : Zanderij / Suriname Old TM
 <epsg31171> : Zanderij / Suriname TM
 <epsg31251> : MGI (Ferro) / Austria GK West Zone
 <epsg31252> : MGI (Ferro) / Austria GK Central Zone
 <epsg31253> : MGI (Ferro) / Austria GK East Zone
 <epsg31254> : MGI / Austria GK West
 <epsg31255> : MGI / Austria GK Central
 <epsg31256> : MGI / Austria GK East
 <epsg31257> : MGI / Austria GK M28
 <epsg31258> : MGI / Austria GK M31
 <epsg31259> : MGI / Austria GK M34
 <epsg31265> : MGI / 3-degree Gauss zone 5 (deprecated)
 <epsg31266> : MGI / 3-degree Gauss zone 6 (deprecated)
 <epsg31267> : MGI / 3-degree Gauss zone 7 (deprecated)
 <epsg31268> : MGI / 3-degree Gauss zone 8 (deprecated)
 <epsg31275> : MGI / Balkans zone 5
 <epsg31276> : MGI / Balkans zone 6
 <epsg31277> : MGI / Balkans zone 7
 <epsg31278> : MGI / Balkans zone 8 (deprecated)
 <epsg31279> : MGI / Balkans zone 8
 <epsg31281> : MGI (Ferro) / Austria West Zone
 <epsg31282> : MGI (Ferro) / Austria Central Zone
 <epsg31283> : MGI (Ferro) / Austria East Zone
 <epsg31284> : MGI / Austria M28
 <epsg31285> : MGI / Austria M31
 <epsg31286> : MGI / Austria M34
 <epsg31287> : MGI / Austria Lambert
 <epsg31288> : MGI (Ferro) / M28
 <epsg31289> : MGI (Ferro) / M31
 <epsg31290> : MGI (Ferro) / M34
 <epsg31291> : MGI (Ferro) / Austria West Zone (deprecated)
 <epsg31292> : MGI (Ferro) / Austria Central Zone (deprecated)
 <epsg31293> : MGI (Ferro) / Austria East Zone (deprecated)
 <epsg31294> : MGI / M28 (deprecated)
 <epsg31295> : MGI / M31 (deprecated)
 <epsg31296> : MGI / M34 (deprecated)
 <epsg31297> : MGI / Austria Lambert (deprecated)
 <epsg31300> : Belge 1972 / Belge Lambert 72
 <epsg31370> : Belge 1972 / Belgian Lambert 72
 <epsg31461> : DHDN / 3-degree Gauss zone 1 (deprecated)
 <epsg31462> : DHDN / 3-degree Gauss zone 2 (deprecated)
 <epsg31463> : DHDN / 3-degree Gauss zone 3 (deprecated)
 <epsg31464> : DHDN / 3-degree Gauss zone 4 (deprecated)
 <epsg31465> : DHDN / 3-degree Gauss zone 5 (deprecated)
 <epsg31466> : DHDN / Gauss-Kruger zone 2
 <epsg31467> : DHDN / Gauss-Kruger zone 3
 <epsg31468> : DHDN / Gauss-Kruger zone 4
 <epsg31469> : DHDN / Gauss-Kruger zone 5
 <epsg31528> : Conakry 1905 / UTM zone 28N
 <epsg31529> : Conakry 1905 / UTM zone 29N
 <epsg31600> : Dealul Piscului 1933/ Stereo 33
 <epsg31700> : Dealul Piscului 1970/ Stereo 70
 <epsg31838> : NGN / UTM zone 38N
 <epsg31839> : NGN / UTM zone 39N
 <epsg31900> : KUDAMS / KTM (deprecated)
 <epsg31901> : KUDAMS / KTM
 <epsg31965> : SIRGAS 2000 / UTM zone 11N
 <epsg31966> : SIRGAS 2000 / UTM zone 12N
 <epsg31967> : SIRGAS 2000 / UTM zone 13N
 <epsg31968> : SIRGAS 2000 / UTM zone 14N
 <epsg31969> : SIRGAS 2000 / UTM zone 15N
 <epsg31970> : SIRGAS 2000 / UTM zone 16N
 <epsg31971> : SIRGAS 2000 / UTM zone 17N
 <epsg31972> : SIRGAS 2000 / UTM zone 18N
 <epsg31973> : SIRGAS 2000 / UTM zone 19N
 <epsg31974> : SIRGAS 2000 / UTM zone 20N
 <epsg31975> : SIRGAS 2000 / UTM zone 21N
 <epsg31976> : SIRGAS 2000 / UTM zone 22N
 <epsg31977> : SIRGAS 2000 / UTM zone 17S
 <epsg31978> : SIRGAS 2000 / UTM zone 18S
 <epsg31979> : SIRGAS 2000 / UTM zone 19S
 <epsg31980> : SIRGAS 2000 / UTM zone 20S
 <epsg31981> : SIRGAS 2000 / UTM zone 21S
 <epsg31982> : SIRGAS 2000 / UTM zone 22S
 <epsg31983> : SIRGAS 2000 / UTM zone 23S
 <epsg31984> : SIRGAS 2000 / UTM zone 24S
 <epsg31985> : SIRGAS 2000 / UTM zone 25S
 <epsg31986> : SIRGAS / UTM zone 17N
 <epsg31987> : SIRGAS / UTM zone 18N
 <epsg31988> : SIRGAS / UTM zone 19N
 <epsg31989> : SIRGAS / UTM zone 20N
 <epsg31990> : SIRGAS / UTM zone 21N
 <epsg31991> : SIRGAS / UTM zone 22N
 <epsg31992> : SIRGAS / UTM zone 17S
 <epsg31993> : SIRGAS / UTM zone 18S
 <epsg31994> : SIRGAS / UTM zone 19S
 <epsg31995> : SIRGAS / UTM zone 20S
 <epsg31996> : SIRGAS / UTM zone 21S
 <epsg31997> : SIRGAS / UTM zone 22S
 <epsg31998> : SIRGAS / UTM zone 23S
 <epsg31999> : SIRGAS / UTM zone 24S
 <epsg32000> : SIRGAS / UTM zone 25S
 <epsg32001> : NAD27 / Montana North
 <epsg32002> : NAD27 / Montana Central
 <epsg32003> : NAD27 / Montana South
 <epsg32005> : NAD27 / Nebraska North
 <epsg32006> : NAD27 / Nebraska South
 <epsg32007> : NAD27 / Nevada East
 <epsg32008> : NAD27 / Nevada Central
 <epsg32009> : NAD27 / Nevada West
 <epsg32010> : NAD27 / New Hampshire
 <epsg32011> : NAD27 / New Jersey
 <epsg32012> : NAD27 / New Mexico East
 <epsg32013> : NAD27 / New Mexico Central
 <epsg32014> : NAD27 / New Mexico West
 <epsg32015> : NAD27 / New York East
 <epsg32016> : NAD27 / New York Central
 <epsg32017> : NAD27 / New York West
 <epsg32018> : NAD27 / New York Long Island
 <epsg32019> : NAD27 / North Carolina
 <epsg32020> : NAD27 / North Dakota North
 <epsg32021> : NAD27 / North Dakota South
 <epsg32022> : NAD27 / Ohio North
 <epsg32023> : NAD27 / Ohio South
 <epsg32024> : NAD27 / Oklahoma North
 <epsg32025> : NAD27 / Oklahoma South
 <epsg32026> : NAD27 / Oregon North
 <epsg32027> : NAD27 / Oregon South
 <epsg32028> : NAD27 / Pennsylvania North
 <epsg32029> : NAD27 / Pennsylvania South
 <epsg32030> : NAD27 / Rhode Island
 <epsg32031> : NAD27 / South Carolina North
 <epsg32033> : NAD27 / South Carolina South
 <epsg32034> : NAD27 / South Dakota North
 <epsg32035> : NAD27 / South Dakota South
 <epsg32036> : NAD27 / Tennessee (deprecated)
 <epsg32037> : NAD27 / Texas North
 <epsg32038> : NAD27 / Texas North Central
 <epsg32039> : NAD27 / Texas Central
 <epsg32040> : NAD27 / Texas South Central
 <epsg32041> : NAD27 / Texas South
 <epsg32042> : NAD27 / Utah North
 <epsg32043> : NAD27 / Utah Central
 <epsg32044> : NAD27 / Utah South
 <epsg32045> : NAD27 / Vermont
 <epsg32046> : NAD27 / Virginia North
 <epsg32047> : NAD27 / Virginia South
 <epsg32048> : NAD27 / Washington North
 <epsg32049> : NAD27 / Washington South
 <epsg32050> : NAD27 / West Virginia North
 <epsg32051> : NAD27 / West Virginia South
 <epsg32052> : NAD27 / Wisconsin North
 <epsg32053> : NAD27 / Wisconsin Central
 <epsg32054> : NAD27 / Wisconsin South
 <epsg32055> : NAD27 / Wyoming East
 <epsg32056> : NAD27 / Wyoming East Central
 <epsg32057> : NAD27 / Wyoming West Central
 <epsg32058> : NAD27 / Wyoming West
 <epsg32061> : NAD27 / Guatemala Norte
 <epsg32062> : NAD27 / Guatemala Sur
 <epsg32064> : NAD27 / BLM 14N (ftUS)
 <epsg32065> : NAD27 / BLM 15N (ftUS)
 <epsg32066> : NAD27 / BLM 16N (ftUS)
 <epsg32067> : NAD27 / BLM 17N (ftUS)
 <epsg32074> : NAD27 / BLM 14N (feet) (deprecated)
 <epsg32075> : NAD27 / BLM 15N (feet) (deprecated)
 <epsg32076> : NAD27 / BLM 16N (feet) (deprecated)
 <epsg32077> : NAD27 / BLM 17N (feet) (deprecated)
 <epsg32081> : NAD27 / MTM zone 1
 <epsg32082> : NAD27 / MTM zone 2
 <epsg32083> : NAD27 / MTM zone 3
 <epsg32084> : NAD27 / MTM zone 4
 <epsg32085> : NAD27 / MTM zone 5
 <epsg32086> : NAD27 / MTM zone 6
 <epsg32098> : NAD27 / Quebec Lambert
 <epsg32099> : NAD27 / Louisiana Offshore
 <epsg32100> : NAD83 / Montana
 <epsg32104> : NAD83 / Nebraska
 <epsg32107> : NAD83 / Nevada East
 <epsg32108> : NAD83 / Nevada Central
 <epsg32109> : NAD83 / Nevada West
 <epsg32110> : NAD83 / New Hampshire
 <epsg32111> : NAD83 / New Jersey
 <epsg32112> : NAD83 / New Mexico East
 <epsg32113> : NAD83 / New Mexico Central
 <epsg32114> : NAD83 / New Mexico West
 <epsg32115> : NAD83 / New York East
 <epsg32116> : NAD83 / New York Central
 <epsg32117> : NAD83 / New York West
 <epsg32118> : NAD83 / New York Long Island
 <epsg32119> : NAD83 / North Carolina
 <epsg32120> : NAD83 / North Dakota North
 <epsg32121> : NAD83 / North Dakota South
 <epsg32122> : NAD83 / Ohio North
 <epsg32123> : NAD83 / Ohio South
 <epsg32124> : NAD83 / Oklahoma North
 <epsg32125> : NAD83 / Oklahoma South
 <epsg32126> : NAD83 / Oregon North
 <epsg32127> : NAD83 / Oregon South
 <epsg32128> : NAD83 / Pennsylvania North
 <epsg32129> : NAD83 / Pennsylvania South
 <epsg32130> : NAD83 / Rhode Island
 <epsg32133> : NAD83 / South Carolina
 <epsg32134> : NAD83 / South Dakota North
 <epsg32135> : NAD83 / South Dakota South
 <epsg32136> : NAD83 / Tennessee
 <epsg32137> : NAD83 / Texas North
 <epsg32138> : NAD83 / Texas North Central
 <epsg32139> : NAD83 / Texas Central
 <epsg32140> : NAD83 / Texas South Central
 <epsg32141> : NAD83 / Texas South
 <epsg32142> : NAD83 / Utah North
 <epsg32143> : NAD83 / Utah Central
 <epsg32144> : NAD83 / Utah South
 <epsg32145> : NAD83 / Vermont
 <epsg32146> : NAD83 / Virginia North
 <epsg32147> : NAD83 / Virginia South
 <epsg32148> : NAD83 / Washington North
 <epsg32149> : NAD83 / Washington South
 <epsg32150> : NAD83 / West Virginia North
 <epsg32151> : NAD83 / West Virginia South
 <epsg32152> : NAD83 / Wisconsin North
 <epsg32153> : NAD83 / Wisconsin Central
 <epsg32154> : NAD83 / Wisconsin South
 <epsg32155> : NAD83 / Wyoming East
 <epsg32156> : NAD83 / Wyoming East Central
 <epsg32157> : NAD83 / Wyoming West Central
 <epsg32158> : NAD83 / Wyoming West
 <epsg32161> : NAD83 / Puerto Rico & Virgin Is.
 <epsg32164> : NAD83 / BLM 14N (ftUS)
 <epsg32165> : NAD83 / BLM 15N (ftUS)
 <epsg32166> : NAD83 / BLM 16N (ftUS)
 <epsg32167> : NAD83 / BLM 17N (ftUS)
 <epsg32180> : NAD83 / SCoPQ zone 2
 <epsg32181> : NAD83 / MTM zone 1
 <epsg32182> : NAD83 / MTM zone 2
 <epsg32183> : NAD83 / MTM zone 3
 <epsg32184> : NAD83 / MTM zone 4
 <epsg32185> : NAD83 / MTM zone 5
 <epsg32186> : NAD83 / MTM zone 6
 <epsg32187> : NAD83 / MTM zone 7
 <epsg32188> : NAD83 / MTM zone 8
 <epsg32189> : NAD83 / MTM zone 9
 <epsg32190> : NAD83 / MTM zone 10
 <epsg32191> : NAD83 / MTM zone 11
 <epsg32192> : NAD83 / MTM zone 12
 <epsg32193> : NAD83 / MTM zone 13
 <epsg32194> : NAD83 / MTM zone 14
 <epsg32195> : NAD83 / MTM zone 15
 <epsg32196> : NAD83 / MTM zone 16
 <epsg32197> : NAD83 / MTM zone 17
 <epsg32198> : NAD83 / Quebec Lambert
 <epsg32199> : NAD83 / Louisiana Offshore
 <epsg32201> : WGS 72 / UTM zone 1N
 <epsg32202> : WGS 72 / UTM zone 2N
 <epsg32203> : WGS 72 / UTM zone 3N
 <epsg32204> : WGS 72 / UTM zone 4N
 <epsg32205> : WGS 72 / UTM zone 5N
 <epsg32206> : WGS 72 / UTM zone 6N
 <epsg32207> : WGS 72 / UTM zone 7N
 <epsg32208> : WGS 72 / UTM zone 8N
 <epsg32209> : WGS 72 / UTM zone 9N
 <epsg32210> : WGS 72 / UTM zone 10N
 <epsg32211> : WGS 72 / UTM zone 11N
 <epsg32212> : WGS 72 / UTM zone 12N
 <epsg32213> : WGS 72 / UTM zone 13N
 <epsg32214> : WGS 72 / UTM zone 14N
 <epsg32215> : WGS 72 / UTM zone 15N
 <epsg32216> : WGS 72 / UTM zone 16N
 <epsg32217> : WGS 72 / UTM zone 17N
 <epsg32218> : WGS 72 / UTM zone 18N
 <epsg32219> : WGS 72 / UTM zone 19N
 <epsg32220> : WGS 72 / UTM zone 20N
 <epsg32221> : WGS 72 / UTM zone 21N
 <epsg32222> : WGS 72 / UTM zone 22N
 <epsg32223> : WGS 72 / UTM zone 23N
 <epsg32224> : WGS 72 / UTM zone 24N
 <epsg32225> : WGS 72 / UTM zone 25N
 <epsg32226> : WGS 72 / UTM zone 26N
 <epsg32227> : WGS 72 / UTM zone 27N
 <epsg32228> : WGS 72 / UTM zone 28N
 <epsg32229> : WGS 72 / UTM zone 29N
 <epsg32230> : WGS 72 / UTM zone 30N
 <epsg32231> : WGS 72 / UTM zone 31N
 <epsg32232> : WGS 72 / UTM zone 32N
 <epsg32233> : WGS 72 / UTM zone 33N
 <epsg32234> : WGS 72 / UTM zone 34N
 <epsg32235> : WGS 72 / UTM zone 35N
 <epsg32236> : WGS 72 / UTM zone 36N
 <epsg32237> : WGS 72 / UTM zone 37N
 <epsg32238> : WGS 72 / UTM zone 38N
 <epsg32239> : WGS 72 / UTM zone 39N
 <epsg32240> : WGS 72 / UTM zone 40N
 <epsg32241> : WGS 72 / UTM zone 41N
 <epsg32242> : WGS 72 / UTM zone 42N
 <epsg32243> : WGS 72 / UTM zone 43N
 <epsg32244> : WGS 72 / UTM zone 44N
 <epsg32245> : WGS 72 / UTM zone 45N
 <epsg32246> : WGS 72 / UTM zone 46N
 <epsg32247> : WGS 72 / UTM zone 47N
 <epsg32248> : WGS 72 / UTM zone 48N
 <epsg32249> : WGS 72 / UTM zone 49N
 <epsg32250> : WGS 72 / UTM zone 50N
 <epsg32251> : WGS 72 / UTM zone 51N
 <epsg32252> : WGS 72 / UTM zone 52N
 <epsg32253> : WGS 72 / UTM zone 53N
 <epsg32254> : WGS 72 / UTM zone 54N
 <epsg32255> : WGS 72 / UTM zone 55N
 <epsg32256> : WGS 72 / UTM zone 56N
 <epsg32257> : WGS 72 / UTM zone 57N
 <epsg32258> : WGS 72 / UTM zone 58N
 <epsg32259> : WGS 72 / UTM zone 59N
 <epsg32260> : WGS 72 / UTM zone 60N
 <epsg32301> : WGS 72 / UTM zone 1S
 <epsg32302> : WGS 72 / UTM zone 2S
 <epsg32303> : WGS 72 / UTM zone 3S
 <epsg32304> : WGS 72 / UTM zone 4S
 <epsg32305> : WGS 72 / UTM zone 5S
 <epsg32306> : WGS 72 / UTM zone 6S
 <epsg32307> : WGS 72 / UTM zone 7S
 <epsg32308> : WGS 72 / UTM zone 8S
 <epsg32309> : WGS 72 / UTM zone 9S
 <epsg32310> : WGS 72 / UTM zone 10S
 <epsg32311> : WGS 72 / UTM zone 11S
 <epsg32312> : WGS 72 / UTM zone 12S
 <epsg32313> : WGS 72 / UTM zone 13S
 <epsg32314> : WGS 72 / UTM zone 14S
 <epsg32315> : WGS 72 / UTM zone 15S
 <epsg32316> : WGS 72 / UTM zone 16S
 <epsg32317> : WGS 72 / UTM zone 17S
 <epsg32318> : WGS 72 / UTM zone 18S
 <epsg32319> : WGS 72 / UTM zone 19S
 <epsg32320> : WGS 72 / UTM zone 20S
 <epsg32321> : WGS 72 / UTM zone 21S
 <epsg32322> : WGS 72 / UTM zone 22S
 <epsg32323> : WGS 72 / UTM zone 23S
 <epsg32324> : WGS 72 / UTM zone 24S
 <epsg32325> : WGS 72 / UTM zone 25S
 <epsg32326> : WGS 72 / UTM zone 26S
 <epsg32327> : WGS 72 / UTM zone 27S
 <epsg32328> : WGS 72 / UTM zone 28S
 <epsg32329> : WGS 72 / UTM zone 29S
 <epsg32330> : WGS 72 / UTM zone 30S
 <epsg32331> : WGS 72 / UTM zone 31S
 <epsg32332> : WGS 72 / UTM zone 32S
 <epsg32333> : WGS 72 / UTM zone 33S
 <epsg32334> : WGS 72 / UTM zone 34S
 <epsg32335> : WGS 72 / UTM zone 35S
 <epsg32336> : WGS 72 / UTM zone 36S
 <epsg32337> : WGS 72 / UTM zone 37S
 <epsg32338> : WGS 72 / UTM zone 38S
 <epsg32339> : WGS 72 / UTM zone 39S
 <epsg32340> : WGS 72 / UTM zone 40S
 <epsg32341> : WGS 72 / UTM zone 41S
 <epsg32342> : WGS 72 / UTM zone 42S
 <epsg32343> : WGS 72 / UTM zone 43S
 <epsg32344> : WGS 72 / UTM zone 44S
 <epsg32345> : WGS 72 / UTM zone 45S
 <epsg32346> : WGS 72 / UTM zone 46S
 <epsg32347> : WGS 72 / UTM zone 47S
 <epsg32348> : WGS 72 / UTM zone 48S
 <epsg32349> : WGS 72 / UTM zone 49S
 <epsg32350> : WGS 72 / UTM zone 50S
 <epsg32351> : WGS 72 / UTM zone 51S
 <epsg32352> : WGS 72 / UTM zone 52S
 <epsg32353> : WGS 72 / UTM zone 53S
 <epsg32354> : WGS 72 / UTM zone 54S
 <epsg32355> : WGS 72 / UTM zone 55S
 <epsg32356> : WGS 72 / UTM zone 56S
 <epsg32357> : WGS 72 / UTM zone 57S
 <epsg32358> : WGS 72 / UTM zone 58S
 <epsg32359> : WGS 72 / UTM zone 59S
 <epsg32360> : WGS 72 / UTM zone 60S
 <epsg32401> : WGS 72BE / UTM zone 1N
 <epsg32402> : WGS 72BE / UTM zone 2N
 <epsg32403> : WGS 72BE / UTM zone 3N
 <epsg32404> : WGS 72BE / UTM zone 4N
 <epsg32405> : WGS 72BE / UTM zone 5N
 <epsg32406> : WGS 72BE / UTM zone 6N
 <epsg32407> : WGS 72BE / UTM zone 7N
 <epsg32408> : WGS 72BE / UTM zone 8N
 <epsg32409> : WGS 72BE / UTM zone 9N
 <epsg32410> : WGS 72BE / UTM zone 10N
 <epsg32411> : WGS 72BE / UTM zone 11N
 <epsg32412> : WGS 72BE / UTM zone 12N
 <epsg32413> : WGS 72BE / UTM zone 13N
 <epsg32414> : WGS 72BE / UTM zone 14N
 <epsg32415> : WGS 72BE / UTM zone 15N
 <epsg32416> : WGS 72BE / UTM zone 16N
 <epsg32417> : WGS 72BE / UTM zone 17N
 <epsg32418> : WGS 72BE / UTM zone 18N
 <epsg32419> : WGS 72BE / UTM zone 19N
 <epsg32420> : WGS 72BE / UTM zone 20N
 <epsg32421> : WGS 72BE / UTM zone 21N
 <epsg32422> : WGS 72BE / UTM zone 22N
 <epsg32423> : WGS 72BE / UTM zone 23N
 <epsg32424> : WGS 72BE / UTM zone 24N
 <epsg32425> : WGS 72BE / UTM zone 25N
 <epsg32426> : WGS 72BE / UTM zone 26N
 <epsg32427> : WGS 72BE / UTM zone 27N
 <epsg32428> : WGS 72BE / UTM zone 28N
 <epsg32429> : WGS 72BE / UTM zone 29N
 <epsg32430> : WGS 72BE / UTM zone 30N
 <epsg32431> : WGS 72BE / UTM zone 31N
 <epsg32432> : WGS 72BE / UTM zone 32N
 <epsg32433> : WGS 72BE / UTM zone 33N
 <epsg32434> : WGS 72BE / UTM zone 34N
 <epsg32435> : WGS 72BE / UTM zone 35N
 <epsg32436> : WGS 72BE / UTM zone 36N
 <epsg32437> : WGS 72BE / UTM zone 37N
 <epsg32438> : WGS 72BE / UTM zone 38N
 <epsg32439> : WGS 72BE / UTM zone 39N
 <epsg32440> : WGS 72BE / UTM zone 40N
 <epsg32441> : WGS 72BE / UTM zone 41N
 <epsg32442> : WGS 72BE / UTM zone 42N
 <epsg32443> : WGS 72BE / UTM zone 43N
 <epsg32444> : WGS 72BE / UTM zone 44N
 <epsg32445> : WGS 72BE / UTM zone 45N
 <epsg32446> : WGS 72BE / UTM zone 46N
 <epsg32447> : WGS 72BE / UTM zone 47N
 <epsg32448> : WGS 72BE / UTM zone 48N
 <epsg32449> : WGS 72BE / UTM zone 49N
 <epsg32450> : WGS 72BE / UTM zone 50N
 <epsg32451> : WGS 72BE / UTM zone 51N
 <epsg32452> : WGS 72BE / UTM zone 52N
 <epsg32453> : WGS 72BE / UTM zone 53N
 <epsg32454> : WGS 72BE / UTM zone 54N
 <epsg32455> : WGS 72BE / UTM zone 55N
 <epsg32456> : WGS 72BE / UTM zone 56N
 <epsg32457> : WGS 72BE / UTM zone 57N
 <epsg32458> : WGS 72BE / UTM zone 58N
 <epsg32459> : WGS 72BE / UTM zone 59N
 <epsg32460> : WGS 72BE / UTM zone 60N
 <epsg32501> : WGS 72BE / UTM zone 1S
 <epsg32502> : WGS 72BE / UTM zone 2S
 <epsg32503> : WGS 72BE / UTM zone 3S
 <epsg32504> : WGS 72BE / UTM zone 4S
 <epsg32505> : WGS 72BE / UTM zone 5S
 <epsg32506> : WGS 72BE / UTM zone 6S
 <epsg32507> : WGS 72BE / UTM zone 7S
 <epsg32508> : WGS 72BE / UTM zone 8S
 <epsg32509> : WGS 72BE / UTM zone 9S
 <epsg32510> : WGS 72BE / UTM zone 10S
 <epsg32511> : WGS 72BE / UTM zone 11S
 <epsg32512> : WGS 72BE / UTM zone 12S
 <epsg32513> : WGS 72BE / UTM zone 13S
 <epsg32514> : WGS 72BE / UTM zone 14S
 <epsg32515> : WGS 72BE / UTM zone 15S
 <epsg32516> : WGS 72BE / UTM zone 16S
 <epsg32517> : WGS 72BE / UTM zone 17S
 <epsg32518> : WGS 72BE / UTM zone 18S
 <epsg32519> : WGS 72BE / UTM zone 19S
 <epsg32520> : WGS 72BE / UTM zone 20S
 <epsg32521> : WGS 72BE / UTM zone 21S
 <epsg32522> : WGS 72BE / UTM zone 22S
 <epsg32523> : WGS 72BE / UTM zone 23S
 <epsg32524> : WGS 72BE / UTM zone 24S
 <epsg32525> : WGS 72BE / UTM zone 25S
 <epsg32526> : WGS 72BE / UTM zone 26S
 <epsg32527> : WGS 72BE / UTM zone 27S
 <epsg32528> : WGS 72BE / UTM zone 28S
 <epsg32529> : WGS 72BE / UTM zone 29S
 <epsg32530> : WGS 72BE / UTM zone 30S
 <epsg32531> : WGS 72BE / UTM zone 31S
 <epsg32532> : WGS 72BE / UTM zone 32S
 <epsg32533> : WGS 72BE / UTM zone 33S
 <epsg32534> : WGS 72BE / UTM zone 34S
 <epsg32535> : WGS 72BE / UTM zone 35S
 <epsg32536> : WGS 72BE / UTM zone 36S
 <epsg32537> : WGS 72BE / UTM zone 37S
 <epsg32538> : WGS 72BE / UTM zone 38S
 <epsg32539> : WGS 72BE / UTM zone 39S
 <epsg32540> : WGS 72BE / UTM zone 40S
 <epsg32541> : WGS 72BE / UTM zone 41S
 <epsg32542> : WGS 72BE / UTM zone 42S
 <epsg32543> : WGS 72BE / UTM zone 43S
 <epsg32544> : WGS 72BE / UTM zone 44S
 <epsg32545> : WGS 72BE / UTM zone 45S
 <epsg32546> : WGS 72BE / UTM zone 46S
 <epsg32547> : WGS 72BE / UTM zone 47S
 <epsg32548> : WGS 72BE / UTM zone 48S
 <epsg32549> : WGS 72BE / UTM zone 49S
 <epsg32550> : WGS 72BE / UTM zone 50S
 <epsg32551> : WGS 72BE / UTM zone 51S
 <epsg32552> : WGS 72BE / UTM zone 52S
 <epsg32553> : WGS 72BE / UTM zone 53S
 <epsg32554> : WGS 72BE / UTM zone 54S
 <epsg32555> : WGS 72BE / UTM zone 55S
 <epsg32556> : WGS 72BE / UTM zone 56S
 <epsg32557> : WGS 72BE / UTM zone 57S
 <epsg32558> : WGS 72BE / UTM zone 58S
 <epsg32559> : WGS 72BE / UTM zone 59S
 <epsg32560> : WGS 72BE / UTM zone 60S
 <epsg32601> : WGS 84 / UTM zone 1N
 <epsg32602> : WGS 84 / UTM zone 2N
 <epsg32603> : WGS 84 / UTM zone 3N
 <epsg32604> : WGS 84 / UTM zone 4N
 <epsg32605> : WGS 84 / UTM zone 5N
 <epsg32606> : WGS 84 / UTM zone 6N
 <epsg32607> : WGS 84 / UTM zone 7N
 <epsg32608> : WGS 84 / UTM zone 8N
 <epsg32609> : WGS 84 / UTM zone 9N
 <epsg32610> : WGS 84 / UTM zone 10N
 <epsg32611> : WGS 84 / UTM zone 11N
 <epsg32612> : WGS 84 / UTM zone 12N
 <epsg32613> : WGS 84 / UTM zone 13N
 <epsg32614> : WGS 84 / UTM zone 14N
 <epsg32615> : WGS 84 / UTM zone 15N
 <epsg32616> : WGS 84 / UTM zone 16N
 <epsg32617> : WGS 84 / UTM zone 17N
 <epsg32618> : WGS 84 / UTM zone 18N
 <epsg32619> : WGS 84 / UTM zone 19N
 <epsg32620> : WGS 84 / UTM zone 20N
 <epsg32621> : WGS 84 / UTM zone 21N
 <epsg32622> : WGS 84 / UTM zone 22N
 <epsg32623> : WGS 84 / UTM zone 23N
 <epsg32624> : WGS 84 / UTM zone 24N
 <epsg32625> : WGS 84 / UTM zone 25N
 <epsg32626> : WGS 84 / UTM zone 26N
 <epsg32627> : WGS 84 / UTM zone 27N
 <epsg32628> : WGS 84 / UTM zone 28N
 <epsg32629> : WGS 84 / UTM zone 29N
 <epsg32630> : WGS 84 / UTM zone 30N
 <epsg32631> : WGS 84 / UTM zone 31N
 <epsg32632> : WGS 84 / UTM zone 32N
 <epsg32633> : WGS 84 / UTM zone 33N
 <epsg32634> : WGS 84 / UTM zone 34N
 <epsg32635> : WGS 84 / UTM zone 35N
 <epsg32636> : WGS 84 / UTM zone 36N
 <epsg32637> : WGS 84 / UTM zone 37N
 <epsg32638> : WGS 84 / UTM zone 38N
 <epsg32639> : WGS 84 / UTM zone 39N
 <epsg32640> : WGS 84 / UTM zone 40N
 <epsg32641> : WGS 84 / UTM zone 41N
 <epsg32642> : WGS 84 / UTM zone 42N
 <epsg32643> : WGS 84 / UTM zone 43N
 <epsg32644> : WGS 84 / UTM zone 44N
 <epsg32645> : WGS 84 / UTM zone 45N
 <epsg32646> : WGS 84 / UTM zone 46N
 <epsg32647> : WGS 84 / UTM zone 47N
 <epsg32648> : WGS 84 / UTM zone 48N
 <epsg32649> : WGS 84 / UTM zone 49N
 <epsg32650> : WGS 84 / UTM zone 50N
 <epsg32651> : WGS 84 / UTM zone 51N
 <epsg32652> : WGS 84 / UTM zone 52N
 <epsg32653> : WGS 84 / UTM zone 53N
 <epsg32654> : WGS 84 / UTM zone 54N
 <epsg32655> : WGS 84 / UTM zone 55N
 <epsg32656> : WGS 84 / UTM zone 56N
 <epsg32657> : WGS 84 / UTM zone 57N
 <epsg32658> : WGS 84 / UTM zone 58N
 <epsg32659> : WGS 84 / UTM zone 59N
 <epsg32660> : WGS 84 / UTM zone 60N
 <epsg32661> : WGS 84 / UPS North
 <epsg32662> : WGS 84 / Plate Carree
 <epsg32664> : WGS 84 / BLM 14N (ftUS)
 <epsg32665> : WGS 84 / BLM 15N (ftUS)
 <epsg32666> : WGS 84 / BLM 16N (ftUS)
 <epsg32667> : WGS 84 / BLM 17N (ftUS)
 <epsg32701> : WGS 84 / UTM zone 1S
 <epsg32702> : WGS 84 / UTM zone 2S
 <epsg32703> : WGS 84 / UTM zone 3S
 <epsg32704> : WGS 84 / UTM zone 4S
 <epsg32705> : WGS 84 / UTM zone 5S
 <epsg32706> : WGS 84 / UTM zone 6S
 <epsg32707> : WGS 84 / UTM zone 7S
 <epsg32708> : WGS 84 / UTM zone 8S
 <epsg32709> : WGS 84 / UTM zone 9S
 <epsg32710> : WGS 84 / UTM zone 10S
 <epsg32711> : WGS 84 / UTM zone 11S
 <epsg32712> : WGS 84 / UTM zone 12S
 <epsg32713> : WGS 84 / UTM zone 13S
 <epsg32714> : WGS 84 / UTM zone 14S
 <epsg32715> : WGS 84 / UTM zone 15S
 <epsg32716> : WGS 84 / UTM zone 16S
 <epsg32717> : WGS 84 / UTM zone 17S
 <epsg32718> : WGS 84 / UTM zone 18S
 <epsg32719> : WGS 84 / UTM zone 19S
 <epsg32720> : WGS 84 / UTM zone 20S
 <epsg32721> : WGS 84 / UTM zone 21S
 <epsg32722> : WGS 84 / UTM zone 22S
 <epsg32723> : WGS 84 / UTM zone 23S
 <epsg32724> : WGS 84 / UTM zone 24S
 <epsg32725> : WGS 84 / UTM zone 25S
 <epsg32726> : WGS 84 / UTM zone 26S
 <epsg32727> : WGS 84 / UTM zone 27S
 <epsg32728> : WGS 84 / UTM zone 28S
 <epsg32729> : WGS 84 / UTM zone 29S
 <epsg32730> : WGS 84 / UTM zone 30S
 <epsg32731> : WGS 84 / UTM zone 31S
 <epsg32732> : WGS 84 / UTM zone 32S
 <epsg32733> : WGS 84 / UTM zone 33S
 <epsg32734> : WGS 84 / UTM zone 34S
 <epsg32735> : WGS 84 / UTM zone 35S
 <epsg32736> : WGS 84 / UTM zone 36S
 <epsg32737> : WGS 84 / UTM zone 37S
 <epsg32738> : WGS 84 / UTM zone 38S
 <epsg32739> : WGS 84 / UTM zone 39S
 <epsg32740> : WGS 84 / UTM zone 40S
 <epsg32741> : WGS 84 / UTM zone 41S
 <epsg32742> : WGS 84 / UTM zone 42S
 <epsg32743> : WGS 84 / UTM zone 43S
 <epsg32744> : WGS 84 / UTM zone 44S
 <epsg32745> : WGS 84 / UTM zone 45S
 <epsg32746> : WGS 84 / UTM zone 46S
 <epsg32747> : WGS 84 / UTM zone 47S
 <epsg32748> : WGS 84 / UTM zone 48S
 <epsg32749> : WGS 84 / UTM zone 49S
 <epsg32750> : WGS 84 / UTM zone 50S
 <epsg32751> : WGS 84 / UTM zone 51S
 <epsg32752> : WGS 84 / UTM zone 52S
 <epsg32753> : WGS 84 / UTM zone 53S
 <epsg32754> : WGS 84 / UTM zone 54S
 <epsg32755> : WGS 84 / UTM zone 55S
 <epsg32756> : WGS 84 / UTM zone 56S
 <epsg32757> : WGS 84 / UTM zone 57S
 <epsg32758> : WGS 84 / UTM zone 58S
 <epsg32759> : WGS 84 / UTM zone 59S
 <epsg32760> : WGS 84 / UTM zone 60S
 <epsg32761> : WGS 84 / UPS South
 <epsg32766> : WGS 84 / TM 36 SE

 ---------------------------------------------------------
 Additional EPSG-like projection definitions
 ---------------------------------------------------------
 OGC-defined extended codes  (41000--41999) see http://www.digitalearth.gov/wmt/auto.html
 ---------------------------------------------------------
 <epsg41001> : WGS84 / Simple Mercator

 ---------------------------------------------------------
 CubeWerx-defined extended codes  (42100--42199)
 ---------------------------------------------------------
 <epsg42101> : WGS 84 / LCC Canada
 <epsg42102> : NAD83 / BC Albers (this has been superceeded but is kept for compatibility)
 <epsg42103> : WGS 84 / LCC USA
 <epsg42103> : NAD83 / MTM zone 8 Québec
 <epsg42105> : WGS84 / Merc NorthAm
 <epsg42106> : WGS84 / Lambert Azim Mozambique

 ---------------------------------------------------------
 CubeWerx-customer definitions  (42300--42399)
 ---------------------------------------------------------
 <epsg42301> : NAD27 / Polar Stereographic / CM=-98
 <epsg42302>JapanOrtho.09 09
 <epsg42303> : NAD83 / Albers NorthAm
 <epsg42304> : NAD83 / NRCan LCC Canada
 <epsg42305> : France_II
 <epsg42306> : NAD83/QC_LCC
 <epsg42307> : NAD83 / Texas Central - feet
 <epsg42308> : NAD27 / California Albers
 <epsg42309> : NAD 83 / LCC Canada AVHRR-2
 <epsg42310> : WGS84+GRS80 / Mercator
 <epsg42311> : NAD83 / LCC Statcan

 ---------------------------------------------------------
 ESRI projection definitions
 ---------------------------------------------------------
 <esri2000> : Anguilla 1957 / British West Indies Grid
 <esri2001> : Antigua 1943 / British West Indies Grid
 <esri2002> : Dominica 1945 / British West Indies Grid
 <esri2003> : Grenada 1953 / British West Indies Grid
 <esri2004> : Montserrat 58 / British West Indies Grid
 <esri2005> : St Kitts 1955 / British West Indies Grid
 <esri2006> : St Lucia 1955 / British West Indies Grid
 <esri2007> : St Vincent 45 / British West Indies Grid
 <esri2008> : NAD27(CGQ77) / SCoPQ zone 2
 <esri2009> : NAD27(CGQ77) / SCoPQ zone 3
 <esri2010> : NAD27(CGQ77) / SCoPQ zone 4
 <esri2011> : NAD27(CGQ77) / SCoPQ zone 5
 <esri2012> : NAD27(CGQ77) / SCoPQ zone 6
 <esri2013> : NAD27(CGQ77) / SCoPQ zone 7
 <esri2014> : NAD27(CGQ77) / SCoPQ zone 8
 <esri2015> : NAD27(CGQ77) / SCoPQ zone 9
 <esri2016> : NAD27(CGQ77) / SCoPQ zone 10
 <esri2017> : NAD27(76) / MTM zone 8
 <esri2018> : NAD27(76) / MTM zone 9
 <esri2019> : NAD27(76) / MTM zone 10
 <esri2020> : NAD27(76) / MTM zone 11
 <esri2021> : NAD27(76) / MTM zone 12
 <esri2022> : NAD27(76) / MTM zone 13
 <esri2023> : NAD27(76) / MTM zone 14
 <esri2024> : NAD27(76) / MTM zone 15
 <esri2025> : NAD27(76) / MTM zone 16
 <esri2026> : NAD27(76) / MTM zone 17
 <esri2027> : NAD27(76) / UTM zone 15N
 <esri2028> : NAD27(76) / UTM zone 16N
 <esri2029> : NAD27(76) / UTM zone 17N
 <esri2030> : NAD27(76) / UTM zone 18N
 <esri2031> : NAD27(CGQ77) / UTM zone 17N
 <esri2032> : NAD27(CGQ77) / UTM zone 18N
 <esri2033> : NAD27(CGQ77) / UTM zone 19N
 <esri2034> : NAD27(CGQ77) / UTM zone 20N
 <esri2035> : NAD27(CGQ77) / UTM zone 21N
 <esri2036> : NAD83(CSRS98) / New Brunswick Stereo
 <esri2037> : NAD83(CSRS98) / UTM zone 19N
 <esri2038> : NAD83(CSRS98) / UTM zone 20N
 <esri2039> : Israel / Israeli TM Grid
 <esri2040> : Locodjo 1965 / UTM zone 30N
 <esri2041> : Abidjan 1987 / UTM zone 30N
 <esri2042> : Locodjo 1965 / UTM zone 29N
 <esri2043> : Abidjan 1987 / UTM zone 29N
 <esri2044> : Hanoi 1972 / Gauss-Kruger zone 18
 <esri2045> : Hanoi 1972 / Gauss-Kruger zone 19
 <esri2056> : CH1903+ / LV95
 <esri2057> : Rassadiran / Nakhl e Taqi
 <esri2058> : ED50(ED77) / UTM zone 38N
 <esri2059> : ED50(ED77) / UTM zone 39N
 <esri2060> : ED50(ED77) / UTM zone 40N
 <esri2061> : ED50(ED77) / UTM zone 41N
 <esri2062> : Madrid 1870 (Madrid) / Spain
 <esri2063> : Dabola 1981 / UTM zone 28N
 <esri2064> : Dabola 1981 / UTM zone 29N
 <esri2065> : S-JTSK (Ferro) / Krovak
 <esri2066> : Mount Dillon / Tobago Grid
 <esri2067> : Naparima 1955 / UTM zone 20N
 <esri2068> : ELD79 / Libya zone 5
 <esri2069> : ELD79 / Libya zone 6
 <esri2070> : ELD79 / Libya zone 7
 <esri2071> : ELD79 / Libya zone 8
 <esri2072> : ELD79 / Libya zone 9
 <esri2073> : ELD79 / Libya zone 10
 <esri2074> : ELD79 / Libya zone 11
 <esri2075> : ELD79 / Libya zone 12
 <esri2076> : ELD79 / Libya zone 13
 <esri2077> : ELD79 / UTM zone 32N
 <esri2078> : ELD79 / UTM zone 33N
 <esri2079> : ELD79 / UTM zone 34N
 <esri2080> : ELD79 / UTM zone 35N
 <esri2081> : Chos Malal 1914 / Argentina zone 2
 <esri2082> : Pampa del Castillo / Argentina zone 2
 <esri2083> : Hito XVIII 1963 / Argentina zone 2
 <esri2084> : Hito XVIII 1963 / UTM zone 19S
 <esri2085> : NAD27 / Cuba Norte
 <esri2086> : NAD27 / Cuba Sur
 <esri2087> : ELD79 / TM 12 NE
 <esri2088> : Carthage / TM 11 NE
 <esri2089> : Yemen NGN96 / UTM zone 38N
 <esri2090> : Yemen NGN96 / UTM zone 39N
 <esri2091> : South Yemen / Gauss Kruger zone 8
 <esri2092> : South Yemen / Gauss Kruger zone 9
 <esri2093> : Hanoi 1972 / GK 106 NE
 <esri2094> : WGS 72BE / TM 106 NE
 <esri2095> : Bissau / UTM zone 28N
 <esri2096> : Korean 1985 / Korea East Belt
 <esri2097> : Korean 1985 / Korea Central Belt
 <esri2098> : Korean 1985 / Korea West Belt
 <esri2099> : Qatar 1948 / Qatar Grid
 <esri2100> : GGRS87 / Greek Grid
 <esri2101> : Lake / Maracaibo Grid M1
 <esri2102> : Lake / Maracaibo Grid
 <esri2103> : Lake / Maracaibo Grid M3
 <esri2104> : Lake / Maracaibo La Rosa Grid
 <esri2105> : NZGD2000 / Mount Eden Circuit 2000
 <esri2106> : NZGD2000 / Bay of Plenty Circuit 2000
 <esri2107> : NZGD2000 / Poverty Bay Circuit 2000
 <esri2108> : NZGD2000 / Hawkes Bay Circuit 2000
 <esri2109> : NZGD2000 / Taranaki Circuit 2000
 <esri2110> : NZGD2000 / Tuhirangi Circuit 2000
 <esri2111> : NZGD2000 / Wanganui Circuit 2000
 <esri2112> : NZGD2000 / Wairarapa Circuit 2000
 <esri2113> : NZGD2000 / Wellington Circuit 2000
 <esri2114> : NZGD2000 / Collingwood Circuit 2000
 <esri2115> : NZGD2000 / Nelson Circuit 2000
 <esri2116> : NZGD2000 / Karamea Circuit 2000
 <esri2117> : NZGD2000 / Buller Circuit 2000
 <esri2118> : NZGD2000 / Grey Circuit 2000
 <esri2119> : NZGD2000 / Amuri Circuit 2000
 <esri2120> : NZGD2000 / Marlborough Circuit 2000
 <esri2121> : NZGD2000 / Hokitika Circuit 2000
 <esri2122> : NZGD2000 / Okarito Circuit 2000
 <esri2123> : NZGD2000 / Jacksons Bay Circuit 2000
 <esri2124> : NZGD2000 / Mount Pleasant Circuit 2000
 <esri2125> : NZGD2000 / Gawler Circuit 2000
 <esri2126> : NZGD2000 / Timaru Circuit 2000
 <esri2127> : NZGD2000 / Lindis Peak Circuit 2000
 <esri2128> : NZGD2000 / Mount Nicholas Circuit 2000
 <esri2129> : NZGD2000 / Mount York Circuit 2000
 <esri2130> : NZGD2000 / Observation Point Circuit 2000
 <esri2131> : NZGD2000 / North Taieri Circuit 2000
 <esri2132> : NZGD2000 / Bluff Circuit 2000
 <esri2133> : NZGD2000 / UTM zone 58S
 <esri2134> : NZGD2000 / UTM zone 59S
 <esri2135> : NZGD2000 / UTM zone 60S
 <esri2136> : Accra / Ghana National Grid
 <esri2137> : Accra / TM 1 NW
 <esri2138> : NAD27(CGQ77) / Quebec Lambert
 <esri2139> : NAD83(CSRS98) / SCoPQ zone 2
 <esri2140> : NAD83(CSRS98) / MTM zone 3
 <esri2141> : NAD83(CSRS98) / MTM zone 4
 <esri2142> : NAD83(CSRS98) / MTM zone 5
 <esri2143> : NAD83(CSRS98) / MTM zone 6
 <esri2144> : NAD83(CSRS98) / MTM zone 7
 <esri2145> : NAD83(CSRS98) / MTM zone 8
 <esri2146> : NAD83(CSRS98) / MTM zone 9
 <esri2147> : NAD83(CSRS98) / MTM zone 10
 <esri2148> : NAD83(CSRS98) / UTM zone 21N
 <esri2149> : NAD83(CSRS98) / UTM zone 18N
 <esri2150> : NAD83(CSRS98) / UTM zone 17N
 <esri2151> : NAD83(CSRS98) / UTM zone 13N
 <esri2152> : NAD83(CSRS98) / UTM zone 12N
 <esri2153> : NAD83(CSRS98) / UTM zone 11N
 <esri2154> : RGF93 / Lambert-93
 <esri2155> : American Samoa 1962 / American Samoa Lambert
 <esri2156> : NAD83(HARN) / UTM zone 59S
 <esri2157> : IRENET95 / Irish Transverse Mercator
 <esri2158> : IRENET95 / UTM zone 29N
 <esri2159> : Sierra Leone 1924 / New Colony Grid
 <esri2160> : Sierra Leone 1924 / New War Office Grid
 <esri2161> : Sierra Leone 1968 / UTM zone 28N
 <esri2162> : Sierra Leone 1968 / UTM zone 29N
 <esri2163> : US National Atlas Equal Area
 <esri2164> : Locodjo 1965 / TM 5 NW
 <esri2165> : Abidjan 1987 / TM 5 NW
 <esri2166> : Pulkovo 1942(83) / Gauss Kruger zone 3
 <esri2167> : Pulkovo 1942(83) / Gauss Kruger zone 4
 <esri2168> : Pulkovo 1942(83) / Gauss Kruger zone 5
 <esri2169> : Luxembourg 1930 / Gauss
 <esri2170> : MGI / Slovenia Grid
 <esri2171> : Pulkovo 1942(58) / Poland zone I
 <esri2172> : Pulkovo 1942(58) / Poland zone II
 <esri2173> : Pulkovo 1942(58) / Poland zone III
 <esri2174> : Pulkovo 1942(58) / Poland zone IV
 <esri2175> : Pulkovo 1942(58) / Poland zone V
 <esri2176> : ETRS89 / Poland CS2000 zone 5
 <esri2177> : ETRS89 / Poland CS2000 zone 6
 <esri2178> : ETRS89 / Poland CS2000 zone 7
 <esri2179> : ETRS89 / Poland CS2000 zone 8
 <esri2180> : ETRS89 / Poland CS92
 <esri2188> : Azores Occidental 1939 / UTM zone 25N
 <esri2189> : Azores Central 1948 / UTM zone 26N
 <esri2190> : Azores Oriental 1940 / UTM zone 26N
 <esri2191> : Madeira 1936 / UTM zone 28N
 <esri2192> : ED50 / France EuroLambert
 <esri2193> : NZGD2000 / New Zealand Transverse Mercator
 <esri2194> : American Samoa 1962 / American Samoa Lambert
 <esri2195> : NAD83(HARN) / UTM zone 2S
 <esri2196> : ETRS89 / Kp2000 Jutland
 <esri2197> : ETRS89 / Kp2000 Zealand
 <esri2198> : ETRS89 / Kp2000 Bornholm
 <esri2199> : Albanian 1987 / Gauss Kruger zone 4
 <esri2200> : ATS77 / New Brunswick Stereographic (ATS77)
 <esri2201> : REGVEN / UTM zone 18N
 <esri2202> : REGVEN / UTM zone 19N
 <esri2203> : REGVEN / UTM zone 20N
 <esri2204> : NAD27 / Tennessee
 <esri2205> : NAD83 / Kentucky North
 <esri2206> : ED50 / 3-degree Gauss-Kruger zone 9
 <esri2207> : ED50 / 3-degree Gauss-Kruger zone 10
 <esri2208> : ED50 / 3-degree Gauss-Kruger zone 11
 <esri2209> : ED50 / 3-degree Gauss-Kruger zone 12
 <esri2210> : ED50 / 3-degree Gauss-Kruger zone 13
 <esri2211> : ED50 / 3-degree Gauss-Kruger zone 14
 <esri2212> : ED50 / 3-degree Gauss-Kruger zone 15
 <esri2213> : ETRS89 / TM 30 NE
 <esri2214> : Douala 1948 / AOF west
 <esri2215> : Manoca 1962 / UTM zone 32N
 <esri2216> : Qornoq 1927 / UTM zone 22N
 <esri2217> : Qornoq 1927 / UTM zone 23N
 <esri2219> : ATS77 / UTM zone 19N
 <esri2220> : ATS77 / UTM zone 20N
 <esri2222> : NAD83 / Arizona East (ft)
 <esri2223> : NAD83 / Arizona Central (ft)
 <esri2224> : NAD83 / Arizona West (ft)
 <esri2225> : NAD83 / California zone 1 (ftUS)
 <esri2226> : NAD83 / California zone 2 (ftUS)
 <esri2227> : NAD83 / California zone 3 (ftUS)
 <esri2228> : NAD83 / California zone 4 (ftUS)
 <esri2229> : NAD83 / California zone 5 (ftUS)
 <esri2230> : NAD83 / California zone 6 (ftUS)
 <esri2231> : NAD83 / Colorado North (ftUS)
 <esri2232> : NAD83 / Colorado Central (ftUS)
 <esri2233> : NAD83 / Colorado South (ftUS)
 <esri2234> : NAD83 / Connecticut (ftUS)
 <esri2235> : NAD83 / Delaware (ftUS)
 <esri2236> : NAD83 / Florida East (ftUS)
 <esri2237> : NAD83 / Florida West (ftUS)
 <esri2238> : NAD83 / Florida North (ftUS)
 <esri2239> : NAD83 / Georgia East (ftUS)
 <esri2240> : NAD83 / Georgia West (ftUS)
 <esri2241> : NAD83 / Idaho East (ftUS)
 <esri2242> : NAD83 / Idaho Central (ftUS)
 <esri2243> : NAD83 / Idaho West (ftUS)
 <esri2244> : NAD83 / Indiana East (ftUS)
 <esri2245> : NAD83 / Indiana West (ftUS)
 <esri2246> : NAD83 / Kentucky North (ftUS)
 <esri2247> : NAD83 / Kentucky South (ftUS)
 <esri2248> : NAD83 / Maryland (ftUS)
 <esri2249> : NAD83 / Massachusetts Mainland (ftUS)
 <esri2250> : NAD83 / Massachusetts Island (ftUS)
 <esri2251> : NAD83 / Michigan North (ft)
 <esri2252> : NAD83 / Michigan Central (ft)
 <esri2253> : NAD83 / Michigan South (ft)
 <esri2254> : NAD83 / Mississippi East (ftUS)
 <esri2255> : NAD83 / Mississippi West (ftUS)
 <esri2256> : NAD83 / Montana (ft)
 <esri2257> : NAD83 / New Mexico East (ftUS)
 <esri2258> : NAD83 / New Mexico Central (ftUS)
 <esri2259> : NAD83 / New Mexico West (ftUS)
 <esri2260> : NAD83 / New York East (ftUS)
 <esri2261> : NAD83 / New York Central (ftUS)
 <esri2262> : NAD83 / New York West (ftUS)
 <esri2263> : NAD83 / New York Long Island (ftUS)
 <esri2264> : NAD83 / North Carolina (ftUS)
 <esri2265> : NAD83 / North Dakota North (ft)
 <esri2266> : NAD83 / North Dakota South (ft)
 <esri2267> : NAD83 / Oklahoma North (ftUS)
 <esri2268> : NAD83 / Oklahoma South (ftUS)
 <esri2269> : NAD83 / Oregon North (ft)
 <esri2270> : NAD83 / Oregon South (ft)
 <esri2271> : NAD83 / Pennsylvania North (ftUS)
 <esri2272> : NAD83 / Pennsylvania South (ftUS)
 <esri2273> : NAD83 / South Carolina (ft)
 <esri2274> : NAD83 / Tennessee (ftUS)
 <esri2275> : NAD83 / Texas North (ftUS)
 <esri2276> : NAD83 / Texas North Central (ftUS)
 <esri2277> : NAD83 / Texas Central (ftUS)
 <esri2278> : NAD83 / Texas South Central (ftUS)
 <esri2279> : NAD83 / Texas South (ftUS)
 <esri2280> : NAD83 / Utah North (ft)
 <esri2281> : NAD83 / Utah Central (ft)
 <esri2282> : NAD83 / Utah South (ft)
 <esri2283> : NAD83 / Virginia North (ftUS)
 <esri2284> : NAD83 / Virginia South (ftUS)
 <esri2285> : NAD83 / Washington North (ftUS)
 <esri2286> : NAD83 / Washington South (ftUS)
 <esri2287> : NAD83 / Wisconsin North (ftUS)
 <esri2288> : NAD83 / Wisconsin Central (ftUS)
 <esri2289> : NAD83 / Wisconsin South (ftUS)
 <esri2290> : ATS77 / Prince Edward Isl. Stereographic (ATS77)
 <esri2291> : NAD83(CSRS98) / Prince Edward Isl. Stereographic (NAD83)
 <esri2292> : NAD83(CSRS98) / Prince Edward Isl. Stereographic (NAD83)
 <esri2294> : ATS77 / MTM Nova Scotia zone 4
 <esri2295> : ATS77 / MTM Nova Scotia zone 5
 <esri2308> : Batavia / TM 109 SE
 <esri2309> : WGS 84 / TM 116 SE
 <esri2310> : WGS 84 / TM 132 SE
 <esri2311> : WGS 84 / TM 6 NE
 <esri2312> : Garoua / UTM zone 33N
 <esri2313> : Kousseri / UTM zone 33N
 <esri2314> : Trinidad 1903 / Trinidad Grid (ftCla)
 <esri2315> : Campo Inchauspe / UTM zone 19S
 <esri2316> : Campo Inchauspe / UTM zone 20S
 <esri2317> : PSAD56 / ICN Regional
 <esri2318> : Ain el Abd / Aramco Lambert
 <esri2319> : ED50 / TM27
 <esri2320> : ED50 / TM30
 <esri2321> : ED50 / TM33
 <esri2322> : ED50 / TM36
 <esri2323> : ED50 / TM39
 <esri2324> : ED50 / TM42
 <esri2325> : ED50 / TM45
 <esri2326> : Hong Kong 1980 Grid System
 <esri2327> : Xian 1980 / Gauss-Kruger zone 13
 <esri2328> : Xian 1980 / Gauss-Kruger zone 14
 <esri2329> : Xian 1980 / Gauss-Kruger zone 15
 <esri2330> : Xian 1980 / Gauss-Kruger zone 16
 <esri2331> : Xian 1980 / Gauss-Kruger zone 17
 <esri2332> : Xian 1980 / Gauss-Kruger zone 18
 <esri2333> : Xian 1980 / Gauss-Kruger zone 19
 <esri2334> : Xian 1980 / Gauss-Kruger zone 20
 <esri2335> : Xian 1980 / Gauss-Kruger zone 21
 <esri2336> : Xian 1980 / Gauss-Kruger zone 22
 <esri2337> : Xian 1980 / Gauss-Kruger zone 23
 <esri2338> : Xian 1980 / Gauss-Kruger CM 75E
 <esri2339> : Xian 1980 / Gauss-Kruger CM 81E
 <esri2340> : Xian 1980 / Gauss-Kruger CM 87E
 <esri2341> : Xian 1980 / Gauss-Kruger CM 93E
 <esri2342> : Xian 1980 / Gauss-Kruger CM 99E
 <esri2343> : Xian 1980 / Gauss-Kruger CM 105E
 <esri2344> : Xian 1980 / Gauss-Kruger CM 111E
 <esri2345> : Xian 1980 / Gauss-Kruger CM 117E
 <esri2346> : Xian 1980 / Gauss-Kruger CM 123E
 <esri2347> : Xian 1980 / Gauss-Kruger CM 129E
 <esri2348> : Xian 1980 / Gauss-Kruger CM 135E
 <esri2349> : Xian 1980 / 3-degree Gauss-Kruger zone 25
 <esri2350> : Xian 1980 / 3-degree Gauss-Kruger zone 26
 <esri2351> : Xian 1980 / 3-degree Gauss-Kruger zone 27
 <esri2352> : Xian 1980 / 3-degree Gauss-Kruger zone 28
 <esri2353> : Xian 1980 / 3-degree Gauss-Kruger zone 29
 <esri2354> : Xian 1980 / 3-degree Gauss-Kruger zone 30
 <esri2355> : Xian 1980 / 3-degree Gauss-Kruger zone 31
 <esri2356> : Xian 1980 / 3-degree Gauss-Kruger zone 32
 <esri2357> : Xian 1980 / 3-degree Gauss-Kruger zone 33
 <esri2358> : Xian 1980 / 3-degree Gauss-Kruger zone 34
 <esri2359> : Xian 1980 / 3-degree Gauss-Kruger zone 35
 <esri2360> : Xian 1980 / 3-degree Gauss-Kruger zone 36
 <esri2361> : Xian 1980 / 3-degree Gauss-Kruger zone 37
 <esri2362> : Xian 1980 / 3-degree Gauss-Kruger zone 38
 <esri2363> : Xian 1980 / 3-degree Gauss-Kruger zone 39
 <esri2364> : Xian 1980 / 3-degree Gauss-Kruger zone 40
 <esri2365> : Xian 1980 / 3-degree Gauss-Kruger zone 41
 <esri2366> : Xian 1980 / 3-degree Gauss-Kruger zone 42
 <esri2367> : Xian 1980 / 3-degree Gauss-Kruger zone 43
 <esri2368> : Xian 1980 / 3-degree Gauss-Kruger zone 44
 <esri2369> : Xian 1980 / 3-degree Gauss-Kruger zone 45
 <esri2370> : Xian 1980 / 3-degree Gauss-Kruger CM 75E
 <esri2371> : Xian 1980 / 3-degree Gauss-Kruger CM 78E
 <esri2372> : Xian 1980 / 3-degree Gauss-Kruger CM 81E
 <esri2373> : Xian 1980 / 3-degree Gauss-Kruger CM 84E
 <esri2374> : Xian 1980 / 3-degree Gauss-Kruger CM 87E
 <esri2375> : Xian 1980 / 3-degree Gauss-Kruger CM 90E
 <esri2376> : Xian 1980 / 3-degree Gauss-Kruger CM 93E
 <esri2377> : Xian 1980 / 3-degree Gauss-Kruger CM 96E
 <esri2378> : Xian 1980 / 3-degree Gauss-Kruger CM 99E
 <esri2379> : Xian 1980 / 3-degree Gauss-Kruger CM 102E
 <esri2380> : Xian 1980 / 3-degree Gauss-Kruger CM 105E
 <esri2381> : Xian 1980 / 3-degree Gauss-Kruger CM 108E
 <esri2382> : Xian 1980 / 3-degree Gauss-Kruger CM 111E
 <esri2383> : Xian 1980 / 3-degree Gauss-Kruger CM 114E
 <esri2384> : Xian 1980 / 3-degree Gauss-Kruger CM 117E
 <esri2385> : Xian 1980 / 3-degree Gauss-Kruger CM 120E
 <esri2386> : Xian 1980 / 3-degree Gauss-Kruger CM 123E
 <esri2387> : Xian 1980 / 3-degree Gauss-Kruger CM 126E
 <esri2388> : Xian 1980 / 3-degree Gauss-Kruger CM 129E
 <esri2389> : Xian 1980 / 3-degree Gauss-Kruger CM 132E
 <esri2390> : Xian 1980 / 3-degree Gauss-Kruger CM 135E
 <esri2391> : KKJ / Finland zone 1
 <esri2392> : KKJ / Finland zone 2
 <esri2393> : KKJ / Finland Uniform Coordinate System
 <esri2394> : KKJ / Finland zone 4
 <esri2395> : South Yemen / Gauss-Kruger zone 8
 <esri2396> : South Yemen / Gauss-Kruger zone 9
 <esri2397> : Pulkovo 1942(83) / Gauss-Kruger zone 3
 <esri2398> : Pulkovo 1942(83) / Gauss-Kruger zone 4
 <esri2399> : Pulkovo 1942(83) / Gauss-Kruger zone 5
 <esri2400> : RT90 2.5 gon W
 <esri2401> : Beijing 1954 / 3-degree Gauss-Kruger zone 25
 <esri2402> : Beijing 1954 / 3-degree Gauss-Kruger zone 26
 <esri2403> : Beijing 1954 / 3-degree Gauss-Kruger zone 27
 <esri2404> : Beijing 1954 / 3-degree Gauss-Kruger zone 28
 <esri2405> : Beijing 1954 / 3-degree Gauss-Kruger zone 29
 <esri2406> : Beijing 1954 / 3-degree Gauss-Kruger zone 30
 <esri2407> : Beijing 1954 / 3-degree Gauss-Kruger zone 31
 <esri2408> : Beijing 1954 / 3-degree Gauss-Kruger zone 32
 <esri2409> : Beijing 1954 / 3-degree Gauss-Kruger zone 33
 <esri2410> : Beijing 1954 / 3-degree Gauss-Kruger zone 34
 <esri2411> : Beijing 1954 / 3-degree Gauss-Kruger zone 35
 <esri2412> : Beijing 1954 / 3-degree Gauss-Kruger zone 36
 <esri2413> : Beijing 1954 / 3-degree Gauss-Kruger zone 37
 <esri2414> : Beijing 1954 / 3-degree Gauss-Kruger zone 38
 <esri2415> : Beijing 1954 / 3-degree Gauss-Kruger zone 39
 <esri2416> : Beijing 1954 / 3-degree Gauss-Kruger zone 40
 <esri2417> : Beijing 1954 / 3-degree Gauss-Kruger zone 41
 <esri2418> : Beijing 1954 / 3-degree Gauss-Kruger zone 42
 <esri2419> : Beijing 1954 / 3-degree Gauss-Kruger zone 43
 <esri2420> : Beijing 1954 / 3-degree Gauss-Kruger zone 44
 <esri2421> : Beijing 1954 / 3-degree Gauss-Kruger zone 45
 <esri2422> : Beijing 1954 / 3-degree Gauss-Kruger CM 75E
 <esri2423> : Beijing 1954 / 3-degree Gauss-Kruger CM 78E
 <esri2424> : Beijing 1954 / 3-degree Gauss-Kruger CM 81E
 <esri2425> : Beijing 1954 / 3-degree Gauss-Kruger CM 84E
 <esri2426> : Beijing 1954 / 3-degree Gauss-Kruger CM 87E
 <esri2427> : Beijing 1954 / 3-degree Gauss-Kruger CM 90E
 <esri2428> : Beijing 1954 / 3-degree Gauss-Kruger CM 93E
 <esri2429> : Beijing 1954 / 3-degree Gauss-Kruger CM 96E
 <esri2430> : Beijing 1954 / 3-degree Gauss-Kruger CM 99E
 <esri2431> : Beijing 1954 / 3-degree Gauss-Kruger CM 102E
 <esri2432> : Beijing 1954 / 3-degree Gauss-Kruger CM 105E
 <esri2433> : Beijing 1954 / 3-degree Gauss-Kruger CM 108E
 <esri2434> : Beijing 1954 / 3-degree Gauss-Kruger CM 111E
 <esri2435> : Beijing 1954 / 3-degree Gauss-Kruger CM 114E
 <esri2436> : Beijing 1954 / 3-degree Gauss-Kruger CM 117E
 <esri2437> : Beijing 1954 / 3-degree Gauss-Kruger CM 120E
 <esri2438> : Beijing 1954 / 3-degree Gauss-Kruger CM 123E
 <esri2439> : Beijing 1954 / 3-degree Gauss-Kruger CM 126E
 <esri2440> : Beijing 1954 / 3-degree Gauss-Kruger CM 129E
 <esri2441> : Beijing 1954 / 3-degree Gauss-Kruger CM 132E
 <esri2442> : Beijing 1954 / 3-degree Gauss-Kruger CM 135E
 <esri2443> : JGD2000 / Japan Plane Rectangular CS I
 <esri2444> : JGD2000 / Japan Plane Rectangular CS II
 <esri2445> : JGD2000 / Japan Plane Rectangular CS III
 <esri2446> : JGD2000 / Japan Plane Rectangular CS IV
 <esri2447> : JGD2000 / Japan Plane Rectangular CS V
 <esri2448> : JGD2000 / Japan Plane Rectangular CS VI
 <esri2449> : JGD2000 / Japan Plane Rectangular CS VII
 <esri2450> : JGD2000 / Japan Plane Rectangular CS VIII
 <esri2451> : JGD2000 / Japan Plane Rectangular CS IX
 <esri2452> : JGD2000 / Japan Plane Rectangular CS X
 <esri2453> : JGD2000 / Japan Plane Rectangular CS XI
 <esri2454> : JGD2000 / Japan Plane Rectangular CS XII
 <esri2455> : JGD2000 / Japan Plane Rectangular CS XIII
 <esri2456> : JGD2000 / Japan Plane Rectangular CS XIV
 <esri2457> : JGD2000 / Japan Plane Rectangular CS XV
 <esri2458> : JGD2000 / Japan Plane Rectangular CS XVI
 <esri2459> : JGD2000 / Japan Plane Rectangular CS XVII
 <esri2460> : JGD2000 / Japan Plane Rectangular CS XVIII
 <esri2461> : JGD2000 / Japan Plane Rectangular CS XIX
 <esri2462> : Albanian 1987 / Gauss-Kruger zone 4
 <esri2463> : Pulkovo 1995 / Gauss-Kruger CM 21E
 <esri2464> : Pulkovo 1995 / Gauss-Kruger CM 27E
 <esri2465> : Pulkovo 1995 / Gauss-Kruger CM 33E
 <esri2466> : Pulkovo 1995 / Gauss-Kruger CM 39E
 <esri2467> : Pulkovo 1995 / Gauss-Kruger CM 45E
 <esri2468> : Pulkovo 1995 / Gauss-Kruger CM 51E
 <esri2469> : Pulkovo 1995 / Gauss-Kruger CM 57E
 <esri2470> : Pulkovo 1995 / Gauss-Kruger CM 63E
 <esri2471> : Pulkovo 1995 / Gauss-Kruger CM 69E
 <esri2472> : Pulkovo 1995 / Gauss-Kruger CM 75E
 <esri2473> : Pulkovo 1995 / Gauss-Kruger CM 81E
 <esri2474> : Pulkovo 1995 / Gauss-Kruger CM 87E
 <esri2475> : Pulkovo 1995 / Gauss-Kruger CM 93E
 <esri2476> : Pulkovo 1995 / Gauss-Kruger CM 99E
 <esri2477> : Pulkovo 1995 / Gauss-Kruger CM 105E
 <esri2478> : Pulkovo 1995 / Gauss-Kruger CM 111E
 <esri2479> : Pulkovo 1995 / Gauss-Kruger CM 117E
 <esri2480> : Pulkovo 1995 / Gauss-Kruger CM 123E
 <esri2481> : Pulkovo 1995 / Gauss-Kruger CM 129E
 <esri2482> : Pulkovo 1995 / Gauss-Kruger CM 135E
 <esri2483> : Pulkovo 1995 / Gauss-Kruger CM 141E
 <esri2484> : Pulkovo 1995 / Gauss-Kruger CM 147E
 <esri2485> : Pulkovo 1995 / Gauss-Kruger CM 153E
 <esri2486> : Pulkovo 1995 / Gauss-Kruger CM 159E
 <esri2487> : Pulkovo 1995 / Gauss-Kruger CM 165E
 <esri2488> : Pulkovo 1995 / Gauss-Kruger CM 171E
 <esri2489> : Pulkovo 1995 / Gauss-Kruger CM 177E
 <esri2490> : Pulkovo 1995 / Gauss-Kruger CM 177W
 <esri2491> : Pulkovo 1995 / Gauss-Kruger CM 171W
 <esri2492> : Pulkovo 1942 / Gauss-Kruger CM 9E
 <esri2493> : Pulkovo 1942 / Gauss-Kruger CM 15E
 <esri2494> : Pulkovo 1942 / Gauss-Kruger CM 21E
 <esri2495> : Pulkovo 1942 / Gauss-Kruger CM 27E
 <esri2496> : Pulkovo 1942 / Gauss-Kruger CM 33E
 <esri2497> : Pulkovo 1942 / Gauss-Kruger CM 39E
 <esri2498> : Pulkovo 1942 / Gauss-Kruger CM 45E
 <esri2499> : Pulkovo 1942 / Gauss-Kruger CM 51E
 <esri2500> : Pulkovo 1942 / Gauss-Kruger CM 57E
 <esri2501> : Pulkovo 1942 / Gauss-Kruger CM 63E
 <esri2502> : Pulkovo 1942 / Gauss-Kruger CM 69E
 <esri2503> : Pulkovo 1942 / Gauss-Kruger CM 75E
 <esri2504> : Pulkovo 1942 / Gauss-Kruger CM 81E
 <esri2505> : Pulkovo 1942 / Gauss-Kruger CM 87E
 <esri2506> : Pulkovo 1942 / Gauss-Kruger CM 93E
 <esri2507> : Pulkovo 1942 / Gauss-Kruger CM 99E
 <esri2508> : Pulkovo 1942 / Gauss-Kruger CM 105E
 <esri2509> : Pulkovo 1942 / Gauss-Kruger CM 111E
 <esri2510> : Pulkovo 1942 / Gauss-Kruger CM 117E
 <esri2511> : Pulkovo 1942 / Gauss-Kruger CM 123E
 <esri2512> : Pulkovo 1942 / Gauss-Kruger CM 129E
 <esri2513> : Pulkovo 1942 / Gauss-Kruger CM 135E
 <esri2514> : Pulkovo 1942 / Gauss-Kruger CM 141E
 <esri2515> : Pulkovo 1942 / Gauss-Kruger CM 147E
 <esri2516> : Pulkovo 1942 / Gauss-Kruger CM 153E
 <esri2517> : Pulkovo 1942 / Gauss-Kruger CM 159E
 <esri2518> : Pulkovo 1942 / Gauss-Kruger CM 165E
 <esri2519> : Pulkovo 1942 / Gauss-Kruger CM 171E
 <esri2520> : Pulkovo 1942 / Gauss-Kruger CM 177E
 <esri2521> : Pulkovo 1942 / Gauss-Kruger CM 177W
 <esri2522> : Pulkovo 1942 / Gauss-Kruger CM 171W
 <esri2523> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 7
 <esri2524> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 8
 <esri2525> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 9
 <esri2526> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 10
 <esri2527> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 11
 <esri2528> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 12
 <esri2529> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 13
 <esri2530> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 14
 <esri2531> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 15
 <esri2532> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 16
 <esri2533> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 17
 <esri2534> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 18
 <esri2535> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 19
 <esri2536> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 20
 <esri2537> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 21
 <esri2538> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 22
 <esri2539> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 23
 <esri2540> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 24
 <esri2541> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 25
 <esri2542> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 26
 <esri2543> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 27
 <esri2544> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 28
 <esri2545> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 29
 <esri2546> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 30
 <esri2547> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 31
 <esri2548> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 32
 <esri2549> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 33
 <esri2550> : Samboja / UTM zone 50S
 <esri2551> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 34
 <esri2552> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 35
 <esri2553> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 36
 <esri2554> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 37
 <esri2555> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 38
 <esri2556> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 39
 <esri2557> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 40
 <esri2558> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 41
 <esri2559> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 42
 <esri2560> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 43
 <esri2561> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 44
 <esri2562> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 45
 <esri2563> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 46
 <esri2564> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 47
 <esri2565> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 48
 <esri2566> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 49
 <esri2567> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 50
 <esri2568> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 51
 <esri2569> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 52
 <esri2570> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 53
 <esri2571> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 54
 <esri2572> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 55
 <esri2573> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 56
 <esri2574> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 57
 <esri2575> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 58
 <esri2576> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 59
 <esri2577> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 60
 <esri2578> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 61
 <esri2579> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 62
 <esri2580> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 63
 <esri2581> : Pulkovo 1942 / 3-degree Gauss-Kruger zone 64
 <esri2582> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 21E
 <esri2583> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 24E
 <esri2584> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 27E
 <esri2585> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 30E
 <esri2586> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 33E
 <esri2587> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 36E
 <esri2588> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 39E
 <esri2589> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 42E
 <esri2590> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 45E
 <esri2591> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 48E
 <esri2592> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 51E
 <esri2593> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 54E
 <esri2594> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 57E
 <esri2595> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 60E
 <esri2596> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 63E
 <esri2597> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 66E
 <esri2598> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 69E
 <esri2599> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 72E
 <esri2600> : Lietuvos Koordinoei Sistema 1994
 <esri2601> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 75E
 <esri2602> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 78E
 <esri2603> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 81E
 <esri2604> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 84E
 <esri2605> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 87E
 <esri2606> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 90E
 <esri2607> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 93E
 <esri2608> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 96E
 <esri2609> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 99E
 <esri2610> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 102E
 <esri2611> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 105E
 <esri2612> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 108E
 <esri2613> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 111E
 <esri2614> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 114E
 <esri2615> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 117E
 <esri2616> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 120E
 <esri2617> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 123E
 <esri2618> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 126E
 <esri2619> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 129E
 <esri2620> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 132E
 <esri2621> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 135E
 <esri2622> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 138E
 <esri2623> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 141E
 <esri2624> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 144E
 <esri2625> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 147E
 <esri2626> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 150E
 <esri2627> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 153E
 <esri2628> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 156E
 <esri2629> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 159E
 <esri2630> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 162E
 <esri2631> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 165E
 <esri2632> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 168E
 <esri2633> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 171E
 <esri2634> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 174E
 <esri2635> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 177E
 <esri2636> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 180E
 <esri2637> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 177W
 <esri2638> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 174W
 <esri2639> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 171W
 <esri2640> : Pulkovo 1942 / 3-degree Gauss-Kruger CM 168W
 <esri2641> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 7
 <esri2642> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 8
 <esri2643> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 9
 <esri2644> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 10
 <esri2645> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 11
 <esri2646> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 12
 <esri2647> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 13
 <esri2648> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 14
 <esri2649> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 15
 <esri2650> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 16
 <esri2651> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 17
 <esri2652> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 18
 <esri2653> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 19
 <esri2654> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 20
 <esri2655> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 21
 <esri2656> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 22
 <esri2657> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 23
 <esri2658> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 24
 <esri2659> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 25
 <esri2660> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 26
 <esri2661> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 27
 <esri2662> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 28
 <esri2663> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 29
 <esri2664> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 30
 <esri2665> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 31
 <esri2666> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 32
 <esri2667> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 33
 <esri2668> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 34
 <esri2669> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 35
 <esri2670> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 36
 <esri2671> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 37
 <esri2672> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 38
 <esri2673> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 39
 <esri2674> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 40
 <esri2675> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 41
 <esri2676> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 42
 <esri2677> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 43
 <esri2678> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 44
 <esri2679> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 45
 <esri2680> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 46
 <esri2681> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 47
 <esri2682> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 48
 <esri2683> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 49
 <esri2684> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 50
 <esri2685> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 51
 <esri2686> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 52
 <esri2687> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 53
 <esri2688> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 54
 <esri2689> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 55
 <esri2690> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 56
 <esri2691> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 57
 <esri2692> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 58
 <esri2693> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 59
 <esri2694> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 60
 <esri2695> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 61
 <esri2696> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 62
 <esri2697> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 63
 <esri2698> : Pulkovo 1995 / 3-degree Gauss-Kruger zone 64
 <esri2699> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 21E
 <esri2700> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 24E
 <esri2701> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 27E
 <esri2702> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 30E
 <esri2703> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 33E
 <esri2704> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 36E
 <esri2705> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 39E
 <esri2706> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 42E
 <esri2707> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 45E
 <esri2708> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 48E
 <esri2709> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 51E
 <esri2710> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 54E
 <esri2711> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 57E
 <esri2712> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 60E
 <esri2713> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 63E
 <esri2714> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 66E
 <esri2715> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 69E
 <esri2716> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 72E
 <esri2717> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 75E
 <esri2718> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 78E
 <esri2719> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 81E
 <esri2720> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 84E
 <esri2721> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 87E
 <esri2722> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 90E
 <esri2723> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 93E
 <esri2724> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 96E
 <esri2725> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 99E
 <esri2726> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 102E
 <esri2727> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 105E
 <esri2728> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 108E
 <esri2729> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 111E
 <esri2730> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 114E
 <esri2731> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 117E
 <esri2732> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 120E
 <esri2733> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 123E
 <esri2734> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 126E
 <esri2735> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 129E
 <esri2736> : Tete / UTM zone 36S
 <esri2737> : Tete / UTM zone 37S
 <esri2738> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 132E
 <esri2739> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 135E
 <esri2740> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 138E
 <esri2741> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 141E
 <esri2742> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 144E
 <esri2743> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 147E
 <esri2744> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 150E
 <esri2745> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 153E
 <esri2746> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 156E
 <esri2747> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 159E
 <esri2748> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 162E
 <esri2749> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 165E
 <esri2750> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 168E
 <esri2751> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 171E
 <esri2752> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 174E
 <esri2753> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 177E
 <esri2754> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 180E
 <esri2755> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 177W
 <esri2756> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 174W
 <esri2757> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 171W
 <esri2758> : Pulkovo 1995 / 3-degree Gauss-Kruger CM 168W
 <esri2759> : NAD83(HARN) / Alabama East
 <esri2760> : NAD83(HARN) / Alabama West
 <esri2761> : NAD83(HARN) / Arizona East
 <esri2762> : NAD83(HARN) / Arizona Central
 <esri2763> : NAD83(HARN) / Arizona West
 <esri2764> : NAD83(HARN) / Arkansas North
 <esri2765> : NAD83(HARN) / Arkansas South
 <esri2766> : NAD83(HARN) / California zone 1
 <esri2767> : NAD83(HARN) / California zone 2
 <esri2768> : NAD83(HARN) / California zone 3
 <esri2769> : NAD83(HARN) / California zone 4
 <esri2770> : NAD83(HARN) / California zone 5
 <esri2771> : NAD83(HARN) / California zone 6
 <esri2772> : NAD83(HARN) / Colorado North
 <esri2773> : NAD83(HARN) / Colorado Central
 <esri2774> : NAD83(HARN) / Colorado South
 <esri2775> : NAD83(HARN) / Connecticut
 <esri2776> : NAD83(HARN) / Delaware
 <esri2777> : NAD83(HARN) / Florida East
 <esri2778> : NAD83(HARN) / Florida West
 <esri2779> : NAD83(HARN) / Florida North
 <esri2780> : NAD83(HARN) / Georgia East
 <esri2781> : NAD83(HARN) / Georgia West
 <esri2782> : NAD83(HARN) / Hawaii zone 1
 <esri2783> : NAD83(HARN) / Hawaii zone 2
 <esri2784> : NAD83(HARN) / Hawaii zone 3
 <esri2785> : NAD83(HARN) / Hawaii zone 4
 <esri2786> : NAD83(HARN) / Hawaii zone 5
 <esri2787> : NAD83(HARN) / Idaho East
 <esri2788> : NAD83(HARN) / Idaho Central
 <esri2789> : NAD83(HARN) / Idaho West
 <esri2790> : NAD83(HARN) / Illinois East
 <esri2791> : NAD83(HARN) / Illinois West
 <esri2792> : NAD83(HARN) / Indiana East
 <esri2793> : NAD83(HARN) / Indiana West
 <esri2794> : NAD83(HARN) / Iowa North
 <esri2795> : NAD83(HARN) / Iowa South
 <esri2796> : NAD83(HARN) / Kansas North
 <esri2797> : NAD83(HARN) / Kansas South
 <esri2798> : NAD83(HARN) / Kentucky North
 <esri2799> : NAD83(HARN) / Kentucky South
 <esri2800> : NAD83(HARN) / Louisiana North
 <esri2801> : NAD83(HARN) / Louisiana South
 <esri2802> : NAD83(HARN) / Maine East
 <esri2803> : NAD83(HARN) / Maine West
 <esri2804> : NAD83(HARN) / Maryland
 <esri2805> : NAD83(HARN) / Massachusetts Mainland
 <esri2806> : NAD83(HARN) / Massachusetts Island
 <esri2807> : NAD83(HARN) / Michigan North
 <esri2808> : NAD83(HARN) / Michigan Central
 <esri2809> : NAD83(HARN) / Michigan South
 <esri2810> : NAD83(HARN) / Minnesota North
 <esri2811> : NAD83(HARN) / Minnesota Central
 <esri2812> : NAD83(HARN) / Minnesota South
 <esri2813> : NAD83(HARN) / Mississippi East
 <esri2814> : NAD83(HARN) / Mississippi West
 <esri2815> : NAD83(HARN) / Missouri East
 <esri2816> : NAD83(HARN) / Missouri Central
 <esri2817> : NAD83(HARN) / Missouri West
 <esri2818> : NAD83(HARN) / Montana
 <esri2819> : NAD83(HARN) / Nebraska
 <esri2820> : NAD83(HARN) / Nevada East
 <esri2821> : NAD83(HARN) / Nevada Central
 <esri2822> : NAD83(HARN) / Nevada West
 <esri2823> : NAD83(HARN) / New Hampshire
 <esri2824> : NAD83(HARN) / New Jersey
 <esri2825> : NAD83(HARN) / New Mexico East
 <esri2826> : NAD83(HARN) / New Mexico Central
 <esri2827> : NAD83(HARN) / New Mexico West
 <esri2828> : NAD83(HARN) / New York East
 <esri2829> : NAD83(HARN) / New York Central
 <esri2830> : NAD83(HARN) / New York West
 <esri2831> : NAD83(HARN) / New York Long Island
 <esri2832> : NAD83(HARN) / North Dakota North
 <esri2833> : NAD83(HARN) / North Dakota South
 <esri2834> : NAD83(HARN) / Ohio North
 <esri2835> : NAD83(HARN) / Ohio South
 <esri2836> : NAD83(HARN) / Oklahoma North
 <esri2837> : NAD83(HARN) / Oklahoma South
 <esri2838> : NAD83(HARN) / Oregon North
 <esri2839> : NAD83(HARN) / Oregon South
 <esri2840> : NAD83(HARN) / Rhode Island
 <esri2841> : NAD83(HARN) / South Dakota North
 <esri2842> : NAD83(HARN) / South Dakota South
 <esri2843> : NAD83(HARN) / Tennessee
 <esri2844> : NAD83(HARN) / Texas North
 <esri2845> : NAD83(HARN) / Texas North Central
 <esri2846> : NAD83(HARN) / Texas Central
 <esri2847> : NAD83(HARN) / Texas South Central
 <esri2848> : NAD83(HARN) / Texas South
 <esri2849> : NAD83(HARN) / Utah North
 <esri2850> : NAD83(HARN) / Utah Central
 <esri2851> : NAD83(HARN) / Utah South
 <esri2852> : NAD83(HARN) / Vermont
 <esri2853> : NAD83(HARN) / Virginia North
 <esri2854> : NAD83(HARN) / Virginia South
 <esri2855> : NAD83(HARN) / Washington North
 <esri2856> : NAD83(HARN) / Washington South
 <esri2857> : NAD83(HARN) / West Virginia North
 <esri2858> : NAD83(HARN) / West Virginia South
 <esri2859> : NAD83(HARN) / Wisconsin North
 <esri2860> : NAD83(HARN) / Wisconsin Central
 <esri2861> : NAD83(HARN) / Wisconsin South
 <esri2862> : NAD83(HARN) / Wyoming East
 <esri2863> : NAD83(HARN) / Wyoming East Central
 <esri2864> : NAD83(HARN) / Wyoming West Central
 <esri2865> : NAD83(HARN) / Wyoming West
 <esri2866> : NAD83(HARN) / Puerto Rico & Virgin Is.
 <esri2867> : NAD83(HARN) / Arizona East (ft)
 <esri2868> : NAD83(HARN) / Arizona Central (ft)
 <esri2869> : NAD83(HARN) / Arizona West (ft)
 <esri2870> : NAD83(HARN) / California zone 1 (ftUS)
 <esri2871> : NAD83(HARN) / California zone 2 (ftUS)
 <esri2872> : NAD83(HARN) / California zone 3 (ftUS)
 <esri2873> : NAD83(HARN) / California zone 4 (ftUS)
 <esri2874> : NAD83(HARN) / California zone 5 (ftUS)
 <esri2875> : NAD83(HARN) / California zone 6 (ftUS)
 <esri2876> : NAD83(HARN) / Colorado North (ftUS)
 <esri2877> : NAD83(HARN) / Colorado Central (ftUS)
 <esri2878> : NAD83(HARN) / Colorado South (ftUS)
 <esri2879> : NAD83(HARN) / Connecticut (ftUS)
 <esri2880> : NAD83(HARN) / Delaware (ftUS)
 <esri2881> : NAD83(HARN) / Florida East (ftUS)
 <esri2882> : NAD83(HARN) / Florida West (ftUS)
 <esri2883> : NAD83(HARN) / Florida North (ftUS)
 <esri2884> : NAD83(HARN) / Georgia East (ftUS)
 <esri2885> : NAD83(HARN) / Georgia West (ftUS)
 <esri2886> : NAD83(HARN) / Idaho East (ftUS)
 <esri2887> : NAD83(HARN) / Idaho Central (ftUS)
 <esri2888> : NAD83(HARN) / Idaho West (ftUS)
 <esri2889> : NAD83(HARN) / Indiana East (ftUS)
 <esri2890> : NAD83(HARN) / Indiana West (ftUS)
 <esri2891> : NAD83(HARN) / Kentucky North (ftUS)
 <esri2892> : NAD83(HARN) / Kentucky South (ftUS)
 <esri2893> : NAD83(HARN) / Maryland (ftUS)
 <esri2894> : NAD83(HARN) / Massachusetts Mainland (ftUS)
 <esri2895> : NAD83(HARN) / Massachusetts Island (ftUS)
 <esri2896> : NAD83(HARN) / Michigan North (ft)
 <esri2897> : NAD83(HARN) / Michigan Central (ft)
 <esri2898> : NAD83(HARN) / Michigan South (ft)
 <esri2899> : NAD83(HARN) / Mississippi East (ftUS)
 <esri2900> : NAD83(HARN) / Mississippi West (ftUS)
 <esri2901> : NAD83(HARN) / Montana (ft)
 <esri2902> : NAD83(HARN) / New Mexico East (ftUS)
 <esri2903> : NAD83(HARN) / New Mexico Central (ftUS)
 <esri2904> : NAD83(HARN) / New Mexico West (ftUS)
 <esri2905> : NAD83(HARN) / New York East (ftUS)
 <esri2906> : NAD83(HARN) / New York Central (ftUS)
 <esri2907> : NAD83(HARN) / New York West (ftUS)
 <esri2908> : NAD83(HARN) / New York Long Island (ftUS)
 <esri2909> : NAD83(HARN) / North Dakota North (ft)
 <esri2910> : NAD83(HARN) / North Dakota South (ft)
 <esri2911> : NAD83(HARN) / Oklahoma North (ftUS)
 <esri2912> : NAD83(HARN) / Oklahoma South (ftUS)
 <esri2913> : NAD83(HARN) / Oregon North (ft)
 <esri2914> : NAD83(HARN) / Oregon South (ft)
 <esri2915> : NAD83(HARN) / Tennessee (ftUS)
 <esri2916> : NAD83(HARN) / Texas North (ftUS)
 <esri2917> : NAD83(HARN) / Texas North Central (ftUS)
 <esri2918> : NAD83(HARN) / Texas Central (ftUS)
 <esri2919> : NAD83(HARN) / Texas South Central (ftUS)
 <esri2920> : NAD83(HARN) / Texas South (ftUS)
 <esri2921> : NAD83(HARN) / Utah North (ft)
 <esri2922> : NAD83(HARN) / Utah Central (ft)
 <esri2923> : NAD83(HARN) / Utah South (ft)
 <esri2924> : NAD83(HARN) / Virginia North (ftUS)
 <esri2925> : NAD83(HARN) / Virginia South (ftUS)
 <esri2926> : NAD83(HARN) / Washington North (ftUS)
 <esri2927> : NAD83(HARN) / Washington South (ftUS)
 <esri2928> : NAD83(HARN) / Wisconsin North (ftUS)
 <esri2929> : NAD83(HARN) / Wisconsin Central (ftUS)
 <esri2930> : NAD83(HARN) / Wisconsin South (ftUS)
 <esri2931> : Beduaram / TM 13 NE
 <esri2932> : QND95 / Qatar National Grid
 <esri2933> : Segara / UTM zone 50S
 <esri2934> : Segara (Jakarta) / NEIEZ
 <esri2935> : Pulkovo 1942 / CS63 zone A1
 <esri2936> : Pulkovo 1942 / CS63 zone A2
 <esri2937> : Pulkovo 1942 / CS63 zone A3
 <esri2938> : Pulkovo 1942 / CS63 zone A4
 <esri2939> : Pulkovo 1942 / CS63 zone K2
 <esri2940> : Pulkovo 1942 / CS63 zone K3
 <esri2941> : Pulkovo 1942 / CS63 zone K4
 <esri2942> : Porto Santo / UTM zone 28N
 <esri2943> : Selvagem Grande / UTM zone 28N
 <esri2944> : NAD83(CSRS) / SCoPQ zone 2
 <esri2945> : NAD83(CSRS) / MTM zone 3
 <esri2946> : NAD83(CSRS) / MTM zone 4
 <esri2947> : NAD83(CSRS) / MTM zone 5
 <esri2948> : NAD83(CSRS) / MTM zone 6
 <esri2949> : NAD83(CSRS) / MTM zone 7
 <esri2950> : NAD83(CSRS) / MTM zone 8
 <esri2951> : NAD83(CSRS) / MTM zone 9
 <esri2952> : NAD83(CSRS) / MTM zone 10
 <esri2953> : NAD83(CSRS) / New Brunswick Stereo
 <esri2954> : NAD83(CSRS) / Prince Edward Isl. Stereographic (NAD83)
 <esri2955> : NAD83(CSRS) / UTM zone 11N
 <esri2956> : NAD83(CSRS) / UTM zone 12N
 <esri2957> : NAD83(CSRS) / UTM zone 13N
 <esri2958> : NAD83(CSRS) / UTM zone 17N
 <esri2959> : NAD83(CSRS) / UTM zone 18N
 <esri2960> : NAD83(CSRS) / UTM zone 19N
 <esri2961> : NAD83(CSRS) / UTM zone 20N
 <esri2962> : NAD83(CSRS) / UTM zone 21N
 <esri3036> : Moznet / UTM zone 36S
 <esri3037> : Moznet / UTM zone 37S
 <esri3148> : Indian 1960 / UTM zone 48N
 <esri3149> : Indian 1960 / UTM zone 49N
 <esri3176> : Indian 1960 / TM 106 NE
 <esri3200> : FD58 / Iraq zone
 <esri3300> : Estonian Coordinate System of 1992
 <esri3301> : Estonian Coordinate System of 1997
 <esri3439> : PSD93 / UTM zone 39N
 <esri3440> : PSD93 / UTM zone 40N
 <esri3561> : Old Hawaiian / Hawaii zone 1
 <esri3562> : Old Hawaiian / Hawaii zone 2
 <esri3563> : Old Hawaiian / Hawaii zone 3
 <esri3564> : Old Hawaiian / Hawaii zone 4
 <esri3565> : Old Hawaiian / Hawaii zone 5
 <esri3920> : Puerto Rico / UTM zone 20N
 <esri3991> : Puerto Rico State Plane CS of 1927
 <esri3992> : Puerto Rico / St. Croix
 <esri4001> : Unknown datum based upon the Airy 1830 ellipsoid
 <esri4002> : Unknown datum based upon the Airy Modified 1849 ellipsoid
 <esri4003> : Unknown datum based upon the Australian National Spheroid
 <esri4004> : Unknown datum based upon the Bessel 1841 ellipsoid
 <esri4005> : Unknown datum based upon the Bessel Modified ellipsoid
 <esri4006> : Unknown datum based upon the Bessel Namibia ellipsoid
 <esri4007> : Unknown datum based upon the Clarke 1858 ellipsoid
 <esri4008> : Unknown datum based upon the Clarke 1866 ellipsoid
 <esri4009> : Unknown datum based upon the Clarke 1866 Michigan ellipsoid
 <esri4010> : Unknown datum based upon the Clarke 1880 (Benoit) ellipsoid
 <esri4011> : Unknown datum based upon the Clarke 1880 (IGN) ellipsoid
 <esri4012> : Unknown datum based upon the Clarke 1880 (RGS) ellipsoid
 <esri4013> : Unknown datum based upon the Clarke 1880 (Arc) ellipsoid
 <esri4014> : Unknown datum based upon the Clarke 1880 (SGA 1922) ellipsoid
 <esri4015> : Unknown datum based upon the Everest 1830 (1937 Adjustment) ellipsoid
 <esri4016> : Unknown datum based upon the Everest 1830 (1967 Definition) ellipsoid
 <esri4018> : Unknown datum based upon the Everest 1830 Modified ellipsoid
 <esri4019> : Unknown datum based upon the GRS 1980 ellipsoid
 <esri4020> : Unknown datum based upon the Helmert 1906 ellipsoid
 <esri4021> : Unknown datum based upon the Indonesian National Spheroid
 <esri4022> : Unknown datum based upon the International 1924 ellipsoid
 <esri4024> : Unknown datum based upon the Krassowsky 1940 ellipsoid
 <esri4025> : Unknown datum based upon the NWL 9D ellipsoid
 <esri4027> : Unknown datum based upon the Plessis 1817 ellipsoid
 <esri4028> : Unknown datum based upon the Struve 1860 ellipsoid
 <esri4029> : Unknown datum based upon the War Office ellipsoid
 <esri4030> : Unknown datum based upon the WGS 84 ellipsoid
 <esri4031> : Unknown datum based upon the GEM 10C ellipsoid
 <esri4032> : Unknown datum based upon the OSU86F ellipsoid
 <esri4033> : Unknown datum based upon the OSU91A ellipsoid
 <esri4034> : Unknown datum based upon the Clarke 1880 ellipsoid
 <esri4035> : Unknown datum based upon the Authalic Sphere
 <esri4036> : Unknown datum based upon the GRS 1967 ellipsoid
 <esri4041> : Unknown datum based upon the Average Terrestrial System 1977 ellipsoid
 <esri4042> : Unknown datum based upon the Everest (1830 Definition) ellipsoid
 <esri4043> : Unknown datum based upon the WGS 72 ellipsoid
 <esri4044> : Unknown datum based upon the Everest 1830 (1962 Definition) ellipsoid
 <esri4045> : Unknown datum based upon the Everest 1830 (1975 Definition) ellipsoid
 <esri4047> : Unspecified based upon the GRS 1980 Authalic Sphere
 <esri4120> : Greek
 <esri4121> : GGRS87
 <esri4122> : ATS77
 <esri4123> : KKJ
 <esri4124> : RT90
 <esri4125> : Samboja
 <esri4126> : LKS94 (ETRS89)
 <esri4127> : Tete
 <esri4128> : Madzansua
 <esri4129> : Observatario
 <esri4130> : Moznet
 <esri4131> : Indian 1960
 <esri4132> : FD58
 <esri4133> : EST92
 <esri4134> : PDO Survey Datum 1993
 <esri4135> : Old Hawaiian
 <esri4136> : St. Lawrence Island
 <esri4137> : St. Paul Island
 <esri4138> : St. George Island
 <esri4139> : Puerto Rico
 <esri4140> : NAD83(CSRS98)
 <esri4141> : Israel
 <esri4142> : Locodjo 1965
 <esri4143> : Abidjan 1987
 <esri4144> : Kalianpur 1937
 <esri4145> : Kalianpur 1962
 <esri4146> : Kalianpur 1975
 <esri4147> : Hanoi 1972
 <esri4148> : Hartebeesthoek94
 <esri4149> : CH1903
 <esri4150> : CH1903+
 <esri4151> : CHTRF95
 <esri4152> : NAD83(HARN)
 <esri4153> : Rassadiran
 <esri4154> : ED50(ED77)
 <esri4155> : Dabola 1981
 <esri4156> : S-JTSK
 <esri4157> : Mount Dillon
 <esri4158> : Naparima 1955
 <esri4159> : ELD79
 <esri4160> : Chos Malal 1914
 <esri4161> : Pampa del Castillo
 <esri4162> : Korean 1985
 <esri4163> : Yemen NGN96
 <esri4164> : South Yemen
 <esri4165> : Bissau
 <esri4166> : Korean 1995
 <esri4167> : NZGD2000
 <esri4168> : Accra
 <esri4169> : American Samoa 1962
 <esri4170> : SIRGAS
 <esri4171> : RGF93
 <esri4172> : POSGAR
 <esri4173> : IRENET95
 <esri4174> : Sierra Leone 1924
 <esri4175> : Sierra Leone 1968
 <esri4176> : Australian Antarctic
 <esri4178> : Pulkovo 1942(83)
 <esri4179> : Pulkovo 1942(58)
 <esri4180> : EST97
 <esri4181> : Luxembourg 1930
 <esri4182> : Azores Occidental 1939
 <esri4183> : Azores Central 1948
 <esri4184> : Azores Oriental 1940
 <esri4185> : Madeira 1936
 <esri4188> : OSNI 1952
 <esri4189> : REGVEN
 <esri4190> : POSGAR 98
 <esri4191> : Albanian 1987
 <esri4192> : Douala 1948
 <esri4193> : Manoca 1962
 <esri4194> : Qornoq 1927
 <esri4195> : Scoresbysund 1952
 <esri4196> : Ammassalik 1958
 <esri4197> : Garoua
 <esri4198> : Kousseri
 <esri4199> : Egypt 1930
 <esri4200> : Pulkovo 1995
 <esri4201> : Adindan
 <esri4202> : AGD66
 <esri4203> : AGD84
 <esri4204> : Ain el Abd
 <esri4205> : Afgooye
 <esri4206> : Agadez
 <esri4207> : Lisbon
 <esri4208> : Aratu
 <esri4209> : Arc 1950
 <esri4210> : Arc 1960
 <esri4211> : Batavia
 <esri4212> : Barbados 1938
 <esri4213> : Beduaram
 <esri4214> : Beijing 1954
 <esri4215> : Belge 1950
 <esri4216> : Bermuda 1957
 <esri4218> : Bogota 1975
 <esri4219> : Bukit Rimpah
 <esri4220> : Camacupa
 <esri4221> : Campo Inchauspe
 <esri4222> : Cape
 <esri4223> : Carthage
 <esri4224> : Chua
 <esri4225> : Corrego Alegre
 <esri4226> : Cote d'Ivoire
 <esri4227> : Deir ez Zor
 <esri4228> : Douala
 <esri4229> : Egypt 1907
 <esri4230> : ED50
 <esri4231> : ED87
 <esri4232> : Fahud
 <esri4233> : Gandajika 1970
 <esri4234> : Garoua
 <esri4235> : Guyane Francaise
 <esri4236> : Hu Tzu Shan
 <esri4237> : HD72
 <esri4238> : ID74
 <esri4239> : Indian 1954
 <esri4240> : Indian 1975
 <esri4241> : Jamaica 1875
 <esri4242> : JAD69
 <esri4243> : Kalianpur 1880
 <esri4244> : Kandawala
 <esri4245> : Kertau
 <esri4246> : KOC
 <esri4247> : La Canoa
 <esri4248> : PSAD56
 <esri4249> : Lake
 <esri4250> : Leigon
 <esri4251> : Liberia 1964
 <esri4252> : Lome
 <esri4253> : Luzon 1911
 <esri4254> : Hito XVIII 1963
 <esri4255> : Herat North
 <esri4256> : Mahe 1971
 <esri4257> : Makassar
 <esri4258> : ETRS89
 <esri4259> : Malongo 1987
 <esri4260> : Manoca
 <esri4261> : Merchich
 <esri4262> : Massawa
 <esri4263> : Minna
 <esri4264> : Mhast
 <esri4265> : Monte Mario
 <esri4266> : M'poraloko
 <esri4267> : NAD27
 <esri4268> : NAD27 Michigan
 <esri4269> : NAD83
 <esri4270> : Nahrwan 1967
 <esri4271> : Naparima 1972
 <esri4272> : NZGD49
 <esri4273> : NGO 1948
 <esri4274> : Datum 73
 <esri4275> : NTF
 <esri4276> : NSWC 9Z-2
 <esri4277> : OSGB 1936
 <esri4278> : OSGB70
 <esri4279> : OS(SN)80
 <esri4280> : Padang
 <esri4281> : Palestine 1923
 <esri4282> : Pointe Noire
 <esri4283> : GDA94
 <esri4284> : Pulkovo 1942
 <esri4285> : Qatar 1974
 <esri4286> : Qatar 1948
 <esri4287> : Qornoq
 <esri4288> : Loma Quintana
 <esri4289> : Amersfoort
 <esri4291> : SAD69
 <esri4292> : Sapper Hill 1943
 <esri4293> : Schwarzeck
 <esri4294> : Segora
 <esri4295> : Serindung
 <esri4296> : Sudan
 <esri4297> : Tananarive
 <esri4298> : Timbalai 1948
 <esri4299> : TM65
 <esri4300> : TM75
 <esri4301> : Tokyo
 <esri4302> : Trinidad 1903
 <esri4303> : TC(1948)
 <esri4304> : Voirol 1875
 <esri4306> : Bern 1938
 <esri4307> : Nord Sahara 1959
 <esri4308> : RT38
 <esri4309> : Yacare
 <esri4310> : Yoff
 <esri4311> : Zanderij
 <esri4312> : MGI
 <esri4313> : Belge 1972
 <esri4314> : DHDN
 <esri4315> : Conakry 1905
 <esri4316> : Dealul Piscului 1933
 <esri4317> : Dealul Piscului 1970
 <esri4318> : NGN
 <esri4319> : KUDAMS
 <esri4322> : WGS 72
 <esri4324> : WGS 72BE
 <esri4326> : WGS 84
 <esri4600> : Anguilla 1957
 <esri4601> : Antigua 1943
 <esri4602> : Dominica 1945
 <esri4603> : Grenada 1953
 <esri4604> : Montserrat 1958
 <esri4605> : St. Kitts 1955
 <esri4606> : St. Lucia 1955
 <esri4607> : St. Vincent 1945
 <esri4608> : NAD27(76)
 <esri4609> : NAD27(CGQ77)
 <esri4610> : Xian 1980
 <esri4611> : Hong Kong 1980
 <esri4612> : JGD2000
 <esri4613> : Segara
 <esri4614> : QND95
 <esri4615> : Porto Santo
 <esri4616> : Selvagem Grande
 <esri4617> : NAD83(CSRS)
 <esri4801> : Bern 1898 (Bern)
 <esri4802> : Bogota 1975 (Bogota)
 <esri4803> : Lisbon (Lisbon)
 <esri4804> : Makassar (Jakarta)
 <esri4805> : MGI (Ferro)
 <esri4806> : Monte Mario (Rome)
 <esri4807> : NTF (Paris)
 <esri4808> : Padang (Jakarta)
 <esri4809> : Belge 1950 (Brussels)
 <esri4810> : Tananarive (Paris)
 <esri4811> : Voirol 1875 (Paris)
 <esri4813> : Batavia (Jakarta)
 <esri4814> : RT38 (Stockholm)
 <esri4815> : Greek (Athens)
 <esri4816> : Carthage (Paris)
 <esri4817> : NGO 1948 (Oslo)
 <esri4818> : S-JTSK (Ferro)
 <esri4819> : Nord Sahara 1959 (Paris)
 <esri4820> : Segara (Jakarta)
 <esri4901> : ATF (Paris)
 <esri4902> : NDG (Paris)
 <esri4903> : Madrid 1870 (Madrid)
 <esri4904> : Lisbon 1890 (Lisbon)
 <esri20004> : Pulkovo 1995 / Gauss-Kruger zone 4
 <esri20005> : Pulkovo 1995 / Gauss-Kruger zone 5
 <esri20006> : Pulkovo 1995 / Gauss-Kruger zone 6
 <esri20007> : Pulkovo 1995 / Gauss-Kruger zone 7
 <esri20008> : Pulkovo 1995 / Gauss-Kruger zone 8
 <esri20009> : Pulkovo 1995 / Gauss-Kruger zone 9
 <esri20010> : Pulkovo 1995 / Gauss-Kruger zone 10
 <esri20011> : Pulkovo 1995 / Gauss-Kruger zone 11
 <esri20012> : Pulkovo 1995 / Gauss-Kruger zone 12
 <esri20013> : Pulkovo 1995 / Gauss-Kruger zone 13
 <esri20014> : Pulkovo 1995 / Gauss-Kruger zone 14
 <esri20015> : Pulkovo 1995 / Gauss-Kruger zone 15
 <esri20016> : Pulkovo 1995 / Gauss-Kruger zone 16
 <esri20017> : Pulkovo 1995 / Gauss-Kruger zone 17
 <esri20018> : Pulkovo 1995 / Gauss-Kruger zone 18
 <esri20019> : Pulkovo 1995 / Gauss-Kruger zone 19
 <esri20020> : Pulkovo 1995 / Gauss-Kruger zone 20
 <esri20021> : Pulkovo 1995 / Gauss-Kruger zone 21
 <esri20022> : Pulkovo 1995 / Gauss-Kruger zone 22
 <esri20023> : Pulkovo 1995 / Gauss-Kruger zone 23
 <esri20024> : Pulkovo 1995 / Gauss-Kruger zone 24
 <esri20025> : Pulkovo 1995 / Gauss-Kruger zone 25
 <esri20026> : Pulkovo 1995 / Gauss-Kruger zone 26
 <esri20027> : Pulkovo 1995 / Gauss-Kruger zone 27
 <esri20028> : Pulkovo 1995 / Gauss-Kruger zone 28
 <esri20029> : Pulkovo 1995 / Gauss-Kruger zone 29
 <esri20030> : Pulkovo 1995 / Gauss-Kruger zone 30
 <esri20031> : Pulkovo 1995 / Gauss-Kruger zone 31
 <esri20032> : Pulkovo 1995 / Gauss-Kruger zone 32
 <esri20064> : Pulkovo 1995 / Gauss-Kruger 4N
 <esri20065> : Pulkovo 1995 / Gauss-Kruger 5N
 <esri20066> : Pulkovo 1995 / Gauss-Kruger 6N
 <esri20067> : Pulkovo 1995 / Gauss-Kruger 7N
 <esri20068> : Pulkovo 1995 / Gauss-Kruger 8N
 <esri20069> : Pulkovo 1995 / Gauss-Kruger 9N
 <esri20070> : Pulkovo 1995 / Gauss-Kruger 10N
 <esri20071> : Pulkovo 1995 / Gauss-Kruger 11N
 <esri20072> : Pulkovo 1995 / Gauss-Kruger 12N
 <esri20073> : Pulkovo 1995 / Gauss-Kruger 13N
 <esri20074> : Pulkovo 1995 / Gauss-Kruger 14N
 <esri20075> : Pulkovo 1995 / Gauss-Kruger 15N
 <esri20076> : Pulkovo 1995 / Gauss-Kruger 16N
 <esri20077> : Pulkovo 1995 / Gauss-Kruger 17N
 <esri20078> : Pulkovo 1995 / Gauss-Kruger 18N
 <esri20079> : Pulkovo 1995 / Gauss-Kruger 19N
 <esri20080> : Pulkovo 1995 / Gauss-Kruger 20N
 <esri20081> : Pulkovo 1995 / Gauss-Kruger 21N
 <esri20082> : Pulkovo 1995 / Gauss-Kruger 22N
 <esri20083> : Pulkovo 1995 / Gauss-Kruger 23N
 <esri20084> : Pulkovo 1995 / Gauss-Kruger 24N
 <esri20085> : Pulkovo 1995 / Gauss-Kruger 25N
 <esri20086> : Pulkovo 1995 / Gauss-Kruger 26N
 <esri20087> : Pulkovo 1995 / Gauss-Kruger 27N
 <esri20088> : Pulkovo 1995 / Gauss-Kruger 28N
 <esri20089> : Pulkovo 1995 / Gauss-Kruger 29N
 <esri20090> : Pulkovo 1995 / Gauss-Kruger 30N
 <esri20091> : Pulkovo 1995 / Gauss-Kruger 31N
 <esri20092> : Pulkovo 1995 / Gauss-Kruger 32N
 <esri20137> : Adindan / UTM zone 37N
 <esri20138> : Adindan / UTM zone 38N
 <esri20248> : AGD66 / AMG zone 48
 <esri20249> : AGD66 / AMG zone 49
 <esri20250> : AGD66 / AMG zone 50
 <esri20251> : AGD66 / AMG zone 51
 <esri20252> : AGD66 / AMG zone 52
 <esri20253> : AGD66 / AMG zone 53
 <esri20254> : AGD66 / AMG zone 54
 <esri20255> : AGD66 / AMG zone 55
 <esri20256> : AGD66 / AMG zone 56
 <esri20257> : AGD66 / AMG zone 57
 <esri20258> : AGD66 / AMG zone 58
 <esri20348> : AGD84 / AMG zone 48
 <esri20349> : AGD84 / AMG zone 49
 <esri20350> : AGD84 / AMG zone 50
 <esri20351> : AGD84 / AMG zone 51
 <esri20352> : AGD84 / AMG zone 52
 <esri20353> : AGD84 / AMG zone 53
 <esri20354> : AGD84 / AMG zone 54
 <esri20355> : AGD84 / AMG zone 55
 <esri20356> : AGD84 / AMG zone 56
 <esri20357> : AGD84 / AMG zone 57
 <esri20358> : AGD84 / AMG zone 58
 <esri20437> : Ain el Abd / UTM zone 37N
 <esri20438> : Ain el Abd / UTM zone 38N
 <esri20439> : Ain el Abd / UTM zone 39N
 <esri20499> : Ain el Abd / Bahrain Grid
 <esri20538> : Afgooye / UTM zone 38N
 <esri20539> : Afgooye / UTM zone 39N
 <esri20790> : Lisbon (Lisbon)/Portuguese National Grid
 <esri20791> : Lisbon (Lisbon)/Portuguese Grid
 <esri20822> : Aratu / UTM zone 22S
 <esri20823> : Aratu / UTM zone 23S
 <esri20824> : Aratu / UTM zone 24S
 <esri20934> : Arc 1950 / UTM zone 34S
 <esri20935> : Arc 1950 / UTM zone 35S
 <esri20936> : Arc 1950 / UTM zone 36S
 <esri21035> : Arc 1960 / UTM zone 35S
 <esri21036> : Arc 1960 / UTM zone 36S
 <esri21037> : Arc 1960 / UTM zone 37S
 <esri21095> : Arc 1960 / UTM zone 35N
 <esri21096> : Arc 1960 / UTM zone 36N
 <esri21097> : Arc 1960 / UTM zone 37N
 <esri21100> : Batavia (Jakarta) / NEIEZ
 <esri21148> : Batavia / UTM zone 48S
 <esri21149> : Batavia / UTM zone 49S
 <esri21150> : Batavia / UTM zone 50S
 <esri21291> : Barbados 1938 / British West Indies Grid
 <esri21292> : Barbados 1938 / Barbados National Grid
 <esri21413> : Beijing 1954 / Gauss-Kruger zone 13
 <esri21414> : Beijing 1954 / Gauss-Kruger zone 14
 <esri21415> : Beijing 1954 / Gauss-Kruger zone 15
 <esri21416> : Beijing 1954 / Gauss-Kruger zone 16
 <esri21417> : Beijing 1954 / Gauss-Kruger zone 17
 <esri21418> : Beijing 1954 / Gauss-Kruger zone 18
 <esri21419> : Beijing 1954 / Gauss-Kruger zone 19
 <esri21420> : Beijing 1954 / Gauss-Kruger zone 20
 <esri21421> : Beijing 1954 / Gauss-Kruger zone 21
 <esri21422> : Beijing 1954 / Gauss-Kruger zone 22
 <esri21423> : Beijing 1954 / Gauss-Kruger zone 23
 <esri21453> : Beijing 1954 / Gauss-Kruger CM 75E
 <esri21454> : Beijing 1954 / Gauss-Kruger CM 81E
 <esri21455> : Beijing 1954 / Gauss-Kruger CM 87E
 <esri21456> : Beijing 1954 / Gauss-Kruger CM 93E
 <esri21457> : Beijing 1954 / Gauss-Kruger CM 99E
 <esri21458> : Beijing 1954 / Gauss-Kruger CM 105E
 <esri21459> : Beijing 1954 / Gauss-Kruger CM 111E
 <esri21460> : Beijing 1954 / Gauss-Kruger CM 117E
 <esri21461> : Beijing 1954 / Gauss-Kruger CM 123E
 <esri21462> : Beijing 1954 / Gauss-Kruger CM 129E
 <esri21463> : Beijing 1954 / Gauss-Kruger CM 135E
 <esri21473> : Beijing 1954 / Gauss-Kruger 13N
 <esri21474> : Beijing 1954 / Gauss-Kruger 14N
 <esri21475> : Beijing 1954 / Gauss-Kruger 15N
 <esri21476> : Beijing 1954 / Gauss-Kruger 16N
 <esri21477> : Beijing 1954 / Gauss-Kruger 17N
 <esri21478> : Beijing 1954 / Gauss-Kruger 18N
 <esri21479> : Beijing 1954 / Gauss-Kruger 19N
 <esri21480> : Beijing 1954 / Gauss-Kruger 20N
 <esri21481> : Beijing 1954 / Gauss-Kruger 21N
 <esri21482> : Beijing 1954 / Gauss-Kruger 22N
 <esri21483> : Beijing 1954 / Gauss-Kruger 23N
 <esri21500> : Belge 1950 (Brussels) / Belge Lambert 50
 <esri21780> : Bern 1898 (Bern) / LV03C
 <esri21781> : CH1903 / LV03
 <esri21817> : Bogota 1975 / UTM zone 17N
 <esri21818> : Bogota 1975 / UTM zone 18N
 <esri21891> : Bogota 1975 / Colombia West zone
 <esri21892> : Bogota 1975 / Colombia Bogota zone
 <esri21893> : Bogota 1975 / Colombia East Central zone
 <esri21894> : Bogota 1975 / Colombia East
 <esri22032> : Camacupa / UTM zone 32S
 <esri22033> : Camacupa / UTM zone 33S
 <esri22091> : Camacupa / TM 11.30 SE
 <esri22092> : Camacupa / TM 12 SE
 <esri22191> : Campo Inchauspe / Argentina 1
 <esri22192> : Campo Inchauspe / Argentina 2
 <esri22193> : Campo Inchauspe / Argentina 3
 <esri22194> : Campo Inchauspe / Argentina 4
 <esri22195> : Campo Inchauspe / Argentina 5
 <esri22196> : Campo Inchauspe / Argentina 6
 <esri22197> : Campo Inchauspe / Argentina 7
 <esri22234> : Cape / UTM zone 34S
 <esri22235> : Cape / UTM zone 35S
 <esri22236> : Cape / UTM zone 36S
 <esri22332> : Carthage / UTM zone 32N
 <esri22391> : Carthage / Nord Tunisie
 <esri22392> : Carthage / Sud Tunisie
 <esri22523> : Corrego Alegre / UTM zone 23S
 <esri22524> : Corrego Alegre / UTM zone 24S
 <esri22700> : Deir ez Zor / Levant Zone
 <esri22770> : Deir ez Zor / Syria Lambert
 <esri22780> : Deir ez Zor / Levant Stereographic
 <esri22832> : Douala / UTM zone 32N
 <esri22991> : Egypt 1907 / Blue Belt
 <esri22992> : Egypt 1907 / Red Belt
 <esri22993> : Egypt 1907 / Purple Belt
 <esri22994> : Egypt 1907 / Extended Purple Belt
 <esri23028> : ED50 / UTM zone 28N
 <esri23029> : ED50 / UTM zone 29N
 <esri23030> : ED50 / UTM zone 30N
 <esri23031> : ED50 / UTM zone 31N
 <esri23032> : ED50 / UTM zone 32N
 <esri23033> : ED50 / UTM zone 33N
 <esri23034> : ED50 / UTM zone 34N
 <esri23035> : ED50 / UTM zone 35N
 <esri23036> : ED50 / UTM zone 36N
 <esri23037> : ED50 / UTM zone 37N
 <esri23038> : ED50 / UTM zone 38N
 <esri23090> : ED50 / TM 0 N
 <esri23095> : ED50 / TM 5 NE
 <esri23239> : Fahud / UTM zone 39N
 <esri23240> : Fahud / UTM zone 40N
 <esri23433> : Garoua / UTM zone 33N
 <esri23700> : HD72 / EOV
 <esri23846> : ID74 / UTM zone 46N
 <esri23847> : ID74 / UTM zone 47N
 <esri23848> : ID74 / UTM zone 48N
 <esri23849> : ID74 / UTM zone 49N
 <esri23850> : ID74 / UTM zone 50N
 <esri23851> : ID74 / UTM zone 51N
 <esri23852> : ID74 / UTM zone 52N
 <esri23853> : ID74 / UTM zone 53N
 <esri23886> : ID74 / UTM zone 46S
 <esri23887> : ID74 / UTM zone 47S
 <esri23888> : ID74 / UTM zone 48S
 <esri23889> : ID74 / UTM zone 49S
 <esri23890> : ID74 / UTM zone 50S
 <esri23891> : ID74 / UTM zone 51S
 <esri23892> : ID74 / UTM zone 52S
 <esri23893> : ID74 / UTM zone 53S
 <esri23894> : ID74 / UTM zone 54S
 <esri23946> : Indian 1954 / UTM zone 46N
 <esri23947> : Indian 1954 / UTM zone 47N
 <esri23948> : Indian 1954 / UTM zone 48N
 <esri24047> : Indian 1975 / UTM zone 47N
 <esri24048> : Indian 1975 / UTM zone 48N
 <esri24100> : Jamaica 1875 / Jamaica (Old Grid)
 <esri24200> : JAD69 / Jamaica National Grid
 <esri24305> : Kalianpur 1937 / UTM zone 45N
 <esri24306> : Kalianpur 1937 / UTM zone 46N
 <esri24311> : Kalianpur 1962 / UTM zone 41N
 <esri24312> : Kalianpur 1962 / UTM zone 42N
 <esri24313> : Kalianpur 1962 / UTM zone 43N
 <esri24342> : Kalianpur 1975 / UTM zone 42N
 <esri24343> : Kalianpur 1975 / UTM zone 43N
 <esri24344> : Kalianpur 1975 / UTM zone 44N
 <esri24345> : Kalianpur 1975 / UTM zone 45N
 <esri24346> : Kalianpur 1975 / UTM zone 46N
 <esri24347> : Kalianpur 1975 / UTM zone 47N
 <esri24370> : Kalianpur 1880 / India zone 0
 <esri24371> : Kalianpur 1880 / India zone I
 <esri24372> : Kalianpur 1880 / India zone IIa
 <esri24373> : Kalianpur 1880 / India zone III
 <esri24374> : Kalianpur 1880 / India zone IV
 <esri24375> : Kalianpur 1937 / India zone IIb
 <esri24376> : Kalianpur 1962 / India zone I
 <esri24377> : Kalianpur 1962 / India zone IIa
 <esri24378> : Kalianpur 1975 / India zone I
 <esri24379> : Kalianpur 1975 / India zone IIa
 <esri24380> : Kalianpur 1975 / India zone IIb
 <esri24381> : Kalianpur 1975 / India zone III
 <esri24382> : Kalianpur 1880 / India zone IIb
 <esri24383> : Kalianpur 1975 / India zone IV
 <esri24500> : Kertau / Singapore Grid
 <esri24547> : Kertau / UTM zone 47N
 <esri24548> : Kertau / UTM zone 48N
 <esri24571> : Kertau / R.S.O. Malaya (ch)
 <esri24600> : KOC Lambert
 <esri24718> : La Canoa / UTM zone 18N
 <esri24719> : La Canoa / UTM zone 19N
 <esri24720> : La Canoa / UTM zone 20N
 <esri24818> : PSAD56 / UTM zone 18N
 <esri24819> : PSAD56 / UTM zone 19N
 <esri24820> : PSAD56 / UTM zone 20N
 <esri24821> : PSAD56 / UTM zone 21N
 <esri24877> : PSAD56 / UTM zone 17S
 <esri24878> : PSAD56 / UTM zone 18S
 <esri24879> : PSAD56 / UTM zone 19S
 <esri24880> : PSAD56 / UTM zone 20S
 <esri24882> : PSAD56 / UTM zone 22S
 <esri24891> : PSAD56 / Peru west zone
 <esri24892> : PSAD56 / Peru central zone
 <esri24893> : PSAD56 / Peru east zone
 <esri25000> : Leigon / Ghana Metre Grid
 <esri25231> : Lome / UTM zone 31N
 <esri25391> : Luzon 1911 / Philippines zone I
 <esri25392> : Luzon 1911 / Philippines zone II
 <esri25393> : Luzon 1911 / Philippines zone III
 <esri25394> : Luzon 1911 / Philippines zone IV
 <esri25395> : Luzon 1911 / Philippines zone V
 <esri25700> : Makassar (Jakarta) / NEIEZ
 <esri25828> : ETRS89 / UTM zone 28N
 <esri25829> : ETRS89 / UTM zone 29N
 <esri25830> : ETRS89 / UTM zone 30N
 <esri25831> : ETRS89 / UTM zone 31N
 <esri25832> : ETRS89 / UTM zone 32N
 <esri25833> : ETRS89 / UTM zone 33N
 <esri25834> : ETRS89 / UTM zone 34N
 <esri25835> : ETRS89 / UTM zone 35N
 <esri25836> : ETRS89 / UTM zone 36N
 <esri25837> : ETRS89 / UTM zone 37N
 <esri25838> : ETRS89 / UTM zone 38N
 <esri25884> : ETRS89 / TM Baltic93
 <esri25932> : Malongo 1987 / UTM zone 32S
 <esri26191> : Merchich / Nord Maroc
 <esri26192> : Merchich / Sud Maroc
 <esri26193> : Merchich / Sahara
 <esri26237> : Massawa / UTM zone 37N
 <esri26331> : Minna / UTM zone 31N
 <esri26332> : Minna / UTM zone 32N
 <esri26391> : Minna / Nigeria West Belt
 <esri26392> : Minna / Nigeria Mid Belt
 <esri26393> : Minna / Nigeria East Belt
 <esri26432> : Mhast / UTM zone 32S
 <esri26591> : Monte Mario (Rome) / Italy zone 1
 <esri26592> : Monte Mario (Rome) / Italy zone 2
 <esri26632> : M'poraloko / UTM zone 32N
 <esri26692> : M'poraloko / UTM zone 32S
 <esri26703> : NAD27 / UTM zone 3N
 <esri26704> : NAD27 / UTM zone 4N
 <esri26705> : NAD27 / UTM zone 5N
 <esri26706> : NAD27 / UTM zone 6N
 <esri26707> : NAD27 / UTM zone 7N
 <esri26708> : NAD27 / UTM zone 8N
 <esri26709> : NAD27 / UTM zone 9N
 <esri26710> : NAD27 / UTM zone 10N
 <esri26711> : NAD27 / UTM zone 11N
 <esri26712> : NAD27 / UTM zone 12N
 <esri26713> : NAD27 / UTM zone 13N
 <esri26714> : NAD27 / UTM zone 14N
 <esri26715> : NAD27 / UTM zone 15N
 <esri26716> : NAD27 / UTM zone 16N
 <esri26717> : NAD27 / UTM zone 17N
 <esri26718> : NAD27 / UTM zone 18N
 <esri26719> : NAD27 / UTM zone 19N
 <esri26720> : NAD27 / UTM zone 20N
 <esri26721> : NAD27 / UTM zone 21N
 <esri26722> : NAD27 / UTM zone 22N
 <esri26729> : NAD27 / Alabama East
 <esri26730> : NAD27 / Alabama West
 <esri26731> : NAD27 / Alaska zone 1
 <esri26732> : NAD27 / Alaska zone 2
 <esri26733> : NAD27 / Alaska zone 3
 <esri26734> : NAD27 / Alaska zone 4
 <esri26735> : NAD27 / Alaska zone 5
 <esri26736> : NAD27 / Alaska zone 6
 <esri26737> : NAD27 / Alaska zone 7
 <esri26738> : NAD27 / Alaska zone 8
 <esri26739> : NAD27 / Alaska zone 9
 <esri26740> : NAD27 / Alaska zone 10
 <esri26741> : NAD27 / California zone I
 <esri26742> : NAD27 / California zone II
 <esri26743> : NAD27 / California zone III
 <esri26744> : NAD27 / California zone IV
 <esri26745> : NAD27 / California zone V
 <esri26746> : NAD27 / California zone VI
 <esri26747> : NAD27 / California zone VII
 <esri26748> : NAD27 / Arizona East
 <esri26749> : NAD27 / Arizona Central
 <esri26750> : NAD27 / Arizona West
 <esri26751> : NAD27 / Arkansas North
 <esri26752> : NAD27 / Arkansas South
 <esri26753> : NAD27 / Colorado North
 <esri26754> : NAD27 / Colorado Central
 <esri26755> : NAD27 / Colorado South
 <esri26756> : NAD27 / Connecticut
 <esri26757> : NAD27 / Delaware
 <esri26758> : NAD27 / Florida East
 <esri26759> : NAD27 / Florida West
 <esri26760> : NAD27 / Florida North
 <esri26766> : NAD27 / Georgia East
 <esri26767> : NAD27 / Georgia West
 <esri26768> : NAD27 / Idaho East
 <esri26769> : NAD27 / Idaho Central
 <esri26770> : NAD27 / Idaho West
 <esri26771> : NAD27 / Illinois East
 <esri26772> : NAD27 / Illinois West
 <esri26773> : NAD27 / Indiana East
 <esri26774> : NAD27 / Indiana West
 <esri26775> : NAD27 / Iowa North
 <esri26776> : NAD27 / Iowa South
 <esri26777> : NAD27 / Kansas North
 <esri26778> : NAD27 / Kansas South
 <esri26779> : NAD27 / Kentucky North
 <esri26780> : NAD27 / Kentucky South
 <esri26781> : NAD27 / Louisiana North
 <esri26782> : NAD27 / Louisiana South
 <esri26783> : NAD27 / Maine East
 <esri26784> : NAD27 / Maine West
 <esri26785> : NAD27 / Maryland
 <esri26786> : NAD27 / Massachusetts Mainland
 <esri26787> : NAD27 / Massachusetts Island
 <esri26791> : NAD27 / Minnesota North
 <esri26792> : NAD27 / Minnesota Central
 <esri26793> : NAD27 / Minnesota South
 <esri26794> : NAD27 / Mississippi East
 <esri26795> : NAD27 / Mississippi West
 <esri26796> : NAD27 / Missouri East
 <esri26797> : NAD27 / Missouri Central
 <esri26798> : NAD27 / Missouri West
 <esri26801> : NAD Michigan / Michigan East
 <esri26802> : NAD Michigan / Michigan Old Central
 <esri26803> : NAD Michigan / Michigan West
 <esri26811> : NAD Michigan / Michigan North
 <esri26812> : NAD Michigan / Michigan Central
 <esri26813> : NAD Michigan / Michigan South
 <esri26903> : NAD83 / UTM zone 3N
 <esri26904> : NAD83 / UTM zone 4N
 <esri26905> : NAD83 / UTM zone 5N
 <esri26906> : NAD83 / UTM zone 6N
 <esri26907> : NAD83 / UTM zone 7N
 <esri26908> : NAD83 / UTM zone 8N
 <esri26909> : NAD83 / UTM zone 9N
 <esri26910> : NAD83 / UTM zone 10N
 <esri26911> : NAD83 / UTM zone 11N
 <esri26912> : NAD83 / UTM zone 12N
 <esri26913> : NAD83 / UTM zone 13N
 <esri26914> : NAD83 / UTM zone 14N
 <esri26915> : NAD83 / UTM zone 15N
 <esri26916> : NAD83 / UTM zone 16N
 <esri26917> : NAD83 / UTM zone 17N
 <esri26918> : NAD83 / UTM zone 18N
 <esri26919> : NAD83 / UTM zone 19N
 <esri26920> : NAD83 / UTM zone 20N
 <esri26921> : NAD83 / UTM zone 21N
 <esri26922> : NAD83 / UTM zone 22N
 <esri26923> : NAD83 / UTM zone 23N
 <esri26929> : NAD83 / Alabama East
 <esri26930> : NAD83 / Alabama West
 <esri26931> : NAD83 / Alaska zone 1
 <esri26932> : NAD83 / Alaska zone 2
 <esri26933> : NAD83 / Alaska zone 3
 <esri26934> : NAD83 / Alaska zone 4
 <esri26935> : NAD83 / Alaska zone 5
 <esri26936> : NAD83 / Alaska zone 6
 <esri26937> : NAD83 / Alaska zone 7
 <esri26938> : NAD83 / Alaska zone 8
 <esri26939> : NAD83 / Alaska zone 9
 <esri26940> : NAD83 / Alaska zone 10
 <esri26941> : NAD83 / California zone 1
 <esri26942> : NAD83 / California zone 2
 <esri26943> : NAD83 / California zone 3
 <esri26944> : NAD83 / California zone 4
 <esri26945> : NAD83 / California zone 5
 <esri26946> : NAD83 / California zone 6
 <esri26948> : NAD83 / Arizona East
 <esri26949> : NAD83 / Arizona Central
 <esri26950> : NAD83 / Arizona West
 <esri26951> : NAD83 / Arkansas North
 <esri26952> : NAD83 / Arkansas South
 <esri26953> : NAD83 / Colorado North
 <esri26954> : NAD83 / Colorado Central
 <esri26955> : NAD83 / Colorado South
 <esri26956> : NAD83 / Connecticut
 <esri26957> : NAD83 / Delaware
 <esri26958> : NAD83 / Florida East
 <esri26959> : NAD83 / Florida West
 <esri26960> : NAD83 / Florida North
 <esri26961> : NAD83 / Hawaii zone 1
 <esri26962> : NAD83 / Hawaii zone 2
 <esri26963> : NAD83 / Hawaii zone 3
 <esri26964> : NAD83 / Hawaii zone 4
 <esri26965> : NAD83 / Hawaii zone 5
 <esri26966> : NAD83 / Georgia East
 <esri26967> : NAD83 / Georgia West
 <esri26968> : NAD83 / Idaho East
 <esri26969> : NAD83 / Idaho Central
 <esri26970> : NAD83 / Idaho West
 <esri26971> : NAD83 / Illinois East
 <esri26972> : NAD83 / Illinois West
 <esri26973> : NAD83 / Indiana East
 <esri26974> : NAD83 / Indiana West
 <esri26975> : NAD83 / Iowa North
 <esri26976> : NAD83 / Iowa South
 <esri26977> : NAD83 / Kansas North
 <esri26978> : NAD83 / Kansas South
 <esri26979> : NAD83 / Kentucky North
 <esri26980> : NAD83 / Kentucky South
 <esri26981> : NAD83 / Louisiana North
 <esri26982> : NAD83 / Louisiana South
 <esri26983> : NAD83 / Maine East
 <esri26984> : NAD83 / Maine West
 <esri26985> : NAD83 / Maryland
 <esri26986> : NAD83 / Massachusetts Mainland
 <esri26987> : NAD83 / Massachusetts Island
 <esri26988> : NAD83 / Michigan North
 <esri26989> : NAD83 / Michigan Central
 <esri26990> : NAD83 / Michigan South
 <esri26991> : NAD83 / Minnesota North
 <esri26992> : NAD83 / Minnesota Central
 <esri26993> : NAD83 / Minnesota South
 <esri26994> : NAD83 / Mississippi East
 <esri26995> : NAD83 / Mississippi West
 <esri26996> : NAD83 / Missouri East
 <esri26997> : NAD83 / Missouri Central
 <esri26998> : NAD83 / Missouri West
 <esri27038> : Nahrwan 1967 / UTM zone 38N
 <esri27039> : Nahrwan 1967 / UTM zone 39N
 <esri27040> : Nahrwan 1967 / UTM zone 40N
 <esri27120> : Naparima 1972 / UTM zone 20N
 <esri27200> : NZGD49 / New Zealand Map Grid
 <esri27205> : NZGD49 / Mount Eden Circuit
 <esri27206> : NZGD49 / Bay of Plenty Circuit
 <esri27207> : NZGD49 / Poverty Bay Circuit
 <esri27208> : NZGD49 / Hawkes Bay Circuit
 <esri27209> : NZGD49 / Taranaki Circuit
 <esri27210> : NZGD49 / Tuhirangi Circuit
 <esri27211> : NZGD49 / Wanganui Circuit
 <esri27212> : NZGD49 / Wairarapa Circuit
 <esri27213> : NZGD49 / Wellington Circuit
 <esri27214> : NZGD49 / Collingwood Circuit
 <esri27215> : NZGD49 / Nelson Circuit
 <esri27216> : NZGD49 / Karamea Circuit
 <esri27217> : NZGD49 / Buller Circuit
 <esri27218> : NZGD49 / Grey Circuit
 <esri27219> : NZGD49 / Amuri Circuit
 <esri27220> : NZGD49 / Marlborough Circuit
 <esri27221> : NZGD49 / Hokitika Circuit
 <esri27222> : NZGD49 / Okarito Circuit
 <esri27223> : NZGD49 / Jacksons Bay Circuit
 <esri27224> : NZGD49 / Mount Pleasant Circuit
 <esri27225> : NZGD49 / Gawler Circuit
 <esri27226> : NZGD49 / Timaru Circuit
 <esri27227> : NZGD49 / Lindis Peak Circuit
 <esri27228> : NZGD49 / Mount Nicholas Circuit
 <esri27229> : NZGD49 / Mount York Circuit
 <esri27230> : NZGD49 / Observation Point Circuit
 <esri27231> : NZGD49 / North Taieri Circuit
 <esri27232> : NZGD49 / Bluff Circuit
 <esri27258> : NZGD49 / UTM zone 58S
 <esri27259> : NZGD49 / UTM zone 59S
 <esri27260> : NZGD49 / UTM zone 60S
 <esri27291> : NZGD49 / North Island Grid
 <esri27292> : NZGD49 / South Island Grid
 <esri27391> : NGO 1948 (Oslo) / NGO zone I
 <esri27392> : NGO 1948 (Oslo) / NGO zone II
 <esri27393> : NGO 1948 (Oslo) / NGO zone III
 <esri27394> : NGO 1948 (Oslo) / NGO zone IV
 <esri27395> : NGO 1948 (Oslo) / NGO zone V
 <esri27396> : NGO 1948 (Oslo) / NGO zone VI
 <esri27397> : NGO 1948 (Oslo) / NGO zone VII
 <esri27398> : NGO 1948 (Oslo) / NGO zone VIII
 <esri27429> : Datum 73 / UTM zone 29N
 <esri27492> : Datum 73 / Modified Portuguese Grid
 <esri27500> : ATF (Paris) / Nord de Guerre
 <esri27561> : NTF (Paris) / Lambert Nord France
 <esri27562> : NTF (Paris) / Lambert Centre France
 <esri27563> : NTF (Paris) / Lambert Sud France
 <esri27564> : NTF (Paris) / Lambert Corse
 <esri27571> : NTF (Paris) / Lambert zone I
 <esri27572> : NTF (Paris) / Lambert zone II
 <esri27573> : NTF (Paris) / Lambert zone III
 <esri27574> : NTF (Paris) / Lambert zone IV
 <esri27581> : NTF (Paris) / France I
 <esri27582> : NTF (Paris) / France II
 <esri27583> : NTF (Paris) / France III
 <esri27584> : NTF (Paris) / France IV
 <esri27591> : NTF (Paris) / Nord France
 <esri27592> : NTF (Paris) / Centre France
 <esri27593> : NTF (Paris) / Sud France
 <esri27594> : NTF (Paris) / Corse
 <esri27700> : OSGB 1936 / British National Grid
 <esri28191> : Palestine 1923 / Palestine Grid
 <esri28192> : Palestine 1923 / Palestine Belt
 <esri28193> : Palestine 1923 / Israeli CS Grid
 <esri28232> : Pointe Noire / UTM zone 32S
 <esri28348> : GDA94 / MGA zone 48
 <esri28349> : GDA94 / MGA zone 49
 <esri28350> : GDA94 / MGA zone 50
 <esri28351> : GDA94 / MGA zone 51
 <esri28352> : GDA94 / MGA zone 52
 <esri28353> : GDA94 / MGA zone 53
 <esri28354> : GDA94 / MGA zone 54
 <esri28355> : GDA94 / MGA zone 55
 <esri28356> : GDA94 / MGA zone 56
 <esri28357> : GDA94 / MGA zone 57
 <esri28358> : GDA94 / MGA zone 58
 <esri28402> : Pulkovo 1942 / Gauss-Kruger zone 2
 <esri28403> : Pulkovo 1942 / Gauss-Kruger zone 3
 <esri28404> : Pulkovo 1942 / Gauss-Kruger zone 4
 <esri28405> : Pulkovo 1942 / Gauss-Kruger zone 5
 <esri28406> : Pulkovo 1942 / Gauss-Kruger zone 6
 <esri28407> : Pulkovo 1942 / Gauss-Kruger zone 7
 <esri28408> : Pulkovo 1942 / Gauss-Kruger zone 8
 <esri28409> : Pulkovo 1942 / Gauss-Kruger zone 9
 <esri28410> : Pulkovo 1942 / Gauss-Kruger zone 10
 <esri28411> : Pulkovo 1942 / Gauss-Kruger zone 11
 <esri28412> : Pulkovo 1942 / Gauss-Kruger zone 12
 <esri28413> : Pulkovo 1942 / Gauss-Kruger zone 13
 <esri28414> : Pulkovo 1942 / Gauss-Kruger zone 14
 <esri28415> : Pulkovo 1942 / Gauss-Kruger zone 15
 <esri28416> : Pulkovo 1942 / Gauss-Kruger zone 16
 <esri28417> : Pulkovo 1942 / Gauss-Kruger zone 17
 <esri28418> : Pulkovo 1942 / Gauss-Kruger zone 18
 <esri28419> : Pulkovo 1942 / Gauss-Kruger zone 19
 <esri28420> : Pulkovo 1942 / Gauss-Kruger zone 20
 <esri28421> : Pulkovo 1942 / Gauss-Kruger zone 21
 <esri28422> : Pulkovo 1942 / Gauss-Kruger zone 22
 <esri28423> : Pulkovo 1942 / Gauss-Kruger zone 23
 <esri28424> : Pulkovo 1942 / Gauss-Kruger zone 24
 <esri28425> : Pulkovo 1942 / Gauss-Kruger zone 25
 <esri28426> : Pulkovo 1942 / Gauss-Kruger zone 26
 <esri28427> : Pulkovo 1942 / Gauss-Kruger zone 27
 <esri28428> : Pulkovo 1942 / Gauss-Kruger zone 28
 <esri28429> : Pulkovo 1942 / Gauss-Kruger zone 29
 <esri28430> : Pulkovo 1942 / Gauss-Kruger zone 30
 <esri28431> : Pulkovo 1942 / Gauss-Kruger zone 31
 <esri28432> : Pulkovo 1942 / Gauss-Kruger zone 32
 <esri28462> : Pulkovo 1942 / Gauss-Kruger 2N
 <esri28463> : Pulkovo 1942 / Gauss-Kruger 3N
 <esri28464> : Pulkovo 1942 / Gauss-Kruger 4N
 <esri28465> : Pulkovo 1942 / Gauss-Kruger 5N
 <esri28466> : Pulkovo 1942 / Gauss-Kruger 6N
 <esri28467> : Pulkovo 1942 / Gauss-Kruger 7N
 <esri28468> : Pulkovo 1942 / Gauss-Kruger 8N
 <esri28469> : Pulkovo 1942 / Gauss-Kruger 9N
 <esri28470> : Pulkovo 1942 / Gauss-Kruger 10N
 <esri28471> : Pulkovo 1942 / Gauss-Kruger 11N
 <esri28472> : Pulkovo 1942 / Gauss-Kruger 12N
 <esri28473> : Pulkovo 1942 / Gauss-Kruger 13N
 <esri28474> : Pulkovo 1942 / Gauss-Kruger 14N
 <esri28475> : Pulkovo 1942 / Gauss-Kruger 15N
 <esri28476> : Pulkovo 1942 / Gauss-Kruger 16N
 <esri28477> : Pulkovo 1942 / Gauss-Kruger 17N
 <esri28478> : Pulkovo 1942 / Gauss-Kruger 18N
 <esri28479> : Pulkovo 1942 / Gauss-Kruger 19N
 <esri28480> : Pulkovo 1942 / Gauss-Kruger 20N
 <esri28481> : Pulkovo 1942 / Gauss-Kruger 21N
 <esri28482> : Pulkovo 1942 / Gauss-Kruger 22N
 <esri28483> : Pulkovo 1942 / Gauss-Kruger 23N
 <esri28484> : Pulkovo 1942 / Gauss-Kruger 24N
 <esri28485> : Pulkovo 1942 / Gauss-Kruger 25N
 <esri28486> : Pulkovo 1942 / Gauss-Kruger 26N
 <esri28487> : Pulkovo 1942 / Gauss-Kruger 27N
 <esri28488> : Pulkovo 1942 / Gauss-Kruger 28N
 <esri28489> : Pulkovo 1942 / Gauss-Kruger 29N
 <esri28490> : Pulkovo 1942 / Gauss-Kruger 30N
 <esri28491> : Pulkovo 1942 / Gauss-Kruger 31N
 <esri28492> : Pulkovo 1942 / Gauss-Kruger 32N
 <esri28600> : Qatar 1974 / Qatar National Grid
 <esri28991> : Amersfoort / RD Old
 <esri28992> : Amersfoort / RD New
 <esri29100> : SAD69 / Brazil Polyconic
 <esri29118> : SAD69 / UTM zone 18N
 <esri29119> : SAD69 / UTM zone 19N
 <esri29120> : SAD69 / UTM zone 20N
 <esri29121> : SAD69 / UTM zone 21N
 <esri29122> : SAD69 / UTM zone 22N
 <esri29177> : SAD69 / UTM zone 17S
 <esri29178> : SAD69 / UTM zone 18S
 <esri29179> : SAD69 / UTM zone 19S
 <esri29180> : SAD69 / UTM zone 20S
 <esri29181> : SAD69 / UTM zone 21S
 <esri29182> : SAD69 / UTM zone 22S
 <esri29183> : SAD69 / UTM zone 23S
 <esri29184> : SAD69 / UTM zone 24S
 <esri29185> : SAD69 / UTM zone 25S
 <esri29220> : Sapper Hill 1943 / UTM zone 20S
 <esri29221> : Sapper Hill 1943 / UTM zone 21S
 <esri29333> : Schwarzeck / UTM zone 33S
 <esri29635> : Sudan / UTM zone 35N
 <esri29636> : Sudan / UTM zone 36N
 <esri29700> : Tananarive (Paris) / Laborde Grid
 <esri29738> : Tananarive / UTM zone 38S
 <esri29739> : Tananarive / UTM zone 39S
 <esri29849> : Timbalai 1948 / UTM zone 49N
 <esri29850> : Timbalai 1948 / UTM zone 50N
 <esri29871> : Timbalai 1948 / R.S.O. Borneo (ch)
 <esri29872> : Timbalai 1948 / R.S.O. Borneo (ft)
 <esri29873> : Timbalai 1948 / R.S.O. Borneo (m)
 <esri29900> : TM65 / Irish National Grid
 <esri29901> : OSNI 1952 / Irish National Grid
 <esri29902> : TM65 / Irish Grid
 <esri29903> : TM75 / Irish Grid
 <esri30161> : Tokyo / Japan Plane Rectangular CS I
 <esri30162> : Tokyo / Japan Plane Rectangular CS II
 <esri30163> : Tokyo / Japan Plane Rectangular CS III
 <esri30164> : Tokyo / Japan Plane Rectangular CS IV
 <esri30165> : Tokyo / Japan Plane Rectangular CS V
 <esri30166> : Tokyo / Japan Plane Rectangular CS VI
 <esri30167> : Tokyo / Japan Plane Rectangular CS VII
 <esri30168> : Tokyo / Japan Plane Rectangular CS VIII
 <esri30169> : Tokyo / Japan Plane Rectangular CS IX
 <esri30170> : Tokyo / Japan Plane Rectangular CS X
 <esri30171> : Tokyo / Japan Plane Rectangular CS XI
 <esri30172> : Tokyo / Japan Plane Rectangular CS XII
 <esri30173> : Tokyo / Japan Plane Rectangular CS XIII
 <esri30174> : Tokyo / Japan Plane Rectangular CS XIV
 <esri30175> : Tokyo / Japan Plane Rectangular CS XV
 <esri30176> : Tokyo / Japan Plane Rectangular CS XVI
 <esri30177> : Tokyo / Japan Plane Rectangular CS XVII
 <esri30178> : Tokyo / Japan Plane Rectangular CS XVIII
 <esri30179> : Tokyo / Japan Plane Rectangular CS XIX
 <esri30200> : Trinidad 1903 / Trinidad Grid
 <esri30339> : TC(1948) / UTM zone 39N
 <esri30340> : TC(1948) / UTM zone 40N
 <esri30491> : Voirol 1875 / Nord Algerie (ancienne)
 <esri30492> : Voirol 1875 / Sud Algerie (ancienne)
 <esri30729> : Nord Sahara 1959 / UTM zone 29N
 <esri30730> : Nord Sahara 1959 / UTM zone 30N
 <esri30731> : Nord Sahara 1959 / UTM zone 31N
 <esri30732> : Nord Sahara 1959 / UTM zone 32N
 <esri30791> : Nord Sahara 1959 / Voirol Unifie Nord
 <esri30792> : Nord Sahara 1959 / Voirol Unifie Sud
 <esri30800> : RT38 2.5 gon W
 <esri31028> : Yoff / UTM zone 28N
 <esri31121> : Zanderij / UTM zone 21N
 <esri31154> : Zanderij / TM 54 NW
 <esri31170> : Zanderij / Suriname Old TM
 <esri31171> : Zanderij / Suriname TM
 <esri31265> : MGI / 3-degree Gauss zone 5
 <esri31266> : MGI / 3-degree Gauss zone 6
 <esri31267> : MGI / 3-degree Gauss zone 7
 <esri31268> : MGI / 3-degree Gauss zone 8
 <esri31275> : MGI / Balkans zone 5
 <esri31276> : MGI / Balkans zone 6
 <esri31277> : MGI / Balkans zone 7
 <esri31278> : MGI / Balkans zone 8
 <esri31279> : MGI / Balkans zone 8
 <esri31281> : MGI (Ferro) / Austria West Zone
 <esri31282> : MGI (Ferro) / Austria Central Zone
 <esri31283> : MGI (Ferro) / Austria East Zone
 <esri31284> : MGI / M28
 <esri31285> : MGI / M31
 <esri31286> : MGI / M34
 <esri31287> : MGI / Austria Lambert
 <esri31291> : MGI (Ferro) / Austria West Zone
 <esri31292> : MGI (Ferro) / Austria Central Zone
 <esri31293> : MGI (Ferro) / Austria East Zone
 <esri31294> : MGI / M28
 <esri31295> : MGI / M31
 <esri31296> : MGI / M34
 <esri31297> : MGI / Austria Lambert
 <esri31300> : Belge 1972 / Belge Lambert 72
 <esri31370> : Belge 1972 / Belgian Lambert 72
 <esri31461> : DHDN / 3-degree Gauss zone 1
 <esri31462> : DHDN / 3-degree Gauss zone 2
 <esri31463> : DHDN / 3-degree Gauss zone 3
 <esri31464> : DHDN / 3-degree Gauss zone 4
 <esri31465> : DHDN / 3-degree Gauss zone 5
 <esri31466> : DHDN / Gauss-Kruger zone 2
 <esri31467> : DHDN / Gauss-Kruger zone 3
 <esri31468> : DHDN / Gauss-Kruger zone 4
 <esri31469> : DHDN / Gauss-Kruger zone 5
 <esri31528> : Conakry 1905 / UTM zone 28N
 <esri31529> : Conakry 1905 / UTM zone 29N
 <esri31600> : Dealul Piscului 1933/ Stereo 33
 <esri31700> : Dealul Piscului 1970/ Stereo 70
 <esri31838> : NGN / UTM zone 38N
 <esri31839> : NGN / UTM zone 39N
 <esri31900> : KUDAMS / KTM
 <esri31986> : SIRGAS / UTM zone 17N
 <esri31987> : SIRGAS / UTM zone 18N
 <esri31988> : SIRGAS / UTM zone 19N
 <esri31989> : SIRGAS / UTM zone 20N
 <esri31990> : SIRGAS / UTM zone 21N
 <esri31991> : SIRGAS / UTM zone 22N
 <esri31992> : SIRGAS / UTM zone 17S
 <esri31993> : SIRGAS / UTM zone 18S
 <esri31994> : SIRGAS / UTM zone 19S
 <esri31995> : SIRGAS / UTM zone 20S
 <esri31996> : SIRGAS / UTM zone 21S
 <esri31997> : SIRGAS / UTM zone 22S
 <esri31998> : SIRGAS / UTM zone 23S
 <esri31999> : SIRGAS / UTM zone 24S
 <esri32000> : SIRGAS / UTM zone 25S
 <esri32001> : NAD27 / Montana North
 <esri32002> : NAD27 / Montana Central
 <esri32003> : NAD27 / Montana South
 <esri32005> : NAD27 / Nebraska North
 <esri32006> : NAD27 / Nebraska South
 <esri32007> : NAD27 / Nevada East
 <esri32008> : NAD27 / Nevada Central
 <esri32009> : NAD27 / Nevada West
 <esri32010> : NAD27 / New Hampshire
 <esri32011> : NAD27 / New Jersey
 <esri32012> : NAD27 / New Mexico East
 <esri32013> : NAD27 / New Mexico Central
 <esri32014> : NAD27 / New Mexico West
 <esri32015> : NAD27 / New York East
 <esri32016> : NAD27 / New York Central
 <esri32017> : NAD27 / New York West
 <esri32018> : NAD27 / New York Long Island
 <esri32019> : NAD27 / North Carolina
 <esri32020> : NAD27 / North Dakota North
 <esri32021> : NAD27 / North Dakota South
 <esri32022> : NAD27 / Ohio North
 <esri32023> : NAD27 / Ohio South
 <esri32024> : NAD27 / Oklahoma North
 <esri32025> : NAD27 / Oklahoma South
 <esri32026> : NAD27 / Oregon North
 <esri32027> : NAD27 / Oregon South
 <esri32028> : NAD27 / Pennsylvania North
 <esri32029> : NAD27 / Pennsylvania South
 <esri32030> : NAD27 / Rhode Island
 <esri32031> : NAD27 / South Carolina North
 <esri32033> : NAD27 / South Carolina South
 <esri32034> : NAD27 / South Dakota North
 <esri32035> : NAD27 / South Dakota South
 <esri32036> : NAD27 / Tennessee
 <esri32037> : NAD27 / Texas North
 <esri32038> : NAD27 / Texas North Central
 <esri32039> : NAD27 / Texas Central
 <esri32040> : NAD27 / Texas South Central
 <esri32041> : NAD27 / Texas South
 <esri32042> : NAD27 / Utah North
 <esri32043> : NAD27 / Utah Central
 <esri32044> : NAD27 / Utah South
 <esri32045> : NAD27 / Vermont
 <esri32046> : NAD27 / Virginia North
 <esri32047> : NAD27 / Virginia South
 <esri32048> : NAD27 / Washington North
 <esri32049> : NAD27 / Washington South
 <esri32050> : NAD27 / West Virginia North
 <esri32051> : NAD27 / West Virginia South
 <esri32052> : NAD27 / Wisconsin North
 <esri32053> : NAD27 / Wisconsin Central
 <esri32054> : NAD27 / Wisconsin South
 <esri32055> : NAD27 / Wyoming East
 <esri32056> : NAD27 / Wyoming East Central
 <esri32057> : NAD27 / Wyoming West Central
 <esri32058> : NAD27 / Wyoming West
 <esri32061> : NAD27 / Guatemala Norte
 <esri32062> : NAD27 / Guatemala Sur
 <esri32064> : NAD27 / BLM 14N (ftUS)
 <esri32065> : NAD27 / BLM 15N (ftUS)
 <esri32066> : NAD27 / BLM 16N (ftUS)
 <esri32067> : NAD27 / BLM 17N (ftUS)
 <esri32074> : NAD27 / BLM 14N (feet)
 <esri32075> : NAD27 / BLM 15N (feet)
 <esri32076> : NAD27 / BLM 16N (feet)
 <esri32077> : NAD27 / BLM 17N (feet)
 <esri32081> : NAD27 / MTM zone 1
 <esri32082> : NAD27 / MTM zone 2
 <esri32083> : NAD27 / MTM zone 3
 <esri32084> : NAD27 / MTM zone 4
 <esri32085> : NAD27 / MTM zone 5
 <esri32086> : NAD27 / MTM zone 6
 <esri32098> : NAD27 / Quebec Lambert
 <esri32100> : NAD83 / Montana
 <esri32104> : NAD83 / Nebraska
 <esri32107> : NAD83 / Nevada East
 <esri32108> : NAD83 / Nevada Central
 <esri32109> : NAD83 / Nevada West
 <esri32110> : NAD83 / New Hampshire
 <esri32111> : NAD83 / New Jersey
 <esri32112> : NAD83 / New Mexico East
 <esri32113> : NAD83 / New Mexico Central
 <esri32114> : NAD83 / New Mexico West
 <esri32115> : NAD83 / New York East
 <esri32116> : NAD83 / New York Central
 <esri32117> : NAD83 / New York West
 <esri32118> : NAD83 / New York Long Island
 <esri32119> : NAD83 / North Carolina
 <esri32120> : NAD83 / North Dakota North
 <esri32121> : NAD83 / North Dakota South
 <esri32122> : NAD83 / Ohio North
 <esri32123> : NAD83 / Ohio South
 <esri32124> : NAD83 / Oklahoma North
 <esri32125> : NAD83 / Oklahoma South
 <esri32126> : NAD83 / Oregon North
 <esri32127> : NAD83 / Oregon South
 <esri32128> : NAD83 / Pennsylvania North
 <esri32129> : NAD83 / Pennsylvania South
 <esri32130> : NAD83 / Rhode Island
 <esri32133> : NAD83 / South Carolina
 <esri32134> : NAD83 / South Dakota North
 <esri32135> : NAD83 / South Dakota South
 <esri32136> : NAD83 / Tennessee
 <esri32137> : NAD83 / Texas North
 <esri32138> : NAD83 / Texas North Central
 <esri32139> : NAD83 / Texas Central
 <esri32140> : NAD83 / Texas South Central
 <esri32141> : NAD83 / Texas South
 <esri32142> : NAD83 / Utah North
 <esri32143> : NAD83 / Utah Central
 <esri32144> : NAD83 / Utah South
 <esri32145> : NAD83 / Vermont
 <esri32146> : NAD83 / Virginia North
 <esri32147> : NAD83 / Virginia South
 <esri32148> : NAD83 / Washington North
 <esri32149> : NAD83 / Washington South
 <esri32150> : NAD83 / West Virginia North
 <esri32151> : NAD83 / West Virginia South
 <esri32152> : NAD83 / Wisconsin North
 <esri32153> : NAD83 / Wisconsin Central
 <esri32154> : NAD83 / Wisconsin South
 <esri32155> : NAD83 / Wyoming East
 <esri32156> : NAD83 / Wyoming East Central
 <esri32157> : NAD83 / Wyoming West Central
 <esri32158> : NAD83 / Wyoming West
 <esri32161> : NAD83 / Puerto Rico & Virgin Is.
 <esri32180> : NAD83 / SCoPQ zone 2
 <esri32181> : NAD83 / MTM zone 1
 <esri32182> : NAD83 / MTM zone 2
 <esri32183> : NAD83 / MTM zone 3
 <esri32184> : NAD83 / MTM zone 4
 <esri32185> : NAD83 / MTM zone 5
 <esri32186> : NAD83 / MTM zone 6
 <esri32187> : NAD83 / MTM zone 7
 <esri32188> : NAD83 / MTM zone 8
 <esri32189> : NAD83 / MTM zone 9
 <esri32190> : NAD83 / MTM zone 10
 <esri32191> : NAD83 / MTM zone 11
 <esri32192> : NAD83 / MTM zone 12
 <esri32193> : NAD83 / MTM zone 13
 <esri32194> : NAD83 / MTM zone 14
 <esri32195> : NAD83 / MTM zone 15
 <esri32196> : NAD83 / MTM zone 16
 <esri32197> : NAD83 / MTM zone 17
 <esri32198> : NAD83 / Quebec Lambert
 <esri32201> : WGS 72 / UTM zone 1N
 <esri32202> : WGS 72 / UTM zone 2N
 <esri32203> : WGS 72 / UTM zone 3N
 <esri32204> : WGS 72 / UTM zone 4N
 <esri32205> : WGS 72 / UTM zone 5N
 <esri32206> : WGS 72 / UTM zone 6N
 <esri32207> : WGS 72 / UTM zone 7N
 <esri32208> : WGS 72 / UTM zone 8N
 <esri32209> : WGS 72 / UTM zone 9N
 <esri32210> : WGS 72 / UTM zone 10N
 <esri32211> : WGS 72 / UTM zone 11N
 <esri32212> : WGS 72 / UTM zone 12N
 <esri32213> : WGS 72 / UTM zone 13N
 <esri32214> : WGS 72 / UTM zone 14N
 <esri32215> : WGS 72 / UTM zone 15N
 <esri32216> : WGS 72 / UTM zone 16N
 <esri32217> : WGS 72 / UTM zone 17N
 <esri32218> : WGS 72 / UTM zone 18N
 <esri32219> : WGS 72 / UTM zone 19N
 <esri32220> : WGS 72 / UTM zone 20N
 <esri32221> : WGS 72 / UTM zone 21N
 <esri32222> : WGS 72 / UTM zone 22N
 <esri32223> : WGS 72 / UTM zone 23N
 <esri32224> : WGS 72 / UTM zone 24N
 <esri32225> : WGS 72 / UTM zone 25N
 <esri32226> : WGS 72 / UTM zone 26N
 <esri32227> : WGS 72 / UTM zone 27N
 <esri32228> : WGS 72 / UTM zone 28N
 <esri32229> : WGS 72 / UTM zone 29N
 <esri32230> : WGS 72 / UTM zone 30N
 <esri32231> : WGS 72 / UTM zone 31N
 <esri32232> : WGS 72 / UTM zone 32N
 <esri32233> : WGS 72 / UTM zone 33N
 <esri32234> : WGS 72 / UTM zone 34N
 <esri32235> : WGS 72 / UTM zone 35N
 <esri32236> : WGS 72 / UTM zone 36N
 <esri32237> : WGS 72 / UTM zone 37N
 <esri32238> : WGS 72 / UTM zone 38N
 <esri32239> : WGS 72 / UTM zone 39N
 <esri32240> : WGS 72 / UTM zone 40N
 <esri32241> : WGS 72 / UTM zone 41N
 <esri32242> : WGS 72 / UTM zone 42N
 <esri32243> : WGS 72 / UTM zone 43N
 <esri32244> : WGS 72 / UTM zone 44N
 <esri32245> : WGS 72 / UTM zone 45N
 <esri32246> : WGS 72 / UTM zone 46N
 <esri32247> : WGS 72 / UTM zone 47N
 <esri32248> : WGS 72 / UTM zone 48N
 <esri32249> : WGS 72 / UTM zone 49N
 <esri32250> : WGS 72 / UTM zone 50N
 <esri32251> : WGS 72 / UTM zone 51N
 <esri32252> : WGS 72 / UTM zone 52N
 <esri32253> : WGS 72 / UTM zone 53N
 <esri32254> : WGS 72 / UTM zone 54N
 <esri32255> : WGS 72 / UTM zone 55N
 <esri32256> : WGS 72 / UTM zone 56N
 <esri32257> : WGS 72 / UTM zone 57N
 <esri32258> : WGS 72 / UTM zone 58N
 <esri32259> : WGS 72 / UTM zone 59N
 <esri32260> : WGS 72 / UTM zone 60N
 <esri32301> : WGS 72 / UTM zone 1S
 <esri32302> : WGS 72 / UTM zone 2S
 <esri32303> : WGS 72 / UTM zone 3S
 <esri32304> : WGS 72 / UTM zone 4S
 <esri32305> : WGS 72 / UTM zone 5S
 <esri32306> : WGS 72 / UTM zone 6S
 <esri32307> : WGS 72 / UTM zone 7S
 <esri32308> : WGS 72 / UTM zone 8S
 <esri32309> : WGS 72 / UTM zone 9S
 <esri32310> : WGS 72 / UTM zone 10S
 <esri32311> : WGS 72 / UTM zone 11S
 <esri32312> : WGS 72 / UTM zone 12S
 <esri32313> : WGS 72 / UTM zone 13S
 <esri32314> : WGS 72 / UTM zone 14S
 <esri32315> : WGS 72 / UTM zone 15S
 <esri32316> : WGS 72 / UTM zone 16S
 <esri32317> : WGS 72 / UTM zone 17S
 <esri32318> : WGS 72 / UTM zone 18S
 <esri32319> : WGS 72 / UTM zone 19S
 <esri32320> : WGS 72 / UTM zone 20S
 <esri32321> : WGS 72 / UTM zone 21S
 <esri32322> : WGS 72 / UTM zone 22S
 <esri32323> : WGS 72 / UTM zone 23S
 <esri32324> : WGS 72 / UTM zone 24S
 <esri32325> : WGS 72 / UTM zone 25S
 <esri32326> : WGS 72 / UTM zone 26S
 <esri32327> : WGS 72 / UTM zone 27S
 <esri32328> : WGS 72 / UTM zone 28S
 <esri32329> : WGS 72 / UTM zone 29S
 <esri32330> : WGS 72 / UTM zone 30S
 <esri32331> : WGS 72 / UTM zone 31S
 <esri32332> : WGS 72 / UTM zone 32S
 <esri32333> : WGS 72 / UTM zone 33S
 <esri32334> : WGS 72 / UTM zone 34S
 <esri32335> : WGS 72 / UTM zone 35S
 <esri32336> : WGS 72 / UTM zone 36S
 <esri32337> : WGS 72 / UTM zone 37S
 <esri32338> : WGS 72 / UTM zone 38S
 <esri32339> : WGS 72 / UTM zone 39S
 <esri32340> : WGS 72 / UTM zone 40S
 <esri32341> : WGS 72 / UTM zone 41S
 <esri32342> : WGS 72 / UTM zone 42S
 <esri32343> : WGS 72 / UTM zone 43S
 <esri32344> : WGS 72 / UTM zone 44S
 <esri32345> : WGS 72 / UTM zone 45S
 <esri32346> : WGS 72 / UTM zone 46S
 <esri32347> : WGS 72 / UTM zone 47S
 <esri32348> : WGS 72 / UTM zone 48S
 <esri32349> : WGS 72 / UTM zone 49S
 <esri32350> : WGS 72 / UTM zone 50S
 <esri32351> : WGS 72 / UTM zone 51S
 <esri32352> : WGS 72 / UTM zone 52S
 <esri32353> : WGS 72 / UTM zone 53S
 <esri32354> : WGS 72 / UTM zone 54S
 <esri32355> : WGS 72 / UTM zone 55S
 <esri32356> : WGS 72 / UTM zone 56S
 <esri32357> : WGS 72 / UTM zone 57S
 <esri32358> : WGS 72 / UTM zone 58S
 <esri32359> : WGS 72 / UTM zone 59S
 <esri32360> : WGS 72 / UTM zone 60S
 <esri32401> : WGS 72BE / UTM zone 1N
 <esri32402> : WGS 72BE / UTM zone 2N
 <esri32403> : WGS 72BE / UTM zone 3N
 <esri32404> : WGS 72BE / UTM zone 4N
 <esri32405> : WGS 72BE / UTM zone 5N
 <esri32406> : WGS 72BE / UTM zone 6N
 <esri32407> : WGS 72BE / UTM zone 7N
 <esri32408> : WGS 72BE / UTM zone 8N
 <esri32409> : WGS 72BE / UTM zone 9N
 <esri32410> : WGS 72BE / UTM zone 10N
 <esri32411> : WGS 72BE / UTM zone 11N
 <esri32412> : WGS 72BE / UTM zone 12N
 <esri32413> : WGS 72BE / UTM zone 13N
 <esri32414> : WGS 72BE / UTM zone 14N
 <esri32415> : WGS 72BE / UTM zone 15N
 <esri32416> : WGS 72BE / UTM zone 16N
 <esri32417> : WGS 72BE / UTM zone 17N
 <esri32418> : WGS 72BE / UTM zone 18N
 <esri32419> : WGS 72BE / UTM zone 19N
 <esri32420> : WGS 72BE / UTM zone 20N
 <esri32421> : WGS 72BE / UTM zone 21N
 <esri32422> : WGS 72BE / UTM zone 22N
 <esri32423> : WGS 72BE / UTM zone 23N
 <esri32424> : WGS 72BE / UTM zone 24N
 <esri32425> : WGS 72BE / UTM zone 25N
 <esri32426> : WGS 72BE / UTM zone 26N
 <esri32427> : WGS 72BE / UTM zone 27N
 <esri32428> : WGS 72BE / UTM zone 28N
 <esri32429> : WGS 72BE / UTM zone 29N
 <esri32430> : WGS 72BE / UTM zone 30N
 <esri32431> : WGS 72BE / UTM zone 31N
 <esri32432> : WGS 72BE / UTM zone 32N
 <esri32433> : WGS 72BE / UTM zone 33N
 <esri32434> : WGS 72BE / UTM zone 34N
 <esri32435> : WGS 72BE / UTM zone 35N
 <esri32436> : WGS 72BE / UTM zone 36N
 <esri32437> : WGS 72BE / UTM zone 37N
 <esri32438> : WGS 72BE / UTM zone 38N
 <esri32439> : WGS 72BE / UTM zone 39N
 <esri32440> : WGS 72BE / UTM zone 40N
 <esri32441> : WGS 72BE / UTM zone 41N
 <esri32442> : WGS 72BE / UTM zone 42N
 <esri32443> : WGS 72BE / UTM zone 43N
 <esri32444> : WGS 72BE / UTM zone 44N
 <esri32445> : WGS 72BE / UTM zone 45N
 <esri32446> : WGS 72BE / UTM zone 46N
 <esri32447> : WGS 72BE / UTM zone 47N
 <esri32448> : WGS 72BE / UTM zone 48N
 <esri32449> : WGS 72BE / UTM zone 49N
 <esri32450> : WGS 72BE / UTM zone 50N
 <esri32451> : WGS 72BE / UTM zone 51N
 <esri32452> : WGS 72BE / UTM zone 52N
 <esri32453> : WGS 72BE / UTM zone 53N
 <esri32454> : WGS 72BE / UTM zone 54N
 <esri32455> : WGS 72BE / UTM zone 55N
 <esri32456> : WGS 72BE / UTM zone 56N
 <esri32457> : WGS 72BE / UTM zone 57N
 <esri32458> : WGS 72BE / UTM zone 58N
 <esri32459> : WGS 72BE / UTM zone 59N
 <esri32460> : WGS 72BE / UTM zone 60N
 <esri32501> : WGS 72BE / UTM zone 1S
 <esri32502> : WGS 72BE / UTM zone 2S
 <esri32503> : WGS 72BE / UTM zone 3S
 <esri32504> : WGS 72BE / UTM zone 4S
 <esri32505> : WGS 72BE / UTM zone 5S
 <esri32506> : WGS 72BE / UTM zone 6S
 <esri32507> : WGS 72BE / UTM zone 7S
 <esri32508> : WGS 72BE / UTM zone 8S
 <esri32509> : WGS 72BE / UTM zone 9S
 <esri32510> : WGS 72BE / UTM zone 10S
 <esri32511> : WGS 72BE / UTM zone 11S
 <esri32512> : WGS 72BE / UTM zone 12S
 <esri32513> : WGS 72BE / UTM zone 13S
 <esri32514> : WGS 72BE / UTM zone 14S
 <esri32515> : WGS 72BE / UTM zone 15S
 <esri32516> : WGS 72BE / UTM zone 16S
 <esri32517> : WGS 72BE / UTM zone 17S
 <esri32518> : WGS 72BE / UTM zone 18S
 <esri32519> : WGS 72BE / UTM zone 19S
 <esri32520> : WGS 72BE / UTM zone 20S
 <esri32521> : WGS 72BE / UTM zone 21S
 <esri32522> : WGS 72BE / UTM zone 22S
 <esri32523> : WGS 72BE / UTM zone 23S
 <esri32524> : WGS 72BE / UTM zone 24S
 <esri32525> : WGS 72BE / UTM zone 25S
 <esri32526> : WGS 72BE / UTM zone 26S
 <esri32527> : WGS 72BE / UTM zone 27S
 <esri32528> : WGS 72BE / UTM zone 28S
 <esri32529> : WGS 72BE / UTM zone 29S
 <esri32530> : WGS 72BE / UTM zone 30S
 <esri32531> : WGS 72BE / UTM zone 31S
 <esri32532> : WGS 72BE / UTM zone 32S
 <esri32533> : WGS 72BE / UTM zone 33S
 <esri32534> : WGS 72BE / UTM zone 34S
 <esri32535> : WGS 72BE / UTM zone 35S
 <esri32536> : WGS 72BE / UTM zone 36S
 <esri32537> : WGS 72BE / UTM zone 37S
 <esri32538> : WGS 72BE / UTM zone 38S
 <esri32539> : WGS 72BE / UTM zone 39S
 <esri32540> : WGS 72BE / UTM zone 40S
 <esri32541> : WGS 72BE / UTM zone 41S
 <esri32542> : WGS 72BE / UTM zone 42S
 <esri32543> : WGS 72BE / UTM zone 43S
 <esri32544> : WGS 72BE / UTM zone 44S
 <esri32545> : WGS 72BE / UTM zone 45S
 <esri32546> : WGS 72BE / UTM zone 46S
 <esri32547> : WGS 72BE / UTM zone 47S
 <esri32548> : WGS 72BE / UTM zone 48S
 <esri32549> : WGS 72BE / UTM zone 49S
 <esri32550> : WGS 72BE / UTM zone 50S
 <esri32551> : WGS 72BE / UTM zone 51S
 <esri32552> : WGS 72BE / UTM zone 52S
 <esri32553> : WGS 72BE / UTM zone 53S
 <esri32554> : WGS 72BE / UTM zone 54S
 <esri32555> : WGS 72BE / UTM zone 55S
 <esri32556> : WGS 72BE / UTM zone 56S
 <esri32557> : WGS 72BE / UTM zone 57S
 <esri32558> : WGS 72BE / UTM zone 58S
 <esri32559> : WGS 72BE / UTM zone 59S
 <esri32560> : WGS 72BE / UTM zone 60S
 <esri32601> : WGS 84 / UTM zone 1N
 <esri32602> : WGS 84 / UTM zone 2N
 <esri32603> : WGS 84 / UTM zone 3N
 <esri32604> : WGS 84 / UTM zone 4N
 <esri32605> : WGS 84 / UTM zone 5N
 <esri32606> : WGS 84 / UTM zone 6N
 <esri32607> : WGS 84 / UTM zone 7N
 <esri32608> : WGS 84 / UTM zone 8N
 <esri32609> : WGS 84 / UTM zone 9N
 <esri32610> : WGS 84 / UTM zone 10N
 <esri32611> : WGS 84 / UTM zone 11N
 <esri32612> : WGS 84 / UTM zone 12N
 <esri32613> : WGS 84 / UTM zone 13N
 <esri32614> : WGS 84 / UTM zone 14N
 <esri32615> : WGS 84 / UTM zone 15N
 <esri32616> : WGS 84 / UTM zone 16N
 <esri32617> : WGS 84 / UTM zone 17N
 <esri32618> : WGS 84 / UTM zone 18N
 <esri32619> : WGS 84 / UTM zone 19N
 <esri32620> : WGS 84 / UTM zone 20N
 <esri32621> : WGS 84 / UTM zone 21N
 <esri32622> : WGS 84 / UTM zone 22N
 <esri32623> : WGS 84 / UTM zone 23N
 <esri32624> : WGS 84 / UTM zone 24N
 <esri32625> : WGS 84 / UTM zone 25N
 <esri32626> : WGS 84 / UTM zone 26N
 <esri32627> : WGS 84 / UTM zone 27N
 <esri32628> : WGS 84 / UTM zone 28N
 <esri32629> : WGS 84 / UTM zone 29N
 <esri32630> : WGS 84 / UTM zone 30N
 <esri32631> : WGS 84 / UTM zone 31N
 <esri32632> : WGS 84 / UTM zone 32N
 <esri32633> : WGS 84 / UTM zone 33N
 <esri32634> : WGS 84 / UTM zone 34N
 <esri32635> : WGS 84 / UTM zone 35N
 <esri32636> : WGS 84 / UTM zone 36N
 <esri32637> : WGS 84 / UTM zone 37N
 <esri32638> : WGS 84 / UTM zone 38N
 <esri32639> : WGS 84 / UTM zone 39N
 <esri32640> : WGS 84 / UTM zone 40N
 <esri32641> : WGS 84 / UTM zone 41N
 <esri32642> : WGS 84 / UTM zone 42N
 <esri32643> : WGS 84 / UTM zone 43N
 <esri32644> : WGS 84 / UTM zone 44N
 <esri32645> : WGS 84 / UTM zone 45N
 <esri32646> : WGS 84 / UTM zone 46N
 <esri32647> : WGS 84 / UTM zone 47N
 <esri32648> : WGS 84 / UTM zone 48N
 <esri32649> : WGS 84 / UTM zone 49N
 <esri32650> : WGS 84 / UTM zone 50N
 <esri32651> : WGS 84 / UTM zone 51N
 <esri32652> : WGS 84 / UTM zone 52N
 <esri32653> : WGS 84 / UTM zone 53N
 <esri32654> : WGS 84 / UTM zone 54N
 <esri32655> : WGS 84 / UTM zone 55N
 <esri32656> : WGS 84 / UTM zone 56N
 <esri32657> : WGS 84 / UTM zone 57N
 <esri32658> : WGS 84 / UTM zone 58N
 <esri32659> : WGS 84 / UTM zone 59N
 <esri32660> : WGS 84 / UTM zone 60N
 <esri32661> : WGS 84 / UPS North
 <esri32701> : WGS 84 / UTM zone 1S
 <esri32702> : WGS 84 / UTM zone 2S
 <esri32703> : WGS 84 / UTM zone 3S
 <esri32704> : WGS 84 / UTM zone 4S
 <esri32705> : WGS 84 / UTM zone 5S
 <esri32706> : WGS 84 / UTM zone 6S
 <esri32707> : WGS 84 / UTM zone 7S
 <esri32708> : WGS 84 / UTM zone 8S
 <esri32709> : WGS 84 / UTM zone 9S
 <esri32710> : WGS 84 / UTM zone 10S
 <esri32711> : WGS 84 / UTM zone 11S
 <esri32712> : WGS 84 / UTM zone 12S
 <esri32713> : WGS 84 / UTM zone 13S
 <esri32714> : WGS 84 / UTM zone 14S
 <esri32715> : WGS 84 / UTM zone 15S
 <esri32716> : WGS 84 / UTM zone 16S
 <esri32717> : WGS 84 / UTM zone 17S
 <esri32718> : WGS 84 / UTM zone 18S
 <esri32719> : WGS 84 / UTM zone 19S
 <esri32720> : WGS 84 / UTM zone 20S
 <esri32721> : WGS 84 / UTM zone 21S
 <esri32722> : WGS 84 / UTM zone 22S
 <esri32723> : WGS 84 / UTM zone 23S
 <esri32724> : WGS 84 / UTM zone 24S
 <esri32725> : WGS 84 / UTM zone 25S
 <esri32726> : WGS 84 / UTM zone 26S
 <esri32727> : WGS 84 / UTM zone 27S
 <esri32728> : WGS 84 / UTM zone 28S
 <esri32729> : WGS 84 / UTM zone 29S
 <esri32730> : WGS 84 / UTM zone 30S
 <esri32731> : WGS 84 / UTM zone 31S
 <esri32732> : WGS 84 / UTM zone 32S
 <esri32733> : WGS 84 / UTM zone 33S
 <esri32734> : WGS 84 / UTM zone 34S
 <esri32735> : WGS 84 / UTM zone 35S
 <esri32736> : WGS 84 / UTM zone 36S
 <esri32737> : WGS 84 / UTM zone 37S
 <esri32738> : WGS 84 / UTM zone 38S
 <esri32739> : WGS 84 / UTM zone 39S
 <esri32740> : WGS 84 / UTM zone 40S
 <esri32741> : WGS 84 / UTM zone 41S
 <esri32742> : WGS 84 / UTM zone 42S
 <esri32743> : WGS 84 / UTM zone 43S
 <esri32744> : WGS 84 / UTM zone 44S
 <esri32745> : WGS 84 / UTM zone 45S
 <esri32746> : WGS 84 / UTM zone 46S
 <esri32747> : WGS 84 / UTM zone 47S
 <esri32748> : WGS 84 / UTM zone 48S
 <esri32749> : WGS 84 / UTM zone 49S
 <esri32750> : WGS 84 / UTM zone 50S
 <esri32751> : WGS 84 / UTM zone 51S
 <esri32752> : WGS 84 / UTM zone 52S
 <esri32753> : WGS 84 / UTM zone 53S
 <esri32754> : WGS 84 / UTM zone 54S
 <esri32755> : WGS 84 / UTM zone 55S
 <esri32756> : WGS 84 / UTM zone 56S
 <esri32757> : WGS 84 / UTM zone 57S
 <esri32758> : WGS 84 / UTM zone 58S
 <esri32759> : WGS 84 / UTM zone 59S
 <esri32760> : WGS 84 / UTM zone 60S
 <esri32761> : WGS 84 / UPS South
 <esri32766> : WGS 84 / TM 36 SE
 <esri20002> : Pulkovo 1995 GK Zone 2
 <esri20003> : Pulkovo 1995 GK Zone 3
 <esri20062> : Pulkovo 1995 GK Zone 2N
 <esri20063> : Pulkovo 1995 GK Zone 3N
 <esri24721> : La Canoa UTM Zone 21N
 <esri26761> : NAD 1927 StatePlane Hawaii 1 FIPS 5101
 <esri26762> : NAD 1927 StatePlane Hawaii 2 FIPS 5102
 <esri26763> : NAD 1927 StatePlane Hawaii 3 FIPS 5103
 <esri26764> : NAD 1927 StatePlane Hawaii 4 FIPS 5104
 <esri26765> : NAD 1927 StatePlane Hawaii 5 FIPS 5105
 <esri26788> : NAD 1927 StatePlane Michigan North FIPS 2111
 <esri26789> : NAD 1927 StatePlane Michigan Central FIPS 2112
 <esri26790> : NAD 1927 StatePlane Michigan South FIPS 2113
 <esri30591> : Nord Algerie
 <esri30592> : Sud Algerie
 <esri31491> : Germany Zone 1
 <esri31492> : Germany Zone 2
 <esri31493> : Germany Zone 3
 <esri31494> : Germany Zone 4
 <esri31495> : Germany Zone 5
 <esri32059> : NAD 1927 StatePlane Puerto Rico FIPS 5201
 <esri32060> : NAD 1927 StatePlane Virgin Islands St Croix FIPS 5202
 <esri53001> : Sphere Plate Carree
 <esri53002> : Sphere Equidistant Cylindrical
 <esri53003> : Sphere Miller Cylindrical
 <esri53004> : Sphere Mercator
 <esri53008> : Sphere Sinusoidal
 <esri53009> : Sphere Mollweide
 <esri53010> : Sphere Eckert VI
 <esri53011> : Sphere Eckert V
 <esri53012> : Sphere Eckert IV
 <esri53013> : Sphere Eckert III
 <esri53014> : Sphere Eckert II
 <esri53015> : Sphere Eckert I
 <esri53016> : Sphere Gall Stereographic
 <esri53017> : Sphere Behrmann
 <esri53018> : Sphere Winkel I
 <esri53019> : Sphere Winkel II
 <esri53021> : Sphere Polyconic
 <esri53022> : Sphere Quartic Authalic
 <esri53023> : Sphere Loximuthal
 <esri53024> : Sphere Bonne
 <esri53025> : Sphere Hotine
 <esri53026> : Sphere Stereographic
 <esri53027> : Sphere Equidistant Conic
 <esri53028> : Sphere Cassini
 <esri53029> : Sphere Van der Grinten I
 <esri53030> : Sphere Robinson
 <esri53031> : Sphere Two Point Equidistant
 <esri53032> : Sphere Azimuthal Equidistant
 <esri54001> : World Plate Carree
 <esri54002> : World Equidistant Cylindrical
 <esri54003> : World Miller Cylindrical
 <esri54004> : World Mercator
 <esri54008> : World Sinusoidal
 <esri54009> : World Mollweide
 <esri54010> : World Eckert VI
 <esri54011> : World Eckert V
 <esri54012> : World Eckert IV
 <esri54013> : World Eckert III
 <esri54014> : World Eckert II
 <esri54015> : World Eckert I
 <esri54016> : World Gall Stereographic
 <esri54017> : World Behrmann
 <esri54018> : World Winkel I
 <esri54019> : World Winkel II
 <esri54021> : World Polyconic
 <esri54022> : World Quartic Authalic
 <esri54023> : World Loximuthal
 <esri54024> : World Bonne
 <esri54025> : World Hotine
 <esri54026> : World Stereographic
 <esri54027> : World Equidistant Conic
 <esri54028> : World Cassini
 <esri54029> : World Van der Grinten I
 <esri54030> : World Robinson
 <esri54031> : World Two Point Equidistant
 <esri54032> : World Azimuthal Equidistant
 <esri65061> : NAD 1927 StatePlane Guam FIPS 5400
 <esri65161> : NAD 1983 StatePlane Guam FIPS 5400
 <esri102001> : Canada Albers Equal Area Conic
 <esri102002> : Canada Lambert Conformal Conic
 <esri102003> : USA Contiguous Albers Equal Area Conic
 <esri102004> : USA Contiguous Lambert Conformal Conic
 <esri102005> : USA Contiguous Equidistant Conic
 <esri102006> : Alaska Albers Equal Area Conic
 <esri102007> : Hawaii Albers Equal Area Conic
 <esri102008> : North America Albers Equal Area Conic
 <esri102009> : North America Lambert Conformal Conic
 <esri102010> : North America Equidistant Conic
 <esri102011> : Africa Sinusoidal
 <esri102012> : Asia Lambert Conformal Conic
 <esri102013> : Europe Albers Equal Area Conic
 <esri102014> : Europe Lambert Conformal Conic
 <esri102015> : South America Lambert Conformal Conic
 <esri102016> : North Pole Azimuthal Equidistant
 <esri102017> : North Pole Lambert Azimuthal Equal Area
 <esri102018> : North Pole Stereographic
 <esri102019> : South Pole Azimuthal Equidistant
 <esri102020> : South Pole Lambert Azimuthal Equal Area
 <esri102021> : South Pole Stereographic
 <esri102022> : Africa Albers Equal Area Conic
 <esri102023> : Africa Equidistant Conic
 <esri102024> : Africa Lambert Conformal Conic
 <esri102025> : Asia North Albers Equal Area Conic
 <esri102026> : Asia North Equidistant Conic
 <esri102027> : Asia North Lambert Conformal Conic
 <esri102028> : Asia South Albers Equal Area Conic
 <esri102029> : Asia South Equidistant Conic
 <esri102030> : Asia South Lambert Conformal Conic
 <esri102031> : Europe Equidistant Conic
 <esri102032> : South America Equidistant Conic
 <esri102033> : South America Albers Equal Area Conic
 <esri102065> : S-JTSK Krovak
 <esri102066> : S-JTSK Ferro Krovak East North
 <esri102067> : S-JTSK Krovak East North
 <esri102091> : Monte Mario Italy 1
 <esri102092> : Monte Mario Italy 2
 <esri102101> : NGO 1948 Norway Zone 1
 <esri102102> : NGO 1948 Norway Zone 2
 <esri102103> : NGO 1948 Norway Zone 3
 <esri102104> : NGO 1948 Norway Zone 4
 <esri102105> : NGO 1948 Norway Zone 5
 <esri102106> : NGO 1948 Norway Zone 6
 <esri102107> : NGO 1948 Norway Zone 7
 <esri102108> : NGO 1948 Norway Zone 8
 <esri102110> : RGF 1993 Lambert 93
 <esri102114> : Old Hawaiian UTM Zone 4N
 <esri102115> : Old Hawaiian UTM Zone 5N
 <esri102120> : NAD 1927 Michigan GeoRef Feet US
 <esri102121> : NAD 1983 Michigan GeoRef Feet US
 <esri102122> : NAD 1927 Michigan GeoRef Meters
 <esri102123> : NAD 1983 Michigan GeoRef Meters
 <esri102132> : NGO 1948 UTM Zone 32N
 <esri102133> : NGO 1948 UTM Zone 33N
 <esri102134> : NGO 1948 UTM Zone 34N
 <esri102135> : NGO 1948 UTM Zone 35N
 <esri102140> : Hong Kong 1980 Grid
 <esri102141> : Hong Kong 1980 UTM Zone 49N
 <esri102142> : Hong Kong 1980 UTM Zone 50N
 <esri102151> : Tokyo UTM Zone 51N
 <esri102152> : Tokyo UTM Zone 52N
 <esri102153> : Tokyo UTM Zone 53N
 <esri102154> : Tokyo UTM Zone 54N
 <esri102155> : Tokyo UTM Zone 55N
 <esri102156> : Tokyo UTM Zone 56N
 <esri102160> : Datum 73 Hayford Gauss IGeoE
 <esri102161> : Datum 73 Hayford Gauss IPCC
 <esri102162> : Graciosa Base SW 1948 UTM Zone 26N
 <esri102163> : Lisboa Bessel Bonne
 <esri102164> : Lisboa Hayford Gauss IGeoE
 <esri102165> : Lisboa Hayford Gauss IPCC
 <esri102166> : Observ Meteorologico 1939 UTM Zone 25N
 <esri102167> : Porto Santo 1936 UTM Zone 28N
 <esri102168> : Sao Braz UTM Zone 26N
 <esri102169> : Selvagem Grande 1938 UTM Zone 28N
 <esri102191> : Nord Maroc Degree
 <esri102192> : Sud Maroc Degree
 <esri102193> : Sahara Degree
 <esri102229> : NAD 1983 HARN StatePlane Alabama East FIPS 0101
 <esri102230> : NAD 1983 HARN StatePlane Alabama West FIPS 0102
 <esri102241> : NAD 1983 HARN StatePlane California I FIPS 0401
 <esri102242> : NAD 1983 HARN StatePlane California II FIPS 0402
 <esri102243> : NAD 1983 HARN StatePlane California III FIPS 0403
 <esri102244> : NAD 1983 HARN StatePlane California IV FIPS 0404
 <esri102245> : NAD 1983 HARN StatePlane California V FIPS 0405
 <esri102246> : NAD 1983 HARN StatePlane California VI FIPS 0406
 <esri102248> : NAD 1983 HARN StatePlane Arizona East FIPS 0201
 <esri102249> : NAD 1983 HARN StatePlane Arizona Central FIPS 0202
 <esri102250> : NAD 1983 HARN StatePlane Arizona West FIPS 0203
 <esri102251> : NAD 1983 HARN StatePlane Arkansas North FIPS 0301
 <esri102252> : NAD 1983 HARN StatePlane Arkansas South FIPS 0302
 <esri102253> : NAD 1983 HARN StatePlane Colorado North FIPS 0501
 <esri102254> : NAD 1983 HARN StatePlane Colorado Central FIPS 0502
 <esri102255> : NAD 1983 HARN StatePlane Colorado South FIPS 0503
 <esri102256> : NAD 1983 HARN StatePlane Connecticut FIPS 0600
 <esri102257> : NAD 1983 HARN StatePlane Delaware FIPS 0700
 <esri102258> : NAD 1983 HARN StatePlane Florida East FIPS 0901
 <esri102259> : NAD 1983 HARN StatePlane Florida West FIPS 0902
 <esri102260> : NAD 1983 HARN StatePlane Florida North FIPS 0903
 <esri102261> : NAD 1983 HARN StatePlane Hawaii 1 FIPS 5101
 <esri102262> : NAD 1983 HARN StatePlane Hawaii 2 FIPS 5102
 <esri102263> : NAD 1983 HARN StatePlane Hawaii 3 FIPS 5103
 <esri102264> : NAD 1983 HARN StatePlane Hawaii 4 FIPS 5104
 <esri102265> : NAD 1983 HARN StatePlane Hawaii 5 FIPS 5105
 <esri102266> : NAD 1983 HARN StatePlane Georgia East FIPS 1001
 <esri102267> : NAD 1983 HARN StatePlane Georgia West FIPS 1002
 <esri102268> : NAD 1983 HARN StatePlane Idaho East FIPS 1101
 <esri102269> : NAD 1983 HARN StatePlane Idaho Central FIPS 1102
 <esri102270> : NAD 1983 HARN StatePlane Idaho West FIPS 1103
 <esri102271> : NAD 1983 HARN StatePlane Illinois East FIPS 1201
 <esri102272> : NAD 1983 HARN StatePlane Illinois West FIPS 1202
 <esri102273> : NAD 1983 HARN StatePlane Indiana East FIPS 1301
 <esri102274> : NAD 1983 HARN StatePlane Indiana West FIPS 1302
 <esri102277> : NAD 1983 HARN StatePlane Kansas North FIPS 1501
 <esri102278> : NAD 1983 HARN StatePlane Kansas South FIPS 1502
 <esri102279> : NAD 1983 HARN StatePlane Kentucky North FIPS 1601
 <esri102280> : NAD 1983 HARN StatePlane Kentucky South FIPS 1602
 <esri102281> : NAD 1983 HARN StatePlane Louisiana North FIPS 1701
 <esri102282> : NAD 1983 HARN StatePlane Louisiana South FIPS 1702
 <esri102283> : NAD 1983 HARN StatePlane Maine East FIPS 1801
 <esri102284> : NAD 1983 HARN StatePlane Maine West FIPS 1802
 <esri102285> : NAD 1983 HARN StatePlane Maryland FIPS 1900
 <esri102286> : NAD 1983 HARN StatePlane Massachusetts Mainland FIPS 2001
 <esri102287> : NAD 1983 HARN StatePlane Massachusetts Island FIPS 2002
 <esri102288> : NAD 1983 HARN StatePlane Michigan North FIPS 2111
 <esri102289> : NAD 1983 HARN StatePlane Michigan Central FIPS 2112
 <esri102290> : NAD 1983 HARN StatePlane Michigan South FIPS 2113
 <esri102291> : NAD 1983 HARN StatePlane Minnesota North FIPS 2201
 <esri102292> : NAD 1983 HARN StatePlane Minnesota Central FIPS 2202
 <esri102293> : NAD 1983 HARN StatePlane Minnesota South FIPS 2203
 <esri102294> : NAD 1983 HARN StatePlane Mississippi East FIPS 2301
 <esri102295> : NAD 1983 HARN StatePlane Mississippi West FIPS 2302
 <esri102296> : NAD 1983 HARN StatePlane Missouri East FIPS 2401
 <esri102297> : NAD 1983 HARN StatePlane Missouri Central FIPS 2402
 <esri102298> : NAD 1983 HARN StatePlane Missouri West FIPS 2403
 <esri102300> : NAD 1983 HARN StatePlane Montana FIPS 2500
 <esri102304> : NAD 1983 HARN StatePlane Nebraska FIPS 2600
 <esri102307> : NAD 1983 HARN StatePlane Nevada East FIPS 2701
 <esri102308> : NAD 1983 HARN StatePlane Nevada Central FIPS 2702
 <esri102309> : NAD 1983 HARN StatePlane Nevada West FIPS 2703
 <esri102310> : NAD 1983 HARN StatePlane New Hampshire FIPS 2800
 <esri102311> : NAD 1983 HARN StatePlane New Jersey FIPS 2900
 <esri102312> : NAD 1983 HARN StatePlane New Mexico East FIPS 3001
 <esri102313> : NAD 1983 HARN StatePlane New Mexico Central FIPS 3002
 <esri102314> : NAD 1983 HARN StatePlane New Mexico West FIPS 3003
 <esri102315> : NAD 1983 HARN StatePlane New York East FIPS 3101
 <esri102316> : NAD 1983 HARN StatePlane New York Central FIPS 3102
 <esri102317> : NAD 1983 HARN StatePlane New York West FIPS 3103
 <esri102318> : NAD 1983 HARN StatePlane New York Long Island FIPS 3104
 <esri102320> : NAD 1983 HARN StatePlane North Dakota North FIPS 3301
 <esri102321> : NAD 1983 HARN StatePlane North Dakota South FIPS 3302
 <esri102322> : NAD 1983 HARN StatePlane Ohio North FIPS 3401
 <esri102323> : NAD 1983 HARN StatePlane Ohio South FIPS 3402
 <esri102324> : NAD 1983 HARN StatePlane Oklahoma North FIPS 3501
 <esri102325> : NAD 1983 HARN StatePlane Oklahoma South FIPS 3502
 <esri102326> : NAD 1983 HARN StatePlane Oregon North FIPS 3601
 <esri102327> : NAD 1983 HARN StatePlane Oregon South FIPS 3602
 <esri102330> : NAD 1983 HARN StatePlane Rhode Island FIPS 3800
 <esri102334> : NAD 1983 HARN StatePlane South Dakota North FIPS 4001
 <esri102335> : NAD 1983 HARN StatePlane South Dakota South FIPS 4002
 <esri102336> : NAD 1983 HARN StatePlane Tennessee FIPS 4100
 <esri102337> : NAD 1983 HARN StatePlane Texas North FIPS 4201
 <esri102338> : NAD 1983 HARN StatePlane Texas North Central FIPS 4202
 <esri102339> : NAD 1983 HARN StatePlane Texas Central FIPS 4203
 <esri102340> : NAD 1983 HARN StatePlane Texas South Central FIPS 4204
 <esri102341> : NAD 1983 HARN StatePlane Texas South FIPS 4205
 <esri102342> : NAD 1983 HARN StatePlane Utah North FIPS 4301
 <esri102343> : NAD 1983 HARN StatePlane Utah Central FIPS 4302
 <esri102344> : NAD 1983 HARN StatePlane Utah South FIPS 4303
 <esri102345> : NAD 1983 HARN StatePlane Vermont FIPS 4400
 <esri102346> : NAD 1983 HARN StatePlane Virginia North FIPS 4501
 <esri102347> : NAD 1983 HARN StatePlane Virginia South FIPS 4502
 <esri102348> : NAD 1983 HARN StatePlane Washington North FIPS 4601
 <esri102349> : NAD 1983 HARN StatePlane Washington South FIPS 4602
 <esri102350> : NAD 1983 HARN StatePlane West Virginia North FIPS 4701
 <esri102351> : NAD 1983 HARN StatePlane West Virginia South FIPS 4702
 <esri102352> : NAD 1983 HARN StatePlane Wisconsin North FIPS 4801
 <esri102353> : NAD 1983 HARN StatePlane Wisconsin Central FIPS 4802
 <esri102354> : NAD 1983 HARN StatePlane Wisconsin South FIPS 4803
 <esri102355> : NAD 1983 HARN StatePlane Wyoming East FIPS 4901
 <esri102356> : NAD 1983 HARN StatePlane Wyoming East Central FIPS 4902
 <esri102357> : NAD 1983 HARN StatePlane Wyoming West Central FIPS 4903
 <esri102358> : NAD 1983 HARN StatePlane Wyoming West FIPS 4904
 <esri102361> : NAD 1983 HARN StatePlane Puerto Rico Virgin Islands FIPS 5200
 <esri102491> : Nord Algerie Ancienne Degree
 <esri102492> : Sud Algerie Ancienne Degree
 <esri102581> : NTF France I degrees
 <esri102582> : NTF France II degrees
 <esri102583> : NTF France III degrees
 <esri102584> : NTF France IV degrees
 <esri102591> : Nord Algerie Degree
 <esri102592> : Sud Algerie Degree
 <esri102629> : NAD 1983 StatePlane Alabama East FIPS 0101 Feet
 <esri102630> : NAD 1983 StatePlane Alabama West FIPS 0102 Feet
 <esri102631> : NAD 1983 StatePlane Alaska 1 FIPS 5001 Feet
 <esri102632> : NAD 1983 StatePlane Alaska 2 FIPS 5002 Feet
 <esri102633> : NAD 1983 StatePlane Alaska 3 FIPS 5003 Feet
 <esri102634> : NAD 1983 StatePlane Alaska 4 FIPS 5004 Feet
 <esri102635> : NAD 1983 StatePlane Alaska 5 FIPS 5005 Feet
 <esri102636> : NAD 1983 StatePlane Alaska 6 FIPS 5006 Feet
 <esri102637> : NAD 1983 StatePlane Alaska 7 FIPS 5007 Feet
 <esri102638> : NAD 1983 StatePlane Alaska 8 FIPS 5008 Feet
 <esri102639> : NAD 1983 StatePlane Alaska 9 FIPS 5009 Feet
 <esri102640> : NAD 1983 StatePlane Alaska 10 FIPS 5010 Feet
 <esri102641> : NAD 1983 StatePlane California I FIPS 0401 Feet
 <esri102642> : NAD 1983 StatePlane California II FIPS 0402 Feet
 <esri102643> : NAD 1983 StatePlane California III FIPS 0403 Feet
 <esri102644> : NAD 1983 StatePlane California IV FIPS 0404 Feet
 <esri102645> : NAD 1983 StatePlane California V FIPS 0405 Feet
 <esri102646> : NAD 1983 StatePlane California VI FIPS 0406 Feet
 <esri102648> : NAD 1983 StatePlane Arizona East FIPS 0201 Feet
 <esri102649> : NAD 1983 StatePlane Arizona Central FIPS 0202 Feet
 <esri102650> : NAD 1983 StatePlane Arizona West FIPS 0203 Feet
 <esri102651> : NAD 1983 StatePlane Arkansas North FIPS 0301 Feet
 <esri102652> : NAD 1983 StatePlane Arkansas South FIPS 0302 Feet
 <esri102653> : NAD 1983 StatePlane Colorado North FIPS 0501 Feet
 <esri102654> : NAD 1983 StatePlane Colorado Central FIPS 0502 Feet
 <esri102655> : NAD 1983 StatePlane Colorado South FIPS 0503 Feet
 <esri102656> : NAD 1983 StatePlane Connecticut FIPS 0600 Feet
 <esri102657> : NAD 1983 StatePlane Delaware FIPS 0700 Feet
 <esri102658> : NAD 1983 StatePlane Florida East FIPS 0901 Feet
 <esri102659> : NAD 1983 StatePlane Florida West FIPS 0902 Feet
 <esri102660> : NAD 1983 StatePlane Florida North FIPS 0903 Feet
 <esri102661> : NAD 1983 StatePlane Hawaii 1 FIPS 5101 Feet
 <esri102662> : NAD 1983 StatePlane Hawaii 2 FIPS 5102 Feet
 <esri102663> : NAD 1983 StatePlane Hawaii 3 FIPS 5103 Feet
 <esri102664> : NAD 1983 StatePlane Hawaii 4 FIPS 5104 Feet
 <esri102665> : NAD 1983 StatePlane Hawaii 5 FIPS 5105 Feet
 <esri102666> : NAD 1983 StatePlane Georgia East FIPS 1001 Feet
 <esri102667> : NAD 1983 StatePlane Georgia West FIPS 1002 Feet
 <esri102668> : NAD 1983 StatePlane Idaho East FIPS 1101 Feet
 <esri102669> : NAD 1983 StatePlane Idaho Central FIPS 1102 Feet
 <esri102670> : NAD 1983 StatePlane Idaho West FIPS 1103 Feet
 <esri102671> : NAD 1983 StatePlane Illinois East FIPS 1201 Feet
 <esri102672> : NAD 1983 StatePlane Illinois West FIPS 1202 Feet
 <esri102673> : NAD 1983 StatePlane Indiana East FIPS 1301 Feet
 <esri102674> : NAD 1983 StatePlane Indiana West FIPS 1302 Feet
 <esri102675> : NAD 1983 StatePlane Iowa North FIPS 1401 Feet
 <esri102676> : NAD 1983 StatePlane Iowa South FIPS 1402 Feet
 <esri102677> : NAD 1983 StatePlane Kansas North FIPS 1501 Feet
 <esri102678> : NAD 1983 StatePlane Kansas South FIPS 1502 Feet
 <esri102679> : NAD 1983 StatePlane Kentucky North FIPS 1601 Feet
 <esri102680> : NAD 1983 StatePlane Kentucky South FIPS 1602 Feet
 <esri102681> : NAD 1983 StatePlane Louisiana North FIPS 1701 Feet
 <esri102682> : NAD 1983 StatePlane Louisiana South FIPS 1702 Feet
 <esri102683> : NAD 1983 StatePlane Maine East FIPS 1801 Feet
 <esri102684> : NAD 1983 StatePlane Maine West FIPS 1802 Feet
 <esri102685> : NAD 1983 StatePlane Maryland FIPS 1900 Feet
 <esri102686> : NAD 1983 StatePlane Massachusetts Mainland FIPS 2001 Feet
 <esri102687> : NAD 1983 StatePlane Massachusetts Island FIPS 2002 Feet
 <esri102688> : NAD 1983 StatePlane Michigan North FIPS 2111 Feet
 <esri102689> : NAD 1983 StatePlane Michigan Central FIPS 2112 Feet
 <esri102690> : NAD 1983 StatePlane Michigan South FIPS 2113 Feet
 <esri102691> : NAD 1983 StatePlane Minnesota North FIPS 2201 Feet
 <esri102692> : NAD 1983 StatePlane Minnesota Central FIPS 2202 Feet
 <esri102693> : NAD 1983 StatePlane Minnesota South FIPS 2203 Feet
 <esri102694> : NAD 1983 StatePlane Mississippi East FIPS 2301 Feet
 <esri102695> : NAD 1983 StatePlane Mississippi West FIPS 2302 Feet
 <esri102696> : NAD 1983 StatePlane Missouri East FIPS 2401 Feet
 <esri102697> : NAD 1983 StatePlane Missouri Central FIPS 2402 Feet
 <esri102698> : NAD 1983 StatePlane Missouri West FIPS 2403 Feet
 <esri102700> : NAD 1983 StatePlane Montana FIPS 2500 Feet
 <esri102704> : NAD 1983 StatePlane Nebraska FIPS 2600 Feet
 <esri102707> : NAD 1983 StatePlane Nevada East FIPS 2701 Feet
 <esri102708> : NAD 1983 StatePlane Nevada Central FIPS 2702 Feet
 <esri102709> : NAD 1983 StatePlane Nevada West FIPS 2703 Feet
 <esri102710> : NAD 1983 StatePlane New Hampshire FIPS 2800 Feet
 <esri102711> : NAD 1983 StatePlane New Jersey FIPS 2900 Feet
 <esri102712> : NAD 1983 StatePlane New Mexico East FIPS 3001 Feet
 <esri102713> : NAD 1983 StatePlane New Mexico Central FIPS 3002 Feet
 <esri102714> : NAD 1983 StatePlane New Mexico West FIPS 3003 Feet
 <esri102715> : NAD 1983 StatePlane New York East FIPS 3101 Feet
 <esri102716> : NAD 1983 StatePlane New York Central FIPS 3102 Feet
 <esri102717> : NAD 1983 StatePlane New York West FIPS 3103 Feet
 <esri102718> : NAD 1983 StatePlane New York Long Island FIPS 3104 Feet
 <esri102719> : NAD 1983 StatePlane North Carolina FIPS 3200 Feet
 <esri102720> : NAD 1983 StatePlane North Dakota North FIPS 3301 Feet
 <esri102721> : NAD 1983 StatePlane North Dakota South FIPS 3302 Feet
 <esri102722> : NAD 1983 StatePlane Ohio North FIPS 3401 Feet
 <esri102723> : NAD 1983 StatePlane Ohio South FIPS 3402 Feet
 <esri102724> : NAD 1983 StatePlane Oklahoma North FIPS 3501 Feet
 <esri102725> : NAD 1983 StatePlane Oklahoma South FIPS 3502 Feet
 <esri102726> : NAD 1983 StatePlane Oregon North FIPS 3601 Feet
 <esri102727> : NAD 1983 StatePlane Oregon South FIPS 3602 Feet
 <esri102728> : NAD 1983 StatePlane Pennsylvania North FIPS 3701 Feet
 <esri102729> : NAD 1983 StatePlane Pennsylvania South FIPS 3702 Feet
 <esri102730> : NAD 1983 StatePlane Rhode Island FIPS 3800 Feet
 <esri102733> : NAD 1983 StatePlane South Carolina FIPS 3900 Feet
 <esri102734> : NAD 1983 StatePlane South Dakota North FIPS 4001 Feet
 <esri102735> : NAD 1983 StatePlane South Dakota South FIPS 4002 Feet
 <esri102736> : NAD 1983 StatePlane Tennessee FIPS 4100 Feet
 <esri102737> : NAD 1983 StatePlane Texas North FIPS 4201 Feet
 <esri102738> : NAD 1983 StatePlane Texas North Central FIPS 4202 Feet
 <esri102739> : NAD 1983 StatePlane Texas Central FIPS 4203 Feet
 <esri102740> : NAD 1983 StatePlane Texas South Central FIPS 4204 Feet
 <esri102741> : NAD 1983 StatePlane Texas South FIPS 4205 Feet
 <esri102742> : NAD 1983 StatePlane Utah North FIPS 4301 Feet
 <esri102743> : NAD 1983 StatePlane Utah Central FIPS 4302 Feet
 <esri102744> : NAD 1983 StatePlane Utah South FIPS 4303 Feet
 <esri102745> : NAD 1983 StatePlane Vermont FIPS 4400 Feet
 <esri102746> : NAD 1983 StatePlane Virginia North FIPS 4501 Feet
 <esri102747> : NAD 1983 StatePlane Virginia South FIPS 4502 Feet
 <esri102748> : NAD 1983 StatePlane Washington North FIPS 4601 Feet
 <esri102749> : NAD 1983 StatePlane Washington South FIPS 4602 Feet
 <esri102750> : NAD 1983 StatePlane West Virginia North FIPS 4701 Feet
 <esri102751> : NAD 1983 StatePlane West Virginia South FIPS 4702 Feet
 <esri102752> : NAD 1983 StatePlane Wisconsin North FIPS 4801 Feet
 <esri102753> : NAD 1983 StatePlane Wisconsin Central FIPS 4802 Feet
 <esri102754> : NAD 1983 StatePlane Wisconsin South FIPS 4803 Feet
 <esri102755> : NAD 1983 StatePlane Wyoming East FIPS 4901 Feet
 <esri102756> : NAD 1983 StatePlane Wyoming East Central FIPS 4902 Feet
 <esri102757> : NAD 1983 StatePlane Wyoming West Central FIPS 4903 Feet
 <esri102758> : NAD 1983 StatePlane Wyoming West FIPS 4904 Feet
 <esri102761> : NAD 1983 StatePlane Puerto Rico Virgin Islands FIPS 5200 Feet
 <esri102766> : NAD 1983 StatePlane Guam FIPS 5400 Feet
 <esri103300> : Belge Lambert 1972
 <esri4023> : GCS International 1967
 <esri4217> : GCS Bern 1898
 <esri4305> : GCS Voirol Unifie 1960
 <esri4404> : GCS Montserrat 1958
 <esri4812> : GCS Voirol Unifie 1960 Paris
 <esri37001> : GCS WGS 1966
 <esri37002> : GCS Fischer 1960
 <esri37003> : GCS Fischer 1968
 <esri37004> : GCS Fischer Modified
 <esri37005> : GCS Hough 1960
 <esri37006> : GCS Everest Modified 1969
 <esri37007> : GCS Walbeck
 <esri37008> : GCS Sphere ARC INFO
 <esri37201> : GCS European 1979
 <esri37202> : GCS Everest Bangladesh
 <esri37203> : GCS Everest India Nepal
 <esri37204> : GCS Hjorsey 1955
 <esri37205> : GCS Hong Kong 1963
 <esri37206> : GCS Oman
 <esri37207> : GCS South Asia Singapore
 <esri37208> : GCS Ayabelle
 <esri37211> : GCS Point 58
 <esri37212> : GCS Beacon E 1945
 <esri37213> : GCS Tern Island 1961
 <esri37214> : GCS Astro 1952
 <esri37215> : GCS Bellevue IGN
 <esri37216> : GCS Canton 1966
 <esri37217> : GCS Chatham Island 1971
 <esri37218> : GCS DOS 1968
 <esri37219> : GCS Easter Island 1967
 <esri37220> : GCS Guam 1963
 <esri37221> : GCS GUX 1
 <esri37222> : GCS Johnston Island 1961
 <esri37223> : GCS Carthage Degree
 <esri37224> : GCS Midway 1961
 <esri37226> : GCS Pitcairn 1967
 <esri37227> : GCS Santo DOS 1965
 <esri37228> : GCS Viti Levu 1916
 <esri37229> : GCS Wake Eniwetok 1960
 <esri37230> : GCS Wake Island 1952
 <esri37231> : GCS Anna 1 1965
 <esri37232> : GCS Gan 1970
 <esri37233> : GCS ISTS 073 1969
 <esri37234> : GCS Kerguelen Island 1949
 <esri37235> : GCS Reunion
 <esri37237> : GCS Ascension Island 1958
 <esri37238> : GCS DOS 71 4
 <esri37239> : GCS Cape Canaveral
 <esri37240> : GCS Fort Thomas 1955
 <esri37241> : GCS Graciosa Base SW 1948
 <esri37242> : GCS ISTS 061 1968
 <esri37243> : GCS LC5 1961
 <esri37245> : GCS Observ Meteorologico 1939
 <esri37246> : GCS Pico de Las Nieves
 <esri37247> : GCS Porto Santo 1936
 <esri37249> : GCS Sao Braz
 <esri37250> : GCS Selvagem Grande 1938
 <esri37251> : GCS Tristan 1968
 <esri37252> : GCS Samoa 1962
 <esri37253> : GCS Camp Area
 <esri37254> : GCS Deception Island
 <esri37255> : GCS Gunung Segara
 <esri37257> : GCS S42 Hungary
 <esri37259> : GCS Kusaie 1951
 <esri37260> : GCS Alaskan Islands
 <esri104000> : GCS Assumed Geographic 1
 <esri104101> : GCS Estonia 1937
 <esri104102> : GCS Hermannskogel
 <esri104103> : GCS Sierra Leone 1960
 <esri104104> : GCS Hong Kong 1980
 <esri104105> : GCS Datum Lisboa Bessel
 <esri104106> : GCS Datum Lisboa Hayford
 <esri104107> : GCS RGF 1993
 <esri104108> : GCS NZGD 2000
 <esri104261> : GCS Merchich Degree
 <esri104304> : GCS Voirol 1875 Degree
 <esri104305> : GCS Voirol Unifie 1960 Degree

 ---------------------------------------------------------
 Google mercator
 ---------------------------------------------------------
 <900913> Chris' funny epsgish code for the google mercator

 ---------------------------------------------------------
 IGNF (French Mapping Agency) projection definitions
 ---------------------------------------------------------
 <ANAA92> : MOP92 (Anaa) Tuamotu
 <APAT86> : MOP86 (Apataki, Rapa, Hao) Tuamotu
 <ATI> : Ancienne Triangulation des Ingenieurs
 <CAD97> : Cadastre 1997
 <CIOBIH> : CIO-BIH
 <CROZ63> : Crozet 1963
 <CSG67> : Guyane CSG67
 <ED50> : ED50
 <EFATE57> : EFATE-IGN 1957
 <FANGA84> : MOP84 (Fangataufa 1984)
 <GUAD48> : Guadeloupe Ste Anne
 <GUADFM49> : Guadeloupe Fort Marigot
 <IGN63> : IGN 1963 (Hiva Oa, Tahuata, Mohotani)
 <IGN72> : IGN 1972 Grande-Terre / Ile des Pins
 <KAUE70> : MHPF70 (Kauehi) Tuamotu
 <KERG62CAR> : Kerguelen - K0
 <LIFOU56> : Lifou - Iles Loyaute (IGN56)
 <LUREF> : Nouvelle Triangulation du Grand Duche du Luxembourg
 <MARE53> : Mare - Iles Loyaute (IGN53)
 <MARQUI72> : IGN 1972 (Eiao, Hiva Oa, Mohotani) Marquises
 <MART38> : Martinique Fort-Desaix
 <MAYO50> : Mayotte Combani
 <MHEFO55F> : MHEFO 1955 (Fatu Huku)
 <MHPF67> : MHPF67 (Mangareva, Agakauitai, Aukena, Mekiro) Gambiers (Iles)
 <MOOREA87> : Moorea 1987
 <MOP90> : MOP90 (Tetiaroa) Iles de la Societe
 <NTF> : Nouvelle Triangulation Francaise
 <NUKU72> : IGN 1972 Nuku Hiva
 <NUKU94> : SAT94 (Nukutavake) Tuamotu
 <OUVEA72CAR> : Ouvea - Iles Loyaute (MHNC 1972 - OUVEA)
 <PETRELS72> : Petrels - IGN 1972
 <RAIA53> : IGN53 (IGN Raiatea-Tahaa) Raiatea-Tahaa-Bora Bora-Huahine
 <REUN47> : Reunion 1947
 <RGF93> : Reseau geodesique francais 1993
 <RGFG95> : Reseau geodesique francais de Guyane 1995
 <RGM04> : RGM04 (Reseau Geodesique de Mayotte 2004)
 <RGNC> : Reseau Geodesique de Nouvelle-Caledonie
 <RGPF> : RGPF (Reseau Geodesique de Polynesie Francaise)
 <RGR92> : Reseau geodesique Reunion 1992
 <RGSPM06> : Reseau Geodesique Saint-Pierre-et-Miquelon (2006)
 <RRAF91> : RRAF 1991 (Reseau de Reference des Antilles Francaises)
 <SAT84> : SAT84 (Rurutu) Iles Australes
 <SHOM84> : SHOM 1984 Martinique Montagne Du Vauclin
 <STPM50> : St Pierre et Miquelon 1950
 <TAHAA> : Raiatea - Tahaa 51-54 (Tahaa, Base Terme Est)
 <TAHI51> : Tahiti-Terme Nord 1951
 <TAHI79> : IGN79 (Tahiti) Iles de la Societe
 <TANNA> : Tanna Bloc Sud
 <TERA50> : Pointe Geologie - Perroud 1950
 <TUBU69> : MHPF 1969 (Tubuai) Iles Australes
 <WALL78> : Wallis-Uvea 1978 (MOP78)
 <WGS72> : World Geodetic System 1972
 <WGS84> : World Geodetic System 1984
 <ANAA92GEO> : MOP92 (Anaa) Tuamotu
 <APAT86GEO> : MOP86 (Apataki, Rapa, Hao) Tuamotu
 <ATIGEO> : Ancienne Triangulation des Ingenieurs
 <CAD97GEO> : Cadastre 1997
 <CROZ63GEO> : Crozet 1963
 <CSG67GEO> : Guyane CSG67 UTM fuseau 21
 <ED50G> : ED50
 <EFATE57GEO> : EFATE-IGN 1957
 <FANGA84GEO> : MOP84 (Fangataufa 1984)
 <GUAD48GEO> : Guadeloupe Ste Anne
 <GUADFM49GEO> : Guadeloupe Fort Marigot
 <IGN63GEO> : IGN 1963 (Hiva Oa, Tahuata, Mohotani)
 <IGN72GEO> : IGN 1972 Grande-Terre / Ile des Pins
 <KAUE70GEO> : MHPF70 (Kauehi) Tuamotu
 <KERG62GEO> : Kerguelen - K0
 <LIFOU56GEO> : Lifou - Iles Loyaute (IGN56)
 <LUXGEO> : Nouvelle Triangulation du Grand Duche du Luxembourg
 <MARE53GEO> : Mare - Iles Loyaute (IGN53)
 <MARQUI72GEO> : IGN 1972 (Eiao, Hiva Oa, Mohotani) Marquises
 <MART38GEO> : Martinique Fort-Desaix
 <MAYO50GEO> : Mayotte Combani
 <MHEFO55FGEO> : MHEFO 1955 (Fatu Huku)
 <MHPF67GEO> : MHPF67 (Mangareva, Agakauitai, Aukena, Mekiro) Gambiers (Iles)
 <MOOREA87GEO> : Moorea 1987
 <MOP90GEO> : MOP90 (Tetiaroa) Iles de la Societe
 <NTFG> : Nouvelle Triangulation Francaise Greenwich degres sexagesimaux
 <NTFP> : Nouvelle Triangulation Francaise Paris grades
 <NUKU72GEO> : IGN 1972 Nuku Hiva
 <NUKU94GEO> : SAT94 (Nukutavake) Tuamotu
 <OUVEA72GEO> : Ouvea - Iles Loyaute (MHNC 1972 - OUVEA)
 <RAIA53GEO> : IGN53 (IGN Raiatea-Tahaa) Raiatea-Tahaa-Bora Bora-Huahine
 <REUN47GEO> : Reunion 1947
 <RGF93G> : Reseau geodesique francais 1993
 <RGFG95GEO> : Reseau geodesique francais de Guyane 1995
 <RGM04GEO> : RGM04 (Reseau Geodesique de Mayotte 2004)
 <RGNCGEO> : Reseau Geodesique de Nouvelle-Caledonie
 <RGPFGEO> : RGPF (Reseau Geodesique de Polynesie Francaise)
 <RGR92GEO> : Reseau geodesique de la Reunion 1992
 <RGSPM06GEO> : Saint-Pierre-et-Miquelon (2006)
 <SAT84GEO> : SAT84 (Rurutu) Iles Australes
 <SHOM84GEO> : SHOM 1984 Martinique Montagne Du Vauclin
 <STPM50GEO> : St Pierre et Miquelon 1950 
 <TAHAAGEO> : Raiatea - Tahaa 51-54 (Tahaa, Base Terme Est)
 <TAHI51GEO> : Tahiti-Terme Nord 1951
 <TAHI79GEO> : IGN79 (Tahiti) Iles de la Societe
 <TANNAGEO> : Tanna Bloc Sud
 <TERA50GEO> : Pointe Geologie - Perroud 1950
 <TUBU69GEO> : MHPF 1969 (Tubuai) Iles Australes
 <WALL78GEO> : Wallis - Uvea 1978 (MOP78)
 <WGS72G> : WGS72
 <WGS84G> : World Geodetic System 1984
 <WGS84RRAFGEO> : Reseau de reference des Antilles francaises (1988-1991)
 <XGEO> : Systeme CIO-BIH
 <ANAA92UTM6S> : MOP92 (Anaa) Tuamotu - UTM fuseau 6 Sud
 <APAT86UTM6S> : MOP86 (Apataki, Rapa, Hao) Tuamotu - UTM fuseau 6 Sud
 <APAT86UTM7S> : MOP86 (Apataki, Rapa, Hao) Tuamotu - UTM fuseau 7 Sud
 <CAD97UTM38S> : Cadastre 1997 - UTM fuseau 38 Sud
 <CROZ63UTM39S> : Crozet 1963
 <CSG67UTM21> : Guyane CSG67 UTM fuseau 21
 <CSG67UTM22> : Guyane CSG67 UTM fuseau 22
 <EFATE57UT59S> : EFATE-IGN 1957 - UTM fuseau 59 Sud
 <FANGA84UTM7S> : Fangataufa 1984 - UTM fuseau 7 Sud
 <GEOPORTALANF> : Geoportail - Antilles francaises
 <GEOPORTALCRZ> : Geoportail - Crozet
 <GEOPORTALFXX> : Geoportail - France metropolitaine
 <GEOPORTALGUF> : Geoportail - Guyane
 <GEOPORTALKER> : Geoportail - Kerguelen
 <GEOPORTALMYT> : Geoportail - Mayotte
 <GEOPORTALNCL> : Geoportail - Nouvelle-Caledonie
 <GEOPORTALPYF> : Geoportail - Polynesie francaise
 <GEOPORTALREU> : Geoportail - Reunion et dependances
 <GEOPORTALSPM> : Geoportail - Saint-Pierre et Miquelon
 <GEOPORTALWLF> : Geoportail - Wallis et Futuna
 <GUAD48UTM20> : Guadeloupe Ste Anne
 <GUADFM49U20> : Guadeloupe Fort Marigot 
 <IGN63UTM7S> : IGN 1963 - Hiva Oa, Tahuata, Mohotani - UTM fuseau 7 Sud
 <IGN72LAM> : IGN 1972 - Lambert Nouvelle Caledonie
 <IGN72UTM58S> : IGN 1972 - UTM fuseau 58 Sud
 <KAUE70UTM6S> : MHPF70 (Kauehi) Tuamotu - UTM fuseau 6 Sud
 <KERG62UTM42S> : Kerguelen 1962
 <LAMB1> : Lambert I
 <LAMB1C> : Lambert I Carto
 <LAMB2> : Lambert II
 <LAMB2C> : Lambert II Carto
 <LAMB3> : Lambert III
 <LAMB3C> : Lambert III Carto
 <LAMB4> : Lambert IV
 <LAMB4C> : Lambert IV Carto
 <LAMB93> : Lambert 93
 <LAMBE> : Lambert II etendu
 <LAMBGC> : Lambert grand champ
 <LUXGAUSSK> : Luxembourg 1929
 <MARE53UTM58S> : Mare - Iles Loyaute - UTM fuseau 58 Sud
 <MART38UTM20> : Martinique Fort-Desaix
 <MAYO50UTM38S> : Mayotte Combani
 <MHPF67UTM8S> : MHPF67 (Mangareva, Agakauitai, Aukena, Mekiro) Gambiers (Iles) - UTM 8 S
 <MILLER> : Geoportail - Monde
 <MOOREA87U6S> : Moorea 1987 - UTM fuseau 6 Sud
 <MOP90UTM6S> : MOP90 (Tetiaroa) Iles de la Societe - UTM fuseau 6 Sud
 <NUKU72U7S> : IGN 1972 Nuku Hiva - UTM fuseau 7 Sud
 <NUKU94UTM7S> : IGN 1994 Nuku Hiva - UTM fuseau 7 Sud
 <OUVEA72U58S> : Ouvea - Iles Loyaute - UTM fuseau 58 Sud
 <RAIA53UTM5S> : IGN53 (IGN Raiatea-Tahaa) Raiatea-Tahaa-Bora Bora-Huahine - UTM fuseau 5
 <REUN47GAUSSL> : Reunion Gauss Laborde
 <RGF93CC42> : Projection conique conforme Zone 1
 <RGF93CC43> : Projection conique conforme Zone 2
 <RGF93CC44> : Projection conique conforme Zone 3
 <RGF93CC45> : Projection conique conforme Zone 4
 <RGF93CC46> : Projection conique conforme Zone 5
 <RGF93CC47> : Projection conique conforme Zone 6
 <RGF93CC48> : Projection conique conforme Zone 7
 <RGF93CC49> : Projection conique conforme Zone 8
 <RGF93CC50> : Projection conique conforme Zone 9
 <RGM04UTM38S> : UTM fuseau 38 Sud (Reseau Geodesique de Mayotte 2004)
 <RGNCLAM> : Reseau Geodesique de Nouvelle-Caledonie - Lambert Nouvelle Caledonie
 <RGNCUTM57S> : Reseau Geodesique de Nouvelle-Caledonie - UTM fuseau 57 Sud
 <RGNCUTM58S> : Reseau Geodesique de Nouvelle-Caledonie - UTM fuseau 58 Sud
 <RGNCUTM59S> : Reseau Geodesique de Nouvelle-Caledonie - UTM fuseau 59 Sud
 <RGPFUTM5S> : RGPF - UTM fuseau 5 Sud
 <RGPFUTM6S> : RGPF - UTM fuseau 6 Sud
 <RGPFUTM7S> : RGPF - UTM fuseau 7 Sud
 <RGR92UTM40S> : RGR92 UTM fuseau 40 Sud
 <RGSPM06U21> : Saint-Pierre-et-Miquelon (2006) UTM Fuseau 21 Nord
 <SAT84UTM5S> : SAT84 (Rurutu) Iles Australes - UTM fuseau 5 Sud
 <STEREOSX> : Stereographique polaire Sud
 <STPM50UTM21> : St Pierre et Miquelon 1950
 <TAHAAUTM05S> : Tahaa 1951
 <TAHI51UTM06S> : Tahiti-Terme Nord UTM fuseau 6 Sud
 <TAHI79UTM6S> : Tahiti 1979
 <TANNAUTM59S> : Tanna Bloc Sud - UTM fuseau 59 Sud
 <TERA50SPTA> : Terre Adelie Stereo polaire Terre Adelie
 <TERA50STEREO> : Terre Adelie 1950
 <TUBU69UTM6S> : Tubuai - Iles Australes - UTM fuseau 6 Sud
 <UTM01SW72> : World Geodetic System 1972 UTM fuseau 01 Sud
 <UTM01SW84> : World Geodetic System 1984 UTM fuseau 01 Sud
 <UTM01W84> : World Geodetic System 1984 UTM fuseau 01
 <UTM02SW84> : World Geodetic System 1984 UTM fuseau 02 Sud
 <UTM02W84> : World Geodetic System 1984 UTM fuseau 02
 <UTM03SW84> : World Geodetic System 1984 UTM fuseau 03 Sud
 <UTM03W84> : World Geodetic System 1984 UTM fuseau 03
 <UTM04SW84> : World Geodetic System 1984 UTM fuseau 04 Sud
 <UTM04W84> : World Geodetic System 1984 UTM fuseau 04
 <UTM05SW84> : World Geodetic System 1984 UTM fuseau 05 Sud
 <UTM05W84> : World Geodetic System 1984 UTM fuseau 05
 <UTM06SW84> : World Geodetic System 1984 UTM fuseau 06 Sud
 <UTM06W84> : World Geodetic System 1984 UTM fuseau 06
 <UTM07SW84> : World Geodetic System 1984 UTM fuseau 07 Sud
 <UTM07W84> : World Geodetic System 1984 UTM fuseau 07
 <UTM08SW84> : World Geodetic System 1984 UTM fuseau 08 Sud
 <UTM08W84> : World Geodetic System 1984 UTM fuseau 08
 <UTM09SW84> : World Geodetic System 1984 UTM fuseau 09 Sud
 <UTM09W84> : World Geodetic System 1984 UTM fuseau 09
 <UTM10SW84> : World Geodetic System 1984 UTM fuseau 10 Sud
 <UTM10W84> : World Geodetic System 1984 UTM fuseau 10
 <UTM11SW84> : World Geodetic System 1984 UTM fuseau 11 Sud
 <UTM11W84> : World Geodetic System 1984 UTM fuseau 11
 <UTM12SW84> : World Geodetic System 1984 UTM fuseau 12 Sud
 <UTM12W84> : World Geodetic System 1984 UTM fuseau 12
 <UTM13SW84> : World Geodetic System 1984 UTM fuseau 13 Sud
 <UTM13W84> : World Geodetic System 1984 UTM fuseau 13
 <UTM14SW84> : World Geodetic System 1984 UTM fuseau 14 Sud
 <UTM14W84> : World Geodetic System 1984 UTM fuseau 14
 <UTM15SW84> : World Geodetic System 1984 UTM fuseau 15 Sud
 <UTM15W84> : World Geodetic System 1984 UTM fuseau 15
 <UTM16SW84> : World Geodetic System 1984 UTM fuseau 16 Sud
 <UTM16W84> : World Geodetic System 1984 UTM fuseau 16
 <UTM17SW84> : World Geodetic System 1984 UTM fuseau 17 Sud
 <UTM17W84> : World Geodetic System 1984 UTM fuseau 17
 <UTM18SW84> : World Geodetic System 1984 UTM fuseau 18 Sud
 <UTM18W84> : World Geodetic System 1984 UTM fuseau 18
 <UTM19SW84> : World Geodetic System 1984 UTM fuseau 19 Sud
 <UTM19W84> : World Geodetic System 1984 UTM fuseau 19
 <UTM20SW84> : World Geodetic System 1984 UTM fuseau 20 Sud
 <UTM20W84> : World Geodetic System 1984 UTM fuseau 20
 <UTM20W84GUAD> : World Geodetic System 1984 UTM fuseau 20 Nord-Guadeloupe
 <UTM20W84MART> : World Geodetic System 1984 UTM fuseau 20 Nord-Martinique
 <UTM21SW84> : World Geodetic System 1984 UTM fuseau 21 Sud
 <UTM21W84> : World Geodetic System 1984 UTM fuseau 21
 <UTM22RGFG95> : RGFG95 UTM fuseau 22 Nord-Guyane
 <UTM22SW84> : World Geodetic System 1984 UTM fuseau 22 Sud
 <UTM22W84> : World Geodetic System 1984 UTM fuseau 22
 <UTM23SW84> : World Geodetic System 1984 UTM fuseau 23 Sud
 <UTM23W84> : World Geodetic System 1984 UTM fuseau 23
 <UTM24SW84> : World Geodetic System 1984 UTM fuseau 24 Sud
 <UTM24W84> : World Geodetic System 1984 UTM fuseau 24
 <UTM25SW84> : World Geodetic System 1984 UTM fuseau 25 Sud
 <UTM25W84> : World Geodetic System 1984 UTM fuseau 25
 <UTM26SW84> : World Geodetic System 1984 UTM fuseau 26 Sud
 <UTM26W84> : World Geodetic System 1984 UTM fuseau 26
 <UTM27SW84> : World Geodetic System 1984 UTM fuseau 27 Sud
 <UTM27W84> : World Geodetic System 1984 UTM fuseau 27
 <UTM28SW84> : World Geodetic System 1984 UTM fuseau 28 Sud
 <UTM28W84> : World Geodetic System 1984 UTM fuseau 28
 <UTM29SW84> : World Geodetic System 1984 UTM fuseau 29 Sud
 <UTM29W84> : World Geodetic System 1984 UTM fuseau 29
 <UTM30> : European Datum 1950 UTM fuseau 30
 <UTM30RGF93> : RGF93 UTM fuseau 30
 <UTM30SW84> : World Geodetic System 1984 UTM fuseau 30 Sud
 <UTM30W72> : World Geodetic System 1972 UTM fuseau 30
 <UTM30W84> : World Geodetic System 1984 UTM fuseau 30
 <UTM31> : European Datum 1950 UTM fuseau 31
 <UTM31RGF93> : RGF93 UTM fuseau 31
 <UTM31SW84> : World Geodetic System 1984 UTM fuseau 31 Sud
 <UTM31W72> : World Geodetic System 1972 UTM fuseau 31
 <UTM31W84> : World Geodetic System 1984 UTM fuseau 31
 <UTM32> : European Datum 1950 UTM fuseau 32
 <UTM32RGF93> : RGF93 UTM fuseau 32
 <UTM32SW84> : World Geodetic System 1984 UTM fuseau 32 Sud
 <UTM32W72> : World Geodetic System 1972 UTM fuseau 32
 <UTM32W84> : World Geodetic System 1984 UTM fuseau 32
 <UTM33SW84> : World Geodetic System 1984 UTM fuseau 33 Sud
 <UTM33W84> : World Geodetic System 1984 UTM fuseau 33
 <UTM34SW84> : World Geodetic System 1984 UTM fuseau 34 Sud
 <UTM34W84> : World Geodetic System 1984 UTM fuseau 34
 <UTM35SW84> : World Geodetic System 1984 UTM fuseau 35 Sud
 <UTM35W84> : World Geodetic System 1984 UTM fuseau 35
 <UTM36SW84> : World Geodetic System 1984 UTM fuseau 36 Sud
 <UTM36W84> : World Geodetic System 1984 UTM fuseau 36
 <UTM37SW84> : World Geodetic System 1984 UTM fuseau 37 Sud
 <UTM37W84> : World Geodetic System 1984 UTM fuseau 37
 <UTM38SW84> : World Geodetic System 1984 UTM fuseau 38 Sud
 <UTM38W84> : World Geodetic System 1984 UTM fuseau 38
 <UTM39SW84> : World Geodetic System 1984 UTM fuseau 39 Sud
 <UTM39W84> : World Geodetic System 1984 UTM fuseau 39
 <UTM40SW84> : World Geodetic System 1984 UTM fuseau 40 Sud
 <UTM40W84> : World Geodetic System 1984 UTM fuseau 40
 <UTM41SW84> : World Geodetic System 1984 UTM fuseau 41 Sud
 <UTM41W84> : World Geodetic System 1984 UTM fuseau 41
 <UTM42SW84> : World Geodetic System 1984 UTM fuseau 42 Sud
 <UTM42W84> : World Geodetic System 1984 UTM fuseau 42
 <UTM43SW84> : World Geodetic System 1984 UTM fuseau 43 Sud
 <UTM43W84> : World Geodetic System 1984 UTM fuseau 43
 <UTM44SW84> : World Geodetic System 1984 UTM fuseau 44 Sud
 <UTM44W84> : World Geodetic System 1984 UTM fuseau 44
 <UTM45SW84> : World Geodetic System 1984 UTM fuseau 45 Sud
 <UTM45W84> : World Geodetic System 1984 UTM fuseau 45
 <UTM46SW84> : World Geodetic System 1984 UTM fuseau 46 Sud
 <UTM46W84> : World Geodetic System 1984 UTM fuseau 46
 <UTM47SW84> : World Geodetic System 1984 UTM fuseau 47 Sud
 <UTM47W84> : World Geodetic System 1984 UTM fuseau 47
 <UTM48SW84> : World Geodetic System 1984 UTM fuseau 48 Sud
 <UTM48W84> : World Geodetic System 1984 UTM fuseau 48
 <UTM49SW84> : World Geodetic System 1984 UTM fuseau 49 Sud
 <UTM49W84> : World Geodetic System 1984 UTM fuseau 49
 <UTM50SW84> : World Geodetic System 1984 UTM fuseau 50 Sud
 <UTM50W84> : World Geodetic System 1984 UTM fuseau 50
 <UTM51SW84> : World Geodetic System 1984 UTM fuseau 51 Sud
 <UTM51W84> : World Geodetic System 1984 UTM fuseau 51
 <UTM52SW84> : World Geodetic System 1984 UTM fuseau 52 Sud
 <UTM52W84> : World Geodetic System 1984 UTM fuseau 52
 <UTM53SW84> : World Geodetic System 1984 UTM fuseau 53 Sud
 <UTM53W84> : World Geodetic System 1984 UTM fuseau 53
 <UTM54SW84> : World Geodetic System 1984 UTM fuseau 54 Sud
 <UTM54W84> : World Geodetic System 1984 UTM fuseau 54
 <UTM55SW84> : World Geodetic System 1984 UTM fuseau 55 Sud
 <UTM55W84> : World Geodetic System 1984 UTM fuseau 55
 <UTM56SW84> : World Geodetic System 1984 UTM fuseau 56 Sud
 <UTM56W84> : World Geodetic System 1984 UTM fuseau 56
 <UTM57SW84> : World Geodetic System 1984 UTM fuseau 57 Sud
 <UTM57W84> : World Geodetic System 1984 UTM fuseau 57
 <UTM58SW84> : World Geodetic System 1984 UTM fuseau 58 Sud
 <UTM58W84> : World Geodetic System 1984 UTM fuseau 58
 <UTM59SW84> : World Geodetic System 1984 UTM fuseau 59 Sud
 <UTM59W84> : World Geodetic System 1984 UTM fuseau 59
 <UTM60SW84> : World Geodetic System 1984 UTM fuseau 60 Sud
 <UTM60W84> : World Geodetic System 1984 UTM fuseau 60
 <WALL78UTM1S> : Wallis-Uvea 1978 (MOP78) UTM 1 SUD

 ---------------------------------------------------------
 Various Non-U.S. Coordinate Systems,
 ---------------------------------------------------------
 <CH1903> : Swiss Coordinate System
 <madagascar> : Laborde grid for Madagascar
 <new_zealand> : New Zealand Map Grid (NZMG) - Projection unique to N.Z. so all factors fixed
 ---------------------------------------------------------
 Secondary grids DMA TM8358.1, p. 4.3
 ---------------------------------------------------------
 <bwi> : British West Indies
 <costa-n> : Costa Rica Norte
 <costa-s> : Costa Rica Sud
 <cuba-n> : Cuba Norte
 <cuba-s> : Cuba Sud
 <domin_rep> : Dominican Republic
 <egypt-1> : Egypt
 <egypt-2> : Egypt
 <egypt-3> : Egypt
 <egypt-4> : Egypt
 <egypt-5> : Egypt
 <el_sal> : El Salvador
 <guat-n> : Guatemala Norte
 <guat-s> : Guatemala Sud
 <haiti> : Haiti
 <hond-n> : Honduras Norte
 <hond-s> : Honduras Sud
 <levant> : Levant
 <nica-n> : Nicaragua Norte
 <nica-s> : Nicaragua Sud
 <nw-africa> : Northwest Africa
 <palestine> : Palestine
 <panama> : Panama

 ---------------------------------------------------------
 other grids in DMA TM8358.1
 ---------------------------------------------------------
 <bng> : British National Grid
 <malay> : West Malaysian RSO Grid
 <india-I> : India Zone I
 <india-IIA> : India Zone IIA
 <india-IIB> : India Zone IIB
 <india-IIIA> : India Zone IIIA
 <india-IIIB> : India Zone IIIB
 <india-IVA> : India Zone IVA
 <india-IVB> : India Zone IVB
 <ceylon> : Ceylon Belt
 <irish> : Irish Transverse Mercator Grid
 <neiez> : Netherlands East Indies Equitorial Zone
 <n-alger> : Nord Algerie Grid
 <n-maroc> : Nord Maroc Grid
 <n-tunis> : Nord Tunisie Grid
 <s-alger> : Sud Algerie Grid
 <s-maroc> : Sud Maroc Grid
 <s-tunis> : Sud Tunisie Grid
     
 ---------------------------------------------------------
 Gauss Krueger Grids for Germany
 ---------------------------------------------------------
 <gk2-d> : Gauss Krueger Grid for Germany
 <gk3-d> : Gauss Krueger Grid for Germany
 <gk4-d> : Gauss Krueger Grid for Germany
