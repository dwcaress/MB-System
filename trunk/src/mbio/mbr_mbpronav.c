/*--------------------------------------------------------------------
 *    The MB-system:	mbr_mbpronav.c	5/20/99
 *	$Id: mbr_mbpronav.c,v 4.2 2000-09-30 06:34:20 caress Exp $
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

/*--------------------------------------------------------------------*/
int mbr_alm_mbpronav(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
 static char res_id[]="$Id: mbr_mbpronav.c,v 4.2 2000-09-30 06:34:20 caress Exp $";
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
int mbr_dem_mbpronav(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
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
int mbr_zero_mbpronav(verbose,data_ptr,error)
int	verbose;
char	*data_ptr;
int	*error;
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
int mbr_rt_mbpronav(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
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

	/* reset values in mb_io_ptr */
	mb_io_ptr->new_kind = MB_DATA_NONE;
	mb_io_ptr->new_time_i[0] = 0;
	mb_io_ptr->new_time_i[1] = 0;
	mb_io_ptr->new_time_i[2] = 0;
	mb_io_ptr->new_time_i[3] = 0;
	mb_io_ptr->new_time_i[4] = 0;
	mb_io_ptr->new_time_i[5] = 0;
	mb_io_ptr->new_time_i[6] = 0;
	mb_io_ptr->new_time_d = 0.0;
	mb_io_ptr->new_lon = 0.0;
	mb_io_ptr->new_lat = 0.0;
	mb_io_ptr->new_heading = 0.0;
	mb_io_ptr->new_speed = 0.0;
	for (i=0;i<mb_io_ptr->beams_bath;i++)
		{
		mb_io_ptr->new_beamflag[i] = MB_FLAG_NULL;
		mb_io_ptr->new_bath[i] = 0.0;
		mb_io_ptr->new_bath_acrosstrack[i] = 0.0;
		mb_io_ptr->new_bath_alongtrack[i] = 0.0;
		}

	/* read next data from file */
	status = mbr_mbpronav_rd_data(verbose,mbio_ptr,error);

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = data->kind;

	/* translate time and navigation values to current
		ping variables in mbio descriptor structure */
	if (status == MB_SUCCESS
		&& data->kind == MB_DATA_DATA)
		{
		/* get time */
		mb_io_ptr->new_time_d = data->time_d;
		mb_get_date(verbose,mb_io_ptr->new_time_d, 
			mb_io_ptr->new_time_i);

		/* get navigation */
		mb_io_ptr->new_lon = data->longitude;
		mb_io_ptr->new_lat = data->latitude;
		if (mb_io_ptr->lonflip < 0)
			{
			if (mb_io_ptr->new_lon > 0.)
				mb_io_ptr->new_lon = mb_io_ptr->new_lon - 360.;
			else if (mb_io_ptr->new_lon < -360.)
				mb_io_ptr->new_lon = mb_io_ptr->new_lon + 360.;
			}
		else if (mb_io_ptr->lonflip == 0)
			{
			if (mb_io_ptr->new_lon > 180.)
				mb_io_ptr->new_lon = mb_io_ptr->new_lon - 360.;
			else if (mb_io_ptr->new_lon < -180.)
				mb_io_ptr->new_lon = mb_io_ptr->new_lon + 360.;
			}
		else
			{
			if (mb_io_ptr->new_lon > 360.)
				mb_io_ptr->new_lon = mb_io_ptr->new_lon - 360.;
			else if (mb_io_ptr->new_lon < 0.)
				mb_io_ptr->new_lon = mb_io_ptr->new_lon + 360.;
			}

		/* get heading */
		mb_io_ptr->new_heading = data->heading;

		/* get speed */
		mb_io_ptr->new_speed = data->speed;
		
		/* print debug statements */
		if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg4  New ping read by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  New ping values:\n");
			fprintf(stderr,"dbg4       error:      %d\n",
				mb_io_ptr->new_error);
			fprintf(stderr,"dbg4       kind:       %d\n",
				mb_io_ptr->new_kind);
			fprintf(stderr,"dbg4       time_i[0]:  %d\n",
				mb_io_ptr->new_time_i[0]);
			fprintf(stderr,"dbg4       time_i[1]:  %d\n",
				mb_io_ptr->new_time_i[1]);
			fprintf(stderr,"dbg4       time_i[2]:  %d\n",
				mb_io_ptr->new_time_i[2]);
			fprintf(stderr,"dbg4       time_i[3]:  %d\n",
				mb_io_ptr->new_time_i[3]);
			fprintf(stderr,"dbg4       time_i[4]:  %d\n",
				mb_io_ptr->new_time_i[4]);
			fprintf(stderr,"dbg4       time_i[5]:  %d\n",
				mb_io_ptr->new_time_i[5]);
			fprintf(stderr,"dbg4       time_i[6]:  %d\n",
				mb_io_ptr->new_time_i[6]);
			fprintf(stderr,"dbg4       time_d:     %f\n",
				mb_io_ptr->new_time_d);
			fprintf(stderr,"dbg4       longitude:  %f\n",
				mb_io_ptr->new_lon);
			fprintf(stderr,"dbg4       latitude:   %f\n",
				mb_io_ptr->new_lat);
			fprintf(stderr,"dbg4       speed:      %f\n",
				mb_io_ptr->new_speed);
			fprintf(stderr,"dbg4       heading:    %f\n",
				mb_io_ptr->new_heading);
			fprintf(stderr,"dbg4       beams_bath: %d\n",
				mb_io_ptr->beams_bath);
			for (i=0;i<mb_io_ptr->beams_bath;i++)
			  fprintf(stderr,"dbg4       bath[%d]: %f\n",
				i,mb_io_ptr->new_bath[i]);
			}
		}

	/* copy comment to mbio descriptor structure */
	if (status == MB_SUCCESS
		&& data->kind == MB_DATA_COMMENT)
		{
		/* copy comment */
		strncpy(mb_io_ptr->new_comment,data->comment,
			MBF_MBPRONAV_MAXLINE);

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  New ping read by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  New ping values:\n");
			fprintf(stderr,"dbg4       error:      %d\n",
				mb_io_ptr->new_error);
			fprintf(stderr,"dbg4       comment:    %s\n",
				mb_io_ptr->new_comment);
			}
		}

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
int mbr_wt_mbpronav(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
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

	/* set kind from current ping */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR)
		data->kind = mb_io_ptr->new_kind;

	/* check for comment to be copied from mb_io_ptr */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_COMMENT)
		{
		strncpy(data->comment,mb_io_ptr->new_comment,
			MBF_MBPRONAV_MAXLINE);
		}

	/* else check for ping data to be copied from mb_io_ptr */
	else if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_DATA)
		{
		/* get time */
		data->time_d = mb_io_ptr->new_time_d;
		data->time_i[0] = mb_io_ptr->new_time_i[0];
		data->time_i[1] = mb_io_ptr->new_time_i[1];
		data->time_i[2] = mb_io_ptr->new_time_i[2];
		data->time_i[3] = mb_io_ptr->new_time_i[3];
		data->time_i[4] = mb_io_ptr->new_time_i[4];
		data->time_i[5] = mb_io_ptr->new_time_i[5];
		data->time_i[6] = mb_io_ptr->new_time_i[6];

		/* get navigation */
		data->longitude = mb_io_ptr->new_lon;
		data->latitude = mb_io_ptr->new_lat;

		/* get heading */
		data->heading = mb_io_ptr->new_heading;

		/* get speed */
		data->speed = mb_io_ptr->new_speed;
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
int mbr_mbpronav_rd_data(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
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
int mbr_mbpronav_wr_data(verbose,mbio_ptr,data_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*data_ptr;
int	*error;
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
