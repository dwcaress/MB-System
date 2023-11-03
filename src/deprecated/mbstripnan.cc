/*--------------------------------------------------------------------
 *    The MB-system:    mbstripNaN.c        8/8/02
 *
 *    Copyright (c) 2002-2023 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes 
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *
 *    This program mbstripNaN created by:
 *    Mike McCann
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *     
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/

/* mbstripnan.c
 * Filter output from GMT's grd2xyz removing any nodes
 * that have NaN elevations.  Output meant to feed into GMT's surface
 * in support of the mbm_grd2geovrml macro.
 *
 * Mike McCann   8 August 2002
 * MBARI
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

struct node {
	double lon;
	double lat;
	double height;
};

/*
 * Read double x,y,z on stdin and send to stdout all but NaNs
 */
int main() {
	struct node n;
        const size_t chunk = sizeof(n);
	while ((fread(&n, chunk, 1, stdin) != chunk)) {
		if (std::isnan(n.lon) || std::isnan(n.lat) || std::isnan(n.height)) {
			continue;
		}
		fwrite(&n, chunk, 1, stdout);
	}
	exit(EXIT_SUCCESS);
}
