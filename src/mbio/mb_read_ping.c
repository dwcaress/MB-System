/*--------------------------------------------------------------------
 *    The MB-system:	mb_read_ping.c	2/3/93
 *    $Id$

 *    Copyright (c) 1993-2009 by
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
 * $Log: mb_read_ping.c,v $
 * Revision 5.5  2005/11/05 00:48:03  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.4  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.3  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.2  2002/05/29 23:36:53  caress
 * Release 5.0.beta18
 *
 * Revision 5.1  2001/07/20 00:31:11  caress
 * Release 5.0.beta03
 *
 * Revision 5.0  2000/12/01 22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.22  2000/10/11  01:02:30  caress
 * Convert to ANSI C
 *
 * Revision 4.21  2000/09/30  06:32:11  caress
 * Snapshot for Dale.
 *
 * Revision 4.20  1999/10/21  22:38:36  caress
 * Added MBPRONAV format.
 *
 * Revision 4.19  1999/08/08  04:12:45  caress
 * Added ELMK2XSE format.
 *
 * Revision 4.18  1999/07/16  19:24:15  caress
 * Yet another version.
 *
 * Revision 4.17  1999/03/31  18:11:35  caress
 * MB-System 4.6beta7
 *
 * Revision 4.16  1999/01/01  23:41:06  caress
 * MB-System version 4.6beta6
 *
 * Revision 4.15  1998/12/17  23:01:15  caress
 * MB-System version 4.6beta4
 *
 * Revision 4.14  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.13  1997/09/15  19:06:40  caress
 * Real Version 4.5
 *
 * Revision 4.12  1997/07/25  14:19:53  caress
 * Version 4.5beta2.
 * Much mucking, particularly with Simrad formats.
 *
 * Revision 4.11  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.11  1997/04/17  15:07:36  caress
 * MB-System 4.5 Beta Release
 *
 * Revision 4.10  1996/08/26  17:24:56  caress
 * Release 4.4 revision.
 *
 * Revision 4.9  1996/08/05  15:21:58  caress
 * Just redid i/o for Simrad sonars, including adding EM12S and EM121 support.
 *
 * Revision 4.8  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.7  1996/04/22  10:57:09  caress
 * DTR define now in mb_io.h
 *
 * Revision 4.6  1996/03/12  17:21:55  caress
 * Added format 63, short HMR1 processing format.
 *
 * Revision 4.5  1996/01/26  21:23:30  caress
 * Version 4.3 distribution
 *
 * Revision 4.4  1995/09/28  18:10:48  caress
 * Various bug fixes working toward release 4.3.
 *
 * Revision 4.3  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.2  1994/10/21  12:11:53  caress
 * Release V4.0
 *
 * Revision 4.1  1994/07/29  18:46:51  caress
 * Changes associated with supporting Lynx OS (byte swapped) and
 * using unix second time base (for time_d values).
 *
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
 * Revision 4.3  1994/03/05  22:51:44  caress
 * Added ability to handle Simrad EM12 system and
 * format MBF_EM12DARW.
 *
 * Revision 4.2  1994/03/05  02:09:29  caress
 * Altered to add MBF_SB2100RW format.
 *
 * Revision 4.1  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.0  1994/02/20  01:54:09  caress
 * First cut of new version.
 *
 * Revision 3.1  1993/05/14  22:42:47  sohara
 * fixed rcs_id message
 *
 * Revision 3.1  1993/05/14  22:42:47  sohara
 * fixed rcs_id message
 *
 * Revision 3.0  1993/04/23  18:53:05  dale
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
		fprintf(stderr,"dbg2       mb_ptr:     %ld\n",(size_t)mbio_ptr);
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
