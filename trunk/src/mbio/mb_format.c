/*--------------------------------------------------------------------
 *    The MB-system:	mb_format.c	2/18/94
 *    $Id: mb_format.c,v 4.1 1994-04-22 17:49:13 caress Exp $
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
 * mb_format.c finds the internal format array location associated 
 * with an MBIO format id.  If the format id is invalid, 0 is returned.
 *
 * Author:	D. W. Caress
 * Date:	Februrary 18, 1994
 * 
 * $Log: not supported by cvs2svn $
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
 * Revision 4.5  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.4  1994/02/21  21:35:02  caress
 * Set format alias message to go to stdout if verbose == 1
 * and stderr if verbose > 1.
 *
 * Revision 4.3  1994/02/21  20:54:16  caress
 * Added verbose message for format alias events.
 *
 * Revision 4.2  1994/02/21  19:45:26  caress
 * Made it actually work and added aliasing of old format
 * id's to new format id's.
 *
 * Revision 4.1  1994/02/21  04:55:25  caress
 * Fixed some simple errors.
 *
 * Revision 4.0  1994/02/21  04:52:01  caress
 * Initial version.
 *
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
int mb_format(verbose,format,format_num,error)
int	verbose;
int	*format;
int	*format_num;
int	*error;
{
  static char rcs_id[]="$Id: mb_format.c,v 4.1 1994-04-22 17:49:13 caress Exp $";
	char	*function_name = "mb_format";
	int	status;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:   %d\n",verbose);
		fprintf(stderr,"dbg2       format:    %d\n",*format);
		}

	/* check for old format id and provide alias if needed */
	if (*format > 0 && *format < 10)
		{
		/* find current format value */
		i = format_alias_table[*format];

		/* print output debug statements */
		if (verbose >= 2)
			{
			fprintf(stderr,"\ndbg2  Old format id aliased to current value in MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg2  Old format value:\n");
			fprintf(stderr,"dbg2       format:     %d\n",*format);
			fprintf(stderr,"dbg2  Current format value:\n");
			fprintf(stderr,"dbg2       format:     %d\n",i);
			}

		/* set new format value */
		*format = i;
		}

	/* look for the corresponding format_num */
	*format_num = 0;
	for (i=1;i<=MB_FORMATS;i++)
		if (format_table[i] == *format)
			*format_num = i;

	/* set flags */
	if (*format_num > 0)
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_FORMAT;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       format:     %d\n",*format);
		fprintf(stderr,"dbg2       format_num: %d\n",*format_num);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
