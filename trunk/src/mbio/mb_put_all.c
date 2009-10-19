/*--------------------------------------------------------------------
 *    The MB-system:	mb_put_all.c	2/4/93
 *    $Id: mb_put_all.c,v 5.4 2006/03/06 21:47:48 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 2000, 2002, 2003 by
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
 * mb_put_all.c writes multibeam data to a file
 * which has been initialized by mb_write_init(). Crosstrack distances
 * are used rather than lon and lat for the beams. Values are also read 
 * from a storage data structure including
 * all possible values output by the particular multibeam system 
 * associated with the specified format.
 *
 * Author:	D. W. Caress
 * Date:	February 4, 1993
 *
 * $Log: mb_put_all.c,v $
 * Revision 5.4  2006/03/06 21:47:48  caress
 * Implemented changes suggested by Bob Courtney of the Geological Survey of Canada to support translating Reson data to GSF.
 *
 * Revision 5.3  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.2  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.1  2001/07/20 00:31:11  caress
 * Release 5.0.beta03
 *
 * Revision 5.0  2000/12/01 22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.8  2000/10/11  01:02:30  caress
 * Convert to ANSI C
 *
 * Revision 4.7  2000/09/30  06:32:11  caress
 * Snapshot for Dale.
 *
 * Revision 4.6  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.5  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.4  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.4  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.3  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.2  1994/10/21  12:11:53  caress
 * Release V4.0
 *
 * Revision 4.1  1994/05/11  18:43:58  caress
 * Fixed debug statement for time_d value.
 *
 * Revision 4.1  1994/05/11  18:43:58  caress
 * Fixed debug statement for time_d value.
 *
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
 * Revision 4.4  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.3  1994/02/23  00:32:27  caress
 * Fixed several debug messages plus a couple real bugs.
 *
 * Revision 4.2  1994/02/20  03:37:05  caress
 * Fixed a number of bad variable names.
 *
 * Revision 4.1  1994/02/20  02:36:37  caress
 * Fixed the calling parameters.
 *
 * Revision 4.0  1994/02/20  02:33:08  caress
 * First cut at new version. Includes new handling of
 * sidescan and amplitude data.
 *
 * Revision 3.2  1993/06/15  16:00:29  caress
 * Fixed bug which caused mbmerge to change non-survey data
 * records into survey data records.  Code change is on line
 * 142 which used to read:
 *      mb_io_ptr->new_kind = MB_DATA_DATA;
 * but now reads:
 *      mb_io_ptr->new_kind = kind;
 *
 * Revision 3.1  1993/05/14  22:37:56  sohara
 * fixed rcs_id message
 *
 * Revision 3.0  1993/04/23  18:21:17  dale
 * Initial version
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_io.h"
#include "../../include/mb_define.h"

/*--------------------------------------------------------------------*/
int mb_put_all(int verbose, void *mbio_ptr, void *store_ptr,
		int usevalues, int kind, 
		int time_i[7], double time_d,
		double navlon, double navlat, 
		double speed, double heading,
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
  static char rcs_id[]="$Id: mb_put_all.c,v 5.4 2006/03/06 21:47:48 caress Exp $";
	char	*function_name = "mb_put_all";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int	i;
	int	done;
	double	mtodeglon, mtodeglat;
	double	dx, dy;
	double	delta_time;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		fprintf(stderr,"dbg2       usevalues:  %d\n",usevalues);
		fprintf(stderr,"dbg2       kind:       %d\n",kind);
		}
	if (verbose >= 2 && usevalues == MB_YES && kind != MB_DATA_COMMENT)
		{
		fprintf(stderr,"dbg2       time_i[0]:  %d\n",time_i[0]);
		fprintf(stderr,"dbg2       time_i[1]:  %d\n",time_i[1]);
		fprintf(stderr,"dbg2       time_i[2]:  %d\n",time_i[2]);
		fprintf(stderr,"dbg2       time_i[3]:  %d\n",time_i[3]);
		fprintf(stderr,"dbg2       time_i[4]:  %d\n",time_i[4]);
		fprintf(stderr,"dbg2       time_i[5]:  %d\n",time_i[5]);
		fprintf(stderr,"dbg2       time_i[6]:  %d\n",time_i[6]);
		fprintf(stderr,"dbg2       time_d:     %f\n",time_d);
		fprintf(stderr,"dbg2       navlon:     %f\n",navlon);
		fprintf(stderr,"dbg2       navlat:     %f\n",navlat);
		fprintf(stderr,"dbg2       speed:      %f\n",speed);
		fprintf(stderr,"dbg2       heading:    %f\n",heading);
		}
	if (verbose >= 2 && usevalues == MB_YES && kind == MB_DATA_DATA)
		{
		fprintf(stderr,"dbg2       nbath:      %d\n",nbath);
		if (verbose >= 3 && nbath > 0)
		  {
		  fprintf(stderr,"dbg3       beam  flag  bath  crosstrack alongtrack\n");
		  for (i=0;i<nbath;i++)
		    fprintf(stderr,"dbg3       %4d   %3d   %f    %f     %f\n",
			i,beamflag[i],bath[i],
			bathacrosstrack[i],bathalongtrack[i]);
		  }
		fprintf(stderr,"dbg2       namp:       %d\n",namp);
		if (verbose >= 3 && namp > 0)
		  {
		  fprintf(stderr,"dbg3       beam    amp  crosstrack alongtrack\n");
		  for (i=0;i<namp;i++)
		    fprintf(stderr,"dbg3       %4d   %f    %f     %f\n",
			i,amp[i],
			bathacrosstrack[i],bathalongtrack[i]);
		  }
		fprintf(stderr,"dbg2       nss:        %d\n",nss);
		if (verbose >= 3 && nss > 0)
		  {
		  fprintf(stderr,"dbg3       pixel sidescan crosstrack alongtrack\n");
		  for (i=0;i<nss;i++)
		    fprintf(stderr,"dbg3       %4d   %f    %f     %f\n",
			i,ss[i],
			ssacrosstrack[i],ssalongtrack[i]);
		  }
		}
	if (verbose >= 2 && usevalues == MB_YES && kind == MB_DATA_COMMENT)
		{
		fprintf(stderr,"dbg2       comment:    %s\n",comment);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* insert values into structure if requested */
	if (usevalues == MB_YES)
		{
		status = mb_insert(verbose,mbio_ptr,store_ptr,
			kind,time_i,time_d,navlon,navlat,speed,heading,
			nbath,namp,nss,
			beamflag,bath,amp,
			bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,error);
		}

	/* write the data */
	status = mb_write_ping(verbose,mbio_ptr,store_ptr,error);

	/* increment counters */
	if (status == MB_SUCCESS)
		{
		if (kind == MB_DATA_DATA)
			mb_io_ptr->ping_count++;
		else if (kind == MB_DATA_NAV)
			mb_io_ptr->nav_count++;
		else if (kind == MB_DATA_COMMENT)
			mb_io_ptr->comment_count++;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
