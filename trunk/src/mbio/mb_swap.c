/*--------------------------------------------------------------------
 *    The MB-system:	mb_swap.c	7/6/94
 *    $Id: mb_swap.c,v 4.5 2000-09-30 06:32:11 caress Exp $
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
 * mb_swap.c contains two functions used to swap bytes for 4 byte float
 * and 8 byte double values.  These are used to deal with the 
 * differences between "big endian" and "little endian" machines.
 * The usual MBIO debug statements have been left out to avoid
 * slowing the execution of these functions. The functions
 * mb_swap_float() and mb_swap_double() do not handle conversions
 * between IEEE values and other representations.
 * Note that the functions take pointers to float or double values
 * as arguments.

 *
 * Author:	D. W. Caress
 * Date:	July 6, 1994
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.4  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.3  1997/09/15  19:06:40  caress
 * Real Version 4.5
 *
 * Revision 4.2  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.1  1994/10/21  12:11:53  caress
 * Release V4.0
 *
 * Revision 4.1  1994/10/21  12:11:53  caress
 * Release V4.0
 *
 * Revision 4.0  1994/07/29  18:58:22  caress
 * Initial Revision
 *
 * Revision 1.1  1994/07/29  18:46:51  caress
 * Initial revision
 *
 * Revision 1.1  1994/07/29  18:46:51  caress
 * Initial revision
 *
 *
 *
 */

/* include for byte swapping on little-endian machines */
#include "../../include/mb_status.h"
#include "../../include/mb_swap.h"

/*--------------------------------------------------------------------*/
/* function mb_swap_float swaps the bytes of an 4 byte float value */
int mb_swap_float(a)
float	*a;
{
	unsigned int	*t;
	t = (unsigned int *) a;
	*t = mb_swap_int(*t);
	
	return(MB_SUCCESS);
}

/*--------------------------------------------------------------------*/
/* function mb_swap_double swaps the bytes of an 8 byte double value */
int mb_swap_double(a)
double	*a;
{
	double b;
	unsigned int *t1;
	unsigned int *t2;

	b = *a;
	t1 = (unsigned int *) &b;
	t2 = (unsigned int *) a;
	t2[1] = mb_swap_int(t1[0]);
	t2[0] = mb_swap_int(t1[1]);
	
	return(MB_SUCCESS);
}
/*--------------------------------------------------------------------*/
