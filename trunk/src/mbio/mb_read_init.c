/*--------------------------------------------------------------------
 *    The MB-system:	mb_read_init.c	1/25/93
 *    $Id: mb_read_init.c,v 5.4 2001-06-29 22:48:04 caress Exp $
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
 * mb_read_init.c opens and initializes a multibeam data file 
 * for reading with mb_read or mb_get.
 *
 * Author:	D. W. Caress
 * Date:	January 25, 1993
 * 
 * $Log: not supported by cvs2svn $
 * Revision 5.3  2001/06/08  21:44:01  caress
 * Version 5.0.beta01
 *
 * Revision 5.2  2001/03/22  20:45:56  caress
 * Trying to make 5.0.beta0...
 *
 * Revision 5.1  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.16  2000/10/11  01:02:30  caress
 * Convert to ANSI C
 *
 * Revision 4.15  2000/09/30  06:32:11  caress
 * Snapshot for Dale.
 *
 * Revision 4.14  1999/03/31  18:11:35  caress
 * MB-System 4.6beta7
 *
 * Revision 4.13  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.12  1997/07/25  14:19:53  caress
 * Version 4.5beta2.
 * Much mucking, particularly with Simrad formats.
 *
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
#include <string.h>

/* XDR i/o include file */
#ifdef IRIX
#include <rpc/rpc.h>
#endif
#ifdef IRIX64
#include <rpc/rpc.h>
#endif
#ifdef SOLARIS
#include <rpc/rpc.h>
#endif
#ifdef LINUX
#include <rpc/rpc.h>
#endif
#ifdef LYNX
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
#include "../../include/gsf.h"

/*--------------------------------------------------------------------*/
int mb_read_init(int verbose, char *file, 
		int format, int pings, int lonflip, double bounds[4],
		int btime_i[7], int etime_i[7], 
		double speedmin, double timegap,
		char **mbio_ptr, double *btime_d, double *etime_d,
		int *beams_bath, int *beams_amp, int *pixels_ss, 
		int *error)
{
	static char rcs_id[]="$Id: mb_read_init.c,v 5.4 2001-06-29 22:48:04 caress Exp $";
	char	*function_name = "mb_read_init";
	int	status;
	struct mb_io_struct *mb_io_ptr;
	int	status_save;
	int	error_save;
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
		fprintf(stderr,"dbg2       btime_i[6]: %d\n",btime_i[6]);
		fprintf(stderr,"dbg2       etime_i[0]: %d\n",etime_i[0]);
		fprintf(stderr,"dbg2       etime_i[1]: %d\n",etime_i[1]);
		fprintf(stderr,"dbg2       etime_i[2]: %d\n",etime_i[2]);
		fprintf(stderr,"dbg2       etime_i[3]: %d\n",etime_i[3]);
		fprintf(stderr,"dbg2       etime_i[4]: %d\n",etime_i[4]);
		fprintf(stderr,"dbg2       etime_i[5]: %d\n",etime_i[5]);
		fprintf(stderr,"dbg2       etime_i[6]: %d\n",etime_i[6]);
		fprintf(stderr,"dbg2       speedmin:   %f\n",speedmin);
		fprintf(stderr,"dbg2       timegap:    %f\n",timegap);
		}

	/* allocate memory for mbio descriptor */
	status = mb_malloc(verbose,sizeof(struct mb_io_struct),
				mbio_ptr,error);
	if (status == MB_SUCCESS)
		{
		memset(*mbio_ptr, 0, sizeof(struct mb_io_struct));
		mb_io_ptr = (struct mb_io_struct *) *mbio_ptr;
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
			status = mb_free(verbose,mbio_ptr,error);
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
	mb_io_ptr->xdrs2 = NULL;

	/* load control parameters into the mbio descriptor */
	mb_io_ptr->format = format;
	mb_io_ptr->pings = pings;
	mb_io_ptr->lonflip = lonflip;
	for (i=0;i<4;i++)
		mb_io_ptr->bounds[i] = bounds[i];
	for (i=0;i<7;i++)
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
	
	/* initialize ancillary variables used
		to save information in certain cases */
	mb_io_ptr->save_flag = MB_NO;
	mb_io_ptr->save_label_flag = MB_NO;

	/* allocate arrays */
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_bath_max*sizeof(char),
				&mb_io_ptr->beamflag,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_bath_max*sizeof(double),
				&mb_io_ptr->bath,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_amp_max*sizeof(double),
				&mb_io_ptr->amp,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_bath_max*sizeof(double),
				&mb_io_ptr->bath_acrosstrack,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_bath_max*sizeof(double),
				&mb_io_ptr->bath_alongtrack,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_bath_max*sizeof(int),
				&mb_io_ptr->bath_num,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_amp_max*sizeof(int),
				&mb_io_ptr->amp_num,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->pixels_ss_max*sizeof(double),
				&mb_io_ptr->ss,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->pixels_ss_max*sizeof(double),
				&mb_io_ptr->ss_acrosstrack,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->pixels_ss_max*sizeof(double),
				&mb_io_ptr->ss_alongtrack,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->pixels_ss_max*sizeof(int),
				&mb_io_ptr->ss_num,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_bath_max*sizeof(char),
				&mb_io_ptr->new_beamflag,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_bath_max*sizeof(double),
				&mb_io_ptr->new_bath,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_amp_max*sizeof(double),
				&mb_io_ptr->new_amp,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_bath_max*sizeof(double),
				&mb_io_ptr->new_bath_acrosstrack,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->beams_bath_max*sizeof(double),
				&mb_io_ptr->new_bath_alongtrack,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->pixels_ss_max*sizeof(double),
				&mb_io_ptr->new_ss,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->pixels_ss_max*sizeof(double),
				&mb_io_ptr->new_ss_acrosstrack,error);
	if (status == MB_SUCCESS)
		status = mb_malloc(verbose,mb_io_ptr->pixels_ss_max*sizeof(double),
				&mb_io_ptr->new_ss_alongtrack,error);

	/* call routine to allocate memory for format dependent i/o */
	if (status == MB_SUCCESS)
		status = (*mb_io_ptr->mb_io_format_alloc)(verbose,*mbio_ptr,error);

	/* deal with a memory allocation failure */
	if (status == MB_FAILURE)
		{
		status = mb_free(verbose,&mb_io_ptr->beamflag,error);
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
		status = mb_free(verbose,&mb_io_ptr->beamflag,error);
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

	/* handle normal or xdr files to be opened 
	   directly with fopen */
	if (mb_io_ptr->filetype == MB_FILETYPE_NORMAL
	    || mb_io_ptr->filetype == MB_FILETYPE_XDR)
	    {
	    /* open the first file */
	    if (strncmp(file,stdin_string,5) == 0)
		mb_io_ptr->mbfp = stdin;
	    else
		if ((mb_io_ptr->mbfp = fopen(mb_io_ptr->file, "r")) == NULL) 
		    {
		    *error = MB_ERROR_OPEN_FAIL;
		    status = MB_FAILURE;
		    }
    
	    /* open the second file if required */
	    if (status == MB_SUCCESS 
		&& mb_io_ptr->numfile >= 2)
		{
		if ((mb_io_ptr->mbfp2 = fopen(mb_io_ptr->file2, "r")) == NULL) 
		    {
		    *error = MB_ERROR_OPEN_FAIL;
		    status = MB_FAILURE;
		    }
		}
    
	    /* or open the second file if desired and possible */
	    else if (status == MB_SUCCESS 
		&& mb_io_ptr->numfile <= -2)
		{
		mb_io_ptr->mbfp2 = fopen(mb_io_ptr->file2, "r");
		}
 
	    /* open the third file if required */
	    if (status == MB_SUCCESS 
		&& mb_io_ptr->numfile >= 3)
		{
		if ((mb_io_ptr->mbfp3 = fopen(mb_io_ptr->file3, "r")) == NULL) 
		    {
		    *error = MB_ERROR_OPEN_FAIL;
		    status = MB_FAILURE;
		    }
		}
 
	    /* or open the third file if desired and possible */
	    else if (status == MB_SUCCESS 
		&& mb_io_ptr->numfile <= -3)
		mb_io_ptr->mbfp3 = fopen(mb_io_ptr->file3, "r");

	    /* if needed, initialize XDR stream */
	    if (status == MB_SUCCESS 
		&& mb_io_ptr->filetype == MB_FILETYPE_XDR)
		{
		status = mb_malloc(verbose,sizeof(XDR),
				&mb_io_ptr->xdrs,error);
		if (status == MB_SUCCESS)
		    {
		    xdrstdio_create((XDR *)mb_io_ptr->xdrs, 
			    mb_io_ptr->mbfp, XDR_DECODE);
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
		status = mb_malloc(verbose,sizeof(XDR),
				&mb_io_ptr->xdrs2,error);
		if (status == MB_SUCCESS)
		    {
		    xdrstdio_create((XDR *)mb_io_ptr->xdrs2, 
			    mb_io_ptr->mbfp2, XDR_DECODE);
		    }
		else
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_MEMORY_FAIL;
		    }
		}
	    }
	    
	/* else handle gsf files to be opened with gsflib */
	else if (mb_io_ptr->filetype == MB_FILETYPE_GSF)
	    {
	    status = gsfOpen(mb_io_ptr->file, 
				GSF_READONLY, 
				(int *) &(mb_io_ptr->mbfp));
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

	/* if error terminate */
	if (status == MB_FAILURE)
		{
		/* save status and error values */
		status_save = status;
		error_save = *error;

		/* free allocated memory */
		if (mb_io_ptr->filetype == MB_FILETYPE_XDR
		    && mb_io_ptr->xdrs != NULL)
			status = mb_free(verbose,&mb_io_ptr->xdrs,error);
		if (mb_io_ptr->filetype == MB_FILETYPE_XDR
		    && mb_io_ptr->xdrs2 != NULL)
			status = mb_free(verbose,&mb_io_ptr->xdrs2,error);
		status = mb_free(verbose,&mb_io_ptr->beamflag,error);
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
		status = mb_free(verbose,&mb_io_ptr->beamflag,error);
		status = mb_free(verbose,&mb_io_ptr->new_bath,error);
		status = mb_free(verbose,&mb_io_ptr->new_amp,error);
		status = mb_free(verbose,&mb_io_ptr->new_bath_acrosstrack,error);
		status = mb_free(verbose,&mb_io_ptr->new_bath_alongtrack,error);
		status = mb_free(verbose,&mb_io_ptr->new_ss,error);
		status = mb_free(verbose,&mb_io_ptr->new_ss_acrosstrack,error);
		status = mb_free(verbose,&mb_io_ptr->new_ss_alongtrack,error);
		status = mb_free(verbose,&mb_io_ptr,error);

		/* restore error and status values */
		status = status_save;
		*error = error_save;

		/* output debug message */
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
	mb_io_ptr->nav_count = 0;
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
