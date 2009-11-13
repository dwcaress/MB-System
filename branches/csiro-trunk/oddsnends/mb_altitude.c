/*--------------------------------------------------------------------
 *    The MB-system:	mb_altitude.c	4/28/98
 *    $Id: mb_altitude.c,v 4.6 2000-10-11 01:02:30 caress Exp $

 *    Copyright (c) 1998, 2000 by
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
 * mb_altitude.c calls the appropriate mbsys_ routine for 
 * extracting the sonar transducer depth below the seafloor
 * and the the sonar transducer altitude above the seafloor
 * from a stored survey data ping. These values are useful for
 * sidescan processing applications. Both transducer depths and
 * altitudes are reported in meters.
 *
 * Author:	D. W. Caress
 * Date:	April 28, 1998
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.5  2000/09/30  06:26:58  caress
 * Snapshot for Dale.
 *
 * Revision 4.4  1999/08/08  04:12:45  caress
 * Added ELMK2XSE format.
 *
 * Revision 4.3  1999/07/16  19:24:15  caress
 * Yet another version.
 *
 * Revision 4.2  1999/03/31  18:11:35  caress
 * MB-System 4.6beta7
 *
 * Revision 4.1  1998/12/17  22:56:15  caress
 * MB-System version 4.6beta4
 *
 * Revision 4.0  1998/10/05  19:16:02  caress
 * MB-System version 4.6beta
 *
 * Revision 1.1  1998/10/05  18:22:40  caress
 * Initial revision
 *
 * Revision 1.1  1998/10/05  18:22:40  caress
 * Initial revision
 *
 * Revision 1.1  1998/10/05  17:46:15  caress
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
int mb_altitude(int verbose, char *mbio_ptr, char *store_ptr,
		int *kind,
		double *transducer_depth, double *altitude,
		int *error)
{
	static char rcs_id[]="$Id: mb_altitude.c,v 4.6 2000-10-11 01:02:30 caress Exp $";
	char	*function_name = "mb_altitude";
	int	status;
	int	system;
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

	/* get multibeam system id */
	system = mb_system_table[mb_io_ptr->format_num];

	/* call the appropriate mbsys_ extraction routine */
	if (system == MB_SYS_SB)
		{
		status = mbsys_sb_altitude(verbose,mbio_ptr,store_ptr,
				kind,transducer_depth,altitude,error);
		}
	else if (system == MB_SYS_HSDS)
		{
		status = mbsys_hsds_altitude(verbose,mbio_ptr,store_ptr,
				kind,transducer_depth,altitude,error);
		}
	else if (system == MB_SYS_SB2000)
		{
		status = mbsys_sb2000_altitude(verbose,mbio_ptr,store_ptr,
				kind,transducer_depth,altitude,error);
		}
	else if (system == MB_SYS_SB2100)
		{
		status = mbsys_sb2100_altitude(verbose,mbio_ptr,store_ptr,
				kind,transducer_depth,altitude,error);
		}
	else if (system == MB_SYS_SIMRAD)
		{
		status = mbsys_simrad_altitude(verbose,mbio_ptr,store_ptr,
				kind,transducer_depth,altitude,error);
		}
	else if (system == MB_SYS_SIMRAD2)
		{
		status = mbsys_simrad2_altitude(verbose,mbio_ptr,store_ptr,
				kind,transducer_depth,altitude,error);
		}
	else if (system == MB_SYS_MR1)
		{
		status = mbsys_mr1_altitude(verbose,mbio_ptr,store_ptr,
				kind,transducer_depth,altitude,error);
		}
	else if (system == MB_SYS_MR1B)
		{
		status = mbsys_mr1b_altitude(verbose,mbio_ptr,store_ptr,
				kind,transducer_depth,altitude,error);
		}
	else if (system == MB_SYS_LDEOIH)
		{
		status = mbsys_ldeoih_altitude(verbose,mbio_ptr,store_ptr,
				kind,transducer_depth,altitude,error);
		}
	else if (system == MB_SYS_RESON)
		{
		status = mbsys_reson_altitude(verbose,mbio_ptr,store_ptr,
				kind,transducer_depth,altitude,error);
		}
	else if (system == MB_SYS_ELAC)
		{
		status = mbsys_elac_altitude(verbose,mbio_ptr,store_ptr,
				kind,transducer_depth,altitude,error);
		}
	else if (system == MB_SYS_ELACMK2)
		{
		status = mbsys_elacmk2_altitude(verbose,mbio_ptr,store_ptr,
				kind,transducer_depth,altitude,error);
		}
	else if (system == MB_SYS_HSMD)
		{
		status = mbsys_hsmd_altitude(verbose,mbio_ptr,store_ptr,
				kind,transducer_depth,altitude,error);
		}
	else if (system == MB_SYS_DSL)
		{
		status = mbsys_dsl_altitude(verbose,mbio_ptr,store_ptr,
				kind,transducer_depth,altitude,error);
		}
	else if (system == MB_SYS_GSF)
		{
		status = mbsys_gsf_altitude(verbose,mbio_ptr,store_ptr,
				kind,transducer_depth,altitude,error);
		}
	else if (system == MB_SYS_MSTIFF)
		{
		status = mbsys_mstiff_altitude(verbose,mbio_ptr,store_ptr,
				kind,transducer_depth,altitude,error);
		}
	else if (system == MB_SYS_OIC)
		{
		status = mbsys_oic_altitude(verbose,mbio_ptr,store_ptr,
				kind,transducer_depth,altitude,error);
		}
	else if (system == MB_SYS_HDCS)
		{
		status = mbsys_hdcs_altitude(verbose,mbio_ptr,store_ptr,
				kind,transducer_depth,altitude,error);
		}
	else if (system == MB_SYS_SINGLEBEAM)
		{
		status = mbsys_singlebeam_altitude(verbose,mbio_ptr,store_ptr,
				kind,transducer_depth,altitude,error);
		}
	else if (system == MB_SYS_XSE)
		{
		status = mbsys_xse_altitude(verbose,mbio_ptr,store_ptr,
				kind,transducer_depth,altitude,error);
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
		fprintf(stderr,"dbg2       kind:              %d\n",*kind);
		fprintf(stderr,"dbg2       transducer_depth:  %f\n",*transducer_depth);
		fprintf(stderr,"dbg2       altitude:          %f\n",*altitude);
		fprintf(stderr,"dbg2       error:             %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:            %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_insert_altitude(int verbose, char *mbio_ptr, char *store_ptr,
		double transducer_depth, double altitude,
		int *error)
{
	static char rcs_id[]="$Id: mb_altitude.c,v 4.6 2000-10-11 01:02:30 caress Exp $";
	char	*function_name = "mb_insert_altitude";
	int	status;
	int	system;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:            %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:         %d\n",store_ptr);
		fprintf(stderr,"dbg2       transducer_depth:  %f\n",transducer_depth);
		fprintf(stderr,"dbg2       altitude:          %f\n",altitude);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get multibeam system id */
	system = mb_system_table[mb_io_ptr->format_num];

	/* call the appropriate mbsys_ extraction routine */
	if (system == MB_SYS_SB)
		{
		}
	else if (system == MB_SYS_HSDS)
		{
		}
	else if (system == MB_SYS_SB2000)
		{
		}
	else if (system == MB_SYS_SB2100)
		{
		}
	else if (system == MB_SYS_SIMRAD)
		{
		}
	else if (system == MB_SYS_SIMRAD2)
		{
		}
	else if (system == MB_SYS_MR1)
		{
		}
	else if (system == MB_SYS_MR1B)
		{
		}
	else if (system == MB_SYS_LDEOIH)
		{
		status = mbsys_ldeoih_insert_altitude(verbose,mbio_ptr,store_ptr,
				transducer_depth,altitude,error);
		}
	else if (system == MB_SYS_RESON)
		{
		}
	else if (system == MB_SYS_ELAC)
		{
		}
	else if (system == MB_SYS_ELACMK2)
		{
		}
	else if (system == MB_SYS_HSMD)
		{
		}
	else if (system == MB_SYS_DSL)
		{
		}
	else if (system == MB_SYS_GSF)
		{
		status = mbsys_gsf_insert_altitude(verbose,mbio_ptr,store_ptr,
				transducer_depth,altitude,error);
		}
	else if (system == MB_SYS_MSTIFF)
		{
		}
	else if (system == MB_SYS_OIC)
		{
		status = mbsys_oic_insert_altitude(verbose,mbio_ptr,store_ptr,
				transducer_depth,altitude,error);
		}
	else if (system == MB_SYS_HDCS)
		{
		status = mbsys_hdcs_insert_altitude(verbose,mbio_ptr,store_ptr,
				transducer_depth,altitude,error);
		}
	else if (system == MB_SYS_SINGLEBEAM)
		{
		status = mbsys_singlebeam_insert_altitude(verbose,mbio_ptr,store_ptr,
				transducer_depth,altitude,error);
		}
	else if (system == MB_SYS_XSE)
		{
		status = mbsys_xse_insert_altitude(verbose,mbio_ptr,store_ptr,
				transducer_depth,altitude,error);
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
		fprintf(stderr,"dbg2       error:             %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:            %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
