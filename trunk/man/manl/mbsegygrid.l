.TH mbsegygrid l "3 July 2009" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbsegygrid\fP - Generate time vs. trace number grids of seismic data from segy files.

.SH VERSION
Version 5.0

.SH SYNOPSIS
\fBmbsegygrid\fP \fB-I\fIsegyfile\fP \fB-O\fIgridfile\fP 
[\fB-A\fIshotscale/timescale\fP \fB-B\fImaxvalue/window\fP \fB-D\fIdecimatex/decimatey\fP
\fB-G\fImode/gain[/window]\fP 
\fB-S\fImode[/start/end[/schan/echan]]\fP \fB-T\fIsweep[/delay]\fP 
\fB-W\fImode/start/end\fP \fB-H \fB-V\fP]";

.SH DESCRIPTION
\fBMBsegygrid\fP generates grids of seismic data from segy files. 
The program works by inserting trace data into a grid in
which the y-axis is some measure of trace number, range, or distance
along a profile, and the y-axis is time. The output files are \fBGMT\fP netCDF 
format grid files. 

The x-dimension of the grid is determined by the number of traces specified
by the \fB-S\fP option and any decimation specified with option \fB-D\fP. The 
y-dimension of the grid is determined by the sample spacing, any decimation
specified with option \fB-D\fP, and the time sweep specified by the  \fB-T\fP
option.

If the \fB-S\fP option is not specified, then all of the traces in the segy file
will be gridded. If the time sweep and delay (if any) are not specified using
the \fB-T\fP option, then the sweep and delay will be set so that all trace samples
are incorporated into the grid.

A simple time-varying gain can be applied to the traces before gridding using
the \fB-G\fP option. The start time for the gain may be either the start 
of the trace or the seafloor arrival time. 

Regions of the grid without data are indicated in the output by NaN values.

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
\fIshotscale/timescale\fP
.br
This option causes the x-axis to be rescaled from shot number to distance in meters
and the y-axis to be rescaled from time in seconds to depth in meters.
The \fIshotscale\fP value represents the shot spacing in meters and the
\fItimescale\fP value is the scaling from time to depth (typically 750 m/s
for the water column).
.TP
.B \-B
\fImaxvalue[/window]\fP
.br
This option causes each trace to be normalized so that it's maximum value.
.TP
.B \-D
\fIdecimatex/decimatey\fP
.br
Sets the decimation of traces (\fIdecimatex\fP) and samples (\fIdecimatey\fP) used in
generating the output grid. The downsampling of the data is actually accomplished by averaging
the values within each grid bin rather than omitting traces or samples.
.TP
.B \-G
\fImode/gain[/window]\fP
.br
Sets the application of gain to the trace data. The \fImode\fP value determines how
the gain is applied. If \fImode\fP = 0, no gain is applied.If \fImode\fP = 1, 
the gain is applied starting from the beginning of the trace. If  \fImode\fP = 2,
the gain is applied starting at the seafloor (assuming that a seafloor pick has
been defined in the segy data file). The \fIgain\fP value sets the gain applied
as follows. If G = \fIgain\fP and t is the time in seconds since the start of 
gain application, then each sample is multiplied by a factor of (1 + (G * t)). 
The parameter \fIwindow\fP, if given, sets the length of the time interval
in seconds that gain is applied. When the gain is windowed, trace values after
the end of the gain window are zeroed.
.TP
.B \-H
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \-I
\fIsegyfile\fP
.br
Sets the filename of the input segy seismic data file to be gridded.
.TP
.B \-O
\fIgridfile\fP
.br
Sets the filename for the output GMT netCDF format grid.
.TP
.B \-S
\fImode[/start/end[/schan/echan]]\fP
.br
This option sets the range of traces that are gridded, and thus determines
the x-dimension of the output grid (also impacted by any decimation
specified with \fB-D\fP). If \fImode\fP = 0, then \fIstart\fP and \fIend\fP 
refer to shot numbers. This typically is useful for subbottom data or seismic
data in shot gather form. If \fImode\fP = 1, then \fIstart\fP and \fIend\fP 
refer to CMP (or RP or CDP) numbers. This typically is useful for seismic
reflection data in stacked or CMP gather forms. If the data are multichannel 
seismic reflection or seismic refraction in either shot or CMP gathers, the
start and end of the channels selected for gridding is set using the optional \fIschan\fP
and \fIechan\fP, respectively. The x-dimension of the output grid is determined by
(\fIend\fP - \fIstart\fP + 1) * (\fIechan\fP - \fIschan\fP + 1) / \fIdecimatex\fP.
.TP
.B \-T
\fIsweep[/delay]\fP
.br
Sets the time range of seismic data to be gridded. The \fIsweep\fP specifies
the number of seconds of data comprising the y-axis of the grid. The number of
samples follows from the sample interval of the data, and any decimation
applied using \fB-D\fP. The optional \fIdelay\fP value sets the sweep
start time, again in seconds.
The y-dimension of the output grid is determined by
\fIsweep\fP / sampleinterval / \fIdecimatey\fP, where the sampleinterval is
a parameter read with the seismic data.
.TP
.B \-V
Normally, \fBmbsegygrid\fP prints out information regarding its
controlling parameters during execution; the 
\fB-V\fP option causes the program to also print out statements
indicating its progress.
.TP
.B \-W
\fImode/start/end\fP
.br
This option can be used to limit the data being gridded to a particular time
window in various ways. This option does not impact the definition of the overall
grid bounds, but does restrict the data gridded to samples within particular times
of interest. If \fImode\fP = 1, then \fIstart\fP and \fIend\fP are simply start and end 
times of good data in seconds. If \fImode\fP = 2, then \fIstart\fP and \fIend\fP are relative to
the time of the bottom return. In this case \fIstart\fP is often negative so that the
grid shows data above the seafloor, and then down into the subsurface. Finally, if \fImode\fP = 3, 
then \fIstart\fP and \fIend\fP are relative to the time corresponding to the sonar
depth.

.SH EXAMPLES
Suppose that we have a Reson 7k format file (format 88) called 20040722_152111.s7k
that contains subbottom profiler data. Further suppose that we 
have used \fBmbextractsegy\fP to extract the subbottom profiler data
into a segy format file called 20040722_152111.s7k.segy. \fBMBextractsegy\fP 
also generates a "sinf" file containing statistics about the data in the segy
file. In this case, the contents of the sinf file 20040722_152111.s7k.segy.sinf are:
 	SEGY Data File:      20040722_152111.s7k.segy
 	
 	File Header Info:
 	  Channels:                          1
 	  Auxilliary Channels:               0
 	  Sample Interval (usec):           64
 	  Number of Samples in Trace:     8330
 	  Trace length (sec):         0.533120
 	  Data Format:                IEEE 32 bit integer
 	  CDP Fold:                          0
 	
 	Data Totals:
 	  Number of Traces:               2527
 	  Min Max Delta:
 	    Shot number:                    56     2582     2527
 	    Shot trace:                      1        1        1
 	    RP number:                      56     2582     2527
 	    RP trace:                        1        1        1
 	    Delay (sec):              0.000000 0.000000 0.000000
 	    Range (m):                0.000000 0.000000 0.000000
 	    Receiver Elevation (m):   -224.030000 -2.860000 -221.170000
 	    Source Elevation (m):     -224.030000 -2.860000 -221.170000
 	    Source Depth (m):         2.860000 224.030000 -221.170000
 	    Receiver Water Depth (m): 51.510000 487.670000 -436.160000
 	    Source Water Depth (m):   51.510000 487.670000 -436.160000
 	
 	Navigation Totals:
 	
 	  Start of Data:
 	    Time:  07 22 2004 15:20:37.029000  JD204
 	    Lon: -121.8573     Lat:   36.7755
 	
 	  End of Data:
 	    Time:  07 22 2004 15:44:15.438000  JD204
 	    Lon: -121.8572     Lat:   36.7952 
 	
 	Limits:
 	  Minimum Longitude:    -121.8574   Maximum Longitude:    -121.8572
 	  Minimum Latitude:       36.7755   Maximum Latitude:       36.7952

In order to generate a time vs. trace number grid of all traces in the
segy file, with a column for each trace and a row for each time sample,
one can simply invoke \fBmbsegygrid\fP without specifying a range of 
desired traces or the desired time sweep. The program automatically sizes
the grid to include all traces and samples, taking into account trace start
delays if necessary. The root of the output grid filename is specified with
the \fB-O\fP option; if "SubbottomGrid" is specified then the output
grid will be named SubbottomGrid.grd. The command is:
 	mbsegygrid -I 20040722_152111.s7k.segy \
 		-O SubbottomGrid
.br
and the output to the terminal looks like:
 	
 	MBsegygrid Parameters:
 	Input segy file:         20040722_152111.s7k.segy
 	Output fileroot:         SubbottomGrid
 	Input Parameters:
 	     trace mode:         1
 	     trace start:        56
 	     trace end:          2582
 	     channel start:      1
 	     channel end:        1
 	     trace decimation:   1
 	     time sweep:         0.533120 seconds
 	     time delay:         0.000000 seconds
 	     sample interval:    0.000064 seconds
 	     sample decimation:  1
 	     window mode:        0
 	     window start:       0.000000 seconds
 	     window end:         0.000000 seconds
 	     gain mode:          0
 	     gain:               0.000000
 	Output Parameters:
 	     grid filename:      SubbottomGrid.grd
 	     x grid dimension:   2527
 	     y grid dimension:   8331
 	     grid xmin:          55.500000
 	     grid xmax:          2582.500000
 	     grid ymin:          -0.533152
 	     grid ymax:          0.000032
 	     NaN values used to flag regions with no data
 	PROCESS read:0 position:0 rp:56 channel:1 2004/204 15:20:37.029 samples:8330 interval:64 usec minmax: 0.000000 2425.784912
 	PROCESS read:25 position:25 rp:81 channel:1 2004/204 15:20:51.068 samples:8330 interval:64 usec minmax: 0.000000 3832.087402
 	PROCESS read:50 position:50 rp:106 channel:1 2004/204 15:21:05.106 samples:8330 interval:64 usec minmax: 0.000000 1590.149658
 	....................
 	PROCESS read:2475 position:2475 rp:2531 channel:1 2004/204 15:43:46.800 samples:8330 interval:64 usec minmax: 0.000000 991.466064
 	PROCESS read:2500 position:2500 rp:2556 channel:1 2004/204 15:44:00.838 samples:8330 interval:64 usec minmax: 0.000000 838.286926
 	PROCESS read:2525 position:2525 rp:2581 channel:1 2004/204 15:44:14.877 samples:8330 interval:64 usec minmax: 0.000000 985.870667
 		
 	--------------
 	
 	Plot generation shellscript <SubbottomGrid.grd.cmd> created.
 	
 	Instructions:
 	  Execute <SubbottomGrid.grd.cmd> to generate Postscript plot <SubbottomGrid.grd.ps>.
 	  Executing <SubbottomGrid.grd.cmd> also invokes gv to view the plot on the screen.
 	
 	--------------

The size of the the output grid can be checked using the \fBGMT\fP program
\fBgrdinfo\fP. The results of the command:
 	grdinfo SubbottomGrid.grd
.br
look like:
 	SubbottomGrid.grd: Title: Seismic Grid from 20040722_152111.s7k.segy
 	SubbottomGrid.grd: Command: 
 	SubbottomGrid.grd: Remark: 
 	        Projection: SeismicProfile
 	        Grid created by MBsegygrid
 	        MB-system Version 5.0.4
 	        Run by <caress> on <hess> at <Thu Sep 23 16:24:39 2004>
 	SubbottomGrid.grd: Pixel node registration used
 	SubbottomGrid.grd: grdfile format # 0
 	SubbottomGrid.grd: x_min: 55.5 x_max: 2582.5 x_inc: 1 units: Trace Number nx: 2527
 	SubbottomGrid.grd: y_min: -0.533152 y_max: 3.2e-05 y_inc: 6.4e-05 units: Time (seconds) ny: 8331
 	SubbottomGrid.grd: z_min: 0 z_max: 2623.34 units: Trace Signal
 	SubbottomGrid.grd: scale_factor: 1 add_offset: 0

\fBMBsegygrid\fP also uses the macro \fBmbm_grdplot\fP to generate a shellscript
called SubbottomGrid.grd.cmd that, when executed, will use \fBGMT\fP commands
to generate a Postscript plot of the grid.

In order to generate a grid of a subset of the subbottom data (for instance,
shots 1500 - 2000 and the time sweep from 0.25 to 0.35 seconds), use the \fB-S\fP
and \fB-T\fP options:
 	mbsegygrid -I 20040722_152111.s7k.segy \
 		-O SubbottomGrid2 \
 		-S0/1500/2000 -T0.1/0.25
.br
This time the \fBgrdinfo\fP output looks like:
 	SubbottomGrid2.grd: Title: Seismic Grid from 20040722_152111.s7k.segy
 	SubbottomGrid2.grd: Command: 
 	SubbottomGrid2.grd: Remark: 
 	        Projection: SeismicProfile
 	        Grid created by MBsegygrid
 	        MB-system Version 5.0.4
 	        Run by <caress> on <hess> at <Thu Sep 23 16:40:34 2004>
 	SubbottomGrid2.grd: Pixel node registration used
 	SubbottomGrid2.grd: grdfile format # 0
 	SubbottomGrid2.grd: x_min: 1499.5 x_max: 2000.5 x_inc: 1 units: Trace Number nx: 501
 	SubbottomGrid2.grd: y_min: -0.35 y_max: -0.249968 y_inc: 6.4e-05 units: Time (seconds) ny: 1563
 	SubbottomGrid2.grd: z_min: 0 z_max: 558.42 units: Trace Signal
 	SubbottomGrid2.grd: scale_factor: 1 add_offset: 0

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbm_grdplot\fP(l), \fBmbmosaic\fP(l), \fBmbm_grid\fP(l)

.SH BUGS
This tool isn't close to being complete...


