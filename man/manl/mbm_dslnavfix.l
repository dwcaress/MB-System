.TH mbm_dslnavfix l "17 December 1998" "MB-System 4.6" "MB-System 4.6"
.SH NAME
\fBmbm_dslnavfix\fP - Translate WHOI DSL AMS-120 navigation from
UTM eastings and northings to longitude and latitude.

.SH VERSION
Version 4.6

.SH SYNOPSIS
\fBmbm_dslnavfix\fP \fB-F\fP\fInavformat\fP 
\fB-I\fP\fIinfile\fP \fB-J\fP\fIutm_zone\fP 
\fB-O\fP\fIoutfile\fP 
[\fB-D\fP\fIdecimate\fP \fB-E\fP\fIellipsoid\fP \fB-H\fP \fB-V\fP]

.SH DESCRIPTION
\fBmbm_dslnavfix\fP is a macro to translate 
WHOI DSL AMS-120 navigation from
UTM eastings and northings to longitude and latitude.
The WHOI DSL group typically provides processed AMS-120
navigation is files separate from the bathymetry and
sidescan data. Moreover, the navigation is generally
sampled less frequently than the sonar
pings, and is reported in UTM easting and northing meters.
The \fBmbm_dslnavfix\fP macro is used to translate the
eastings and northings into the geographic coordinates
(longitude and latitude) used by MB-System programs. The
program \fBmbmerge\fP can be used to merge the translated
navigation in with the bathymetry and sidescan.

The DSL AMS-120 bathymetry and sidescan are provided in
either a set of parallel files supported as \fBMB-System\fP
format 111 (the old scheme) or in the OIC single file format
supported as \fBMB-System\fP format 141 (the new scheme).
In either case, we recommend translating the data to similar
but enhanced formats 112 and 142, respectively, to ease
processing.

We recommend the following procedure when
processing WHOI DSL AMS-120 data with MB-System:
.br
 	1) Concatenate all of the processed navigation
 	   files into a single file.
 		cat *.nav > cruise.rnav
 	2) Translate the DSL UTM navigation into geographic
 	   coordinates. In the "old" case the navigation is 
           in a format designated by \fB-F\fP\fI1\fP and 
           in the new case by \fB-F\fP\fI2\fP. Note that 
           you must know the UTM zone used - here we assume 
           the zone is +27.
 		mbm_dslnavfix -Icruise.rnav \\
 			-Ocruise.mrg2 -F1 -J27 -V
 	3) Copy the bathymetry and sidescan data in each
 	   file from format 111 to format 112 or from
           format 141 to 142.
 		mbcopy -F111/112 \\
 			-IDSL120.940630_1100.bat.dat \\
 			-ODSL120.940630_1100.mb112
 	4) Merge the translated navigation with each of the 
 	   format 112 data files. Note  that \fBmbm_dslnavfix\fP 
           produces navigation in \fBmbmerge\fP format 2
 	   so that the \fB-M\fP\fI2\fP option is used.
 		mbmerge -F112 \\
 			-IDSL120.940630_1100.mb112 \\
 			-ODSL120.940630_1100_n.mb112 \\
 			-M2 -Ncruise.mrg2 -V

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
.B \-D
\fIdecimate\fP
.br
This option allows the user to decimate the input navigation.
The \fIdecimate\fP value represents the decimation factor.
The default is \fIdecimate\fP = 1, or no decimation.
.TP
.B \-E
\fIellipsoid\fP
.br
This option sets the ellipsoid used in the UTM projection.
The choices are:
 	WGS-84		1984 World Geodetic System
 	GRS-80		1980 International Geodetic Reference System
 	WGS-72		1972 World Geodetic System
 	Australian	1965 Used down under
 	Krasovsky	1940 Used in the Soviet Union
 	International	1924 Worldwide use
 	Hayford-1909	1909 Same as the International 1924
 	Clarke-1880	1880 Most of Africa, France
 	Clarke-1866	1866 North America, the Phillipines
 	Airy		1830 Great Britain
 	Bessel		1841 Central Europe, Chile, Indonesia
 	Everest		1830 India, Burma, Pakistan,  etc.
 	Sphere		1980 The mean radius in GRS-80
.br
Default: Clarke-1866 as per USGS standard.
.TP
.B \-F
\fInavformat\fP
.br
Sets the input navigation format. The available formats
include the old, pre-OIC format (\fB-F\fP\fI1\fP), the new, OIC
associated formats (\fB-F\fP\fI2\fP), and navigation obtained
using \fBmblist\fP with the \fB-OtXY\fP option.
.TP
.B \-H
This "help" flag cause the program to print out a description
of its operation and then exit immediately.
.TP
.B \-I
\fIfilename\fP
.br
Sets the input WHOI DSL AMS-120 navigation filename.
.TP
.B \-J
\fIutm_zone\fP
.br
Sets the UTM zone used for the translation. The UTM zones
are 6 degrees wide in longitude and begin with zone 1 extending
from 180 W to 174 W, zone 2 extending from 174 W to 168 W, and
so on through zone 60. Negative zone
values are used for the Southern Hemisphere. 
.TP
.B \-O
\fIoutfile\fP
.br
Sets the output filename for the translated navigation. 
.TP
.B \-V
Causes \fBmbm_plot\fP to operate in "verbose" mode 
so that it outputs
more information than usual.

.SH EXAMPLES
Suppose we have obtained a WHOI DSL navigation file
as outlined above and execute the macro as suggested:
.br
 		mbm_dslnavfix -Icruise.rnav \\
 			-Ocruise.mrg2 -F1 -J27 -V
.br
The output will appear as follows:
 
 Running mbm_dslnavfix...
 
 Program Status:
 --------------
 
   Input DSL navigation format:   1
   Input DSL navigation file:     dsl.nav
   Output navigation file:        dsl.mrg2
   Temporary projected nav file:  tmp_proj_8044.nav
   Temporary geographic nav file: tmp_geo_8044.nav

   ------------
 
   Projection:                    UTM Zone 27
   Projection units:              meters
 
   ------------
 
 586 navigation records read...
 
 Running mapproject...
 mapproject tmp_proj_8044.nav -Ju27/1:1000000 -I \\
 	-R-20:0/0:0/-19:0/1:0r > tmp_geo_8044.nav
 
 Deleting tmp_proj_8044.nav, tmp_geo_8044.nav...
 
 Done...

.SH BUGS
Let us know...


