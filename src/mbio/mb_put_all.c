/*--------------------------------------------------------------------
 *    The MB-system:	mb_put_all.c	3.00	2/4/93
 *    $Id: mb_put_all.c,v 3.2 1993-06-15 16:00:29 caress Exp $
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
 * $Log: not supported by cvs2svn $
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
#include <strings.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_io.h"

/*--------------------------------------------------------------------*/
int mb_put_all(verbose,mbio_ptr,store_ptr,usevalues,kind,time_i,time_d,
		navlon,navlat,speed,heading,
		nbath,bath,bathdist,nback,back,backdist,
		comment,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	usevalues;
int	kind;
int	time_i[6];
double	time_d;
double	navlon;
double	navlat;
double	speed;
double	heading;
int	nbath;
int	*bath;
int	*bathdist;
int	nback;
int	*back;
int	*backdist;
char	*comment;
int	*error;
{
  static char rcs_id[]="$Id: mb_put_all.c,v 3.2 1993-06-15 16:00:29 caress Exp $";
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
	if (verbose >= 2 && usevalues == MB_YES && kind != 2)
		{
		fprintf(stderr,"dbg2       time_i[0]:  %d\n",time_i[0]);
		fprintf(stderr,"dbg2       time_i[1]:  %d\n",time_i[1]);
		fprintf(stderr,"dbg2       time_i[2]:  %d\n",time_i[2]);
		fprintf(stderr,"dbg2       time_i[3]:  %d\n",time_i[3]);
		fprintf(stderr,"dbg2       time_i[4]:  %d\n",time_i[4]);
		fprintf(stderr,"dbg2       time_i[5]:  %d\n",time_i[5]);
		fprintf(stderr,"dbg2       time_d:     %d\n",time_d);
		fprintf(stderr,"dbg2       navlon:     %f\n",navlon);
		fprintf(stderr,"dbg2       navlat:     %f\n",navlat);
		fprintf(stderr,"dbg2       speed:      %f\n",speed);
		fprintf(stderr,"dbg2       heading:    %f\n",heading);
		}
	if (verbose >= 2 && usevalues == MB_YES && kind == 1)
		{
		fprintf(stderr,"dbg2       nbath:      %d\n",nbath);
		if (verbose >= 3) 
		 for (i=0;i<nbath;i++)
		  fprintf(stderr,"dbg3       bath[%d]: %d  bathdist[%d]: %d\n",
			i,bath[i],i,bathdist[i]);
		fprintf(stderr,"dbg2       nback:      %d\n",nback);
		if (verbose >= 3) 
		 for (i=0;i<nback;i++)
		  fprintf(stderr,"dbg3       back[%d]: %d  backdist[%d]: %d\n",
			i,back[i],i,backdist[i]);
		}
	if (verbose >= 2 && usevalues == MB_YES && kind == 2)
		{
		fprintf(stderr,"dbg2       comment:    %s\n",comment);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* check numbers of beams */
	if (nbath != mb_io_ptr->beams_bath)
		mb_io_ptr->beams_bath = nbath;
	if (nback != mb_io_ptr->beams_back)
		mb_io_ptr->beams_back = nback;

	/* transfer values to mb_io_ptr structure if requested */
	if (usevalues == MB_YES && kind == MB_DATA_COMMENT)
		{
		mb_io_ptr->new_error = MB_ERROR_NO_ERROR;
		mb_io_ptr->new_kind = MB_DATA_COMMENT;
		strcpy(mb_io_ptr->new_comment,comment);
		}
	else if (usevalues == MB_YES)
		{
		mb_io_ptr->new_error = MB_ERROR_NO_ERROR;
		mb_io_ptr->new_kind = kind;
		if (time_i[0] == 0)
			mb_get_time(verbose,time_d,mb_io_ptr->new_time_i);
		else
			for (i=0;i<6;i++)
				mb_io_ptr->new_time_i[i] = time_i[i];
		mb_io_ptr->new_time_d = time_d;
		mb_io_ptr->new_lon = navlon;
		mb_io_ptr->new_lat = navlat;
		mb_io_ptr->new_speed = speed;
		mb_io_ptr->new_heading = heading;
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			mb_io_ptr->new_bath[i] = bath[i];
			mb_io_ptr->new_bathdist[i] = bathdist[i];
			}
		for (i=0;i<mb_io_ptr->beams_back;i++)
			{
			mb_io_ptr->new_back[i] = back[i];
			mb_io_ptr->new_backdist[i] = backdist[i];
			}
		}
	else
		mb_io_ptr->new_error = MB_ERROR_IGNORE;

	/* write the data */
	status = mb_write_ping(verbose,mbio_ptr,store_ptr,error);

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
