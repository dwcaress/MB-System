/*--------------------------------------------------------------------
 *    The MB-system:	mbr_l3xseraw.c	3/27/2000
 *	$Id$
 *
 *    Copyright (c) 2000-2009 by 
 *    D. W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbr_sb2102xs.c contains the functions for reading and writing
 * multibeam data in the L3XSERAW format.  
 * These functions include:
 *   mbr_alm_l3xseraw	- allocate read/write memory
 *   mbr_dem_l3xseraw	- deallocate read/write memory
 *   mbr_rt_l3xseraw	- read and translate data
 *   mbr_wt_l3xseraw	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	March 27, 1999
 * Additional Authors:	P. A. Cohen and S. Dzurenko
 *
 * $Log: mbr_l3xseraw.c,v $
 * Revision 5.21  2008/07/10 06:43:40  caress
 * Preparing for 5.1.1beta20
 *
 * Revision 5.20  2007/07/03 17:28:08  caress
 * Fixes to XSE format.
 *
 * Revision 5.19  2007/06/18 01:19:48  caress
 * Changes as of 17 June 2007.
 *
 * Revision 5.18  2006/12/15 21:36:16  caress
 * Turned off debug mode.
 *
 * Revision 5.17  2006/09/11 18:55:52  caress
 * Changes during Western Flyer and Thomas Thompson cruises, August-September
 * 2006.
 *
 * Revision 5.16  2006/08/04 03:56:41  caress
 * Working towards 5.1.0 release.
 *
 * Revision 5.15  2006/01/06 18:27:19  caress
 * Working towards 5.0.8
 *
 * Revision 5.14  2005/11/05 00:48:04  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.13  2003/05/20 18:05:32  caress
 * Added svp_source to data source parameters.
 *
 * Revision 5.12  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.11  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.10  2001/12/30 20:32:12  caress
 * Fixed array overflows in handling XSE data.
 *
 * Revision 5.9  2001/12/20 20:48:51  caress
 * Release 5.0.beta11
 *
 * Revision 5.8  2001/10/23  02:34:12  caress
 * Added error checking for corrupted depth and heave values.
 *
 * Revision 5.7  2001/08/23  20:50:24  caress
 * Fixed problems with SB2120 data.
 *
 * Revision 5.6  2001/08/10  22:41:19  dcaress
 * Release 5.0.beta07
 *
 * Revision 5.5  2001-07-22 14:17:01-07  caress
 * Fixed bug that deallocated an unallocated array.
 *
 * Revision 5.4  2001/07/20 00:32:54  caress
 * Release 5.0.beta03
 *
 * Revision 5.3  2001/06/08  21:44:01  caress
 * Version 5.0.beta01
 *
 * Revision 5.2  2001/06/03  06:54:56  caress
 * Fixed support for xse format on byte swapped computers.
 *
 * Revision 5.1  2001/04/09  18:04:51  caress
 * Fixed bug related to nav handling.
 *
 * Revision 5.0  2001/04/05  18:33:25  caress
 * Initial version.
 * Consolidates two former i/o modules elmk2xse and sb2120xs.
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
#include "../../include/mbsys_xse.h"

/* include for byte swapping */
#include "../../include/mb_swap.h"


/* #define MB_DEBUG 1 */
/* #define MB_DEBUG2 1 */

/* set up byte swapping scenario */
#ifdef DATAINPCBYTEORDER
#define SWAPFLAG    MB_YES
#else
#define SWAPFLAG    MB_NO
#endif

/* essential function prototypes */
int mbr_register_l3xseraw(int verbose, void *mbio_ptr, 
		int *error);
int mbr_info_l3xseraw(int verbose, 
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
int mbr_alm_l3xseraw(int verbose, void *mbio_ptr, int *error);
int mbr_dem_l3xseraw(int verbose, void *mbio_ptr, int *error);
int mbr_rt_l3xseraw(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_l3xseraw(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_l3xseraw_rd_data(int verbose,void *mbio_ptr,void *store_ptr,int *error);
int mbr_l3xseraw_rd_nav(int verbose,int buffer_size,char *buffer,void *store_ptr,int *error);
int mbr_l3xseraw_rd_svp(int verbose,int buffer_size,char *buffer,void *store_ptr,int *error);
int mbr_l3xseraw_rd_tide(int verbose,int buffer_size,char *buffer,void *store_ptr,int *error);
int mbr_l3xseraw_rd_ship(int verbose,int buffer_size,char *buffer,void *store_ptr,int *error);
int mbr_l3xseraw_rd_sidescan(int verbose,int buffer_size,char *buffer,void *store_ptr,int *error);
int mbr_l3xseraw_rd_multibeam(int verbose,int buffer_size,char *buffer,void *store_ptr,int *error);
int mbr_l3xseraw_rd_singlebeam(int verbose,int buffer_size,char *buffer,void *store_ptr,int *error);
int mbr_l3xseraw_rd_message(int verbose,int buffer_size,char *buffer,void *store_ptr,int *error);
int mbr_l3xseraw_rd_seabeam(int verbose,int buffer_size,char *buffer,void *store_ptr,int *error);
int mbr_l3xseraw_rd_geodetic(int verbose,int buffer_size,char *buffer,void *store_ptr,int *error);
int mbr_l3xseraw_rd_native(int verbose,int buffer_size,char *buffer,void *store_ptr,int *error);
int mbr_l3xseraw_rd_product(int verbose,int buffer_size,char *buffer,void *store_ptr,int *error);
int mbr_l3xseraw_rd_bathymetry(int verbose,int buffer_size,char *buffer,void *store_ptr,int *error);
int mbr_l3xseraw_rd_control(int verbose,int buffer_size,char *buffer,void *store_ptr,int *error);
int mbr_l3xseraw_rd_comment(int verbose,int buffer_size,char *buffer,void *store_ptr,int *error);
int mbr_l3xseraw_wr_data(int verbose,void *mbio_ptr,void *store_ptr,int *error);
int mbr_l3xseraw_wr_nav(int verbose,int *buffer_size,char *buffer,void *store_ptr,int *error);
int mbr_l3xseraw_wr_svp(int verbose,int *buffer_size,char *buffer,void *store_ptr,int *error);
int mbr_l3xseraw_wr_ship(int verbose,int *buffer_size,char *buffer,void *store_ptr,int *error);
int mbr_l3xseraw_wr_multibeam(int verbose,int *buffer_size,char *buffer,void *store_ptr,int *error);
int mbr_l3xseraw_wr_sidescan(int verbose,int *buffer_size,char *buffer,void *store_ptr,int *error);
int mbr_l3xseraw_wr_seabeam(int verbose,int *buffer_size,char *buffer,void *store_ptr,int *error);
int mbr_l3xseraw_wr_comment(int verbose,int *buffer_size,char *buffer,void *store_ptr,int *error);

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/
int mbr_register_l3xseraw(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_register_l3xseraw";
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
	status = mbr_info_l3xseraw(verbose, 
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_l3xseraw;
	mb_io_ptr->mb_io_format_free = &mbr_dem_l3xseraw; 
	mb_io_ptr->mb_io_store_alloc = &mbsys_xse_alloc; 
	mb_io_ptr->mb_io_store_free = &mbsys_xse_deall; 
	mb_io_ptr->mb_io_read_ping = &mbr_rt_l3xseraw; 
	mb_io_ptr->mb_io_write_ping = &mbr_wt_l3xseraw; 
	mb_io_ptr->mb_io_dimensions = &mbsys_xse_dimensions; 
	mb_io_ptr->mb_io_extract = &mbsys_xse_extract; 
	mb_io_ptr->mb_io_insert = &mbsys_xse_insert; 
	mb_io_ptr->mb_io_extract_nav = &mbsys_xse_extract_nav; 
	mb_io_ptr->mb_io_insert_nav = &mbsys_xse_insert_nav; 
	mb_io_ptr->mb_io_extract_altitude = &mbsys_xse_extract_altitude; 
	mb_io_ptr->mb_io_insert_altitude = NULL; 
	mb_io_ptr->mb_io_extract_svp = &mbsys_xse_extract_svp; 
	mb_io_ptr->mb_io_insert_svp = &mbsys_xse_insert_svp; 
	mb_io_ptr->mb_io_ttimes = &mbsys_xse_ttimes; 
	mb_io_ptr->mb_io_copyrecord = &mbsys_xse_copy; 
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
		fprintf(stderr,"dbg2       format_alloc:       %lu\n",(size_t)mb_io_ptr->mb_io_format_alloc);
		fprintf(stderr,"dbg2       format_free:        %lu\n",(size_t)mb_io_ptr->mb_io_format_free);
		fprintf(stderr,"dbg2       store_alloc:        %lu\n",(size_t)mb_io_ptr->mb_io_store_alloc);
		fprintf(stderr,"dbg2       store_free:         %lu\n",(size_t)mb_io_ptr->mb_io_store_free);
		fprintf(stderr,"dbg2       read_ping:          %lu\n",(size_t)mb_io_ptr->mb_io_read_ping);
		fprintf(stderr,"dbg2       write_ping:         %lu\n",(size_t)mb_io_ptr->mb_io_write_ping);
		fprintf(stderr,"dbg2       extract:            %lu\n",(size_t)mb_io_ptr->mb_io_extract);
		fprintf(stderr,"dbg2       insert:             %lu\n",(size_t)mb_io_ptr->mb_io_insert);
		fprintf(stderr,"dbg2       extract_nav:        %lu\n",(size_t)mb_io_ptr->mb_io_extract_nav);
		fprintf(stderr,"dbg2       insert_nav:         %lu\n",(size_t)mb_io_ptr->mb_io_insert_nav);
		fprintf(stderr,"dbg2       extract_altitude:   %lu\n",(size_t)mb_io_ptr->mb_io_extract_altitude);
		fprintf(stderr,"dbg2       insert_altitude:    %lu\n",(size_t)mb_io_ptr->mb_io_insert_altitude);
		fprintf(stderr,"dbg2       extract_svp:        %lu\n",(size_t)mb_io_ptr->mb_io_extract_svp);
		fprintf(stderr,"dbg2       insert_svp:         %lu\n",(size_t)mb_io_ptr->mb_io_insert_svp);
		fprintf(stderr,"dbg2       ttimes:             %lu\n",(size_t)mb_io_ptr->mb_io_ttimes);
		fprintf(stderr,"dbg2       extract_rawss:      %lu\n",(size_t)mb_io_ptr->mb_io_extract_rawss);
		fprintf(stderr,"dbg2       insert_rawss:       %lu\n",(size_t)mb_io_ptr->mb_io_insert_rawss);
		fprintf(stderr,"dbg2       copyrecord:         %lu\n",(size_t)mb_io_ptr->mb_io_copyrecord);
		fprintf(stderr,"dbg2       error:              %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		}

	/* return status */
	return(status);
}

/*--------------------------------------------------------------------*/
int mbr_info_l3xseraw(int verbose, 
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
	char	*function_name = "mbr_info_l3xseraw";
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
	*system = MB_SYS_XSE;
	*beams_bath_max = 151;
	*beams_amp_max = 151;
	*pixels_ss_max = 2000;
	strncpy(format_name, "L3XSERAW", MB_NAME_LENGTH);
	strncpy(system_name, "XSE", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_L3XSERAW\nInformal Description: ELAC/SeaBeam XSE vendor format\nAttributes:           Bottomchart MkII 50 kHz and 180 kHz multibeam, \n                      SeaBeam 2120 20 KHz multibeam,\n		      bathymetry, amplitude and sidescan,\n                      variable beams and pixels, binary, \n                      L3 Communications (Elac Nautik \n                      and SeaBeam Instruments).\n", MB_DESCRIPTION_LENGTH);
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
int mbr_alm_l3xseraw(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_alm_l3xseraw";
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

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set initial status */
	status = MB_SUCCESS;

	/* allocate memory for data structure */
	status = mb_mallocd(verbose,__FILE__,__LINE__,MBSYS_XSE_BUFFER_SIZE,
				(void **)&mb_io_ptr->hdr_comment,error);
	mbsys_xse_alloc(verbose,mbio_ptr,
				&mb_io_ptr->store_data,error);

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
int mbr_dem_l3xseraw(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_l3xseraw";
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

	/* get pointers to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* deallocate memory for data descriptor */
	status = mb_freed(verbose,__FILE__, __LINE__, (void **)&mb_io_ptr->store_data,error);
	status = mb_freed(verbose,__FILE__, __LINE__, (void **)&mb_io_ptr->hdr_comment,error);

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
int mbr_rt_l3xseraw(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_l3xseraw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_xse_struct *store;
	double	time_d;
	double	lon, lat;
	double	heading;
	double	speed;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get pointers to mbio descriptor and data structures */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	store = (struct mbsys_xse_struct *) store_ptr;

	/* read next data from file */
	status = mbr_l3xseraw_rd_data(verbose,mbio_ptr,store_ptr,error);

/*fprintf(stderr, "read kind:%d\n", store->kind);
fprintf(stderr, "store->mul_frame:%d store->sid_frame:%d\n\n", 
store->mul_frame, store->sid_frame);*/

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = store->kind;

	/* save fix if nav data */
	if (status == MB_SUCCESS
		&& store->kind == MB_DATA_NAV)
		{
		/* get time */
		time_d = store->nav_sec 
			    - MBSYS_XSE_TIME_OFFSET
			    + 0.000001 * store->nav_usec;
			    
		/* add nav to navlist */
		if (store->nav_group_position == MB_YES)
			mb_navint_add(verbose, mbio_ptr, time_d, 
				RTD * store->nav_x, 
				RTD * store->nav_y, error);	    
			    
		/* add heading to navlist */
		if (store->nav_group_heading == MB_YES)
			mb_hedint_add(verbose, mbio_ptr, time_d, 
				RTD * store->nav_hdg_heading, error);	    
		else if (store->nav_group_motiongt == MB_YES)
			mb_hedint_add(verbose, mbio_ptr, time_d, 
				RTD * store->nav_course_ground, error);	    
		else if (store->nav_group_motiontw == MB_YES)
			mb_hedint_add(verbose, mbio_ptr, time_d, 
				RTD * store->nav_course_water, error);	    
		}

	/* interpolate navigation for survey pings if needed */
	if (status == MB_SUCCESS 
		&& store->kind == MB_DATA_DATA
		&& store->mul_group_mbsystemnav == MB_NO)
		{
		/* get timestamp */
		time_d = store->mul_sec 
			    - MBSYS_XSE_TIME_OFFSET
			    + 0.000001 * store->mul_usec;
			    
		/* interpolate heading */
		mb_hedint_interp(verbose, mbio_ptr, time_d, 
				 &heading, error);

		/* get speed if possible */
		if (store->nav_group_log == MB_YES)
			speed = 3.6 * store->nav_log_speed;	    
		else if (store->nav_group_motiongt == MB_YES)
			speed = 3.6 * store->nav_speed_ground;	    
		else if (store->nav_group_motiontw == MB_YES)
			speed = 3.6 * store->nav_speed_water;
		else
			speed = 0.0;

		/* interpolate position */
		mb_navint_interp(verbose, mbio_ptr, time_d, heading, speed, 
				    &lon, &lat, &speed, error);
				    
		/* set values */
		store->mul_lon = DTR * lon;
		store->mul_lat = DTR * lat;
		store->mul_heading = DTR * heading;
		store->mul_speed = speed / 3.6;
		store->mul_group_mbsystemnav = MB_YES;
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
int mbr_wt_l3xseraw(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_l3xseraw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_xse_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to store data structure */
	store = (struct mbsys_xse_struct *) store_ptr;

	/* write next data to file */
	status = mbr_l3xseraw_wr_data(verbose,mbio_ptr,store_ptr,error);

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
int mbr_l3xseraw_rd_data(int verbose,void *mbio_ptr,void *store_ptr,int *error)
{
	char	*function_name = "mbr_l3xseraw_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_xse_struct *store;
	FILE	*mbfp;
	static char label[4];
	int	done;
	int	frame_id;
	int	frame_source;
	int	frame_sec;
	int	frame_usec;
	int	frame_transaction;
	int	frame_address;
	int	buffer_size;
	int	frame_size;
	int	*buffer_size_max;
	int	*frame_save;
	int	*frame_expect;
	int	*frame_id_save;
	int	*frame_source_save;
	int	*frame_sec_save;
	int	*frame_usec_save;
	int	*buffer_size_save;
	char	*buffer;
	int	index;
	int	read_len;
	int	skip;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to store data structure */
	store = (struct mbsys_xse_struct *) store_ptr;
	mbfp = mb_io_ptr->mbfp;

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;
	
	/* read until done */
	*error = MB_ERROR_NO_ERROR;
	frame_expect = (int *) &mb_io_ptr->save1;
	frame_save = (int *) &mb_io_ptr->save2;
	frame_id_save = (int *) &mb_io_ptr->save3;
	frame_source_save = (int *) &mb_io_ptr->save4;
	frame_sec_save = (int *) &mb_io_ptr->save5;
	frame_usec_save = (int *) &mb_io_ptr->save6;
	buffer_size_save = (int *) &mb_io_ptr->save7;
	buffer_size_max = (int *) &mb_io_ptr->save8;
	buffer = mb_io_ptr->hdr_comment;
	store->sbm_properties = MB_NO;
	store->sbm_hrp = MB_NO;
	store->sbm_center = MB_NO;
	store->sbm_message = MB_NO;
	done = MB_NO;
	if (*frame_save == MB_YES)
	    {
	    store->mul_frame = MB_NO;
	    store->sid_frame = MB_NO;
	    }
	while (done == MB_NO)
	    {
	    /* use saved frame if available */
	    if (*frame_save == MB_YES)
			{
			frame_id = *frame_id_save;
			frame_source = *frame_source_save;
			frame_sec = *frame_sec_save;
			frame_usec = *frame_usec_save;
			buffer_size = *buffer_size_save;
			*frame_save = MB_NO;
			}
		
	    /* else read from file */
	    else
		{
		
		/* look for the next frame start */
		skip = 0;
		if ((read_len = fread(&label[0],1,4,mb_io_ptr->mbfp)) != 4)
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;
		    }
#ifdef DATAINPCBYTEORDER
		while (status == MB_SUCCESS
		    && strncmp(label, "FSH$", 4))
#else
		while (status == MB_SUCCESS
		    && strncmp(label, "$HSF", 4))
#endif
		    {
		    /* get next byte */
		    for (i=0;i<3;i++)
				label[i] = label[i+1];
		    if ((read_len = fread(&label[3],1,1,mb_io_ptr->mbfp)) != 1)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				}
		    else
				{
				skip++;
				}
		    }

		/* Read entire data record into buffer. The XSE frame byte count value */
		/* is notorious for being incorrect.  So we read the data record by */
		/* reading up to the next frame end mark. */
		
		/* copy the frame start label to the buffer */
		if(status==MB_SUCCESS)
			{
			strncpy(buffer, label, 4);
			index = 4;
			buffer_size = 4;
			}

		/* Read next four bytes from the file into buffer to get us started. */
		if (status == MB_SUCCESS)
		    	{
			if ((read_len = fread(&buffer[index],1,4,mb_io_ptr->mbfp)) != 4)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				frame_size = 0;
				}
			else
				{
				buffer_size += 4;
		    		mb_get_binary_int(SWAPFLAG, &buffer[4], (int *) &frame_size);
				}
			}
		
		/* now read a byte at a time, continuing until we find the end mark */
#ifdef DATAINPCBYTEORDER
		while (status == MB_SUCCESS
		    && strncmp(&buffer[index], "FSH#", 4))
#else
		while (status == MB_SUCCESS
		    && strncmp(&buffer[index], "#HSF", 4))
#endif
		    {
		    /* read next byte */
		    if ((read_len = fread(&buffer[buffer_size],1,1,mb_io_ptr->mbfp)) != 1)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				}
			else
				{
				buffer_size++;
				index++;
				}

		    /* don't let buffer overflow - error if record exceeds max size */
		    if (buffer_size >= MBSYS_XSE_BUFFER_SIZE)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_UNINTELLIGIBLE;
			}
		    }
		*buffer_size_max = MAX(buffer_size,*buffer_size_max);

#ifdef MB_DEBUG
if (*error != MB_ERROR_EOF)
{
if (skip > 0)
fprintf(stderr, "\nBYTES SKIPPED BETWEEN FRAMES: %d\n", skip);
fprintf(stderr, "\nBUFFER SIZE: %u  MAX FOUND: %u  MAX: %u\n", 
buffer_size,*buffer_size_max,MBSYS_XSE_BUFFER_SIZE);
}
#endif

		/* parse header values */
		if (status == MB_SUCCESS)
		    {
		    /* get frame id, source, and time */
		    index = 8;
		    mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &frame_id); index += 4;
		    mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &frame_source); index += 4;
		    mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &frame_sec); index += 4;
		    mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &frame_usec); index += 4;

		    /* if it's a control frame, get the transaction and address values */
		    if(frame_id == MBSYS_XSE_CNT_FRAME)
			    {
			    mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &frame_transaction); index += 4;
			    mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &frame_address); index += 4;
			    }
		    }
		}
				
	    /* parse data if possible */
	    if (status == MB_SUCCESS)
			{
#ifdef MB_DEBUG
fprintf(stderr, "FRAME ID: %u  BUFFER SIZE:%d  FRAME SIZE:%d\n",frame_id,buffer_size,frame_size);
#endif
			if (frame_id == MBSYS_XSE_NAV_FRAME)
			    {
#ifdef MB_DEBUG
fprintf(stderr, "READ NAV\n");
#endif
			    status = mbr_l3xseraw_rd_nav(verbose,buffer_size,buffer,store_ptr,error);
			    if (store->nav_source > 0)
				{
				store->kind = MB_DATA_NAV;
#ifdef MB_DEBUG
fprintf(stderr,"nav_source:%d  time:%u.%6.6u\n",store->nav_source,store->nav_sec,store->nav_usec);
#endif
				}
			    else
				store->kind = MB_DATA_RAW_LINE;
			    done = MB_YES;
			    }
			else if (frame_id == MBSYS_XSE_SVP_FRAME)
			    {
#ifdef MB_DEBUG
fprintf(stderr, "READ SVP\n");
#endif
			    store->kind = MB_DATA_VELOCITY_PROFILE;
			    status = mbr_l3xseraw_rd_svp(verbose,buffer_size,buffer,store_ptr,error);
			    done = MB_YES;
			    }
			else if (frame_id == MBSYS_XSE_TID_FRAME)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ TIDE\n");
#endif
				store->kind = MB_DATA_RAW_LINE;
				status = mbr_l3xseraw_rd_tide(verbose,buffer_size,buffer,store_ptr,error);
				done = MB_YES;
				}
			else if (frame_id == MBSYS_XSE_SHP_FRAME)
			    {
#ifdef MB_DEBUG
fprintf(stderr, "READ PARAMETER\n");
#endif
			    store->kind = MB_DATA_PARAMETER;
			    status = mbr_l3xseraw_rd_ship(verbose,buffer_size,buffer,store_ptr,error);
			    done = MB_YES;
			    }
			else if (frame_id == MBSYS_XSE_SSN_FRAME)
			    {
#ifdef MB_DEBUG
fprintf(stderr, "READ SIDESCAN\n");
#endif
			    store->kind = MB_DATA_DATA;
			    status = mbr_l3xseraw_rd_sidescan(verbose,buffer_size,buffer,store_ptr,error);
			    store->sid_frame = MB_YES;
			    if (frame_id == *frame_expect
				    && store->sid_ping == store->mul_ping
				    && store->sid_group_avl == MB_YES)
				    {
				    *frame_expect = MBSYS_XSE_NONE_FRAME;
				    done = MB_YES;
				    }
			    else if (frame_id == *frame_expect
				    && store->sid_ping == store->mul_ping
				    && store->sid_group_avl == MB_NO)
				    {
				    done = MB_NO;
				    }
			    else if (*frame_expect == MBSYS_XSE_NONE_FRAME)
				    {
				    *frame_expect = MBSYS_XSE_MBM_FRAME;
				    done = MB_NO;
				    }
#ifdef MB_DEBUG
fprintf(stderr, "\tframe_id:%d frame_expect:%d ping:%d %d sid_group_avl:%d\n",
frame_id,*frame_expect,store->sid_ping,store->mul_ping,store->sid_group_avl);
fprintf(stderr, "\tDONE:%d BEAMS:%d PIXELS:%d\n", done, store->mul_num_beams, store->sid_avl_num_samples);
#endif
			    }
			else if (frame_id == MBSYS_XSE_MBM_FRAME
				&& *frame_expect == MBSYS_XSE_SSN_FRAME)
			    {
#ifdef MB_DEBUG
fprintf(stderr, "READ NOTHING - SAVE HEADER\n");
#endif
			    store->kind = MB_DATA_DATA;
			    *frame_save = MB_YES;
			    *frame_id_save = frame_id;
			    *frame_source_save = frame_source;
			    *frame_sec_save = frame_sec;
			    *frame_usec_save = frame_usec;
			    *buffer_size_save = buffer_size;
			    *frame_expect = MBSYS_XSE_NONE_FRAME;
			    done = MB_YES;
#ifdef MB_DEBUG
fprintf(stderr, "\tDONE:%d BEAMS:%d PIXELS:%d\n", done, store->mul_num_beams, store->sid_avl_num_samples);
#endif
			    }
			else if (frame_id == MBSYS_XSE_MBM_FRAME)
			    {
#ifdef MB_DEBUG
fprintf(stderr, "READ MULTIBEAM\n");
#endif
			    store->kind = MB_DATA_DATA;
			    status = mbr_l3xseraw_rd_multibeam(verbose,buffer_size,buffer,store_ptr,error);
			    store->mul_frame = MB_YES;
			    if (frame_id == *frame_expect
				    && store->sid_ping == store->mul_ping)
				    {
				    *frame_expect = MBSYS_XSE_NONE_FRAME;
				    done = MB_YES;
				    }
			    else if (frame_id == *frame_expect)
				    {
				    *frame_expect = MBSYS_XSE_SSN_FRAME;
				    done = MB_NO;
				    }
			    else if (*frame_expect == MBSYS_XSE_NONE_FRAME)
				    {
				    *frame_expect = MBSYS_XSE_SSN_FRAME;
				    done = MB_NO;
				    }
#ifdef MB_DEBUG
fprintf(stderr, "\tDONE:%d BEAMS:%d PIXELS:%d\n", done, store->mul_num_beams, store->sid_avl_num_samples);
#endif
			    }
			else if (frame_id == MBSYS_XSE_SNG_FRAME)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ SINGLEBEAM\n");
#endif
				store->kind = MB_DATA_RAW_LINE;
				status = mbr_l3xseraw_rd_singlebeam(verbose,buffer_size,buffer,store_ptr,error);
				done = MB_YES;
				}
			else if (frame_id == MBSYS_XSE_CNT_FRAME)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ CONTROL\n");
#endif
				store->kind = MB_DATA_RAW_LINE;
				status = mbr_l3xseraw_rd_control(verbose,buffer_size,buffer,store_ptr,error);
				done = MB_YES;
				}
			else if (frame_id == MBSYS_XSE_BTH_FRAME)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ BATHYMETRY\n");
#endif
				store->kind = MB_DATA_RAW_LINE;
				status = mbr_l3xseraw_rd_bathymetry(verbose,buffer_size,buffer,store_ptr,error);
				done = MB_YES;
				}
			else if (frame_id == MBSYS_XSE_PRD_FRAME)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ PRODUCT\n");
#endif
				store->kind = MB_DATA_RAW_LINE;
				status = mbr_l3xseraw_rd_product(verbose,buffer_size,buffer,store_ptr,error);
				done = MB_YES;
				}
			else if (frame_id == MBSYS_XSE_NTV_FRAME)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ NATIVE\n");
#endif
				store->kind = MB_DATA_RAW_LINE;
				status = mbr_l3xseraw_rd_native(verbose,buffer_size,buffer,store_ptr,error);
				done = MB_YES;
				}
			else if (frame_id == MBSYS_XSE_GEO_FRAME)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ GEODETIC\n");
#endif
				store->kind = MB_DATA_RAW_LINE;
				status = mbr_l3xseraw_rd_geodetic(verbose,buffer_size,buffer,store_ptr,error);
				done = MB_YES;
				}
			else if (frame_id == MBSYS_XSE_SBM_FRAME)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ SEABEAM\n");
#endif
				status = mbr_l3xseraw_rd_seabeam(verbose,buffer_size,buffer,store_ptr,error);
				if (store->sbm_properties == MB_YES)
				    store->kind = MB_DATA_RUN_PARAMETER;
				else
				    store->kind = MB_DATA_RAW_LINE;
				done = MB_YES;
				}
			else if (frame_id == MBSYS_XSE_MSG_FRAME)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ MESSAGE\n");
#endif
				store->kind = MB_DATA_RAW_LINE;
				status = mbr_l3xseraw_rd_message(verbose,buffer_size,buffer,store_ptr,error);
				done = MB_YES;
				}
			else if (frame_id == MBSYS_XSE_COM_FRAME)
			    {
#ifdef MB_DEBUG
fprintf(stderr, "READ COMMENT\n");
#endif
			    store->kind = MB_DATA_COMMENT;
			    status = mbr_l3xseraw_rd_comment(verbose,buffer_size,buffer,store_ptr,error);
			    done = MB_YES;
			    }
			else /* handle an unrecognized frame */
			    {
#ifdef MB_DEBUG
fprintf(stderr, "READ OTHER\n");
#endif
			    store->kind = MB_DATA_RAW_LINE;
			    }

			if(store->kind==MB_DATA_RAW_LINE)
				{
				store->rawsize = buffer_size;
				for (i=0;i<buffer_size;i++)
					store->raw[i] = buffer[i];
				done = MB_YES;
				}
			}
		else if (*frame_expect != MBSYS_XSE_NONE_FRAME
		    && frame_id != *frame_expect)
		    {
#ifdef MB_DEBUG
fprintf(stderr, "READ NOTHING - SAVE HEADER\n");
#endif
		    store->kind = MB_DATA_DATA;
		    *frame_save = MB_YES;
		    *frame_id_save = frame_id;
		    *frame_source_save = frame_source;
		    *frame_sec_save = frame_sec;
		    *frame_usec_save = frame_usec;
		    *buffer_size_save = buffer_size;
		    *frame_expect = MBSYS_XSE_NONE_FRAME;
		    done = MB_YES;
		    }
		    	
	    /* check for status */
	    if (status == MB_FAILURE)
			{
			done = MB_YES;
			*frame_save = MB_NO;
			}
	    }

	/* get file position */
	mb_io_ptr->file_bytes = ftell(mbfp);
	
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
int mbr_l3xseraw_rd_nav(int verbose,int buffer_size,char *buffer,void *store_ptr,int *error)
{
	char	*function_name = "mbr_l3xseraw_rd_nav";
	int	status = MB_SUCCESS;
	struct mbsys_xse_struct *store;
	int	byte_count;
	int	group_id;
	int	done;
	int	index;
	int	skip;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer_size:%d\n",buffer_size);
		fprintf(stderr,"dbg2       buffer:     %lu\n",(size_t)buffer);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get pointer to store data structure */
	store = (struct mbsys_xse_struct *) store_ptr;
		
	/* get source and time */
	index = 12;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->nav_source); index += 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->nav_sec); index += 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->nav_usec); index += 4;
	
	/* reset group read flags */
	store->nav_group_general = MB_NO;	/* boolean flag */
	store->nav_group_position = MB_NO;	/* boolean flag */
	store->nav_group_accuracy = MB_NO;	/* boolean flag */
	store->nav_group_motiongt = MB_NO;	/* boolean flag */
	store->nav_group_motiontw = MB_NO;	/* boolean flag */
	store->nav_group_track = MB_NO;		/* boolean flag */
	store->nav_group_hrp = MB_NO;		/* boolean flag */
	store->nav_group_heave = MB_NO;		/* boolean flag */
	store->nav_group_roll = MB_NO;		/* boolean flag */
	store->nav_group_pitch = MB_NO;		/* boolean flag */
	store->nav_group_heading = MB_NO;	/* boolean flag */
	store->nav_group_log = MB_NO;		/* boolean flag */
	store->nav_group_gps = MB_NO;		/* boolean flag */	
		
	/* loop over groups */
	done = MB_NO;
	while (index <= buffer_size 
		&& status == MB_SUCCESS
		&& done == MB_NO)
	    {
	    /* look for group start or frame end */
	    skip = 0;
#ifdef DATAINPCBYTEORDER
	    while (index < buffer_size
			&& strncmp(&buffer[index], "GSH$", 4)
			&& strncmp(&buffer[index], "FSH#", 4))
			{
			index++;
			skip++;
			}
	    if (index >= buffer_size
			|| !strncmp(&buffer[index], "FSH#", 4))
			done = MB_YES;
	    else
			index += 4;
#else
	    while (index < buffer_size
			&& strncmp(&buffer[index], "$HSG", 4)
			&& strncmp(&buffer[index], "#HSF", 4))
			{
			index++;
			skip++;
			}
	    if (index >= buffer_size
			|| !strncmp(&buffer[index], "#HSF", 4))
			done = MB_YES;
	    else
			index += 4;
#endif
	    
#ifdef MB_DEBUG
if (skip > 4) fprintf(stderr, "skipped %d bytes in function <%s>\n", skip-4, function_name);
#endif
	    
	    /* deal with group */
	    if (done == MB_NO)
			{
			/* get group size and id */
			mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &byte_count); index += 4;
			mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &group_id); index += 4;

			/* print debug statements */
			if (verbose >= 5)
			    {
			    fprintf(stderr,"\ndbg5  Group %d of %d bytes to be parsed in MBIO function <%s>\n",
				    group_id, byte_count, function_name);
			    }
#ifdef MB_DEBUG
fprintf(stderr, "Group %d of %d bytes to be parsed in MBIO function <%s>\n",
				    group_id, byte_count, function_name);
#endif
	    
		    /* handle general group */
		    if (group_id == MBSYS_XSE_NAV_GROUP_GEN)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ NAV_GROUP_GEN\n");
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->nav_quality); index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->nav_status); index += 4;
				store->nav_group_general = MB_YES;
				}
	    
		    /* handle point group */
		    else if (group_id == MBSYS_XSE_NAV_GROUP_POS)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ NAV_GROUP_POS\n");
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->nav_description_len); index += 4;
				for (i=0;i<store->nav_description_len;i++)
				    {
				    store->nav_description[i] = buffer[index];
				    index++;
				    }
				store->nav_description[store->nav_description_len] = '\0';
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_x); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_y); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_z); index += 8;
				store->nav_group_position = MB_YES;
				}
	    
		    /* handle accuracy group */
		    else if (group_id == MBSYS_XSE_NAV_GROUP_ACCURACY)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ NAV_GROUP_ACCURACY\n");
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->nav_acc_quality); index += 4;
				store->nav_acc_numsatellites = buffer[index]; index++;
				mb_get_binary_float(SWAPFLAG, &buffer[index], (float *) &store->nav_acc_horizdilution); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], (float *) &store->nav_acc_diffage); index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->nav_acc_diffref); index += 4;
				store->nav_group_accuracy = MB_YES;
				}
	    
		    /* handle motion ground truth group */
		    else if (group_id == MBSYS_XSE_NAV_GROUP_MOTIONGT)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ NAV_GROUP_MOTIONGT\n");
#endif
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_speed_ground); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_course_ground); index += 8;
				store->nav_group_motiongt = MB_YES;
				}
	    
		    /* handle motion through water group */
		    else if (group_id == MBSYS_XSE_NAV_GROUP_MOTIONTW)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ NAV_GROUP_MOTIONTW\n");
#endif
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_speed_water); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_course_water); index += 8;
				store->nav_group_motiontw = MB_YES;
				}

			/* handle current track steering properties group */
			else if (group_id == MBSYS_XSE_NAV_GROUP_TRACK)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ NAV_GROUP_TRACK\n");
#endif
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_trk_offset_track); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_trk_offset_sol); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_trk_offset_eol); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_trk_distance_sol); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_trk_azimuth_sol); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_trk_distance_eol); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_trk_azimuth_eol); index += 8;
				store->nav_group_track = MB_YES;
				}

			/* handle the heaverollpitch group */
			else if (group_id == MBSYS_XSE_NAV_GROUP_HRP)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ NAV_GROUP_HRP\n");
#endif
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_hrp_heave); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_hrp_roll); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_hrp_pitch); index += 8;
				store->nav_group_hrp = MB_YES;
				/* heave, roll, and pitch are best obtained from the multibeam frame */
				}
		    
			/* handle the heave group */
			else if (group_id == MBSYS_XSE_NAV_GROUP_HEAVE)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ NAV_GROUP_HEAVE\n");
#endif
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_hea_heave); index += 8;
				store->nav_group_heave = MB_YES;
				/* heave is obtained from the multibeam frame */
				}
		    
			/* handle the roll group */
			else if (group_id == MBSYS_XSE_NAV_GROUP_ROLL)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ NAV_GROUP_ROLL\n");
#endif
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_rol_roll); index += 8;
				store->nav_group_roll = MB_YES;
				/* roll is obtained from the multibeam frame */
				}
		    
			/* handle the pitch group */
			else if (group_id == MBSYS_XSE_NAV_GROUP_PITCH)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ NAV_GROUP_PITCH\n");
#endif
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_pit_pitch); index += 8;
				store->nav_group_pitch = MB_YES;
				/* pitch is obtained from the multibeam frame */
				}
		    
			/* handle the heading group */
			else if (group_id == MBSYS_XSE_NAV_GROUP_HEADING)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ NAV_GROUP_HEADING\n");
#endif
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_hdg_heading); index += 8;
				store->nav_group_heading = MB_YES;
				/* Heading Group value overrides the MTW Group course value */
				}
		    
			/* handle the log group */
			else if (group_id == MBSYS_XSE_NAV_GROUP_LOG)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ NAV_GROUP_LOG\n");
#endif
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->nav_log_speed); index += 8;
				store->nav_group_log = MB_YES;
				/* speed is obtained from the motion ground truth */
				/* and motion through water groups */
				}
		    
			/* handle the gps group */
			else if (group_id == MBSYS_XSE_NAV_GROUP_GPS)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ NAV_GROUP_LOG\n");
#endif
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->nav_gps_altitude); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->nav_gps_geoidalseparation); index += 4;
				store->nav_group_gps = MB_YES;
				}

			else
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ NAV_GROUP_OTHER\n");
#endif
				}
			}
	    }

	/* print debug statements */
	if (verbose >= 5)
	    {
	    fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
	    fprintf(stderr,"dbg5       nav_source:          %d\n",store->nav_source);
	    fprintf(stderr,"dbg5       nav_sec:             %u\n",store->nav_sec);
	    fprintf(stderr,"dbg5       nav_usec:            %u\n",store->nav_usec);
	    fprintf(stderr,"dbg5       nav_quality:         %d\n",store->nav_quality);
	    fprintf(stderr,"dbg5       nav_status:          %d\n",store->nav_status);
	    fprintf(stderr,"dbg5       nav_description_len: %d\n",store->nav_description_len);
	    fprintf(stderr,"dbg5       nav_description:     %s\n",store->nav_description);
	    fprintf(stderr,"dbg5       nav_x:               %f\n",store->nav_x);
	    fprintf(stderr,"dbg5       nav_y:               %f\n",store->nav_y);
	    fprintf(stderr,"dbg5       nav_z:               %f\n",store->nav_z);
	    fprintf(stderr,"dbg5       nav_speed_ground:    %f\n",store->nav_speed_ground);
	    fprintf(stderr,"dbg5       nav_course_ground:   %f\n",store->nav_course_ground);
	    fprintf(stderr,"dbg5       nav_speed_water:     %f\n",store->nav_speed_water);
	    fprintf(stderr,"dbg5       nav_course_water:    %f\n",store->nav_course_water);
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
int mbr_l3xseraw_rd_svp(int verbose,int buffer_size,char *buffer,void *store_ptr,int *error)
{
	char	*function_name = "mbr_l3xseraw_rd_svp";
	int	status = MB_SUCCESS;
	struct mbsys_xse_struct *store;
	int	byte_count;
	int	group_id;
	int	done;
	int	index;
	int	skip;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer_size:%d\n",buffer_size);
		fprintf(stderr,"dbg2       buffer:     %lu\n",(size_t)buffer);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get pointer to store data structure */
	store = (struct mbsys_xse_struct *) store_ptr;
		
	/* get source and time */
	index = 12;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->svp_source); index += 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->svp_sec); index += 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->svp_usec); index += 4;
		
	/* loop over groups */
	done = MB_NO;
	while (index <= buffer_size 
		&& status == MB_SUCCESS
		&& done == MB_NO)
	    {
	    /* look for group start or frame end */
	    skip = 0;
#ifdef DATAINPCBYTEORDER
	    while (index < buffer_size
			&& strncmp(&buffer[index], "GSH$", 4)
			&& strncmp(&buffer[index], "FSH#", 4))
			{
			index++;
			skip++;
			}
	    if (index >= buffer_size
			|| !strncmp(&buffer[index], "FSH#", 4))
			done = MB_YES;
	    else
			index += 4;
#else
	    while (index < buffer_size
			&& strncmp(&buffer[index], "$HSG", 4)
			&& strncmp(&buffer[index], "#HSF", 4))
			{
			index++;
			skip++;
			}
	    if (index >= buffer_size
			|| !strncmp(&buffer[index], "#HSF", 4))
			done = MB_YES;
	    else
			index += 4;
#endif
	    
#ifdef MB_DEBUG
if (skip > 4) fprintf(stderr, "skipped %d bytes in function <%s>\n", skip-4, function_name);
#endif
	    
	    /* deal with group */
	    if (done == MB_NO)
			{
			/* get group size and id */
			mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &byte_count); index += 4;
			mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &group_id); index += 4;

			/* print debug statements */
			if (verbose >= 5)
			    {
			    fprintf(stderr,"\ndbg5  Group %d of %d bytes to be parsed in MBIO function <%s>\n",
				    group_id, byte_count, function_name);
			    }
#ifdef MB_DEBUG
fprintf(stderr, "Group %d of %d bytes to be parsed in MBIO function <%s>\n",
				    group_id, byte_count, function_name);
#endif

			/* handle general group */
			if (group_id == MBSYS_XSE_SVP_GROUP_GEN)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ SVP_GROUP_GEN\n");
#endif
				/* currently unused by MB-System */
				}
	    
		    /* handle depth group */
		    if (group_id == MBSYS_XSE_SVP_GROUP_DEPTH) 
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ SVP_GROUP_DEPTH\n");
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->svp_nsvp); index += 4;
				for (i=0;i<store->svp_nsvp;i++)
				    {
				    if (i < MBSYS_XSE_MAXSVP)
						{
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->svp_depth[i]); index += 8;
						}
				    }
				}
	    
		    /* handle velocity group */
		    else if (group_id == MBSYS_XSE_SVP_GROUP_VELOCITY)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ SVP_GROUP_VELOCITY\n");
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->svp_nsvp); index += 4;
				for (i=0;i<store->svp_nsvp;i++)
				    {
				    if (i < MBSYS_XSE_MAXSVP)
						{
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->svp_velocity[i]); index += 8;
						}
				    }
				}
	    
		    /* handle conductivity group */
		    else if (group_id == MBSYS_XSE_SVP_GROUP_CONDUCTIVITY)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ SVP_GROUP_CONDUCTIVITY\n");
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->svp_nctd); index += 4;
				for (i=0;i<store->svp_nctd;i++)
				    {
				    if (i < MBSYS_XSE_MAXSVP)
						{
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->svp_conductivity[i]); index += 8;
						}
				    }
				}
	    
		    /* handle salinity group */
		    else if (group_id == MBSYS_XSE_SVP_GROUP_SALINITY)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ SVP_GROUP_SALINITY\n");
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->svp_nctd); index += 4;
				for (i=0;i<store->svp_nctd;i++)
				    {
				    if (i < MBSYS_XSE_MAXSVP)
						{
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->svp_salinity[i]); index += 8;
						}
				    }
				}
	    
		    /* handle temperature group */
		    else if (group_id == MBSYS_XSE_SVP_GROUP_TEMP)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ SVP_GROUP_TEMP\n");
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->svp_nctd); index += 4;
				for (i=0;i<store->svp_nctd;i++)
				    {
				    if (i < MBSYS_XSE_MAXSVP)
						{
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->svp_temperature[i]); index += 8;
						}
				    }
				}
	    
		    /* handle pressure group */
		    else if (group_id == MBSYS_XSE_SVP_GROUP_PRESSURE)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ SVP_GROUP_PRESSURE\n");
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->svp_nctd); index += 4;
				for (i=0;i<store->svp_nctd;i++)
				    {
				    if (i < MBSYS_XSE_MAXSVP)
						{
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->svp_pressure[i]); index += 8;
						}
				    }
				}
	    
		    /* handle ssv group */
		    else if (group_id == MBSYS_XSE_SVP_GROUP_SSV)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ SVP_GROUP_SSV\n");
#endif
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->svp_ssv); index += 8;
				}

			/* handle point group */
			else if (group_id == MBSYS_XSE_SVP_GROUP_POS)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ SVP_GROUP_POS\n");
#endif
				/* currently unused by MB-System */
				}

			else
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ SVP_GROUP_OTHER\n");
#endif
				}
			}
	    }

	/* print debug statements */
	if (verbose >= 5)
	    {
	    fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
	    fprintf(stderr,"dbg5       svp_source:          %d\n",store->svp_source);
	    fprintf(stderr,"dbg5       svp_sec:             %u\n",store->svp_sec);
	    fprintf(stderr,"dbg5       svp_usec:            %u\n",store->svp_usec);
	    fprintf(stderr,"dbg5       svp_nsvp:            %d\n",store->svp_nsvp);
	    fprintf(stderr,"dbg5       svp_nctd:            %d\n",store->svp_nctd);
	    fprintf(stderr,"dbg5       svp_ssv:             %f\n",store->svp_ssv);
	    for (i=0;i<store->svp_nsvp;i++)
		fprintf(stderr,"dbg5       svp[%d]:	        %f %f\n",
		    i, store->svp_depth[i], store->svp_velocity[i]);
	    for (i=0;i<store->svp_nctd;i++)
		fprintf(stderr,"dbg5       cstd[%d]:        %f %f %f %f\n",
		    i, store->svp_conductivity[i], store->svp_salinity[i], 
		    store->svp_temperature[i], store->svp_pressure[i]);
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
int mbr_l3xseraw_rd_tide(int verbose,int buffer_size,char *buffer,void *store_ptr,int *error)
{
	char    *function_name = "mbr_l3xseraw_rd_tide";
	int status = MB_SUCCESS;
	struct mbsys_xse_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer_size:%d\n",buffer_size);
		fprintf(stderr,"dbg2       buffer:     %lu\n",(size_t)buffer);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get pointer to store data structure */
	store = (struct mbsys_xse_struct *) store_ptr;

	/* The tide frame is currently unused by MB-System */

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
int mbr_l3xseraw_rd_ship(int verbose,int buffer_size,char *buffer,void *store_ptr,int *error)
{
	char	*function_name = "mbr_l3xseraw_rd_ship";
	int	status = MB_SUCCESS;
	struct mbsys_xse_struct *store;
	int	byte_count;
	int	group_id;
	int	done;
	int	index;
	int	skip;
	int	nchar;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer_size:%d\n",buffer_size);
		fprintf(stderr,"dbg2       buffer:     %lu\n",(size_t)buffer);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get pointer to store data structure */
	store = (struct mbsys_xse_struct *) store_ptr;
		
	/* get source and time */
	index = 12;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->par_source); index += 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->par_sec); index += 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->par_usec); index += 4;
		
	/* loop over groups */
	done = MB_NO;
	while (index <= buffer_size 
		&& status == MB_SUCCESS
		&& done == MB_NO)
	    {
	    /* look for group start or frame end */
	    skip = 0;
#ifdef DATAINPCBYTEORDER
	    while (index < buffer_size
			&& strncmp(&buffer[index], "GSH$", 4)
			&& strncmp(&buffer[index], "FSH#", 4))
			{
			index++;
			skip++;
			}
	    if (index >= buffer_size
			|| !strncmp(&buffer[index], "FSH#", 4))
			done = MB_YES;
	    else
			index += 4;
#else
	    while (index < buffer_size
			&& strncmp(&buffer[index], "$HSG", 4)
			&& strncmp(&buffer[index], "#HSF", 4))
			{
			index++;
			skip++;
			}
	    if (index >= buffer_size
			|| !strncmp(&buffer[index], "#HSF", 4))
			done = MB_YES;
	    else
			index += 4;
#endif
	    
#ifdef MB_DEBUG
if (skip > 4) fprintf(stderr, "skipped %d bytes in function <%s>\n", skip-4, function_name);
#endif
	    
	    /* deal with group */
	    if (done == MB_NO)
			{
			/* get group size and id */
			mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &byte_count); index += 4;
			mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &group_id); index += 4;

			/* print debug statements */
			if (verbose >= 5)
			    {
			    fprintf(stderr,"\ndbg5  Group %d of %d bytes to be parsed in MBIO function <%s>\n",
				    group_id, byte_count, function_name);
			    }
#ifdef MB_DEBUG
fprintf(stderr, "Group %d of %d bytes to be parsed in MBIO function <%s>\n",
				    group_id, byte_count, function_name);
#endif

			/* handle general group */
			if (group_id == MBSYS_XSE_SHP_GROUP_GEN)
				{
				mb_get_binary_int(SWAPFLAG, &buffer[index], &nchar); index += 4;
				for (i=0;i<nchar;i++)
					{
					store->par_ship_name[i] = buffer[index]; index++;
					}
				store->par_ship_name[nchar] = 0;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_ship_length); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_ship_beam); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_ship_draft); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_ship_height); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_ship_displacement); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_ship_weight); index += 8;
				}
		
			/* handle time group */
			else if (group_id == MBSYS_XSE_SHP_GROUP_TIME)
				{
				/* currently unused by MB-System */
				}

			/* handle draft group */
			else if (group_id == MBSYS_XSE_SHP_GROUP_DRAFT)
				{
				/* currently unused by MB-System */
				}

			/* handle sensors group */
			else if (group_id == MBSYS_XSE_SHP_GROUP_SENSORS)
				{
				mb_get_binary_int(SWAPFLAG, &buffer[index], &store->par_ship_nsensor); index += 4;
				for (i=0;i<store->par_ship_nsensor;i++)
					{
					mb_get_binary_int(SWAPFLAG, &buffer[index], &store->par_ship_sensor_id[i]); index += 4;
					}
				for (i=0;i<store->par_ship_nsensor;i++)
					{
					mb_get_binary_int(SWAPFLAG, &buffer[index], &store->par_ship_sensor_type[i]); index += 4;
					}
				for (i=0;i<store->par_ship_nsensor;i++)
					{
					mb_get_binary_int(SWAPFLAG, &buffer[index], &store->par_ship_sensor_frequency[i]); index += 4;
					}
				}

			/* handle motion group */
			else if (group_id == MBSYS_XSE_SHP_GROUP_MOTION)
				{
				/* currently unused by MB-System */
				}

			/* handle geometry group */
			else if (group_id == MBSYS_XSE_SHP_GROUP_GEOMETRY)
				{
				/* currently unused by MB-System */
				}
	
			/* handle description group */
			else if (group_id == MBSYS_XSE_SHP_GROUP_DESCRIPTION)
				{
				/* currently unused by MB-System */
				}
	    
		    /* handle parameter group */
		    else if (group_id == MBSYS_XSE_SHP_GROUP_PARAMETER)
				{
				store->par_parameter = MB_YES;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_roll_bias); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_pitch_bias); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_heading_bias); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_time_delay); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_trans_x_port); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_trans_y_port); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_trans_z_port); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_trans_x_stbd); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_trans_y_stbd); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_trans_z_stbd); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_trans_err_port); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_trans_err_stbd); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_nav_x); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_nav_y); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_nav_z); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_hrp_x); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_hrp_y); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->par_hrp_z); index += 4;
				}
	    
		    /* handle navigation and motion group */
		    else if (group_id == MBSYS_XSE_SHP_GROUP_NAVIGATIONANDMOTION)
				{
				store->par_navigationandmotion = MB_YES;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_nam_roll_bias); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_nam_pitch_bias); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_nam_heave_bias); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_nam_heading_bias); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_nam_time_delay); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_nam_nav_x); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_nam_nav_y); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_nam_nav_z); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_nam_hrp_x); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_nam_hrp_y); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_nam_hrp_z); index += 8;
				}
	    
		    /* handle transducer group */
		    else if (group_id == MBSYS_XSE_SHP_GROUP_TRANSDUCER)
				{
				mb_get_binary_int(SWAPFLAG, &buffer[index], &store->par_xdr_num_transducer); index += 4;
				for (i=0;i<store->par_xdr_num_transducer;i++)
					{
					mb_get_binary_int(SWAPFLAG, &buffer[index], &store->par_xdr_sensorid[i]); index += 4;
					mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->par_xdr_frequency[i]); index += 4;
					store->par_xdr_transducer[i] = buffer[index]; index++;
					store->par_xdr_side[i] = buffer[index]; index++;
					mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_xdr_mountingroll[i]); index += 8;
					mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_xdr_mountingpitch[i]); index += 8;
					mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_xdr_mountingazimuth[i]); index += 8;
					mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_xdr_mountingdistance[i]); index += 8;
					mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_xdr_x[i]); index += 8;
					mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_xdr_y[i]); index += 8;
					mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_xdr_z[i]); index += 8;
					mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_xdr_roll[i]); index += 8;
					mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_xdr_pitch[i]); index += 8;
					mb_get_binary_double(SWAPFLAG, &buffer[index], &store->par_xdr_azimuth[i]); index += 8;
					}
				}
	    
		    /* handle tranducer extended group */
		    else if (group_id == MBSYS_XSE_SHP_GROUP_TRANSDUCEREXTENDED)
				{
				mb_get_binary_int(SWAPFLAG, &buffer[index], &store->par_xdx_num_transducer); index += 4;
				for (i=0;i<store->par_xdx_num_transducer;i++)
					{
					store->par_xdx_roll[i] = buffer[index]; index++;
					store->par_xdx_pitch[i] = buffer[index]; index++;
					store->par_xdx_azimuth[i] = buffer[index]; index++;
					index += 48;
					}
				}
			}
	    }

	/* print debug statements */
	if (verbose >= 5)
	    {
	    fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
	    fprintf(stderr,"dbg5       par_source:          %d\n",store->par_source);
	    fprintf(stderr,"dbg5       par_sec:             %u\n",store->par_sec);
	    fprintf(stderr,"dbg5       par_usec:            %u\n",store->par_usec);
	    fprintf(stderr,"dbg5       par_ship_name:       %s\n",store->par_ship_name);
	    fprintf(stderr,"dbg5       par_ship_length:     %f\n",store->par_ship_length);
	    fprintf(stderr,"dbg5       par_ship_beam:       %f\n",store->par_ship_beam);
	    fprintf(stderr,"dbg5       par_ship_draft:      %f\n",store->par_ship_draft);
	    fprintf(stderr,"dbg5       par_ship_height:     %f\n",store->par_ship_height);
	    fprintf(stderr,"dbg5       par_ship_displacement: %f\n",store->par_ship_displacement);
	    fprintf(stderr,"dbg5       par_ship_weight:     %f\n",store->par_ship_weight);
	    for (i=0;i<store->par_ship_nsensor;i++)
		{
		fprintf(stderr,"dbg5       par_ship_sensor_id[%d]:        %d\n",i,store->par_ship_sensor_id[i]);
		fprintf(stderr,"dbg5       par_ship_sensor_type[%d]:      %d\n",i,store->par_ship_sensor_type[i]);
		fprintf(stderr,"dbg5       par_ship_sensor_frequency[%d]: %d\n",i,store->par_ship_sensor_frequency[i]);
		}
	    fprintf(stderr,"dbg5       par_parameter:       %d\n",store->par_parameter);
	    fprintf(stderr,"dbg5       par_roll_bias:       %f\n",store->par_roll_bias);
	    fprintf(stderr,"dbg5       par_pitch_bias:      %f\n",store->par_pitch_bias);
	    fprintf(stderr,"dbg5       par_heading_bias:    %f\n",store->par_heading_bias);
	    fprintf(stderr,"dbg5       par_time_delay:      %f\n",store->par_time_delay);
	    fprintf(stderr,"dbg5       par_trans_x_port:    %f\n",store->par_trans_x_port);
	    fprintf(stderr,"dbg5       par_trans_y_port:    %f\n",store->par_trans_y_port);
	    fprintf(stderr,"dbg5       par_trans_z_port:    %f\n",store->par_trans_z_port);
	    fprintf(stderr,"dbg5       par_trans_x_stbd:    %f\n",store->par_trans_x_stbd);
	    fprintf(stderr,"dbg5       par_trans_y_stbd:    %f\n",store->par_trans_y_stbd);
	    fprintf(stderr,"dbg5       par_trans_z_stbd:    %f\n",store->par_trans_z_stbd);
	    fprintf(stderr,"dbg5       par_trans_err_port:  %f\n",store->par_trans_err_port);
	    fprintf(stderr,"dbg5       par_trans_err_stbd:  %f\n",store->par_trans_err_stbd);
	    fprintf(stderr,"dbg5       par_nav_x:           %f\n",store->par_nav_x);
	    fprintf(stderr,"dbg5       par_nav_y:           %f\n",store->par_nav_y);
	    fprintf(stderr,"dbg5       par_nav_z:           %f\n",store->par_nav_z);
	    fprintf(stderr,"dbg5       par_hrp_x:           %f\n",store->par_hrp_x);
	    fprintf(stderr,"dbg5       par_hrp_y:           %f\n",store->par_hrp_y);
	    fprintf(stderr,"dbg5       par_hrp_z:           %f\n",store->par_hrp_z);
	    fprintf(stderr,"dbg5       par_navigationandmotion: %d\n",store->par_navigationandmotion);
	    fprintf(stderr,"dbg5       par_nam_roll_bias:       %f\n",store->par_nam_roll_bias);
	    fprintf(stderr,"dbg5       par_nam_pitch_bias:      %f\n",store->par_nam_pitch_bias);
	    fprintf(stderr,"dbg5       par_nam_heave_bias:      %f\n",store->par_nam_heave_bias);
	    fprintf(stderr,"dbg5       par_nam_heading_bias:    %f\n",store->par_nam_heading_bias);
	    fprintf(stderr,"dbg5       par_nam_time_delay:      %f\n",store->par_nam_time_delay);
	    fprintf(stderr,"dbg5       par_nam_nav_x:           %f\n",store->par_nam_nav_x);
	    fprintf(stderr,"dbg5       par_nam_nav_y:           %f\n",store->par_nam_nav_y);
	    fprintf(stderr,"dbg5       par_nam_nav_z:           %f\n",store->par_nam_nav_z);
	    fprintf(stderr,"dbg5       par_nam_hrp_x:           %f\n",store->par_nam_hrp_x);
	    fprintf(stderr,"dbg5       par_nam_hrp_y:           %f\n",store->par_nam_hrp_y);
	    fprintf(stderr,"dbg5       par_nam_hrp_z:           %f\n",store->par_nam_hrp_z);
	    fprintf(stderr,"dbg5       par_xdr_num_transducer:  %d\n",store->par_xdr_num_transducer);
	    fprintf(stderr,"dbg5       # sensor xducer freq side roll pitch azi dist\n");
	    for (i=0;i<store->par_xdr_num_transducer;i++)
	    	fprintf(stderr,"dbg5       %d %d %d %d %d %f %f %f %f\n", i, store->par_xdr_sensorid[i], store->par_xdr_transducer[i], 
	    							store->par_xdr_frequency[i], store->par_xdr_side[i], 
								store->par_xdr_mountingroll[i], store->par_xdr_mountingpitch[i], 
								store->par_xdr_mountingazimuth[i], store->par_xdr_mountingdistance[i]);
	    fprintf(stderr,"dbg5       # x y z roll pitch azimuth\n");
	    for (i=0;i<store->par_xdr_num_transducer;i++)
	    	fprintf(stderr,"dbg5       %d %f %f %f %f %f %f\n", i, store->par_xdr_x[i], store->par_xdr_y[i], 
	    							store->par_xdr_z[i], store->par_xdr_roll[i], 
								store->par_xdr_pitch[i], store->par_xdr_azimuth[i]);
	    fprintf(stderr,"dbg5       par_xdx_num_transducer:  %d\n",store->par_xdx_num_transducer);
	    fprintf(stderr,"dbg5       # roll pitch azimuth\n");
	    for (i=0;i<store->par_xdx_num_transducer;i++)
	    	fprintf(stderr,"dbg5       %d %d %d %d\n", i, store->par_xdx_roll[i], store->par_xdx_pitch[i], store->par_xdx_azimuth[i]);
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
int mbr_l3xseraw_rd_sidescan(int verbose,int buffer_size,char *buffer,void *store_ptr,int *error)
{
	char	*function_name = "mbr_l3xseraw_rd_sidescan";
	int	status = MB_SUCCESS;
	struct mbsys_xse_struct *store;
	int	byte_count;
	int	group_id;
	int	done;
	int	index;
	int	ngoodss;
	double	xmin, xmax, binsize;
	int	skip;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer_size:%d\n",buffer_size);
		fprintf(stderr,"dbg2       buffer:     %lu\n",(size_t)buffer);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get pointer to store data structure */
	store = (struct mbsys_xse_struct *) store_ptr;

	/* get source and time */
	index = 12;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sid_source); index += 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sid_sec); index += 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sid_usec); index += 4;
		
	/* loop over groups */
	done = MB_NO;
	while (index <= buffer_size 
		&& status == MB_SUCCESS
		&& done == MB_NO)
	    {
	    /* look for group start or frame end */
	    skip = 0;
#ifdef DATAINPCBYTEORDER
	    while (index < buffer_size
			&& strncmp(&buffer[index], "GSH$", 4)
			&& strncmp(&buffer[index], "FSH#", 4))
			{
			index++;
			skip++;
			}
	    if (index >= buffer_size
			|| !strncmp(&buffer[index], "FSH#", 4))
			done = MB_YES;
	    else
			index += 4;
#else
	    while (index < buffer_size
			&& strncmp(&buffer[index], "$HSG", 4)
			&& strncmp(&buffer[index], "#HSF", 4))
			{
			index++;
			skip++;
			}
	    if (index >= buffer_size
			|| !strncmp(&buffer[index], "#HSF", 4))
			done = MB_YES;
	    else
			index += 4;
#endif
	    
#ifdef MB_DEBUG
if (skip > 4) fprintf(stderr, "skipped %d bytes in function <%s>\n", skip-4, function_name);
#endif
	    
	    /* deal with group */
	    if (done == MB_NO)
			{
			/* get group size and id */
			mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &byte_count); index += 4;
			mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &group_id); index += 4;

			/* print debug statements */
			if (verbose >= 5)
			    {
			    fprintf(stderr,"\ndbg5  Group %d of %d bytes to be parsed in MBIO function <%s>\n",
				    group_id, byte_count, function_name);
			    }
#ifdef MB_DEBUG
fprintf(stderr, "Group %d of %d bytes to be parsed in MBIO function <%s>\n",
				    group_id, byte_count, function_name);
#endif
	    
		    /* handle general group */
		    if (group_id == MBSYS_XSE_SSN_GROUP_GEN)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ SSN_GROUP_GEN\n");
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sid_ping); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->sid_frequency); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->sid_pulse); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->sid_power); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->sid_bandwidth); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->sid_sample); index += 4;
#ifdef MB_DEBUG2
fprintf(stderr, "ping=%u\n", store->sid_ping);
fprintf(stderr, "frequency=%g\n", store->sid_frequency);
fprintf(stderr, "pulse=%g\n", store->sid_pulse);
fprintf(stderr, "power=%g\n", store->sid_power);
fprintf(stderr, "bandwidth=%g\n", store->sid_bandwidth);
fprintf(stderr, "sample=%g\n", store->sid_sample);
#endif
				}
	
			/* handle amplitude vs traveltime group */
			else if (group_id == MBSYS_XSE_SSN_GROUP_AMPVSTT)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ SSN_GROUP_AMPVSTT\n");
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sid_avt_sampleus); index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], &store->sid_avt_offset); index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], &store->sid_avt_num_samples); index += 4;
				for (i=0;i<store->sid_avt_num_samples;i++)
				    if (i < MBSYS_XSE_MAXPIXELS)
						{
						mb_get_binary_short(SWAPFLAG, &buffer[index], &store->sid_avt_amp[i]); index += 2;
						}
				store->sid_group_avt = MB_YES;
#ifdef MB_DEBUG2
fprintf(stderr, "sid_avt_sampleus=%d\n", store->sid_avt_sampleus);
fprintf(stderr, "sid_avt_offset=%d\n", store->sid_avt_offset);
fprintf(stderr, "sid_avt_num_samples=%d\n", store->sid_avt_num_samples);
for (i=0;i<store->sid_avt_num_samples;i++)
	fprintf(stderr, "sid_avt_amp[%d]:%d\n", i, store->sid_avt_amp[i]);
#endif
				}

			/* handle phase vs traveltime group */  
			else if (group_id == MBSYS_XSE_SSN_GROUP_PHASEVSTT)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ SSN_GROUP_PHASEVSTT\n");
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sid_pvt_sampleus); index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], &store->sid_pvt_offset); index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], &store->sid_pvt_num_samples); index += 4;
				for (i=0;i<store->sid_pvt_num_samples;i++)
				    if (i < MBSYS_XSE_MAXPIXELS)
						{
						mb_get_binary_short(SWAPFLAG, &buffer[index], &store->sid_pvt_phase[i]); index += 2;
						}
				store->sid_group_pvt = MB_YES;
#ifdef MB_DEBUG2
fprintf(stderr, "sid_pvt_sampleus=%d\n", store->sid_pvt_sampleus);
fprintf(stderr, "sid_pvt_offset=%d\n", store->sid_pvt_offset);
fprintf(stderr, "sid_pvt_num_samples=%d\n", store->sid_pvt_num_samples);
for (i=0;i<store->sid_pvt_num_samples;i++)
	fprintf(stderr, "sid_pvt_phase[%d]:%d\n", i, store->sid_pvt_phase[i]);
#endif
				}
	    
		    /* handle amplitude vs lateral group */
		    else if (group_id == MBSYS_XSE_SSN_GROUP_AMPVSLAT)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ SSN_GROUP_AMPVSLAT\n");
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sid_avl_binsize); index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sid_avl_offset); index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sid_avl_num_samples); index += 4;
				for (i=0;i<store->sid_avl_num_samples;i++)
				    if (i < MBSYS_XSE_MAXPIXELS)
						{
						mb_get_binary_short(SWAPFLAG, &buffer[index], &store->sid_avl_amp[i]); index += 2;
						}
				store->sid_group_avl = MB_YES;
#ifdef MB_DEBUG2
fprintf(stderr, "sid_avl_binsize=%d\n", store->sid_avl_binsize);
fprintf(stderr, "sid_avl_offset=%d\n", store->sid_avl_offset);
fprintf(stderr, "sid_avl_num_samples=%d\n", store->sid_avl_num_samples);
for (i=0;i<store->sid_avl_num_samples;i++)
	fprintf(stderr, "sid_avl_amp[%d]:%d\n", i, store->sid_avl_amp[i]);
#endif
				}

			else if (group_id == MBSYS_XSE_SSN_GROUP_PHASEVSLAT)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ SSN_GROUP_PHASEVSLAT\n");
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sid_pvl_binsize); index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sid_pvl_offset); index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sid_pvl_num_samples); index += 4;
				for (i=0;i<store->sid_pvl_num_samples;i++)
				    if (i < MBSYS_XSE_MAXPIXELS)
						{
						mb_get_binary_short(SWAPFLAG, &buffer[index], &store->sid_pvl_phase[i]); index += 2;
						}
				store->sid_group_pvl = MB_YES;
#ifdef MB_DEBUG2
fprintf(stderr, "sid_pvl_binsize=%d\n", store->sid_pvl_binsize);
fprintf(stderr, "sid_pvl_offset=%d\n", store->sid_pvl_offset);
fprintf(stderr, "sid_pvl_num_samples=%d\n", store->sid_pvl_num_samples);
for (i=0;i<store->sid_pvl_num_samples;i++)
	fprintf(stderr, "sid_pvl_phase[%d]:%d\n", i, store->sid_pvl_phase[i]);
#endif
				}

			else if (group_id == MBSYS_XSE_SSN_GROUP_SIGNAL)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ MBSYS_XSE_SSN_GROUP_SIGNAL\n");
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sid_sig_ping); index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sid_sig_channel); index += 4;
				mb_get_binary_double(SWAPFLAG, &buffer[index], (double *) &store->sid_sig_offset); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], (double *) &store->sid_sig_sample); index += 8;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sid_sig_num_samples); index += 4;
				for (i=0;i<store->sid_sig_num_samples;i++)
				    if (i < MBSYS_XSE_MAXPIXELS)
						{
						mb_get_binary_short(SWAPFLAG, &buffer[index], &store->sid_sig_phase[i]); index += 2;
						}
				store->sid_group_signal = MB_YES;
#ifdef MB_DEBUG2
fprintf(stderr, "sid_sig_ping=%d\n", store->sid_sig_ping);
fprintf(stderr, "sid_sig_channel=%d\n", store->sid_sig_channel);
fprintf(stderr, "sid_sig_offset=%g\n", store->sid_sig_offset);
fprintf(stderr, "sid_sig_sample=%g\n", store->sid_sig_sample);
fprintf(stderr, "sid_sig_num_samples=%d\n", store->sid_sig_num_samples);
for (i=0;i<store->sid_sig_num_samples;i++)
	fprintf(stderr, "sid_sig_phase[%d]:%d\n", i, store->sid_sig_phase[i]);
#endif
				}

			else if (group_id == MBSYS_XSE_SSN_GROUP_PINGTYPE)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ MBSYS_XSE_SSN_GROUP_PINGTYPE\n");
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sid_png_pulse); index += 4;
				mb_get_binary_double(SWAPFLAG, &buffer[index], (double *) &store->sid_png_startfrequency); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], (double *) &store->sid_png_endfrequency); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], (double *) &store->sid_png_duration); index += 8;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sid_png_mancode); index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sid_png_pulseid); index += 4;
				for (i=0;i<byte_count-40;i++)
					{
					store->sid_png_pulsename[i] = buffer[index]; index++;
					}
				store->sid_group_ping = MB_YES;
#ifdef MB_DEBUG2
fprintf(stderr, "sid_png_pulse=%d\n", store->sid_png_pulse);
fprintf(stderr, "sid_png_startfrequency=%f\n", store->sid_png_startfrequency);
fprintf(stderr, "sid_png_endfrequency=%f\n", store->sid_png_endfrequency);
fprintf(stderr, "sid_png_duration=%f\n", store->sid_png_duration);
fprintf(stderr, "sid_png_mancode=%d\n", store->sid_png_mancode);
fprintf(stderr, "sid_png_pulseid=%d\n", store->sid_png_pulseid);
fprintf(stderr, "sid_png_pulsename=%s\n", store->sid_png_pulsename);
#endif
				}

			else if (group_id == MBSYS_XSE_SSN_GROUP_COMPLEXSIGNAL)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ MBSYS_XSE_SSN_GROUP_COMPLEXSIGNAL\n");
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sid_cmp_ping); index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sid_cmp_channel); index += 4;
				mb_get_binary_double(SWAPFLAG, &buffer[index], (double *) &store->sid_cmp_offset); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], (double *) &store->sid_cmp_sample); index += 8;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sid_cmp_num_samples); index += 4;
				for (i=0;i<store->sid_cmp_num_samples;i++)
				    if (i < MBSYS_XSE_MAXPIXELS)
						{
						mb_get_binary_short(SWAPFLAG, &buffer[index], &store->sid_cmp_real[i]); index += 2;
						mb_get_binary_short(SWAPFLAG, &buffer[index], &store->sid_cmp_imaginary[i]); index += 2;
						}
				store->sid_group_complex = MB_YES;
#ifdef MB_DEBUG2
fprintf(stderr, "sid_cmp_ping=%d\n", store->sid_cmp_ping);
fprintf(stderr, "sid_cmp_channel=%d\n", store->sid_cmp_channel);
fprintf(stderr, "sid_cmp_offset=%f\n", store->sid_cmp_offset);
fprintf(stderr, "sid_cmp_sample=%f\n", store->sid_cmp_sample);
fprintf(stderr, "sid_cmp_num_samples=%d\n", store->sid_cmp_num_samples);
for (i=0;i<store->sid_cmp_num_samples;i++)
	fprintf(stderr, "sid_cmp_real[%d]:%d sid_cmp_imaginary[%d]:%d\n", i, store->sid_cmp_real[i], i, store->sid_cmp_imaginary[i]);
#endif
				}

			else if (group_id == MBSYS_XSE_SSN_GROUP_WEIGHTING)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ MBSYS_XSE_SSN_GROUP_WEIGHTING\n");
#endif
				mb_get_binary_short(SWAPFLAG, &buffer[index], (short *) &store->sid_wgt_factorleft); index += 2;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sid_wgt_samplesleft); index += 4;
				mb_get_binary_short(SWAPFLAG, &buffer[index], (short *) &store->sid_wgt_factorright); index += 2;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sid_wgt_samplesright); index += 4;
				store->sid_group_weighting = MB_YES;
#ifdef MB_DEBUG2
fprintf(stderr, "sid_wgt_factorleft=%d\n", store->sid_wgt_factorleft);
fprintf(stderr, "sid_wgt_samplesleft=%d\n", store->sid_wgt_samplesleft);
fprintf(stderr, "sid_wgt_factorright=%d\n", store->sid_wgt_factorright);
fprintf(stderr, "sid_wgt_samplesright=%d\n", store->sid_wgt_samplesright);
#endif
				}

			else
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ SSN_GROUP_OTHER\n");
#endif
				}
			}
	    }
	    
	/* now if multibeam already read but bin size lacking then
	   calculate bin size from bathymetry */
	if (store->mul_frame == MB_YES
	    && store->mul_num_beams > 1
	    && store->sid_avl_num_samples > 1
	    && store->sid_avl_binsize <= 0)
	    {
	    /* get width of bathymetry swath size */
	    xmin = 9999999.9;
	    xmax = -9999999.9;
	    for (i=0;i<store->mul_num_beams;i++)
			{
			xmin = MIN(xmin, store->beams[i].lateral);
			xmax = MAX(xmax, store->beams[i].lateral);
			}
	    
	    /* get number of nonzero pixels */
	    ngoodss = 0;
	    for (i=0;i<store->sid_avl_num_samples;i++)
			if (store->sid_avl_amp[i] != 0) ngoodss++;
		
	    /* get bin size */
	    if (xmax > xmin && ngoodss > 1)
			{
			binsize = (xmax - xmin) / (ngoodss - 1);
			store->sid_avl_binsize = 1000 * binsize;
			}
	    }

	/* print debug statements */
	if (verbose >= 5)
	    {
	    fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
	    fprintf(stderr,"dbg5       sid_frame:            %d\n",store->sid_frame);
	    fprintf(stderr,"dbg5       sid_group_avt:        %d\n",store->sid_group_avt);
	    fprintf(stderr,"dbg5       sid_group_pvt:        %d\n",store->sid_group_pvt);
	    fprintf(stderr,"dbg5       sid_group_avl:        %d\n",store->sid_group_avl);
	    fprintf(stderr,"dbg5       sid_group_pvl:        %d\n",store->sid_group_pvl);
	    fprintf(stderr,"dbg5       sid_group_signal:     %d\n",store->sid_group_signal);
	    fprintf(stderr,"dbg5       sid_group_ping:       %d\n",store->sid_group_ping);
	    fprintf(stderr,"dbg5       sid_group_complex:    %d\n",store->sid_group_complex);
	    fprintf(stderr,"dbg5       sid_group_weighting:  %d\n",store->sid_group_weighting);
	    fprintf(stderr,"dbg5       sid_source:           %d\n",store->sid_source);
	    fprintf(stderr,"dbg5       sid_sec:              %d\n",store->sid_sec);
	    fprintf(stderr,"dbg5       sid_usec:             %u\n",store->sid_usec);
	    fprintf(stderr,"dbg5       sid_ping:             %u\n",store->sid_ping);
	    fprintf(stderr,"dbg5       sid_frequency:        %f\n",store->sid_frequency);
	    fprintf(stderr,"dbg5       sid_pulse:            %f\n",store->sid_pulse);
	    fprintf(stderr,"dbg5       sid_power:            %f\n",store->sid_power);
	    fprintf(stderr,"dbg5       sid_bandwidth:        %f\n",store->sid_bandwidth);
	    fprintf(stderr,"dbg5       sid_sample:           %f\n",store->sid_sample);
	    fprintf(stderr,"dbg5       sid_avt_sampleus:     %d\n",store->sid_avt_sampleus);
	    fprintf(stderr,"dbg5       sid_avt_offset:       %d\n",store->sid_avt_offset);
	    fprintf(stderr,"dbg5       sid_avt_num_samples:  %d\n",store->sid_avt_num_samples);
	    for (i=0;i<store->sid_avt_num_samples;i++)
	    	fprintf(stderr,"dbg5       sid_avt_amp[%d]:%d\n",i,store->sid_avt_amp[i]);
	    fprintf(stderr,"dbg5       sid_pvt_sampleus:  %d\n",store->sid_pvt_sampleus);
	    fprintf(stderr,"dbg5       sid_pvt_offset:  %d\n",store->sid_pvt_offset);
	    fprintf(stderr,"dbg5       sid_pvt_num_samples:  %d\n",store->sid_pvt_num_samples);
	    for (i=0;i<store->sid_pvt_num_samples;i++)
	    	fprintf(stderr,"dbg5       sid_pvt_phase[%d]:%d\n",i,store->sid_pvt_phase[i]);
	    fprintf(stderr,"dbg5       sid_avl_binsize:  %d\n",store->sid_avl_binsize);
	    fprintf(stderr,"dbg5       sid_avl_offset:  %d\n",store->sid_avl_offset);
	    fprintf(stderr,"dbg5       sid_avl_num_samples:  %d\n",store->sid_avl_num_samples);
	    for (i=0;i<store->sid_avl_num_samples;i++)
	    	fprintf(stderr,"dbg5       sid_avl_amp[%d]:%d\n",i,store->sid_avl_amp[i]);
	    fprintf(stderr,"dbg5       sid_pvl_binsize:  %d\n",store->sid_pvl_binsize);
	    fprintf(stderr,"dbg5       sid_pvl_offset:  %d\n",store->sid_pvl_offset);
	    fprintf(stderr,"dbg5       sid_pvl_num_samples:  %d\n",store->sid_pvl_num_samples);
	    for (i=0;i<store->sid_pvl_num_samples;i++)
	    	fprintf(stderr,"dbg5       sid_pvl_phase[%d]:%d\n",i,store->sid_pvl_phase[i]);
	    fprintf(stderr,"dbg5       sid_sig_ping:  %d\n",store->sid_sig_ping);
	    fprintf(stderr,"dbg5       sid_sig_channel:  %d\n",store->sid_sig_channel);
	    fprintf(stderr,"dbg5       sid_sig_offset:  %f\n",store->sid_sig_offset);
	    fprintf(stderr,"dbg5       sid_sig_sample:  %f\n",store->sid_sig_sample);
	    fprintf(stderr,"dbg5       sid_sig_num_samples:  %d\n",store->sid_sig_num_samples);
	    for (i=0;i<store->sid_sig_num_samples;i++)
	    	fprintf(stderr,"dbg5       sid_sig_phase[%d]:%d\n",i,store->sid_sig_phase[i]);
	    fprintf(stderr,"dbg5       sid_png_pulse:  %u\n",store->sid_png_pulse);
	    fprintf(stderr,"dbg5       sid_png_startfrequency:  %f\n",store->sid_png_startfrequency);
	    fprintf(stderr,"dbg5       sid_png_endfrequency:  %f\n",store->sid_png_endfrequency);
	    fprintf(stderr,"dbg5       sid_png_duration:  %f\n",store->sid_png_duration);
	    fprintf(stderr,"dbg5       sid_png_mancode:  %d\n",store->sid_png_mancode);
	    fprintf(stderr,"dbg5       sid_png_pulseid:  %d\n",store->sid_png_pulseid);
	    fprintf(stderr,"dbg5       sid_png_pulsename:  %s\n",store->sid_png_pulsename);
	    fprintf(stderr,"dbg5       sid_cmp_ping:  %d\n",store->sid_cmp_ping);
	    fprintf(stderr,"dbg5       sid_cmp_channel:  %d\n",store->sid_cmp_channel);
	    fprintf(stderr,"dbg5       sid_cmp_offset:  %f\n",store->sid_cmp_offset);
	    fprintf(stderr,"dbg5       sid_cmp_sample:  %f\n",store->sid_cmp_sample);
	    fprintf(stderr,"dbg5       sid_cmp_num_samples:  %d\n",store->sid_cmp_num_samples);
	    for (i=0;i<store->sid_sig_num_samples;i++)
	    	fprintf(stderr,"dbg5       sid_cmp_real[%d]:%d sid_cmp_imaginary[%d]:%d\n",
				i,store->sid_cmp_real[i],i,store->sid_cmp_imaginary[i]);
	    fprintf(stderr,"dbg5       sid_wgt_factorleft:  %d\n",store->sid_wgt_factorleft);
	    fprintf(stderr,"dbg5       sid_wgt_samplesleft:  %d\n",store->sid_wgt_samplesleft);
	    fprintf(stderr,"dbg5       sid_wgt_factorright:  %d\n",store->sid_wgt_factorright);
	    fprintf(stderr,"dbg5       sid_wgt_samplesright:  %d\n",store->sid_wgt_samplesright);
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
int mbr_l3xseraw_rd_multibeam(int verbose,int buffer_size,char *buffer,void *store_ptr,int *error)
{
	char	*function_name = "mbr_l3xseraw_rd_multibeam";
	int	status = MB_SUCCESS;
	struct mbsys_xse_struct *store;
	int	byte_count;
	int	group_id;
	int	done;
	int	index;
	double	alpha, beta, theta, phi;
	double	rr, xx, zz;
	double	xmin, xmax, binsize;
	int	ngoodss;
	int	skip;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer_size:%d\n",buffer_size);
		fprintf(stderr,"dbg2       buffer:     %lu\n",(size_t)buffer);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get pointer to store data structure */
	store = (struct mbsys_xse_struct *) store_ptr;

	/* set group flags off */
	store->mul_group_beam = MB_NO;	/* boolean flag - beam group read */
	store->mul_group_tt = MB_NO;	/* boolean flag - tt group read */
	store->mul_group_quality = MB_NO;/* boolean flag - quality group read */
	store->mul_group_amp = MB_NO;	/* boolean flag - amp group read */
	store->mul_group_delay = MB_NO;	/* boolean flag - delay group read */
	store->mul_group_lateral = MB_NO;/* boolean flag - lateral group read */
	store->mul_group_along = MB_NO;	/* boolean flag - along group read */
	store->mul_group_depth = MB_NO;	/* boolean flag - depth group read */
	store->mul_group_angle = MB_NO;	/* boolean flag - angle group read */
	store->mul_group_heave = MB_NO;	/* boolean flag - heave group read */
	store->mul_group_roll = MB_NO;	/* boolean flag - roll group read */
	store->mul_group_pitch = MB_NO;	/* boolean flag - pitch group read */
	store->mul_group_gates = MB_NO;	/* boolean flag - gates group read */
	store->mul_group_noise = MB_NO;	/* boolean flag - noise group read */
	store->mul_group_length = MB_NO;	/* boolean flag - length group read */
	store->mul_group_hits = MB_NO;		/* boolean flag - hits group read */
	store->mul_group_heavereceive = MB_NO;	/* boolean flag - heavereceive group read */
	store->mul_group_azimuth = MB_NO;	/* boolean flag - azimuth group read */
	store->mul_group_mbsystemnav = MB_NO;	/* boolean flag - mbsystemnav group read */
		
	/* get source and time */
	index = 12;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->mul_source); index += 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->mul_sec); index += 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->mul_usec); index += 4;
		
	/* loop over groups */
	done = MB_NO;
	while (index <= buffer_size 
		&& status == MB_SUCCESS
		&& done == MB_NO)
	    {
	    /* look for group start or frame end */
	    skip = 0;
#ifdef DATAINPCBYTEORDER
	    while (index < buffer_size
			&& strncmp(&buffer[index], "GSH$", 4)
			&& strncmp(&buffer[index], "FSH#", 4))
			{
			index++;
			skip++;
			}
	    if (index >= buffer_size
			|| !strncmp(&buffer[index], "FSH#", 4))
			done = MB_YES;
	    else
			index += 4;
#else
	    while (index < buffer_size
			&& strncmp(&buffer[index], "$HSG", 4)
			&& strncmp(&buffer[index], "#HSF", 4))
			{
			index++;
			skip++;
			}
	    if (index >= buffer_size
			|| !strncmp(&buffer[index], "#HSF", 4))
			done = MB_YES;
	    else
			index += 4;
#endif

#ifdef MB_DEBUG
if (skip > 4) fprintf(stderr, "skipped %d bytes in function <%s>\n", skip-4, function_name);
#endif
	    
	    /* deal with group */
	    if (done == MB_NO)
			{
			/* get group size and id */
			mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &byte_count); index += 4;
			mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &group_id); index += 4;

			/* print debug statements */
			if (verbose >= 5)
			    {
			    fprintf(stderr,"\ndbg5  Group %d of %d bytes to be parsed in MBIO function <%s>\n",
				    group_id, byte_count, function_name);
			    }
#ifdef MB_DEBUG
fprintf(stderr, "Group %d of %d bytes to be parsed in MBIO function <%s>\n",
				    group_id, byte_count, function_name);
#endif
	    
		    /* handle general group */
		    if (group_id == MBSYS_XSE_MBM_GROUP_GEN)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ MBM_GROUP_GEN\n");
#endif
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->mul_ping); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->mul_frequency); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->mul_pulse); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->mul_power); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->mul_bandwidth); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->mul_sample); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->mul_swath); index += 4;
#ifdef MB_DEBUG2
fprintf(stderr, "ping=%u\n", store->mul_ping);
fprintf(stderr, "frequency=%f\n", store->mul_frequency);
fprintf(stderr, "pulse=%f\n", store->mul_pulse);
fprintf(stderr, "power=%f\n", store->mul_power);
fprintf(stderr, "bandwidth=%f\n", store->mul_bandwidth);
fprintf(stderr, "sample=%f\n", store->mul_sample);
fprintf(stderr, "swath=%f\n", store->mul_swath);
#endif
				}
	    
		    /* handle beam group */
		    else if (group_id == MBSYS_XSE_MBM_GROUP_BEAM)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ MBM_GROUP_BEAM\n");
#endif
				store->mul_group_beam = MB_YES;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->mul_num_beams); index += 4;
				for (i=0;i<store->mul_num_beams;i++)
				    {
				    if (i < MBSYS_XSE_MAXBEAMS)
						{
						mb_get_binary_short(SWAPFLAG, &buffer[index], &store->beams[i].beam); index += 2;
						}
				    }
				}
	    
		    /* handle traveltime group */
		    else if (group_id == MBSYS_XSE_MBM_GROUP_TT)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ MBM_GROUP_TT\n");
#endif
				store->mul_group_tt = MB_YES;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->mul_num_beams); index += 4;
				for (i=0;i<store->mul_num_beams;i++)
				    {
				    if (i < MBSYS_XSE_MAXBEAMS)
						{
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->beams[i].tt); index += 8;
						}
				    }
#ifdef MB_DEBUG2
fprintf(stderr, "N=%u\n", store->mul_num_beams);
for(i=0;i<store->mul_num_beams;i++)
	fprintf(stderr, "tt[%d]=%f\n", i, store->beams[i].tt);
#endif
				}
	    
		    /* handle quality group */
		    else if (group_id == MBSYS_XSE_MBM_GROUP_QUALITY)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ MBM_GROUP_QUALITY\n");
#endif
				store->mul_group_quality = MB_YES;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->mul_num_beams); index += 4;
				for (i=0;i<store->mul_num_beams;i++)
				    {
				    if (i < MBSYS_XSE_MAXBEAMS)
						store->beams[i].quality = buffer[index++];
				    }
#ifdef MB_DEBUG2
fprintf(stderr, "N=%u\n", store->mul_num_beams);
for(i=0;i<store->mul_num_beams;i++)
	fprintf(stderr, "quality[%d]=%u\n", i, store->beams[i].quality);
#endif
				}
	    
		    /* handle amplitude group */
		    else if (group_id == MBSYS_XSE_MBM_GROUP_AMP)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ MBM_GROUP_AMP\n");
#endif
				store->mul_group_amp = MB_YES;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->mul_num_beams); index += 4;
				for (i=0;i<store->mul_num_beams;i++)
				    {
				    if (i < MBSYS_XSE_MAXBEAMS)
						{
						mb_get_binary_short(SWAPFLAG, &buffer[index], &store->beams[i].amplitude); index += 2;
						}
				    }
#ifdef MB_DEBUG2
fprintf(stderr, "N=%u\n", store->mul_num_beams);
for(i=0;i<store->mul_num_beams;i++)
	fprintf(stderr, "amp[%d]=%d\n", i, store->beams[i].amplitude);
#endif
				}
	    
		    /* handle delay group */
		    else if (group_id == MBSYS_XSE_MBM_GROUP_DELAY)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ MBM_GROUP_DELAY\n");
#endif
				store->mul_group_delay = MB_YES;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->mul_num_beams); index += 4;
				for (i=0;i<store->mul_num_beams;i++)
				    {
				    if (i < MBSYS_XSE_MAXBEAMS)
						{
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->beams[i].delay); index += 8;
						}
				    }
#ifdef MB_DEBUG2
fprintf(stderr, "N=%u\n", store->mul_num_beams);
for(i=0;i<store->mul_num_beams;i++)
	fprintf(stderr, "delay[%d]=%lf\n", i, store->beams[i].delay);
#endif
				}
	    
		    /* handle lateral group */
		    else if (group_id == MBSYS_XSE_MBM_GROUP_LATERAL)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ MBM_GROUP_LATERAL\n");
#endif
				store->mul_group_lateral = MB_YES;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->mul_num_beams); index += 4;
				for (i=0;i<store->mul_num_beams;i++)
				    {
				    if (i < MBSYS_XSE_MAXBEAMS)
						{
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->beams[i].lateral); index += 8;
						}
				    }
#ifdef MB_DEBUG2
fprintf(stderr, "N=%u\n", store->mul_num_beams);
for(i=0;i<store->mul_num_beams;i++)
	fprintf(stderr, "lateral[%d]=%lf\n", i, store->beams[i].lateral);
#endif
				}
	    
		    /* handle along group */
		    else if (group_id == MBSYS_XSE_MBM_GROUP_ALONG)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ MBM_GROUP_ALONG\n");
#endif
				store->mul_group_along = MB_YES;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->mul_num_beams); index += 4;
				for (i=0;i<store->mul_num_beams;i++)
				    {
				    if (i < MBSYS_XSE_MAXBEAMS)
						{
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->beams[i].along); index += 8;
						}
				    }
#ifdef MB_DEBUG2
fprintf(stderr, "N=%u\n", store->mul_num_beams);
for(i=0;i<store->mul_num_beams;i++)
	fprintf(stderr, "along[%d]=%lf\n", i, store->beams[i].along);
#endif
				}
	    
		    /* handle depth group */
		    else if (group_id == MBSYS_XSE_MBM_GROUP_DEPTH)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ MBM_GROUP_DEPTH\n");
#endif
				store->mul_group_depth = MB_YES;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->mul_num_beams); index += 4;
				for (i=0;i<store->mul_num_beams;i++)
				    {
				    if (i < MBSYS_XSE_MAXBEAMS)
						{
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->beams[i].depth); index += 8;
						}
				    }
#ifdef MB_DEBUG2
fprintf(stderr, "N=%u\n", store->mul_num_beams);
for(i=0;i<store->mul_num_beams;i++)
	fprintf(stderr, "depth[%d]=%lf\n", i, store->beams[i].depth);
#endif
				}
	    
		    /* handle angle group */
		    else if (group_id == MBSYS_XSE_MBM_GROUP_ANGLE)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ MBM_GROUP_ANGLE\n");
#endif
				store->mul_group_angle = MB_YES;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->mul_num_beams); index += 4;
				for (i=0;i<store->mul_num_beams;i++)
				    {
				    if (i < MBSYS_XSE_MAXBEAMS)
						{
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->beams[i].angle); index += 8;
						}
				    }
#ifdef MB_DEBUG2
fprintf(stderr, "N=%u\n", store->mul_num_beams);
for(i=0;i<store->mul_num_beams;i++)
	fprintf(stderr, "angle[%d]=%lf\n", i, store->beams[i].angle);
#endif
				}
	    
		    /* handle heave group */
		    else if (group_id == MBSYS_XSE_MBM_GROUP_HEAVE)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ MBM_GROUP_HEAVE\n");
#endif
				store->mul_group_heave = MB_YES;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->mul_num_beams); index += 4;
				for (i=0;i<store->mul_num_beams;i++)
				    {
				    if (i < MBSYS_XSE_MAXBEAMS)
						{
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->beams[i].heave); index += 8;
						}
				    }
#ifdef MB_DEBUG2
fprintf(stderr, "N=%u\n", store->mul_num_beams);
for(i=0;i<store->mul_num_beams;i++)
	fprintf(stderr, "heave[%d]=%lf\n", i, store->beams[i].heave);
#endif
				}
	    
		    /* handle roll group */
		    else if (group_id == MBSYS_XSE_MBM_GROUP_ROLL)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ MBM_GROUP_ROLL\n");
#endif
				store->mul_group_roll = MB_YES;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->mul_num_beams); index += 4;
				for (i=0;i<store->mul_num_beams;i++)
				    {
				    if (i < MBSYS_XSE_MAXBEAMS)
						{
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->beams[i].roll); index += 8;
						}
				    }
#ifdef MB_DEBUG2
fprintf(stderr, "N=%u\n", store->mul_num_beams);
for(i=0;i<store->mul_num_beams;i++)
	fprintf(stderr, "roll[%d]=%lf\n", i, store->beams[i].roll);
#endif
				}
	    
		    /* handle pitch group */
		    else if (group_id == MBSYS_XSE_MBM_GROUP_PITCH)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ MBM_GROUP_PITCH\n");
#endif
				store->mul_group_pitch = MB_YES;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->mul_num_beams); index += 4;
				for (i=0;i<store->mul_num_beams;i++)
				    {
				    if (i < MBSYS_XSE_MAXBEAMS)
						{
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->beams[i].pitch); index += 8;
						}
				    }
#ifdef MB_DEBUG2
fprintf(stderr, "N=%u\n", store->mul_num_beams);
for(i=0;i<store->mul_num_beams;i++)
	fprintf(stderr, "pitch[%d]=%lf\n", i, store->beams[i].pitch);
#endif
				}

			/* handle gates group */
			else if (group_id == MBSYS_XSE_MBM_GROUP_GATES)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ MBM_GROUP_GATES\n");
#endif
				store->mul_group_gates = MB_YES;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->mul_num_beams); index += 4;
				for (i=0;i<store->mul_num_beams;i++)
				    {
				    if (i < MBSYS_XSE_MAXBEAMS)
						{
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->beams[i].gate_angle); index += 8;
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->beams[i].gate_start); index += 8;
						mb_get_binary_double(SWAPFLAG, &buffer[index], &store->beams[i].gate_stop); index += 8;
						}
				    }
#ifdef MB_DEBUG2
fprintf(stderr, "N=%u\n", store->mul_num_beams);
for(i=0;i<store->mul_num_beams;i++)
	fprintf(stderr, "gate_angle[%d]=%lf gate_start[%d]=%lf gate_stop[%d]=%lf\n", 
	    i, store->beams[i].gate_angle, i,store->beams[i].gate_start, i,store->beams[i].gate_stop);
#endif
				}

			/* handle noise group */
			else if (group_id == MBSYS_XSE_MBM_GROUP_NOISE)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ MBM_GROUP_NOISE\n");
#endif
				store->mul_group_noise = MB_YES;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->mul_num_beams); index += 4;
				for (i=0;i<store->mul_num_beams;i++)
				    {
				    if (i < MBSYS_XSE_MAXBEAMS)
						{
						mb_get_binary_float(SWAPFLAG, &buffer[index], &store->beams[i].noise); index += 4;
						}
				    }
#ifdef MB_DEBUG2
fprintf(stderr, "N=%u\n", store->mul_num_beams);
for(i=0;i<store->mul_num_beams;i++)
	fprintf(stderr, "noise[%d]=%f\n", i, store->beams[i].noise);
#endif
				}

			/* handle length group */
			else if (group_id == MBSYS_XSE_MBM_GROUP_LENGTH)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ MBM_GROUP_LENGTH\n");
#endif
				store->mul_group_length = MB_YES;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->mul_num_beams); index += 4;
				for (i=0;i<store->mul_num_beams;i++)
				    {
				    if (i < MBSYS_XSE_MAXBEAMS)
						{
						mb_get_binary_float(SWAPFLAG, &buffer[index], &store->beams[i].length); index += 4;
						}
				    }
#ifdef MB_DEBUG2
fprintf(stderr, "N=%u\n", store->mul_num_beams);
for(i=0;i<store->mul_num_beams;i++)
	fprintf(stderr, "length[%d]=%f\n", i, store->beams[i].length);
#endif
				}

			/* handle hits group */
			else if (group_id == MBSYS_XSE_MBM_GROUP_HITS)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ MBM_GROUP_HITS\n");
#endif
				store->mul_group_hits = MB_YES;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->mul_num_beams); index += 4;
				for (i=0;i<store->mul_num_beams;i++)
				    {
				    if (i < MBSYS_XSE_MAXBEAMS)
						{
						mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->beams[i].hits); index += 4;
						}
				    }
#ifdef MB_DEBUG2
fprintf(stderr, "N=%u\n", store->mul_num_beams);
for(i=0;i<store->mul_num_beams;i++)
	fprintf(stderr, "hits[%d]=%d\n", i, store->beams[i].hits);
#endif
				}

			/* handle heavereceive group */
			else if (group_id == MBSYS_XSE_MBM_GROUP_HEAVERECEIVE)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ MBM_GROUP_HEAVERECEIVE\n");
#endif
				store->mul_group_heavereceive = MB_YES;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->mul_num_beams); index += 4;
				for (i=0;i<store->mul_num_beams;i++)
				    {
				    if (i < MBSYS_XSE_MAXBEAMS)
						{
						mb_get_binary_double(SWAPFLAG, &buffer[index], (double *) &store->beams[i].heavereceive); index += 8;
						}
				    }
#ifdef MB_DEBUG2
fprintf(stderr, "N=%u\n", store->mul_num_beams);
for(i=0;i<store->mul_num_beams;i++)
	fprintf(stderr, "heavereceive[%d]=%f\n", i, store->beams[i].heavereceive);
#endif
				}

			/* handle azimuth group */
			else if (group_id == MBSYS_XSE_MBM_GROUP_AZIMUTH)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ MBM_GROUP_AZIMUTH\n");
#endif
				store->mul_group_azimuth = MB_YES;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->mul_num_beams); index += 4;
				for (i=0;i<store->mul_num_beams;i++)
				    {
				    if (i < MBSYS_XSE_MAXBEAMS)
						{
						mb_get_binary_double(SWAPFLAG, &buffer[index], (double *) &store->beams[i].azimuth); index += 4;
						}
				    }
#ifdef MB_DEBUG2
fprintf(stderr, "N=%u\n", store->mul_num_beams);
for(i=0;i<store->mul_num_beams;i++)
	fprintf(stderr, "azimuth[%d]=%f\n", i, store->beams[i].azimuth);
#endif
				}

			/* handle mbsystemnav group */
			else if (group_id == MBSYS_XSE_MBM_GROUP_MBSYSTEMNAV)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ MBSYS_XSE_MBM_GROUP_MBSYSTEMNAV\n");
#endif
				store->mul_group_mbsystemnav = MB_YES;
				mb_get_binary_double(SWAPFLAG, &buffer[index], (double *) &store->mul_lon); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], (double *) &store->mul_lat); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], (double *) &store->mul_heading); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], (double *) &store->mul_speed); index += 8;
#ifdef MB_DEBUG2
fprintf(stderr, "lon=%f\n", store->mul_lon);
fprintf(stderr, "lon=%f\n", store->mul_lat);
fprintf(stderr, "lon=%f\n", store->mul_heading);
fprintf(stderr, "lon=%f\n", store->mul_speed);
#endif
				}

			else
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ MBM_GROUP_OTHER  group_id:%d\n", group_id);
#endif
				}
		    }
		}
	    
	/* now if tt and angles read but bathymetry not read
	   calculate bathymetry assuming 1500 m/s velocity */
	if (status == MB_SUCCESS
	    && store->mul_group_tt == MB_YES
	    && store->mul_group_angle == MB_YES
	    && store->mul_group_heave == MB_YES
	    && store->mul_group_roll == MB_YES
	    && store->mul_group_pitch == MB_YES
	    && store->mul_group_depth == MB_NO)
	    {
	    store->mul_group_lateral = MB_YES;
	    store->mul_group_along = MB_YES;
	    store->mul_group_depth = MB_YES;
	    for (i=0;i<store->mul_num_beams;i++)
		{
		beta = 90.0 - RTD * store->beams[i].angle;
		alpha = RTD * store->beams[i].pitch;
		mb_rollpitch_to_takeoff(verbose, 
		    alpha, beta, &theta, 
		    &phi, error);
		/* divide range by 2 because of round trip travel time */
		rr = 1500.0 * store->beams[i].tt/2.0;
		xx = rr * sin(DTR * theta);
		zz = rr * cos(DTR * theta);
		store->beams[i].lateral 
			= xx * cos(DTR * phi);
		store->beams[i].along 
			= xx * sin(DTR * phi) 
		    + 0.5 * store->nav_speed_ground 
			* store->beams[i].delay;
		store->beams[i].depth = zz;
		}
	    }
	    
	/* check for sensible bathymetry */
	if (status == MB_SUCCESS
	    && store->mul_group_depth == MB_YES)
	    {
	    for (i=0;i<store->mul_num_beams;i++)
		{
		if (fabs(store->beams[i].depth) > 11000.0)
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_UNINTELLIGIBLE;
		    
		    if (fabs(store->beams[i].heave) > 100.0)
			store->beams[i].heave = 0.0;
		    }
		}
	    }
	    
	/* now if sidescan already read but bin size lacking then
	   calculate bin size from bathymetry */
	if (store->mul_num_beams > 1
	    && store->sid_frame == MB_YES
	    && store->sid_avl_num_samples > 1
	    && store->sid_avl_binsize <= 0)
	    {
	    /* get width of bathymetry swath size */
	    xmin = 9999999.9;
	    xmax = -9999999.9;
	    for (i=0;i<store->mul_num_beams;i++)
			{
			xmin = MIN(xmin, store->beams[i].lateral);
			xmax = MAX(xmax, store->beams[i].lateral);
			}
	    
	    /* get number of nonzero pixels */
	    ngoodss = 0;
	    for (i=0;i<store->sid_avl_num_samples;i++)
			if (store->sid_avl_amp[i] != 0) ngoodss++;
		
	    /* get bin size */
	    if (xmax > xmin && ngoodss > 1)
			{
			binsize = (xmax - xmin) / (ngoodss - 1);
			store->sid_avl_binsize = 1000 * binsize;
			}
	    }

	/* print debug statements */
	if (verbose >= 5)
	    {
	    fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
	    fprintf(stderr,"dbg5       mul_source:          %d\n",store->mul_source);
	    fprintf(stderr,"dbg5       mul_sec:             %u\n",store->mul_sec);
	    fprintf(stderr,"dbg5       mul_usec:            %u\n",store->mul_usec);
	    fprintf(stderr,"dbg5       mul_ping:            %d\n",store->mul_ping);
	    fprintf(stderr,"dbg5       mul_frequency:       %f\n",store->mul_frequency);
	    fprintf(stderr,"dbg5       mul_pulse:           %f\n",store->mul_pulse);
	    fprintf(stderr,"dbg5       mul_power:           %f\n",store->mul_power);
	    fprintf(stderr,"dbg5       mul_bandwidth:       %f\n",store->mul_bandwidth);
	    fprintf(stderr,"dbg5       mul_sample:          %f\n",store->mul_sample);
	    fprintf(stderr,"dbg5       mul_swath:           %f\n",store->mul_swath);
	    fprintf(stderr,"dbg5       mul_group_beam:      %d\n",store->mul_group_beam);
	    fprintf(stderr,"dbg5       mul_group_tt:        %d\n",store->mul_group_tt);
	    fprintf(stderr,"dbg5       mul_group_quality:   %d\n",store->mul_group_quality);
	    fprintf(stderr,"dbg5       mul_group_amp:       %d\n",store->mul_group_amp);
	    fprintf(stderr,"dbg5       mul_group_delay:     %d\n",store->mul_group_delay);
	    fprintf(stderr,"dbg5       mul_group_lateral:   %d\n",store->mul_group_lateral);
	    fprintf(stderr,"dbg5       mul_group_along:     %d\n",store->mul_group_along);
	    fprintf(stderr,"dbg5       mul_group_depth:     %d\n",store->mul_group_depth);
	    fprintf(stderr,"dbg5       mul_group_angle:     %d\n",store->mul_group_angle);
	    fprintf(stderr,"dbg5       mul_group_heave:     %d\n",store->mul_group_heave);
	    fprintf(stderr,"dbg5       mul_group_roll:      %d\n",store->mul_group_roll);
	    fprintf(stderr,"dbg5       mul_group_pitch:     %d\n",store->mul_group_pitch);
	    fprintf(stderr,"dbg5       mul_num_beams:       %d\n",store->mul_num_beams);
	    for (i=0;i<store->mul_num_beams;i++)
		fprintf(stderr,"dbg5       beam[%d]: %3d %7.2f %7.2f %7.2f %3d %3d %6.3f %6.2f %5.3f %5.2f %6.2f %6.2f\n",
		    i, store->beams[i].beam, 
		    store->beams[i].lateral, 
		    store->beams[i].along, 
		    store->beams[i].depth, 
		    store->beams[i].amplitude, 
		    store->beams[i].quality, 
		    store->beams[i].tt, 
		    store->beams[i].angle, 
		    store->beams[i].delay, 
		    store->beams[i].heave, 
		    store->beams[i].roll, 
		    store->beams[i].pitch);
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
int mbr_l3xseraw_rd_singlebeam(int verbose,int buffer_size,char *buffer,void *store_ptr,int *error)
{
	char    *function_name = "mbr_l3xseraw_rd_singlebeam";
	int status = MB_SUCCESS;
	struct mbsys_xse_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer_size:%d\n",buffer_size);
		fprintf(stderr,"dbg2       buffer:     %lu\n",(size_t)buffer);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get pointer to store data structure */
	store = (struct mbsys_xse_struct *) store_ptr;

	/* The singlebeam frame is currently unused by MB-System */

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
int mbr_l3xseraw_rd_message(int verbose,int buffer_size,char *buffer,void *store_ptr,int *error)
{
	char    *function_name = "mbr_l3xseraw_rd_message";
	int status = MB_SUCCESS;
	struct mbsys_xse_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer_size:%d\n",buffer_size);
		fprintf(stderr,"dbg2       buffer:     %lu\n",(size_t)buffer);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get pointer to store data structure */
	store = (struct mbsys_xse_struct *) store_ptr;

	/* The message frame is currently unused by MB-System */

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
int mbr_l3xseraw_rd_seabeam(int verbose,int buffer_size,char *buffer,void *store_ptr,int *error)
{
	char    *function_name = "mbr_l3xseraw_rd_seabeam";
	int status = MB_SUCCESS;
	struct mbsys_xse_struct *store;
	int byte_count;
	int group_id;
	int done;
	int index;
	int	skip;
	int i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer_size:%d\n",buffer_size);
		fprintf(stderr,"dbg2       buffer:     %lu\n",(size_t)buffer);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}


	/* get pointer to store data structure */
	store = (struct mbsys_xse_struct *) store_ptr;
		
	/* get source and time */
	index = 12;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sbm_source); index += 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sbm_sec); index += 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sbm_usec); index += 4;
		
	/* loop over groups */
	done = MB_NO;
	while (index <= buffer_size 
		&& status == MB_SUCCESS
		&& done == MB_NO)
	    {
	    /* look for group start or frame end */
	    skip = 0;
#ifdef DATAINPCBYTEORDER
	    while (index < buffer_size
			&& strncmp(&buffer[index], "GSH$", 4)
			&& strncmp(&buffer[index], "FSH#", 4))
			{
			index++;
			skip++;
			}
	    if (index >= buffer_size
			|| !strncmp(&buffer[index], "FSH#", 4))
			done = MB_YES;
	    else
			index += 4;
#else
	    while (index < buffer_size
			&& strncmp(&buffer[index], "$HSG", 4)
			&& strncmp(&buffer[index], "#HSF", 4))
			{
			index++;
			skip++;
			}
	    if (index >= buffer_size
			|| !strncmp(&buffer[index], "#HSF", 4))
			done = MB_YES;
	    else
			index += 4;
#endif
	    
#ifdef MB_DEBUG
if (skip > 4) fprintf(stderr, "skipped %d bytes in function <%s>\n", skip-4, function_name);
#endif
	    
	    /* deal with group */
	    if (done == MB_NO)
		    {
		    /* get group size and id */
		    mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &byte_count); index += 4;
		    mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &group_id); index += 4;
	
		    /* print debug statements */
		    if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg5  Group %d of %d bytes to be parsed in MBIO function <%s>\n",
				group_id, byte_count, function_name);
			}
#ifdef MB_DEBUG
fprintf(stderr, "Group %d of %d bytes to be parsed in MBIO function <%s>\n",
				    group_id, byte_count, function_name);
#endif
		    
		    /* handle properties group */
		    if (group_id == MBSYS_XSE_SBM_GROUP_PROPERTIES)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ SBM_GROUP_PROPERTIES\n");
#endif
				store->sbm_properties = MB_YES;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sbm_ping); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->sbm_ping_gain); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->sbm_pulse_width); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->sbm_transmit_power); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->sbm_pixel_width); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->sbm_swath_width); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->sbm_time_slice); index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sbm_depth_mode); index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sbm_beam_mode); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->sbm_ssv); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->sbm_frequency); index += 4;
				mb_get_binary_float(SWAPFLAG, &buffer[index], &store->sbm_bandwidth); index += 4;
				}
		    
		    /* handle hrp group */
		    if (group_id == MBSYS_XSE_SBM_GROUP_HRP)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ SBM_GROUP_HRP\n");
#endif
				store->sbm_hrp = MB_YES;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->sbm_heave); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->sbm_roll); index += 8;
				mb_get_binary_double(SWAPFLAG, &buffer[index], &store->sbm_pitch); index += 8;
				}
		    
		    /* handle center group */
		    if (group_id == MBSYS_XSE_SBM_GROUP_CENTER)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ SBM_GROUP_CENTER\n");
#endif
				store->sbm_center = MB_YES;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sbm_center_beam); index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sbm_center_count); index += 4;
				store->sbm_center_count = MIN(MBSYS_XSE_MAXSAMPLES, store->sbm_center_count);
				for (i=0;i<store->sbm_center_count;i++)
				    {
				    mb_get_binary_float(SWAPFLAG, &buffer[index], &store->sbm_center_amp[i]); 
				    index += 4;
				    }
				}
		    
		    /* handle message group */
		    if (group_id == MBSYS_XSE_SBM_GROUP_MESSAGE)
				{
#ifdef MB_DEBUG
fprintf(stderr, "READ SBM_GROUP_MESSAGE\n");
#endif
				store->sbm_message = MB_YES;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sbm_message_id); index += 4;
				mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->sbm_message_len); index += 4;
if (store->sbm_message_len > buffer_size)
 fprintf(stderr,"Read message: %d %d %d\n",buffer_size,store->sbm_message_len,store->sbm_message_id);
				for (i=0;i<store->sbm_message_len;i++)
				    {
				    store->sbm_message_txt[i] = buffer[index]; 
				    index++;
				    }
				store->sbm_message_txt[store->sbm_message_len] = 0;
				}
		    }
	    }

	/* print debug statements */
	if (verbose >= 5)
	    {
	    fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
	    fprintf(stderr,"dbg5       sbm_source:          %d\n",store->sbm_source);
	    fprintf(stderr,"dbg5       sbm_sec:             %u\n",store->sbm_sec);
	    fprintf(stderr,"dbg5       sbm_usec:            %u\n",store->sbm_usec);
	    }
	if (verbose >= 5 && store->sbm_properties == MB_YES)
	    {
	    fprintf(stderr,"dbg5       sbm_ping:            %d\n",store->sbm_ping);
	    fprintf(stderr,"dbg5       sbm_ping_gain:       %f\n",store->sbm_ping_gain);
	    fprintf(stderr,"dbg5       sbm_pulse_width:     %f\n",store->sbm_pulse_width);
	    fprintf(stderr,"dbg5       sbm_transmit_power:  %f\n",store->sbm_transmit_power);
	    fprintf(stderr,"dbg5       sbm_pixel_width:     %f\n",store->sbm_pixel_width);
	    fprintf(stderr,"dbg5       sbm_swath_width:     %f\n",store->sbm_swath_width);
	    fprintf(stderr,"dbg5       sbm_time_slice:      %f\n",store->sbm_time_slice);
	    fprintf(stderr,"dbg5       sbm_depth_mode:      %d\n",store->sbm_depth_mode);
	    fprintf(stderr,"dbg5       sbm_beam_mode:       %d\n",store->sbm_beam_mode);
	    fprintf(stderr,"dbg5       sbm_ssv:             %f\n",store->sbm_ssv);
	    fprintf(stderr,"dbg5       sbm_frequency:       %f\n",store->sbm_frequency);
	    fprintf(stderr,"dbg5       sbm_bandwidth:       %f\n",store->sbm_bandwidth);
	    }
	if (verbose >= 5 && store->sbm_hrp == MB_YES)
	    {
	    fprintf(stderr,"dbg5       sbm_heave:           %f\n",store->sbm_heave);
	    fprintf(stderr,"dbg5       sbm_roll:            %f\n",store->sbm_roll);
	    fprintf(stderr,"dbg5       sbm_pitch:           %f\n",store->sbm_pitch);
	    }
	if (verbose >= 5 && store->sbm_center == MB_YES)
	    {
	    fprintf(stderr,"dbg5       sbm_center_beam:     %d\n",store->sbm_center_beam);
	    fprintf(stderr,"dbg5       sbm_center_count:    %d\n",store->sbm_center_count);
	    for (i=0;i<store->sbm_center_count;i++)
		fprintf(stderr,"dbg5       sample[%d]: %f\n",
		    i, store->sbm_center_amp[i]);
	    }
	if (verbose >= 5 && store->sbm_message == MB_YES)
	    {
	    fprintf(stderr,"dbg5       sbm_message_id:      %d\n",store->sbm_message_id);
	    fprintf(stderr,"dbg5       sbm_message_len:     %d\n",store->sbm_message_len);
	    fprintf(stderr,"dbg5       sbm_message_txt:     %s\n",store->sbm_message_txt);
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
int mbr_l3xseraw_rd_geodetic(int verbose,int buffer_size,char *buffer,void *store_ptr,int *error)
{
	char    *function_name = "mbr_l3xseraw_rd_geodetic";
	int status = MB_SUCCESS;
	struct mbsys_xse_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer_size:%d\n",buffer_size);
		fprintf(stderr,"dbg2       buffer:     %lu\n",(size_t)buffer);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get pointer to store data structure */
	store = (struct mbsys_xse_struct *) store_ptr;

	/* The geodetic frame is currently unused by MB-System */

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
int mbr_l3xseraw_rd_native(int verbose,int buffer_size,char *buffer,void *store_ptr,int *error)
{
	char    *function_name = "mbr_l3xseraw_rd_native";
	int status = MB_SUCCESS;
	struct mbsys_xse_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer_size:%d\n",buffer_size);
		fprintf(stderr,"dbg2       buffer:     %lu\n",(size_t)buffer);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get pointer to store data structure */
	store = (struct mbsys_xse_struct *) store_ptr;

	/* The native frame is currently unused by MB-System */

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
int mbr_l3xseraw_rd_product(int verbose,int buffer_size,char *buffer,void *store_ptr,int *error)
{
	char    *function_name = "mbr_l3xseraw_rd_product";
	int status = MB_SUCCESS;
	struct mbsys_xse_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer_size:%d\n",buffer_size);
		fprintf(stderr,"dbg2       buffer:     %lu\n",(size_t)buffer);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get pointer to store data structure */
	store = (struct mbsys_xse_struct *) store_ptr;

	/* The product frame is currently unused by MB-System */

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
int mbr_l3xseraw_rd_bathymetry(int verbose,int buffer_size,char *buffer,void *store_ptr,int *error)
{
	char    *function_name = "mbr_l3xseraw_rd_bathymetry";
	int status = MB_SUCCESS;
	struct mbsys_xse_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer_size:%d\n",buffer_size);
		fprintf(stderr,"dbg2       buffer:     %lu\n",(size_t)buffer);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get pointer to store data structure */
	store = (struct mbsys_xse_struct *) store_ptr;

	/* The bathymetry frame is currently unused by MB-System */

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
int mbr_l3xseraw_rd_control(int verbose,int buffer_size,char *buffer,void *store_ptr,int *error)
{
	char    *function_name = "mbr_l3xseraw_rd_control";
	int status = MB_SUCCESS;
	struct mbsys_xse_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer_size:%d\n",buffer_size);
		fprintf(stderr,"dbg2       buffer:     %lu\n",(size_t)buffer);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get pointer to store data structure */
	store = (struct mbsys_xse_struct *) store_ptr;

	/* The control frame is currently unused by MB-System */

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
int mbr_l3xseraw_rd_comment(int verbose,int buffer_size,char *buffer,void *store_ptr,int *error)
{
	char	*function_name = "mbr_l3xseraw_rd_comment";
	int	status = MB_SUCCESS;
	struct mbsys_xse_struct *store;
	int	byte_count;
	int	group_id;
	int	done;
	int	index;
	int	skip;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       buffer_size:%d\n",buffer_size);
		fprintf(stderr,"dbg2       buffer:     %lu\n",(size_t)buffer);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get pointer to store data structure */
	store = (struct mbsys_xse_struct *) store_ptr;
		
	/* get source and time */
	index = 12;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->com_source); index += 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->com_sec); index += 4;
	mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &store->com_usec); index += 4;
		
	/* loop over groups */
	done = MB_NO;
	while (index <= buffer_size 
		&& status == MB_SUCCESS
		&& done == MB_NO)
	    {
	    /* look for group start or frame end */
	    skip = 0;
#ifdef DATAINPCBYTEORDER
	    while (index < buffer_size
			&& strncmp(&buffer[index], "GSH$", 4)
			&& strncmp(&buffer[index], "FSH#", 4))
			{
			index++;
			skip++;
			}
	    if (index >= buffer_size
			|| !strncmp(&buffer[index], "FSH#", 4))
			done = MB_YES;
	    else
			index += 4;
#else
	    while (index < buffer_size
			&& strncmp(&buffer[index], "$HSG", 4)
			&& strncmp(&buffer[index], "#HSF", 4))
			{
			index++;
			skip++;
			}
	    if (index >= buffer_size
			|| !strncmp(&buffer[index], "#HSF", 4))
			done = MB_YES;
	    else
			index += 4;
#endif
	    
#ifdef MB_DEBUG
if (skip > 4) fprintf(stderr, "skipped %d bytes in function <%s>\n", skip-4, function_name);
#endif
	    
	    /* deal with group */
	    if (done == MB_NO)
			{
			/* get group size and id */
			mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &byte_count); index += 4;
			mb_get_binary_int(SWAPFLAG, &buffer[index], (int *) &group_id); index += 4;

			/* print debug statements */
			if (verbose >= 5)
			    {
			    fprintf(stderr,"\ndbg5  Group %d of %d bytes to be parsed in MBIO function <%s>\n",
				    group_id, byte_count, function_name);
			    }
#ifdef MB_DEBUG
fprintf(stderr, "Group %d of %d bytes to be parsed in MBIO function <%s>\n",
				    group_id, byte_count, function_name);
#endif
	    
		    /* handle general group */
		    if (group_id == MBSYS_XSE_COM_GROUP_GEN)
				{
				for (i=0;i<byte_count;i++)
				    {
				    if (i<MBSYS_XSE_COMMENT_LENGTH-1)
						store->comment[i] = buffer[index++];
				    }
				store->comment[MIN(byte_count-4, MBSYS_XSE_COMMENT_LENGTH-1)] = '\0';
				}
			}
	    }

	/* print debug statements */
	if (verbose >= 5)
	    {
	    fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
	    fprintf(stderr,"dbg5       comment:             %s\n",store->comment);
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
int mbr_l3xseraw_wr_data(int verbose,void *mbio_ptr,void *store_ptr,int *error)
{
	char	*function_name = "mbr_l3xseraw_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_xse_struct *store;
	FILE	*mbfp;
	int	buffer_size;
	int	write_size;
	char	*buffer;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to store data structure */
	store = (struct mbsys_xse_struct *) store_ptr;
	mbfp = mb_io_ptr->mbfp;
	buffer = mb_io_ptr->hdr_comment;

#ifdef MB_DEBUG
fprintf(stderr, "WRITE KIND: %d\n",store->kind);
#endif

	if (store->kind == MB_DATA_COMMENT)
		{
#ifdef MB_DEBUG
fprintf(stderr, "WRITE COMMMENT\n");
#endif
		status = mbr_l3xseraw_wr_comment(verbose,&buffer_size,buffer,store_ptr,error);
		if ((write_size = fwrite(buffer,1,buffer_size,mbfp)) != buffer_size)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		}
	else if (store->kind == MB_DATA_NAV)
		{
#ifdef MB_DEBUG
fprintf(stderr, "WRITE NAV\n");
#endif
		status = mbr_l3xseraw_wr_nav(verbose,&buffer_size,buffer,store_ptr,error);
		if ((write_size = fwrite(buffer,1,buffer_size,mbfp)) != buffer_size)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		}
	else if (store->kind == MB_DATA_VELOCITY_PROFILE)
		{
#ifdef MB_DEBUG
fprintf(stderr, "WRITE SVP\n");
#endif
		status = mbr_l3xseraw_wr_svp(verbose,&buffer_size,buffer,store_ptr,error);
		if ((write_size = fwrite(buffer,1,buffer_size,mbfp)) != buffer_size)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		}
	else if (store->kind == MB_DATA_PARAMETER)
		{
#ifdef MB_DEBUG
fprintf(stderr, "WRITE SHIP\n");
#endif
		status = mbr_l3xseraw_wr_ship(verbose,&buffer_size,buffer,store_ptr,error);
		if ((write_size = fwrite(buffer,1,buffer_size,mbfp)) != buffer_size)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		}
	else if (store->kind == MB_DATA_DATA)
		{
#ifdef MB_DEBUG
fprintf(stderr, "WRITE MULTIBEAM\n");
#endif
		if (store->mul_frame == MB_YES)
		    {
		    status = mbr_l3xseraw_wr_multibeam(verbose,&buffer_size,buffer,store_ptr,error);
		    if ((write_size = fwrite(buffer,1,buffer_size,mbfp)) != buffer_size)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		    }
#ifdef MB_DEBUG
fprintf(stderr, "WRITE SIDESCAN\n");
#endif
		if (store->sid_frame == MB_YES)
		    {
		    status = mbr_l3xseraw_wr_sidescan(verbose,&buffer_size,buffer,store_ptr,error);
		    if ((write_size = fwrite(buffer,1,buffer_size,mbfp)) != buffer_size)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		    }
		}
	else if (store->kind == MB_DATA_RUN_PARAMETER)
		{
#ifdef MB_DEBUG
fprintf(stderr, "WRITE RUN PARAMETER\n");
#endif
		status = mbr_l3xseraw_wr_seabeam(verbose,&buffer_size,buffer,store_ptr,error);
		if ((write_size = fwrite(buffer,1,buffer_size,mbfp)) != buffer_size)
		    {
		    *error = MB_ERROR_WRITE_FAIL;
		    status = MB_FAILURE;
		    }
		}
	else if (store->kind == MB_DATA_RAW_LINE)
		{
#ifdef MB_DEBUG
fprintf(stderr, "WRITE RAW LINE\n");
#endif
		if (store->rawsize > 0)
		    {
		    if ((write_size = fwrite(store->raw,1,store->rawsize,mbfp)) != store->rawsize)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		    }
		}
	else
		{
#ifdef MB_DEBUG
fprintf(stderr, "WRITE FAILURE BAD KIND\n");
#endif
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_KIND;
		}

	/* print output debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Data record kind in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       kind:       %d\n",store->kind);
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
int mbr_l3xseraw_wr_nav(int verbose,int *buffer_size,char *buffer,void *store_ptr,int *error)
{
	char	*function_name = "mbr_l3xseraw_wr_nav";
	int	status = MB_SUCCESS;
	struct mbsys_xse_struct *store;
	int	index;
	int frame_count;
	int group_count;
	int frame_cnt_index;
	int	group_cnt_index;
	int frame_id;
	int group_id;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
	    fprintf(stderr,"dbg2  Input arguments:\n");
	    fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
	    fprintf(stderr,"dbg2       buffer:     %lu\n",(size_t)buffer);
	    fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
	    }

	/* get pointer to store data structure */
	store = (struct mbsys_xse_struct *) store_ptr;

	/* print debug statements */
	if (verbose >= 5)
	    {
	    fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",function_name);
	    fprintf(stderr,"dbg5       nav_source:          %d\n",store->nav_source);
	    fprintf(stderr,"dbg5       nav_sec:             %u\n",store->nav_sec);
	    fprintf(stderr,"dbg5       nav_usec:            %u\n",store->nav_usec);
	    fprintf(stderr,"dbg5       nav_quality:         %d\n",store->nav_quality);
	    fprintf(stderr,"dbg5       nav_status:          %d\n",store->nav_status);
	    fprintf(stderr,"dbg5       nav_description_len: %d\n",store->nav_description_len);
	    fprintf(stderr,"dbg5       nav_description:     %s\n",store->nav_description);
	    fprintf(stderr,"dbg5       nav_x:               %f\n",store->nav_x);
	    fprintf(stderr,"dbg5       nav_y:               %f\n",store->nav_y);
	    fprintf(stderr,"dbg5       nav_z:               %f\n",store->nav_z);
	    fprintf(stderr,"dbg5       nav_speed_ground:    %f\n",store->nav_speed_ground);
	    fprintf(stderr,"dbg5       nav_course_ground:   %f\n",store->nav_course_ground);
	    fprintf(stderr,"dbg5       nav_speed_water:     %f\n",store->nav_speed_water);
	    fprintf(stderr,"dbg5       nav_course_water:    %f\n",store->nav_course_water);
	    }

	/* get the frame label */
	index = 0;
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH$", 4);
#else
	strncpy(&buffer[index], "$HSF", 4);
#endif
	index += 4;

	/* start the frame byte count, but don't write it to buffer yet */
	/* increment index so we skip the count value in the buffer */
	frame_count = 0;
	frame_cnt_index = index;
	index += 4;
	
	/* get frame time */
	frame_id = MBSYS_XSE_NAV_FRAME;
	mb_put_binary_int(SWAPFLAG, frame_id, &buffer[index]); index += 4;
	mb_put_binary_int(SWAPFLAG, store->nav_source, &buffer[index]); index += 4;
	mb_put_binary_int(SWAPFLAG, store->nav_sec, &buffer[index]); index += 4;
	mb_put_binary_int(SWAPFLAG, store->nav_usec, &buffer[index]); index += 4;
	frame_count += 16;
	
	/*****************************************/
	
	/* write general group */
	if (store->nav_group_general == MB_YES)
		{
		/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get pos group */
		group_id = MBSYS_XSE_NAV_GROUP_GEN;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
		mb_put_binary_int(SWAPFLAG, store->nav_quality, &buffer[index]); index += 4;
		mb_put_binary_int(SWAPFLAG, store->nav_status, &buffer[index]); index += 4;

		/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += store->nav_description_len + 32;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
		}
	
	/*****************************************/
	
	/* write position group */
	if (store->nav_group_position == MB_YES)
		{
		/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get pos group */
		group_id = MBSYS_XSE_NAV_GROUP_POS;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
		mb_put_binary_int(SWAPFLAG, store->nav_description_len, &buffer[index]); index += 4;
		for (i=0;i<store->nav_description_len;i++)
		    {
		    buffer[index] = store->nav_description[i]; index++;
		    }
		mb_put_binary_double(SWAPFLAG, store->nav_x, &buffer[index]); index += 8;
		mb_put_binary_double(SWAPFLAG, store->nav_y, &buffer[index]); index += 8;
		mb_put_binary_double(SWAPFLAG, store->nav_z, &buffer[index]); index += 8;

		/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += store->nav_description_len + 32;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
		}
	
	/*****************************************/
	
	/* write accuracy group */
	if (store->nav_group_accuracy == MB_YES)
		{
		/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get pos group */
		group_id = MBSYS_XSE_NAV_GROUP_ACCURACY;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
		mb_put_binary_short(SWAPFLAG, store->nav_acc_quality, &buffer[index]); index += 2;
		buffer[index] = store->nav_acc_numsatellites; index++;
		mb_put_binary_float(SWAPFLAG, store->nav_acc_horizdilution, &buffer[index]); index += 4;
		mb_put_binary_float(SWAPFLAG, store->nav_acc_diffage, &buffer[index]); index += 4;
		mb_put_binary_int(SWAPFLAG, store->nav_acc_diffref, &buffer[index]); index += 4;

		/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 19;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
		}
	
	/*****************************************/

	/* write motion ground truth group */
	if (store->nav_group_motiongt == MB_YES)
		{
		/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get motion ground truth group */
		group_id = MBSYS_XSE_NAV_GROUP_MOTIONGT;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
		mb_put_binary_double(SWAPFLAG, store->nav_speed_ground, &buffer[index]); index += 8;
		mb_put_binary_double(SWAPFLAG, store->nav_course_ground, &buffer[index]); index += 8;

		/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 20;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
		}
	
	/*****************************************/

	/* write motion through water group */
	if (store->nav_group_motiontw == MB_YES)
		{
		/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;
	
		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;
		
		/* get motion through water group */
		group_id = MBSYS_XSE_NAV_GROUP_MOTIONTW;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
		mb_put_binary_double(SWAPFLAG, store->nav_speed_water, &buffer[index]); index += 8;
		mb_put_binary_double(SWAPFLAG, store->nav_course_water, &buffer[index]); index += 8;
	
		/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 20;
	
		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);
	
		/* add group count to the frame count */
		frame_count += group_count + 12;
		}
	
	/*****************************************/

	/* write track steering group */
	if (store->nav_group_track == MB_YES)
		{
		/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;
	
		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;
		
		/* get motion through water group */
		group_id = MBSYS_XSE_NAV_GROUP_TRACK;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
		mb_put_binary_double(SWAPFLAG, store->nav_trk_offset_track, &buffer[index]); index += 8;
		mb_put_binary_double(SWAPFLAG, store->nav_trk_offset_sol, &buffer[index]); index += 8;
		mb_put_binary_double(SWAPFLAG, store->nav_trk_offset_eol, &buffer[index]); index += 8;
		mb_put_binary_double(SWAPFLAG, store->nav_trk_distance_sol, &buffer[index]); index += 8;
		mb_put_binary_double(SWAPFLAG, store->nav_trk_azimuth_sol, &buffer[index]); index += 8;
		mb_put_binary_double(SWAPFLAG, store->nav_trk_distance_eol, &buffer[index]); index += 8;
		mb_put_binary_double(SWAPFLAG, store->nav_trk_azimuth_eol, &buffer[index]); index += 8;
	
		/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 20;
	
		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);
	
		/* add group count to the frame count */
		frame_count += group_count + 12;
		}
	
	/*****************************************/

	/* write heave roll pitch group */
	if (store->nav_group_hrp == MB_YES)
		{
		/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;
	
		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;
		
		/* get motion through water group */
		group_id = MBSYS_XSE_NAV_GROUP_HRP;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
		mb_put_binary_double(SWAPFLAG, store->nav_hrp_heave, &buffer[index]); index += 8;
		mb_put_binary_double(SWAPFLAG, store->nav_hrp_roll, &buffer[index]); index += 8;
		mb_put_binary_double(SWAPFLAG, store->nav_hrp_pitch, &buffer[index]); index += 8;
	
		/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 20;
	
		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);
	
		/* add group count to the frame count */
		frame_count += group_count + 12;
		}
	
	/*****************************************/

	/* write heave group */
	if (store->nav_group_hrp == MB_YES)
		{
		/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;
	
		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;
		
		/* get motion through water group */
		group_id = MBSYS_XSE_NAV_GROUP_HEAVE;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
		mb_put_binary_double(SWAPFLAG, store->nav_hea_heave, &buffer[index]); index += 8;
	
		/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 20;
	
		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);
	
		/* add group count to the frame count */
		frame_count += group_count + 12;
		}
	
	/*****************************************/

	/* write roll group */
	if (store->nav_group_roll == MB_YES)
		{
		/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;
	
		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;
		
		/* get motion through water group */
		group_id = MBSYS_XSE_NAV_GROUP_ROLL;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
		mb_put_binary_double(SWAPFLAG, store->nav_rol_roll, &buffer[index]); index += 8;
	
		/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 20;
	
		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);
	
		/* add group count to the frame count */
		frame_count += group_count + 12;
		}
	
	/*****************************************/

	/* write pitch group */
	if (store->nav_group_pitch == MB_YES)
		{
		/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;
	
		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;
		
		/* get motion through water group */
		group_id = MBSYS_XSE_NAV_GROUP_PITCH;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
		mb_put_binary_double(SWAPFLAG, store->nav_pit_pitch, &buffer[index]); index += 8;
	
		/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 20;
	
		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);
	
		/* add group count to the frame count */
		frame_count += group_count + 12;
		}
	
	/*****************************************/

	/* write heading group */
	if (store->nav_group_heading == MB_YES)
		{
		/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;
	
		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;
		
		/* get motion through water group */
		group_id = MBSYS_XSE_NAV_GROUP_HEADING;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
		mb_put_binary_double(SWAPFLAG, store->nav_hdg_heading, &buffer[index]); index += 8;
	
		/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 12;
	
		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);
	
		/* add group count to the frame count */
		frame_count += group_count + 12;
		}
	
	/*****************************************/

	/* write speed log group */
	if (store->nav_group_log == MB_YES)
		{
		/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;
	
		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;
		
		/* get motion through water group */
		group_id = MBSYS_XSE_NAV_GROUP_LOG;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
		mb_put_binary_double(SWAPFLAG, store->nav_log_speed, &buffer[index]); index += 8;
	
		/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 12;
	
		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);
	
		/* add group count to the frame count */
		frame_count += group_count + 12;
		}
	
	/*****************************************/

	/* write gps altitude group */
	if (store->nav_group_gps == MB_YES)
		{
		/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;
	
		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;
		
		/* get motion through water group */
		group_id = MBSYS_XSE_NAV_GROUP_GPS;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
		mb_put_binary_float(SWAPFLAG, store->nav_gps_altitude, &buffer[index]); index += 4;
		mb_put_binary_float(SWAPFLAG, store->nav_gps_geoidalseparation, &buffer[index]); index += 4;
	
		/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 12;
	
		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);
	
		/* add group count to the frame count */
		frame_count += group_count + 12;
		}
	
	/*****************************************/
	
	/* get end of frame label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH#", 4);
#else
	strncpy(&buffer[index], "#HSF", 4);
#endif
	index += 4;	

	/* go back and fill in frame byte count */
	mb_put_binary_int(SWAPFLAG, frame_count, &buffer[frame_cnt_index]);
	
	/* set buffer size */
	*buffer_size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2       buffer_size:%d\n",*buffer_size);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_l3xseraw_wr_svp(int verbose,int *buffer_size,char *buffer,void *store_ptr,int *error)
{
	char	*function_name = "mbr_l3xseraw_wr_svp";
	int	status = MB_SUCCESS;
	struct mbsys_xse_struct *store;
	int	index;
	int frame_count;
	int group_count;
	int frame_cnt_index;
	int group_cnt_index;
	int frame_id;
	int group_id;
	int write_conductivity = MB_NO;
	int write_salinity = MB_NO;
	int write_temperature = MB_NO;
	int write_pressure = MB_NO;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
	    fprintf(stderr,"dbg2  Input arguments:\n");
	    fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
	    fprintf(stderr,"dbg2       buffer:     %lu\n",(size_t)buffer);
	    fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
	    }

	/* get pointer to store data structure */
	store = (struct mbsys_xse_struct *) store_ptr;

	/* print debug statements */
	if (verbose >= 5)
	    {
	    fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",function_name);
	    fprintf(stderr,"dbg5       svp_source:          %d\n",store->svp_source);
	    fprintf(stderr,"dbg5       svp_sec:             %u\n",store->svp_sec);
	    fprintf(stderr,"dbg5       svp_usec:            %u\n",store->svp_usec);
	    fprintf(stderr,"dbg5       svp_nsvp:            %d\n",store->svp_nsvp);
	    fprintf(stderr,"dbg5       svp_nctd:            %d\n",store->svp_nctd);
	    fprintf(stderr,"dbg5       svp_ssv:             %f\n",store->svp_ssv);
	    for (i=0;i<store->svp_nsvp;i++)
		fprintf(stderr,"dbg5       svp[%d]:	        %f %f\n",
		    i, store->svp_depth[i], store->svp_velocity[i]);
	    for (i=0;i<store->svp_nctd;i++)
		fprintf(stderr,"dbg5       cstd[%d]:        %f %f %f %f\n",
		    i, store->svp_conductivity[i], store->svp_salinity[i], 
		    store->svp_temperature[i], store->svp_pressure[i]);
	    }

	/* get the frame label */
	index = 0;
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH$", 4);
#else
	strncpy(&buffer[index], "$HSF", 4);
#endif
	index += 4;

	/* start the frame byte count, but don't write it to buffer yet */
	/* increment index so we skip the count value in the buffer */
	frame_count = 0;
	frame_cnt_index = index;
	index += 4;
	
	/* get frame time */
	frame_id = MBSYS_XSE_SVP_FRAME;
	mb_put_binary_int(SWAPFLAG, frame_id, &buffer[index]); index += 4;
	mb_put_binary_int(SWAPFLAG, store->svp_source, &buffer[index]); index += 4;
	mb_put_binary_int(SWAPFLAG, store->svp_sec, &buffer[index]); index += 4;
	mb_put_binary_int(SWAPFLAG, store->svp_usec, &buffer[index]); index += 4;
	frame_count += 16;

	/* get depth and velocity groups */
	if (store->svp_nsvp > 0)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;

	    /* start the group byte count, but don't write it to buffer yet */
	    /* mark the byte count spot in the buffer and increment index so we skip it */
	    group_count = 0;
	    group_cnt_index = index;
	    index += 4;
	    
	    /* get depth array */
	    group_id = MBSYS_XSE_SVP_GROUP_DEPTH;
	    mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->svp_nsvp, &buffer[index]); index += 4;
	    for (i=0;i<store->svp_nsvp;i++)
		{
		mb_put_binary_double(SWAPFLAG, store->svp_depth[i], &buffer[index]); index += 8;
		}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;
	    group_count += 8 + store->svp_nsvp*8;

	    /* go back and fill in group byte count */
	    mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	    /* add group count to the frame count */
	    frame_count += group_count + 12;

	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;

	    /* start the group byte count, but don't write it to buffer yet */
	    /* mark the byte count spot in the buffer and increment index so we skip it */
	    group_count = 0;
	    group_cnt_index = index;
	    index += 4;
	    
	    /* get depth array */
	    group_id = MBSYS_XSE_SVP_GROUP_VELOCITY;
	    mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->svp_nsvp, &buffer[index]); index += 4;
	    for (i=0;i<store->svp_nsvp;i++)
			{
			mb_put_binary_double(SWAPFLAG, store->svp_velocity[i], &buffer[index]); index += 8;
			}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;
	    group_count += 8 + store->svp_nsvp*8;		

	    /* go back and fill in group byte count */
	    mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	    /* add group count to the frame count */
	    frame_count += group_count + 12;
	    }

	/* figure out which ctd groups are nonzero */
	if (store->svp_nctd > 0)
	    {
	    for (i=0;i<store->svp_nctd;i++)
		{
		if (store->svp_conductivity[i] != 0.0)
			write_conductivity = MB_YES;
		if (store->svp_salinity[i] != 0.0)
			write_salinity = MB_YES;
		if (store->svp_temperature[i] != 0.0)
			write_temperature = MB_YES;
		if (store->svp_pressure[i] != 0.0)
			write_pressure = MB_YES;
		}
	    }

	/* get conductivity group */
	if (store->svp_nctd > 0
		&& write_conductivity == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;

	    /* start the group byte count, but don't write it to buffer yet */
	    /* mark the byte count spot in the buffer and increment index so we skip it */
	    group_count = 0;
	    group_cnt_index = index;
	    index += 4;
	    
	    /* get conductivity array */
	    group_id = MBSYS_XSE_SVP_GROUP_CONDUCTIVITY;
	    mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->svp_nctd, &buffer[index]); index += 4;
	    for (i=0;i<store->svp_nctd;i++)
			{
			mb_put_binary_double(SWAPFLAG, store->svp_conductivity[i], &buffer[index]); index += 8;
			}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;
	    group_count += 8 + store->svp_nctd*8;

	    /* go back and fill in group byte count */
	    mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	    /* add group count to the frame count */
	    frame_count += group_count + 12;
	    }

	/* get salinity group */
	if (store->svp_nctd > 0
		&& write_salinity == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* start the group byte count, but don't write it to buffer yet */
	    /* mark the byte count spot in the buffer and increment index so we skip it */
	    group_count = 0;
	    group_cnt_index = index;
	    index += 4;
	    
	    /* get salinity array */
	    group_id = MBSYS_XSE_SVP_GROUP_SALINITY;
	    mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->svp_nctd, &buffer[index]); index += 4;
	    for (i=0;i<store->svp_nctd;i++)
			{
			mb_put_binary_double(SWAPFLAG, store->svp_salinity[i], &buffer[index]); index += 8;
			}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;
	    group_count += 8 + store->svp_nctd*8;

	    /* go back and fill in group byte count */
	    mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	    /* add group count to the frame count */
	    frame_count += group_count + 12;
	    }

	/* get temperature group */
	if (store->svp_nctd > 0
		&& write_temperature == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;

	    /* start the group byte count, but don't write it to buffer yet */
	    /* mark the byte count spot in the buffer and increment index so we skip it */
	    group_count = 0;
	    group_cnt_index = index;
	    index += 4;
	    
	    /* get temperature array */
	    group_id = MBSYS_XSE_SVP_GROUP_TEMP;
	    mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->svp_nctd, &buffer[index]); index += 4;
	    for (i=0;i<store->svp_nctd;i++)
			{
			mb_put_binary_double(SWAPFLAG, store->svp_temperature[i], &buffer[index]); index += 8;
			}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;
	    group_count += 8 + store->svp_nctd*8;

	    /* go back and fill in group byte count */
	    mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	    /* add group count to the frame count */
	    frame_count += group_count + 12;
	    }

	/* get pressure group */
	if (store->svp_nctd > 0
		&& write_pressure == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* start the group byte count, but don't write it to buffer yet */
	    /* mark the byte count spot in the buffer and increment index so we skip it */
	    group_count = 0;
	    group_cnt_index = index;
	    index += 4;
	    
	    /* get pressure array */
	    group_id = MBSYS_XSE_SVP_GROUP_PRESSURE;
	    mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->svp_nctd, &buffer[index]); index += 4;
	    for (i=0;i<store->svp_nctd;i++)
			{
			mb_put_binary_double(SWAPFLAG, store->svp_pressure[i], &buffer[index]); index += 8;
			}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;
	    group_count += 8 + store->svp_nctd*8;
	    
	    /* go back and fill in group byte count */
	    mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	    /* add group count to the frame count */
	    frame_count += group_count + 12;
	    }
	    
	if (store->svp_ssv > 0.0)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* start the group byte count, but don't write it to buffer yet */
	    /* mark the byte count spot in the buffer and increment index so we skip it */
	    group_count = 0;
	    group_cnt_index = index;
	    index += 4;
	    
	    /* get ssv */
	    group_id = MBSYS_XSE_SVP_GROUP_SSV;
	    mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	    mb_put_binary_double(SWAPFLAG, store->svp_ssv, &buffer[index]); index += 8;

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;
	    group_count += 12;

	    /* go back and fill in group byte count */
	    mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	    /* add group count to the frame count */
	    frame_count += group_count + 12;
	    }

	/* get end of frame label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH#", 4);
#else
	strncpy(&buffer[index], "#HSF", 4);
#endif
	index += 4;	

	/* go back and fill in frame byte count */
	mb_put_binary_int(SWAPFLAG, frame_count, &buffer[frame_cnt_index]);
 	
	/* set buffer size */	
	*buffer_size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2       buffer_size:%d\n",*buffer_size);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_l3xseraw_wr_ship(int verbose,int *buffer_size,char *buffer,void *store_ptr,int *error)
{
	char	*function_name = "mbr_l3xseraw_wr_ship";
	int	status = MB_SUCCESS;
	struct mbsys_xse_struct *store;
	int	index;
	int frame_count;
	int group_count;
	int frame_cnt_index;
	int group_cnt_index;
	int frame_id;
	int group_id;
	int	nchar;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
	    fprintf(stderr,"dbg2  Input arguments:\n");
	    fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
	    fprintf(stderr,"dbg2       buffer:     %lu\n",(size_t)buffer);
	    fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
	    }

	/* get pointer to store data structure */
	store = (struct mbsys_xse_struct *) store_ptr;

	/* print debug statements */
	if (verbose >= 5)
	    {
	    fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",function_name);
	    fprintf(stderr,"dbg5       par_source:          %d\n",store->par_source);
	    fprintf(stderr,"dbg5       par_sec:             %u\n",store->par_sec);
	    fprintf(stderr,"dbg5       par_usec:            %u\n",store->par_usec);
	    fprintf(stderr,"dbg5       par_ship_name:       %s\n",store->par_ship_name);
	    fprintf(stderr,"dbg5       par_ship_length:     %f\n",store->par_ship_length);
	    fprintf(stderr,"dbg5       par_ship_beam:       %f\n",store->par_ship_beam);
	    fprintf(stderr,"dbg5       par_ship_draft:      %f\n",store->par_ship_draft);
	    fprintf(stderr,"dbg5       par_ship_height:     %f\n",store->par_ship_height);
	    fprintf(stderr,"dbg5       par_ship_displacement: %f\n",store->par_ship_displacement);
	    fprintf(stderr,"dbg5       par_ship_weight:     %f\n",store->par_ship_weight);
	    for (i=0;i<store->par_ship_nsensor;i++)
		{
		fprintf(stderr,"dbg5       par_ship_sensor_id[%d]:        %d\n",i,store->par_ship_sensor_id[i]);
		fprintf(stderr,"dbg5       par_ship_sensor_type[%d]:      %d\n",i,store->par_ship_sensor_type[i]);
		fprintf(stderr,"dbg5       par_ship_sensor_frequency[%d]: %d\n",i,store->par_ship_sensor_frequency[i]);
		}
	    fprintf(stderr,"dbg5       par_parameter:       %d\n",store->par_parameter);
	    fprintf(stderr,"dbg5       par_roll_bias:       %f\n",store->par_roll_bias);
	    fprintf(stderr,"dbg5       par_pitch_bias:      %f\n",store->par_pitch_bias);
	    fprintf(stderr,"dbg5       par_heading_bias:    %f\n",store->par_heading_bias);
	    fprintf(stderr,"dbg5       par_time_delay:      %f\n",store->par_time_delay);
	    fprintf(stderr,"dbg5       par_trans_x_port:    %f\n",store->par_trans_x_port);
	    fprintf(stderr,"dbg5       par_trans_y_port:    %f\n",store->par_trans_y_port);
	    fprintf(stderr,"dbg5       par_trans_z_port:    %f\n",store->par_trans_z_port);
	    fprintf(stderr,"dbg5       par_trans_x_stbd:    %f\n",store->par_trans_x_stbd);
	    fprintf(stderr,"dbg5       par_trans_y_stbd:    %f\n",store->par_trans_y_stbd);
	    fprintf(stderr,"dbg5       par_trans_z_stbd:    %f\n",store->par_trans_z_stbd);
	    fprintf(stderr,"dbg5       par_trans_err_port:  %f\n",store->par_trans_err_port);
	    fprintf(stderr,"dbg5       par_trans_err_stbd:  %f\n",store->par_trans_err_stbd);
	    fprintf(stderr,"dbg5       par_nav_x:           %f\n",store->par_nav_x);
	    fprintf(stderr,"dbg5       par_nav_y:           %f\n",store->par_nav_y);
	    fprintf(stderr,"dbg5       par_nav_z:           %f\n",store->par_nav_z);
	    fprintf(stderr,"dbg5       par_hrp_x:           %f\n",store->par_hrp_x);
	    fprintf(stderr,"dbg5       par_hrp_y:           %f\n",store->par_hrp_y);
	    fprintf(stderr,"dbg5       par_hrp_z:           %f\n",store->par_hrp_z);
	    fprintf(stderr,"dbg5       par_navigationandmotion: %d\n",store->par_navigationandmotion);
	    fprintf(stderr,"dbg5       par_nam_roll_bias:       %f\n",store->par_nam_roll_bias);
	    fprintf(stderr,"dbg5       par_nam_pitch_bias:      %f\n",store->par_nam_pitch_bias);
	    fprintf(stderr,"dbg5       par_nam_heave_bias:      %f\n",store->par_nam_heave_bias);
	    fprintf(stderr,"dbg5       par_nam_heading_bias:    %f\n",store->par_nam_heading_bias);
	    fprintf(stderr,"dbg5       par_nam_time_delay:      %f\n",store->par_nam_time_delay);
	    fprintf(stderr,"dbg5       par_nam_nav_x:           %f\n",store->par_nam_nav_x);
	    fprintf(stderr,"dbg5       par_nam_nav_y:           %f\n",store->par_nam_nav_y);
	    fprintf(stderr,"dbg5       par_nam_nav_z:           %f\n",store->par_nam_nav_z);
	    fprintf(stderr,"dbg5       par_nam_hrp_x:           %f\n",store->par_nam_hrp_x);
	    fprintf(stderr,"dbg5       par_nam_hrp_y:           %f\n",store->par_nam_hrp_y);
	    fprintf(stderr,"dbg5       par_nam_hrp_z:           %f\n",store->par_nam_hrp_z);
	    fprintf(stderr,"dbg5       par_xdr_num_transducer:  %d\n",store->par_xdr_num_transducer);
	    fprintf(stderr,"dbg5       # sensor xducer freq side roll pitch azi dist\n");
	    for (i=0;i<store->par_xdr_num_transducer;i++)
	    	fprintf(stderr,"dbg5       %d %d %d %d %d %f %f %f %f\n", i, store->par_xdr_sensorid[i], store->par_xdr_transducer[i], 
	    							store->par_xdr_frequency[i], store->par_xdr_side[i], 
								store->par_xdr_mountingroll[i], store->par_xdr_mountingpitch[i], 
								store->par_xdr_mountingazimuth[i], store->par_xdr_mountingdistance[i]);
	    fprintf(stderr,"dbg5       # x y z roll pitch azimuth\n");
	    for (i=0;i<store->par_xdr_num_transducer;i++)
	    	fprintf(stderr,"dbg5       %d %f %f %f %f %f %f\n", i, store->par_xdr_x[i], store->par_xdr_y[i], 
	    							store->par_xdr_z[i], store->par_xdr_roll[i], 
								store->par_xdr_pitch[i], store->par_xdr_azimuth[i]);
	    fprintf(stderr,"dbg5       par_xdx_num_transducer:  %d\n",store->par_xdx_num_transducer);
	    fprintf(stderr,"dbg5       # roll pitch azimuth\n");
	    for (i=0;i<store->par_xdx_num_transducer;i++)
	    	fprintf(stderr,"dbg5       %d %d %d %d\n", i, store->par_xdx_roll[i], store->par_xdx_pitch[i], store->par_xdx_azimuth[i]);
	    }

	/* get the frame label */
	index = 0;
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH$", 4);
#else
	strncpy(&buffer[index], "$HSF", 4);
#endif
	index += 4;

	/* start the frame byte count, but don't write it to buffer yet */
	/* increment index so we skip the count value in the buffer */
	frame_count = 0;
	frame_cnt_index = index;
	index += 4;
	
	/* get frame time */
	frame_id = MBSYS_XSE_SHP_FRAME;
	mb_put_binary_int(SWAPFLAG, frame_id, &buffer[index]); index += 4;
	mb_put_binary_int(SWAPFLAG, store->par_source, &buffer[index]); index += 4;
	mb_put_binary_int(SWAPFLAG, store->par_sec, &buffer[index]); index += 4;
	mb_put_binary_int(SWAPFLAG, store->par_usec, &buffer[index]); index += 4;
	frame_count += 16;
	
	/*****************************************/

	/* get group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH$", 4);
#else
	strncpy(&buffer[index], "$HSG", 4);
#endif
	index += 4;

	/* start the group byte count, but don't write it to buffer yet */
	/* mark the byte count spot in the buffer and increment index so we skip it */
	group_count = 0;
	group_cnt_index = index;
	index += 4;
	
	/* get general group */
	group_id = MBSYS_XSE_SHP_GROUP_GEN;
	mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	nchar = strlen(store->par_ship_name);
	mb_put_binary_int(SWAPFLAG, nchar, &buffer[index]); index += 4;
	for (i=0;i<nchar;i++)
		{
		buffer[index] = store->par_ship_name[i]; index++;
		}
	mb_put_binary_double(SWAPFLAG, store->par_ship_length, &buffer[index]); index += 8;
	mb_put_binary_double(SWAPFLAG, store->par_ship_beam, &buffer[index]); index += 8;
	mb_put_binary_double(SWAPFLAG, store->par_ship_draft, &buffer[index]); index += 8;
	mb_put_binary_double(SWAPFLAG, store->par_ship_height, &buffer[index]); index += 8;
	mb_put_binary_double(SWAPFLAG, store->par_ship_displacement, &buffer[index]); index += 8;
	mb_put_binary_double(SWAPFLAG, store->par_ship_weight, &buffer[index]); index += 8;

	/* get end of group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH#", 4);
#else
	strncpy(&buffer[index], "#HSG", 4);
#endif
	index += 4;
	group_count += nchar + 56;

	/* go back and fill in group byte count */
	mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	/* add group count to the frame count */
	frame_count += group_count + 12;
	
	/*****************************************/

	if (store->par_ship_nsensor > 0)
		{
		/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get sensors group */
		group_id = MBSYS_XSE_SHP_GROUP_SENSORS;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
		mb_put_binary_int(SWAPFLAG, store->par_ship_nsensor, &buffer[index]); index += 4;
		for (i=0;i<store->par_ship_nsensor;i++)
			{
			mb_put_binary_int(SWAPFLAG, store->par_ship_sensor_id[i], &buffer[index]); index += 4;
			}
		for (i=0;i<store->par_ship_nsensor;i++)
			{
			mb_put_binary_int(SWAPFLAG, store->par_ship_sensor_type[i], &buffer[index]); index += 4;
			}
		for (i=0;i<store->par_ship_nsensor;i++)
			{
			mb_put_binary_int(SWAPFLAG, store->par_ship_sensor_frequency[i], &buffer[index]); index += 4;
			}

		/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 12 * store->par_ship_nsensor + 8;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
		}
	
	/*****************************************/

	if (store->par_parameter == MB_YES)
		{
		/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get parameter group */
		group_id = MBSYS_XSE_SHP_GROUP_PARAMETER;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_roll_bias, &buffer[index]); index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_pitch_bias, &buffer[index]); index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_heading_bias, &buffer[index]); index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_time_delay, &buffer[index]); index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_trans_x_port, &buffer[index]); index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_trans_y_port, &buffer[index]); index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_trans_z_port, &buffer[index]); index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_trans_x_stbd, &buffer[index]); index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_trans_y_stbd, &buffer[index]); index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_trans_z_stbd, &buffer[index]); index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_trans_err_port, &buffer[index]); index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_trans_err_stbd, &buffer[index]); index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_nav_x, &buffer[index]); index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_nav_y, &buffer[index]); index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_nav_z, &buffer[index]); index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_hrp_x, &buffer[index]); index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_hrp_y, &buffer[index]); index += 4;
		mb_put_binary_float(SWAPFLAG, store->par_hrp_z, &buffer[index]); index += 4;

		/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 76;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
		}
	
	/*****************************************/

	if (store->par_navigationandmotion == MB_YES)
		{
		/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get parameter group */
		group_id = MBSYS_XSE_SHP_GROUP_NAVIGATIONANDMOTION;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
		mb_put_binary_double(SWAPFLAG, store->par_nam_roll_bias, &buffer[index]); index += 8;
		mb_put_binary_double(SWAPFLAG, store->par_nam_pitch_bias, &buffer[index]); index += 8;
		mb_put_binary_double(SWAPFLAG, store->par_nam_heave_bias, &buffer[index]); index += 8;
		mb_put_binary_double(SWAPFLAG, store->par_nam_heading_bias, &buffer[index]); index += 8;
		mb_put_binary_double(SWAPFLAG, store->par_nam_time_delay, &buffer[index]); index += 8;
		mb_put_binary_double(SWAPFLAG, store->par_nam_nav_x, &buffer[index]); index += 8;
		mb_put_binary_double(SWAPFLAG, store->par_nam_nav_y, &buffer[index]); index += 8;
		mb_put_binary_double(SWAPFLAG, store->par_nam_nav_z, &buffer[index]); index += 8;
		mb_put_binary_double(SWAPFLAG, store->par_nam_hrp_x, &buffer[index]); index += 8;
		mb_put_binary_double(SWAPFLAG, store->par_nam_hrp_y, &buffer[index]); index += 8;
		mb_put_binary_double(SWAPFLAG, store->par_nam_hrp_z, &buffer[index]); index += 8;

		/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 92;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
		}
	
	/*****************************************/

	if (store->par_xdr_num_transducer >0)
		{
		/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get parameter group */
		group_id = MBSYS_XSE_SHP_GROUP_TRANSDUCER;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
		mb_put_binary_int(SWAPFLAG, store->par_xdr_num_transducer, &buffer[index]); index += 4;
		for (i=0;i<store->par_xdr_num_transducer;i++)
			{
			mb_put_binary_int(SWAPFLAG, store->par_xdr_sensorid[i], &buffer[index]); index += 4;
			mb_put_binary_int(SWAPFLAG, store->par_xdr_frequency[i], &buffer[index]); index += 4;
			buffer[index] =  store->par_xdr_transducer[i]; index++;
			buffer[index] =  store->par_xdr_side[i]; index++;
			mb_put_binary_double(SWAPFLAG, store->par_xdr_mountingroll[i], &buffer[index]); index += 8;
			mb_put_binary_double(SWAPFLAG, store->par_xdr_mountingpitch[i], &buffer[index]); index += 8;
			mb_put_binary_double(SWAPFLAG, store->par_xdr_mountingazimuth[i], &buffer[index]); index += 8;
			mb_put_binary_double(SWAPFLAG, store->par_xdr_mountingdistance[i], &buffer[index]); index += 8;
			mb_put_binary_double(SWAPFLAG, store->par_xdr_x[i], &buffer[index]); index += 8;
			mb_put_binary_double(SWAPFLAG, store->par_xdr_y[i], &buffer[index]); index += 8;
			mb_put_binary_double(SWAPFLAG, store->par_xdr_z[i], &buffer[index]); index += 8;
			mb_put_binary_double(SWAPFLAG, store->par_xdr_roll[i], &buffer[index]); index += 8;
			mb_put_binary_double(SWAPFLAG, store->par_xdr_pitch[i], &buffer[index]); index += 8;
			mb_put_binary_double(SWAPFLAG, store->par_xdr_azimuth[i], &buffer[index]); index += 8;
			}

		/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 8 + store->par_xdr_num_transducer * 90;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
		}
	
	/*****************************************/

	if (store->par_xdx_num_transducer >0)
		{
		/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get parameter group */
		group_id = MBSYS_XSE_SHP_GROUP_TRANSDUCEREXTENDED;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
		mb_put_binary_int(SWAPFLAG, store->par_xdx_num_transducer, &buffer[index]); index += 4;
		for (i=0;i<store->par_xdx_num_transducer;i++)
			{
			buffer[index] = store->par_xdx_roll[i]; index++;
			buffer[index] = store->par_xdx_pitch[i]; index++;
			buffer[index] = store->par_xdx_azimuth[i]; index++;
			for (j=0;j<48;j++)
				{
				buffer[index] = 0; index++;
				}
			}

		/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 8 + store->par_xdx_num_transducer * 51;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
		}
	
	/*****************************************/

	/* get end of frame label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH#", 4);
#else
	strncpy(&buffer[index], "#HSF", 4);
#endif
	index += 4;	

	/* go back and fill in frame byte count */
	mb_put_binary_int(SWAPFLAG, frame_count, &buffer[frame_cnt_index]);
        
	/* set buffer size */
	*buffer_size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2       buffer_size:%d\n",*buffer_size);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_l3xseraw_wr_multibeam(int verbose,int *buffer_size,char *buffer,void *store_ptr,int *error)
{
	char	*function_name = "mbr_l3xseraw_wr_multibeam";
	int	status = MB_SUCCESS;
	struct mbsys_xse_struct *store;
	int	index;
	int frame_count;
	int group_count;
	int frame_cnt_index;
	int group_cnt_index;
	int frame_id;
	int group_id;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
	    fprintf(stderr,"dbg2  Input arguments:\n");
	    fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
	    fprintf(stderr,"dbg2       buffer:     %lu\n",(size_t)buffer);
	    fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
	    }

	/* get pointer to store data structure */
	store = (struct mbsys_xse_struct *) store_ptr;

	/* print debug statements */
	if (verbose >= 5)
	    {
	    fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",function_name);
	    fprintf(stderr,"dbg5       mul_source:          %d\n",store->mul_source);
	    fprintf(stderr,"dbg5       mul_sec:             %u\n",store->mul_sec);
	    fprintf(stderr,"dbg5       mul_usec:            %u\n",store->mul_usec);
	    fprintf(stderr,"dbg5       mul_ping:            %d\n",store->mul_ping);
	    fprintf(stderr,"dbg5       mul_frequency:       %f\n",store->mul_frequency);
	    fprintf(stderr,"dbg5       mul_pulse:           %f\n",store->mul_pulse);
	    fprintf(stderr,"dbg5       mul_power:           %f\n",store->mul_power);
	    fprintf(stderr,"dbg5       mul_bandwidth:       %f\n",store->mul_bandwidth);
	    fprintf(stderr,"dbg5       mul_sample:          %f\n",store->mul_sample);
	    fprintf(stderr,"dbg5       mul_swath:           %f\n",store->mul_swath);
	    fprintf(stderr,"dbg5       mul_group_beam:      %d\n",store->mul_group_beam);
	    fprintf(stderr,"dbg5       mul_group_tt:        %d\n",store->mul_group_tt);
	    fprintf(stderr,"dbg5       mul_group_quality:   %d\n",store->mul_group_quality);
	    fprintf(stderr,"dbg5       mul_group_amp:       %d\n",store->mul_group_amp);
	    fprintf(stderr,"dbg5       mul_group_delay:     %d\n",store->mul_group_delay);
	    fprintf(stderr,"dbg5       mul_group_lateral:   %d\n",store->mul_group_lateral);
	    fprintf(stderr,"dbg5       mul_group_along:     %d\n",store->mul_group_along);
	    fprintf(stderr,"dbg5       mul_group_depth:     %d\n",store->mul_group_depth);
	    fprintf(stderr,"dbg5       mul_group_angle:     %d\n",store->mul_group_angle);
	    fprintf(stderr,"dbg5       mul_group_heave:     %d\n",store->mul_group_heave);
	    fprintf(stderr,"dbg5       mul_group_roll:      %d\n",store->mul_group_roll);
	    fprintf(stderr,"dbg5       mul_group_pitch:     %d\n",store->mul_group_pitch);
	    fprintf(stderr,"dbg5       mul_num_beams:       %d\n",store->mul_num_beams);
	    for (i=0;i<store->mul_num_beams;i++)
		fprintf(stderr,"dbg5       beam[%d]: %3d %7.2f %7.2f %7.2f %3d %3d %6.3f %6.2f %5.3f %5.2f %6.2f %6.2f\n",
		    i, store->beams[i].beam, 
		    store->beams[i].lateral, 
		    store->beams[i].along, 
		    store->beams[i].depth, 
		    store->beams[i].amplitude, 
		    store->beams[i].quality, 
		    store->beams[i].tt, 
		    store->beams[i].angle, 
		    store->beams[i].delay, 
		    store->beams[i].heave, 
		    store->beams[i].roll, 
		    store->beams[i].pitch);
	    }

	/* get the frame label */
	index = 0;
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH$", 4);
#else
	strncpy(&buffer[index], "$HSF", 4);
#endif
	index += 4;

	/* start the frame byte count, but don't write it to buffer yet */
	/* increment index so we skip the count value in the buffer */
	frame_count = 0;
	frame_cnt_index = index;
	index += 4;
	
	/* get frame time */
	frame_id = MBSYS_XSE_MBM_FRAME;
	mb_put_binary_int(SWAPFLAG, frame_id, &buffer[index]); index += 4;
	mb_put_binary_int(SWAPFLAG, store->mul_source, &buffer[index]); index += 4;
	mb_put_binary_int(SWAPFLAG, store->mul_sec, &buffer[index]); index += 4;
	mb_put_binary_int(SWAPFLAG, store->mul_usec, &buffer[index]); index += 4;
	frame_count += 16;

	/* get group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH$", 4);
#else
	strncpy(&buffer[index], "$HSG", 4);
#endif
	index += 4;

	/* start the group byte count, but don't write it to buffer yet */
	/* mark the byte count spot in the buffer and increment index so we skip it */
	group_count = 0;
	group_cnt_index = index;
	index += 4;
	
	/* get general group */
	group_id = MBSYS_XSE_MBM_GROUP_GEN;
	mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	mb_put_binary_int(SWAPFLAG, store->mul_ping, &buffer[index]); index += 4;
	mb_put_binary_float(SWAPFLAG, store->mul_frequency, &buffer[index]); index += 4;
	mb_put_binary_float(SWAPFLAG, store->mul_pulse, &buffer[index]); index += 4;
	mb_put_binary_float(SWAPFLAG, store->mul_power, &buffer[index]); index += 4;
	mb_put_binary_float(SWAPFLAG, store->mul_bandwidth, &buffer[index]); index += 4;
	mb_put_binary_float(SWAPFLAG, store->mul_sample, &buffer[index]); index += 4;
	mb_put_binary_float(SWAPFLAG, store->mul_swath, &buffer[index]); index += 4;

	/* get end of group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH#", 4);
#else
	strncpy(&buffer[index], "#HSG", 4);
#endif
	index += 4;
	group_count += 32;

	/* go back and fill in group byte count */
	mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	/* add group count to the frame count */
	frame_count += group_count + 12;

	/* get beam groups */
	if (store->mul_group_beam == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;

	    /* start the group byte count, but don't write it to buffer yet */
	    /* mark the byte count spot in the buffer and increment index so we skip it */
	    group_count = 0;
	    group_cnt_index = index;
	    index += 4;
	    
	    /* get beam array */
	    group_id = MBSYS_XSE_MBM_GROUP_BEAM;
	    mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]); index += 4;
	    for (i=0;i<store->mul_num_beams;i++)
			{
			mb_put_binary_short(SWAPFLAG, store->beams[i].beam, &buffer[index]); index += 2;
			}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;
	    group_count += 8 + store->mul_num_beams*2;

	    /* go back and fill in group byte count */
	    mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	    /* add group count to the frame count */
	    frame_count += group_count + 12;
	    }

	/* get tt groups */
	if (store->mul_group_tt == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* start the group byte count, but don't write it to buffer yet */
	    /* mark the byte count spot in the buffer and increment index so we skip it */
	    group_count = 0;
	    group_cnt_index = index;
	    index += 4;
	    
	    /* get tt array */
	    group_id = MBSYS_XSE_MBM_GROUP_TT;
	    mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]); index += 4;
	    for (i=0;i<store->mul_num_beams;i++)
			{
			mb_put_binary_double(SWAPFLAG, store->beams[i].tt, &buffer[index]); index += 8;
			}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;
	    group_count += 8 + store->mul_num_beams*8;

	    /* go back and fill in group byte count */
	    mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	    /* add group count to the frame count */
	    frame_count += group_count + 12;
	    }

	/* get quality groups */
	if (store->mul_group_quality == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* start the group byte count, but don't write it to buffer yet */
	    /* mark the byte count spot in the buffer and increment index so we skip it */
	    group_count = 0;
	    group_cnt_index = index;
	    index += 4;
	    
	    /* get quality array */
	    group_id = MBSYS_XSE_MBM_GROUP_QUALITY;
	    mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]); index += 4;
	    for (i=0;i<store->mul_num_beams;i++)
			{
			buffer[index] = store->beams[i].quality; index++;		    
			}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += sizeof(int);
	    group_count += 8 + store->mul_num_beams;

	    /* go back and fill in group byte count */
	    mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	    /* add group count to the frame count */
	    frame_count += group_count + 12;
	    }

	/* get amplitude groups */
	if (store->mul_group_amp == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* start the group byte count, but don't write it to buffer yet */
	    /* mark the byte count spot in the buffer and increment index so we skip it */
	    group_count = 0;
	    group_cnt_index = index;
	    index += 4;
	    
	    /* get amplitude array */
	    group_id = MBSYS_XSE_MBM_GROUP_AMP;
	    mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]); index += 4;
	    for (i=0;i<store->mul_num_beams;i++)
			{
			mb_put_binary_short(SWAPFLAG, store->beams[i].amplitude, &buffer[index]); index += 2;
			}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;
	    group_count += 8 + store->mul_num_beams*2;

	    /* go back and fill in group byte count */
	    mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	    /* add group count to the frame count */
	    frame_count += group_count + 12;
	    }

	/* get delay groups */
	if (store->mul_group_delay == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* start the group byte count, but don't write it to buffer yet */
	    /* mark the byte count spot in the buffer and increment index so we skip it */
	    group_count = 0;
	    group_cnt_index = index;
	    index += 4;
	    
	    /* get delay array */
	    group_id = MBSYS_XSE_MBM_GROUP_DELAY;
	    mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]); index += 4;
	    for (i=0;i<store->mul_num_beams;i++)
			{
			mb_put_binary_double(SWAPFLAG, store->beams[i].delay, &buffer[index]); index += 8;
			}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;
	    group_count += 8 + store->mul_num_beams*8;

	    /* go back and fill in group byte count */
	    mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	    /* add group count to the frame count */
	    frame_count += group_count + 12;
	    }

	/* get lateral groups */
	if (store->mul_group_lateral == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* start the group byte count, but don't write it to buffer yet */
	    /* mark the byte count spot in the buffer and increment index so we skip it */
	    group_count = 0;
	    group_cnt_index = index;
	    index += 4;
	    
	    /* get lateral array */
	    group_id = MBSYS_XSE_MBM_GROUP_LATERAL;
	    mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]); index += 4;
	    for (i=0;i<store->mul_num_beams;i++)
			{
			mb_put_binary_double(SWAPFLAG, store->beams[i].lateral, &buffer[index]); index += 8;
			}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;
	    group_count += 8 + store->mul_num_beams*8;

	    /* go back and fill in group byte count */
	    mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	    /* add group count to the frame count */
	    frame_count += group_count + 12;
	    }

	/* get along groups */
	if (store->mul_group_along == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* start the group byte count, but don't write it to buffer yet */
	    /* mark the byte count spot in the buffer and increment index so we skip it */
	    group_count = 0;
	    group_cnt_index = index;
	    index += 4;
	    
	    /* get along array */
	    group_id = MBSYS_XSE_MBM_GROUP_ALONG;
	    mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]); index += 4;
	    for (i=0;i<store->mul_num_beams;i++)
			{
			mb_put_binary_double(SWAPFLAG, store->beams[i].along, &buffer[index]); index += 8;
			}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += sizeof(int);
	    group_count += 8 + store->mul_num_beams*8;

	    /* go back and fill in group byte count */
	    mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	    /* add group count to the frame count */
	    frame_count += group_count + 12;
	    }

	/* get depth groups */
	if (store->mul_group_depth == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* start the group byte count, but don't write it to buffer yet */
	    /* mark the byte count spot in the buffer and increment index so we skip it */
	    group_count = 0;
	    group_cnt_index = index;
	    index += 4;
	    
	    /* get depth array */
	    group_id = MBSYS_XSE_MBM_GROUP_DEPTH;
	    mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]); index += 4;
	    for (i=0;i<store->mul_num_beams;i++)
			{
			mb_put_binary_double(SWAPFLAG, store->beams[i].depth, &buffer[index]); index += 8;
			}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += sizeof(int);
	    group_count += 8 + store->mul_num_beams*8;

	    /* go back and fill in group byte count */
	    mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	    /* add group count to the frame count */
	    frame_count += group_count + 12;
	    }

	/* get angle groups */
	if (store->mul_group_angle == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* start the group byte count, but don't write it to buffer yet */
	    /* mark the byte count spot in the buffer and increment index so we skip it */
	    group_count = 0;
	    group_cnt_index = index;
	    index += 4;
	    
	    /* get angle array */
	    group_id = MBSYS_XSE_MBM_GROUP_ANGLE;
	    mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]); index += 4;
	    for (i=0;i<store->mul_num_beams;i++)
			{
			mb_put_binary_double(SWAPFLAG, store->beams[i].angle, &buffer[index]); index += 8;
			}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;
	    group_count += 8 + store->mul_num_beams*8;

	    /* go back and fill in group byte count */
	    mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	    /* add group count to the frame count */
	    frame_count += group_count + 12;
	    }

	/* get heave groups */
	if (store->mul_group_heave == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* start the group byte count, but don't write it to buffer yet */
	    /* mark the byte count spot in the buffer and increment index so we skip it */
	    group_count = 0;
	    group_cnt_index = index;
	    index += 4;
	    
	    /* get heave array */
	    group_id = MBSYS_XSE_MBM_GROUP_HEAVE;
	    mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]); index += 4;
	    for (i=0;i<store->mul_num_beams;i++)
			{
			mb_put_binary_double(SWAPFLAG, store->beams[i].heave, &buffer[index]); index += 8;
			}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;
	    group_count += 8 + store->mul_num_beams*8;

	    /* go back and fill in group byte count */
	    mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	    /* add group count to the frame count */
	    frame_count += group_count + 12;
	    }

	/* get roll groups */
	if (store->mul_group_roll == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;

	    /* start the group byte count, but don't write it to buffer yet */
	    /* mark the byte count spot in the buffer and increment index so we skip it */
	    group_count = 0;
	    group_cnt_index = index;
	    index += 4;
	    
	    /* get roll array */
	    group_id = MBSYS_XSE_MBM_GROUP_ROLL;
	    mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]); index += 4;
	    for (i=0;i<store->mul_num_beams;i++)
			{
			mb_put_binary_double(SWAPFLAG, store->beams[i].roll, &buffer[index]); index += 8;
			}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;
	    group_count += 8 + store->mul_num_beams*8;

	    /* go back and fill in group byte count */
	    mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	    /* add group count to the frame count */
	    frame_count += group_count + 12;
	    }

	/* get pitch groups */
	if (store->mul_group_pitch == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* start the group byte count, but don't write it to buffer yet */
	    /* mark the byte count spot in the buffer and increment index so we skip it */
	    group_count = 0;
	    group_cnt_index = index;
	    index += 4;
	    
	    /* get pitch array */
	    group_id = MBSYS_XSE_MBM_GROUP_PITCH;
	    mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]); index += 4;
	    for (i=0;i<store->mul_num_beams;i++)
			{
			mb_put_binary_double(SWAPFLAG, store->beams[i].pitch, &buffer[index]); index += 8;
			}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;
	    group_count += 8 + store->mul_num_beams*8;

	    /* go back and fill in group byte count */
	    mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	    /* add group count to the frame count */
	    frame_count += group_count + 12;
	    }

	/* get gates groups */
	if (store->mul_group_gates == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* start the group byte count, but don't write it to buffer yet */
	    /* mark the byte count spot in the buffer and increment index so we skip it */
	    group_count = 0;
	    group_cnt_index = index;
	    index += 4;
	    
	    /* get gates array */
	    group_id = MBSYS_XSE_MBM_GROUP_GATES;
	    mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]); index += 4;
	    for (i=0;i<store->mul_num_beams;i++)
			{
			mb_put_binary_double(SWAPFLAG, store->beams[i].gate_angle, &buffer[index]); index += 8;
			mb_put_binary_double(SWAPFLAG, store->beams[i].gate_start, &buffer[index]); index += 8;
			mb_put_binary_double(SWAPFLAG, store->beams[i].gate_stop, &buffer[index]); index += 8;
			}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;
	    group_count += 8 + store->mul_num_beams*24;

	    /* go back and fill in group byte count */
	    mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	    /* add group count to the frame count */
	    frame_count += group_count + 12;
	    }

	/* get noise groups */
	if (store->mul_group_noise == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* start the group byte count, but don't write it to buffer yet */
	    /* mark the byte count spot in the buffer and increment index so we skip it */
	    group_count = 0;
	    group_cnt_index = index;
	    index += 4;
	    
	    /* get noise array */
	    group_id = MBSYS_XSE_MBM_GROUP_NOISE;
	    mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]); index += 4;
	    for (i=0;i<store->mul_num_beams;i++)
			{
			mb_put_binary_float(SWAPFLAG, store->beams[i].noise, &buffer[index]); index += 4;
			}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;
	    group_count += 8 + store->mul_num_beams*4;

	    /* go back and fill in group byte count */
	    mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	    /* add group count to the frame count */
	    frame_count += group_count + 12;
	    }

	/* get length groups */
	if (store->mul_group_length == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* start the group byte count, but don't write it to buffer yet */
	    /* mark the byte count spot in the buffer and increment index so we skip it */
	    group_count = 0;
	    group_cnt_index = index;
	    index += 4;
	    
	    /* get length array */
	    group_id = MBSYS_XSE_MBM_GROUP_LENGTH;
	    mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]); index += 4;
	    for (i=0;i<store->mul_num_beams;i++)
			{
			mb_put_binary_float(SWAPFLAG, store->beams[i].length, &buffer[index]); index += 4;
			}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;
	    group_count += 8 + store->mul_num_beams*4;

	    /* go back and fill in group byte count */
	    mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	    /* add group count to the frame count */
	    frame_count += group_count + 12;
	    }

	/* get hits groups */
	if (store->mul_group_hits == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* start the group byte count, but don't write it to buffer yet */
	    /* mark the byte count spot in the buffer and increment index so we skip it */
	    group_count = 0;
	    group_cnt_index = index;
	    index += 4;
	    
	    /* get hits array */
	    group_id = MBSYS_XSE_MBM_GROUP_HITS;
	    mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]); index += 4;
	    for (i=0;i<store->mul_num_beams;i++)
			{
			mb_put_binary_int(SWAPFLAG, store->beams[i].hits, &buffer[index]); index += 4;
			}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;
	    group_count += 8 + store->mul_num_beams*4;

	    /* go back and fill in group byte count */
	    mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	    /* add group count to the frame count */
	    frame_count += group_count + 12;
	    }

	/* get heavereceive groups */
	if (store->mul_group_heavereceive == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* start the group byte count, but don't write it to buffer yet */
	    /* mark the byte count spot in the buffer and increment index so we skip it */
	    group_count = 0;
	    group_cnt_index = index;
	    index += 4;
	    
	    /* get heavereceive array */
	    group_id = MBSYS_XSE_MBM_GROUP_HEAVERECEIVE;
	    mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]); index += 4;
	    for (i=0;i<store->mul_num_beams;i++)
			{
			mb_put_binary_double(SWAPFLAG, store->beams[i].heavereceive, &buffer[index]); index += 8;
			}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;
	    group_count += 8 + store->mul_num_beams*8;

	    /* go back and fill in group byte count */
	    mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	    /* add group count to the frame count */
	    frame_count += group_count + 12;
	    }

	/* get azimuth groups */
	if (store->mul_group_azimuth == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* start the group byte count, but don't write it to buffer yet */
	    /* mark the byte count spot in the buffer and increment index so we skip it */
	    group_count = 0;
	    group_cnt_index = index;
	    index += 4;
	    
	    /* get azimuth array */
	    group_id = MBSYS_XSE_MBM_GROUP_AZIMUTH;
	    mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->mul_num_beams, &buffer[index]); index += 4;
	    for (i=0;i<store->mul_num_beams;i++)
			{
			mb_put_binary_double(SWAPFLAG, store->beams[i].azimuth, &buffer[index]); index += 8;
			}

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;
	    group_count += 8 + store->mul_num_beams*8;

	    /* go back and fill in group byte count */
	    mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	    /* add group count to the frame count */
	    frame_count += group_count + 12;
	    }

	/* get azimuth groups */
	if (store->mul_group_mbsystemnav == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* start the group byte count, but don't write it to buffer yet */
	    /* mark the byte count spot in the buffer and increment index so we skip it */
	    group_count = 0;
	    group_cnt_index = index;
	    index += 4;
	    
	    /* get azimuth array */
	    group_id = MBSYS_XSE_MBM_GROUP_MBSYSTEMNAV;
	    mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	    mb_put_binary_double(SWAPFLAG, store->mul_lon, &buffer[index]); index += 8;
	    mb_put_binary_double(SWAPFLAG, store->mul_lat, &buffer[index]); index += 8;
	    mb_put_binary_double(SWAPFLAG, store->mul_heading, &buffer[index]); index += 8;
	    mb_put_binary_double(SWAPFLAG, store->mul_speed, &buffer[index]); index += 8;

	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;
	    group_count += 36;

	    /* go back and fill in group byte count */
	    mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	    /* add group count to the frame count */
	    frame_count += group_count + 12;
	    }

	/* get end of frame label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH#", 4);
#else
	strncpy(&buffer[index], "#HSF", 4);
#endif
	index += 4;	

	/* go back and fill in frame byte count */
	mb_put_binary_int(SWAPFLAG, frame_count, &buffer[frame_cnt_index]);
	
	/* set buffer size */
	*buffer_size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2       buffer_size:%d\n",*buffer_size);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_l3xseraw_wr_sidescan(int verbose,int *buffer_size,char *buffer,void *store_ptr,int *error)
{
	char	*function_name = "mbr_l3xseraw_wr_sidescan";
	int	status = MB_SUCCESS;
	struct mbsys_xse_struct *store;
	int	index;
	int frame_count;
	int group_count;
	int frame_cnt_index;
	int group_cnt_index;
	int frame_id;
	int group_id;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
	    fprintf(stderr,"dbg2  Input arguments:\n");
	    fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
	    fprintf(stderr,"dbg2       buffer:     %lu\n",(size_t)buffer);
	    fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
	    }

	/* get pointer to store data structure */
	store = (struct mbsys_xse_struct *) store_ptr;

	/* print debug statements */
	if (verbose >= 5)
	    {
	    fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
	    fprintf(stderr,"dbg5       sid_frame:            %d\n",store->sid_frame);
	    fprintf(stderr,"dbg5       sid_group_avt:        %d\n",store->sid_group_avt);
	    fprintf(stderr,"dbg5       sid_group_pvt:        %d\n",store->sid_group_pvt);
	    fprintf(stderr,"dbg5       sid_group_avl:        %d\n",store->sid_group_avl);
	    fprintf(stderr,"dbg5       sid_group_pvl:        %d\n",store->sid_group_pvl);
	    fprintf(stderr,"dbg5       sid_group_signal:     %d\n",store->sid_group_signal);
	    fprintf(stderr,"dbg5       sid_group_ping:       %d\n",store->sid_group_ping);
	    fprintf(stderr,"dbg5       sid_group_complex:    %d\n",store->sid_group_complex);
	    fprintf(stderr,"dbg5       sid_group_weighting:  %d\n",store->sid_group_weighting);
	    fprintf(stderr,"dbg5       sid_source:           %d\n",store->sid_source);
	    fprintf(stderr,"dbg5       sid_sec:              %d\n",store->sid_sec);
	    fprintf(stderr,"dbg5       sid_usec:             %u\n",store->sid_usec);
	    fprintf(stderr,"dbg5       sid_ping:             %u\n",store->sid_ping);
	    fprintf(stderr,"dbg5       sid_frequency:        %f\n",store->sid_frequency);
	    fprintf(stderr,"dbg5       sid_pulse:            %f\n",store->sid_pulse);
	    fprintf(stderr,"dbg5       sid_power:            %f\n",store->sid_power);
	    fprintf(stderr,"dbg5       sid_bandwidth:        %f\n",store->sid_bandwidth);
	    fprintf(stderr,"dbg5       sid_sample:           %f\n",store->sid_sample);
	    fprintf(stderr,"dbg5       sid_avt_sampleus:     %d\n",store->sid_avt_sampleus);
	    fprintf(stderr,"dbg5       sid_avt_offset:       %d\n",store->sid_avt_offset);
	    fprintf(stderr,"dbg5       sid_avt_num_samples:  %d\n",store->sid_avt_num_samples);
	    for (i=0;i<store->sid_avt_num_samples;i++)
	    	fprintf(stderr,"dbg5       sid_avt_amp[%d]:%d\n",i,store->sid_avt_amp[i]);
	    fprintf(stderr,"dbg5       sid_pvt_sampleus:  %d\n",store->sid_pvt_sampleus);
	    fprintf(stderr,"dbg5       sid_pvt_offset:  %d\n",store->sid_pvt_offset);
	    fprintf(stderr,"dbg5       sid_pvt_num_samples:  %d\n",store->sid_pvt_num_samples);
	    for (i=0;i<store->sid_pvt_num_samples;i++)
	    	fprintf(stderr,"dbg5       sid_pvt_phase[%d]:%d\n",i,store->sid_pvt_phase[i]);
	    fprintf(stderr,"dbg5       sid_avl_binsize:  %d\n",store->sid_avl_binsize);
	    fprintf(stderr,"dbg5       sid_avl_offset:  %d\n",store->sid_avl_offset);
	    fprintf(stderr,"dbg5       sid_avl_num_samples:  %d\n",store->sid_avl_num_samples);
	    for (i=0;i<store->sid_avl_num_samples;i++)
	    	fprintf(stderr,"dbg5       sid_avl_amp[%d]:%d\n",i,store->sid_avl_amp[i]);
	    fprintf(stderr,"dbg5       sid_pvl_binsize:  %d\n",store->sid_pvl_binsize);
	    fprintf(stderr,"dbg5       sid_pvl_offset:  %d\n",store->sid_pvl_offset);
	    fprintf(stderr,"dbg5       sid_pvl_num_samples:  %d\n",store->sid_pvl_num_samples);
	    for (i=0;i<store->sid_pvl_num_samples;i++)
	    	fprintf(stderr,"dbg5       sid_pvl_phase[%d]:%d\n",i,store->sid_pvl_phase[i]);
	    fprintf(stderr,"dbg5       sid_sig_ping:  %d\n",store->sid_sig_ping);
	    fprintf(stderr,"dbg5       sid_sig_channel:  %d\n",store->sid_sig_channel);
	    fprintf(stderr,"dbg5       sid_sig_offset:  %f\n",store->sid_sig_offset);
	    fprintf(stderr,"dbg5       sid_sig_sample:  %f\n",store->sid_sig_sample);
	    fprintf(stderr,"dbg5       sid_sig_num_samples:  %d\n",store->sid_sig_num_samples);
	    for (i=0;i<store->sid_sig_num_samples;i++)
	    	fprintf(stderr,"dbg5       sid_sig_phase[%d]:%d\n",i,store->sid_sig_phase[i]);
	    fprintf(stderr,"dbg5       sid_png_pulse:  %u\n",store->sid_png_pulse);
	    fprintf(stderr,"dbg5       sid_png_startfrequency:  %f\n",store->sid_png_startfrequency);
	    fprintf(stderr,"dbg5       sid_png_endfrequency:  %f\n",store->sid_png_endfrequency);
	    fprintf(stderr,"dbg5       sid_png_duration:  %f\n",store->sid_png_duration);
	    fprintf(stderr,"dbg5       sid_png_mancode:  %d\n",store->sid_png_mancode);
	    fprintf(stderr,"dbg5       sid_png_pulseid:  %d\n",store->sid_png_pulseid);
	    fprintf(stderr,"dbg5       sid_png_pulsename:  %s\n",store->sid_png_pulsename);
	    fprintf(stderr,"dbg5       sid_cmp_ping:  %d\n",store->sid_cmp_ping);
	    fprintf(stderr,"dbg5       sid_cmp_channel:  %d\n",store->sid_cmp_channel);
	    fprintf(stderr,"dbg5       sid_cmp_offset:  %f\n",store->sid_cmp_offset);
	    fprintf(stderr,"dbg5       sid_cmp_sample:  %f\n",store->sid_cmp_sample);
	    fprintf(stderr,"dbg5       sid_cmp_num_samples:  %d\n",store->sid_cmp_num_samples);
	    for (i=0;i<store->sid_sig_num_samples;i++)
	    	fprintf(stderr,"dbg5       sid_cmp_real[%d]:%d sid_cmp_imaginary[%d]:%d\n",
				i,store->sid_cmp_real[i],i,store->sid_cmp_imaginary[i]);
	    fprintf(stderr,"dbg5       sid_wgt_factorleft:  %d\n",store->sid_wgt_factorleft);
	    fprintf(stderr,"dbg5       sid_wgt_samplesleft:  %d\n",store->sid_wgt_samplesleft);
	    fprintf(stderr,"dbg5       sid_wgt_factorright:  %d\n",store->sid_wgt_factorright);
	    fprintf(stderr,"dbg5       sid_wgt_samplesright:  %d\n",store->sid_wgt_samplesright);
	    }

	/* get the frame label */
	index = 0;
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH$", 4);
#else
	strncpy(&buffer[index], "$HSF", 4);
#endif
	index += 4;

	/* start the frame byte count, but don't write it to buffer yet */
	/* increment index so we skip the count value in the buffer */
	frame_count = 0;
	frame_cnt_index = index;
	index += 4;
	
	/* get frame time */
	frame_id = MBSYS_XSE_SSN_FRAME;
	mb_put_binary_int(SWAPFLAG, frame_id, &buffer[index]); index += 4;
	mb_put_binary_int(SWAPFLAG, store->sid_source, &buffer[index]); index += 4;
	mb_put_binary_int(SWAPFLAG, store->sid_sec, &buffer[index]); index += 4;
	mb_put_binary_int(SWAPFLAG, store->sid_usec, &buffer[index]); index += 4;
	frame_count += 16;
	
	/*****************************************/

	/* get general group */
	/* get group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH$", 4);
#else
	strncpy(&buffer[index], "$HSG", 4);
#endif
	index += 4;

	/* start the group byte count, but don't write it to buffer yet */
	/* mark the byte count spot in the buffer and increment index so we skip it */
	group_count = 0;
	group_cnt_index = index;
	index += 4;
	
	/* get general group */
	group_id = MBSYS_XSE_SSN_GROUP_GEN;
	mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	mb_put_binary_int(SWAPFLAG, store->sid_ping, &buffer[index]); index += 4;
	mb_put_binary_float(SWAPFLAG, store->sid_frequency, &buffer[index]); index += 4;
	mb_put_binary_float(SWAPFLAG, store->sid_pulse, &buffer[index]); index += 4;
	mb_put_binary_float(SWAPFLAG, store->sid_power, &buffer[index]); index += 4;
	mb_put_binary_float(SWAPFLAG, store->sid_bandwidth, &buffer[index]); index += 4;
	mb_put_binary_float(SWAPFLAG, store->sid_sample, &buffer[index]); index += 4;

	/* get end of group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH#", 4);
#else
	strncpy(&buffer[index], "#HSG", 4);
#endif
	index += 4;
	group_count += 28;

	/* go back and fill in group byte count */
	mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

	/* add group count to the frame count */
	    frame_count += group_count + 12;

	/*****************************************/

	/* get amplitude vs traveltime group */
	if (store->sid_group_avt == MB_YES)
		{
		/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get amplitude array */
		group_id = MBSYS_XSE_SSN_GROUP_AMPVSTT;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_avt_sampleus, &buffer[index]); index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_avt_offset, &buffer[index]); index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_avt_num_samples, &buffer[index]); index += 4;
		for (i=0;i<store->sid_avt_num_samples;i++)
		    {
		    mb_put_binary_short(SWAPFLAG, store->sid_avt_amp[i], &buffer[index]); index += 2;
		    }

		/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 16 + 2 * store->sid_avt_num_samples;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
		}

	/*****************************************/

	/* get phase vs traveltime group */
	if (store->sid_group_avt == MB_YES)
		{
		/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get amplitude array */
		group_id = MBSYS_XSE_SSN_GROUP_PHASEVSTT;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_pvt_sampleus, &buffer[index]); index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_pvt_offset, &buffer[index]); index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_pvt_num_samples, &buffer[index]); index += 4;
		for (i=0;i<store->sid_pvt_num_samples;i++)
		    {
		    mb_put_binary_short(SWAPFLAG, store->sid_pvt_phase[i], &buffer[index]); index += 2;
		    }

		/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 16 + 2 * store->sid_pvt_num_samples;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
		}

	/*****************************************/

	/* get amplitude vs lateral group */
	if (store->sid_group_avl == MB_YES)
		{
		/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get amplitude array */
		group_id = MBSYS_XSE_SSN_GROUP_AMPVSLAT;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_avl_binsize, &buffer[index]); index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_avl_offset, &buffer[index]); index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_avl_num_samples, &buffer[index]); index += 4;
		for (i=0;i<store->sid_avl_num_samples;i++)
		    {
		    mb_put_binary_short(SWAPFLAG, store->sid_avl_amp[i], &buffer[index]); index += 2;
		    }

		/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 16 + 2 * store->sid_avl_num_samples;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
		}

	/*****************************************/

	/* get phase vs lateral group */
	if (store->sid_group_pvl == MB_YES)
		{
		/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get amplitude array */
		group_id = MBSYS_XSE_SSN_GROUP_PHASEVSLAT;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_pvl_binsize, &buffer[index]); index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_pvl_offset, &buffer[index]); index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_pvl_num_samples, &buffer[index]); index += 4;
		for (i=0;i<store->sid_pvl_num_samples;i++)
		    {
		    mb_put_binary_short(SWAPFLAG, store->sid_pvl_phase[i], &buffer[index]); index += 2;
		    }

		/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 16 + 2 * store->sid_pvl_num_samples;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
		}

	/*****************************************/

	/* get signal group */
	if (store->sid_group_signal == MB_YES)
		{
		/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get amplitude array */
		group_id = MBSYS_XSE_SSN_GROUP_SIGNAL;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_sig_ping, &buffer[index]); index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_sig_channel, &buffer[index]); index += 4;
		mb_put_binary_double(SWAPFLAG, store->sid_sig_offset, &buffer[index]); index += 8;
		mb_put_binary_double(SWAPFLAG, store->sid_sig_sample, &buffer[index]); index += 8;
		mb_put_binary_int(SWAPFLAG, store->sid_sig_num_samples, &buffer[index]); index += 4;
		for (i=0;i<store->sid_sig_num_samples;i++)
		    {
		    mb_put_binary_short(SWAPFLAG, store->sid_sig_phase[i], &buffer[index]); index += 2;
		    }

		/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 32 + 2 * store->sid_sig_num_samples;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
		}

	/*****************************************/

	/* get ping group */
	if (store->sid_group_ping == MB_YES)
		{
		/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get amplitude array */
		group_id = MBSYS_XSE_SSN_GROUP_PINGTYPE;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
		mb_put_binary_double(SWAPFLAG, store->sid_png_startfrequency, &buffer[index]); index += 8;
		mb_put_binary_double(SWAPFLAG, store->sid_png_endfrequency, &buffer[index]); index += 8;
		mb_put_binary_double(SWAPFLAG, store->sid_png_duration, &buffer[index]); index += 8;
		mb_put_binary_int(SWAPFLAG, store->sid_png_mancode, &buffer[index]); index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_png_pulseid, &buffer[index]); index += 4;
		strcpy(&buffer[index], store->sid_png_pulsename); index += strlen(store->sid_png_pulsename);
		buffer[index] = 0; index++;
		if (strlen(store->sid_png_pulsename) % 2 > 0)
			{
			buffer[index] = 0; index++;
			}

		/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 36 + strlen(store->sid_png_pulsename) + 1 + (strlen(store->sid_png_pulsename) % 2);

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
		}

	/*****************************************/

	/* get complex signal group */
	if (store->sid_group_ping == MB_YES)
		{
		/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get amplitude array */
		group_id = MBSYS_XSE_SSN_GROUP_COMPLEXSIGNAL;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_cmp_ping, &buffer[index]); index += 4;
		mb_put_binary_int(SWAPFLAG, store->sid_cmp_channel, &buffer[index]); index += 4;
		mb_put_binary_double(SWAPFLAG, store->sid_cmp_offset, &buffer[index]); index += 8;
		mb_put_binary_double(SWAPFLAG, store->sid_cmp_sample, &buffer[index]); index += 8;
		mb_put_binary_int(SWAPFLAG, store->sid_cmp_num_samples, &buffer[index]); index += 4;
		for (i=0;i<store->sid_cmp_num_samples;i++)
		    {
		    mb_put_binary_short(SWAPFLAG, store->sid_cmp_real[i], &buffer[index]); index += 2;
		    mb_put_binary_short(SWAPFLAG, store->sid_cmp_imaginary[i], &buffer[index]); index += 2;
		    }

		/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 32 + 4 * store->sid_cmp_num_samples;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
		}

	/*****************************************/

	/* get weighting group */
	if (store->sid_group_weighting == MB_YES)
		{
		/* get group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH$", 4);
#else
		strncpy(&buffer[index], "$HSG", 4);
#endif
		index += 4;

		/* start the group byte count, but don't write it to buffer yet */
		/* mark the byte count spot in the buffer and increment index so we skip it */
		group_count = 0;
		group_cnt_index = index;
		index += 4;

		/* get amplitude array */
		group_id = MBSYS_XSE_SSN_GROUP_WEIGHTING;
		mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
		mb_put_binary_short(SWAPFLAG, store->sid_wgt_factorleft, &buffer[index]); index += 2;
		mb_put_binary_int(SWAPFLAG, store->sid_wgt_samplesleft, &buffer[index]); index += 4;
		mb_put_binary_short(SWAPFLAG, store->sid_wgt_factorright, &buffer[index]); index += 2;
		mb_put_binary_int(SWAPFLAG, store->sid_wgt_samplesright, &buffer[index]); index += 4;

		/* get end of group label */
#ifdef DATAINPCBYTEORDER
		strncpy(&buffer[index], "GSH#", 4);
#else
		strncpy(&buffer[index], "#HSG", 4);
#endif
		index += 4;
		group_count += 16;

		/* go back and fill in group byte count */
		mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);

		/* add group count to the frame count */
		frame_count += group_count + 12;
		}

	/*****************************************/

	/* get end of frame label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH#", 4);
#else
	strncpy(&buffer[index], "#HSF", 4);
#endif
	index += 4;	

	/* go back and fill in frame byte count */
	mb_put_binary_int(SWAPFLAG, frame_count, &buffer[frame_cnt_index]);

	/* set buffer size */
	*buffer_size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2       buffer_size:%d\n",*buffer_size);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_l3xseraw_wr_seabeam(int verbose,int *buffer_size,char *buffer,void *store_ptr,int *error)
{
	char	*function_name = "mbr_l3xseraw_wr_seabeam";
	int	status = MB_SUCCESS;
	struct mbsys_xse_struct *store;
	int	index;
	int frame_count;
	int group_count;
	int frame_cnt_index;
	int group_cnt_index;
	int frame_id;
	int group_id;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
	    fprintf(stderr,"dbg2  Input arguments:\n");
	    fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
	    fprintf(stderr,"dbg2       buffer:     %lu\n",(size_t)buffer);
	    fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
	    }

	/* get pointer to store data structure */
	store = (struct mbsys_xse_struct *) store_ptr;

	/* print debug statements */
	if (verbose >= 5)
	    {
	    fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",function_name);
	    fprintf(stderr,"dbg5       sbm_source:          %d\n",store->sbm_source);
	    fprintf(stderr,"dbg5       sbm_sec:             %u\n",store->sbm_sec);
	    fprintf(stderr,"dbg5       sbm_usec:            %u\n",store->sbm_usec);
	    }
	if (verbose >= 5 && store->sbm_properties == MB_YES)
	    {
	    fprintf(stderr,"dbg5       sbm_ping:            %d\n",store->sbm_ping);
	    fprintf(stderr,"dbg5       sbm_ping_gain:       %f\n",store->sbm_ping_gain);
	    fprintf(stderr,"dbg5       sbm_pulse_width:     %f\n",store->sbm_pulse_width);
	    fprintf(stderr,"dbg5       sbm_transmit_power:  %f\n",store->sbm_transmit_power);
	    fprintf(stderr,"dbg5       sbm_pixel_width:     %f\n",store->sbm_pixel_width);
	    fprintf(stderr,"dbg5       sbm_swath_width:     %f\n",store->sbm_swath_width);
	    fprintf(stderr,"dbg5       sbm_time_slice:      %f\n",store->sbm_time_slice);
	    fprintf(stderr,"dbg5       sbm_depth_mode:      %d\n",store->sbm_depth_mode);
	    fprintf(stderr,"dbg5       sbm_beam_mode:       %d\n",store->sbm_beam_mode);
	    fprintf(stderr,"dbg5       sbm_ssv:             %f\n",store->sbm_ssv);
	    fprintf(stderr,"dbg5       sbm_frequency:       %f\n",store->sbm_frequency);
	    fprintf(stderr,"dbg5       sbm_bandwidth:       %f\n",store->sbm_bandwidth);
	    }
	if (verbose >= 5 && store->sbm_hrp == MB_YES)
	    {
	    fprintf(stderr,"dbg5       sbm_heave:           %f\n",store->sbm_heave);
	    fprintf(stderr,"dbg5       sbm_roll:            %f\n",store->sbm_roll);
	    fprintf(stderr,"dbg5       sbm_pitch:           %f\n",store->sbm_pitch);
	    }
	if (verbose >= 5 && store->sbm_center == MB_YES)
	    {
	    fprintf(stderr,"dbg5       sbm_center_beam:     %d\n",store->sbm_center_beam);
	    fprintf(stderr,"dbg5       sbm_center_count:    %d\n",store->sbm_center_count);
	    for (i=0;i<store->sbm_center_count;i++)
		fprintf(stderr,"dbg5       sample[%d]: %f\n",
		    i, store->sbm_center_amp[i]);
	    }
	if (verbose >= 5 && store->sbm_message == MB_YES)
	    {
	    fprintf(stderr,"dbg5       sbm_message_id:      %d\n",store->sbm_message_id);
	    fprintf(stderr,"dbg5       sbm_message_len:     %d\n",store->sbm_message_len);
	    fprintf(stderr,"dbg5       sbm_message_txt:     %s\n",store->sbm_message_txt);
	    }

	/* get the frame label */
	index = 0;
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH$", 4);
#else
	strncpy(&buffer[index], "$HSF", 4);
#endif
	index += 4;

	/* start the frame byte count, but don't write it to buffer yet */
	/* increment index so we skip the count value in the buffer */
	frame_count = 0;
	frame_cnt_index = index;
	index += 4;
	
	/* get frame time */
	frame_id = MBSYS_XSE_SBM_FRAME;
	mb_put_binary_int(SWAPFLAG, frame_id, &buffer[index]); index += 4;
	mb_put_binary_int(SWAPFLAG, store->sbm_source, &buffer[index]); index += 4;
	mb_put_binary_int(SWAPFLAG, store->sbm_sec, &buffer[index]); index += 4;
	mb_put_binary_int(SWAPFLAG, store->sbm_usec, &buffer[index]); index += 4;
	frame_count += 16;

	/* deal with properties group */
	if (store->sbm_properties == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* start the group byte count, but don't write it to buffer yet */
	    /* mark the byte count spot in the buffer and increment index so we skip it */
	    group_count = 0;
	    group_cnt_index = index;
	    index += 4;
	    
	    /* get properties group */
	    group_id = MBSYS_XSE_SBM_GROUP_PROPERTIES;
	    mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->sbm_ping, &buffer[index]); index += 4;
	    mb_put_binary_float(SWAPFLAG, store->sbm_ping_gain, &buffer[index]); index += 4;
	    mb_put_binary_float(SWAPFLAG, store->sbm_pulse_width, &buffer[index]); index += 4;
	    mb_put_binary_float(SWAPFLAG, store->sbm_transmit_power, &buffer[index]); index += 4;
	    mb_put_binary_float(SWAPFLAG, store->sbm_pixel_width, &buffer[index]); index += 4;
	    mb_put_binary_float(SWAPFLAG, store->sbm_swath_width, &buffer[index]); index += 4;
	    mb_put_binary_float(SWAPFLAG, store->sbm_time_slice, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->sbm_depth_mode, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->sbm_beam_mode, &buffer[index]); index += 4;
	    mb_put_binary_float(SWAPFLAG, store->sbm_ssv, &buffer[index]); index += 4;
	    mb_put_binary_float(SWAPFLAG, store->sbm_frequency, &buffer[index]); index += 4;
	    mb_put_binary_float(SWAPFLAG, store->sbm_bandwidth, &buffer[index]); index += 4;
	    group_count += 52;
    
	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;
    
	    /* go back and fill in group byte count */
	    mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);
    
	    /* add group count to the frame count */
	    frame_count += group_count + 12;
	    }

	/* deal with hrp group */
	if (store->sbm_hrp == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* start the group byte count, but don't write it to buffer yet */
	    /* mark the byte count spot in the buffer and increment index so we skip it */
	    group_count = 0;
	    group_cnt_index = index;
	    index += 4;
	    
	    /* get hrp group */
	    group_id = MBSYS_XSE_SBM_GROUP_HRP;
	    mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	    mb_put_binary_double(SWAPFLAG, store->sbm_heave, &buffer[index]); index += 8;
	    mb_put_binary_double(SWAPFLAG, store->sbm_roll, &buffer[index]); index += 8;
	    mb_put_binary_double(SWAPFLAG, store->sbm_pitch, &buffer[index]); index += 8;
	    group_count += 28;
    
	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;
    
	    /* go back and fill in group byte count */
	    mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);
    
	    /* add group count to the frame count */
	    frame_count += group_count + 12;
	    }

	/* deal with center group */
	if (store->sbm_center == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* start the group byte count, but don't write it to buffer yet */
	    /* mark the byte count spot in the buffer and increment index so we skip it */
	    group_count = 0;
	    group_cnt_index = index;
	    index += 4;
	    
	    /* get center group */
	    group_id = MBSYS_XSE_SBM_GROUP_CENTER;
	    mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->sbm_center_beam, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->sbm_center_count, &buffer[index]); index += 4;
	    for (i=0;i<store->sbm_center_count;i++)
		{
		mb_put_binary_float(SWAPFLAG, store->sbm_center_amp[i], &buffer[index]); 
		index += 4;
		}
	    group_count += 12 + 4 * store->sbm_center_count;
    
	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;
    
	    /* go back and fill in group byte count */
	    mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);
    
	    /* add group count to the frame count */
	    frame_count += group_count + 12;
	    }

	/* deal with message group */
	if (store->sbm_message == MB_YES)
	    {
	    /* get group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH$", 4);
#else
	    strncpy(&buffer[index], "$HSG", 4);
#endif
	    index += 4;
    
	    /* start the group byte count, but don't write it to buffer yet */
	    /* mark the byte count spot in the buffer and increment index so we skip it */
	    group_count = 0;
	    group_cnt_index = index;
	    index += 4;
	    
	    /* get center group */
	    group_id = MBSYS_XSE_SBM_GROUP_MESSAGE;
	    mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->sbm_message_id, &buffer[index]); index += 4;
	    mb_put_binary_int(SWAPFLAG, store->sbm_message_len, &buffer[index]); index += 4;
	    for (i=0;i<store->sbm_message_len;i++)
		{
		buffer[index] = store->sbm_message_txt[i]; 
		index++;
		}
	    group_count += 12 + store->sbm_message_len;
    
	    /* get end of group label */
#ifdef DATAINPCBYTEORDER
	    strncpy(&buffer[index], "GSH#", 4);
#else
	    strncpy(&buffer[index], "#HSG", 4);
#endif
	    index += 4;
    
	    /* go back and fill in group byte count */
	    mb_put_binary_int(SWAPFLAG, group_count, &buffer[group_cnt_index]);
    
	    /* add group count to the frame count */
	    frame_count += group_count + 12;
	    }

	/* get end of frame label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH#", 4);
#else
	strncpy(&buffer[index], "#HSF", 4);
#endif
	index += 4;	

	/* go back and fill in frame byte count */
	mb_put_binary_int(SWAPFLAG, frame_count, &buffer[frame_cnt_index]);
	
	/* set buffer size */
	*buffer_size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2       buffer_size:%d\n",*buffer_size);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_l3xseraw_wr_comment(int verbose,int *buffer_size,char *buffer,void *store_ptr,int *error)
{
	char	*function_name = "mbr_l3xseraw_wr_comment";
	int	status = MB_SUCCESS;
	struct mbsys_xse_struct *store;
	int	index;
	int	size;
	int	len;
	int frame_id;
	int group_id;

	/* print input debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
	    fprintf(stderr,"dbg2  Input arguments:\n");
	    fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
	    fprintf(stderr,"dbg2       buffer:     %lu\n",(size_t)buffer);
	    fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
	    }

	/* get pointer to store data structure */
	store = (struct mbsys_xse_struct *) store_ptr;

	/* print debug statements */
	if (verbose >= 5)
	    {
	    fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",function_name);
	    fprintf(stderr,"dbg5       comment:             %s\n",store->comment);
	    }

	/* get the frame label */
	index = 0;
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH$", 4);
#else
	strncpy(&buffer[index], "$HSF", 4);
#endif
	index += 4;

	/* get frame size */
	len = strlen(store->comment) + 4;
	if (len % 4 > 0)
	    len += 4 - (len % 4);
	size = len + 32;
	mb_put_binary_int(SWAPFLAG, size, &buffer[index]); index += 4;
	
	/* get frame time */
	frame_id = MBSYS_XSE_COM_FRAME;
	mb_put_binary_int(SWAPFLAG, frame_id, &buffer[index]); index += 4;
	mb_put_binary_int(SWAPFLAG, store->com_source, &buffer[index]); index += 4;
	mb_put_binary_int(SWAPFLAG, store->com_sec, &buffer[index]); index += 4;
	mb_put_binary_int(SWAPFLAG, store->com_usec, &buffer[index]); index += 4;

	/* get group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH$", 4);
#else
	strncpy(&buffer[index], "$HSG", 4);
#endif
	index += 4;

	/* get group size and id */
	mb_put_binary_int(SWAPFLAG, (len + 4), &buffer[index]); index += 4;
	group_id = MBSYS_XSE_COM_GROUP_GEN;
	mb_put_binary_int(SWAPFLAG, group_id, &buffer[index]); index += 4;
	strncpy(&buffer[index], store->comment, len); index += len;

	/* get end of group label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "GSH#", 4);
#else
	strncpy(&buffer[index], "#HSG", 4);
#endif
	index += 4;

	/* get end of frame label */
#ifdef DATAINPCBYTEORDER
	strncpy(&buffer[index], "FSH#", 4);
#else
	strncpy(&buffer[index], "#HSF", 4);
#endif
	index += 4;
	
	*buffer_size = index;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2       buffer_size:%d\n",*buffer_size);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
