/*--------------------------------------------------------------------
 *    The MB-system:	mb_compare.c	11/19/98
 *    $Id: mb_compare.c,v 5.0 2000-12-01 22:48:41 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 2000 by
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
 * $Log: not supported by cvs2svn $
 * Revision 4.2  2000/10/11  01:02:30  caress
 * Convert to ANSI C
 *
 * Revision 4.1  2000/09/30  06:26:58  caress
 * Snapshot for Dale.
 *
 * Revision 4.0  1998/12/17  22:57:26  caress
 * MB-System version 4.6beta4
 *
 *
 *
 */

/* mbio include files */
#include "../../include/mb_define.h"

static char rcs_id[]="$Id: mb_compare.c,v 5.0 2000-12-01 22:48:41 caress Exp $";

/*--------------------------------------------------------------------*/
/* 	function mb_int_compare compares int values. */
int mb_int_compare(int *a, int *b)
{
	if (*a > *b)
		return(1);
	else
		return(-1);
}
/*--------------------------------------------------------------------*/
/* 	function mb_double_compare compares double values. */
int mb_double_compare(double *a, double *b)
{
	if (*a > *b)
		return(1);
	else
		return(-1);
}
/*--------------------------------------------------------------------*/
