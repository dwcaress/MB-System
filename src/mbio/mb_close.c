/*--------------------------------------------------------------------
 *    The MB-system:  mb_close.c  1/25/93
 *
 *    Copyright (c) 1993-2019 by
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
 * mb_close.c closes a multibeam data file which had been opened for
 * reading or writing.
 *
 * Author:  D. W. Caress
 * Date:  January 25, 1993
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "mb_config.h"

#ifdef ENABLE_GSF
#include "gsf.h"
#endif
#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_segy.h"
#include "mb_status.h"
#include "netcdf.h"
#include "../surf/mb_sapi.h"

/*--------------------------------------------------------------------*/
int mb_close(int verbose, void **mbio_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)*mbio_ptr);
  }

  /* get pointer to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)*mbio_ptr;

  /* deallocate format dependent structures */
  int status = (*mb_io_ptr->mb_io_format_free)(verbose, *mbio_ptr, error);

  /* deallocate system dependent structures */
  /*status = (*mb_io_ptr->mb_io_store_free)
          (verbose,*mbio_ptr,&(mb_io_ptr->store_data),error);*/

  /* deallocate memory for arrays within the mbio descriptor */
  if (mb_io_ptr->filetype == MB_FILETYPE_XDR && mb_io_ptr->xdrs != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->xdrs, error);
  if (mb_io_ptr->filetype == MB_FILETYPE_XDR && mb_io_ptr->xdrs2 != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->xdrs2, error);
  if (mb_io_ptr->filetype == MB_FILETYPE_XDR && mb_io_ptr->xdrs3 != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->xdrs3, error);
  if (mb_io_ptr->hdr_comment != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->hdr_comment, error);
  status &= mb_deall_ioarrays(verbose, *mbio_ptr, error);

  /* close the files if normal */
  if (mb_io_ptr->filetype == MB_FILETYPE_NORMAL || mb_io_ptr->filetype == MB_FILETYPE_XDR) {
    if (mb_io_ptr->mbfp != NULL)
      fclose(mb_io_ptr->mbfp);
    if (mb_io_ptr->mbfp2 != NULL)
      fclose(mb_io_ptr->mbfp2);
    if (mb_io_ptr->mbfp3 != NULL)
      fclose(mb_io_ptr->mbfp3);
  }

  /* else handle single normal files to be closed with mb_fileio_close() */
  else if (mb_io_ptr->filetype == MB_FILETYPE_SINGLE) {
    status &= mb_fileio_close(verbose, *mbio_ptr, error);
  }

#ifdef ENABLE_GSF
  /* else if gsf then use gsfClose */
  else if (mb_io_ptr->filetype == MB_FILETYPE_GSF) {
    gsfClose((int)mb_io_ptr->gsfid);
  }
#endif

  /* else if netcdf then use nc_close */
  else if (mb_io_ptr->filetype == MB_FILETYPE_NETCDF) {
    if (mb_io_ptr->filemode == MB_FILEMODE_WRITE)
      nc_enddef(mb_io_ptr->ncid);
    nc_close(mb_io_ptr->ncid);
  }

  /* else handle surf files to be opened with libsapi */
  else if (mb_io_ptr->filetype == MB_FILETYPE_SURF) {
    SAPI_close();
  }

  /* else if MB_FILETYPE_INPUT (usually socket) deallocate mbsp if non-null */
  else if (mb_io_ptr->filetype == MB_FILETYPE_INPUT) {
    if (mb_io_ptr->mbsp != NULL)
      status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&mb_io_ptr->mbsp, error);
  }

  /* deallocate UTM projection if required */
  if (mb_io_ptr->projection_initialized == true) {
    mb_io_ptr->projection_initialized = false;
    mb_proj_free(verbose, &(mb_io_ptr->pjptr), error);
  }

  /* deallocate the mbio descriptor */
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)mbio_ptr, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return value:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
