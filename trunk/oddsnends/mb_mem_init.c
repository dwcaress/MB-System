/*--------------------------------------------------------------------
 *    The MB-system:	mb_mem_init.c	2/3/93
 *    $Id: mb_mem_init.c,v 4.8 1996-04-22 13:21:19 caress Exp $
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
 * mb_mem_init.c calls the appropriate mbr_ routine for allocating
 * memory needed to read or write data of a particular format.
 *
 * Author:	D. W. Caress
 * Date:	February 3, 1993
 *
 * $Log: not supported by cvs2svn $
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
 * Revision 4.4  1994/03/05  22:51:44  caress
 * Added ability to handle Simrad EM12 system and
 * format MBF_EM12DARW.
 *
 * Revision 4.3  1994/03/05  02:09:29  caress
 * Altered to add MBF_SB2100RW format.
 *
 * Revision 4.2  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.1  1994/03/03  03:15:16  caress
 * Not sure what was changed.
 *
 * Revision 4.0  1994/02/21  04:04:35  caress
 * First cut at new version.  No changes.
 *
 * Revision 3.1  1993/05/14  22:37:06  sohara
 * fixed rcs_id message
 *
 * Revision 3.0  1993/04/23  18:18:02  dale
 * Initial Version
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
int mb_mem_init(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
  static char rcs_id[]="$Id: mb_mem_init.c,v 4.8 1996-04-22 13:21:19 caress Exp $";
	char	*function_name = "mb_mem_init";
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

	/* print out debug statements */
	if (verbose >= 4)
		{
		fprintf(stderr,"\ndbg4  Format values in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg4       format:     %d\n",
			mb_io_ptr->format);
		fprintf(stderr,"dbg4       format_num: %d\n",
			mb_io_ptr->format_num);
		}

	/* call the appropriate mbr_ read and translate routine */
	if (mb_io_ptr->format == MBF_SBSIOMRG)
		{
		status = mbr_alm_sbsiomrg(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SBSIOCEN)
		{
		status = mbr_alm_sbsiocen(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SBSIOLSI)
		{
		status = mbr_alm_sbsiolsi(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SBURICEN)
		{
		status = mbr_alm_sburicen(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SBURIVAX)
		{
		status = mbr_alm_sburivax(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SBSIOSWB)
		{
		status = mbr_alm_sbsioswb(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SBIFREMR)
		{
		status = mbr_alm_sbifremr(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_HSLDEDMB)
		{
		status = mbr_alm_hsldedmb(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_HSURICEN)
		{
		status = mbr_alm_hsuricen(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_HSATLRAW)
		{
		status = mbr_alm_hsatlraw(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_HSLDEOIH)
		{
		status = mbr_alm_hsldeoih(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_HSURIVAX)
		{
		status = mbr_alm_hsurivax(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SB2000SB)
		{
		status = mbr_alm_sb2000sb(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SB2000SS)
		{
		status = mbr_alm_sb2000ss(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SB2100RW)
		{
		status = mbr_alm_sb2100rw(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_EM1000RW)
		{
		status = mbr_alm_em1000rw(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_EM12DARW)
		{
		status = mbr_alm_em12darw(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_MR1PRHIG)
		{
		status = mbr_alm_mr1prhig(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_MR1ALDEO)
		{
		status = mbr_alm_mr1aldeo(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_MR1BLDEO)
		{
		status = mbr_alm_mr1bldeo(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_MBLDEOIH)
		{
		status = mbr_alm_mbldeoih(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_CBAT9001)
		{
		status = mbr_alm_cbat9001(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_BCHRTUNB)
		{
		status = mbr_alm_bchrtunb(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_HSMDARAW)
		{
		status = mbr_alm_hsmdaraw(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_HSMDLDIH)
		{
		status = mbr_alm_hsmdldih(verbose,mbio_ptr,error);
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
