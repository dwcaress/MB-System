/*--------------------------------------------------------------------
 *    The MB-system:	mb_ttimes.c	4/9/94
 *    $Id: mb_ttimes.c,v 4.6 1995-11-27 21:49:01 caress Exp $

 *    Copyright (c) 1994 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mb_ttimes.c calls the appropriate mbsys_ routine for 
 * extracting travel times, beam angles, and bad data flags from
 * a stored survey data ping.
 *
 * Author:	D. W. Caress
 * Date:	April 9, 1994
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.5  1995/09/28  18:10:48  caress
 * Various bug fixes working toward release 4.3.
 *
 * Revision 4.4  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.3  1994/11/09  21:40:34  caress
 * Changed ttimes extraction routines to handle forward beam angles
 * so that alongtrack distances can be calculated.
 *
 * Revision 4.2  1994/10/21  12:11:53  caress
 * Release V4.0
 *
 * Revision 4.1  1994/07/29  18:46:51  caress
 * Changes associated with supporting Lynx OS (byte swapped) and
 * using unix second time base (for time_d values).
 *
 * Revision 4.0  1994/04/11  23:37:14  caress
 * Setting version number properly.
 *
 * Revision 1.1  1994/04/11  23:34:41  caress
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

/*--------------------------------------------------------------------*/
int mb_ttimes(verbose,mbio_ptr,store_ptr,kind,nbeams,ttimes,
	angles,angles_forward,angles_null,flags,
	depthadd,ssv,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*kind;
int	*nbeams;
double	*ttimes;
double	*angles;
double	*angles_forward;
double	*angles_null;
int	*flags;
double	*depthadd;
double	*ssv;
int	*error;
{
  static char rcs_id[]="$Id: mb_ttimes.c,v 4.6 1995-11-27 21:49:01 caress Exp $";
	char	*function_name = "mb_ttimes";
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
		status = mbsys_sb_ttimes(verbose,mbio_ptr,store_ptr,
				kind,nbeams,ttimes,
				angles,angles_forward,angles_null,
				flags,depthadd,ssv,error);
		}
	else if (system == MB_SYS_HSDS)
		{
		status = mbsys_hsds_ttimes(verbose,mbio_ptr,store_ptr,
				kind,nbeams,ttimes,
				angles,angles_forward,angles_null,
				flags,depthadd,ssv,error);
		}
	else if (system == MB_SYS_SB2000)
		{
		status = mbsys_sb2000_ttimes(verbose,mbio_ptr,store_ptr,
				kind,nbeams,ttimes,
				angles,angles_forward,angles_null,
				flags,depthadd,ssv,error);
		}
	else if (system == MB_SYS_SB2100)
		{
		status = mbsys_sb2100_ttimes(verbose,mbio_ptr,store_ptr,
				kind,nbeams,ttimes,
				angles,angles_forward,angles_null,
				flags,depthadd,ssv,error);
		}
	else if (system == MB_SYS_SIMRAD)
		{
		status = mbsys_simrad_ttimes(verbose,mbio_ptr,store_ptr,
				kind,nbeams,ttimes,
				angles,angles_forward,angles_null,
				flags,depthadd,ssv,error);
		}
	else if (system == MB_SYS_MR1)
		{
		status = mbsys_mr1_ttimes(verbose,mbio_ptr,store_ptr,
				kind,nbeams,ttimes,
				angles,angles_forward,angles_null,
				flags,depthadd,ssv,error);
		}
	else if (system == MB_SYS_LDEOIH)
		{
		status = mbsys_ldeoih_ttimes(verbose,mbio_ptr,store_ptr,
				kind,nbeams,ttimes,
				angles,angles_forward,angles_null,
				flags,depthadd,ssv,error);
		}
	else if (system == MB_SYS_RESON)
		{
		status = mbsys_reson_ttimes(verbose,mbio_ptr,store_ptr,
				kind,nbeams,ttimes,
				angles,angles_forward,angles_null,
				flags,depthadd,ssv,error);
		}
	else if (system == MB_SYS_ELAC)
		{
		status = mbsys_elac_ttimes(verbose,mbio_ptr,store_ptr,
				kind,nbeams,ttimes,
				angles,angles_forward,angles_null,
				flags,depthadd,ssv,error);
		}
	else if (system == MB_SYS_HSMD)
		{
		status = mbsys_hsmd_ttimes(verbose,mbio_ptr,store_ptr,
				kind,nbeams,ttimes,
				angles,angles_forward,angles_null,
				flags,depthadd,ssv,error);
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
		fprintf(stderr,"dbg2       kind:       %d\n",*kind);
		fprintf(stderr,"dbg2       nbeams:     %d\n",*nbeams);
		fprintf(stderr,"dbg2       depthadd:   %f\n",*depthadd);
		fprintf(stderr,"dbg2       ssv:        %f\n",*ssv);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
