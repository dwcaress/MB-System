/*--------------------------------------------------------------------
 *    The MB-system:	mb_format_inf.c	1/21/93
 *    $Id: mb_format_inf.c,v 4.3 1997-04-21 17:02:07 caress Exp $
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
 * mb_format_inf.c returns a short description of the specified 
 * data format.
 *
 * Author:	D. W. Caress
 * Date:	January 21, 1993
 * 
 * $Log: not supported by cvs2svn $
 * Revision 4.2  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
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
 * Revision 4.2  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.1  1994/03/03  03:15:16  caress
 * Not sure what was changed.
 *
 * Revision 4.0  1994/02/21  04:03:53  caress
 * First cut at new version.  No changes.
 *
 * Revision 3.1  1993/05/14  22:34:46  sohara
 * fixed rcs_id message
 *
 * Revision 3.0  1993/04/23  15:52:35  dale
 * Initial version
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "../../include/mb_format.h"
#include "../../include/mb_status.h"

/*--------------------------------------------------------------------*/
int mb_format_inf(verbose,format_num,message)
int	verbose;
int	format_num;
char	**message;
{
  static char rcs_id[]="$Id: mb_format_inf.c,v 4.3 1997-04-21 17:02:07 caress Exp $";
	char	*function_name = "mb_format_inf";
	int	status;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       format_num: %d\n",format_num);
		}

	/* set the message and status */
	if (format_num >0 && format_num <= MB_FORMATS)
		{
		*message = format_description[format_num];
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
