/*--------------------------------------------------------------------
 *    The MB-system:    mbstripNaN.c        8/8/02
 *    $Id: mbstripnan.c,v 5.2 2003-04-17 21:18:57 caress Exp $
 *
 *    Copyright (c) 2002, 2003 by
 *    Mike McCann (mccann@mbari.org)
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/

/* mbstripnan.c
 * Little program to filter output from GMT's grd2xyz removing any nodes 
 * that have NaN elevations.  Output meant to feed into GMT's surface
 * in support of the mbm_grd2geovrml macro.
 *
 * Mike McCann   8 August 2002
 * MBARI
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.1  2002/08/21 00:57:11  caress
 * Release 5.0.beta22
 *
 * Revision 5.0  2002/08/10 07:00:21  caress
 * First version from Mike McCann.
 *
 */

#include <stdio.h>
#include <math.h>
/*--------------------------------------------------------------------*/
/*
 * Read double x,y,z on stdin and send to stdout all but NaNs
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
/*--------------------------------------------------------------------*/

