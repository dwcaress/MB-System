/*--------------------------------------------------------------------
 *    The MB-system:	mbr_em12darw.c	2/2/93
 *	$Id: mbr_em12darw.c,v 4.0 1994-03-06 00:01:56 caress Exp $
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
 * mbr_em12darw.c contains the functions for reading and writing
 * multibeam data in the EM12DARW format.  
 * These functions include:
 *   mbr_alm_em12darw	- allocate read/write memory
 *   mbr_dem_em12darw	- deallocate read/write memory
 *   mbr_rt_em12darw	- read and translate data
 *   mbr_wt_em12darw	- translate and write data
 *
 * Author:	R. B. Owens
 * Date:	January 24, 1994
 * $Log: not supported by cvs2svn $
 * Revision 4.0  1994/03/05  22:54:09  caress
 * First cut.
 *
 * Revision 3.0  1993/05/14  22:56:29  sohara
 * initial version
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
#include "mbf_em12darw.h"
#include "mbsys_em12.h"

/*--------------------------------------------------------------------*/
int mbr_alm_em12darw(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
 static char res_id[]="$Id: mbr_em12darw.c,v 4.0 1994-03-06 00:01:56 caress Exp $";
	char	*function_name = "mbr_alm_em12darw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_em12darw_struct *data;
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
	mb_io_ptr->structure_size = sizeof(struct mbf_em12darw_struct);
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mb_malloc(verbose,sizeof(struct mbsys_em12_struct),
				&mb_io_ptr->store_data,error);

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	data = (struct mbf_em12darw_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;

	/* initialize everything to zeros */
	mbr_zero_em12darw(verbose,data_ptr,error);

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
int mbr_dem_em12darw(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	char	*function_name = "mbr_dem_em12darw";
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
	status = mb_free(verbose,mb_io_ptr->store_data,error);


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
int mbr_zero_em12darw(verbose,data_ptr,error)
int	verbose;
char	*data_ptr;
int	*error;
{
	char	*function_name = "mbr_zero_em12darw";
	int	status = MB_SUCCESS;
	struct mbf_em12darw_struct *data;
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
	data = (struct mbf_em12darw_struct *) data_ptr;

	/* initialize everything to zeros */
	if (data != NULL)
		{
		/* record type */
		data->func = 150;

		/* time */
		data->year = 0;
		data->jday = 0;
		data->minute = 0;
		data->secs = 0;

		/* navigation */
		data->latitude = 0.0;
		data->longitude = 0.0;
		data->speed = 0.0;
		data->gyro = 0.0;
		data->roll = 0.0;
		data->pitch = 0.0;
		data->heave = 0.0;

		/* other parameters */
		data->corflag = 0;
		data->utm_merd = 0.0;
		data->utm_zone = 0;
		data->posq = 0;
		data->pingno = 0;
		data->mode = 0;
		data->depthl = 0.0;
		data->sndval = 0.0;

		/* beam values */
		for (i=0;i<MBSYS_EM12_BEAMS;i++)
			{
			data->depth[i] = 0;
			data->distacr[i] = 0;
			data->distalo[i] = 0;
			data->range[i] = 0;
			data->refl[i] = 0;
			data->beamq[i] = 0;
			}
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
int mbr_rt_em12darw(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;  
int	*error;
{
	char	*function_name = "mbr_rt_em12darw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_em12darw_struct *data;
	struct mbsys_em12_struct *store;
	char	*datacomment;
	int	time_j[4];
	int	i, j, k, n, data_kind;
	int	iamp;
	double	depthscale,dacrscale,daloscale,rangescale,reflscale;

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
	data = (struct mbf_em12darw_struct *) mb_io_ptr->raw_data;
	datacomment = (char *) data->depth;
	store = (struct mbsys_em12_struct *) store_ptr;	


	/* read next record from file */
	if ((status = fread(data,1,mb_io_ptr->structure_size,
			mb_io_ptr->mbfp)) == mb_io_ptr->structure_size) 
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
		if (data->func == 100)
			{
			data_kind = MB_DATA_COMMENT;
			}
		else if (data->year == 0)
			{
			data_kind = MB_DATA_NONE;
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		else
			{
			data_kind = MB_DATA_DATA;
			}
		}

	/* set kind and error in mb_io_ptr */
	mb_io_ptr->new_kind = data_kind;
	mb_io_ptr->new_error = *error;

	/* translate values to current ping variables 
		in mbio descriptor structure */
	if (status == MB_SUCCESS 
		&& data_kind == MB_DATA_DATA)
		{
		/* get time */
		time_j[0] = data->year + 1900;
		time_j[1] = data->jday;
		time_j[2] = data->minute;
		time_j[3] = data->secs/100;
		mb_get_itime(verbose,time_j,mb_io_ptr->new_time_i);
		mb_get_time(verbose,mb_io_ptr->new_time_i,
			&(mb_io_ptr->new_time_d));

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
		mb_io_ptr->new_heading = data->gyro;

		/* get speed (convert nm/hr to km/hr) */
		mb_io_ptr->new_speed = 1.8333333*data->speed;

		/* read beam values into storage arrays */
		/* DO NOT switch order of data as it is read 
			into the global arrays */
		if (data->mode == 1)
			{
			depthscale = 0.1;
			dacrscale  = 0.2;
			daloscale  = 0.2;
			rangescale = 0.2;
			reflscale  = 0.5;
			}
		else if (data->mode == 2)
			{
			depthscale = 0.2;
			dacrscale  = 0.5;
			daloscale  = 0.5;
			rangescale = 0.8;
			reflscale  = 0.5;
			}
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			mb_io_ptr->new_bath[i] 
				= (int)(depthscale*data->depth[i]);
			mb_io_ptr->new_bath_acrosstrack[i] 
				= (int)(dacrscale*data->distacr[i]);
			mb_io_ptr->new_bath_alongtrack[i] 
				= (int)(daloscale*data->distalo[i]);
			}
		for (i=0;i<mb_io_ptr->beams_amp;i++)
			{
			iamp = (int)(reflscale*data->refl[i]);
			mb_io_ptr->new_amp[i] = iamp+200;
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
			fprintf(stderr,"dbg4       beams_amp:  %d\n",
				mb_io_ptr->beams_amp);
			for (i=0;i<mb_io_ptr->beams_bath;i++)
			  fprintf(stderr,"dbg4       beam:%d  bath:%d  amp:%d  acrosstrack:%d  alongtrack:%d\n",
				i,mb_io_ptr->new_bath[i],
				mb_io_ptr->new_amp[i],
				mb_io_ptr->new_bath_acrosstrack[i],
				mb_io_ptr->new_bath_alongtrack[i]);
			}

		/* done translating values */

		}
	else if (status == MB_SUCCESS 
		&& data_kind == MB_DATA_COMMENT)
		{
		/* copy comment */
		strncpy(mb_io_ptr->new_comment,datacomment,256);

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

	/* translate values to em12 data storage structure */
	if (status == MB_SUCCESS
		&& store != NULL)
		{
		/* type of data record */
		store->kind = data_kind;

		/* time */
		store->year = data->year;
		store->jday = data->jday;
		store->minute = data->minute;
		store->secs = data->secs;

		/* navigation */
		store->latitude = data->latitude;
		store->longitude = data->longitude;
		store->speed = data->speed;
		store->gyro = data->gyro;
		store->roll = data->roll;
		store->pitch = data->pitch;
		store->heave = data->heave;

		/* other parameters */
		store->corflag = data->corflag;
		store->utm_merd = data->utm_merd;
		store->utm_zone = data->utm_zone;
		store->posq = data->posq;
		store->pingno = data->pingno;
		store->mode = data->mode;
		store->depthl = data->depthl;
		store->sndval = data->sndval;

		/* beam values */
		for (i=0;i<MBSYS_EM12_BEAMS;i++)
			{
			store->depth[i] = data->depth[i];
			store->distacr[i] = data->distacr[i];
			store->distalo[i] = data->distalo[i];
			store->range[i] = data->range[i];
			store->refl[i] = data->refl[i];
			store->beamq[i] = data->beamq[i];
			}

		/* comment */
		strncpy(store->comment,mb_io_ptr->new_comment,
			MBSYS_EM12_MAXLINE);
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
int mbr_wt_em12darw(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
{
	char	*function_name = "mbr_wt_em12darw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_em12darw_struct *data;
	struct mbsys_em12_struct *store;
	char	*datacomment;
	int	time_i[6];
	int	time_j[4];
	double	lon, lat;
	double	depthscale, dacrscale,daloscale,rangescale,reflscale;
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
	data = (struct mbf_em12darw_struct *) mb_io_ptr->raw_data;
	datacomment = (char *) data->depth;
	store = (struct mbsys_em12_struct *) store_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Status at beginning of MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       new_kind:       %d\n",
			mb_io_ptr->new_kind);
		fprintf(stderr,"dbg5       new_error:      %d\n",
			mb_io_ptr->new_error);
		fprintf(stderr,"dbg5       error:          %d\n",*error);
		fprintf(stderr,"dbg5       status:         %d\n",status);
		}


	/*  translate values from em12 data storage structure */
	if (store != NULL)
		{
		if (store->kind == MB_DATA_DATA)
			{
			/* record type */
			data->func = 150;

			/* time */
			data->year = store->year;
			data->jday = store->jday;
			data->minute = store->minute;
			data->secs = store->secs;

			/* navigation */
			data->latitude = store->latitude;
			data->longitude = store->longitude;
			data->speed = store->speed;
			data->gyro = store->gyro;
			data->roll = store->roll;
			data->pitch = store->pitch;
			data->heave = store->heave;

			/* other parameters */
			data->corflag = store->corflag;
			data->utm_merd = store->utm_merd;
			data->utm_zone = store->utm_zone;
			data->posq = store->posq;
			data->pingno = store->pingno;
			data->mode = store->mode;
			data->depthl = store->depthl;
			data->sndval = store->sndval;

			/* beam values */
			for (i=0;i<MBSYS_EM12_BEAMS;i++)
				{
				data->depth[i] = store->depth[i];
				data->distacr[i] = store->distacr[i];
				data->distalo[i] = store->distalo[i];
				data->range[i] = store->range[i];
				data->refl[i] = store->refl[i];
				data->beamq[i] = store->beamq[i];
				}
			}

		/* comment */
		else if (store->kind == MB_DATA_COMMENT)
			{
			data->func=100;
			strncpy(datacomment,store->comment,MBSYS_EM12_MAXLINE);
			}
		}

	/* check for comment */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_COMMENT)
		{
		data->func=100;
		strncpy(datacomment,mb_io_ptr->new_comment,MBSYS_EM12_MAXLINE);
		}

	/* else translate current ping data to em12darw data structure */
	else if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_DATA)
		{
		/* record type */
		data->func=150;
		if (store == NULL)
			data->mode = 2;	/* DEEP mode (see scaling
						 factors below) */

		/* get time */
		mb_get_jtime(verbose,mb_io_ptr->new_time_i,time_j);
		data->year = time_j[0]-1900;
		data->jday = time_j[1];
		data->minute = time_j[2];
		data->secs = 100*time_j[3];

		/* get navigation */
		if (mb_io_ptr->new_lon < -180.0)
			mb_io_ptr->new_lon = mb_io_ptr->new_lon + 360.0;
		if (mb_io_ptr->new_lon > 180.0)
			mb_io_ptr->new_lon = mb_io_ptr->new_lon - 360.0;
		data->longitude = mb_io_ptr->new_lon;
		data->latitude = mb_io_ptr->new_lat;
		data->corflag = 0;		/* lat/lon co-ords */
		data->utm_merd = 0.0;
		data->utm_zone = 0;

		/* get heading */
		data->gyro = mb_io_ptr->new_heading;

		/* get speed (convert km/hr to nm/hr) */
		data->speed = 0.545454*mb_io_ptr->new_speed;
		data->depthl = mb_io_ptr->new_bath[mb_io_ptr->beams_bath/2];

		/* put beam values 
			into em12darw data structure */
		if (data->mode == 1)
			{
			depthscale = 0.1;
			dacrscale  = 0.2;
			daloscale  = 0.2;
			rangescale = 0.2;
			reflscale  = 0.5;
			}
		else
			{
			data->mode = 2;
			depthscale = 0.2;
			dacrscale  = 0.5;
			daloscale  = 0.5;
			rangescale = 0.8;
			reflscale  = 0.5;
			}
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			data->depth[i] = (int)(mb_io_ptr->new_bath[i]
					/depthscale);
			data->distacr[i] = 
				(int) (mb_io_ptr->new_bath_acrosstrack[i]
				/dacrscale);
			data->distalo[i] = 
				(int) (mb_io_ptr->new_bath_alongtrack[i]
				/daloscale);
			}
		for (i=0;i<mb_io_ptr->beams_amp;i++)
			{
			data->refl[i] = (int)((mb_io_ptr->new_amp[i]-200)
				/reflscale);
			}

		} /* end if on mb_io_ptr->new_kind */

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Ready to write data in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       kind:       %d\n", 
				mb_io_ptr->new_kind);
		fprintf(stderr,"dbg5       error:      %d\n",*error);
		fprintf(stderr,"dbg5       status:     %d\n",status);
		}

	/* write next record to file */
	if (data->func == 150 || data->func == 100)
		{
		if ((status = fwrite(data,1,mb_io_ptr->structure_size,
				mb_io_ptr->mbfp)) 
				== mb_io_ptr->structure_size) 
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
