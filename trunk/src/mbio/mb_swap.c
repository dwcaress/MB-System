/*--------------------------------------------------------------------
 *    The MB-system:	mb_swap.c	7/6/94
 *    $Id: mb_swap.c,v 5.7 2008/08/12 05:31:54 caress Exp $
 *
 *    Copyright (c) 1993-2009 by
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
 * $Log: mb_swap.c,v $
 * Revision 5.7  2008/08/12 05:31:54  caress
 * Fixed swapping of 8 byte values. Fix suggested by Jeremy Robst of NERC.
 *
 * Revision 5.6  2008/05/16 22:56:24  caress
 * Release 5.1.1beta18.
 *
 * Revision 5.5  2006/02/12 04:28:09  caress
 * For 5.0.9.
 *
 * Revision 5.4  2006/01/06 18:27:18  caress
 * Working towards 5.0.8
 *
 * Revision 5.3  2004/11/06 03:55:15  caress
 * Working to support the Reson 7k format.
 *
 * Revision 5.2  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.1  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.0  2000/12/01 22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.6  2000/10/11  01:02:30  caress
 * Convert to ANSI C
 *
 * Revision 4.5  2000/09/30  06:32:11  caress
 * Snapshot for Dale.
 *
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

/* standard include files */
#include <stdio.h>
#include <stdlib.h>

/* include for byte swapping on little-endian machines */
#include "../../include/mb_status.h"
#include "../../include/mb_swap.h"
#include "../../include/mb_define.h"

/* static	char	rcs_id[]="$Id: $"; */

/*--------------------------------------------------------------------*/
/* function mb_swap_check determines if the cpu is byteswapped */
int mb_swap_check()
{
	unsigned short testshort = 255;
	char		*testchar;
	int	byteswapped;
	testchar = (char *) &testshort;
	if (testchar[0] == 0)
		byteswapped = MB_NO;
	else
		byteswapped = MB_YES;
	
	return(byteswapped);
}

/*--------------------------------------------------------------------*/
/* function mb_swap_float swaps the bytes of an 4 byte float value */
int mb_swap_float(float *a)
{
	unsigned int	*t;
	t = (unsigned int *) a;
	*t = mb_swap_int(*t);
	
	return(MB_SUCCESS);
}

/*--------------------------------------------------------------------*/
/* function mb_swap_double swaps the bytes of an 8 byte double value */
int mb_swap_double(double *a)
{
	mb_u_char bc[8];
	mb_u_char *ac;
	
	ac = (mb_u_char *) a;
	bc[0] = ac[7];
	bc[1] = ac[6];
	bc[2] = ac[5];
	bc[3] = ac[4];
	bc[4] = ac[3];
	bc[5] = ac[2];
	bc[6] = ac[1];
	bc[7] = ac[0];
	ac[0] = bc[0];
	ac[1] = bc[1];
	ac[2] = bc[2];
	ac[3] = bc[3];
	ac[4] = bc[4];
	ac[5] = bc[5];
	ac[6] = bc[6];
	ac[7] = bc[7];
	
	return(MB_SUCCESS);
}
/*--------------------------------------------------------------------*/
/* function mb_swap_long swaps the bytes of an 8 byte long value */
int mb_swap_long(mb_s_long *a)
{
	mb_u_char bc[8];
	mb_u_char *ac;
	
	ac = (mb_u_char *) a;
	bc[0] = ac[7];
	bc[1] = ac[6];
	bc[2] = ac[5];
	bc[3] = ac[4];
	bc[4] = ac[3];
	bc[5] = ac[2];
	bc[6] = ac[1];
	bc[7] = ac[0];
	ac[0] = bc[0];
	ac[1] = bc[1];
	ac[2] = bc[2];
	ac[3] = bc[3];
	ac[4] = bc[4];
	ac[5] = bc[5];
	ac[6] = bc[6];
	ac[7] = bc[7];
	
	return(MB_SUCCESS);
}
/*--------------------------------------------------------------------*/
