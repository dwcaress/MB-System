/*--------------------------------------------------------------------
 *    The MB-system:	mb_coor_scale.c	1/21/93
  *
 *    Copyright (c) 1993-2024 by
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
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * The function mb_coor_scale.c returns scaling factors
 * to turn longitude and latitude differences into distances in meters.
 * This function is based on code by James Charters (Scripps Institution
 * of Oceanography).
 *
 * Author:	D. W. Caress
 * Date:	January 21, 1993
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "mb_define.h"
#include "mb_io.h"
#include "mb_status.h"

/*--------------------------------------------------------------------*/
int mb_coor_scale(int verbose, double latitude, double *mtodeglon, double *mtodeglat) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose: %d\n", verbose);
		fprintf(stderr, "dbg2       latitude: %f\n", latitude);
	}
	
	/* ellipsoid coefficients from World Geodetic System Ellipsoid of 1972
	 * - see Bowditch (H.O. 9 -- American Practical Navigator). */
	const double C1 = 111412.84;
	const double C2 = -93.5;
	const double C3 = 0.118;
	const double C4 = 111132.92;
	const double C5 = -559.82;
	const double C6 = 1.175;
	const double C7 = 0.0023;

	/* check that the latitude value is sensible */
	int status = MB_SUCCESS;
	if (fabs(latitude) <= 90.0) {
		const double radlat = DTR * latitude;
		*mtodeglon = 1. / fabs(C1 * cos(radlat) + C2 * cos(3 * radlat) + C3 * cos(5 * radlat));
		*mtodeglat = 1. / fabs(C4 + C5 * cos(2 * radlat) + C6 * cos(4 * radlat) + C7 * cos(6 * radlat));
	}

	/* set error flag if needed */
	else {
		status = MB_FAILURE;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return arguments:\n");
		fprintf(stderr, "dbg2       mtodeglon: %g\n", *mtodeglon);
		fprintf(stderr, "dbg2       mtodeglat: %g\n", *mtodeglat);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:    %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_alvinxy_scale(int verbose, double latitude, double *mtodeglon, double *mtodeglat) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose: %d\n", verbose);
		fprintf(stderr, "dbg2       latitude: %f\n", latitude);
	}
	
	/* Ellipsoid coefficients from Clark 1866 spheroid. */
	/* Taken from: Murphy, C., Singh, H., "Rectilinear Coordinate Frames for Deep Sea Navigation"
		C. Murphy and H. Singh, "Rectilinear coordinate frames for Deep sea navigation," 
		2010 IEEE/OES Autonomous Underwater Vehicles, Monterey, CA, USA, 2010, pp. 1-10, 
		doi: 10.1109/AUV.2010.5779654." */
	const double C1 = 111415.13;
	const double C2 = -94.55;
	const double C3 = -0.12;
	const double C4 = 111132.09;
	const double C5 = -566.05;
	const double C6 = 1.20;
	const double C7 = -0.002;

	/* check that the latitude value is sensible */
	int status = MB_SUCCESS;
	if (fabs(latitude) <= 90.0) {
		const double radlat = DTR * latitude;
		*mtodeglon = 1. / fabs(C1 * cos(radlat) + C2 * cos(3 * radlat) + C3 * cos(5 * radlat));
		*mtodeglat = 1. / fabs(C4 + C5 * cos(2 * radlat) + C6 * cos(4 * radlat) + C7 * cos(6 * radlat));
	}

	/* set error flag if needed */
	else {
		status = MB_FAILURE;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return arguments:\n");
		fprintf(stderr, "dbg2       mtodeglon: %g\n", *mtodeglon);
		fprintf(stderr, "dbg2       mtodeglat: %g\n", *mtodeglat);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:    %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_apply_lonflip(int verbose, int lonflip, double *longitude) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose: %d\n", verbose);
		fprintf(stderr, "dbg2       lonflip:   %d\n", lonflip);
		fprintf(stderr, "dbg2       longitude: %f\n", *longitude);
	}

	/* apply lonflip */
	if (lonflip < 0) {
		if (*longitude > 0.)
			*longitude = *longitude - 360.;
		else if (*longitude < -360.)
			*longitude = *longitude + 360.;
	}
	else if (lonflip == 0) {
		if (*longitude > 180.)
			*longitude = *longitude - 360.;
		else if (*longitude < -180.)
			*longitude = *longitude + 360.;
	}
	else {
		if (*longitude > 360.)
			*longitude = *longitude - 360.;
		else if (*longitude < 0.)
			*longitude = *longitude + 360.;
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return arguments:\n");
		fprintf(stderr, "dbg2       longitude: %f\n", *longitude);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:    %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
