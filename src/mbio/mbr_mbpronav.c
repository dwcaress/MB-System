/*--------------------------------------------------------------------
 *    The MB-system:	mbr_mbpronav.c	5/20/99
 *	$Id: mbr_mbpronav.c,v 5.1 2000-12-10 20:26:50 caress Exp $
 *
 *    Copyright (c) 1999, 2000 by
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
 * mbr_mbpronav.c contains the functions for reading and writing
 * navigation data in the MBPRONAV format.
 * These functions include:
 *   mbr_alm_mbpronav	- allocate read/write memory
 *   mbr_dem_mbpronav	- deallocate read/write memory
 *   mbr_rt_mbpronav	- read and translate data
 *   mbr_wt_mbpronav	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	October 18, 1999
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.3  2000/10/11  01:03:21  caress
 * Convert to ANSI C
 *
 * Revision 4.2  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.1  1999/12/29  00:34:06  caress
 * Release 4.6.8
 *
 * Revision 4.0  1999/10/21  22:39:24  caress
 * Added MBPRONAV format.
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
#include "../../include/mbf_mbpronav.h"

/* essential function prototypes */
int mbr_info_mbpronav(int verbose, 
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
			int (**format_alloc)(), 
			int (**format_free)(), 
			int (**store_alloc)(), 
			int (**store_free)(), 
			int (**read_ping)(), 
			int (**write_ping)(), 
			int (**extract)(), 
			int (**insert)(), 
			int (**extract_nav)(), 
			int (**insert_nav)(), 
			int (**altitude)(), 
			int (**insert_altitude)(), 
			int (**ttimes)(), 
			int (**copyrecord)(), 
			int *error);
int mbr_alm_mbpronav(int verbose, char *mbio_ptr, int *error);
int mbr_dem_mbpronav(int verbose, char *mbio_ptr, int *error);
int mbr_rt_mbpronav(int verbose, char *mbio_ptr, char *store_ptr, int *error);
int mbr_wt_mbpronav(int verbose, char *mbio_ptr, char *store_ptr, int *error);

/*--------------------------------------------------------------------*/
int mbr_info_mbpronav(int verbose, 
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
			int (**format_alloc)(), 
			int (**format_free)(), 
			int (**store_alloc)(), 
			int (**store_free)(), 
			int (**read_ping)(), 
			int (**write_ping)(), 
			int (**extract)(), 
			int (**insert)(), 
			int (**extract_nav)(), 
			int (**insert_nav)(), 
			int (**altitude)(), 
			int (**insert_altitude)(), 
			int (**ttimes)(), 
			int (**copyrecord)(), 
			int *error)
{
	static char res_id[]="$Id: mbr_mbpronav.c,v 5.1 2000-12-10 20:26:50 caress Exp $";
	char	*function_name = "mbr_info_mbpronav";
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
	*system = MB_SYS_SINGLEBEAM;
	*beams_bath_max = 1;
	*beams_amp_max = 0;
	*pixels_ss_max = 0;
	strncpy(format_name, "MBPRONAV", MB_NAME_LENGTH);
	strncpy(system_name, "SINGLEBEAM", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_MBPRONAV\nInformal Description: MB-System simple navigation format\nAttributes:           navigation, MBARI\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_NO;
	*traveltime = MB_YES;
	*beam_flagging = MB_NO;
	*nav_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_DATA;
	*beamwidth_xtrack = 0.0;
	*beamwidth_ltrack = 0.0;

	/* set format and system specific function pointers */
	*format_alloc = &mbr_alm_mbpronav;
	*format_free = &mbr_dem_mbpronav; 
	*store_alloc = &mbsys_singlebeam_alloc; 
	*store_free = &mbsys_singlebeam_deall; 
	*read_ping = &mbr_rt_mbpronav; 
	*write_ping = &mbr_wt_mbpronav; 
	*extract = &mbsys_singlebeam_extract; 
	*insert = &mbsys_singlebeam_insert; 
	*extract_nav = &mbsys_singlebeam_extract_nav; 
	*insert_nav = &mbsys_singlebeam_insert_nav; 
	*altitude = &mbsys_singlebeam_altitude; 
	*insert_altitude = NULL;
	*ttimes = &mbsys_singlebeam_ttimes; 
	*copyrecord = &mbsys_singlebeam_copy; 

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
		fprintf(stderr,"dbg2       format_alloc:       %d\n",*format_alloc);
		fprintf(stderr,"dbg2       format_free:        %d\n",*format_free);
		fprintf(stderr,"dbg2       store_alloc:        %d\n",*store_alloc);
		fprintf(stderr,"dbg2       store_free:         %d\n",*store_free);
		fprintf(stderr,"dbg2       read_ping:          %d\n",*read_ping);
		fprintf(stderr,"dbg2       write_ping:         %d\n",*write_ping);
		fprintf(stderr,"dbg2       extract:            %d\n",*extract);
		fprintf(stderr,"dbg2       insert:             %d\n",*insert);
		fprintf(stderr,"dbg2       extract_nav:        %d\n",*extract_nav);
		fprintf(stderr,"dbg2       insert_nav:         %d\n",*insert_nav);
		fprintf(stderr,"dbg2       altitude:           %d\n",*altitude);
		fprintf(stderr,"dbg2       insert_altitude:    %d\n",*insert_altitude);
		fprintf(stderr,"dbg2       ttimes:             %d\n",*ttimes);
		fprintf(stderr,"dbg2       copyrecord:         %d\n",*copyrecord);
		fprintf(stderr,"dbg2       error:              %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		}

	/* return status */
	return(status);
}


/*--------------------------------------------------------------------*/
int mbr_alm_mbpronav(int verbose, char *mbio_ptr, int *error)
{
 static char res_id[]="$Id: mbr_mbpronav.c,v 5.1 2000-12-10 20:26:50 caress Exp $";
	char	*function_name = "mbr_alm_mbpronav";
	int	status = MB_SUCCESS;
	int	i;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mbpronav_struct *data;
	char	*data_ptr;

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
	mb_io_ptr->structure_size = sizeof(struct mbf_mbpronav_struct);
	mb_io_ptr->data_structure_size = 0;
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mb_malloc(verbose,sizeof(struct mbsys_singlebeam_struct),
				&mb_io_ptr->store_data,error);

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	data = (struct mbf_mbpronav_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;
	
	/* set number of header records read to zero */
	mb_io_ptr->save1 = 0;

	/* initialize everything to zeros */
	mbr_zero_mbpronav(verbose,data_ptr,error);

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
int mbr_dem_mbpronav(int verbose, char *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_mbpronav";
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

	/* deallocate memory for data descriptor */
	status = mb_free(verbose,&mb_io_ptr->raw_data,error);
	status = mb_free(verbose,&mb_io_ptr->store_data,error);

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
int mbr_zero_mbpronav(int verbose, char *data_ptr, int *error)
{
	char	*function_name = "mbr_zero_mbpronav";
	int	status = MB_SUCCESS;
	struct mbf_mbpronav_struct *data;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to data descriptor */
	data = (struct mbf_mbpronav_struct *) data_ptr;

	/* initialize everything to zeros */
	if (data != NULL)
		{
		data->kind = MB_DATA_NONE;
		data->time_d = 0.0;
		for (i=0;i<7;i++)
		    data->time_i[i] = 0;
		data->longitude = 0.0;
		data->latitude = 0.0;
		data->heading = 0.0;
		data->speed = 0.0;
		data->sonardepth = 0.0;
		data->roll = 0.0;
		data->pitch = 0.0;
		data->heave = 0.0;
		for (i=0;i<MBF_MBPRONAV_MAXLINE;i++)
		    data->comment[i] = 0;
		}

	/* assume success */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

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
int mbr_rt_mbpronav(int verbose, char *mbio_ptr, char *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_mbpronav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mbpronav_struct *data;
	struct mbsys_singlebeam_struct *store;
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
	data = (struct mbf_mbpronav_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_singlebeam_struct *) store_ptr;

	/* read next data from file */
	status = mbr_mbpronav_rd_data(verbose,mbio_ptr,error);

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = data->kind;

	/* translate values to data storage structure */
	if (status == MB_SUCCESS
		&& store != NULL)
		{
		store->kind = data->kind;
		store->time_d = data->time_d;
		for (i=0;i<7;i++)
		    store->time_i[i] = data->time_i[i];
		store->longitude = data->longitude;
		store->latitude = data->latitude;
		store->heading = data->heading;
		store->speed = data->speed;
		store->sonar_depth = data->sonardepth;
		store->roll = data->roll;
		store->pitch = data->pitch;
		store->heave = data->heave;
        	for (i=0;i<MBSYS_SINGLEBEAM_MAXLINE;i++)
		    store->comment[i] = data->comment[i];
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
int mbr_wt_mbpronav(int verbose, char *mbio_ptr, char *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_mbpronav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mbpronav_struct *data;
	struct mbsys_singlebeam_struct *store;
	int	i;

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

	/* get pointer to raw data structure */
	data = (struct mbf_mbpronav_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_singlebeam_struct *) store_ptr;

	/* first translate values from data storage structure */
	if (store != NULL)
		{
		data->kind = store->kind;
		data->time_d = store->time_d;
		for (i=0;i<7;i++)
		    data->time_i[i] = store->time_i[i];
		data->longitude = store->longitude;
		data->latitude = store->latitude;
		data->heading = store->heading;
		data->speed = store->speed;
		data->sonardepth = store->sonar_depth;
		data->roll = store->roll;
		data->pitch = store->pitch;
		data->heave = store->heave;
		for (i=0;i<MBSYS_SINGLEBEAM_MAXLINE;i++)
		    data->comment[i] = store->comment[i];
		}

	/* write next data to file */
	status = mbr_mbpronav_wr_data(verbose,mbio_ptr,data,error);

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
int mbr_mbpronav_rd_data(int verbose, char *mbio_ptr, int *error)
{
	char	*function_name = "mbr_mbpronav_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mbpronav_struct *data;
	char	line[MBF_MBPRONAV_MAXLINE+1];
	int	read_len;
	double	timetag;
	char	*line_ptr;
	int	nread;
	double  sec;
	double  d1, d2, d3, d4, d5;
	double  d6, d7, d8, d9;
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

	/* get pointer to raw data structure */
	data = (struct mbf_mbpronav_struct *) mb_io_ptr->raw_data;

	/* initialize everything to zeros */
	mbr_zero_mbpronav(verbose,data,error);

	/* set file position */
	mb_io_ptr->file_bytes = ftell(mb_io_ptr->mbfp);
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* read next record */
	if ((line_ptr = fgets(line, MBF_MBPRONAV_MAXLINE, 
			mb_io_ptr->mbfp)) != NULL) 
		{
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
	    data->kind = MB_DATA_COMMENT;
            strncpy(data->comment,&line[1],MBF_MBPRONAV_MAXLINE);
	    }
	else if (status == MB_SUCCESS)
	    {
	    data->kind = MB_DATA_DATA;

	    /* read data */
	    nread = sscanf(line,
			"%d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
			&data->time_i[0],
			&data->time_i[1],
			&data->time_i[2],
			&data->time_i[3],
			&data->time_i[4],
			&sec,
			&d1,
			&d2,
			&d3,
			&d4,
			&d5, 
			&d6,
			&d7,
			&d8,
			&d9);
	    data->time_i[5] = (int) sec;
	    data->time_i[6] = 1000000.0 * (sec - data->time_i[5]);
	    if (nread == 8)
	        {
	        mb_get_time(verbose,data->time_i,&data->time_d);
		data->longitude = d1;
		data->latitude = d2;
		data->heading = 0.0;
		data->speed = 0.0;
		data->sonardepth = 0.0;
		data->roll = 0.0;
		data->pitch = 0.0;
		data->heave = 0.0;
	    	status = MB_SUCCESS;
	   	*error = MB_ERROR_NO_ERROR;
		}
	    else if (nread == 9)
	        {
	        data->time_d = d1;
		data->longitude = d2;
		data->latitude = d3;
		data->heading = 0.0;
		data->speed = 0.0;
		data->sonardepth = 0.0;
		data->roll = 0.0;
		data->pitch = 0.0;
		data->heave = 0.0;
	    	status = MB_SUCCESS;
	   	*error = MB_ERROR_NO_ERROR;
		}
	    else if (nread == 10)
	        {
	        data->time_d = d1;
		data->longitude = d2;
		data->latitude = d3;
		data->heading = d4;
		data->speed = 0.0;
		data->sonardepth = 0.0;
		data->roll = 0.0;
		data->pitch = 0.0;
		data->heave = 0.0;
	    	status = MB_SUCCESS;
	   	*error = MB_ERROR_NO_ERROR;
		}
	    else if (nread == 11)
	        {
	        data->time_d = d1;
		data->longitude = d2;
		data->latitude = d3;
		data->heading = d4;
		data->speed = d5;
		data->sonardepth = 0.0;
		data->roll = 0.0;
		data->pitch = 0.0;
		data->heave = 0.0;
	    	status = MB_SUCCESS;
	   	*error = MB_ERROR_NO_ERROR;
		}
	    else if (nread == 12)
	        {
	        data->time_d = d1;
		data->longitude = d2;
		data->latitude = d3;
		data->heading = d4;
		data->speed = d5;
		data->sonardepth = d6;
		data->roll = 0.0;
		data->pitch = 0.0;
		data->heave = 0.0;
	    	status = MB_SUCCESS;
	   	*error = MB_ERROR_NO_ERROR;
		}
	    else if (nread == 15)
	        {
	        data->time_d = d1;
		data->longitude = d2;
		data->latitude = d3;
		data->heading = d4;
		data->speed = d5;
		data->sonardepth = d6;
		data->roll = d7;
		data->pitch = d8;
		data->heave = d9;
	    	status = MB_SUCCESS;
	   	*error = MB_ERROR_NO_ERROR;
		}

	    if (status == MB_SUCCESS)
	        {
		/* print output debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  Data read in MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  Values,read:\n");
			fprintf(stderr,"dbg4       time_i[0]:    %f\n",data->time_i[0]);
			fprintf(stderr,"dbg4       time_i[1]:    %f\n",data->time_i[1]);
			fprintf(stderr,"dbg4       time_i[2]:    %f\n",data->time_i[2]);
			fprintf(stderr,"dbg4       time_i[3]:    %f\n",data->time_i[3]);
			fprintf(stderr,"dbg4       time_i[4]:    %f\n",data->time_i[4]);
			fprintf(stderr,"dbg4       time_i[5]:    %f\n",data->time_i[5]);
			fprintf(stderr,"dbg4       time_i[6]:    %f\n",data->time_i[6]);
			fprintf(stderr,"dbg4       time_d:       %f\n",data->time_d);
			fprintf(stderr,"dbg4       latitude:     %f\n",data->latitude);
			fprintf(stderr,"dbg4       longitude:    %f\n",data->longitude);
			fprintf(stderr,"dbg4       heading:      %f\n",data->heading);
			fprintf(stderr,"dbg4       speed:        %f\n",data->speed);
			fprintf(stderr,"dbg4       sonardepth:   %f\n",data->sonardepth);
			fprintf(stderr,"dbg4       roll:         %f\n",data->roll);
			fprintf(stderr,"dbg4       pitch:        %f\n",data->pitch);
			fprintf(stderr,"dbg4       heave:        %f\n",data->heave);
			fprintf(stderr,"dbg4       error:        %d\n",*error);
			fprintf(stderr,"dbg4       status:       %d\n",status);
			}
	    	}
	    	
	    else
	    	{	    	
	    	status = MB_FAILURE;
	   	*error = MB_ERROR_UNINTELLIGIBLE;
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
int mbr_mbpronav_wr_data(int verbose, char *mbio_ptr, char *data_ptr, int *error)
{
	char	*function_name = "mbr_mbpronav_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mbpronav_struct *data;
	char	line[MBF_MBPRONAV_MAXLINE+1];
	char	*line_ptr;
	int	len;
	int	i;

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

	/* get pointer to raw data structure */
	data = (struct mbf_mbpronav_struct *) data_ptr;

	/* handle the data */
	if (data->kind == MB_DATA_COMMENT)
	    {
	    line[0] = '#';
            strncpy(&line[1],data->comment,MBF_MBPRONAV_MAXLINE-2);
            len = strlen(line);
            line[len] = '\n';
            line[len+1] = '\0';
	    }
	else if (data->kind == MB_DATA_DATA)
	    {
	    /* print output debug statements */
	    if (verbose >= 4)
		    {
		    fprintf(stderr,"\ndbg4  Data to be written in MBIO function <%s>\n",
			    function_name);
		    fprintf(stderr,"dbg4  Values,read:\n");
		    fprintf(stderr,"dbg4       time_i[0]:    %f\n",data->time_i[0]);
		    fprintf(stderr,"dbg4       time_i[1]:    %f\n",data->time_i[1]);
		    fprintf(stderr,"dbg4       time_i[2]:    %f\n",data->time_i[2]);
		    fprintf(stderr,"dbg4       time_i[3]:    %f\n",data->time_i[3]);
		    fprintf(stderr,"dbg4       time_i[4]:    %f\n",data->time_i[4]);
		    fprintf(stderr,"dbg4       time_i[5]:    %f\n",data->time_i[5]);
		    fprintf(stderr,"dbg4       time_i[6]:    %f\n",data->time_i[6]);
		    fprintf(stderr,"dbg4       time_d:       %f\n",data->time_d);
		    fprintf(stderr,"dbg4       latitude:     %f\n",data->latitude);
		    fprintf(stderr,"dbg4       longitude:    %f\n",data->longitude);
		    fprintf(stderr,"dbg4       heading:      %f\n",data->heading);
		    fprintf(stderr,"dbg4       speed:        %f\n",data->speed);
		    fprintf(stderr,"dbg4       sonardepth:   %f\n",data->sonardepth);
		    fprintf(stderr,"dbg4       roll:         %f\n",data->roll);
		    fprintf(stderr,"dbg4       pitch:        %f\n",data->pitch);
		    fprintf(stderr,"dbg4       heave:        %f\n",data->heave);
		    fprintf(stderr,"dbg4       error:        %d\n",*error);
		    fprintf(stderr,"dbg4       status:       %d\n",status);
		    }

            sprintf(line,
			"%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.6f %.6f %.2f %.2f %.2f %.2f %.2f %.2f\r\n",
			data->time_i[0],
			data->time_i[1],
			data->time_i[2],
			data->time_i[3],
			data->time_i[4],
			data->time_i[5],
			data->time_i[6],
			data->time_d,
			data->longitude,
			data->latitude,
			data->heading,
			data->speed,
			data->sonardepth,
			data->roll,
			data->pitch,
			data->heave);
	    }

	if (fputs(line,mb_io_ptr->mbfp) == EOF)
		{
		*error = MB_ERROR_WRITE_FAIL;
		status = MB_FAILURE;
		}
	else
		{
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		}


	/* print output debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Data record kind in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       kind:       %d\n",data->kind);
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
