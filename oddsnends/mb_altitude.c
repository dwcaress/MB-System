/*--------------------------------------------------------------------
 *    The MB-system:	mb_altitude.c	4/28/98
 *    $Id: mb_altitude.c,v 4.0 1998-10-05 19:16:02 caress Exp $

 *    Copyright (c) 1998 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
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
int mb_altitude(verbose,mbio_ptr,store_ptr,kind,
	transducer_depth,altitude,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*kind;
double	*transducer_depth;
double	*altitude;
int	*error;
{
	static char rcs_id[]="$Id: mb_altitude.c,v 4.0 1998-10-05 19:16:02 caress Exp $";
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
int mb_insert_altitude(verbose,mbio_ptr,store_ptr,
	transducer_depth,altitude,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
double	transducer_depth;
double	altitude;
int	*error;
{
	static char rcs_id[]="$Id: mb_altitude.c,v 4.0 1998-10-05 19:16:02 caress Exp $";
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
