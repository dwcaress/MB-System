.TH mbmosaic l "17 September 1997" "MB-System 4.5" "MB-System 4.5"
.SH NAME
\fBmbmosaic\fP - Mosaic amplitude or sidescan data 
from swath mapping sonar data files.

.SH VERSION
Version 4.5

.SH SYNOPSIS
\fBmbmosaic\fP \fB-I\fIfilelist\fP \fB-O\fIroot\fP 
\fB-R\fIwest/east/south/north\fP [\fB-A\fIdatatype\fP 
\fB-B\fIborder\fP -C\fIclip\fP \fB-D\fIxdim/ydim\fP 
\fB-E\fIdx/dy/units\fP \fB-F\fIpriority_range\fP \fB-G\fIgridkind\fP 
\fB-H -L\fIlonflip\fP \fB-M -N -P\fIpings\fP
\fB-S\fIspeed\fP \fB-T\fItension\fP \fB-U\fIazimuth/factor\fP 
\fB-V\fP -W\fIscale\fP \fB-X\fIextend\fP 
\fB-Y\fIpriority_file\fP \fB-Z\fIbath_default\fP]

.SH DESCRIPTION
\fBmbmosaic\fP is an utility used to mosaic amplitude or sidescan 
data contained in a set of swath sonar data files. This program 
allows users to prioritize data according to the associated grazing
angle and according to look azimuth. Individual mosaic bin values
can be either the value of the highest priority sample in the bin
or the Gaussian weighted mean of the highest priority samples 
in the neighborhood of the bin (the samples used here are those
with priorities within a specified range of the highest priority
sample found). Users can thus construct 
mosaics which are dominantly from a particular part of the swath
(e.g. prioritize the outer swath higher than the inner swath)
or which are dominantly from a particular look azimuth
(e.g. construct an east-looking mosaic by specifying a 
preferred look azimuth of 90 degrees).

The user must specify the bounds of the region to be 
mosaiced (\fB-R\fP), the dimensions (\fB-D\fP) or node 
spacing \fB-E\fP of the mosaic, a file containing a list of 
the data files to be used and their data formats (\fB-I\fP), 
and a character string to be used as the root of the
output filenames (\fB-O\fP).  The user can also specify the 
type of the input data (\fB-A\fP), the range of allowable 
sample priorities used in the mosaicing (\fB-F\fP), the 
preferred look azimuth (\fB-U\fP), the 
maximum distance from data points that the spline 
interpolation is used (\fB-C\fP), the format of the output 
files, and other parameters.

Before opening an input swath data file, \fBmbmosaic\fP checks for
an ascii file in the same directory having the same name except
that ".inf" is appended to the end. The program assumes that this
ascii file contains the output of the program \fBmbinfo\fP run on
the input data file. If the ".inf" file exists, \fBmbmosaic\fP reads
the minimum and maximum longitude and latitude bounds from the 
\fBmbinfo\fP output and compares those to the working bounds
for the grid. If the ".inf" file indicates that none of the data
in the input file lies inside the working grid bounds, that input
file is skipped. This allows users to maintain a single master list
of data files for use in all gridding without the performance penalty
of \fBmbmosaic\fP reading through all the data files, even those
with no relevent data. We recommend that users maintain a ".inf"
file for each swath data file used for gridding or plotting. The
programs \fBmbgrid\fP, \fBmbswath\fP, and \fBmbcontour\fP 
also use ".inf" files in the same fashion.

Usually, the internal working grid has the same boundaries as the
output grid. However, the \fB-X\fP option
allows the size of the internal grid to be increased so that data outside
the grid can be used to guide the spline interpolation of data gaps which
happen to lie at the the edge of the grid.  This is particularly important
when adjacent grids are created which should match along the edges.
The data input bounds are set to a region three times as large as
the working grid in both longitude and latitude.  The program reads all
pings which lie within the data input bounds, and accepts all data
values with locations within the working grid.

The sample priorities are set according to either or both of two
criterea. The first criterea is set by using the \fB-Y\fP option
to specify a file containing a list of data priorities
as a function of apparent grazing angle (this angle is the
arctan(x/z) where x is acrosstrack distance and z is depth, so
that the center of the swath has an apparent grazing angle of
zero, the port swath edge has a large negative angle, and the
starboard swath edge has a large positive angle). The highest
priority assigned should be one, and the lowest zero. 

The second criteria is set by using the \fB-U\fP option to specify
a preferred look azimuth (data on the port side of the swath have
a look azimuth equal to the heading - 90 degrees, and data on the
starboard side have a look azimuth equal to the heading + 90 degrees).
The second parameter in the \fB-U\fP option is a factor (f) which
modulates how rapidly the priority degrades away from the preferred
look azimuth. The priority (p) is assigned as follows:
        p = cos( f * (Ap - Al))
.br
where Ap is the preferred look azimuth and A is the actual
look azimuth. If f = 1.0, the priority will be 1.0
at the preferred look azimuth and will fall to zero for look
azimuths more than 90 degrees away from the preferred look
azimuth. If f > 1.0, the range of nonzero priorities will shrink
two azimuths closer to the preferred look azimuth (e.g. if f = 2.0,
nonzero priorities will be restricted to look azimuths within
45 degrees of the preferred look azimuth). If f < 1.0, the
range of nonzero priorities will expand (e.g. if f = 0.5, only
look azimuths 180 degrees away from the preferred look azimuth
will have a zero priority).

If data priorities are set using both apparent grazing angles
and look azimuths, the priorities used in the mosaicing are
found by multiplying the grazing angle and look azimuth 
priorities together.

The default behavior is to set each bin to the value of the highest
priority sample found in that bin. If more than one sample has
the same highest priority, the first such sample is used. 
If the \fB-F\fP option is used to set a range of acceptable 
priorities, then the mosaicing is done using a Gaussian weighted
mean algorithm. The samples used are those with priorities larger
than the highest priority found minus the range value.
In this scheme, each sample's contribution 
to a Gaussian weighted average for each nearby grid cell 
is calculated using this weighting function:
 	W(r) = Aexp(r**2/a**2)
.br
where r is the distance from the center of the bin to the data point,
a is the distance at which the weighting function falls to 1/e of its
maximum value, and A is a normalizing factor set so that the sum of all
the weights adds to a value of 1.  Normally, the distance a is set to be
half the average grid point spacing, but this can be varied using the \fB-W\fP
option. 

If the \fB-C\fP option is used to set clipping to a value > 0, then a
2D thin plate spline algorithm is applied to the successfully calculated 
grid values to fill in, or interpolate, gaps in the data.  New values
are assigned only at grid points within a specified distance from
the nearest data points (this distance is specified in terms of 
grid point spacing using the \fB-C\fP command).  Thus, small gaps
in the data can be filled, or the whole grid can be filled in with
a smooth interpolation.

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

The \fB-M\fP option causes \fBmbmosaic\fP to output two additional
grids, the first ("root_num.grd") being the number of high priority
data points used within each bin, and the second ("root_sd.grd") 
being the standard deviation of the data points used within each bin.  
The \fB-M\fP option can only be used if Gaussian weighted mean
mosaicing is enabled with the \fB-F\fP option.
Plotting shellscripts
called "root_num.grd.cmd" and "root_sd.grd.cmd" are also output
if the grids are in the GMT version 2 GRD format.

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
Sets the type of data to be read and mosaiced.  
If \fIdatatype\fP = 3,
amplitude data will be gridded.  If \fIdatatype\fP = 4,
sidescan data will be gridded.  
Default: \fIdatatype\fP = 4 (sidescan).
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
\fIpriority_range\fP
.br
Turns on Gaussian weighted mean mosaicing. The 
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
If \fIgridkind\fP = 3, \fBmbmosaic\fP also outputs shellscripts 
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
is given in the \fBMBIO\fP manual page.
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
points in each grid cell. This option is ignored unless the
Gaussian weighted mean mosaicing is enabled with the \fP-F\fP option.
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
.br
Sets the ping averaging of the input data. If \fIpings\fP > 0, then
that number of input pings will be averaged to produce one output
ping.  If \fIpings\fP = 0, then the ping averaging will automatically
be done so that the along-track ping spacing is equal to the across-track
beam spacing. Default: \fIpings\fP = 1.
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
Normally, \fBmbmosaic\fP prints out information regarding its
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
Suppose you want to mosaic some SeaBeam 2112 sidescan data
in six data files over a region with longitude 
bounds of 113.4833E to 115.4333E and latitude bounds
of 32.1166S to 31.5500S, and you would like a 100 m grid spacing.
First, create a datalist file using a text editor which contains the
data filenames followed by the appropriate format identifier:

 	kn_perth_01.mb41 41
 	kn_perth_05.mb41 41
 	kn_perth_09.mb41 41
 	kn_perth_13.mb41 41
 	kn_perth_17.mb41 41
 	kn_perth_03.mb41 41

The nadir region of the sidescan swath is generally of little
use because it is dominated by specular reflection rather
than backscatter. In order to allow \fBmbmosaic\fP to 
preferentially use data from the outer swath, where possible,
create another file containing a list of data priority as
a function of apparent grazing angle:

 	-60.0 0.2
 	-45.0 1.0
 	-15.0 0.8
 	-14.9 0.1
 	14.9 0.1
 	15.0 0.8
 	45.0 1.0
 	60.0 0.2

Here the negative angles denote the port side of the swath and
the positive angles denote the starboard side of the swath.
The priorities are linearly interpolated between the discreet
angle/priority pairs.
Note that the nadir region (angles less than +/- 15 degrees)
has low priority, and that the highest priority is given to
angles of 45 degrees. No data with angle magnitudes greater
than 60 degrees will be used.

Then, run \fBmbmosaic\fP as follows:
 	mbmosaic -Idatalist \\
 		-R114.1333/114.7833/-32.1166/-31.55 \\
 		-A4 -E100/100/meters -N \\
 		-Yangle_priority.dat -F0.10 -C10 \\
 		-Operth_ss -V

The \fB-E\fP option specifies grid spacings of 100 meters in
both the longitude and latitude; the dimensions of the output
grid are calculated according by \fBmbmosaic\fP. The \fB-F\fP
option enables Gaussian weighted mean mosaicing and the priority
range of 0.10 means that for each bin, all data with priorities
within 0.1 of the highest priority will be used.
By specifying a clipping dimension of 10 we cause small gaps in
the mosaic to be filled in through spine interpolation . 
The output looks like:
 
 Program mbmosaic
 Version $Id: mbmosaic.l,v 4.2 1997-09-17 19:22:06 caress Exp $
 MB-system Version 4.5

 MBMOSAIC Parameters:
 List of input files: datalist
 Output fileroot:     perth_ss
 Input Data Type:     Sidescan
 Grid projection: Geographic
 Grid dimensions: 616 629
 Grid bounds:
   Longitude:  114.1333  114.7833
   Latitude:   -32.1166  -31.5500
 Working grid dimensions: 616 629
 Working Grid bounds:
   Longitude:  114.1333  114.7833
   Latitude:   -32.1166  -31.5500
 Longitude interval: 0.001057 degrees or 100.051035 m
 Latitude interval:  0.000902 degrees or 100.042679 m
 Specified Longitude interval: 100.000000 meters
 Specified Latitude interval:  100.000000 meters
 Input data bounds:
   Longitude:  113.4833  115.4333
   Latitude:   -32.6832  -30.9834
 Mosaicing algorithm:  
   Average of highest weighted pixels
   Pixel weighting range: 0.100000
   Pixels weighted by look azimuth
   Preferred look azimuth: 90.000000
   Look azimuth factor:    2.000000
   Spline interpolation applied with clipping dimension: 10
   Spline tension (range 0.0 to infinity): 10000000000.000000
 Grid format 3:  GMT version 2 grd (netCDF)
 NaN values used to flag regions with no data
 MBIO parameters:
   Ping averaging:       1
   Longitude flipping:   1
   Speed minimum:       0.0 km/hr

 330814 data points processed in kn_perth_01.mb41
 239709 data points processed in kn_perth_05.mb41
 234034 data points processed in kn_perth_09.mb41
 310066 data points processed in kn_perth_13.mb41
 297318 data points processed in kn_perth_17.mb41
 336504 data points processed in kn_perth_03.mb41

 1748445 total data points processed in highest weight pass

 330814 data points processed in kn_perth_01.mb41
 239709 data points processed in kn_perth_05.mb41
 234034 data points processed in kn_perth_09.mb41
 310066 data points processed in kn_perth_13.mb41
 297318 data points processed in kn_perth_17.mb41
 336504 data points processed in kn_perth_03.mb41

 1748445 total data points processed in averaging pass

 Making raw grid...

 Doing spline interpolation with 86414 data points...

 Total number of bins:            387464
 Bins set using data:             86414
 Bins set using interpolation:    58051
 Bins not set:                    242999
 Maximum number of data in a bin: 121
 Minimum value:   -6573.12   Maximum value:   97704.64
 Minimum sigma:    4.50125   Maximum sigma: 42979.49567

 Outputting results...

 executing mbm_grdplot...
 Running grdhisteq...

 Program Status:
 --------------

   Plot Style:
     Color Fill
     Horizontal Color Scale

   Input Files:
     Data GRD File:            perth_ss.grd
     Intensity GRD List File:   

   Output Files:
     Output plot name root:    perth_ss.grd
     Color pallette table:     perth_ss.grd.cpt
     Plotting shellscript:     perth_ss.grd.cmd
     Plot file:                perth_ss.grd.ps

   Plot Attributes:
     Plot width:               6.5000
     Plot height:              6.6369
     Page size:                a
     Page width:               8.5
     Page height:              11
     Projection:               -Jm10
     Axes annotation:          10m/10m
     Orientation:              portrait
     Number of colors:         11
     Color Pallette:           Grayscale
     Colors reversed

   Grid Data Attributes:
     Longitude min max:         114.1330   114.7830
     Latitude min max:          -32.1166   -31.5500
     Data min max:                 -6573   9.77e+04

   Primary Grid Plotting Controls:
     Color start datum:        -12500.000000
     Color end datum:          112500.000000
     Histogram stretch applied to color pallette

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

 Plot generation shellscript <perth_ss.grd.cmd> created.

 Instructions:
   Execute <perth_ss.grd.cmd> to generate 
        Postscript plot <perth_ss.grd.ps>.
   Executing <perth_ss.grd.cmd> also invokes 
        xpsview to view the plot on the screen.

 --------------


 Done.

The names of the output files are based on the root character string
specified using the \fB-O\fP option.  Since the grid is output as a netCDF
GRD format file, its filename is "perth_ss.grd"; a shellscript
which will allow the contents of the grid to viewed using GMT programs
is also output with the filename "perth_ss.grd.cmd".

Suppose that one wants to produce a mosaic composed only of data
with a more or less eastward look azimuth. Then add the \fB-U\fP
option with a preferred azimuth of 90 degrees. A look azimuth
factor of 2.0 will cause \fBmbmosaic\fP to reject any data with
look azimuths outside a range of 45 to 135 degrees.
The following will suffice:
 	mbmosaic -Idatalist \\
 		-R114.1333/114.7833/-32.1166/-31.55 \\
 		-A4 -E100/100/meters -N \\
 		-Yangle_priority.dat -F0.10 -C10 \\
 		-Operth_ss -U90/2.0 -V

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbgrid\fP(l), \fBmbcontour\fP(l), \fBmbswath\fP(l)

.SH BUGS
Please let us know.
