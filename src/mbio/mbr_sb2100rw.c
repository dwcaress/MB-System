/*--------------------------------------------------------------------
 *    The MB-system:	mbr_sb2100rw.c	3/3/94
 *	$Id: mbr_sb2100rw.c,v 5.13 2008/03/01 09:14:03 caress Exp $
 *
 *    Copyright (c) 1994-2009 by
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
 * mbr_sb2100rw.c contains the functions for reading and writing
 * multibeam data in the SB2100RW format.  
 * These functions include:
 *   mbr_alm_sb2100rw	- allocate read/write memory
 *   mbr_dem_sb2100rw	- deallocate read/write memory
 *   mbr_rt_sb2100rw	- read and translate data
 *   mbr_wt_sb2100rw	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	March 3, 1994
 * $Log: mbr_sb2100rw.c,v $
 * Revision 5.13  2008/03/01 09:14:03  caress
 * Some housekeeping changes.
 *
 * Revision 5.12  2005/11/05 00:48:04  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.11  2003/05/20 18:05:32  caress
 * Added svp_source to data source parameters.
 *
 * Revision 5.10  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.9  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.8  2002/07/20 20:42:40  caress
 * Release 5.0.beta20
 *
 * Revision 5.7  2002/04/30 17:41:29  caress
 * Fixed writing of number of pixels.
 *
 * Revision 5.6  2001/12/18 04:27:45  caress
 * Release 5.0.beta11.
 *
 * Revision 5.5  2001/07/27  19:07:16  caress
 * Added data cutting.
 *
 * Revision 5.4  2001/07/20 00:32:54  caress
 * Release 5.0.beta03
 *
 * Revision 5.3  2001/03/22  20:50:02  caress
 * Trying to make version 5.0.beta0
 *
 * Revision 5.2  2001/01/23  01:16:54  caress
 * Fixed passing of bathymetry when sidescan records don't
 * have range scale value.
 *
 * Revision 5.1  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.29  2000/10/11  01:03:21  caress
 * Convert to ANSI C
 *
 * Revision 4.28  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.27  1999/09/14  20:39:11  caress
 * Fixed bugs handling HSMD
 *
 * Revision 4.26  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.25  1997/07/25  14:19:53  caress
 * Version 4.5beta2.
 * Much mucking, particularly with Simrad formats.
 *
 * Revision 4.24  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.24  1997/04/17  15:07:36  caress
 * MB-System 4.5 Beta Release
 *
 * Revision 4.23  1996/06/05  21:06:27  caress
 * Fixed problem handling gain of sidescan and amplitude data.
 * Previously transmit attenuation was treated as a gain value
 * (positive) rather than an attenuation value.
 *
 * Revision 4.22  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.21  1996/01/26  21:23:30  caress
 * Version 4.3 distribution
 *
 * Revision 4.20  1995/09/28  18:10:48  caress
 * Various bug fixes working toward release 4.3.
 *
 * Revision 4.19  1995/08/17  14:42:45  caress
 * Revision for release 4.3.
 *
 * Revision 4.18  1995/07/18  15:38:29  caress
 * Added rounding to calculation of output navigation.
 *
 * Revision 4.17  1995/07/13  19:13:36  caress
 * Intermediate check-in during major bug-fixing flail.
 *
 * Revision 4.16  1995/06/06  13:28:49  caress
 * Explicit cast to int on line 1298 fixes warning under Solaris 2.4
 *
 * Revision 4.15  1995/06/03  03:25:13  caress
 * Fixes to handling of changes to vendor SB2100 format
 *
 * Revision 4.14  1995/05/08  21:26:28  caress
 * Made changes consistent with new i/o spec for SB2100 data.
 *
 * Revision 4.13  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.12  1995/02/15  14:37:51  caress
 * Changed "signed short" declarations to "short" in order to
 * placate the SunOS 4.1 compiler.
 *
 * Revision 4.11  1995/02/14  21:59:53  caress
 * Version 4.2
 *
 * Revision 4.10  1995/01/17  23:19:57  caress
 * Fixed bug where fractional seconds were set to zero
 * when writing data.
 *
 * Revision 4.9  1995/01/16  12:32:15  caress
 * Changed output of transmit_attenuation values so they
 * are not prepended with a "+", as Dale Chayes found
 * this was breaking Palmer data.
 *
 * Revision 4.8  1994/12/21  20:18:10  caress
 * Not sure what changes have been made.
 *
 * Revision 4.7  1994/11/07  14:04:33  caress
 * Fixed data flagging.
 *
 * Revision 4.6  1994/10/21  12:20:01  caress
 * Release V4.0
 *
 * Revision 4.5  1994/07/29  18:46:51  caress
 * Changes associated with supporting Lynx OS (byte swapped) and
 * using unix second time base (for time_d values).
 *
 * Revision 4.4  1994/06/21  22:54:21  caress
 * Added #ifdef statements to handle byte swapping.
 *
 * Revision 4.3  1994/04/09  15:49:21  caress
 * Altered to fit latest iteration of SeaBeam 2100 vendor format.
 *
 * Revision 4.2  1994/03/25  14:02:38  caress
 * Made changes in accordance with latest iteration of
 * SeaBeam 2100 vendor format.
 *
 * Revision 4.1  1994/03/13  04:48:05  caress
 * Changed order in which parameters are read and written.
 *
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
 * Revision 4.0  1994/03/05  02:13:52  caress
 * First cut for MBF_SB2100RW format.
 *
 * Revision 1.1  1994/03/05  02:09:29  caress
 * Initial revision
 *
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
#include "../../include/mbsys_sb2100.h"
#include "../../include/mbf_sb2100rw.h"

/* include for byte swapping on little-endian machines */
#ifdef BYTESWAPPED
#include "../../include/mb_swap.h"
#endif

/* essential function prototypes */
int mbr_register_sb2100rw(int verbose, void *mbio_ptr, 
		int *error);
int mbr_info_sb2100rw(int verbose, 
			int *system, 
			int *beams_bath_max, 
			int *beams_amp_max, 
			int *pixels_ss_max, 
			char *format_name, 
			char *system_name, 
			char *format_description, 
			int *numfile, 
			int *filetype, 
			int *variable_beams, 
			int *traveltime, 
			int *beam_flagging, 
			int *nav_source, 
			int *heading_source, 
			int *vru_source, 
			int *svp_source,
			double *beamwidth_xtrack, 
			double *beamwidth_ltrack, 
			int *error);
int mbr_alm_sb2100rw(int verbose, void *mbio_ptr, int *error);
int mbr_dem_sb2100rw(int verbose, void *mbio_ptr, int *error);
int mbr_zero_sb2100rw(int verbose, void *data_ptr, int *error);
int mbr_rt_sb2100rw(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_sb2100rw(int verbose, void *mbio_ptr, void *store_ptr, int *error);

int mbr_sb2100rw_rd_data(int verbose, void *mbio_ptr, int *error);
int mbr_sb2100rw_read_line(int verbose, FILE *mbfp,int minimum_size, char *line, int *error);
int mbr_sb2100rw_rd_label(int verbose, FILE *mbfp, char *line, int *type, int *error);
int mbr_sb2100rw_rd_pr(int verbose, FILE *mbfp, struct mbf_sb2100rw_struct *data, int *error);
int mbr_sb2100rw_rd_tr(int verbose, FILE *mbfp, struct mbf_sb2100rw_struct *data, int *error);
int mbr_sb2100rw_rd_dr(int verbose, FILE *mbfp, struct mbf_sb2100rw_struct *data, int *error);
int mbr_sb2100rw_rd_ss(int verbose, FILE *mbfp, struct mbf_sb2100rw_struct *data, int *error);
int mbr_sb2100rw_wr_data(int verbose, void *mbio_ptr, void *data_ptr, int *error);
int mbr_sb2100rw_wr_label(int verbose, FILE *mbfp, char type, int *error);
int mbr_sb2100rw_write_line(int verbose, FILE *mbfp, char *line, int *error);
int mbr_sb2100rw_wr_rawline(int verbose, FILE *mbfp, void *data_ptr, int *error);
int mbr_sb2100rw_wr_pr(int verbose, FILE *mbfp, void *data_ptr, int *error);
int mbr_sb2100rw_wr_tr(int verbose, FILE *mbfp, void *data_ptr, int *error);
int mbr_sb2100rw_wr_dr(int verbose, FILE *mbfp, void *data_ptr, int *error);
int mbr_sb2100rw_wr_ss(int verbose, FILE *mbfp, void *data_ptr, int *error);

static char rcs_id[]="$Id: mbr_sb2100rw.c,v 5.13 2008/03/01 09:14:03 caress Exp $";

/*--------------------------------------------------------------------*/
int mbr_register_sb2100rw(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_register_sb2100rw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		}

	/* get mb_io_ptr */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set format info parameters */
	status = mbr_info_sb2100rw(verbose, 
			&mb_io_ptr->system, 
			&mb_io_ptr->beams_bath_max, 
			&mb_io_ptr->beams_amp_max, 
			&mb_io_ptr->pixels_ss_max, 
			mb_io_ptr->format_name, 
			mb_io_ptr->system_name, 
			mb_io_ptr->format_description, 
			&mb_io_ptr->numfile, 
			&mb_io_ptr->filetype, 
			&mb_io_ptr->variable_beams, 
			&mb_io_ptr->traveltime, 
			&mb_io_ptr->beam_flagging, 
			&mb_io_ptr->nav_source, 
			&mb_io_ptr->heading_source, 
			&mb_io_ptr->vru_source, 
			&mb_io_ptr->svp_source, 
			&mb_io_ptr->beamwidth_xtrack, 
			&mb_io_ptr->beamwidth_ltrack, 
			error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_sb2100rw;
	mb_io_ptr->mb_io_format_free = &mbr_dem_sb2100rw; 
	mb_io_ptr->mb_io_store_alloc = &mbsys_sb2100_alloc; 
	mb_io_ptr->mb_io_store_free = &mbsys_sb2100_deall; 
	mb_io_ptr->mb_io_read_ping = &mbr_rt_sb2100rw; 
	mb_io_ptr->mb_io_write_ping = &mbr_wt_sb2100rw; 
	mb_io_ptr->mb_io_dimensions = &mbsys_sb2100_dimensions; 
	mb_io_ptr->mb_io_extract = &mbsys_sb2100_extract; 
	mb_io_ptr->mb_io_insert = &mbsys_sb2100_insert; 
	mb_io_ptr->mb_io_extract_nav = &mbsys_sb2100_extract_nav; 
	mb_io_ptr->mb_io_insert_nav = &mbsys_sb2100_insert_nav; 
	mb_io_ptr->mb_io_extract_altitude = &mbsys_sb2100_extract_altitude; 
	mb_io_ptr->mb_io_insert_altitude = NULL; 
	mb_io_ptr->mb_io_extract_svp = &mbsys_sb2100_extract_svp; 
	mb_io_ptr->mb_io_insert_svp = &mbsys_sb2100_insert_svp; 
	mb_io_ptr->mb_io_ttimes = &mbsys_sb2100_ttimes; 
	mb_io_ptr->mb_io_detects = &mbsys_sb2100_detects; 
	mb_io_ptr->mb_io_gains = &mbsys_sb2100_gains; 
	mb_io_ptr->mb_io_copyrecord = &mbsys_sb2100_copy; 
	mb_io_ptr->mb_io_extract_rawss = NULL; 
	mb_io_ptr->mb_io_insert_rawss = NULL; 

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");	
		fprintf(stderr,"dbg2       system:             %d\n",mb_io_ptr->system);
		fprintf(stderr,"dbg2       beams_bath_max:     %d\n",mb_io_ptr->beams_bath_max);
		fprintf(stderr,"dbg2       beams_amp_max:      %d\n",mb_io_ptr->beams_amp_max);
		fprintf(stderr,"dbg2       pixels_ss_max:      %d\n",mb_io_ptr->pixels_ss_max);
		fprintf(stderr,"dbg2       format_name:        %s\n",mb_io_ptr->format_name);
		fprintf(stderr,"dbg2       system_name:        %s\n",mb_io_ptr->system_name);
		fprintf(stderr,"dbg2       format_description: %s\n",mb_io_ptr->format_description);
		fprintf(stderr,"dbg2       numfile:            %d\n",mb_io_ptr->numfile);
		fprintf(stderr,"dbg2       filetype:           %d\n",mb_io_ptr->filetype);
		fprintf(stderr,"dbg2       variable_beams:     %d\n",mb_io_ptr->variable_beams);
		fprintf(stderr,"dbg2       traveltime:         %d\n",mb_io_ptr->traveltime);
		fprintf(stderr,"dbg2       beam_flagging:      %d\n",mb_io_ptr->beam_flagging);
		fprintf(stderr,"dbg2       nav_source:         %d\n",mb_io_ptr->nav_source);
		fprintf(stderr,"dbg2       heading_source:     %d\n",mb_io_ptr->heading_source);
		fprintf(stderr,"dbg2       vru_source:         %d\n",mb_io_ptr->vru_source);
		fprintf(stderr,"dbg2       svp_source:         %d\n",mb_io_ptr->svp_source);
		fprintf(stderr,"dbg2       beamwidth_xtrack:   %f\n",mb_io_ptr->beamwidth_xtrack);
		fprintf(stderr,"dbg2       beamwidth_ltrack:   %f\n",mb_io_ptr->beamwidth_ltrack);
		fprintf(stderr,"dbg2       format_alloc:       %ld\n",(long)mb_io_ptr->mb_io_format_alloc);
		fprintf(stderr,"dbg2       format_free:        %ld\n",(long)mb_io_ptr->mb_io_format_free);
		fprintf(stderr,"dbg2       store_alloc:        %ld\n",(long)mb_io_ptr->mb_io_store_alloc);
		fprintf(stderr,"dbg2       store_free:         %ld\n",(long)mb_io_ptr->mb_io_store_free);
		fprintf(stderr,"dbg2       read_ping:          %ld\n",(long)mb_io_ptr->mb_io_read_ping);
		fprintf(stderr,"dbg2       write_ping:         %ld\n",(long)mb_io_ptr->mb_io_write_ping);
		fprintf(stderr,"dbg2       extract:            %ld\n",(long)mb_io_ptr->mb_io_extract);
		fprintf(stderr,"dbg2       insert:             %ld\n",(long)mb_io_ptr->mb_io_insert);
		fprintf(stderr,"dbg2       extract_nav:        %ld\n",(long)mb_io_ptr->mb_io_extract_nav);
		fprintf(stderr,"dbg2       insert_nav:         %ld\n",(long)mb_io_ptr->mb_io_insert_nav);
		fprintf(stderr,"dbg2       extract_altitude:   %ld\n",(long)mb_io_ptr->mb_io_extract_altitude);
		fprintf(stderr,"dbg2       insert_altitude:    %ld\n",(long)mb_io_ptr->mb_io_insert_altitude);
		fprintf(stderr,"dbg2       extract_svp:        %ld\n",(long)mb_io_ptr->mb_io_extract_svp);
		fprintf(stderr,"dbg2       insert_svp:         %ld\n",(long)mb_io_ptr->mb_io_insert_svp);
		fprintf(stderr,"dbg2       ttimes:             %ld\n",(long)mb_io_ptr->mb_io_ttimes);
		fprintf(stderr,"dbg2       detects:            %ld\n",(long)mb_io_ptr->mb_io_detects);
		fprintf(stderr,"dbg2       extract_rawss:      %ld\n",(long)mb_io_ptr->mb_io_extract_rawss);
		fprintf(stderr,"dbg2       insert_rawss:       %ld\n",(long)mb_io_ptr->mb_io_insert_rawss);
		fprintf(stderr,"dbg2       copyrecord:         %ld\n",(long)mb_io_ptr->mb_io_copyrecord);
		fprintf(stderr,"dbg2       error:              %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		}

	/* return status */
	return(status);
}

/*--------------------------------------------------------------------*/
int mbr_info_sb2100rw(int verbose, 
			int *system, 
			int *beams_bath_max, 
			int *beams_amp_max, 
			int *pixels_ss_max, 
			char *format_name, 
			char *system_name, 
			char *format_description, 
			int *numfile, 
			int *filetype, 
			int *variable_beams, 
			int *traveltime, 
			int *beam_flagging, 
			int *nav_source, 
			int *heading_source, 
			int *vru_source, 
			int *svp_source, 
			double *beamwidth_xtrack, 
			double *beamwidth_ltrack, 
			int *error)
{
	char	*function_name = "mbr_info_sb2100rw";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		}

	/* set format info parameters */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	*system = MB_SYS_SB2100;
	*beams_bath_max = 151;
	*beams_amp_max = 151;
	*pixels_ss_max = 2000;
	strncpy(format_name, "SB2100RW", MB_NAME_LENGTH);
	strncpy(system_name, "SB2100", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_SB2100RW\nInformal Description: SeaBeam 2100 series vender format\nAttributes:           SeaBeam 2100, bathymetry, amplitude \n                      and sidescan, 151 beams and 2000 pixels, ascii \n                      with binary sidescan, SeaBeam Instruments.\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_YES;
	*traveltime = MB_YES;
	*beam_flagging = MB_YES;
	*nav_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_DATA;
	*svp_source = MB_DATA_VELOCITY_PROFILE;
	*beamwidth_xtrack = 2.0;
	*beamwidth_ltrack = 2.0;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");	
		fprintf(stderr,"dbg2       system:             %d\n",*system);
		fprintf(stderr,"dbg2       beams_bath_max:     %d\n",*beams_bath_max);
		fprintf(stderr,"dbg2       beams_amp_max:      %d\n",*beams_amp_max);
		fprintf(stderr,"dbg2       pixels_ss_max:      %d\n",*pixels_ss_max);
		fprintf(stderr,"dbg2       format_name:        %s\n",format_name);
		fprintf(stderr,"dbg2       system_name:        %s\n",system_name);
		fprintf(stderr,"dbg2       format_description: %s\n",format_description);
		fprintf(stderr,"dbg2       numfile:            %d\n",*numfile);
		fprintf(stderr,"dbg2       filetype:           %d\n",*filetype);
		fprintf(stderr,"dbg2       variable_beams:     %d\n",*variable_beams);
		fprintf(stderr,"dbg2       traveltime:         %d\n",*traveltime);
		fprintf(stderr,"dbg2       beam_flagging:      %d\n",*beam_flagging);
		fprintf(stderr,"dbg2       nav_source:         %d\n",*nav_source);
		fprintf(stderr,"dbg2       heading_source:     %d\n",*heading_source);
		fprintf(stderr,"dbg2       vru_source:         %d\n",*vru_source);
		fprintf(stderr,"dbg2       svp_source:         %d\n",*svp_source);
		fprintf(stderr,"dbg2       beamwidth_xtrack:   %f\n",*beamwidth_xtrack);
		fprintf(stderr,"dbg2       beamwidth_ltrack:   %f\n",*beamwidth_ltrack);
		fprintf(stderr,"dbg2       error:              %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_alm_sb2100rw(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_alm_sb2100rw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_sb2100_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %ld\n",(long)mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set initial status */
	status = MB_SUCCESS;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = sizeof(struct mbf_sb2100rw_struct);
	mb_io_ptr->data_structure_size = 0;
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mb_malloc(verbose,sizeof(struct mbsys_sb2100_struct),
				&mb_io_ptr->store_data,error);

	/* get store structure pointer */
	store = (struct mbsys_sb2100_struct *) mb_io_ptr->store_data;
				
	/* set comment pointer */
	store->comment = (char *) &(store->roll_bias_port);

	/* initialize everything to zeros */
	mbr_zero_sb2100rw(verbose,mb_io_ptr->raw_data,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_dem_sb2100rw(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_sb2100rw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %ld\n",(long)mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* deallocate memory for data descriptor */
	status = mb_free(verbose,&mb_io_ptr->raw_data,error);
	status = mb_free(verbose,&mb_io_ptr->store_data,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_zero_sb2100rw(int verbose, void *data_ptr, int *error)
{
	char	*function_name = "mbr_zero_sb2100rw";
	int	status = MB_SUCCESS;
	struct mbf_sb2100rw_struct *data;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       data_ptr:   %ld\n",(long)data_ptr);
		}

	/* get pointer to data descriptor */
	data = (struct mbf_sb2100rw_struct *) data_ptr;

	/* initialize everything to zeros */
	if (data != NULL)
		{
		/* type of data record */
		data->kind = MB_DATA_NONE;

		/* time stamp (all records ) */
		data->year = 0;
		data->jday = 0;
		data->hour = 0;
		data->minute = 0;
		data->msec = 0;

		/* sonar parameters (PR) */
		data->roll_bias_port = 0;
		data->roll_bias_starboard = 0;
		data->pitch_bias = 0;
		data->ship_draft = 0;
		data->num_svp = 0;
		for (i=0;i<MBF_SB2100RW_MAXVEL;i++)
			{
			data->vdepth[i] = 0;
			data->velocity[i] = 0;
			}

		/* DR and SS header info */
		data->longitude = 0.0;
		data->latitude = 0.0;
		data->speed = 0;
		data->heave = 0;
		data->range_scale = 'D';
		data->surface_sound_velocity = 0;
		data->ssv_source = 'U';
		data->depth_gate_mode = 'U';

		/* DR header info */
		data->num_beams = 0;
		data->svp_corr_beams = '0';
		for (i=0;i<2;i++)
			data->spare_dr[i] = ' ';
		data->num_algorithms = 1;
		for (i=0;i<4;i++)
			data->algorithm_order[i] = ' ';

		/* SS header info */
		data->num_pixels = 0;
		data->ss_data_length = 0;
		data->pixel_algorithm = 'D';
		data->pixel_size_scale = 'D';
		data->svp_corr_ss = '0';
		data->num_pixels_12khz = 0;
		data->pixel_size_12khz = 0.0;
		data->num_pixels_36khz = 0;
		data->pixel_size_36khz = 0.0;
		data->spare_ss = ' ';

		/* transmit parameters and navigation (DR and SS) */
		data->frequency[0] = 'L';
		data->frequency[1] = 'L';
		data->ping_gain_12khz = 0;
		data->ping_pulse_width_12khz = 0;
		data->transmitter_attenuation_12khz = 0;
		data->pitch_12khz = 0;
		data->roll_12khz = 0;
		data->heading_12khz = 0;
		data->ping_gain_36khz = 0;
		data->ping_pulse_width_36khz = 0;
		data->transmitter_attenuation_36khz = 0;
		data->pitch_36khz = 0;
		data->roll_36khz = 0;
		data->heading_36khz = 0;

		/* formed beam data (DR) */
		for (i=0;i<MBF_SB2100RW_BEAMS;i++)
			{
			data->source[i] = 'U';
			data->travel_time[i] = 0;
			data->angle_across[i] = 0;
			data->angle_forward[i] = 0;
			data->depth[i] = 0;
			data->acrosstrack_beam[i] = 0;
			data->alongtrack_beam[i] = 0;
			data->amplitude_beam[i] = 0;
			data->signal_to_noise[i] = 0;
			data->echo_length[i] = 0;
			data->quality[i] = '0';
			}

		/* sidescan data (SS) */
		for (i=0;i<MBF_SB2100RW_PIXELS;i++)
			{
			data->amplitude_ss[i] = 0;
			data->alongtrack_ss[i] = 0;
			}

		/* comment (TR) */
		strncpy(data->comment,"\0",MBF_SB2100RW_MAXLINE);

		}

	/* assume success */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_rt_sb2100rw(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_sb2100rw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sb2100rw_struct *data;
	struct mbsys_sb2100_struct *store;
	double	scale;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %ld\n",(long)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %ld\n",(long)store_ptr);
		}

	/* get pointers to mbio descriptor and data structures */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	data = (struct mbf_sb2100rw_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_sb2100_struct *) store_ptr;

	/* read next data from file */
	status = mbr_sb2100rw_rd_data(verbose,mbio_ptr,error);

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = data->kind;

	/* translate values to sb2100 data storage structure */
	if (status == MB_SUCCESS
		&& store != NULL)
		{
		/* type of data record */
		store->kind = data->kind;

		/* sonar parameters (PR) */
		if (data->kind == MB_DATA_VELOCITY_PROFILE)
		    {
		    store->year = data->year;
		    store->jday = data->jday;
		    store->hour = data->hour;
		    store->minute = data->minute;
		    store->sec = 0.001 * data->msec;
		    store->msec = data->msec - 1000 * store->sec;
		    store->roll_bias_port = 0.01 * data->roll_bias_port;
		    store->roll_bias_starboard = 0.01 * data->roll_bias_starboard;
		    store->pitch_bias = 0.01 * data->pitch_bias;
		    store->ship_draft = 0.01 * data->ship_draft;
		    store->offset_x = 0.0;
		    store->offset_y = 0.0;
		    store->offset_z = 0.0;
		    store->num_svp = data->num_svp;
		    for (i=0;i<MBF_SB2100RW_MAXVEL;i++)
			    {
			    store->svp[i].depth = 0.01 * data->vdepth[i];
			    store->svp[i].velocity = 0.01 * data->velocity[i];
			    }
		    }
		    
		/* ping data */
		else if (data->kind == MB_DATA_DATA)
		    {
		    /* time stamp */
		    store->year = data->year;
		    store->jday = data->jday;
		    store->hour = data->hour;
		    store->minute = data->minute;
		    store->sec = 0.001 * data->msec;
		    store->msec = data->msec - 1000 * store->sec;

		    /* DR and SS header info */
		    store->longitude = data->longitude;
		    store->latitude = data->latitude;
		    store->speed = 0.01 * data->speed;
		    store->heave = 0.001 * data->heave;
		    store->range_scale = data->range_scale;
		    store->ssv = 0.01 * data->surface_sound_velocity;
		    store->ssv_source = data->ssv_source;
		    store->depth_gate_mode = data->depth_gate_mode;
    
		    /* DR header info */
		    store->nbeams = data->num_beams;
		    store->svp_correction = data->svp_corr_beams;
		    for (i=0;i<2;i++)
			    store->spare_dr[i] = data->spare_dr[i];
		    store->num_algorithms = data->num_algorithms;
		    for (i=0;i<4;i++)
			    store->algorithm_order[i] = data->algorithm_order[i];
     
		    /* transmit parameters and navigation (DR and SS) */
		    store->frequency = data->frequency[0];
		    if (data->frequency[0] != 'H')
			{
			store->ping_gain = data->ping_gain_12khz;
			store->ping_pulse_width = data->ping_pulse_width_12khz;
			store->transmitter_attenuation 
				= data->transmitter_attenuation_12khz;
			store->pitch = 0.001 * data->pitch_12khz;
			store->roll = 0.001 * data->roll_12khz;
			store->heading = 0.001 * data->heading_12khz;
			}
		    else
			{
			store->ping_gain = data->ping_gain_36khz;
			store->ping_pulse_width = data->ping_pulse_width_36khz;
			store->transmitter_attenuation 
				= data->transmitter_attenuation_36khz;
			store->pitch = 0.001 * data->pitch_36khz;
			store->roll = 0.001 * data->roll_36khz;
			store->heading = 0.001 * data->heading_36khz;
			}
    
		    /* formed beam data (DR) */
		    if (data->range_scale == 'S')
			    scale = 0.01;
		    else if (data->range_scale == 'I')
			    scale = 0.1;
		    else if (data->range_scale == 'D')
			    scale = 1.0;
		    for (i=0;i<MBF_SB2100RW_BEAMS;i++)
			    {
			    store->beams[i].depth = scale * data->depth[i];
			    store->beams[i].acrosstrack = scale * data->acrosstrack_beam[i];
			    store->beams[i].alongtrack = scale * data->alongtrack_beam[i];
			    store->beams[i].range = 0.001 * data->travel_time[i];
			    store->beams[i].angle_across = 0.001 * data->angle_across[i];
			    store->beams[i].angle_forward = 0.01 * data->angle_forward[i];
			    store->beams[i].amplitude = data->amplitude_beam[i];
			    store->beams[i].signal_to_noise = data->signal_to_noise[i];
			    store->beams[i].echo_length = data->echo_length[i];
			    store->beams[i].quality = data->quality[i];
			    store->beams[i].source = data->source[i];
			    }
   
		    /* SS header info */
		    store->ss_data_length = data->ss_data_length;
		    store->npixels = data->num_pixels;
		    store->pixel_algorithm = data->pixel_algorithm;
		    store->pixel_size_scale = data->pixel_size_scale;
		    store->svp_corr_ss = data->svp_corr_ss;
		    if (data->frequency[0] != 'H')
			store->pixel_size = data->pixel_size_12khz;
		    else
			store->pixel_size = data->pixel_size_36khz;
		    store->spare_ss = data->spare_ss;
    
		    /* sidescan data (SS) */
		    for (i=0;i<MBF_SB2100RW_PIXELS;i++)
			    {
			    store->pixels[i].amplitude = data->amplitude_ss[i];
			    store->pixels[i].alongtrack = scale * data->alongtrack_ss[i];
			    }
		    }

		/* comment (TR) */
		else if (data->kind == MB_DATA_COMMENT)
		    strncpy(store->comment,data->comment,
			    MBF_SB2100RW_MAXLINE);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_wt_sb2100rw(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_sb2100rw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sb2100rw_struct *data;
	char	*data_ptr;
	struct mbsys_sb2100_struct *store;
	double	scale;
	double	depth_max, across_max, along_max;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %ld\n",(long)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %ld\n",(long)store_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	data = (struct mbf_sb2100rw_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;
	store = (struct mbsys_sb2100_struct *) store_ptr;

	/* first translate values from data storage structure */
	if (store != NULL)
		{
		/* type of data record */
		data->kind = store->kind;

		/* sonar parameters (PR) */
		if (data->kind == MB_DATA_VELOCITY_PROFILE)
		    {
		    data->year = store->year;
		    data->jday = store->jday;
		    data->hour = store->hour;
		    data->minute = store->minute;
		    data->msec = 1000 * store->sec + store->msec;
		    data->roll_bias_port = 100 * store->roll_bias_port;
		    data->roll_bias_starboard = 100 * store->roll_bias_starboard;
		    data->pitch_bias = 100 * store->pitch_bias;
		    data->ship_draft = 100 * store->ship_draft;
		    data->num_svp = store->num_svp;
		    for (i=0;i<MBF_SB2100RW_MAXVEL;i++)
			    {
			    data->vdepth[i] = 100 * store->svp[i].depth;
			    data->velocity[i] = 100 * store->svp[i].velocity;
			    }
		    }
		    
		/* ping data */
		else if (data->kind == MB_DATA_DATA)
		    {
		    /* time stamp */
		    data->year = store->year;
		    data->jday = store->jday;
		    data->hour = store->hour;
		    data->minute = store->minute;
		    data->msec = 1000 * store->sec + store->msec;

		    /* DR and SS header info */
		    data->longitude = store->longitude;
		    data->latitude = store->latitude;
		    data->speed = 100 * store->speed;
		    data->heave = 1000 * store->heave;
		    data->range_scale = store->range_scale;
		    data->surface_sound_velocity = 100 * store->ssv;
		    data->ssv_source = store->ssv_source;
		    data->depth_gate_mode = store->depth_gate_mode;
    
		    /* DR header info */
		    data->num_beams = store->nbeams;
		    data->svp_corr_beams = store->svp_correction;
		    for (i=0;i<2;i++)
			    data->spare_dr[i] = store->spare_dr[i];
		    data->num_algorithms = store->num_algorithms;
		    for (i=0;i<4;i++)
			    data->algorithm_order[i] = store->algorithm_order[i];
     
		    /* transmit parameters and navigation (DR and SS) */
		    data->frequency[0] = store->frequency;
		    data->frequency[1] = store->frequency;
		    if (store->frequency != 'H')
			{
			if (store->frequency == 'L')
				{
				data->frequency[0] = 'L';
				data->frequency[1] = 'L';
				}
			else if (store->frequency == '2')
				{
				data->frequency[0] = '2';
                                data->frequency[1] = '0';
                                }
			data->ping_gain_12khz = store->ping_gain;
			data->ping_pulse_width_12khz 
				= store->ping_pulse_width;
			data->transmitter_attenuation_12khz 
				= store->transmitter_attenuation;
			data->pitch_12khz = 1000 * store->pitch;
			data->roll_12khz = 1000 * store->roll;
			data->heading_12khz = 1000 * store->heading;
			data->ping_gain_36khz = 0;
			data->ping_pulse_width_36khz = 0;
			data->transmitter_attenuation_36khz = 0;
			data->pitch_36khz = 0;
			data->roll_36khz = 0;
			data->heading_36khz = 0;
			}
		    else
			{
                        data->frequency[0] = 'H';
                        data->frequency[1] = 'H';
			data->ping_gain_12khz = 0;
			data->ping_pulse_width_12khz = 0;
			data->transmitter_attenuation_12khz = 0;
			data->pitch_12khz = 0;
			data->roll_12khz = 0;
			data->heading_12khz = 0;
			data->ping_gain_36khz = store->ping_gain;
			data->ping_pulse_width_36khz 
				= store->ping_pulse_width;
			data->transmitter_attenuation_36khz 
				= store->transmitter_attenuation;
			data->pitch_36khz = 1000 * store->pitch;
			data->roll_36khz = 1000 * store->roll;
			data->heading_36khz = 1000 * store->heading;
			}
    
		    /* formed beam data (DR) */
		    if (data->range_scale == 'S')
			    scale = 0.01;
		    else if (data->range_scale == 'I')
			    scale = 0.1;
		    else if (data->range_scale == 'D')
			    scale = 1.0;
		    else
			    {
			    /* find best scale for data */
			    depth_max = 0.0;
			    across_max = 0.0;
			    along_max = 0.0;
			    for (i=0;i<MBF_SB2100RW_BEAMS;i++)
				{
				if (store->beams[i].depth != 0.0 
					&& store->beams[i].quality == ' ')
				    {
				    depth_max = MAX(depth_max, fabs(store->beams[i].depth));
				    across_max = MAX(across_max, fabs(store->beams[i].acrosstrack));
				    along_max = MAX(along_max, fabs(store->beams[i].alongtrack));
				    }
				}
			    if (depth_max > 9999.9
				|| across_max > 9999.9
				|| along_max > 9999.9)
				{			    
				scale = 1.0;
				data->range_scale = 'D';
				}
			    else if (depth_max > 999.9
				|| across_max > 999.9
				|| along_max > 999.9)
				{			    
				scale = 0.1;
				data->range_scale = 'I';
				}
			    else
				{			    
				scale = 0.01;
				data->range_scale = 'S';
				}
			    }
		    for (i=0;i<MBF_SB2100RW_BEAMS;i++)
			    {
			    data->depth[i] = store->beams[i].depth / scale;
			    data->acrosstrack_beam[i] = store->beams[i].acrosstrack / scale;
			    data->alongtrack_beam[i] = store->beams[i].alongtrack / scale;
			    data->travel_time[i] = 1000 * store->beams[i].range;
			    data->angle_across[i] = 1000 * store->beams[i].angle_across;
			    data->angle_forward[i] = 100 * store->beams[i].angle_forward;
			    data->amplitude_beam[i] = store->beams[i].amplitude;
			    data->signal_to_noise[i] = store->beams[i].signal_to_noise;
			    data->echo_length[i] = store->beams[i].echo_length;
			    data->quality[i] = store->beams[i].quality;
			    data->source[i] = store->beams[i].source;
			    }
   
		    /* SS header info */
		    data->ss_data_length = store->ss_data_length;
		    data->num_pixels = store->npixels;
		    data->pixel_algorithm = store->pixel_algorithm;
		    data->pixel_size_scale = 'D';
		    data->svp_corr_ss = store->svp_corr_ss;
		    if (data->frequency[0] != 'H')
			{
			data->num_pixels_12khz = store->npixels;
			data->pixel_size_12khz = store->pixel_size;
			data->num_pixels_36khz = 0;
			data->pixel_size_36khz = 0.0;
			}
		    else
			{
			data->num_pixels_12khz = 0;
			data->pixel_size_12khz = 0.0;
			data->num_pixels_36khz = store->npixels;
			data->pixel_size_36khz = store->pixel_size;
			}
		    store->spare_ss = data->spare_ss;
    
		    /* sidescan data (SS) */
		    for (i=0;i<MBF_SB2100RW_PIXELS;i++)
			    {
			    data->amplitude_ss[i] = store->pixels[i].amplitude;
			    data->alongtrack_ss[i] = store->pixels[i].alongtrack / scale;
			    }
		    }

		/* comment (TR) */
		else if (data->kind == MB_DATA_COMMENT)
		    strncpy(data->comment,store->comment,
			    MBF_SB2100RW_MAXLINE);

		}

	/* write next data to file */
	status = mbr_sb2100rw_wr_data(verbose,mbio_ptr,data_ptr,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_sb2100rw_rd_data(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_sb2100rw_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sb2100rw_struct *data;
	char	*data_ptr;
	FILE	*mbfp;
	int	done;
	int	expect;

	static int line_save_flag = MB_NO;
	static char raw_line[MBF_SB2100RW_MAXLINE] = "\0";
	static int type = MBF_SB2100RW_NONE;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %ld\n",(long)mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	data = (struct mbf_sb2100rw_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;
	mbfp = mb_io_ptr->mbfp;

	/* initialize everything to zeros */
	mbr_zero_sb2100rw(verbose,data_ptr,error);
	
	/* get file position at record beginning */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;	

	done = MB_NO;
	expect = MBF_SB2100RW_NONE;
	while (done == MB_NO)
		{

		/* get next record label */
		if (line_save_flag == MB_NO)
			{
			/* save position in file */
			mb_io_ptr->file_bytes = ftell(mbfp);
			
			/* read the label */
			status = mbr_sb2100rw_rd_label(verbose,mbfp,
				raw_line,&type,error);
			}
		else
			line_save_flag = MB_NO;

		/* read the appropriate data records */
		if (status == MB_FAILURE && expect == MBF_SB2100RW_NONE)
			{
			mb_io_ptr->file_bytes = ftell(mbfp);
			done = MB_YES;
			}
		else if (status == MB_FAILURE && expect != MBF_SB2100RW_NONE)
			{
			mb_io_ptr->file_bytes = ftell(mbfp);
			done = MB_YES;
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else if (expect != MBF_SB2100RW_NONE && expect != type)
			{
			done = MB_YES;
			expect = MBF_SB2100RW_NONE;
			line_save_flag = MB_YES;
			}
		else if (type == MBF_SB2100RW_RAW_LINE)
			{
			strcpy(data->comment,raw_line);
			mb_io_ptr->file_bytes = ftell(mbfp);
			done = MB_YES;
			data->kind = MB_DATA_RAW_LINE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			status = MB_FAILURE;
			}
		else if (type == MBF_SB2100RW_PR)
			{
			status = mbr_sb2100rw_rd_pr(
				verbose,mbfp,data,error);
			mb_io_ptr->file_bytes = ftell(mbfp);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				data->kind = MB_DATA_VELOCITY_PROFILE;
				}
			}
		else if (type == MBF_SB2100RW_TR)
			{
			status = mbr_sb2100rw_rd_tr(
				verbose,mbfp,data,error);
			mb_io_ptr->file_bytes = ftell(mbfp);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				data->kind = MB_DATA_COMMENT;
				}
			}
		else if (type == MBF_SB2100RW_DR)
			{
			status = mbr_sb2100rw_rd_dr(
				verbose,mbfp,data,error);
			mb_io_ptr->file_bytes = ftell(mbfp);
			if (status == MB_SUCCESS)
				{
				done = MB_NO;
				data->kind = MB_DATA_DATA;
				expect = MBF_SB2100RW_SS;
				}
			}
		else if (type == MBF_SB2100RW_SS)
			{
			status = mbr_sb2100rw_rd_ss(
				verbose,mbfp,data,error);
			mb_io_ptr->file_bytes = ftell(mbfp);
			if (status == MB_SUCCESS 
				&& expect == MBF_SB2100RW_SS)
				{
				done = MB_YES;
				}
			else if (status == MB_SUCCESS)
				{
				done = MB_YES;
				expect = MBF_SB2100RW_NONE;
				*error = MB_ERROR_UNINTELLIGIBLE;
				status = MB_FAILURE;
				}
			else if (status == MB_FAILURE
				&& *error ==  MB_ERROR_UNINTELLIGIBLE
				&& expect == MBF_SB2100RW_SS)
				{
				/* this preserves the bathymetry
				   that has already been read */
				done = MB_YES;
				status = MB_SUCCESS;
				*error = MB_ERROR_NO_ERROR;
				}
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int	mbr_sb2100rw_rd_label(int verbose, FILE *mbfp, 
		char *line, int *type, int *error)
{
	char	*function_name = "mbr_sb2100rw_rd_label";
	int	status = MB_SUCCESS;
	int	i, j;
	char	*label;
	int	icmp;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %ld\n",(long)mbfp);
		}

	/* read next line in file */
	status = mbr_sb2100rw_read_line(verbose,mbfp,1,line,error);

	/* see if we just encountered an identifier record */
	if (status == MB_SUCCESS)
		{
		*type = MBF_SB2100RW_RAW_LINE;
		for (i=1;i<MBF_SB2100RW_RECORDS;i++)
			{
			icmp = strncmp(line,mbf_sb2100rw_labels[i],8);
			if (icmp == 0) 
				*type = i;
			}

		/* if it looks like a raw line, check for up to 
		   four lost bytes */
		if (*type == MBF_SB2100RW_RAW_LINE)
		for (i=1;i<MBF_SB2100RW_RECORDS;i++)
		    for (j=1;j<5;j++)
			{
			label = mbf_sb2100rw_labels[i];
			icmp = strncmp(line,&label[j],8-j);
			if (icmp == 0) 
				*type = i;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       line:       %s\n",line);
		fprintf(stderr,"dbg2       type:       %d\n",*type);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int	mbr_sb2100rw_read_line(int verbose, FILE *mbfp,
		int minimum_size, char *line, int *error)
{
	char	*function_name = "mbr_sb2100rw_read_line";
	int	status = MB_SUCCESS;
	int	nchars;
	int	done;
	char	*result;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %ld\n",(long)mbfp);
		}

	/* read next good line in file */
	done = MB_NO;
	do
		{
		/* read next line in file */
		strncpy(line,"\0",MBF_SB2100RW_MAXLINE);
		result = fgets(line,MBF_SB2100RW_MAXLINE,mbfp);

		/* check size of line */
		nchars = strlen(line);

		/* check for eof */
		if (result == line)
			{
			if (nchars >= minimum_size)
				done = MB_YES;
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else
			{
			done = MB_YES;
			*error = MB_ERROR_EOF;
			status = MB_FAILURE;
			}

		/* print debug statements */
		if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg5  New line read in function <%s>\n",function_name);
			fprintf(stderr,"dbg5       line:       %s\n",line);
			fprintf(stderr,"dbg5       chars:      %d\n",nchars);
			}

		}
	while (done == MB_NO);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       line:       %s\n",line);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_sb2100rw_rd_pr(int verbose, FILE *mbfp, 
		struct mbf_sb2100rw_struct *data, int *error)
{
	char	*function_name = "mbr_sb2100rw_rd_pr";
	int	status = MB_SUCCESS;
	char	line[MBF_SB2100RW_MAXLINE];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %ld\n",(long)mbfp);
		fprintf(stderr,"dbg2       data:       %ld\n",(long)data);
		}

	/* read and parse data from first line of record */
	status = mbr_sb2100rw_read_line(verbose,mbfp,1,line,error);

	/* parse data from first line */
	if (status == MB_SUCCESS)
		{
		mb_get_int(&(data->year),                 line,     4);
		mb_get_int(&(data->jday),                 line+4,   3);
		mb_get_int(&(data->hour),                 line+7,   2);
		mb_get_int(&(data->minute),               line+9,   2);
		mb_get_int(&(data->msec),                 line+11,  5);
		if ((int)strlen(line) >= 39)
			{
			mb_get_int(&(data->roll_bias_port),       line+16,  6);
			data->roll_bias_starboard = data->roll_bias_port;
			mb_get_int(&(data->pitch_bias),           line+22,  6);
			mb_get_int(&(data->num_svp),              line+28,  2);
			mb_get_int(&(data->ship_draft),           line+30,  7);
			}
		else
			{
			mb_get_int(&(data->roll_bias_port),       line+16,  6);
			mb_get_int(&(data->roll_bias_starboard),  line+22,  6);
			mb_get_int(&(data->pitch_bias),           line+28,  6);
			mb_get_int(&(data->num_svp),              line+34,  2);
			data->ship_draft = 0;
			}
		}

	/* read and parse data from other lines of record */
	for (i=0;i<data->num_svp;i++)
		{
		if ((status = mbr_sb2100rw_read_line(verbose,mbfp,1,line,error))
			== MB_SUCCESS)
			{
			mb_get_int(&(data->vdepth[i]),line,7);
			mb_get_int(&(data->velocity[i]),line+7,6);
			}
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       julian day:       %d\n",data->jday);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",data->minute);
		fprintf(stderr,"dbg5       msec:             %d\n",data->msec);
		fprintf(stderr,"dbg5       roll_bias_port:   %d\n",data->roll_bias_port);
		fprintf(stderr,"dbg5       roll_bias_strbrd: %d\n",data->roll_bias_starboard);
		fprintf(stderr,"dbg5       pitch_bias:       %d\n",data->pitch_bias);
		fprintf(stderr,"dbg5       num_svp:          %d\n",data->num_svp);
		fprintf(stderr,"dbg5       ship_draft:       %d\n",data->ship_draft);
		fprintf(stderr,"dbg5       Sound Velocity Profile:\n");
		for (i=0;i<data->num_svp;i++)
			fprintf(stderr,"dbg5       %d  depth:%d  velocity:%d\n",
				i,data->vdepth[i],data->velocity[i]);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_sb2100rw_rd_tr(int verbose, FILE *mbfp, 
		struct mbf_sb2100rw_struct *data, int *error)
{
	char	*function_name = "mbr_sb2100rw_rd_tr";
	int	status = MB_SUCCESS;
	char	line[MBF_SB2100RW_MAXLINE];
	int	nchars;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %ld\n",(long)mbfp);
		fprintf(stderr,"dbg2       data:       %ld\n",(long)data);
		}

	/* read comment record from file */
	status = mbr_sb2100rw_read_line(verbose,mbfp,1,line,error);

	/* copy comment into data structure */
	if (status == MB_SUCCESS)
		{
		nchars = strlen(line);
		strncpy(data->comment,line,nchars-1);
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Value read in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       comment:          %s\n",
			data->comment);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_sb2100rw_rd_dr(int verbose, FILE *mbfp, 
		struct mbf_sb2100rw_struct *data, int *error)
{
	char	*function_name = "mbr_sb2100rw_rd_dr";
	int	status = MB_SUCCESS;
	char	line[MBF_SB2100RW_MAXLINE];
	int	shift;
	char	ew, ns;
	int	degrees, minutes;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %ld\n",(long)mbfp);
		fprintf(stderr,"dbg2       data:       %ld\n",(long)data);
		}

	/* read and parse data from first line of record */
	status = mbr_sb2100rw_read_line(verbose,mbfp,1,line,error);

	/* parse data from first line */
	if (status == MB_SUCCESS)
		{
		/* get time and navigation */
		mb_get_int(&(data->year),                 line,     4);
		mb_get_int(&(data->jday),                 line+4,   3);
		mb_get_int(&(data->hour),                 line+7,   2);
		mb_get_int(&(data->minute),               line+9,   2);
		mb_get_int(&(data->msec),                 line+11,  5);
		ns = line[16];
		mb_get_int(&degrees,    line+17,2);
		mb_get_int(&minutes,    line+19,6);
		data->latitude = degrees + 0.0001*minutes/60.;
		if (ns == 's' || ns == 'S')
			data->latitude = -data->latitude;
		ew = line[25];
		mb_get_int(&degrees,    line+26,3);
		mb_get_int(&minutes,    line+29,6);
		data->longitude = degrees + 0.0001*minutes/60.;
		if (ew == 'W' || ns == 'w')
			data->longitude = -data->longitude;
		mb_get_int(&(data->speed),                 line+35,7);

		/* now get other stuff */
		mb_get_int(&(data->num_beams),            line+42,  4);
		data->svp_corr_beams = line[46];
		data->frequency[0] = line[47];
		data->frequency[1] = line[48];
		mb_get_int(&(data->heave),                line+49,  6);
		for (i=0;i<2;i++)
			data->spare_dr[i] = line[55+i];
		data->range_scale = line[57];
		mb_get_int(&(data->surface_sound_velocity),line+58,6);
		data->ssv_source = line[64];
		data->depth_gate_mode = line[65];

		/* handle 12 kHz parameters if not in 36 kHz mode */
		shift = 66;
		if (data->frequency[0] != 'H')
		  {
		  mb_get_int(&(data->ping_gain_12khz),    line+shift,    2);
		  mb_get_int(&(data->ping_pulse_width_12khz),
							  line+2+shift,  2);
		  mb_get_int(&(data->transmitter_attenuation_12khz),
							  line+4+shift,  2);
		  mb_get_int(&(data->pitch_12khz),        line+6+shift,  6);
		  mb_get_int(&(data->roll_12khz),         line+12+shift, 6);
		  mb_get_int(&(data->heading_12khz),      line+18+shift, 6);
		  shift = shift + 24;
		  }

		/* handle 36 kHz parameters if if in 36 kHz 
			or dual frequency mode */
		else
		  {
		  mb_get_int(&(data->ping_gain_36khz),    line+shift,    2);
		  mb_get_int(&(data->ping_pulse_width_36khz),
							  line+2+shift,  2);
		  mb_get_int(&(data->transmitter_attenuation_36khz),
							  line+4+shift,  2);
		  mb_get_int(&(data->pitch_36khz),        line+6+shift,  6);
		  mb_get_int(&(data->roll_36khz),         line+12+shift, 6);
		  mb_get_int(&(data->heading_36khz),      line+18+shift, 6);
		  shift = shift + 24;
		  }

		/* now get last things in header */
		mb_get_int(&(data->num_algorithms),       line+shift,  1);
		for (i=0;i<4;i++)
			data->algorithm_order[i] = line[1+shift+i];
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       julian day:       %d\n",data->jday);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",
			data->minute);
		fprintf(stderr,"dbg5       msec:             %d\n",data->msec);
		fprintf(stderr,"dbg5       latitude:         %f\n",
			data->latitude);
		fprintf(stderr,"dbg5       longitude:        %f\n",
			data->longitude);
		fprintf(stderr,"dbg5       speed:            %d\n",
			data->speed);
		fprintf(stderr,"dbg5       num_beams:        %d\n",
			data->num_beams);
		fprintf(stderr,"dbg5       svp_corr_beams:   %c\n",
			data->svp_corr_beams);
		fprintf(stderr,"dbg5       frequency:        %c%c\n",
			data->frequency[0],data->frequency[1]);
		fprintf(stderr,"dbg5       heave:            %d\n",
			data->heave);
		fprintf(stderr,"dbg5       spare:            ");
		for (i=0;i<2;i++)
			fprintf(stderr,"%c",data->spare_dr[i]);
		fprintf(stderr,"\n");
		fprintf(stderr,"dbg5       range_scale:      %c\n",
			data->range_scale);
		fprintf(stderr,"dbg5       surface_sound_velocity: %d\n",
			data->surface_sound_velocity);
		fprintf(stderr,"dbg5       ssv_source:       %c\n",
			data->ssv_source);
		fprintf(stderr,"dbg5       depth_gate_mode:  %c\n",
			data->depth_gate_mode);
		fprintf(stderr,"dbg5       ping_gain_12khz:  %d\n",
			data->ping_gain_12khz);
		fprintf(stderr,"dbg5       ping_pulse_width_12khz:        %d\n",
			data->ping_pulse_width_12khz);
		fprintf(stderr,"dbg5       transmitter_attenuation_12khz: %d\n",
			data->transmitter_attenuation_12khz);
		fprintf(stderr,"dbg5       pitch_12khz:      %d\n",
			data->pitch_12khz);
		fprintf(stderr,"dbg5       roll_12khz:       %d\n",
			data->roll_12khz);
		fprintf(stderr,"dbg5       heading_12khz:    %d\n",
			data->heading_12khz);
		fprintf(stderr,"dbg5       ping_gain_36khz:  %d\n",
			data->ping_gain_36khz);
		fprintf(stderr,"dbg5       ping_pulse_width_36khz:        %d\n",
			data->ping_pulse_width_36khz);
		fprintf(stderr,"dbg5       transmitter_attenuation_36khz: %d\n",
			data->transmitter_attenuation_36khz);
		fprintf(stderr,"dbg5       pitch_36khz:      %d\n",
			data->pitch_36khz);
		fprintf(stderr,"dbg5       roll_36khz:       %d\n",
			data->roll_36khz);
		fprintf(stderr,"dbg5       heading_36khz:    %d\n",
			data->heading_36khz);
		fprintf(stderr,"dbg5       num_algorithms:   %d\n",
			data->num_algorithms);
		fprintf(stderr,"dbg5       algorithm_order:  ");
		for (i=0;i<4;i++)
			fprintf(stderr,"%c",data->algorithm_order[i]);
		fprintf(stderr,"\n");
		}

	/* read and parse data from subsequent lines of record
		- one line per beam */
	for (i=0;i<data->num_beams;i++)
	  {
	  if ((status = mbr_sb2100rw_read_line(verbose,mbfp,1,line,error)) 
		== MB_SUCCESS)
		{
		data->source[i] = line[0];
		mb_get_int(&(data->travel_time[i]),      line+1,  5);
		mb_get_int(&(data->angle_across[i]),     line+6,  6);
		mb_get_int(&(data->angle_forward[i]),    line+12, 5);
		mb_get_int(&(data->depth[i]),            line+17, 5);
		mb_get_int(&(data->acrosstrack_beam[i]), line+22, 6);
		mb_get_int(&(data->alongtrack_beam[i]),  line+28, 6);
		mb_get_int(&(data->amplitude_beam[i]),   line+34, 3);
		mb_get_int(&(data->signal_to_noise[i]),  line+37, 2);
		mb_get_int(&(data->echo_length[i]),      line+39, 3);
		data->quality[i] = line[42];
		}
	  }

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"dbg5       beam src tt angle angfor depth xtrack ltrack amp sig2noise echo quality\n");
		for (i=0;i<data->num_beams;i++)
		  {
		  fprintf(stderr,"dbg5       %3d %c %5d %6d %5d %5d %6d %6d %3d %2d %3d %c\n",
			i,
			data->source[i],
			data->travel_time[i],
			data->angle_across[i],
			data->angle_forward[i],
			data->depth[i],
			data->acrosstrack_beam[i],
			data->alongtrack_beam[i],
			data->amplitude_beam[i],
			data->signal_to_noise[i],
			data->echo_length[i],
			data->quality[i]);
		  }
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_sb2100rw_rd_ss(int verbose, FILE *mbfp, 
		struct mbf_sb2100rw_struct *data, int *error)
{
	char	*function_name = "mbr_sb2100rw_rd_ss";
	int	status = MB_SUCCESS;
	char	line[MBF_SB2100RW_MAXLINE];
	int	shift;
	char	ew, ns;
	unsigned short	read_ss[2*MBF_SB2100RW_PIXELS+2];
	char	*char_ptr;
	short	*read_ss_ptr;
	int	degrees, minutes;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %ld\n",(long)mbfp);
		fprintf(stderr,"dbg2       data:       %ld\n",(long)data);
		}

	/* read and parse data from first line of record */
	status = mbr_sb2100rw_read_line(verbose,mbfp,1,line,error);

	/* parse data from first line */
	if (status == MB_SUCCESS)
		{
		/* get time and navigation */
		mb_get_int(&(data->year),                 line,     4);
		mb_get_int(&(data->jday),                 line+4,   3);
		mb_get_int(&(data->hour),                 line+7,   2);
		mb_get_int(&(data->minute),               line+9,   2);
		mb_get_int(&(data->msec),                 line+11,  5);
		ns = line[16];
		mb_get_int(&degrees,    line+17,2);
		mb_get_int(&minutes,    line+19,6);
		data->latitude = degrees + 0.0001*minutes/60.;
		if (ns == 's' || ns == 'S')
			data->latitude = -data->latitude;
		ew = line[25];
		mb_get_int(&degrees,    line+26,3);
		mb_get_int(&minutes,    line+29,6);
		data->longitude = degrees + 0.0001*minutes/60.;
		if (ew == 'W' || ns == 'w')
			data->longitude = -data->longitude;
		mb_get_int(&(data->speed),                 line+35,  7);

		/* now get other stuff */
		mb_get_int(&(data->ss_data_length),        line+42,  4);
		data->num_pixels = (data->ss_data_length)/4;

		data->svp_corr_beams = line[46];
		data->frequency[0] = line[47];
		data->frequency[1] = line[48];
		mb_get_int(&(data->heave),                 line+49,  6);
		if (line[55] != ' ')
		    data->range_scale = line[55];
		data->spare_ss = line[56];
		data->pixel_size_scale = line[57];
		data->pixel_algorithm = line[58];
		mb_get_int(&(data->surface_sound_velocity),line+59,6);
		data->ssv_source = line[65];
		data->depth_gate_mode = line[66];

		/* handle 12 kHz parameters if not in 36 kHz mode */
		shift = 67;
		if (data->frequency[0] != 'H')
		  {
		  mb_get_int(&(data->num_pixels_12khz),   line+shift,     4);
		  mb_get_double(&(data->pixel_size_12khz),line+4+shift,   4);
		  mb_get_int(&(data->ping_gain_12khz),    line+8+shift,   2);
		  mb_get_int(&(data->ping_pulse_width_12khz),
							  line+10+shift,   2);
		  mb_get_int(&(data->transmitter_attenuation_12khz),
							  line+12+shift,  2);
		  mb_get_int(&(data->pitch_12khz),        line+14+shift,  6);
		  mb_get_int(&(data->roll_12khz),         line+20+shift,  6);
		  mb_get_int(&(data->heading_12khz),      line+26+shift,  6);
		  shift = shift + 32;
		  }

		/* handle 36 kHz parameters if if in 36 kHz 
			or dual frequency mode */
		else
		  {
		  mb_get_int(&(data->num_pixels_36khz),   line+shift,  4);
		  mb_get_double(&(data->pixel_size_36khz),line+4+shift,  4);
		  mb_get_int(&(data->ping_gain_36khz),    line+8+shift,2);
		  mb_get_int(&(data->ping_pulse_width_36khz),
							  line+10+shift,2);
		  mb_get_int(&(data->transmitter_attenuation_36khz),
							  line+12+shift,2);
		  mb_get_int(&(data->pitch_36khz),        line+14+shift,6);
		  mb_get_int(&(data->roll_36khz),         line+20+shift,6);
		  mb_get_int(&(data->heading_36khz),      line+26+shift,6);
		  shift = shift + 32;
		  }
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       julian day:       %d\n",data->jday);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",
			data->minute);
		fprintf(stderr,"dbg5       msec:             %d\n",data->msec);
		fprintf(stderr,"dbg5       latitude:         %f\n",
			data->latitude);
		fprintf(stderr,"dbg5       longitude:        %f\n",
			data->longitude);
		fprintf(stderr,"dbg5       speed:            %d\n",
			data->speed);
		fprintf(stderr,"dbg5       num_pixels:       %d\n",
			data->num_pixels);
		fprintf(stderr,"dbg5       svp_corr_beams:   %c\n",
			data->svp_corr_beams);
		fprintf(stderr,"dbg5       frequency:        %c%c\n",
			data->frequency[0],data->frequency[1]);
		fprintf(stderr,"dbg5       heave:            %d\n",
			data->heave);
		fprintf(stderr,"dbg5       range_scale:      %c\n",
			data->range_scale);
		fprintf(stderr,"dbg5       spare_ss:         %c\n", 
			data->spare_ss);
		fprintf(stderr,"dbg5       pixel_size_scale: %c\n",
			data->pixel_size_scale);
		fprintf(stderr,"dbg5       pixel_algorithm:  %c\n",
			data->pixel_algorithm);
		fprintf(stderr,"dbg5       surface_sound_velocity: %d\n",
			data->surface_sound_velocity);
		fprintf(stderr,"dbg5       ssv_source:       %c\n",
			data->ssv_source);
		fprintf(stderr,"dbg5       depth_gate_mode:  %c\n",
			data->depth_gate_mode);
		fprintf(stderr,"dbg5       num_pixels_12khz: %d\n",
			data->num_pixels_12khz);
		fprintf(stderr,"dbg5       pixel_size_12khz: %f\n",
			data->pixel_size_12khz);
		fprintf(stderr,"dbg5       ping_gain_12khz:  %d\n",
			data->ping_gain_12khz);
		fprintf(stderr,"dbg5       ping_pulse_width_12khz:        %d\n",
			data->ping_pulse_width_12khz);
		fprintf(stderr,"dbg5       transmitter_attenuation_12khz: %d\n",
			data->transmitter_attenuation_12khz);
		fprintf(stderr,"dbg5       pitch_12khz:      %d\n",
			data->pitch_12khz);
		fprintf(stderr,"dbg5       roll_12khz:       %d\n",
			data->roll_12khz);
		fprintf(stderr,"dbg5       heading_12khz:    %d\n",
			data->heading_12khz);
		fprintf(stderr,"dbg5       num_pixels_36khz: %d\n",
			data->num_pixels_36khz);
		fprintf(stderr,"dbg5       pixel_size_36khz: %f\n",
			data->pixel_size_36khz);
		fprintf(stderr,"dbg5       ping_gain_36khz:  %d\n",
			data->ping_gain_36khz);
		fprintf(stderr,"dbg5       ping_pulse_width_36khz:        %d\n",
			data->ping_pulse_width_36khz);
		fprintf(stderr,"dbg5       transmitter_attenuation_36khz: %d\n",
			data->transmitter_attenuation_36khz);
		fprintf(stderr,"dbg5       pitch_36khz:      %d\n",
			data->pitch_36khz);
		fprintf(stderr,"dbg5       roll_36khz:       %d\n",
			data->roll_36khz);
		fprintf(stderr,"dbg5       heading_36khz:    %d\n",
			data->heading_36khz);
		fprintf(stderr,"dbg5       ss_data_length:   %d\n",
			data->ss_data_length);
		}

	/* read sidescan data from character array */
	if ((status = fread(read_ss,1,data->ss_data_length+2,
		mbfp)) != data->ss_data_length+2)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
	else 
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}

	/* check for CR LF end of record - if out of place
	    we have a broken record */
	if (status == MB_SUCCESS)
		{
		char_ptr = ((char *) &read_ss[0]) + data->ss_data_length;
		if (char_ptr[0] != '\r'
			|| char_ptr[1] != '\n')
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		}
	    
	/* get the data */
	if (status == MB_SUCCESS)
		{
		read_ss_ptr = (short *) read_ss;
		for (i=0;i<data->num_pixels;i++)
			{
			/* deal with byte swapping if necessary */
#ifdef BYTESWAPPED
                        data->amplitude_ss[i] = 
				(int) mb_swap_short(read_ss[2*i]);
                        data->alongtrack_ss[i] = 
				(int) ((short) mb_swap_short(read_ss_ptr[2*i+1]));
#else
			data->amplitude_ss[i] = (int) read_ss[2*i];
			data->alongtrack_ss[i] = (int) read_ss_ptr[2*i+1];
#endif
	  		}
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"dbg5       beam amp_ss ltrack\n");
		for (i=0;i<data->num_pixels;i++)
		  {
		  fprintf(stderr,"dbg5       %3d %6d %6d\n",
			i,
			data->amplitude_ss[i],
			data->alongtrack_ss[i]);
		  }
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_sb2100rw_wr_data(int verbose, void *mbio_ptr, void *data_ptr, int *error)
{
	char	*function_name = "mbr_sb2100rw_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sb2100rw_struct *data;
	FILE	*mbfp;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %ld\n",(long)mbio_ptr);
		fprintf(stderr,"dbg2       data_ptr:   %ld\n",(long)data_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	data = (struct mbf_sb2100rw_struct *) data_ptr;
	mbfp = mb_io_ptr->mbfp;

	if (data->kind == MB_DATA_RAW_LINE)
		{
		status = mbr_sb2100rw_wr_rawline(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_VELOCITY_PROFILE)
		{
		status = mbr_sb2100rw_wr_pr(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_COMMENT)
		{
		status = mbr_sb2100rw_wr_tr(verbose,mbfp,data,error);
		}
	else if (data->kind == MB_DATA_DATA)
		{
		status = mbr_sb2100rw_wr_dr(verbose,mbfp,data,error);
		status = mbr_sb2100rw_wr_ss(verbose,mbfp,data,error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_KIND;
		}

	/* print output debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Data record kind in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       kind:       %d\n",data->kind);
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int	mbr_sb2100rw_wr_label(int verbose, FILE *mbfp, char type, int *error)
{
	char	*function_name = "mbr_sb2100rw_wr_label";
	int	status = MB_SUCCESS;
	char	line[MBF_SB2100RW_MAXLINE];

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %ld\n",(long)mbfp);
		fprintf(stderr,"dbg2       type:       %d\n",(int)type);
		}

	/* write label in file */
	sprintf(line,"%8s\r\n",mbf_sb2100rw_labels[(int)type]);
	status = mbr_sb2100rw_write_line(verbose,mbfp,line,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int	mbr_sb2100rw_write_line(int verbose, FILE *mbfp, char *line, int *error)
{
	char	*function_name = "mbr_sb2100rw_write_line";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %ld\n",(long)mbfp);
		fprintf(stderr,"dbg2       line:       %s\n",line);
		}

	/* write next line in file */
	if ((status = fputs(line,mbfp)) != EOF)
		{
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		}
	else
		{
		*error = MB_ERROR_WRITE_FAIL;
		status = MB_FAILURE;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_sb2100rw_wr_rawline(int verbose, FILE *mbfp, void *data_ptr, int *error)
{
	char	*function_name = "mbr_sb2100rw_wr_rawline";
	int	status = MB_SUCCESS;
	struct mbf_sb2100rw_struct *data;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %ld\n",(long)mbfp);
		fprintf(stderr,"dbg2       data_ptr:   %ld\n",(long)data_ptr);
		}

	/* get pointer to raw data structure */
	data = (struct mbf_sb2100rw_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       raw line:         %s\n",
			data->comment);
		}

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* output the line */
		status = fprintf(mbfp,"%s\n",data->comment);
		if (status >= 0)
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_sb2100rw_wr_pr(int verbose, FILE *mbfp, void *data_ptr, int *error)
{
	char	*function_name = "mbr_sb2100rw_wr_pr";
	int	status = MB_SUCCESS;
	struct mbf_sb2100rw_struct *data;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %ld\n",(long)mbfp);
		fprintf(stderr,"dbg2       data_ptr:   %ld\n",(long)data_ptr);
		}

	/* get pointer to raw data structure */
	data = (struct mbf_sb2100rw_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       julian day:       %d\n",data->jday);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",data->minute);
		fprintf(stderr,"dbg5       msec:             %d\n",data->msec);
		fprintf(stderr,"dbg5       roll_bias_port:   %d\n",data->roll_bias_port);
		fprintf(stderr,"dbg5       roll_bias_strbrd: %d\n",data->roll_bias_starboard);
		fprintf(stderr,"dbg5       pitch_bias:       %d\n",data->pitch_bias);
		fprintf(stderr,"dbg5       ship_draft:       %d\n",data->ship_draft);
		fprintf(stderr,"dbg5       num_svp:          %d\n",data->num_svp);
		fprintf(stderr,"dbg5       Sound Velocity Profile:\n");
		for (i=0;i<data->num_svp;i++)
			fprintf(stderr,"dbg5       %d  depth:%d  velocity:%d\n",
				i,data->vdepth[i],data->velocity[i]);
		}

	/* write the record label */
	status = mbr_sb2100rw_wr_label(verbose,mbfp,
		MBF_SB2100RW_PR,error);

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* output the first line */
		status = fprintf(mbfp,"%4.4d",data->year);
		status = fprintf(mbfp,"%3.3d",data->jday);
		status = fprintf(mbfp,"%2.2d",data->hour);
		status = fprintf(mbfp,"%2.2d",data->minute);
		status = fprintf(mbfp,"%5.5d",data->msec);
		status = fprintf(mbfp,"%+06d",data->roll_bias_port);
		status = fprintf(mbfp,"%+06d",data->pitch_bias);
		status = fprintf(mbfp,"%2.2d",data->num_svp);
		status = fprintf(mbfp,"%7.7d",data->ship_draft);
		status = fprintf(mbfp,"\r\n");

		/* output the second line */
		for (i=0;i<data->num_svp;i++)
			{
			status = fprintf(mbfp,"%7.7d",data->vdepth[i]);
			status = fprintf(mbfp,"%6.6d",data->velocity[i]);
			status = fprintf(mbfp,"\r\n");
			}

		/* check for an error */
		if (status > 0)
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_sb2100rw_wr_tr(int verbose, FILE *mbfp, void *data_ptr, int *error)
{
	char	*function_name = "mbr_sb2100rw_wr_tr";
	int	status = MB_SUCCESS;
	struct mbf_sb2100rw_struct *data;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %ld\n",(long)mbfp);
		fprintf(stderr,"dbg2       data_ptr:   %ld\n",(long)data_ptr);
		}

	/* get pointer to raw data structure */
	data = (struct mbf_sb2100rw_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       comment:          %s\n",
			data->comment);
		}

	/* write the record label */
	status = mbr_sb2100rw_wr_label(verbose,mbfp,
		MBF_SB2100RW_TR,error);

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* output the event line */
		status = fprintf(mbfp,"%s\r\n",data->comment);

		/* check for an error */
		if (status >= 0)
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_sb2100rw_wr_dr(int verbose, FILE *mbfp, void *data_ptr, int *error)
{
	char	*function_name = "mbr_sb2100rw_wr_dr";
	int	status = MB_SUCCESS;
	struct mbf_sb2100rw_struct *data;
	double	degrees;
	int	idegrees, minutes;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %ld\n",(long)mbfp);
		fprintf(stderr,"dbg2       data_ptr:   %ld\n",(long)data_ptr);
		}

	/* get pointer to raw data structure */
	data = (struct mbf_sb2100rw_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       julian day:       %d\n",data->jday);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",
			data->minute);
		fprintf(stderr,"dbg5       msec:             %d\n",data->msec);
		fprintf(stderr,"dbg5       latitude:         %f\n",
			data->latitude);
		fprintf(stderr,"dbg5       longitude:        %f\n",
			data->longitude);
		fprintf(stderr,"dbg5       speed:            %d\n",
			data->speed);
		fprintf(stderr,"dbg5       num_beams:        %d\n",
			data->num_beams);
		fprintf(stderr,"dbg5       svp_corr_beams:   %c\n",
			data->svp_corr_beams);
		fprintf(stderr,"dbg5       frequency:        %c%c\n",
			data->frequency[0],data->frequency[1]);
		fprintf(stderr,"dbg5       heave:            %d\n",
			data->heave);
		fprintf(stderr,"dbg5       spare:            ");
		for (i=0;i<2;i++)
			fprintf(stderr,"%c",data->spare_dr[i]);
		fprintf(stderr,"\n");
		fprintf(stderr,"dbg5       range_scale:      %c\n",
			data->range_scale);
		fprintf(stderr,"dbg5       num_algorithms:   %d\n",
			data->num_algorithms);
		fprintf(stderr,"dbg5       algorithm_order:  ");
		for (i=0;i<4;i++)
			fprintf(stderr,"%c",data->algorithm_order[i]);
		fprintf(stderr,"\n");
		fprintf(stderr,"dbg5       ping_gain_12khz:  %d\n",
			data->ping_gain_12khz);
		fprintf(stderr,"dbg5       ping_pulse_width_12khz:        %d\n",
			data->ping_pulse_width_12khz);
		fprintf(stderr,"dbg5       transmitter_attenuation_12khz: %d\n",
			data->transmitter_attenuation_12khz);
		fprintf(stderr,"dbg5       pitch_12khz:      %d\n",
			data->pitch_12khz);
		fprintf(stderr,"dbg5       roll_12khz:       %d\n",
			data->roll_12khz);
		fprintf(stderr,"dbg5       heading_12khz:    %d\n",
			data->heading_12khz);
		fprintf(stderr,"dbg5       ping_gain_36khz:  %d\n",
			data->ping_gain_36khz);
		fprintf(stderr,"dbg5       ping_pulse_width_36khz:        %d\n",
			data->ping_pulse_width_36khz);
		fprintf(stderr,"dbg5       transmitter_attenuation_36khz: %d\n",
			data->transmitter_attenuation_36khz);
		fprintf(stderr,"dbg5       pitch_36khz:      %d\n",
			data->pitch_36khz);
		fprintf(stderr,"dbg5       roll_36khz:       %d\n",
			data->roll_36khz);
		fprintf(stderr,"dbg5       heading_36khz:    %d\n",
			data->heading_36khz);
		fprintf(stderr,"dbg5       surface_sound_velocity: %d\n",
			data->surface_sound_velocity);
		fprintf(stderr,"dbg5       ssv_source:       %c\n",
			data->ssv_source);
		fprintf(stderr,"dbg5       depth_gate_mode:  %c\n",
			data->depth_gate_mode);
		fprintf(stderr,"dbg5       beam src tt angle angfor depth xtrack ltrack amp sig2noise echo quality\n");
		for (i=0;i<data->num_beams;i++)
		  {
		  fprintf(stderr,"dbg5       %3d %c %5d %6d %5d %5d %6d %6d %3d %2d %3d %c\n",
			i,
			data->source[i],
			data->travel_time[i],
			data->angle_across[i],
			data->angle_forward[i],
			data->depth[i],
			data->acrosstrack_beam[i],
			data->alongtrack_beam[i],
			data->amplitude_beam[i],
			data->signal_to_noise[i],
			data->echo_length[i],
			data->quality[i]);
		  }
		}

	/* write the record label */
	status = mbr_sb2100rw_wr_label(verbose,mbfp,
		MBF_SB2100RW_DR,error);

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* output the first line */
		status = fprintf(mbfp,"%4.4d",data->year);
		status = fprintf(mbfp,"%3.3d",data->jday);
		status = fprintf(mbfp,"%2.2d",data->hour);
		status = fprintf(mbfp,"%2.2d",data->minute);
		status = fprintf(mbfp,"%5.5d",data->msec);
		degrees = data->latitude;
		if (degrees < 0.0)
			{
			status = fprintf(mbfp,"S");
			degrees = -degrees;
			}
		else
			status = fprintf(mbfp,"N");
		idegrees = (int) degrees;
		minutes = (int) (600000.0*(degrees - idegrees) + 0.5);
		status = fprintf(mbfp,"%2.2d",idegrees);
		status = fprintf(mbfp,"%6.6d",minutes);
		degrees = data->longitude;
		if (degrees < -180.0)
			degrees = degrees + 360.0;
		if (degrees > 180.0)
			degrees = degrees - 360.0;
		if (degrees < 0.0)
			{
			status = fprintf(mbfp,"W");
			degrees = -degrees;
			}
		else
			status = fprintf(mbfp,"E");
		idegrees = (int) degrees;
		minutes = (int) (600000.0*(degrees - idegrees) + 0.5);
		status = fprintf(mbfp,"%3.3d",idegrees);
		status = fprintf(mbfp,"%6.6d",minutes);
		if (data->speed > 999999 || data->speed < -999999)
			data->speed = 0;
		status = fprintf(mbfp,"%+07d",data->speed);
		
		status = fprintf(mbfp,"%4.4d",data->num_beams);
		status = fprintf(mbfp,"%c",data->svp_corr_beams);
		status = fprintf(mbfp,"%c%c",
			data->frequency[0],data->frequency[1]);
		status = fprintf(mbfp,"%+06d",data->heave);
		for (i=0;i<2;i++)
			status = fprintf(mbfp,"%c",data->spare_dr[i]);
		status = fprintf(mbfp,"%c",data->range_scale);
		status = fprintf(mbfp,"%6.6d",data->surface_sound_velocity);
		status = fprintf(mbfp,"%c",data->ssv_source);
		status = fprintf(mbfp,"%c",data->depth_gate_mode);
		if (data->frequency[0] != 'H')
			{
			status = fprintf(mbfp,"%2.2d",data->ping_gain_12khz);
			status = fprintf(mbfp,"%2.2d",
				data->ping_pulse_width_12khz);
			status = fprintf(mbfp,"%02d",
				data->transmitter_attenuation_12khz);
			status = fprintf(mbfp,"%+06d",data->pitch_12khz);
			status = fprintf(mbfp,"%+06d",data->roll_12khz);
			status = fprintf(mbfp,"%6.6d",data->heading_12khz);
			}
		else
			{
			status = fprintf(mbfp,"%2.2d",data->ping_gain_36khz);
			status = fprintf(mbfp,"%2.2d",data->
				ping_pulse_width_36khz);
			status = fprintf(mbfp,"%02d",
				data->transmitter_attenuation_36khz);
			status = fprintf(mbfp,"%+06d",data->pitch_36khz);
			status = fprintf(mbfp,"%+06d",data->roll_36khz);
			status = fprintf(mbfp,"%6.6d",data->heading_36khz);
			}
		status = fprintf(mbfp,"%1d",data->num_algorithms);
		for (i=0;i<4;i++)
			status = fprintf(mbfp,"%c",data->algorithm_order[i]);
		status = fprintf(mbfp,"\r\n");

		/* output a line for each beam */
		for (i=0;i<data->num_beams;i++)
			{
			if (data->quality[i] == '0')
				{
				status = fprintf(mbfp,"                                          0\r\n");
				}
			else
				{
			        status = fprintf(mbfp,"%c",data->source[i]);
				status = fprintf(mbfp,"%5.5d",data->travel_time[i]);
				status = fprintf(mbfp,"%+06d",data->angle_across[i]);
				status = fprintf(mbfp,"%+05d",data->angle_forward[i]);
				status = fprintf(mbfp,"%5.5d",data->depth[i]);
				status = fprintf(mbfp,"%+06d",data->acrosstrack_beam[i]);
				status = fprintf(mbfp,"%+06d",data->alongtrack_beam[i]);
				status = fprintf(mbfp,"%3.3d",data->amplitude_beam[i]);
				status = fprintf(mbfp,"%2.2d",data->signal_to_noise[i]);
				status = fprintf(mbfp,"%3.3d",data->echo_length[i]);
				status = fprintf(mbfp,"%c",data->quality[i]);
				status = fprintf(mbfp,"\r\n");
				}
			}

		/* check for an error */
		if (status > 0)
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_sb2100rw_wr_ss(int verbose, FILE *mbfp, void *data_ptr, int *error)
{
	char	*function_name = "mbr_sb2100rw_wr_ss";
	int	status = MB_SUCCESS;
	struct mbf_sb2100rw_struct *data;
	unsigned short	write_ss[2*MBF_SB2100RW_PIXELS];
	short	*write_ss_ptr;
	double	degrees;
	int	idegrees, minutes;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %ld\n",(long)mbfp);
		fprintf(stderr,"dbg2       data_ptr:   %ld\n",(long)data_ptr);
		}

	/* get pointer to raw data structure */
	data = (struct mbf_sb2100rw_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       julian day:       %d\n",data->jday);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",
			data->minute);
		fprintf(stderr,"dbg5       msec:             %d\n",data->msec);
		fprintf(stderr,"dbg5       latitude:         %f\n",
			data->latitude);
		fprintf(stderr,"dbg5       longitude:        %f\n",
			data->longitude);
		fprintf(stderr,"dbg5       speed:            %d\n",
			data->speed);
		fprintf(stderr,"dbg5       num_pixels:       %d\n",
			data->num_pixels);
		fprintf(stderr,"dbg5       ss_data_length:   %d\n",
			data->ss_data_length);
		fprintf(stderr,"dbg5       svp_corr_beams:   %c\n",
			data->svp_corr_beams);
		fprintf(stderr,"dbg5       frequency:        %c%c\n",
			data->frequency[0],data->frequency[1]);
		fprintf(stderr,"dbg5       heave:            %d\n",
			data->heave);
		fprintf(stderr,"dbg5       range_scale:      %c\n",
			data->range_scale);
		fprintf(stderr,"dbg5       spare_ss:         %c\n",
			data->spare_ss);
		fprintf(stderr,"dbg5       pixel_size_scale: %c\n",
			data->pixel_size_scale);
		fprintf(stderr,"dbg5       pixel_algorithm:  %c\n",
			data->pixel_algorithm);
		fprintf(stderr,"dbg5       surface_sound_velocity: %d\n",
			data->surface_sound_velocity);
		fprintf(stderr,"dbg5       ssv_source:       %c\n",
			data->ssv_source);
		fprintf(stderr,"dbg5       depth_gate_mode:  %c\n",
			data->depth_gate_mode);
		fprintf(stderr,"dbg5       num_pixels_12khz: %d\n",
			data->num_pixels_12khz);
		fprintf(stderr,"dbg5       pixel_size_12khz: %f\n",
			data->pixel_size_12khz);
		fprintf(stderr,"dbg5       ping_gain_12khz:  %d\n",
			data->ping_gain_12khz);
		fprintf(stderr,"dbg5       ping_pulse_width_12khz:        %d\n",
			data->ping_pulse_width_12khz);
		fprintf(stderr,"dbg5       transmitter_attenuation_12khz: %d\n",
			data->transmitter_attenuation_12khz);
		fprintf(stderr,"dbg5       pitch_12khz:      %d\n",
			data->pitch_12khz);
		fprintf(stderr,"dbg5       roll_12khz:       %d\n",
			data->roll_12khz);
		fprintf(stderr,"dbg5       heading_12khz:    %d\n",
			data->heading_12khz);
		fprintf(stderr,"dbg5       num_pixels_36khz: %d\n",
			data->num_pixels_36khz);
		fprintf(stderr,"dbg5       pixel_size_36khz: %f\n",
			data->pixel_size_36khz);
		fprintf(stderr,"dbg5       ping_gain_36khz:  %d\n",
			data->ping_gain_36khz);
		fprintf(stderr,"dbg5       ping_pulse_width_36khz:        %d\n",
			data->ping_pulse_width_36khz);
		fprintf(stderr,"dbg5       transmitter_attenuation_36khz: %d\n",
			data->transmitter_attenuation_36khz);
		fprintf(stderr,"dbg5       pitch_36khz:      %d\n",
			data->pitch_36khz);
		fprintf(stderr,"dbg5       roll_36khz:       %d\n",
			data->roll_36khz);
		fprintf(stderr,"dbg5       heading_36khz:    %d\n",
			data->heading_36khz);
		fprintf(stderr,"dbg5       beam amp_ss ltrack\n");
		for (i=0;i<data->num_pixels;i++)
		  {
		  fprintf(stderr,"dbg5       %3d %6d %6d\n",
			i,
			data->amplitude_ss[i],
			data->alongtrack_ss[i]);
		  }
		}

	/* write the record label */
	status = mbr_sb2100rw_wr_label(verbose,mbfp,
		MBF_SB2100RW_SS,error);

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* output the event line */
		status = fprintf(mbfp,"%4.4d",data->year);
		status = fprintf(mbfp,"%3.3d",data->jday);
		status = fprintf(mbfp,"%2.2d",data->hour);
		status = fprintf(mbfp,"%2.2d",data->minute);
		status = fprintf(mbfp,"%5.5d",data->msec);
		degrees = data->latitude;
		if (degrees < 0.0)
			{
			status = fprintf(mbfp,"S");
			degrees = -degrees;
			}
		else
			status = fprintf(mbfp,"N");
		idegrees = (int) degrees;
		minutes = (int) (600000.0*(degrees - idegrees) + 0.5);
		status = fprintf(mbfp,"%2.2d",idegrees);
		status = fprintf(mbfp,"%6.6d",minutes);
		degrees = data->longitude;
		if (degrees < -180.0)
			degrees = degrees + 360.0;
		if (degrees > 180.0)
			degrees = degrees - 360.0;
		if (degrees < 0.0)
			{
			status = fprintf(mbfp,"W");
			degrees = -degrees;
			}
		else
			status = fprintf(mbfp,"E");
		idegrees = (int) degrees;
		minutes = (int) (600000.0*(degrees - idegrees) + 0.5);
		status = fprintf(mbfp,"%3.3d",idegrees);
		status = fprintf(mbfp,"%6.6d",minutes);
		status = fprintf(mbfp,"%+07d",data->speed);
		data->ss_data_length = 4*data->num_pixels;
		status = fprintf(mbfp,"%4.4d",data->ss_data_length);
		status = fprintf(mbfp,"%c",data->svp_corr_beams);
		status = fprintf(mbfp,"%c%c",
			data->frequency[0],data->frequency[1]);
		status = fprintf(mbfp,"%+06d",data->heave);
		status = fprintf(mbfp,"%c",data->range_scale);
		status = fprintf(mbfp,"%c",data->spare_ss);
		status = fprintf(mbfp,"%c",data->pixel_size_scale);
		status = fprintf(mbfp,"%c",data->pixel_algorithm);
		status = fprintf(mbfp,"%6.6d",data->surface_sound_velocity);
		status = fprintf(mbfp,"%c",data->ssv_source);
		status = fprintf(mbfp,"%c",data->depth_gate_mode);
		if (data->frequency[0] != 'H')
			{
			status = fprintf(mbfp,"%4.4d",data->num_pixels_12khz);
			if (data->pixel_size_12khz > 9.99)
				status = fprintf(mbfp,"%4.1f",
					data->pixel_size_12khz);
			else if (data->pixel_size_12khz > 0.999)
				status = fprintf(mbfp,"%4.2f",
					data->pixel_size_12khz);
			else
				status = fprintf(mbfp,".%03d",
					(int)(1000*data->pixel_size_12khz));
			status = fprintf(mbfp,"%2.2d",data->ping_gain_12khz);
			status = fprintf(mbfp,"%2.2d",
				data->ping_pulse_width_12khz);
			status = fprintf(mbfp,"%02d",
				data->transmitter_attenuation_12khz);
			status = fprintf(mbfp,"%+06d",data->pitch_12khz);
			status = fprintf(mbfp,"%+06d",data->roll_12khz);
			status = fprintf(mbfp,"%6.6d",data->heading_12khz);
			}
		else 
			{
			status = fprintf(mbfp,"%4.4d",data->num_pixels_36khz);
			if (data->pixel_size_36khz > 9.99)
				status = fprintf(mbfp,"%4.1f",
					data->pixel_size_36khz);
			else if (data->pixel_size_36khz > 0.999)
				status = fprintf(mbfp,"%4.2f",
					data->pixel_size_36khz);
			else
				status = fprintf(mbfp,".%03d",
					(int)(1000*data->pixel_size_36khz));
			status = fprintf(mbfp,"%2.2d",data->ping_gain_36khz);
			status = fprintf(mbfp,"%2.2d",data->
				ping_pulse_width_36khz);
			status = fprintf(mbfp,"%02d",
				data->transmitter_attenuation_36khz);
			status = fprintf(mbfp,"%+06d",data->pitch_36khz);
			status = fprintf(mbfp,"%+06d",data->roll_36khz);
			status = fprintf(mbfp,"%6.6d",data->heading_36khz);
			}
		status = fprintf(mbfp,"\r\n");

		/* construct and write out sidescan data */
		write_ss_ptr = (short *) write_ss;
		for (i=0;i<data->num_pixels;i++)
			{
			/* deal with byte swapping if necessary */
#ifdef BYTESWAPPED
			write_ss[2*i] = mb_swap_short((unsigned short) 
				data->amplitude_ss[i]);
			write_ss_ptr[2*i+1] = mb_swap_short((short) 
				data->alongtrack_ss[i]);
#else
			write_ss[2*i] = (unsigned short) data->amplitude_ss[i];
			write_ss_ptr[2*i+1] = (short) data->alongtrack_ss[i];
#endif
	  		}
		if ((status = fwrite(write_ss,1,data->ss_data_length,mbfp))
			!= data->ss_data_length)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		else
			{
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			status = fprintf(mbfp,"\r\n");
			}


		/* check for an error */
		if (status > 0)
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
