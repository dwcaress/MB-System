.TH mbgrid l "5 October 1998" "MB-System 4.6" "MB-System 4.6"
.SH NAME
\fBmbgrid\fP - Grid bathymetry, amplitude, or sidescan data 
from swath sonar data files.

.SH VERSION
Version 4.5

.SH SYNOPSIS
\fBmbgrid\fP \fB-I\fIfilelist\fP \fB-O\fIroot\fP 
\fB-R\fIwest/east/south/north\fP [\fB-A\fIdatatype\fP 
\fB-B\fIborder\fP -C\fIclip\fP \fB-D\fIxdim/ydim\fP 
\fB-E\fIdx/dy/units\fP \fB-F\fImode\fP \fB-G\fIgridkind\fP 
\fB-L\fIlonflip\fP \fB-M -N -P\fIpings\fP \fB-Q\fP
\fB-S\fIspeed\fP \fB-T\fItension\fP \fB-U\fItime\fP 
\fB-V\fP -W\fIscale\fP \fB-X\fIextend\fP]

.SH DESCRIPTION
\fBmbgrid\fP is an utility used to grid bathymetry, amplitude, or sidescan 
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
specified using the \fB-O\fP option.  If the grid is output as a GMT 
version 2 GRD format file (netCDF) or a GMT version 1 GRD format file  
(binary), then its filename is "root.grd".   If the
\fB-G\fP\fI1\fP option is used to specify an ascii format grid, then the
output grid filename will be "root.asc". If the output grid is
in the GMT version 2 GRD format, a shellscript
which will allow the contents of the grid to viewed using GMT programs
is also output with the filename "root.grd.cmd".

The \fB-M\fP option causes \fBmbgrid\fP to output two additional
grids, the first ("root_num.grd") being the number of data 
points located within each bin, and the second ("root_sd.grd") 
being the standard deviation of the
data points located within each bin.  Plotting shellscripts
called "root_num.grd.cmd" and "root_sd.grd.cmd" are also output
if the grids are in the GMT version 2 GRD format. The \fB-M\fP
option is ignored when the minimum or maximum filter gridding
algorithms are used.

.SH AUTHORSHIP
David W. Caress (caress@lamont.ldeo.columbia.edu)
.br
Dale N. Chayes (dale@lamont.ldeo.columbia.edu)
.br
Lamont-Doherty Earth Observatory
.br
Palisades, NY 10964

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
\fImode\fP
.br
Sets the gridding algorithm to be used.
 	\fImode\fP = 1:         Gaussian Weighted Mean
 	\fImode\fP = 2:         Median Filter
 	\fImode\fP = 3:         Minimum Filter
 	\fImode\fP = 4:         Maximum Filter
The default is \fImode\fP = 1 (Gaussian Weighted Mean).
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
\fIdatalist\fP
.br
Sets the filename of the file containing a list of the input swath sonar
data files and their formats.  In the \fIdatalist\fP file, each
data file should be followed by a data format identifier, e.g.:
 	datafile1 1
 	datafile2 8
.br
This program uses the \fBMBIO\fP library and will read or write any swath sonar
format supported by \fBMBIO\fP. A list of the swath sonar data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page.  A format identifier of 0 indicates
that the file contains lines of (lon, lat, depth) triples which can be read
in free format.
Default: \fIdatalist\fP = data.list
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
 Version $Id: mbgrid.l,v 4.16 1998-10-05 19:47:23 caress Exp $
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
which will allow the contents of the grid to viewed using GMT programs
is also output with the filename "urville_int.grd.cmd".

Suppose that one wants to grid the same data considered above using
the median filtering scheme instead of the default Gaussian weighted
mean scheme and also outputting grids of the data density and 
standard deviation values. The following will suffice:
 	mbgrid -R220.1/220.35/-9.7/-9.45 -D251/251 \\
 		-L1 -C251 -N \\
 		-Idatalist -Ourville_int -F2 -M -V

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbcontour\fP(l), \fBmbswath\fP(l)

.SH BUGS
Please let us know.
