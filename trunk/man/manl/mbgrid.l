.TH mbgrid l "13 November 2002" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbgrid\fP - Grid bathymetry, amplitude, or sidescan data 
from swath sonar data files.

.SH VERSION
Version 5.0

.SH SYNOPSIS
\fBmbgrid\fP \fB-I\fIfilelist\fP \fB-O\fIroot\fP 
\fB-R\fIwest/east/south/north\fP [\fB-A\fIdatatype\fP 
\fB-B\fIborder\fP -C\fIclip\fP \fB-D\fIxdim/ydim\fP 
\fB-E\fIdx/dy/units[!]\fP \fB-F\fImode\fP 
\fB-G\fIgridkind\fP \fB-J\fIprojection\fP
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

The user must specify the bounds of the region to be gridded (\fB-R\fP), 
the dimensions (\fB-D\fP) or node spacing \fB-E\fP of the grid, 
a file containing a 
list of the data files to be used and their data formats (\fB-I\fP), 
and a character string to be used as the root of the
output filenames (\fB-O\fP).  The user can also specify the type of the
input data (\fB-A\fP), the width of the gaussian filter used for 
weighted average gridding (\fB-W\fP), the maximum distance from 
data points that the spline 
interpolation is used (\fB-C\fP), the format of the output files, and
other parameters.

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
 	W(r) = Aexp(r**2/a**2)
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
represent the fraction of the beam contained within the cell. This
approach allows one to sensibly grid data using a resolution
greater than that of the raw data. Thus, if one has data with
a large depth variation, one can generate a grid with a cell spacing
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
lage. This option works best with the median
filter scheme. 

If the \fB-C\fP option is used to set clipping to a value > 0, then a
2D thin plate spline algorithm is applied to the successfully calculated 
grid values to fill in, or interpolate, gaps in the data.  New values
are assigned only at grid points within a specified distance from
the nearest data points (this distance is specified in terms of 
grid point spacing using the \fB-C\fP command).  Thus, small gaps
in the data can be filled, or the whole grid can be filled in with
a smooth interpolation.

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
sidescan data will be gridded.  
Default: \fIdatatype\fP = 1 (bathymetry).
.TP
.B \-B
\fIborder\fP
.br
Sets the border of a smoothly interpolated grid to the value \fIborder\fP
wherever no data exist, provided \fIborder\fP > 0.0.
Default: \fIborder\fP = 0.0
.TP
.B \-C
\fIclip\fP
.br
Sets the clipping dimension for the spline interpolation. If \fIclip\fP=0
no spline interpolation will be done.  If \fIclip\fP>0 then the spline
interpolation will fill data gaps to a distance of \fIclip\fP times
the grid spacing.
Default: \fIclip\fP = 0.
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
yields a pure spline solution. The \fItension\fP must be zero or
greater.  
Default: \fItension\fP = 1.0e10 (pure spline solution).
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
 Version $Id: mbgrid.l,v 5.6 2002-11-14 03:52:25 caress Exp $
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
     Projection:               -Jm26
     Axes annotation:          5m/5m:."File urville_int.grd - Bathymetry Grid":
     Orientation:              portrait
      Number of colors:         11
     Color Pallette:           Haxby Colors
     Colors reversed
 
   Grid Data Attributes:
     Longitude min max:         220.1000   220.3500
     Latitude min max:           -9.7000    -9.4500
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
distribution obtained from http://www.remotesensing.org/ 
in September 2002.The proj library source code has been 
incorporated unchanged into the MB-System package.

The first item on each line is the PCS identifier inside brackets, such
as <UTM10N> or <epsg32749>. To specify using one of these PCS's, use
the \fB-J\fP option, e.g. \fB-J\fP\fIUTM10N\fP or \fB-J\fP\fIepsg32749\fP.

Standard Universal Transverse Mercator (UTM) 
and Universal Polar Stereographic (UPS)
projected coordinate systems:
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

State Plane Coordinate Systems using the
North American Datum 1927
     <nad27_101> : alabama east: nad27
     <nad27_102> : alabama west: nad27
     <nad27_5010> : alaska zone no. 10: nad27
     <nad27_5300> : american samoa: nad27
     <nad27_201> : arizona east: nad27
     <nad27_202> : arizona central: nad27
     <nad27_203> : arizona west: nad27
     <nad27_301> : arkansas north: nad27
     <nad27_302> : arkansas south: nad27
     <nad27_401> : california i: nad27
     <nad27_402> : california ii: nad27
     <nad27_403> : california iii: nad27
     <nad27_404> : california iv: nad27
     <nad27_405> : california v: nad27
     <nad27_406> : california vi: nad27
     <nad27_407> : california vii: nad27
     <nad27_501> : colorado north: nad27
     <nad27_502> : colorado central: nad27
     <nad27_503> : colorado south: nad27
     <nad27_600> : connecticut ---: nad27
     <nad27_700> : delaware ---: nad27
     <nad27_901> : florida east: nad27
     <nad27_902> : florida west: nad27
     <nad27_903> : florida north: nad27
     <nad27_1001> : georgia east: nad27
     <nad27_1002> : georgia west: nad27
     <nad27_5101> : hawaii 1: nad27
     <nad27_5102> : hawaii 2: nad27
     <nad27_5103> : hawaii 3: nad27
     <nad27_5104> : hawaii 4: nad27
     <nad27_5105> : hawaii 5: nad27
     <nad27_1101> : idaho east: nad27
     <nad27_1102> : idaho central: nad27
     <nad27_1103> : idaho west: nad27
     <nad27_1201> : illinois east: nad27
     <nad27_1202> : illinois west: nad27
     <nad27_1301> : indiana east: nad27
     <nad27_1302> : indiana west: nad27
     <nad27_1401> : iowa north: nad27
     <nad27_1402> : iowa south: nad27
     <nad27_1501> : kansas north: nad27
     <nad27_1502> : kansas south: nad27
     <nad27_1601> : kentucky north: nad27
     <nad27_1602> : kentucky south: nad27
     <nad27_1701> : louisiana north: nad27
     <nad27_1702> : louisiana south: nad27
     <nad27_1703> : louisiana offshore: nad27
     <nad27_1801> : maine east: nad27
     <nad27_1802> : maine west: nad27
     <nad27_1900> : maryland ---: nad27
     <nad27_2001> : massachusetts mainland: nad27
     <nad27_2002> : massachusetts island: nad27
     <nad27_2101> : michigan east: nad27
     <nad27_2102> : michigan central/m: nad27
     <nad27_2103> : michigan west: nad27
     <nad27_2111> : michigan north: nad27
     <nad27_2112> : michigan central/l: nad27
     <nad27_2113> : michigan south: nad27
     <nad27_2201> : minnesota north: nad27
     <nad27_2202> : minnesota central: nad27
     <nad27_2203> : minnesota south: nad27
     <nad27_2301> : mississippi east: nad27
     <nad27_2302> : mississippi west: nad27
     <nad27_2401> : missouri east: nad27
     <nad27_2402> : missouri central: nad27
     <nad27_2403> : missouri west: nad27
     <nad27_2501> : montana north: nad27
     <nad27_2502> : montana central: nad27
     <nad27_2503> : montana south: nad27
     <nad27_2601> : nebraska north: nad27
     <nad27_2602> : nebraska south: nad27
     <nad27_2701> : nevada east: nad27
     <nad27_2702> : nevada central: nad27
     <nad27_2703> : nevada west: nad27
     <nad27_2800> : new hampshire ---: nad27
     <nad27_2900> : new jersey ---: nad27
     <nad27_3001> : new mexico east: nad27
     <nad27_3002> : new mexico central: nad27
     <nad27_3003> : new mexico west: nad27
     <nad27_3101> : new york east: nad27
     <nad27_3102> : new york central: nad27
     <nad27_3103> : new york west: nad27
     <nad27_3104> : new york long island: nad27
     <nad27_3200> : north carolina ---: nad27
     <nad27_3301> : north dakota north: nad27
     <nad27_3302> : north dakota south: nad27
     <nad27_3401> : ohio north: nad27
     <nad27_3402> : ohio south: nad27
     <nad27_3501> : oklahoma north: nad27
     <nad27_3502> : oklahoma south: nad27
     <nad27_3601> : oregon north: nad27
     <nad27_3602> : oregon south: nad27
     <nad27_3701> : pennsylvania north: nad27
     <nad27_3702> : pennsylvania south: nad27
     <nad27_3800> : rhode island ---: nad27
     <nad27_3901> : south carolina north: nad27
     <nad27_3902> : south carolina south: nad27
     <nad27_4001> : south dakota north: nad27
     <nad27_4002> : south dakota south: nad27
     <nad27_4100> : tennessee ---: nad27
     <nad27_4201> : texas north: nad27
     <nad27_4202> : texas north central: nad27
     <nad27_4203> : texas central: nad27
     <nad27_4204> : texas south central: nad27
     <nad27_4205> : texas south: nad27
     <nad27_4301> : utah north: nad27
     <nad27_4302> : utah central: nad27
     <nad27_4303> : utah south: nad27
     <nad27_4400> : vermont ---: nad27
     <nad27_4501> : virginia north: nad27
     <nad27_4502> : virginia south: nad27
     <nad27_4601> : washington north: nad27
     <nad27_4602> : washington south: nad27
     <nad27_4701> : west virginia north: nad27
     <nad27_4702> : west virginia south: nad27
     <nad27_4801> : wisconsin north: nad27
     <nad27_4802> : wisconsin central: nad27
     <nad27_4803> : wisconsin south: nad27
     <nad27_4901> : wyoming east: nad27
     <nad27_4902> : wyoming east central: nad27
     <nad27_4903> : wyoming west central: nad27
     <nad27_4904> : wyoming west: nad27
     <nad27_5001> : alaska zone no. 1: nad27
     <nad27_5002> : alaska zone no. 2: nad27
     <nad27_5003> : alaska zone no. 3: nad27
     <nad27_5004> : alaska zone no. 4: nad27
     <nad27_5005> : alaska zone no. 5: nad27
     <nad27_5006> : alaska zone no. 6: nad27
     <nad27_5007> : alaska zone no. 7: nad27
     <nad27_5008> : alaska zone no. 8: nad27
     <nad27_5009> : alaska zone no. 9: nad27
     <nad27_5201> : puerto rico and virgin islands: nad27
     <nad27_5202> : virgin islands st. croix: nad27
     <nad27_5400> : guam island: nad27

State Plane Coordinate Systems using the
North American Datum 1983:
     <nad83_101> : alabama east: nad83
     <nad83_102> : alabama west: nad83
     <nad83_5010> : alaska zone no. 10: nad83
     <nad83_201> : arizona east: nad83
     <nad83_202> : arizona central: nad83
     <nad83_203> : arizona west: nad83
     <nad83_301> : arkansas north: nad83
     <nad83_302> : arkansas south: nad83
     <nad83_401> : california i: nad83
     <nad83_402> : california ii: nad83
     <nad83_403> : california iii: nad83
     <nad83_404> : california iv: nad83
     <nad83_405> : california v: nad83
     <nad83_406> : california vi: nad83
     <nad83_501> : colorado north: nad83
     <nad83_502> : colorado central: nad83
     <nad83_503> : colorado south: nad83
     <nad83_600> : connecticut ---: nad83
     <nad83_700> : delaware ---: nad83
     <nad83_901> : florida east: nad83
     <nad83_902> : florida west: nad83
     <nad83_903> : florida north: nad83
     <nad83_1001> : georgia east: nad83
     <nad83_1002> : georgia west: nad83
     <nad83_5101> : hawaii 1: nad83
     <nad83_5102> : hawaii 2: nad83
     <nad83_5103> : hawaii 3: nad83
     <nad83_5104> : hawaii 4: nad83
     <nad83_5105> : hawaii 5: nad83
     <nad83_1101> : idaho east: nad83
     <nad83_1102> : idaho central: nad83
     <nad83_1103> : idaho west: nad83
     <nad83_1201> : illinois east: nad83
     <nad83_1202> : illinois west: nad83
     <nad83_1301> : indiana east: nad83
     <nad83_1302> : indiana west: nad83
     <nad83_1401> : iowa north: nad83
     <nad83_1402> : iowa south: nad83
     <nad83_1501> : kansas north: nad83
     <nad83_1502> : kansas south: nad83
     <nad83_1601> : kentucky north: nad83
     <nad83_1602> : kentucky south: nad83
     <nad83_1701> : louisiana north: nad83
     <nad83_1702> : louisiana south: nad83
     <nad83_1703> : louisiana offshore: nad83
     <nad83_1801> : maine east: nad83
     <nad83_1802> : maine west: nad83
     <nad83_1900> : maryland ---: nad83
     <nad83_2001> : massachusetts mainland: nad83
     <nad83_2002> : massachusetts island: nad83
     <nad83_2111> : michigan north: nad83
     <nad83_2112> : michigan central/l: nad83
     <nad83_2113> : michigan south: nad83
     <nad83_2201> : minnesota north: nad83
     <nad83_2202> : minnesota central: nad83
     <nad83_2203> : minnesota south: nad83
     <nad83_2301> : mississippi east: nad83
     <nad83_2302> : mississippi west: nad83
     <nad83_2401> : missouri east: nad83
     <nad83_2402> : missouri central: nad83
     <nad83_2403> : missouri west: nad83
     <nad83_2500> : montana: nad83
     <nad83_2600> : nebraska: nad83
     <nad83_2701> : nevada east: nad83
     <nad83_2702> : nevada central: nad83
     <nad83_2703> : nevada west: nad83
     <nad83_2800> : new hampshire ---: nad83
     <nad83_2900> : new jersey ---: nad83
     <nad83_3001> : new mexico east: nad83
     <nad83_3002> : new mexico central: nad83
     <nad83_3003> : new mexico west: nad83
     <nad83_3101> : new york east: nad83
     <nad83_3102> : new york central: nad83
     <nad83_3103> : new york west: nad83
     <nad83_3104> : new york long island: nad83
     <nad83_3200> : north carolina ---: nad83
     <nad83_3301> : north dakota north: nad83
     <nad83_3302> : north dakota south: nad83
     <nad83_3401> : ohio north: nad83
     <nad83_3402> : ohio south: nad83
     <nad83_3501> : oklahoma north: nad83
     <nad83_3502> : oklahoma south: nad83
     <nad83_3601> : oregon north: nad83
     <nad83_3602> : oregon south: nad83
     <nad83_3701> : pennsylvania north: nad83
     <nad83_3702> : pennsylvania south: nad83
     <nad83_3800> : rhode island ---: nad83
     <nad83_3900> : south carolina: nad83
     <nad83_4001> : south dakota north: nad83
     <nad83_4002> : south dakota south: nad83
     <nad83_4100> : tennessee ---: nad83
     <nad83_4201> : texas north: nad83
     <nad83_4202> : texas north central: nad83
     <nad83_4203> : texas central: nad83
     <nad83_4204> : texas south central: nad83
     <nad83_4205> : texas south: nad83
     <nad83_4301> : utah north: nad83
     <nad83_4302> : utah central: nad83
     <nad83_4303> : utah south: nad83
     <nad83_4400> : vermont ---: nad83
     <nad83_4501> : virginia north: nad83
     <nad83_4502> : virginia south: nad83
     <nad83_4601> : washington north: nad83
     <nad83_4602> : washington south: nad83
     <nad83_4701> : west virginia north: nad83
     <nad83_4702> : west virginia south: nad83
     <nad83_4801> : wisconsin north: nad83
     <nad83_4802> : wisconsin central: nad83
     <nad83_4803> : wisconsin south: nad83
     <nad83_4901> : wyoming east: nad83
     <nad83_4902> : wyoming east central: nad83
     <nad83_4903> : wyoming west central: nad83
     <nad83_4904> : wyoming west: nad83
     <nad83_5001> : alaska zone no. 1: nad83
     <nad83_5002> : alaska zone no. 2: nad83
     <nad83_5003> : alaska zone no. 3: nad83
     <nad83_5004> : alaska zone no. 4: nad83
     <nad83_5005> : alaska zone no. 5: nad83
     <nad83_5006> : alaska zone no. 6: nad83
     <nad83_5007> : alaska zone no. 7: nad83
     <nad83_5008> : alaska zone no. 8: nad83
     <nad83_5009> : alaska zone no. 9: nad83
     <nad83_5200> : puerto rico and virgin islands: nad83

Various Non-U.S. Coordinate Systems,
     <CH1903> : Swiss Coordinate System
     <madagascar> : Laborde grid for Madagascar
     <new_zealand> : New Zealand Map Grid (NZMG)
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
     <gk2-d> : Gauss Krueger Grid for Germany
     <gk3-d> : Gauss Krueger Grid for Germany
     <gk4-d> : Gauss Krueger Grid for Germany

Great Lakes Coordinate Systems,
     <erie-etal> : Lake Erie, Ontario and St. Lawrence River.
     <huron> : Lake Huron
     <michigan> : Lake Michigan
     <superior> : Lake Superior, Lake of the Woods

European Petroleum Survey Group (EPSG) Coordinate System Definitions
     <epsg200> : Anguilla 1957 / British West Indies Grid
     <epsg201> : Antigua 1943 / British West Indies Grid
     <epsg202> : Dominica 1945 / British West Indies Grid
     <epsg203> : Grenada 1953 / British West Indies Grid
     <epsg204> : Montserrat 58 / British West Indies Grid
     <epsg205> : St Kitts 1955 / British West Indies Grid
     <epsg206> : St Lucia 1955 / British West Indies Grid
     <epsg207> : St Vincent 45 / British West Indies Grid
     <epsg208> : NAD27(CGQ77) / SCoPQ zone 2
     <epsg209> : NAD27(CGQ77) / SCoPQ zone 3
     <epsg210> : NAD27(CGQ77) / SCoPQ zone 4
     <epsg211> : NAD27(CGQ77) / SCoPQ zone 5
     <epsg212> : NAD27(CGQ77) / SCoPQ zone 6
     <epsg213> : NAD27(CGQ77) / SCoPQ zone 7
     <epsg214> : NAD27(CGQ77) / SCoPQ zone 8
     <epsg215> : NAD27(CGQ77) / SCoPQ zone 9
     <epsg216> : NAD27(CGQ77) / SCoPQ zone 10
     <epsg217> : NAD27(76) / MTM zone 8
     <epsg218> : NAD27(76) / MTM zone 9
     <epsg219> : NAD27(76) / MTM zone 10
     <epsg220> : NAD27(76) / MTM zone 11
     <epsg221> : NAD27(76) / MTM zone 12
     <epsg222> : NAD27(76) / MTM zone 13
     <epsg223> : NAD27(76) / MTM zone 14
     <epsg224> : NAD27(76) / MTM zone 15
     <epsg225> : NAD27(76) / MTM zone 16
     <epsg226> : NAD27(76) / MTM zone 17
     <epsg227> : NAD27(76) / UTM zone 15N
     <epsg228> : NAD27(76) / UTM zone 16N
     <epsg229> : NAD27(76) / UTM zone 17N
     <epsg230> : NAD27(76) / UTM zone 18N
     <epsg231> : NAD27(CGQ77) / UTM zone 17N
     <epsg232> : NAD27(CGQ77) / UTM zone 18N
     <epsg233> : NAD27(CGQ77) / UTM zone 19N
     <epsg234> : NAD27(CGQ77) / UTM zone 20N
     <epsg235> : NAD27(CGQ77) / UTM zone 21N
     <epsg236> : NAD83(CSRS98) / New Brunswick Stereo
     <epsg237> : NAD83(CSRS98) / UTM zone 19N
     <epsg238> : NAD83(CSRS98) / UTM zone 20N
     <epsg239> : Israel / Israeli TM Grid
     <epsg240> : Locodjo 1965 / UTM zone 30N
     <epsg241> : Abidjan 1987 / UTM zone 30N
     <epsg242> : Locodjo 1965 / UTM zone 29N
     <epsg243> : Abidjan 1987 / UTM zone 29N
     <epsg244> : Hanoi 1972 / Gauss-Kruger zone 18
     <epsg245> : Hanoi 1972 / Gauss-Kruger zone 19
     <epsg256> : CH1903+ / LV95
     <epsg257> : Rassadiran / Nakhl e Taqi
     <epsg258> : ED50(ED77) / UTM zone 38N
     <epsg259> : ED50(ED77) / UTM zone 39N
     <epsg260> : ED50(ED77) / UTM zone 40N
     <epsg261> : ED50(ED77) / UTM zone 41N
     <epsg2100> : GGRS87 / Greek Grid
     <epsg2200> : ATS77 / New Brunswick Stereographic
     <epsg2219> : ATS77 / UTM zone 19N
     <epsg2220> : ATS77 / UTM zone 20N
     <epsg2290> : ATS77 / Prince Edward Isl. Stereographic
     <epsg2294> : ATS77 / MTM Nova Scotia zone 4
     <epsg2295> : ATS77 / MTM Nova Scotia zone 5
     <epsg2391> : KKJ / Finland zone 1
     <epsg2392> : KKJ / Finland zone 2
     <epsg2393> : KKJ / Finland Uniform Coordinate System
     <epsg2394> : KKJ / Finland zone 4
     <epsg2400> : RT90 2.5 gon W
     <epsg2550> : Samboja / UTM zone 50S
     <epsg2600> : Lietuvos Koordinoei Sistema 1994
     <epsg2736> : Tete / UTM zone 36S
     <epsg2737> : Tete / UTM zone 37S
     <epsg3036> : Moznet / UTM zone 36S
     <epsg3037> : Moznet / UTM zone 37S
     <epsg3148> : Indian 1960 / UTM zone 48N
     <epsg3149> : Indian 1960 / UTM zone 49N
     <epsg3176> : Indian 1960 / TM 106 NE
     <epsg3300> : Estonian Coordinate System of 1992
     <epsg3439> : PSD93 / UTM zone 39N
     <epsg3440> : PSD93 / UTM zone 40N
     <epsg3561> : Old Hawaiian / Hawaii zone 1
     <epsg3562> : Old Hawaiian / Hawaii zone 2
     <epsg3563> : Old Hawaiian / Hawaii zone 3
     <epsg3564> : Old Hawaiian / Hawaii zone 4
     <epsg3565> : Old Hawaiian / Hawaii zone 5
     <epsg3920> : Puerto Rico / UTM zone 20N
     <epsg3991> : Puerto Rico State Plane CS of 1927
     <epsg3992> : Puerto Rico / St. Croix
     <epsg4001> : Unknown datum based upon ellipsoid 7001
     <epsg4002> : Unknown datum based upon ellipsoid 7002
     <epsg4003> : Unknown datum based upon ellipsoid 7003
     <epsg4004> : Unknown datum based upon ellipsoid 7004
     <epsg4005> : Unknown datum based upon ellipsoid 7005
     <epsg4006> : Unknown datum based upon ellipsoid 7006
     <epsg4007> : Unknown datum based upon ellipsoid 7007
     <epsg4008> : Unknown datum based upon ellipsoid 7008
     <epsg4009> : Unknown datum based upon ellipsoid 7009
     <epsg4010> : Unknown datum based upon ellipsoid 7010
     <epsg4011> : Unknown datum based upon ellipsoid 7011
     <epsg4012> : Unknown datum based upon ellipsoid 7012
     <epsg4013> : Unknown datum based upon ellipsoid 7013
     <epsg4014> : Unknown datum based upon ellipsoid 7014
     <epsg4015> : Unknown datum based upon ellipsoid 7015
     <epsg4016> : Unknown datum based upon ellipsoid 7016
     <epsg4017> : Unknown datum based upon ellipsoid 7017
     <epsg4018> : Unknown datum based upon ellipsoid 7018
     <epsg4019> : Unknown datum based upon ellipsoid 7019
     <epsg4020> : Unknown datum based upon ellipsoid 7020
     <epsg4021> : Unknown datum based upon ellipsoid 7021
     <epsg4022> : Unknown datum based upon ellipsoid 7022
     <epsg4024> : Unknown datum based upon ellipsoid 7024
     <epsg4025> : Unknown datum based upon ellipsoid 7025
     <epsg4027> : Unknown datum based upon ellipsoid 7027
     <epsg4028> : Unknown datum based upon ellipsoid 7028
     <epsg4029> : Unknown datum based upon ellipsoid 7029
     <epsg4030> : Unknown datum based upon ellipsoid 7030
     <epsg4031> : Unknown datum based upon ellipsoid 7031
     <epsg4032> : Unknown datum based upon ellipsoid 7032
     <epsg4033> : Unknown datum based upon ellipsoid 7033
     <epsg4034> : Unknown datum based upon ellipsoid 7034
     <epsg4035> : Unknown datum based upon ellipsoid 7035
     <epsg4036> : Unknown datum based upon ellipsoid 7036
     <epsg4041> : Unknown datum based upon ellipsoid 7041
     <epsg4042> : Unknown datum based upon ellipsoid 7042
     <epsg4043> : Unknown datum based upon ellipsoid 7043
     <epsg4044> : Unknown datum based upon ellipsoid 7044
     <epsg4045> : Unknown datum based upon ellipsoid 7045
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
     <epsg4148> : Hartbeesthoek94
     <epsg4149> : CH1903
     <epsg4150> : CH1903+
     <epsg4151> : CHTRF95
     <epsg4152> : NAD83(HARN)
     <epsg4153> : Rassadiran
     <epsg4154> : ED50(ED77)
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
     <epsg4218> : Bogota
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
     <epsg4245> : Kertau
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
     <epsg4258> : ETRF89
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
     <epsg4272> : GD49
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
     <epsg4285> : Qatar
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
     <epsg4305> : Voirol Unifie
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
     <epsg4801> : Bern 1898 (Bern)
     <epsg4802> : Bogota (Bogota)
     <epsg4803> : Lisbon (Lisbon)
     <epsg4804> : Makassar (Jakarta)
     <epsg4805> : MGI (Ferro)
     <epsg4806> : Monte Mario (Rome)
     <epsg4807> : NTF (Paris)
     <epsg4808> : Padang (Jakarta)
     <epsg4809> : Belge 1950 (Brussels)
     <epsg4810> : Tananarive (Paris)
     <epsg4811> : Voirol 1875 (Paris)
     <epsg4812> : Voirol Unifie (Paris)
     <epsg4813> : Batavia (Jakarta)
     <epsg4814> : RT38 (Stockholm)
     <epsg4815> : Greek (Athens)
     <epsg4816> : Carthage (Paris)
     <epsg4817> : NGO 1948 (Oslo)
     <epsg4901> : ATF (Paris)
     <epsg4902> : NDG (Paris)
     <epsg4903> : Madrid 1870 (Madrid)
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
     <epsg20064> : Pulkovo 1995 / Gauss-Kruger 4N
     <epsg20065> : Pulkovo 1995 / Gauss-Kruger 5N
     <epsg20066> : Pulkovo 1995 / Gauss-Kruger 6N
     <epsg20067> : Pulkovo 1995 / Gauss-Kruger 7N
     <epsg20068> : Pulkovo 1995 / Gauss-Kruger 8N
     <epsg20069> : Pulkovo 1995 / Gauss-Kruger 9N
     <epsg20070> : Pulkovo 1995 / Gauss-Kruger 10N
     <epsg20071> : Pulkovo 1995 / Gauss-Kruger 11N
     <epsg20072> : Pulkovo 1995 / Gauss-Kruger 12N
     <epsg20073> : Pulkovo 1995 / Gauss-Kruger 13N
     <epsg20074> : Pulkovo 1995 / Gauss-Kruger 14N
     <epsg20075> : Pulkovo 1995 / Gauss-Kruger 15N
     <epsg20076> : Pulkovo 1995 / Gauss-Kruger 16N
     <epsg20077> : Pulkovo 1995 / Gauss-Kruger 17N
     <epsg20078> : Pulkovo 1995 / Gauss-Kruger 18N
     <epsg20079> : Pulkovo 1995 / Gauss-Kruger 19N
     <epsg20080> : Pulkovo 1995 / Gauss-Kruger 20N
     <epsg20081> : Pulkovo 1995 / Gauss-Kruger 21N
     <epsg20082> : Pulkovo 1995 / Gauss-Kruger 22N
     <epsg20083> : Pulkovo 1995 / Gauss-Kruger 23N
     <epsg20084> : Pulkovo 1995 / Gauss-Kruger 24N
     <epsg20085> : Pulkovo 1995 / Gauss-Kruger 25N
     <epsg20086> : Pulkovo 1995 / Gauss-Kruger 26N
     <epsg20087> : Pulkovo 1995 / Gauss-Kruger 27N
     <epsg20088> : Pulkovo 1995 / Gauss-Kruger 28N
     <epsg20089> : Pulkovo 1995 / Gauss-Kruger 29N
     <epsg20090> : Pulkovo 1995 / Gauss-Kruger 30N
     <epsg20091> : Pulkovo 1995 / Gauss-Kruger 31N
     <epsg20092> : Pulkovo 1995 / Gauss-Kruger 32N
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
     <epsg20437> : Ain el Abd / UTM zone 37N
     <epsg20438> : Ain el Abd / UTM zone 38N
     <epsg20439> : Ain el Abd / UTM zone 39N
     <epsg20499> : Ain el Abd / Bahrain Grid
     <epsg20538> : Afgooye / UTM zone 38N
     <epsg20539> : Afgooye / UTM zone 39N
     <epsg20790> : Lisbon (Lisbon)/Portuguese National Grid
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
     <epsg21100> : Batavia (Jakarta) / NEIEZ
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
     <epsg21473> : Beijing 1954 / Gauss-Kruger 13N
     <epsg21474> : Beijing 1954 / Gauss-Kruger 14N
     <epsg21475> : Beijing 1954 / Gauss-Kruger 15N
     <epsg21476> : Beijing 1954 / Gauss-Kruger 16N
     <epsg21477> : Beijing 1954 / Gauss-Kruger 17N
     <epsg21478> : Beijing 1954 / Gauss-Kruger 18N
     <epsg21479> : Beijing 1954 / Gauss-Kruger 19N
     <epsg21480> : Beijing 1954 / Gauss-Kruger 20N
     <epsg21481> : Beijing 1954 / Gauss-Kruger 21N
     <epsg21482> : Beijing 1954 / Gauss-Kruger 22N
     <epsg21483> : Beijing 1954 / Gauss-Kruger 23N
     <epsg21500> : Belge 1950 (Brussels) / Belge Lambert 50
     <epsg21780> : Bern 1898 (Bern) / LV03C
     <epsg21781> : CH1903 / LV03
     <epsg21817> : Bogota / UTM zone 17N
     <epsg21818> : Bogota / UTM zone 18N
     <epsg21891> : Bogota / Colombia West zone
     <epsg21892> : Bogota / Colombia Bogota zone
     <epsg21893> : Bogota / Colombia East Central zone
     <epsg21894> : Bogota / Colombia East
     <epsg22032> : Camacupa / UTM zone 32S
     <epsg22033> : Camacupa / UTM zone 33S
     <epsg22091> : Camacupa / TM 11.30 SE
     <epsg22092> : Camacupa / TM 12 SE
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
     <epsg22523> : Corrego Alegre / UTM zone 23S
     <epsg22524> : Corrego Alegre / UTM zone 24S
     <epsg22832> : Douala / UTM zone 32N
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
     <epsg23433> : Garoua / UTM zone 33N
     <epsg23700> : HD72 / EOV
     <epsg23846> : ID74 / UTM zone 46N
     <epsg23847> : ID74 / UTM zone 47N
     <epsg23848> : ID74 / UTM zone 48N
     <epsg23849> : ID74 / UTM zone 49N
     <epsg23850> : ID74 / UTM zone 50N
     <epsg23851> : ID74 / UTM zone 51N
     <epsg23852> : ID74 / UTM zone 52N
     <epsg23853> : ID74 / UTM zone 53N
     <epsg23886> : ID74 / UTM zone 46S
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
     <epsg24500> : Kertau / Singapore Grid
     <epsg24547> : Kertau / UTM zone 47N
     <epsg24548> : Kertau / UTM zone 48N
     <epsg24571> : Kertau / R.S.O. Malaya (ch)
     <epsg24718> : La Canoa / UTM zone 18N
     <epsg24719> : La Canoa / UTM zone 19N
     <epsg24720> : La Canoa / UTM zone 20N
     <epsg24818> : PSAD56 / UTM zone 18N
     <epsg24819> : PSAD56 / UTM zone 19N
     <epsg24820> : PSAD56 / UTM zone 20N
     <epsg24821> : PSAD56 / UTM zone 21N
     <epsg24877> : PSAD56 / UTM zone 17S
     <epsg24878> : PSAD56 / UTM zone 18S
     <epsg24879> : PSAD56 / UTM zone 19S
     <epsg24880> : PSAD56 / UTM zone 20S
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
     <epsg25700> : Makassar (Jakarta) / NEIEZ
     <epsg25828> : ETRF89 / UTM zone 28N
     <epsg25829> : ETRF89 / UTM zone 29N
     <epsg25830> : ETRF89 / UTM zone 30N
     <epsg25831> : ETRF89 / UTM zone 31N
     <epsg25832> : ETRF89 / UTM zone 32N
     <epsg25833> : ETRF89 / UTM zone 33N
     <epsg25834> : ETRF89 / UTM zone 34N
     <epsg25835> : ETRF89 / UTM zone 35N
     <epsg25836> : ETRF89 / UTM zone 36N
     <epsg25837> : ETRF89 / UTM zone 37N
     <epsg25838> : ETRF89 / UTM zone 38N
     <epsg25884> : ETRF89 / TM Baltic93
     <epsg25932> : Malongo 1987 / UTM zone 32S
     <epsg26237> : Massawa / UTM zone 37N
     <epsg26331> : Minna / UTM zone 31N
     <epsg26332> : Minna / UTM zone 32N
     <epsg26391> : Minna / Nigeria West Belt
     <epsg26392> : Minna / Nigeria Mid Belt
     <epsg26393> : Minna / Nigeria East Belt
     <epsg26432> : Mhast / UTM zone 32S
     <epsg26591> : Monte Mario (Rome) / Italy zone 1
     <epsg26592> : Monte Mario (Rome) / Italy zone 2
     <epsg26632> : M'poraloko / UTM zone 32N
     <epsg26692> : M'poraloko / UTM zone 32S
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
     <epsg26747> : NAD27 / California zone VII
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
     <epsg26801> : NAD Michigan / Michigan East
     <epsg26802> : NAD Michigan / Michigan Old Central
     <epsg26803> : NAD Michigan / Michigan West
     <epsg26811> : NAD Michigan / Michigan North
     <epsg26812> : NAD Michigan / Michigan Central
     <epsg26813> : NAD Michigan / Michigan South
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
     <epsg26948> : NAD83 / Arizona  East
     <epsg26949> : NAD83 / Arizona Central
     <epsg26950> : NAD83 / Arizona West
     <epsg26951> : NAD83 / Arkansas North
     <epsg26952> : NAD83 / Arkansas South
     <epsg26953> : NAD83 / Colorado North
     <epsg26954> : NAD83 / Colorado Central
     <epsg26955> : NAD83 / Colorado South
     <epsg26956> : NAD83 / Connecticut
     <epsg26957> : NAD83 / Delaware
     <epsg26958> : NAD83 / Florida  East
     <epsg26959> : NAD83 / Florida  West
     <epsg26960> : NAD83 / Florida North
     <epsg26961> : NAD83 / Hawaii  zone 1
     <epsg26962> : NAD83 / Hawaii  zone 2
     <epsg26963> : NAD83 / Hawaii  zone 3
     <epsg26964> : NAD83 / Hawaii  zone 4
     <epsg26965> : NAD83 / Hawaii  zone 5
     <epsg26966> : NAD83 / Georgia  East
     <epsg26967> : NAD83 / Georgia West
     <epsg26968> : NAD83 / Idaho  East
     <epsg26969> : NAD83 / Idaho  Central
     <epsg26970> : NAD83 / Idaho  West
     <epsg26971> : NAD83 / Illinois  East
     <epsg26972> : NAD83 / Illinois  West
     <epsg26973> : NAD83 / Indiana  East
     <epsg26974> : NAD83 / Indiana  West
     <epsg26975> : NAD83 / Iowa North
     <epsg26976> : NAD83 / Iowa South
     <epsg26977> : NAD83 / Kansas North
     <epsg26978> : NAD83 / Kansas South
     <epsg26979> : NAD83 / Kentucky North
     <epsg26980> : NAD83 / Kentucky South
     <epsg26981> : NAD83 / Louisiana North
     <epsg26982> : NAD83 / Louisiana South
     <epsg26983> : NAD83 / Maine  East
     <epsg26984> : NAD83 / Maine  West
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
     <epsg26996> : NAD83 / Missouri  East
     <epsg26997> : NAD83 / Missouri Central
     <epsg26998> : NAD83 / Missouri  West
     <epsg27038> : Nahrwan 1967 / UTM zone 38N
     <epsg27039> : Nahrwan 1967 / UTM zone 39N
     <epsg27040> : Nahrwan 1967 / UTM zone 40N
     <epsg27120> : Naparima 1972 / UTM zone 20N
     <epsg27291> : GD49 / North Island Grid
     <epsg27292> : GD49 / South Island Grid
     <epsg27391> : NGO 1948 (Oslo) / NGO zone I
     <epsg27392> : NGO 1948 (Oslo) / NGO zone II
     <epsg27393> : NGO 1948 (Oslo) / NGO zone III
     <epsg27394> : NGO 1948 (Oslo) / NGO zone IV
     <epsg27395> : NGO 1948 (Oslo) / NGO zone V
     <epsg27396> : NGO 1948 (Oslo) / NGO zone VI
     <epsg27397> : NGO 1948 (Oslo) / NGO zone VII
     <epsg27398> : NGO 1948 (Oslo) / NGO zone VIII
     <epsg27429> : Datum 73 / UTM zone 29N
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
     <epsg28462> : Pulkovo 1942 / Gauss-Kruger 2N
     <epsg28463> : Pulkovo 1942 / Gauss-Kruger 3N
     <epsg28464> : Pulkovo 1942 / Gauss-Kruger 4N
     <epsg28465> : Pulkovo 1942 / Gauss-Kruger 5N
     <epsg28466> : Pulkovo 1942 / Gauss-Kruger 6N
     <epsg28467> : Pulkovo 1942 / Gauss-Kruger 7N
     <epsg28468> : Pulkovo 1942 / Gauss-Kruger 8N
     <epsg28469> : Pulkovo 1942 / Gauss-Kruger 9N
     <epsg28470> : Pulkovo 1942 / Gauss-Kruger 10N
     <epsg28471> : Pulkovo 1942 / Gauss-Kruger 11N
     <epsg28472> : Pulkovo 1942 / Gauss-Kruger 12N
     <epsg28473> : Pulkovo 1942 / Gauss-Kruger 13N
     <epsg28474> : Pulkovo 1942 / Gauss-Kruger 14N
     <epsg28475> : Pulkovo 1942 / Gauss-Kruger 15N
     <epsg28476> : Pulkovo 1942 / Gauss-Kruger 16N
     <epsg28477> : Pulkovo 1942 / Gauss-Kruger 17N
     <epsg28478> : Pulkovo 1942 / Gauss-Kruger 18N
     <epsg28479> : Pulkovo 1942 / Gauss-Kruger 19N
     <epsg28480> : Pulkovo 1942 / Gauss-Kruger 20N
     <epsg28481> : Pulkovo 1942 / Gauss-Kruger 21N
     <epsg28482> : Pulkovo 1942 / Gauss-Kruger 22N
     <epsg28483> : Pulkovo 1942 / Gauss-Kruger 23N
     <epsg28484> : Pulkovo 1942 / Gauss-Kruger 24N
     <epsg28485> : Pulkovo 1942 / Gauss-Kruger 25N
     <epsg28486> : Pulkovo 1942 / Gauss-Kruger 26N
     <epsg28487> : Pulkovo 1942 / Gauss-Kruger 27N
     <epsg28488> : Pulkovo 1942 / Gauss-Kruger 28N
     <epsg28489> : Pulkovo 1942 / Gauss-Kruger 29N
     <epsg28490> : Pulkovo 1942 / Gauss-Kruger 30N
     <epsg28491> : Pulkovo 1942 / Gauss-Kruger 31N
     <epsg28492> : Pulkovo 1942 / Gauss-Kruger 32N
     <epsg28600> : Qatar / Qatar National Grid
     <epsg28991> : RD / Netherlands Old
     <epsg28992> : RD / Netherlands New
     <epsg29100> : SAD69 / Brazil Polyconic
     <epsg29118> : SAD69 / UTM zone 18N
     <epsg29119> : SAD69 / UTM zone 19N
     <epsg29120> : SAD69 / UTM zone 20N
     <epsg29121> : SAD69 / UTM zone 21N
     <epsg29122> : SAD69 / UTM zone 22N
     <epsg29177> : SAD69 / UTM zone 17S
     <epsg29178> : SAD69 / UTM zone 18S
     <epsg29179> : SAD69 / UTM zone 19S
     <epsg29180> : SAD69 / UTM zone 20S
     <epsg29181> : SAD69 / UTM zone 21S
     <epsg29182> : SAD69 / UTM zone 22S
     <epsg29183> : SAD69 / UTM zone 23S
     <epsg29184> : SAD69 / UTM zone 24S
     <epsg29185> : SAD69 / UTM zone 25S
     <epsg29220> : Sapper Hill 1943 / UTM zone 20S
     <epsg29221> : Sapper Hill 1943 / UTM zone 21S
     <epsg29333> : Schwarzeck / UTM zone 33S
     <epsg29635> : Sudan / UTM zone 35N
     <epsg29636> : Sudan / UTM zone 36N
     <epsg29700> : Tananarive (Paris) / Laborde Grid
     <epsg29738> : Tananarive / UTM zone 38S
     <epsg29739> : Tananarive / UTM zone 39S
     <epsg29849> : Timbalai 1948 / UTM zone 49N
     <epsg29850> : Timbalai 1948 / UTM zone 50N
     <epsg29871> : Timbalai 1948 / R.S.O. Borneo (ch)
     <epsg29872> : Timbalai 1948 / R.S.O. Borneo (ft)
     <epsg29873> : Timbalai 1948 / R.S.O. Borneo (m)
     <epsg29900> : TM65 / Irish National Grid
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
     <epsg30729> : Nord Sahara 1959 / UTM zone 29N
     <epsg30730> : Nord Sahara 1959 / UTM zone 30N
     <epsg30731> : Nord Sahara 1959 / UTM zone 31N
     <epsg30732> : Nord Sahara 1959 / UTM zone 32N
     <epsg30800> : RT38 2.5 gon W
     <epsg31028> : Yoff / UTM zone 28N
     <epsg31121> : Zanderij / UTM zone 21N
     <epsg31291> : MGI (Ferro) / Austria West Zone
     <epsg31292> : MGI (Ferro) / Austria Central Zone
     <epsg31293> : MGI (Ferro) / Austria East Zone
     <epsg31294> : MGI / M28
     <epsg31295> : MGI / M31
     <epsg31296> : MGI / M34
     <epsg31297> : MGI / Austria Lambert
     <epsg31300> : Belge 1972 / Belge Lambert 72
     <epsg31491> : DHDN / Germany zone 1
     <epsg31492> : DHDN / Germany zone 2
     <epsg31493> : DHDN / Germany zone 3
     <epsg31494> : DHDN / Germany zone 4
     <epsg31495> : DHDN / Germany zone 5
     <epsg31600> : Dealul Piscului 1933/ Stereo 33
     <epsg31700> : Dealul Piscului 1970/ Stereo 70
     <epsg31838> : NGN / UTM zone 38N
     <epsg31839> : NGN / UTM zone 39N
     <epsg31900> : KUDAMS / KTM
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
     <epsg32036> : NAD27 / Tennessee
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
     <epsg32074> : NAD27 / BLM 14N (feet)
     <epsg32075> : NAD27 / BLM 15N (feet)
     <epsg32076> : NAD27 / BLM 16N (feet)
     <epsg32077> : NAD27 / BLM 17N (feet)
     <epsg32081> : NAD27 / MTM zone 1
     <epsg32082> : NAD27 / MTM zone 2
     <epsg32083> : NAD27 / MTM zone 3
     <epsg32084> : NAD27 / MTM zone 4
     <epsg32085> : NAD27 / MTM zone 5
     <epsg32086> : NAD27 / MTM zone 6
     <epsg32100> : NAD83 / Montana
     <epsg32104> : NAD83 / Nebraska
     <epsg32107> : NAD83 / Nevada  East
     <epsg32108> : NAD83 / Nevada Central
     <epsg32109> : NAD83 / Nevada West
     <epsg32110> : NAD83 / New Hampshire
     <epsg32111> : NAD83 / New Jersey
     <epsg32112> : NAD83 / New Mexico East
     <epsg32113> : NAD83 / New Mexico Central
     <epsg32114> : NAD83 / New Mexico West
     <epsg32115> : NAD83 / New York  East
     <epsg32116> : NAD83 / New York Central
     <epsg32117> : NAD83 / New York  West
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
