/*--------------------------------------------------------------------
 *    The MB-system:	mb_read_ping.c	2/3/93
 *    $Id: mb_read_ping.c,v 4.0 1994-03-06 00:01:56 caress Exp $

 *    Copyright (c) 1993, 1994 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
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
 * $Log: not supported by cvs2svn $
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
#include <strings.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_io.h"

/*--------------------------------------------------------------------*/
int mb_read_ping(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
  static char rcs_id[]="$Id: mb_read_ping.c,v 4.0 1994-03-06 00:01:56 caress Exp $";
	char	*function_name = "mb_read_ping";
	int	status;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* call the appropriate mbr_ read and translate routine */
	if (mb_io_ptr->format == MBF_SBSIOMRG)
		{
		status = mbr_rt_sbsiomrg(verbose,mbio_ptr,store_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SBSIOCEN)
		{
		status = mbr_rt_sbsiocen(verbose,mbio_ptr,store_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SBSIOLSI)
		{
		status = mbr_rt_sbsiolsi(verbose,mbio_ptr,store_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SBURICEN)
		{
		status = mbr_rt_sburicen(verbose,mbio_ptr,store_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_HSLDEDMB)
		{
		status = mbr_rt_hsldedmb(verbose,mbio_ptr,store_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_HSURICEN)
		{
		status = mbr_rt_hsuricen(verbose,mbio_ptr,store_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_HSATLRAW)
		{
		status = mbr_rt_hsatlraw(verbose,mbio_ptr,store_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_HSLDEOIH)
		{
		status = mbr_rt_hsldeoih(verbose,mbio_ptr,store_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SB2100RW)
		{
		status = mbr_rt_sb2100rw(verbose,mbio_ptr,store_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_EM12DARW)
		{
		status = mbr_rt_em12darw(verbose,mbio_ptr,store_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_MBLDEOIH)
		{
		status = mbr_rt_mbldeoih(verbose,mbio_ptr,store_ptr,error);
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
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
