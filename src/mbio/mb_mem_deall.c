/*--------------------------------------------------------------------
 *    The MB-system:	mb_mem_deall.c	3.00	2/3/93
 *    $Id: mb_mem_deall.c,v 3.0 1993-04-23 18:16:41 dale Exp $
 *
 *    Copyright (c) 1993 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mb_mem_deall.c calls the appropriate mbr_ routine for deallocating
 * memory used to read or write data of a particular format.
 *
 * Author:	D. W. Caress
 * Date:	February 3, 1993
 *
 * $Log: not supported by cvs2svn $
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
int mb_mem_deall(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
  char rcs_id[]="$Id: mb_mem_deall.c,v 3.0 1993-04-23 18:16:41 dale Exp $";
	char	*function_name = "mb_mem_deall";
	int	status;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* call the appropriate mbr_ read and translate routine */
	if (mb_io_ptr->format == MBF_SBSIOMRG)
		{
		status = mbr_dem_sbsiomrg(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SBSIOCEN)
		{
		status = mbr_dem_sbsiocen(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SBSIOLSI)
		{
		status = mbr_dem_sbsiolsi(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_SBURICEN)
		{
		status = mbr_dem_sburicen(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_HSLDEDMB)
		{
		status = mbr_dem_hsldedmb(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_HSURICEN)
		{
		status = mbr_dem_hsuricen(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_HSATLRAW)
		{
		status = mbr_dem_hsatlraw(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_HSLDEOIH)
		{
		status = mbr_dem_hsldeoih(verbose,mbio_ptr,error);
		}
	else if (mb_io_ptr->format == MBF_MBLDEOIH)
		{
		status = mbr_dem_mbldeoih(verbose,mbio_ptr,error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_FORMAT;
		}

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
