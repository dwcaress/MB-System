/*--------------------------------------------------------------------
 *    The MB-system:	mb_get_value.c	2/15/93
 *    $Id: mb_get_value.c,v 4.0 1994-03-06 00:01:56 caress Exp $
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
 * mb_get_value.c includes the "mb_" functions used to get int and double
 * values out of parts of strings - these functions are useful for
 * reading ascii data formats.
 *
 * Author:	D. W. Caress
 * Date:	February 15, 1993
 *
 * $Log: not supported by cvs2svn $
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
#include <math.h>
#include <strings.h>

/* maximum line length in characters */
#define MB_GET_VALUE_MAXLINE 200

static char rcs_id[]="$Id: mb_get_value.c,v 4.0 1994-03-06 00:01:56 caress Exp $";

/*--------------------------------------------------------------------*/
/*	function mb_get_double reads a double value from a string.
 */
int mb_get_double(value,str,nchar)
double *value;
char *str;
int nchar;
{
	char	tmp[MB_GET_VALUE_MAXLINE];
	strncpy(tmp,"\0",sizeof(tmp));
	*value = 0.0;
	*value = atof(strncpy(tmp,str,nchar));
	return(0);
}
/*--------------------------------------------------------------------*/
/*	function mb_get_int reads a int value from a string.
 */
int mb_get_int(value,str,nchar)
int *value;
char *str;
int nchar;
{
	char	tmp[MB_GET_VALUE_MAXLINE];
	strncpy(tmp,"\0",sizeof(tmp));
	*value = atoi(strncpy(tmp,str,nchar));
	return(0);
}
/*--------------------------------------------------------------------*/
