/*--------------------------------------------------------------------
 *    The MB-system:	mbr_hir2rnav.c	5/20/99
 *	$Id: mbr_hir2rnav.c 1829 2010-02-05 02:53:39Z caress $
 *
 *    Copyright (c) 1999-2011 by
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
 * mbr_hir2rnav.c contains the functions for reading and writing
 * multibeam data in the HIR2RNAV format.
 * These functions include:
 *   mbr_alm_hir2rnav	- allocate read/write memory
 *   mbr_dem_hir2rnav	- deallocate read/write memory
 *   mbr_rt_hir2rnav	- read and translate data
 *   mbr_wt_hir2rnav	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	May 20, 1999
 *
 * $Log: mbr_hir2rnav.c,v $
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
#include "../../include/mbsys_singlebeam.h"

/* essential function prototypes */
int mbr_register_hir2rnav(int verbose, void *mbio_ptr, 
		int *error);
int mbr_info_hir2rnav(int verbose, 
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
int mbr_alm_hir2rnav(int verbose, void *mbio_ptr, int *error);
int mbr_dem_hir2rnav(int verbose, void *mbio_ptr, int *error);
int mbr_rt_hir2rnav(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_hir2rnav(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_hir2rnav_rd_data(int verbose, void *mbio_ptr, int *error);
int mbr_hir2rnav_wr_data(int verbose, void *mbio_ptr, void *data_ptr, int *error);

static char rcs_id[]="$Id: mbr_hir2rnav.c 1829 2010-02-05 02:53:39Z caress $";

/*--------------------------------------------------------------------*/
int mbr_register_hir2rnav(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_register_hir2rnav";
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
	status = mbr_info_hir2rnav(verbose, 
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_hir2rnav;
	mb_io_ptr->mb_io_format_free = &mbr_dem_hir2rnav; 
	mb_io_ptr->mb_io_store_alloc = &mbsys_singlebeam_alloc; 
	mb_io_ptr->mb_io_store_free = &mbsys_singlebeam_deall; 
	mb_io_ptr->mb_io_read_ping = &mbr_rt_hir2rnav; 
	mb_io_ptr->mb_io_write_ping = &mbr_wt_hir2rnav; 
	mb_io_ptr->mb_io_dimensions = &mbsys_singlebeam_dimensions; 
	mb_io_ptr->mb_io_extract = &mbsys_singlebeam_extract; 
	mb_io_ptr->mb_io_insert = &mbsys_singlebeam_insert; 
	mb_io_ptr->mb_io_extract_nav = &mbsys_singlebeam_extract_nav; 
	mb_io_ptr->mb_io_insert_nav = &mbsys_singlebeam_insert_nav; 
	mb_io_ptr->mb_io_extract_altitude = &mbsys_singlebeam_extract_altitude; 
	mb_io_ptr->mb_io_insert_altitude = NULL; 
	mb_io_ptr->mb_io_extract_svp = NULL; 
	mb_io_ptr->mb_io_insert_svp = NULL; 
	mb_io_ptr->mb_io_ttimes = &mbsys_singlebeam_ttimes; 
	mb_io_ptr->mb_io_copyrecord = &mbsys_singlebeam_copy; 
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
		fprintf(stderr,"dbg2       format_alloc:       %lu\n",(size_t)mb_io_ptr->mb_io_format_alloc);
		fprintf(stderr,"dbg2       format_free:        %lu\n",(size_t)mb_io_ptr->mb_io_format_free);
		fprintf(stderr,"dbg2       store_alloc:        %lu\n",(size_t)mb_io_ptr->mb_io_store_alloc);
		fprintf(stderr,"dbg2       store_free:         %lu\n",(size_t)mb_io_ptr->mb_io_store_free);
		fprintf(stderr,"dbg2       read_ping:          %lu\n",(size_t)mb_io_ptr->mb_io_read_ping);
		fprintf(stderr,"dbg2       write_ping:         %lu\n",(size_t)mb_io_ptr->mb_io_write_ping);
		fprintf(stderr,"dbg2       extract:            %lu\n",(size_t)mb_io_ptr->mb_io_extract);
		fprintf(stderr,"dbg2       insert:             %lu\n",(size_t)mb_io_ptr->mb_io_insert);
		fprintf(stderr,"dbg2       extract_nav:        %lu\n",(size_t)mb_io_ptr->mb_io_extract_nav);
		fprintf(stderr,"dbg2       insert_nav:         %lu\n",(size_t)mb_io_ptr->mb_io_insert_nav);
		fprintf(stderr,"dbg2       extract_altitude:   %lu\n",(size_t)mb_io_ptr->mb_io_extract_altitude);
		fprintf(stderr,"dbg2       insert_altitude:    %lu\n",(size_t)mb_io_ptr->mb_io_insert_altitude);
		fprintf(stderr,"dbg2       extract_svp:        %lu\n",(size_t)mb_io_ptr->mb_io_extract_svp);
		fprintf(stderr,"dbg2       insert_svp:         %lu\n",(size_t)mb_io_ptr->mb_io_insert_svp);
		fprintf(stderr,"dbg2       ttimes:             %lu\n",(size_t)mb_io_ptr->mb_io_ttimes);
		fprintf(stderr,"dbg2       extract_rawss:      %lu\n",(size_t)mb_io_ptr->mb_io_extract_rawss);
		fprintf(stderr,"dbg2       insert_rawss:       %lu\n",(size_t)mb_io_ptr->mb_io_insert_rawss);
		fprintf(stderr,"dbg2       copyrecord:         %lu\n",(size_t)mb_io_ptr->mb_io_copyrecord);
		fprintf(stderr,"dbg2       error:              %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		}

	/* return status */
	return(status);
}

/*--------------------------------------------------------------------*/
int mbr_info_hir2rnav(int verbose, 
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
	char	*function_name = "mbr_info_hir2rnav";
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
	*system = MB_SYS_SINGLEBEAM;
	*beams_bath_max = 0;
	*beams_amp_max = 0;
	*pixels_ss_max = 0;
	strncpy(format_name, "HIR2RNAV", MB_NAME_LENGTH);
	strncpy(system_name, "SINGLEBEAM", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_HIR2RNAV\nInformal Description: SIO GDC R2R navigation format\nAttributes:           R2R navigation, ascii, SIO\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_NO;
	*traveltime = MB_NO;
	*beam_flagging = MB_NO;
	*nav_source = MB_DATA_DATA;
	*heading_source = MB_DATA_NONE;
	*vru_source = MB_DATA_NONE;
	*svp_source = MB_DATA_NONE;
	*beamwidth_xtrack = 0.0;
	*beamwidth_ltrack = 0.0;

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
int mbr_alm_hir2rnav(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_alm_hir2rnav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set initial status */
	status = MB_SUCCESS;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = 0;
	mb_io_ptr->data_structure_size = 0;
	status = mbsys_singlebeam_alloc(verbose, mbio_ptr, &(mb_io_ptr->store_data), error);
	
	/* set number of records read or written to zero */
	mb_io_ptr->save1 = 0;

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
int mbr_dem_hir2rnav(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_hir2rnav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* deallocate memory for data descriptor */
	status = mbsys_singlebeam_deall(verbose, mbio_ptr, &(mb_io_ptr->store_data), error);

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
int mbr_rt_hir2rnav(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_hir2rnav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_singlebeam_struct *store;
	char	line[MB_PATH_MAXLINE];
	char	*line_ptr;
	int	*read_count;
	int	nget;
	double	sec;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get pointers to mbio descriptor and data structure */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	store = (struct mbsys_singlebeam_struct *) store_ptr;
	
	/* get pointer to read counter */
	read_count = (int *) &mb_io_ptr->save1;
	    
	/* set file position */
	mb_io_ptr->file_bytes = ftell(mb_io_ptr->mbfp);
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* read next record */
	if ((line_ptr = fgets(line, MB_PATH_MAXLINE, 
			mb_io_ptr->mbfp)) != NULL) 
		{
		/* set status */
		mb_io_ptr->file_bytes += strlen(line);
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
	mb_io_ptr->file_bytes = ftell(mb_io_ptr->mbfp);

	/* handle the data */
	if (status == MB_SUCCESS
	    && line[0] == '#')
	    {
	    store->kind = MB_DATA_COMMENT;
            strncpy(store->comment,&line[1],MBSYS_SINGLEBEAM_MAXLINE);
	    if (store->comment[strlen(store->comment)-1] == '\n')
		store->comment[strlen(store->comment)-1] = '\0';
	    (*read_count)++;
	    }
	else if (status == MB_SUCCESS)
	    {
	    store->kind = MB_DATA_DATA;

	    /* read data */
	    nget = sscanf(line,"%d-%d-%dT%d:%d:%lfZ %lf %lf %d %d %lf %d",
		    &store->time_i[0],&store->time_i[1],&store->time_i[2],
		    &store->time_i[3],&store->time_i[4],&sec,
		    &store->longitude,&store->latitude, 
		    &store->gps_quality,&store->gps_nsat,
		    &store->gps_dilution,&store->gps_height);
/* fprintf(stderr,"\nLINE:%s\tnget:%d %d/%d/%d %d:%d:%f  lon:%f lat:%f  gps:%d %d %f %d\n",
line,nget,
store->time_i[0],store->time_i[1],store->time_i[2],
store->time_i[3],store->time_i[4],sec,
store->longitude,store->latitude, 
store->gps_quality,store->gps_nsat,
store->gps_dilution,store->gps_height);*/
	    if (nget != 12)
		    {
		    store->gps_quality = 0;
		    store->gps_nsat = 0;
		    store->gps_dilution = 0.0;
		    store->gps_height = 0;
		    }
	    if ((nget == 8 || nget == 12) && store->time_i[0] != 0)
		    {
	    	    status = MB_SUCCESS;
	   	    *error = MB_ERROR_NO_ERROR;

	    	    store->time_i[5] = (int) floor(sec);
	    	    store->time_i[6] = (int)((sec - store->time_i[5]) * 1000000);
	    	    mb_get_time(verbose,store->time_i,&store->time_d);
	    	    (*read_count)++;
	    	    }

	    /* catch erroneous records */
	    else
	    	{
	    	status = MB_FAILURE;
	   	*error = MB_ERROR_UNINTELLIGIBLE;
		}		
	    }
	    
	/* print output debug statements */
	if (status == MB_SUCCESS && verbose >= 4)
	    {
	    if (store->kind == MB_DATA_DATA)
		{
		fprintf(stderr,"\ndbg4  Data read in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg4  Values read:\n");
		fprintf(stderr,"dbg4       time_i[0]:    %d\n",store->time_i[0]);
		fprintf(stderr,"dbg4       time_i[1]:    %d\n",store->time_i[1]);
		fprintf(stderr,"dbg4       time_i[2]:    %d\n",store->time_i[2]);
		fprintf(stderr,"dbg4       time_i[3]:    %d\n",store->time_i[3]);
		fprintf(stderr,"dbg4       time_i[4]:    %d\n",store->time_i[4]);
		fprintf(stderr,"dbg4       time_i[5]:    %d\n",store->time_i[5]);
		fprintf(stderr,"dbg4       time_i[6]:    %d\n",store->time_i[6]);
		fprintf(stderr,"dbg4       time_d:       %f\n",store->time_d);
		fprintf(stderr,"dbg4       longitude:    %f\n",store->longitude);
		fprintf(stderr,"dbg4       latitude:     %f\n",store->latitude);
		fprintf(stderr,"dbg4       gps_quality:  %d\n",store->gps_quality);
		fprintf(stderr,"dbg4       gps_nsat:     %d\n",store->gps_nsat);
		fprintf(stderr,"dbg4       gps_dilution: %f\n",store->gps_dilution);
		fprintf(stderr,"dbg4       gps_height:   %d\n",store->gps_height);
		fprintf(stderr,"dbg4       error:        %d\n",*error);
		fprintf(stderr,"dbg4       status:       %d\n",status);
		}
	    else if (store->kind == MB_DATA_COMMENT)
		{
		fprintf(stderr,"\ndbg4  Data read in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg4  Values read:\n");
		fprintf(stderr,"dbg4       comment:      %s\n",store->comment);
		}
	    }

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = store->kind;

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
int mbr_wt_hir2rnav(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_hir2rnav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_singlebeam_struct *store;
	char	line[MB_PATH_MAXLINE];
	int	*write_count;
	int	len;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get pointer to mbio descriptor and data structure*/
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	store = (struct mbsys_singlebeam_struct *) store_ptr;
	
	/* get pointer to write counter */
	write_count = (int *) &mb_io_ptr->save1;
	    
	/* print output debug statements */
	if (store != NULL && verbose >= 4)
	    {
	    if (store->kind == MB_DATA_DATA)
		{
		fprintf(stderr,"\ndbg4  Data to be written in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg4  Values to be written:\n");
		fprintf(stderr,"dbg4       time_i[0]:    %d\n",store->time_i[0]);
		fprintf(stderr,"dbg4       time_i[1]:    %d\n",store->time_i[1]);
		fprintf(stderr,"dbg4       time_i[2]:    %d\n",store->time_i[2]);
		fprintf(stderr,"dbg4       time_i[3]:    %d\n",store->time_i[3]);
		fprintf(stderr,"dbg4       time_i[4]:    %d\n",store->time_i[4]);
		fprintf(stderr,"dbg4       time_i[5]:    %d\n",store->time_i[5]);
		fprintf(stderr,"dbg4       time_i[6]:    %d\n",store->time_i[6]);
		fprintf(stderr,"dbg4       time_d:       %f\n",store->time_d);
		fprintf(stderr,"dbg4       longitude:    %f\n",store->longitude);
		fprintf(stderr,"dbg4       latitude:     %f\n",store->latitude);
		fprintf(stderr,"dbg4       gps_quality:  %d\n",store->gps_quality);
		fprintf(stderr,"dbg4       gps_nsat:     %d\n",store->gps_nsat);
		fprintf(stderr,"dbg4       gps_dilution: %f\n",store->gps_dilution);
		fprintf(stderr,"dbg4       gps_height:   %d\n",store->gps_height);
		fprintf(stderr,"dbg4       error:        %d\n",*error);
		fprintf(stderr,"dbg4       status:       %d\n",status);
		}
	    else if (store->kind == MB_DATA_COMMENT)
		{
		fprintf(stderr,"\ndbg4  Data to be written in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg4  Values to be written:\n");
		fprintf(stderr,"dbg4       comment:      %s\n",store->comment);
		}
	    }

	/* write the record */
	if (store != NULL)
		{
		/* deal with comment */
		if (store->kind == MB_DATA_COMMENT)
		    {
		    line[0] = '#';
        	    strncpy(&line[1],store->comment,MBSYS_SINGLEBEAM_MAXLINE-2);
        	    len = strlen(line);
		    if (line[len-1] != '\n')
			{
			line[len] = '\n';
			line[len+1] = '\0';
			}
		    }
		else if (store->kind == MB_DATA_DATA)
		    {
		    /* deal with data */
		    if (store->gps_nsat > 0)
			    sprintf(line,"%4.4d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2d.%3.3dZ\t%11.6f\t%10.6f\t%d\t%d\t%.1f\t%d\n",
				    store->time_i[0],store->time_i[1],store->time_i[2],
				    store->time_i[3],store->time_i[4],store->time_i[5],1000*store->time_i[6],
				    store->longitude,store->latitude, 
				    store->gps_quality,store->gps_nsat,
				    store->gps_dilution,store->gps_height);
		    else
			    sprintf(line,"%4.4d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2d.%3.3dZ\t%11.6f\t%10.6f\n",
				    store->time_i[0],store->time_i[1],store->time_i[2],
				    store->time_i[3],store->time_i[4],store->time_i[5],1000*store->time_i[6],
				    store->longitude,store->latitude);
		    }

		/* write data */
		if (fputs(line,mb_io_ptr->mbfp) == EOF)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			(*write_count)++;
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
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
