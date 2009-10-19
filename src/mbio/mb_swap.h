/*--------------------------------------------------------------------
 *    The MB-system:	mb_swap.h	6/21/94
 *    $Id: mb_swap.h,v 5.3 2004/11/06 03:55:15 caress Exp $
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
 * mb_swap.h is an include file with macro definitions 
 * used to swap bytes for 2 and 4 byte integer values to deal with the
 * differences between "big endian" and "little endian" machines.
 * The two macros mb_swap_short and mb_swap_long come courtesy of 
 * Paul Cohen of Sonatech.
 *
 * Author:	D. W. Caress
 * Date:	June 21, 1994
 *
 * $Log: mb_swap.h,v $
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
 * Revision 4.6  2000/09/30  06:29:44  caress
 * Snapshot for Dale.
 *
 * Revision 4.5  1998/12/17  23:01:15  caress
 * MB-System version 4.6beta4
 *
 * Revision 4.4  1998/10/05 17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.3  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.2  1994/10/21  12:11:53  caress
 * Release V4.0
 *
 * Revision 4.2  1994/10/21  12:11:53  caress
 * Release V4.0
 *
 * Revision 4.1  1994/07/29  18:46:51  caress
 * Changes associated with supporting Lynx OS (byte swapped) and
 * using unix second time base (for time_d values).
 *
 * Revision 4.0  1994/06/21  23:22:59  caress
 * First cut.
 *
 *
 *
 */

/*--------------------------------------------------------------------*/

/* include this code only once */
#ifndef MB_SWAP_DEF
#define MB_SWAP_DEF

#define mb_swap_short(a) ( ((a & 0xff) << 8) | ((unsigned short)(a) >> 8) )

#define mb_swap_int(a) ( ((a) << 24) | \
                       (((a) << 8) & 0x00ff0000) | \
                       (((a) >> 8) & 0x0000ff00) | \
                        ((unsigned int)(a) >>24) )

/* end conditional include */
#endif

/*--------------------------------------------------------------------*/
