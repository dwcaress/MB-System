/*--------------------------------------------------------------------
 *    The MB-system:	mbr_mbldeoih.c	3.00	2/2/93
 *	$Id: mbr_mbldeoih.c,v 3.0 1993-05-14 22:56:57 sohara Exp $
 *
 *    Copyright (c) 1993 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbr_mbldeoih.c contains the functions for reading and writing
 * multibeam data in the MBLDEOIH format.  
 * These functions include:
 *   mbr_alm_mbldeoih	- allocate read/write memory
 *   mbr_dem_mbldeoih	- deallocate read/write memory
 *   mbr_rt_mbldeoih	- read and translate data
 *   mbr_wt_mbldeoih	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	February 2, 1993
 * $Log: not supported by cvs2svn $
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <strings.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_io.h"
#include "../../include/mbf_mbldeoih.h"
#include "../../include/mbsys_ldeoih.h"

/*--------------------------------------------------------------------*/
int mbr_alm_mbldeoih(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
 static char res_id[]="$Id: mbr_mbldeoih.c,v 3.0 1993-05-14 22:56:57 sohara Exp $";
	char	*function_name = "mbr_alm_mbldeoih";
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
	mb_io_ptr->structure_size = sizeof(struct mbf_mbldeoih_struct);
	mb_io_ptr->header_structure_size = 
		sizeof(struct mbf_mbldeoih_header_struct);
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mbsys_ldeoih_alloc(verbose,mbio_ptr,
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
int mbr_dem_mbldeoih(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	char	*function_name = "mbr_dem_mbldeoih";
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
	status = mb_free(verbose,mb_io_ptr->raw_data,error);
	status = mbsys_ldeoih_deall(verbose,mbio_ptr,
		mb_io_ptr->store_data,error);

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
int mbr_rt_mbldeoih(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_rt_mbldeoih";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mbldeoih_struct *dataplus;
	struct mbf_mbldeoih_header_struct *header;
	struct mbf_mbldeoih_data_struct *data;
	struct mbsys_ldeoih_struct *store;
	char	*comment;
	short int	*bath;
	short int	*bathdist;
	short int	*back;
	short int	*backdist;
	int	read_size;
	int	time_j[4];
	double	bathscale;
	double	backscale;
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

	/* get pointer to mbio descriptor and data structure */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	store = (struct mbsys_ldeoih_struct *) store_ptr;

	/* get pointer to raw data structure */
	dataplus = (struct mbf_mbldeoih_struct *) mb_io_ptr->raw_data;
	header = &(dataplus->header);
	data = &(dataplus->data);
	comment = dataplus->comment;
	bath = data->bath;
	bathdist = data->bathdist;
	back = data->back;
	backdist = data->backdist;

	/* read next header from file */
	if ((status = fread(header,1,mb_io_ptr->header_structure_size,
			mb_io_ptr->mbfp)) == mb_io_ptr->header_structure_size) 
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* check for comment or unintelligible records */
	if (status == MB_SUCCESS)
		{
		if (header->flag == 8995)
			{
			dataplus->kind = MB_DATA_COMMENT;
			}
		else if (header->flag == 25700)
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

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  New header read in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       flag:       %d\n",header->flag);
		fprintf(stderr,"dbg5       year:       %d\n",header->year);
		fprintf(stderr,"dbg5       day:        %d\n",header->day);
		fprintf(stderr,"dbg5       minute:     %d\n",header->min);
		fprintf(stderr,"dbg5       second:     %d\n",header->sec);
		fprintf(stderr,"dbg5       lonu:       %d\n",header->lon2u);
		fprintf(stderr,"dbg5       lonb:       %d\n",header->lon2b);
		fprintf(stderr,"dbg5       latu:       %d\n",header->lat2u);
		fprintf(stderr,"dbg5       latb:       %d\n",header->lat2b);
		fprintf(stderr,"dbg5       heading:    %d\n",header->heading);
		fprintf(stderr,"dbg5       speed:      %d\n",header->speed);
		fprintf(stderr,"dbg5       beams bath: %d\n",
			header->beams_bath);
		fprintf(stderr,"dbg5       beams back: %d\n",
			header->beams_back);
		fprintf(stderr,"dbg5       bath scale: %d\n",header->bathscale);
		fprintf(stderr,"dbg5       back scale: %d\n",header->backscale);
		fprintf(stderr,"dbg5       status:     %d\n",status);
		fprintf(stderr,"dbg5       error:      %d\n",*error);
		}

	/* read next chunk of the data */
	if (status == MB_SUCCESS 
		&& dataplus->kind == MB_DATA_COMMENT)
		{
		read_size = 128;
		if ((status = fread(comment,1,read_size,mb_io_ptr->mbfp))
			== read_size) 
			{
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}

		/* print debug messages */
		if (verbose >= 5 && status == MB_SUCCESS)
			{
			fprintf(stderr,"\ndbg5  New comment read in function <%s>\n",
				function_name);
			fprintf(stderr,"dbg5       comment: %s\n",comment);
			}
		}
	else if (status == MB_SUCCESS 
		&& dataplus->kind == MB_DATA_DATA)
		{
		/* if needed reset numbers of beams */
		if (header->beams_bath != mb_io_ptr->beams_bath)
			mb_io_ptr->beams_bath = header->beams_bath;
		if (header->beams_back != mb_io_ptr->beams_back)
			mb_io_ptr->beams_back = header->beams_back;

		/* read bathymetry */
		read_size = sizeof(short int)*header->beams_bath;
		status = fread(bath,1,read_size,mb_io_ptr->mbfp);
		status = fread(bathdist,1,read_size,mb_io_ptr->mbfp);

		/* read backscatter */
		read_size = sizeof(short int)*header->beams_back;
		status = fread(back,1,read_size,mb_io_ptr->mbfp);
		status = fread(backdist,1,read_size,mb_io_ptr->mbfp);

		/* check for end of file */
		if (status == read_size) 
			{
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}

		/* print debug messages */
		if (verbose >= 5 && status == MB_SUCCESS)
			{
			fprintf(stderr,"\ndbg5  New data read in function <%s>\n",
				function_name);
			fprintf(stderr,"dbg5       beams_bath: %d\n",
				header->beams_bath);
			for (i=0;i<header->beams_bath;i++)
			  fprintf(stderr,"dbg5       bath[%d]: %d  bathdist[%d]: %d\n",
				i,bath[i],i,bathdist[i]);
			fprintf(stderr,"dbg5       beams_back: %d\n",
				header->beams_back);
			for (i=0;i<header->beams_back;i++)
			  fprintf(stderr,"dbg5       back[%d]: %d  backdist[%d]: %d\n",
				i,back[i],i,backdist[i]);
			}
		}

	/* translate values to current ping variables 
		in mbio descriptor structure */
	if (status == MB_SUCCESS 
		&& dataplus->kind == MB_DATA_DATA)
		{
		/* get time */
		time_j[0] = header->year;
		time_j[1] = header->day;
		time_j[2] = header->min;
		time_j[3] = header->sec;
		mb_get_itime(verbose,time_j,mb_io_ptr->new_time_i);
		mb_get_time(verbose,mb_io_ptr->new_time_i,
			&(mb_io_ptr->new_time_d));

		/* get navigation */
		mb_io_ptr->new_lon = header->lon2u/60. 
			+ header->lon2b/600000.;
		mb_io_ptr->new_lat = header->lat2u/60. 
			+ header->lat2b/600000. - 90.;
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

		/* get heading (360 degrees = 65536) */
		mb_io_ptr->new_heading = header->heading*0.0054932;

		/* get speed */
		mb_io_ptr->new_speed = 0.01*header->speed;

		/* get scales */
		bathscale = 0.001*header->bathscale;
		backscale = 0.001*header->backscale;

		/* read distance and depth values into storage arrays */
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			mb_io_ptr->new_bath[i] = 
				(short int) bathscale*bath[i];
			mb_io_ptr->new_bathdist[i] = 
				(short int) bathscale*bathdist[i];
			}

		/* read distance and backscatter values into storage arrays */
		for (i=0;i<mb_io_ptr->beams_back;i++)
			{
			mb_io_ptr->new_back[i] = 
				(short int) backscale*back[i];
			mb_io_ptr->new_backdist[i] = 
				(short int) backscale*backdist[i];
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
			  fprintf(stderr,"dbg4       bath[%d]: %d  bathdist[%d]: %d\n",
				i,mb_io_ptr->new_bath[i],
				i,mb_io_ptr->new_bathdist[i]);
			fprintf(stderr,"dbg4       beams_back: %d\n",
				mb_io_ptr->beams_back);
			for (i=0;i<mb_io_ptr->beams_back;i++)
			  fprintf(stderr,"dbg4       back[%d]: %d  backdist[%d]: %d\n",
				i,mb_io_ptr->new_back[i],
				i,mb_io_ptr->new_backdist[i]);
			}

		/* done translating values */

		}
	else if (status == MB_SUCCESS 
		&& dataplus->kind == MB_DATA_COMMENT)
		{
		/* copy comment */
		strncpy(mb_io_ptr->new_comment,comment,128);

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

		/* position */
		store->lon2u = header->lon2u;
		store->lon2b = header->lon2b;
		store->lat2u = header->lat2u;
		store->lat2b = header->lat2b;

		/* time stamp */
		store->year = header->year;
		store->day = header->day;
		store->min = header->min;
		store->sec = header->sec;

		/* heading and speed */
		store->heading = header->heading;
		store->speed = header->speed;

		/* numbers of beams and scaling */
		store->beams_bath = header->beams_bath;
		store->beams_back = header->beams_back;
		store->bathscale = header->bathscale;
		store->backscale = header->backscale;

		/* depths and backscatter */
		for (i=0;i<store->beams_bath;i++)
			{
			store->bath[i] = data->bath[i];
			store->bathdist[i] = data->bathdist[i];
			}
		for (i=0;i<store->beams_back;i++)
			{
			store->back[i] = data->back[i];
			store->backdist[i] = data->backdist[i];
			}

		/* comment */
		strcpy(store->comment,comment);
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
int mbr_wt_mbldeoih(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_wt_mbldeoih";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_mbldeoih_struct *dataplus;
	struct mbf_mbldeoih_header_struct *header;
	struct mbf_mbldeoih_data_struct *data;
	struct mbsys_ldeoih_struct *store;
	char	*comment;
	short int	*bath;
	short int	*bathdist;
	short int	*back;
	short int	*backdist;
	int	write_size;
	int	time_j[4];
	double	lon;
	double	lat;
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

	/* get pointer to mbio descriptor and data storage */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	store = (struct mbsys_ldeoih_struct *) store_ptr;

	/* get pointer to raw data structure */
	dataplus = (struct mbf_mbldeoih_struct *) mb_io_ptr->raw_data;
	header = &(dataplus->header);
	data = &(dataplus->data);
	comment = dataplus->comment;
	bath = data->bath;
	bathdist = data->bathdist;
	back = data->back;
	backdist = data->backdist;

	/* first translate values from data storage structure */
	if (store != NULL)
		{
		/* type of data record */
		dataplus->kind = store->kind;

		/* position */
		header->lon2u = store->lon2u;
		header->lon2b = store->lon2b;
		header->lat2u = store->lat2u;
		header->lat2b = store->lat2b;

		/* time stamp */
		header->year = store->year;
		header->day = store->day;
		header->min = store->min;
		header->sec = store->sec;

		/* heading and speed */
		header->heading = store->heading;
		header->speed = store->speed;

		/* numbers of beams and scaling */
		header->beams_bath = store->beams_bath;
		header->beams_back = store->beams_back;
		header->bathscale = store->bathscale;
		header->backscale = store->backscale;

		/* depths and backscatter */
		for (i=0;i<header->beams_bath;i++)
			{
			bath[i] = store->bath[i];
			bathdist[i] = store->bathdist[i];
			}
		for (i=0;i<header->beams_back;i++)
			{
			back[i] = store->back[i];
			backdist[i] = store->backdist[i];
			}

		/* comment */
		strcpy(comment,store->comment);
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
		strncpy(comment,mb_io_ptr->new_comment,127);
		}

	/* else translate current ping data to mbldeoih data structure */
	else if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_DATA)
		{
		dataplus->kind = MB_DATA_DATA;

		/* get time */
		mb_get_jtime(verbose,mb_io_ptr->new_time_i,time_j);
		header->year = time_j[0];
		header->day = time_j[1];
		header->min = time_j[2];
		header->sec = time_j[3];

		/* get navigation */
		lon = mb_io_ptr->new_lon;
		if (lon < 0.0) lon = lon + 360.0;
		header->lon2u = (short int) 60.0*lon;
		header->lon2b 
			= (short int) (600000.0*(lon - header->lon2u/60.0));
		lat = mb_io_ptr->new_lat + 90.0;
		header->lat2u = (short int) 60.0*lat;
		header->lat2b 
			= (short int) (600000.0*(lat - header->lat2u/60.0));

		/* get heading (360 degrees = 65536) */
		header->heading = 182.044444*mb_io_ptr->new_heading;

		/* get speed */
		header->speed = 100.*mb_io_ptr->new_speed;

		/* set numbers of bathymetry and backscatter beams */
		header->beams_bath = mb_io_ptr->beams_bath;
		header->beams_back = mb_io_ptr->beams_back;

		/* set bathymetry and backscatter scalings */
		header->bathscale = 1000;
		header->backscale = 1000;

		/* put distance and depth values 
			into mbldeoih data structure */
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			bath[i] = mb_io_ptr->new_bath[i];
			bathdist[i] = mb_io_ptr->new_bathdist[i];
			}

		/* put distance and backscatter values 
			into mbldeoih data structure */
		for (i=0;i<mb_io_ptr->beams_back;i++)
			{
			back[i] = mb_io_ptr->new_back[i];
			backdist[i] = mb_io_ptr->new_backdist[i];
			}
		}

	/* set data flag 
		(data: flag='dd'=25700 or comment:flag='##'=8995) */
	if (dataplus->kind == MB_DATA_DATA)
		header->flag = 25700;
	else
		header->flag = 8995;


	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  New header set in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       flag:       %d\n",header->flag);
		fprintf(stderr,"dbg5       year:       %d\n",header->year);
		fprintf(stderr,"dbg5       day:        %d\n",header->day);
		fprintf(stderr,"dbg5       minute:     %d\n",header->min);
		fprintf(stderr,"dbg5       second:     %d\n",header->sec);
		fprintf(stderr,"dbg5       lonu:       %d\n",header->lon2u);
		fprintf(stderr,"dbg5       lonb:       %d\n",header->lon2b);
		fprintf(stderr,"dbg5       latu:       %d\n",header->lat2u);
		fprintf(stderr,"dbg5       latb:       %d\n",header->lat2b);
		fprintf(stderr,"dbg5       heading:    %d\n",header->heading);
		fprintf(stderr,"dbg5       speed:      %d\n",header->speed);
		fprintf(stderr,"dbg5       beams bath: %d\n",
			header->beams_bath);
		fprintf(stderr,"dbg5       beams back: %d\n",
			header->beams_back);
		fprintf(stderr,"dbg5       bath scale: %d\n",header->bathscale);
		fprintf(stderr,"dbg5       back scale: %d\n",header->backscale);
		fprintf(stderr,"dbg5       status:     %d\n",status);
		fprintf(stderr,"dbg5       error:      %d\n",*error);
		}

	/* write next header to file */
	if ((status = fwrite(header,1,mb_io_ptr->header_structure_size,
			mb_io_ptr->mbfp)) == mb_io_ptr->header_structure_size) 
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Going to write data in function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       kind:       %d\n",dataplus->kind);
		fprintf(stderr,"dbg5       status:     %d\n",status);
		fprintf(stderr,"dbg5       error:      %d\n",*error);
		}
	if (verbose >=5 && dataplus->kind == MB_DATA_COMMENT)
		{
		fprintf(stderr,"dbg5       comment:    %s\n",comment);
		}
	if (verbose >=5 && dataplus->kind == MB_DATA_DATA)
		{
		fprintf(stderr,"dbg5       beams_bath: %d\n",header->beams_bath);
		for (i=0;i<header->beams_bath;i++)
			fprintf(stderr,"dbg5       bath[%d]: %d  bathdist[%d]: %d\n",
				i,bath[i],i,bathdist[i]);
		fprintf(stderr,"dbg5       beams_back: %d\n",header->beams_back);
		for (i=0;i<header->beams_back;i++)
			fprintf(stderr,"dbg5       back[%d]: %d  backdist[%d]: %d\n",
				i,back[i],i,backdist[i]);
		}

	/* write next chunk of the data */
	if (dataplus->kind == MB_DATA_COMMENT)
		{
		write_size = 128;
		if ((status = fwrite(comment,1,write_size,mb_io_ptr->mbfp))
			== write_size) 
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
	else if (dataplus->kind == MB_DATA_DATA)
		{

		/* write bathymetry */
		write_size = sizeof(short int)*header->beams_bath;
		status = fwrite(bath,1,write_size,mb_io_ptr->mbfp);
		status = fwrite(bathdist,1,write_size,mb_io_ptr->mbfp);

		/* write backscatter */
		write_size = sizeof(short int)*header->beams_back;
		status = fwrite(back,1,write_size,mb_io_ptr->mbfp);
		status = fwrite(backdist,1,write_size,mb_io_ptr->mbfp);

		/* check for error */
		if (status == write_size) 
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
