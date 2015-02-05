/*--------------------------------------------------------------------
 *    The MB-system:	mb_compare.c	11/19/98
 *    $Id$
 *
 *    Copyright (c) 1993-2015 by
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
 * mb_compare.c includes the "mb_" functions for comparing values
 * which are passed to the qsort routine.
 *
 * Author:	D. W. Caress
 * Date:	November 19,  1998
 *
 *
 */
#include <stdio.h>
#include <math.h>

/* mbio include files */
#include "mb_define.h"
#include "mb_process.h"

/* static char rcs_id[]="$Id: mb_compare.c,v 5.4 2003/07/26 17:59:32 caress Exp
$"; */

/*--------------------------------------------------------------------*/
/* 	function mb_int_compare compares int values. */
int mb_int_compare(const void *a, const void *b)
{
	int	*aa, *bb;

	aa = (int *) a;
	bb = (int *) b;

	if (*aa > *bb)
		return(1);
	else
		return(-1);
}
/*--------------------------------------------------------------------*/
/* 	function mb_double_compare compares double values. */
int mb_double_compare(const void *a, const void *b)
{
	double	*aa, *bb;

	aa = (double *) a;
	bb = (double *) b;

	if (*aa > *bb)
		return(1);
	else
		return(-1);
}
/*--------------------------------------------------------------------*/
/* 	function mb_edit_compare compares mb_edit_struct values. */
int mb_edit_compare(const void *a, const void *b)
{
	struct mb_edit_struct	*aa, *bb;

	aa = (struct mb_edit_struct *) a;
	bb = (struct mb_edit_struct *) b;

/*if (fabs(aa->time_d - bb->time_d) < MB_ESF_MAXTIMEDIFF && aa->time_d != bb->time_d)
{
fprintf(stderr,"aa:%.7f bb:%.7f diff:%g\n",aa->time_d,bb->time_d,aa->time_d - bb->time_d);
}*/
	if (fabs(aa->time_d - bb->time_d) < MB_ESF_MAXTIMEDIFF)
		{
		if (aa->beam > bb->beam)
			return(1);
		else if (aa->beam < bb->beam)
			return(-1);
		else
			return(0);
		}
	else if (aa->time_d > bb->time_d)
		return(1);
	else
		return(-1);
}
/*--------------------------------------------------------------------*/
