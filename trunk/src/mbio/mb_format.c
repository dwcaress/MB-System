/*--------------------------------------------------------------------
 *    The MB-system:	mb_format.c	2/18/94
 *    $Id: mb_format.c,v 4.7 2000-01-25 01:45:10 caress Exp $
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
 * mb_format.c contains two functions.
 * mb_format() finds the internal format array location associated 
 *     with an MBIO format id.  If the format id is invalid, 
 *     0 is returned.
 * mb_get_format() guesses the format id of a filename
 *     based on the file suffix.
 *
 * Author:	D. W. Caress
 * Date:	Februrary 18, 1994
 * 
 * $Log: not supported by cvs2svn $
 * Revision 4.6  2000/01/20  00:09:23  caress
 * Added function mb_get_format().
 *
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
 * Revision 4.1  1994/04/22  17:49:13  caress
 * Removed output messages which went to stdout, messing up
 * Postscript output from mbswath and mbcontour.
 *
 * Revision 4.1  1994/04/22  17:49:13  caress
 * Removed output messages which went to stdout, messing up
 * Postscript output from mbswath and mbcontour.
 *
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
#include <string.h>

/* mbio include files */
#include "../../include/mb_format.h"
#include "../../include/mb_status.h"

static char rcs_id[]="$Id: mb_format.c,v 4.7 2000-01-25 01:45:10 caress Exp $";

/*--------------------------------------------------------------------*/
int mb_format(verbose,format,format_num,error)
int	verbose;
int	*format;
int	*format_num;
int	*error;
{
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

	/* check to be sure format is actually supported */
	if (supported_format_table[*format_num] == MB_NO)
		*format_num = 0;

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
int mb_get_format(verbose,filename,fileroot,format,error)
int	verbose;
char	*filename;
char	*fileroot;
int	*format;
int	*error;
{
	char	*function_name = "mb_get_format";
	int	status = MB_SUCCESS;
	int	found = MB_NO;
	char	*suffix;
	int	suffix_len;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:   %d\n",verbose);
		fprintf(stderr,"dbg2       filename:  %s\n",filename);
		}
		
	/* set format not found */
	found = MB_NO;
	*format = 0;
	
	/* first look for MB suffix convention */
	if (strlen(filename) > 6)
	    i = strlen(filename) - 6;
	else
	    i = 0;
	if ((suffix = strstr(&filename[i],".mb")) != NULL)
	    {
	    suffix_len = strlen(suffix);
	    if (suffix_len >= 4 && suffix_len <= 6)
		{
		strncpy(fileroot, filename, strlen(filename)-suffix_len);
		fileroot[strlen(filename)-suffix_len] = '\0';
		sscanf(suffix, ".mb%d", format);
		found = MB_YES;
		}
	    }

	/* look for SeaBeam suffix convention */
	if (found == MB_NO)
	    {
	    if (strlen(filename) > 4)
		i = strlen(filename) - 4;
	    else
		i = 0;
	    if ((suffix = strstr(&filename[i],".rec")) != NULL)
		{
		suffix_len = strlen(suffix);
		if (suffix_len == 4)
		    {
		    strncpy(fileroot, filename, strlen(filename)-suffix_len);
		    fileroot[strlen(filename)-suffix_len] = '\0';
		    *format = 41;
		    found = MB_YES;
		    }
		}
	    }

	/* check for old format id and provide alias if needed */
	if (found == MB_YES && *format > 0 && *format < 10)
	    {
	    /* find current format value */
	    i = format_alias_table[*format];

	    /* print output debug statements */
	    if (verbose >= 0)
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
		
	/* set error if needed */
	if (found == MB_NO)
	    {
	    *error = MB_ERROR_BAD_FORMAT;
	    status = MB_FAILURE;
	    *format = 0;
	    strcpy(fileroot, filename);
	    }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       fileroot:   %s\n",fileroot);
		fprintf(stderr,"dbg2       format:     %d\n",*format);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
