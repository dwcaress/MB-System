/*--------------------------------------------------------------------
 *    The MB-system:	mbformat.c	1/22/93
 *    $Id: mbformat.c,v 4.7 1997-04-21 17:19:14 caress Exp $
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
 * MBFORMAT provides a description of the multibeam data format
 * associated with a particular MBIO format identifier.  If
 * no format is specified, MBFORMAT will list descriptions of all
 * the currently supported formats.
 *
 * Author:	D. W. Caress
 * Date:	January 22, 1993
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.6  1996/04/22  13:23:05  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.6  1996/04/22  13:23:05  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.5  1995/05/12  17:12:32  caress
 * Made exit status values consistent with Unix convention.
 * 0: ok  nonzero: error
 *
 * Revision 4.4  1995/04/12  16:25:54  caress
 * Added -A and -L options.
 *
 * Revision 4.3  1995/03/06  19:37:59  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.2  1994/10/21  13:02:31  caress
 * Release V4.0
 *
 * Revision 4.1  1994/03/12  01:44:37  caress
 * Added declarations of ctime and/or getenv for compatability
 * with SGI compilers.
 *
 * Revision 4.0  1994/03/06  00:13:22  caress
 * First cut at version 4.0
 *
 * Revision 4.0  1994/03/01  18:59:27  caress
 * First cut at new version. Any changes are associated with
 * support of three data types (beam bathymetry, beam amplitude,
 * and sidescan) instead of two (bathymetry and backscatter).
 *
 * Revision 3.0  1993/05/04  22:38:45  dale
 * Inital version.
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "../../include/mb_format.h"
#include "../../include/mb_status.h"
#include "../../include/mb_define.h"

/*--------------------------------------------------------------------*/

main (argc, argv)
int argc;
char **argv; 
{
	/* id variables */
	static char rcs_id[] = "$Id: mbformat.c,v 4.7 1997-04-21 17:19:14 caress Exp $";
	static char program_name[] = "MBFORMAT";
	static char help_message[] = "MBFORMAT is an utility which identifies the multibeam data formats \nassociated with MBIO format id's.  If no format id is specified, \nMBFORMAT lists all of the currently supported formats.";
	static char usage_message[] = "mbformat [-A -Fformat -L -V -H]";

	/* parsing variables */
	extern char *optarg;
	extern int optkind;
	int	errflg = 0;
	int	c;
	int	error = MB_ERROR_NO_ERROR;
	int	status;
	int	help;
	int	verbose;
	int	format;
	int	format_num;
	char	*message;
	int	list_all;
	int	list_simple;
	int	i;

	char	*getenv();

	help = 0;
	verbose = 0;
	format = 0;
	list_all = MB_NO;
	list_simple = MB_NO;

	/* process argument list */
	while ((c = getopt(argc, argv, "AaF:f:HhLlVv")) != -1)
	  switch (c) 
		{
		case 'A':
		case 'a':
			list_all = MB_YES;
			break;
		case 'F':
		case 'f':
			sscanf (optarg,"%d", &format);
			break;
		case 'L':
		case 'l':
			list_simple = MB_YES;
			break;
		case 'H':
		case 'h':
			help++;
			break;
		case 'V':
		case 'v':
			verbose++;
			break;
		case '?':
			errflg++;
		}

	/* if error flagged then print it and exit */
	if (errflg)
		{
		fprintf(stderr,"usage: %s\n", usage_message);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
		}

	/* print starting message */
	if (verbose == 1)
		{
		fprintf(stderr,"\nProgram %s\n",program_name);
		fprintf(stderr,"Version %s\n",rcs_id);
		fprintf(stderr,"MB-system Version %s\n",MB_VERSION);
		}

	/* print starting debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s>\n",program_name);
		fprintf(stderr,"dbg2  Version %s\n",rcs_id);
		fprintf(stderr,"dbg2  MB-system Version %s\n",MB_VERSION);
		fprintf(stderr,"dbg2  Control Parameters:\n");
		fprintf(stderr,"dbg2       verbose: %d\n",verbose);
		fprintf(stderr,"dbg2       help:    %d\n",help);
		fprintf(stderr,"dbg2       format:  %d\n",format);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}

	/* print out the info */
	if (format != 0)
		{
		status = mb_format(verbose,&format,&format_num,&error);
		status = mb_format_inf(verbose,format_num,&message);
		printf("\nMBIO data format id: %d\n",format);
		printf("%s",message);
		}
	else if (list_simple == MB_NO)
		{
		printf("\nSupported MBIO Formats:\n");
		for (i=1;i<=MB_FORMATS;i++)
			{
			if (supported_format_table[i] == MB_YES)
				{
				status = mb_format_inf(verbose,i,&message);
				printf("\nMBIO Data Format ID:  %d\n",
					format_table[i]);
				printf("%s",message);
				}
			}
		if (list_all == MB_YES)
			{
			printf("\n\nUnsupported MBIO Formats which will be supported someday:\n");
			for (i=1;i<=MB_FORMATS;i++)
				{
				if (supported_format_table[i] == MB_NO)
					{
					status = mb_format_inf(verbose,i,&message);
					printf("\nMBIO Data Format ID:  %d\n",
						format_table[i]);
					printf("%s",message);
					}
				}
			}
		}
	else 
		{
		for (i=1;i<=MB_FORMATS;i++)
			{
			if (supported_format_table[i] == MB_YES
				|| list_all == MB_YES)
				{
				printf("%d\n",format_table[i]);
				}
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  Program <%s> completed\n",
			program_name);
		fprintf(stderr,"dbg2  Ending status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* end it all */
	exit(error);
}
