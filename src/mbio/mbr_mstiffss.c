/*--------------------------------------------------------------------
 *    The MB-system:	mbr_mstiffss.c	4/7/98
 *	$Id: mbr_mstiffss.c,v 4.1 1999-03-31 18:11:35 caress Exp $
 *
 *    Copyright (c) 1998 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
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
 * $Log: not supported by cvs2svn $
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
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_io.h"
#include "../../include/mb_define.h"
#include "../../include/mbsys_mstiff.h"
#include "../../include/mbf_mstiffss.h"

/* include for byte swapping on little-endian machines */
#ifndef BYTESWAPPED
#include "../../include/mb_swap.h"
#endif

/*--------------------------------------------------------------------*/
int mbr_alm_mstiffss(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
 static char res_id[]="$Id: mbr_mstiffss.c,v 4.1 1999-03-31 18:11:35 caress Exp $";
	char	*function_name = "mbr_alm_mstiffss";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int	*n_read;
	int	*n_nav_use;

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
int mbr_dem_mstiffss(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	char	*function_name = "mbr_dem_mstiffss";
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
int mbr_rt_mstiffss(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_rt_mstiffss";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mstiffss_struct *data;
	struct mbsys_mstiff_struct *store;
	char    mstiff_id[4];
	int	ifd_offset;
	int	nentry;
	short	tag;
	short	type;
	int	count;
	int	value_offset;
	int	*n_read;
	int	*bits_per_pixel;
	int	*n_ping_file;
	int	*n_pixel_channel;
	int	*time_corr_offset;
	int	*left_channel_offset;
	int	*right_channel_offset;
	int	*sonar_data_info_offset;
	int	*n_nav;
	int	*n_nav_use;
	int	*nav_info_offset;
	short	corr_time[9];
	int	*ref_windows_time;
	int	corr_time_i[7];
	double	*corr_time_d;
	int	pingtime;
	short   range_code;
	short   range_delay_bin;
	short   altitude_bin;
	int	range_mode;
	int	channel_mode;
	double	range_per_bin;
	double  range;
	double  range_delay;
	double  altitude;
	int	navtime1, navtime2;
	float	lon1, lon2;
	float	lat1, lat2;
	float	speed1, speed2;
	float	course1, course2;
	double	lon, lat, speed, heading;
	double	factor;
	double	course;
	int	idummy[10];
	short	sdummy[1];
	double	range_tot;
	unsigned char left_channel[MBF_MSTIFFSS_PIXELS/2];
	unsigned char right_channel[MBF_MSTIFFSS_PIXELS/2];
	int	ibottom;
	int	istart;
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
	time_corr_offset = &(mb_io_ptr->save3);
	left_channel_offset = &(mb_io_ptr->save4);
	right_channel_offset = &(mb_io_ptr->save5);
	sonar_data_info_offset = &(mb_io_ptr->save6);
	n_nav = &(mb_io_ptr->save7);
	n_nav_use = &(mb_io_ptr->save8);
	nav_info_offset = &(mb_io_ptr->save9);
	ref_windows_time = &(mb_io_ptr->save10);
	corr_time_d = &(mb_io_ptr->saved1);
	
	/* if first time through, read file header and
	    offsets, setting up for later reads */
	if (*n_read <= 0)
	    {
	    /* check for proper file tag */
	    if ((status = fread(mstiff_id, sizeof(char), 
			    4, mb_io_ptr->mbfp)) != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}
	    else if (strncmp(mstiff_id, "MSTL", 4) != 0)
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
	    if ((status = fread(&ifd_offset, 
			    sizeof(int), 1, mb_io_ptr->mbfp)) == 1)
		{
#ifndef BYTESWAPPED
		ifd_offset = mb_swap_int(ifd_offset);
#endif
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
		fseek(mb_io_ptr->mbfp, ifd_offset, SEEK_SET);
		if ((status = fread(&nentry, sizeof(short), 
			1, mb_io_ptr->mbfp)) == 1)
		    {
		    /* loop over all entries in the directory */
#ifndef BYTESWAPPED
		    nentry = mb_swap_short(nentry);
#endif
		    for (i=0;i<nentry && status == MB_SUCCESS;i++)
			{
			/*read entry */
			status = fread(&tag, sizeof(short), 1, mb_io_ptr->mbfp);
			status = fread(&type, sizeof(short), 1, mb_io_ptr->mbfp);
			status = fread(&count, sizeof(int), 1, mb_io_ptr->mbfp);
			if ((status = fread(&value_offset, 
					sizeof(int), 1, mb_io_ptr->mbfp))
					== 1)
			    {
			    status = MB_SUCCESS;
			    *error = MB_ERROR_NO_ERROR;
			    }
			else
			    {
			    status = MB_FAILURE;
			    *error = MB_ERROR_EOF;		    				
			    }
			if (status == MB_SUCCESS)
			    {
#ifndef BYTESWAPPED
			    tag = mb_swap_short(tag);
			    type = mb_swap_short(type);
			    count = mb_swap_int(count);
			    value_offset = mb_swap_int(value_offset);
#endif

			    /* set values for important entries */
			    if (tag == 258)
				{
				*bits_per_pixel = value_offset;
				}
			    else if (tag == 259)
				{
				*n_ping_file = value_offset;
				}
			    else if (tag == 260)
				{
				*n_pixel_channel = value_offset;
				}
			    else if (tag == 262)
				{
				*time_corr_offset = value_offset;
				}
			    else if (tag == 263)
				{
				*left_channel_offset = value_offset;
				}
			    else if (tag == 264)
				{
				*right_channel_offset = value_offset;
				}
			    else if (tag == 265)
				{
				*sonar_data_info_offset = value_offset;
				}
			    else if (tag == 266)
				{
				*n_nav = value_offset;
				}
			    else if (tag == 267)
				{
				*nav_info_offset = value_offset;
				}
			    }
			}
			
		    /* get time correlation values */
		    fseek(mb_io_ptr->mbfp, *time_corr_offset, SEEK_SET);
		    fread(ref_windows_time, sizeof(int), 1, mb_io_ptr->mbfp);
		    if ((status = fread(corr_time, sizeof(short), 
			    9, mb_io_ptr->mbfp)) == 9)
			{
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			}
		    else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		    if (status == MB_SUCCESS)
			{
#ifndef BYTESWAPPED
			*ref_windows_time = mb_swap_int(*ref_windows_time);
			for (i=0;i<9;i++)
			    {
			    corr_time[i] = mb_swap_short(corr_time[i]);
			    }
#endif
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
	    fseek(mb_io_ptr->mbfp, 
		    *sonar_data_info_offset 
			+ (*n_read) * (sizeof(int) + 3 * sizeof(short)), 
		    SEEK_SET);
	    status = fread(&pingtime, sizeof(int), 
			    1, mb_io_ptr->mbfp);
	    status = fread(&range_code, sizeof(short), 
			    1, mb_io_ptr->mbfp);
	    status = fread(&range_delay_bin, sizeof(short),  
			    1, mb_io_ptr->mbfp);
	    if ((status = fread(&altitude_bin, sizeof(short),  
			    1, mb_io_ptr->mbfp)) == 1)
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
	    else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
		
	    /* make sense of sonar data info */
	    if (status == MB_SUCCESS)
		{
#ifndef BYTESWAPPED
		pingtime = mb_swap_int(pingtime);
		range_code = mb_swap_short(range_code);
		range_delay_bin = mb_swap_short(range_delay_bin);
		altitude_bin = mb_swap_short(altitude_bin);
#endif
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
		}
		
	    /* now get navigation */
	    if (status == MB_SUCCESS
		&& *n_nav_use < *n_nav)
		{
		/* read first two nav points starting with last
		   nav used */
		fseek(mb_io_ptr->mbfp, 
		    *nav_info_offset + (*n_nav_use) 
			* (sizeof(short) + 11 * sizeof(int) 
			    + 4 * sizeof(float)), 
		    SEEK_SET);
		fread(&navtime1, sizeof(int), 1, mb_io_ptr->mbfp);
		fread(&lat1, sizeof(float), 1, mb_io_ptr->mbfp);
		fread(&lon1, sizeof(float), 1, mb_io_ptr->mbfp);
		fread(&speed1, sizeof(float), 1, mb_io_ptr->mbfp);
		fread(&course1, sizeof(float), 1, mb_io_ptr->mbfp);
		fread(idummy, sizeof(int), 10, mb_io_ptr->mbfp);
		fread(sdummy, sizeof(short), 1, mb_io_ptr->mbfp);
		fread(&navtime2, sizeof(int), 1, mb_io_ptr->mbfp);
		fread(&lat2, sizeof(float), 1, mb_io_ptr->mbfp);
		fread(&lon2, sizeof(float), 1, mb_io_ptr->mbfp);
		fread(&speed2, sizeof(float), 1, mb_io_ptr->mbfp);
		fread(&course2, sizeof(float), 1, mb_io_ptr->mbfp);
		fread(idummy, sizeof(int), 10, mb_io_ptr->mbfp);
		fread(sdummy, sizeof(short), 1, mb_io_ptr->mbfp);
#ifndef BYTESWAPPED
		navtime1 = mb_swap_int(navtime1);
		mb_swap_float(&lat1);
		mb_swap_float(&lon1);
		mb_swap_float(&speed1);
		mb_swap_float(&course1);
		navtime2 = mb_swap_int(navtime2);
		mb_swap_float(&lat2);
		mb_swap_float(&lon2);
		mb_swap_float(&speed2);
		mb_swap_float(&course2);
#endif
		lon1 /= 60.;
		lat1 /= 60.;
		lon2 /= 60.;
		lat2 /= 60.;
		if (course1 > 359 && course2 <360)
		    course1 = course2;
		else if (course1 > 359)
		    course1 = 0;
		if (course2 > 359)
		    course2 = course1;

		/* if first two nav points don't bracket ping
		   in time keep reading until two found that
		   do or end of nav reached */
		while (pingtime > navtime2 && *n_nav_use < *n_nav - 2)
		    {
		    navtime1 = navtime2;
		    lat1 = lat2;
		    lon1 = lon2;
		    speed1 = speed2;
		    course1 = course2;
		    fread(&navtime2, sizeof(int), 1, mb_io_ptr->mbfp);
		    fread(&lat2, sizeof(float), 1, mb_io_ptr->mbfp);
		    fread(&lon2, sizeof(float), 1, mb_io_ptr->mbfp);
		    fread(&speed2, sizeof(float), 1, mb_io_ptr->mbfp);
		    fread(&course2, sizeof(float), 1, mb_io_ptr->mbfp);
		    fread(idummy, sizeof(int), 10, mb_io_ptr->mbfp);
		    fread(sdummy, sizeof(short), 1, mb_io_ptr->mbfp);
#ifndef BYTESWAPPED
		    navtime2 = mb_swap_int(navtime2);
		    mb_swap_float(&lat2);
		    mb_swap_float(&lon2);
		    mb_swap_float(&speed2);
		    mb_swap_float(&course2);
#endif
		    lon2 /= 60.;
		    lat2 /= 60.;
		    if (course2 > 359)
			course2 = course1;
		    (*n_nav_use)++;
		    }
		    
		/* now interpolate nav */
		factor = ((double)(pingtime - navtime1)) 
			    / ((double)(navtime2 - navtime1));
		lon = lon1 + factor * (lon2 - lon1);
		lat = lat1 + factor * (lat2 - lat1);
		speed = speed1 + factor * (speed2 - speed1);
		if (factor <= 0.5)
		    course = course1;
		else
		    course = course2;
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
		data->heading = course;
		data->speed = speed;
		data->altitude = altitude;
		data->pixels_ss = 2 * (*n_pixel_channel);
		istart = (altitude - range_delay) / range_per_bin;
		for (i=0;i<istart; i++)
		    {
		    j = (*n_pixel_channel) - 1 - i;
		    data->ss[j] = left_channel[i] & 63;
		    data->ssacrosstrack[j] = 0.0;
		    j = (*n_pixel_channel) + i;
		    data->ss[j] = right_channel[i] & 63;
		    data->ssacrosstrack[j] = 0.0;
		    }
		for (i=istart;i<*n_pixel_channel; i++)
		    {
		    range_tot = range_delay + i * range_per_bin;
		    j = (*n_pixel_channel) - 1 - i;
		    data->ss[j] = left_channel[i] & 63;
		    data->ssacrosstrack[j] 
			= -sqrt(range_tot * range_tot
				- altitude * altitude);
		    j = (*n_pixel_channel) + i;
		    data->ss[j] = right_channel[i] & 63;
		    data->ssacrosstrack[j] 
			= sqrt(range_tot * range_tot
				- altitude * altitude);
		    }
		}
	    }

	/* print debug statements */
	if (status == MB_SUCCESS && verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  New data record read by MBIO function <%s>\n",
			function_name);
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

	/* translate values to current ping variables 
		in mbio descriptor structure */
	if (status == MB_SUCCESS)
		{
		/* get time */
		mb_io_ptr->new_time_d = data->time_d;
		mb_get_date(verbose, mb_io_ptr->new_time_d, mb_io_ptr->new_time_i);

		/* get navigation */
		mb_io_ptr->new_lon = data->lon;
		mb_io_ptr->new_lat = data->lat;
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

		/* set no bathymetry */
		mb_io_ptr->beams_bath = 0;
		mb_io_ptr->beams_amp = 0;
		mb_io_ptr->pixels_ss = data->pixels_ss;
		for (i=0;i<data->pixels_ss;i++)
		    {
		    mb_io_ptr->new_ss[i] = data->ss[i];
		    mb_io_ptr->new_ss_acrosstrack[i] = 
			    data->ssacrosstrack[i];
		    mb_io_ptr->new_ss_alongtrack[i] = 0.0;
		    }

		/* print debug statements */
		if (verbose >= 5)
		    {
		    fprintf(stderr,"\ndbg5  New ping read by MBIO function <%s>\n",
			    function_name);
		    fprintf(stderr,"dbg5  New ping values:\n");
		    fprintf(stderr,"dbg5       error:      %d\n",
			    mb_io_ptr->new_error);
		    fprintf(stderr,"dbg5       time_i[0]:  %d\n",
			    mb_io_ptr->new_time_i[0]);
		    fprintf(stderr,"dbg5       time_i[1]:  %d\n",
			    mb_io_ptr->new_time_i[1]);
		    fprintf(stderr,"dbg5       time_i[2]:  %d\n",
			    mb_io_ptr->new_time_i[2]);
		    fprintf(stderr,"dbg5       time_i[3]:  %d\n",
			    mb_io_ptr->new_time_i[3]);
		    fprintf(stderr,"dbg5       time_i[4]:  %d\n",
			    mb_io_ptr->new_time_i[4]);
		    fprintf(stderr,"dbg5       time_i[5]:  %d\n",
			    mb_io_ptr->new_time_i[5]);
		    fprintf(stderr,"dbg5       time_i[6]:  %d\n",
			    mb_io_ptr->new_time_i[6]);
		    fprintf(stderr,"dbg5       time_d:     %f\n",
			    mb_io_ptr->new_time_d);
		    fprintf(stderr,"dbg5       longitude:  %f\n",
			    mb_io_ptr->new_lon);
		    fprintf(stderr,"dbg5       latitude:   %f\n",
			    mb_io_ptr->new_lat);
		    fprintf(stderr,"dbg5       speed:      %f\n",
			    mb_io_ptr->new_speed);
		    fprintf(stderr,"dbg5       heading:    %f\n",
			    mb_io_ptr->new_heading);
		    fprintf(stderr,"dbg5       pixels_ss:  %d\n",
			    mb_io_ptr->pixels_ss);
		    for (i=0;i<mb_io_ptr->pixels_ss;i++)
		      fprintf(stderr,"dbg5       ss[%d]: %f  ssdist[%d]: %f\n",
			    i,mb_io_ptr->new_ss[i],
			    i,mb_io_ptr->new_ss_acrosstrack[i]);
		    }

		/* done translating values */

		}

	/* translate values to seabeam data storage structure */
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
		store->speed = data->speed;
		store->altitude = data->altitude;
		
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
int mbr_wt_mstiffss(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_wt_mstiffss";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mstiffss_struct *data;
	struct mbsys_mstiff_struct *store;
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
