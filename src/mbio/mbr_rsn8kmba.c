/*--------------------------------------------------------------------
 *    The MB-system:	mbr_xtfr8101.c	8/8/94
 *	$Id: mbr_rsn8kmba.c,v 5.2 2003-04-17 21:05:23 caress Exp $
 *
 *    Copyright (c) 2001, 2002, 2003 by
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
 * mbr_xtfr8101.c contains the functions for reading and writing
 * multibeam data in the XTFR8101 format.  
 * These functions include:
 *   mbr_alm_xtfr8101	- allocate read/write memory
 *   mbr_dem_xtfr8101	- deallocate read/write memory
 *   mbr_rt_xtfr8101	- read and translate data
 *   mbr_wt_xtfr8101	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	October 21, 2001
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
#include "../../include/mbsys_reson8k.h"

/* essential function prototypes */
int mbr_register_rsn8kmba(int verbose, void *mbio_ptr, 
		int *error);
int mbr_info_rsn8kmba(int verbose, 
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
			double *beamwidth_xtrack, 
			double *beamwidth_ltrack, 
			int *error);
int mbr_alm_rsn8kmba(int verbose, void *mbio_ptr, int *error);
int mbr_dem_rsn8kmba(int verbose, void *mbio_ptr, int *error);
int mbr_rt_rsn8kmba(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_rsn8kmba(int verbose, void *mbio_ptr, void *store_ptr, int *error);

#define MBF_RSN8KMBA_BEGINRECORD_LENGTH 12
#define MBF_RSN8KMBA_ENDRECORD_LENGTH 8
#define MBF_RSN8KMBA_PARAMETER_LENGTH 72
#define MBF_RSN8KMBA_NAV_LENGTH 36
#define MBF_RSN8KMBA_ATTITUDE_LENGTH 32
#define MBF_RSN8KMBA_SVPSTART_LENGTH 12
#define MBF_RSN8KMBA_SVP_LENGTH 8
#define MBF_RSN8KMBA_COMMENT_LENGTH MBSYS_RESON8K_COMMENT_LENGTH + 8
/*--------------------------------------------------------------------*/
int mbr_register_rsn8kmba(int verbose, void *mbio_ptr, int *error)
{
	static char res_id[]="$Id: mbr_rsn8kmba.c,v 5.2 2003-04-17 21:05:23 caress Exp $";
	char	*function_name = "mbr_register_rsn8kmba";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		}

	/* get mb_io_ptr */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set format info parameters */
	status = mbr_info_rsn8kmba(verbose, 
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
			&mb_io_ptr->beamwidth_xtrack, 
			&mb_io_ptr->beamwidth_ltrack, 
			error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_rsn8kmba;
	mb_io_ptr->mb_io_format_free = &mbr_dem_rsn8kmba; 
	mb_io_ptr->mb_io_store_alloc = &mbsys_reson8k_alloc; 
	mb_io_ptr->mb_io_store_free = &mbsys_reson8k_deall; 
	mb_io_ptr->mb_io_read_ping = &mbr_rt_rsn8kmba; 
	mb_io_ptr->mb_io_write_ping = &mbr_wt_rsn8kmba; 
	mb_io_ptr->mb_io_extract = &mbsys_reson8k_extract; 
	mb_io_ptr->mb_io_insert = &mbsys_reson8k_insert; 
	mb_io_ptr->mb_io_extract_nav = &mbsys_reson8k_extract_nav; 
	mb_io_ptr->mb_io_insert_nav = &mbsys_reson8k_insert_nav; 
	mb_io_ptr->mb_io_extract_altitude = &mbsys_reson8k_extract_altitude; 
	mb_io_ptr->mb_io_insert_altitude = NULL; 
	mb_io_ptr->mb_io_extract_svp = &mbsys_reson8k_extract_svp; 
	mb_io_ptr->mb_io_insert_svp = &mbsys_reson8k_insert_svp; 
	mb_io_ptr->mb_io_ttimes = &mbsys_reson8k_ttimes; 
	mb_io_ptr->mb_io_copyrecord = &mbsys_reson8k_copy; 
	mb_io_ptr->mb_io_extract_rawss = NULL; 
	mb_io_ptr->mb_io_insert_rawss = NULL; 

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
		fprintf(stderr,"dbg2       heading_source:     %d\n",mb_io_ptr->heading_source);
		fprintf(stderr,"dbg2       beamwidth_xtrack:   %f\n",mb_io_ptr->beamwidth_xtrack);
		fprintf(stderr,"dbg2       beamwidth_ltrack:   %f\n",mb_io_ptr->beamwidth_ltrack);
		fprintf(stderr,"dbg2       format_alloc:       %d\n",mb_io_ptr->mb_io_format_alloc);
		fprintf(stderr,"dbg2       format_free:        %d\n",mb_io_ptr->mb_io_format_free);
		fprintf(stderr,"dbg2       store_alloc:        %d\n",mb_io_ptr->mb_io_store_alloc);
		fprintf(stderr,"dbg2       store_free:         %d\n",mb_io_ptr->mb_io_store_free);
		fprintf(stderr,"dbg2       read_ping:          %d\n",mb_io_ptr->mb_io_read_ping);
		fprintf(stderr,"dbg2       write_ping:         %d\n",mb_io_ptr->mb_io_write_ping);
		fprintf(stderr,"dbg2       extract:            %d\n",mb_io_ptr->mb_io_extract);
		fprintf(stderr,"dbg2       insert:             %d\n",mb_io_ptr->mb_io_insert);
		fprintf(stderr,"dbg2       extract_nav:        %d\n",mb_io_ptr->mb_io_extract_nav);
		fprintf(stderr,"dbg2       insert_nav:         %d\n",mb_io_ptr->mb_io_insert_nav);
		fprintf(stderr,"dbg2       extract_altitude:   %d\n",mb_io_ptr->mb_io_extract_altitude);
		fprintf(stderr,"dbg2       insert_altitude:    %d\n",mb_io_ptr->mb_io_insert_altitude);
		fprintf(stderr,"dbg2       extract_svp:        %d\n",mb_io_ptr->mb_io_extract_svp);
		fprintf(stderr,"dbg2       insert_svp:         %d\n",mb_io_ptr->mb_io_insert_svp);
		fprintf(stderr,"dbg2       ttimes:             %d\n",mb_io_ptr->mb_io_ttimes);
		fprintf(stderr,"dbg2       extract_rawss:      %d\n",mb_io_ptr->mb_io_extract_rawss);
		fprintf(stderr,"dbg2       insert_rawss:       %d\n",mb_io_ptr->mb_io_insert_rawss);
		fprintf(stderr,"dbg2       copyrecord:         %d\n",mb_io_ptr->mb_io_copyrecord);
		fprintf(stderr,"dbg2       error:              %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		}

	/* return status */
	return(status);
}

/*--------------------------------------------------------------------*/
int mbr_info_rsn8kmba(int verbose, 
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
			double *beamwidth_xtrack, 
			double *beamwidth_ltrack, 
			int *error)
{
	static char res_id[]="$Id: mbr_rsn8kmba.c,v 5.2 2003-04-17 21:05:23 caress Exp $";
	char	*function_name = "mbr_info_rsn8kmba";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		}

	/* set format info parameters */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	*system = MB_SYS_RESON;
	*beams_bath_max = MBSYS_RESON8K_MAXBEAMS;
	*beams_amp_max = MBSYS_RESON8K_MAXBEAMS;
	*pixels_ss_max = MBSYS_RESON8K_MAXPIXELS;
	strncpy(format_name, "RSN8KMBA", MB_NAME_LENGTH);
	strncpy(system_name, "RESON8K", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_RSN8KMBA\nInformal Description: XTF format Reson SeaBat 81XX\nAttributes:           240 beam bathymetry and amplitude,\n		      1024 pixel sidescan\n                      binary, read-only,\n                      Triton-Elics.\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_NO;
	*traveltime = MB_YES;
	*beam_flagging = MB_YES;
	*nav_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_DATA;
	*beamwidth_xtrack = 1.5;
	*beamwidth_ltrack = 1.5;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
		fprintf(stderr,"dbg2       heading_source:     %d\n",*heading_source);
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
int mbr_alm_rsn8kmba(int verbose, void *mbio_ptr, int *error)
{
	static char res_id[]="$Id: mbr_rsn8kmba.c,v 5.2 2003-04-17 21:05:23 caress Exp $";
	char	*function_name = "mbr_alm_rsn8kmba";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set initial status */
	status = MB_SUCCESS;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = 0;
	mb_io_ptr->data_structure_size = 0;
	status = mb_malloc(verbose,sizeof(struct mbsys_reson8k_struct),
				&mb_io_ptr->store_data,error);

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
int mbr_dem_rsn8kmba(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_rsn8kmba";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		}

	/* get pointers to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

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
int mbr_rt_rsn8kmba(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_rsn8kmba";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_rsn8kmba_struct *data;
	struct mbsys_reson8k_struct *store;
	int	nchan;
	int	time_i[7];
	double	time_d, ntime_d, dtime, timetag;
	double	ttscale, angscale;
	int	icenter, istart, quality;
	int	intensity_max;
	double	angle, theta, phi;
	double	rr, xx, zz, r, a, a2;
	double	*pixel_size, *swath_width;
	int	i, j, k;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointers to mbio descriptor and data structures */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	data = (struct mbf_rsn8kmba_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_reson8k_struct *) store_ptr;

	/* read next data from file */
	status = mbr_rsn8kmba_rd_data(verbose,mbio_ptr,error);

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = store->kind;
			    
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
int mbr_wt_rsn8kmba(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_rsn8kmba";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
		
	/* write next data to file */
	status = mbr_em300mba_wr_data(verbose,mbio_ptr,store_ptr,error);

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
int mbr_rsn8kmba_rd_data(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_rsn8kmba_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson8k_struct *store;
	char	line[8];
	int	skip, done, found;
	int	read_len;
	unsigned short recordlength;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
				
	/* get pointer to data structure */
	store = (struct mbsys_reson8k_struct *) mb_io_ptr->store_data;

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;
		
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	done = MB_NO;
		
	/* look for next recognizable record */
	while (status == MB_SUCCESS && done == MB_NO)
	    {
	    /* find the next beginning of record */
	    found = MB_NO;
	    skip = 0;
	    if ((read_len = fread(line,1,8,mb_io_ptr->mbfp)) != 8)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
	    else if (strncmp(line,"RSN8KMBA",8) == 0)
		found = MB_YES;
	    while (status == MB_SUCCESS 
		&& found == MB_NO)
		{
		line[0] = line[1];
		if ((read_len = fread(&(line[1]),1,1,mb_io_ptr->mbfp)) != 1)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
	        else if (strncmp(line,"RSN8KMBA",8) == 0)
			{
			found = MB_YES;
			skip++;
			}
		}
	    }


	/* read record type and size */
	if (status == MB_SUCCESS && found == MB_YES)
	    {
	    /* get record type and size */
	    if (read_len = fread(line,1,4,mb_io_ptr->mbfp) == 4)
		{
		/* read the record size */
		mb_get_binary_short(MB_NO, &line[2], (short int *)&recordlength);

		/* parse the record */
		if (strncmp(line, "PR", 2) == 0)
			{
			store->kind = MB_DATA_PARAMETER;
			status = mbr_rsn8kmba_rd_parameter(verbose, store, (int)recordlength, error);
			}
		else if (strncmp(line, "CM", 2) == 0)
			{
			store->kind = MB_DATA_COMMENT;
			status = mbr_rsn8kmba_rd_comment(verbose, store, (int)recordlength, error);
			}
		else if (strncmp(line, "SR", 2) == 0)
			{
			store->kind = MB_DATA_DATA;
			status = mbr_rsn8kmba_rd_ping(verbose, store, (int)recordlength, error);
			}
		else if (strncmp(line, "NV", 2) == 0)
			{
			store->kind = MB_DATA_NAV;
			status = mbr_rsn8kmba_rd_nav(verbose, store, (int)recordlength, error);
			}
		else if (strncmp(line, "SV", 2) == 0)
			{
			store->kind = MB_DATA_VELOCITY_PROFILE;
			status = mbr_rsn8kmba_rd_svp(verbose, store, (int)recordlength, error);
			}
		else if (strncmp(line, "AT", 2) == 0)
			{
			store->kind = MB_DATA_ATTITUDE;
			status = mbr_rsn8kmba_rd_attitude(verbose, store, (int)recordlength, error);
			}
		}

	    /* else set error */
	    else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
	    }
		
	/* get file position */
	mb_io_ptr->file_bytes = ftell(mb_io_ptr->mbfp);

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
int mbr_rsn8kmba_wr_data(int verbose, void *mbio_ptr, char *data_ptr, int *error)
{
	char	*function_name = "mbr_rsn8kmba_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_reson8k_struct *store;
	FILE	*mbfp;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to data structure */
	store = (struct mbsys_reson8k_struct *) mb_io_ptr->store_data;
	mbfp = mb_io_ptr->mbfp;

	if (store->kind == MB_DATA_PARAMETER)
		{
		status = mbr_rsn8kmba_wr_parameter(verbose, mbfp, mb_io_ptr->store_data, error);
		}
	else if (store->kind == MB_DATA_COMMENT)
		{
		status = mbr_rsn8kmba_wr_comment(verbose, mbfp, mb_io_ptr->store_data, error);
		}
	else if (store->kind == MB_DATA_DATA)
		{
		status = mbr_rsn8kmba_wr_bath(verbose, mbfp, mb_io_ptr->store_data, error);
		}
	else if (store->kind == MB_DATA_NAV)
		{
		status = mbr_rsn8kmba_wr_nav(verbose, mbfp, mb_io_ptr->store_data, error);
		}
	else if (store->kind == MB_DATA_VELOCITY_PROFILE)
		{
		status = mbr_rsn8kmba_wr_svp(verbose, mbfp, mb_io_ptr->store_data, error);
		}
	else if (store->kind == MB_DATA_ATTITUDE)
		{
		status = mbr_rsn8kmba_wr_attitude(verbose, mbfp, mb_io_ptr->store_data, error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_KIND;
		}

	/* print output debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Data record kind in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       kind:       %d\n",store->kind);
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
int mbr_rsn8kmba_wr_comment(int verbose, FILE *mbfp, char *store_data, int *error)
{
	char	*function_name = "mbr_cbat8101_wr_comment";
	int	status = MB_SUCCESS;
	struct mbsys_reson8k_struct *store;
	char	line[MBSYS_RESON8K_COMMENT_LENGTH];
	char	label[10];
	unsigned short int *recordlength;
	int	write_len, comment_len;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store_data: %d\n",store_data);
		}

	/* get pointer to data structure */
	store = (struct mbsys_reson8k_struct *) store_data;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       comment:          %s\n",store->comment);
		}

	/* write the record label */
	strncpy(label, "RSN8KMBACM", 10);
	write_len = MBSYS_RESON8K_COMMENT_LENGTH + 8;
	mb_put_binary_short(MB_NO, (unsigned short)write_len, (void *) &line[10]);
	if ((status = fwrite(label,1,MBF_RSN8KMBA_BEGINRECORD_LENGTH,mbfp)) 
		!= MBF_RSN8KMBA_BEGINRECORD_LENGTH)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}

	/* write out the comment and eor tag */
	if (status == MB_SUCCESS)
		{
		/* construct record */
		comment_len = MIN(strlen(store->comment), MBSYS_RESON8K_COMMENT_LENGTH);
		for (i=0;i<comment_len;i++)
			line[i] = store->comment[i];
		for (i=comment_len;i<MBSYS_RESON8K_COMMENT_LENGTH;i++)
			line[i] = '\0';
		strncpy(&line[MBSYS_RESON8K_COMMENT_LENGTH], "ENDRECRD", 8);

		/* write out data */
		if ((status = fwrite(line,1,write_len,mbfp))
			!= write_len)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
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
int mbr_rsn8kmba_wr_parameter(int verbose, FILE *mbfp, char *store_data, int *error)
{
	char	*function_name = "mbr_rsn8kmba_wr_parameter";
	int	status = MB_SUCCESS;
	struct mbsys_reson8k_struct *store;
	char	line[MBF_RSN8KMBA_PARAMETER_LENGTH];
	char	label[10];
	unsigned short int *recordlength;
	int	write_len, comment_len;
	int	index;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store_data: %d\n",store_data);
		}

	/* get pointer to data structure */
	store = (struct mbsys_reson8k_struct *) store_data;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       comment:          %s\n",store->comment);
		}

	/* write the record label */
	strncpy(label, "RSN8KMBAPR", 10);
	write_len = MBF_RSN8KMBA_PARAMETER_LENGTH;
	mb_put_binary_short(MB_NO, (unsigned short)write_len, (void *) &line[10]);
	if ((status = fwrite(label,1,MBF_RSN8KMBA_BEGINRECORD_LENGTH,mbfp)) 
		!= MBF_RSN8KMBA_BEGINRECORD_LENGTH)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}

	/* write out the comment and eor tag */
	if (status == MB_SUCCESS)
		{
		/* construct record */
		index = 0;
		mb_put_binary_int(MB_NO, (int)store->sonar, (void *) &line[index]);
		index += 4;
		mb_put_binary_float(MB_NO, (float)store->MBOffsetX, (void *) &line[index]);
		index += 4;
		mb_put_binary_float(MB_NO, (float)store->MBOffsetY, (void *) &line[index]);
		index += 4;
		mb_put_binary_float(MB_NO, (float)store->MBOffsetZ, (void *) &line[index]);
		index += 4;
		mb_put_binary_float(MB_NO, (float)store->NavLatency, (void *) &line[index]);
		index += 4;
		mb_put_binary_float(MB_NO, (float)store->NavOffsetX, (void *) &line[index]);
		index += 4;
		mb_put_binary_float(MB_NO, (float)store->NavOffsetY, (void *) &line[index]);
		index += 4;
		mb_put_binary_float(MB_NO, (float)store->NavOffsetZ, (void *) &line[index]);
		index += 4;
		mb_put_binary_float(MB_NO, (float)store->NavOffsetYaw, (void *) &line[index]);
		index += 4;
		mb_put_binary_float(MB_NO, (float)store->MRUOffsetX, (void *) &line[index]);
		index += 4;
		mb_put_binary_float(MB_NO, (float)store->MRUOffsetY, (void *) &line[index]);
		index += 4;
		mb_put_binary_float(MB_NO, (float)store->MRUOffsetZ, (void *) &line[index]);
		index += 4;
		mb_put_binary_float(MB_NO, (float)store->MRUOffsetPitch, (void *) &line[index]);
		index += 4;
		mb_put_binary_float(MB_NO, (float)store->MRUOffsetRoll, (void *) &line[index]);
		index += 4;
		strncpy(&line[index], "ENDRECRD", 8);
		index += 8;

		/* write out data */
		if ((status = fwrite(line,1,write_len,mbfp))
			!= write_len)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
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
int mbr_rsn8kmba_wr_nav(int verbose, FILE *mbfp, char *store_data, int *error)
{
	char	*function_name = "mbr_rsn8kmba_wr_nav";
	int	status = MB_SUCCESS;
	struct mbsys_reson8k_struct *store;
	char	line[MBF_RSN8KMBA_NAV_LENGTH];
	char	label[10];
	unsigned short int *recordlength;
	int	write_len, comment_len;
	int	index;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store_data: %d\n",store_data);
		}

	/* get pointer to data structure */
	store = (struct mbsys_reson8k_struct *) store_data;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       nav_time_d:       %f\n",store->nav_time_d);
		fprintf(stderr,"dbg5       nav_longitude:    %f\n",store->nav_longitude);
		fprintf(stderr,"dbg5       nav_latitude:     %f\n",store->nav_latitude);
		fprintf(stderr,"dbg5       nav_heading:      %f\n",store->nav_heading);
		}

	/* write the record label */
	strncpy(label, "RSN8KMBANV", 10);
	write_len = MBF_RSN8KMBA_NAV_LENGTH;
	mb_put_binary_short(MB_NO, (unsigned short)write_len, (void *) &line[10]);
	if ((status = fwrite(label,1,MBF_RSN8KMBA_BEGINRECORD_LENGTH,mbfp)) 
		!= MBF_RSN8KMBA_BEGINRECORD_LENGTH)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}

	/* write out the nav data and eor tag */
	if (status == MB_SUCCESS)
		{
		/* construct record */
		index = 0;
		mb_put_binary_double(MB_NO, (double)store->nav_time_d, (void *) &line[index]);
		index += 8;
		mb_put_binary_double(MB_NO, (double)store->nav_longitude, (void *) &line[index]);
		index += 8;
		mb_put_binary_double(MB_NO, (double)store->nav_latitude, (void *) &line[index]);
		index += 8;
		mb_put_binary_float(MB_NO, (float)store->nav_heading, (void *) &line[index]);
		index += 4;
		strncpy(&line[index], "ENDRECRD", 8);
		index += 8;

		/* write out data */
		if ((status = fwrite(line,1,write_len,mbfp))
			!= write_len)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
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
int mbr_rsn8kmba_wr_attitude(int verbose, FILE *mbfp, char *store_data, int *error)
{
	char	*function_name = "mbr_rsn8kmba_wr_attitude";
	int	status = MB_SUCCESS;
	struct mbsys_reson8k_struct *store;
	char	line[MBF_RSN8KMBA_ATTITUDE_LENGTH];
	char	label[10];
	unsigned short int *recordlength;
	int	write_len, comment_len;
	int	index;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store_data: %d\n",store_data);
		}

	/* get pointer to data structure */
	store = (struct mbsys_reson8k_struct *) store_data;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       att_timetag:       %f\n",store->att_timetag);
		fprintf(stderr,"dbg5       att_heading:       %f\n",store->att_heading);
		fprintf(stderr,"dbg5       att_heave:         %f\n",store->att_heave);
		fprintf(stderr,"dbg5       att_roll:          %f\n",store->att_roll);
		fprintf(stderr,"dbg5       att_pitch:         %f\n",store->att_pitch);
		}

	/* write the record label */
	strncpy(label, "RSN8KMBAAT", 10);
	write_len = MBF_RSN8KMBA_ATTITUDE_LENGTH;
	mb_put_binary_short(MB_NO, (unsigned short)write_len, (void *) &line[10]);
	if ((status = fwrite(label,1,MBF_RSN8KMBA_BEGINRECORD_LENGTH,mbfp)) 
		!= MBF_RSN8KMBA_BEGINRECORD_LENGTH)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}

	/* write out the attitude data and eor tag */
	if (status == MB_SUCCESS)
		{
		/* construct record */
		index = 0;
		mb_put_binary_double(MB_NO, (double)store->att_timetag, (void *) &line[index]);
		index += 8;
		mb_put_binary_float(MB_NO, (float)store->att_heading, (void *) &line[index]);
		index += 4;
		mb_put_binary_float(MB_NO, (float)store->att_heave, (void *) &line[index]);
		index += 4;
		mb_put_binary_float(MB_NO, (float)store->att_roll, (void *) &line[index]);
		index += 4;
		mb_put_binary_float(MB_NO, (float)store->att_pitch, (void *) &line[index]);
		index += 4;
		strncpy(&line[index], "ENDRECRD", 8);
		index += 8;

		/* write out data */
		if ((status = fwrite(line,1,write_len,mbfp))
			!= write_len)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
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
int mbr_rsn8kmba_wr_svp(int verbose, FILE *mbfp, char *store_data, int *error)
{
	char	*function_name = "mbr_rsn8kmba_wr_svp";
	int	status = MB_SUCCESS;
	struct mbsys_reson8k_struct *store;
	char	line[MBF_RSN8KMBA_SVPSTART_LENGTH];
	char	label[10];
	unsigned short int *recordlength;
	int	write_len, comment_len;
	int	index;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store_data: %d\n",store_data);
		}

	/* get pointer to data structure */
	store = (struct mbsys_reson8k_struct *) store_data;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       svp_time_d:        %f\n",store->svp_time_d);
		fprintf(stderr,"dbg5       svp_num:           %d\n",store->svp_num);
		for (i=0;i<store->svp_num;i++)
			fprintf(stderr,"dbg5            i:%d depth:%f vel:%f\n",
			i, store->svp_depth[i], store->svp_vel[i]);
		}

	/* write the record label */
	strncpy(label, "RSN8KMBASV", 10);
	write_len = MBF_RSN8KMBA_SVPSTART_LENGTH 
			+ MBF_RSN8KMBA_SVP_LENGTH * store->svp_num 
			+ MBF_RSN8KMBA_ENDRECORD_LENGTH;
	mb_put_binary_short(MB_NO, (unsigned short)write_len, (void *) &line[10]);
	if ((status = fwrite(label,1,MBF_RSN8KMBA_BEGINRECORD_LENGTH,mbfp)) 
		!= MBF_RSN8KMBA_BEGINRECORD_LENGTH)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}

	/* write out the attitude data and eor tag */
	if (status == MB_SUCCESS)
		{
		/* construct record */
		for (i=0;i<store->svp_num;i++)
			{
			index = 0;
			mb_put_binary_float(MB_NO, (float)store->svp_depth[i], (void *) &line[index]);
			index += 4;
			mb_put_binary_float(MB_NO, (float)store->svp_vel[i], (void *) &line[index]);
			index += 4;
	
			/* write out data */
			write_len = 8;
			if ((status = fwrite(line,1,write_len,mbfp))
				!= write_len)
				{
				*error = MB_ERROR_WRITE_FAIL;
				status = MB_FAILURE;
				}
			else
				{
				*error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
				}
			}
			
		/* set up end of record */
		index = 0;
		strncpy(&line[index], "ENDRECRD", 8);
		index += 8;

		/* write out data */
		write_len = 8;
		if ((status = fwrite(line,1,write_len,mbfp))
			!= write_len)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
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
int mbr_rsn8kmba_wr_bath(int verbose, FILE *mbfp, char *store_data, int *error)
{
	char	*function_name = "mbr_rsn8kmba_wr_bath";
	int	status = MB_SUCCESS;
	struct mbsys_reson8k_struct *store;
	char	line[MBF_RSN8KMBA_SVPSTART_LENGTH];
	char	label[10];
	unsigned short int *recordlength;
	int	write_len, comment_len;
	int	index;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store_data: %d\n",store_data);
		}

	/* get pointer to data structure */
	store = (struct mbsys_reson8k_struct *) store_data;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       svp_time_d:        %f\n",store->svp_time_d);
		fprintf(stderr,"dbg5       svp_num:           %d\n",store->svp_num);
		for (i=0;i<store->svp_num;i++)
			fprintf(stderr,"dbg5            i:%d depth:%f vel:%f\n",
			i, store->svp_depth[i], store->svp_vel[i]);
		}

	/* write the record label */
	strncpy(label, "RSN8KMBASV", 10);
	write_len = MBF_RSN8KMBA_BATHSTART_LENGTH + MBF_RSN8KMBA_BEAM_LENGTH * store->svp_num + 8;
	mb_put_binary_short(MB_NO, (unsigned short)write_len, (void *) &line[10]);
	if ((status = fwrite(label,1,MBF_RSN8KMBA_BEGINRECORD_LENGTH,mbfp)) 
		!= MBF_RSN8KMBA_BEGINRECORD_LENGTH)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}

	/* write out the attitude data and eor tag */
	if (status == MB_SUCCESS)
		{
		/* construct record */
		for (i=0;i<store->svp_num;i++)
			{
			index = 0;
			mb_put_binary_float(MB_NO, (float)store->svp_depth[i], (void *) &line[index]);
			index += 4;
			mb_put_binary_float(MB_NO, (float)store->svp_vel[i], (void *) &line[index]);
			index += 4;
	
			/* write out data */
			write_len = 8;
			if ((status = fwrite(line,1,write_len,mbfp))
				!= write_len)
				{
				*error = MB_ERROR_WRITE_FAIL;
				status = MB_FAILURE;
				}
			else
				{
				*error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
				}
			}
			
		/* set up end of record */
		index = 0;
		strncpy(&line[index], "ENDRECRD", 8);
		index += 8;

		/* write out data */
		write_len = 8;
		if ((status = fwrite(line,1,write_len,mbfp))
			!= write_len)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
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
