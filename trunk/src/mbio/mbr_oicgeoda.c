/*--------------------------------------------------------------------
 *    The MB-system:	mbr_oicgeoda.c	2/16/99
 *	$Id: mbr_oicgeoda.c,v 5.0 2000-12-01 22:48:41 caress Exp $
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
 * mbr_oicgeoda.c contains the functions for reading and writing
 * multibeam data in the MBF_OICGEODA format.  
 * These functions include:
 *   mbr_alm_oicgeoda	- allocate read/write memory
 *   mbr_dem_oicgeoda	- deallocate read/write memory
 *   mbr_rt_oicgeoda	- read and translate data
 *   mbr_wt_oicgeoda	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	February 16, 1999
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.4  2000/10/11  01:03:21  caress
 * Convert to ANSI C
 *
 * Revision 4.3  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.2  1999/12/29  00:34:06  caress
 * Release 4.6.8
 *
 * Revision 4.1  1999/10/21  22:40:10  caress
 * Let module pass easting northing nav.
 *
 * Revision 4.0  1999/03/31  18:29:20  caress
 * MB-System 4.6beta7
 *
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
#include "../../include/mbf_oicgeoda.h"
#include "../../include/mbsys_oic.h"

/* include for byte swapping on little-endian machines */
#ifdef BYTESWAPPED
#include "../../include/mb_swap.h"
#endif

/* essential function prototypes */
int mbr_info_oicgeoda(int verbose, 
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
int mbr_alm_oicgeoda(int verbose, char *mbio_ptr, int *error);
int mbr_dem_oicgeoda(int verbose, char *mbio_ptr, int *error);
int mbr_rt_oicgeoda(int verbose, char *mbio_ptr, char *store_ptr, int *error);
int mbr_wt_oicgeoda(int verbose, char *mbio_ptr, char *store_ptr, int *error);

/*--------------------------------------------------------------------*/
int mbr_info_oicgeoda(int verbose, 
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
	static char res_id[]="$Id: mbr_oicgeoda.c,v 5.0 2000-12-01 22:48:41 caress Exp $";
	char	*function_name = "mbr_info_oicgeoda";
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
	*system = MB_SYS_OIC;
	*beams_bath_max = 1024;
	*beams_amp_max = 256;
	*pixels_ss_max = 2048;
	strncpy(format_name, "OICGEODA", MB_NAME_LENGTH);
	strncpy(system_name, "OIC", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_OICGEODA\nInformal Description: OIC swath sonar format\nAttributes:           variable beam bathymetry and\n                      amplitude, variable pixel sidescan, binary,\n		      Oceanic Imaging Consultants\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_YES;
	*traveltime = MB_YES;
	*beam_flagging = MB_YES;
	*nav_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_DATA;
	*beamwidth_xtrack = 0.0;
	*beamwidth_ltrack = 0.0;

	/* set format and system specific function pointers */
	*format_alloc = &mbr_alm_oicgeoda;
	*format_free = &mbr_dem_oicgeoda; 
	*store_alloc = &mbsys_oic_alloc; 
	*store_free = &mbsys_oic_deall; 
	*read_ping = &mbr_rt_oicgeoda; 
	*write_ping = &mbr_wt_oicgeoda; 
	*extract = &mbsys_oic_extract; 
	*insert = &mbsys_oic_insert; 
	*extract_nav = &mbsys_oic_extract_nav; 
	*insert_nav = &mbsys_oic_insert_nav; 
	*altitude = &mbsys_oic_altitude; 
	*insert_altitude = &mbsys_oic_insert_altitude; 
	*ttimes = &mbsys_oic_ttimes; 
	*copyrecord = &mbsys_oic_copy; 

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
int mbr_alm_oicgeoda(int verbose, char *mbio_ptr, int *error)
{
 static char res_id[]="$Id: mbr_oicgeoda.c,v 5.0 2000-12-01 22:48:41 caress Exp $";
	char	*function_name = "mbr_alm_oicgeoda";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_oicgeoda_struct *dataplus;
	struct mbf_oicgeoda_header_struct *header;
	struct mbf_oicgeoda_data_struct *data;
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
	mb_io_ptr->structure_size = sizeof(struct mbf_oicgeoda_struct);
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	dataplus = (struct mbf_oicgeoda_struct *) mb_io_ptr->raw_data;
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
	for (i=0;i<MBF_OICGEODA_MAX_CHANNELS;i++)
	    {
	    header->channel[i].offset = 0;
	    header->channel[i].num_samples = 0;
	    data->rawsize[i] = 0;
	    data->raw[i] = NULL;
	    }
	data->beams_bath_alloc = 0;
	data->beams_amp_alloc = 0;
	data->pixels_ss_alloc = 0;
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
int mbr_dem_oicgeoda(int verbose, char *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_oicgeoda";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_oicgeoda_struct *dataplus;
	struct mbf_oicgeoda_header_struct *header;
	struct mbf_oicgeoda_data_struct *data;
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
	dataplus = (struct mbf_oicgeoda_struct *) mb_io_ptr->raw_data;
	header = &(dataplus->header);
	data = &(dataplus->data);

	/* deallocate memory for data descriptor */
	for (i=0;i<MBF_OICGEODA_MAX_CHANNELS;i++)
	    {
	    if (data->raw[i] != NULL)
		status = mb_free(verbose,&(data->raw[i]),error);
	    }
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
int mbr_rt_oicgeoda(int verbose, char *mbio_ptr, char *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_oicgeoda";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_oicgeoda_struct *dataplus;
	struct mbf_oicgeoda_header_struct *header;
	struct mbf_oicgeoda_data_struct *data;
	struct mbsys_oic_struct *store;
	char	buffer[MBF_OICGEODA_HEADER_SIZE];
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
	dataplus = (struct mbf_oicgeoda_struct *) mb_io_ptr->raw_data;
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
		|| buffer[2] != 'O'))
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
				1,MBF_OICGEODA_HEADER_SIZE-4,
				mb_io_ptr->mbfp)
		!= MBF_OICGEODA_HEADER_SIZE-4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}
	    }
	    
	/* now parse the header */
	if (status == MB_SUCCESS)
	    {
	    index = 3;
	    header->type = buffer[index]; index += 1;
 	    mb_get_binary_int(MB_NO,&buffer[index],&header->proc_status);
	    index += 4;
 	    mb_get_binary_int(MB_NO,&buffer[index],&header->data_size);
	    index += 4;
	    header->client_size = buffer[index]; 
	    index += 1;
	    header->fish_status = buffer[index]; 
	    index += 1;
	    header->nav_used = buffer[index]; 
	    index += 1;
	    header->nav_type = buffer[index]; 
	    index += 1;
 	    mb_get_binary_int(MB_NO,&buffer[index],&header->utm_zone);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->ship_x);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->ship_y);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->ship_course);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->ship_speed);
	    index += 4;
 	    mb_get_binary_int(MB_NO,&buffer[index],&header->sec);
	    index += 4;
 	    mb_get_binary_int(MB_NO,&buffer[index],&header->usec);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->spare_gain);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->fish_heading);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->fish_depth);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->fish_range);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->fish_pulse_width);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->gain_c0);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->gain_c1);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->gain_c2);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->fish_pitch);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->fish_roll);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->fish_yaw);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->fish_x);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->fish_y);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->fish_layback);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->fish_altitude);
	    index += 4;
 	    mb_get_binary_int(MB_NO,&buffer[index],&header->fish_altitude_samples);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->fish_ping_period);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->sound_velocity);
	    index += 4;
 	    mb_get_binary_int(MB_NO,&buffer[index],&header->num_chan);
	    index += 4;
	    for (i=0;i<MBF_OICGEODA_MAX_CHANNELS;i++)
		{
 		mb_get_binary_int(MB_NO,&buffer[index],&header->channel[i].offset);
		index += 4;
		}
	    for (i=0;i<MBF_OICGEODA_MAX_CHANNELS;i++)
		{
		header->channel[i].type = buffer[index]; 
		index += 1;
		header->channel[i].side = buffer[index]; 
		index += 1;
		header->channel[i].size = buffer[index]; 
		index += 1;
		header->channel[i].empty = buffer[index]; 
		index += 1;
 		mb_get_binary_int(MB_NO,&buffer[index],&header->channel[i].frequency);
		index += 4;
 		mb_get_binary_int(MB_NO,&buffer[index],&header->channel[i].num_samples);
		index += 4;
		}
	    }
	
	/* read client specific data */
	if (status == MB_SUCCESS && header->client_size > 0)
	    {
	    if (read_size = fread(dataplus->client,1,
				(int)(header->client_size),mb_io_ptr->mbfp)
		== (int)(header->client_size))
		{
		if (header->client_size < MBF_OICGEODA_MAX_CLIENT)
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
			    mb_swap_float(&float_ptr[j]);
			    }
			}
		    else if (header->channel[i].size == OIC_SIZE_3FLOAT)
			{
			float_ptr = (float *) data->raw[i];
			for (j=0;j<3*header->channel[i].num_samples;j++)
			    {
			    mb_swap_float(&float_ptr[j]);
			    }
			}
		    }
#endif
		}
	    }
		
	/* figure out number of beams and pixels in data */
	if (status == MB_SUCCESS 
	    && dataplus->kind == MB_DATA_DATA)
	    {
	    header->beams_bath = 0;
	    beams_bath_port = 0;
	    beams_bath_stbd = 0;
	    header->beams_amp = 0;
	    header->bath_chan_port = -1;
	    header->bath_chan_stbd = -1;
	    header->pixels_ss = 0;
	    header->ss_chan_port = -1;
	    header->ss_chan_stbd = -1;
	    for (ichan=0;ichan<header->num_chan;ichan++)
		{
		if (header->channel[ichan].type == OIC_TYPE_SIDESCAN)
		    {
		    if (header->channel[ichan].side == OIC_PORT
			&& header->ss_chan_port == -1)
			{
			header->ss_chan_port = ichan;
			header->fish_altitude_samples 
				= 2.0 * header->fish_altitude 
					* header->channel[ichan].num_samples
					/ header->sound_velocity
					/ header->fish_ping_period;
			header->pixels_ss += header->channel[ichan].num_samples 
			    - MIN(header->fish_altitude_samples - 1, 0);
			}
		    else if (header->channel[ichan].side == OIC_STARBOARD
			&& header->ss_chan_stbd == -1)
			{
			header->ss_chan_stbd = ichan;
			header->fish_altitude_samples 
				= 2.0 * header->fish_altitude 
					* header->channel[ichan].num_samples
					/ header->sound_velocity
					/ header->fish_ping_period;
			header->pixels_ss += header->channel[ichan].num_samples 
			    - MIN(header->fish_altitude_samples - 1, 0);
			}
		    }
		else if (header->channel[ichan].type == OIC_TYPE_ANGLE)
		    {
		    if (header->channel[ichan].side == OIC_PORT
			&& header->bath_chan_port == -1)
			{
			header->bath_chan_port = ichan;
			beams_bath_port = 0;
			if (header->channel[ichan].size == OIC_SIZE_SHORT)
			    short_ptr = (short *) data->raw[ichan];
			else if (header->channel[ichan].size == OIC_SIZE_INT)
			    int_ptr = (int *) data->raw[ichan];
			else if (header->channel[ichan].size == OIC_SIZE_FLOAT)
			    float_ptr = (float *) data->raw[ichan];
			for (i=0;i<header->channel[ichan].num_samples;i++)
			    {
			    if (header->channel[ichan].size == OIC_SIZE_SHORT)
				if (short_ptr[i] > 0) beams_bath_port++;
			    else if (header->channel[ichan].size == OIC_SIZE_INT)
				if (int_ptr[i] > 0) beams_bath_port++;
			    else if (header->channel[ichan].size == OIC_SIZE_FLOAT)
				if (float_ptr[i] > 0.0) beams_bath_port++;
			    }
			header->beams_bath = 2 * MAX(beams_bath_port, beams_bath_stbd) + 1;
			}
		    else if (header->channel[ichan].side == OIC_STARBOARD
			&& header->bath_chan_stbd == -1)
			{
			header->bath_chan_stbd = ichan;
			beams_bath_stbd = 0;
			if (header->channel[ichan].size == OIC_SIZE_SHORT)
			    short_ptr = (short *) data->raw[ichan];
			else if (header->channel[ichan].size == OIC_SIZE_INT)
			    int_ptr = (int *) data->raw[ichan];
			else if (header->channel[ichan].size == OIC_SIZE_FLOAT)
			    float_ptr = (float *) data->raw[ichan];
			for (i=0;i<header->channel[ichan].num_samples;i++)
			    {
			    if (header->channel[ichan].size == OIC_SIZE_SHORT)
				if (short_ptr[i] > 0) beams_bath_stbd++;
			    else if (header->channel[ichan].size == OIC_SIZE_INT)
				if (int_ptr[i] > 0) beams_bath_stbd++;
			    else if (header->channel[ichan].size == OIC_SIZE_FLOAT)
				if (float_ptr[i] > 0.0) beams_bath_stbd++;
			    }
			header->beams_bath = 2 * MAX(beams_bath_port, beams_bath_stbd) + 1;
			}
		    }
		else if (header->channel[ichan].type == OIC_TYPE_MULTIBEAM)
		    {
		    if (header->channel[ichan].side == OIC_PORT
			&& header->bath_chan_port == -1)
			{
			header->bath_chan_port = ichan;
			header->beams_bath += header->channel[ichan].num_samples;
			header->beams_amp += header->channel[ichan].num_samples;
			}
		    else if (header->channel[ichan].side == OIC_STARBOARD
			&& header->bath_chan_stbd == -1)
			{
			header->bath_chan_stbd = ichan;
			header->beams_bath += header->channel[ichan].num_samples;
			header->beams_amp += header->channel[ichan].num_samples;
			}
		    }
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

	/* construct bathymetry and sidescan from raw data */
	if (status == MB_SUCCESS 
	    && dataplus->kind == MB_DATA_DATA)
	    {
	    /* allocate arrays if needed */
	    if (header->beams_bath > data->beams_bath_alloc
		|| data->bath == NULL)
		{
		data->beams_bath_alloc = header->beams_bath;
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
		
	    /* initialize bathymetry and sidescan */
	    for (j=0;j<header->beams_bath;j++)
		{
		data->bathacrosstrack[j] = 0.0;
		data->bathalongtrack[j] = 0.0;
		data->bath[j] = 0.0;
		data->tt[j] = 0.0;
		data->angle[j] = 0.0;
		}
	    for (j=0;j<header->beams_amp;j++)
		{
		data->amp[j] = 0.0;
		}
	    for (j=0;j<header->pixels_ss;j++)
		{
		data->ssacrosstrack[j] = 0.0;
		data->ssalongtrack[j] = 0.0;
		data->ss[j] = 0.0;
		}

	    /* get center bathymetry */
	    if (header->bath_chan_port >= 0
		&& header->channel[header->bath_chan_port].type 
		    == OIC_TYPE_ANGLE
		&& header->bath_chan_stbd >= 0
		&& header->channel[header->bath_chan_stbd].type 
		    == OIC_TYPE_ANGLE)
		{
		j = header->beams_bath / 2;
		rr = header->fish_altitude;
		beta = 90.0;
		alpha = header->fish_pitch;
		status = mb_rollpitch_to_takeoff(verbose,
			    alpha,beta,&theta,&phi,error);
		xx = rr * sin(DTR * theta);
		zz = rr * cos(DTR * theta);
		data->bathacrosstrack[j] 
			= xx * cos(DTR * phi);
		data->bathalongtrack[j] 
			= xx * sin(DTR * phi);
		data->bath[j] = zz + header->fish_depth;
		data->tt[j] = 2.0 * rr / header->sound_velocity;
		data->angle[j] = beta;
		}

	    /* get port bathymetry */
	    if (header->bath_chan_port >= 0)
		{
		ichan = header->bath_chan_port;
		if (header->channel[ichan].size == OIC_SIZE_3FLOAT)
		    {
		    j = header->channel[ichan].num_samples;
		    }
		else
		    {
		    dx = header->fish_range 
			    / header->channel[ichan].num_samples;
		    j = header->beams_bath / 2;
		    }
		if (header->channel[ichan].size == OIC_SIZE_SHORT)
		    short_ptr = (short *) data->raw[ichan];
		else if (header->channel[ichan].size == OIC_SIZE_INT)
		    int_ptr = (int *) data->raw[ichan];
		else if (header->channel[ichan].size == OIC_SIZE_FLOAT)
		    float_ptr = (float *) data->raw[ichan];
		else if (header->channel[ichan].size == OIC_SIZE_3FLOAT)
		    float_ptr = (float *) data->raw[ichan];
		for (i=0;i<header->channel[ichan].num_samples;i++)
		    {
		    if (header->channel[ichan].size == OIC_SIZE_3FLOAT)
			{
			j--;
			beta = -float_ptr[3 * i + 1] + header->fish_roll;
			rr = 0.5 * header->sound_velocity 
				* float_ptr[3 * i];
			data->amp[j] = float_ptr[3 * i + 2];
			}
		    else
			{
			if (header->channel[ichan].size == OIC_SIZE_SHORT)
			    zz = 0.1 * short_ptr[i];
			else if (header->channel[ichan].size == OIC_SIZE_INT)
			    zz = 0.1 * int_ptr[i];
			else if (header->channel[ichan].size == OIC_SIZE_FLOAT)
			    zz = float_ptr[i];
			if (zz > 0.0)
			    {
			    j--;
			    xx = -(i + 0.5) * dx;
			    rr = sqrt(xx * xx + zz * zz);
			    beta = RTD * acos(xx / rr);
			    }
			else
			    rr = 0.0;
			}
		    if (rr > 0.0)
			{
			alpha = header->fish_pitch;
			status = mb_rollpitch_to_takeoff(verbose,
				    alpha,beta,&theta,&phi,error);
			xx = rr * sin(DTR * theta);
			zz = rr * cos(DTR * theta);
			data->bathacrosstrack[j] 
				= xx * cos(DTR * phi);
			data->bathalongtrack[j] 
				= xx * sin(DTR * phi);
			data->bath[j] = zz + header->fish_depth;
			data->tt[j] = 2.0 * rr / header->sound_velocity;
			data->angle[j] = beta;
			}
		    else if (header->channel[ichan].size == OIC_SIZE_3FLOAT)
			{
			data->bathacrosstrack[j] = 0.0;
			data->bathalongtrack[j] = 0.0;
			data->bath[j] = 0.0;
			data->tt[j] = 0.0;
			data->angle[j] = 0.0;
			}
		    }
		}

	    /* get starboard bathymetry */
	    if (header->bath_chan_stbd >= 0)
		{
		ichan = header->bath_chan_stbd;
		if (header->channel[ichan].size == OIC_SIZE_3FLOAT)
		    {
		    j = header->beams_bath - header->channel[ichan].num_samples - 1;
		    }
		else
		    {
		    dx = header->fish_range 
			    / header->channel[ichan].num_samples;
		    j = header->beams_bath / 2;
		    }
		if (header->channel[ichan].size == OIC_SIZE_SHORT)
		    short_ptr = (short *) data->raw[ichan];
		else if (header->channel[ichan].size == OIC_SIZE_INT)
		    int_ptr = (int *) data->raw[ichan];
		else if (header->channel[ichan].size == OIC_SIZE_FLOAT)
		    float_ptr = (float *) data->raw[ichan];
		else if (header->channel[ichan].size == OIC_SIZE_3FLOAT)
		    float_ptr = (float *) data->raw[ichan];
		for (i=0;i<header->channel[ichan].num_samples;i++)
		    {
		    if (header->channel[ichan].size == OIC_SIZE_3FLOAT)
			{
			j++;
			beta = float_ptr[3 * i + 1] + header->fish_roll;
			rr = 0.5 * header->sound_velocity 
				* float_ptr[3 * i];
			data->amp[j] = float_ptr[3 * i + 2];
			}
		    else
			{
			if (header->channel[ichan].size == OIC_SIZE_SHORT)
			    zz = 0.1 * short_ptr[i];
			else if (header->channel[ichan].size == OIC_SIZE_INT)
			    zz = 0.1 * int_ptr[i];
			else if (header->channel[ichan].size == OIC_SIZE_FLOAT)
			    zz = float_ptr[i];
			if (zz > 0.0)
			    {
			    j++;
			    xx = (i + 0.5) * dx;
			    rr = sqrt(xx * xx + zz * zz);
			    beta = RTD * acos(xx / rr);
			    }
			else
			    rr = 0.0;
			}
		    if (rr > 0.0)
			{
			alpha = header->fish_pitch;
			status = mb_rollpitch_to_takeoff(verbose,
				    alpha,beta,&theta,&phi,error);
			xx = rr * sin(DTR * theta);
			zz = rr * cos(DTR * theta);
			data->bathacrosstrack[j] 
				= xx * cos(DTR * phi);
			data->bathalongtrack[j] 
				= xx * sin(DTR * phi);
			data->bath[j] = zz + header->fish_depth;
			data->tt[j] = 2.0 * rr / header->sound_velocity;
			data->angle[j] = beta;
			}
		    else if (header->channel[ichan].size == OIC_SIZE_3FLOAT)
			{
			data->bathacrosstrack[j] = 0.0;
			data->bathalongtrack[j] = 0.0;
			data->bath[j] = 0.0;
			data->tt[j] = 0.0;
			data->angle[j] = 0.0;
			}
		    }
		}

	    /* get port sidescan */
	    if (header->ss_chan_port >= 0)
		{
		ichan = header->ss_chan_port;
		sample_interval = header->fish_ping_period
				    / header->channel[ichan].num_samples;
		if (header->channel[ichan].size == OIC_SIZE_CHAR)
		    char_ptr = (char *) data->raw[ichan];
		else if (header->channel[ichan].size == OIC_SIZE_SHORT)
		    short_ptr = (short *) data->raw[ichan];
		else if (header->channel[ichan].size == OIC_SIZE_INT)
		    int_ptr = (int *) data->raw[ichan];
		else if (header->channel[ichan].size == OIC_SIZE_FLOAT)
		    float_ptr = (float *) data->raw[ichan];
		for (i=header->fish_altitude_samples+1;
		    i<header->channel[ichan].num_samples;i++)
		    {
		    j = header->channel[ichan].num_samples - i
			    + header->fish_altitude_samples;
		    if (header->channel[ichan].size == OIC_SIZE_CHAR)
			data->ss[j] = char_ptr[i];
		    else if (header->channel[ichan].size == OIC_SIZE_SHORT)
			data->ss[j] = short_ptr[i];
		    else if (header->channel[ichan].size == OIC_SIZE_INT)
			data->ss[j] = int_ptr[i];
		    else if (header->channel[ichan].size == OIC_SIZE_FLOAT)
			data->ss[j] = float_ptr[i];
		    beta = 180.0 - asin(((double)header->fish_altitude_samples) 
				/ ((double)i)) / DTR;
		    alpha = header->fish_pitch;
		    rr = 0.5 * header->sound_velocity 
			    * sample_interval * i;
		    status = mb_rollpitch_to_takeoff(verbose,
				alpha,beta,&theta,&phi,error);
		    xx = rr * sin(DTR * theta);
		    zz = rr * cos(DTR * theta);
		    data->ssacrosstrack[j] = xx * cos(DTR * phi);
		    data->ssalongtrack[j] = xx * sin(DTR * phi);
		    }
		}

	    /* get starboard sidescan */
	    if (header->ss_chan_stbd >= 0)
		{
		ichan = header->ss_chan_stbd;
		sample_interval = header->fish_ping_period
				    / header->channel[ichan].num_samples;
		if (header->channel[ichan].size == OIC_SIZE_CHAR)
		    char_ptr = (char *) data->raw[ichan];
		else if (header->channel[ichan].size == OIC_SIZE_SHORT)
		    short_ptr = (short *) data->raw[ichan];
		else if (header->channel[ichan].size == OIC_SIZE_INT)
		    int_ptr = (int *) data->raw[ichan];
		else if (header->channel[ichan].size == OIC_SIZE_FLOAT)
		    float_ptr = (float *) data->raw[ichan];
		for (i=header->fish_altitude_samples+1;
		    i<header->channel[ichan].num_samples;i++)
		    {
		    j = header->pixels_ss 
			    - header->channel[ichan].num_samples 
			    + i - header->fish_altitude_samples - 1;
		    if (header->channel[ichan].size == OIC_SIZE_CHAR)
			data->ss[j] = char_ptr[i];
		    else if (header->channel[ichan].size == OIC_SIZE_SHORT)
			data->ss[j] = short_ptr[i];
		    else if (header->channel[ichan].size == OIC_SIZE_INT)
			data->ss[j] = int_ptr[i];
		    else if (header->channel[ichan].size == OIC_SIZE_FLOAT)
			data->ss[j] = float_ptr[i];
		    beta = asin(((double)header->fish_altitude_samples) 
				/ ((double)i)) / DTR;
		    alpha = header->fish_pitch;
		    rr = 0.5 * header->sound_velocity 
			    * sample_interval * i;
		    status = mb_rollpitch_to_takeoff(verbose,
				alpha,beta,&theta,&phi,error);
		    xx = rr * sin(DTR * theta);
		    zz = rr * cos(DTR * theta);
		    data->ssacrosstrack[j] = xx * cos(DTR * phi);
		    data->ssalongtrack[j] = xx * sin(DTR * phi);
		    }
		}
	    }

	/* print debug statements */
	if (verbose >= 5 && status == MB_SUCCESS)
	    {
	    fprintf(stderr,"\ndbg5  New processed data generated in function <%s>\n",
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

	    /* nav */
	    store->nav_used = header->nav_used;
	    store->nav_type = header->nav_type;
	    store->utm_zone = header->utm_zone;
	    store->ship_x = header->ship_x;
	    store->ship_y = header->ship_y;
	    store->ship_course = header->ship_course;
	    store->ship_speed = header->ship_speed;

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
		if (data->bath[i] == 0.0)
		    store->beamflag[i] = MB_FLAG_NULL;
		else
		    store->beamflag[i] = MB_FLAG_NONE;
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
int mbr_wt_oicgeoda(int verbose, char *mbio_ptr, char *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_oicgeoda";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_oicgeoda_struct *dataplus;
	struct mbf_oicgeoda_header_struct *header;
	struct mbf_oicgeoda_data_struct *data;
	struct mbsys_oic_struct *store;
	char	buffer[MBF_OICGEODA_HEADER_SIZE];
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
	dataplus = (struct mbf_oicgeoda_struct *) mb_io_ptr->raw_data;
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

	    /* nav */
	    header->nav_used = store->nav_used;
	    header->nav_type = store->nav_type;
	    header->utm_zone = store->utm_zone;
	    header->ship_x = store->ship_x;
	    header->ship_y = store->ship_y;
	    header->ship_course = store->ship_course;
	    header->ship_speed = store->ship_speed;

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
		|| data->bath == NULL
		|| data->bathacrosstrack == NULL
		|| data->bathalongtrack == NULL
		|| data->tt == NULL
		|| data->angle == NULL)
		{
		data->beams_bath_alloc = header->beams_bath;
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
	    
	/* now reverse parse the header */
	if (status == MB_SUCCESS)
	    {
	    index = 0;
	    buffer[index] = 'G'; 
	    index += 1;
	    buffer[index] = 'E'; 
	    index += 1;
	    buffer[index] = 'O'; 
	    index += 1;
	    buffer[index] = header->type; 
	    index += 1;
 	    mb_put_binary_int(MB_NO,header->proc_status,&buffer[index]);
	    index += 4;
	    mb_put_binary_int(MB_NO,header->data_size,&buffer[index]);
	    index += 4;
	    buffer[index] = header->client_size; index += 1;
	    buffer[index] = header->fish_status; index += 1;
	    buffer[index] = header->nav_used; index += 1;
	    buffer[index] = header->nav_type; index += 1;
 	    mb_put_binary_int(MB_NO,header->utm_zone,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->ship_x,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->ship_y,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->ship_course,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->ship_speed,&buffer[index]);
	    index += 4;
 	    mb_put_binary_int(MB_NO,header->sec,&buffer[index]);
	    index += 4;
 	    mb_put_binary_int(MB_NO,header->usec,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->spare_gain,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->fish_heading,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->fish_depth,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->fish_range,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->fish_pulse_width,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->gain_c0,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->gain_c1,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->gain_c2,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->fish_pitch,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->fish_roll,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->fish_yaw,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->fish_x,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->fish_y,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->fish_layback,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->fish_altitude,&buffer[index]);
	    index += 4;
 	    mb_put_binary_int(MB_NO,header->fish_altitude_samples,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->fish_ping_period,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->sound_velocity,&buffer[index]);
	    index += 4;
 	    mb_put_binary_int(MB_NO,header->num_chan,&buffer[index]);
	    index += 4;
	    for (i=0;i<MBF_OICGEODA_MAX_CHANNELS;i++)
		{
 		mb_put_binary_int(MB_NO,header->channel[i].offset,&buffer[index]);
		index += 4;
		}
	    for (i=0;i<MBF_OICGEODA_MAX_CHANNELS;i++)
		{
		buffer[index] = header->channel[i].type; index += 1;
		buffer[index] = header->channel[i].side; index += 1;
		buffer[index] = header->channel[i].size; index += 1;
		buffer[index] = header->channel[i].empty; index += 1;
 		mb_put_binary_int(MB_NO,header->channel[i].frequency,&buffer[index]);
		index += 4;
 		mb_put_binary_int(MB_NO,header->channel[i].num_samples,&buffer[index]);
		index += 4;
		}
	    }

	/* write next header to file */
	if ((write_size = fwrite(buffer,1,MBF_OICGEODA_HEADER_SIZE,
			mb_io_ptr->mbfp)) == MBF_OICGEODA_HEADER_SIZE) 
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
			    mb_swap_float(&float_ptr[j]);
			    }
			}
		    else if (header->channel[i].size == OIC_SIZE_3FLOAT)
			{
			float_ptr = (float *) data->raw[i];
			for (j=0;j<3*header->channel[i].num_samples;j++)
			    {
			    mb_swap_float(&float_ptr[j]);
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
