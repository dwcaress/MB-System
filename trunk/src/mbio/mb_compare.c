/*--------------------------------------------------------------------
 *    The MB-system:	mb_compare.c	11/19/98
 *    $Id: mb_compare.c,v 4.0 1998-12-17 22:57:26 caress Exp $
 *
 *    Copyright (c) 1993, 1994 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
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
 *
 *
 */

static char rcs_id[]="$Id: mb_compare.c,v 4.0 1998-12-17 22:57:26 caress Exp $";

/*--------------------------------------------------------------------*/
/* 	function mb_int_compare compares int values. */
int mb_int_compare(a,b)
int	*a;
int	*b;
{
	if (*a > *b)
		return(1);
	else
		return(-1);
}
/*--------------------------------------------------------------------*/
/* 	function mb_double_compare compares double values. */
int mb_double_compare(a,b)
double	*a;
double	*b;
{
	if (*a > *b)
		return(1);
	else
		return(-1);
}
/*--------------------------------------------------------------------*/
