.TH mbm_grdplot l "31 October 1994"
.SH NAME
\fBmbm_grdplot\fP - Create an executable shellscript which will generate
a GMT plot of gridded data in a GMT grd file.

.SH VERSION
Version 4.1

.SH SYNOPSIS
\fBmbm_grdplot\fP \fB-I\fIfile \fP[\fB-A\fIazimuth\fP \fB-C -D -E\fIazimuth/elevation\fP \fB-G -H -O\fIroot\fP \fB-P -S -T -V -X\fP]

.SH DESCRIPTION
\fBmbm_grdplot\fP is an utility which generates a shellscript for making
a Postscript plot of the gridded data contained in the specified file.  
Several styles of plots can be generated, including color 
fill maps, contour maps, color fill maps overlaid with contours, 
shaded relief color maps, and 3D color shaded views.  Once the basic 
plotting capability exists, the user
can edit the shellscript to obtain precisely the sort of plot desired.
This macro makes using GMT a bit easier by providing a working template
so that the user does not always have to start from scratch. 
The plot will be scaled to fit on an 8.5 X 11 \ninch page.  

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
.B \fB-A\fIazimuth\fP
Sets the illumination azimuth in degrees for shaded relief plots.  
An \fIazimuth\fP
of 0 represents illumination from the north, and \fIazimuth\fP = 90
represents illumination from the east.  Default: \fIazimuth\fP = 90.
.TP
.B \fB-C\fP
Sets flag indicating that a contour map of the data is desired.
.TP
.B \fB-D\fP
Normally, the color table used for color maps and views runs from
cool colors for low grid values to hot colors for high grid values.
This option reverses the color table so that cool colors correspond
to high values and hot colors to low values.
.TP
.TP
.B \fB-E\fIazimuth/elevation\fP
Sets the azimuth and elevation in degrees of the viewpoint for 3D
perspective views.  An \fIazimuth\fP
of 0 represents a view from the north, and \fIazimuth\fP = 90
represents a view from the east.  An \fIelevation\fP of 0 represents
a horizontal view.  Default: \fIazimuth\fP = 200.; \fIelevation\fP = 40.
.TP
.B \fB-G\fP
Sets flag indicating that a color image of the data is desired.
.TP
.B \fB-H\fP
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \fB-I\fIgrdfile\fP
Sets the name of the gridded data file to be plotted.  The data must be
in the GMT version 2 GRD format (netCDF based). 
.TP
.B \fB-O\fIroot\fP
Sets the root used to construct the filename of the output shellscript
(\fIroot\fP.cmd) and names of files created when the shellscript is 
run.  Normally the 
name of the input GRD file is used as the \fIroot\fP. 
.TP
.B \fB-P\fP
This option will cause the shellscript to include lines which execute
a program to display the Postscript image on the screen.  The program
to be used to display the Postscript is set using \fBmbdefaults\fP;
the default value can be overridden by setting the environment
variable $MB_PS_VIEWER.
.TP
.B \fB-S\fP
Sets flag indicating that a color shaded relief map of the data is desired.
This flag implicitly sets the \fB-F\fP flag as well.  The illumination
azimuth is controlled by the \fB-A\fP option.
.TP
.B \fB-T\fP
Sets flag indicating that a 3D perspective view is desired.
.TP
.B \fB-V\fP
Causes \fBmbm_grdplot\fP to operate in "verbose" mode so that it outputs
more information than usual.
.TP
.B \fB-X\fP
Normally, \fBmbm_grdplot\fP creates an executable shellscript and
then exits.  This option will cause the shellscript to be executed
before \fBmbm_grdplot\fP exits.

.SH EXAMPLES
Suppose one has obtained a GRD file containing gridded bathymetry
data called grd_seamount. In order to generate a shellscript
which will in turn generate a contour plot of the bathymetry,
the following will suffice:
 	mbm_grdplot -Igrd_seamount -C -P -V -Ocontour
.br
To generate a color fill plot use:
 	mbm_grdplot -Igrd_seamount -G -D -P -V -Ocolorfill
.br
where the \fB-D\fP option is used so that hot colors are
shallow and cool colors are deep.
To generate a color shaded relief plot with illumination
from the east use:
 	mbm_grdplot -Igrd_seamount -G -S -A90 -D -P -V 
 		-Oshadedrelief
.br
To generate a 3D perspective view with illumination from the
east, a viewpoint azimuth of 200 degrees, and a viewpoint
elevation of 40 degrees:
 	mbm_grdplot -Igrd_seamount -G -S -T -A90 -E200/40 
 		 -D -P -V -Othreed
.br
As an example, the contents of the shellscript "shadedrelief.cmd" are:
 #
 # Shellscript to create Postscript plot of data in grd file
 # Created by macro mbm_grdplot
 #
 # Make color pallette table file
 echo Making color pallette table file...
 echo -6176 37 57 175 -5904 21 92 236 > shadedrelief.cpt
 echo -5904 21 92 236 -5632 40 127 251 >> shadedrelief.cpt
 echo -5632 40 127 251 -5359 0 160 255 >> shadedrelief.cpt
 echo -5359 0 160 255 -5087 50 190 255 >> shadedrelief.cpt
 echo -5087 50 190 255 -4815 87 215 255 >> shadedrelief.cpt
 echo -4815 87 215 255 -4543 106 235 255 >> shadedrelief.cpt
 echo -4543 106 235 255 -4271 138 236 174 >> shadedrelief.cpt
 echo -4271 138 236 174 -3998 205 255 162 >> shadedrelief.cpt
 echo -3998 205 255 162 -3726 240 236 121 >> shadedrelief.cpt
 echo -3726 240 236 121 -3454 255 189 87 >> shadedrelief.cpt
 echo -3454 255 189 87 -3182 255 161 68 >> shadedrelief.cpt
 echo -3182 255 161 68 -2909 255 186 133 >> shadedrelief.cpt
 echo -2909 255 186 133 -2637 255 221 171 >> shadedrelief.cpt
 echo -2637 255 221 171 -2365 255 255 255 >> shadedrelief.cpt
 #
 # Get shading array
 echo Getting shading array...
 echo Running grdgradient...
 grdgradient grd_seamount -A90 -Ggrd_seamount.grad -N -M
 echo Running grdhisteq...
 grdhisteq grd_seamount.grad -Ggrd_seamount.eq -N
 echo Running grdmath...
 grdmath grd_seamount.eq / 6.0 = grd_seamount.int
 rm -f grd_seamount.grad grd_seamount.eq
 #
 # Make color image
 echo Running grdimage...
 grdimage grd_seamount -Jm10.258074503462568 -R-157.5/-156.75/21.75/22.25 -Cshadedrelief.cpt -Igrd_seamount.int -X1.0 -Y1.5 -K -V > shadedrelief.ps
 #
 # Make color scale
 echo Running psscale...
 psscale  -Cshadedrelief.cpt -D3.8467779387984633/-0.5/6.1548447020775416/0.14999999999999999h -K -O -V >> shadedrelief.ps
 #
 # Make basemap
 echo Running psbasemap...
 psbasemap -Jm10.258074503462568 -R-157.5/-156.75/21.75/22.25 -B0.1666667/0.1666667 -U -O -V >> shadedrelief.ps
 #
 # Delete surplus files
 echo Deleting surplus files...
 rm -f shadedrelief.cpt
 #
 # Run xpsview
 echo Running xpsview in background...
 xpsview shadedrelief.ps &
 #
 # All done!
 echo All done!

.SH SEE ALSO
\fBmbsystem\fP(l), \fBmbgrid\fP(l), \fBmbcontour\fP(l), 
\fBmbswath\fP(l), \fBmbdefaults\fP(l), \fBmbm_plot\fP(l)

.SH BUGS
Uses only the standard mercator projection and is thus useless for
polar data.  Doesn't do a particularly good job of choosing tick
and contour intervals. Not titles or axis annotations.



