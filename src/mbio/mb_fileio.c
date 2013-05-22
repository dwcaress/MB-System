/*--------------------------------------------------------------------
 *    The MB-system:	mb_fileio.c	5/23/2012
 *    $Id:  $
 *
 *    Copyright (c) 2012-2013 by
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
 * mb_fileio.c contains the functions handling reading and writing of bytes to and from
 * single, regular files. In some cases this may be done directly using
 * fread() and fwrite() with standard buffering, in others there may be
 * local buffering, and in others mmap may be used.
 *
 * These functions include:
 *   mb_fileio_open	- initialize i/o, called by mb_read_init() and mb_write_init()
 *   mb_fileio_close	- cleanup i/o, called by mb_close()
 *   mb_fileio_get	- get bytes from input
 *   mb_fileio_put	- put bytes to output
 *
 * Author:	D. W. Caress
 * Date:	23 May 2012
 *
 * $Log: $
 *
 */

/* standard include files */
#include <stdio.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_io.h"
#include "mb_define.h"

static char rcs_id[]="$Id: $";

/*--------------------------------------------------------------------*/
int mb_fileio_open(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mb_fileio_open";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int	fileiobuffer;
	size_t	fileiobufferbytes;
	int	buffer_status = MB_SUCCESS;
	int	buffer_error = MB_ERROR_NO_ERROR;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* open the file for reading */
	if (mb_io_ptr->filemode == MB_FILEMODE_READ)
		{
		if ((mb_io_ptr->mbfp = fopen(mb_io_ptr->file, "r")) == NULL)
			 {
			 *error = MB_ERROR_OPEN_FAIL;
			 status = MB_FAILURE;
			 }
		}
	else if (mb_io_ptr->filemode == MB_FILEMODE_WRITE)
		{
		if ((mb_io_ptr->mbfp = fopen(mb_io_ptr->file, "wb")) == NULL)
			 {
			 *error = MB_ERROR_OPEN_FAIL;
			 status = MB_FAILURE;
			 }
		}

	/* set buffering if desired
		fileiomode: mode of single normal file i/o:
                        0   use fread() and fwrite() with standard buffering
                        >0  use fread() and fwrite() with user defined buffer
                        <0  use mmap for file i/o */
	if (status == MB_SUCCESS)
		{
		mb_fileiobuffer(verbose, &fileiobuffer);
		if (fileiobuffer > 0)
			{
			/* the buffer size must be a multiple of 512, plus 8 to be efficient */
			fileiobufferbytes = (fileiobuffer * 1024) + 8;

			/* allocate the buffer */
			buffer_error = MB_ERROR_NO_ERROR;
			buffer_status = mb_mallocd(verbose,__FILE__, __LINE__, fileiobufferbytes,
							(void **) &mb_io_ptr->file_iobuffer, &buffer_error);

			/* apply the buffer */
			if (buffer_status == MB_SUCCESS)
				{
				buffer_status = setvbuf(mb_io_ptr->mbfp, mb_io_ptr->file_iobuffer, _IOFBF, fileiobufferbytes);
/* printf(stderr,"Called setvbuf size:%d status:%d\n",fileiobufferbytes,buffer_status); */
				}
			}
		}

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
int mb_fileio_close(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mb_fileio_close";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* open the file */
	if (mb_io_ptr->mbfp != NULL)
		{
                fclose(mb_io_ptr->mbfp);
                mb_io_ptr->mbfp = NULL;
                }

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
int mb_fileio_get(int verbose, void *mbio_ptr, char *buffer, size_t *size, int *error)
{
	char	*function_name = "mb_fileio_get";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
        size_t  read_len;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       buffer:     %lu\n",(size_t)buffer);
		fprintf(stderr,"dbg2       size:       %lu\n",(size_t)size);
		fprintf(stderr,"dbg2       *size:      %lu\n",(size_t)(*size));
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

        /* read expected number of bytes into buffer */
        if ((read_len = fread(buffer, 1, *size, mb_io_ptr->mbfp)) != *size)
                {
                status = MB_FAILURE;
                *error = MB_ERROR_EOF;
                *size = read_len;
                }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       size:       %lu\n",(size_t)size);
		fprintf(stderr,"dbg2       *size:      %lu\n",(size_t)(*size));
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mb_fileio_put(int verbose, void *mbio_ptr, char *buffer, size_t *size, int *error)
{
	char	*function_name = "mb_fileio_put";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	size_t	write_len;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       buffer:     %lu\n",(size_t)buffer);
		fprintf(stderr,"dbg2       size:       %lu\n",(size_t)size);
		fprintf(stderr,"dbg2       *size:      %lu\n",(size_t)(*size));
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

        /* write expected number of bytes from buffer */
        if ((write_len = fwrite(buffer, 1, *size, mb_io_ptr->mbfp)) != *size)
                {
                status = MB_FAILURE;
                *error = MB_ERROR_EOF;
                *size = write_len;
                }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       size:       %lu\n",(size_t)size);
		fprintf(stderr,"dbg2       *size:      %lu\n",(size_t)(*size));
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
