.TH mbm_project l "13 May 2002" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbm_project\fP - Create an executable shellscript which will generate
a TIFF image of gridded data in a GMT grd file.

.SH VERSION
Version 5.0

.SH SYNOPSIS
\fBmbm_project\fP \fB-JZ\fP\fIprojection\fP 
[\fB-I\fP\fIfile\fP \fB-H -Q\fP]

.SH DESCRIPTION
\fBmbm_project\fP is a macro to generate a shellscript 
which, when executed, will 
generate a geographically located TIFF image of gridded 
data. The primary purpose of this macro is to allow the simple, 
semi-automated production of a nice looking image with a few 
command line arguments. This image can be loaded into the 
ArcInfo and ArcView GIS packages as geographically located 
coverages. Several styles of images can be generated, including 
color fill and shaded relief maps. The available options mirror 
a subset of the options in \fBmbm_grdplot\fP, allowing users to easily 
generate equivalent Postscript plots and TIFF images of gridded 
data. 

.SH AUTHORSHIP
David W. Caress (caress@mbari.org)
.br
  Monterey Bay Aquarium Research Institute
.br
Dale N. Chayes (dale@ldeo.columbia.edu)
.br
  Lamont-Doherty Earth Observatory

.SH SIMPLE DESCRIPTION OF BASIC OPTIONS
.TP
.B \fB-H\fP
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \fB-I\fP\fIfile\fP
Sets the name of the data file to be processed.
The first two columns of the ASCII input
data stream must be either longitude and
latitude values (for geographic to projected
transformation) or easting and northing values
(for projected to geographic transformation). 
Any additional data is not affected by the transformation.
.TP
\fB\-JZ\fP\fIprojection\fP
.br
The supported projections include defined ellipsoids and
origins, allowing the calculation of locations
in projected meters or feet; these units are called
eastings and northings. The \fIprojection\fP parameter
includes a six character projection id followed by a
one character units flag. The currently supported
special projections are:
.br
 	Projection Name		Projection ID
 	---------------		-------------
 	UTM Zone +01			UTM+01
 	UTM Zone +02			UTM+02
	............			......
 	UTM Zone +18			UTM+18
.br
The units flag controls the units of the eastings
and northings. Values
of 'M' or 'm' indicate eastings and northings calculated
in feet from the projection origin. Values
of 'F' or 'f' indicate eastings and northings calculated
in meters.
.TP
.B \fB-Q\fP
This option causes the transformation to be from
projected coordinates (eastings and northings) to
geographic coordinates (longitude and latitude).
The default is to transform from geographic to
projected coordinates.

.SH EXAMPLES
Suppose we have a file containing longitude, latitude,
and depth triples called llz.dat. To transform the
coordinates to eastings and northings in the 
UTM Zone 10 projection, use:
.br
 	mbm_project -Illz.dat -JZUTM+10 > enz.dat
.br
The file enz.dat will contain eastings and northings
instead of longitude and latitude values. To transform
back to geographic coordinates, use:
.br
 	mbm_project -Ienz.dat -JZUTM+10 -Q > llz2.dat
.br

.SH SEE ALSO
\fBmbsystem\fP(l), \fBproject\fP

.SH BUGS
More projections need to be supported. 
