/*--------------------------------------------------------------------
 *    The MB-system:	mbr_mstiffss.c	4/7/98
 *	$Id$
 *
 *    Copyright (c) 1998-2013 by
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
 * mbr_mstiffss.c contains the functions for reading
 * sidescan data in the MSTIFFSS format.
 * These functions include:
 *   mbr_alm_mstiffss	- allocate read/write memory
 *   mbr_dem_mstiffss	- deallocate read/write memory
 *   mbr_rt_mstiffss	- read and translate data
 *   mbr_wt_mstiffss	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	April 7, 1998
 * $Log: mbr_mstiffss.c,v $
 * Revision 5.8  2005/11/05 00:48:05  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.7  2003/05/20 18:05:32  caress
 * Added svp_source to data source parameters.
 *
 * Revision 5.6  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.5  2003/01/15 20:51:48  caress
 * Release 5.0.beta28
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
 * Revision 4.3  2000/10/11  01:03:21  caress
 * Convert to ANSI C
 *
 * Revision 4.2  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.1  1999/03/31  18:11:35  caress
 * MB-System 4.6beta7
 *
 * Revision 4.0  1998/10/05  19:16:02  caress
 * MB-System version 4.6beta
 *
 * Revision 1.1  1998/10/05  18:32:27  caress
 * Initial revision
 *
 * Revision 1.1  1998/10/05  18:22:40  caress
 * Initial revision
 *
 * Revision 1.1  1998/10/05  17:46:15  caress
 * Initial revision
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_define.h"
#include "mbsys_mstiff.h"
#include "mbf_mstiffss.h"

/* essential function prototypes */
int mbr_register_mstiffss(int verbose, void *mbio_ptr,
		int *error);
int mbr_info_mstiffss(int verbose,
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
int mbr_alm_mstiffss(int verbose, void *mbio_ptr, int *error);
int mbr_dem_mstiffss(int verbose, void *mbio_ptr, int *error);
int mbr_rt_mstiffss(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_mstiffss(int verbose, void *mbio_ptr, void *store_ptr, int *error);

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/
int mbr_register_mstiffss(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_register_mstiffss";
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
	status = mbr_info_mstiffss(verbose,
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_mstiffss;
	mb_io_ptr->mb_io_format_free = &mbr_dem_mstiffss;
	mb_io_ptr->mb_io_store_alloc = &mbsys_mstiff_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_mstiff_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_mstiffss;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_mstiffss;
	mb_io_ptr->mb_io_dimensions = &mbsys_mstiff_dimensions;
	mb_io_ptr->mb_io_extract = &mbsys_mstiff_extract;
	mb_io_ptr->mb_io_insert = &mbsys_mstiff_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_mstiff_extract_nav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_mstiff_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_mstiff_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = NULL;
	mb_io_ptr->mb_io_insert_svp = NULL;
	mb_io_ptr->mb_io_ttimes = &mbsys_mstiff_ttimes;
	mb_io_ptr->mb_io_detects = &mbsys_mstiff_detects;
	mb_io_ptr->mb_io_copyrecord = &mbsys_mstiff_copy;
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
		fprintf(stderr,"dbg2       format_alloc:       %p\n",(void *)mb_io_ptr->mb_io_format_alloc);
		fprintf(stderr,"dbg2       format_free:        %p\n",(void *)mb_io_ptr->mb_io_format_free);
		fprintf(stderr,"dbg2       store_alloc:        %p\n",(void *)mb_io_ptr->mb_io_store_alloc);
		fprintf(stderr,"dbg2       store_free:         %p\n",(void *)mb_io_ptr->mb_io_store_free);
		fprintf(stderr,"dbg2       read_ping:          %p\n",(void *)mb_io_ptr->mb_io_read_ping);
		fprintf(stderr,"dbg2       write_ping:         %p\n",(void *)mb_io_ptr->mb_io_write_ping);
		fprintf(stderr,"dbg2       extract:            %p\n",(void *)mb_io_ptr->mb_io_extract);
		fprintf(stderr,"dbg2       insert:             %p\n",(void *)mb_io_ptr->mb_io_insert);
		fprintf(stderr,"dbg2       extract_nav:        %p\n",(void *)mb_io_ptr->mb_io_extract_nav);
		fprintf(stderr,"dbg2       insert_nav:         %p\n",(void *)mb_io_ptr->mb_io_insert_nav);
		fprintf(stderr,"dbg2       extract_altitude:   %p\n",(void *)mb_io_ptr->mb_io_extract_altitude);
		fprintf(stderr,"dbg2       insert_altitude:    %p\n",(void *)mb_io_ptr->mb_io_insert_altitude);
		fprintf(stderr,"dbg2       extract_svp:        %p\n",(void *)mb_io_ptr->mb_io_extract_svp);
		fprintf(stderr,"dbg2       insert_svp:         %p\n",(void *)mb_io_ptr->mb_io_insert_svp);
		fprintf(stderr,"dbg2       ttimes:             %p\n",(void *)mb_io_ptr->mb_io_ttimes);
		fprintf(stderr,"dbg2       detects:            %p\n",(void *)mb_io_ptr->mb_io_detects);
		fprintf(stderr,"dbg2       extract_rawss:      %p\n",(void *)mb_io_ptr->mb_io_extract_rawss);
		fprintf(stderr,"dbg2       insert_rawss:       %p\n",(void *)mb_io_ptr->mb_io_insert_rawss);
		fprintf(stderr,"dbg2       copyrecord:         %p\n",(void *)mb_io_ptr->mb_io_copyrecord);
		fprintf(stderr,"dbg2       error:              %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		}

	/* return status */
	return(status);
}

/*--------------------------------------------------------------------*/
int mbr_info_mstiffss(int verbose,
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
	char	*function_name = "mbr_info_mstiffss";
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
	*system = MB_SYS_MSTIFF;
	*beams_bath_max = 0;
	*beams_amp_max = 0;
	*pixels_ss_max = 1024;
	strncpy(format_name, "MSTIFFSS", MB_NAME_LENGTH);
	strncpy(system_name, "MSTIFF", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_MSTIFFSS\nInformal Description: MSTIFF sidescan format\nAttributes:           variable pixels,  sidescan,\n                      binary TIFF variant, single files, Sea Scan. \n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_NO;
	*traveltime = MB_NO;
	*beam_flagging = MB_NO;
	*nav_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_DATA;
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
int mbr_alm_mstiffss(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_alm_mstiffss";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int	*n_read;
	int	*n_nav_use;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set initial status */
	status = MB_SUCCESS;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = sizeof(struct mbf_mstiffss_struct);
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mb_malloc(verbose,sizeof(struct mbsys_mstiff_struct),
				&mb_io_ptr->store_data,error);

	/* set number read counter */
	n_read = &(mb_io_ptr->save_label_flag);
	n_nav_use = &(mb_io_ptr->save8);
	*n_read = 0;
	*n_nav_use = 0;

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
int mbr_dem_mstiffss(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_mstiffss";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* deallocate memory for data descriptor */
	status = mb_free(verbose,&mb_io_ptr->raw_data,error);
	status = mb_free(verbose,&mb_io_ptr->store_data,error);

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
int mbr_rt_mstiffss(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_mstiffss";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mstiffss_struct *data;
	struct mbsys_mstiff_struct *store;
	char	buffer[MBF_MSTIFFSS_BUFFERSIZE];
	int	ifd_offset;
	short	nentry;
	short	tag;
	short	type;
	int	count;
	int	value_offset;
	int	*n_read;
	int	*bits_per_pixel;
	int	*n_ping_file;
	int	*n_pixel_channel;
	int	*left_channel_offset;
	int	*right_channel_offset;
	int	*sonar_data_info_offset;
	int	*sonar_data_info_tag;
	int	*n_nav;
	int	*n_nav_use;
	int	*nav_info_offset;
	int	*nav_info_tag;
	int	timecorr_tag;
	int	timecorr_offset;
	short	corr_time[9];
	int	*ref_windows_time;
	int	corr_time_i[7];
	double	*corr_time_d;
	int	date, time;
	int	pingtime;
	short   range_code;
	short	frequency_code;
	short   range_delay_bin;
	short   altitude_bin;
	int	range_mode;
	int	channel_mode;
	double	range_per_bin;
	double  range;
	double  range_delay;
	double  altitude;
	double	frequency;
	short	sonar_gain[16];
	int	navsize;
	int	navtime1, navtime2;
	float	lon1, lon2;
	float	lat1, lat2;
	float	speed1, speed2;
	float	course1, course2;
	float	heading1, heading2;
	double	lon, lat, speed, course, heading;
	double	factor;
	double	range_tot;
	unsigned char left_channel[MBF_MSTIFFSS_PIXELS/2];
	unsigned char right_channel[MBF_MSTIFFSS_PIXELS/2];
	int	ibottom;
	int	istart;
	int	index;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	data = (struct mbf_mstiffss_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_mstiff_struct *) store_ptr;

	/* get pointers to saved values in mb_io_ptr */
	n_read = &(mb_io_ptr->save_label_flag);
	bits_per_pixel = &(mb_io_ptr->save_flag);
	n_ping_file = &(mb_io_ptr->save1);
	n_pixel_channel = &(mb_io_ptr->save2);
	left_channel_offset = &(mb_io_ptr->save3);
	right_channel_offset = &(mb_io_ptr->save4);
	sonar_data_info_offset = &(mb_io_ptr->save5);
	sonar_data_info_tag = &(mb_io_ptr->save6);
	n_nav = &(mb_io_ptr->save7);
	n_nav_use = &(mb_io_ptr->save8);
	nav_info_offset = &(mb_io_ptr->save9);
	nav_info_tag = &(mb_io_ptr->save10);
	ref_windows_time = &(mb_io_ptr->save11);
	corr_time_d = &(mb_io_ptr->saved1);

	/* if first time through, read file header and
	    offsets, setting up for later reads */
	if (*n_read <= 0)
	    {
	    /* set defaults */
	    *bits_per_pixel = 8;
	    /* check for proper file tag */
	    if ((status = fread(buffer, 4 * sizeof(char),
			    1, mb_io_ptr->mbfp)) != 1)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
	    else if (strncmp(buffer, "MSTL", 4) != 0)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_DATA;
		}
	    else
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}

	    /* now get the image file directory offset */
	    if ((status = fread(buffer,
			    sizeof(int), 1, mb_io_ptr->mbfp)) == 1)
		{
		mb_get_binary_int(MB_YES, buffer, &ifd_offset);
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
	    else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	    /* now parse through the image file directory
		and grab the important values */
	    if (status == MB_SUCCESS)
		{
		status = fseek(mb_io_ptr->mbfp, (size_t) ifd_offset, SEEK_SET);
		status = ftell(mb_io_ptr->mbfp);
		if ((status = fread(buffer, sizeof(short),
			1, mb_io_ptr->mbfp)) == 1)
		    {
		    mb_get_binary_short(MB_YES, buffer, &nentry);

		    /* loop over all entries in the directory */
		    if ((status = status = fread(buffer,
		    	6 * nentry * sizeof(short), 1, mb_io_ptr->mbfp)) == 1)
			{
			index = 0;
		    	for (i=0;i<nentry && status == MB_SUCCESS;i++)
			     {
		    	     mb_get_binary_short(MB_YES, &(buffer[index]), &tag);
			     index += sizeof(short);
		    	     mb_get_binary_short(MB_YES, &(buffer[index]), &type);
			     index += sizeof(short);
		    	     mb_get_binary_int(MB_YES, &(buffer[index]), &count);
			     index += sizeof(int);
		    	     mb_get_binary_int(MB_YES, &(buffer[index]), &value_offset);
			     index += sizeof(int);
/*fprintf(stderr,"tag:%d type:%d count:%d value_offset:%d\n",
tag,type,count,value_offset);*/

			    /* set values for important entries */

			    /* BitsPerBin */
			    if (tag == BitsPerBin)
				{
				*bits_per_pixel = value_offset;
				}

			    /* SonarLines */
			    else if (tag == SonarLines)
				{
				*n_ping_file = value_offset;
				}

			    /* BinsPerChannel */
			    else if (tag == BinsPerChannel)
				{
				*n_pixel_channel = value_offset;
				}

			    /* TimeCorrelation */
			    else if (tag == TimeCorrelation)
				{
				timecorr_tag = tag;
				timecorr_offset = value_offset;
				}

			    /* Y2KTimeCorrelation */
			    else if (tag == Y2KTimeCorrelation)
				{
				timecorr_tag = tag;
				timecorr_offset = value_offset;
				}

			    /* LeftChannel */
			    else if (tag == LeftChannel)
				{
				*left_channel_offset = value_offset;
				}

			    /* LeftChannel2 */
			    else if (tag == LeftChannel2)
				{
				*left_channel_offset = value_offset;
				}

			    /* RightChannel */
			    else if (tag == RightChannel)
				{
				*right_channel_offset = value_offset;
				}

			    /* RightChannel2 */
			    else if (tag == RightChannel2)
				{
				*right_channel_offset = value_offset;
				}

			    /* SonarDataInfo */
			    else if (tag == SonarDataInfo)
				{
				*sonar_data_info_offset = value_offset;
				*sonar_data_info_tag = tag;
				}

			    /* SonarDataInfo2 */
			    else if (tag == SonarDataInfo2)
				{
				*sonar_data_info_offset = value_offset;
				*sonar_data_info_tag = tag;
				}

			    /* SonarDataInfo3 */
			    else if (tag == SonarDataInfo3)
				{
				*sonar_data_info_offset = value_offset;
				*sonar_data_info_tag = tag;
				}

			    /* NavInfoCount */
			    else if (tag == NavInfoCount)
				{
				*n_nav = value_offset;
				}

			    /* NavInfo */
			    else if (tag == NavInfo)
				{
				*nav_info_offset = value_offset;
				*nav_info_tag = tag;
				}

			    /* NavInfo2 */
			    else if (tag == NavInfo2)
				{
				*nav_info_offset = value_offset;
				*nav_info_tag = tag;
				}

			    /* NavInfo3 */
			    else if (tag == NavInfo3)
				{
				*nav_info_offset = value_offset;
				*nav_info_tag = tag;
				}

			    /* NavInfo4 */
			    else if (tag == NavInfo4)
				{
				*nav_info_offset = value_offset;
				*nav_info_tag = tag;
				}

			    /* NavInfo5 */
			    else if (tag == NavInfo5)
				{
				*nav_info_offset = value_offset;
				*nav_info_tag = tag;
				}

			    /* NavInfo6 */
			    else if (tag == NavInfo6)
				{
				*nav_info_offset = value_offset;
				*nav_info_tag = tag;
				}
			     }
			}
		    else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		    }
		else
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    }
		}

	    /* set the time correlation */
	    if (status == MB_SUCCESS)
		{
	    	/* TimeCorrelation */
	    	if (timecorr_tag == TimeCorrelation)
		    {
		    /* get time correlation values */
		    fseek(mb_io_ptr->mbfp, timecorr_offset, SEEK_SET);
		    if ((status = fread(buffer, sizeof(int) + 9 * sizeof(short),
			    1, mb_io_ptr->mbfp)) == 1)
			{
			index = 0;
		    	mb_get_binary_int(MB_YES, &(buffer[index]), ref_windows_time);
			index += sizeof(int);
			for (i=0;i<9;i++)
			    {
		    	    mb_get_binary_short(MB_YES, &(buffer[index]),
			    			&(corr_time[i]));
			    index += sizeof(short);
			    }
			mb_fix_y2k(verbose,(int)corr_time[5],
				    &corr_time_i[0]);
			corr_time_i[1] = corr_time[4] + 1;
			corr_time_i[2] = corr_time[3];
			corr_time_i[3] = corr_time[2];
			corr_time_i[4] = corr_time[1];
			corr_time_i[5] = corr_time[0];
			corr_time_i[6] = 0;
			mb_get_time(verbose,corr_time_i,
				corr_time_d);

			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			}
		    else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		    }

		/* Y2KTimeCorrelation */
		else if (timecorr_tag == Y2KTimeCorrelation)
		    {
		    /* get time correlation values */
		    fseek(mb_io_ptr->mbfp, timecorr_offset, SEEK_SET);
		    if ((status = fread(buffer, 3 * sizeof(int),
			    1, mb_io_ptr->mbfp)) == 1)
			{
			index = 0;
		    	mb_get_binary_int(MB_YES, &(buffer[index]), ref_windows_time);
			index += sizeof(int);
		    	mb_get_binary_int(MB_YES, &(buffer[index]), &date);
			index += sizeof(int);
		    	mb_get_binary_int(MB_YES, &(buffer[index]), &time);
			index += sizeof(int);
			corr_time_i[0] = date / 10000;
			corr_time_i[1] = (date - corr_time_i[0] * 10000)/ 100;
			corr_time_i[2] = (date - corr_time_i[0] * 10000
				    	    - corr_time_i[1] * 100);
			corr_time_i[3] = (time / 3600);
			corr_time_i[4] = (time - corr_time_i[3] * 3600) / 60;
			corr_time_i[5] = (time - corr_time_i[3] * 3600
				    	    - corr_time_i[4] * 60);
			corr_time_i[6] = 0;
			mb_get_time(verbose,corr_time_i,
				corr_time_d);
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			}
		    else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		    }
		else
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    }
		}
	    }

	/* if all pings already read then return EOF */
	if (status == MB_SUCCESS && *n_read >= *n_ping_file)
	    {
	    status = MB_FAILURE;
	    *error = MB_ERROR_EOF;
	    }

	/* else read next ping */
	else if (status == MB_SUCCESS)
	    {
	    /* get sonar data info */
	    if (*sonar_data_info_tag == SonarDataInfo)
	    	{
		fseek(mb_io_ptr->mbfp,
			*sonar_data_info_offset
			    + (*n_read) * (sizeof(int) + 3 * sizeof(short)),
			SEEK_SET);
		if ((status = fread(buffer, sizeof(int) + 3 * sizeof(short),
				1, mb_io_ptr->mbfp)) == 1)
		    {
		    index = 0;
		    mb_get_binary_int(MB_YES, &(buffer[index]), &pingtime);
		    index += sizeof(int);
		    mb_get_binary_short(MB_YES, &(buffer[index]), &range_code);
		    index += sizeof(short);
		    frequency_code = FREQ_UNKNOWN;
		    mb_get_binary_short(MB_YES, &(buffer[index]), &range_delay_bin);
		    index += sizeof(short);
		    mb_get_binary_short(MB_YES, &(buffer[index]), &altitude_bin);
		    index += sizeof(short);
		    status = MB_SUCCESS;
		    *error = MB_ERROR_NO_ERROR;
		    for (i=0;i<16;i++)
		    	sonar_gain[i] = 0;
		    }
		else
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    }
		}

	    /* get sonar data info */
	    else if (*sonar_data_info_tag == SonarDataInfo2)
	    	{
		fseek(mb_io_ptr->mbfp,
			*sonar_data_info_offset
			    + (*n_read) * (sizeof(int) + 4 * sizeof(short)),
			SEEK_SET);
		if ((status = fread(buffer, sizeof(int) + 4 * sizeof(short),
				1, mb_io_ptr->mbfp)) == 1)
		    {
		    index = 0;
		    mb_get_binary_int(MB_YES, &(buffer[index]), &pingtime);
		    index += sizeof(int);
		    mb_get_binary_short(MB_YES, &(buffer[index]), &range_code);
		    index += sizeof(short);
		    mb_get_binary_short(MB_YES, &(buffer[index]), &frequency_code);
		    index += sizeof(short);
		    mb_get_binary_short(MB_YES, &(buffer[index]), &range_delay_bin);
		    index += sizeof(short);
		    mb_get_binary_short(MB_YES, &(buffer[index]), &altitude_bin);
		    index += sizeof(short);
		    status = MB_SUCCESS;
		    *error = MB_ERROR_NO_ERROR;
		    for (i=0;i<16;i++)
		    	sonar_gain[i] = 0;
		    status = MB_SUCCESS;
		    *error = MB_ERROR_NO_ERROR;
		    }
		else
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    }
		}

	    /* get sonar data info */
	    else if (*sonar_data_info_tag == SonarDataInfo3)
	    	{
		fseek(mb_io_ptr->mbfp,
			*sonar_data_info_offset
			    + (*n_read) * (sizeof(int) + 20 * sizeof(short)),
			SEEK_SET);
		if ((status = fread(buffer, sizeof(int) + 20 * sizeof(short),
				1, mb_io_ptr->mbfp)) == 1)
		    {
		    index = 0;
		    mb_get_binary_int(MB_YES, &(buffer[index]), &pingtime);
		    index += sizeof(int);
		    mb_get_binary_short(MB_YES, &(buffer[index]), &range_code);
		    index += sizeof(short);
		    mb_get_binary_short(MB_YES, &(buffer[index]), &frequency_code);
		    index += sizeof(short);
		    mb_get_binary_short(MB_YES, &(buffer[index]), &range_delay_bin);
		    index += sizeof(short);
		    mb_get_binary_short(MB_YES, &(buffer[index]), &altitude_bin);
		    index += sizeof(short);
		    for (i=0;i<16;i++)
		    	{
			mb_get_binary_short(MB_YES, &(buffer[index]), &(sonar_gain[i]));
		        index += sizeof(short);
			}
		    status = MB_SUCCESS;
		    *error = MB_ERROR_NO_ERROR;
		    }
		else
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    }
		}

	    /* make sense of sonar data info */
	    if (status == MB_SUCCESS)
		{
		channel_mode = (range_code & ~63) >> 6;
		if (channel_mode == 3) channel_mode = 0;
		range_mode = range_code & 15;
		switch (range_mode)
		    {
		    case 1:
			range = 5;
			break;
		    case 2:
			range = 10;
			break;
		    case 3:
			range = 20;
			break;
		    case 4:
			range = 50;
			break;
		    case 5:
			range = 75;
			break;
		    case 6:
			range = 100;
			break;
		    case 7:
			range = 150;
			break;
		    case 8:
			range = 200;
			break;
		    case 9:
			range = 300;
			break;
		    case 10:
			range = 500;
			break;
		    default:
			range = 5;
			break;
		    }
		range_per_bin = range / *n_pixel_channel;
		range_delay = range_delay_bin * range_per_bin;
		altitude = altitude_bin * range_per_bin;
		switch (frequency_code)
		    {
		    case 1:
			frequency = 150.0;
			break;
		    case 2:
			frequency = 300.0;
			break;
		    case 3:
			frequency = 600.0;
			break;
		    case 4:
			frequency = 1200.0;
			break;
		    case 5:
			frequency = 0.0;
			break;
		    }
		}

	    /* now get navigation */
	    if (status == MB_SUCCESS
		&& *n_nav_use < *n_nav)
		{
		switch (*nav_info_tag)
		    {
		    case NavInfo:
			navsize = 16 * sizeof(int);
			break;
		    case NavInfo2:
			navsize = 19 * sizeof(int);
			break;
		    case NavInfo3:
			navsize = 16 * sizeof(int);
			break;
		    case NavInfo4:
			navsize = 19 * sizeof(int);
			break;
		    case NavInfo5:
			navsize = 20 * sizeof(int);
			break;
		    case NavInfo6:
			navsize = 20 * sizeof(int);
			break;
		    default:
			navsize = 20 * sizeof(int);
			break;
		    }

		/* read first nav point starting with last
		   nav used */
		fseek(mb_io_ptr->mbfp,
		    *nav_info_offset + (*n_nav_use)
			* navsize, SEEK_SET);
		if ((status = fread(buffer, navsize,
				1, mb_io_ptr->mbfp)) == 1)
		    {
		    index = 0;
		    mb_get_binary_int(MB_YES, &(buffer[index]), &navtime1);
		    index += sizeof(int);
		    mb_get_binary_float(MB_YES, &(buffer[index]), &lat1);
		    index += sizeof(float);
		    mb_get_binary_float(MB_YES, &(buffer[index]), &lon1);
		    index += sizeof(float);
		    mb_get_binary_float(MB_YES, &(buffer[index]), &speed1);
		    index += sizeof(float);
		    mb_get_binary_float(MB_YES, &(buffer[index]), &course1);
		    if (*nav_info_tag == NavInfo6)
		    	{
			index += 3 * sizeof(float);
		        mb_get_binary_float(MB_YES, &(buffer[index]), &heading1);
			}
		    else
			{
			heading1 = course1;
			}
		    lon1 /= 60.;
		    lat1 /= 60.;
		    status = MB_SUCCESS;
		    *error = MB_ERROR_NO_ERROR;
		    }
		else
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    }

		/* read second nav point starting with last
		   nav used */
		fseek(mb_io_ptr->mbfp,
		    *nav_info_offset + (*n_nav_use+1)
			* navsize, SEEK_SET);
		if ((status = fread(buffer, navsize,
				1, mb_io_ptr->mbfp)) == 1)
		    {
		    index = 0;
		    mb_get_binary_int(MB_YES, &(buffer[index]), &navtime2);
		    index += sizeof(int);
		    mb_get_binary_float(MB_YES, &(buffer[index]), &lat2);
		    index += sizeof(float);
		    mb_get_binary_float(MB_YES, &(buffer[index]), &lon2);
		    index += sizeof(float);
		    mb_get_binary_float(MB_YES, &(buffer[index]), &speed2);
		    index += sizeof(float);
		    mb_get_binary_float(MB_YES, &(buffer[index]), &course2);
		    if (*nav_info_tag == NavInfo6)
		    	{
			index += 3 * sizeof(float);
		        mb_get_binary_float(MB_YES, &(buffer[index]), &heading2);
			}
		    else
			{
			heading2 = course2;
			}
		    lon2 /= 60.;
		    lat2 /= 60.;
		    status = MB_SUCCESS;
		    *error = MB_ERROR_NO_ERROR;
		    }
		else
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    navtime2 = 0.0;
		    }

		/* if first two nav points don't bracket ping
		   in time keep reading until two found that
		   do or end of nav reached */
		while (pingtime > navtime2 && *n_nav_use < *n_nav - 2)
		    {
		    /* move nav 2 to nav 1 */
		    navtime1 = navtime2;
		    lat1 = lat2;
		    lon1 = lon2;
		    speed1 = speed2;
		    course1 = course2;

		    /* read second nav point starting with last
		       nav used */
		    fseek(mb_io_ptr->mbfp,
			*nav_info_offset + (*n_nav_use+2)
			    * navsize, SEEK_SET);
		    if ((status = fread(buffer, navsize,
				    1, mb_io_ptr->mbfp)) == 1)
			{
			index = 0;
			mb_get_binary_int(MB_YES, &(buffer[index]), &navtime2);
			index += sizeof(int);
			mb_get_binary_float(MB_YES, &(buffer[index]), &lat2);
			index += sizeof(float);
			mb_get_binary_float(MB_YES, &(buffer[index]), &lon2);
			index += sizeof(float);
			mb_get_binary_float(MB_YES, &(buffer[index]), &speed2);
			index += sizeof(float);
			mb_get_binary_float(MB_YES, &(buffer[index]), &course2);
			if (*nav_info_tag == NavInfo6)
		    	    {
			    index += 3 * sizeof(float);
		            mb_get_binary_float(MB_YES, &(buffer[index]), &heading2);
			    }
			else
		    	    {
			    heading2 = course2;
			    }
			lon2 /= 60.;
			lat2 /= 60.;
		        (*n_nav_use)++;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			}
		    else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}

		    }

		/* now interpolate nav */
		factor = ((double)(pingtime - navtime1))
			    / ((double)(navtime2 - navtime1));
		lon = lon1 + factor * (lon2 - lon1);
		lat = lat1 + factor * (lat2 - lat1);
		speed = speed1 + factor * (speed2 - speed1);
		if (course2 - course1 > 180.0)
			{
			course = course1 + factor * (course2 - course1 - 360.0);
			}
		else if (course2 - course1 < -180.0)
			{
			course = course1 + factor * (course2 - course1 + 360.0);
			}
		else
			{
			course = course1 + factor * (course2 - course1);
			}
		if (heading2 - heading1 > 180.0)
			{
			heading = heading1 + factor * (heading2 - heading1 - 360.0);
			}
		else if (heading2 - heading1 < -180.0)
			{
			heading = heading1 + factor * (heading2 - heading1 + 360.0);
			}
		else
			{
			heading = heading1 + factor * (heading2 - heading1);
			}
		}

	    /* now get left channel sonar data */
	    if (status == MB_SUCCESS)
		{
		fseek(mb_io_ptr->mbfp,
		    *left_channel_offset + (*n_read) * (*n_pixel_channel), SEEK_SET);
		if ((status= fread(left_channel, sizeof(unsigned char),
		    *n_pixel_channel, mb_io_ptr->mbfp))
		    == *n_pixel_channel)
		    {
		    status = MB_SUCCESS;
		    *error = MB_ERROR_NO_ERROR;
		    }
		else
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    }
		}

	    /* now get right channel sonar data */
	    if (status == MB_SUCCESS)
		{
		fseek(mb_io_ptr->mbfp,
		    *right_channel_offset + (*n_read) * (*n_pixel_channel), SEEK_SET);
		if ((status= fread(right_channel, sizeof(unsigned char),
		    *n_pixel_channel, mb_io_ptr->mbfp))
		    == *n_pixel_channel)
		    {
		    status = MB_SUCCESS;
		    *error = MB_ERROR_NO_ERROR;
		    }
		else
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    }
		}

	    /* if no altitude do bottom detect */
	    if (status == MB_SUCCESS && altitude <= 0.0)
		{
		ibottom = 0;
		for (i=0;i<*n_pixel_channel && ibottom == 0; i++)
		    {
		    range_tot = range_delay + i * range_per_bin;
		    if (range_tot > MBF_MSTIFF_TRANSMIT_BINS * range_per_bin
			&& right_channel[i] > MBF_MSTIFF_BOTTOM_THRESHOLD
			&& left_channel[i] > MBF_MSTIFF_BOTTOM_THRESHOLD)
			{
			ibottom = i;
			altitude = range_tot;
			}
		    }
		if (altitude != range_tot)
			{
			/* There's either no amplitude data or the bottom, VES */
			/* threshold is too high. Set a default value */
			altitude = range_delay;
			}
		}

	    /* increment reading counter */
	    if (status == MB_SUCCESS)
		(*n_read)++;

	    /* copy to proper storage, doing slant range correction */
	    if (status == MB_SUCCESS)
		{
		data->time_d = *corr_time_d
				+ 0.001 * (pingtime - *ref_windows_time);
		data->lon = lon;
		data->lat = lat;
		data->heading = heading;
		data->course = course;
		data->speed = speed;
		data->altitude = altitude;
		data->slant_range_max = range;
		data->range_delay = range_delay;
		data->sample_interval = range_per_bin;
		data->sonar_depth = 0.0;
		data->frequency = frequency;
		data->pixels_ss = 2 * (*n_pixel_channel);
		istart = (altitude - range_delay) / range_per_bin + 1;

		/* port and starboard channels */
		if (channel_mode == 0)
		    {
		    for (i=0;i<istart; i++)
			{
			j = (*n_pixel_channel) - 1 - i;
			data->ss[j] = left_channel[i];
			data->ssacrosstrack[j] = 0.0;
			j = (*n_pixel_channel) + i;
			data->ss[j] = right_channel[i];
			data->ssacrosstrack[j] = 0.0;
			}
		    for (i=istart;i<*n_pixel_channel; i++)
			{
			range_tot = range_delay + i * range_per_bin;
			j = (*n_pixel_channel) - 1 - i;
			data->ss[j] = left_channel[i];
			data->ssacrosstrack[j]
			    = -sqrt(range_tot * range_tot
				    - altitude * altitude);
			j = (*n_pixel_channel) + i;
			data->ss[j] = right_channel[i];
			data->ssacrosstrack[j]
			    = sqrt(range_tot * range_tot
				    - altitude * altitude);
			}
		    }

		/* port channel only */
		else if (channel_mode == 1)
		    {
		    for (i=0;i<istart; i++)
			{
			j = 2 * (*n_pixel_channel) - 1 - 2 * i;
			data->ss[j] = left_channel[i];
			data->ssacrosstrack[j] = 0.0;
			j = 2 * (*n_pixel_channel) - 2 - 2 * i;
			data->ss[j] = right_channel[i];
			data->ssacrosstrack[j] = 0.0;
			}
		    for (i=istart;i<*n_pixel_channel; i++)
			{
			range_tot = range_delay + (i - 0.5) * range_per_bin;
			j = 2 * (*n_pixel_channel) - 1 - 2 * i;
			data->ss[j] = left_channel[i];
			data->ssacrosstrack[j]
			    = -sqrt(range_tot * range_tot
				    - altitude * altitude);
			range_tot = range_delay + i * range_per_bin;
			j = 2 * (*n_pixel_channel) - 2 - 2 * i;
			data->ss[j] = right_channel[i];
			data->ssacrosstrack[j]
			    = -sqrt(range_tot * range_tot
				    - altitude * altitude);
			}
		    }

		/* starboard channel only */
		else if (channel_mode == 2)
		    {
		    for (i=0;i<istart; i++)
			{
			j = 2 * i;
			data->ss[j] = right_channel[i];
			data->ssacrosstrack[j] = 0.0;
			j = 2 * i + 1;
			data->ss[j] = left_channel[i];
			data->ssacrosstrack[j] = 0.0;
			}
		    for (i=istart;i<*n_pixel_channel; i++)
			{
			range_tot = range_delay + (i - 0.5) * range_per_bin;
			j = 2 * i;
			data->ss[j] = right_channel[i];
			data->ssacrosstrack[j]
			    = sqrt(range_tot * range_tot
				    - altitude * altitude);
			range_tot = range_delay + i * range_per_bin;
			j = 2 * i + 1;
			data->ss[j] = left_channel[i];
			data->ssacrosstrack[j]
			    = sqrt(range_tot * range_tot
				    - altitude * altitude);
			}
		    }
		}
	    }

	/* print debug statements */
	if (status == MB_SUCCESS && verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  New data record read by MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5  Raw values:\n");
		fprintf(stderr,"dbg5       n_ping_file:      %d\n", *n_ping_file);
		fprintf(stderr,"dbg5       bits_per_pixel:   %d\n", *bits_per_pixel);
		fprintf(stderr,"dbg5       n_pixel_channel:  %d\n", *n_pixel_channel);
		fprintf(stderr,"dbg5       n_nav:            %d\n", *n_nav);
		fprintf(stderr,"dbg5       n_nav_use:        %d\n", *n_nav_use);
		fprintf(stderr,"dbg5       corr_time_d:      %f\n", *corr_time_d);
		fprintf(stderr,"dbg5       ref_windows_time: %d\n", *ref_windows_time);
		fprintf(stderr,"dbg5       pingtime:         %d\n", pingtime);
		fprintf(stderr,"dbg5       range_code:       %d\n", range_code);
		fprintf(stderr,"dbg5       channel_mode:     %d\n", channel_mode);
		fprintf(stderr,"dbg5       range_mode:       %d\n", range_mode);
		fprintf(stderr,"dbg5       range:            %f\n", range);
		fprintf(stderr,"dbg5       range_delay_bin:  %d\n", range_delay_bin);
		fprintf(stderr,"dbg5       range_delay:      %f\n", range_delay);
		fprintf(stderr,"dbg5       altitude_bin:     %d\n", altitude_bin);
		fprintf(stderr,"dbg5       altitude:         %f\n", altitude);
		for (i=0;i<*n_pixel_channel;i++)
		    fprintf(stderr,"dbg5       %4d  ss_left: %d  ss_right: %d\n",
			    i, left_channel[i], right_channel[i]);
		fprintf(stderr,"dbg5  Stored data values:\n");
		fprintf(stderr,"dbg5       time:       %f\n",data->time_d);
		fprintf(stderr,"dbg5       lon:        %f\n",data->lon);
		fprintf(stderr,"dbg5       lat:        %f\n",data->lat);
		fprintf(stderr,"dbg5       heading:    %f\n",data->heading);
		fprintf(stderr,"dbg5       speed:      %f\n",data->speed);
		fprintf(stderr,"dbg5       altitude:   %f\n",data->altitude);
		fprintf(stderr,"dbg5       pixels_ss:  %d\n",data->pixels_ss);
		for (i=0;i<data->pixels_ss;i++)
		    fprintf(stderr,"dbg5       ss[%4d]: %d  xtrack:%f\n",
			    i, data->ss[i], data->ssacrosstrack[i]);
		}

	/* set kind and error in mb_io_ptr */
	mb_io_ptr->new_kind = MB_DATA_DATA;
	mb_io_ptr->new_error = *error;

	/* translate values to mstiff data storage structure */
	if (status == MB_SUCCESS
		&& store != NULL)
		{
		/* time stamp */
		store->time_d = data->time_d;

		/* position */
		store->lon = data->lon;
		store->lat = data->lat;

		/* heading and speed */
		store->heading = data->heading;
		store->course = data->course;
		store->speed = data->speed;
		store->altitude = data->altitude;
		store->slant_range_max = data->slant_range_max;
		store->range_delay = data->range_delay;
		store->sample_interval = data->sample_interval;
		store->sonar_depth = data->sonar_depth;
		store->frequency = data->frequency;

		/* sidescan data */
		store->pixels_ss = data->pixels_ss;
		for (i=0;i<data->pixels_ss;i++)
		    {
		    store->ss[i] = data->ss[i];
		    store->ssacrosstrack[i] = data->ssacrosstrack[i];
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
int mbr_wt_mstiffss(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_mstiffss";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mstiffss_struct *data;
	struct mbsys_mstiff_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	data = (struct mbf_mstiffss_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_mstiff_struct *) store_ptr;

	/* set error as this is a read only format */
	status = MB_FAILURE;
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
