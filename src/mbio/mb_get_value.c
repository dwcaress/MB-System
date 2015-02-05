/*--------------------------------------------------------------------
 *    The MB-system:	mb_get_value.c	2/15/93
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
 * mb_get_value.c includes the "mb_" functions used to get int and double
 * values out of string buffers and the functions used to get
 * values into or out of binary buffers.
 *
 * Author:	D. W. Caress
 * Date:	February 15, 1993
 *
 * $Log: mb_get_value.c,v $
 * Revision 5.5  2008/09/13 06:08:09  caress
 * Updates to apply suggested patches to segy handling. Also fixes to remove compiler warnings.
 *
 * Revision 5.4  2004/11/06 03:55:16  caress
 * Working to support the Reson 7k format.
 *
 * Revision 5.3  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.2  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.1  2001/03/22 20:45:56  caress
 * Trying to make 5.0.beta0...
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.8  2000/10/11  01:02:30  caress
 * Convert to ANSI C
 *
 * Revision 4.7  2000/09/30  06:26:58  caress
 * Snapshot for Dale.
 *
 * Revision 4.6  1999/12/29  00:34:06  caress
 * Release 4.6.8
 *
 * Revision 4.5  1999/08/08  04:12:45  caress
 * Added ELMK2XSE format.
 *
 * Revision 4.4  1998/10/05  18:32:27  caress
 * MB-System version 4.6beta
 *
 * Revision 4.3  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.3  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.3  1997/04/17  15:07:36  caress
 * MB-System 4.5 Beta Release
 *
 * Revision 4.3  1997/04/17  15:07:36  caress
 * MB-System 4.5 Beta Release
 *
 * Revision 4.2  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.1  1994/10/21  12:11:53  caress
 * Release V4.0
 *
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
 * Revision 4.1  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.0  1994/02/21  04:03:53  caress
 * First cut at new version.  No changes.
 *
 * Revision 3.2  1993/05/15  14:44:54  caress
 * removed excess rcs_id message
 *
 * Revision 3.1  1993/05/14  22:27:02  sohara
 * fixed rcs_id message
 *
 * Revision 3.0  1993/04/23  16:00:04  dale
 * Inital version
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_swap.h"
#include "mb_define.h"

/* maximum line length in characters */
#define MB_GET_VALUE_MAXLINE 200

/* static char rcs_id[]="$Id$"; */
char	tmp[MB_GET_VALUE_MAXLINE];

/*--------------------------------------------------------------------*/
/*	function mb_get_double reads a double value from a string.
 */
int mb_get_double(double *value, char *str, int nchar)
{
	memset(tmp, 0, MB_GET_VALUE_MAXLINE);
	*value = 0.0;
	*value = atof(strncpy(tmp,str,nchar));
	return(0);
}
/*--------------------------------------------------------------------*/
/*	function mb_get_int reads a int value from a string.
 */
int mb_get_int(int *value, char *str, int nchar)
{
	memset(tmp, 0, MB_GET_VALUE_MAXLINE);
	*value = atoi(strncpy(tmp,str,nchar));
	return(0);
}
/*--------------------------------------------------------------------*/
/*	function mb_get_binary_short copies a binary short from
 *	a buffer, swapping if necessary
 */
int mb_get_binary_short(int swapped, void *buffer, void *ptr)
{
	short *value;

	value = (short *) ptr;
	memcpy(value, buffer, sizeof(short));
#ifdef BYTESWAPPED
	if (swapped == MB_NO)
	    *value = mb_swap_short(*((short *)value));
#else
	if (swapped == MB_YES)
	    *value = mb_swap_short(*value);
#endif
	return(0);
}
/*--------------------------------------------------------------------*/
/*	function mb_get_binary_int copies a binary int from
 *	a buffer, swapping if necessary
 */
int mb_get_binary_int(int swapped, void *buffer, void *ptr)
{
	int *value;

	value = (int *) ptr;
	memcpy(value, buffer, sizeof(int));
#ifdef BYTESWAPPED
	if (swapped == MB_NO)
	    *value = mb_swap_int(*value);
#else
	if (swapped == MB_YES)
	    *value = mb_swap_int(*value);
#endif
	return(0);
}
/*--------------------------------------------------------------------*/
/*	function mb_get_binary_float copies a binary float from
 *	a buffer, swapping if necessary
 */
int mb_get_binary_float(int swapped, void *buffer, void *ptr)
{
	float *value;

	value = (float *) ptr;
	memcpy(value, buffer, sizeof(float));
#ifdef BYTESWAPPED
	if (swapped == MB_NO)
	    mb_swap_float(value);
#else
	if (swapped == MB_YES)
	    mb_swap_float(value);
#endif
	return(0);
}
/*--------------------------------------------------------------------*/
/*	function mb_get_binary_double copies a binary double from
 *	a buffer, swapping if necessary
 */
int mb_get_binary_double(int swapped, void *buffer, void *ptr)
{
	double *value;

	value = (double *) ptr;
	memcpy(value, buffer, sizeof(double));
#ifdef BYTESWAPPED
	if (swapped == MB_NO)
	    mb_swap_double(value);
#else
	if (swapped == MB_YES)
	    mb_swap_double(value);
#endif
	return(0);
}
/*--------------------------------------------------------------------*/
/*	function mb_get_binary_long copies a binary long from
 *	a buffer, swapping if necessary
 */
int mb_get_binary_long(int swapped, void *buffer, void *ptr)
{
	mb_s_long *value;

	value = (mb_s_long *) ptr;
	memcpy(value, buffer, sizeof(mb_s_long));
#ifdef BYTESWAPPED
	if (swapped == MB_NO)
	    mb_swap_long(value);
#else
	if (swapped == MB_YES)
	    mb_swap_long(value);
#endif
	return(0);
}
/*--------------------------------------------------------------------*/
/*	function mb_put_binary_short copies a binary short to
 *	a buffer, swapping if necessary
 */
int mb_put_binary_short(int swapped, short value, void *buffer)
{
#ifdef BYTESWAPPED
	if (swapped == MB_NO)
	    value = mb_swap_short(value);
#else
	if (swapped == MB_YES)
	    value = mb_swap_short(value);
#endif
	memcpy(buffer, &value, sizeof(short));
	return(0);
}
/*--------------------------------------------------------------------*/
/*	function mb_put_binary_int copies a binary int to
 *	a buffer, swapping if necessary
 */
int mb_put_binary_int(int swapped, int value, void *buffer)
{
#ifdef BYTESWAPPED
	if (swapped == MB_NO)
	    value = mb_swap_int(value);
#else
	if (swapped == MB_YES)
	    value = mb_swap_int(value);
#endif
	memcpy(buffer, &value, sizeof(int));
	return(0);
}
/*--------------------------------------------------------------------*/
/*	function mb_put_binary_float copies a binary float to
 *	a buffer, swapping if necessary
 */
int mb_put_binary_float(int swapped, float value, void *buffer)
{
#ifdef BYTESWAPPED
	if (swapped == MB_NO)
	    mb_swap_float(&value);
#else
	if (swapped == MB_YES)
	    mb_swap_float(&value);
#endif
	memcpy(buffer, &value, sizeof(float));
	return(0);
}
/*--------------------------------------------------------------------*/
/*	function mb_put_binary_double copies a binary double to
 *	a buffer, swapping if necessary
 */
int mb_put_binary_double(int swapped, double value, void *buffer)
{
#ifdef BYTESWAPPED
	if (swapped == MB_NO)
	    mb_swap_double(&value);
#else
	if (swapped == MB_YES)
	    mb_swap_double(&value);
#endif
	memcpy(buffer, &value, sizeof(double));
	return(0);
}
/*--------------------------------------------------------------------*/
/*	function mb_put_binary_long copies a binary long to
 *	a buffer, swapping if necessary
 */
int mb_put_binary_long(int swapped, mb_s_long value, void *buffer)
{
#ifdef BYTESWAPPED
	if (swapped == MB_NO)
	    mb_swap_long(&value);
#else
	if (swapped == MB_YES)
	    mb_swap_long(&value);
#endif
	memcpy(buffer, &value, sizeof(mb_s_long));
	return(0);
}
/*--------------------------------------------------------------------*/
/*	function mb_get_bounds interprets longitude and
 *	latitude values in decimal degrees and degrees:minutes:seconds
 *	form. This code derives from code in GMT (gmt_init.c).
 */
int mb_get_bounds (char *text, double *bounds)
{
	int	status = MB_SUCCESS;
	char	*result;
	int	i;

	result = strtok(text, "/");
	i = 0;
	while (result && i < 4)
	    {
	    bounds[i] = mb_ddmmss_to_degree (result);
	    i++;
	    result = strtok (NULL, "/");
	    }
	if (i == 4)
	    status = MB_SUCCESS;
	else
	    status = MB_FAILURE;

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
/*	function mb_ddmmss_to_degree interprets longitude and
 *	latitude values in decimal degrees and degrees:minutes:seconds
 *	form. This code has been taken from GMT (gmt_init.c).
 */
double mb_ddmmss_to_degree(char *text)
{
	int i, colons = 0, suffix;
	double degree, minute, degfrac, second;

	for (i = 0; text[i]; i++) if (text[i] == ':') colons++;
	suffix = (int)text[i-1];	/* Last character in string */
	if (colons == 2) {	/* dd:mm:ss format */
		sscanf (text, "%lf:%lf:%lf", &degree, &minute, &second);
		degfrac = degree + copysign (minute / 60.0 + second / 3600.0, degree);
	}
	else if (colons == 1) {	/* dd:mm format */
		sscanf (text, "%lf:%lf", &degree, &minute);
		degfrac = degree + copysign (minute / 60.0, degree);
	}
	else
		degfrac = atof (text);
	if (suffix == 'W' || suffix == 'w' || suffix == 'S' || suffix == 's') degfrac = -degfrac;	/* Sign was given implicitly */
	return (degfrac);
}
/*--------------------------------------------------------------------*/
