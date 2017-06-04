/*--------------------------------------------------------------------
 *    The MB-system:	mb_swap.h	6/21/94
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
 * mb_swap.h is an include file with macro definitions
 * used to swap bytes for 2 and 4 byte integer values to deal with the
 * differences between "big endian" and "little endian" machines.
 * The two macros mb_swap_short and mb_swap_long come courtesy of
 * Paul Cohen of Sonatech.
 *
 * Author:	D. W. Caress
 * Date:	June 21, 1994
 *
 *
 *
 */

/*--------------------------------------------------------------------*/

/* include this code only once */
#ifndef MB_SWAP_DEF
#define MB_SWAP_DEF

#define mb_swap_short(a) (((a & 0xff) << 8) | ((unsigned short)(a) >> 8))

#define mb_swap_int(a) (((a) << 24) | (((a) << 8) & 0x00ff0000) | (((a) >> 8) & 0x0000ff00) | ((unsigned int)(a) >> 24))

/* end conditional include */
#endif

/*--------------------------------------------------------------------*/
