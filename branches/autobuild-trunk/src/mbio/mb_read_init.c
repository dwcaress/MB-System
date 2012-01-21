/* Added HAVE_CONFIG_H for autogen files */
#ifdef HAVE_CONFIG_H
#  include <mbsystem_config.h>
#endif


/*--------------------------------------------------------------------
 *    The MB-system:	mb_read_init.c	1/25/93
 *    $Id: mb_read_init.c 1898 2011-06-13 19:49:07Z caress $
 *
 *    Copyright (c) 1993-2011 by
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
 * $Log: mb_read_init.c,v $
 * Revision 5.27  2009/03/13 07:05:58  caress
 * Release 5.1.2beta02
 *
 * Revision 5.26  2009/03/02 18:51:52  caress
 * Fixed problems with formats 58 and 59, and also updated copyright dates in several source files.
 *
 * Revision 5.25  2008/10/17 07:30:22  caress
 * Added format 26 supporting Hydrosweep DS data used by SOPAC.
 *
 * Revision 5.24  2008/09/20 00:57:40  caress
 * Release 5.1.1beta23
 *
 * Revision 5.23  2008/07/10 06:43:40  caress
 * Preparing for 5.1.1beta20
 *
 * Revision 5.22  2007/10/08 15:59:34  caress
 * MBIO changes as of 8 October 2007.
 *
 * Revision 5.21  2007/05/14 06:22:01  caress
 * Changed fopen in rw mode to fopen in r mode.
 *
 * Revision 5.20  2006/11/10 22:36:04  caress
 * Working towards release 5.1.0
 *
 * Revision 5.19  2006/01/11 07:37:29  caress
 * Working towards 5.0.8
 *
 * Revision 5.18  2006/01/06 18:27:19  caress
 * Working towards 5.0.8
 *
 * Revision 5.17  2005/11/05 00:48:05  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.16  2004/07/15 19:25:04  caress
 * Progress in supporting Reson 7k data.
 *
 * Revision 5.15  2004/04/27 01:46:13  caress
 * Various updates of April 26, 2004.
 *
 * Revision 5.14  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.13  2003/02/27 04:33:33  caress
 * Fixed handling of SURF format data.
 *
 * Revision 5.12  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.11  2002/07/20 20:42:40  caress
 * Release 5.0.beta20
 *
 * Revision 5.10  2002/05/29 23:36:53  caress
 * Release 5.0.beta18
 *
 * Revision 5.9  2002/05/02 03:55:34  caress
 * Release 5.0.beta17
 *
 * Revision 5.8  2002/02/22 09:03:43  caress
 * Release 5.0.beta13
 *
 * Revision 5.7  2002/01/24 02:30:58  caress
 * Added DARWIN.
 *
 * Revision 5.6  2001/10/12  21:08:37  caress
 * Added interpolation of attitude data.
 *
 * Revision 5.5  2001/07/20 00:31:11  caress
 * Release 5.0.beta03
 *
 * Revision 5.4  2001/06/29 22:48:04  caress
 * Added support for HSDS2RAW
 *
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
#include <sys/stat.h>

/* XDR i/o include file */
#ifdef HAVE_RPC_RPC_H
# include <rpc/rpc.h>
#endif
#ifdef HAVE_RPC_TYPES_H
# include <rpc/types.h>
# include <rpc/xdr.h>
#endif

/* mbio include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_define.h"
#include "sapi.h"
#include "mb_segy.h"
#include "gsf.h"
#include "netcdf.h"

static char rcs_id[]="$Id: mb_read_init.c 1898 2011-06-13 19:49:07Z caress $";

/*--------------------------------------------------------------------*/
int mb_read_init(int verbose, char *file, 
		int format, int pings, int lonflip, double bounds[4],
		int btime_i[7], int etime_i[7], 
		double speedmin, double timegap,
		void **mbio_ptr, double *btime_d, double *etime_d,
		int *beams_bath, int *beams_amp, int *pixels_ss, 
		int *error)
{
	char	*function_name = "mb_read_init";
	int	status;
	struct mb_io_struct *mb_io_ptr;
	int	status_save;
	int	error_save;
	int	sapi_status;
	char	*lastslash;
	char	path[MB_PATH_MAXLINE], name[MB_PATH_MAXLINE];
	char	prjfile[MB_PATH_MAXLINE];
	char	projection_id[MB_NAME_LENGTH];
	int	proj_status;
	FILE	*pfp;
	struct stat file_status;
	int	fstat;
	int	i;
	char	*stdin_string = "stdin";

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
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
	status = mb_mallocd(verbose,__FILE__, __LINE__,sizeof(struct mb_io_struct),
				(void **) mbio_ptr,error);
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
	mb_io_ptr->filemode = MB_FILEMODE_READ;
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
				(void **) &mb_io_ptr->beamflag,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->beams_bath_alloc*sizeof(double),
				(void **) &mb_io_ptr->bath,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->beams_amp_alloc*sizeof(double),
				(void **) &mb_io_ptr->amp,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->beams_bath_alloc*sizeof(double),
				(void **) &mb_io_ptr->bath_acrosstrack,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->beams_bath_alloc*sizeof(double),
				(void **) &mb_io_ptr->bath_alongtrack,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->beams_bath_alloc*sizeof(int),
				(void **) &mb_io_ptr->bath_num,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->beams_amp_alloc*sizeof(int),
				(void **) &mb_io_ptr->amp_num,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->pixels_ss_alloc*sizeof(double),
				(void **) &mb_io_ptr->ss,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->pixels_ss_alloc*sizeof(double),
				(void **) &mb_io_ptr->ss_acrosstrack,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->pixels_ss_alloc*sizeof(double),
				(void **) &mb_io_ptr->ss_alongtrack,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->pixels_ss_alloc*sizeof(int),
				(void **) &mb_io_ptr->ss_num,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->beams_bath_alloc*sizeof(char),
				(void **) &mb_io_ptr->new_beamflag,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->beams_bath_alloc*sizeof(double),
				(void **) &mb_io_ptr->new_bath,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->beams_amp_alloc*sizeof(double),
				(void **) &mb_io_ptr->new_amp,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->beams_bath_alloc*sizeof(double),
				(void **) &mb_io_ptr->new_bath_acrosstrack,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->beams_bath_alloc*sizeof(double),
				(void **) &mb_io_ptr->new_bath_alongtrack,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->pixels_ss_alloc*sizeof(double),
				(void **) &mb_io_ptr->new_ss,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->pixels_ss_alloc*sizeof(double),
				(void **) &mb_io_ptr->new_ss_acrosstrack,error);
	if (status == MB_SUCCESS)
		status = mb_mallocd(verbose,__FILE__, __LINE__,mb_io_ptr->pixels_ss_alloc*sizeof(double),
				(void **) &mb_io_ptr->new_ss_alongtrack,error);

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
		if ((fstat = stat(mb_io_ptr->file2, &file_status)) == 0
		    && (file_status.st_mode & S_IFMT) != S_IFDIR
		    && file_status.st_size > 0)
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
		{
		if ((fstat = stat(mb_io_ptr->file2, &file_status)) == 0
		    && (file_status.st_mode & S_IFMT) != S_IFDIR
		    && file_status.st_size > 0)
			mb_io_ptr->mbfp3 = fopen(mb_io_ptr->file3, "r");
		}

	    /* if needed, initialize XDR stream */
	    if (status == MB_SUCCESS 
		&& mb_io_ptr->filetype == MB_FILETYPE_XDR)
		{
		status = mb_mallocd(verbose,__FILE__, __LINE__,sizeof(XDR),
				(void **) &mb_io_ptr->xdrs,error);
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
		status = mb_mallocd(verbose,__FILE__, __LINE__,sizeof(XDR),
				(void **) &mb_io_ptr->xdrs2,error);
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

	    /* if needed, initialize third XDR stream */
	    if (status == MB_SUCCESS 
		&& mb_io_ptr->filetype == MB_FILETYPE_XDR
		&& (mb_io_ptr->numfile >= 3 || mb_io_ptr->numfile <= -3)
		&& mb_io_ptr->mbfp3 != NULL)
		{
		status = mb_mallocd(verbose,__FILE__, __LINE__,sizeof(XDR),
				(void **) &mb_io_ptr->xdrs3,error);
		if (status == MB_SUCCESS)
		    {
		    xdrstdio_create((XDR *)mb_io_ptr->xdrs3, 
			    mb_io_ptr->mbfp3, XDR_DECODE);
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
	    status = nc_open(mb_io_ptr->file, 
				NC_NOWRITE, 
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
	    status = mb_segy_read_init(verbose, mb_io_ptr->file, 
		(void **)&(mb_io_ptr->mbfp), NULL, NULL, error);
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
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->bath_acrosstrack,error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->bath_alongtrack,error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->bath_num,error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->amp_num,error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->ss,error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->ss_acrosstrack,error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->ss_alongtrack,error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->ss_num,error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->new_beamflag,error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->new_bath,error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->new_amp,error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->new_bath_acrosstrack,error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->new_bath_alongtrack,error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->new_ss,error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->new_ss_acrosstrack,error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr->new_ss_alongtrack,error);
		status = mb_freed(verbose,__FILE__, __LINE__,(void **)&mb_io_ptr,error);

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
		
	/* check for projection specification file */
	sprintf(prjfile, "%s.prj", file);
	if ((pfp = fopen(prjfile, "r")) != NULL)
		{
		fscanf(pfp,"%s", projection_id);
		proj_status = mb_proj_init(verbose,projection_id, 
			&(mb_io_ptr->pjptr), error);
		if (proj_status == MB_SUCCESS)
			{
			mb_io_ptr->projection_initialized = MB_YES;
			}
		else
			{
			fprintf(stderr, "Unable to initialize projection %s from file %s\n\n",
				projection_id, prjfile);
			}
		fclose(pfp);
		}
	else
		{
		*error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
		}

	/* set error and status (if you got here you succeeded */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)*mbio_ptr);
		fprintf(stderr,"dbg2       ->numfile:  %d\n",mb_io_ptr->numfile);
		fprintf(stderr,"dbg2       ->file:     %s\n",mb_io_ptr->file);
		if (mb_io_ptr->numfile >= 2 || mb_io_ptr->numfile <= -2) 
		    fprintf(stderr,"dbg2       ->file2:    %s\n",mb_io_ptr->file2);
		if (mb_io_ptr->numfile >= 3 || mb_io_ptr->numfile <= -3) 
		    fprintf(stderr,"dbg2       ->file3:    %s\n",mb_io_ptr->file3);
		fprintf(stderr,"dbg2       ->mbfp:     %lu\n",(size_t)mb_io_ptr->mbfp);
		if (mb_io_ptr->numfile >= 2 || mb_io_ptr->numfile <= -2) 
		    fprintf(stderr,"dbg2       ->mbfp2:    %lu\n",(size_t)mb_io_ptr->mbfp2);
		if (mb_io_ptr->numfile >= 3 || mb_io_ptr->numfile <= -3) 
		    fprintf(stderr,"dbg2       ->mbfp3:    %lu\n",(size_t)mb_io_ptr->mbfp3);
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
