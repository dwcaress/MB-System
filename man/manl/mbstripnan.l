.TH mbstripNaN l "8 August 2002" "MB-System 5.0" "MB-System 5.0"
.SH NAME
\fBmbstripNaN\fP - filter to remove NaN nodes 

.SH VERSION
Version 5.0

.SH SYNOPSIS
\fBmbstripNaN\fP

.SH DESCRIPTION
\fBmbstripNaN\fP is a utility for removing NaN nodes that are produced
by the GMT utilities \fBgrd2xyz\fP and \fBblockmean\fP with the -bo 
option.  The standard output
of \fBmbstripNaN\fP may be fed into the standard input of \fBsurface\fP with 
the -bi option.  This is used by the \fBmbm_grd2geovrml\fP utility in 
order to spline fill areas of no data so that irregular bathymetry may
be effectively visualized using GeoVRML which has no concept of NaN.

Note that \fBmbstripNaN\fP works with a double precision data stream.

The code is simple.  Here it is:

  #include <stdio.h>
  #include <math.h>

  /*
   * Read double x,y,z on stdin and send to stdout all 
   * triplets but ones where z == NaN
   */
  main () {
 	  struct node { double lon, lat, height; };
 	  struct node n;
 	  while ( ( fread(&n, 24, 1, stdin) > 0 ) ) {
 		  if ( ! isnan(n.height) ) {
 			  fwrite(&n, 24, 1, stdout);
 		  }
 	  }
   }
  

.SH EXAMPLE

This is a command that \fBmbm_grd2geovrml\fP constructs and executes:

  #
  # Convert grid to xyz format for filling in blank 
  # areas, preprocess w/blockmean, use surface to 
  # extrapolate to no data areas
  #
  grd2xyz -bo Samp_OregonMarginI_bath.grd | blockmean \\
    -bi -bo -V -I0.00336363636363635/0.00134545454545457 \\
    -R-125.2/-124.867/45/45.1332 | mbstripNaN | surface \\
    -bi -I0.00336363636363635/0.00134545454545457 \\
    -S0.333312 -T0.35 -GTmp_OregonMarginI_bath.grd -V \\
    -R-125.2/-124.867/45/45.1332 


.SH SEE ALSO

\fBgrd2xyz\fP(1),
\fBblockmean\fP(1),
\fBsurface\fP(1),
\fBmbm_grd2geovrml\fP(1)



.SH BUGS

How many bugs can there be in an 8 line program?  


