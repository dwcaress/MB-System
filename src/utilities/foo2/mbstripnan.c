/*--------------------------------------------------------------------
 *    The MB-system:    mbstripNaN.c        8/8/02
 *
 *    Copyright (c) 2002-2019 by
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
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#if defined(_WIN32) && !defined(isnan)
#define isnan(x) _isnan(x)
#endif

/*--------------------------------------------------------------------*/
/*
 * Read double x,y,z on stdin and send to stdout all but NaNs
 */
int main() {
	struct node {
		double lon, lat, height;
	};
	struct node n;
	while ((fread(&n, 24, 1, stdin) > 0)) {
		if (!isnan(n.height)) {
			fwrite(&n, 24, 1, stdout);
		}
	}
	exit(0);
}
/*--------------------------------------------------------------------*/
