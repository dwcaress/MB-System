/*--------------------------------------------------------------------
 *    The MB-system:  mb_fileio.c  5/23/2012
 *
 *    Copyright (c) 2012-2023 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes 
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *     
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mb_fileio.c contains the functions handling reading and writing of bytes to and from
 * single, regular files. In some cases this may be done directly using
 * fread() and fwrite() with standard buffering, in others there may be
 * local buffering, and in others mmap may be used.
 *
 * These functions include:
 *   mb_fileio_open  - initialize i/o, called by mb_read_init() and mb_write_init()
 *   mb_fileio_close  - cleanup i/o, called by mb_close()
 *   mb_fileio_get  - get bytes from input
 *   mb_fileio_put  - put bytes to output
 *
 * Author:  D. W. Caress
 * Date:  23 May 2012
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_io.h"
#include "mb_status.h"

/*--------------------------------------------------------------------*/
int mb_fileio_open(int verbose, void *mbio_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:            %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:           %p\n", (void *)mbio_ptr);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  if (verbose >= 2) {
    fprintf(stderr, "dbg2       mbio_ptr->filemode: %d\n", mb_io_ptr->filemode);
    fprintf(stderr, "dbg2       mbio_ptr->file:     %s\n", mb_io_ptr->file);
  }

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
      const int buffer_status =
          mb_mallocd(verbose, __FILE__, __LINE__, fileiobufferbytes, (void **)&mb_io_ptr->file_iobuffer, &buffer_error);

      /* apply the buffer */
      if (buffer_status == MB_SUCCESS) {
        /* buffer_status = */ setvbuf(mb_io_ptr->mbfp, mb_io_ptr->file_iobuffer, _IOFBF, fileiobufferbytes);
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

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

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

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  int status = MB_SUCCESS;

  size_t read_len = 0;
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
  else if (mb_io_ptr->mbsp != NULL) {
    mb_io_ptr->mb_io_input_read(verbose, mbio_ptr, size, buffer, error);
  }
  else {
      fprintf(stderr,"mb_io file and socket pointers both NULL\n");
      status = MB_FAILURE;
      *error = MB_ERROR_EOF;
      *size = read_len;
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

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  int status = MB_SUCCESS;

  /* write expected number of bytes from buffer */
  const size_t write_len = fwrite(buffer, 1, *size, mb_io_ptr->mbfp);
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
int mb_copyfile(int verbose, const char *src, const char *dst, int *error)
{
  /* The code here is modified from an example within a comment to the
     following Stackoverflow post:
        https://stackoverflow.com/questions/66362309/proper-methods-to-copy-files-folders-\
              programmatically-in-c-using-posix-functions
  */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       src:        %p\n", (void *)src);
    fprintf(stderr, "dbg2       dst:        %p\n", (void *)dst);
    if (src != NULL)
      fprintf(stderr, "dbg2       src:        %s\n", src);
    if (dst != NULL)
      fprintf(stderr, "dbg2       dst:        %s\n", dst);
  }

    int status = MB_SUCCESS;
    *error = MB_ERROR_NO_ERROR;
    const int bufsz = 65536;
    char *buf = malloc(bufsz);
    if (!buf) {
      status = MB_FAILURE;
      *error = MB_ERROR_MEMORY_FAIL;
      return(status); 
    }
    FILE *hin = fopen(src, "r");
    if (!hin) { 
      free(buf); 
      status = MB_FAILURE;
      *error = MB_ERROR_OPEN_FAIL;
      return(status);
    }
    FILE *hout = fopen(dst, "w");
    if (!hout) { 
      free(buf); 
      fclose(hin); 
      status = MB_FAILURE;
      *error = MB_ERROR_OPEN_FAIL;
      return(status);
    }
    size_t buflen;
    while ((buflen = fread(buf, 1, bufsz, hin)) > 0) {
      if (buflen != fwrite(buf, 1, buflen, hout)) {
        /* IO error writing data */
        fclose(hout);
        fclose(hin);
        free(buf);
        status = MB_FAILURE;
        *error = MB_ERROR_WRITE_FAIL;
        return(status);
      }
    }
    free(buf);
    fclose(hin);
    /* final case: check if IO error flushing buffer 
        -- don't omit this it really can happen; calling `fflush()` won't help. */
    if (ferror(hout) != 0) {
        status = MB_FAILURE;
        *error = MB_ERROR_WRITE_FAIL;
    }
    fclose(hout);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

    return(status);
}
/*--------------------------------------------------------------------*/
int mb_catfiles(int verbose, const char *src1, const char *src2, const char *dst, int *error)
{
  /* The code here is modified from an example within a comment to the
     following Stackoverflow post:
        https://stackoverflow.com/questions/66362309/proper-methods-to-copy-files-folders-\
              programmatically-in-c-using-posix-functions
  */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       src1:       %p\n", (void *)src1);
    fprintf(stderr, "dbg2       src2:       %p\n", (void *)src2);
    fprintf(stderr, "dbg2       dst:        %p\n", (void *)dst);
    if (src1 != NULL)
      fprintf(stderr, "dbg2       src1:       %s\n", src1);
    if (src2 != NULL)
      fprintf(stderr, "dbg2       src2:       %s\n", src2);
  }
    int status = MB_SUCCESS;
    *error = MB_ERROR_NO_ERROR;
    const int bufsz = 65536;
    char *buf = malloc(bufsz);
    if (!buf) {
      status = MB_FAILURE;
      *error = MB_ERROR_MEMORY_FAIL;
      return(status); 
    }
    if (!(strlen(src1) > 0 && strlen(src2) > 0 && strlen(dst) > 0)) {
      status = MB_FAILURE;
      *error = MB_ERROR_MEMORY_FAIL;
      return(status); 
    }

    /* src1 == dst so concatenate src2 onto src1 in place */
    if (strcmp(src1, dst) == 0) {
      FILE *hin = fopen(src2, "r");
      if (!hin) { 
        free(buf); 
        status = MB_FAILURE;
        *error = MB_ERROR_OPEN_FAIL;
        return(status);
      }
      FILE *hout = fopen(dst, "a");
      if (!hout) { 
        free(buf); 
        fclose(hin); 
        status = MB_FAILURE;
        *error = MB_ERROR_OPEN_FAIL;
        return(status);
      }
      size_t buflen;
      while ((buflen = fread(buf, 1, bufsz, hin)) > 0) {
        if (buflen != fwrite(buf, 1, buflen, hout)) {
          /* IO error writing data */
          fclose(hout);
          fclose(hin);
          free(buf);
          status = MB_FAILURE;
          *error = MB_ERROR_WRITE_FAIL;
          return(status);
        }
      }
      fclose(hin);
      free(buf);

      /* final case: check if IO error flushing buffer 
          -- don't omit this it really can happen; calling `fflush()` won't help. */
      if (ferror(hout) != 0) {
          status = MB_FAILURE;
          *error = MB_ERROR_WRITE_FAIL;
      }
      fclose(hout);
    }
 
    /* src1 != dst so concatenate src2 onto src1 in dst */
    else {
      FILE *hin = fopen(src1, "r");
      if (!hin) { 
        free(buf); 
        status = MB_FAILURE;
        *error = MB_ERROR_OPEN_FAIL;
        return(status);
      }
      FILE *hout = fopen(dst, "w");
      if (!hout) { 
        free(buf); 
        fclose(hin); 
        status = MB_FAILURE;
        *error = MB_ERROR_OPEN_FAIL;
        return(status);
      }
      size_t buflen;
      while ((buflen = fread(buf, 1, bufsz, hin)) > 0) {
        if (buflen != fwrite(buf, 1, buflen, hout)) {
          /* IO error writing data */
          fclose(hout);
          fclose(hin);
          free(buf);
          status = MB_FAILURE;
          *error = MB_ERROR_WRITE_FAIL;
          return(status);
        }
      }
      fclose(hin);
      hin = fopen(src2, "r");
      if (!hin) { 
        free(buf); 
        status = MB_FAILURE;
        *error = MB_ERROR_OPEN_FAIL;
        return(status);
      }
      while ((buflen = fread(buf, 1, bufsz, hin)) > 0) {
        if (buflen != fwrite(buf, 1, buflen, hout)) {
          /* IO error writing data */
          fclose(hout);
          fclose(hin);
          free(buf);
          status = MB_FAILURE;
          *error = MB_ERROR_WRITE_FAIL;
          return(status);
        }
      }
      fclose(hin);
      free(buf);

      /* final case: check if IO error flushing buffer 
          -- don't omit this it really can happen; calling `fflush()` won't help. */
      if (ferror(hout) != 0) {
          status = MB_FAILURE;
          *error = MB_ERROR_WRITE_FAIL;
      }
      fclose(hout);

    }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       dst:        %p\n", (void *)dst);
    if (dst != NULL)
      fprintf(stderr, "dbg2       dst:        %s\n", dst);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

    return(status);
}
/*--------------------------------------------------------------------*/
