/*--------------------------------------------------------------------
 *    The MB-system:	mb_close.c	1/25/93
 *	$Id: mb_close.c,v 4.0 1994-03-05 23:55:38 caress Exp $
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
 * mb_close.c closes a multibeam data file which had been opened for
 * reading or writing.
 *
 * Author:	D. W. Caress
 * Date:	January 25, 1993
 *	$Log: not supported by cvs2svn $
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
#include <strings.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_io.h"

/*--------------------------------------------------------------------*/
int mb_close(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
	static	char	rcs_id[]="$Id: mb_close.c,v 4.0 1994-03-05 23:55:38 caress Exp $";
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
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* deallocate format dependent structures */
	status = mb_mem_deall(verbose,mbio_ptr,error);

	/* deallocate memory for arrays within the mbio descriptor */
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

	/* close the file */
	fclose(mb_io_ptr->mbfp);

	/* deallocate the mbio descriptor */
	status = mb_free(verbose,mb_io_ptr,error);

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
