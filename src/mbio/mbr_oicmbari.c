/*--------------------------------------------------------------------
 *    The MB-system:	mbr_oicgeoda.c	2/16/99
 *	$Id: mbr_oicmbari.c,v 4.0 1999-03-31 18:29:20 caress Exp $
 *
 *    Copyright (c) 1999 by 
 *    D. W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbr_oicmbari.c contains the functions for reading and writing
 * multibeam data in the MBF_OICMBARI format.  
 * These functions include:
 *   mbr_alm_oicmbari	- allocate read/write memory
 *   mbr_dem_oicmbari	- deallocate read/write memory
 *   mbr_rt_oicmbari	- read and translate data
 *   mbr_wt_oicmbari	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	February 16, 1999
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1  1999/03/31  18:11:35  caress
 * Initial revision
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_io.h"
#include "../../include/mb_define.h"
#include "../../include/mbf_oicmbari.h"
#include "../../include/mbsys_oic.h"

/* include for byte swapping on little-endian machines */
#ifdef BYTESWAPPED
#include "../../include/mb_swap.h"
#endif

/*--------------------------------------------------------------------*/
int mbr_alm_oicmbari(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
 static char res_id[]="$Id: mbr_oicmbari.c,v 4.0 1999-03-31 18:29:20 caress Exp $";
	char	*function_name = "mbr_alm_oicmbari";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_oicmbari_struct *dataplus;
	struct mbf_oicmbari_header_struct *header;
	struct mbf_oicmbari_data_struct *data;
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

	/* set initial status */
	status = MB_SUCCESS;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = sizeof(struct mbf_oicmbari_struct);
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	dataplus = (struct mbf_oicmbari_struct *) mb_io_ptr->raw_data;
	header = &(dataplus->header);
	data = &(dataplus->data);
	header->num_chan = 0;
	header->beams_bath = 0;
	header->beams_amp = 0;
	header->bath_chan_port = -1;
	header->bath_chan_stbd = -1;
	header->pixels_ss = 0;
	header->ss_chan_port = -1;
	header->ss_chan_stbd = -1;
	for (i=0;i<MBF_OICMBARI_MAX_CHANNELS;i++)
	    {
	    header->channel[i].offset = 0;
	    header->channel[i].num_samples = 0;
	    data->rawsize[i] = 0;
	    data->raw[i] = NULL;
	    }
	data->beams_bath_alloc = 0;
	data->beams_amp_alloc = 0;
	data->pixels_ss_alloc = 0;
	data->beamflag = NULL;
	data->bath = NULL;
	data->amp = NULL;
	data->bathacrosstrack = NULL;
	data->bathalongtrack = NULL;
	data->tt = NULL;
	data->angle = NULL;
	data->ss = NULL;
	data->ssacrosstrack = NULL;
	data->ssalongtrack = NULL;
	status = mbsys_oic_alloc(verbose,mbio_ptr,
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
int mbr_dem_oicmbari(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	char	*function_name = "mbr_dem_oicmbari";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_oicmbari_struct *dataplus;
	struct mbf_oicmbari_header_struct *header;
	struct mbf_oicmbari_data_struct *data;
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
	dataplus = (struct mbf_oicmbari_struct *) mb_io_ptr->raw_data;
	header = &(dataplus->header);
	data = &(dataplus->data);

	/* deallocate memory for data descriptor */
	for (i=0;i<MBF_OICMBARI_MAX_CHANNELS;i++)
	    {
	    if (data->raw[i] != NULL)
		status = mb_free(verbose,&(data->raw[i]),error);
	    }
	if (data->beamflag != NULL)
	    status = mb_free(verbose,&(data->beamflag),error);
	if (data->bath != NULL)
	    status = mb_free(verbose,&(data->bath),error);
	if (data->amp != NULL)
	    status = mb_free(verbose,&(data->amp),error);
	if (data->bathacrosstrack != NULL)
	    status = mb_free(verbose,&(data->bathacrosstrack),error);
	if (data->bathalongtrack != NULL)
	    status = mb_free(verbose,&(data->bathalongtrack),error);
	if (data->tt != NULL)
	    status = mb_free(verbose,&(data->tt),error);
	if (data->angle != NULL)
	    status = mb_free(verbose,&(data->angle),error);
	if (data->ss != NULL)
	    status = mb_free(verbose,&(data->ss),error);
	if (data->ssacrosstrack != NULL)
	    status = mb_free(verbose,&(data->ssacrosstrack),error);
	if (data->ssalongtrack != NULL)
	    status = mb_free(verbose,&(data->ssalongtrack),error);
	status = mb_free(verbose,&mb_io_ptr->raw_data,error);
	status = mbsys_oic_deall(verbose,mbio_ptr,
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
int mbr_rt_oicmbari(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_rt_oicmbari";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_oicmbari_struct *dataplus;
	struct mbf_oicmbari_header_struct *header;
	struct mbf_oicmbari_data_struct *data;
	struct mbsys_oic_struct *store;
	char	buffer[MBF_OICMBARI_HEADER_SIZE];
	char	*comment;
	int	read_size;
	int	data_size;
	char	*char_ptr;
	short	*short_ptr;
	int	*int_ptr;
	float	*float_ptr;
	double	sample_interval;
	int	beams_bath_port;
	int	beams_bath_stbd;
	double	dx, rr, xx, zz;
	double	alpha, beta, theta, phi;
	int	index, ichan;
	int	i, j;

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

	/* get pointer to mbio descriptor and data structure */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	store = (struct mbsys_oic_struct *) store_ptr;

	/* get pointer to raw data structure */
	dataplus = (struct mbf_oicmbari_struct *) mb_io_ptr->raw_data;
	header = &(dataplus->header);
	data = &(dataplus->data);
	comment = dataplus->client;
	
	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* read next four bytes to look for start of header */
	if (read_size = fread(buffer,1,4,mb_io_ptr->mbfp)
	    != 4)
	    {
	    status = MB_FAILURE;
	    *error = MB_ERROR_EOF;		    
	    }
	    
	/* read another byted at a time until header found */
	while (status == MB_SUCCESS
	    && (buffer[0] != 'G'
		|| buffer[1] != 'E'
		|| buffer[2] != '2'))
	    {
	    for (i=0;i<3;i++)
		buffer[i] = buffer[i+1];
	    if (read_size = fread(&buffer[3],1,1,mb_io_ptr->mbfp)
		!= 1)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}
	    }
	    
	/* now read the rest of the header */
	if (status == MB_SUCCESS)
	    {
	    if (read_size = fread(&buffer[4],
				1,MBF_OICMBARI_HEADER_SIZE-4,
				mb_io_ptr->mbfp)
		!= MBF_OICMBARI_HEADER_SIZE-4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}
	    }
	    
	/* now parse the header */
	if (status == MB_SUCCESS)
	    {
#ifdef BYTESWAPPED
	    index = 3;
	    header->type = buffer[index]; index += 1;
	    int_ptr = (int *) &buffer[index]; index += 4;
	    header->proc_status = mb_swap_int(*int_ptr);
	    int_ptr = (int *) &buffer[index]; index += 4;
	    header->data_size = mb_swap_int(*int_ptr);
	    header->client_size = buffer[index]; index += 1;
	    header->fish_status = buffer[index]; index += 1;
	    header->nav_used = buffer[index]; index += 1;
	    header->nav_type = buffer[index]; index += 1;
	    int_ptr = (int *) &buffer[index]; index += 4;
	    header->utm_zone = mb_swap_int(*int_ptr);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->ship_x = mb_swap_float(*float_ptr);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->ship_y = mb_swap_float(*float_ptr);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->ship_course = mb_swap_float(*float_ptr);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->ship_speed = mb_swap_float(*float_ptr);
	    int_ptr = (int *) &buffer[index]; index += 4;
	    header->sec = mb_swap_int(*int_ptr);
	    int_ptr = (int *) &buffer[index]; index += 4;
	    header->usec = mb_swap_int(*int_ptr);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->spare_gain = mb_swap_float(*float_ptr);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->fish_heading = mb_swap_float(*float_ptr);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->fish_depth = mb_swap_float(*float_ptr);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->fish_range = mb_swap_float(*float_ptr);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->fish_pulse_width = mb_swap_float(*float_ptr);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->gain_c0 = mb_swap_float(*float_ptr);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->gain_c1 = mb_swap_float(*float_ptr);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->gain_c2 = mb_swap_float(*float_ptr);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->fish_pitch = mb_swap_float(*float_ptr);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->fish_roll = mb_swap_float(*float_ptr);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->fish_yaw = mb_swap_float(*float_ptr);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->fish_x = mb_swap_float(*float_ptr);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->fish_y = mb_swap_float(*float_ptr);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->fish_layback = mb_swap_float(*float_ptr);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->fish_altitude = mb_swap_float(*float_ptr);
	    int_ptr = (int *) &buffer[index]; index += 4;
	    header->fish_altitude_samples = mb_swap_int(*int_ptr);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->fish_ping_period = mb_swap_float(*float_ptr);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->sound_velocity = mb_swap_float(*float_ptr);
	    int_ptr = (int *) &buffer[index]; index += 4;
	    header->num_chan = mb_swap_int(*int_ptr);
	    for (i=0;i<MBF_OICMBARI_MAX_CHANNELS;i++)
		{
		int_ptr = (int *) &buffer[index]; index += 4;
		header->channel[i].offset = mb_swap_int(*int_ptr);
		}
	    for (i=0;i<MBF_OICMBARI_MAX_CHANNELS;i++)
		{
		header->channel[i].type = buffer[index]; index += 1;
		header->channel[i].side = buffer[index]; index += 1;
		header->channel[i].size = buffer[index]; index += 1;
		header->channel[i].empty = buffer[index]; index += 1;
		int_ptr = (int *) &buffer[index]; index += 4;
		header->channel[i].frequency = mb_swap_int(*int_ptr);
		int_ptr = (int *) &buffer[index]; index += 4;
		header->channel[i].num_samples = mb_swap_int(*int_ptr);
		}
	    int_ptr = (int *) &buffer[index]; index += 4;
	    header->beams_bath = mb_swap_int(*int_ptr);
	    int_ptr = (int *) &buffer[index]; index += 4;
	    header->beams_amp = mb_swap_int(*int_ptr);
	    int_ptr = (int *) &buffer[index]; index += 4;
	    header->bath_chan_port = mb_swap_int(*int_ptr);
	    int_ptr = (int *) &buffer[index]; index += 4;
	    header->bath_chan_stbd = mb_swap_int(*int_ptr);
	    int_ptr = (int *) &buffer[index]; index += 4;
	    header->pixels_ss = mb_swap_int(*int_ptr);
	    int_ptr = (int *) &buffer[index]; index += 4;
	    header->ss_chan_port = mb_swap_int(*int_ptr);
	    int_ptr = (int *) &buffer[index]; index += 4;
	    header->ss_chan_stbd = mb_swap_int(*int_ptr);
#else
	    index = 3;
	    header->type = buffer[index]; index += 1;
	    int_ptr = (int *) &buffer[index]; index += 4;
	    header->proc_status = *int_ptr;
	    int_ptr = (int *) &buffer[index]; index += 4;
	    header->data_size = *int_ptr;
	    header->client_size = buffer[index]; index += 1;
	    header->fish_status = buffer[index]; index += 1;
	    header->nav_used = buffer[index]; index += 1;
	    header->nav_type = buffer[index]; index += 1;
	    int_ptr = (int *) &buffer[index]; index += 4;
	    header->utm_zone = *int_ptr;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->ship_x = *float_ptr;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->ship_y = *float_ptr;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->ship_course = *float_ptr;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->ship_speed = *float_ptr;
	    int_ptr = (int *) &buffer[index]; index += 4;
	    header->sec = *int_ptr;
	    int_ptr = (int *) &buffer[index]; index += 4;
	    header->usec = *int_ptr;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->spare_gain = *float_ptr;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->fish_heading = *float_ptr;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->fish_depth = *float_ptr;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->fish_range = *float_ptr;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->fish_pulse_width = *float_ptr;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->gain_c0 = *float_ptr;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->gain_c1 = *float_ptr;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->gain_c2 = *float_ptr;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->fish_pitch = *float_ptr;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->fish_roll = *float_ptr;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->fish_yaw = *float_ptr;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->fish_x = *float_ptr;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->fish_y = *float_ptr;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->fish_layback = *float_ptr;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->fish_altitude = *float_ptr;
	    int_ptr = (int *) &buffer[index]; index += 4;
	    header->fish_altitude_samples = *int_ptr;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->fish_ping_period = *float_ptr;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    header->sound_velocity = *float_ptr;
	    int_ptr = (int *) &buffer[index]; index += 4;
	    header->num_chan = *int_ptr;
	    for (i=0;i<MBF_OICMBARI_MAX_CHANNELS;i++)
		{
		int_ptr = (int *) &buffer[index]; index += 4;
		header->channel[i].offset = *int_ptr;
		}
	    for (i=0;i<MBF_OICMBARI_MAX_CHANNELS;i++)
		{
		header->channel[i].type = buffer[index]; index += 1;
		header->channel[i].side = buffer[index]; index += 1;
		header->channel[i].size = buffer[index]; index += 1;
		header->channel[i].empty = buffer[index]; index += 1;
		int_ptr = (int *) &buffer[index]; index += 4;
		header->channel[i].frequency = *int_ptr;
		int_ptr = (int *) &buffer[index]; index += 4;
		header->channel[i].num_samples = *int_ptr;
		}
	    int_ptr = (int *) &buffer[index]; index += 4;
	    header->beams_bath = *int_ptr;
	    int_ptr = (int *) &buffer[index]; index += 4;
	    header->beams_amp = *int_ptr;
	    int_ptr = (int *) &buffer[index]; index += 4;
	    header->bath_chan_port = *int_ptr;
	    int_ptr = (int *) &buffer[index]; index += 4;
	    header->bath_chan_stbd = *int_ptr;
	    int_ptr = (int *) &buffer[index]; index += 4;
	    header->pixels_ss = *int_ptr;
	    int_ptr = (int *) &buffer[index]; index += 4;
	    header->ss_chan_port = *int_ptr;
	    int_ptr = (int *) &buffer[index]; index += 4;
	    header->ss_chan_stbd = *int_ptr;
#endif
	    }
	
	/* read client specific data */
	if (status == MB_SUCCESS && header->client_size > 0)
	    {
	    if (read_size = fread(dataplus->client,1,
				(int)(header->client_size),mb_io_ptr->mbfp)
		== (int)(header->client_size))
		{
		if (header->client_size < MBF_OICMBARI_MAX_CLIENT)
		    dataplus->client[header->client_size] = 0;
		}
	    else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}
	    }

	/* check for comment or unintelligible records */
	if (status == MB_SUCCESS)
	    {
	    if (header->type == OIC_ID_COMMENT)
		{
		dataplus->kind = MB_DATA_COMMENT;
		}
	    else if (header->num_chan > 0 && header->num_chan <= 8)
		{
		dataplus->kind = MB_DATA_DATA;
		}
	    else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
		dataplus->kind = MB_DATA_NONE;
		}
	    }

	/* set kind and error in mb_io_ptr */
	mb_io_ptr->new_kind = dataplus->kind;
	mb_io_ptr->new_error = *error;
	    
	/* loop over each data channel and read data */
	if (status == MB_SUCCESS && header->num_chan > 0)
	    {
	    for (i=0;i<header->num_chan;i++)
		{
		/* get size of data array */
		if (header->channel[i].size == OIC_SIZE_CHAR)
		    data_size = sizeof(char) * header->channel[i].num_samples;
		else if (header->channel[i].size == OIC_SIZE_SHORT)
		    data_size = sizeof(short) * header->channel[i].num_samples;
		else if (header->channel[i].size == OIC_SIZE_INT)
		    data_size = sizeof(int) * header->channel[i].num_samples;
		else if (header->channel[i].size == OIC_SIZE_FLOAT)
		    data_size = sizeof(float) * header->channel[i].num_samples;
		else if (header->channel[i].size == OIC_SIZE_3FLOAT)
		    data_size = 3 * sizeof(float) * header->channel[i].num_samples;
		
		/* allocate data if needed */
		if (data_size > data->rawsize[i] 
		    || data->raw[i] == NULL)
		    {
		    if (data->raw[i] != NULL)
			status = mb_free(verbose, &(data->raw[i]), error);
		    status = mb_malloc(verbose, data_size, &(data->raw[i]),error);
		    data->rawsize[i] = data_size;
		    }
		    
		/* read the data */
		if (status == MB_SUCCESS)
		    {
		    if (read_size = fread(data->raw[i],1,data_size,mb_io_ptr->mbfp)
			!= data_size)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;		    
			}
		    }

		/* byte swap the data if necessary */
#ifdef BYTESWAPPED
		if (status == MB_SUCCESS)
		    {
		    if (header->channel[i].size == OIC_SIZE_SHORT)
			{
			short_ptr = (short *) data->raw[i];
			for (j=0;j<header->channel[i].num_samples;j++)
			    {
			    short_ptr[j] = mb_swap_short(short_ptr[j]);
			    }
			}
		    else if (header->channel[i].size == OIC_SIZE_INT)
			{
			int_ptr = (int *) data->raw[i];
			for (j=0;j<header->channel[i].num_samples;j++)
			    {
			    int_ptr[j] = mb_swap_int(int_ptr[j]);
			    }
			}
		    else if (header->channel[i].size == OIC_SIZE_FLOAT)
			{
			float_ptr = (float *) data->raw[i];
			for (j=0;j<header->channel[i].num_samples;j++)
			    {
			    float_ptr[j] = mb_swap_float(float_ptr[j]);
			    }
			}
		    else if (header->channel[i].size == OIC_SIZE_3FLOAT)
			{
			float_ptr = (float *) data->raw[i];
			for (j=0;j<3*header->channel[i].num_samples;j++)
			    {
			    float_ptr[j] = mb_swap_float(float_ptr[j]);
			    }
			}
		    }
#endif
		}
	    }

	/* print debug statements */
	if (verbose >= 5)
	    {
	    fprintf(stderr,"\ndbg5  New header read in function <%s>\n",
		    function_name);
	    fprintf(stderr,"dbg5       type:             %d\n",header->type);
	    fprintf(stderr,"dbg5       proc_status:      %d\n",header->proc_status);
	    fprintf(stderr,"dbg5       data_size:        %d\n",header->data_size);
	    fprintf(stderr,"dbg5       client_size:      %d\n",header->client_size);
	    fprintf(stderr,"dbg5       fish_status:      %d\n",header->fish_status);
	    fprintf(stderr,"dbg5       nav_used:         %d\n",header->nav_used);
	    fprintf(stderr,"dbg5       nav_type:         %d\n",header->nav_type);
	    fprintf(stderr,"dbg5       utm_zone:         %d\n",header->utm_zone);
	    fprintf(stderr,"dbg5       ship_x:           %f\n",header->ship_x);
	    fprintf(stderr,"dbg5       ship_y:           %f\n",header->ship_y);
	    fprintf(stderr,"dbg5       ship_course:      %f\n",header->ship_course);
	    fprintf(stderr,"dbg5       ship_speed:       %f\n",header->ship_speed);
	    fprintf(stderr,"dbg5       sec:              %d\n",header->sec);
	    fprintf(stderr,"dbg5       usec:             %d\n",header->usec);
	    fprintf(stderr,"dbg5       spare_gain:       %f\n",header->spare_gain);
	    fprintf(stderr,"dbg5       fish_heading:     %f\n",header->fish_heading);
	    fprintf(stderr,"dbg5       fish_depth:       %f\n",header->fish_depth);
	    fprintf(stderr,"dbg5       fish_range:       %f\n",header->fish_range);
	    fprintf(stderr,"dbg5       fish_pulse_width: %f\n",header->fish_pulse_width);
	    fprintf(stderr,"dbg5       gain_c0:          %f\n",header->gain_c0);
	    fprintf(stderr,"dbg5       gain_c1:          %f\n",header->gain_c1);
	    fprintf(stderr,"dbg5       gain_c2:          %f\n",header->gain_c2);
	    fprintf(stderr,"dbg5       fish_pitch:       %f\n",header->fish_pitch);
	    fprintf(stderr,"dbg5       fish_roll:        %f\n",header->fish_roll);
	    fprintf(stderr,"dbg5       fish_yaw:         %f\n",header->fish_yaw);
	    fprintf(stderr,"dbg5       fish_x:           %f\n",header->fish_x);
	    fprintf(stderr,"dbg5       fish_y:           %f\n",header->fish_y);
	    fprintf(stderr,"dbg5       fish_layback:     %f\n",header->fish_layback);
	    fprintf(stderr,"dbg5       fish_altitude:    %f\n",header->fish_altitude);
	    fprintf(stderr,"dbg5       fish_altitude_samples: %d\n",header->fish_altitude_samples);
	    fprintf(stderr,"dbg5       fish_ping_period: %f\n",header->fish_ping_period);
	    fprintf(stderr,"dbg5       sound_velocity:   %f\n",header->sound_velocity);
	    fprintf(stderr,"dbg5       num_chan:         %d\n",header->num_chan);
	    fprintf(stderr,"dbg5       beams_bath:       %d\n",header->beams_bath);
	    fprintf(stderr,"dbg5       beams_amp:        %d\n",header->beams_amp);
	    fprintf(stderr,"dbg5       bath_chan_port:   %d\n",header->bath_chan_port);
	    fprintf(stderr,"dbg5       bath_chan_stbd:   %d\n",header->bath_chan_stbd);
	    fprintf(stderr,"dbg5       pixels_ss:        %d\n",header->pixels_ss);
	    fprintf(stderr,"dbg5       ss_chan_port:     %d\n",header->ss_chan_port);
	    fprintf(stderr,"dbg5       ss_chan_stbd:     %d\n",header->ss_chan_stbd);
	    for (i=0;i<header->num_chan;i++)
		{
		fprintf(stderr,"dbg5       offset[%1d]:      %d\n",
			     i, header->channel[i].offset);
		fprintf(stderr,"dbg5       type[%1d]:        %d\n", 
			     i, header->channel[i].type);
		fprintf(stderr,"dbg5       side[%1d]:        %d\n", 
			     i, header->channel[i].side);
		fprintf(stderr,"dbg5       size[%1d]:        %d\n", 
			     i, header->channel[i].size);
		fprintf(stderr,"dbg5       empty[%1d]:       %d\n", 
			     i, header->channel[i].empty);
		fprintf(stderr,"dbg5       frequency[%1d]:   %d\n", 
			     i, header->channel[i].frequency);
		fprintf(stderr,"dbg5       num_samples[%1d]: %d\n", 
			     i, header->channel[i].num_samples);
		}
	    for (i=0;i<header->num_chan;i++)
		{
		fprintf(stderr,"\ndbg5  New data read in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       channel:   %d\n", i);
		if (header->channel[i].type == OIC_TYPE_SIDESCAN)
		    fprintf(stderr, "dbg5       data type: sidescan\n");
		else if (header->channel[i].type == OIC_TYPE_ANGLE)
		    fprintf(stderr, "dbg5       data type: angle\n");
		else if (header->channel[i].type == OIC_TYPE_MULTIBEAM)
		    fprintf(stderr, "dbg5       data type: multibeam\n");
		else
		    fprintf(stderr, "dbg5       data type: unknown\n");
		if (header->channel[i].side == OIC_PORT)
		    fprintf(stderr, "dbg5       side:      port\n");
		else if (header->channel[i].side == OIC_STARBOARD)
		    fprintf(stderr, "dbg5       side:      starboard\n");
		else
		    fprintf(stderr, "dbg5       side:      unknown\n");
		fprintf(stderr,"dbg5       frequency:   %d\n", header->channel[i].frequency);
		fprintf(stderr,"dbg5       num samples: %d\n", header->channel[i].num_samples);
		if (header->channel[i].size == OIC_SIZE_CHAR)
		    {
		    fprintf(stderr, "dbg5       size:      char (1 byte)\n");
		    char_ptr = (char *) data->raw[i];
		    for (j=0;j<header->channel[i].num_samples;j++)
			{
			fprintf(stderr, "dbg5      %5d  %5d\n", j, char_ptr[j]);
			}
		    }
		else if (header->channel[i].size == OIC_SIZE_SHORT)
		    {
		    fprintf(stderr, "dbg5        size:      short (2 bytes)\n");
		    short_ptr = (short *) data->raw[i];
		    for (j=0;j<header->channel[i].num_samples;j++)
			{
			fprintf(stderr, "dbg5      %5d  %5d\n", j, short_ptr[j]);
			}
		    }
		else if (header->channel[i].size == OIC_SIZE_INT)
		    {
		    fprintf(stderr, "dbg5       size:       int (4 bytes)\n");
		    int_ptr = (int *) data->raw[i];
		    for (j=0;j<header->channel[i].num_samples;j++)
			{
			fprintf(stderr, "dbg5      %5d  %5d\n", j, int_ptr[j]);
			}
		    }
		else if (header->channel[i].size == OIC_SIZE_FLOAT)
		    {
		    fprintf(stderr, "dbg5       size:       float (4 bytes)\n");
		    float_ptr = (float *) data->raw[i];
		    for (j=0;j<header->channel[i].num_samples;j++)
			{
			fprintf(stderr, "dbg5      %5d  %10f\n", j, float_ptr[j]);
			}
		    }
		else if (header->channel[i].size == OIC_SIZE_3FLOAT)
		    {
		    fprintf(stderr, "dbg5       size:       3 floats (12 bytes)\n");
		    float_ptr = (float *) data->raw[i];
		    for (j=0;j<header->channel[i].num_samples;j++)
			{
			fprintf(stderr, "dbg5      %5d  %10f %10f %10f\n", 
				j, float_ptr[3*j], float_ptr[3*j+1], float_ptr[3*j+2]);
			}
		    }
		else
		    fprintf(stderr, "dbg5      size:       unknown\n");
		}
	    fprintf(stderr,"dbg5       status:     %d\n",status);
	    fprintf(stderr,"dbg5       error:      %d\n",*error);
	    }

	/* allocate memory for processed bathymetry and sidescan data */
	if (status == MB_SUCCESS 
	    && dataplus->kind == MB_DATA_DATA)
	    {
	    /* allocate arrays if needed */
	    if (header->beams_bath > data->beams_bath_alloc
		|| data->bath == NULL)
		{
		data->beams_bath_alloc = header->beams_bath;
		if (data->bath != NULL)
		    status = mb_free(verbose, &(data->beamflag), error);
		if (data->bath != NULL)
		    status = mb_free(verbose, &(data->bath), error);
		if (data->bathacrosstrack != NULL)
		    status = mb_free(verbose, &(data->bathacrosstrack), error);
		if (data->bathalongtrack != NULL)
		    status = mb_free(verbose, &(data->bathalongtrack), error);
		if (data->tt != NULL)
		    status = mb_free(verbose, &(data->tt), error);
		if (data->angle != NULL)
		    status = mb_free(verbose, &(data->angle), error);
		status = mb_malloc(verbose, header->beams_bath * sizeof(char), 
				    &(data->beamflag), error);
		status = mb_malloc(verbose, header->beams_bath * sizeof(float), 
				    &(data->bath), error);
		status = mb_malloc(verbose, header->beams_bath * sizeof(float), 
				    &(data->bathacrosstrack), error);
		status = mb_malloc(verbose, header->beams_bath * sizeof(float), 
				    &(data->bathalongtrack), error);
		status = mb_malloc(verbose, header->beams_bath * sizeof(float), 
				    &(data->tt), error);
		status = mb_malloc(verbose, header->beams_bath * sizeof(float), 
				    &(data->angle), error);
		}
	    if (header->beams_amp > data->beams_amp_alloc
		|| data->amp == NULL)
		{
		data->beams_amp_alloc = header->beams_amp;
		if (data->amp != NULL)
		    status = mb_free(verbose, &(data->amp), error);
		status = mb_malloc(verbose, header->beams_amp * sizeof(float), 
				    &(data->amp), error);
		}
	    if (header->pixels_ss > data->pixels_ss_alloc
		|| data->ss == NULL)
		{
		data->pixels_ss_alloc = header->pixels_ss;
		if (data->ss != NULL)
		    status = mb_free(verbose, &(data->ss), error);
		if (data->ssacrosstrack != NULL)
		    status = mb_free(verbose, &(data->ssacrosstrack), error);
		if (data->ssalongtrack != NULL)
		    status = mb_free(verbose, &(data->ssalongtrack), error);
		status = mb_malloc(verbose, header->pixels_ss * sizeof(float), 
				    &(data->ss), error);
		status = mb_malloc(verbose, header->pixels_ss * sizeof(float), 
				    &(data->ssacrosstrack), error);
		status = mb_malloc(verbose, header->pixels_ss * sizeof(float), 
				    &(data->ssalongtrack), error);
		}
	    }

	/* read processed bathymetry */
	if (status == MB_SUCCESS 
	    && dataplus->kind == MB_DATA_DATA
	    && header->beams_bath > 0)
	    {
	    /* get beamflag array */
	    data_size = header->beams_bath * sizeof(char);
	    if (read_size = fread(data->beamflag,1,data_size,mb_io_ptr->mbfp)
		!= data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}

	    /* get bath array */
	    data_size = header->beams_bath * sizeof(float);
	    if (read_size = fread(data->bath,1,data_size,mb_io_ptr->mbfp)
		!= data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}

	    /* get bathacrosstrack array */
	    data_size = header->beams_bath * sizeof(float);
	    if (read_size = fread(data->bathacrosstrack,1,data_size,mb_io_ptr->mbfp)
		!= data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}

	    /* get bathalongtrack array */
	    data_size = header->beams_bath * sizeof(float);
	    if (read_size = fread(data->bathalongtrack,1,data_size,mb_io_ptr->mbfp)
		!= data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}

	    /* get tt array */
	    data_size = header->beams_bath * sizeof(float);
	    if (read_size = fread(data->tt,1,data_size,mb_io_ptr->mbfp)
		!= data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}

	    /* get angle array */
	    data_size = header->beams_bath * sizeof(float);
	    if (read_size = fread(data->angle,1,data_size,mb_io_ptr->mbfp)
		!= data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}
	    }

	/* read processed amplitude */
	if (status == MB_SUCCESS 
	    && dataplus->kind == MB_DATA_DATA
	    && header->beams_amp > 0)
	    {
	    /* get amplitude array */
	    data_size = header->beams_amp * sizeof(char);
	    if (read_size = fread(data->amp,1,data_size,mb_io_ptr->mbfp)
		!= data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}
	    }

	/* read processed sidescan */
	if (status == MB_SUCCESS 
	    && dataplus->kind == MB_DATA_DATA
	    && header->pixels_ss > 0)
	    {
	    /* get ss array */
	    data_size = header->pixels_ss * sizeof(float);
	    if (read_size = fread(data->ss,1,data_size,mb_io_ptr->mbfp)
		!= data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}

	    /* get ssacrosstrack array */
	    data_size = header->pixels_ss * sizeof(float);
	    if (read_size = fread(data->ssacrosstrack,1,data_size,mb_io_ptr->mbfp)
		!= data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}

	    /* get ssalongtrack array */
	    data_size = header->pixels_ss * sizeof(float);
	    if (read_size = fread(data->ssalongtrack,1,data_size,mb_io_ptr->mbfp)
		!= data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}
	    }

	/* byte swap the processed data if necessary */
#ifdef BYTESWAPPED
	if (status == MB_SUCCESS)
	    {
	    for (j=0;j<header->beams_bath;j++)
		{
		data->bath[j] = mb_swap_float(bath[j]);
		data->bathacrosstrack[j] = mb_swap_float(bathacrosstrack[j]);
		data->bathalongtrack[j] = mb_swap_float(bathalongtrack[j]);
		data->tt[j] = mb_swap_float(tt[j]);
		data->angle[j] = mb_swap_float(angle[j]);
		}
	    for (j=0;j<header->beams_amp;j++)
		{
		data->amp[j] = mb_swap_float(amp[j]);
		}
	    for (j=0;j<header->pixels_ss;j++)
		{
		data->ssacrosstrack[j] = mb_swap_float(ssacrosstrack[j]);
		data->ssalongtrack[j] = mb_swap_float(ssalongtrack[j]);
		data->ss[j] = mb_swap_float(ss[j]);
		}
	    }
#endif

	/* print debug statements */
	if (verbose >= 5 && status == MB_SUCCESS)
	    {
	    fprintf(stderr,"\ndbg5  New processed data read in function <%s>\n",
		    function_name);
	    fprintf(stderr,"dbg5       beams_bath:       %d\n",header->beams_bath);
	    fprintf(stderr,"dbg5       beam   bath  xtrack ltrack   tt   angle\n");
	    for (i=0;i<header->beams_bath;i++)
		{
		fprintf(stderr,"dbg5       %4d %10f %10f %10f %10f %10f\n", 
			i, data->bath[i], data->bathacrosstrack[i], 
			data->bathalongtrack[i], 
			data->tt[i], data->angle[i]);
		}
	    fprintf(stderr,"dbg5       beams_amp:       %d\n",header->beams_amp);
	    fprintf(stderr,"dbg5       beam   amp  xtrack ltrack\n");
	    for (i=0;i<header->beams_amp;i++)
		{
		fprintf(stderr,"dbg5       %4d %10f %10f %10f\n", 
			i, data->amp[i], data->bathacrosstrack[i], 
			data->bathalongtrack[i]);
		}
	    fprintf(stderr,"dbg5       pixels_ss:       %d\n",header->pixels_ss);
	    fprintf(stderr,"dbg5       beam   ss  xtrack ltrack\n");
	    for (i=0;i<header->beams_amp;i++)
		{
		fprintf(stderr,"dbg5       %4d %10f %10f %10f\n", 
			i, data->ss[i], data->ssacrosstrack[i], 
			data->ssalongtrack[i]);
		}
	    }
	
	/* translate values to current ping variables 
		in mbio descriptor structure */
	if (status == MB_SUCCESS 
		&& dataplus->kind == MB_DATA_DATA)
	    {
	    /* get time */
	    mb_io_ptr->new_time_d = header->sec + 0.000001 * header->usec;
	    mb_get_date(verbose,mb_io_ptr->new_time_d,mb_io_ptr->new_time_i);

	    /* get navigation */
	    if (header->nav_type == OIC_NAV_LONLAT)
		{
		mb_io_ptr->new_lon = header->fish_x;
		mb_io_ptr->new_lat = header->fish_y;
		}
	    else
		{
		mb_io_ptr->new_lon = 0.0;
		mb_io_ptr->new_lat = 0.0;
		}
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
	    mb_io_ptr->new_heading = header->fish_heading;

	    /* get speed */
	    mb_io_ptr->new_speed = 3.6 * header->ship_speed;

	    /* read depth and beam location values into storage arrays */
	    mb_io_ptr->beams_bath = header->beams_bath;
	    for (i=0;i<mb_io_ptr->beams_bath;i++)
		{
		mb_io_ptr->new_beamflag[i] = data->beamflag[i];
		mb_io_ptr->new_bath[i] = data->bath[i];
		mb_io_ptr->new_bath_acrosstrack[i] = 
			data->bathacrosstrack[i];
		mb_io_ptr->new_bath_alongtrack[i] = 
			data->bathalongtrack[i];
		}

	    /* read amplitude values into storage arrays */
	    mb_io_ptr->beams_amp = header->beams_amp;
	    for (i=0;i<mb_io_ptr->beams_amp;i++)
		{
		mb_io_ptr->new_amp[i] = data->amp[i];
		}

	    /* read sidescan and pixel location values into storage arrays */
	    mb_io_ptr->pixels_ss = header->pixels_ss;
	    for (i=0;i<mb_io_ptr->pixels_ss;i++)
		{
		mb_io_ptr->new_ss[i] = data->ss[i];
		mb_io_ptr->new_ss_acrosstrack[i] =  data->ssacrosstrack[i];
		mb_io_ptr->new_ss_alongtrack[i] = data->ssalongtrack[i];
		}

	    /* print debug statements */
	    if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg4  New ping read by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg4  New ping values:\n");
		fprintf(stderr,"dbg4       error:      %d\n",
			mb_io_ptr->new_error);
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
		fprintf(stderr,"dbg4       beams_amp:  %d\n",
			mb_io_ptr->beams_amp);
		for (i=0;i<mb_io_ptr->beams_bath;i++)
		  fprintf(stderr,"dbg4       beam:%d  flag:%d  bath:%f  acrosstrack:%f  alongtrack:%f\n",
			i,mb_io_ptr->new_beamflag[i],
			mb_io_ptr->new_bath[i],
			mb_io_ptr->new_bath_acrosstrack[i],
			mb_io_ptr->new_bath_alongtrack[i]);
		for (i=0;i<mb_io_ptr->beams_amp;i++)
		  fprintf(stderr,"dbg4       beam:%d  amp:%f  acrosstrack:%f  alongtrack:%f\n",
			i,mb_io_ptr->new_amp[i],
			mb_io_ptr->new_bath_acrosstrack[i],
			mb_io_ptr->new_bath_alongtrack[i]);
		fprintf(stderr,"dbg4       pixels_ss:  %d\n",
			mb_io_ptr->pixels_ss);
		  fprintf(stderr,"dbg4       pixel:%d  ss:%f acrosstrack:%f  alongtrack:%f\n",
			i,mb_io_ptr->new_ss[i],
			mb_io_ptr->new_ss_acrosstrack[i],
			mb_io_ptr->new_ss_alongtrack[i]);
		}

	    /* done translating values */

	    }
	else if (status == MB_SUCCESS 
		&& dataplus->kind == MB_DATA_COMMENT)
	    {
	    /* copy comment */
	    strncpy(mb_io_ptr->new_comment,comment,MBF_OICMBARI_MAX_COMMENT);

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
	    /* type of data record */
	    store->kind = dataplus->kind;
	    store->type = header->type;

	    /* status and size */
	    store->proc_status = header->proc_status;
	    store->data_size = header->data_size;
	    store->client_size = header->client_size;
	    store->fish_status = header->fish_status;
	    store->type = header->type;
	    store->type = header->type;

	    /* nav */
	    store->nav_used = header->nav_used;
	    store->nav_type = header->nav_type;
	    store->utm_zone = header->utm_zone;
	    store->ship_x = header->ship_x;
	    store->ship_y = header->ship_y;
	    store->ship_course = header->ship_course;
	    store->ship_speed = header->ship_speed;
	    store->ship_x = header->ship_x;

	    /* time stamp */
	    store->sec = header->sec;
	    store->usec = header->usec;

	    /* more stuff */
	    store->spare_gain = header->spare_gain;
	    store->fish_heading = header->fish_heading;
	    store->fish_depth = header->fish_depth;
	    store->fish_range = header->fish_range;
	    store->fish_pulse_width = header->fish_pulse_width;
	    store->gain_c0 = header->gain_c0;
	    store->gain_c1 = header->gain_c1;
	    store->gain_c2 = header->gain_c2;
	    store->fish_pitch = header->fish_pitch;
	    store->fish_roll = header->fish_roll;
	    store->fish_yaw = header->fish_yaw;
	    store->fish_x = header->fish_x;
	    store->fish_y = header->fish_y;
	    store->fish_layback = header->fish_layback;
	    store->fish_altitude = header->fish_altitude;
	    store->fish_altitude_samples = header->fish_altitude_samples;
	    store->fish_ping_period = header->fish_ping_period;
	    store->sound_velocity = header->sound_velocity;

	    /* numbers of beams and scaling */
	    store->num_chan = header->num_chan;
	    store->beams_bath = header->beams_bath;
	    store->beams_amp = header->beams_amp;
	    store->bath_chan_port = header->bath_chan_port;
	    store->bath_chan_stbd = header->bath_chan_stbd;
	    store->pixels_ss = header->pixels_ss;
	    store->ss_chan_port = header->ss_chan_port;
	    store->ss_chan_stbd = header->ss_chan_stbd;
	    
	    /* raw data */
	    for (i=0;i<store->num_chan;i++)
		{
		/* copy channel info */
		store->channel[i].offset = header->channel[i].offset;
		store->channel[i].type = header->channel[i].type;
		store->channel[i].side = header->channel[i].side;
		store->channel[i].size = header->channel[i].size;
		store->channel[i].empty = header->channel[i].empty;
		store->channel[i].frequency = header->channel[i].frequency;
		store->channel[i].num_samples = header->channel[i].num_samples;
		
		/* allocate data if needed */
		if (data->rawsize[i] > store->rawsize[i] 
		    || store->raw[i] == NULL)
		    {
		    if (store->raw[i] != NULL)
			status = mb_free(verbose, &(store->raw[i]), error);
		    store->rawsize[i] = data->rawsize[i];
		    status = mb_malloc(verbose,store->rawsize[i], &(store->raw[i]),error);
		    }
		    
		/* copy the data */
		if (status == MB_SUCCESS)
		    {
		    for (j=0;j<store->rawsize[i];j++)
			{
			store->raw[i][j] = data->raw[i][j];	    
			}
		    }
		}

	    /* depths and sidescan */
	    if (header->beams_bath > store->beams_bath_alloc
		|| store->beamflag == NULL
		|| store->bath == NULL
		|| store->bathacrosstrack == NULL
		|| store->bathalongtrack == NULL
		|| store->tt == NULL
		|| store->angle == NULL)
		{
		store->beams_bath_alloc = header->beams_bath;
		if (store->beamflag != NULL)
		    status = mb_free(verbose, &(store->beamflag), error);
		if (store->bath != NULL)
		    status = mb_free(verbose, &(store->bath), error);
		if (store->bathacrosstrack != NULL)
		    status = mb_free(verbose, &(store->bathacrosstrack), error);
		if (store->bathalongtrack != NULL)
		    status = mb_free(verbose, &(store->bathalongtrack), error);
		if (store->tt != NULL)
		    status = mb_free(verbose, &(store->tt), error);
		if (store->angle != NULL)
		    status = mb_free(verbose, &(store->angle), error);
		status = mb_malloc(verbose,store->beams_bath_alloc * sizeof(char), 
				    &(store->beamflag),error);
		status = mb_malloc(verbose,store->beams_bath_alloc * sizeof(float), 
				    &(store->bath),error);
		status = mb_malloc(verbose,store->beams_bath_alloc * sizeof(float), 
				    &(store->bathacrosstrack),error);
		status = mb_malloc(verbose,store->beams_bath_alloc * sizeof(float), 
				    &(store->bathalongtrack),error);
		status = mb_malloc(verbose,store->beams_bath_alloc * sizeof(float), 
				    &(store->tt),error);
		status = mb_malloc(verbose,store->beams_bath_alloc * sizeof(float), 
				    &(store->angle),error);
		}
	    if (header->beams_amp > store->beams_amp_alloc
		|| store->amp == NULL)
		{
		store->beams_amp_alloc = header->beams_amp;
		if (store->amp != NULL)
		    status = mb_free(verbose, &(store->amp), error);
		status = mb_malloc(verbose,store->beams_amp_alloc * sizeof(float), 
				    &(store->amp),error);
		}
	    if (header->pixels_ss > store->pixels_ss_alloc
		|| store->ss == NULL
		|| store->ssacrosstrack == NULL
		|| store->ssalongtrack == NULL)
		{
		store->pixels_ss_alloc = header->pixels_ss;
		if (store->ss != NULL)
		    status = mb_free(verbose, &(store->ss), error);
		if (store->ssacrosstrack != NULL)
		    status = mb_free(verbose, &(store->ssacrosstrack), error);
		if (store->ssalongtrack != NULL)
		    status = mb_free(verbose, &(store->ssalongtrack), error);
		status = mb_malloc(verbose,store->pixels_ss_alloc * sizeof(float), 
				    &(store->ss),error);
		status = mb_malloc(verbose,store->pixels_ss_alloc * sizeof(float), 
				    &(store->ssacrosstrack),error);
		status = mb_malloc(verbose,store->pixels_ss_alloc * sizeof(float), 
				    &(store->ssalongtrack),error);
		}
	    for (i=0;i<store->beams_bath;i++)
		{
		store->beamflag[i] = data->beamflag[i];
		store->bath[i] = data->bath[i];
		store->bathacrosstrack[i] = data->bathacrosstrack[i];
		store->bathalongtrack[i] = data->bathalongtrack[i];
		store->tt[i] = data->tt[i];
		store->angle[i] = data->angle[i];
		}
	    for (i=0;i<store->beams_amp;i++)
		{
		store->amp[i] = data->amp[i];
		}
	    for (i=0;i<store->pixels_ss;i++)
		{
		store->ss[i] = data->ss[i];
		store->ssacrosstrack[i] = data->ssacrosstrack[i];
		store->ssalongtrack[i] = data->ssalongtrack[i];
		}
		    
	    /* client */
	    for (i=0;i<header->client_size;i++)
		store->client[i] = dataplus->client[i];
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
int mbr_wt_oicmbari(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_wt_oicmbari";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_oicmbari_struct *dataplus;
	struct mbf_oicmbari_header_struct *header;
	struct mbf_oicmbari_data_struct *data;
	struct mbsys_oic_struct *store;
	char	buffer[MBF_OICMBARI_HEADER_SIZE];
	char	*comment;
	int	write_size;
	int	data_size;
	char	*char_ptr;
	short	*short_ptr;
	int	*int_ptr;
	float	*float_ptr;
	int	index;
	int	i, j;

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

	/* get pointer to mbio descriptor and data storage */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	store = (struct mbsys_oic_struct *) store_ptr;

	/* get pointer to raw data structure */
	dataplus = (struct mbf_oicmbari_struct *) mb_io_ptr->raw_data;
	header = &(dataplus->header);
	data = &(dataplus->data);
	comment = dataplus->client;

	/* first translate values from data storage structure */
	if (store != NULL)
	    {
	    /* type of data record */
	    dataplus->kind = store->kind;
	    header->type = store->type;

	    /* status and size */
	    header->proc_status = store->proc_status;
	    header->data_size = store->data_size;
	    header->client_size = store->client_size;
	    header->fish_status = store->fish_status;
	    header->type = store->type;
	    header->type = store->type;

	    /* nav */
	    header->nav_used = store->nav_used;
	    header->nav_type = store->nav_type;
	    header->utm_zone = store->utm_zone;
	    header->ship_x = store->ship_x;
	    header->ship_y = store->ship_y;
	    header->ship_course = store->ship_course;
	    header->ship_speed = store->ship_speed;
	    header->ship_x = store->ship_x;

	    /* time stamp */
	    header->sec = store->sec;
	    header->usec = store->usec;

	    /* more stuff */
	    header->spare_gain = store->spare_gain;
	    header->fish_heading = store->fish_heading;
	    header->fish_depth = store->fish_depth;
	    header->fish_range = store->fish_range;
	    header->fish_pulse_width = store->fish_pulse_width;
	    header->gain_c0 = store->gain_c0;
	    header->gain_c1 = store->gain_c1;
	    header->gain_c2 = store->gain_c2;
	    header->fish_pitch = store->fish_pitch;
	    header->fish_roll = store->fish_roll;
	    header->fish_yaw = store->fish_yaw;
	    header->fish_x = store->fish_x;
	    header->fish_y = store->fish_y;
	    header->fish_layback = store->fish_layback;
	    header->fish_altitude = store->fish_altitude;
	    header->fish_altitude_samples = store->fish_altitude_samples;
	    header->fish_ping_period = store->fish_ping_period;
	    header->sound_velocity = store->sound_velocity;

	    /* numbers of beams and scaling */
	    header->num_chan = store->num_chan;
	    header->beams_bath = store->beams_bath;
	    header->beams_amp = store->beams_amp;
	    header->bath_chan_port = store->bath_chan_port;
	    header->bath_chan_stbd = store->bath_chan_stbd;
	    header->pixels_ss = store->pixels_ss;
	    header->ss_chan_port = store->ss_chan_port;
	    header->ss_chan_stbd = store->ss_chan_stbd;
	    
	    /* raw data */
	    for (i=0;i<header->num_chan;i++)
		{
		/* copy channel info */
		header->channel[i].offset = store->channel[i].offset;
		header->channel[i].type = store->channel[i].type;
		header->channel[i].side = store->channel[i].side;
		header->channel[i].size = store->channel[i].size;
		header->channel[i].empty = store->channel[i].empty;
		header->channel[i].frequency = store->channel[i].frequency;
		header->channel[i].num_samples = store->channel[i].num_samples;
		
		/* allocate data if needed */
		if (store->rawsize[i] > data->rawsize[i] 
		    || data->raw[i] == NULL)
		    {
		    if (data->raw[i] != NULL)
			status = mb_free(verbose, &(data->raw[i]), error);
		    data->rawsize[i] = store->rawsize[i];
		    status = mb_malloc(verbose, data->rawsize[i], &(data->raw[i]),error);
		    }
		    
		/* copy the data */
		if (status == MB_SUCCESS)
		    {
		    for (j=0;j<data->rawsize[i];j++)
			{
			data->raw[i][j] = store->raw[i][j];		    
			}
		    }
		}

	    /* depths and sidescan */
	    if (header->beams_bath > data->beams_bath_alloc
		|| data->beamflag == NULL
		|| data->bath == NULL
		|| data->bathacrosstrack == NULL
		|| data->bathalongtrack == NULL
		|| data->tt == NULL
		|| data->angle == NULL)
		{
		data->beams_bath_alloc = header->beams_bath;
		if (data->beamflag != NULL)
		    status = mb_free(verbose, &(data->beamflag), error);
		if (data->bath != NULL)
		    status = mb_free(verbose, &(data->bath), error);
		if (data->bathacrosstrack != NULL)
		    status = mb_free(verbose, &(data->bathacrosstrack), error);
		if (data->bathalongtrack != NULL)
		    status = mb_free(verbose, &(data->bathalongtrack), error);
		if (data->tt != NULL)
		    status = mb_free(verbose, &(data->tt), error);
		if (data->angle != NULL)
		    status = mb_free(verbose, &(data->angle), error);
		status = mb_malloc(verbose,data->beams_bath_alloc * sizeof(char), 
				    &(data->beamflag),error);
		status = mb_malloc(verbose,data->beams_bath_alloc * sizeof(float), 
				    &(data->bath),error);
		status = mb_malloc(verbose,data->beams_bath_alloc * sizeof(float), 
				    &(data->bathacrosstrack),error);
		status = mb_malloc(verbose,data->beams_bath_alloc * sizeof(float), 
				    &(data->bathalongtrack),error);
		status = mb_malloc(verbose,data->beams_bath_alloc * sizeof(float), 
				    &(data->tt),error);
		status = mb_malloc(verbose,data->beams_bath_alloc * sizeof(float), 
				    &(data->angle),error);
		}
	    if (header->beams_amp > data->beams_amp_alloc
		|| data->amp == NULL)
		{
		data->beams_amp_alloc = header->beams_amp;
		if (data->amp != NULL)
		    status = mb_free(verbose, &(data->amp), error);
		status = mb_malloc(verbose,data->beams_amp_alloc * sizeof(float), 
				    &(data->amp),error);
		}
	    if (header->pixels_ss > data->pixels_ss_alloc
		|| data->ss == NULL
		|| data->ssacrosstrack == NULL
		|| data->ssalongtrack == NULL)
		{
		data->pixels_ss_alloc = header->pixels_ss;
		if (data->ss != NULL)
		    status = mb_free(verbose, &(data->ss), error);
		if (data->ssacrosstrack != NULL)
		    status = mb_free(verbose, &(data->ssacrosstrack), error);
		if (data->ssalongtrack != NULL)
		    status = mb_free(verbose, &(data->ssalongtrack), error);
		status = mb_malloc(verbose,data->pixels_ss_alloc * sizeof(float), 
				    &(data->ss),error);
		status = mb_malloc(verbose,data->pixels_ss_alloc * sizeof(float), 
				    &(data->ssacrosstrack),error);
		status = mb_malloc(verbose,data->pixels_ss_alloc * sizeof(float), 
				    &(data->ssalongtrack),error);
		}
	    for (i=0;i<header->beams_bath;i++)
		{
		data->beamflag[i] = store->beamflag[i];
		data->bath[i] = store->bath[i];
		data->bathacrosstrack[i] = store->bathacrosstrack[i];
		data->bathalongtrack[i] = store->bathalongtrack[i];
		data->tt[i] = store->tt[i];
		data->angle[i] = store->angle[i];
		}
	    for (i=0;i<header->beams_amp;i++)
		{
		data->amp[i] = store->amp[i];
		}
	    for (i=0;i<header->pixels_ss;i++)
		{
		data->ss[i] = store->ss[i];
		data->ssacrosstrack[i] = store->ssacrosstrack[i];
		data->ssalongtrack[i] = store->ssalongtrack[i];
		}
		    
	    /* client */
	    for (i=0;i<header->client_size;i++)
		dataplus->client[i] = store->client[i];
	    }

	/* set kind from current ping */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR)
		dataplus->kind = mb_io_ptr->new_kind;

	/* check for comment */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_COMMENT)
	    {
	    dataplus->kind = MB_DATA_COMMENT;

	    /* copy comment */
	    strncpy(comment,mb_io_ptr->new_comment,MBF_OICMBARI_MAX_COMMENT);
	    header->client_size = strlen(comment) + strlen(comment) % 2;
	    header->type = OIC_ID_COMMENT;
	    }

	/* else translate current ping data to oicmbari data structure */
	else if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
	    && mb_io_ptr->new_kind == MB_DATA_DATA)
	    {
	    dataplus->kind = MB_DATA_DATA;

	    /* get time */
	    header->sec = (int) mb_io_ptr->new_time_d;
	    header->usec = (int) (1000000 * (mb_io_ptr->new_time_d - header->sec));

	    /* get navigation */
	    header->nav_type = OIC_NAV_LONLAT;
	    header->fish_x = mb_io_ptr->new_lon;
	    header->fish_y = mb_io_ptr->new_lat;

	    /* get heading */
	    header->fish_heading = mb_io_ptr->new_heading;

	    /* get speed */
	    header->ship_speed = mb_io_ptr->new_speed / 3.6;

	    /* set numbers of beams and sidescan */
	    header->beams_bath = mb_io_ptr->beams_bath;
	    header->beams_amp = mb_io_ptr->beams_amp;
	    header->pixels_ss = mb_io_ptr->pixels_ss;

	    /* depths and sidescan */
	    if (header->beams_bath > data->beams_bath_alloc
		|| data->beamflag == NULL
		|| data->bath == NULL
		|| data->bathacrosstrack == NULL
		|| data->bathalongtrack == NULL
		|| data->tt == NULL
		|| data->angle == NULL)
		{
		data->beams_bath_alloc = header->beams_bath;
		if (data->beamflag != NULL)
		    status = mb_free(verbose, &(data->beamflag), error);
		if (data->bath != NULL)
		    status = mb_free(verbose, &(data->bath), error);
		if (data->bathacrosstrack != NULL)
		    status = mb_free(verbose, &(data->bathacrosstrack), error);
		if (data->bathalongtrack != NULL)
		    status = mb_free(verbose, &(data->bathalongtrack), error);
		if (data->tt != NULL)
		    status = mb_free(verbose, &(data->tt), error);
		if (data->angle != NULL)
		    status = mb_free(verbose, &(data->angle), error);
		status = mb_malloc(verbose,data->beams_bath_alloc * sizeof(char), 
				    &(data->beamflag),error);
		status = mb_malloc(verbose,data->beams_bath_alloc * sizeof(float), 
				    &(data->bath),error);
		status = mb_malloc(verbose,data->beams_bath_alloc * sizeof(float), 
				    &(data->bathacrosstrack),error);
		status = mb_malloc(verbose,data->beams_bath_alloc * sizeof(float), 
				    &(data->bathalongtrack),error);
		status = mb_malloc(verbose,data->beams_bath_alloc * sizeof(float), 
				    &(data->tt),error);
		status = mb_malloc(verbose,data->beams_bath_alloc * sizeof(float), 
				    &(data->angle),error);
		}
	    if (header->beams_amp > data->beams_amp_alloc
		|| data->amp == NULL)
		{
		data->beams_amp_alloc = header->beams_amp;
		if (data->amp != NULL)
		    status = mb_free(verbose, &(data->amp), error);
		status = mb_malloc(verbose,data->beams_amp_alloc * sizeof(float), 
				    &(data->amp),error);
		}
	    if (header->pixels_ss > data->pixels_ss_alloc
		|| data->ss == NULL
		|| data->ssacrosstrack == NULL
		|| data->ssalongtrack == NULL)
		{
		data->pixels_ss_alloc = header->pixels_ss;
		if (data->ss != NULL)
		    status = mb_free(verbose, &(data->ss), error);
		if (data->ssacrosstrack != NULL)
		    status = mb_free(verbose, &(data->ssacrosstrack), error);
		if (data->ssalongtrack != NULL)
		    status = mb_free(verbose, &(data->ssalongtrack), error);
		status = mb_malloc(verbose,data->pixels_ss_alloc * sizeof(float), 
				    &(data->ss),error);
		status = mb_malloc(verbose,data->pixels_ss_alloc * sizeof(float), 
				    &(data->ssacrosstrack),error);
		status = mb_malloc(verbose,data->pixels_ss_alloc * sizeof(float), 
				    &(data->ssalongtrack),error);
		}
	    for (i=0;i<header->beams_bath;i++)
		{
		data->beamflag[i] = mb_io_ptr->new_beamflag[i];
		data->bath[i] = mb_io_ptr->new_bath[i];
		data->bathacrosstrack[i] = mb_io_ptr->new_bath_acrosstrack[i];
		data->bathalongtrack[i] = mb_io_ptr->new_bath_alongtrack[i];
		}
	    for (i=0;i<header->beams_amp;i++)
		{
		data->amp[i] = mb_io_ptr->new_amp[i];
		}
	    for (i=0;i<header->pixels_ss;i++)
		{
		data->ss[i] = mb_io_ptr->new_ss[i];
		data->ssacrosstrack[i] = mb_io_ptr->new_ss_acrosstrack[i];
		data->ssalongtrack[i] = mb_io_ptr->new_ss_alongtrack[i];
		}
	    }

	/* print debug statements */
	if (verbose >= 5)
	    {
	    fprintf(stderr,"\ndbg5  New header set in function <%s>\n",
		    function_name);
	    fprintf(stderr,"dbg5       type:             %d\n",header->type);
	    fprintf(stderr,"dbg5       proc_status:      %d\n",header->proc_status);
	    fprintf(stderr,"dbg5       data_size:        %d\n",header->data_size);
	    fprintf(stderr,"dbg5       client_size:      %d\n",header->client_size);
	    fprintf(stderr,"dbg5       fish_status:      %d\n",header->fish_status);
	    fprintf(stderr,"dbg5       nav_used:         %d\n",header->nav_used);
	    fprintf(stderr,"dbg5       nav_type:         %d\n",header->nav_type);
	    fprintf(stderr,"dbg5       utm_zone:         %d\n",header->utm_zone);
	    fprintf(stderr,"dbg5       ship_x:           %f\n",header->ship_x);
	    fprintf(stderr,"dbg5       ship_y:           %f\n",header->ship_y);
	    fprintf(stderr,"dbg5       ship_course:      %f\n",header->ship_course);
	    fprintf(stderr,"dbg5       ship_speed:       %f\n",header->ship_speed);
	    fprintf(stderr,"dbg5       sec:              %d\n",header->sec);
	    fprintf(stderr,"dbg5       usec:             %d\n",header->usec);
	    fprintf(stderr,"dbg5       spare_gain:       %f\n",header->spare_gain);
	    fprintf(stderr,"dbg5       fish_heading:     %f\n",header->fish_heading);
	    fprintf(stderr,"dbg5       fish_depth:       %f\n",header->fish_depth);
	    fprintf(stderr,"dbg5       fish_range:       %f\n",header->fish_range);
	    fprintf(stderr,"dbg5       fish_pulse_width: %f\n",header->fish_pulse_width);
	    fprintf(stderr,"dbg5       gain_c0:          %f\n",header->gain_c0);
	    fprintf(stderr,"dbg5       gain_c1:          %f\n",header->gain_c1);
	    fprintf(stderr,"dbg5       gain_c2:          %f\n",header->gain_c2);
	    fprintf(stderr,"dbg5       fish_pitch:       %f\n",header->fish_pitch);
	    fprintf(stderr,"dbg5       fish_roll:        %f\n",header->fish_roll);
	    fprintf(stderr,"dbg5       fish_yaw:         %f\n",header->fish_yaw);
	    fprintf(stderr,"dbg5       fish_x:           %f\n",header->fish_x);
	    fprintf(stderr,"dbg5       fish_y:           %f\n",header->fish_y);
	    fprintf(stderr,"dbg5       fish_layback:     %f\n",header->fish_layback);
	    fprintf(stderr,"dbg5       fish_altitude:    %f\n",header->fish_altitude);
	    fprintf(stderr,"dbg5       fish_altitude_samples: %d\n",header->fish_altitude_samples);
	    fprintf(stderr,"dbg5       fish_ping_period: %f\n",header->fish_ping_period);
	    fprintf(stderr,"dbg5       sound_velocity:   %f\n",header->sound_velocity);
	    fprintf(stderr,"dbg5       num_chan:         %d\n",header->num_chan);
	    fprintf(stderr,"dbg5       beams_bath:       %d\n",header->beams_bath);
	    fprintf(stderr,"dbg5       beams_amp:        %d\n",header->beams_amp);
	    fprintf(stderr,"dbg5       bath_chan_port:   %d\n",header->bath_chan_port);
	    fprintf(stderr,"dbg5       bath_chan_stbd:   %d\n",header->bath_chan_stbd);
	    fprintf(stderr,"dbg5       pixels_ss:        %d\n",header->pixels_ss);
	    fprintf(stderr,"dbg5       ss_chan_port:     %d\n",header->ss_chan_port);
	    fprintf(stderr,"dbg5       ss_chan_stbd:     %d\n",header->ss_chan_stbd);
	    for (i=0;i<header->num_chan;i++)
		{
		fprintf(stderr,"dbg5       offset[%1d]:      %d\n",
			     i, header->channel[i].offset);
		fprintf(stderr,"dbg5       type[%1d]:        %d\n", 
			     i, header->channel[i].type);
		fprintf(stderr,"dbg5       side[%1d]:        %d\n", 
			     i, header->channel[i].side);
		fprintf(stderr,"dbg5       size[%1d]:        %d\n", 
			     i, header->channel[i].size);
		fprintf(stderr,"dbg5       empty[%1d]:       %d\n", 
			     i, header->channel[i].empty);
		fprintf(stderr,"dbg5       frequency[%1d]:   %d\n", 
			     i, header->channel[i].frequency);
		fprintf(stderr,"dbg5       num_samples[%1d]: %d\n", 
			     i, header->channel[i].num_samples);
		}
	    fprintf(stderr,"dbg5       status:     %d\n",status);
	    fprintf(stderr,"dbg5       error:      %d\n",*error);
	    }

	/* print debug messages */
	if (verbose >= 5 && status == MB_SUCCESS)
	    {
	    for (i=0;i<header->num_chan;i++)
		{
		fprintf(stderr,"\ndbg5  New data set in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       channel:   %d\n", i);
		if (header->channel[i].type == OIC_TYPE_SIDESCAN)
		    fprintf(stderr, "dbg5       data type: sidescan\n");
		else if (header->channel[i].type == OIC_TYPE_ANGLE)
		    fprintf(stderr, "dbg5       data type: angle\n");
		else if (header->channel[i].type == OIC_TYPE_MULTIBEAM)
		    fprintf(stderr, "dbg5       data type: multibeam\n");
		else
		    fprintf(stderr, "dbg5       data type: unknown\n");
		if (header->channel[i].side == OIC_PORT)
		    fprintf(stderr, "dbg5       side:      port\n");
		else if (header->channel[i].side == OIC_STARBOARD)
		    fprintf(stderr, "dbg5       side:      starboard\n");
		else
		    fprintf(stderr, "dbg5       side:      unknown\n");
		fprintf(stderr,"dbg5       frequency:   %d\n", header->channel[i].frequency);
		fprintf(stderr,"dbg5       num samples: %d\n", header->channel[i].num_samples);
		if (header->channel[i].size == OIC_SIZE_CHAR)
		    {
		    fprintf(stderr, "dbg5       size:      char (1 byte)\n");
		    char_ptr = (char *) data->raw[i];
		    for (j=0;j<header->channel[i].num_samples;j++)
			{
			fprintf(stderr, "dbg5      %5d  %5d\n", j, char_ptr[j]);
			}
		    }
		else if (header->channel[i].size == OIC_SIZE_SHORT)
		    {
		    fprintf(stderr, "dbg5       size:      short (2 bytes)\n");
		    short_ptr = (short *) data->raw[i];
		    for (j=0;j<header->channel[i].num_samples;j++)
			{
			fprintf(stderr, "dbg5      %5d  %5d\n", j, short_ptr[j]);
			}
		    }
		else if (header->channel[i].size == OIC_SIZE_INT)
		    {
		    fprintf(stderr, "dbg5       size:      int (4 bytes)\n");
		    int_ptr = (int *) data->raw[i];
		    for (j=0;j<header->channel[i].num_samples;j++)
			{
			fprintf(stderr, "dbg5      %5d  %5d\n", j, int_ptr[j]);
			}
		    }
		else if (header->channel[i].size == OIC_SIZE_FLOAT)
		    {
		    fprintf(stderr, "dbg5       size:      float (4 bytes)\n");
		    float_ptr = (float *) data->raw[i];
		    for (j=0;j<header->channel[i].num_samples;j++)
			{
			fprintf(stderr, "dbg5      %5d  %10f\n", j, float_ptr[j]);
			}
		    }
		else if (header->channel[i].size == OIC_SIZE_3FLOAT)
		    {
		    fprintf(stderr, "dbg5       size:      3 floats (12 bytes)\n");
		    float_ptr = (float *) data->raw[i];
		    for (j=0;j<header->channel[i].num_samples;j++)
			{
			fprintf(stderr, "dbg5      %5d  %10f %10f %10f\n", 
				j, float_ptr[3*j], float_ptr[3*j+1], float_ptr[3*j+2]);
			}
		    }
		else
		    fprintf(stderr, "dbg5       size:      unknown\n");
		}
	    }

	/* print debug statements */
	if (verbose >= 5 && status == MB_SUCCESS)
	    {
	    fprintf(stderr,"\ndbg5  New processed data set in function <%s>\n",
		    function_name);
	    fprintf(stderr,"dbg5       beams_bath:       %d\n",header->beams_bath);
	    fprintf(stderr,"dbg5       beam   bath  xtrack ltrack   tt   angle\n");
	    for (i=0;i<header->beams_bath;i++)
		{
		fprintf(stderr,"dbg5       %4d %10f %10f %10f %10f %10f\n", 
			i, data->bath[i], data->bathacrosstrack[i], 
			data->bathalongtrack[i], 
			data->tt[i], data->angle[i]);
		}
	    fprintf(stderr,"dbg5       beams_amp:       %d\n",header->beams_amp);
	    fprintf(stderr,"dbg5       beam   amp  xtrack ltrack\n");
	    for (i=0;i<header->beams_amp;i++)
		{
		fprintf(stderr,"dbg5       %4d %10f %10f %10f\n", 
			i, data->amp[i], data->bathacrosstrack[i], 
			data->bathalongtrack[i]);
		}
	    fprintf(stderr,"dbg5       pixels_ss:       %d\n",header->pixels_ss);
	    fprintf(stderr,"dbg5       beam   ss  xtrack ltrack\n");
	    for (i=0;i<header->pixels_ss;i++)
		{
		fprintf(stderr,"dbg5       %4d %10f %10f %10f\n", 
			i, data->ss[i], data->ssacrosstrack[i], 
			data->ssalongtrack[i]);
		}
	    }
	    
	/* now reverse parse the header */
	if (status == MB_SUCCESS)
	    {
#ifdef BYTESWAPPED
	    index = 0;
	    buffer[index] = 'G'; index += 1;
	    buffer[index] = 'E'; index += 1;
	    buffer[index] = '2'; index += 1;
	    buffer[index] = header->type; index += 1;
	    int_ptr = (int *) &buffer[index]; index += 4;
	    *int_ptr = mb_swap_int(header->proc_status);
	    int_ptr = (int *) &buffer[index]; index += 4;
	    *int_ptr = mb_swap_int(header->data_size);
	    buffer[index] = header->client_size; index += 1;
	    buffer[index] = header->fish_status; index += 1;
	    buffer[index] = header->nav_used; index += 1;
	    buffer[index] = header->nav_type; index += 1;
	    int_ptr = (int *) &buffer[index]; index += 4;
	    *int_ptr = mb_swap_int(header->utm_zone);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = mb_swap_float(header->ship_x);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = mb_swap_float(header->ship_y);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = mb_swap_float(header->ship_course);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = mb_swap_float(header->ship_speed);
	    int_ptr = (int *) &buffer[index]; index += 4;
	    *int_ptr = mb_swap_int(header->sec);
	    int_ptr = (int *) &buffer[index]; index += 4;
	    *int_ptr = mb_swap_int(header->usec);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = mb_swap_float(header->spare_gain);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = mb_swap_float(header->fish_heading);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = mb_swap_float(header->fish_depth);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = mb_swap_float(header->fish_range);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = mb_swap_float(header->fish_pulse_width);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = mb_swap_float(header->gain_c0);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = mb_swap_float(header->gain_c1);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = mb_swap_float(header->gain_c2);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = mb_swap_float(header->fish_pitch);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = mb_swap_float(header->fish_roll);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = mb_swap_float(header->fish_yaw);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = mb_swap_float(header->fish_x);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = mb_swap_float(header->fish_y);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = mb_swap_float(header->fish_layback);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = mb_swap_float(header->fish_altitude);
	    int_ptr = (int *) &buffer[index]; index += 4;
	    *int_ptr = mb_swap_int(header->fish_altitude_samples);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = mb_swap_float(header->fish_ping_period);
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = mb_swap_float(header->sound_velocity);
	    int_ptr = (int *) &buffer[index]; index += 4;
	    *int_ptr = mb_swap_int(header->num_chan);
	    for (i=0;i<MBF_OICMBARI_MAX_CHANNELS;i++)
		{
		int_ptr = (int *) &buffer[index]; index += 4;
		*int_ptr = mb_swap_int(header->channel[i].offset);
		}
	    for (i=0;i<MBF_OICMBARI_MAX_CHANNELS;i++)
		{
		buffer[index] = header->channel[i].type; index += 1;
		buffer[index] = header->channel[i].side; index += 1;
		buffer[index] = header->channel[i].size; index += 1;
		buffer[index] = header->channel[i].empty; index += 1;
		int_ptr = (int *) &buffer[index]; index += 4;
		*int_ptr = mb_swap_int(header->channel[i].frequency);
		int_ptr = (int *) &buffer[index]; index += 4;
		*int_ptr = mb_swap_int(header->channel[i].num_samples);
		}
	    int_ptr = (int *) &buffer[index]; index += 4;
	    *int_ptr = mb_swap_int(header->beams_bath);
	    int_ptr = (int *) &buffer[index]; index += 4;
	    *int_ptr = mb_swap_int(header->beams_amp);
	    int_ptr = (int *) &buffer[index]; index += 4;
	    *int_ptr = mb_swap_int(header->bath_chan_port);
	    int_ptr = (int *) &buffer[index]; index += 4;
	    *int_ptr = mb_swap_int(header->bath_chan_stbd);
	    int_ptr = (int *) &buffer[index]; index += 4;
	    *int_ptr = mb_swap_int(header->pixels_ss);
	    int_ptr = (int *) &buffer[index]; index += 4;
	    *int_ptr = mb_swap_int(header->ss_chan_port);
	    int_ptr = (int *) &buffer[index]; index += 4;
	    *int_ptr = mb_swap_int(header->ss_chan_stbd);
#else
	    index = 0;
	    buffer[index] = 'G'; index += 1;
	    buffer[index] = 'E'; index += 1;
	    buffer[index] = '2'; index += 1;
	    buffer[index] = header->type; index += 1;
	    int_ptr = (int *) &buffer[index]; index += 4;
	    *int_ptr = header->proc_status;
	    int_ptr = (int *) &buffer[index]; index += 4;
	    *int_ptr = header->data_size;
	    buffer[index] = header->client_size; index += 1;
	    buffer[index] = header->fish_status; index += 1;
	    buffer[index] = header->nav_used; index += 1;
	    buffer[index] = header->nav_type; index += 1;
	    int_ptr = (int *) &buffer[index]; index += 4;
	    *int_ptr = header->utm_zone;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = header->ship_x;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = header->ship_y;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = header->ship_course;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = header->ship_speed;
	    int_ptr = (int *) &buffer[index]; index += 4;
	    *int_ptr = header->sec;
	    int_ptr = (int *) &buffer[index]; index += 4;
	    *int_ptr = header->usec;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = header->spare_gain;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = header->fish_heading;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = header->fish_depth;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = header->fish_range;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = header->fish_pulse_width;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = header->gain_c0;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = header->gain_c1;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = header->gain_c2;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = header->fish_pitch;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = header->fish_roll;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = header->fish_yaw;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = header->fish_x;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = header->fish_y;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = header->fish_layback;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = header->fish_altitude;
	    int_ptr = (int *) &buffer[index]; index += 4;
	    *int_ptr = header->fish_altitude_samples;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = header->fish_ping_period;
	    float_ptr = (float *) &buffer[index]; index += 4;
	    *float_ptr = header->sound_velocity;
	    int_ptr = (int *) &buffer[index]; index += 4;
	    *int_ptr = header->num_chan;
	    for (i=0;i<MBF_OICMBARI_MAX_CHANNELS;i++)
		{
		int_ptr = (int *) &buffer[index]; index += 4;
		*int_ptr = header->channel[i].offset;
		}
	    for (i=0;i<MBF_OICMBARI_MAX_CHANNELS;i++)
		{
		buffer[index] = header->channel[i].type; index += 1;
		buffer[index] = header->channel[i].side; index += 1;
		buffer[index] = header->channel[i].size; index += 1;
		buffer[index] = header->channel[i].empty; index += 1;
		int_ptr = (int *) &buffer[index]; index += 4;
		*int_ptr = header->channel[i].frequency;
		int_ptr = (int *) &buffer[index]; index += 4;
		*int_ptr = header->channel[i].num_samples;
		}
	    int_ptr = (int *) &buffer[index]; index += 4;
	    *int_ptr = header->beams_bath;
	    int_ptr = (int *) &buffer[index]; index += 4;
	    *int_ptr = header->beams_amp;
	    int_ptr = (int *) &buffer[index]; index += 4;
	    *int_ptr = header->bath_chan_port;
	    int_ptr = (int *) &buffer[index]; index += 4;
	    *int_ptr = header->bath_chan_stbd;
	    int_ptr = (int *) &buffer[index]; index += 4;
	    *int_ptr = header->pixels_ss;
	    int_ptr = (int *) &buffer[index]; index += 4;
	    *int_ptr = header->ss_chan_port;
	    int_ptr = (int *) &buffer[index]; index += 4;
	    *int_ptr = header->ss_chan_stbd;
#endif
	    }

	/* write next header to file */
	if ((write_size = fwrite(buffer,1,MBF_OICMBARI_HEADER_SIZE,
			mb_io_ptr->mbfp)) == MBF_OICMBARI_HEADER_SIZE) 
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	
	/* write client specific data */
	if (status == MB_SUCCESS && header->client_size > 0)
	    {
	    if (write_size = fwrite(dataplus->client,1,
				(int)(header->client_size),mb_io_ptr->mbfp)
		!= (int)(header->client_size))
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;		    
		}
	    }
	    
	/* loop over each data channel and write data */
	if (status == MB_SUCCESS && header->num_chan > 0)
	    {
	    for (i=0;i<header->num_chan;i++)
		{
		/* get size of data array */
		if (header->channel[i].size == OIC_SIZE_CHAR)
		    data_size = sizeof(char) * header->channel[i].num_samples;
		else if (header->channel[i].size == OIC_SIZE_SHORT)
		    data_size = sizeof(short) * header->channel[i].num_samples;
		else if (header->channel[i].size == OIC_SIZE_INT)
		    data_size = sizeof(int) * header->channel[i].num_samples;
		else if (header->channel[i].size == OIC_SIZE_FLOAT)
		    data_size = sizeof(float) * header->channel[i].num_samples;
		else if (header->channel[i].size == OIC_SIZE_3FLOAT)
		    data_size = 3 * sizeof(float) * header->channel[i].num_samples;

		/* byte swap the data if necessary */
#ifdef BYTESWAPPED
		if (status == MB_SUCCESS)
		    {
		    if (header->channel[i].size == OIC_SIZE_SHORT)
			{
			short_ptr = (short *) data->raw[i];
			for (j=0;j<header->channel[i].num_samples;j++)
			    {
			    short_ptr[j] = mb_swap_short(short_ptr[j]);
			    }
			}
		    else if (header->channel[i].size == OIC_SIZE_INT)
			{
			int_ptr = (int *) data->raw[i];
			for (j=0;j<header->channel[i].num_samples;j++)
			    {
			    int_ptr[j] = mb_swap_int(int_ptr[j]);
			    }
			}
		    else if (header->channel[i].size == OIC_SIZE_FLOAT)
			{
			float_ptr = (float *) data->raw[i];
			for (j=0;j<header->channel[i].num_samples;j++)
			    {
			    float_ptr[j] = mb_swap_float(float_ptr[j]);
			    }
			}
		    else if (header->channel[i].size == OIC_SIZE_3FLOAT)
			{
			float_ptr = (float *) data->raw[i];
			for (j=0;j<3*header->channel[i].num_samples;j++)
			    {
			    float_ptr[j] = mb_swap_float(float_ptr[j]);
			    }
			}
		    }
#endif
		    
		/* write the data */
		if (status == MB_SUCCESS)
		    {
		    if (write_size = fwrite(data->raw[i],1,data_size,mb_io_ptr->mbfp)
			!= data_size)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;		    
			}
		    }
		}
	    }

	/* byte swap the processed data if necessary */
#ifdef BYTESWAPPED
	if (status == MB_SUCCESS)
	    {
	    for (j=0;j<header->beams_bath;j++)
		{
		data->bath[j] = mb_swap_float(bath[j]);
		data->bathacrosstrack[j] = mb_swap_float(bathacrosstrack[j]);
		data->bathalongtrack[j] = mb_swap_float(bathalongtrack[j]);
		data->tt[j] = mb_swap_float(tt[j]);
		data->angle[j] = mb_swap_float(angle[j]);
		}
	    for (j=0;j<header->beams_amp;j++)
		{
		data->amp[j] = mb_swap_float(amp[j]);
		}
	    for (j=0;j<header->pixels_ss;j++)
		{
		data->ssacrosstrack[j] = mb_swap_float(ssacrosstrack[j]);
		data->ssalongtrack[j] = mb_swap_float(ssalongtrack[j]);
		data->ss[j] = mb_swap_float(ss[j]);
		}
	    }
#endif

	/* write processed bathymetry */
	if (status == MB_SUCCESS 
	    && dataplus->kind == MB_DATA_DATA
	    && header->beams_bath > 0)
	    {
	    /* get beamflag array */
	    data_size = header->beams_bath * sizeof(char);
	    if (write_size = fwrite(data->beamflag,1,data_size,mb_io_ptr->mbfp)
		!= data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}

	    /* get bath array */
	    data_size = header->beams_bath * sizeof(float);
	    if (write_size = fwrite(data->bath,1,data_size,mb_io_ptr->mbfp)
		!= data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}

	    /* get bathacrosstrack array */
	    data_size = header->beams_bath * sizeof(float);
	    if (write_size = fwrite(data->bathacrosstrack,1,data_size,mb_io_ptr->mbfp)
		!= data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}

	    /* get bathalongtrack array */
	    data_size = header->beams_bath * sizeof(float);
	    if (write_size = fwrite(data->bathalongtrack,1,data_size,mb_io_ptr->mbfp)
		!= data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}

	    /* get tt array */
	    data_size = header->beams_bath * sizeof(float);
	    if (write_size = fwrite(data->tt,1,data_size,mb_io_ptr->mbfp)
		!= data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}

	    /* get angle array */
	    data_size = header->beams_bath * sizeof(float);
	    if (write_size = fwrite(data->angle,1,data_size,mb_io_ptr->mbfp)
		!= data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}
	    }

	/* write processed amplitude */
	if (status == MB_SUCCESS 
	    && dataplus->kind == MB_DATA_DATA
	    && header->beams_amp > 0)
	    {
	    /* get amplitude array */
	    data_size = header->beams_amp * sizeof(char);
	    if (write_size = fwrite(data->amp,1,data_size,mb_io_ptr->mbfp)
		!= data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}
	    }

	/* write processed sidescan */
	if (status == MB_SUCCESS 
	    && dataplus->kind == MB_DATA_DATA
	    && header->pixels_ss > 0)
	    {
	    /* get ss array */
	    data_size = header->pixels_ss * sizeof(float);
	    if (write_size = fwrite(data->ss,1,data_size,mb_io_ptr->mbfp)
		!= data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}

	    /* get ssacrosstrack array */
	    data_size = header->pixels_ss * sizeof(float);
	    if (write_size = fwrite(data->ssacrosstrack,1,data_size,mb_io_ptr->mbfp)
		!= data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}

	    /* get ssalongtrack array */
	    data_size = header->pixels_ss * sizeof(float);
	    if (write_size = fwrite(data->ssalongtrack,1,data_size,mb_io_ptr->mbfp)
		!= data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
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
