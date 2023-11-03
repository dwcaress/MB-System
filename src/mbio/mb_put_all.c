/*--------------------------------------------------------------------
 *    The MB-system:	mb_put_all.c	2/4/93
 *
 *    Copyright (c) 1993-2023 by
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
 * mb_put_all.c writes multibeam data to a file
 * which has been initialized by mb_write_init(). Crosstrack distances
 * are used rather than lon and lat for the beams. Values are also read
 * from a storage data structure including
 * all possible values output by the particular multibeam system
 * associated with the specified format.
 *
 * Author:	D. W. Caress
 * Date:	February 4, 1993
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"

/*--------------------------------------------------------------------*/
// TODO(schwehr): Make usevalues a bool
int mb_put_all(int verbose, void *mbio_ptr, void *store_ptr, int usevalues, int kind, int time_i[7], double time_d, double navlon,
               double navlat, double speed, double heading, int nbath, int namp, int nss, char *beamflag, double *bath,
               double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss, double *ssacrosstrack,
               double *ssalongtrack, char *comment, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       usevalues:  %d\n", usevalues);
		fprintf(stderr, "dbg2       kind:       %d\n", kind);
	}
	if (verbose >= 2 && usevalues && kind != MB_DATA_COMMENT) {
		fprintf(stderr, "dbg2       time_i[0]:  %d\n", time_i[0]);
		fprintf(stderr, "dbg2       time_i[1]:  %d\n", time_i[1]);
		fprintf(stderr, "dbg2       time_i[2]:  %d\n", time_i[2]);
		fprintf(stderr, "dbg2       time_i[3]:  %d\n", time_i[3]);
		fprintf(stderr, "dbg2       time_i[4]:  %d\n", time_i[4]);
		fprintf(stderr, "dbg2       time_i[5]:  %d\n", time_i[5]);
		fprintf(stderr, "dbg2       time_i[6]:  %d\n", time_i[6]);
		fprintf(stderr, "dbg2       time_d:     %f\n", time_d);
		fprintf(stderr, "dbg2       navlon:     %f\n", navlon);
		fprintf(stderr, "dbg2       navlat:     %f\n", navlat);
		fprintf(stderr, "dbg2       speed:      %f\n", speed);
		fprintf(stderr, "dbg2       heading:    %f\n", heading);
	}
	if (verbose >= 2 && usevalues && kind == MB_DATA_DATA) {
		fprintf(stderr, "dbg2       nbath:      %d\n", nbath);
		if (verbose >= 3 && nbath > 0) {
			fprintf(stderr, "dbg3       beam  flag  bath  crosstrack alongtrack\n");
			for (int i = 0; i < nbath; i++)
				fprintf(stderr, "dbg3       %4d   %3d   %f    %f     %f\n", i, beamflag[i], bath[i], bathacrosstrack[i],
				        bathalongtrack[i]);
		}
		fprintf(stderr, "dbg2       namp:       %d\n", namp);
		if (verbose >= 3 && namp > 0) {
			fprintf(stderr, "dbg3       beam    amp  crosstrack alongtrack\n");
			for (int i = 0; i < namp; i++)
				fprintf(stderr, "dbg3       %4d   %f    %f     %f\n", i, amp[i], bathacrosstrack[i], bathalongtrack[i]);
		}
		fprintf(stderr, "dbg2       nss:        %d\n", nss);
		if (verbose >= 3 && nss > 0) {
			fprintf(stderr, "dbg3       pixel sidescan crosstrack alongtrack\n");
			for (int i = 0; i < nss; i++)
				fprintf(stderr, "dbg3       %4d   %f    %f     %f\n", i, ss[i], ssacrosstrack[i], ssalongtrack[i]);
		}
	}
	if (verbose >= 2 && usevalues && kind == MB_DATA_COMMENT) {
		fprintf(stderr, "dbg2       comment:    %s\n", comment);
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* insert values into structure if requested */
	int status = MB_SUCCESS;
	if (usevalues) {
		status = mb_insert(verbose, mbio_ptr, store_ptr, kind, time_i, time_d, navlon, navlat, speed, heading, nbath, namp, nss,
		                   beamflag, bath, amp, bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, error);
	}

	/* write the data */
	status &= mb_write_ping(verbose, mbio_ptr, store_ptr, error);

	/* increment counters */
	if (status == MB_SUCCESS) {
		if (kind == MB_DATA_DATA)
			mb_io_ptr->ping_count++;
		else if (kind == MB_DATA_NAV)
			mb_io_ptr->nav_count++;
		else if (kind == MB_DATA_COMMENT)
			mb_io_ptr->comment_count++;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
