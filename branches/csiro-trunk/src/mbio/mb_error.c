/*--------------------------------------------------------------------
 *    The MB-system:	mb_error.c	2/2/93
 *    $Id$
 *
 *    Copyright (c) 1993-2017 by
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
 * mb_error.c returns a short error message associated with the
 * input error code.
 *
 * Author:	D. W. Caress
 * Date:	February 2, 1993
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#define DEFINE_MB_MESSAGES 1
#include "mb_status.h"
#include "mb_define.h"
#include "mb_io.h"

static char rcs_id[] = "$Id$";

/*--------------------------------------------------------------------*/
int mb_error(int verbose, int error, char **message) {
	char *function_name = "mb_error";
	int status;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       rcs_id:  %s\n", rcs_id);
		fprintf(stderr, "dbg2       verbose: %d\n", verbose);
		fprintf(stderr, "dbg2       error:   %d\n", error);
		fprintf(stderr, "dbg2       message: %p\n", (void *)message);
		fprintf(stderr, "dbg2       MB_ERROR_MIN: %d\n", MB_ERROR_MIN);
		fprintf(stderr, "dbg2       MB_ERROR_MAX: %d\n", MB_ERROR_MAX);
	}

	/* set the message and status */
	if (error < MB_ERROR_MIN || error > MB_ERROR_MAX) {
		*message = unknown_error_msg[0];
		status = MB_FAILURE;
	}
	else if (error > MB_ERROR_NO_ERROR) {
		*message = fatal_error_msg[error];
		status = MB_SUCCESS;
	}
	else {
		*message = nonfatal_error_msg[-error];
		status = MB_SUCCESS;
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       message: %s\n", *message);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/
int mb_notice_log_datatype(int verbose, void *mbio_ptr, int data_id) {
	char *function_name = "mb_notice_log_datatype";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       rcs_id:     %s\n", rcs_id);
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       data_id:    %d\n", data_id);
	}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* log data record type in the notice list */
	if (data_id > 0 && data_id <= MB_DATA_KINDS) {
		mb_io_ptr->notice_list[data_id]++;
	}
	else {
		status = MB_FAILURE;
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/
int mb_notice_log_error(int verbose, void *mbio_ptr, int error_id) {
	char *function_name = "mb_notice_log_error";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       rcs_id:     %s\n", rcs_id);
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       error_id:   %d\n", error_id);
	}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* log any nonfatal error in the notice list */
	if (error_id < 0 && error_id >= MB_ERROR_MIN) {
		mb_io_ptr->notice_list[MB_DATA_KINDS - error_id]++;
	}
	else {
		status = MB_FAILURE;
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/
int mb_notice_log_problem(int verbose, void *mbio_ptr, int problem_id) {
	char *function_name = "mb_notice_log_problem";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       rcs_id:     %s\n", rcs_id);
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       problem_id: %d\n", problem_id);
	}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* log data record type in the notice list */
	if (problem_id > 0 && problem_id <= MB_PROBLEM_MAX) {
		mb_io_ptr->notice_list[MB_DATA_KINDS - MB_ERROR_MIN + problem_id]++;
	}
	else {
		status = MB_FAILURE;
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/
int mb_notice_get_list(int verbose, void *mbio_ptr, int *notice_list) {
	char *function_name = "mb_notice_get_list";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int i;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       rcs_id:         %s\n", rcs_id);
		fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:       %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       notice_list:    %p\n", (void *)notice_list);
	}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* copy notice list */
	for (i = 0; i < MB_NOTICE_MAX; i++) {
		notice_list[i] = mb_io_ptr->notice_list[i];
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Return value:\n");
		for (i = 0; i < MB_NOTICE_MAX; i++)
			fprintf(stderr, "dbg2       notice_list[%2.2d]: %d\n", i, notice_list[i]);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/
int mb_notice_message(int verbose, int notice, char **message) {
	char *function_name = "mb_notice_message";
	int status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       rcs_id:     %s\n", rcs_id);
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       notice:     %d\n", notice);
	}

	/* set the message and status */
	if (notice < 0 || notice > MB_NOTICE_MAX) {
		*message = unknown_notice_msg[0];
		status = MB_FAILURE;
	}
	else {
		*message = notice_msg[notice];
		status = MB_SUCCESS;
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       message: %s\n", *message);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/
