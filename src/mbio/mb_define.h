/*--------------------------------------------------------------------
 *    The MB-system:	mb_io.h	4/21/96
 *    $Id: mb_define.h,v 4.4 2000-09-30 06:29:44 caress Exp $
 *
 *    Copyright (c) 1996, 2000 by
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
 * mb_define.h defines macros used by MB-System programs and functions
 * for degree/radian conversions and min/max calculations.
 *
 * Author:	D. W. Caress
 * Date:	April 21, 1996
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.3  1998/12/17  23:01:15  caress
 * MB-System version 4.6beta4
 *
 * Revision 4.2  1998/10/05 17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.1  1997/09/15  19:06:40  caress
 * Real Version 4.5
 *
 * Revision 4.0  1996/08/05  15:24:55  caress
 * Initial revision.
 *
 *
 */
 
/* include this code only once */
#ifndef MB_DEFINE_DEF
#define MB_DEFINE_DEF
 
/* type definitions of signed and unsigned char */
typedef unsigned char	mb_u_char;
#ifdef IRIX
typedef signed char	mb_s_char;
#endif
#ifdef IRIX64
typedef signed char	mb_s_char;
#endif
#ifdef SOLARIS
typedef signed char	mb_s_char;
#endif
#ifdef LINUX
typedef signed char	mb_s_char;
#endif
#ifdef LYNX
typedef signed char	mb_s_char;
#endif
#ifdef SUN
typedef char	mb_s_char;
#endif
#ifdef HPUX
typedef signed char	mb_s_char;
#endif
#ifdef OTHER
typedef signed char	mb_s_char;
#endif

/* declare PI if needed */
#ifndef M_PI
#define	M_PI	3.14159265358979323846
#endif

/* multiply this by degrees to get radians */
#define DTR	0.01745329251994329500

/* multiply this by radians to get degrees */
#define RTD	57.2957795130823230000

/* min max define */
#define	MIN(A, B)	((A) < (B) ? (A) : (B))
#define	MAX(A, B)	((A) > (B) ? (A) : (B))

/* end conditional include */
#endif
