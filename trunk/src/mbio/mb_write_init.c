/*--------------------------------------------------------------------
 *    The MB-system:	mb_write_init.c	1/25/93
 *    $Id$
 *
 *    Copyright (c) 1993-2015 by
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
 * mb_write_init.c opens and initializes a multibeam data file
 * for writing with mb_write or mb_put.
 *
 * Author:	D. W. Caress
 * Date:	January 25, 1993
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
#include "mb_segy.h"
#include "../surf/mb_sapi.h"
#include "gsf.h"
#include "netcdf.h"

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/
int mb_write_init(int verbose,
		char *file, int format, void **mbio_ptr,
		int *beams_bath, int *beams_amp, int *pixels_ss,
		int *error)
{
	char	*function_name = "mb_write_init";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int	status_save;
	int	error_save;
	int	sapi_status;
	char	*lastslash;
	char	path[MB_PATH_MAXLINE], name[MB_PATH_MAXLINE];
	int	i;
	char	*stdout_string = "stdout";

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       file:       %s\n",file);
		fprintf(stderr,"dbg2       format:     %d\n",format);
		}

	/* allocate memory for mbio descriptor */
	status = mb_mallocd(verbose,__FILE__, __LINE__,sizeof(struct mb_io_struct),
				(void **)mbio_ptr,error);
	if (status == MB_SUCCESS)
		{
		memset(*mbio_ptr, 0, sizeof(struct mb_io_struct));
		mb_io_ptr = (struct mb_io_struct *) *mbio_ptr;
		}

	/* set system byte order flag */
	if (status == MB_SUCCESS)
		{
		mb_io_ptr->byteswapped = mb_swap_check();
		}

	/* get format information */
	if (status == MB_SUCCESS)
		{
		status = mb_format_register(verbose, &format,
					*mbio_ptr, error);
		}

	/* quit if there is a problem */
	if (status == MB_FAILURE)
		{
			/* free memory for mbio descriptor */
		if (mbio_ptr != NULL)
			{
			status_save = status;
			error_save = *error;
			status = mb_freed(verbose,__FILE__, __LINE__,(void **)mbio_ptr,error);
			status = status_save;
			*error = error_save;
			}

		/* output debug information */
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

	/* initialize file access for the mbio descriptor */
	mb_io_ptr->filemode = MB_FILEMODE_WRITE;
	mb_io_ptr->mbfp = NULL;
	strcpy(mb_io_ptr->file,file);
	mb_io_ptr->file_pos = 0;
	mb_io_ptr->file_bytes = 0;
	mb_io_ptr->mbfp2 = NULL;
	strcpy(mb_io_ptr->file2,"\0");
	mb_io_ptr->file2_pos = 0;
	mb_io_ptr->file2_bytes = 0;
	mb_io_ptr->mbfp3 = NULL;
	strcpy(mb_io_ptr->file3,"\0");
	mb_io_ptr->file3_pos = 0;
	mb_io_ptr->file3_bytes = 0;
	mb_io_ptr->ncid = 0;
	mb_io_ptr->gsfid = 0;
	mb_io_ptr->xdrs = NULL;
	mb_io_ptr->xdrs2 = NULL;
	mb_io_ptr->xdrs3 = NULL;

	/* load control parameters into the mbio descriptor */
	mb_io_ptr->format = format;
	mb_io_ptr->pings = 0;
	mb_io_ptr->lonflip = 0;
	for (i=0;i<4;i++)
		mb_io_ptr->bounds[i] = 0;
	for (i=0;i<7;i++)
		{
		mb_io_ptr->btime_i[i] = 0;
		mb_io_ptr->etime_i[i] = 0;
		}
	mb_io_ptr->speedmin = 0.0;
	mb_io_ptr->timegap = 0.0;
	mb_io_ptr->btime_d = 0.0;
	mb_io_ptr->etime_d = 0.0;

	/* set the number of beams */
	*beams_bath = mb_io_ptr->beams_bath_max;
	*beams_amp = mb_io_ptr->beams_amp_max;
	*pixels_ss = mb_io_ptr->pixels_ss_max;
	mb_io_ptr->new_beams_bath = 0;
	mb_io_ptr->new_beams_amp = 0;
	mb_io_ptr->new_pixels_ss = 0;
	if (verbose >= 4)
		{
		fprintf(stderr,"\ndbg4  Beam and pixel dimensions set in MBIO function <%s>\n",
				function_name);
		fprintf(stderr,"dbg4       beams_bath: %d\n",
			mb_io_ptr->beams_bath_max);
		fprintf(stderr,"dbg4       beams_amp:  %d\n",
			mb_io_ptr->beams_amp_max);
		fprintf(stderr,"dbg4       pixels_ss:  %d\n",
			mb_io_ptr->pixels_ss_max);
		}

	/* initialize pointers */
	mb_io_ptr->raw_data = NULL;
	mb_io_ptr->store_data = NULL;
	mb_io_ptr->beamflag = NULL;
	mb_io_ptr->bath = NULL;
	mb_io_ptr->amp = NULL;
	mb_io_ptr->bath_acrosstrack = NULL;
	mb_io_ptr->bath_alongtrack = NULL;
	mb_io_ptr->bath_num = NULL;
	mb_io_ptr->amp_num = NULL;
	mb_io_ptr->ss = NULL;
	mb_io_ptr->ss_acrosstrack = NULL;
	mb_io_ptr->ss_alongtrack = NULL;
	mb_io_ptr->ss_num = NULL;
	mb_io_ptr->new_beamflag = NULL;
	mb_io_ptr->new_bath = NULL;
	mb_io_ptr->new_amp = NULL;
	mb_io_ptr->new_bath_acrosstrack = NULL;
	mb_io_ptr->new_bath_alongtrack = NULL;
	mb_io_ptr->new_ss = NULL;
	mb_io_ptr->new_ss_acrosstrack = NULL;
	mb_io_ptr->new_ss_alongtrack = NULL;

	/* initialize projection parameters */
	mb_io_ptr->projection_initialized = MB_NO;
	mb_io_ptr->pjptr = NULL;

	/* initialize ancillary variables used
		to save information in certain cases */
	mb_io_ptr->save_flag = MB_NO;
	mb_io_ptr->save_label_flag = MB_NO;
	mb_io_ptr->save1 = 0;
	mb_io_ptr->save2 = 0;
	mb_io_ptr->save3 = 0;
	mb_io_ptr->save4 = 0;
	mb_io_ptr->save5 = 0;
	mb_io_ptr->save6 = 0;
	mb_io_ptr->save7 = 0;
	mb_io_ptr->save8 = 0;
	mb_io_ptr->save9 = 0;
	mb_io_ptr->save10 = 0;
	mb_io_ptr->save11 = 0;
	mb_io_ptr->save12 = 0;
	mb_io_ptr->save13 = 0;
	mb_io_ptr->save14 = 0;
	mb_io_ptr->saved1 = 0;
	mb_io_ptr->saved2 = 0;
	mb_io_ptr->saved3 = 0;
	mb_io_ptr->saved4 = 0;
	mb_io_ptr->saved5 = 0;
	mb_io_ptr->saveptr1 = NULL;
	mb_io_ptr->saveptr2 = NULL;

	/* allocate arrays */
	mb_io_ptr->beams_bath_alloc = mb_io_ptr->beams_bath_max;
	mb_io_ptr->beams_amp_alloc = mb_io_ptr->beams_amp_max;
	mb_io_ptr->pixels_ss_alloc = mb_io_ptr->pixels_ss_max;
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->beams_bath_alloc*sizeof(char),
				(void **)&mb_io_ptr->beamflag,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->beams_bath_alloc*sizeof(double),
				(void **)&mb_io_ptr->bath,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->beams_amp_alloc*sizeof(double),
				(void **)&mb_io_ptr->amp,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->beams_bath_alloc*sizeof(double),
				(void **)&mb_io_ptr->bath_acrosstrack,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->beams_bath_alloc*sizeof(double),
				(void **)&mb_io_ptr->bath_alongtrack,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->beams_bath_alloc*sizeof(int),
				(void **)&mb_io_ptr->bath_num,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->beams_amp_alloc*sizeof(int),
				(void **)&mb_io_ptr->amp_num,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->pixels_ss_alloc*sizeof(double),
				(void **)&mb_io_ptr->ss,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->pixels_ss_alloc*sizeof(double),
				(void **)&mb_io_ptr->ss_acrosstrack,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->pixels_ss_alloc*sizeof(double),
				(void **)&mb_io_ptr->ss_alongtrack,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->pixels_ss_alloc*sizeof(int),
				(void **)&mb_io_ptr->ss_num,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->beams_bath_alloc*sizeof(char),
				(void **)&mb_io_ptr->new_beamflag,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->beams_bath_alloc*sizeof(double),
				(void **)&mb_io_ptr->new_bath,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->beams_amp_alloc*sizeof(double),
				(void **)&mb_io_ptr->new_amp,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->beams_bath_alloc*sizeof(double),
				(void **)&mb_io_ptr->new_bath_acrosstrack,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->beams_bath_alloc*sizeof(double),
				(void **)&mb_io_ptr->new_bath_alongtrack,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->pixels_ss_alloc*sizeof(double),
				(void **)&mb_io_ptr->new_ss,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->pixels_ss_alloc*sizeof(double),
				(void **)&mb_io_ptr->new_ss_acrosstrack,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->pixels_ss_alloc*sizeof(double),
				(void **)&mb_io_ptr->new_ss_alongtrack,error);

	/* call routine to allocate memory for format dependent i/o */
	if (status == MB_SUCCESS)
		status = (*mb_io_ptr->mb_io_format_alloc)(verbose,*mbio_ptr,error);

	/* deal with a memory allocation failure */
	if (status == MB_FAILURE)
		{
		status = mb_deall_ioarrays(verbose, mbio_ptr, error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr,error);
		mb_io_ptr->beams_bath_alloc = 0;
		mb_io_ptr->beams_amp_alloc = 0;
		mb_io_ptr->pixels_ss_alloc = 0;
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

	/* handle normal or xdr files to be opened
	   directly with fopen */
	if (mb_io_ptr->filetype == MB_FILETYPE_NORMAL
	    || mb_io_ptr->filetype == MB_FILETYPE_XDR)
	    {
	    /* open the first file */
	    if (strncmp(file,stdout_string,6) == 0)
		mb_io_ptr->mbfp = stdout;
	    else
		if ((mb_io_ptr->mbfp = fopen(mb_io_ptr->file, "wb")) == NULL)
		    {
		    *error = MB_ERROR_OPEN_FAIL;
		    status = MB_FAILURE;
		    }

	    /* open the second file if required */
	    if (status == MB_SUCCESS
		&& mb_io_ptr->numfile >= 2)
		{
		if ((mb_io_ptr->mbfp2 = fopen(mb_io_ptr->file2, "wb")) == NULL)
		    {
		    *error = MB_ERROR_OPEN_FAIL;
		    status = MB_FAILURE;
		    }
		}

	    /* or open the second file if desired and possible */
	    else if (status == MB_SUCCESS
		&& mb_io_ptr->numfile <= -2)
		mb_io_ptr->mbfp2 = fopen(mb_io_ptr->file2, "wb");

	    /* open the third file if required */
	    if (status == MB_SUCCESS
		&& mb_io_ptr->numfile >= 3)
		{
		if ((mb_io_ptr->mbfp3 = fopen(mb_io_ptr->file3, "wb")) == NULL)
		    {
		    *error = MB_ERROR_OPEN_FAIL;
		    status = MB_FAILURE;
		    }
		}

	    /* or open the third file if desired and possible */
	    else if (status == MB_SUCCESS
		&& mb_io_ptr->numfile <= -3)
		mb_io_ptr->mbfp3 = fopen(mb_io_ptr->file3, "wb");

	    /* if needed, initialize XDR stream */
	    if (status == MB_SUCCESS && mb_io_ptr->filetype == MB_FILETYPE_XDR)
		{
		status = mb_mallocd(verbose,__FILE__, __LINE__,sizeof(XDR),
				(void **)&mb_io_ptr->xdrs,error);
		if (status == MB_SUCCESS)
		    {
		    xdrstdio_create((XDR *)mb_io_ptr->xdrs,
			    mb_io_ptr->mbfp, XDR_ENCODE);
		    }
		else
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_MEMORY_FAIL;
		    }
		}

	    /* if needed, initialize second XDR stream */
	    if (status == MB_SUCCESS
		&& mb_io_ptr->filetype == MB_FILETYPE_XDR
		&& (mb_io_ptr->numfile >= 2 || mb_io_ptr->numfile <= -2)
		&& mb_io_ptr->mbfp2 != NULL)
		{
		status = mb_mallocd(verbose,__FILE__, __LINE__,sizeof(XDR),
				(void **)&mb_io_ptr->xdrs2,error);
		if (status == MB_SUCCESS)
		    {
		    xdrstdio_create((XDR *)mb_io_ptr->xdrs2,
			    mb_io_ptr->mbfp2, XDR_ENCODE);
		    }
		else
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_MEMORY_FAIL;
		    }
		}

	    /* if needed, initialize third XDR stream */
	    if (status == MB_SUCCESS
		&& mb_io_ptr->filetype == MB_FILETYPE_XDR
		&& (mb_io_ptr->numfile >= 3 || mb_io_ptr->numfile <= -3)
		&& mb_io_ptr->mbfp3 != NULL)
		{
		status = mb_mallocd(verbose,__FILE__, __LINE__,sizeof(XDR),
				(void **)&mb_io_ptr->xdrs3,error);
		if (status == MB_SUCCESS)
		    {
		    xdrstdio_create((XDR *)mb_io_ptr->xdrs3,
			    mb_io_ptr->mbfp3, XDR_ENCODE);
		    }
		else
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_MEMORY_FAIL;
		    }
		}
	    }

	/* else handle single normal files to be opened with mb_fileio_open() */
	else if (mb_io_ptr->filetype == MB_FILETYPE_SINGLE)
	    {
	    status = mb_fileio_open(verbose, *mbio_ptr, error);
	    }

	/* else handle gsf files to be opened with gsflib */
	else if (mb_io_ptr->filetype == MB_FILETYPE_GSF)
	    {
	    status = gsfOpen(mb_io_ptr->file,
				GSF_CREATE,
				(int *) &(mb_io_ptr->gsfid));
	    if (status == 0)
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
	    else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_OPEN_FAIL;
		}
	    }

	/* else handle netcdf files to be opened with libnetcdf */
	else if (mb_io_ptr->filetype == MB_FILETYPE_NETCDF)
	    {
	    status = nc_create(mb_io_ptr->file,
				NC_CLOBBER,
				(int *) &(mb_io_ptr->ncid));
	    if (status == 0)
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
	    else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_OPEN_FAIL;
		}
	    }

	/* else handle surf files to be opened with libsapi */
	else if (mb_io_ptr->filetype == MB_FILETYPE_SURF)
	    {
	    lastslash = strrchr(file, '/');
	    if (lastslash != NULL && strlen(lastslash) > 1)
	    	{
		strcpy(name,&(lastslash[1]));
		strcpy(path,file);
		path[strlen(file) - strlen(lastslash)] = '\0';
		}
	    else if (strlen(file) > 0)
	    	{
		strcpy(path, ".");
		strcpy(name, file);
		}
	    else
	     	{
		status = MB_FAILURE;
		*error = MB_ERROR_OPEN_FAIL;
		}
	    if (status == MB_SUCCESS)
	    	{
		if (strcmp(&name[strlen(name)-4],".sda") == 0)
			name[strlen(name)-4] = '\0';
		else if (strcmp(&name[strlen(name)-4],".SDA") == 0)
			name[strlen(name)-4] = '\0';
		else if (strcmp(&name[strlen(name)-4],".six") == 0)
			name[strlen(name)-4] = '\0';
		else if (strcmp(&name[strlen(name)-4],".SIX") == 0)
			name[strlen(name)-4] = '\0';
	    	sapi_status = SAPI_open(path,name,verbose);
	    	if (sapi_status == 0)
			{
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			}
	    	else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_OPEN_FAIL;
			}
		}
	    else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_OPEN_FAIL;
		}
	    }

	/* else handle segy files to be opened with mb_segy */
	else if (mb_io_ptr->filetype == MB_FILETYPE_SEGY)
	    {
	    status = mb_segy_write_init(verbose, mb_io_ptr->file,
		NULL, NULL, (void **)&(mb_io_ptr->mbfp), error);
	    if (status != MB_SUCCESS)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_OPEN_FAIL;
		}
	    }

	/* if error terminate */
	if (status == MB_FAILURE)
		{
		/* save status and error values */
		status_save = status;
		error_save = *error;

		/* free allocated memory */
		if (mb_io_ptr->filetype == MB_FILETYPE_XDR
		    && mb_io_ptr->xdrs != NULL)
			status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->xdrs,error);
		if (mb_io_ptr->filetype == MB_FILETYPE_XDR
		    && mb_io_ptr->xdrs2 != NULL)
			status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->xdrs2,error);
		if (mb_io_ptr->filetype == MB_FILETYPE_XDR
		    && mb_io_ptr->xdrs3 != NULL)
			status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->xdrs3,error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->beamflag,error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->bath,error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->amp,error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->bath_acrosstrack,
						error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->bath_alongtrack,
						error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->bath_num,error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->amp_num,error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->ss,error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->ss_acrosstrack,
						error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->ss_alongtrack,
						error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->ss_num,error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->new_beamflag,error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->new_bath,error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->new_amp,error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&
					mb_io_ptr->new_bath_acrosstrack,error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->new_bath_alongtrack,
					error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->new_ss,error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->new_ss_acrosstrack,
					error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->new_ss_alongtrack,
					error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr,error);

		/* restore error and status values */
		if (status == MB_SUCCESS)
			{
			status = status_save;
			*error = error_save;
			}

		/* output debug message */
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

	/* initialize the working variables */
	mb_io_ptr->ping_count = 0;
	mb_io_ptr->nav_count = 0;
	mb_io_ptr->comment_count = 0;
	mb_io_ptr->pings_avg = 1;
	mb_io_ptr->pings_read = 0;
	mb_io_ptr->error_save = MB_ERROR_NO_ERROR;
	mb_io_ptr->last_time_d = 0.0;
	mb_io_ptr->last_lon = 0.0;
	mb_io_ptr->last_lat = 0.0;
	mb_io_ptr->old_time_d = 0.0;
	mb_io_ptr->old_lon = 0.0;
	mb_io_ptr->old_lat = 0.0;
	mb_io_ptr->old_ntime_d = 0.0;
	mb_io_ptr->old_nlon = 0.0;
	mb_io_ptr->old_nlat = 0.0;
	mb_io_ptr->time_d = 0.0;
	mb_io_ptr->lon = 0.0;
	mb_io_ptr->lat = 0.0;
	mb_io_ptr->speed = 0.0;
	mb_io_ptr->heading = 0.0;
	for (i=0;i<mb_io_ptr->beams_bath_max;i++)
		{
		mb_io_ptr->beamflag[i] = MB_FLAG_NULL;
		mb_io_ptr->bath[i] = 0.0;
		mb_io_ptr->bath_acrosstrack[i] = 0.0;
		mb_io_ptr->bath_alongtrack[i] = 0.0;
		mb_io_ptr->bath_num[i] = 0;
		}
	for (i=0;i<mb_io_ptr->beams_amp_max;i++)
		{
		mb_io_ptr->amp[i] = 0.0;
		mb_io_ptr->amp_num[i] = 0;
		}
	for (i=0;i<mb_io_ptr->pixels_ss_max;i++)
		{
		mb_io_ptr->ss[i] = 0.0;
		mb_io_ptr->ss_acrosstrack[i] = 0.0;
		mb_io_ptr->ss_alongtrack[i] = 0.0;
		mb_io_ptr->ss_num[i] = 0;
		}
	mb_io_ptr->need_new_ping = MB_YES;

	/* initialize variables for interpolating asynchronous data */
	mb_io_ptr->nfix = 0;
	mb_io_ptr->nattitude = 0;
	mb_io_ptr->nheading = 0;
	mb_io_ptr->nsonardepth = 0;
	mb_io_ptr->naltitude = 0;
	for (i=0;i<MB_ASYNCH_SAVE_MAX;i++)
		{
		mb_io_ptr->fix_time_d[i] = 0.0;
		mb_io_ptr->fix_lon[i] = 0.0;
		mb_io_ptr->fix_lat[i] = 0.0;
		mb_io_ptr->attitude_time_d[i] = 0.0;
		mb_io_ptr->attitude_heave[i] = 0.0;
		mb_io_ptr->attitude_roll[i] = 0.0;
		mb_io_ptr->attitude_pitch[i] = 0.0;
		mb_io_ptr->heading_time_d[i] = 0.0;
		mb_io_ptr->heading_heading[i] = 0.0;
		mb_io_ptr->sonardepth_time_d[i] = 0.0;
		mb_io_ptr->sonardepth_sonardepth[i] = 0.0;
		mb_io_ptr->altitude_time_d[i] = 0.0;
		mb_io_ptr->altitude_altitude[i] = 0.0;
		}

	/* initialize notices */
	for (i=0;i<MB_NOTICE_MAX;i++)
		mb_io_ptr->notice_list[i] = 0;

	/* set error and status (if you got here you succeeded */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)*mbio_ptr);
		fprintf(stderr,"dbg2       beams_bath: %d\n",*beams_bath);
		fprintf(stderr,"dbg2       beams_amp:  %d\n",*beams_amp);
		fprintf(stderr,"dbg2       pixels_ss:  %d\n",*pixels_ss);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
