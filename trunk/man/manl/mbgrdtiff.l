.TH mbgrdtiff l "4 January 2001" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbgrdtiff\fP - generates a geographically located TIFF image from
a GMT grid file.

.SH VERSION
Version 5.0

.SH SYNOPSIS
\fBmbgrdtiff\fP \fB-C\fIcptfile\fP \fB-I\fIgrdfile\fP 
\fB-O\fItiff_file\fP [\fB-H\fP \fB-K\fIintensfile\fP \fB-V\fP]

.SH DESCRIPTION
\fBmbgrdtiff\fP generates a TIFF image from a \fBGMT\fP grid. The 
image generation is similar to that of the \fBGMT\fP program 
\fBgrdimage\fP. In particular, the color map is applied from 
a \fBGMT\fP CPT file, and shading overlay grids may be applied. 
The output TIFF file contains information allowing
the ArcView and ArcInfo GIS packages to import the image
as a geographically located coverage. The image is 8 bits
per pixel if the color map is a grayscale, and 24 bits
per pixel otherwise. In order to automatically generate a
reasonable image of a grid, use the macro \fBmbm_grdtiff\fP.

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
.B \-C
Sets the color palette (CPT) file which controls the color of the plot.
See documentation of the GMT package for a complete description
of cpt files. If the R, G, and B values in the CPT file are all
equal for each data level (i.e. R=G=B), the the output TIFF
image will be 8 bits per pixel. Otherwise, the output TIFF image
will be 24 bits per pixel.
.TP
.B \-H
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \-I
\fIgrdfile\fP
.br
Sets the name of the gridded data file to be plotted.  
The data must be in a form acceptable to \fBGMT\fP version 3
programs (see the \fBGMT\fP Cookbook & Technical Reference).
.TP
.B \-K
\fIintensity_file\fP
.br
Sets the name of the gridded data file containing
intensity values to be used for shading the map.  
Alternatively, \fIgrdfile\fP may be a list of grid files
(one filename on each line) to be used together. If a
list of files is supplied, the intensity files must
conform in order to the list of data grid files they will shade.
.TP
.B \-O
\fIroot\fP
.br
Sets the root used to construct the filename of the output shellscript
(\fIroot\fP.cmd) and names of files created when the shellscript is 
run.  Normally the 
name of the input grid file or grid file list is 
used as the \fIroot\fP. 
.TP
.B \-V
Selects verbose mode [Default runs "silently"].
.SH EXAMPLES
Suppose we have obtained two GRD files called PunaA_bath.grd
and PunaA_ss.grd, both with dimensions
1162 x 1068. The file PunaA_bath.grd contains seafloor
topography (depth values are negative, ranging
from -5035 m to -1619 m), and the file PunaA_ss.grd 
contains a sidescan mosaic (reflectivity values
ranging from 0 to 128 dB). In order to generate
a 24 bit color TIFF image of the seafloor topography, we use:
.br
 	mbgrdtiff -I PunaA_bath.grd \
 		-O PunaA_bath.tif \
 		-C bath.cpt -V
.br
where the color is controlled by a GMT cpt file bath.cpt 
containing:
.br
 	-5250  37  57 175 -4875  40 127 251
 	-4875  40 127 251 -4500  50 190 255
 	-4500  50 190 255 -4125 106 235 255
 	-4125 106 235 255 -3750 138 236 174
 	-3750 138 236 174 -3375 205 255 162
 	-3375 205 255 162 -3000 240 236 121
 	-3000 240 236 121 -2625 255 189  87
 	-2625 255 189  87 -2250 255 161  68
 	-2250 255 161  68 -1875 255 186 133
 	-1875 255 186 133 -1500 255 255 255
.br
In order to generate
an 8 bit grayscale TIFF image of the sidescan mosaic, we use:
.br
 	mbgrdtiff -I PunaA_ss.grd \
 		-O PunaA_ss.tif \
 		-C ss.cpt -V
.br
where the grayscale is controlled by a GMT cpt file ss.cpt 
containing:
.br
 	 0.00   0   0   0   16.50  26  26  26
 	16.50  26  26  26   22.87  51  51  51
 	22.87  51  51  51   27.00  77  77  77
 	27.00  77  77  77   30.00 102 102 102
 	30.00 102 102 102   32.25 128 128 128
 	32.25 128 128 128   34.25 153 153 153
 	34.25 153 153 153   36.25 179 179 179
 	36.25 179 179 179   38.50 204 204 204
 	38.50 204 204 204   41.23 230 230 230
 	41.23 230 230 230  128.00 255 255 255
.br
Here greater reflectivity amplitudes are represented by
light grayscales.

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbm_grdplot\fP(l), \fBgmt\fP(l)

.SH BUGS
Let us know.
