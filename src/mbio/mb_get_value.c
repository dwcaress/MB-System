/*--------------------------------------------------------------------
 *    The MB-system:	mb_get_value.c	2/15/93
 *
 *    Copyright (c) 1993-2023 by
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
 * mb_get_value.c includes the "mb_" functions used to get int and double
 * values out of string buffers and the functions used to get
 * values into or out of binary buffers.
 *
 * Author:	D. W. Caress
 * Date:	February 15, 1993
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_status.h"
#include "mb_swap.h"

/* maximum line length in characters */
#define MB_GET_VALUE_MAXLINE 200

/*--------------------------------------------------------------------*/
/*	function mb_get_double reads a double value from a string.
 */
int mb_get_double(double *value, char *str, int nchar) {
  char tmp[MB_GET_VALUE_MAXLINE] = "";
	*value = 0.0;
	*value = (double)strtod(strncpy(tmp, str, nchar), NULL);
	return (0);
}
/*--------------------------------------------------------------------*/
/*	function mb_get_int reads a int value from a string.
 */
int mb_get_int(int *value, char *str, int nchar) {
  char tmp[MB_GET_VALUE_MAXLINE] = "";
	*value = (int)strtol(strncpy(tmp, str, nchar), NULL, 10);
	return (0);
}
/*--------------------------------------------------------------------*/
/*	function mb_get_binary_short copies a binary short from
 *	a buffer, swapping if necessary
 */
int mb_get_binary_short(bool swapped, void *buffer, const void *ptr) {
	short *value = (short *)ptr;
	memcpy(value, buffer, sizeof(short));
#ifdef BYTESWAPPED
	if (!swapped)
		*value = mb_swap_short(*((short *)value));
#else
	if (swapped)
		*value = mb_swap_short(*value);
#endif
	return (0);
}
/*--------------------------------------------------------------------*/
/*	function mb_get_binary_int copies a binary int from
 *	a buffer, swapping if necessary
 */
int mb_get_binary_int(bool swapped, void *buffer, const void *ptr) {
	int *value = (int *)ptr;
	memcpy(value, buffer, sizeof(int));
#ifdef BYTESWAPPED
	if (!swapped)
		*value = mb_swap_int(*value);
#else
	if (swapped)
		*value = mb_swap_int(*value);
#endif
	return (0);
}
/*--------------------------------------------------------------------*/
/*	function mb_get_binary_float copies a binary float from
 *	a buffer, swapping if necessary
 */
int mb_get_binary_float(bool swapped, void *buffer, const void *ptr) {
	float *value = (float *)ptr;
	memcpy(value, buffer, sizeof(float));
#ifdef BYTESWAPPED
	if (!swapped)
		mb_swap_float(value);
#else
	if (swapped)
		mb_swap_float(value);
#endif
	return (0);
}
/*--------------------------------------------------------------------*/
/*	function mb_get_binary_double copies a binary double from
 *	a buffer, swapping if necessary
 */
int mb_get_binary_double(bool swapped, void *buffer, const void *ptr) {
	double *value = (double *)ptr;
	memcpy(value, buffer, sizeof(double));
#ifdef BYTESWAPPED
	if (!swapped)
		mb_swap_double(value);
#else
	if (swapped)
		mb_swap_double(value);
#endif
	return (0);
}
/*--------------------------------------------------------------------*/
/*	function mb_get_binary_long copies a binary long from
 *	a buffer, swapping if necessary
 */
int mb_get_binary_long(bool swapped, void *buffer, const void *ptr) {
	mb_s_long *value = (mb_s_long *)ptr;
	memcpy(value, buffer, sizeof(mb_s_long));
#ifdef BYTESWAPPED
	if (!swapped)
		mb_swap_long(value);
#else
	if (swapped)
		mb_swap_long(value);
#endif
	return (0);
}
/*--------------------------------------------------------------------*/
/*	function mb_put_binary_short copies a binary short to
 *	a buffer, swapping if necessary
 */
int mb_put_binary_short(bool swapped, short value, void *buffer) {
#ifdef BYTESWAPPED
	if (!swapped)
		value = mb_swap_short(value);
#else
	if (swapped)
		value = mb_swap_short(value);
#endif
	memcpy(buffer, &value, sizeof(short));
	return (0);
}
/*--------------------------------------------------------------------*/
/*	function mb_put_binary_int copies a binary int to
 *	a buffer, swapping if necessary
 */
int mb_put_binary_int(bool swapped, int value, void *buffer) {
#ifdef BYTESWAPPED
	if (!swapped)
		value = mb_swap_int(value);
#else
	if (swapped)
		value = mb_swap_int(value);
#endif
	memcpy(buffer, &value, sizeof(int));
	return (0);
}
/*--------------------------------------------------------------------*/
/*	function mb_put_binary_float copies a binary float to
 *	a buffer, swapping if necessary
 */
int mb_put_binary_float(bool swapped, float value, void *buffer) {
#ifdef BYTESWAPPED
	if (!swapped)
		mb_swap_float(&value);
#else
	if (swapped)
		mb_swap_float(&value);
#endif
	memcpy(buffer, &value, sizeof(float));
	return (0);
}
/*--------------------------------------------------------------------*/
/*	function mb_put_binary_double copies a binary double to
 *	a buffer, swapping if necessary
 */
int mb_put_binary_double(bool swapped, double value, void *buffer) {
#ifdef BYTESWAPPED
	if (!swapped)
		mb_swap_double(&value);
#else
	if (swapped)
		mb_swap_double(&value);
#endif
	memcpy(buffer, &value, sizeof(double));
	return (0);
}
/*--------------------------------------------------------------------*/
/*	function mb_put_binary_long copies a binary long to
 *	a buffer, swapping if necessary
 */
int mb_put_binary_long(bool swapped, mb_s_long value, void *buffer) {
#ifdef BYTESWAPPED
	if (!swapped)
		mb_swap_long(&value);
#else
	if (swapped)
		mb_swap_long(&value);
#endif
	memcpy(buffer, &value, sizeof(mb_s_long));
	return (0);
}
/*--------------------------------------------------------------------*/
/*	function mb_get_bounds interprets longitude and
 *	latitude values in decimal degrees and degrees:minutes:seconds
 *	form. This code derives from code in GMT (gmt_init.c).
 */
int mb_get_bounds(char *text, double *bounds) {
  char *saveptr;
	char *result = strtok_r(text, "/", &saveptr);
	int i = 0;
	while (result && i < 4) {
		bounds[i] = mb_ddmmss_to_degree(result);
		i++;
		result = strtok_r(NULL, "/", &saveptr);
	}
	int status = MB_SUCCESS;
	if (i == 4)
		status = MB_SUCCESS;
	else
		status = MB_FAILURE;

	return (status);
}
/*--------------------------------------------------------------------*/
/*	function mb_ddmmss_to_degree interprets longitude and
 *	latitude values in decimal degrees and degrees:minutes:seconds
 *	form. This code has been taken from GMT (gmt_init.c).
 */
double mb_ddmmss_to_degree(const char *text) {
	int colons = 0;
	int i = 0;  /* Used after for. */
	for (; text[i]; i++)
		if (text[i] == ':')
			colons++;

	const int suffix = (int)text[i - 1]; /* Last character in string */
	double degfrac;

	if (colons == 2) {         /* dd:mm:ss format */
		double second;
		double degree;
		double minute;
		sscanf(text, "%lf:%lf:%lf", &degree, &minute, &second);
		degfrac = degree + copysign(minute / 60.0 + second / 3600.0, degree);
	}
	else if (colons == 1) { /* dd:mm format */
		double degree;
		double minute;
		sscanf(text, "%lf:%lf", &degree, &minute);
		degfrac = degree + copysign(minute / 60.0, degree);
	}
	else
		degfrac = strtod(text, NULL);

	if (suffix == 'W' || suffix == 'w' || suffix == 'S' || suffix == 's')
		degfrac = -degfrac; /* Sign was given implicitly */

	return (degfrac);
}
/*--------------------------------------------------------------------*/
