/*--------------------------------------------------------------------
 *    The MB-system:	mb_write_init.c	1/25/93
 *    $Id: mb_write_init.c,v 4.12 1997-07-25 14:19:53 caress Exp $
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
 * Revision 4.11  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.11  1997/04/17  18:48:52  caress
 * Added LINUX ifdef.
 *
 * Revision 4.10  1996/08/26  17:24:56  caress
 * Release 4.4 revision.
 *
 * Revision 4.9  1996/08/05  15:21:58  caress
 * Just redid i/o for Simrad sonars, including adding EM12S and EM121 support.
 *
 * Revision 4.8  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.7  1995/11/02  19:48:51  caress
 * Fixed error handling.
 *
 * Revision 4.6  1995/03/22  19:14:25  caress
 * Added #ifdef's for HPUX.
 *
 * Revision 4.5  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.4  1995/01/25  17:13:46  caress
 * Added ifdef for SOLARIS.
 *
 * Revision 4.3  1994/10/21  12:11:53  caress
 * Release V4.0
 *
 * Revision 4.2  1994/07/29  18:46:51  caress
 * Changes associated with supporting Lynx OS (byte swapped) and
 * using unix second time base (for time_d values).
 *
 * Revision 4.1  1994/04/21  21:02:39  caress
 * Fixed bug so file open errors are passed back to calling function.
 *
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
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
#include <string.h>

/* XDR i/o include file */
#ifdef IRIX
#include <rpc/rpc.h>
#endif
#ifdef SOLARIS
#include <rpc/rpc.h>
#endif
#ifdef LYNX
#include <rpc/rpc.h>
#endif
#ifdef LINUX
#include <rpc/rpc.h>
#endif
#ifdef SUN
#include <rpc/xdr.h>
#endif
#ifdef HPUX
#include <rpc/rpc.h>
#endif
#ifdef OTHER
#include <rpc/xdr.h>
#endif

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_io.h"
#include "../../include/mb_define.h"

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
	static char rcs_id[]="$Id: mb_write_init.c,v 4.12 1997-07-25 14:19:53 caress Exp $";
	char	*function_name = "mb_write_init";
	int	status = MB_SUCCESS;
	int	format_num;
	struct mb_io_struct *mb_io_ptr;
	int	status_save;
	int	error_save;
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
				mbio_ptr,error);
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
	mb_io_ptr = (struct mb_io_struct *) *mbio_ptr;

	/* initialize file access for the mbio descriptor */
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
	mb_io_ptr->xdrs = NULL;

	/* load control parameters into the mbio descriptor */
	mb_io_ptr->format = format;
	mb_io_ptr->format_num = format_num;
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
	*beams_bath = beams_bath_table[format_num];
	*beams_amp = beams_amp_table[format_num];
	*pixels_ss = pixels_ss_table[format_num];
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

	/* initialize pointers */
	mb_io_ptr->raw_data = NULL;
	mb_io_ptr->store_data = NULL;
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
	mb_io_ptr->new_bath = NULL;
	mb_io_ptr->new_amp = NULL;
	mb_io_ptr->new_bath_acrosstrack = NULL;
	mb_io_ptr->new_bath_alongtrack = NULL;
	mb_io_ptr->new_ss = NULL;
	mb_io_ptr->new_ss_acrosstrack = NULL;
	mb_io_ptr->new_ss_alongtrack = NULL;
	
	/* initialize ancillary variables used
		to save information in certain cases */
	mb_io_ptr->save_flag = MB_NO;
	mb_io_ptr->save_label_flag = MB_NO;

	/* allocate arrays */
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_bath*sizeof(double),
				&mb_io_ptr->bath,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_amp*sizeof(double),
				&mb_io_ptr->amp,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_bath*sizeof(double),
				&mb_io_ptr->bath_acrosstrack,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_bath*sizeof(double),
				&mb_io_ptr->bath_alongtrack,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_bath*sizeof(int),
				&mb_io_ptr->bath_num,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_amp*sizeof(int),
				&mb_io_ptr->amp_num,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->pixels_ss*sizeof(double),
				&mb_io_ptr->ss,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->pixels_ss*sizeof(double),
				&mb_io_ptr->ss_acrosstrack,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->pixels_ss*sizeof(double),
				&mb_io_ptr->ss_alongtrack,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->pixels_ss*sizeof(int),
				&mb_io_ptr->ss_num,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_bath*sizeof(double),
				&mb_io_ptr->new_bath,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_amp*sizeof(double),
				&mb_io_ptr->new_amp,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_bath*sizeof(double),
				&mb_io_ptr->new_bath_acrosstrack,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_bath*sizeof(double),
				&mb_io_ptr->new_bath_alongtrack,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->pixels_ss*sizeof(double),
				&mb_io_ptr->new_ss,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->pixels_ss*sizeof(double),
				&mb_io_ptr->new_ss_acrosstrack,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->pixels_ss*sizeof(double),
				&mb_io_ptr->new_ss_alongtrack,error);

	/* call routine to allocate memory for format dependent i/o */
	if (status == MB_SUCCESS)
		status = mb_mem_init(verbose,*mbio_ptr,error);

	/* deal with a memory allocation failure */
	if (status == MB_FAILURE)
		{
		status = mb_free(verbose,&mb_io_ptr->bath,error);
		status = mb_free(verbose,&mb_io_ptr->amp,error);
		status = mb_free(verbose,&mb_io_ptr->bath_acrosstrack,error);
		status = mb_free(verbose,&mb_io_ptr->bath_alongtrack,error);
		status = mb_free(verbose,&mb_io_ptr->bath_num,error);
		status = mb_free(verbose,&mb_io_ptr->amp_num,error);
		status = mb_free(verbose,&mb_io_ptr->ss,error);
		status = mb_free(verbose,&mb_io_ptr->ss_acrosstrack,error);
		status = mb_free(verbose,&mb_io_ptr->ss_alongtrack,error);
		status = mb_free(verbose,&mb_io_ptr->ss_num,error);
		status = mb_free(verbose,&mb_io_ptr->new_bath,error);
		status = mb_free(verbose,&mb_io_ptr->new_amp,error);
		status = mb_free(verbose,&mb_io_ptr->new_bath_acrosstrack,error);
		status = mb_free(verbose,&mb_io_ptr->new_bath_alongtrack,error);
		status = mb_free(verbose,&mb_io_ptr->new_ss,error);
		status = mb_free(verbose,&mb_io_ptr->new_ss_acrosstrack,error);
		status = mb_free(verbose,&mb_io_ptr->new_ss_alongtrack,error);
		status = mb_free(verbose,&mb_io_ptr,error);
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

	/* open the first file */
	if (strncmp(file,stdout_string,6) == 0)
		mb_io_ptr->mbfp = stdout;
	else
		if ((mb_io_ptr->mbfp = fopen(mb_io_ptr->file, "w")) == NULL) 
			{
			*error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
			}

	/* open the second file if required */
	if (status == MB_SUCCESS 
		&& mb_numfile_table[format_num] >= 2)
		if ((mb_io_ptr->mbfp2 = fopen(mb_io_ptr->file2, "w")) == NULL) 
			{
			*error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
			}

	/* open the third file if required */
	if (status == MB_SUCCESS 
		&& mb_numfile_table[format_num] >= 3)
		if ((mb_io_ptr->mbfp3 = fopen(mb_io_ptr->file3, "w")) == NULL) 
			{
			*error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
			}

	/* if needed, initialize XDR stream */
	if (status == MB_SUCCESS && mb_xdr_table[format_num] == MB_YES)
		{
		status = mb_malloc(verbose,sizeof(XDR),
				&mb_io_ptr->xdrs,error);
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

	/* if error terminate */
	if (status == MB_FAILURE)
		{
		/* save status and error values */
		status_save = status;
		error_save = *error;

		/* free allocated memory */
		if (mb_xdr_table[format_num] == MB_YES)
			status = mb_free(verbose,&mb_io_ptr->xdrs,error);
		status = mb_free(verbose,&mb_io_ptr->bath,error);
		status = mb_free(verbose,&mb_io_ptr->amp,error);
		status = mb_free(verbose,&mb_io_ptr->bath_acrosstrack,
						error);
		status = mb_free(verbose,&mb_io_ptr->bath_alongtrack,
						error);
		status = mb_free(verbose,&mb_io_ptr->bath_num,error);
		status = mb_free(verbose,&mb_io_ptr->amp_num,error);
		status = mb_free(verbose,&mb_io_ptr->ss,error);
		status = mb_free(verbose,&mb_io_ptr->ss_acrosstrack,
						error);
		status = mb_free(verbose,&mb_io_ptr->ss_alongtrack,
						error);
		status = mb_free(verbose,&mb_io_ptr->ss_num,error);
		status = mb_free(verbose,&mb_io_ptr->new_bath,error);
		status = mb_free(verbose,&mb_io_ptr->new_amp,error);
		status = mb_free(verbose,&
					mb_io_ptr->new_bath_acrosstrack,error);
		status = mb_free(verbose,&mb_io_ptr->new_bath_alongtrack,
					error);
		status = mb_free(verbose,&mb_io_ptr->new_ss,error);
		status = mb_free(verbose,&mb_io_ptr->new_ss_acrosstrack,
					error);
		status = mb_free(verbose,&mb_io_ptr->new_ss_alongtrack,
					error);
		status = mb_free(verbose,&mb_io_ptr,error);

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
		mb_io_ptr->bath[i] = 0.0;
		mb_io_ptr->bath_acrosstrack[i] = 0.0;
		mb_io_ptr->bath_alongtrack[i] = 0.0;
		mb_io_ptr->bath_num[i] = 0;
		}
	for (i=0;i<mb_io_ptr->beams_amp;i++)
		{
		mb_io_ptr->amp[i] = 0.0;
		mb_io_ptr->amp_num[i] = 0;
		}
	for (i=0;i<mb_io_ptr->pixels_ss;i++)
		{
		mb_io_ptr->ss[i] = 0.0;
		mb_io_ptr->ss_acrosstrack[i] = 0.0;
		mb_io_ptr->ss_alongtrack[i] = 0.0;
		mb_io_ptr->ss_num[i] = 0;
		}
	mb_io_ptr->need_new_ping = MB_YES;

	/* initialize variables for extrapolating navigation */
	mb_io_ptr->nfix = 0;
	for (i=0;i<5;i++)
		{
		mb_io_ptr->fix_time_d[i] = 0.0;
		mb_io_ptr->fix_lon[i] = 0.0;
		mb_io_ptr->fix_lat[i] = 0.0;
		}

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
