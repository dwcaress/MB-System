/*--------------------------------------------------------------------
 *    The MB-system:	mb_write_init.c	3.00	1/25/93
 *    $Id: mb_write_init.c,v 3.0 1993-04-23 19:08:53 dale Exp $
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
 * mb_write_init.c opens and initializes a multibeam data file 
 * for writing with mb_write or mb_put.
 *
 * Author:	D. W. Caress
 * Date:	January 25, 1993
 *
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

/*--------------------------------------------------------------------*/
int mb_write_init(verbose,file,format,
		mbio_ptr,beams_bath,beams_back,error)
int	verbose;
char	*file;
int	format;
char	**mbio_ptr;
int	*beams_bath;
int	*beams_back;
int	*error;
{
  char rcs_id[]="$Id";
  char	*function_name = "mb_write_init";
  int	status = MB_SUCCESS;
  struct mb_io_struct *mb_io_ptr;
	int	i;
	char	*stdout_string = "stdout";

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       file:       %s\n",file);
		fprintf(stderr,"dbg2       format:     %d\n",format);
		}

	/* allocate memory for mbio descriptor */
	status = mb_malloc(verbose,sizeof(struct mb_io_struct),
				&mb_io_ptr,error);
	if (status == MB_FAILURE)
		{
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
	*mbio_ptr = (char *) mb_io_ptr;

	/* load control parameters into the mbio descriptor */
	strcpy(mb_io_ptr->file,file);
	mb_io_ptr->usage = MB_WRITE_ONLY;
	mb_io_ptr->format = format;
	mb_io_ptr->pings = 0;
	mb_io_ptr->lonflip = 0;
	for (i=0;i<4;i++)
		mb_io_ptr->bounds[i] = 0;
	for (i=0;i<6;i++)
		{
		mb_io_ptr->btime_i[i] = 0;
		mb_io_ptr->etime_i[i] = 0;
		}
	mb_io_ptr->speedmin = 0.0;
	mb_io_ptr->timegap = 0.0;
	mb_io_ptr->btime_d = 0.0;
	mb_io_ptr->etime_d = 0.0;

	/* set the number of beams and allocate storage arrays */	
	*beams_bath = beams_bath_table[format];
	*beams_back = beams_back_table[format];
	mb_io_ptr->beams_bath = *beams_bath;
	mb_io_ptr->beams_back = *beams_back;
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_bath*sizeof(int),
				&mb_io_ptr->bath,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_bath*sizeof(int),
				&mb_io_ptr->bathdist,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_bath*sizeof(int),
				&mb_io_ptr->bathnum,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_back*sizeof(int),
				&mb_io_ptr->back,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_back*sizeof(int),
				&mb_io_ptr->backdist,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_back*sizeof(int),
				&mb_io_ptr->backnum,error);

	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_bath*sizeof(int),
				&mb_io_ptr->new_bath,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_bath*sizeof(int),
				&mb_io_ptr->new_bathdist,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_back*sizeof(int),
				&mb_io_ptr->new_back,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_back*sizeof(int),
				&mb_io_ptr->new_backdist,error);
	if (status == MB_FAILURE)
		{
		status = mb_free(verbose,mb_io_ptr->bath,error);
		status = mb_free(verbose,mb_io_ptr->bathdist,error);
		status = mb_free(verbose,mb_io_ptr->bathnum,error);
		status = mb_free(verbose,mb_io_ptr->back,error);
		status = mb_free(verbose,mb_io_ptr->backdist,error);
		status = mb_free(verbose,mb_io_ptr->backnum,error);
		status = mb_free(verbose,mb_io_ptr->new_bath,error);
		status = mb_free(verbose,mb_io_ptr->new_bathdist,error);
		status = mb_free(verbose,mb_io_ptr->new_back,error);
		status = mb_free(verbose,mb_io_ptr->new_backdist,error);
		status = mb_free(verbose,mb_io_ptr,error);
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

	/* call routine to allocate memory for format dependent i/o */
	if ((status = mb_mem_init(verbose,*mbio_ptr,error)) == MB_FAILURE)
		{
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

	/* open the file */
	if (strncmp(file,stdout_string,6) == 0)
		mb_io_ptr->mbfp = stdout;
	else
		if ((mb_io_ptr->mbfp = fopen(file, "w")) == NULL) 
			{
			*error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
			status = mb_free(verbose,mb_io_ptr->bath,error);
			status = mb_free(verbose,mb_io_ptr->bathdist,error);
			status = mb_free(verbose,mb_io_ptr->bathnum,error);
			status = mb_free(verbose,mb_io_ptr->back,error);
			status = mb_free(verbose,mb_io_ptr->backdist,error);
			status = mb_free(verbose,mb_io_ptr->backnum,error);
			status = mb_free(verbose,mb_io_ptr,error);
			if (verbose >= 2)
				{
				fprintf(stderr,"\ndbg2  MBIO function <%s> terminated with error\n",
					function_name);
				fprintf(stderr,"dbg2  Return values:\n");
				fprintf(stderr,"dbg2       error:      %d\n",
					*error);
				fprintf(stderr,"dbg2  Return status:\n");
				fprintf(stderr,"dbg2       status:  %d\n",
					status);
				}
			return(status);
			}

	/* initialize the working variables */
	mb_io_ptr->ping_count = 0;
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
	mb_io_ptr->time_d = 0.0;
	mb_io_ptr->lon = 0.0;
	mb_io_ptr->lat = 0.0;
	mb_io_ptr->speed = 0.0;
	mb_io_ptr->heading = 0.0;
	for (i=0;i<mb_io_ptr->beams_bath;i++)
		{
		mb_io_ptr->bath[i] = 0;
		mb_io_ptr->bathdist[i] = 0;
		mb_io_ptr->bathnum[i] = 0;
		}
	for (i=0;i<mb_io_ptr->beams_back;i++)
		{
		mb_io_ptr->back[i] = 0;
		mb_io_ptr->backdist[i] = 0;
		mb_io_ptr->backnum[i] = 0;
		}
	mb_io_ptr->need_new_ping = MB_YES;

	/* set error and status (if you got here you succeeded */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",*mbio_ptr);
		fprintf(stderr,"dbg2       beams_bath: %d\n",*beams_bath);
		fprintf(stderr,"dbg2       beams_back: %d\n",*beams_back);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
