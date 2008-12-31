/*--------------------------------------------------------------------
 *    The MB-system:	mb_defaults.c	10/7/94
 *    $Id: mb_defaults.c,v 5.9 2008-12-31 08:47:38 caress Exp $
 *
 *    Copyright (c) 1993-2008 by
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
 * mb_defaults.c contains two functions - mb_defaults() and mb_env().
 * mb_defaults() returns the default MBIO control parameters and 
 * mb_env() returns the default MB-System environment variables - all
 * values are read from ~/.mbio_defaults providing this file exists.
 * The return values are MB_SUCCESS if the file exists and MB_FAILURE
 * if it does not exist.
 *
 * Author:	D. W. Caress
 * Date:	January 23, 1993
 * 
 * $Log: not supported by cvs2svn $
 * Revision 5.8  2007/10/08 15:59:34  caress
 * MBIO changes as of 8 October 2007.
 *
 * Revision 5.7  2006/09/11 18:55:52  caress
 * Changes during Western Flyer and Thomas Thompson cruises, August-September
 * 2006.
 *
 * Revision 5.6  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.5  2002/10/02 23:55:42  caress
 * Release 5.0.beta24
 *
 * Revision 5.4  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.3  2001/11/20 21:52:13  caress
 * The .mbio_defaults file no longer controls format,
 * pings, bounds, btime_i, and etime_i.
 *
 * Revision 5.2  2001/07/20  00:31:11  caress
 * Release 5.0.beta03
 *
 * Revision 5.1  2001/04/30  05:13:06  caress
 * Changed handling of mbdefaults - more flexible parsing of
 * defaults file and addition of image viewer default.
 *
 * Revision 5.0  2000/12/01 22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.8  2000/10/11  01:02:30  caress
 * Convert to ANSI C
 *
 * Revision 4.7  2000/09/30  06:26:58  caress
 * Snapshot for Dale.
 *
 * Revision 4.6  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.5  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.5  1997/04/17  18:53:33  caress
 * Added LINUX ifdef.
 *
 * Revision 4.4  1995/03/22  19:14:25  caress
 * Added #ifdef's for HPUX.
 *
 * Revision 4.3  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.2  1995/01/25  18:17:24  caress
 * Let the operating system define determine the
 * system default postscript viewer.
 *
 * Revision 4.1  1994/10/21  12:11:53  caress
 * Release V4.0
 *
 * Revision 4.0  1994/03/05  23:55:38  caress
 * First cut at version 4.0
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
 * Revision 3.1  1993/05/14  22:33:05  sohara
 * fixed rcs_id message
 *
 * Revision 3.1  1993/05/14  22:33:05  sohara
 * fixed rcs_id message
 *
 * Revision 3.0  1993/04/23  15:46:59  dale
 * Initial version
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_define.h"

/*--------------------------------------------------------------------*/
int mb_defaults(int verbose, int *format, int *pings,
		int *lonflip, double bounds[4], 
		int *btime_i, int *etime_i,
		double *speedmin, double *timegap)
{
  static char rcs_id[]="$Id: mb_defaults.c,v 5.9 2008-12-31 08:47:38 caress Exp $";
	char	*function_name = "mb_defaults";
	int	status;
	FILE	*fp;
	char	file[MB_PATH_MAXLINE];
	char	home[MB_PATH_MAXLINE];
	char	string[MB_PATH_MAXLINE];
	char	*HOME = "HOME";
	char	*home_ptr;
	char	*getenv();

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose: %d\n",verbose);
		}

	/* set system default values */
	*format = 0;
	*pings = 1;
	*lonflip = 0;
	bounds[0] = -360.;
	bounds[1] = 360.;
	bounds[2] = -90.;
	bounds[3] = 90.;
	btime_i[0] = 1962;
	btime_i[1] = 2;
	btime_i[2] = 21;
	btime_i[3] = 10;
	btime_i[4] = 30;
	btime_i[5] = 0;
	btime_i[6] = 0;
	etime_i[0] = 2062;
	etime_i[1] = 2;
	etime_i[2] = 21;
	etime_i[3] = 10;
	etime_i[4] = 30;
	etime_i[5] = 0;
	etime_i[6] = 0;
	*speedmin = 0.0;
	*timegap = 1.0;

	/* set the filename */
	if ((home_ptr = getenv(HOME)) != NULL)
		{
		strcpy(file,home_ptr);
		strcat(file,"/.mbio_defaults");

		/* open and read values from file if possible */
		if ((fp = fopen(file, "r")) != NULL)
			{
			status = MB_SUCCESS;
			while (fgets(string,sizeof(string),fp) != NULL)
				{
				if (strncmp(string,"lonflip:",8) == 0)
					sscanf(string,"lonflip: %d",lonflip);
				if (strncmp(string,"speed:",6) == 0)
					sscanf(string,"timegap: %lf",timegap);
				}
 			fclose(fp);
			}
		else
			status = MB_FAILURE;
		}
	else
		status = MB_FAILURE;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       format:     %d\n",*format);
		fprintf(stderr,"dbg2       pings:      %d\n",*pings);
		fprintf(stderr,"dbg2       lonflip:    %d\n",*lonflip);
		fprintf(stderr,"dbg2       bounds[0]:  %f\n",bounds[0]);
		fprintf(stderr,"dbg2       bounds[1]:  %f\n",bounds[1]);
		fprintf(stderr,"dbg2       bounds[2]:  %f\n",bounds[2]);
		fprintf(stderr,"dbg2       bounds[3]:  %f\n",bounds[3]);
		fprintf(stderr,"dbg2       btime_i[0]: %d\n",btime_i[0]);
		fprintf(stderr,"dbg2       btime_i[1]: %d\n",btime_i[1]);
		fprintf(stderr,"dbg2       btime_i[2]: %d\n",btime_i[2]);
		fprintf(stderr,"dbg2       btime_i[3]: %d\n",btime_i[3]);
		fprintf(stderr,"dbg2       btime_i[4]: %d\n",btime_i[4]);
		fprintf(stderr,"dbg2       btime_i[5]: %d\n",btime_i[5]);
		fprintf(stderr,"dbg2       btime_i[6]: %d\n",btime_i[6]);
		fprintf(stderr,"dbg2       etime_i[0]: %d\n",etime_i[0]);
		fprintf(stderr,"dbg2       etime_i[1]: %d\n",etime_i[1]);
		fprintf(stderr,"dbg2       etime_i[2]: %d\n",etime_i[2]);
		fprintf(stderr,"dbg2       etime_i[3]: %d\n",etime_i[3]);
		fprintf(stderr,"dbg2       etime_i[4]: %d\n",etime_i[4]);
		fprintf(stderr,"dbg2       etime_i[5]: %d\n",etime_i[5]);
		fprintf(stderr,"dbg2       etime_i[6]: %d\n",etime_i[6]);
		fprintf(stderr,"dbg2       speedmin:   %f\n",*speedmin);
		fprintf(stderr,"dbg2       timegap:    %f\n",*timegap);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_env(int verbose, char *psdisplay, char *imgdisplay, char *mbproject)
{
  static char rcs_id[]="$Id: mb_defaults.c,v 5.9 2008-12-31 08:47:38 caress Exp $";
	char	*function_name = "mbenv";
	int	status;
	FILE	*fp;
	char	file[MB_PATH_MAXLINE]; 
	char	home[MB_PATH_MAXLINE];
	char	string[MB_PATH_MAXLINE];
	char	*HOME = "HOME";
	char	*home_ptr;
	char	*getenv();

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose: %d\n",verbose);
		}

	/* set system default Postscript displayer */
#ifdef IRIX
	strcpy(psdisplay, "xpsview");
	strcpy(imgdisplay, "xv");
#endif
#ifdef IRIX64
	strcpy(psdisplay, "xpsview");
	strcpy(imgdisplay, "xv");
#endif
#ifdef SOLARIS
	strcpy(psdisplay, "pageview");
	strcpy(imgdisplay, "xv");
#endif
#ifdef LYNX
	strcpy(psdisplay, "ghostview");
	strcpy(imgdisplay, "xv");
#endif
#ifdef LINUX
	strcpy(psdisplay, "ghostview");
	strcpy(imgdisplay, "gimp");
#endif
#ifdef SUN
	strcpy(psdisplay, "pageview");
	strcpy(imgdisplay, "xv");
#endif
#ifdef HPUX
	strcpy(psdisplay, "ghostview");
	strcpy(imgdisplay, "xv");
#endif
#ifdef DARWIN
	strcpy(psdisplay, "gv");
	strcpy(imgdisplay, "display");
#endif
#ifdef CYGWIN
	strcpy(psdisplay, "ghostview");
	strcpy(imgdisplay, "xv");
#endif
#ifdef OTHER
	strcpy(psdisplay, "ghostview");
	strcpy(imgdisplay, "xv");
#endif

	/* set system default project name */
	strcpy(mbproject, "none");

	/* set the filename */
	if ((home_ptr = getenv(HOME)) != NULL)
		{
		strcpy(file,home_ptr);
		strcat(file,"/.mbio_defaults");

		/* open and read values from file if possible */
		if ((fp = fopen(file, "r")) != NULL)
			{
			status = MB_SUCCESS;
			while (fgets(string,sizeof(string),fp) != NULL)
				{
				if (strncmp(string,"ps viewer:",10) == 0)
					sscanf(string,"ps viewer: %s",psdisplay);
				if (strncmp(string,"img viewer:",10) == 0)
					sscanf(string,"img viewer: %s",imgdisplay);
				if (strncmp(string,"project:",8) == 0)
					sscanf(string,"project: %s",mbproject);
				}
 			fclose(fp);
			}
		else
			status = MB_FAILURE;
		}
	else
		status = MB_FAILURE;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       psdisplay:  %s\n",psdisplay);
		fprintf(stderr,"dbg2       mbproject:  %s\n",mbproject);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_lonflip(int verbose, int *lonflip)
{
  static char rcs_id[]="$Id: mb_defaults.c,v 5.9 2008-12-31 08:47:38 caress Exp $";
	char	*function_name = "mb_lonflip";
	int	status;
	FILE	*fp;
	char	file[MB_PATH_MAXLINE];
	char	home[MB_PATH_MAXLINE];
	char	string[MB_PATH_MAXLINE];
	char	*HOME = "HOME";
	char	*home_ptr;
	char	*getenv();

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose: %d\n",verbose);
		}

	/* set system default values */
	*lonflip = 0;

	/* set the filename */
	if ((home_ptr = getenv(HOME)) != NULL)
		{
		strcpy(file,home_ptr);
		strcat(file,"/.mbio_defaults");

		/* open and read values from file if possible */
		if ((fp = fopen(file, "r")) != NULL)
			{
			status = MB_SUCCESS;
			while (fgets(string,sizeof(string),fp) != NULL)
				{
				if (strncmp(string,"lonflip:",8) == 0)
					sscanf(string,"lonflip: %d",lonflip);
				}
 			fclose(fp);
			}
		else
			status = MB_FAILURE;
		}
	else
		status = MB_FAILURE;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       lonflip:    %d\n",*lonflip);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
