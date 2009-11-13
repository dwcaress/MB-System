/*--------------------------------------------------------------------
 *    The MB-system:	mb_nav.c	8/29/2000
 *    $Id: mb_nav.c,v 4.1 2000-10-11 01:02:30 caress Exp $
 *
 *    Copyright (c) 2000 by
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
 * mb_nav.c calls the appropriate mbsys_ routine for 
 * extracting navigation from a stored navigation record 
 * or survey data ping.
 * 
 * Author:	D. W. Caress
 * Date:	August 29, 2000
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.0  2000/09/30  06:28:42  caress
 * Snapshot for Dale.
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
int mb_extract_nav(int verbose, char *mbio_ptr, char *store_ptr, int *kind,
	int time_i[7], double *time_d, 
	double *navlon, double *navlat,
	double *speed, double *heading, double *draft, 
	double *roll, double *pitch, double *heave, 
	int *error)
{
  static char rcs_id[]="$Id: mb_nav.c,v 4.1 2000-10-11 01:02:30 caress Exp $";
	char	*function_name = "mb_extract_nav";
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
		status = mbsys_sb_extract_nav(verbose,mbio_ptr,store_ptr,
				kind,time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_HSDS)
		{
		status = mbsys_hsds_extract_nav(verbose,mbio_ptr,store_ptr,
				kind,time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_SB2000)
		{
		status = mbsys_sb2000_extract_nav(verbose,mbio_ptr,store_ptr,
				kind,time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_SB2100)
		{
		status = mbsys_sb2100_extract_nav(verbose,mbio_ptr,store_ptr,
				kind,time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_SIMRAD)
		{
		status = mbsys_simrad_extract_nav(verbose,mbio_ptr,store_ptr,
				kind,time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_SIMRAD2)
		{
		status = mbsys_simrad2_extract_nav(verbose,mbio_ptr,store_ptr,
				kind,time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_MR1)
		{
		status = mbsys_mr1_extract_nav(verbose,mbio_ptr,store_ptr,
				kind,time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_MR1B)
		{
		status = mbsys_mr1b_extract_nav(verbose,mbio_ptr,store_ptr,
				kind,time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_LDEOIH)
		{
		status = mbsys_ldeoih_extract_nav(verbose,mbio_ptr,store_ptr,
				kind,time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_RESON)
		{
		status = mbsys_reson_extract_nav(verbose,mbio_ptr,store_ptr,
				kind,time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_ELAC)
		{
		status = mbsys_elac_extract_nav(verbose,mbio_ptr,store_ptr,
				kind,time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_ELACMK2)
		{
		status = mbsys_elacmk2_extract_nav(verbose,mbio_ptr,store_ptr,
				kind,time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_HSMD)
		{
		status = mbsys_hsmd_extract_nav(verbose,mbio_ptr,store_ptr,
				kind,time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_DSL)
		{
		status = mbsys_dsl_extract_nav(verbose,mbio_ptr,store_ptr,
				kind,time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_GSF)
		{
		status = mbsys_gsf_extract_nav(verbose,mbio_ptr,store_ptr,
				kind,time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_MSTIFF)
		{
		status = mbsys_mstiff_extract_nav(verbose,mbio_ptr,store_ptr,
				kind,time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_OIC)
		{
		status = mbsys_oic_extract_nav(verbose,mbio_ptr,store_ptr,
				kind,time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_HDCS)
		{
		status = mbsys_hdcs_extract_nav(verbose,mbio_ptr,store_ptr,
				kind,time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_SINGLEBEAM)
		{
		status = mbsys_singlebeam_extract_nav(verbose,mbio_ptr,store_ptr,
				kind,time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
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
		fprintf(stderr,"dbg2       time_i[0]:     %d\n",time_i[0]);
		fprintf(stderr,"dbg2       time_i[1]:     %d\n",time_i[1]);
		fprintf(stderr,"dbg2       time_i[2]:     %d\n",time_i[2]);
		fprintf(stderr,"dbg2       time_i[3]:     %d\n",time_i[3]);
		fprintf(stderr,"dbg2       time_i[4]:     %d\n",time_i[4]);
		fprintf(stderr,"dbg2       time_i[5]:     %d\n",time_i[5]);
		fprintf(stderr,"dbg2       time_i[6]:     %d\n",time_i[6]);
		fprintf(stderr,"dbg2       time_d:        %f\n",*time_d);
		fprintf(stderr,"dbg2       longitude:     %f\n",*navlon);
		fprintf(stderr,"dbg2       latitude:      %f\n",*navlat);
		fprintf(stderr,"dbg2       speed:         %f\n",*speed);
		fprintf(stderr,"dbg2       heading:       %f\n",*heading);
		fprintf(stderr,"dbg2       draft:         %f\n",*draft);
		fprintf(stderr,"dbg2       roll:          %f\n",*roll);
		fprintf(stderr,"dbg2       pitch:         %f\n",*pitch);
		fprintf(stderr,"dbg2       heave:         %f\n",*heave);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_insert_nav(int verbose, char *mbio_ptr, char *store_ptr,
	int time_i[7], double time_d, 
	double navlon, double navlat,
	double speed, double heading, double draft, 
	double roll, double pitch, double heave, 
	int *error)
{
  static char rcs_id[]="$Id: mb_nav.c,v 4.1 2000-10-11 01:02:30 caress Exp $";
	char	*function_name = "mb_insert_nav";
	int	status;
	int	system;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:       %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:        %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:     %d\n",store_ptr);
		fprintf(stderr,"dbg2       time_i[0]:     %d\n",time_i[0]);
		fprintf(stderr,"dbg2       time_i[1]:     %d\n",time_i[1]);
		fprintf(stderr,"dbg2       time_i[2]:     %d\n",time_i[2]);
		fprintf(stderr,"dbg2       time_i[3]:     %d\n",time_i[3]);
		fprintf(stderr,"dbg2       time_i[4]:     %d\n",time_i[4]);
		fprintf(stderr,"dbg2       time_i[5]:     %d\n",time_i[5]);
		fprintf(stderr,"dbg2       time_i[6]:     %d\n",time_i[6]);
		fprintf(stderr,"dbg2       time_d:        %f\n",time_d);
		fprintf(stderr,"dbg2       longitude:     %f\n",navlon);
		fprintf(stderr,"dbg2       latitude:      %f\n",navlat);
		fprintf(stderr,"dbg2       speed:         %f\n",speed);
		fprintf(stderr,"dbg2       heading:       %f\n",heading);
		fprintf(stderr,"dbg2       draft:         %f\n",draft);
		fprintf(stderr,"dbg2       roll:          %f\n",roll);
		fprintf(stderr,"dbg2       pitch:         %f\n",pitch);
		fprintf(stderr,"dbg2       heave:         %f\n",heave);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get multibeam system id */
	system = mb_system_table[mb_io_ptr->format_num];

	/* call the appropriate mbsys_ extraction routine */
	if (system == MB_SYS_SB)
		{
		status = mbsys_sb_insert_nav(verbose,mbio_ptr,store_ptr,
				time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_HSDS)
		{
		status = mbsys_hsds_insert_nav(verbose,mbio_ptr,store_ptr,
				time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_SB2000)
		{
		status = mbsys_sb2000_insert_nav(verbose,mbio_ptr,store_ptr,
				time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_SB2100)
		{
		status = mbsys_sb2100_insert_nav(verbose,mbio_ptr,store_ptr,
				time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_SIMRAD)
		{
		status = mbsys_simrad_insert_nav(verbose,mbio_ptr,store_ptr,
				time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_SIMRAD2)
		{
		status = mbsys_simrad2_insert_nav(verbose,mbio_ptr,store_ptr,
				time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_MR1)
		{
		status = mbsys_mr1_insert_nav(verbose,mbio_ptr,store_ptr,
				time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_MR1B)
		{
		status = mbsys_mr1b_insert_nav(verbose,mbio_ptr,store_ptr,
				time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_LDEOIH)
		{
		status = mbsys_ldeoih_insert_nav(verbose,mbio_ptr,store_ptr,
				time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_RESON)
		{
		status = mbsys_reson_insert_nav(verbose,mbio_ptr,store_ptr,
				time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_ELAC)
		{
		status = mbsys_elac_insert_nav(verbose,mbio_ptr,store_ptr,
				time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_ELACMK2)
		{
		status = mbsys_elacmk2_insert_nav(verbose,mbio_ptr,store_ptr,
				time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_HSMD)
		{
		status = mbsys_hsmd_insert_nav(verbose,mbio_ptr,store_ptr,
				time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_DSL)
		{
		status = mbsys_dsl_insert_nav(verbose,mbio_ptr,store_ptr,
				time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_GSF)
		{
		status = mbsys_gsf_insert_nav(verbose,mbio_ptr,store_ptr,
				time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_MSTIFF)
		{
		status = mbsys_mstiff_insert_nav(verbose,mbio_ptr,store_ptr,
				time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_OIC)
		{
		status = mbsys_oic_insert_nav(verbose,mbio_ptr,store_ptr,
				time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_HDCS)
		{
		status = mbsys_hdcs_insert_nav(verbose,mbio_ptr,store_ptr,
				time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
		}
	else if (system == MB_SYS_SINGLEBEAM)
		{
		status = mbsys_singlebeam_insert_nav(verbose,mbio_ptr,store_ptr,
				time_i,time_d,
				navlon,navlat,
				speed,heading,draft,
				roll,pitch,heave, 
				error);
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
