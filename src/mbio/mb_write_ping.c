/*--------------------------------------------------------------------
 *    The MB-system:	mb_write_ping.c	2/3/93
 *
 *    Copyright (c) 1993-2019 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mb_write_ping.c calls the appropriate mbr_ routine for writing
 * the next ping to a multibeam data file.  The current ping data
 * must be in the "new_" variables in the mbio structure pointed
 * to by mbio_ptr.
 *
 * Author:	D. W. Caress
 * Date:	Febrary 3, 1993
 *
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"

/*--------------------------------------------------------------------*/
int mb_write_ping(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	char *function_name = "mb_write_ping";
	int status;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* call the appropriate mbr_ write and translate routine */
	if (mb_io_ptr->mb_io_write_ping != NULL) {
		status = (*mb_io_ptr->mb_io_write_ping)(verbose, mbio_ptr, store_ptr, error);
	}
	else {
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_FORMAT;
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
