/*--------------------------------------------------------------------
 *    The MB-system:	mb_buffer.c	2/25/93
 *    $Id: mb_buffer.c,v 4.6 1995-08-17 14:42:45 caress Exp $
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
 * mb_buffer.c contains the functions for buffered i/o of multibeam
 * data.   
 * These functions include:
 *   mb_buffer_init	- initialize buffer structure
 *   mb_buffer_close	- close buffer structure
 *   mb_buffer_load	- load data from file into buffer
 *   mb_buffer_dump	- dump data from buffer into file
 *   mb_buffer_clear	- clear data from buffer
 *   mb_buffer_info	- get system and kind of specified record 
 *   				in buffer
 *   mb_buffer_extract	- extract navigation and bathymetry/backscatter 
 *   				from next suitable record in buffer
 *   mb_buffer_insert	- insert altered navigation and 
 *   				bathymetry/backscatter data into 
 *   				record in buffer
 *   mb_buffer_insert_nav - insert altered navigation into 
 *   				record in buffer
 *   mb_buffer_alloc	- allocate memory in buffer
 *   mb_buffer_deall	- deallocate memory in buffer
 *
 * Author:	D. W. Caress
 * Date:	February 25, 1993
 *
 * $Log: not supported by cvs2svn $
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

/*--------------------------------------------------------------------*/
int mb_buffer_init(verbose,buff_ptr,error)
int	verbose;
char	**buff_ptr;
int	*error;
{
  static char rcs_id[]="$Id: mb_buffer.c,v 4.6 1995-08-17 14:42:45 caress Exp $";
	char	*function_name = "mb_buffer_init";
	int	status = MB_SUCCESS;
	struct mb_buffer_struct *buff;
	struct mb_io_struct *mb_io_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		}

	/* allocate memory for data structure */
	status = mb_malloc(verbose,sizeof(struct mb_buffer_struct),buff_ptr,error);
	buff = (struct mb_buffer_struct *) *buff_ptr;

	/* set nbuffer to zero */
	buff->nbuffer = 0;
	for (i=0;i<MB_BUFFER_MAX;i++)
		buff->buffer[i] = NULL;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       buff_ptr:   %d\n",*buff_ptr);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_buffer_close(verbose,buff_ptr,mbio_ptr,error)
int	verbose;
char	**buff_ptr;
char	*mbio_ptr;
int	*error;
{
	char	*function_name = "mb_buffer_close";
	int	status = MB_SUCCESS;
	struct mb_buffer_struct *buff;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buff_ptr:   %d\n",*buff_ptr);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
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
				fprintf(stderr,"dbg4       Record[%d] pointer: %d\n",i,buff->buffer[i]);
			}
		for (i=0;i<buff->nbuffer;i++);
			status = mb_buffer_deall(verbose,*buff_ptr,mbio_ptr,
				&buff->buffer[i],error);
		}

	/* deallocate memory for data structure */
	status = mb_free(verbose,&buff_ptr,error);

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
int mb_buffer_load(verbose,buff_ptr,mbio_ptr,nwant,nload,nbuff,error)
int	verbose;
char	*buff_ptr;
char	*mbio_ptr;
int	nwant;
int	*nload;
int	*nbuff;
int	*error;
{
	char	*function_name = "mb_buffer_load";
	int	status = MB_SUCCESS;
	struct mb_buffer_struct *buff;
	struct mb_io_struct *mb_io_ptr;
	char	*store_ptr;
	int	nget;
	int	kind;

	/* mbio dummy read values */
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	int	nbath;
	int	namp;
	int	nss;
	double	*bath;
	double	*amp;
	double	*bathacrosstrack;
	double	*bathalongtrack;
	double	*ss;
	double	*ssacrosstrack;
	double	*ssalongtrack;
	char	comment[256];

	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buff_ptr:   %d\n",buff_ptr);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       nwant:      %d\n",nwant);
		}

	/* get buffer structure */
	buff = (struct mb_buffer_struct *) buff_ptr;

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* coopt binning arrays in mb_io_ptr structure to use
		for dummy reading arrays - there's no binning
		when using buffer */
	bath = mb_io_ptr->bath;
	amp = mb_io_ptr->amp;
	bathacrosstrack = mb_io_ptr->bath_acrosstrack;
	bathalongtrack = mb_io_ptr->bath_alongtrack;
	ss = mb_io_ptr->ss;
	ssacrosstrack = mb_io_ptr->ss_acrosstrack;
	ssalongtrack = mb_io_ptr->ss_alongtrack;

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
		status = mb_get_all(verbose,mbio_ptr,&store_ptr,&kind,
			time_i,&time_d,&navlon,&navlat,&speed,
			&heading,&distance,
			&nbath,&namp,&nss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,error);

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  New record read by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4       kind:          %d\n",kind);
			fprintf(stderr,"dbg4       store_ptr:     %d\n",
				store_ptr);
			fprintf(stderr,"dbg4       nbuffer:       %d\n",
				buff->nbuffer);
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

			/* print debug statements */
			status = mb_buffer_alloc(verbose,buff_ptr,mbio_ptr,
				&buff->buffer[buff->nbuffer],
				error);
			status = mb_copy_record(verbose,buff_ptr,mbio_ptr,
				store_ptr,
				buff->buffer[buff->nbuffer],error);
			buff->buffer_kind[buff->nbuffer] = kind;
			buff->nbuffer++;
			if (status == MB_SUCCESS)
				(*nload)++;
			}

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  Buffer status in MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4       nbuffer:       %d\n",
				buff->nbuffer);
			fprintf(stderr,"dbg4       nload:         %d\n",
				*nload);
			fprintf(stderr,"dbg4       nget:          %d\n",
				nget);
			fprintf(stderr,"dbg4       nwant:         %d\n",
				nwant);
			fprintf(stderr,"dbg4       error:         %d\n",
				*error);
			fprintf(stderr,"dbg4       status:        %d\n",
				status);
			for (i=0;i<buff->nbuffer;i++)
				{
				fprintf(stderr,"dbg4       i:%d  kind:%d  ptr:%d\n",
				i,buff->buffer_kind[i],buff->buffer[i]);
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
		status == MB_FAILURE;
		*error = MB_ERROR_NO_DATA_REQUESTED;
		}
	else if (nget <= 0)
		{
		status == MB_FAILURE;
		*error = MB_ERROR_BUFFER_FULL;
		}
	else if (*nload <= 0 && *error <= MB_ERROR_NO_ERROR)
		{
		status == MB_FAILURE;
		*error = MB_ERROR_NO_DATA_LOADED;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
int mb_buffer_dump(verbose,buff_ptr,mbio_ptr,nhold,ndump,nbuff,error)
int	verbose;
char	*buff_ptr;
char	*mbio_ptr;
int	nhold;
int	*ndump;
int	*nbuff;
int	*error;
{
	char	*function_name = "mb_buffer_dump";
	int	status = MB_SUCCESS;
	struct mb_buffer_struct *buff;
	struct mb_io_struct *mb_io_ptr;

	/* mbio dummy write values */
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	int	nbath;
	int	namp;
	int	nss;
	double	*bath;
	double	*amp;
	double	*bathacrosstrack;
	double	*bathalongtrack;
	double	*ss;
	double	*ssacrosstrack;
	double	*ssalongtrack;
	char	comment[256];

	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buff_ptr:   %d\n",buff_ptr);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
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
			fprintf(stderr,"dbg4       i:%d  kind:%d  ptr:%d\n",
			i,buff->buffer_kind[i],buff->buffer[i]);
			}
		}

	/* set beam and pixel numbers */
	nbath = mb_io_ptr->beams_bath;
	namp = mb_io_ptr->beams_amp;
	nss = mb_io_ptr->pixels_ss;

	/* dump records from buffer */
	if (status == MB_SUCCESS)
		{
		for (i=0;i<*ndump;i++)
			{
			/* print debug statements */
			if (verbose >= 4)
				{
				fprintf(stderr,"\ndbg4  Dumping record in MBIO function <%s>\n",
					function_name);
				fprintf(stderr,"dbg4       record:      %d\n",
					i);
				fprintf(stderr,"dbg4       ptr:         %d\n",
					buff->buffer[i]);
				fprintf(stderr,"dbg4       kind:        %d\n",
					buff->buffer_kind[i]);
				}

			status = mb_put_all(verbose,mbio_ptr,
				buff->buffer[i],MB_NO,
				buff->buffer_kind[i],
				time_i,time_d,
				navlon,navlat,speed,heading,
				nbath,namp,nss,
				bath,amp,bathacrosstrack,bathalongtrack,
				ss,ssacrosstrack,ssalongtrack,
				comment,error);

			/* print debug statements */
			if (verbose >= 4)
				{
				fprintf(stderr,"\ndbg4  Deallocating record in MBIO function <%s>\n",
					function_name);
				fprintf(stderr,"dbg4       record:      %d\n",
					i);
				fprintf(stderr,"dbg4       ptr:         %d\n",
					buff->buffer[i]);
				fprintf(stderr,"dbg4       kind:        %d\n",
					buff->buffer_kind[i]);
				}

			status = mb_buffer_deall(verbose,buff_ptr,mbio_ptr,
				&buff->buffer[i],error);
			buff->buffer[i] = NULL;

			/* print debug statements */
			if (verbose >= 4)
				{
				fprintf(stderr,"\ndbg4  Done dumping record in MBIO function <%s>\n",
					function_name);
				fprintf(stderr,"dbg4       record:      %d\n",
					i);
				fprintf(stderr,"dbg4       ptr:         %d\n",
					buff->buffer[i]);
				fprintf(stderr,"dbg4       kind:        %d\n",
					buff->buffer_kind[i]);
				}
			}
		for (i=0;i<nhold;i++)
			{
			/* print debug statements */
			if (verbose >= 4)
				{
				fprintf(stderr,"\ndbg4  Moving buffer record in MBIO function <%s>\n",
					function_name);
				fprintf(stderr,"dbg4       old:         %d\n",
					*ndump + i);
				fprintf(stderr,"dbg4       new:         %d\n",
					i);
				fprintf(stderr,"dbg4       old ptr:     %d\n",
					buff->buffer[*ndump + i]);
				fprintf(stderr,"dbg4       new ptr:     %d\n",
					buff->buffer[i]);
				fprintf(stderr,"dbg4       old kind:    %d\n",
					buff->buffer_kind[*ndump + i]);
				fprintf(stderr,"dbg4       new kind:    %d\n",
					buff->buffer_kind[i]);
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
				fprintf(stderr,"\ndbg4  Done moving buffer record in MBIO function <%s>\n",
					function_name);
				fprintf(stderr,"dbg4       old:         %d\n",
					*ndump + i);
				fprintf(stderr,"dbg4       new:         %d\n",
					i);
				fprintf(stderr,"dbg4       old ptr:     %d\n",
					buff->buffer[*ndump + i]);
				fprintf(stderr,"dbg4       new ptr:     %d\n",
					buff->buffer[i]);
				fprintf(stderr,"dbg4       old kind:    %d\n",
					buff->buffer_kind[*ndump + i]);
				fprintf(stderr,"dbg4       new kind:    %d\n",
					buff->buffer_kind[i]);
				}
			}
		buff->nbuffer = buff->nbuffer - *ndump;
		}

	/* print debug statements */
	if (verbose >= 4)
		{
		fprintf(stderr,"\ndbg4  Buffer list at end of MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg4       nbuffer:     %d\n",buff->nbuffer);
		for (i=0;i<buff->nbuffer;i++)
			{
			fprintf(stderr,"dbg4       i:%d  kind:%d  ptr:%d\n",
			i,buff->buffer_kind[i],buff->buffer[i]);
			}
		}

	/* set return value */
	*nbuff = buff->nbuffer;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
int mb_buffer_clear(verbose,buff_ptr,mbio_ptr,nhold,ndump,nbuff,error)
int	verbose;
char	*buff_ptr;
char	*mbio_ptr;
int	nhold;
int	*ndump;
int	*nbuff;
int	*error;
{
	char	*function_name = "mb_buffer_clear";
	int	status = MB_SUCCESS;
	struct mb_buffer_struct *buff;
	struct mb_io_struct *mb_io_ptr;

	/* mbio dummy write values */
	int	time_i[7];
	double	time_d;
	double	navlon;
	double	navlat;
	double	speed;
	double	heading;
	double	distance;
	int	nbath;
	int	namp;
	int	nss;
	double	*bath;
	double	*amp;
	double	*bathacrosstrack;
	double	*bathalongtrack;
	double	*ss;
	double	*ssacrosstrack;
	double	*ssalongtrack;
	char	comment[256];

	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buff_ptr:   %d\n",buff_ptr);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
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
			fprintf(stderr,"dbg4       i:%d  kind:%d  ptr:%d\n",
			i,buff->buffer_kind[i],buff->buffer[i]);
			}
		}

	/* set beam and pixel numbers */
	nbath = mb_io_ptr->beams_bath;
	namp = mb_io_ptr->beams_amp;
	nss = mb_io_ptr->pixels_ss;

	/* dump records from buffer */
	if (status == MB_SUCCESS)
		{
		for (i=0;i<*ndump;i++)
			{
			/* print debug statements */
			if (verbose >= 4)
				{
				fprintf(stderr,"\ndbg4  Deallocating record in MBIO function <%s>\n",
					function_name);
				fprintf(stderr,"dbg4       record:      %d\n",
					i);
				fprintf(stderr,"dbg4       ptr:         %d\n",
					buff->buffer[i]);
				fprintf(stderr,"dbg4       kind:        %d\n",
					buff->buffer_kind[i]);
				}

			status = mb_buffer_deall(verbose,buff_ptr,mbio_ptr,
				&buff->buffer[i],error);
			buff->buffer[i] = NULL;

			/* print debug statements */
			if (verbose >= 4)
				{
				fprintf(stderr,"\ndbg4  Done dumping record in MBIO function <%s>\n",
					function_name);
				fprintf(stderr,"dbg4       record:      %d\n",
					i);
				fprintf(stderr,"dbg4       ptr:         %d\n",
					buff->buffer[i]);
				fprintf(stderr,"dbg4       kind:        %d\n",
					buff->buffer_kind[i]);
				}
			}
		for (i=0;i<nhold;i++)
			{
			/* print debug statements */
			if (verbose >= 4)
				{
				fprintf(stderr,"\ndbg4  Moving buffer record in MBIO function <%s>\n",
					function_name);
				fprintf(stderr,"dbg4       old:         %d\n",
					*ndump + i);
				fprintf(stderr,"dbg4       new:         %d\n",
					i);
				fprintf(stderr,"dbg4       old ptr:     %d\n",
					buff->buffer[*ndump + i]);
				fprintf(stderr,"dbg4       new ptr:     %d\n",
					buff->buffer[i]);
				fprintf(stderr,"dbg4       old kind:    %d\n",
					buff->buffer_kind[*ndump + i]);
				fprintf(stderr,"dbg4       new kind:    %d\n",
					buff->buffer_kind[i]);
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
				fprintf(stderr,"\ndbg4  Done moving buffer record in MBIO function <%s>\n",
					function_name);
				fprintf(stderr,"dbg4       old:         %d\n",
					*ndump + i);
				fprintf(stderr,"dbg4       new:         %d\n",
					i);
				fprintf(stderr,"dbg4       old ptr:     %d\n",
					buff->buffer[*ndump + i]);
				fprintf(stderr,"dbg4       new ptr:     %d\n",
					buff->buffer[i]);
				fprintf(stderr,"dbg4       old kind:    %d\n",
					buff->buffer_kind[*ndump + i]);
				fprintf(stderr,"dbg4       new kind:    %d\n",
					buff->buffer_kind[i]);
				}
			}
		buff->nbuffer = buff->nbuffer - *ndump;
		}

	/* print debug statements */
	if (verbose >= 4)
		{
		fprintf(stderr,"\ndbg4  Buffer list at end of MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg4       nbuffer:     %d\n",buff->nbuffer);
		for (i=0;i<buff->nbuffer;i++)
			{
			fprintf(stderr,"dbg4       i:%d  kind:%d  ptr:%d\n",
			i,buff->buffer_kind[i],buff->buffer[i]);
			}
		}

	/* set return value */
	*nbuff = buff->nbuffer;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
int mb_buffer_info(verbose,buff_ptr,mbio_ptr,id,system,kind,error)
int	verbose;
char	*buff_ptr;
char	*mbio_ptr;
int	id;
int	*system;
int	*kind;
int	*error;
{
	char	*function_name = "mb_buffer_info";
	int	status = MB_SUCCESS;
	struct mb_buffer_struct *buff;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       id:         %d\n",id);
		}

	/* get buffer structure */
	buff = (struct mb_buffer_struct *) buff_ptr;

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get multibeam system id */
	*system = mb_system_table[mb_io_ptr->format_num];

	/* get record kind */
	if (id < 0 || id >= buff->nbuffer)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_BUFFER_ID;
		}
	else
		{
		*kind = buff->buffer_kind[id];
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       system:     %d\n",*system);
		fprintf(stderr,"dbg2       kind:       %d\n",*kind);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_buffer_get_next_data(verbose,buff_ptr,mbio_ptr,start,id,
		time_i,time_d,navlon,navlat,speed,heading,
		nbath,namp,nss,
		bath,amp,bathacrosstrack,bathalongtrack,
		ss,ssacrosstrack,ssalongtrack,error)
int	verbose;
char	*buff_ptr;
char	*mbio_ptr;
int	start;
int	*id;
int	time_i[7];
double	*time_d;
double	*navlon;
double	*navlat;
double	*speed;
double	*heading;
int	*nbath;
int	*namp;
int	*nss;
double	*bath;
double	*amp;
double	*bathacrosstrack;
double	*bathalongtrack;
double	*ss;
double	*ssacrosstrack;
double	*ssalongtrack;
int	*error;
{
	char	*function_name = "mb_buffer_get_next_data";
	int	status = MB_SUCCESS;
	struct mb_buffer_struct *buff;
	struct mb_io_struct *mb_io_ptr;
	char	*store_ptr;
	int	system;
	int	kind;
	char	comment[200];
	int	found;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buff_ptr:   %d\n",buff_ptr);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
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
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
		  fprintf(stderr,"dbg4       beam   bath  crosstrack alongtrack\n");
		  for (i=0;i<*nbath;i++)
		    fprintf(stderr,"dbg4       %4d   %f    %f     %f\n",
			i,bath[i],bathacrosstrack[i],bathalongtrack[i]);
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
int mb_buffer_get_next_nav(verbose,buff_ptr,mbio_ptr,start,id,
		time_i,time_d,navlon,navlat,speed,heading,
		roll,pitch,heave,error)
int	verbose;
char	*buff_ptr;
char	*mbio_ptr;
int	start;
int	*id;
int	time_i[7];
double	*time_d;
double	*navlon;
double	*navlat;
double	*speed;
double	*heading;
double	*roll;
double	*pitch;
double	*heave;
int	*error;
{
	char	*function_name = "mb_buffer_get_next_data";
	int	status = MB_SUCCESS;
	struct mb_buffer_struct *buff;
	struct mb_io_struct *mb_io_ptr;
	char	*store_ptr;
	int	system;
	int	kind;
	char	comment[200];
	int	found;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buff_ptr:   %d\n",buff_ptr);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
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
		status = mb_buffer_extract_nav(verbose,
			buff_ptr,mbio_ptr,*id,&kind,
			time_i,time_d,navlon,navlat,speed,heading,
			roll,pitch,heave,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
int mb_buffer_extract(verbose,buff_ptr,mbio_ptr,id,kind,
		time_i,time_d,navlon,navlat,speed,heading,
		nbath,namp,nss,
		bath,amp,bathacrosstrack,bathalongtrack,
		ss,ssacrosstrack,ssalongtrack,
		comment,error)
int	verbose;
char	*buff_ptr;
char	*mbio_ptr;
int	id;
int	*kind;
int	time_i[7];
double	*time_d;
double	*navlon;
double	*navlat;
double	*speed;
double	*heading;
int	*nbath;
int	*namp;
int	*nss;
double	*bath;
double	*amp;
double	*bathacrosstrack;
double	*bathalongtrack;
double	*ss;
double	*ssacrosstrack;
double	*ssalongtrack;
char	*comment;
int	*error;
{
	char	*function_name = "mb_buffer_extract";
	int	status = MB_SUCCESS;
	struct mb_buffer_struct *buff;
	struct mb_io_struct *mb_io_ptr;
	char	*store_ptr;
	int	system;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buff_ptr:   %d\n",buff_ptr);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
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

		/* get multibeam system id */
		system = mb_system_table[mb_io_ptr->format_num];

		/* call appropriate extraction routine */
		if (system == MB_SYS_SB)
			{
			status = mbsys_sb_extract(verbose,mbio_ptr,
				store_ptr,kind,
				time_i,time_d,navlon,navlat,speed,heading,
				nbath,namp,nss,
				bath,amp,bathacrosstrack,bathalongtrack,
				ss,ssacrosstrack,ssalongtrack,
				comment,error);
			}
		else if (system == MB_SYS_HSDS)
			{
			status = mbsys_hsds_extract(verbose,mbio_ptr,
				store_ptr,kind,
				time_i,time_d,navlon,navlat,speed,heading,
				nbath,namp,nss,
				bath,amp,bathacrosstrack,bathalongtrack,
				ss,ssacrosstrack,ssalongtrack,
				comment,error);
			}
		else if (system == MB_SYS_SB2000)
			{
			status = mbsys_sb2000_extract(verbose,mbio_ptr,
				store_ptr,kind,
				time_i,time_d,navlon,navlat,speed,heading,
				nbath,namp,nss,
				bath,amp,bathacrosstrack,bathalongtrack,
				ss,ssacrosstrack,ssalongtrack,
				comment,error);
			}
		else if (system == MB_SYS_SB2100)
			{
			status = mbsys_sb2100_extract(verbose,mbio_ptr,
				store_ptr,kind,
				time_i,time_d,navlon,navlat,speed,heading,
				nbath,namp,nss,
				bath,amp,bathacrosstrack,bathalongtrack,
				ss,ssacrosstrack,ssalongtrack,
				comment,error);
			}
		else if (system == MB_SYS_SIMRAD)
			{
			status = mbsys_simrad_extract(verbose,mbio_ptr,
				store_ptr,kind,
				time_i,time_d,navlon,navlat,speed,heading,
				nbath,namp,nss,
				bath,amp,bathacrosstrack,bathalongtrack,
				ss,ssacrosstrack,ssalongtrack,
				comment,error);
			}
		else if (system == MB_SYS_MR1)
			{
			status = mbsys_mr1_extract(verbose,mbio_ptr,
				store_ptr,kind,
				time_i,time_d,navlon,navlat,speed,heading,
				nbath,namp,nss,
				bath,amp,bathacrosstrack,bathalongtrack,
				ss,ssacrosstrack,ssalongtrack,
				comment,error);
			}
		else if (system == MB_SYS_LDEOIH)
			{
			status = mbsys_ldeoih_extract(verbose,mbio_ptr,
				store_ptr,kind,
				time_i,time_d,navlon,navlat,speed,heading,
				nbath,namp,nss,
				bath,amp,bathacrosstrack,bathalongtrack,
				ss,ssacrosstrack,ssalongtrack,
				comment,error);
			}
		else if (system == MB_SYS_RESON)
			{
			status = mbsys_reson_extract(verbose,mbio_ptr,
				store_ptr,kind,
				time_i,time_d,navlon,navlat,speed,heading,
				nbath,namp,nss,
				bath,amp,bathacrosstrack,bathalongtrack,
				ss,ssacrosstrack,ssalongtrack,
				comment,error);
			}
		else if (system == MB_SYS_ELAC)
			{
			status = mbsys_elac_extract(verbose,mbio_ptr,
				store_ptr,kind,
				time_i,time_d,navlon,navlat,speed,heading,
				nbath,namp,nss,
				bath,amp,bathacrosstrack,bathalongtrack,
				ss,ssacrosstrack,ssalongtrack,
				comment,error);
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_SYSTEM;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
		  fprintf(stderr,"dbg4       beam   bath  crosstrack alongtrack\n");
		  for (i=0;i<*nbath;i++)
		    fprintf(stderr,"dbg4       %4d   %f    %f     %f\n",
			i,bath[i],bathacrosstrack[i],bathalongtrack[i]);
		  }
		fprintf(stderr,"dbg4       namp:          %d\n",*namp);
		if (*namp > 0)
		  {
		  fprintf(stderr,"dbg4       beam    amp  crosstrack alongtrack\n");
		  for (i=0;i<*nbath;i++)
		    fprintf(stderr,"dbg4       %f   %f    %f     %f\n",
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
int mb_buffer_extract_nav(verbose,buff_ptr,mbio_ptr,id,kind,
		time_i,time_d,navlon,navlat,speed,heading,
		roll,pitch,heave, 
		error)
int	verbose;
char	*buff_ptr;
char	*mbio_ptr;
int	id;
int	*kind;
int	time_i[7];
double	*time_d;
double	*navlon;
double	*navlat;
double	*speed;
double	*heading;
double	*roll;
double	*pitch;
double	*heave;
int	*error;
{
	char	*function_name = "mb_buffer_extract_nav";
	int	status = MB_SUCCESS;
	struct mb_buffer_struct *buff;
	struct mb_io_struct *mb_io_ptr;
	char	*store_ptr;
	int	system;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buff_ptr:   %d\n",buff_ptr);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
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

		/* get multibeam system id */
		system = mb_system_table[mb_io_ptr->format_num];

		/* call appropriate extraction routine */
		if (system == MB_SYS_SB)
			{
			status = mbsys_sb_extract_nav(verbose,mbio_ptr,
				store_ptr,kind,
				time_i,time_d,navlon,navlat,speed,heading,
				roll,pitch,heave, 
				error);
			}
		else if (system == MB_SYS_HSDS)
			{
			status = mbsys_hsds_extract_nav(verbose,mbio_ptr,
				store_ptr,kind,
				time_i,time_d,navlon,navlat,speed,heading,
				roll,pitch,heave, 
				error);
			}
		else if (system == MB_SYS_SB2000)
			{
			status = mbsys_sb2000_extract_nav(verbose,mbio_ptr,
				store_ptr,kind,
				time_i,time_d,navlon,navlat,speed,heading,
				roll,pitch,heave, 
				error);
			}
		else if (system == MB_SYS_SB2100)
			{
			status = mbsys_sb2100_extract_nav(verbose,mbio_ptr,
				store_ptr,kind,
				time_i,time_d,navlon,navlat,speed,heading,
				roll,pitch,heave, 
				error);
			}
		else if (system == MB_SYS_SIMRAD)
			{
			status = mbsys_simrad_extract_nav(verbose,mbio_ptr,
				store_ptr,kind,
				time_i,time_d,navlon,navlat,speed,heading,
				roll,pitch,heave, 
				error);
			}
		else if (system == MB_SYS_MR1)
			{
			status = mbsys_mr1_extract_nav(verbose,mbio_ptr,
				store_ptr,kind,
				time_i,time_d,navlon,navlat,speed,heading,
				roll,pitch,heave, 
				error);
			}
		else if (system == MB_SYS_LDEOIH)
			{
			status = mbsys_ldeoih_extract_nav(verbose,mbio_ptr,
				store_ptr,kind,
				time_i,time_d,navlon,navlat,speed,heading,
				roll,pitch,heave, 
				error);
			}
		else if (system == MB_SYS_RESON)
			{
			status = mbsys_reson_extract_nav(verbose,mbio_ptr,
				store_ptr,kind,
				time_i,time_d,navlon,navlat,speed,heading,
				roll,pitch,heave, 
				error);
			}
		else if (system == MB_SYS_ELAC)
			{
			status = mbsys_elac_extract_nav(verbose,mbio_ptr,
				store_ptr,kind,
				time_i,time_d,navlon,navlat,speed,heading,
				roll,pitch,heave, 
				error);
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_BAD_SYSTEM;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
int mb_buffer_insert(verbose,buff_ptr,mbio_ptr,id,
		time_i,time_d,navlon,navlat,speed,heading,
		nbath,namp,nss,
		bath,amp,bathacrosstrack,bathalongtrack,
		ss,ssacrosstrack,ssalongtrack,
		comment,error)
int	verbose;
char	*buff_ptr;
char	*mbio_ptr;
int	id;
int	time_i[7];
double	time_d;
double	navlon;
double	navlat;
double	speed;
double	heading;
int	nbath;
int	namp;
int	nss;
double	*bath;
double	*amp;
double	*bathacrosstrack;
double	*bathalongtrack;
double	*ss;
double	*ssacrosstrack;
double	*ssalongtrack;
char	*comment;
int	*error;
{
	char	*function_name = "mb_buffer_insert";
	int	status = MB_SUCCESS;
	struct mb_buffer_struct *buff;
	struct mb_io_struct *mb_io_ptr;
	char	*store_ptr;
	int	system;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
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
		  fprintf(stderr,"dbg4       beam   bath  crosstrack alongtrack\n");
		  for (i=0;i<nbath;i++)
		    fprintf(stderr,"dbg4       %4d   %f    %f     %f\n",
			i,bath[i],bathacrosstrack[i],bathalongtrack[i]);
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

	/* get multibeam system id */
	system = mb_system_table[mb_io_ptr->format_num];

	/* call appropriate insertion routine */
	if (system == MB_SYS_SB)
		{
		status = mbsys_sb_insert(verbose,mbio_ptr,store_ptr,
			time_i,time_d,navlon,navlat,speed,heading,
			nbath,namp,nss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,error);
		}
	else if (system == MB_SYS_HSDS)
		{
		status = mbsys_hsds_insert(verbose,mbio_ptr,store_ptr,
			time_i,time_d,navlon,navlat,speed,heading,
			nbath,namp,nss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,error);
		}
	else if (system == MB_SYS_SB2000)
		{
		status = mbsys_sb2000_insert(verbose,mbio_ptr,store_ptr,
			time_i,time_d,navlon,navlat,speed,heading,
			nbath,namp,nss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,error);
		}
	else if (system == MB_SYS_SB2100)
		{
		status = mbsys_sb2100_insert(verbose,mbio_ptr,store_ptr,
			time_i,time_d,navlon,navlat,speed,heading,
			nbath,namp,nss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,error);
		}
	else if (system == MB_SYS_SIMRAD)
		{
		status = mbsys_simrad_insert(verbose,mbio_ptr,store_ptr,
			time_i,time_d,navlon,navlat,speed,heading,
			nbath,namp,nss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,error);
		}
	else if (system == MB_SYS_MR1)
		{
		status = mbsys_mr1_insert(verbose,mbio_ptr,store_ptr,
			time_i,time_d,navlon,navlat,speed,heading,
			nbath,namp,nss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,error);
		}
	else if (system == MB_SYS_LDEOIH)
		{
		status = mbsys_ldeoih_insert(verbose,mbio_ptr,store_ptr,
			time_i,time_d,navlon,navlat,speed,heading,
			nbath,namp,nss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,error);
		}
	else if (system == MB_SYS_RESON)
		{
		status = mbsys_reson_insert(verbose,mbio_ptr,store_ptr,
			time_i,time_d,navlon,navlat,speed,heading,
			nbath,namp,nss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,error);
		}
	else if (system == MB_SYS_ELAC)
		{
		status = mbsys_elac_insert(verbose,mbio_ptr,store_ptr,
			time_i,time_d,navlon,navlat,speed,heading,
			nbath,namp,nss,
			bath,amp,bathacrosstrack,bathalongtrack,
			ss,ssacrosstrack,ssalongtrack,
			comment,error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_SYSTEM;
		}

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
int mb_buffer_insert_nav(verbose,buff_ptr,mbio_ptr,id,
		time_i,time_d,navlon,navlat,speed,heading,
		roll,pitch,heave, 
		error)
int	verbose;
char	*buff_ptr;
char	*mbio_ptr;
int	id;
int	time_i[7];
double	time_d;
double	navlon;
double	navlat;
double	speed;
double	heading;
double	roll;
double	pitch;
double	heave;
int	*error;
{
	char	*function_name = "mb_buffer_insert";
	int	status = MB_SUCCESS;
	struct mb_buffer_struct *buff;
	struct mb_io_struct *mb_io_ptr;
	char	*store_ptr;
	int	system;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
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
		fprintf(stderr,"dbg2       heading:    %f\n",roll);
		fprintf(stderr,"dbg2       heading:    %f\n",pitch);
		fprintf(stderr,"dbg2       heading:    %f\n",heave);
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

	/* get multibeam system id */
	system = mb_system_table[mb_io_ptr->format_num];

	/* call appropriate insertion routine */
	if (system == MB_SYS_SB)
		{
		status = mbsys_sb_insert_nav(verbose,
			mbio_ptr,store_ptr,
			time_i,time_d,navlon,navlat,speed,heading,
			roll,pitch,heave, 
			error);
		}
	else if (system == MB_SYS_HSDS)
		{
		status = mbsys_hsds_insert_nav(verbose,
			mbio_ptr,store_ptr,
			time_i,time_d,navlon,navlat,speed,heading,
			roll,pitch,heave, 
			error);
		}
	else if (system == MB_SYS_SB2000)
		{
		status = mbsys_sb2000_insert_nav(verbose,
			mbio_ptr,store_ptr,
			time_i,time_d,navlon,navlat,speed,heading,
			roll,pitch,heave, 
			error);
		}
	else if (system == MB_SYS_SB2100)
		{
		status = mbsys_sb2100_insert_nav(verbose,
			mbio_ptr,store_ptr,
			time_i,time_d,navlon,navlat,speed,heading,
			roll,pitch,heave, 
			error);
		}
	else if (system == MB_SYS_SIMRAD)
		{
		status = mbsys_simrad_insert_nav(verbose,
			mbio_ptr,store_ptr,
			time_i,time_d,navlon,navlat,speed,heading,
			roll,pitch,heave, 
			error);
		}
	else if (system == MB_SYS_MR1)
		{
		status = mbsys_mr1_insert_nav(verbose,
			mbio_ptr,store_ptr,
			time_i,time_d,navlon,navlat,speed,heading,
			roll,pitch,heave, 
			error);
		}
	else if (system == MB_SYS_LDEOIH)
		{
		status = mbsys_ldeoih_insert_nav(verbose,
			mbio_ptr,store_ptr,
			time_i,time_d,navlon,navlat,speed,heading,
			roll,pitch,heave, 
			error);
		}
	else if (system == MB_SYS_RESON)
		{
		status = mbsys_reson_insert_nav(verbose,
			mbio_ptr,store_ptr,
			time_i,time_d,navlon,navlat,speed,heading,
			roll,pitch,heave, 
			error);
		}
	else if (system == MB_SYS_ELAC)
		{
		status = mbsys_elac_insert_nav(verbose,
			mbio_ptr,store_ptr,
			time_i,time_d,navlon,navlat,speed,heading,
			roll,pitch,heave, 
			error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_SYSTEM;
		}

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
int mb_buffer_alloc(verbose,buff_ptr,mbio_ptr,store_ptr,error)
int	verbose;
char	*buff_ptr;
char	*mbio_ptr;
char	**store_ptr;
int	*error;
{
	char	*function_name = "mb_buffer_alloc";
	int	status = MB_SUCCESS;
	struct mb_buffer_struct *buff;
	struct mb_io_struct *mb_io_ptr;
	int	system;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		}

	/* get buffer structure */
	buff = (struct mb_buffer_struct *) buff_ptr;

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get multibeam system id */
	system = mb_system_table[mb_io_ptr->format_num];

	/* call appropriate memory allocation routine */
	if (system == MB_SYS_SB)
		{
		status = mbsys_sb_alloc(verbose,mbio_ptr,store_ptr,error);
		}
	else if (system == MB_SYS_HSDS)
		{
		status = mbsys_hsds_alloc(verbose,mbio_ptr,store_ptr,error);
		}
	else if (system == MB_SYS_SB2000)
		{
		status = mbsys_sb2000_alloc(verbose,mbio_ptr,store_ptr,error);
		}
	else if (system == MB_SYS_SB2100)
		{
		status = mbsys_sb2100_alloc(verbose,mbio_ptr,store_ptr,error);
		}
	else if (system == MB_SYS_SIMRAD)
		{
		status = mbsys_simrad_alloc(verbose,mbio_ptr,store_ptr,error);
		}
	else if (system == MB_SYS_MR1)
		{
		status = mbsys_mr1_alloc(verbose,mbio_ptr,store_ptr,error);
		}
	else if (system == MB_SYS_LDEOIH)
		{
		status = mbsys_ldeoih_alloc(verbose,mbio_ptr,store_ptr,error);
		}
	else if (system == MB_SYS_RESON)
		{
		status = mbsys_reson_alloc(verbose,mbio_ptr,store_ptr,error);
		}
	else if (system == MB_SYS_ELAC)
		{
		status = mbsys_elac_alloc(verbose,mbio_ptr,store_ptr,error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_SYSTEM;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       store_ptr:  %d\n",*store_ptr);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_buffer_deall(verbose,buff_ptr,mbio_ptr,store_ptr,error)
int	verbose;
char	*buff_ptr;
char	*mbio_ptr;
char	**store_ptr;
int	*error;
{
	char	*function_name = "mb_buffer_deall";
	int	status = MB_SUCCESS;
	struct mb_buffer_struct *buff;
	struct mb_io_struct *mb_io_ptr;
	int	system;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",*store_ptr);
		}

	/* get buffer structure */
	buff = (struct mb_buffer_struct *) buff_ptr;

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get multibeam system id */
	system = mb_system_table[mb_io_ptr->format_num];

	/* call appropriate memory deallocation routine */
	if (system == MB_SYS_SB)
		{
		status = mbsys_sb_deall(verbose,mbio_ptr,store_ptr,error);
		}
	else if (system == MB_SYS_HSDS)
		{
		status = mbsys_hsds_deall(verbose,mbio_ptr,store_ptr,error);
		}
	else if (system == MB_SYS_SB2000)
		{
		status = mbsys_sb2000_deall(verbose,mbio_ptr,store_ptr,error);
		}
	else if (system == MB_SYS_SB2100)
		{
		status = mbsys_sb2100_deall(verbose,mbio_ptr,store_ptr,error);
		}
	else if (system == MB_SYS_SIMRAD)
		{
		status = mbsys_simrad_deall(verbose,mbio_ptr,store_ptr,error);
		}
	else if (system == MB_SYS_MR1)
		{
		status = mbsys_mr1_deall(verbose,mbio_ptr,store_ptr,error);
		}
	else if (system == MB_SYS_LDEOIH)
		{
		status = mbsys_ldeoih_deall(verbose,mbio_ptr,store_ptr,error);
		}
	else if (system == MB_SYS_RESON)
		{
		status = mbsys_reson_deall(verbose,mbio_ptr,store_ptr,error);
		}
	else if (system == MB_SYS_ELAC)
		{
		status = mbsys_elac_deall(verbose,mbio_ptr,store_ptr,error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_SYSTEM;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_copy_record(verbose,buff_ptr,mbio_ptr,store_ptr,copy_ptr,error)
int	verbose;
char	*buff_ptr;
char	*mbio_ptr;
char	*store_ptr;
char	*copy_ptr;
int	*error;
{
	char	*function_name = "mb_copy_record";
	int	status = MB_SUCCESS;
	struct mb_buffer_struct *buff;
	struct mb_io_struct *mb_io_ptr;
	int	system;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		fprintf(stderr,"dbg2       copy_ptr:   %d\n",copy_ptr);
		}

	/* get buffer structure */
	buff = (struct mb_buffer_struct *) buff_ptr;

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get multibeam system id */
	system = mb_system_table[mb_io_ptr->format_num];

	/* call appropriate memory copy routine */
	if (system == MB_SYS_SB)
		{
		status = mbsys_sb_copy(verbose,mbio_ptr,
			store_ptr,copy_ptr,error);
		}
	else if (system == MB_SYS_HSDS)
		{
		status = mbsys_hsds_copy(verbose,mbio_ptr,
			store_ptr,copy_ptr,error);
		}
	else if (system == MB_SYS_SB2000)
		{
		status = mbsys_sb2000_copy(verbose,mbio_ptr,
			store_ptr,copy_ptr,error);
		}
	else if (system == MB_SYS_SB2100)
		{
		status = mbsys_sb2100_copy(verbose,mbio_ptr,
			store_ptr,copy_ptr,error);
		}
	else if (system == MB_SYS_SIMRAD)
		{
		status = mbsys_simrad_copy(verbose,mbio_ptr,
			store_ptr,copy_ptr,error);
		}
	else if (system == MB_SYS_MR1)
		{
		status = mbsys_mr1_copy(verbose,mbio_ptr,
			store_ptr,copy_ptr,error);
		}
	else if (system == MB_SYS_LDEOIH)
		{
		status = mbsys_ldeoih_copy(verbose,mbio_ptr,
			store_ptr,copy_ptr,error);
		}
	else if (system == MB_SYS_RESON)
		{
		status = mbsys_reson_copy(verbose,mbio_ptr,
			store_ptr,copy_ptr,error);
		}
	else if (system == MB_SYS_ELAC)
		{
		status = mbsys_elac_copy(verbose,mbio_ptr,
			store_ptr,copy_ptr,error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_SYSTEM;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
