.TH mbm_grid l "13 November 2002" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbm_grid\fP - Create an executable shellscript which will generate
a grid (bathymetry or topography) or mosaic (amplitude or sidescan)
of swath sonar swath data.

.SH VERSION
Version 5.0

.SH SYNOPSIS
mbm_grid \fB-I\fP\fIfile\fP
\fB-F\fP\fIformat\fP \fB-O\fP\fIroot\fP
\fB-R\fP\fIwest/east/south/north\fP
[\fB-A\fP\fIdatatype\fP \fB-B\fP\fIborder\fP \fB-C\fP\fIclip\fP
\fB-D\fP\fIxdim/ydim\fP \fB-E\fP\fIdx/dy/units\fP
{\fB-f\fP\fIpriority_range\fP | \fB-f\fP\fImode\fP}
\fB-G\fP\fIgridkind\fP \fB-H\fP
\fB-J\fP\fIprojection\fP
\fB-L\fP\fIlonflip\fP \fB-M\fP \fB-N\fP \fB-P\fP\fIpings\fP
\fB-S\fP\fIspeed\fP \fB-T\fP\fItension\fP
\fB-U\fP\fI{azimuth/factor | time}\fP
\fB-V\fP \fB-W\fP\fIscale\fP \fB-X\fP\fIextend\fP
\fB-Y\fP\fIpriority_file\fP \fB-Z\fP\fIbathdef\fP]

.SH DESCRIPTION
\fBmbm_grid\fP is a macro to generate a shellscript of MB-System commands
which, when executed, will generate a grid or mosaic of the
specified swath sonar data. The grid or mosaic may be of
bathymetry (positive down, \fB-A\fP\fI1\fP),
topography (positive up, \fB-A\fP\fI2\fP),
amplitude (\fB-A\fP\fI3\fP), or sidescan data (\fB-A\fP\fI4\fP).
The primary purpose of this
macro is to allow the simple, semi-automated production of
grids and mosaics with a few command line arguments. The macro
can determine the area covered by the swath data and set the
grid bounds and dimensions accordingly. For users
seeking more control over the grid and mosaic parameters,
almost the full suite of mbgrid and mbmosaic commands are supported.
See the manual pages for \fBmbgrid\fP and \fBmbmosaic\fP for
complete explanations of how these programs work.

By default, \fBmbgrid\fP and \fBmbmosaic\fP generate grids in Geographic coordinates,
meaning that position is defined in longitude and latitude using
the WGS84 horizontal datum. The \fB-J\fP option can be used to specify 
an alternate, projected coordinate system (PCS). When a PCS is used,
position will be defined in eastings and northings (meters) relative
to the origin of the particular PCS. Universal Transverse Mercator
is the most commonly used PCS in the oceanographic community, but
\fBmbgrid\fP supports a large number of other PCS as well. A list of
the supported PCS's is provided at the end of the \fBmbgrid\fP and
\fBmbmosaic\fP manual pages.

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
Sets the type of data to be read and gridded or mosaiced.
If \fIdatatype\fP = 1,
bathymetry data will be gridded (positive downwards).
If \fIdatatype\fP = 2,
bathymetry data will be gridded as topography (positive upwards).
If \fIdatatype\fP = 3,
amplitude data will be mosaiced.  If \fIdatatype\fP = 4,
sidescan data will be mosaiced.
Default: \fIdatatype\fP = 1 (bathymetry).
.TP
.B \-B
\fIborder\fP
.br
Sets the border of a smoothly interpolated grid
to the value \fIborder\fP
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
Sets the dimensions of the output grid.
This option is superceded
if the user specifies the grid spacing with the \fB-E\fP option.
Default:  \fIxdim\fP = \fIydim\fP = 101.
.TP
.B \-E
\fIdx/dy/units\fP
.br
Sets the grid cell spacing to \fIdx\fP in longitude and \fIdy\fP
in latitude. If \fIunits\fP is not specified, the \fIdx\fP
and \fIdy\fP values are assumed to be in meters. Valid values
for \fIunits\fP include "km", "meters", and "degrees".
By default, the grid spacing is calculated from
the grid bounds and the grid dimensions. When the user
uses the \fB-E\fP option to set the grid spacing, the
grid dimensions are calculated using the grid bounds and
grid cell spacings. However, slight adjustments to the
grid cell spacing values are usually required to keep
the grid bounds as specified.
.TP
.B \-F
\fIformat\fP
.br
Sets the data format for the input data.
If \fIformat\fP < 0, then the input file specified
with the \fB-I\fP option will actually contain a list of input swath sonar
data files. This program uses the \fBMBIO\fP library
and will read or write any swath sonar
format supported by \fBMBIO\fP. A list of the swath sonar data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page. Default: \fIformat\fP = -1.
.TP
.B \-f
\fImode\fP
.br
Sets the gridding algorithm to be used by \fBmbgrid\fP for bathymetry
(\fB-A\fP\fI1\fP) or topography c data.
 	\fImode\fP = 1:         Gaussian Weighted Mean
 	\fImode\fP = 2:         Median Filter
 	\fImode\fP = 3:         Minimum Filter
 	\fImode\fP = 4:         Maximum Filter
The default is \fImode\fP = 1 (Gaussian Weighted Mean).
.TP
.B \-f
\fIpriority_range\fP
.br
Turns on Gaussian weighted mean mosaicing in \fBmbmosaic\fP
for amplitude (\fB-A\fP\fI3\fP) or sidescan (\fB-A\fP\fI4\fP) data. The
\fIpriority_range\fP value determines which data points are
used in the mosaicing. The minimum priority threshold for
each bin is the highest priority value found among the
samples in that bin minus the \fIpriority_range\fP value.
Only samples with priorities greater than this threshold
are used in the Gaussian weighted mean mosaicing.
The default is to simply set each bin's value equal to the
value of the highest priority sample in that bin.
.TP
.B \-G
\fIgridkind\fP
.br
Sets the format of the output grid file.
 	\fIgridkind\fP = 1:	Ascii table
 	\fIgridkind\fP = 2:	binary file (GMT version 1 GRD file)
 	\fIgridkind\fP = 3:	netCDF file (GMT version 2 GRD file)
.br
If \fIgridkind\fP = 3, \fBmbgrid\fP also outputs shellscripts
which run GMT version 3 programs to provide preliminary color
fill maps of the gridded data. These shellscripts are
generated using the \fBmbm_grdplot\fP macro.
Default: \fIgridkind\fP = 3.
.TP
.B \-H
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \-I
\fIfilename\fP
.br
Sets the input filename. If \fIformat\fP > 0 (set with the
\fB-f\fP option) then the swath sonar data contained in \fIinfile\fP
is read and processed. If \fIformat\fP < 0 (the default),
then \fIinfile\fP
is assumed to be an ascii file containing a list of the input swath sonar
data files to be processed and their formats.  The program will read
the data in each one of these files.
In the \fIinfile\fP file, each
data file should be followed by a data format identifier, e.g.:
 	datafile1 11
 	datafile2 24
.br
This program uses the \fBMBIO\fP library and will read or write any swath sonar
format supported by \fBMBIO\fP. A list of the swath sonar data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page.
.TP
.B \-J
\fIprojection\fP
By default, \fBmbgrid\fP and \fBmbmosaic\fP generate grids in Geographic coordinates,
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
output filenames. For example, if the grid is output as a GMT
version 2 GRD format (netCDF) file (the default),
then its filename is "root.grd". If the
\fB-G\fP\fI1\fP option is used to specify an ascii format grid, then the
output grid filename will be "root.asc". If the
\fB-G\fP\fI2\fP option is used to specify a version 1 GRD
format (binary) grid, then the
output grid filename will be "root.grd1". If the output grid is
in the GMT version 2 GRD format, a shellscript
which will allow the contents of the grid to viewed using GMT programs
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
.B \-U
\fIazimuth/factor\fP
.br
Enables prioritizing data points according to their look azimuth
(data on the port side of the swath have
a look azimuth equal to the heading - 90 degrees, and data on the
starboard side have a look azimuth equal to the heading + 90 degrees).
Here \fIazimuth\fP is the preferred look azimuth, and \fIfactor\fP
modulates how rapidly the priority degrades away from the preferred
look azimuth. The priority (p) for a data point is assigned as follows:
        p = cos(f * (Ap - Al))
.br
when -90 < (f * (Ap - Al)) < 90 and
        p = 0
.br
otherwise, where f = \fIfactor\fP, Ap = \fIazimuth\fP, and Al is the
look azimuth of the data point.
If \fIfactor\fP = 1.0, the priority will be 1.0
at \fIazimuth\fP and will fall to zero for look
azimuths more than 90 degrees away from \fIazimuth\fP.
If \fIfactor\fP > 1.0, the range of nonzero priorities will shrink
to azimuths closer to \fIazimuth\fP (e.g. if \fIfactor\fP = 2.0,
nonzero priorities will be restricted to look azimuths within
45 degrees of \fIazimuth\fP). If \fIfactor\fP < 1.0, the
range of nonzero priorities will expand (e.g. if \fIfactor\fP = 0.5, only
look azimuths 180 degrees away from \fIazimuth\fP
will have a zero priority).
.TP
.B \-V
The \fB-V\fP option causes \fBmbm_grid\fP to print out statements
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
.TP
.B \-Y
\fIpriority_file\fP
.br
Enables priortization of data points based on their apparent
grazing angle (this angle is the
arctan(x/z) where x is acrosstrack distance and z is depth, so
that the center of the swath has an apparent grazing angle of
zero, the port swath edge has a large negative angle, and the
starboard swath edge has a large positive angle). The file
\fIpriority_file\fP must contain a list of data priorities
as a function of apparent grazing angle. The first line of
the file should contain the minimum, or port-most grazing angle
followed by the associated priority. The following lines
should contain increasingly large grazing angles (and associated
priorities) up to the maximum, or starboard-most, grazing angle.
The highest priority assigned should be one, and the lowest zero.
Priorities for grazing angles less than the minimum or greater
than the maximum will be zero. See the examples below for a
further explanation of the use of \fIpriority_file\fP.
.TP
.B \-Z
\fIbath_default\fP
.br
Sets the default depth used for calculating grazing angles for
amplitude or sidescan values where depths are not available.
Default: \fIscale\fP = 1000.0

.SH EXAMPLES
Suppose we have obtained a swath sonar data file called
example_hs.mb24 collected using a SeaBeam 2112 sonar.
This file contains bathymetry, beam amplitude, and
sidescan data. In order to obtain a first cut bathymetry
grid and first cut amplitude and sidescan mosaics, we
use \fBmbm_grid\fP to generate shellscripts which in turn
run \fBmbgrid\fP or \fBmbmosaic\fP to generate grids and mosaics.
The following four commands generate gridding shellscripts
for bathymetry, topography, amplitude, and sidescan,
respectively:

 	mbm_grid -F24 -I example_hs.mb24 \\
		-A1 -V -Obath
 	mbm_grid -F24 -I example_hs.mb24 \\
 		-A2 -V -Otopo
 	mbm_grid -F24 -I example_hs.mb24 \\
 		-A3 -V -Oamp
 	mbm_grid -F24 -I example_hs.mb24 \\
 		-A4 -V -Oss

When the following shellscripts are executed, each will
generate a both a grid (or mosaic) file and an additional
shellscript which in turn will (when run) generate and display a
postscript plot file:

 	bath_mbgrid.cmd
 	topo_mbgrid.cmd
 	amp_mbmosaic.cmd
 	ss_mbmosaic.cmd

The program \fBmbinfo\fP is executed by \fBmbm_grid\fP
to obtain the file statistics used to determine the grid
bounds and bin size. The macro \fBmbm_grdplot\fP is executed
by \fBmbgrid\fP or \fBmbmosaic\fP to generate the initial
plots of the gridded data.

As an example, the contents of the gridding shellscript
"bath_mbgrid.cmd" are:

 #! /bin/csh -f
 #
 # Shellscript to grid or mosaic swath sonar data
 # Created by macro mbm_grid
 #
 # This shellscript created by following command line:
 # mbm_grid -F24 -I example_hs.mb24 -A1 -V -Obath
 #
 # Define shell variables used in this script:
 set REGION       = -49.316085/-49.096415/12.06972/12.18588
 set INPUT_FILE   = example_hs.mb24
 set INPUT_FORMAT = 24
 set ROOT         = bath
 #
 # Make datalist file
 echo Making datalist file...
 echo $INPUT_FILE $INPUT_FORMAT >! datalist$$
 #
 # Run mbgrid
 echo Running mbgrid...
 mbgrid -Idatalist$$ \\
 	-R$REGION \\
  	-O$ROOT \\
 	-A1 -N \\
  	-E363.3/363.3/meters \\
 #
 # All done!
 echo All done!

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbgrid\fP(l),
\fBmbmosaic\fP(l), \fBmbinfo\fP(l),
\fBmbm_grdplot\fP(l)

.SH BUGS
This macro is new and hasn't been tested in serious
usage yet - let us know what to fix, add, or
change!.


