/*--------------------------------------------------------------------
 *    The MB-system:	mbr_mgd77dat.c	5/18/99
 *	$Id: mbr_mgd77dat.c,v 4.3 2000-10-11 01:03:21 caress Exp $
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
 * mbr_mgd77dat.c contains the functions for reading and writing
 * multibeam data in the MGD77DAT format.
 * These functions include:
 *   mbr_alm_mgd77dat	- allocate read/write memory
 *   mbr_dem_mgd77dat	- deallocate read/write memory
 *   mbr_rt_mgd77dat	- read and translate data
 *   mbr_wt_mgd77dat	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	May 18, 1999
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.2  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.1  1999/07/16  19:29:09  caress
 * First revision.
 *
 * Revision 1.1  1999/07/16  19:24:15  caress
 * Initial revision
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
#include "../../include/mbf_mgd77dat.h"

/*--------------------------------------------------------------------*/
int mbr_alm_mgd77dat(int verbose, char *mbio_ptr, int *error)
{
 static char res_id[]="$Id: mbr_mgd77dat.c,v 4.3 2000-10-11 01:03:21 caress Exp $";
	char	*function_name = "mbr_alm_mgd77dat";
	int	status = MB_SUCCESS;
	int	i;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mgd77dat_struct *data;
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
	mb_io_ptr->structure_size = sizeof(struct mbf_mgd77dat_struct);
	mb_io_ptr->data_structure_size = 0;
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mb_malloc(verbose,sizeof(struct mbsys_singlebeam_struct),
				&mb_io_ptr->store_data,error);

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	data = (struct mbf_mgd77dat_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;
	
	/* set number of header records read to zero */
	mb_io_ptr->save1 = 0;

	/* initialize everything to zeros */
	mbr_zero_mgd77dat(verbose,data_ptr,error);

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
int mbr_dem_mgd77dat(int verbose, char *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_mgd77dat";
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
int mbr_zero_mgd77dat(int verbose, char *data_ptr, int *error)
{
	char	*function_name = "mbr_zero_mgd77dat";
	int	status = MB_SUCCESS;
	struct mbf_mgd77dat_struct *data;
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
	data = (struct mbf_mgd77dat_struct *) data_ptr;

	/* initialize everything to zeros */
	if (data != NULL)
		{
		data->kind = MB_DATA_NONE;
		for (i=0;i<8;i++)
		    data->survey_id[i] = 0;
		data->time_d = 0.0;
		for (i=0;i<7;i++)
		    data->time_i[i] = 0;
		data->timezone = 0;
		data->longitude = 0.0;
		data->latitude = 0.0;
		data->heading = 0.0;
		data->speed = 0.0;
		data->nav_type = 9;
		data->nav_quality = 9;
		data->roll = 0.0;
		data->pitch = 0.0;
		data->heave = 0.0;
		data->tt = 0.0;
		data->flag = MB_FLAG_NULL;
		data->bath = 0.0;
		data->bath_corr = 99;
		data->bath_type = 9;
		data->mag_tot_1 = 0.0;
		data->mag_tot_2 = 0.0;
		data->mag_res = 0.0;
		data->mag_res_sensor = 9;
		data->mag_diurnal = 0.0;
		data->mag_altitude = 0.0;
		data->gravity = 0.0;
		data->eotvos = 0.0;
		data->free_air = 0.0;
		data->seismic_line = 0;
		data->seismic_shot = 0;
		for (i=0;i<MBF_MGD77DAT_DATA_LEN;i++)
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
int mbr_rt_mgd77dat(int verbose, char *mbio_ptr, char *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_mgd77dat";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mgd77dat_struct *data;
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
	data = (struct mbf_mgd77dat_struct *) mb_io_ptr->raw_data;
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
	status = mbr_mgd77dat_rd_data(verbose,mbio_ptr,error);

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = data->kind;

	/* translate time and navigation values to current
		ping variables in mbio descriptor structure */
	if (status == MB_SUCCESS
		&& data->kind == MB_DATA_DATA)
		{
		/* get time */
		mb_io_ptr->new_time_i[0] = data->time_i[0];
		mb_io_ptr->new_time_i[1] = data->time_i[1];
		mb_io_ptr->new_time_i[2] = data->time_i[2];
		mb_io_ptr->new_time_i[3] = data->time_i[3];
		mb_io_ptr->new_time_i[4] = data->time_i[4];
		mb_io_ptr->new_time_i[5] = data->time_i[5];
		mb_io_ptr->new_time_i[6] = data->time_i[6];
		mb_get_time(verbose,mb_io_ptr->new_time_i,
			&mb_io_ptr->new_time_d);

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
		
		/* get bath */
		mb_io_ptr->new_bath[0] = data->bath;
		mb_io_ptr->new_bath_acrosstrack[0] = 0.0;
		mb_io_ptr->new_bath_alongtrack[0] = 0.0;
		mb_io_ptr->new_beamflag[0] = data->flag;

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
			MBF_MGD77DAT_DATA_LEN);

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
		for (i=0;i<8;i++)
		    store->survey_id[i] = data->survey_id[i];
		store->time_d = data->time_d;
		for (i=0;i<7;i++)
		    store->time_i[i] = data->time_i[i];
		store->timezone = data->timezone;
		store->longitude = data->longitude;
		store->latitude = data->latitude;
		store->heading = data->heading;
		store->speed = data->speed;
		store->nav_type = data->nav_type;
		store->nav_quality = data->nav_quality;
		store->roll = data->roll;
		store->pitch = data->pitch;
		store->heave = data->heave;
		store->flag = data->flag;
		store->tt = data->tt;
		store->bath = data->bath;
		store->bath_corr = data->bath_corr;
		store->bath_type = data->bath_type;
		store->mag_tot_1 = data->mag_tot_1;
		store->mag_tot_2 = data->mag_tot_2;
		store->mag_res = data->mag_res;
		store->mag_res_sensor = data->mag_res_sensor;
		store->mag_diurnal = data->mag_diurnal;
		store->mag_altitude = data->mag_altitude;
		store->gravity = data->gravity;
		store->eotvos = data->eotvos;
		store->free_air = data->free_air;
		store->seismic_line = data->seismic_line;
		store->seismic_shot = data->seismic_shot;
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
int mbr_wt_mgd77dat(int verbose, char *mbio_ptr, char *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_mgd77dat";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mgd77dat_struct *data;
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
	data = (struct mbf_mgd77dat_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_singlebeam_struct *) store_ptr;

	/* first translate values from data storage structure */
	if (store != NULL)
		{
		data->kind = store->kind;
		for (i=0;i<8;i++)
		    data->survey_id[i] = store->survey_id[i];
		data->time_d = store->time_d;
		for (i=0;i<7;i++)
		    data->time_i[i] = store->time_i[i];
		data->timezone = store->timezone;
		data->longitude = store->longitude;
		data->latitude = store->latitude;
		data->heading = store->heading;
		data->speed = store->speed;
		data->nav_type = store->nav_type;
		data->nav_quality = store->nav_quality;
		data->roll = store->roll;
		data->pitch = store->pitch;
		data->heave = store->heave;
		data->flag = store->flag;
		data->tt = store->tt;
		data->bath = store->bath;
		data->bath_corr = store->bath_corr;
		data->bath_type = store->bath_type;
		data->mag_tot_1 = store->mag_tot_1;
		data->mag_tot_2 = store->mag_tot_2;
		data->mag_res = store->mag_res;
		data->mag_res_sensor = store->mag_res_sensor;
		data->mag_diurnal = store->mag_diurnal;
		data->mag_altitude = store->mag_altitude;
		data->gravity = store->gravity;
		data->eotvos = store->eotvos;
		data->free_air = store->free_air;
		data->seismic_line = store->seismic_line;
		data->seismic_shot = store->seismic_shot;
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
			MBF_MGD77DAT_DATA_LEN);
		}

	/* else check for ping data to be copied from mb_io_ptr */
	else if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_DATA)
		{
		/* get time */
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
		
		/* get bath */
		data->bath = mb_io_ptr->new_bath[0];
		data->flag = mb_io_ptr->new_beamflag[0];
		}

	/* write next data to file */
	status = mbr_mgd77dat_wr_data(verbose,mbio_ptr,data,error);

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
int mbr_mgd77dat_rd_data(int verbose, char *mbio_ptr, int *error)
{
	char	*function_name = "mbr_mgd77dat_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mgd77dat_struct *data;
	int	*header_read;
	char	line[MBF_MGD77DAT_DATA_LEN];
	int	read_len;
	int	shift;
	int	itmp;
	double	dtmp;
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
	data = (struct mbf_mgd77dat_struct *) mb_io_ptr->raw_data;
	header_read = (int *) &mb_io_ptr->save1;

	/* initialize everything to zeros */
	mbr_zero_mgd77dat(verbose,data,error);

	/* set file position */
	mb_io_ptr->file_bytes = ftell(mb_io_ptr->mbfp);
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* read next record */
	if ((read_len = fread(line,1,MBF_MGD77DAT_DATA_LEN,
			mb_io_ptr->mbfp)) == MBF_MGD77DAT_DATA_LEN) 
		{
		mb_io_ptr->file_bytes += read_len;
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
	else
		{
		mb_io_ptr->file_bytes += read_len;
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
	mb_io_ptr->file_bytes = ftell(mb_io_ptr->mbfp);

	/* handle the data */
	if (status == MB_SUCCESS
	    && *header_read > 0
	    && *header_read < MBF_MGD77DAT_HEADER_NUM)
	    {
	    data->kind = MB_DATA_HEADER;
	    *header_read++;
	    for (i=0;i<MBF_MGD77DAT_DATA_LEN;i++)
		data->comment[i] = line[i];
	    }
	else if (status == MB_SUCCESS
	    && (line[0] == '1' || line[0] == '4'))
	    {
	    data->kind = MB_DATA_HEADER;
	    *header_read = 1;
	    for (i=0;i<MBF_MGD77DAT_DATA_LEN;i++)
		data->comment[i] = line[i];
	    }
	else if (status == MB_SUCCESS
	    && line[0] == '#')
	    {
	    data->kind = MB_DATA_COMMENT;
            strncpy(data->comment,&line[1],MBF_MGD77DAT_DATA_LEN-1);
	    }
	else if (status == MB_SUCCESS
	    && line[0] == '3')
	    {
	    data->kind = MB_DATA_DATA;
	    
	    /* get survey id */
	    shift = 1;
	    for (i=0;i<8;i++)
		data->survey_id[i] = line[i+shift];
		
	    /* get time */
	    shift += 8;
	    mb_get_int(&data->timezone, &line[shift], 5); shift += 5;
	    data->timezone = data->timezone / 100;
	    mb_get_int(&itmp, &line[shift], 2); shift += 2;
	    mb_fix_y2k(verbose, itmp, &data->time_i[0]);
	    mb_get_int(&data->time_i[1], &line[shift], 2); shift += 2;
	    mb_get_int(&data->time_i[2], &line[shift], 2); shift += 2;
	    mb_get_int(&data->time_i[3], &line[shift], 2); shift += 2;
	    mb_get_int(&itmp, &line[shift], 5); shift += 5;
	    data->time_i[4] = 0.001 * itmp;
	    dtmp = (itmp - 1000 * data->time_i[4]) / 60.0;
	    data->time_i[5] = (int) dtmp;
	    data->time_i[6] = 1000000 * (dtmp - data->time_i[5]);
	    mb_get_time(verbose,data->time_i,&data->time_d);
	    
	    /* get nav */
	    mb_get_int(&itmp, &line[shift], 8); shift += 8;
	    data->latitude = 0.00001 * itmp;
	    mb_get_int(&itmp, &line[shift], 9); shift += 9;
	    data->longitude = 0.00001 * itmp;
	    mb_get_int(&data->nav_type, &line[shift], 1); shift += 1;
	    
	    /* get bath */
	    mb_get_int(&itmp, &line[shift], 6); shift += 6;
	    data->tt = 0.0001 * itmp;
	    mb_get_int(&itmp, &line[shift], 6); shift += 6;
	    data->bath = 0.1 * itmp;
	    mb_get_int(&data->bath_corr, &line[shift], 2); shift += 2;
	    mb_get_int(&data->bath_type, &line[shift], 1); shift += 1;
	    if (data->bath > 0.0)
		data->flag = MB_FLAG_NONE;
	    else
		data->flag = MB_FLAG_NULL;
	    
	    /* get magnetics */
	    mb_get_int(&itmp, &line[shift], 6); shift += 6;
	    data->mag_tot_1 = 0.1 * itmp;
	    mb_get_int(&itmp, &line[shift], 6); shift += 6;
	    data->mag_tot_2 = 0.1 * itmp;
	    mb_get_int(&itmp, &line[shift], 6); shift += 6;
	    data->mag_res = 0.1 * itmp;
	    mb_get_int(&data->mag_res_sensor, &line[shift], 1); shift += 1;
	    mb_get_int(&itmp, &line[shift], 5); shift += 5;
	    data->mag_diurnal = 0.1 * itmp;
	    mb_get_int(&itmp, &line[shift], 6); shift += 6;
	    data->mag_altitude = itmp;
	    
	    /* get gravity */
	    mb_get_int(&itmp, &line[shift], 7); shift += 7;
	    data->gravity = 0.1 * itmp;
	    mb_get_int(&itmp, &line[shift], 6); shift += 6;
	    data->eotvos = 0.1 * itmp;
	    mb_get_int(&itmp, &line[shift], 5); shift += 5;
	    data->free_air = 0.1 * itmp;
	    mb_get_int(&data->seismic_line, &line[shift], 5); shift += 5;
	    mb_get_int(&data->seismic_shot, &line[shift], 6); shift += 6;
	    
	    /* get nav quality */
	    mb_get_int(&data->nav_quality, &line[shift], 1); shift += 1;
	    }
	else if (status == MB_SUCCESS
	    && line[0] == '5')
	    {
	    data->kind = MB_DATA_DATA;
	    
	    /* get survey id */
	    shift = 1;
	    for (i=0;i<8;i++)
		data->survey_id[i] = line[i+shift];
		
	    /* get time */
	    shift += 8;
	    mb_get_int(&data->timezone, &line[shift], 3); shift += 3;
	    mb_get_int(&data->time_i[0], &line[shift], 4); shift += 4;
	    mb_get_int(&data->time_i[1], &line[shift], 2); shift += 2;
	    mb_get_int(&data->time_i[2], &line[shift], 2); shift += 2;
	    mb_get_int(&data->time_i[3], &line[shift], 2); shift += 2;
	    mb_get_int(&itmp, &line[shift], 5); shift += 5;
	    data->time_i[4] = 0.001 * itmp;
	    dtmp = (itmp - 1000 * data->time_i[4]) / 60.0;
	    data->time_i[5] = (int) dtmp;
	    data->time_i[6] = 1000000 * (dtmp - data->time_i[5]);
	    mb_get_time(verbose,data->time_i,&data->time_d);
	    
	    /* get nav */
	    mb_get_int(&itmp, &line[shift], 8); shift += 8;
	    data->latitude = 0.00001 * itmp;
	    mb_get_int(&itmp, &line[shift], 9); shift += 9;
	    data->longitude = 0.00001 * itmp;
	    mb_get_int(&data->nav_type, &line[shift], 1); shift += 1;
	    
	    /* get bath */
	    mb_get_int(&itmp, &line[shift], 6); shift += 6;
	    data->tt = 0.0001 * itmp;
	    mb_get_int(&itmp, &line[shift], 6); shift += 6;
	    data->bath = 0.1 * itmp;
	    mb_get_int(&data->bath_corr, &line[shift], 2); shift += 2;
	    mb_get_int(&data->bath_type, &line[shift], 1); shift += 1;
	    if (data->bath > 0.0)
		data->flag = MB_FLAG_NONE;
	    else
		data->flag = MB_FLAG_NULL;
	    
	    /* get magnetics */
	    mb_get_int(&itmp, &line[shift], 6); shift += 6;
	    data->mag_tot_1 = 0.1 * itmp;
	    mb_get_int(&itmp, &line[shift], 6); shift += 6;
	    data->mag_tot_2 = 0.1 * itmp;
	    mb_get_int(&itmp, &line[shift], 6); shift += 6;
	    data->mag_res = 0.1 * itmp;
	    mb_get_int(&data->mag_res_sensor, &line[shift], 1); shift += 1;
	    mb_get_int(&itmp, &line[shift], 5); shift += 5;
	    data->mag_diurnal = 0.1 * itmp;
	    mb_get_int(&itmp, &line[shift], 6); shift += 6;
	    data->mag_altitude = itmp;
	    
	    /* get gravity */
	    mb_get_int(&itmp, &line[shift], 7); shift += 7;
	    data->gravity = 0.1 * itmp;
	    mb_get_int(&itmp, &line[shift], 6); shift += 6;
	    data->eotvos = 0.1 * itmp;
	    mb_get_int(&itmp, &line[shift], 5); shift += 5;
	    data->free_air = 0.1 * itmp;
	    mb_get_int(&data->seismic_line, &line[shift], 5); shift += 5;
	    mb_get_int(&data->seismic_shot, &line[shift], 6); shift += 6;
	    
	    /* get nav quality */
	    mb_get_int(&data->nav_quality, &line[shift], 1); shift += 1;
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
int mbr_mgd77dat_wr_data(int verbose, char *mbio_ptr, char *data_ptr, int *error)
{
	char	*function_name = "mbr_mgd77dat_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mgd77dat_struct *data;
	char	line[MBF_MGD77DAT_DATA_LEN+1];
	int	itmp;
	double	dtmp;
	int	write_len;
	int	shift;
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
	data = (struct mbf_mgd77dat_struct *) data_ptr;

	/* handle the data */
	if (data->kind == MB_DATA_HEADER)
	    {
	    for (i=0;i<MBF_MGD77DAT_DATA_LEN;i++)
		line[i] = data->comment[i];
	    }
	else if (data->kind == MB_DATA_COMMENT)
	    {
	    line[0] = '#';
            strncpy(&line[1],data->comment,MBF_MGD77DAT_DATA_LEN-1);
	    }
	else if (data->kind == MB_DATA_DATA)
	    {
	    /* set data record id */
	    shift = 0;
	    line[0] = '5'; shift += 1;
	    
	    /* get survey id */
	    for (i=0;i<8;i++)
		line[i+shift] = data->survey_id[i]; 
	    shift += 8;
		
	    /* get time */
	    sprintf(&line[shift], "%3.3d", data->timezone); shift += 3;
	    sprintf(&line[shift], "%4.4d", data->time_i[0]); shift += 4;
	    sprintf(&line[shift], "%2.2d", data->time_i[1]); shift += 2;
	    sprintf(&line[shift], "%2.2d", data->time_i[2]); shift += 2;
	    sprintf(&line[shift], "%2.2d", data->time_i[3]); shift += 2;
	    itmp = 1000.0 * data->time_i[4] 
		    + 1000.0 * (data->time_i[5] 
			+ 1000000 * data->time_i[6])/ 60.0;
	    sprintf(&line[shift], "%5.5d", itmp); shift += 5;
	
	    /* get nav */
	    itmp = 100000 * data->latitude;
	    sprintf(&line[shift], "%8.8d", itmp); shift += 8;
	    itmp = 100000 * data->longitude;
	    sprintf(&line[shift], "%9.9d", itmp); shift += 9;
	    sprintf(&line[shift], "%1.1d", data->nav_type); shift += 1;
	    
	    /* get bath */
	    itmp = 10000 * data->tt;
	    sprintf(&line[shift], "%6.6d", itmp); shift += 6;
	    itmp = 10 * data->bath;
	    sprintf(&line[shift], "%6.6d", itmp); shift += 6;
	    sprintf(&line[shift], "%2.2d", data->bath_corr); shift += 2;
	    sprintf(&line[shift], "%1.1d", data->bath_type); shift += 1;

	    /* get magnetics */
	    itmp = 10 * data->mag_tot_1;
	    sprintf(&line[shift], "%6.6d", itmp); shift += 6;
	    itmp = 10 * data->mag_tot_2;
	    sprintf(&line[shift], "%6.6d", itmp); shift += 6;
	    itmp = 10 * data->mag_res;
	    sprintf(&line[shift], "%6.6d", itmp); shift += 6;
	    sprintf(&line[shift], "%1.1d", data->mag_res_sensor); shift += 1;
	    itmp = 10 * data->mag_diurnal;
	    sprintf(&line[shift], "%5.5d", itmp); shift += 5;
	    itmp = data->mag_altitude;
	    sprintf(&line[shift], "%6.6d", itmp); shift += 6;
	
	    /* get gravity */
	    itmp = 10 * data->gravity;
	    sprintf(&line[shift], "%7.7d", itmp); shift += 7;
	    itmp = 10 * data->eotvos;
	    sprintf(&line[shift], "%6.6d", itmp); shift += 6;
	    itmp = 10 * data->free_air;
	    sprintf(&line[shift], "%5.5d", itmp); shift += 5;
	    sprintf(&line[shift], "%5.5d", data->seismic_line); shift += 5;
	    sprintf(&line[shift], "%6.6d", data->seismic_shot); shift += 6;
	
	    /* get nav quality */
	    sprintf(&line[shift], "%1.1d", data->nav_quality); shift += 1;
	    }

	write_len = fwrite(line,1,MBF_MGD77DAT_DATA_LEN,mb_io_ptr->mbfp);
	if (write_len != MBF_MGD77DAT_DATA_LEN)
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
