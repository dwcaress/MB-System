/*--------------------------------------------------------------------
 *    The MB-system:	mbr_sb2000ss.c	10/14/94
 *	$Id: mbr_sb2000ss.c,v 4.4 1996-04-22 13:21:19 caress Exp $
 *
 *    Copyright (c) 1994 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbr_sb2000ss.c contains the functions for reading and writing
 * multibeam data in the SB2000SS format.  
 * These functions include:
 *   mbr_alm_sb2000ss	- allocate read/write memory
 *   mbr_dem_sb2000ss	- deallocate read/write memory
 *   mbr_rt_sb2000ss	- read and translate data
 *   mbr_wt_sb2000ss	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	October 14, 1994
 * $Log: not supported by cvs2svn $
 * Revision 4.3  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.2  1994/12/21  20:21:09  caress
 * Changes to support high resolution SeaBeam 2000 sidescan files
 * from R/V Melville data.
 *
 * Revision 4.0  1994/10/21  12:34:58  caress
 * Release V4.0
 *
 * Revision 1.1  1994/10/21  12:20:01  caress
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
#include "../../include/mbsys_sb2000.h"
#include "../../include/mbf_sb2000ss.h"

/* include for byte swapping on little-endian machines */
#ifdef BYTESWAPPED
#include "../../include/mb_swap.h"
#endif

/* macro for rounding values to nearest integer */
#define	round(X)	X < 0.0 ? ceil(X - 0.5) : floor(X + 0.5)

/*--------------------------------------------------------------------*/
int mbr_alm_sb2000ss(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
 static char res_id[]="$Id: mbr_sb2000ss.c,v 4.4 1996-04-22 13:21:19 caress Exp $";
	char	*function_name = "mbr_alm_sb2000ss";
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
	mb_io_ptr->structure_size = sizeof(struct mbf_sb2000ss_struct);
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mb_malloc(verbose,sizeof(struct mbsys_sb2000_struct),
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
int mbr_dem_sb2000ss(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	char	*function_name = "mbr_dem_sb2000ss";
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
int mbr_rt_sb2000ss(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_rt_sb2000ss";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sb2000ss_struct *data;
	struct mbsys_sb2000_struct *store;
	char	*headerptr;
	char	*sensorssptr;
	char	*datassptr;
	char	*commentptr;
	unsigned short	*short_ptr;
	int	read_status;
	char	dummy[2];
	int	time_j[5];
	int	i, j, k;
	double	lon, lat;
	int	id;

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
	data = (struct mbf_sb2000ss_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_sb2000_struct *) store_ptr;

	/* get pointers to records */
	headerptr = (char *) &data->year;
	sensorssptr = (char *) &data->ping_number;
	datassptr = (char *) &data->ss[0];
	commentptr = (char *) &data->comment[0];

	/* read next header record from file */
	if ((status = fread(headerptr,1,MBF_SB2000SS_HEADER_SIZE,
			mb_io_ptr->mbfp)) == MBF_SB2000SS_HEADER_SIZE) 
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	if (status == MB_SUCCESS)
		{
		data->year = mb_swap_short(data->year);
		data->day = mb_swap_short(data->day);
		data->min = mb_swap_short(data->min);
		data->sec = mb_swap_short(data->sec);
		data->lat = mb_swap_long(data->lat);
		data->lon = mb_swap_long(data->lon);
		data->heading = mb_swap_short(data->heading);
		data->course = mb_swap_short(data->course);
		data->speed = mb_swap_short(data->speed);
		data->speed_ps = mb_swap_short(data->speed_ps);
		data->quality = mb_swap_short(data->quality);
		data->sensor_size = mb_swap_short(data->sensor_size);
		data->data_size = mb_swap_short(data->data_size);
		}
#endif

	/* print debug statements */
	if (status == MB_SUCCESS && verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  New header record read by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5  New header values:\n");
		fprintf(stderr,"dbg5       year:       %d\n",data->year);
		fprintf(stderr,"dbg5       day:        %d\n",data->day);
		fprintf(stderr,"dbg5       min:        %d\n",data->min);
		fprintf(stderr,"dbg5       sec:        %d\n",data->sec);
		fprintf(stderr,"dbg5       lat:        %d\n",data->lat);
		fprintf(stderr,"dbg5       lon:        %d\n",data->lon);
		fprintf(stderr,"dbg5       heading:    %d\n",data->heading);
		fprintf(stderr,"dbg5       course:     %d\n",data->course);
		fprintf(stderr,"dbg5       speed:      %d\n",data->speed);
		fprintf(stderr,"dbg5       speed_ps:   %d\n",data->speed_ps);
		fprintf(stderr,"dbg5       quality:    %d\n",data->quality);
		fprintf(stderr,"dbg5       sensor size:%d\n",data->sensor_size);
		fprintf(stderr,"dbg5       data size:  %d\n",data->data_size);
		fprintf(stderr,"dbg5       speed_ref:  %c%c\n",
			data->speed_ref[0],data->speed_ref[1]);
		fprintf(stderr,"dbg5       sensor_type:%c%c\n",
			data->sensor_type[0],data->sensor_type[1]);
		fprintf(stderr,"dbg5       data_type:  %c%c\n",
			data->data_type[0],data->data_type[1]);
		}

	/* if not a good header search through file to find one */
	while (status == MB_SUCCESS && 
		(strncmp(data->data_type,"SR",2) != 0
		&& strncmp(data->data_type,"RS",2) != 0
		&& strncmp(data->data_type,"SP",2) != 0
		&& strncmp(data->data_type,"TR",2) != 0
		&& strncmp(data->data_type,"IR",2) != 0
		&& strncmp(data->data_type,"AT",2) != 0
		&& strncmp(data->data_type,"SC",2) != 0))
		{
		/* unswap data if necessary */
#ifdef BYTESWAPPED
		data->year = mb_swap_short(data->year);
		data->day = mb_swap_short(data->day);
		data->min = mb_swap_short(data->min);
		data->sec = mb_swap_short(data->sec);
		data->lat = mb_swap_long(data->lat);
		data->lon = mb_swap_long(data->lon);
		data->heading = mb_swap_short(data->heading);
		data->course = mb_swap_short(data->course);
		data->speed = mb_swap_short(data->speed);
		data->speed_ps = mb_swap_short(data->speed_ps);
		data->quality = mb_swap_short(data->quality);
		data->sensor_size = mb_swap_short(data->sensor_size);
		data->data_size = mb_swap_short(data->data_size);
#endif

		/* shift bytes by one */
		for (i=0;i<MBF_SB2000SS_HEADER_SIZE-1;i++)
			headerptr[i] = headerptr[i+1];

		/* read next byte */
		if ((status = fread(&headerptr[MBF_SB2000SS_HEADER_SIZE-1],
			1,1,mb_io_ptr->mbfp)) == 1) 
			{
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}

		/* swap data if necessary */
#ifdef BYTESWAPPED
		data->year = mb_swap_short(data->year);
		data->day = mb_swap_short(data->day);
		data->min = mb_swap_short(data->min);
		data->sec = mb_swap_short(data->sec);
		data->lat = mb_swap_long(data->lat);
		data->lon = mb_swap_long(data->lon);
		data->heading = mb_swap_short(data->heading);
		data->course = mb_swap_short(data->course);
		data->speed = mb_swap_short(data->speed);
		data->speed_ps = mb_swap_short(data->speed_ps);
		data->quality = mb_swap_short(data->quality);
		data->sensor_size = mb_swap_short(data->sensor_size);
		data->data_size = mb_swap_short(data->data_size);
#endif

		/* print debug statements */
		if (status == MB_SUCCESS && verbose >= 5)
			{
			fprintf(stderr,"\ndbg5  Header record after byte shift in MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg5  New header values:\n");
			fprintf(stderr,"dbg5       year:       %d\n",data->year);
			fprintf(stderr,"dbg5       day:        %d\n",data->day);
			fprintf(stderr,"dbg5       min:        %d\n",data->min);
			fprintf(stderr,"dbg5       sec:        %d\n",data->sec);
			fprintf(stderr,"dbg5       lat:        %d\n",data->lat);
			fprintf(stderr,"dbg5       lon:        %d\n",data->lon);
			fprintf(stderr,"dbg5       heading:    %d\n",data->heading);
			fprintf(stderr,"dbg5       course:     %d\n",data->course);
			fprintf(stderr,"dbg5       speed:      %d\n",data->speed);
			fprintf(stderr,"dbg5       speed_ps:   %d\n",data->speed_ps);
			fprintf(stderr,"dbg5       quality:    %d\n",data->quality);
			fprintf(stderr,"dbg5       sensor size:%d\n",data->sensor_size);
			fprintf(stderr,"dbg5       data size:  %d\n",data->data_size);
			fprintf(stderr,"dbg5       speed_ref:  %c%c\n",
				data->speed_ref[0],data->speed_ref[1]);
			fprintf(stderr,"dbg5       sensor_type:%c%c\n",
				data->sensor_type[0],data->sensor_type[1]);
			fprintf(stderr,"dbg5       data_type:  %c%c\n",
				data->data_type[0],data->data_type[1]);
			}
		}

	/* check for unintelligible records */
	if (status == MB_SUCCESS)
		{
		if ((strncmp(data->sensor_type,"SS",2) != 0 || 
			strncmp(data->data_type,"SC",2) != 0)
			&& strncmp(data->data_type,"TR",2) != 0)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			data->kind = MB_DATA_NONE;

			/* read rest of record into dummy */
			for (i=0;i<data->sensor_size;i++)
				{
				if ((read_status = fread(dummy,1,1,
					mb_io_ptr->mbfp)) != 1) 
					{
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
					}
				}
			for (i=0;i<data->data_size;i++)
				{
				if ((read_status = fread(dummy,1,1,
					mb_io_ptr->mbfp)) != 1) 
					{
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
					}
				}

			}
		else if (strncmp(data->data_type,"SC",2) == 0)
			{
			data->kind = MB_DATA_DATA;
			}
		else
			{
			data->kind = MB_DATA_COMMENT;
			}
		}

	/* fix incorrect header records */
	if (status == MB_SUCCESS 
		&& data->kind == MB_DATA_DATA 
		&& data->data_size == 1000)
		{
		data->sensor_size = 32;
		data->data_size = 1001;
		}

	/* print debug statements */
	if (status == MB_SUCCESS && verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  New header record after correction in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5  New header values:\n");
		fprintf(stderr,"dbg5       year:       %d\n",data->year);
		fprintf(stderr,"dbg5       day:        %d\n",data->day);
		fprintf(stderr,"dbg5       min:        %d\n",data->min);
		fprintf(stderr,"dbg5       sec:        %d\n",data->sec);
		fprintf(stderr,"dbg5       lat:        %d\n",data->lat);
		fprintf(stderr,"dbg5       lon:        %d\n",data->lon);
		fprintf(stderr,"dbg5       heading:    %d\n",data->heading);
		fprintf(stderr,"dbg5       course:     %d\n",data->course);
		fprintf(stderr,"dbg5       speed:      %d\n",data->speed);
		fprintf(stderr,"dbg5       speed_ps:   %d\n",data->speed_ps);
		fprintf(stderr,"dbg5       quality:    %d\n",data->quality);
		fprintf(stderr,"dbg5       sensor size:%d\n",data->sensor_size);
		fprintf(stderr,"dbg5       data size:  %d\n",data->data_size);
		fprintf(stderr,"dbg5       speed_ref:  %c%c\n",
			data->speed_ref[0],data->speed_ref[1]);
		fprintf(stderr,"dbg5       sensor_type:%c%c\n",
			data->sensor_type[0],data->sensor_type[1]);
		fprintf(stderr,"dbg5       data_type:  %c%c\n",
			data->data_type[0],data->data_type[1]);
		}

	/* read sensor record from file */
	if (status == MB_SUCCESS)
		{
		if ((status = fread(sensorssptr,1,data->sensor_size,
			mb_io_ptr->mbfp)) == data->sensor_size) 
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

	/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	if (status == MB_SUCCESS)
		{
		data->ping_number = mb_swap_long(data->ping_number);
		data->ping_length = mb_swap_short(data->ping_length);
		data->pixel_size = mb_swap_short(data->pixel_size);
		data->ss_min = mb_swap_short(data->ss_min);
		data->ss_max = mb_swap_short(data->ss_max);
		data->sample_rate = mb_swap_short(data->sample_rate);
		data->start_time = mb_swap_short(data->start_time);
		data->tot_slice = mb_swap_short(data->tot_slice);
		data->pixels_ss = mb_swap_short(data->pixels_ss);
		}
#endif

	/* fix some files with incorrect sensor records */
	if (status == MB_SUCCESS 
		&& data->kind == MB_DATA_DATA 
		&& data->data_size == 1001)
		{
		data->pixels_ss = 1000;
		}

	/* print debug statements */
	if (status == MB_SUCCESS && verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  New sensor record read by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5  New sensor values:\n");
		fprintf(stderr,"dbg5       ping_number:     %d\n",
			data->ping_number);
		fprintf(stderr,"dbg5       ping_length:     %d\n",
			data->ping_length);
		fprintf(stderr,"dbg5       pixel_size:      %d\n",
			data->pixel_size);
		fprintf(stderr,"dbg5       ss_min:          %d\n",
			data->ss_min);
		fprintf(stderr,"dbg5       ss_max:          %d\n",
			data->ss_max);
		fprintf(stderr,"dbg5       sample_rate:     %d\n",
			data->sample_rate);
		fprintf(stderr,"dbg5       start_time:      %d\n",
			data->start_time);
		fprintf(stderr,"dbg5       tot_slice:       %d\n",
			data->tot_slice);
		fprintf(stderr,"dbg5       pixels_ss:       %d\n",
			data->pixels_ss);
		fprintf(stderr,"dbg5       spare_ss:        ");
		for (i=0;i<12;i++)
			fprintf(stderr,"%c", data->spare_ss[i]);
		fprintf(stderr, "\n");
		}

	/* read data record from file */
	if (status == MB_SUCCESS && data->kind == MB_DATA_DATA)
		{
		if ((status = fread(datassptr,1,data->data_size,
			mb_io_ptr->mbfp)) == data->data_size) 
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

	/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	if (status == MB_SUCCESS && data->kind == MB_DATA_DATA)
		{
		if (data->ss[0] == 'R');
			{
			for (i=1;i<=data->pixels_ss;i++)
				{
				short_ptr = &data->ss[4+i*2];
				*short_ptr = mb_swap_short(*short_ptr);
				}
			}
		}
#endif

	/* correct data size if needed */
	if (status == MB_SUCCESS 
		&& data->kind == MB_DATA_DATA 
		&& data->data_size == 1001)
		{
		data->data_size = 1004;
		data->ss[1001] = 'G';
		data->ss[1002] = 'G';
		data->ss[1003] = 'G';
		}

	/* print debug statements */
	if (status == MB_SUCCESS && verbose >= 5 
		&& data->kind == MB_DATA_DATA)
		{
		fprintf(stderr,"\ndbg5  New data record read by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5  New data values:\n");
		fprintf(stderr,"dbg5       sidescan_type:%c\n",
			data->ss[0]);
		if (data->ss[0] == 'G')
			{
			for (i=1;i<=data->pixels_ss;i++)
				fprintf(stderr,"dbg5       pixel: %d  ss: %d\n",
					i,data->ss[i]);
			}
		else if (data->ss[0] == 'R')
			{
			for (i=1;i<=data->pixels_ss;i++)
				{
				short_ptr = (unsigned short *) &data->ss[4+i*2];
				fprintf(stderr,"dbg5       pixel: %d  ss: %d\n",
					i,*short_ptr);
				}
			}
		}

	/* read comment record from file */
	if (status == MB_SUCCESS && data->kind == MB_DATA_COMMENT)
		{
		if ((status = fread(commentptr,1,data->data_size,
			mb_io_ptr->mbfp)) == data->data_size) 
			{
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			for (i=data->data_size;i<MBF_SB2000SS_COMMENT_LENGTH;i++)
				commentptr[i] = '\0';
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}

	/* print debug statements */
	if (status == MB_SUCCESS && verbose >= 5 
		&& data->kind == MB_DATA_COMMENT)
		{
		fprintf(stderr,"\ndbg5  New comment record read by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5  New comment:\n");
		fprintf(stderr,"dbg5       comment:   %s\n",
			data->comment);
		}

	/* set kind and error in mb_io_ptr */
	mb_io_ptr->new_kind = data->kind;
	mb_io_ptr->new_error = *error;

	/* translate values to current ping variables 
		in mbio descriptor structure */
	if (status == MB_SUCCESS 
		&& data->kind == MB_DATA_DATA)
		{
		/* get time */
		time_j[0] = data->year;
		time_j[1] = data->day;
		time_j[2] = data->min;
		time_j[3] = 0.01*data->sec;
		time_j[4] = 10000*(data->sec - 100*time_j[3]);
		mb_get_itime(verbose,time_j,mb_io_ptr->new_time_i);
		mb_get_time(verbose,mb_io_ptr->new_time_i,
			&(mb_io_ptr->new_time_d));

		/* get navigation */
		mb_io_ptr->new_lon = 0.0000001*data->lon;
		mb_io_ptr->new_lat = 0.0000001*data->lat;
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
		mb_io_ptr->new_heading = 0.1*data->heading;

		/* get speed */
		mb_io_ptr->new_speed = 0.00185*data->speed;

		/* set no bathymetry */
		mb_io_ptr->beams_bath = 0;
		mb_io_ptr->beams_amp = 0;
		mb_io_ptr->pixels_ss = data->pixels_ss;
		if (data->ss[0] == 'G')
			{
			for (i=0;i<data->pixels_ss;i++)
				{
				mb_io_ptr->new_ss[i] = data->ss[i+1];
				mb_io_ptr->new_ss_acrosstrack[i] = 
					data->pixel_size*(i-data->pixels_ss/2);
				mb_io_ptr->new_ss_alongtrack[i] = 0.0;
				}
			}
		else if (data->ss[0] == 'R')
			{
			for (i=0;i<data->pixels_ss;i++)
				{
				short_ptr = (unsigned short *) &data->ss[4+2*i];
				mb_io_ptr->new_ss[i] = *short_ptr;
				mb_io_ptr->new_ss_acrosstrack[i] = 
					data->pixel_size*(i-data->pixels_ss/2);
				mb_io_ptr->new_ss_alongtrack[i] = 0.0;
				}
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
	else if (status == MB_SUCCESS 
		&& data->kind == MB_DATA_COMMENT)
		{
		/* copy comment */
		strncpy(mb_io_ptr->new_comment,data->comment,MBF_SB2000SS_COMMENT_LENGTH);

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

	/* translate values to seabeam data storage structure */
	if (status == MB_SUCCESS
		&& store != NULL)
		{
		/* type of data record */
		store->kind = data->kind;

		/* position */
		store->lon = data->lon;
		store->lat = data->lat;

		/* time stamp */
		store->year = data->year;
		store->day = data->day;
		store->min = data->min;
		store->sec = data->sec;

		/* heading and speed */
		store->heading = data->heading;
		store->course = data->course;
		store->speed = data->speed;
		store->speed_ps = data->speed_ps;
		store->quality = data->quality;
		store->sensor_size = data->sensor_size;
		store->data_size = data->data_size;
		store->speed_ref[0] = data->speed_ref[0];
		store->speed_ref[1] = data->speed_ref[1];
		store->sensor_type[0] = data->sensor_type[0];
		store->sensor_type[1] = data->sensor_type[1];
		store->data_type[0] = data->data_type[0];
		store->data_type[1] = data->data_type[1];

		/* additional values */
		store->pitch= 0;
		store->roll = 0;
		store->gain = 0;
		store->correction = 0;
		store->surface_vel = 0;
		store->pulse_width = 0;
		store->attenuation = 0;
		store->spare1 = 0;
		store->spare2 = 0;
		store->mode[0] = '\0';
		store->mode[1] = '\0';
		store->data_correction[0] = '\0';
		store->data_correction[1] = '\0';
		store->ssv_source[0] = '\0';
		store->ssv_source[1] = '\0';
		
		/* svp */
		store->svp_mean = 0;
		store->svp_number = 0;
		store->svp_spare = 0;
		for (i=0;i<30;i++)
			{
			store->svp_depth[i] = 0;
			store->svp_vel[i] = 0;
			}
		store->vru1 = 0;
		store->vru1_port = 0;
		store->vru1_forward = 0;
		store->vru1_vert = 0;
		store->vru2 = 0;
		store->vru2_port = 0;
		store->vru2_forward = 0;
		store->vru2_vert = 0;
		store->pitch_bias = 0;
		store->roll_bias = 0;
		for (i=0;i<8;i++)
			{
			store->vru[i] = '\0';
			}

		/* depths and distances */
		store->beams_bath = 0;
		store->scale_factor = 0;
		
		/* sidescan */
		store->ping_number = data->ping_number;
		store->ping_length = data->ping_length;
		store->pixel_size = data->pixel_size;
		store->ss_min = data->ss_min;
		store->ss_max = data->ss_max;
		store->sample_rate = data->sample_rate;
		store->start_time = data->start_time;
		store->tot_slice = data->tot_slice;
		store->pixels_ss = data->pixels_ss;
		for (i=0;i<12;i++)
			store->spare_ss[i] = data->spare_ss[i];
		store->ss_type = data->ss[0];
		if (store->ss_type == 'R')
			{
			for (i=0;i<2*data->pixels_ss;i++)
				store->ss[i] = data->ss[i+4];
			}
		else
			{
			for (i=0;i<data->pixels_ss;i++)
				store->ss[i] = data->ss[i+1];
			}

		/* comment */
		strncpy(store->comment,mb_io_ptr->new_comment,
			MBSYS_SB2000_COMMENT_LENGTH);
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
int mbr_wt_sb2000ss(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_wt_sb2000ss";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sb2000ss_struct *data;
	struct mbsys_sb2000_struct *store;
	char	*headerptr;
	char	*sensorssptr;
	char	*datassptr;
	char	*commentptr;
	unsigned short *short_ptr;
	int	time_j[5];
	double	lon, lat;
	int	id;
	int	set_pixel_size;
	int	header_size, sensor_size, data_size;
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
	data = (struct mbf_sb2000ss_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_sb2000_struct *) store_ptr;

	/* get pointers to records */
	headerptr = (char *) &data->year;
	sensorssptr = (char *) &data->ping_number;
	datassptr = (char *) &data->ss[0];
	commentptr = (char *) &data->comment[0];

	/* first set some plausible amounts for some of the 
		variables in the SB2000SS record */
	data->year = 0;
	data->day = 0;
	data->min = 0;
	data->sec = 0;
	data->lat = 0;
	data->lon = 0;
	data->heading = 0;
	data->course = 0;
	data->speed = 0;
	data->speed_ps = 0;
	data->quality = 0;
	data->sensor_size = 0;
	data->data_size = 0;
	data->speed_ref[0] = 0;
	data->speed_ref[1] = 0;
	if (mb_io_ptr->new_kind == MB_DATA_DATA)
		{
		data->sensor_type[0] = 'S';
		data->sensor_type[1] = 'S';
		data->data_type[0] = 'S';
		data->data_type[1] = 'C';
		}
	else
		{
		data->sensor_type[0] = 0;
		data->sensor_type[1] = 0;
		data->data_type[0] = 'T';
		data->data_type[1] = 'R';
		}
	data->ping_number = 0;
	data->ping_length = 0;
	data->pixel_size = 0;
	data->ss_min = 0;
	data->ss_max = 0;
	data->sample_rate = 0;
	data->start_time = 0;
	data->tot_slice = 0;
	data->pixels_ss = 0;
	data->ss[0] = 'G';
	
	/* second translate values from seabeam 2000 data storage structure */
	if (store != NULL)
		{
		data->kind = store->kind;
		if (store->kind == MB_DATA_DATA)
			{
			data->sensor_type[0] = 'S';
			data->sensor_type[1] = 'S';
			data->data_type[0] = 'S';
			data->data_type[1] = 'C';
			}
		else
			{
			data->sensor_type[0] = 0;
			data->sensor_type[1] = 0;
			data->data_type[0] = 'T';
			data->data_type[1] = 'R';
			}

		/* position */
		data->lon = store->lon;
		data->lat = store->lat;

		/* time stamp */
		data->year = store->year;
		data->day = store->day;
		data->min = store->min;
		data->sec = store->sec;

		/* heading and speed */
		data->heading = store->heading;
		data->course = store->course;
		data->speed = store->speed;
		data->speed_ps = store->speed_ps;
		data->quality = store->quality;
		data->sensor_size = store->sensor_size;
		data->data_size = store->data_size;
		data->speed_ref[0] = store->speed_ref[0];
		data->speed_ref[1] = store->speed_ref[1];
		data->sensor_type[0] = store->sensor_type[0];
		data->sensor_type[1] = store->sensor_type[1];
		data->data_type[0] = store->data_type[0];
		data->data_type[1] = store->data_type[1];

		if (store->kind == MB_DATA_DATA)
			{
			data->ping_number = store->ping_number;
			data->ping_length = store->ping_length;
			data->pixel_size = store->pixel_size;
			data->ss_min = store->ss_min;
			data->ss_max = store->ss_max;
			data->sample_rate = store->sample_rate;
			data->start_time = store->start_time;
			data->pixels_ss = store->pixels_ss;
			for (i=0;i<12;i++)
				data->spare_ss[i] = store->spare_ss[i];
			if (store->ss_type == 'R')
				{
				data->ss[0] = 'R';
				data->ss[1] = 'R';
				data->ss[2] = 'R';
				data->ss[3] = 'R';
				for (i=0;i<2*data->pixels_ss;i++)
					data->ss[i+4] = store->ss[i];
				}
			else if (store->ss_type == 'G')
				{
				data->ss[0] = store->ss_type;
				data->ss[1001] = store->ss_type;
				data->ss[1002] = store->ss_type;
				data->ss[1003] = store->ss_type;
				for (i=0;i<data->pixels_ss;i++)
					data->ss[i+1] = store->ss[i];
				}
			}

		/* comment */
		else if (store->kind == MB_DATA_COMMENT)
			{
			strncpy(commentptr,store->comment,
				MBF_SB2000SS_COMMENT_LENGTH-1);
			data->data_size = strlen(commentptr);
			data->sensor_size = 0;
			}
		}

	/* set kind from current ping */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR)
		data->kind = mb_io_ptr->new_kind;

	/* check for comment */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_COMMENT)
		{
		strncpy(commentptr,mb_io_ptr->new_comment,
			MBF_SB2000SS_COMMENT_LENGTH-1);
		data->data_size = strlen(commentptr);
		data->sensor_size = 0;
		}

	/* else translate current ping data to sb2000ss data structure */
	else if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_DATA)
		{
		/* get time */
		mb_get_jtime(verbose,mb_io_ptr->new_time_i,time_j);
		data->year = time_j[0];
		data->day = time_j[1];
		data->min = time_j[2];
		data->sec = 100*time_j[3] + 0.0001*time_j[4];

		/* get navigation */
		data->lon = 10000000*mb_io_ptr->new_lon;
		data->lat = 10000000*mb_io_ptr->new_lat;

		/* get heading */
		data->heading = 10*mb_io_ptr->new_heading;

		/* get speed */
		data->speed = 540.5405405*mb_io_ptr->new_speed;

		/* put sidescan values 
			into sb2000ss data structure */
		if (store != NULL)
			set_pixel_size = MB_NO;
		else
			{
			set_pixel_size = MB_YES;
			if (data->pixels_ss == 1000)
				data->ss[0] = 'G';
			else
				data->ss[0] = 'R';
			}
		if (data->ss[0] == 'G')
			{
			for (i=0;i<data->pixels_ss;i++)
				{
				data->ss[i+1] = mb_io_ptr->new_ss[i];
				if (set_pixel_size == MB_YES
					&& mb_io_ptr->new_ss_acrosstrack[i] > 0)
					{
					data->pixel_size = 
						mb_io_ptr->new_ss_acrosstrack[i]/
						(i - data->pixels_ss/2);
					set_pixel_size = MB_NO;
					}
				}
			}
		else if (data->ss[0] == 'R')
			{
			for (i=0;i<data->pixels_ss;i++)
				{
				short_ptr = (unsigned short *) &data->ss[4+2*i];
				*short_ptr = (unsigned short) mb_io_ptr->new_ss[i];
				if (set_pixel_size == MB_YES
					&& mb_io_ptr->new_ss_acrosstrack[i] > 0)
					{
					data->pixel_size = 
						mb_io_ptr->new_ss_acrosstrack[i]/
						(i - data->pixels_ss/2);
					set_pixel_size = MB_NO;
					}
				}
			}
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Ready to write data in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       kind:       %d\n",
			data->kind);
		fprintf(stderr,"dbg5       error:      %d\n",*error);
		fprintf(stderr,"dbg5       status:     %d\n",status);
		}

	/* print debug statements */
	if (status == MB_SUCCESS && verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Header record to be written by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5  Header values:\n");
		fprintf(stderr,"dbg5       year:       %d\n",data->year);
		fprintf(stderr,"dbg5       day:        %d\n",data->day);
		fprintf(stderr,"dbg5       min:        %d\n",data->min);
		fprintf(stderr,"dbg5       sec:        %d\n",data->sec);
		fprintf(stderr,"dbg5       lat:        %d\n",data->lat);
		fprintf(stderr,"dbg5       lon:        %d\n",data->lon);
		fprintf(stderr,"dbg5       heading:    %d\n",data->heading);
		fprintf(stderr,"dbg5       course:     %d\n",data->course);
		fprintf(stderr,"dbg5       speed:      %d\n",data->speed);
		fprintf(stderr,"dbg5       speed_ps:   %d\n",data->speed_ps);
		fprintf(stderr,"dbg5       quality:    %d\n",data->quality);
		fprintf(stderr,"dbg5       sensor size:%d\n",data->sensor_size);
		fprintf(stderr,"dbg5       data size:  %d\n",data->data_size);
		fprintf(stderr,"dbg5       speed_ref:  %c%c\n",
			data->speed_ref[0],data->speed_ref[1]);
		fprintf(stderr,"dbg5       sensor_type:%c%c\n",
			data->sensor_type[0],data->sensor_type[1]);
		fprintf(stderr,"dbg5       data_type:  %c%c\n",
			data->data_type[0],data->data_type[1]);
		}

	/* print debug statements */
	if (status == MB_SUCCESS && verbose >= 5 
		&& data->kind == MB_DATA_DATA)
		{
		fprintf(stderr,"\ndbg5  Sensor record to be written by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5  Sensor values:\n");
		fprintf(stderr,"dbg5       ping_number:     %d\n",
			data->ping_number);
		fprintf(stderr,"dbg5       ping_length:     %d\n",
			data->ping_length);
		fprintf(stderr,"dbg5       pixel_size:      %d\n",
			data->pixel_size);
		fprintf(stderr,"dbg5       ss_min:          %d\n",
			data->ss_min);
		fprintf(stderr,"dbg5       ss_max:          %d\n",
			data->ss_max);
		fprintf(stderr,"dbg5       sample_rate:     %d\n",
			data->sample_rate);
		fprintf(stderr,"dbg5       start_time:      %d\n",
			data->start_time);
		fprintf(stderr,"dbg5       tot_slice:       %d\n",
			data->tot_slice);
		fprintf(stderr,"dbg5       pixels_ss:       %d\n",
			data->pixels_ss);
		fprintf(stderr,"dbg5       spare_ss:        ");
		for (i=0;i<12;i++)
			fprintf(stderr,"%c", data->spare_ss[i]);
		fprintf(stderr, "\n");
		}

	/* print debug statements */
	if (status == MB_SUCCESS && verbose >= 5 
		&& data->kind == MB_DATA_DATA)
		{
		fprintf(stderr,"\ndbg5  Data record to be written by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5  Data values:\n");
		fprintf(stderr,"dbg5       sidescan_type:%d\n",
			data->ss[0]);
		if (data->ss[0] == 'G')
			{
			for (i=1;i<=data->pixels_ss;i++)
				fprintf(stderr,"dbg5       pixel: %d  ss: %d\n",
					i,data->ss[i]);
			}
		else if (data->ss[0] == 'R');
			{
			for (i=1;i<=data->pixels_ss;i++)
				{
				short_ptr = (unsigned short *) &data->ss[4+i*2];
				fprintf(stderr,"dbg5       pixel: %d  ss: %d\n",
					i,*short_ptr);
				}
			}
		}

	/* print debug statements */
	if (status == MB_SUCCESS && verbose >= 5 
		&& data->kind == MB_DATA_COMMENT)
		{
		fprintf(stderr,"\ndbg5  Comment record to be written by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5  Comment:\n");
		fprintf(stderr,"dbg5       comment:   %s\n",
			data->comment);
		}

	/* get sizes of writes */
	header_size = MBF_SB2000SS_HEADER_SIZE;
	sensor_size = data->sensor_size;
	data_size = data->data_size;

	/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	data->year = mb_swap_short(data->year);
	data->day = mb_swap_short(data->day);
	data->min = mb_swap_short(data->min);
	data->sec = mb_swap_short(data->sec);
	data->lat = mb_swap_long(data->lat);
	data->lon = mb_swap_long(data->lon);
	data->heading = mb_swap_short(data->heading);
	data->course = mb_swap_short(data->course);
	data->speed = mb_swap_short(data->speed);
	data->speed_ps = mb_swap_short(data->speed_ps);
	data->quality = mb_swap_short(data->quality);
	data->sensor_size = mb_swap_short(data->sensor_size);
	data->data_size = mb_swap_short(data->data_size);
	if (data->kind == MB_DATA_DATA)
		{
		if (data->ss[0] == 'R');
			{
			for (i=1;i<=data->pixels_ss;i++)
				{
				short_ptr = &data->ss[4+i*2];
				*short_ptr = mb_swap_short(*short_ptr);
				}
			}
		data->ping_number = mb_swap_long(data->ping_number);
		data->ping_length = mb_swap_short(data->ping_length);
		data->pixel_size = mb_swap_short(data->pixel_size);
		data->ss_min = mb_swap_short(data->ss_min);
		data->ss_max = mb_swap_short(data->ss_max);
		data->sample_rate = mb_swap_short(data->sample_rate);
		data->start_time = mb_swap_short(data->start_time);
		data->pixels_ss = mb_swap_short(data->pixels_ss);
		}
#endif

	/* write header record to file */
	if (status == MB_SUCCESS)
		{
		if ((status = fwrite(headerptr,1,header_size,
			mb_io_ptr->mbfp)) == header_size) 
			{
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		}

	/* write sensor record to file */
	if (status == MB_SUCCESS && data->kind == MB_DATA_DATA)
		{
		if ((status = fwrite(sensorssptr,1,sensor_size,
			mb_io_ptr->mbfp)) == sensor_size) 
			{
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		}

	/* write data record to file */
	if (status == MB_SUCCESS && data->kind == MB_DATA_DATA)
		{
		if ((status = fwrite(datassptr,1,data_size,
			mb_io_ptr->mbfp)) == data_size) 
			{
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		}

	/* write comment record to file */
	if (status == MB_SUCCESS && data->kind == MB_DATA_COMMENT)
		{
		if ((status = fwrite(commentptr,1,data_size,
			mb_io_ptr->mbfp)) == data_size) 
			{
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
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
