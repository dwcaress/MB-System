/*--------------------------------------------------------------------
 *    The MB-system:	mbr_sbsioswb.c	9/18/93
 *	$Id: mbr_sbsioswb.c,v 1.1 1994-10-21 12:20:01 caress Exp $
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
 * mbr_sbsioswb.c contains the functions for reading and writing
 * multibeam data in the SBSIOSWB format.  
 * These functions include:
 *   mbr_alm_sbsioswb	- allocate read/write memory
 *   mbr_dem_sbsioswb	- deallocate read/write memory
 *   mbr_rt_sbsioswb	- read and translate data
 *   mbr_wt_sbsioswb	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	February 2, 1993
 * $Log: not supported by cvs2svn $
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <strings.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_io.h"
#include "../../include/mbsys_sb.h"
#include "../../include/mbf_sbsioswb.h"

/* include for byte swapping on little-endian machines */
#ifdef BYTESWAPPED
#include "../../include/mb_swap.h"
#endif

/* macro for rounding values to nearest integer */
#define	round(X)	X < 0.0 ? ceil(X - 0.5) : floor(X + 0.5)

/*--------------------------------------------------------------------*/
int mbr_alm_sbsioswb(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
 static char res_id[]="$Id: mbr_sbsioswb.c,v 1.1 1994-10-21 12:20:01 caress Exp $";
	char	*function_name = "mbr_alm_sbsioswb";
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
	mb_io_ptr->structure_size = sizeof(struct mbf_sbsioswb_struct);
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mb_malloc(verbose,sizeof(struct mbsys_sb_struct),
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
int mbr_dem_sbsioswb(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	char	*function_name = "mbr_dem_sbsioswb";
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
int mbr_rt_sbsioswb(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_rt_sbsioswb";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sbsioswb_struct *data;
	struct mbsys_sb_struct *store;
	char	*headerptr;
	char	*sensorptr;
	char	*datarecptr;
	char	*commentptr;
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
	data = (struct mbf_sbsioswb_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_sb_struct *) store_ptr;

	/* get pointers to records */
	headerptr = (char *) &data->year;
	sensorptr = (char *) &data->eclipse_time;
	datarecptr = (char *) &data->beams_bath;
	commentptr = (char *) &data->comment[0];

	/* read next header record from file */
	if ((status = fread(headerptr,1,MB_SBSIOSWB_HEADER_SIZE,
			mb_io_ptr->mbfp)) == MB_SBSIOSWB_HEADER_SIZE) 
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
		for (i=0;i<MB_SBSIOSWB_HEADER_SIZE-1;i++)
			headerptr[i] = headerptr[i+1];

		/* read next byte */
		if ((status = fread(&headerptr[MB_SBSIOSWB_HEADER_SIZE-1],
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
		if ((strncmp(data->sensor_type,"SB",2) != 0 || 
			strncmp(data->data_type,"SR",2) != 0)
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
		else if (strncmp(data->data_type,"SR",2) == 0)
			{
			data->kind = MB_DATA_DATA;
			}
		else
			{
			data->kind = MB_DATA_COMMENT;
			}
		}

	/* read sensor record from file */
	if (status == MB_SUCCESS)
		{
		if ((status = fread(sensorptr,1,data->sensor_size,
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
		data->eclipse_time = mb_swap_short(data->eclipse_time);
		data->eclipse_heading = mb_swap_short(data->eclipse_heading);
		}
#endif

	/* print debug statements */
	if (status == MB_SUCCESS && verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  New sensor record read by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5  New sensor values:\n");
		fprintf(stderr,"dbg5       eclipse_time:    %d\n",
			data->eclipse_time);
		fprintf(stderr,"dbg5       eclipse_heading: %d\n",
			data->eclipse_heading);
		}

	/* read data record from file */
	if (status == MB_SUCCESS && data->kind == MB_DATA_DATA)
		{
		if ((status = fread(datarecptr,1,data->data_size,
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
		data->beams_bath = mb_swap_short(data->beams_bath);
		data->scale_factor = mb_swap_short(data->scale_factor);
		for (i=0;i<data->beams_bath;i++)
			{
			data->bath_struct[i].bath = 
				mb_swap_short(data->bath_struct[i].bath);
			data->bath_struct[i].bath_acrosstrack = 
				mb_swap_short(data->bath_struct[i].bath_acrosstrack);
			}
		}
#endif

	/* print debug statements */
	if (status == MB_SUCCESS && verbose >= 5 
		&& data->kind == MB_DATA_DATA)
		{
		fprintf(stderr,"\ndbg5  New data record read by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5  New data values:\n");
		fprintf(stderr,"dbg5       beams_bath:   %d\n",
			data->beams_bath);
		fprintf(stderr,"dbg5       scale_factor: %d\n",
			data->scale_factor);
		for (i=0;i<data->beams_bath;i++)
			fprintf(stderr,"dbg5       beam: %d  bath: %d  across_track: %d\n",
				i,data->bath_struct[i].bath,
				data->bath_struct[i].bath_acrosstrack);
		}

	/* read comment record from file */
	if (status == MB_SUCCESS && data->kind == MB_DATA_COMMENT)
		{
		if ((status = fread(commentptr,1,data->data_size,
			mb_io_ptr->mbfp)) == data->data_size) 
			{
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			for (i=data->data_size;i<MB_SBSIOSWB_COMMENT_LENGTH;i++)
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
		mb_io_ptr->new_speed = 0.185*data->speed;

		/* read distance and depth values into storage arrays */
		for (i=0;i<MB_BEAMS_SBSIOSWB;i++)
			{
			mb_io_ptr->new_bath[i] = data->bath_struct[i].bath;
			mb_io_ptr->new_bath_acrosstrack[i] = 
				data->bath_struct[i].bath_acrosstrack;
			mb_io_ptr->new_bath_alongtrack[i] = 0.0;
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
			fprintf(stderr,"dbg5       beams_bath: %d\n",
				mb_io_ptr->beams_bath);
			for (i=0;i<mb_io_ptr->beams_bath;i++)
			  fprintf(stderr,"dbg5       bath[%d]: %f  bathdist[%d]: %f\n",
				i,mb_io_ptr->new_bath[i],
				i,mb_io_ptr->new_bath_acrosstrack[i]);
			}

		/* done translating values */

		}
	else if (status == MB_SUCCESS 
		&& data->kind == MB_DATA_COMMENT)
		{
		/* copy comment */
		strncpy(mb_io_ptr->new_comment,data->comment,MB_SBSIOSWB_COMMENT_LENGTH-1);

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
		lon = 0.0000001*data->lon;
		if (lon < 0.0) lon = lon + 360.0;
		store->lon2u = (short int) 60.0*lon;
		store->lon2b = (short int) round(600000.0*
			(lon - store->lon2u/60.0));
		lat = 0.0000001*data->lat + 90.0;
		store->lat2u = (short int) 60.0*lat;
		store->lat2b = (short int) round(600000.0*
			(lat - store->lat2u/60.0));

		/* time stamp */
		store->year = data->year;
		store->day = data->day;
		store->min = data->min;
		store->sec = 0.01*data->sec;

		/* depths and distances */
		id = data->beams_bath - 1;
		for (i=0;i<data->beams_bath;i++)
			{
			store->deph[id-i] = data->bath_struct[i].bath;
			store->dist[id-i] = 
				data->bath_struct[i].bath_acrosstrack;
			}

		/* additional values */
		store->sbtim = data->eclipse_time;
		store->sbhdg = data->eclipse_heading;
		store->axis = 0;
		store->major = 0;
		store->minor = 0;

		/* comment */
		strncpy(store->comment,mb_io_ptr->new_comment,
			MBSYS_SB_MAXLINE);
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
int mbr_wt_sbsioswb(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_wt_sbsioswb";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sbsioswb_struct *data;
	struct mbsys_sb_struct *store;
	char	*headerptr;
	char	*sensorptr;
	char	*datarecptr;
	char	*commentptr;
	int	time_j[5];
	double	lon, lat;
	int	id;
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
	data = (struct mbf_sbsioswb_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_sb_struct *) store_ptr;

	/* get pointers to records */
	headerptr = (char *) &data->year;
	sensorptr = (char *) &data->eclipse_time;
	datarecptr = (char *) &data->beams_bath;
	commentptr = (char *) &data->comment[0];

	/* first set some plausible amounts for some of the 
		variables in the SBSIOSWB record */
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
		data->sensor_type[1] = 'B';
		data->data_type[0] = 'S';
		data->data_type[1] = 'R';
		}
	else
		{
		data->sensor_type[0] = 0;
		data->sensor_type[1] = 0;
		data->data_type[0] = 'T';
		data->data_type[1] = 'R';
		}
	data->eclipse_time = 0;
	data->eclipse_heading = 0;
	data->beams_bath = MB_BEAMS_SBSIOSWB;
	data->sensor_size = 4;
	data->data_size = 4 + 4*data->beams_bath;
	data->scale_factor = 100;
	for (i=0;i<MB_BEAMS_SBSIOSWB;i++)
		{
		data->bath_struct[i].bath = 0;
		data->bath_struct[i].bath_acrosstrack = 0;
		}

	/* second translate values from seabeam data storage structure */
	if (store != NULL)
		{
		data->kind = store->kind;
		if (store->kind == MB_DATA_DATA)
			{
			data->sensor_type[0] = 'S';
			data->sensor_type[1] = 'B';
			data->data_type[0] = 'S';
			data->data_type[1] = 'R';
			}
		else
			{
			data->sensor_type[0] = 0;
			data->sensor_type[1] = 0;
			data->data_type[0] = 'T';
			data->data_type[1] = 'R';
			}

		/* position */
		lon = 10000000*(store->lon2u/60. 
			+ store->lon2b/600000.);
		if (lon > 1800000000.)
			lon = lon - 3600000000.;
		lat = 10000000*(store->lat2u/60. 
			+ store->lat2b/600000. - 90.);
		data->lon = lon;
		data->lat = lat;

		/* time stamp */
		data->year = store->year;
		data->day = store->day;
		data->min = store->min;
		data->sec = 100*store->sec;

		/* additional values */
		data->eclipse_time = store->sbtim;
		data->eclipse_heading = store->sbhdg;

		if (store->kind == MB_DATA_DATA)
			{
			/* put distance and depth values 
				into sbsioswb data structure */
			id = data->beams_bath - 1;
			for (i=0;i<MB_BEAMS_SBSIOSWB;i++)
				{
				data->bath_struct[id-i].bath = store->deph[i];;
				data->bath_struct[id-i].bath_acrosstrack = 
					store->dist[i];;
				}
			}

		/* comment */
		else if (store->kind == MB_DATA_COMMENT)
			{
			strncpy(commentptr,store->comment,
				MB_SBSIOSWB_COMMENT_LENGTH-1);
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
			MB_SBSIOSWB_COMMENT_LENGTH-1);
		data->data_size = strlen(commentptr);
		data->sensor_size = 0;
		}

	/* else translate current ping data to sbsioswb data structure */
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
		data->speed = 5.405405405*mb_io_ptr->new_speed;

		/* put distance and depth values 
			into sbsioswb data structure */

		/* initialize depth and distance in output structure */
		for (i=0;i<MB_BEAMS_SBSIOSWB;i++)
			{
			data->bath_struct[i].bath = mb_io_ptr->new_bath[i];;
			data->bath_struct[i].bath_acrosstrack = 
				mb_io_ptr->new_bath_acrosstrack[i];;
			}
		}

	/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	if (data->kind == MB_DATA_DATA)
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
		data->eclipse_time = mb_swap_short(data->eclipse_time);
		data->eclipse_heading = mb_swap_short(data->eclipse_heading);
		data->beams_bath = mb_swap_short(data->beams_bath);
		data->scale_factor = mb_swap_short(data->scale_factor);
		for (i=0;i<data->beams_bath;i++)
			{
			data->bath_struct[i].bath = 
				mb_swap_short(data->bath_struct[i].bath);
			data->bath_struct[i].bath_acrosstrack = 
				mb_swap_short(data->bath_struct[i].bath_acrosstrack);
			}
		}
#endif

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
		fprintf(stderr,"dbg5       eclipse_time:    %d\n",
			data->eclipse_time);
		fprintf(stderr,"dbg5       eclipse_heading: %d\n",
			data->eclipse_heading);
		}

	/* print debug statements */
	if (status == MB_SUCCESS && verbose >= 5 
		&& data->kind == MB_DATA_DATA)
		{
		fprintf(stderr,"\ndbg5  Data record to be written by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5  Data values:\n");
		fprintf(stderr,"dbg5       beams_bath:   %d\n",
			data->beams_bath);
		fprintf(stderr,"dbg5       scale_factor: %d\n",
			data->scale_factor);
		for (i=0;i<data->beams_bath;i++)
			fprintf(stderr,"dbg5       beam: %d  bath: %d  across_track: %d\n",
				i,data->bath_struct[i].bath,
				data->bath_struct[i].bath_acrosstrack);
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

	/* write header record to file */
	if (status == MB_SUCCESS)
		{
		if ((status = fwrite(headerptr,1,MB_SBSIOSWB_HEADER_SIZE,
			mb_io_ptr->mbfp)) == MB_SBSIOSWB_HEADER_SIZE) 
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
	if (status == MB_SUCCESS)
		{
		if ((status = fwrite(sensorptr,1,data->sensor_size,
			mb_io_ptr->mbfp)) == data->sensor_size) 
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
		if ((status = fwrite(datarecptr,1,data->data_size,
			mb_io_ptr->mbfp)) == data->data_size) 
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
		if ((status = fwrite(commentptr,1,strlen(data->comment),
			mb_io_ptr->mbfp)) == strlen(data->comment)) 
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
