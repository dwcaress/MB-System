/*--------------------------------------------------------------------
 *    The MB-system:	mb_put_comment.c	7/15/97
 *    $Id$
 *
 *    Copyright (c) 1997-2009 by
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
 * mb_put.c writes comments to a swath sonar data file
 * which has been initialized by mb_write_init().
 *
 * Author:	D. W. Caress
 * Date:	July 15, 1997
 *
 * $Log: mb_put_comment.c,v $
 * Revision 5.3  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.2  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.1  2001/07/20 00:31:11  caress
 * Release 5.0.beta03
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.3  2000/10/11  01:02:30  caress
 * Convert to ANSI C
 *
 * Revision 4.2  2000/09/30  06:32:11  caress
 * Snapshot for Dale.
 *
 * Revision 4.1  1998/10/05  18:32:27  caress
 * MB-System version 4.6beta
 *
 * Revision 4.0  1997/07/25  14:25:40  caress
 * Version 4.5beta2.
 *
 * Revision 4.0  1997/07/25  14:25:40  caress
 * Version 4.5beta2.
 *
 * Revision 1.1  1997/07/25  14:19:53  caress
 * Initial revision
 *
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
int mb_put_comment(int verbose, void *mbio_ptr, char *comment, int *error)
{
  static char rcs_id[]="$Id $";
	char	*function_name = "mb_put_comment";
	int	status;
	struct mb_io_struct *mb_io_ptr;
	int	time_i[7] = {0, 0, 0, 0, 0, 0, 0};
	double	time_d = 0.0;
	double	navlon = 0.0;
	double	navlat = 0.0;
	double	speed = 0.0;
	double	heading = 0.0;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %ld\n",(long)mbio_ptr);
		fprintf(stderr,"dbg2       comment:    %s\n",comment);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* insert comment using mb_insert */
	for (i=0;i<7;i++)
	    time_i[i] = 0;
	time_d = 0.0;
	navlon = 0.0;
	status = mb_insert(verbose,mbio_ptr,mb_io_ptr->store_data,
			MB_DATA_COMMENT, 
			time_i,time_d,navlon,navlat,speed,heading,
			0,0,0,
			NULL,NULL,NULL,
			NULL,NULL,
			NULL,NULL,NULL,
			comment,error);

	/* write the data */
	status = mb_write_ping(verbose,mbio_ptr,mb_io_ptr->store_data,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
