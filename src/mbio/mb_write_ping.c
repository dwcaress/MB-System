/*--------------------------------------------------------------------
 *    The MB-system:	mb_write_ping.c	2/3/93
 *	$Id: mb_write_ping.c,v 4.0 1994-03-06 00:01:56 caress Exp $
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
 * mb_write_ping.c calls the appropriate mbr_ routine for writing
 * the next ping to a multibeam data file.  The current ping data
 * must be in the "new_" variables in the mbio structure pointed
 * to by mbio_ptr.
 *
 * Author:	D. W. Caress
 * Date:	Febrary 3, 1993
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
 * Revision 4.0  1994/02/20  01:57:11  caress
 * First cut of new version.
 *
 * Revision 3.0  1993/05/14  22:48:53  sohara
 * installed in rcs
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
int mb_write_ping(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
 static char res_id[]="$Id: mb_write_ping.c,v 4.0 1994-03-06 00:01:56 caress Exp $";
	char	*function_name = "mb_write_ping";
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
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* call the appropriate mbr_ write and translate routine */
	if (mb_io_ptr->format == MBF_SBSIOMRG)
		{
		status = mbr_wt_sbsiomrg(verbose,mbio_ptr,store_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SBSIOCEN)
		{
		status = mbr_wt_sbsiocen(verbose,mbio_ptr,store_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SBSIOLSI)
		{
		status = mbr_wt_sbsiolsi(verbose,mbio_ptr,store_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SBURICEN)
		{
		status = mbr_wt_sburicen(verbose,mbio_ptr,store_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_HSLDEDMB)
		{
		status = mbr_wt_hsldedmb(verbose,mbio_ptr,store_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_HSURICEN)
		{
		status = mbr_wt_hsuricen(verbose,mbio_ptr,store_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_HSATLRAW)
		{
		status = mbr_wt_hsatlraw(verbose,mbio_ptr,store_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_HSLDEOIH)
		{
		status = mbr_wt_hsldeoih(verbose,mbio_ptr,store_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SB2100RW)
		{
		status = mbr_wt_sb2100rw(verbose,mbio_ptr,store_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_EM12DARW)
		{
		status = mbr_wt_em12darw(verbose,mbio_ptr,store_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_MBLDEOIH)
		{
		status = mbr_wt_mbldeoih(verbose,mbio_ptr,store_ptr,error);
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
