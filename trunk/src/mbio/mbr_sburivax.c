/*--------------------------------------------------------------------
 *    The MB-system:	mbr_sburivax.c	2/2/93
 *	$Id: mbr_sburivax.c,v 4.4 1996-04-22 13:21:19 caress Exp $
 *
 *    Copyright (c) 1993, 1994 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbr_sburivax.c contains the functions for reading and writing
 * multibeam data in the SBURIVAX format. The only difference between
 * the SBURIVAX format and the SBURICEN format is that the data files
 * are stored in VAX byte order ("little endian"). This is why the
 * format structure definitions are taken from the include file 
 * mbf_sburicen.h.
 * These functions include:
 *   mbr_alm_sburivax	- allocate read/write memory
 *   mbr_dem_sburivax	- deallocate read/write memory
 *   mbr_rt_sburivax	- read and translate data
 *   mbr_wt_sburivax	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	February 2, 1993
 * $Log: not supported by cvs2svn $
 * Revision 4.3  1995/03/22  18:58:09  caress
 * Added handling of extra 2 bytes which occurs every 5 records in
 * data derived from VAX VMS.
 *
 * Revision 4.2  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.1  1994/10/21  12:20:01  caress
 * Release V4.0
 *
 * Revision 4.0  1994/07/29  18:59:33  caress
 * Initial Revision.
 *
 * Revision 1.1  1994/07/29  18:46:51  caress
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
#include "../../include/mbsys_sb.h"
#include "../../include/mbf_sburicen.h"

/* include for byte swapping on little-endian machines */
#ifndef BYTESWAPPED
#include "../../include/mb_swap.h"
#endif

/*--------------------------------------------------------------------*/
int mbr_alm_sburivax(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
 static char res_id[]="$Id: mbr_sburivax.c,v 4.4 1996-04-22 13:21:19 caress Exp $";
	char	*function_name = "mbr_alm_sburivax";
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
	mb_io_ptr->structure_size = sizeof(struct mbf_sburicen_struct);
	mb_io_ptr->data_structure_size = 
		sizeof(struct mbf_sburicen_data_struct);
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mb_malloc(verbose,sizeof(struct mbsys_sb_struct),
				&mb_io_ptr->store_data,error);

	/* set record counters to zero */
	mb_io_ptr->irecord_count = 0;
	mb_io_ptr->orecord_count = 0;

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
int mbr_dem_sburivax(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	char	*function_name = "mbr_dem_sburivax";
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
int mbr_rt_sburivax(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_rt_sburivax";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sburicen_struct *dataplus;
	struct mbf_sburicen_data_struct *data;
	struct mbsys_sb_struct *store;
	char	*datacomment;
	int	time_j[5];
	short	dummy;
	int	i, j, k;
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
	dataplus = (struct mbf_sburicen_struct *) mb_io_ptr->raw_data;
	data = &(dataplus->data);
	datacomment = (char *) data;
	dataplus->kind = MB_DATA_DATA;
	store = (struct mbsys_sb_struct *) store_ptr;

	/* read next record from file */
	if ((status = fread(data,1,mb_io_ptr->data_structure_size,
			mb_io_ptr->mbfp)) == mb_io_ptr->data_structure_size) 
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		mb_io_ptr->irecord_count++;
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* if needed read the extra two bytes that occur
		every five records */
	if (status == MB_SUCCESS && mb_io_ptr->irecord_count >= 5)
		{
		mb_io_ptr->irecord_count = 0;
		fread(&dummy, 1, sizeof(short), mb_io_ptr->mbfp);
		}

	/* byte swap the data if necessary */
#ifndef BYTESWAPPED
	if (status == MB_SUCCESS)
		{
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			data->dist[i] = mb_swap_short(data->dist[i]);
			data->deph[i] = mb_swap_short(data->deph[i]);
			}
		data->axis = mb_swap_short(data->axis);
		data->major = mb_swap_short(data->major);
		data->minor = mb_swap_short(data->minor);
		data->sbhdg = mb_swap_short(data->sbhdg);
		data->lat2b = mb_swap_short(data->lat2b);
		data->lat2u = mb_swap_short(data->lat2u);
		data->lon2b = mb_swap_short(data->lon2b);
		data->lon2u = mb_swap_short(data->lon2u);
		data->sec = mb_swap_short(data->sec);
		data->min = mb_swap_short(data->min);
		data->day = mb_swap_short(data->day);
		data->year = mb_swap_short(data->year);
		data->sbtim = mb_swap_short(data->sbtim);
		}
#endif

	/* check for comment or unintelligible records */
	if (status == MB_SUCCESS)
		{
		if (data->deph[0] > 15000)
			{
			dataplus->kind = MB_DATA_COMMENT;
			}
		else if (data->year == 0)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		else
			{
			dataplus->kind = MB_DATA_DATA;
			}
		}

	/* set kind and error in mb_io_ptr */
	mb_io_ptr->new_kind = dataplus->kind;
	mb_io_ptr->new_error = *error;

	/* translate values to current ping variables 
		in mbio descriptor structure */
	if (status == MB_SUCCESS 
		&& dataplus->kind == MB_DATA_DATA)
		{
		/* get time */
		time_j[0] = data->year;
		time_j[1] = data->day;
		time_j[2] = data->min;
		time_j[3] = data->sec;
		time_j[4] = 0;
		mb_get_itime(verbose,time_j,mb_io_ptr->new_time_i);
		mb_get_time(verbose,mb_io_ptr->new_time_i,
			&(mb_io_ptr->new_time_d));

		/* get navigation */
		mb_io_ptr->new_lon = data->lon2u/60. 
			+ data->lon2b/600000.;
		mb_io_ptr->new_lat = data->lat2u/60. 
			+ data->lat2b/600000. - 90.;
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
		mb_io_ptr->new_heading = data->sbhdg*0.0054932;

		/* set speed to zero */
		mb_io_ptr->new_speed = 0.0;

		/* read distance and depth values into storage arrays */
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			mb_io_ptr->new_bath[i] = data->deph[i];
			mb_io_ptr->new_bath_acrosstrack[i] = data->dist[i];
			mb_io_ptr->new_bath_alongtrack[i] = 0.0;
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
			for (i=0;i<mb_io_ptr->beams_bath;i++)
			  fprintf(stderr,"dbg4       bath[%d]: %f  bathdist[%d]: %f\n",
				i,mb_io_ptr->new_bath[i],
				i,mb_io_ptr->new_bath_acrosstrack[i]);
			}

		/* done translating values */

		}
	else if (status == MB_SUCCESS 
		&& dataplus->kind == MB_DATA_COMMENT)
		{
		/* copy comment */
		strncpy(mb_io_ptr->new_comment,&datacomment[2],253);

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
		store->kind = dataplus->kind;

		/* position */
		store->lon2u = data->lon2u;
		store->lon2b = data->lon2b;
		store->lat2u = data->lat2u;
		store->lat2b = data->lat2b;

		/* time stamp */
		store->year = data->year;
		store->day = data->day;
		store->min = data->min;
		store->sec = data->sec;

		/* depths and distances */
		/* switch order of data as it is read into the global arrays */
		id = mb_io_ptr->beams_bath - 1;
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			store->dist[id-i] = data->dist[i];
			store->deph[id-i] = data->deph[i];
			}

		/* additional values */
		store->sbtim = data->sbtim;
		store->sbhdg = data->sbhdg;
		store->axis = data->axis;
		store->major = data->major;
		store->minor = data->minor;

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
int mbr_wt_sburivax(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_wt_sburivax";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sburicen_struct *dataplus;
	struct mbf_sburicen_data_struct *data;
	struct mbsys_sb_struct *store;
	char	*datacomment;
	int	time_j[5];
	double	lon, lat;
	short	dummy = 0;
	int	i, j;
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
	dataplus = (struct mbf_sburicen_struct *) mb_io_ptr->raw_data;
	data = &(dataplus->data);
	datacomment = (char *) data;
	store = (struct mbsys_sb_struct *) store_ptr;

	/* first set some plausible amounts for some of the 
		variables in the MBURICEN record */
	data->sbtim = 0;
	data->axis = 0;
	data->major = 0;
	data->minor = 0;

	/* second translate values from seabeam data storage structure */
	if (store != NULL)
		{
		dataplus->kind = store->kind;
		if (store->kind == MB_DATA_DATA)
			{
			/* position */
			data->lon2u = store->lon2u;
			data->lon2b = store->lon2b;
			data->lat2u = store->lat2u;
			data->lat2b = store->lat2b;

			/* time stamp */
			data->year = store->year;
			data->day = store->day;
			data->min = store->min;
			data->sec = store->sec;

			/* depths and distances */
			/* switch order of data as it is read 
				into the output arrays */
			id = mb_io_ptr->beams_bath - 1;
			for (i=0;i<mb_io_ptr->beams_bath;i++)
				{
				data->dist[i] = store->dist[id-i];
				data->deph[i] = store->deph[id-i];
				}

			/* additional values */
			data->sbtim = store->sbtim;
			data->sbhdg = store->sbhdg;
			data->axis = store->axis;
			data->major = store->major;
			data->minor = store->minor;
			}

		/* comment */
		else if (store->kind == MB_DATA_COMMENT)
			{
			strcpy(datacomment,"cc");
			strncat(datacomment,store->comment,MBSYS_SB_MAXLINE);
			}
		}

	/* set kind from current ping */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR)
		dataplus->kind = mb_io_ptr->new_kind;

	/* check for comment */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_COMMENT)
		{
		strcpy(datacomment,"cc");
		strncat(datacomment,mb_io_ptr->new_comment,
			mb_io_ptr->data_structure_size-3);
		}

	/* else translate current ping data to sburicen data structure */
	else if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_DATA)
		{
		/* get time */
		mb_get_jtime(verbose,mb_io_ptr->new_time_i,time_j);
		data->year = time_j[0];
		data->day = time_j[1];
		data->min = time_j[2];
		data->sec = time_j[3];

		/* get navigation */
		lon = mb_io_ptr->new_lon;
		if (lon < 0.0) lon = lon + 360.0;
		data->lon2u = (short int) 60.0*lon;
		data->lon2b = (short int) (600000.0*(lon - data->lon2u/60.0));
		lat = mb_io_ptr->new_lat + 90.0;
		data->lat2u = (short int) 60.0*lat;
		data->lat2b = (short int) (600000.0*(lat - data->lat2u/60.0));

		/* get heading (360 degrees = 65536) */
		data->sbhdg = 182.044444*mb_io_ptr->new_heading;

		/* put distance and depth values 
			into sburicen data structure */
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			data->deph[i] = mb_io_ptr->new_bath[i];
			data->dist[i] = mb_io_ptr->new_bath_acrosstrack[i];
			}
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Ready to write data in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       kind:       %d\n",
			dataplus->kind);
		fprintf(stderr,"dbg5       error:      %d\n",*error);
		fprintf(stderr,"dbg5       status:     %d\n",status);
		}

	/* byte swap the data if necessary */
#ifndef BYTESWAPPED
	if (dataplus->kind == MB_DATA_DATA
		|| dataplus->kind == MB_DATA_COMMENT)
		{
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			data->dist[i] = mb_swap_short(data->dist[i]);
			data->deph[i] = mb_swap_short(data->deph[i]);
			}
		data->axis = mb_swap_short(data->axis);
		data->major = mb_swap_short(data->major);
		data->minor = mb_swap_short(data->minor);
		data->sbhdg = mb_swap_short(data->sbhdg);
		data->lat2b = mb_swap_short(data->lat2b);
		data->lat2u = mb_swap_short(data->lat2u);
		data->lon2b = mb_swap_short(data->lon2b);
		data->lon2u = mb_swap_short(data->lon2u);
		data->sec = mb_swap_short(data->sec);
		data->min = mb_swap_short(data->min);
		data->day = mb_swap_short(data->day);
		data->year = mb_swap_short(data->year);
		data->sbtim = mb_swap_short(data->sbtim);
		}
#endif

	/* write next record to file */
	if (dataplus->kind == MB_DATA_DATA
		|| dataplus->kind == MB_DATA_COMMENT)
		{
		if ((status = fwrite(data,1,mb_io_ptr->data_structure_size,
			mb_io_ptr->mbfp)) 
			== mb_io_ptr->data_structure_size) 
			{
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			mb_io_ptr->orecord_count++;
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}

		/* if needed write the extra two bytes that occur
			every five records */
		if (status == MB_SUCCESS && mb_io_ptr->orecord_count >= 5)
			{
			mb_io_ptr->orecord_count = 0;
			fwrite(&dummy, 1, sizeof(short), mb_io_ptr->mbfp);
			}
		}
	else
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		if (verbose >= 5)
			fprintf(stderr,"\ndbg5  No data written in MBIO function <%s>\n",
				function_name);
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
