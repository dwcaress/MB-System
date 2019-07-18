/*--------------------------------------------------------------------
 *    The MB-system:	mb_fileio.c	5/23/2012
 *
 *    Copyright (c) 2012-2019 by
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
 */

#include <stdio.h>

#include "mb_define.h"
#include "mb_io.h"
#include "mb_status.h"

/*--------------------------------------------------------------------*/
int mb_fileio_open(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	int status = MB_SUCCESS;

	int buffer_error = MB_ERROR_NO_ERROR;

	/* open the file for reading */
	if (mb_io_ptr->filemode == MB_FILEMODE_READ) {
		if ((mb_io_ptr->mbfp = fopen(mb_io_ptr->file, "rb")) == NULL) {
			*error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
		}
	}
	else if (mb_io_ptr->filemode == MB_FILEMODE_WRITE) {
		if ((mb_io_ptr->mbfp = fopen(mb_io_ptr->file, "wb")) == NULL) {
			*error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
		}
	}

	/* set buffering if desired
	    fileiomode: mode of single normal file i/o:
	                    0   use fread() and fwrite() with standard buffering
	                    >0  use fread() and fwrite() with user defined buffer
	                    <0  use mmap for file i/o */
	int fileiobuffer;
	if (status == MB_SUCCESS) {
		mb_fileiobuffer(verbose, &fileiobuffer);
		if (fileiobuffer > 0) {
			/* the buffer size must be a multiple of 512, plus 8 to be efficient */
			const size_t fileiobufferbytes = (fileiobuffer * 1024) + 8;

			/* allocate the buffer */
			buffer_error = MB_ERROR_NO_ERROR;
			int buffer_status =
			    mb_mallocd(verbose, __FILE__, __LINE__, fileiobufferbytes, (void **)&mb_io_ptr->file_iobuffer, &buffer_error);

			/* apply the buffer */
			if (buffer_status == MB_SUCCESS) {
				buffer_status = setvbuf(mb_io_ptr->mbfp, mb_io_ptr->file_iobuffer, _IOFBF, fileiobufferbytes);
				/* printf(stderr,"Called setvbuf size:%d status:%d\n",fileiobufferbytes,buffer_status); */
			}
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_fileio_close(int verbose, void *mbio_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* open the file */
	if (mb_io_ptr->mbfp != NULL) {
		fclose(mb_io_ptr->mbfp);
		mb_io_ptr->mbfp = NULL;
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_fileio_get(int verbose, void *mbio_ptr, char *buffer, size_t *size, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       size:       %p\n", (void *)size);
		fprintf(stderr, "dbg2       *size:      %p\n", (void *)(*size));
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	int status = MB_SUCCESS;

	size_t read_len;
    if (mb_io_ptr->mbfp != NULL) {
        /* read expected number of bytes into buffer */
        if ((read_len = fread(buffer, 1, *size, mb_io_ptr->mbfp)) != *size) {
            status = MB_FAILURE;
            *error = MB_ERROR_EOF;
            *size = read_len;
        }
        else {
            *error = MB_ERROR_NO_ERROR;
        }
    }
#ifdef MBTRN_ENABLED
    else {
        if (NULL != mb_io_ptr->mbsp) {
            // use the socket reader
            // read and return single frame
            uint32_t sync_bytes=0;
            int64_t rbytes=-1;

            if( (rbytes = mbtrn_read_stripped_frame(mb_io_ptr->mbsp, (byte *) buffer, R7K_MAX_FRAME_BYTES, MBR_NET_STREAM, 0.0, MBTRN_READ_TMOUT_MSEC,  &sync_bytes)) < 0){
          
                status   = MB_FAILURE;
                *error   = MB_ERROR_EOF;
                *size    = (size_t)rbytes;
                if (me_errno==ME_ESOCK) {
                    fprintf(stderr,"mbtrn_reader server connection closed.\n");
                }else if (me_errno==ME_EOF) {
                    fprintf(stderr,"mbtrn_reader end of file (server connection closed).\n");
                }else{
                    fprintf(stderr,"mbtrn_read_stripped_frame me_errno %d/%s\n",me_errno,me_strerror(me_errno));
                }
            }else {
                *error = MB_ERROR_NO_ERROR;
                *size    = (size_t)rbytes;
            }
        }
        else{
            fprintf(stderr,"mb_io file and socket pointers both NULL\n");
            status = MB_FAILURE;
            *error = MB_ERROR_EOF;
            *size = read_len;
        }
    }
#else
    else {
        status = MB_FAILURE;
        *error = MB_ERROR_EOF;
        *size = read_len;
    }
#endif

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       size:       %p\n", (void *)size);
		fprintf(stderr, "dbg2       *size:      %p\n", (void *)(*size));
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mb_fileio_put(int verbose, void *mbio_ptr, char *buffer, size_t *size, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
		fprintf(stderr, "dbg2       size:       %p\n", (void *)size);
		fprintf(stderr, "dbg2       *size:      %p\n", (void *)(*size));
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	int status = MB_SUCCESS;

	/* write expected number of bytes from buffer */
	size_t write_len = fwrite(buffer, 1, *size, mb_io_ptr->mbfp);
	if (write_len != *size) {
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		*size = write_len;
	}
	else {
		*error = MB_ERROR_NO_ERROR;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       size:       %p\n", (void *)size);
		fprintf(stderr, "dbg2       *size:      %p\n", (void *)(*size));
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
