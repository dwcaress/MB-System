/*--------------------------------------------------------------------
 *    The MB-system:	mb_swap.h	6/21/94
 *    $Id: mb_swap.h,v 4.1 1994-07-29 18:46:51 caress Exp $
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
 * mb_swap.h is an include file with macro definitions 
 * used to swap bytes for 2 and 4 byte integer values to deal with the
 * differences between "big endian" and "little endian" machines.
 * The two macros mb_swap_short and mb_swap_long come courtesy of 
 * Paul Cohen of Sonatech.
 *
 * Author:	D. W. Caress
 * Date:	June 21, 1994
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.0  1994/06/21  23:22:59  caress
 * First cut.
 *
 *
 *
 */

/*--------------------------------------------------------------------*/

#define mb_swap_short(a) ( ((a & 0xff) << 8) | ((unsigned short)(a) >> 8) )

#define mb_swap_long(a) ( ((a) << 24) | \
                       (((a) << 8) & 0x00ff0000) | \
                       (((a) >> 8) & 0x0000ff00) | \
                        ((unsigned long)(a) >>24) )

/* this will work someday when we have 8 byte "longer int" values */
/*#define mb_swap_longer(a) ( ((a) << 56) | \
                       (((a) << 40) & 0x00ff000000000000) | \
                       (((a) << 24) & 0x0000ff0000000000) | \
                       (((a) <<  8) & 0x000000ff00000000) | \
                       (((a) >>  8) & 0x00000000ff000000) | \
                       (((a) >> 24) & 0x0000000000ff0000) | \
                       (((a) >> 40) & 0x000000000000ff00) | \
                        ((unsigned longer)(a) >> 56)) 
*/

/*--------------------------------------------------------------------*/
