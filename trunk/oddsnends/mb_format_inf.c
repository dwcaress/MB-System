/*--------------------------------------------------------------------
 *    The MB-system:	mb_format_inf.c	3.00	1/21/93
 *    $Id: mb_format_inf.c,v 3.1 1993-05-14 22:34:46 sohara Exp $
 *
 *    Copyright (c) 1993 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mb_format_inf.c returns a short description of the specified 
 * data format.
 *
 * Author:	D. W. Caress
 * Date:	January 21, 1993
 * 
 * $Log: not supported by cvs2svn $
 * Revision 3.0  1993/04/23  15:52:35  dale
 * Initial version
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <strings.h>

/* mbio include files */
#include "../../include/mb_format.h"
#include "../../include/mb_status.h"

/*--------------------------------------------------------------------*/
int mb_format_inf(verbose,format,message)
int	verbose;
int	format;
char	**message;
{
  static char rcs_id[]="$Id: mb_format_inf.c,v 3.1 1993-05-14 22:34:46 sohara Exp $";
	char	*function_name = "mb_format_inf";
	int	status;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose: %d\n",verbose);
		fprintf(stderr,"dbg2       format: %d\n",format);
		}

	/* set the message and status */
	if (format >0 && format <= MB_FORMATS)
		{
		*message = format_description[format];
		status = MB_SUCCESS;
		}
	else
		{
		*message = format_description[0];
		status = MB_FAILURE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       message: %s\n",*message);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
