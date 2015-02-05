/*--------------------------------------------------------------------
 *    The MB-system:	mb_read_ping.c	2/3/93
 *    $Id$

 *    Copyright (c) 1993-2015 by
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
 * mb_read_ping.c calls the appropriate mbr_ routine for reading
 * the next ping from a multibeam data file.  The new ping data
 * will be placed in the "new_" variables in the mbio structure pointed
 * to by mbio_ptr.
 *
 * Author:	D. W. Caress
 * Date:	February 3, 1993
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_define.h"

/*--------------------------------------------------------------------*/
int mb_read_ping(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *error)
{
  static char rcs_id[]="$Id$";
	char	*function_name = "mb_read_ping";
	int	status;
	struct mb_io_struct *mb_io_ptr;
	int	localkind;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %p\n",(void *)mbio_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* call the appropriate mbr_ read and translate routine */
	if (mb_io_ptr->mb_io_read_ping != NULL)
		{
		status = (*mb_io_ptr->mb_io_read_ping)(verbose,mbio_ptr,store_ptr,error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_FORMAT;
		}

	/* set data record kind */
	if (status == MB_SUCCESS)
		{
		*kind = mb_io_ptr->new_kind;
		mb_notice_log_datatype(verbose, mb_io_ptr, *kind);
		}
	else
		*kind = MB_DATA_NONE;

	/* check that io arrays are large enough, allocate larger arrays if necessary */
	if (status == MB_SUCCESS
		&& mb_io_ptr->new_kind == MB_DATA_DATA)
		{
		/* check size of arrays needed for newly read data */
		status = mb_dimensions(verbose,mbio_ptr,store_ptr,&localkind,
					&beams_bath,&beams_amp,&pixels_ss,error);

		/* if existing allocations are insufficient, allocate larger arrays
			- this includes both arrays hidden within the mbio_ptr structure
			and arrays registered by the application */
		if (beams_bath > mb_io_ptr->beams_bath_alloc
			|| beams_amp > mb_io_ptr->beams_amp_alloc
			|| pixels_ss > mb_io_ptr->pixels_ss_alloc)
			{
			status = mb_update_arrays(verbose, mbio_ptr,
				beams_bath, beams_amp, pixels_ss, error);
			}
		mb_io_ptr->beams_bath_max = MAX(mb_io_ptr->beams_bath_max, beams_bath);
		mb_io_ptr->beams_amp_max = MAX(mb_io_ptr->beams_amp_max, beams_amp);
		mb_io_ptr->pixels_ss_max = MAX(mb_io_ptr->pixels_ss_max, pixels_ss);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:       %d\n",*kind);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
