/*--------------------------------------------------------------------
 *    The MB-system:	mb_read_init.c	1/25/93
 *    $Id: mb_read_init.c,v 4.2 1994-07-29 18:46:51 caress Exp $
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
 * mb_read_init.c opens and initializes a multibeam data file 
 * for reading with mb_read or mb_get.
 *
 * Author:	D. W. Caress
 * Date:	January 25, 1993
 * 
 * $Log: not supported by cvs2svn $
 * Revision 4.1  1994/04/21  21:02:39  caress
 * Fixed bug so file open errors are passed back to calling function.
 *
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
 * Revision 4.8  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.7  1994/03/03  03:15:16  caress
 * Fixed a minor bug.
 *
 * Revision 4.6  1994/02/20  03:15:01  caress
 * Fixed error line 336.
 *
 * Revision 4.5  1994/02/20  02:08:54  caress
 * Fixed closing debug statements.
 *
 * Revision 4.4  1994/02/20  02:03:18  caress
 * Removed spurious line of code.
 *
 * Revision 4.3  1994/02/20  02:00:53  caress
 * Added tabs in front of some variable definitions.
 *
 * Revision 4.2  1994/02/18  21:21:15  caress
 * Added amp_num array.
 *
 * Revision 4.1  1994/02/18  20:38:07  caress
 * Changed some of the variables missed in the previous cut.
 *
 * Revision 4.0  1994/02/18  18:41:34  caress
 * First cut at new version.  Added sidescan and associated
 * changes.  Added indirect format id's.
 *
 * Revision 3.1  1993/05/14  22:38:32  sohara
 * fixed rcs_id message
 *
 * Revision 3.0  1993/04/23  18:45:53  dale
 * Initial version
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <strings.h>

/* XDR i/o include file */
#include <rpc/xdr.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_io.h"

/*--------------------------------------------------------------------*/
int mb_read_init(verbose,file,format,pings,lonflip,bounds,
		btime_i,etime_i,speedmin,timegap,
		mbio_ptr,btime_d,etime_d,
		beams_bath,beams_amp,pixels_ss,error)
int	verbose;
char	*file;
int	format;
int	pings;
int	lonflip;
double	bounds[4];
int	btime_i[6];
int	etime_i[6];
double	speedmin;
double	timegap;
char	**mbio_ptr;
double	*btime_d;
double	*etime_d;
int	*beams_bath;
int	*beams_amp;
int	*pixels_ss;
int	*error;
{
	static char rcs_id[]="$Id: mb_read_init.c,v 4.2 1994-07-29 18:46:51 caress Exp $";
	char	*function_name = "mb_read_init";
	int	status;
	int	format_num;
	struct mb_io_struct *mb_io_ptr;
	int	i;
	char	*stdin_string = "stdin";

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       file:       %s\n",file);
		fprintf(stderr,"dbg2       format:     %d\n",format);
		fprintf(stderr,"dbg2       pings:      %d\n",pings);
		fprintf(stderr,"dbg2       lonflip:    %d\n",lonflip);
		fprintf(stderr,"dbg2       bounds[0]:  %f\n",bounds[0]);
		fprintf(stderr,"dbg2       bounds[1]:  %f\n",bounds[1]);
		fprintf(stderr,"dbg2       bounds[2]:  %f\n",bounds[2]);
		fprintf(stderr,"dbg2       bounds[3]:  %f\n",bounds[3]);
		fprintf(stderr,"dbg2       btime_i[0]: %d\n",btime_i[0]);
		fprintf(stderr,"dbg2       btime_i[1]: %d\n",btime_i[1]);
		fprintf(stderr,"dbg2       btime_i[2]: %d\n",btime_i[2]);
		fprintf(stderr,"dbg2       btime_i[3]: %d\n",btime_i[3]);
		fprintf(stderr,"dbg2       btime_i[4]: %d\n",btime_i[4]);
		fprintf(stderr,"dbg2       btime_i[5]: %d\n",btime_i[5]);
		fprintf(stderr,"dbg2       etime_i[0]: %d\n",etime_i[0]);
		fprintf(stderr,"dbg2       etime_i[1]: %d\n",etime_i[1]);
		fprintf(stderr,"dbg2       etime_i[2]: %d\n",etime_i[2]);
		fprintf(stderr,"dbg2       etime_i[3]: %d\n",etime_i[3]);
		fprintf(stderr,"dbg2       etime_i[4]: %d\n",etime_i[4]);
		fprintf(stderr,"dbg2       etime_i[5]: %d\n",etime_i[5]);
		fprintf(stderr,"dbg2       speedmin:   %f\n",speedmin);
		fprintf(stderr,"dbg2       timegap:    %f\n",timegap);
		}

	/* check validity of format */
	status = mb_format(verbose,&format,&format_num,error);

	/* allocate memory for mbio descriptor */
	if (status == MB_SUCCESS)
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
	mb_io_ptr->pings = pings;
	mb_io_ptr->lonflip = lonflip;
	for (i=0;i<4;i++)
		mb_io_ptr->bounds[i] = bounds[i];
	for (i=0;i<6;i++)
		{
		mb_io_ptr->btime_i[i] = btime_i[i];
		mb_io_ptr->etime_i[i] = etime_i[i];
		}
	mb_io_ptr->speedmin = speedmin;
	mb_io_ptr->timegap = timegap;

	/* get mbio internal time */
	status = mb_get_time(verbose,mb_io_ptr->btime_i,btime_d);
	status = mb_get_time(verbose,mb_io_ptr->etime_i,etime_d);
	mb_io_ptr->btime_d = *btime_d;
	mb_io_ptr->etime_d = *etime_d;

	/* set the number of beams and allocate storage arrays */	
	*beams_bath = beams_bath_table[format_num];
	*beams_amp = beams_amp_table[format_num];
	*pixels_ss = beams_ss_table[format_num];
	mb_io_ptr->beams_bath = *beams_bath;
	mb_io_ptr->beams_amp = *beams_amp;
	mb_io_ptr->pixels_ss = *pixels_ss;
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
	if (strncmp(file,stdin_string,5) == 0)
		mb_io_ptr->mbfp = stdin;
	else
		if ((mb_io_ptr->mbfp = fopen(file, "r")) == NULL) 
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
			xdrstdio_create(mb_io_ptr->xdrs, 
				mb_io_ptr->mbfp, XDR_DECODE);
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
		if (mb_xdr_table[format_num] == MB_YES)
			status = mb_free(verbose,mb_io_ptr->xdrs,error);
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
		*error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
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
	if (pings == 0)
		mb_io_ptr->pings_avg = 2;
	else
		mb_io_ptr->pings_avg = pings;
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
		fprintf(stderr,"dbg2       btime_d:    %f\n",*btime_d);
		fprintf(stderr,"dbg2       etime_d:    %f\n",*etime_d);
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
