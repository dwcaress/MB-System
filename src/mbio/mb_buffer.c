/*--------------------------------------------------------------------
 *    The MB-system:	mb_buffer.c	2/25/93
 *    $Id$
 *
 *    Copyright (c) 1993-2012 by
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
 * mb_buffer.c contains the functions for buffered i/o of multibeam
 * data.   
 * These functions include:
 *   mb_buffer_init	- initialize buffer structure
 *   mb_buffer_close	- close buffer structure
 *   mb_buffer_load	- load data from file into buffer
 *   mb_buffer_dump	- dump data from buffer into file
 *   mb_buffer_clear	- clear data from buffer
 *   mb_buffer_get_next_data	- extract navigation and bathymetry/backscatter 
 *   				from next suitable record in buffer
 *   mb_buffer_get_next_nav	- extract navigation and vru 
 *   				from next suitable record in buffer
 *   mb_buffer_extract	- extract navigation and bathymetry/backscatter 
 *   				from specified record in buffer
 *   mb_buffer_insert	- insert altered navigation and 
 *   				bathymetry/backscatter data into 
 *   				record in buffer
 *   mb_buffer_extract_nav - extract navigation and vru 
 *   				from specified record in buffer
 *   mb_buffer_insert_nav - insert altered navigation into 
 *   				record in buffer
 *
 * Author:	D. W. Caress
 * Date:	February 25, 1993
 *
 * $Log: mb_buffer.c,v $
 * Revision 5.8  2009/03/02 18:51:52  caress
 * Fixed problems with formats 58 and 59, and also updated copyright dates in several source files.
 *
 * Revision 5.7  2005/11/05 00:48:05  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.6  2004/04/27 01:46:13  caress
 * Various updates of April 26, 2004.
 *
 * Revision 5.5  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.4  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.3  2002/07/20 20:42:40  caress
 * Release 5.0.beta20
 *
 * Revision 5.2  2002/05/29 23:36:53  caress
 * Release 5.0.beta18
 *
 * Revision 5.1  2001/07/20 00:31:11  caress
 * Release 5.0.beta03
 *
 * Revision 5.0  2000/12/01 22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.21  2000/10/11  01:02:30  caress
 * Convert to ANSI C
 *
 * Revision 4.20  2000/09/30  06:26:58  caress
 * Snapshot for Dale.
 *
 * Revision 4.19  1999/08/08  04:12:45  caress
 * Added ELMK2XSE format.
 *
 * Revision 4.18  1999/07/16  19:24:15  caress
 * Yet another version.
 *
 * Revision 4.17  1999/03/31  18:11:35  caress
 * MB-System 4.6beta7
 *
 * Revision 4.16  1998/12/17  22:56:15  caress
 * MB-System version 4.6beta4
 *
 * Revision 4.15  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.14  1997/07/28  15:10:28  caress
 * Fixed typos.
 *
 * Revision 4.13  1997/07/25  14:19:53  caress
 * Version 4.5beta2.
 * Much mucking, particularly with Simrad formats.
 *
 * Revision 4.12  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.11  1996/08/26  17:24:56  caress
 * Release 4.4 revision.
 *
 * Revision 4.11  1996/08/26  17:24:56  caress
 * Release 4.4 revision.
 *
 * Revision 4.10  1996/08/05  15:21:58  caress
 * Just redid i/o for Simrad sonars, including adding EM12S and EM121 support.
 *
 * Revision 4.9  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.8  1996/03/12  17:21:55  caress
 * Added format 63, short HMR1 processing format.
 *
 * Revision 4.7  1995/09/28  18:10:48  caress
 * Various bug fixes working toward release 4.3.
 *
 * Revision 4.6  1995/08/17  14:42:45  caress
 * Revision for release 4.3.
 *
 * Revision 4.5  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.4  1995/02/10  22:10:46  caress
 * Added mb_buffer_clear() function to allow data to be cleared
 * without being dumped to a file.
 *
 * Revision 4.3  1994/10/21  12:11:53  caress
 * Release V4.0
 *
 * Revision 4.2  1994/07/29  18:46:51  caress
 * Changes associated with supporting Lynx OS (byte swapped) and
 * using unix second time base (for time_d values).
 *
 * Revision 4.1  1994/04/12  00:24:27  caress
 * Added mbio_ptr to input list for mb_buffer_close to fix
 * segmentation fault on closing.
 *
 * Revision 4.0  1994/03/05  23:55:38  caress
 * First cut at version 4.0
 *
 * Revision 4.4  1994/03/05  22:51:44  caress
 * Added ability to handle Simrad EM12 system and
 * format MBF_EM12DARW.
 *
 * Revision 4.3  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.2  1994/02/25  01:56:17  caress
 * Fixed some significant bugs that caused seg faults.
 *
 * Revision 4.1  1994/02/24  05:23:36  caress
 * Fixed some major problems with buffered i/o - the
 * switch to bath,amp,ss hadn't been completed for all
 * the functions!
 *
 * Revision 4.0  1994/02/20  03:49:44  caress
 * First cut at new version. Now supports both amplitude
 * and sidescan data.
 *
 * Revision 3.3  1993/06/15  06:08:02  caress
 * Added some debug messages in mb_buffer_close.
 *
 * Revision 3.2  1993/06/10  05:43:19  caress
 * Added facility in mb_buffer_close to deallocate memory associate
 * with any remaining records in buffer before deallocating the
 * buffer structure itself.
 *
 * Revision 3.1  1993/05/14  22:04:16  dale
 * Fix rcsid declaration
 *
 * Revision 3.0  1993/04/23  15:40:29  dale
 * Initial version
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

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/
int mb_buffer_init(int verbose, void **buff_ptr, int *error)
{
	char	*function_name = "mb_buffer_init";
	int	status = MB_SUCCESS;
	struct mb_buffer_struct *buff;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		}

	/* allocate memory for data structure */
	status = mb_mallocd(verbose,__FILE__,__LINE__,sizeof(struct mb_buffer_struct),buff_ptr,error);
	buff = (struct mb_buffer_struct *) *buff_ptr;

	/* set nbuffer to zero */
	buff->nbuffer = 0;
	for (i=0;i<MB_BUFFER_MAX;i++)
		buff->buffer[i] = NULL;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       buff_ptr:   %lu\n",(size_t)*buff_ptr);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_buffer_close(int verbose, void **buff_ptr, void *mbio_ptr, int *error)
{
	char	*function_name = "mb_buffer_close";
	int	status = MB_SUCCESS;
	struct mb_buffer_struct *buff;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buff_ptr:   %lu\n",(size_t)*buff_ptr);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		}

	/* get buffer structure */
	buff = (struct mb_buffer_struct *) *buff_ptr;

	/* deal with any remaining records in the buffer */
	if (buff->nbuffer > 0)
		{
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  Remaining records in buffer: %d\n",buff->nbuffer);
			for (i=0;i<buff->nbuffer;i++);
				fprintf(stderr,"dbg4       Record[%d] pointer: %lu\n",i,(size_t)(buff->buffer[i]));
			}
		for (i=0;i<buff->nbuffer;i++)
			status = mb_deall(verbose,mbio_ptr,
				&buff->buffer[i],error);
		}

	/* deallocate memory for data structure */
	status = mb_free(verbose,buff_ptr,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_buffer_load(int verbose, void *buff_ptr,void *mbio_ptr,
		    int nwant, int *nload, int *nbuff, int *error)
{
	char	*function_name = "mb_buffer_load";
	int	status = MB_SUCCESS;
	struct mb_buffer_struct *buff;
	struct mb_io_struct *mb_io_ptr;
	char	*store_ptr;
	int	nget;
	int	kind;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buff_ptr:   %lu\n",(size_t)buff_ptr);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       nwant:      %d\n",nwant);
		}

	/* get buffer structure */
	buff = (struct mb_buffer_struct *) buff_ptr;

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	store_ptr = mb_io_ptr->store_data;

	/* can't get more than the buffer will hold */
	nget = nwant - buff->nbuffer;
	if (buff->nbuffer + nget > MB_BUFFER_MAX)
		nget = MB_BUFFER_MAX - buff->nbuffer;
	*nload = 0;
	*error = MB_ERROR_NO_ERROR;

	/* print debug statements */
	if (verbose >= 4)
		{
		fprintf(stderr,"\ndbg4  Getting ready to read records in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg4       nwant:         %d\n",nwant);
		fprintf(stderr,"dbg4       nget:          %d\n",nget);
		fprintf(stderr,"dbg4       nload:         %d\n",*nload);
		fprintf(stderr,"dbg4       error:         %d\n",*error);
		}

	/* read records into the buffer until its full or eof */
	while (*error <= MB_ERROR_NO_ERROR && *nload < nget)
		{
		/* read the next data record */
		status = mb_read_ping(verbose,mbio_ptr,store_ptr,
					&kind,error);
			
		/* log errors */
		if (*error < MB_ERROR_NO_ERROR)
			mb_notice_log_error(verbose, mbio_ptr, *error);

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  New record read by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4       kind:          %d\n",kind);
			fprintf(stderr,"dbg4       store_ptr:     %lu\n",(size_t)store_ptr);
			fprintf(stderr,"dbg4       nbuffer:       %d\n",buff->nbuffer);
			fprintf(stderr,"dbg4       nwant:         %d\n",nwant);
			fprintf(stderr,"dbg4       nget:          %d\n",nget);
			fprintf(stderr,"dbg4       nload:         %d\n",*nload);
			fprintf(stderr,"dbg4       error:         %d\n",*error);
			fprintf(stderr,"dbg4       status:        %d\n",status);
			}

		/* ignore time gaps */
		if (*error == MB_ERROR_TIME_GAP)
			{
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			}

		/* deal with good data */
		if (*error == MB_ERROR_NO_ERROR && store_ptr != NULL)
			{

			/* allocate space and copy the data */
			status = mb_alloc(verbose,mbio_ptr,
				&buff->buffer[buff->nbuffer],
				error);
			if (status == MB_SUCCESS)
			status = mb_copyrecord(verbose,mbio_ptr,
				store_ptr,
				buff->buffer[buff->nbuffer],error);
			if (status == MB_SUCCESS)
				{
				buff->buffer_kind[buff->nbuffer] = kind;
				buff->nbuffer++;
				(*nload)++;
				}
			}

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  Buffer status in MBIO function <%s>\n",function_name);
			fprintf(stderr,"dbg4       nbuffer:       %d\n",buff->nbuffer);
			fprintf(stderr,"dbg4       nload:         %d\n",*nload);
			fprintf(stderr,"dbg4       nget:          %d\n",nget);
			fprintf(stderr,"dbg4       nwant:         %d\n",nwant);
			fprintf(stderr,"dbg4       error:         %d\n",*error);
			fprintf(stderr,"dbg4       status:        %d\n",status);
			for (i=0;i<buff->nbuffer;i++)
				{
				fprintf(stderr,"dbg4       i:%d  kind:%d  ptr:%lu\n",
					i,buff->buffer_kind[i],(size_t)buff->buffer[i]);
				}
			}
		}
	*nbuff = buff->nbuffer;

	/* error only if no records were loaded */
	if (*nload > 0)
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
	else if (nwant <= 0)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_NO_DATA_REQUESTED;
		}
	else if (nget <= 0)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BUFFER_FULL;
		}
	else if (*nload <= 0 && *error <= MB_ERROR_NO_ERROR)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_NO_DATA_LOADED;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       nload:      %d\n",*nload);
		fprintf(stderr,"dbg2       nbuff:      %d\n",*nbuff);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_buffer_dump(int verbose, void *buff_ptr, void *mbio_ptr, void *ombio_ptr,
		    int nhold, int *ndump, int *nbuff, int *error)
{
	char	*function_name = "mb_buffer_dump";
	int	status = MB_SUCCESS;
	struct mb_buffer_struct *buff;
	struct mb_io_struct *mb_io_ptr;

	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buff_ptr:   %lu\n",(size_t)buff_ptr);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       omb_ptr:    %lu\n",(size_t)ombio_ptr);
		fprintf(stderr,"dbg2       nhold:      %d\n",nhold);
		}

	/* get buffer structure */
	buff = (struct mb_buffer_struct *) buff_ptr;

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* figure out how much to dump */
	*ndump = buff->nbuffer - nhold;
	if (buff->nbuffer <= 0)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BUFFER_EMPTY;
		*ndump = 0;
		}
	else if (*ndump <= 0)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_NO_DATA_DUMPED;
		*ndump = 0;
		}

	/* print debug statements */
	if (verbose >= 4)
		{
		fprintf(stderr,"\ndbg4  Buffer list in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg4       nbuffer:     %d\n",buff->nbuffer);
		for (i=0;i<buff->nbuffer;i++)
			{
			fprintf(stderr,"dbg4       i:%d  kind:%d  ptr:%lu\n",
			i,buff->buffer_kind[i],(size_t)buff->buffer[i]);
			}
		}

	/* dump records from buffer */
	if (status == MB_SUCCESS)
		{
		for (i=0;i<*ndump;i++)
			{
			/* print debug statements */
			if (verbose >= 4)
				{
				fprintf(stderr,"\ndbg4  Dumping record in MBIO function <%s>\n",function_name);
				fprintf(stderr,"dbg4       record:      %d\n",i);
				fprintf(stderr,"dbg4       ptr:         %lu\n",(size_t)buff->buffer[i]);
				fprintf(stderr,"dbg4       kind:        %d\n",buff->buffer_kind[i]);
				}

			/* only write out data if output defined */
			if (ombio_ptr != NULL)
				status = mb_write_ping(verbose,ombio_ptr,buff->buffer[i],error);

			/* print debug statements */
			if (verbose >= 4)
				{
				fprintf(stderr,"\ndbg4  Deallocating record in MBIO function <%s>\n",function_name);
				fprintf(stderr,"dbg4       record:      %d\n",i);
				fprintf(stderr,"dbg4       ptr:         %lu\n",(size_t)buff->buffer[i]);
				fprintf(stderr,"dbg4       kind:        %d\n",buff->buffer_kind[i]);
				}

			status = mb_deall(verbose,mbio_ptr,
				&buff->buffer[i],error);
			buff->buffer[i] = NULL;

			/* print debug statements */
			if (verbose >= 4)
				{
				fprintf(stderr,"\ndbg4  Done dumping record in MBIO function <%s>\n",function_name);
				fprintf(stderr,"dbg4       record:      %d\n",i);
				fprintf(stderr,"dbg4       ptr:         %lu\n",(size_t)buff->buffer[i]);
				fprintf(stderr,"dbg4       kind:        %d\n",buff->buffer_kind[i]);
				}
			}
		for (i=0;i<nhold;i++)
			{
			/* print debug statements */
			if (verbose >= 4)
				{
				fprintf(stderr,"\ndbg4  Moving buffer record in MBIO function <%s>\n",function_name);
				fprintf(stderr,"dbg4       old:         %d\n",*ndump + i);
				fprintf(stderr,"dbg4       new:         %d\n",i);
				fprintf(stderr,"dbg4       old ptr:     %lu\n",(size_t)buff->buffer[*ndump + i]);
				fprintf(stderr,"dbg4       new ptr:     %lu\n",(size_t)buff->buffer[i]);
				fprintf(stderr,"dbg4       old kind:    %d\n",buff->buffer_kind[*ndump + i]);
				fprintf(stderr,"dbg4       new kind:    %d\n",buff->buffer_kind[i]);
				}

			buff->buffer[i] = 
				buff->buffer[*ndump + i];
			buff->buffer_kind[i] = 
				buff->buffer_kind[*ndump + i];
			buff->buffer[*ndump + i] = NULL;
			buff->buffer_kind[*ndump + i] = 0;

			/* print debug statements */
			if (verbose >= 4)
				{
				fprintf(stderr,"\ndbg4  Done moving buffer record in MBIO function <%s>\n",function_name);
				fprintf(stderr,"dbg4       old:         %d\n",*ndump + i);
				fprintf(stderr,"dbg4       new:         %d\n",i);
				fprintf(stderr,"dbg4       old ptr:     %lu\n",(size_t)buff->buffer[*ndump + i]);
				fprintf(stderr,"dbg4       new ptr:     %lu\n",(size_t)buff->buffer[i]);
				fprintf(stderr,"dbg4       old kind:    %d\n",buff->buffer_kind[*ndump + i]);
				fprintf(stderr,"dbg4       new kind:    %d\n",buff->buffer_kind[i]);
				}
			}
		buff->nbuffer = buff->nbuffer - *ndump;
		}

	/* print debug statements */
	if (verbose >= 4)
		{
		fprintf(stderr,"\ndbg4  Buffer list at end of MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg4       nbuffer:     %d\n",buff->nbuffer);
		for (i=0;i<buff->nbuffer;i++)
			{
			fprintf(stderr,"dbg4       i:%d  kind:%d  ptr:%lu\n",
			i,buff->buffer_kind[i],(size_t)buff->buffer[i]);
			}
		}

	/* set return value */
	*nbuff = buff->nbuffer;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       ndump:      %d\n",*ndump);
		fprintf(stderr,"dbg2       nbuff:      %d\n",*nbuff);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_buffer_clear(int verbose, void *buff_ptr, void *mbio_ptr,
		    int nhold, int *ndump, int *nbuff, int *error)
{
	char	*function_name = "mb_buffer_clear";
	int	status = MB_SUCCESS;
	struct mb_buffer_struct *buff;
	struct mb_io_struct *mb_io_ptr;

	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buff_ptr:   %lu\n",(size_t)buff_ptr);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       nhold:      %d\n",nhold);
		}

	/* get buffer structure */
	buff = (struct mb_buffer_struct *) buff_ptr;

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* figure out how much to dump */
	*ndump = buff->nbuffer - nhold;
	if (buff->nbuffer <= 0)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BUFFER_EMPTY;
		*ndump = 0;
		}
	else if (*ndump <= 0)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_NO_DATA_DUMPED;
		*ndump = 0;
		}

	/* print debug statements */
	if (verbose >= 4)
		{
		fprintf(stderr,"\ndbg4  Buffer list in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg4       nbuffer:     %d\n",buff->nbuffer);
		for (i=0;i<buff->nbuffer;i++)
			{
			fprintf(stderr,"dbg4       i:%d  kind:%d  ptr:%lu\n",
			i,buff->buffer_kind[i],(size_t)buff->buffer[i]);
			}
		}

	/* dump records from buffer */
	if (status == MB_SUCCESS)
		{
		for (i=0;i<*ndump;i++)
			{
			/* print debug statements */
			if (verbose >= 4)
				{
				fprintf(stderr,"\ndbg4  Deallocating record in MBIO function <%s>\n",function_name);
				fprintf(stderr,"dbg4       record:      %d\n",i);
				fprintf(stderr,"dbg4       ptr:         %lu\n",(size_t)buff->buffer[i]);
				fprintf(stderr,"dbg4       kind:        %d\n",buff->buffer_kind[i]);
				}

			status = mb_deall(verbose,mbio_ptr,
				&buff->buffer[i],error);
			buff->buffer[i] = NULL;

			/* print debug statements */
			if (verbose >= 4)
				{
				fprintf(stderr,"\ndbg4  Done dumping record in MBIO function <%s>\n",function_name);
				fprintf(stderr,"dbg4       record:      %d\n",i);
				fprintf(stderr,"dbg4       ptr:         %lu\n",(size_t)buff->buffer[i]);
				fprintf(stderr,"dbg4       kind:        %d\n",buff->buffer_kind[i]);
				}
			}
		for (i=0;i<nhold;i++)
			{
			/* print debug statements */
			if (verbose >= 4)
				{
				fprintf(stderr,"\ndbg4  Moving buffer record in MBIO function <%s>\n",function_name);
				fprintf(stderr,"dbg4       old:         %d\n",*ndump + i);
				fprintf(stderr,"dbg4       new:         %d\n",i);
				fprintf(stderr,"dbg4       old ptr:     %lu\n",(size_t)buff->buffer[*ndump + i]);
				fprintf(stderr,"dbg4       new ptr:     %lu\n",(size_t)buff->buffer[i]);
				fprintf(stderr,"dbg4       old kind:    %d\n",buff->buffer_kind[*ndump + i]);
				fprintf(stderr,"dbg4       new kind:    %d\n",buff->buffer_kind[i]);
				}

			buff->buffer[i] = 
				buff->buffer[*ndump + i];
			buff->buffer_kind[i] = 
				buff->buffer_kind[*ndump + i];
			buff->buffer[*ndump + i] = NULL;
			buff->buffer_kind[*ndump + i] = 0;

			/* print debug statements */
			if (verbose >= 4)
				{
				fprintf(stderr,"\ndbg4  Done moving buffer record in MBIO function <%s>\n",function_name);
				fprintf(stderr,"dbg4       old:         %d\n",*ndump + i);
				fprintf(stderr,"dbg4       new:         %d\n",i);
				fprintf(stderr,"dbg4       old ptr:     %lu\n",(size_t)buff->buffer[*ndump + i]);
				fprintf(stderr,"dbg4       new ptr:     %lu\n",(size_t)buff->buffer[i]);
				fprintf(stderr,"dbg4       old kind:    %d\n",buff->buffer_kind[*ndump + i]);
				fprintf(stderr,"dbg4       new kind:    %d\n",buff->buffer_kind[i]);
				}
			}
		buff->nbuffer = buff->nbuffer - *ndump;
		}

	/* print debug statements */
	if (verbose >= 4)
		{
		fprintf(stderr,"\ndbg4  Buffer list at end of MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg4       nbuffer:     %d\n",buff->nbuffer);
		for (i=0;i<buff->nbuffer;i++)
			{
			fprintf(stderr,"dbg4       i:%d  kind:%d  ptr:%lu\n",
			i,buff->buffer_kind[i],(size_t)buff->buffer[i]);
			}
		}

	/* set return value */
	*nbuff = buff->nbuffer;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       ndump:      %d\n",*ndump);
		fprintf(stderr,"dbg2       nbuff:      %d\n",*nbuff);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_buffer_get_next_data(int verbose, void *buff_ptr, void *mbio_ptr,
		int start, int *id,
		int time_i[7], double *time_d,
		double *navlon, double *navlat, 
		double *speed, double *heading,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp,
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		int *error)
{
	char	*function_name = "mb_buffer_get_next_data";
	int	status = MB_SUCCESS;
	struct mb_buffer_struct *buff;
	struct mb_io_struct *mb_io_ptr;
	int	kind;
	char	comment[200];
	int	found;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buff_ptr:   %lu\n",(size_t)buff_ptr);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       start:      %d\n",start);
		}

	/* get buffer structure */
	buff = (struct mb_buffer_struct *) buff_ptr;

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* look for next survey data */
	found = MB_NO;
	for (i=start;i<buff->nbuffer;i++)
		{
		if (found == MB_NO 
			&& buff->buffer_kind[i] == MB_DATA_DATA)
			{
			*id = i;
			found = MB_YES;
			}
		}
	if (found == MB_NO)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_NO_MORE_DATA;
		*id = -1;
		}

	/* extract the data */
	if (found == MB_YES)
		status = mb_buffer_extract(verbose,buff_ptr,mbio_ptr,*id,&kind,
			time_i,time_d,navlon,navlat,speed,heading,
			nbath,namp,nss,
			beamflag,bath,amp,
			bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       id:         %d\n",*id);
		}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR)
		{
		fprintf(stderr,"dbg2       time_i[0]:     %d\n",time_i[0]);
		fprintf(stderr,"dbg2       time_i[1]:     %d\n",time_i[1]);
		fprintf(stderr,"dbg2       time_i[2]:     %d\n",time_i[2]);
		fprintf(stderr,"dbg2       time_i[3]:     %d\n",time_i[3]);
		fprintf(stderr,"dbg2       time_i[4]:     %d\n",time_i[4]);
		fprintf(stderr,"dbg2       time_i[5]:     %d\n",time_i[5]);
		fprintf(stderr,"dbg2       time_i[6]:     %d\n",time_i[6]);
		fprintf(stderr,"dbg2       time_d:        %f\n",*time_d);
		fprintf(stderr,"dbg2       longitude:     %f\n",*navlon);
		fprintf(stderr,"dbg2       latitude:      %f\n",*navlat);
		fprintf(stderr,"dbg2       speed:         %f\n",*speed);
		fprintf(stderr,"dbg2       heading:       %f\n",*heading);
		fprintf(stderr,"dbg4       nbath:         %d\n",*nbath);
		if (*nbath > 0)
		  {
		  fprintf(stderr,"dbg4       beam   flag   bath  crosstrack alongtrack\n");
		  for (i=0;i<*nbath;i++)
		    fprintf(stderr,"dbg4       %4d   %3d   %f    %f     %f\n",
			i,beamflag[i],bath[i],bathacrosstrack[i],bathalongtrack[i]);
		  }
		fprintf(stderr,"dbg4       namp:          %d\n",*namp);
		if (*namp > 0)
		  {
		  fprintf(stderr,"dbg4       beam    amp  crosstrack alongtrack\n");
		  for (i=0;i<*nbath;i++)
		    fprintf(stderr,"dbg4       %4d   %f    %f     %f\n",
			i,amp[i],bathacrosstrack[i],bathalongtrack[i]);
		  }
		fprintf(stderr,"dbg4       nss:           %d\n",*nss);
		if (*nss > 0)
		  {
		  fprintf(stderr,"dbg4       pixel sidescan crosstrack alongtrack\n");
		  for (i=0;i<*nss;i++)
		    fprintf(stderr,"dbg4       %4d   %f    %f     %f\n",
			i,ss[i],ssacrosstrack[i],ssalongtrack[i]);
		  }
		}
	if (verbose >= 2)
		{
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_buffer_get_next_nav(int verbose, void *buff_ptr, void *mbio_ptr,
		int start, int *id,
		int time_i[7], double *time_d,
		double *navlon, double *navlat, 
		double *speed, double *heading, double *draft, 
		double *roll, double *pitch, double *heave,
		int *error)
{
	char	*function_name = "mb_buffer_get_next_nav";
	int	status = MB_SUCCESS;
	struct mb_buffer_struct *buff;
	struct mb_io_struct *mb_io_ptr;
	int	kind;
	int	found;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buff_ptr:   %lu\n",(size_t)buff_ptr);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       start:      %d\n",start);
		}

	/* get buffer structure */
	buff = (struct mb_buffer_struct *) buff_ptr;

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	
	/* look for next data of the appropriate type */
	found = MB_NO;
	for (i=start;i<buff->nbuffer;i++)
		{
		if (found == MB_NO 
			&& buff->buffer_kind[i] == mb_io_ptr->nav_source)
			{
			*id = i;
			found = MB_YES;
			}
		}
	if (found == MB_NO)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_NO_MORE_DATA;
		*id = -1;
		}

	/* extract the data */
	if (found == MB_YES)
		status = mb_buffer_extract_nav(verbose,
			buff_ptr,mbio_ptr,*id,&kind,
			time_i,time_d,navlon,navlat,
			speed,heading,draft, 
			roll,pitch,heave,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       id:         %d\n",*id);
		}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR)
		{
		fprintf(stderr,"dbg2       time_i[0]:     %d\n",time_i[0]);
		fprintf(stderr,"dbg2       time_i[1]:     %d\n",time_i[1]);
		fprintf(stderr,"dbg2       time_i[2]:     %d\n",time_i[2]);
		fprintf(stderr,"dbg2       time_i[3]:     %d\n",time_i[3]);
		fprintf(stderr,"dbg2       time_i[4]:     %d\n",time_i[4]);
		fprintf(stderr,"dbg2       time_i[5]:     %d\n",time_i[5]);
		fprintf(stderr,"dbg2       time_i[6]:     %d\n",time_i[6]);
		fprintf(stderr,"dbg2       time_d:        %f\n",*time_d);
		fprintf(stderr,"dbg2       longitude:     %f\n",*navlon);
		fprintf(stderr,"dbg2       latitude:      %f\n",*navlat);
		fprintf(stderr,"dbg2       speed:         %f\n",*speed);
		fprintf(stderr,"dbg2       heading:       %f\n",*heading);
		fprintf(stderr,"dbg2       draft:         %f\n",*draft);
		fprintf(stderr,"dbg2       roll:          %f\n",*roll);
		fprintf(stderr,"dbg2       pitch:         %f\n",*pitch);
		fprintf(stderr,"dbg2       heave:         %f\n",*heave);
		}
	if (verbose >= 2)
		{
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_buffer_extract(int verbose, void *buff_ptr, void *mbio_ptr,
		int id, int *kind, 
		int time_i[7], double *time_d,
		double *navlon, double *navlat, 
		double *speed, double *heading,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp,
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, 
		int *error)
{
	char	*function_name = "mb_buffer_extract";
	int	status = MB_SUCCESS;
	struct mb_buffer_struct *buff;
	struct mb_io_struct *mb_io_ptr;
	char	*store_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buff_ptr:   %lu\n",(size_t)buff_ptr);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       id:         %d\n",id);
		}

	/* get buffer structure */
	buff = (struct mb_buffer_struct *) buff_ptr;

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get store_ptr and kind for desired record */
	if (id < 0 || id >= buff->nbuffer)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_BUFFER_ID;
		}
	else
		{
		store_ptr = buff->buffer[id];
		*kind = buff->buffer_kind[id];
		*error = MB_ERROR_NO_ERROR;
		}

	/* if no error then proceed */
	if (status == MB_SUCCESS)
		{
		status = mb_extract(verbose,mbio_ptr,
			store_ptr,kind,
			time_i,time_d,navlon,navlat,speed,heading,
			nbath,namp,nss,
			beamflag,bath,amp,
			bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,error);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:       %d\n",*kind);
		}
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR 
		&& *kind == MB_DATA_COMMENT)
		{
		fprintf(stderr,"dbg2       comment:     \ndbg2       %s\n",
			comment);
		}
	else if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR 
		&& *kind != MB_DATA_COMMENT)
		{
		fprintf(stderr,"dbg2       time_i[0]:     %d\n",time_i[0]);
		fprintf(stderr,"dbg2       time_i[1]:     %d\n",time_i[1]);
		fprintf(stderr,"dbg2       time_i[2]:     %d\n",time_i[2]);
		fprintf(stderr,"dbg2       time_i[3]:     %d\n",time_i[3]);
		fprintf(stderr,"dbg2       time_i[4]:     %d\n",time_i[4]);
		fprintf(stderr,"dbg2       time_i[5]:     %d\n",time_i[5]);
		fprintf(stderr,"dbg2       time_i[6]:     %d\n",time_i[6]);
		fprintf(stderr,"dbg2       time_d:        %f\n",*time_d);
		fprintf(stderr,"dbg2       longitude:     %f\n",*navlon);
		fprintf(stderr,"dbg2       latitude:      %f\n",*navlat);
		fprintf(stderr,"dbg2       speed:         %f\n",*speed);
		fprintf(stderr,"dbg2       heading:       %f\n",*heading);
		}
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR 
		&& *kind == MB_DATA_DATA)
		{
		fprintf(stderr,"dbg4       nbath:         %d\n",*nbath);
		if (*nbath > 0)
		  {
		  fprintf(stderr,"dbg4       beam    flag   bath  crosstrack alongtrack\n");
		  for (i=0;i<*nbath;i++)
		    fprintf(stderr,"dbg4       %4d   %3d   %f    %f     %f\n",
			i,beamflag[i],bath[i],
			bathacrosstrack[i],bathalongtrack[i]);
		  }
		fprintf(stderr,"dbg4       namp:          %d\n",*namp);
		if (*namp > 0)
		  {
		  fprintf(stderr,"dbg4       beam    amp  crosstrack alongtrack\n");
		  for (i=0;i<*nbath;i++)
		    fprintf(stderr,"dbg4       %4d   %f    %f     %f\n",
			i,amp[i],bathacrosstrack[i],bathalongtrack[i]);
		  }
		fprintf(stderr,"dbg4       nss:           %d\n",*nss);
		if (*nss > 0)
		  {
		  fprintf(stderr,"dbg4       pixel sidescan crosstrack alongtrack\n");
		  for (i=0;i<*nss;i++)
		    fprintf(stderr,"dbg4       %4d   %f    %f     %f\n",
			i,ss[i],ssacrosstrack[i],ssalongtrack[i]);
		  }
		}
	if (verbose >= 2)
		{
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_buffer_extract_nav(int verbose, void *buff_ptr, void *mbio_ptr,
		int id, int *kind, 
		int time_i[7], double *time_d,
		double *navlon, double *navlat, 
		double *speed, double *heading, double *draft,
		double *roll, double *pitch, double *heave,
		int *error)
{
	char	*function_name = "mb_buffer_extract_nav";
	int	status = MB_SUCCESS;
	struct mb_buffer_struct *buff;
	struct mb_io_struct *mb_io_ptr;
	char	*store_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buff_ptr:   %lu\n",(size_t)buff_ptr);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       id:         %d\n",id);
		}

	/* get buffer structure */
	buff = (struct mb_buffer_struct *) buff_ptr;

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get store_ptr and kind for desired record */
	if (id < 0 || id >= buff->nbuffer)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_BUFFER_ID;
		}
	else
		{
		store_ptr = buff->buffer[id];
		*kind = buff->buffer_kind[id];
		*error = MB_ERROR_NO_ERROR;
		}

	/* if no error then proceed */
	if (status == MB_SUCCESS)
		{
		status = mb_extract_nav(verbose,mbio_ptr,
			store_ptr,kind,
			time_i,time_d,navlon,navlat,
			speed,heading,draft,
			roll,pitch,heave, 
			error);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:       %d\n",*kind);
		}
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR 
		&& *kind != MB_DATA_COMMENT)
		{
		fprintf(stderr,"dbg2       time_i[0]:     %d\n",time_i[0]);
		fprintf(stderr,"dbg2       time_i[1]:     %d\n",time_i[1]);
		fprintf(stderr,"dbg2       time_i[2]:     %d\n",time_i[2]);
		fprintf(stderr,"dbg2       time_i[3]:     %d\n",time_i[3]);
		fprintf(stderr,"dbg2       time_i[4]:     %d\n",time_i[4]);
		fprintf(stderr,"dbg2       time_i[5]:     %d\n",time_i[5]);
		fprintf(stderr,"dbg2       time_i[6]:     %d\n",time_i[6]);
		fprintf(stderr,"dbg2       time_d:        %f\n",*time_d);
		fprintf(stderr,"dbg2       longitude:     %f\n",*navlon);
		fprintf(stderr,"dbg2       latitude:      %f\n",*navlat);
		fprintf(stderr,"dbg2       speed:         %f\n",*speed);
		fprintf(stderr,"dbg2       heading:       %f\n",*heading);
		fprintf(stderr,"dbg2       draft:         %f\n",*draft);
		fprintf(stderr,"dbg2       roll:          %f\n",*roll);
		fprintf(stderr,"dbg2       pitch:         %f\n",*pitch);
		fprintf(stderr,"dbg2       heave:         %f\n",*heave);
		}
	if (verbose >= 2)
		{
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_buffer_insert(int verbose, void *buff_ptr, void *mbio_ptr,
		int id, int time_i[7], double time_d,
		double navlon, double navlat, 
		double speed, double heading,
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp,
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, 
		int *error)
{
	char	*function_name = "mb_buffer_insert";
	int	status = MB_SUCCESS;
	struct mb_buffer_struct *buff;
	struct mb_io_struct *mb_io_ptr;
	char	*store_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       id:         %d\n",id);
		fprintf(stderr,"dbg2       time_i[0]:  %d\n",time_i[0]);
		fprintf(stderr,"dbg2       time_i[1]:  %d\n",time_i[1]);
		fprintf(stderr,"dbg2       time_i[2]:  %d\n",time_i[2]);
		fprintf(stderr,"dbg2       time_i[3]:  %d\n",time_i[3]);
		fprintf(stderr,"dbg2       time_i[4]:  %d\n",time_i[4]);
		fprintf(stderr,"dbg2       time_i[5]:  %d\n",time_i[5]);
		fprintf(stderr,"dbg2       time_i[6]:  %d\n",time_i[6]);
		fprintf(stderr,"dbg2       time_d:     %f\n",time_d);
		fprintf(stderr,"dbg2       navlon:     %f\n",navlon);
		fprintf(stderr,"dbg2       navlat:     %f\n",navlat);
		fprintf(stderr,"dbg2       speed:      %f\n",speed);
		fprintf(stderr,"dbg2       heading:    %f\n",heading);
		fprintf(stderr,"dbg4       nbath:      %d\n",nbath);
		if (nbath > 0)
		  {
		  fprintf(stderr,"dbg4       beam   flag   bath  crosstrack alongtrack\n");
		  for (i=0;i<nbath;i++)
		    fprintf(stderr,"dbg4       %4d   %3d   %f    %f     %f\n",
			i,beamflag[i],bath[i],
			bathacrosstrack[i],bathalongtrack[i]);
		  }
		fprintf(stderr,"dbg4       namp:          %d\n",namp);
		if (namp > 0)
		  {
		  fprintf(stderr,"dbg4       beam    amp  crosstrack alongtrack\n");
		  for (i=0;i<nbath;i++)
		    fprintf(stderr,"dbg4       %4d   %f    %f     %f\n",
			i,amp[i],bathacrosstrack[i],bathalongtrack[i]);
		  }
		fprintf(stderr,"dbg4       nss:           %d\n",nss);
		if (nss > 0)
		  {
		  fprintf(stderr,"dbg4       pixel sidescan crosstrack alongtrack\n");
		  for (i=0;i<nss;i++)
		    fprintf(stderr,"dbg4       %4d   %f    %f     %f\n",
			i,ss[i],ssacrosstrack[i],ssalongtrack[i]);
		  }
		fprintf(stderr,"dbg2       comment:    %s\n",comment);
		}

	/* get buffer structure */
	buff = (struct mb_buffer_struct *) buff_ptr;

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get store_ptr for specified record */
	if (id < 0 || id >= buff->nbuffer)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_BUFFER_ID;
		}
	else
		{
		store_ptr = buff->buffer[id];
		}

	status = mb_insert(verbose,mbio_ptr,store_ptr,
		buff->buffer_kind[id], 
		time_i,time_d,navlon,navlat,speed,heading,
		nbath,namp,nss,
		beamflag,bath,amp,
		bathacrosstrack,bathalongtrack,
		ss,ssacrosstrack,ssalongtrack,
		comment,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_buffer_insert_nav(int verbose, void *buff_ptr, void *mbio_ptr,
		int id, int time_i[7], double time_d,
		double navlon, double navlat, 
		double speed, double heading, double draft,
		double roll, double pitch, double heave,
		int *error)
{
	char	*function_name = "mb_buffer_insert_nav";
	int	status = MB_SUCCESS;
	struct mb_buffer_struct *buff;
	struct mb_io_struct *mb_io_ptr;
	char	*store_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       id:         %d\n",id);
		fprintf(stderr,"dbg2       time_i[0]:  %d\n",time_i[0]);
		fprintf(stderr,"dbg2       time_i[1]:  %d\n",time_i[1]);
		fprintf(stderr,"dbg2       time_i[2]:  %d\n",time_i[2]);
		fprintf(stderr,"dbg2       time_i[3]:  %d\n",time_i[3]);
		fprintf(stderr,"dbg2       time_i[4]:  %d\n",time_i[4]);
		fprintf(stderr,"dbg2       time_i[5]:  %d\n",time_i[5]);
		fprintf(stderr,"dbg2       time_i[6]:  %d\n",time_i[6]);
		fprintf(stderr,"dbg2       time_d:     %f\n",time_d);
		fprintf(stderr,"dbg2       navlon:     %f\n",navlon);
		fprintf(stderr,"dbg2       navlat:     %f\n",navlat);
		fprintf(stderr,"dbg2       speed:      %f\n",speed);
		fprintf(stderr,"dbg2       heading:    %f\n",heading);
		fprintf(stderr,"dbg2       draft:      %f\n",draft);
		fprintf(stderr,"dbg2       roll:       %f\n",roll);
		fprintf(stderr,"dbg2       pitch:      %f\n",pitch);
		fprintf(stderr,"dbg2       heave:      %f\n",heave);
		}

	/* get buffer structure */
	buff = (struct mb_buffer_struct *) buff_ptr;

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get store_ptr for specified record */
	if (id < 0 || id >= buff->nbuffer)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_BUFFER_ID;
		}
	else
		{
		store_ptr = buff->buffer[id];
		}

	status = mb_insert_nav(verbose,
		mbio_ptr,store_ptr,
		time_i,time_d,navlon,navlat,
		speed,heading,draft,
		roll,pitch,heave, 
		error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_buffer_get_kind(int verbose, void *buff_ptr, void *mbio_ptr,
			int id, int *kind, 
			int *error)
{
	char	*function_name = "mb_buffer_get_kind";
	int	status = MB_SUCCESS;
	struct mb_buffer_struct *buff;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       id:         %d\n",id);
		}

	/* get buffer structure */
	buff = (struct mb_buffer_struct *) buff_ptr;

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get store_ptr for specified record */
	if (id < 0 || id >= buff->nbuffer)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_BUFFER_ID;
		}
	else
		{
		*kind = buff->buffer_kind[id];
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:       %d\n",*kind);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_buffer_get_ptr(int verbose, void *buff_ptr, void *mbio_ptr,
			int id, void **store_ptr, 
			int *error)
{
	char	*function_name = "mb_buffer_get_ptr";
	int	status = MB_SUCCESS;
	struct mb_buffer_struct *buff;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       id:         %d\n",id);
		}

	/* get buffer structure */
	buff = (struct mb_buffer_struct *) buff_ptr;

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get store_ptr for specified record */
	if (id < 0 || id >= buff->nbuffer)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_BUFFER_ID;
		}
	else
		{
		*store_ptr = buff->buffer[id];
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)*store_ptr);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
