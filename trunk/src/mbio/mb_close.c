/*--------------------------------------------------------------------
 *    The MB-system:	mb_close.c	1/25/93
 *	$Id: mb_close.c,v 5.8 2003-04-17 21:05:23 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 2000, 2002, 2003 by
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
 * Author:	D. W. Caress
 * Date:	January 25, 1993
 *	
 * $Log: not supported by cvs2svn $
 * Revision 5.7  2002/10/15 18:34:58  caress
 * Release 5.0.beta25
 *
 * Revision 5.6  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.5  2002/07/20 20:42:40  caress
 * Release 5.0.beta20
 *
 * Revision 5.4  2002/05/02 03:55:34  caress
 * Release 5.0.beta17
 *
 * Revision 5.3  2002/02/22 09:03:43  caress
 * Release 5.0.beta13
 *
 * Revision 5.2  2001/07/20 00:31:11  caress
 * Release 5.0.beta03
 *
 * Revision 5.1  2001/06/29 22:48:04  caress
 * Added support for HSDS2RAW
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.12  2000/10/11  01:02:30  caress
 * Convert to ANSI C
 *
 * Revision 4.11  2000/09/30  06:26:58  caress
 * Snapshot for Dale.
 *
 * Revision 4.10  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.9  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.9  1997/04/17  18:51:12  caress
 * Added LINUX ifdef.
 *
 * Revision 4.8  1996/08/26  17:24:56  caress
 * Release 4.4 revision.
 *
 * Revision 4.7  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
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
 * Revision 4.3  1994/11/24  01:53:22  caress
 * Some fixes related to MR1 data.
 *
 * Revision 4.2  1994/10/21  12:11:53  caress
 * Release V4.0
 *
 * Revision 4.1  1994/07/29  18:46:51  caress
 * Changes associated with supporting Lynx OS (byte swapped) and
 * using unix second time base (for time_d values).
 *
 * Revision 4.0  1994/03/05  23:55:38  caress
 * First cut at version 4.0
 *
 * Revision 4.1  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.0  1994/02/20  03:19:25  caress
 * First cut at new version.  Added support for
 * sidescan and amplitude data.
 *
 * Revision 3.3  1993/05/15  14:35:27  caress
 * fixed rcs_id message
 *
 * Revision 3.2  1993/05/14  22:32:19  sohara
 * fixed rcs id message
 *
 * Revision 3.1  1993/05/14  22:18:32  sohara
 * fixed rcs_id messages
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
#include "netcdf.h"

/*--------------------------------------------------------------------*/
int mb_close(int verbose, void **mbio_ptr, int *error)
{
	static	char	rcs_id[]="$Id: mb_close.c,v 5.8 2003-04-17 21:05:23 caress Exp $";
	char	*function_name = "mb_close";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",*mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) *mbio_ptr;

	/* deallocate format dependent structures */
	status = (*mb_io_ptr->mb_io_format_free)(verbose,*mbio_ptr,error);

	/* deallocate system dependent structures */
	/*status = (*mb_io_ptr->mb_io_store_free)
			(verbose,*mbio_ptr,&(mb_io_ptr->store_data),error);*/

	/* deallocate memory for arrays within the mbio descriptor */
	if (mb_io_ptr->filetype == MB_FILETYPE_XDR
		&& mb_io_ptr->xdrs != NULL)
		status = mb_free(verbose,&mb_io_ptr->xdrs,error);
	if (mb_io_ptr->filetype == MB_FILETYPE_XDR
		&& mb_io_ptr->xdrs2 != NULL)
		status = mb_free(verbose,&mb_io_ptr->xdrs2,error);
	if (mb_io_ptr->hdr_comment != NULL)
		status = mb_free(verbose,&mb_io_ptr->hdr_comment,error);
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
	status = mb_free(verbose,&mb_io_ptr->new_beamflag,error);
	status = mb_free(verbose,&mb_io_ptr->new_bath,error);
	status = mb_free(verbose,&mb_io_ptr->new_amp,error);
	status = mb_free(verbose,&mb_io_ptr->new_bath_acrosstrack,error);
	status = mb_free(verbose,&mb_io_ptr->new_bath_alongtrack,error);
	status = mb_free(verbose,&mb_io_ptr->new_ss,error);
	status = mb_free(verbose,&mb_io_ptr->new_ss_acrosstrack,error);
	status = mb_free(verbose,&mb_io_ptr->new_ss_alongtrack,error);

	/* close the files if normal */
	if (mb_io_ptr->filetype == MB_FILETYPE_NORMAL
	    || mb_io_ptr->filetype == MB_FILETYPE_XDR)
	    {
	    if (mb_io_ptr->mbfp != NULL)
		    fclose(mb_io_ptr->mbfp);
	    if (mb_io_ptr->mbfp2 != NULL)
		    fclose(mb_io_ptr->mbfp2);
	    if (mb_io_ptr->mbfp3 != NULL)
		    fclose(mb_io_ptr->mbfp3);
	    }
	
	/* else if gsf then use gsfClose */
	else if (mb_io_ptr->filetype == MB_FILETYPE_GSF)
	    {
	    gsfClose((int) mb_io_ptr->mbfp);
	    }
	
	/* else if netcdf then use nc_close */
	else if (mb_io_ptr->filetype == MB_FILETYPE_NETCDF)
	    {
	    if (mb_io_ptr->filemode == MB_FILEMODE_WRITE)
		nc_enddef((int) mb_io_ptr->mbfp);
	    nc_close((int) mb_io_ptr->mbfp);
	    }
	    
	/* deallocate UTM projection if required */
	if (mb_io_ptr->projection_initialized == MB_YES)
		{
		mb_io_ptr->projection_initialized = MB_NO;
		mb_proj_free(verbose, &(mb_io_ptr->pjptr), error);
		}

	/* deallocate the mbio descriptor */
	status = mb_free(verbose,mbio_ptr,error);

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
