/*--------------------------------------------------------------------
 *    The MB-system:	mbr_mbldeoih.c	2/2/93
 *	$Id: mbr_mbldeoih.c,v 4.8 1998-10-05 17:46:15 caress Exp $
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
 * Revision 4.4  1995/03/22  19:44:26  caress
 * Added explicit casts to shorts divided by doubles for
 * ansi C compliance.
 *
 * Revision 4.3  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.2  1994/10/21  12:20:01  caress
 * Release V4.0
 *
 * Revision 4.1  1994/07/29  18:46:51  caress
 * Changes associated with supporting Lynx OS (byte swapped) and
 * using unix second time base (for time_d values).
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
 * Revision 3.0  1993/05/14  22:56:57  sohara
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
#include "../../include/mbf_mbldeoih.h"
#include "../../include/mbsys_ldeoih.h"

/* include for byte swapping on little-endian machines */
#ifdef BYTESWAPPED
#include "../../include/mb_swap.h"
#endif

/*--------------------------------------------------------------------*/
int mbr_alm_mbldeoih(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
 static char res_id[]="$Id: mbr_mbldeoih.c,v 4.8 1998-10-05 17:46:15 caress Exp $";
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
	status = mb_free(verbose,&mb_io_ptr->raw_data,error);
	status = mbsys_ldeoih_deall(verbose,mbio_ptr,
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
	unsigned char	*beamflag;
	short int	*bath;
	short int	*amp;
	short int	*bath_acrosstrack;
	short int	*bath_alongtrack;
	short int	*ss;
	short int	*ss_acrosstrack;
	short int	*ss_alongtrack;
	int	read_size;
	int	time_j[5];
	double	depthscale;
	double	distscale;
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
	beamflag = data->beamflag;
	bath = data->bath;
	amp = data->amp;
	bath_acrosstrack = data->bath_acrosstrack;
	bath_alongtrack = data->bath_alongtrack;
	ss = data->ss;
	ss_acrosstrack = data->ss_acrosstrack;
	ss_alongtrack = data->ss_alongtrack;
	
	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* read next header from file */
	if ((status = fread(header,1,mb_io_ptr->header_structure_size,
			mb_io_ptr->mbfp)) == mb_io_ptr->header_structure_size) 
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
		header->flag = mb_swap_short(header->flag);
		header->year = mb_swap_short(header->year);
		header->day = mb_swap_short(header->day);
		header->min = mb_swap_short(header->min);
		header->sec = mb_swap_short(header->sec);
		header->msec = mb_swap_short(header->msec);
		header->lon2u = mb_swap_short(header->lon2u);
		header->lon2b = mb_swap_short(header->lon2b);
		header->lat2u = mb_swap_short(header->lat2u);
		header->lat2b = mb_swap_short(header->lat2b);
		header->heading = mb_swap_short(header->heading);
		header->speed = mb_swap_short(header->speed);
		header->beams_bath = mb_swap_short(header->beams_bath);
		header->beams_amp = mb_swap_short(header->beams_amp);
		header->pixels_ss = mb_swap_short(header->pixels_ss);
		header->depth_scale = mb_swap_short(header->depth_scale);
		header->distance_scale = mb_swap_short(header->distance_scale);
		}
#endif

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
		fprintf(stderr,"dbg5       msec:       %d\n",header->msec);
		fprintf(stderr,"dbg5       lonu:       %d\n",header->lon2u);
		fprintf(stderr,"dbg5       lonb:       %d\n",header->lon2b);
		fprintf(stderr,"dbg5       latu:       %d\n",header->lat2u);
		fprintf(stderr,"dbg5       latb:       %d\n",header->lat2b);
		fprintf(stderr,"dbg5       heading:    %d\n",header->heading);
		fprintf(stderr,"dbg5       speed:      %d\n",header->speed);
		fprintf(stderr,"dbg5       beams bath: %d\n",
			header->beams_bath);
		fprintf(stderr,"dbg5       beams amp:  %d\n",
			header->beams_amp);
		fprintf(stderr,"dbg5       pixels ss:  %d\n",
			header->pixels_ss);
		fprintf(stderr,"dbg5       depth scale:%d\n",header->depth_scale);
		fprintf(stderr,"dbg5       dist scale: %d\n",header->distance_scale);
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
		/* beam and pixel dimensions are equal to maximums,
		   as in first read, reset to zero so 
		   values can follow what is
		   actually stored in the file*/
		if (mb_io_ptr->beams_bath 
		    == beams_bath_table[mb_io_ptr->format_num])
		    mb_io_ptr->beams_bath = 0;
		if (mb_io_ptr->beams_amp 
		    == beams_amp_table[mb_io_ptr->format_num])
		    mb_io_ptr->beams_amp = 0;
		if (mb_io_ptr->pixels_ss 
		    == pixels_ss_table[mb_io_ptr->format_num])
		    mb_io_ptr->pixels_ss = 0;
		
		/* if needed reset numbers of beams and allocate 
		   memory for store arrays */
		if (header->beams_bath > mb_io_ptr->beams_bath)
		    {
		    mb_io_ptr->beams_bath = header->beams_bath;
		    if (store != NULL)
			{
			if (store->beamflag != NULL)
			    status = mb_free(verbose, &store->beamflag, error);
			if (store->bath != NULL)
			    status = mb_free(verbose, &store->bath, error);
			if (store->bath_acrosstrack != NULL)
			    status = mb_free(verbose, &store->bath_acrosstrack, error);
			if (store->bath_alongtrack != NULL)
			    status = mb_free(verbose, &store->bath_alongtrack, error);
			status = mb_malloc(verbose, 
				    mb_io_ptr->beams_bath * sizeof(char),
				    &store->beamflag,error);
			status = mb_malloc(verbose, 
				    mb_io_ptr->beams_bath * sizeof(short),
				    &store->bath,error);
			status = mb_malloc(verbose, 
				    mb_io_ptr->beams_bath * sizeof(short),
				    &store->bath_acrosstrack,error);
			status = mb_malloc(verbose, 
				    mb_io_ptr->beams_bath * sizeof(short),
				    &store->bath_alongtrack,error);

			/* deal with a memory allocation failure */
			if (status == MB_FAILURE)
			    {
			    status = mb_free(verbose, &store->beamflag, error);
			    status = mb_free(verbose, &store->bath, error);
			    status = mb_free(verbose, &store->bath_acrosstrack, error);
			    status = mb_free(verbose, &store->bath_alongtrack, error);
			    status = MB_FAILURE;
			    *error = MB_ERROR_MEMORY_FAIL;
			    if (verbose >= 2)
				    {
				    fprintf(stderr,"\ndbg2  MBIO function <%s> terminated with error\n",
					    function_name);
				    fprintf(stderr,"dbg2  Return values:\n");
				    fprintf(stderr,"dbg2       error:      %d\n",*error);
				    fprintf(stderr,"dbg2  Return status:\n");
				    fprintf(stderr,"dbg2       status:  %d\n",status);
				    }
			    return(status);
			    }
			}
		    }
		if (header->beams_amp > mb_io_ptr->beams_amp)
		    {
		    mb_io_ptr->beams_amp = header->beams_amp;
		    if (store != NULL)
			{
			if (store->amp != NULL)
			    status = mb_free(verbose, &store->amp, error);
			status = mb_malloc(verbose, 
				    mb_io_ptr->beams_amp * sizeof(short),
				    &store->amp,error);

			/* deal with a memory allocation failure */
			if (status == MB_FAILURE)
			    {
			    status = mb_free(verbose, &store->amp, error);
			    status = MB_FAILURE;
			    *error = MB_ERROR_MEMORY_FAIL;
			    if (verbose >= 2)
				    {
				    fprintf(stderr,"\ndbg2  MBIO function <%s> terminated with error\n",
					    function_name);
				    fprintf(stderr,"dbg2  Return values:\n");
				    fprintf(stderr,"dbg2       error:      %d\n",*error);
				    fprintf(stderr,"dbg2  Return status:\n");
				    fprintf(stderr,"dbg2       status:  %d\n",status);
				    }
			    return(status);
			    }
			}
		    }
		if (header->pixels_ss > mb_io_ptr->pixels_ss)
		    {
		    mb_io_ptr->pixels_ss = header->pixels_ss;
		    if (store != NULL)
			{
			if (store->ss != NULL)
			    status = mb_free(verbose, &store->ss, error);
			if (store->ss_acrosstrack != NULL)
			    status = mb_free(verbose, &store->ss_acrosstrack, error);
			if (store->ss_alongtrack != NULL)
			    status = mb_free(verbose, &store->ss_alongtrack, error);
			status = mb_malloc(verbose, 
				    mb_io_ptr->pixels_ss * sizeof(short),
				    &store->ss,error);
			status = mb_malloc(verbose, 
				    mb_io_ptr->pixels_ss * sizeof(short),
				    &store->ss_acrosstrack,error);
			status = mb_malloc(verbose, 
				    mb_io_ptr->pixels_ss * sizeof(short),
				    &store->ss_alongtrack,error);

			/* deal with a memory allocation failure */
			if (status == MB_FAILURE)
			    {
			    status = mb_free(verbose, &store->ss, error);
			    status = mb_free(verbose, &store->ss_acrosstrack, error);
			    status = mb_free(verbose, &store->ss_alongtrack, error);
			    status = MB_FAILURE;
			    *error = MB_ERROR_MEMORY_FAIL;
			    if (verbose >= 2)
				    {
				    fprintf(stderr,"\ndbg2  MBIO function <%s> terminated with error\n",
					    function_name);
				    fprintf(stderr,"dbg2  Return values:\n");
				    fprintf(stderr,"dbg2       error:      %d\n",*error);
				    fprintf(stderr,"dbg2  Return status:\n");
				    fprintf(stderr,"dbg2       status:  %d\n",status);
				    }
			    return(status);
			    }
			}
		    }

		/* read bathymetry */
		read_size = sizeof(char)*header->beams_bath;
		status = fread(beamflag,1,read_size,mb_io_ptr->mbfp);
		read_size = sizeof(short int)*header->beams_bath;
		status = fread(bath,1,read_size,mb_io_ptr->mbfp);
		mb_io_ptr->file_bytes += status;
		status = fread(bath_acrosstrack,1,read_size,mb_io_ptr->mbfp);
		mb_io_ptr->file_bytes += status;
		status = fread(bath_alongtrack,1,read_size,mb_io_ptr->mbfp);
		mb_io_ptr->file_bytes += status;

		/* read amplitudes */
		read_size = sizeof(short int)*header->beams_amp;
		status = fread(amp,1,read_size,mb_io_ptr->mbfp);
		mb_io_ptr->file_bytes += status;

		/* read sidescan */
		read_size = sizeof(short int)*header->pixels_ss;
		status = fread(ss,1,read_size,mb_io_ptr->mbfp);
		mb_io_ptr->file_bytes += status;
		status = fread(ss_acrosstrack,1,read_size,mb_io_ptr->mbfp);
		mb_io_ptr->file_bytes += status;
		status = fread(ss_alongtrack,1,read_size,mb_io_ptr->mbfp);
		mb_io_ptr->file_bytes += status;

		/* byte swap the data if necessary */
#ifdef BYTESWAPPED
		for (i=0;i<header->beams_bath;i++)
			{
			data->bath[i] = mb_swap_short(data->bath[i]);
			data->bath_acrosstrack[i] 
				= mb_swap_short(data->bath_acrosstrack[i]);
			data->bath_alongtrack[i] 
				= mb_swap_short(data->bath_alongtrack[i]);
			}
		for (i=0;i<header->beams_amp;i++)
			{
			data->amp[i] = mb_swap_short(data->amp[i]);
			}
		for (i=0;i<header->pixels_ss;i++)
			{
			data->ss[i] = mb_swap_short(data->ss[i]);
			data->ss_acrosstrack[i] 
				= mb_swap_short(data->ss_acrosstrack[i]);
			data->ss_alongtrack[i] 
				= mb_swap_short(data->ss_alongtrack[i]);
			}
#endif

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
			fprintf(stderr,"dbg5       beams_amp:  %d\n",
				header->beams_amp);
			for (i=0;i<header->beams_bath;i++)
			  fprintf(stderr,"dbg5       beam:%d  flag:%d  bath:%d  amp:%d  acrosstrack:%d  alongtrack:%d\n",
				i,beamflag[i],bath[i],amp[i],
				bath_acrosstrack[i],bath_alongtrack[i]);
			fprintf(stderr,"dbg5       pixels_ss:  %d\n",
				header->pixels_ss);
			  fprintf(stderr,"dbg5       pixel:%d  ss:%d acrosstrack:%d  alongtrack:%d\n",
				i,ss[i],
				ss_acrosstrack[i],ss_alongtrack[i]);
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
		time_j[4] = 1000 * header->msec;
		mb_get_itime(verbose,time_j,mb_io_ptr->new_time_i);
		mb_get_time(verbose,mb_io_ptr->new_time_i,
			&(mb_io_ptr->new_time_d));

		/* get navigation */
		mb_io_ptr->new_lon = ((double) header->lon2u)/60. 
			+ ((double) header->lon2b)/600000.;
		mb_io_ptr->new_lat = ((double) header->lat2u)/60. 
			+ ((double) header->lat2b)/600000. - 90.;
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
		depthscale = 0.001 * header->depth_scale;
		distscale = 0.001 * header->distance_scale;
		
		/* read depth and beam location values into storage arrays */
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			mb_io_ptr->new_beamflag[i] = beamflag[i];
			mb_io_ptr->new_bath[i] = depthscale * bath[i];
			mb_io_ptr->new_bath_acrosstrack[i] = 
				distscale * bath_acrosstrack[i];
			mb_io_ptr->new_bath_alongtrack[i] = 
				distscale * bath_alongtrack[i];
			}

		/* read amplitude values into storage arrays */
		for (i=0;i<mb_io_ptr->beams_amp;i++)
			{
			mb_io_ptr->new_amp[i] = amp[i];
			}

		/* read sidescan and pixel location values into storage arrays */
		for (i=0;i<mb_io_ptr->pixels_ss;i++)
			{
			mb_io_ptr->new_ss[i] = ss[i];
			mb_io_ptr->new_ss_acrosstrack[i] = 
				distscale * ss_acrosstrack[i];
			mb_io_ptr->new_ss_alongtrack[i] = 
				distscale * ss_alongtrack[i];
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
		store->msec = header->msec;

		/* heading and speed */
		store->heading = header->heading;
		store->speed = header->speed;

		/* numbers of beams and scaling */
		store->beams_bath = header->beams_bath;
		store->beams_amp = header->beams_amp;
		store->pixels_ss = header->pixels_ss;
		store->depth_scale = header->depth_scale;
		store->distance_scale = header->distance_scale;

		/* get altitude and transducer depth */
		store->altitude = header->altitude;
		store->transducer_depth = header->transducer_depth;

		/* depths and backscatter */
		for (i=0;i<store->beams_bath;i++)
			{
			store->beamflag[i] = data->beamflag[i];
			store->bath[i] = data->bath[i];
			store->bath_acrosstrack[i] = data->bath_acrosstrack[i];
			store->bath_alongtrack[i] = data->bath_alongtrack[i];
			}
		for (i=0;i<store->beams_amp;i++)
			{
			store->amp[i] = data->amp[i];
			}
		for (i=0;i<store->pixels_ss;i++)
			{
			store->ss[i] = data->ss[i];
			store->ss_acrosstrack[i] = data->ss_acrosstrack[i];
			store->ss_alongtrack[i] = data->ss_alongtrack[i];
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
	unsigned char	*beamflag;
	short int	*bath;
	short int	*amp;
	short int	*bath_acrosstrack;
	short int	*bath_alongtrack;
	short int	*ss;
	short int	*ss_acrosstrack;
	short int	*ss_alongtrack;
	int	write_size;
	int	time_j[5];
	double	lon;
	double	lat;
	int	beams_bath;
	int	beams_amp;
	int	pixels_ss;
	double	depthmax;
	double	distmax;
	double	depthscale;
	double	distscale;
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
	beamflag = data->beamflag;
	bath = data->bath;
	amp = data->amp;
	bath_acrosstrack = data->bath_acrosstrack;
	bath_alongtrack = data->bath_alongtrack;
	ss = data->ss;
	ss_acrosstrack = data->ss_acrosstrack;
	ss_alongtrack = data->ss_alongtrack;
	header->depth_scale = 0;
	header->distance_scale = 0;

	/* first translate values from data storage structure */
	if (store != NULL)
		{
		/* save beam and pixel numbers */
		beams_bath = store->beams_bath;
		beams_amp = store->beams_amp;
		pixels_ss = store->pixels_ss;

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
		header->msec = store->msec;

		/* heading and speed */
		header->heading = store->heading;
		header->speed = store->speed;

		/* numbers of beams and scaling */
		header->beams_bath = store->beams_bath;
		header->beams_amp = store->beams_amp;
		header->pixels_ss = store->pixels_ss;
		header->depth_scale = store->depth_scale;
		header->distance_scale = store->distance_scale;

		/* get altitude and transducer depth */
		header->altitude = store->altitude;
		header->transducer_depth = store->transducer_depth;

		/* depths amplitude and sidescan */
		for (i=0;i<header->beams_bath;i++)
			{
			beamflag[i] = store->beamflag[i];
			bath[i] = store->bath[i];
			bath_acrosstrack[i] = store->bath_acrosstrack[i];
			bath_alongtrack[i] = store->bath_alongtrack[i];
			}
		for (i=0;i<header->beams_amp;i++)
			{
			amp[i] = store->amp[i];
			}
		for (i=0;i<header->pixels_ss;i++)
			{
			ss[i] = store->ss[i];
			ss_acrosstrack[i] = store->ss_acrosstrack[i];
			ss_alongtrack[i] = store->ss_alongtrack[i];
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
		/* save beam and pixel numbers */
		beams_bath = mb_io_ptr->beams_bath;
		beams_amp = mb_io_ptr->beams_amp;
		pixels_ss = mb_io_ptr->pixels_ss;

		dataplus->kind = MB_DATA_DATA;

		/* get time */
		mb_get_jtime(verbose,mb_io_ptr->new_time_i,time_j);
		header->year = time_j[0];
		header->day = time_j[1];
		header->min = time_j[2];
		header->sec = time_j[3];
		header->msec = time_j[4] / 1000;

		/* get navigation */
		lon = mb_io_ptr->new_lon;
		if (lon < 0.0) lon = lon + 360.0;
		header->lon2u = (short int) 60.0*lon;
		header->lon2b = (short int) (600000.0*
			(lon - ((double) header->lon2u)/60.0));
		lat = mb_io_ptr->new_lat + 90.0;
		header->lat2u = (short int) 60.0*lat;
		header->lat2b = (short int) (600000.0*
			(lat - ((double) header->lat2u)/60.0));

		/* get heading (360 degrees = 65536) */
		header->heading = 182.044444*mb_io_ptr->new_heading;

		/* get speed */
		header->speed = 100.*mb_io_ptr->new_speed;

		/* set numbers of beams and sidescan */
		header->beams_bath = mb_io_ptr->beams_bath;
		header->beams_amp = mb_io_ptr->beams_amp;
		header->pixels_ss = mb_io_ptr->pixels_ss;

		/* set bathymetry and backscatter scalings */
		if (store == NULL 
		    || header->depth_scale <= 0
		    || header->distance_scale <= 0)
		    {
		    depthmax = 0.0;
		    distmax = 0.0;
		    for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			depthmax = MAX(depthmax, 
				    fabs(mb_io_ptr->new_bath[i]));
			distmax = MAX(distmax, 
				    fabs(mb_io_ptr->new_bath_acrosstrack[i]));
			distmax = MAX(distmax, 
				    fabs(mb_io_ptr->new_bath_alongtrack[i]));
			}
		    for (i=0;i<mb_io_ptr->pixels_ss;i++)
			{
			distmax = MAX(distmax, 
				    fabs(mb_io_ptr->new_ss_acrosstrack[i]));
			distmax = MAX(distmax, 
				    fabs(mb_io_ptr->new_ss_alongtrack[i]));
			}
		    depthscale = MAX(0.001, depthmax / 32000);
		    header->depth_scale = 1000 * depthscale + 1;
		    depthscale = 0.001 * header->depth_scale;
		    distscale = MAX(0.001, distmax / 32000);
		    header->distance_scale = 1000 * distscale + 1;
		    distscale = 0.001 * header->distance_scale;
		    }

		/* put depth and beam location values 
			into mbldeoih data structure */
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			beamflag[i] = mb_io_ptr->new_beamflag[i];
			bath[i] = mb_io_ptr->new_bath[i] / depthscale;
			bath_acrosstrack[i] 
				= mb_io_ptr->new_bath_acrosstrack[i] 
					/ distscale;
			bath_alongtrack[i] 
				= mb_io_ptr->new_bath_alongtrack[i] 
					/ distscale;
			}

		/* put amplitude values 
			into mbldeoih data structure */
		for (i=0;i<mb_io_ptr->beams_amp;i++)
			{
			amp[i] = mb_io_ptr->new_amp[i];
			}

		/* put sidescan and pixel location values 
			into mbldeoih data structure */
		for (i=0;i<mb_io_ptr->pixels_ss;i++)
			{
			ss[i] = mb_io_ptr->new_ss[i];
			ss_acrosstrack[i] 
				= mb_io_ptr->new_ss_acrosstrack[i] 
					/ distscale;
			ss_alongtrack[i] 
				= mb_io_ptr->new_ss_alongtrack[i] 
					/ distscale;
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
		fprintf(stderr,"dbg5       msec:       %d\n",header->msec);
		fprintf(stderr,"dbg5       lonu:       %d\n",header->lon2u);
		fprintf(stderr,"dbg5       lonb:       %d\n",header->lon2b);
		fprintf(stderr,"dbg5       latu:       %d\n",header->lat2u);
		fprintf(stderr,"dbg5       latb:       %d\n",header->lat2b);
		fprintf(stderr,"dbg5       heading:    %d\n",header->heading);
		fprintf(stderr,"dbg5       speed:      %d\n",header->speed);
		fprintf(stderr,"dbg5       beams bath: %d\n",
			header->beams_bath);
		fprintf(stderr,"dbg5       beams amp:  %d\n",
			header->beams_amp);
		fprintf(stderr,"dbg5       pixels ss:  %d\n",
			header->pixels_ss);
		fprintf(stderr,"dbg5       depth scale: %d\n",header->depth_scale);
		fprintf(stderr,"dbg5       dist scale:  %d\n",header->distance_scale);
		fprintf(stderr,"dbg5       status:     %d\n",status);
		fprintf(stderr,"dbg5       error:      %d\n",*error);
		}

	/* byte swap the data if necessary */
#ifdef BYTESWAPPED
	if (status == MB_SUCCESS)
		{
		header->flag = mb_swap_short(header->flag);
		header->year = mb_swap_short(header->year);
		header->day = mb_swap_short(header->day);
		header->min = mb_swap_short(header->min);
		header->sec = mb_swap_short(header->sec);
		header->msec = mb_swap_short(header->msec);
		header->lon2u = mb_swap_short(header->lon2u);
		header->lon2b = mb_swap_short(header->lon2b);
		header->lat2u = mb_swap_short(header->lat2u);
		header->lat2b = mb_swap_short(header->lat2b);
		header->heading = mb_swap_short(header->heading);
		header->speed = mb_swap_short(header->speed);
		header->beams_bath = mb_swap_short(header->beams_bath);
		header->beams_amp = mb_swap_short(header->beams_amp);
		header->pixels_ss = mb_swap_short(header->pixels_ss);
		header->depth_scale = mb_swap_short(header->depth_scale);
		header->distance_scale = mb_swap_short(header->distance_scale);
		}
#endif

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
		fprintf(stderr,"dbg5       beams_bath: %d\n",beams_bath);
		fprintf(stderr,"dbg5       beams_amp:  %d\n",beams_amp);
		for (i=0;i<beams_bath;i++)
			fprintf(stderr,"dbg5       beam:%d  flag:%d  bath:%d  amp:%d  acrosstrack:%d  alongtrack:%d\n",
				i,beamflag[i],bath[i],amp[i],bath_acrosstrack[i],bath_alongtrack[i]);
		fprintf(stderr,"dbg5       pixels_ss:  %d\n",pixels_ss);
		for (i=0;i<pixels_ss;i++)
			fprintf(stderr,"dbg5       beam:%d  ss:%d  acrosstrack:%d  alongtrack:%d\n",
				i,ss[i],ss_acrosstrack[i],ss_alongtrack[i]);
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

		/* byte swap the data if necessary */
#ifdef BYTESWAPPED
		for (i=0;i<beams_bath;i++)
			{
			data->bath[i] = mb_swap_short(data->bath[i]);
			data->bath_acrosstrack[i] 
				= mb_swap_short(data->bath_acrosstrack[i]);
			data->bath_alongtrack[i] 
				= mb_swap_short(data->bath_alongtrack[i]);
			}
		for (i=0;i<beams_amp;i++)
			{
			data->amp[i] = mb_swap_short(data->amp[i]);
			}
		for (i=0;i<pixels_ss;i++)
			{
			data->ss[i] = mb_swap_short(data->ss[i]);
			data->ss_acrosstrack[i] 
				= mb_swap_short(data->ss_acrosstrack[i]);
			data->ss_alongtrack[i] 
				= mb_swap_short(data->ss_alongtrack[i]);
			}
#endif

		/* write bathymetry */
		write_size = sizeof(char)*beams_bath;
		status = fwrite(beamflag,1,write_size,mb_io_ptr->mbfp);
		write_size = sizeof(short int)*beams_bath;
		status = fwrite(bath,1,write_size,mb_io_ptr->mbfp);
		status = fwrite(bath_acrosstrack,1,write_size,mb_io_ptr->mbfp);
		status = fwrite(bath_alongtrack,1,write_size,mb_io_ptr->mbfp);

		/* write amplitude */
		write_size = sizeof(short int)*beams_amp;
		status = fwrite(amp,1,write_size,mb_io_ptr->mbfp);

		/* write sidescan */
		write_size = sizeof(short int)*pixels_ss;
		status = fwrite(ss,1,write_size,mb_io_ptr->mbfp);
		status = fwrite(ss_acrosstrack,1,write_size,mb_io_ptr->mbfp);
		status = fwrite(ss_alongtrack,1,write_size,mb_io_ptr->mbfp);

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
