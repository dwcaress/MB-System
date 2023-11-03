/*--------------------------------------------------------------------
 *    The MB-system:	mb_compare.c	11/19/98
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
 * mb_compare.c includes the "mb_" functions for comparing values
 * which are passed to the qsort routine.
 *
 * Author:	D. W. Caress
 * Date:	November 19,  1998
 */

#include <math.h>
#include <stdio.h>

#include "mb_define.h"
#include "mb_process.h"

/*--------------------------------------------------------------------*/
/* 	function mb_int_compare compares int values. */
int mb_int_compare(const void *a, const void *b) {
	const int *aa = (int *)a;
	const int *bb = (int *)b;

	if (*aa > *bb)
		return (1);
	else
		return (-1);
}
/*--------------------------------------------------------------------*/
/* 	function mb_double_compare compares double values. */
int mb_double_compare(const void *a, const void *b) {
	const double *aa = (double *)a;
	const double *bb = (double *)b;

	if (*aa > *bb)
		return (1);
	else
		return (-1);
}
/*--------------------------------------------------------------------*/
/* 	function mb_edit_compare compares mb_edit_struct values. */
int mb_edit_compare(const void *a, const void *b) {
	const struct mb_edit_struct *aa = (struct mb_edit_struct *)a;
	const struct mb_edit_struct *bb = (struct mb_edit_struct *)b;

	/*if (fabs(aa->time_d - bb->time_d) < MB_ESF_MAXTIMEDIFF && aa->time_d != bb->time_d)
	{
	fprintf(stderr,"aa:%.7f bb:%.7f diff:%g\n",aa->time_d,bb->time_d,aa->time_d - bb->time_d);
	}*/
	if (fabs(aa->time_d - bb->time_d) < MB_ESF_MAXTIMEDIFF) {
		if (aa->beam > bb->beam)
			return (1);
		else if (aa->beam < bb->beam)
			return (-1);
		else
			return (0);
	}
	else if (aa->time_d > bb->time_d)
		return (1);
	else
		return (-1);
}
/*--------------------------------------------------------------------*/
/* 	function mb_edit_compare compares mb_edit_struct values. */
int mb_edit_compare_coarse(const void *a, const void *b) {
	const struct mb_edit_struct *aa = (struct mb_edit_struct *)a;
	const struct mb_edit_struct *bb = (struct mb_edit_struct *)b;

	if (fabs(aa->time_d - bb->time_d) < MB_ESF_MAXTIMEDIFF_X10) {
		if (aa->beam > bb->beam)
			return (1);
		else if (aa->beam < bb->beam)
			return (-1);
		else
			return (0);
	}
	else if (aa->time_d > bb->time_d)
		return (1);
	else
		return (-1);
}
/*--------------------------------------------------------------------*/
