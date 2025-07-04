/*--------------------------------------------------------------------
 *    The MB-system:	mb_swap.c	7/6/94
 *
 *    Copyright (c) 1993-2025 by
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
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "mb_define.h"
#include "mb_status.h"
#include "mb_swap.h"

/*--------------------------------------------------------------------*/
/* function mb_swap_check determines if the cpu is byteswapped */
int mb_swap_check() {
	unsigned short testshort = 255;
	char *testchar;
	int byteswapped;
	testchar = (char *)&testshort;
	if (testchar[0] == 0)
		byteswapped = false;
	else
		byteswapped = true;

	return (byteswapped);
}

/*--------------------------------------------------------------------*/
/* function mb_swap_float swaps the bytes of an 4 byte float value */
int mb_swap_float(float *a) {
	unsigned int *t;
	t = (unsigned int *)a;
	*t = mb_swap_int(*t);

	return (MB_SUCCESS);
}

/*--------------------------------------------------------------------*/
/* function mb_swap_double swaps the bytes of an 8 byte double value */
int mb_swap_double(double *a) {
	mb_u_char bc[8];
	mb_u_char *ac;

	ac = (mb_u_char *)a;
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

	return (MB_SUCCESS);
}
/*--------------------------------------------------------------------*/
/* function mb_swap_long swaps the bytes of an 8 byte long value */
int mb_swap_long(mb_s_long *a) {
	mb_u_char bc[8];
	mb_u_char *ac;

	ac = (mb_u_char *)a;
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

	return (MB_SUCCESS);
}
/*--------------------------------------------------------------------*/
