/*--------------------------------------------------------------------
 *    The MB-system:	mb_error.c	2/2/93
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
 * mb_error.c returns a short error message associated with the
 * input error code.
 *
 * Author:	D. W. Caress
 * Date:	February 2, 1993
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

#define DEFINE_MB_MESSAGES 1
#include "mb_define.h"
#include "mb_io.h"
#include "mb_status.h"

/*--------------------------------------------------------------------*/
int mb_error(int verbose, int error, char **message) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose: %d\n", verbose);
		fprintf(stderr, "dbg2       error:   %d\n", error);
		fprintf(stderr, "dbg2       message: %p\n", (void *)message);
		fprintf(stderr, "dbg2       MB_ERROR_MIN: %d\n", MB_ERROR_MIN);
		fprintf(stderr, "dbg2       MB_ERROR_MAX: %d\n", MB_ERROR_MAX);
	}

	int status;

	/* set the message and status */
	if (error < MB_ERROR_MIN || error > MB_ERROR_MAX) {
		*message = (char *) unknown_error_msg[0];
		status = MB_FAILURE;
	}
	else if (error > MB_ERROR_NO_ERROR) {
		*message = (char *) fatal_error_msg[error];
		status = MB_SUCCESS;
	}
	else {
		*message = (char *) nonfatal_error_msg[-error];
		status = MB_SUCCESS;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       message: %s\n", *message);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_notice_log_datatype(int verbose, void *mbio_ptr, int data_id) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       data_id:    %d\n", data_id);
	}

	/* get pointer to mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	int status = MB_SUCCESS;

	/* log data record type in the notice list */
	if (data_id > 0 && data_id <= MB_DATA_KINDS) {
		mb_io_ptr->notice_list[data_id]++;
	}
	else {
		status = MB_FAILURE;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_notice_log_error(int verbose, void *mbio_ptr, int error_id) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       error_id:   %d\n", error_id);
	}

	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	int status = MB_SUCCESS;

	/* log any nonfatal error in the notice list */
	if (error_id < 0 && error_id >= MB_ERROR_MIN) {
		mb_io_ptr->notice_list[MB_DATA_KINDS - error_id]++;
	}
	else {
		status = MB_FAILURE;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_notice_log_problem(int verbose, void *mbio_ptr, int problem_id) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       problem_id: %d\n", problem_id);
	}

	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	int status = MB_SUCCESS;

	/* log data record type in the notice list */
	if (problem_id > 0 && problem_id <= MB_PROBLEM_MAX) {
		mb_io_ptr->notice_list[MB_DATA_KINDS - MB_ERROR_MIN + problem_id]++;
	}
	else {
		status = MB_FAILURE;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_notice_get_list(int verbose, void *mbio_ptr, int *notice_list) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:       %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       notice_list:    %p\n", (void *)notice_list);
	}

	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* copy notice list */
	for (int i = 0; i < MB_NOTICE_MAX; i++) {
		notice_list[i] = mb_io_ptr->notice_list[i];
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		for (int i = 0; i < MB_NOTICE_MAX; i++)
			fprintf(stderr, "dbg2       notice_list[%2.2d]: %d\n", i, notice_list[i]);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_notice_message(int verbose, int notice, char **message) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       notice:     %d\n", notice);
	}

	int status = MB_SUCCESS;

	/* set the message and status */
	if (notice < 0 || notice >= MB_NOTICE_MAX) {
		*message = (char *) unknown_notice_msg[0];
		status = MB_FAILURE;
	}
	else {
		*message = (char *) notice_msg[notice];
		status = MB_SUCCESS;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       message: %s\n", *message);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
