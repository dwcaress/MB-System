/*--------------------------------------------------------------------
 *    The MB-system:	mbr_mr1bldeo.c	10/24/95
 *	$Id$
 *
 *    Copyright (c) 1994-2009 by
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
 * mbr_mr1bldeo.c contains the functions for reading and writing
 * multibeam data in the MR1BLDEO format.  
 * These functions include:
 *   mbr_alm_mr1bldeo	- allocate read/write memory
 *   mbr_dem_mr1bldeo	- deallocate read/write memory
 *   mbr_rt_mr1bldeo	- read and translate data
 *   mbr_wt_mr1bldeo	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	October 24, 1995
 * $Log: mbr_mr1bldeo.c,v $
 * Revision 5.8  2008/07/10 06:43:40  caress
 * Preparing for 5.1.1beta20
 *
 * Revision 5.7  2005/11/05 00:48:05  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.6  2003/05/20 18:05:32  caress
 * Added svp_source to data source parameters.
 *
 * Revision 5.5  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.4  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.3  2001/07/20 00:32:54  caress
 * Release 5.0.beta03
 *
 * Revision 5.2  2001/03/22  20:50:02  caress
 * Trying to make version 5.0.beta0
 *
 * Revision 5.1  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 1.9  2000/10/11  01:03:21  caress
 * Convert to ANSI C
 *
 * Revision 1.8  2000/10/03  21:48:03  caress
 * Snapshot for Dale.
 *
 * Revision 1.7  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 1.6  1999/05/05  20:32:19  caress
 * Fixed bugs in handling updated bathymetry through mb_put_all call.
 *
 * Revision 1.5  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 1.4  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 1.3  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 1.3  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 1.2  1996/04/22  11:16:30  caress
 * DTR define now in mb_io.h
 *
 * Revision 1.1  1996/03/12  17:18:14  caress
 * Initial revision
 *
 * Revision 1.1  1996/01/26  21:23:30  caress
 * Initial revision
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <rpc/types.h>
#include <rpc/xdr.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_io.h"
#include "../../include/mb_define.h"
#include "../../include/mbsys_mr1b.h"
#include "../../include/mbf_mr1bldeo.h"

/* essential function prototypes */
int mbr_register_mr1bldeo(int verbose, void *mbio_ptr, int *error);
int mbr_info_mr1bldeo(int verbose, 
			int *system, 
			int *beams_bath_max, 
			int *beams_amp_max, 
			int *pixels_ss_max, 
			char *format_name, 
			char *system_name, 
			char *format_description, 
			int *numfile, 
			int *filetype, 
			int *variable_beams, 
			int *traveltime, 
			int *beam_flagging, 
			int *nav_source, 
			int *heading_source, 
			int *vru_source, 
			int *svp_source, 
			double *beamwidth_xtrack, 
			double *beamwidth_ltrack, 
			int *error);
int mbr_alm_mr1bldeo(int verbose, void *mbio_ptr, int *error);
int mbr_dem_mr1bldeo(int verbose, void *mbio_ptr, int *error);
int mbr_zero_mr1bldeo(int verbose, struct mbf_mr1bldeo_struct *data, int *error);
int mbr_rt_mr1bldeo(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_mr1bldeo(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_mr1bldeo_rd_data(int verbose, void *mbio_ptr, int *error);
int mbr_mr1bldeo_rd_hdr(int verbose, XDR *xdrs, 
		struct mbf_mr1bldeo_struct *data, char **hdr_comment,
		int *error);
int mbr_mr1bldeo_rd_ping(int verbose, XDR *xdrs, 
		struct mbf_mr1bldeo_struct *data, int *error);
int mbr_mr1bldeo_wr_data(int verbose, void *mbio_ptr, struct mbf_mr1bldeo_struct *data, int *error);
int mbr_mr1bldeo_wr_hdr(int verbose, XDR *xdrs, 
		struct mbf_mr1bldeo_struct *data, char **hdr_comment, int *error);
int mbr_mr1bldeo_wr_ping(int verbose, XDR *xdrs, struct mbf_mr1bldeo_struct *data, int *error);

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/
int mbr_register_mr1bldeo(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_register_mr1bldeo";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		}

	/* get mb_io_ptr */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set format info parameters */
	status = mbr_info_mr1bldeo(verbose, 
			&mb_io_ptr->system, 
			&mb_io_ptr->beams_bath_max, 
			&mb_io_ptr->beams_amp_max, 
			&mb_io_ptr->pixels_ss_max, 
			mb_io_ptr->format_name, 
			mb_io_ptr->system_name, 
			mb_io_ptr->format_description, 
			&mb_io_ptr->numfile, 
			&mb_io_ptr->filetype, 
			&mb_io_ptr->variable_beams, 
			&mb_io_ptr->traveltime, 
			&mb_io_ptr->beam_flagging, 
			&mb_io_ptr->nav_source, 
			&mb_io_ptr->heading_source, 
			&mb_io_ptr->vru_source, 
			&mb_io_ptr->svp_source, 
			&mb_io_ptr->beamwidth_xtrack, 
			&mb_io_ptr->beamwidth_ltrack, 
			error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_mr1bldeo;
	mb_io_ptr->mb_io_format_free = &mbr_dem_mr1bldeo; 
	mb_io_ptr->mb_io_store_alloc = &mbsys_mr1b_alloc; 
	mb_io_ptr->mb_io_store_free = &mbsys_mr1b_deall; 
	mb_io_ptr->mb_io_read_ping = &mbr_rt_mr1bldeo; 
	mb_io_ptr->mb_io_write_ping = &mbr_wt_mr1bldeo; 
	mb_io_ptr->mb_io_dimensions = &mbsys_mr1b_dimensions; 
	mb_io_ptr->mb_io_extract = &mbsys_mr1b_extract; 
	mb_io_ptr->mb_io_insert = &mbsys_mr1b_insert; 
	mb_io_ptr->mb_io_extract_nav = &mbsys_mr1b_extract_nav; 
	mb_io_ptr->mb_io_insert_nav = &mbsys_mr1b_insert_nav; 
	mb_io_ptr->mb_io_extract_altitude = &mbsys_mr1b_extract_altitude; 
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = NULL; 
	mb_io_ptr->mb_io_insert_svp = NULL;
	mb_io_ptr->mb_io_ttimes = &mbsys_mr1b_ttimes; 
	mb_io_ptr->mb_io_copyrecord = &mbsys_mr1b_copy; 
	mb_io_ptr->mb_io_extract_rawss = NULL; 
	mb_io_ptr->mb_io_insert_rawss = NULL; 

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");	
		fprintf(stderr,"dbg2       system:             %d\n",mb_io_ptr->system);
		fprintf(stderr,"dbg2       beams_bath_max:     %d\n",mb_io_ptr->beams_bath_max);
		fprintf(stderr,"dbg2       beams_amp_max:      %d\n",mb_io_ptr->beams_amp_max);
		fprintf(stderr,"dbg2       pixels_ss_max:      %d\n",mb_io_ptr->pixels_ss_max);
		fprintf(stderr,"dbg2       format_name:        %s\n",mb_io_ptr->format_name);
		fprintf(stderr,"dbg2       system_name:        %s\n",mb_io_ptr->system_name);
		fprintf(stderr,"dbg2       format_description: %s\n",mb_io_ptr->format_description);
		fprintf(stderr,"dbg2       numfile:            %d\n",mb_io_ptr->numfile);
		fprintf(stderr,"dbg2       filetype:           %d\n",mb_io_ptr->filetype);
		fprintf(stderr,"dbg2       variable_beams:     %d\n",mb_io_ptr->variable_beams);
		fprintf(stderr,"dbg2       traveltime:         %d\n",mb_io_ptr->traveltime);
		fprintf(stderr,"dbg2       beam_flagging:      %d\n",mb_io_ptr->beam_flagging);
		fprintf(stderr,"dbg2       nav_source:         %d\n",mb_io_ptr->nav_source);
		fprintf(stderr,"dbg2       heading_source:     %d\n",mb_io_ptr->heading_source);
		fprintf(stderr,"dbg2       vru_source:         %d\n",mb_io_ptr->vru_source);
		fprintf(stderr,"dbg2       svp_source:         %d\n",mb_io_ptr->svp_source);
		fprintf(stderr,"dbg2       beamwidth_xtrack:   %f\n",mb_io_ptr->beamwidth_xtrack);
		fprintf(stderr,"dbg2       beamwidth_ltrack:   %f\n",mb_io_ptr->beamwidth_ltrack);
		fprintf(stderr,"dbg2       format_alloc:       %ld\n",(size_t)mb_io_ptr->mb_io_format_alloc);
		fprintf(stderr,"dbg2       format_free:        %ld\n",(size_t)mb_io_ptr->mb_io_format_free);
		fprintf(stderr,"dbg2       store_alloc:        %ld\n",(size_t)mb_io_ptr->mb_io_store_alloc);
		fprintf(stderr,"dbg2       store_free:         %ld\n",(size_t)mb_io_ptr->mb_io_store_free);
		fprintf(stderr,"dbg2       read_ping:          %ld\n",(size_t)mb_io_ptr->mb_io_read_ping);
		fprintf(stderr,"dbg2       write_ping:         %ld\n",(size_t)mb_io_ptr->mb_io_write_ping);
		fprintf(stderr,"dbg2       extract:            %ld\n",(size_t)mb_io_ptr->mb_io_extract);
		fprintf(stderr,"dbg2       insert:             %ld\n",(size_t)mb_io_ptr->mb_io_insert);
		fprintf(stderr,"dbg2       extract_nav:        %ld\n",(size_t)mb_io_ptr->mb_io_extract_nav);
		fprintf(stderr,"dbg2       insert_nav:         %ld\n",(size_t)mb_io_ptr->mb_io_insert_nav);
		fprintf(stderr,"dbg2       extract_altitude:   %ld\n",(size_t)mb_io_ptr->mb_io_extract_altitude);
		fprintf(stderr,"dbg2       insert_altitude:    %ld\n",(size_t)mb_io_ptr->mb_io_insert_altitude);
		fprintf(stderr,"dbg2       extract_svp:        %ld\n",(size_t)mb_io_ptr->mb_io_extract_svp);
		fprintf(stderr,"dbg2       insert_svp:         %ld\n",(size_t)mb_io_ptr->mb_io_insert_svp);
		fprintf(stderr,"dbg2       ttimes:             %ld\n",(size_t)mb_io_ptr->mb_io_ttimes);
		fprintf(stderr,"dbg2       extract_rawss:      %ld\n",(size_t)mb_io_ptr->mb_io_extract_rawss);
		fprintf(stderr,"dbg2       insert_rawss:       %ld\n",(size_t)mb_io_ptr->mb_io_insert_rawss);
		fprintf(stderr,"dbg2       copyrecord:         %ld\n",(size_t)mb_io_ptr->mb_io_copyrecord);
		fprintf(stderr,"dbg2       error:              %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		}

	/* return status */
	return(status);
}

/*--------------------------------------------------------------------*/
int mbr_info_mr1bldeo(int verbose, 
			int *system, 
			int *beams_bath_max, 
			int *beams_amp_max, 
			int *pixels_ss_max, 
			char *format_name, 
			char *system_name, 
			char *format_description, 
			int *numfile, 
			int *filetype, 
			int *variable_beams, 
			int *traveltime, 
			int *beam_flagging, 
			int *nav_source, 
			int *heading_source, 
			int *vru_source, 
			int *svp_source, 
			double *beamwidth_xtrack, 
			double *beamwidth_ltrack, 
			int *error)
{
	char	*function_name = "mbr_info_mr1bldeo";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		}

	/* set format info parameters */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	*system = MB_SYS_MR1B;
	*beams_bath_max = 153;
	*beams_amp_max = 0;
	*pixels_ss_max = 4003;
	strncpy(format_name, "MR1BLDEO", MB_NAME_LENGTH);
	strncpy(system_name, "MR1B", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_MR1BLDEO\nInformal Description: L-DEO small MR1 post processed format with travel times\nAttributes:           L-DEO MR1, bathymetry and sidescan,\n                      variable beams and pixels, xdr binary, \n                      L-DEO.\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_XDR;
	*variable_beams = MB_NO;
	*traveltime = MB_YES;
	*beam_flagging = MB_YES;
	*nav_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*svp_source = MB_DATA_NONE;
	*beamwidth_xtrack = 0.0;
	*beamwidth_ltrack = 2.0;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");	
		fprintf(stderr,"dbg2       system:             %d\n",*system);
		fprintf(stderr,"dbg2       beams_bath_max:     %d\n",*beams_bath_max);
		fprintf(stderr,"dbg2       beams_amp_max:      %d\n",*beams_amp_max);
		fprintf(stderr,"dbg2       pixels_ss_max:      %d\n",*pixels_ss_max);
		fprintf(stderr,"dbg2       format_name:        %s\n",format_name);
		fprintf(stderr,"dbg2       system_name:        %s\n",system_name);
		fprintf(stderr,"dbg2       format_description: %s\n",format_description);
		fprintf(stderr,"dbg2       numfile:            %d\n",*numfile);
		fprintf(stderr,"dbg2       filetype:           %d\n",*filetype);
		fprintf(stderr,"dbg2       variable_beams:     %d\n",*variable_beams);
		fprintf(stderr,"dbg2       traveltime:         %d\n",*traveltime);
		fprintf(stderr,"dbg2       beam_flagging:      %d\n",*beam_flagging);
		fprintf(stderr,"dbg2       nav_source:         %d\n",*nav_source);
		fprintf(stderr,"dbg2       heading_source:     %d\n",*heading_source);
		fprintf(stderr,"dbg2       vru_source:         %d\n",*vru_source);
		fprintf(stderr,"dbg2       svp_source:         %d\n",*svp_source);
		fprintf(stderr,"dbg2       beamwidth_xtrack:   %f\n",*beamwidth_xtrack);
		fprintf(stderr,"dbg2       beamwidth_ltrack:   %f\n",*beamwidth_ltrack);
		fprintf(stderr,"dbg2       error:              %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_alm_mr1bldeo(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_alm_mr1bldeo";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mr1bldeo_struct *data;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %ld\n",(size_t)mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set initial status */
	status = MB_SUCCESS;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = sizeof(struct mbf_mr1bldeo_struct);
	mb_io_ptr->data_structure_size = 0;
	status = mb_mallocd(verbose,__FILE__,__LINE__,mb_io_ptr->structure_size,
				(void **)&mb_io_ptr->raw_data,error);
	status = mb_mallocd(verbose,__FILE__,__LINE__,sizeof(struct mbsys_mr1b_struct),
				(void **)&mb_io_ptr->store_data,error);

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	data = (struct mbf_mr1bldeo_struct *) mb_io_ptr->raw_data;

	/* initialize everything to zeros */
	mbr_zero_mr1bldeo(verbose,data,error);
	mb_io_ptr->fileheader = MB_NO;
	mb_io_ptr->hdr_comment_size = 0;
	mb_io_ptr->hdr_comment = NULL;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_dem_mr1bldeo(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_mr1bldeo";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %ld\n",(size_t)mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* deallocate memory for data descriptor */
	status = mb_freed(verbose,__FILE__, __LINE__, (void **)&mb_io_ptr->raw_data,error);
	status = mb_freed(verbose,__FILE__, __LINE__, (void **)&mb_io_ptr->store_data,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_zero_mr1bldeo(int verbose, struct mbf_mr1bldeo_struct *data, int *error)
{
	char	*function_name = "mbr_zero_mr1bldeo";
	int	status = MB_SUCCESS;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       data:       %ld\n",(size_t)data);
		}

	/* initialize everything to zeros */
	if (data != NULL)
		{
		/* type of data record */
		data->kind = MB_DATA_NONE;

		/* file header info */
		data->mf_magic = 6666;
		data->mf_count = 0;
		if (data->mf_log == NULL)

		/* ping header */
		data->sec = 0;
		data->usec = 0;
		data->png_lon = 0.0;
		data->png_lat = 0.0;
		data->png_course = 0.0;
		data->png_compass = 0.0;
		data->png_prdepth = 0.0;
		data->png_alt = 0.0;
		data->png_pitch = 0.0;
		data->png_roll = 0.0;
		data->png_temp = 0.0;
		data->png_atssincr = 0.0;
		data->png_tt = 0.0;

		/* port settings */
		data->port_trans[0] = 0.0;
		data->port_trans[1] = 0.0;
		data->port_gain = 0.0;
		data->port_pulse = 0.0;
		data->port_btycount = 0;
		data->port_btypad = 0;
		data->port_ssoffset = 0.0;
		data->port_sscount = 0;
		data->port_sspad = 0;

		/* starboard settings */
		data->stbd_trans[0] = 0.0;
		data->stbd_trans[1] = 0.0;
		data->stbd_gain = 0.0;
		data->stbd_pulse = 0.0;
		data->stbd_btycount = 0;
		data->stbd_btypad = 0;
		data->stbd_ssoffset = 0.0;
		data->stbd_sscount = 0;
		data->stbd_sspad = 0;

		/* bathymetry */
		for (i=0;i<MBF_MR1BLDEO_BEAMS_SIDE;i++)
			{
			data->bath_acrosstrack_port[i] = 0.0;
			data->bath_port[i] = 0.0;
			data->tt_port[i] = 0.0;
			data->angle_port[i] = 0.0;
			data->bath_acrosstrack_stbd[i] = 0.0;
			data->bath_stbd[i] = 0.0;
			data->tt_stbd[i] = 0.0;
			data->angle_stbd[i] = 0.0;
			}

		/* sidescan */
		for (i=0;i<MBF_MR1BLDEO_PIXELS_SIDE;i++)
			{
			data->ss_port[i] = 0.0;
			data->ss_stbd[i] = 0.0;
			}

		/* comment */
		strncpy(data->comment,"\0",MBF_MR1BLDEO_MAXLINE);

		}

	/* assume success */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_rt_mr1bldeo(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_mr1bldeo";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mr1bldeo_struct *data;
	struct mbsys_mr1b_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %ld\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %ld\n",(size_t)store_ptr);
		}

	/* get pointers to mbio descriptor and data structures */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	data = (struct mbf_mr1bldeo_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_mr1b_struct *) store_ptr;

	/* read next data from file */
	status = mbr_mr1bldeo_rd_data(verbose,mbio_ptr,error);

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = data->kind;

	/* translate values to mr1b data storage structure */
	if (status == MB_SUCCESS
		&& store != NULL)
		{
		/* type of data record */
		store->kind = data->kind;

		/* file header info */
		store->mf_magic = data->mf_magic;
		store->mf_count = data->mf_count;

		/* ping header */
		store->sec = data->sec;
		store->usec = data->usec;
		store->png_lon = data->png_lon;
		store->png_lat = data->png_lat;
		store->png_course = data->png_course;
		store->png_compass = data->png_compass;
		store->png_prdepth = data->png_prdepth;
		store->png_alt = data->png_alt;
		store->png_pitch = data->png_pitch;
		store->png_roll = data->png_roll;
		store->png_temp = data->png_temp;
		store->png_atssincr = data->png_atssincr;
		store->png_tt = data->png_tt;

		/* port settings */
		store->port_trans[0] = data->port_trans[0];
		store->port_trans[1] = data->port_trans[1];
		store->port_gain = data->port_gain;
		store->port_pulse = data->port_pulse;
		store->port_btycount = data->port_btycount;
		store->port_btypad = data->port_btypad;
		store->port_ssoffset = data->port_ssoffset;
		store->port_sscount = data->port_sscount;
		store->port_sspad = data->port_sspad;

		/* starboard settings */
		store->stbd_trans[0] = data->stbd_trans[0];
		store->stbd_trans[1] = data->stbd_trans[1];
		store->stbd_gain = data->stbd_gain;
		store->stbd_pulse = data->stbd_pulse;
		store->stbd_btycount = data->stbd_btycount;
		store->stbd_btypad = data->stbd_btypad;
		store->stbd_ssoffset = data->stbd_ssoffset;
		store->stbd_sscount = data->stbd_sscount;
		store->stbd_sspad = data->stbd_sspad;

		/* bathymetry */
		for (i=0;i<store->port_btycount;i++)
			{
			store->bath_acrosstrack_port[i] 
				= data->bath_acrosstrack_port[i];
			store->bath_port[i] = data->bath_port[i];
			store->tt_port[i] = data->tt_port[i];
			store->angle_port[i] = data->angle_port[i];
			}
		for (i=0;i<store->stbd_btycount;i++)
			{
			store->bath_acrosstrack_stbd[i] 
				= data->bath_acrosstrack_stbd[i];
			store->bath_stbd[i] = data->bath_stbd[i];
			store->tt_stbd[i] = data->tt_stbd[i];
			store->angle_stbd[i] = data->angle_stbd[i];
			}

		/* sidescan */
		for (i=0;i<store->port_sscount;i++)
			{
			store->ss_port[i] = data->ss_port[i];
			}
		for (i=0;i<store->stbd_sscount;i++)
			{
			store->ss_stbd[i] = data->ss_stbd[i];
			}

		/* comment */
		strncpy(store->comment,data->comment,MBF_MR1BLDEO_MAXLINE);

		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_wt_mr1bldeo(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_mr1bldeo";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mr1bldeo_struct *data;
	struct mbsys_mr1b_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %ld\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %ld\n",(size_t)store_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	data = (struct mbf_mr1bldeo_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_mr1b_struct *) store_ptr;

	/* first translate values from data storage structure */
	if (store != NULL)
		{
		/* type of data record */
		data->kind = store->kind;

		/* file header info */
		data->mf_magic = store->mf_magic;
		data->mf_count = store->mf_count;

		/* ping header */
		data->sec = store->sec;
		data->usec = store->usec;
		data->png_lon = store->png_lon;
		data->png_lat = store->png_lat;
		data->png_course = store->png_course;
		data->png_compass = store->png_compass;
		data->png_prdepth = store->png_prdepth;
		data->png_alt = store->png_alt;
		data->png_pitch = store->png_pitch;
		data->png_roll = store->png_roll;
		data->png_temp = store->png_temp;
		data->png_atssincr = store->png_atssincr;
		data->png_tt = store->png_tt;

		/* port settings */
		data->port_trans[0] = store->port_trans[0];
		data->port_trans[1] = store->port_trans[1];
		data->port_gain = store->port_gain;
		data->port_pulse = store->port_pulse;
		data->port_btycount = store->port_btycount;
		data->port_btypad = store->port_btypad;
		data->port_ssoffset = store->port_ssoffset;
		data->port_sscount = store->port_sscount;
		data->port_sspad = store->port_sspad;

		/* starboard settings */
		data->stbd_trans[0] = store->stbd_trans[0];
		data->stbd_trans[1] = store->stbd_trans[1];
		data->stbd_gain = store->stbd_gain;
		data->stbd_pulse = store->stbd_pulse;
		data->stbd_btycount = store->stbd_btycount;
		data->stbd_btypad = store->stbd_btypad;
		data->stbd_ssoffset = store->stbd_ssoffset;
		data->stbd_sscount = store->stbd_sscount;
		data->stbd_sspad = store->stbd_sspad;

		/* bathymetry */
		for (i=0;i<data->port_btycount;i++)
			{
			data->bath_acrosstrack_port[i] 
				= store->bath_acrosstrack_port[i];
			data->bath_port[i] = store->bath_port[i];
			data->tt_port[i] = store->tt_port[i];
			data->angle_port[i] = store->angle_port[i];
			}
		for (i=0;i<data->stbd_btycount;i++)
			{
			data->bath_acrosstrack_stbd[i] 
				= store->bath_acrosstrack_stbd[i];
			data->bath_stbd[i] = store->bath_stbd[i];
			data->tt_stbd[i] = store->tt_stbd[i];
			data->angle_stbd[i] = store->angle_stbd[i];
			}

		/* sidescan */
		for (i=0;i<data->port_sscount;i++)
			{
			data->ss_port[i] = store->ss_port[i];
			}
		for (i=0;i<data->stbd_sscount;i++)
			{
			data->ss_stbd[i] = store->ss_stbd[i];
			}

		/* comment */
		strncpy(data->comment,store->comment,MBF_MR1BLDEO_MAXLINE);

		}

	/* write next data to file */
	status = mbr_mr1bldeo_wr_data(verbose,mbio_ptr,data,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_mr1bldeo_rd_data(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_mr1bldeo_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mr1bldeo_struct *data;
	XDR	*xdrs;
	int	read_size;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %ld\n",(size_t)mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	data = (struct mbf_mr1bldeo_struct *) mb_io_ptr->raw_data;
	xdrs = (XDR *)mb_io_ptr->xdrs;

	/* initialize everything to zeros */
	mbr_zero_mr1bldeo(verbose,data,error);

	/* if first time through read file header */
	if (mb_io_ptr->fileheader == MB_NO)
		{
		status = mbr_mr1bldeo_rd_hdr(verbose,xdrs,data,
			&mb_io_ptr->hdr_comment,error);
		if (status == MB_SUCCESS)
			{
			mb_io_ptr->fileheader = MB_YES;
			if (mb_io_ptr->hdr_comment == NULL)
				mb_io_ptr->hdr_comment_size = 0;
			else
				mb_io_ptr->hdr_comment_size 
					= strlen(mb_io_ptr->hdr_comment);
			mb_io_ptr->hdr_comment_loc = 0;
			if (mb_io_ptr->hdr_comment_size > 80)
				read_size = 80;
			else
				read_size = mb_io_ptr->hdr_comment_size;
			strncpy(data->comment,mb_io_ptr->hdr_comment,read_size);
			mb_io_ptr->hdr_comment_loc = read_size;
			data->kind = MB_DATA_COMMENT;
			}
		}

	/* if comments are still held in mb_io_ptr->hdr_comment then
		extract comment and return */
	else if (mb_io_ptr->hdr_comment_size > mb_io_ptr->hdr_comment_loc)
		{
		if (mb_io_ptr->hdr_comment_size - mb_io_ptr->hdr_comment_loc 
			> 80)
			read_size = 80;
		else
			read_size = mb_io_ptr->hdr_comment_size 
				- mb_io_ptr->hdr_comment_loc;
		strncpy(data->comment,
			&mb_io_ptr->hdr_comment[mb_io_ptr->hdr_comment_loc],
			read_size);
		mb_io_ptr->hdr_comment_loc += read_size;
		data->kind = MB_DATA_COMMENT;
		}

	/* else read data */
	else
		{
		status = mbr_mr1bldeo_rd_ping(verbose,xdrs,data,error);
		if (status == MB_SUCCESS)
			{
			data->kind = MB_DATA_DATA;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_mr1bldeo_rd_hdr(int verbose, XDR *xdrs, 
		struct mbf_mr1bldeo_struct *data, char **hdr_comment,
		int *error)
{
	char	*function_name = "mbr_mr1bldeo_rd_hdr";
	int	status = MB_SUCCESS;
	int	len;
	unsigned int	ulen;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       xdrs:       %ld\n",(size_t)xdrs);
		fprintf(stderr,"dbg2       data:       %ld\n",(size_t)data);
		fprintf(stderr,"dbg2       hdr_comment:%ld\n",(size_t)*hdr_comment);
		}

	/* set status and error */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* read magic number */
	status = xdr_int(xdrs, &data->mf_magic);
		
	/* read ping count */
	if (status == MB_SUCCESS)
		status = xdr_int(xdrs, &data->mf_count);

	/* read header comment */
	if (status == MB_SUCCESS)
		status = xdr_int(xdrs, &len);
	if (status == MB_SUCCESS)
		{
		if (len > 0)
			{
			status = mb_mallocd(verbose,__FILE__,__LINE__,len+1,(void **)hdr_comment,error);
			status = xdr_bytes(xdrs,hdr_comment,&ulen,(unsigned int)(len + 1));
			}
		else if (len < 0)
			status = MB_FAILURE;
		}

	if (status == MB_FAILURE && *error == MB_ERROR_NO_ERROR)
		*error = MB_ERROR_EOF;
	else if (data->mf_magic != 6666)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       mf_magic:         %d\n",data->mf_magic);
		fprintf(stderr,"dbg5       mf_count:         %d\n",data->mf_count);
		fprintf(stderr,"dbg5       hdr_comment:\n%s\n",*hdr_comment);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_mr1bldeo_rd_ping(int verbose, XDR *xdrs, 
		struct mbf_mr1bldeo_struct *data, int *error)
{
	char	*function_name = "mbr_mr1bldeo_rd_ping";
	int	status = MB_SUCCESS;
	int	dummy_count;
	float	dummy;
	long	sec, usec;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       xdrs:       %ld\n",(size_t)xdrs);
		fprintf(stderr,"dbg2       data:       %ld\n",(size_t)data);
		}

	/* read ping header */
	status = xdr_long(xdrs, &sec);
	status = xdr_long(xdrs, &usec);
	data->sec = (int)sec;
	data->usec = (int)usec;
	status = xdr_double(xdrs, &data->png_lon);
	status = xdr_double(xdrs, &data->png_lat);
	status = xdr_float(xdrs, &data->png_course);
	status = xdr_float(xdrs, &data->png_compass);
	status = xdr_float(xdrs, &data->png_prdepth);
	status = xdr_float(xdrs, &data->png_alt);
	status = xdr_float(xdrs, &data->png_pitch);
	status = xdr_float(xdrs, &data->png_roll);
	status = xdr_float(xdrs, &data->png_temp);
	status = xdr_float(xdrs, &data->png_atssincr);
	status = xdr_float(xdrs, &data->png_tt);

	/* read port side header */
	status = xdr_float(xdrs, &data->port_trans[0]);
	status = xdr_float(xdrs, &data->port_trans[1]);
	status = xdr_float(xdrs, &data->port_gain);
	status = xdr_float(xdrs, &data->port_pulse);
	status = xdr_int(xdrs, &data->port_btycount);
	status = xdr_float(xdrs, &data->port_ssoffset);
	status = xdr_int(xdrs, &data->port_sscount);

	/* read starboard side header */
	status = xdr_float(xdrs, &data->stbd_trans[0]);
	status = xdr_float(xdrs, &data->stbd_trans[1]);
	status = xdr_float(xdrs, &data->stbd_gain);
	status = xdr_float(xdrs, &data->stbd_pulse);
	status = xdr_int(xdrs, &data->stbd_btycount);
	status = xdr_float(xdrs, &data->stbd_ssoffset);
	status = xdr_int(xdrs, &data->stbd_sscount);
	
	/* read bathymetry and sidescan data 
		- handle more data than allowed by MBIO by
		  throwing away the excess */

	/* do port bathymetry */
	if (data->port_btycount > MBF_MR1BLDEO_BEAMS_SIDE)
		{
		if (verbose > 0)
			{
			fprintf(stderr, "Port bathymetry count exceeds MBIO maximum: %d %d\n", 
				data->port_btycount, MBF_MR1BLDEO_BEAMS_SIDE);
			}
		dummy_count = data->port_btycount 
			- MBF_MR1BLDEO_BEAMS_SIDE;
		data->port_btycount = MBF_MR1BLDEO_BEAMS_SIDE;
		}
	else
		dummy_count = 0;
	for (i=0;i<data->port_btycount;i++)
		{
		status = xdr_float(xdrs,&data->bath_acrosstrack_port[i]);
		status = xdr_float(xdrs,&data->bath_port[i]);
		status = xdr_float(xdrs,&data->tt_port[i]);
		status = xdr_float(xdrs,&data->angle_port[i]);
		}
	for (i=0;i<dummy_count;i++)
		{
		status = xdr_float(xdrs,&dummy);
		status = xdr_float(xdrs,&dummy);
		status = xdr_float(xdrs,&dummy);
		status = xdr_float(xdrs,&dummy);
		}

	/* do port sidescan */
	if (data->port_sscount > MBF_MR1BLDEO_PIXELS_SIDE)
		{
		if (verbose > 0)
			{
			fprintf(stderr, "Port sidescan count exceeds MBIO maximum: %d %d\n", 
				data->port_sscount, MBF_MR1BLDEO_PIXELS_SIDE);
			}
		dummy_count = data->port_sscount 
			- MBF_MR1BLDEO_PIXELS_SIDE;
		data->port_sscount = MBF_MR1BLDEO_PIXELS_SIDE;
		}
	else
		dummy_count = 0;
	for (i=0;i<data->port_sscount;i++)
		{
		status = xdr_float(xdrs,&data->ss_port[i]);
		}
	for (i=0;i<dummy_count;i++)
		{
		status = xdr_float(xdrs,&dummy);
		}

	/* do starboard bathymetry */
	if (data->stbd_btycount > MBF_MR1BLDEO_BEAMS_SIDE)
		{
		/* output debug messages */
		if (verbose > 0)
			{
			fprintf(stderr, "Starboard bathymetry count exceeds MBIO maximum: %d %d\n", 
				data->stbd_btycount, MBF_MR1BLDEO_BEAMS_SIDE);
			}
		dummy_count = data->stbd_btycount 
			- MBF_MR1BLDEO_BEAMS_SIDE;
		data->stbd_btycount = MBF_MR1BLDEO_BEAMS_SIDE;
		}
	else
		dummy_count = 0;
	for (i=0;i<data->stbd_btycount;i++)
		{
		status = xdr_float(xdrs,&data->bath_acrosstrack_stbd[i]);
		status = xdr_float(xdrs,&data->bath_stbd[i]);
		status = xdr_float(xdrs,&data->tt_stbd[i]);
		status = xdr_float(xdrs,&data->angle_stbd[i]);
		}
	for (i=0;i<dummy_count;i++)
		{
		status = xdr_float(xdrs,&dummy);
		status = xdr_float(xdrs,&dummy);
		status = xdr_float(xdrs,&dummy);
		status = xdr_float(xdrs,&dummy);
		}

	/* do starboard sidescan */
	if (data->stbd_sscount > MBF_MR1BLDEO_PIXELS_SIDE)
		{
		/* output debug messages */
		if (verbose > 0)
			{
			fprintf(stderr, "Starboard sidescan count exceeds MBIO maximum: %d %d\n", 
				data->stbd_sscount, MBF_MR1BLDEO_PIXELS_SIDE);
			}
		dummy_count = data->stbd_sscount 
			- MBF_MR1BLDEO_PIXELS_SIDE;
		data->stbd_sscount = MBF_MR1BLDEO_PIXELS_SIDE;
		}
	else
		dummy_count = 0;
	for (i=0;i<data->stbd_sscount;i++)
		{
		status = xdr_float(xdrs,&data->ss_stbd[i]);
		}
	for (i=0;i<dummy_count;i++)
		{
		status = xdr_float(xdrs,&dummy);
		}

	if (status == MB_FAILURE)
		*error = MB_ERROR_EOF;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       sec:              %d\n",data->sec);
		fprintf(stderr,"dbg5       usec:             %d\n",data->usec);
		fprintf(stderr,"dbg5       longitude:        %f\n",
			data->png_lon);
		fprintf(stderr,"dbg5       latitude:         %f\n",
			data->png_lat);
		fprintf(stderr,"dbg5       course:           %f\n",
			data->png_course);
		fprintf(stderr,"dbg5       heading:          %f\n",
			data->png_compass);
		fprintf(stderr,"dbg5       pressure depth:   %f\n",
			data->png_prdepth);
		fprintf(stderr,"dbg5       altitude:         %f\n",
			data->png_alt);
		fprintf(stderr,"dbg5       pitch:            %f\n",
			data->png_pitch);
		fprintf(stderr,"dbg5       roll:             %f\n",
			data->png_roll);
		fprintf(stderr,"dbg5       temperature:      %f\n",
			data->png_temp);
		fprintf(stderr,"dbg5       pixel spacing:    %f\n",
			data->png_atssincr);
		fprintf(stderr,"dbg5       nadir travel time:%f\n",
			data->png_tt);
		fprintf(stderr,"dbg5       port transmit 0:  %f\n",
			data->port_trans[0]);
		fprintf(stderr,"dbg5       port transmit 1:  %f\n",
			data->port_trans[1]);
		fprintf(stderr,"dbg5       port gain:        %f\n",
			data->port_gain);
		fprintf(stderr,"dbg5       port pulse:       %f\n",
			data->port_pulse);
		fprintf(stderr,"dbg5       port bath count:  %d\n",
			data->port_btycount);
		fprintf(stderr,"dbg5       port ss offset:   %f\n",
			data->port_ssoffset);
		fprintf(stderr,"dbg5       port ss count:    %d\n",
			data->port_sscount);
		fprintf(stderr,"dbg5       stbd transmit 0:  %f\n",
			data->stbd_trans[0]);
		fprintf(stderr,"dbg5       stbd transmit 1:  %f\n",
			data->stbd_trans[1]);
		fprintf(stderr,"dbg5       stbd gain:        %f\n",
			data->stbd_gain);
		fprintf(stderr,"dbg5       stbd pulse:       %f\n",
			data->stbd_pulse);
		fprintf(stderr,"dbg5       stbd bath count:  %d\n",
			data->stbd_btycount);
		fprintf(stderr,"dbg5       stbd ss offset:   %f\n",
			data->stbd_ssoffset);
		fprintf(stderr,"dbg5       stbd ss count:    %d\n",
			data->stbd_sscount);
		fprintf(stderr,"\n");
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"dbg5       port_beam  depth   xtrack    tt   angle\n");
		for (i=0;i<data->port_btycount;i++)
		  {
		  fprintf(stderr,"dbg5       %3d     %12.4g %12.4g %12.4g %12.4g\n",
			i,data->bath_port[i],data->bath_acrosstrack_port[i], 
			data->tt_port[i],data->angle_port[i]);
		  }
		fprintf(stderr,"\n");
		fprintf(stderr,"dbg5       stbd_beam  depth   xtrack    tt   angle\n");
		for (i=0;i<data->stbd_btycount;i++)
		  {
		  fprintf(stderr,"dbg5       %3d     %12.4g %12.4g %12.4g %12.4g\n",
			i,data->bath_stbd[i],data->bath_acrosstrack_stbd[i],
			data->tt_stbd[i],data->angle_stbd[i]);
		  }
		fprintf(stderr,"\n");
		fprintf(stderr,"dbg5       port_pixel  sidescan\n");
		for (i=0;i<data->port_sscount;i++)
		  {
		  fprintf(stderr,"dbg5       %3d     %12.4g\n",
			i,data->ss_port[i]);
		  }
		fprintf(stderr,"\n");
		fprintf(stderr,"dbg5       stbd_pixel  sidescan\n");
		for (i=0;i<data->stbd_sscount;i++)
		  {
		  fprintf(stderr,"dbg5       %3d     %12.4g\n",
			i,data->ss_stbd[i]);
		  }
		fprintf(stderr,"\n");
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_mr1bldeo_wr_data(int verbose, void *mbio_ptr, struct mbf_mr1bldeo_struct *data, int *error)
{
	char	*function_name = "mbr_mr1bldeo_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	XDR	*xdrs;
	char	*tmp;
	int	lenc, lenhc, len;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %ld\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       data:       %ld\n",(size_t)data);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to XDR structure */
	xdrs = (XDR *)mb_io_ptr->xdrs;

	/* if comment and file header not written */
	if (mb_io_ptr->fileheader == MB_NO && data->kind == MB_DATA_COMMENT)
		{
		/* add comment to string mb_io_ptr->hdr_comment
			to be be written in file header */
		lenc = strlen(data->comment);
		lenhc = 0;
		if (mb_io_ptr->hdr_comment != NULL)
			lenhc = strlen(mb_io_ptr->hdr_comment);
		len = lenc + lenhc + 1;
		status = mb_mallocd(verbose,__FILE__,__LINE__,len,(void **)&tmp,error);
		strcpy(tmp,"\0");
		if (lenhc > 0) strcpy(tmp,mb_io_ptr->hdr_comment);
		if (lenc > 0) strcat(tmp,data->comment);
		if (mb_io_ptr->hdr_comment != NULL)
			mb_freed(verbose,__FILE__, __LINE__, (void **)&mb_io_ptr->hdr_comment,error);
		mb_io_ptr->hdr_comment = tmp;
		}

	/* if data and file header not written */
	else if (mb_io_ptr->fileheader == MB_NO 
		&& data->kind != MB_DATA_COMMENT)
		{
		/* write file header */
		status = mbr_mr1bldeo_wr_hdr(verbose,xdrs,data,
				&mb_io_ptr->hdr_comment,error);
		mb_io_ptr->fileheader = MB_YES;

		/* write data */
		status = mbr_mr1bldeo_wr_ping(verbose,xdrs,data,error);
		}

	/* if data and file header written */
	else if (mb_io_ptr->fileheader == MB_YES 
		&& data->kind == MB_DATA_DATA)
		{
		/* write data */
		status = mbr_mr1bldeo_wr_ping(verbose,xdrs,data,error);
		}

	/* if not data and file header written */
	else if (mb_io_ptr->fileheader == MB_YES 
		&& data->kind != MB_DATA_DATA)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_KIND;
		}

	/* print output debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Data record kind in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       kind:       %d\n",data->kind);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_mr1bldeo_wr_hdr(int verbose, XDR *xdrs, 
		struct mbf_mr1bldeo_struct *data, char **hdr_comment, int *error)
{
	char	*function_name = "mbr_mr1bldeo_wr_hdr";
	int	status = MB_SUCCESS;
	int	len;
	unsigned int	ulen;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       xdrs:       %ld\n",(size_t)xdrs);
		fprintf(stderr,"dbg2       data:       %ld\n",(size_t)data);
		fprintf(stderr,"dbg2       hdr_comment:%ld\n",(size_t)*hdr_comment);
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       mf_magic:         %d\n",data->mf_magic);
		fprintf(stderr,"dbg5       mf_count:         %d\n",data->mf_count);
		fprintf(stderr,"dbg5       hdr_comment:\n%s\n",*hdr_comment);
		}

	/* set status and error */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* write magic number */
	status = xdr_int(xdrs, &data->mf_magic);
		
	/* write ping count */
	if (status == MB_SUCCESS)
		{
		status = xdr_int(xdrs, &data->mf_count);
		}

	/* write header comment */
	if (status == MB_SUCCESS)
		{
		if (*hdr_comment == NULL)
			len = 0;
		else
			len = strlen(*hdr_comment);
		status = xdr_int(xdrs, &len);
		}
	if (status == MB_SUCCESS && len > 0)
		{
		ulen = len;
		status = xdr_bytes(xdrs,hdr_comment,
				&ulen,(unsigned int)len);
		}

	/* check for an error */
	if (status != MB_SUCCESS)
		*error = MB_ERROR_WRITE_FAIL;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_mr1bldeo_wr_ping(int verbose, XDR *xdrs, struct mbf_mr1bldeo_struct *data, int *error)
{
	char	*function_name = "mbr_mr1bldeo_wr_ping";
	int	status = MB_SUCCESS;
	long	sec, usec;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       xdrs:       %ld\n",(size_t)xdrs);
		fprintf(stderr,"dbg2       data:       %ld\n",(size_t)data);
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       sec:              %d\n",data->sec);
		fprintf(stderr,"dbg5       usec:             %d\n",data->usec);
		fprintf(stderr,"dbg5       longitude:        %f\n",
			data->png_lon);
		fprintf(stderr,"dbg5       latitude:         %f\n",
			data->png_lat);
		fprintf(stderr,"dbg5       course:           %f\n",
			data->png_course);
		fprintf(stderr,"dbg5       heading:          %f\n",
			data->png_compass);
		fprintf(stderr,"dbg5       pressure depth:   %f\n",
			data->png_prdepth);
		fprintf(stderr,"dbg5       altitude:         %f\n",
			data->png_alt);
		fprintf(stderr,"dbg5       pitch:            %f\n",
			data->png_pitch);
		fprintf(stderr,"dbg5       roll:             %f\n",
			data->png_roll);
		fprintf(stderr,"dbg5       temperature:      %f\n",
			data->png_temp);
		fprintf(stderr,"dbg5       pixel spacing:    %f\n",
			data->png_atssincr);
		fprintf(stderr,"dbg5       nadir travel time:%f\n",
			data->png_tt);
		fprintf(stderr,"dbg5       port transmit 0:  %f\n",
			data->port_trans[0]);
		fprintf(stderr,"dbg5       port transmit 1:  %f\n",
			data->port_trans[1]);
		fprintf(stderr,"dbg5       port gain:        %f\n",
			data->port_gain);
		fprintf(stderr,"dbg5       port pulse:       %f\n",
			data->port_pulse);
		fprintf(stderr,"dbg5       port bath count:  %d\n",
			data->port_btycount);
		fprintf(stderr,"dbg5       port ss offset:   %f\n",
			data->port_ssoffset);
		fprintf(stderr,"dbg5       port ss count:    %d\n",
			data->port_sscount);
		fprintf(stderr,"dbg5       stbd transmit 0:  %f\n",
			data->stbd_trans[0]);
		fprintf(stderr,"dbg5       stbd transmit 1:  %f\n",
			data->stbd_trans[1]);
		fprintf(stderr,"dbg5       stbd gain:        %f\n",
			data->stbd_gain);
		fprintf(stderr,"dbg5       stbd pulse:       %f\n",
			data->stbd_pulse);
		fprintf(stderr,"dbg5       stbd bath count:  %d\n",
			data->stbd_btycount);
		fprintf(stderr,"dbg5       stbd ss offset:   %f\n",
			data->stbd_ssoffset);
		fprintf(stderr,"dbg5       stbd ss count:    %d\n",
			data->stbd_sscount);
		fprintf(stderr,"\n");
		fprintf(stderr,"dbg5       port_beam  depth   xtrack    tt   angle\n");
		for (i=0;i<data->port_btycount;i++)
		  {
		  fprintf(stderr,"dbg5       %3d     %12.4g %12.4g %12.4g %12.4g\n",
			i,data->bath_port[i],data->bath_acrosstrack_port[i], 
			data->tt_port[i],data->angle_port[i]);
		  }
		fprintf(stderr,"\n");
		fprintf(stderr,"dbg5       stbd_beam  depth   xtrack    tt   angle\n");
		for (i=0;i<data->stbd_btycount;i++)
		  {
		  fprintf(stderr,"dbg5       %3d     %12.4g %12.4g %12.4g %12.4g\n",
			i,data->bath_stbd[i],data->bath_acrosstrack_stbd[i],
			data->tt_stbd[i],data->angle_stbd[i]);
		  }
		fprintf(stderr,"\n");
		fprintf(stderr,"dbg5       port_pixel  sidescan\n");
		for (i=0;i<data->port_sscount;i++)
		  {
		  fprintf(stderr,"dbg5       %3d     %12.4g\n",
			i,data->ss_port[i]);
		  }
		fprintf(stderr,"\n");
		fprintf(stderr,"dbg5       stbd_pixel  sidescan\n");
		for (i=0;i<data->stbd_sscount;i++)
		  {
		  fprintf(stderr,"dbg5       %3d     %12.4g\n",
			i,data->ss_stbd[i]);
		  }
		fprintf(stderr,"\n");
		}

	/* write ping header */
	sec = (size_t) data->sec;
	usec = (size_t) data->usec;
	status = xdr_long(xdrs, &sec);
	status = xdr_long(xdrs, &usec);
	status = xdr_double(xdrs, &data->png_lon);
	status = xdr_double(xdrs, &data->png_lat);
	status = xdr_float(xdrs, &data->png_course);
	status = xdr_float(xdrs, &data->png_compass);
	status = xdr_float(xdrs, &data->png_prdepth);
	status = xdr_float(xdrs, &data->png_alt);
	status = xdr_float(xdrs, &data->png_pitch);
	status = xdr_float(xdrs, &data->png_roll);
	status = xdr_float(xdrs, &data->png_temp);
	status = xdr_float(xdrs, &data->png_atssincr);
	status = xdr_float(xdrs, &data->png_tt);

	/* write port side header */
	status = xdr_float(xdrs, &data->port_trans[0]);
	status = xdr_float(xdrs, &data->port_trans[1]);
	status = xdr_float(xdrs, &data->port_gain);
	status = xdr_float(xdrs, &data->port_pulse);
	status = xdr_int(xdrs, &data->port_btycount);
	status = xdr_float(xdrs, &data->port_ssoffset);
	status = xdr_int(xdrs, &data->port_sscount);

	/* write starboard side header */
	status = xdr_float(xdrs, &data->stbd_trans[0]);
	status = xdr_float(xdrs, &data->stbd_trans[1]);
	status = xdr_float(xdrs, &data->stbd_gain);
	status = xdr_float(xdrs, &data->stbd_pulse);
	status = xdr_int(xdrs, &data->stbd_btycount);
	status = xdr_float(xdrs, &data->stbd_ssoffset);
	status = xdr_int(xdrs, &data->stbd_sscount);

	/* write bathymetry and sidescan data */
	for (i=0;i<data->port_btycount;i++)
		{
		status = xdr_float(xdrs,&data->bath_acrosstrack_port[i]);
		status = xdr_float(xdrs,&data->bath_port[i]);
		status = xdr_float(xdrs,&data->tt_port[i]);
		status = xdr_float(xdrs,&data->angle_port[i]);
		}
	for (i=0;i<data->port_sscount;i++)
		{
		status = xdr_float(xdrs,&data->ss_port[i]);
		}
	for (i=0;i<data->stbd_btycount;i++)
		{
		status = xdr_float(xdrs,&data->bath_acrosstrack_stbd[i]);
		status = xdr_float(xdrs,&data->bath_stbd[i]);
		status = xdr_float(xdrs,&data->tt_stbd[i]);
		status = xdr_float(xdrs,&data->angle_stbd[i]);
		}
	for (i=0;i<data->stbd_sscount;i++)
		{
		status = xdr_float(xdrs,&data->ss_stbd[i]);
		}
	if (status == MB_FAILURE)
		*error = MB_ERROR_WRITE_FAIL;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
