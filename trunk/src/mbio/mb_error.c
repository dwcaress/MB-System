/*--------------------------------------------------------------------
 *    The MB-system:	mb_error.c	2/2/93
 *    $Id: mb_error.c,v 4.6 2000-09-30 06:26:58 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 2000 by
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
 * mb_error.c returns a short error message associated with the
 * input error code.
 *
 * Author:	D. W. Caress
 * Date:	February 2, 1993
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.5  1998/10/05  18:32:27  caress
 * MB-System version 4.6beta
 *
 * Revision 4.4  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.4  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.3  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.3  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.2  1994/10/21  12:11:53  caress
 * Release V4.0
 *
 * Revision 4.1  1994/07/29  18:46:51  caress
 * Changes associated with supporting Lynx OS (byte swapped) and
 * using unix second time base (for time_d values).
 *
 * Revision 4.0  1994/03/05  23:55:38  caress
 * First cut at version 4.0
 *
 * Revision 4.1  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.0  1994/02/21  04:03:10  caress
 * First cut at new version.  No changes.
 *
 * Revision 3.1  1993/05/14  22:33:34  sohara
 * fixed rcs_id message
 *
 * Revision 3.1  1993/05/14  22:33:34  sohara
 * fixed rcs_id message
 *
 * Revision 3.0  1993/04/23  15:48:29  dale
 * Initial version
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "../../include/mb_status.h"

/*--------------------------------------------------------------------*/
int mb_error(verbose,error,message)
int	verbose;
int	error;
char	**message;
{
  static char rcs_id[]="$Id: mb_error.c,v 4.6 2000-09-30 06:26:58 caress Exp $";
	char	*function_name = "mb_error";
	int	status;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose: %d\n",verbose);
		fprintf(stderr,"dbg2       error:   %d\n",error);
		fprintf(stderr,"dbg2       message: %d\n",message);
		}

	/* set the message and status */
	if (error < MB_ERROR_MIN || error > MB_ERROR_MAX)
		{
		*message = unknown_error_msg[0];
		status = MB_FAILURE;
		}
	else if (error > MB_ERROR_NO_ERROR)
		{
		*message = fatal_error_msg[error];
		status = MB_SUCCESS;
		}
	else
		{
		*message = nonfatal_error_msg[-error];
		status = MB_SUCCESS;
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
