.TH mbgrid l "14 February 1995"
.SH NAME
\fBmbgrid\fP - Grid bathymetry, amplitude, or sidescan data 
from multibeam data files.

.SH VERSION
Version 4.2

.SH SYNOPSIS
\fBmbgrid\fP \fB-I\fIfilelist\fP \fB-O\fIroot\fP \fB-R\fIwest/east/south/north\fP [\fB-A\fIdatatype\fP \fB-B\fIborder\fP -C\fIclip\fP \fB-D\fIxdim/ydim\fP \fB-E\fIdx/dy\fP \fB-F\fP \fB-G\fIgridkind\fP \fB-L\fIlonflip\fP \fB-M -N \fB-P\fIpings\fP \fB-S\fIspeed\fP \fB-T\fItension\fP \fB-V\fP -W\fIscale\fP \fB-X\fIextend\fP]

.SH DESCRIPTION
\fBmbgrid\fP is an utility used to grid bathymetry, amplitude, or sidescan 
data contained in a set of multibeam data files. This program can use
either a gaussian weighted average scheme or a median filter scheme
to grid regions covered by multibeam 
swaths and then can fill in gaps between the swaths (to the degree 
specified by the user) using the same minimum curvature algorithm
implemented in the \fBGMT\fP program \fBsurface\fP.

The user must specify the bounds of the region to be gridded (\fB-R\fP), 
the dimensions (\fB-D\fP) or node spacing \fB-E\fP of the grid, 
a file containing a 
list of the data files to be used and their data formats (\fB-I\fP), 
and a character string to be used as the root of the
output filenames (\fB-O\fP).  The user can also specify the type of the
input data (\fB-A\fP), the width of the gaussian filter used for 
weighted average gridding (\fB-W\fP), the maximum distance from 
data points that the minimum curvature 
interpolation is used (\fB-C\fP), the format of the output files, and
other parameters.

Usually, the internal working grid has the same boundaries as the
output grid. However, the \fB-X\fP option
allows the size of the internal grid to be increased so that data outside
the grid can be used to guide the spline interpolation of data gaps which
happen to lie at the the edge of the grid.  This is particularly important
when adjacent grids are created which should match along the edges.
The data input bounds are set to a region three times as large as
the working grid in both longitude and latitude.  The program reads all
pings which lie within the data input bounds, and accepts all beam
values with locations within the working grid. In addition to multibeam
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

If the \fB-F\fP option is used, the gridding is performed with a
median filter scheme instead of a Gaussian weighted average. In this
approach, all of the values for each bin are held in memory until
all of the data has been read. Then, the median value for each bin
is assigned as the gridded value for that bin.  The advantage of a
median filter approach is that it is relatively insensitive to 
isolated artifacts in the data, provided that several samples
exist for each bin.  This disadvantages to the median filter is
that in the absence of artifacts, the weighted average scheme
does a better job of representing the gridded field, particularly
if the spectral characteristics of the gridded field are important.
The median filter approach also requires much more memory than
a weighted average.

If the \fB-C\fP option is used to set clipping to a value > 0, then a
2D thin plate spline algorithm is applied to the successfully calculated 
grid values to fill in, or interpolate, gaps in the data.  New values
are assigned only at grid points within a specified distance from
the nearest data points (this distance is specified in terms of 
grid point spacing using the \fB-C\fP command).  Thus, small gaps
in the data can be filled, or the whole grid can be filled in with
a smooth interpolation.

The minimum curvature interpolation algorithm used is identical to that of the
\fBGMT-system\fP program \fBsurface\fP (the source code has been
modified only slightly); see the \fBsurface\fP manual page for
more information on its operation.

For magnetic inversions it is useful to have a bathymetry grid which
tails off to a constant value at the border.  To facilitate this, the
\fB-B\fP option allows the user to set the border of a smoothly
interpolated grid to a constant value wherever no data are present.  
This is useful only if the data are confined to the central region 
of the grid and a smooth interpolation is done with a large \fIclip\fP
so that the entire grid is filled.

The output grid will by default contain values of -99999.9 at cells
containing no data; if the \fB-N\fP option is used then the flagging
value used is NaN, or not-a-number.

The names of the output files are based on the root character string
specified using the \fB-O\fP option.  If the grid is output as a GMT 
version 2 GRD format file (netCDF) or a GMT version 1 GRD format file  
(binary), then its filename is "grd_root"; a shellscript
which will allow the contents of the grid to viewed using GMT programs
is also output with the filename "grd_root.cmd".  If the
\fB-A\fP option is used to specify an ascii format grid, then the
output grid and shellscript filenames will be "asc_root" and "asc_root.cmd";
the shellscript will make use of a graphics program called \fBSHADE\fP
(this is for backwards compatibility - \fBSHADE\fP is obsolete and the
use of ascii grids is now uncommon).

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
.B \fB-A\fIdatatype\fP
Sets the type of data to be read and gridded. If \fIdatatype\fP = 1,
bathymetry data will be gridded (positive downwards). If \fIdatatype\fP = 2,
bathymetry data will be gridded as topography (positive upwards).  
If \fIdatatype\fP = 3,
amplitude data will be gridded.  If \fIdatatype\fP = 4,
sidescan data will be gridded.  
Default: \fIdatatype\fP = 1 (bathymetry).
.TP
.B \fB-B\fIborder\fP
Sets the border of a smoothly interpolated grid to the value \fIborder\fP
wherever no data exist, provided \fIborder\fP > 0.0.
Default: \fIborder\fP = 0.0
.TP
.B \fB-C\fIclip\fP
Sets the clipping dimension for the spline interpolation. If \fIclip\fP=0
no spline interpolation will be done.  If \fIclip\fP>0 then the spline
interpolation will fill data gaps to a distance of \fIclip\fP times
the grid spacing.
Default: \fIclip\fP = 0.
.TP
.B \fB-D\fIxdim/ydim\fP
Sets the dimensions of the output grid.  This option is superceded
if the user specifies the grid spacing with the \fB-E\fP option.
Default:  \fIxdim\fP = \fIydim\fP = 101.
.TP
.B \fB-E\fIdx/dy\fP
Sets the grid spacing in longitude (\fIdx\fP) and latitude (\fIdy\fP
of the output grid. By default, the grid spacing is calculated from 
the grid bounds and the grid dimensions. If the user
uses the \fB-E\fP option to set the grid spacing, then the dimensions
will be calculated from the grid bounds and spacing. In these
circumstances rounding errors will usually require that the eastern
and northern bounds be adjusted to fit exactly with the 
grid dimensions and spacing.
.TP
.B \fB-F\fP
Sets the use of a median filter gridding scheme rather than the
default Gaussian weighted filter scheme.
.TP
.B \fB-G\fIgridkind\fP
Sets the format of the output grid file.
 	\fIgridkind\fP = 1:	Ascii table
 	\fIgridkind\fP = 2:	binary file (GMT version 1 GRD file)
 	\fIgridkind\fP = 3:	netCDF file (GMT version 2 GRD file)
.br
\fBmbgrid\fP also outputs shellscripts which run graphics programs,
thus providing a preliminary view of the gridded data.  If 
\fIgridkind\fP = 1 or 2 then the shellscript will run the program
\fBSHADE\fP (this will probably vanish soon since \fBSHADE\fP is
obsolete and no longer supported).  If \fIgridkind\fP = 3 then the
shellscript will run GMT version 2 programs and then put the image
on the screen using \fBPAGEVIEW\fP.  In either case, the image will
consist of a color fill plot overlaid with contours.
Default: \fIgridkind\fP = 3.
.TP
.B \fB-H\fP
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \fB-I\fIdatalist\fP
Sets the filename of the file containing a list of the input multibeam
data files and their formats.  In the \fIdatalist\fP file, each
data file should be followed by a data format identifier, e.g.:
 	datafile1 1
 	datafile2 8
.br
This program uses the \fBMBIO\fP library and will read or write any multibeam
format supported by \fBMBIO\fP. A list of the multibeam data formats
currently supported by \fBMBIO\fP and their identifier values
is given in the \fBMBIO\fP manual page.  A format identifier of 0 indicates
that the file contains lines of (lon, lat, depth) triples which can be read
in free format.
Default: \fIdatalist\fP = data.list
.TP
.B \fB-L\fIlonflip\fP
Sets the range of the longitude values returned.
If \fIlonflip\fP=-1 then the longitude values will be in
the range from -360 to 0 degrees. If \fIlonflip\fP=0 
then the longitude values will be in
the range from -180 to 180 degrees. If \fIlonflip\fP=1 
then the longitude values will be in
the range from 0 to 360 degrees.
Default: \fIlonflip\fP = 0.
.TP
.B \fB-M\fP
Causes two additional grids to be output. One is a grid containing
the standard deviation of the data within each grid cell relative
to the grid value, the other contains the number of data 
points in each grid cell.
.TP
.B \fB-N\fP
Causes grid cells with no data and no interpolation to be set to a
value of NaN instead of the default value of -99999.9.  The NaN value
is expected by \fBGMT\fP programs such \fBgrdview\fP.
.TP
.B \fB-P\fIpings\fP
Sets the ping averaging of the input data. If \fIpings\fP > 0, then
that number of input pings will be averaged to produce one output
ping.  If \fIpings\fP = 0, then the ping averaging will automatically
be done so that the along-track ping spacing is equal to the across-track
beam spacing. Default: \fIpings\fP = 1.
.TP
.B \fB-R\fIwest/east/south/north\fP
Sets the longitude and latitude bounds of the output grid. If the user
uses the \fB-E\fP option to set the grid spacing, then the dimensions
will be calculated from the grid bounds and spacing. In these
circumstances rounding errors will usually require that the eastern
and northern bounds be adjusted to fit exactly with the 
grid dimensions and spacing.
.TP
.B \fB-S\fIspeed\fP
Sets the minimum speed in km/hr (5.5 kts ~ 10 km/hr) allowed in 
the input data; pings associated with a smaller ship speed will not be
output. Default: \fIspeed\fP = 0.
.TP
.B \fB-T\fItension\fP
Sets the \fItension\fP value used in the thin plate spline interpolation. 
A \fItension\fP of 0 gives a minimum curvature surface with free edges;
a nonzero \fItension\fP tends to suppress spurious oscillations and flatten
the interpolation toward the edges. The \fItension\fP must be between 0 and
1.  See the manual page for the \fBGMT\fP program \fBsurface\fP for more 
information.
Default: \fItension\fP = 0.35.
.TP
.B \fB-V\fP
Normally, \fBmbgrid\fP prints out information regarding its
controlling parameters during execution; the 
\fB-V\fP option causes the program to also print out statements
indicating its progress.
.TP
.B \fB-W\fIscale\fP
Sets the width of the gaussian weighting function in terms of
the grid spacing.  The distance to the 1/e point of the weighting
function is given by half of the grid spacing times \fIscale\fP.
Default: \fIscale\fP = 1.0
.TP
.B \fB-X\fIextend\fP
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
 	mbgrid -R220.1/220.35/-9.7/-9.45 -D251/251 -L1 -C251 
 		-Idatalist -Ourville_int -V

By specifying a clipping dimension of 251 we insure that the grid
will be filled in through spine interpolation even in the areas
not covered by data. The output looks like:

 Program mbgrid
 Version $Id: mbgrid.l,v 4.3 1995-02-14 21:15:05 caress Exp $
 MB-system Version 4.00

 mbgrid Parameters:
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
 Longitude interval: 0.001000 degrees or 0.109779 km
 Latitude interval:  0.001000 degrees or 0.110605 km
 Gaussian filter 1/e length: 0.055096 km
 Spline interpolation applied with clipping dimension: 251
 Spline tension (range 0.0 to 1.0): 0.350000
 Grid format 3:  GMT version 2 grd (netCDF)
 Real value of 99999.900000 used to flag regions with no data
 MBIO parameters:
   Ping averaging:       1
   Longitude flipping:   1
   Speed minimum:       0.0 km/hr

 28795 data points processed in d123e.mb8
 0 data points processed in d125e.mb8
 97200 data points processed in d126e.mb8
 67237 data points processed in d128e.mb8
 20438 data points processed in d129e.mb8
 80122 data points processed in d130e.mb8
 55385 data points processed in d131e.mb8

 349177 total data points processed

 Making raw grid...

 Doing minimum curvature interpolation with 53516 data points...

 Spline interpolation fit information:
 Data points   nodes    mean error     rms error     curvature
     53516     63001   -1.44419e-08   9.3621e-05     1373.21

 Total number of bins:            63001
 Bins set using data:             53516
 Bins set using interpolation:    9485
 Bins not set:                    0
 Maximum number of data in a bin: 65
 Minimum value:     504.93   Maximum value:    3405.75
 Minimum sigma:    0.00924   Maximum sigma:  171.70297

 Outputting results...

 Done.

The names of the output files are based on the root character string
specified using the \fB-O\fP option.  Since the grid is output as a netCDF
GRD format file, its filename is "grd_urville_int"; a shellscript
which will allow the contents of the grid to viewed using GMT programs
is also output with the filename "grd_urville_int.cmd".

Suppose that one wants to grid the same data considered above using
the median filtering scheme instead of the default Gaussian weighted
mean scheme and also outputting grids of the data density and 
standard deviation values. The following will suffice:
 	mbgrid -R220.1/220.35/-9.7/-9.45 -D251/251 -L1 -C251 
 		-Idatalist -Ourville_int -F -M -V

The output looks like:

 Program mbgrid
 Version $Id: mbgrid.l,v 4.3 1995-02-14 21:15:05 caress Exp $
 MB-system Version 4.00

 mbgrid Parameters:
 List of input files: datalist
 Output fileroot:     urville_int
 Input Data Type:     Bathymetry
 Gridding algorithm:  Median Filter
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
 Longitude interval: 0.001000 degrees or 0.109779 km
 Latitude interval:  0.001000 degrees or 0.110605 km
 Gaussian filter 1/e length: 0.055096 km
 Spline interpolation applied with clipping dimension: 251
 Spline tension (range 0.0 to 1.0): 0.350000
 Grid format 3:  GMT version 2 grd (netCDF)
 Real value of 99999.900000 used to flag regions with no data
 Data density and sigma grids also created
 MBIO parameters:
   Ping averaging:       1
   Longitude flipping:   1
   Speed minimum:       0.0 km/hr

 28249 data points counted in d123e.mb8
 0 data points counted in d125e.mb8
 95449 data points counted in d126e.mb8
 64790 data points counted in d128e.mb8
 19977 data points counted in d129e.mb8
 79055 data points counted in d130e.mb8
 54527 data points counted in d131e.mb8
 28249 data points processed in d123e.mb8
 0 data points processed in d125e.mb8
 95449 data points processed in d126e.mb8
 64790 data points processed in d128e.mb8
 19977 data points processed in d129e.mb8
 79055 data points processed in d130e.mb8
 54527 data points processed in d131e.mb8

 342047 total data points processed

 Making raw grid...

 Doing minimum curvature interpolation with 53516 data points...

 Spline interpolation fit information:
 Data points   nodes    mean error     rms error     curvature
     53516     63001   -2.8406e-08   0.000159107     1549.36

 Total number of bins:            63001
 Bins set using data:             53516
 Bins set using interpolation:    9485
 Bins not set:                    0
 Maximum number of data in a bin: 65
 Minimum value:     492.60   Maximum value:    3423.00
 Minimum sigma:    7.00000   Maximum sigma:  259.35433

 Outputting results...

 Done.

.SH SEE ALSO
\fBmbsystem\fP(l)

.SH BUGS
You tell me.
