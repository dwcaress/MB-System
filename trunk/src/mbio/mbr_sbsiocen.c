/*--------------------------------------------------------------------
 *    The MB-system:	mbr_sbsiocen.c	2/2/93
 *	$Id: mbr_sbsiocen.c,v 4.9 2000-09-30 06:34:20 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 2000 by
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
 * mbr_sbsiocen.c contains the functions for reading and writing
 * multibeam data in the SBSIOCEN format.  
 * These functions include:
 *   mbr_alm_sbsiocen	- allocate read/write memory
 *   mbr_dem_sbsiocen	- deallocate read/write memory
 *   mbr_rt_sbsiocen	- read and translate data
 *   mbr_wt_sbsiocen	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	February 2, 1993
 * $Log: not supported by cvs2svn $
 * Revision 4.8  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.7  1997/07/25  14:19:53  caress
 * Version 4.5beta2.
 * Much mucking, particularly with Simrad formats.
 *
 * Revision 4.6  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.5  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.5  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.4  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.3  1994/10/21  12:20:01  caress
 * Release V4.0
 *
 * Revision 4.2  1994/07/29  18:46:51  caress
 * Changes associated with supporting Lynx OS (byte swapped) and
 * using unix second time base (for time_d values).
 *
 * Revision 4.1  1994/05/11  21:23:01  caress
 * Added initialization of bathalongtrack array.
 *
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
 * Revision 4.1  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.0  1994/02/21  03:59:50  caress
 * First cut at new version. Altered to be consistent
 * with passing of three types of data: bathymetry,
 * amplitude, and sidescan.
 *
 * Revision 3.0  1993/05/14  22:57:19  sohara
 * initial version
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
#include "../../include/mbf_sbsiocen.h"

/* include for byte swapping on little-endian machines */
#ifdef BYTESWAPPED
#include "../../include/mb_swap.h"
#endif

/*--------------------------------------------------------------------*/
int mbr_alm_sbsiocen(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
 static char res_id[]="$Id: mbr_sbsiocen.c,v 4.9 2000-09-30 06:34:20 caress Exp $";
	char	*function_name = "mbr_alm_sbsiocen";
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
	mb_io_ptr->structure_size = sizeof(struct mbf_sbsiocen_struct);
	mb_io_ptr->data_structure_size = 
		sizeof(struct mbf_sbsiocen_data_struct);
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
int mbr_dem_sbsiocen(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	char	*function_name = "mbr_dem_sbsiocen";
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
int mbr_rt_sbsiocen(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_rt_sbsiocen";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sbsiocen_struct *dataplus;
	struct mbf_sbsiocen_data_struct *data;
	struct mbsys_sb_struct *store;
	char	*datacomment;
	int	time_j[5];
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
	dataplus = (struct mbf_sbsiocen_struct *) mb_io_ptr->raw_data;
	data = &(dataplus->data);
	datacomment = (char *) data;
	dataplus->kind = MB_DATA_DATA;
	store = (struct mbsys_sb_struct *) store_ptr;

	/* read next record from file */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;
	if ((status = fread(data,1,mb_io_ptr->data_structure_size,
			mb_io_ptr->mbfp)) == mb_io_ptr->data_structure_size) 
		{
		mb_io_ptr->file_bytes += status;
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
	else
		{
		mb_io_ptr->file_bytes += status;
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
		data->major = mb_swap_short(data->major);
		data->minor = mb_swap_short(data->minor);
		data->axis = mb_swap_short(data->axis);
		data->lat2u = mb_swap_short(data->lat2u);
		data->lat2b = mb_swap_short(data->lat2b);
		data->lon2u = mb_swap_short(data->lon2u);
		data->lon2b = mb_swap_short(data->lon2b);
		data->sbtim = mb_swap_short(data->sbtim);
		data->sbhdg = mb_swap_short(data->sbhdg);
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			data->dist[i] = mb_swap_short(data->dist[i]);
			data->deph[i] = mb_swap_short(data->deph[i]);
			}
		data->spare = mb_swap_short(data->spare);
		}
#endif

	/* check for comment or unintelligible records */
	if (status == MB_SUCCESS)
		{
		if (data->year > 7000)
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
		/* switch order of data as it is read into the global arrays */
		id = mb_io_ptr->beams_bath - 1;
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			if (data->deph[i] > 0)
			    {
			    mb_io_ptr->new_beamflag[id-i] = MB_FLAG_NONE;
			    mb_io_ptr->new_bath[id-i] = data->deph[i];
			    }
			else if (data->deph[i] < 0)
			    {
			    mb_io_ptr->new_beamflag[id-i] = 
				MB_FLAG_MANUAL + MB_FLAG_FLAG;
			    mb_io_ptr->new_bath[id-i] = -data->deph[i];
			    }
			else
			    {
			    mb_io_ptr->new_beamflag[id-i] = MB_FLAG_NULL;
			    mb_io_ptr->new_bath[id-i] = data->deph[i];
			    }

			mb_io_ptr->new_bath_acrosstrack[id-i] = data->dist[i];
			mb_io_ptr->new_bath_alongtrack[id-i] = 0.0;
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
			  fprintf(stderr,"dbg4       flag[%d]:%4d  bath: %f  bathdist[%d]: %f\n",
				i,mb_io_ptr->new_beamflag[i],mb_io_ptr->new_bath[i],
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

	/* translate values to sea beam data storage structure */
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
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			store->dist[i] = data->dist[i];
			store->deph[i] = data->deph[i];
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
int mbr_wt_sbsiocen(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_wt_sbsiocen";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sbsiocen_struct *dataplus;
	struct mbf_sbsiocen_data_struct *data;
	struct mbsys_sb_struct *store;
	char	*datacomment;
	int	time_j[5];
	double	lon, lat;
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
	dataplus = (struct mbf_sbsiocen_struct *) mb_io_ptr->raw_data;
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
			for (i=0;i<mb_io_ptr->beams_bath;i++)
				{
				data->dist[i] = store->dist[i];
				data->deph[i] = store->deph[i];
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
		strcpy(datacomment,"##");
		strncat(datacomment,mb_io_ptr->new_comment,
			mb_io_ptr->data_structure_size-3);
		}

	/* else translate current ping data to sbsiocen data structure */
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
			into sbsiocen data structure */
		/* switch order of data as it is read 
			into the output arrays */
		id = mb_io_ptr->beams_bath - 1;
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			if (mb_beam_check_flag(mb_io_ptr->new_beamflag[id-i]))
			    data->deph[i] = -mb_io_ptr->new_bath[id-i];
			else
			    data->deph[i] = mb_io_ptr->new_bath[id-i];
			data->dist[i] = mb_io_ptr->new_bath_acrosstrack[id-i];
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
#ifdef BYTESWAPPED
	if (dataplus->kind == MB_DATA_DATA
		|| dataplus->kind == MB_DATA_COMMENT)
		{
		data->year = mb_swap_short(data->year);
		data->day = mb_swap_short(data->day);
		data->min = mb_swap_short(data->min);
		data->sec = mb_swap_short(data->sec);
		data->major = mb_swap_short(data->major);
		data->minor = mb_swap_short(data->minor);
		data->axis = mb_swap_short(data->axis);
		data->lat2u = mb_swap_short(data->lat2u);
		data->lat2b = mb_swap_short(data->lat2b);
		data->lon2u = mb_swap_short(data->lon2u);
		data->lon2b = mb_swap_short(data->lon2b);
		data->sbtim = mb_swap_short(data->sbtim);
		data->sbhdg = mb_swap_short(data->sbhdg);
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			data->dist[i] = mb_swap_short(data->dist[i]);
			data->deph[i] = mb_swap_short(data->deph[i]);
			}
		data->spare = mb_swap_short(data->spare);
		}
#endif

	/* write next record to file */
	if (dataplus->kind == MB_DATA_DATA
		|| dataplus->kind == MB_DATA_COMMENT)
		{
		if ((status = fwrite(data,1,mb_io_ptr->data_structure_size,
			mb_io_ptr->mbfp)) == mb_io_ptr->data_structure_size) 
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
