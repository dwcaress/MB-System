/* Added HAVE_CONFIG_H for autogen files */
#ifdef HAVE_CONFIG_H
#  include <mbsystem_config.h>
#endif


/*--------------------------------------------------------------------
 *    The MB-system:	mbformat.c	1/22/93
 *    $Id: mbformat.c 1917 2012-01-10 19:25:33Z caress $
 *
 *    Copyright (c) 1993-2012 by
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
 * MBFORMAT provides a description of the swath data format
 * associated with a particular MBIO format identifier.  If
 * no format is specified, MBFORMAT will list descriptions of all
 * the currently supported formats.
 *
 * Author:	D. W. Caress
 * Date:	January 22, 1993
 *
 * $Log: mbformat.c,v $
 * Revision 5.9  2008/10/17 07:52:44  caress
 * Check in on October 17, 2008.
 *
 * Revision 5.8  2006/01/18 15:17:00  caress
 * Added stdlib.h include.
 *
 * Revision 5.7  2003/04/17 21:17:10  caress
 * Release 5.0.beta30
 *
 * Revision 5.6  2003/02/27 04:43:23  caress
 * Added -K option for output of fileroots.
 *
 * Revision 5.5  2002/10/02 23:56:06  caress
 * Release 5.0.beta24
 *
 * Revision 5.4  2001/07/21 22:04:10  caress
 * Made the -i and -f options work better.
 *
 * Revision 5.3  2001/07/20 17:05:25  caress
 * Set mbformat to output accurate format update date with
 * -W option (html output).
 *
 * Revision 5.2  2001/04/25  05:39:43  caress
 * Fixed -W option so html page looks right (white background).
 *
 * Revision 5.1  2001/03/22 21:14:16  caress
 * Trying to make release 5.0.beta0.
 *
 * Revision 5.0  2000/12/01  22:57:08  caress
 * First cut at Version 5.0.
 *
 * Revision 4.10  2000/10/11  01:06:15  caress
 * Convert to ANSI C
 *
 * Revision 4.9  2000/09/30  07:06:28  caress
 * Snapshot for Dale.
 *
 * Revision 4.8  1998/10/05  19:19:24  caress
 * MB-System version 4.6beta
 *
 * Revision 4.7  1997/04/21  17:19:14  caress
 * MB-System 4.5 Beta Release.
 *
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
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "mb_format.h"
#include "mb_status.h"
#include "mb_define.h"

/* local defines */
#define MBFORMAT_LIST_LONG	0
#define MBFORMAT_LIST_SIMPLE	1
#define MBFORMAT_LIST_ROOT	2

static char rcs_id[] = "$Id: mbformat.c 1917 2012-01-10 19:25:33Z caress $";

/*--------------------------------------------------------------------*/

int main (int argc, char **argv)
{
	/* id variables */
	char program_name[] = "MBFORMAT";
	char help_message[] = "MBFORMAT is an utility which identifies the swath data formats \nassociated with MBIO format id's.  If no format id is specified, \nMBFORMAT lists all of the currently supported formats.";
	char usage_message[] = "mbformat [-Fformat -Ifile -L -W -V -H]";

	/* parsing variables */
	extern char *optarg;
	int	errflg = 0;
	int	c;
	int	error = MB_ERROR_NO_ERROR;
	int	status;
	int	help;
	int	html;
	int	verbose;
	char	file[MB_PATH_MAXLINE];
	char	root[MB_PATH_MAXLINE];
	int	file_specified;
	int	format;
	int	format_save;
	int	format_specified;
	char	format_description[MB_DESCRIPTION_LENGTH];
	char	*format_informal_ptr;
	char	*format_attributes_ptr;
	char	format_name[MB_DESCRIPTION_LENGTH];
	char	format_informal[MB_DESCRIPTION_LENGTH];
	char	format_attributes[MB_DESCRIPTION_LENGTH];
	int	list_mode;
	int	i;

	help = 0;
	verbose = 0;
	file_specified = MB_NO;
	format = 0;
	format_specified = MB_NO;
	html = MB_NO;
	list_mode = MBFORMAT_LIST_LONG;

	/* process argument list */
	while ((c = getopt(argc, argv, "F:f:HhI:i:LlKkVvWw")) != -1)
	  switch (c) 
		{
		case 'F':
		case 'f':
			sscanf (optarg,"%d", &format);
			format_specified = MB_YES;
			break;
		case 'L':
		case 'l':
			list_mode = MBFORMAT_LIST_SIMPLE;
			break;
		case 'K':
		case 'k':
			list_mode = MBFORMAT_LIST_ROOT;
			break;
		case 'H':
		case 'h':
			help++;
			break;
		case 'I':
		case 'i':
			sscanf (optarg,"%s", file);
			file_specified = MB_YES;
			break;
		case 'V':
		case 'v':
			verbose++;
			break;
		case 'W':
		case 'w':
			html = MB_YES;
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
	if (verbose == 1 || help)
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
		if (format_specified == MB_YES)
		    fprintf(stderr,"dbg2       format:  %d\n",format);
		if (file_specified == MB_YES)
		    fprintf(stderr,"dbg2       file:    %s\n",file);
		}

	/* if help desired then print it and exit */
	if (help)
		{
		fprintf(stderr,"\n%s\n",help_message);
		fprintf(stderr,"\nusage: %s\n", usage_message);
		exit(error);
		}
		
	/* print out the info */
	if (file_specified == MB_YES)
		{
		format_save = format;
		status = mb_get_format(verbose,file,root,&format,&error);
		}
	else if (format_specified == MB_YES)
		{
		format_save = format;
		status = mb_format(verbose,&format,&error);
		}
	if (file_specified == MB_YES
		&& format == 0)
		{
		if (list_mode == MBFORMAT_LIST_SIMPLE)
			printf("%d\n",format);
		else if (list_mode == MBFORMAT_LIST_ROOT)
			printf("%s %d\n", root, format);
		else
			printf("Program %s unable to infer format from filename %s\n",program_name,file);
		}
	else if (format_specified == MB_YES
		&& format == 0)
		{
		if (list_mode == MBFORMAT_LIST_SIMPLE)
			printf("%d\n",format);
		else if (list_mode == MBFORMAT_LIST_ROOT)
			printf("%s %d\n", root, format);
		else
			printf("Specified format %d invalid for MB-System\n",format_save);
		}
	else if (format != 0)
		{
		if (list_mode == MBFORMAT_LIST_SIMPLE)
			{
			printf("%d\n",format);
			}
		else if (list_mode == MBFORMAT_LIST_ROOT)
			{
			printf("%s %d\n", root, format);
			}
		else
			{
			status = mb_format_description(verbose,&format,format_description,&error);
			if (status == MB_SUCCESS)
				{
				printf("\nMBIO data format id: %d\n",format);
				printf("%s",format_description);
				}
			else if (file_specified == MB_YES)
				{
				printf("Program %s unable to infer format from filename %s\n",program_name,file);
				}
			else if (format_specified == MB_YES)
				{
				printf("Specified format %d invalid for MB-System\n",format_save);
				}
			}
		}
	else if (html == MB_YES)
		{
		printf("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2//EN\">\n");
		printf("<HTML>\n<HEAD>\n   <TITLE>MB-System Supported Data Formats</TITLE>\n");
		printf("</HEAD>\n<BODY TEXT=\"#000000\" BGCOLOR=\"#FFFFFF\" LINK=\"#336699\" VLINK=\"#997040\" ALINK=\"#CC9900\">\n\n");
		printf("<CENTER><P><B><FONT SIZE=+2>MB-System Supported Swath Data Formats</FONT></B></P></CENTER>\n\n");
		printf("<P>Each swath mapping sonar system outputs a data stream which includes\n");
		printf("some values or parameters unique to that system. In general, a number of\n");
		printf("different data formats have come into use for data from each of the sonar\n");
		printf("systems; many of these formats include only a subset of the original data\n");
		printf("stream. Internally, MBIO recognizes which sonar system each data format\n");
		printf("is associated with and uses a data structure including the complete data\n");
		printf("stream for that sonar. At present, formats associated with the following\n");
		printf("sonars are supported: </P>\n\n");
		printf("<UL>\n<LI>Sea Beam &quot;classic&quot; multibeam sonar </LI>\n\n");
		printf("<LI>Hydrosweep DS multibeam sonar </LI>\n\n");
		printf("<LI>Hydrosweep DS2 multibeam sonar </LI>\n\n");
		printf("<LI>Hydrosweep MD multibeam sonar </LI>\n\n");
		printf("<LI>Sea Beam 2000 multibeam sonar </LI>\n\n");
		printf("<LI>Sea Beam 2112 and 2136 multibeam sonars </LI>\n\n");
		printf("<LI>Sea Beam 2120 multibeam sonars </LI>\n\n");
		printf("<LI>Simrad EM12, EM121, EM950, and EM1000 multibeam sonars </LI>\n\n");
		printf("<LI>Simrad EM120, EM300, and EM3000 multibeam sonars</LI>\n\n");
		printf("<LI>Simrad EM122, EM302, EM710, and EM3002 multibeam sonars</LI>\n\n");
		printf("<LI>Simrad Mesotech SM2000 multibeam sonar</LI>\n\n");
		printf("<LI>Hawaii MR-1 shallow tow interferometric sonar </LI>\n\n");
		printf("<LI>ELAC Bottomchart and Bottomchart MkII shallow water multibeam sonars</LI>\n\n");
		printf("<LI>Reson Seabat multibeam sonars (e.g. 9001, 8081, 7125)</LI>\n\n");
		printf("<LI>WHOI DSL AMS-120 deep tow interferometric sonar </LI>\n\n");
		printf("<LI>Sea Scan sidescan sonar</LI>\n\n");
		printf("<LI>Furuno HS-1 multibeam sonar</LI>\n\n");
		printf("<LI>Edgetech sidescan and subbottom profiler sonars</LI>\n\n");
		printf("<LI>Imagenex DeltaT multibeam sonars</LI>\n\n");
		printf("<LI>Odom ES3 multibeam sonar</LI>\n\n");
		printf("</UL>\n\n");
		printf("<P>The following swath mapping sonar data formats are currently supported by MB-System:</P>\n\n");

		for (i=0;i<=1000;i++)
			{
			format = i;
			if ((status = mb_format_description(verbose,&format,format_description,&error)) == MB_SUCCESS
				&& format == i)
				{
				format_informal_ptr = (char *) 
				    strstr(format_description, "Informal Description:");
				format_attributes_ptr = (char *) 
				    strstr(format_description, "Attributes:");
				strncpy(format_name, format_description, 
					strlen(format_description) 
					    - strlen(format_informal_ptr));
				format_name[strlen(format_description) 
					    - strlen(format_informal_ptr) - 1] = '\0';
				strncpy(format_informal, format_informal_ptr, 
					strlen(format_informal_ptr) 
					    - strlen(format_attributes_ptr));
				format_informal[strlen(format_informal_ptr) 
					    - strlen(format_attributes_ptr) - 1] = '\0';
				strcpy(format_attributes, format_attributes_ptr);
				format_attributes[strlen(format_attributes_ptr)-1] = '\0';
				printf("\n<UL>\n<LI>MBIO Data Format ID:  %d </LI>\n",format);
				printf("\n<UL>\n<LI>%s</LI>\n",format_name);
				printf("\n<LI>%s</LI>\n",format_informal);
				printf("\n<LI>%s</LI>\n",format_attributes);
				printf("</UL>\n</UL>\n");
				}
			}

		printf("\n<CENTER><P><BR>\n");
		printf("Last Updated: %s</P></CENTER>\n", MB_FORMAT_UPDATEDATE);
		printf("\n<P>\n<HR WIDTH=\"100%%\"></P>\n\n");
		printf("<P><IMG SRC=\"mbsystem_logo_small.gif\" HEIGHT=55 WIDTH=158><A HREF=\"mbsystem_home.html\">Back\n");
		printf("to MB-System Home Page...</A></P>\n");
		printf("\n</BODY>\n</HTML>\n");

		status = MB_SUCCESS;
		error = MB_ERROR_NO_ERROR;
		}
	else if (list_mode == MB_YES)
		{
		for (i=0;i<=1000;i++)
			{
			format = i;
			if ((status = mb_format(verbose,&format,&error)) == MB_SUCCESS
				&& format == i)
				{
				printf("%d\n",format);
				}
			}
		status = MB_SUCCESS;
		error = MB_ERROR_NO_ERROR;
		}
	else
		{
		printf("\nSupported MBIO Formats:\n");
		for (i=0;i<=1000;i++)
			{
			format = i;
			if ((status = mb_format_description(verbose,&format,format_description,&error)) == MB_SUCCESS
				&& format == i)
				{
				printf("\nMBIO Data Format ID:  %d\n",format);
				printf("%s",format_description);
				}
			}
		status = MB_SUCCESS;
		error = MB_ERROR_NO_ERROR;
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
/*--------------------------------------------------------------------*/
