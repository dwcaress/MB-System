/*--------------------------------------------------------------------
 *    The MB-system:	mb_defaults.c	1/21/93
 *    $Id: mb_defaults.c,v 4.0 1994-03-05 23:55:38 caress Exp $
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
 * mb_defaults.c returns the default MBIO control parameters - the
 * values are read from ~/.mbio_defaults if this file exists.
 * The return value is MB_SUCCESS if the file exists and MB_FAILURE
 * if it does not exist.
 *
 * Author:	D. W. Caress
 * Date:	January 23, 1993
 * 
 * $Log: not supported by cvs2svn $
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
#include <strings.h>

/* mbio include files */
#include "../../include/mb_status.h"

/*--------------------------------------------------------------------*/
int mb_defaults(verbose,format,pings,lonflip,bounds,btime_i,etime_i,
		speedmin,timegap)
int verbose;
int *format;
int *pings;
int *lonflip;
double *bounds;
int *btime_i;
int *etime_i;
double *speedmin;
double *timegap;
{
  static char rcs_id[]="$Id: mb_defaults.c,v 4.0 1994-03-05 23:55:38 caress Exp $";
	char	*function_name = "mb_defaults";
	int	status;
	FILE	*fp;
	char	file[128];
	char	home[128];
	char	dummy[128];
	char	*HOME = "HOME";
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
	*format = 1;
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
	etime_i[0] = 2062;
	etime_i[1] = 2;
	etime_i[2] = 21;
	etime_i[3] = 10;
	etime_i[4] = 30;
	etime_i[5] = 0;
	*speedmin = 0.0;
	*timegap = 1.0;

	/* set the filename */
	strcpy(file,getenv(HOME));
	strcat(file,"/.mbio_defaults");

	/* open and read values from file if possible */
	if ((fp = fopen(file, "r")) != NULL)
		{
		status = MB_SUCCESS;
		fgets(dummy,sizeof(dummy),fp);
		fgets(dummy,sizeof(dummy),fp);
		sscanf(&dummy[12],"%d",format);
		fgets(dummy,sizeof(dummy),fp);
		sscanf(&dummy[12],"%d",pings);
		fgets(dummy,sizeof(dummy),fp);
		sscanf(&dummy[12],"%d",lonflip);
		fgets(dummy,sizeof(dummy),fp);
		sscanf(&dummy[12],"%lf",speedmin);
		fgets(dummy,sizeof(dummy),fp);
		sscanf(&dummy[12],"%lf",timegap);
		fgets(dummy,sizeof(dummy),fp);
		sscanf(&dummy[12],"%lf %lf %lf %lf",
			&bounds[0],&bounds[1],&bounds[2],&bounds[3]);
		fgets(dummy,sizeof(dummy),fp);
		sscanf(&dummy[12],"%d %d %d %d %d %d",
			&btime_i[0],&btime_i[1],&btime_i[2],
			&btime_i[3],&btime_i[4],&btime_i[5]);
		fgets(dummy,sizeof(dummy),fp);
		sscanf(&dummy[12],"%d %d %d %d %d %d",
			&etime_i[0],&etime_i[1],&etime_i[2],
			&etime_i[3],&etime_i[4],&etime_i[5]);
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
		fprintf(stderr,"dbg2       etime_i[0]: %d\n",etime_i[0]);
		fprintf(stderr,"dbg2       etime_i[1]: %d\n",etime_i[1]);
		fprintf(stderr,"dbg2       etime_i[2]: %d\n",etime_i[2]);
		fprintf(stderr,"dbg2       etime_i[3]: %d\n",etime_i[3]);
		fprintf(stderr,"dbg2       etime_i[4]: %d\n",etime_i[4]);
		fprintf(stderr,"dbg2       etime_i[5]: %d\n",etime_i[5]);
		fprintf(stderr,"dbg2       speedmin:   %f\n",*speedmin);
		fprintf(stderr,"dbg2       timegap:    %f\n",*timegap);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
