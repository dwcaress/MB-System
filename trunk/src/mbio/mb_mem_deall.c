/*--------------------------------------------------------------------
 *    The MB-system:	mb_mem_deall.c	2/3/93
 *    $Id: mb_mem_deall.c,v 5.1 2002-09-18 23:32:59 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 2000, 2002 by
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
 * mb_mem_deall.c calls the appropriate mbr_ routine for deallocating
 * memory used to read or write data of a particular format.
 *
 * Author:	D. W. Caress
 * Date:	February 3, 1993
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.0  2000/12/01 22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.22  2000/10/11  01:02:30  caress
 * Convert to ANSI C
 *
 * Revision 4.21  2000/09/30  06:26:58  caress
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
 * Revision 4.0  1994/02/21  04:03:53  caress
 * First cut at new version.  No changes.
 *
 * Revision 3.1  1993/05/14  22:36:38  sohara
 * fixed rcs_id message
 *
 * Revision 3.0  1993/04/23  18:16:41  dale
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
int mb_mem_deall(int verbose, char *mbio_ptr, int *error)
{
  static char rcs_id[]="$Id: mb_mem_deall.c,v 5.1 2002-09-18 23:32:59 caress Exp $";
	char	*function_name = "mb_mem_deall";
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
		status = mbr_dem_sbsiomrg(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SBSIOCEN)
		{
		status = mbr_dem_sbsiocen(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SBSIOLSI)
		{
		status = mbr_dem_sbsiolsi(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SBURICEN)
		{
		status = mbr_dem_sburicen(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SBURIVAX)
		{
		status = mbr_dem_sburivax(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SBSIOSWB)
		{
		status = mbr_dem_sbsioswb(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SBIFREMR)
		{
		status = mbr_dem_sbifremr(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_HSLDEDMB)
		{
		status = mbr_dem_hsldedmb(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_HSURICEN)
		{
		status = mbr_dem_hsuricen(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_HSATLRAW)
		{
		status = mbr_dem_hsatlraw(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_HSLDEOIH)
		{
		status = mbr_dem_hsldeoih(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_HSURIVAX)
		{
		status = mbr_dem_hsurivax(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SB2000SB)
		{
		status = mbr_dem_sb2000sb(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SB2000SS)
		{
		status = mbr_dem_sb2000ss(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SB2100RW)
		{
		status = mbr_dem_sb2100rw(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SB2100B1)
		{
		status = mbr_dem_sb2100b1(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SB2100B2)
		{
		status = mbr_dem_sb2100b2(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_EM1000RW)
		{
		status = mbr_dem_em1000rw(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_EM12SRAW)
		{
		status = mbr_dem_em12sraw(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_EM12DRAW)
		{
		status = mbr_dem_em12draw(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_EM12DARW)
		{
		status = mbr_dem_em12darw(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_EM121RAW)
		{
		status = mbr_dem_em121raw(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_EM300RAW)
		{
		status = mbr_dem_em300raw(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_EM300MBA)
		{
		status = mbr_dem_em300mba(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_MR1PRHIG)
		{
		status = mbr_dem_mr1prhig(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_MR1ALDEO)
		{
		status = mbr_dem_mr1aldeo(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_MR1BLDEO)
		{
		status = mbr_dem_mr1bldeo(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_MBLDEOIH)
		{
		status = mbr_dem_mbldeoih(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_CBAT9001)
		{
		status = mbr_dem_cbat9001(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_CBAT8101)
		{
		status = mbr_dem_cbat8101(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_HYPC8101)
		{
		status = mbr_dem_hypc8101(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_BCHRTUNB)
		{
		status = mbr_dem_bchrtunb(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_ELMK2UNB)
		{
		status = mbr_dem_elmk2unb(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_BCHRXUNB)
		{
		status = mbr_dem_bchrxunb(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_HSMDARAW)
		{
		status = mbr_dem_hsmdaraw(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_HSMDLDIH)
		{
		status = mbr_dem_hsmdldih(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_DSL120PF)
		{
		status = mbr_dem_dsl120pf(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_DSL120SF)
		{
		status = mbr_dem_dsl120sf(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_GSFGENMB)
		{
		status = mbr_dem_gsfgenmb(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_MSTIFFSS)
		{
		status = mbr_dem_mstiffss(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_OICMBARI)
		{
		status = mbr_dem_oicmbari(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_OMGHDCSJ)
		{
		status = mbr_dem_omghdcsj(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_MGD77DAT)
		{
		status = mbr_dem_mgd77dat(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_MBARIROV)
		{
		status = mbr_dem_mbarirov(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_MBPRONAV)
		{
		status = mbr_dem_mbpronav(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_ELMK2XSE)
		{
		status = mbr_dem_elmk2xse(verbose,mbio_ptr,error);
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
