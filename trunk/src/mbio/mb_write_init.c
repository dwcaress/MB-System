/*--------------------------------------------------------------------
 *    The MB-system:	mb_write_init.c	1/25/93
 *    $Id: mb_write_init.c,v 4.0 1994-03-06 00:01:56 caress Exp $
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
 * mb_write_init.c opens and initializes a multibeam data file 
 * for writing with mb_write or mb_put.
 *
 * Author:	D. W. Caress
 * Date:	January 25, 1993
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.3  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.2  1994/02/22  23:50:09  caress
 * Added some debug messages.
 *
 * Revision 4.1  1994/02/20  03:16:28  caress
 * Fixed errors lines 98 and 283.
 *
 * Revision 4.0  1994/02/20  02:09:29  caress
 * First cut at new version. Includes new handling of
 * sidescan and amplitude data.
 *
 * Revision 3.1  1993/05/14  22:45:27  sohara
 * fixed rcs_id message
 *
 * Revision 3.0  1993/04/23  19:08:53  dale
 * Initial version
 *
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
		mbio_ptr,beams_bath,beams_amp,pixels_ss,error)
int	verbose;
char	*file;
int	format;
char	**mbio_ptr;
int	*beams_bath;
int	*beams_amp;
int	*pixels_ss;
int	*error;
{
	static char rcs_id[]="$Id: mb_write_init.c,v 4.0 1994-03-06 00:01:56 caress Exp $";
	char	*function_name = "mb_write_init";
	int	status = MB_SUCCESS;
	int	format_num;
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

	/* check validity of format */
	status = mb_format(verbose,&format,&format_num,error);

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
	mb_io_ptr->format = format;
	mb_io_ptr->format_num = format_num;
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
	*beams_bath = beams_bath_table[format_num];
	*beams_amp = beams_amp_table[format_num];
	*pixels_ss = beams_ss_table[format_num];
	mb_io_ptr->beams_bath = *beams_bath;
	mb_io_ptr->beams_amp = *beams_amp;
	mb_io_ptr->pixels_ss = *pixels_ss;
	if (verbose >= 4)
		{
		fprintf(stderr,"\ndbg4  Beam and pixel dimensions set in MBIO function <%s>\n",
				function_name);
		fprintf(stderr,"dbg4       beams_bath: %d\n",
			mb_io_ptr->beams_bath);
		fprintf(stderr,"dbg4       beams_amp:  %d\n",
			mb_io_ptr->beams_amp);
		fprintf(stderr,"dbg4       pixels_ss:  %d\n",
			mb_io_ptr->pixels_ss);
		}

	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_bath*sizeof(int),
				&mb_io_ptr->bath,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_amp*sizeof(int),
				&mb_io_ptr->amp,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_bath*sizeof(int),
				&mb_io_ptr->bath_acrosstrack,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_bath*sizeof(int),
				&mb_io_ptr->bath_alongtrack,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_bath*sizeof(int),
				&mb_io_ptr->bath_num,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_amp*sizeof(int),
				&mb_io_ptr->amp_num,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->pixels_ss*sizeof(int),
				&mb_io_ptr->ss,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->pixels_ss*sizeof(int),
				&mb_io_ptr->ss_acrosstrack,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->pixels_ss*sizeof(int),
				&mb_io_ptr->ss_alongtrack,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->pixels_ss*sizeof(int),
				&mb_io_ptr->ss_num,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_bath*sizeof(int),
				&mb_io_ptr->new_bath,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_amp*sizeof(int),
				&mb_io_ptr->new_amp,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_bath*sizeof(int),
				&mb_io_ptr->new_bath_acrosstrack,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_bath*sizeof(int),
				&mb_io_ptr->new_bath_alongtrack,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->pixels_ss*sizeof(int),
				&mb_io_ptr->new_ss,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->pixels_ss*sizeof(int),
				&mb_io_ptr->new_ss_acrosstrack,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->pixels_ss*sizeof(int),
				&mb_io_ptr->new_ss_alongtrack,error);

	/* call routine to allocate memory for format dependent i/o */
	if (status == MB_SUCCESS)
		status = mb_mem_init(verbose,*mbio_ptr,error);

	/* deal with a memory allocation failure */
	if (status == MB_FAILURE)
		{
		status = mb_free(verbose,mb_io_ptr->bath,error);
		status = mb_free(verbose,mb_io_ptr->amp,error);
		status = mb_free(verbose,mb_io_ptr->bath_acrosstrack,error);
		status = mb_free(verbose,mb_io_ptr->bath_alongtrack,error);
		status = mb_free(verbose,mb_io_ptr->bath_num,error);
		status = mb_free(verbose,mb_io_ptr->amp_num,error);
		status = mb_free(verbose,mb_io_ptr->ss,error);
		status = mb_free(verbose,mb_io_ptr->ss_acrosstrack,error);
		status = mb_free(verbose,mb_io_ptr->ss_alongtrack,error);
		status = mb_free(verbose,mb_io_ptr->ss_num,error);
		status = mb_free(verbose,mb_io_ptr->new_bath,error);
		status = mb_free(verbose,mb_io_ptr->new_amp,error);
		status = mb_free(verbose,mb_io_ptr->new_bath_acrosstrack,error);
		status = mb_free(verbose,mb_io_ptr->new_bath_alongtrack,error);
		status = mb_free(verbose,mb_io_ptr->new_ss,error);
		status = mb_free(verbose,mb_io_ptr->new_ss_acrosstrack,error);
		status = mb_free(verbose,mb_io_ptr->new_ss_alongtrack,error);
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

	/* open the file */
	if (strncmp(file,stdout_string,6) == 0)
		mb_io_ptr->mbfp = stdout;
	else
		if ((mb_io_ptr->mbfp = fopen(file, "w")) == NULL) 
			{
			*error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
			status = mb_free(verbose,mb_io_ptr->bath,error);
			status = mb_free(verbose,mb_io_ptr->amp,error);
			status = mb_free(verbose,mb_io_ptr->bath_acrosstrack,
						error);
			status = mb_free(verbose,mb_io_ptr->bath_alongtrack,
						error);
			status = mb_free(verbose,mb_io_ptr->bath_num,error);
			status = mb_free(verbose,mb_io_ptr->amp_num,error);
			status = mb_free(verbose,mb_io_ptr->ss,error);
			status = mb_free(verbose,mb_io_ptr->ss_acrosstrack,
						error);
			status = mb_free(verbose,mb_io_ptr->ss_alongtrack,
						error);
			status = mb_free(verbose,mb_io_ptr->ss_num,error);
			status = mb_free(verbose,mb_io_ptr->new_bath,error);
			status = mb_free(verbose,mb_io_ptr->new_amp,error);
			status = mb_free(verbose,
					mb_io_ptr->new_bath_acrosstrack,error);
			status = mb_free(verbose,mb_io_ptr->new_bath_alongtrack,
					error);
			status = mb_free(verbose,mb_io_ptr->new_ss,error);
			status = mb_free(verbose,mb_io_ptr->new_ss_acrosstrack,
					error);
			status = mb_free(verbose,mb_io_ptr->new_ss_alongtrack,
					error);
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
		mb_io_ptr->bath_acrosstrack[i] = 0;
		mb_io_ptr->bath_alongtrack[i] = 0;
		mb_io_ptr->bath_num[i] = 0;
		}
	for (i=0;i<mb_io_ptr->beams_amp;i++)
		{
		mb_io_ptr->amp[i] = 0;
		mb_io_ptr->amp_num[i] = 0;
		}
	for (i=0;i<mb_io_ptr->pixels_ss;i++)
		{
		mb_io_ptr->ss[i] = 0;
		mb_io_ptr->ss_acrosstrack[i] = 0;
		mb_io_ptr->ss_alongtrack[i] = 0;
		mb_io_ptr->ss_num[i] = 0;
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
