/*--------------------------------------------------------------------
 *    The MB-system:	mbr_emoldraw.c	3/4/2001
 *	$Id: mbr_emoldraw.c,v 5.3 2002-07-20 20:42:40 caress Exp $
 *
 *    Copyright (c) 2001 by
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
 * mbr_emoldraw.c contains the functions for reading and writing
 * multibeam data in the emoldraw format.  
 * These functions include:
 *   mbr_alm_emoldraw	- allocate read/write memory
 *   mbr_dem_emoldraw	- deallocate read/write memory
 *   mbr_rt_emoldraw	- read and translate data
 *   mbr_wt_emoldraw	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	March 4, 2001
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.2  2001/07/20 00:31:11  caress
 * Release 5.0.beta03
 *
 * Revision 5.1  2001/06/08  21:44:01  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2001/03/22  20:49:19  caress
 * Trying to make version 5.0.beta0
 *
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
#include "../../include/mbsys_simrad.h"

/* include for byte swapping */
#include "../../include/mb_swap.h"

/* turn on debug statements here */
/*#define MBR_EMOLDRAW_DEBUG 1*/

/* essential function prototypes */
int mbr_register_emoldraw(int verbose, void *mbio_ptr, 
		int *error);
int mbr_info_emoldraw(int verbose, 
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
			double *beamwidth_xtrack, 
			double *beamwidth_ltrack, 
			int *error);
int mbr_alm_emoldraw(int verbose, void *mbio_ptr, int *error);
int mbr_dem_emoldraw(int verbose, void *mbio_ptr, int *error);
int mbr_rt_emoldraw(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_emoldraw(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_emoldraw_chk_label(int verbose, void *mbio_ptr, short type);

/*--------------------------------------------------------------------*/
int mbr_register_emoldraw(int verbose, void *mbio_ptr, int *error)
{
	static char res_id[]="$Id: mbr_emoldraw.c,v 5.3 2002-07-20 20:42:40 caress Exp $";
	char	*function_name = "mbr_register_emoldraw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		}

	/* get mb_io_ptr */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set format info parameters */
	status = mbr_info_emoldraw(verbose, 
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
			&mb_io_ptr->beamwidth_xtrack, 
			&mb_io_ptr->beamwidth_ltrack, 
			error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_emoldraw;
	mb_io_ptr->mb_io_format_free = &mbr_dem_emoldraw; 
	mb_io_ptr->mb_io_store_alloc = &mbsys_simrad_alloc; 
	mb_io_ptr->mb_io_store_free = &mbsys_simrad_deall; 
	mb_io_ptr->mb_io_read_ping = &mbr_rt_emoldraw; 
	mb_io_ptr->mb_io_write_ping = &mbr_wt_emoldraw; 
	mb_io_ptr->mb_io_extract = &mbsys_simrad_extract; 
	mb_io_ptr->mb_io_insert = &mbsys_simrad_insert; 
	mb_io_ptr->mb_io_extract_nav = &mbsys_simrad_extract_nav; 
	mb_io_ptr->mb_io_insert_nav = &mbsys_simrad_insert_nav; 
	mb_io_ptr->mb_io_extract_altitude = &mbsys_simrad_extract_altitude; 
	mb_io_ptr->mb_io_insert_altitude = NULL; 
	mb_io_ptr->mb_io_extract_svp = &mbsys_simrad_extract_svp; 
	mb_io_ptr->mb_io_insert_svp = &mbsys_simrad_insert_svp; 
	mb_io_ptr->mb_io_ttimes = &mbsys_simrad_ttimes; 
	mb_io_ptr->mb_io_detects = &mbsys_simrad_detects; 
	mb_io_ptr->mb_io_copyrecord = &mbsys_simrad_copy; 
	mb_io_ptr->mb_io_extract_rawss = NULL; 
	mb_io_ptr->mb_io_insert_rawss = NULL; 

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
		fprintf(stderr,"dbg2       heading_source:     %d\n",mb_io_ptr->heading_source);
		fprintf(stderr,"dbg2       beamwidth_xtrack:   %f\n",mb_io_ptr->beamwidth_xtrack);
		fprintf(stderr,"dbg2       beamwidth_ltrack:   %f\n",mb_io_ptr->beamwidth_ltrack);
		fprintf(stderr,"dbg2       format_alloc:       %d\n",mb_io_ptr->mb_io_format_alloc);
		fprintf(stderr,"dbg2       format_free:        %d\n",mb_io_ptr->mb_io_format_free);
		fprintf(stderr,"dbg2       store_alloc:        %d\n",mb_io_ptr->mb_io_store_alloc);
		fprintf(stderr,"dbg2       store_free:         %d\n",mb_io_ptr->mb_io_store_free);
		fprintf(stderr,"dbg2       read_ping:          %d\n",mb_io_ptr->mb_io_read_ping);
		fprintf(stderr,"dbg2       write_ping:         %d\n",mb_io_ptr->mb_io_write_ping);
		fprintf(stderr,"dbg2       extract:            %d\n",mb_io_ptr->mb_io_extract);
		fprintf(stderr,"dbg2       insert:             %d\n",mb_io_ptr->mb_io_insert);
		fprintf(stderr,"dbg2       extract_nav:        %d\n",mb_io_ptr->mb_io_extract_nav);
		fprintf(stderr,"dbg2       insert_nav:         %d\n",mb_io_ptr->mb_io_insert_nav);
		fprintf(stderr,"dbg2       extract_altitude:   %d\n",mb_io_ptr->mb_io_extract_altitude);
		fprintf(stderr,"dbg2       insert_altitude:    %d\n",mb_io_ptr->mb_io_insert_altitude);
		fprintf(stderr,"dbg2       extract_svp:        %d\n",mb_io_ptr->mb_io_extract_svp);
		fprintf(stderr,"dbg2       insert_svp:         %d\n",mb_io_ptr->mb_io_insert_svp);
		fprintf(stderr,"dbg2       ttimes:             %d\n",mb_io_ptr->mb_io_ttimes);
		fprintf(stderr,"dbg2       extract_rawss:      %d\n",mb_io_ptr->mb_io_extract_rawss);
		fprintf(stderr,"dbg2       insert_rawss:       %d\n",mb_io_ptr->mb_io_insert_rawss);
		fprintf(stderr,"dbg2       copyrecord:         %d\n",mb_io_ptr->mb_io_copyrecord);
		fprintf(stderr,"dbg2       error:              %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		}

	/* return status */
	return(status);
}

/*--------------------------------------------------------------------*/
int mbr_info_emoldraw(int verbose, 
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
			double *beamwidth_xtrack, 
			double *beamwidth_ltrack, 
			int *error)
{
	static char res_id[]="$Id: mbr_emoldraw.c,v 5.3 2002-07-20 20:42:40 caress Exp $";
	char	*function_name = "mbr_info_emoldraw";
	int	status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		}

	/* set format info parameters */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	*system = MB_SYS_SIMRAD;
	*beams_bath_max = MBSYS_SIMRAD_MAXBEAMS;
	*beams_amp_max = MBSYS_SIMRAD_MAXBEAMS;
	*pixels_ss_max = MBSYS_SIMRAD_MAXPIXELS;
	strncpy(format_name, "EMOLDRAW", MB_NAME_LENGTH);
	strncpy(system_name, "SIMRAD", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_EMOLDRAW\nInformal Description: Old Simrad vendor multibeam format\nAttributes:           Simrad EM1000, EM12S, EM12D, \n                      and EM121 multibeam sonars,\n                      bathymetry, amplitude, and sidescan,\n                      60 beams for EM1000, 81 beams for EM12S/D,\n		      121 beams for EM121, variable pixels,\n		      ascii + binary, Simrad.\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_NO;
	*traveltime = MB_YES;
	*beam_flagging = MB_YES;
	*nav_source = MB_DATA_NAV;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_DATA;
	*beamwidth_xtrack = 2.0;
	*beamwidth_ltrack = 2.0;

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
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
		fprintf(stderr,"dbg2       heading_source:     %d\n",*heading_source);
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
int mbr_alm_emoldraw(int verbose, void *mbio_ptr, int *error)
{
	static char res_id[]="$Id: mbr_emoldraw.c,v 5.3 2002-07-20 20:42:40 caress Exp $";
	char	*function_name = "mbr_alm_emoldraw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int	*wrapper;
	double	*pixel_size;
	double	*swath_width;
	int	*num_bathrec;
	int	*num_ssrec;
	int	*file_has_ss;

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

	/* set initial status */
	status = MB_SUCCESS;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = 0;
	mb_io_ptr->data_structure_size = 0;
	status = mbsys_simrad_alloc(
			verbose,mbio_ptr,
			&mb_io_ptr->store_data,error);

	/* initialize saved variables */
	wrapper = (int *) &mb_io_ptr->save5;
	pixel_size = (double *) &mb_io_ptr->saved1;
	swath_width = (double *) &mb_io_ptr->saved2;
	num_bathrec = (int *) &mb_io_ptr->save6;
	num_ssrec = (int *) &mb_io_ptr->save7;
	file_has_ss = (int *) &mb_io_ptr->save8;
	*wrapper = -1;
	*pixel_size = 0.0;
	*swath_width = 0.0;
	*num_bathrec = 0;
	*num_ssrec = 0;
	*file_has_ss = MB_YES;

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
int mbr_dem_emoldraw(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_emoldraw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		}

	/* get pointers to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* deallocate memory for data descriptor */
	status = mbsys_simrad_deall(
			verbose,mbio_ptr,
			&mb_io_ptr->store_data,error);

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
int mbr_rt_emoldraw(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_emoldraw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad_struct *store;
	struct mbsys_simrad_survey_struct *ping;
	int	ntime_i[7];
	double	ntime_d;
	int	ptime_i[7];
	double	ptime_d;
	double	rawspeed, pheading;
	double	plon, plat, pspeed;
	double	*pixel_size;
	double	*swath_width;
	int	i, j, k;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointers to mbio descriptor and data structures */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	store = (struct mbsys_simrad_struct *) store_ptr;
	pixel_size = (double *) &mb_io_ptr->saved1;
	swath_width = (double *) &mb_io_ptr->saved2;

	/* read next data from file */
	status = mbr_emoldraw_rd_data(verbose,mbio_ptr,store_ptr,error);

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = store->kind;

	/* save fix if nav data */
	if (status == MB_SUCCESS
		&& store->kind == MB_DATA_NAV)
		{							
		/* get nav time */
		mb_fix_y2k(verbose, store->pos_year, 
			    &ntime_i[0]);
		ntime_i[1] = store->pos_month;
		ntime_i[2] = store->pos_day;
		ntime_i[3] = store->pos_hour;
		ntime_i[4] = store->pos_minute;
		ntime_i[5] = store->pos_second;
		ntime_i[6] = 10000 * store->pos_centisecond;
		mb_get_time(verbose, ntime_i, &ntime_d);
		
		/* add latest fix */
		mb_navint_add(verbose, mbio_ptr, 
			ntime_d, 
			store->pos_longitude, 
			store->pos_latitude, 
			error);
		}

	/* handle navigation interpolation and generate sidescan */
	if (status == MB_SUCCESS
		&& store->kind == MB_DATA_DATA)
		{
		/* get ping structure */
		ping = (struct mbsys_simrad_survey_struct *) store->ping;
		
		/* get ping time */
		mb_fix_y2k(verbose, store->year, 
			    &ptime_i[0]);
		ptime_i[1] = store->month;
		ptime_i[2] = store->day;
		ptime_i[3] = store->hour;
		ptime_i[4] = store->minute;
		ptime_i[5] = store->second;
		ptime_i[6] = 10000*store->centisecond;
		mb_get_time(verbose, ptime_i, &ptime_d);

		/* interpolate from saved nav */
		rawspeed =  3.6 * store->speed;
		pheading = store->line_heading;
		mb_navint_interp(verbose, mbio_ptr, ptime_d, pheading, rawspeed, 
				    &plon, &plat, &pspeed, error);

		/* handle lon flipping */
		if (mb_io_ptr->lonflip < 0)
			{
			if (plon > 0.) 
				plon = plon - 360.;
			else if (plon < -360.)
				plon = plon + 360.;
			}
		else if (mb_io_ptr->lonflip == 0)
			{
			if (plon > 180.) 
				plon = plon - 360.;
			else if (plon < -180.)
				plon = plon + 360.;
			}
		else
			{
			if (plon > 360.) 
				plon = plon - 360.;
			else if (plon < 0.)
				plon = plon + 360.;
			}

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"dbg4       Interpolated Navigation:\n");
			fprintf(stderr,"dbg4       longitude:  %f\n", plon);
			fprintf(stderr,"dbg4       latitude:   %f\n", plat);
			fprintf(stderr,"dbg4       speed:      %f\n", pspeed);
			}
			
		/* adopt interpolated navigation */
		ping->longitude = plon;
		ping->latitude = plat;

		/* generate sidescan */
		ping->pixel_size = 0.0;
		ping->pixels_ss = 0;			
		status = mbsys_simrad_makess(verbose,
				mbio_ptr, store_ptr,
				MB_NO, pixel_size, 
				MB_NO, swath_width, 
				0, 
				error);
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
int mbr_wt_emoldraw(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_emoldraw";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad_struct *store;
	struct mbsys_simrad_survey_struct *ping;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	store = (struct mbsys_simrad_struct *) store_ptr;

	/* write next data to file */
	status = mbr_emoldraw_wr_data(verbose,mbio_ptr,store_ptr,error);

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
int mbr_emoldraw_rd_data(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_emoldraw_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad_struct *store;
	FILE	*mbfp;
	int	done;
	int	*wrapper;
	char	*label;
	int	*label_save_flag;
	short int expect;
	short int *type;
	short int first_type;
	int	first_ss;
	int	more_ss;
	short int *expect_save;
	int	*expect_save_flag;
	short int *first_type_save;
	int	*first_ss_save;
	int	*more_ss_save;
	int	*num_bathrec;
	int	*num_ssrec;
	int	*file_has_ss;
	int	read_len;
	int	skip = 0;
	int	i;
	
	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	mbfp = mb_io_ptr->mbfp;
	store = (struct mbsys_simrad_struct *) store_ptr;
	
	/* get saved values */
	wrapper = (int *) &mb_io_ptr->save5;
	label = (char *) mb_io_ptr->save_label;
	type = (short int *) mb_io_ptr->save_label;
	label_save_flag = (int *) &mb_io_ptr->save_label_flag;
	expect_save_flag = (int *) &mb_io_ptr->save_flag;
	expect_save = (short int *) &mb_io_ptr->save1;
	first_type_save = (short int *) &mb_io_ptr->save2;
	first_ss_save = (int *) &mb_io_ptr->save3;
	more_ss_save = (int *) &mb_io_ptr->save4;
	num_bathrec = (int *) &mb_io_ptr->save6;
	num_ssrec = (int *) &mb_io_ptr->save7;
	file_has_ss = (int *) &mb_io_ptr->save8;
	if (*expect_save_flag == MB_YES)
		{
		expect = *expect_save;
		first_type = *first_type_save;
		first_ss = *first_ss_save;
		more_ss = *more_ss_save;
		*expect_save_flag = MB_NO;
		}
	else
		{
		expect = EM_NONE;
		first_type = EM_NONE;
		first_ss = MB_YES;
		more_ss = MB_NO;
		}
		
	/* check if sidescan is to be expected */
	if (*num_bathrec > 3 && *num_ssrec == 0)
		*file_has_ss = MB_NO;
	else
		*file_has_ss = MB_YES;

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* loop over reading data until a record is ready for return */
	done = MB_NO;
	*error = MB_ERROR_NO_ERROR;
	while (done == MB_NO)
		{
		/* if no label saved get next record label */
		if (*label_save_flag == MB_NO)
			{
			/* read four byte wrapper if data stream is known
				to have wrappers */
			if (*wrapper == MB_YES)
				{
				if ((read_len = fread(label,
					1,4,mb_io_ptr->mbfp)) != 4)
					{
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
					}
				}
				
			/* look for label */
			if (status == MB_SUCCESS
				&& (read_len = fread(label,
					1,2,mb_io_ptr->mbfp)) != 2)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				}

			/* check label - if not a good label read a byte 
				at a time until a good label is found */
			skip = 0;
			while (status == MB_SUCCESS
				&& mbr_emoldraw_chk_label(verbose, 
					mbio_ptr, *type) != MB_SUCCESS)
			    {
			    /* get next byte */
			    label[0] = label[1];
			    if ((read_len = fread(&label[1],
				    1,1,mb_io_ptr->mbfp)) != 1)
					{
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
					}
			    skip++;
			    }

			/* set wrapper status if needed */
			if (*wrapper < 0)
				{
				if (skip == 0) 
					*wrapper = MB_NO;
				else if (skip == 4)
					*wrapper = MB_YES;
				}
			}
		
		/* else use saved label */
		else
			*label_save_flag = MB_NO;

		/* swap bytes if necessary */
#ifdef BYTESWAPPED
		*type = (short int) mb_swap_short(*type);
#endif

#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"\nstart of mbr_emoldraw_rd_data loop:\n");
	fprintf(stderr,"done:%d\n",done);
	fprintf(stderr,"wrapper:%d\n",*wrapper);
	fprintf(stderr,"num_bathrec:%d\n",*num_bathrec);
	fprintf(stderr,"num_ssrec:%d\n",*num_ssrec);
	fprintf(stderr,"file_has_ss:%d\n",*file_has_ss);
	fprintf(stderr,"skip:%d\n",skip);
	fprintf(stderr,"expect:%x\n",expect);
	fprintf(stderr,"type:%x\n",*type);
	fprintf(stderr,"first_type:%x\n",first_type);
	fprintf(stderr,"first_ss:%d\n",first_ss);
	fprintf(stderr,"more_ss:%d\n",more_ss);
#endif
		
		/* allocate secondary data structure for
			survey data if needed */
		if (status == MB_SUCCESS && 
			(*type == EM_12S_BATH
			|| *type == EM_12DP_BATH
			|| *type == EM_12DS_BATH
			|| *type == EM_121_BATH
			|| *type == EM_1000_BATH
			|| *type == EM_12S_SS
			|| *type == EM_12DP_SS
			|| *type == EM_12DS_SS
			|| *type == EM_12S_SSP
			|| *type == EM_12DP_SSP
			|| *type == EM_12DS_SSP)
			&& store->ping == NULL)
			{
			status = mbsys_simrad_survey_alloc(
					verbose,mbio_ptr,
					store_ptr,error);
			}

		/* read the appropriate data records */
		if (status == MB_FAILURE && expect == EM_NONE)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call nothing, read failure, no expect\n");
#endif
			done = MB_YES;
			}
		else if (status == MB_FAILURE && expect != EM_NONE)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call nothing, read failure, expect %x\n",expect);
#endif
			store->kind = MB_DATA_DATA;
			done = MB_YES;
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else if (*type != EM_START
			&& *type != EM_STOP
			&& *type != EM_PARAMETER
			&& *type != EM_POS
			&& *type != EM_SVP
			&& *type != EM_12S_BATH
			&& *type != EM_12DP_BATH
			&& *type != EM_12DS_BATH
			&& *type != EM_121_BATH
			&& *type != EM_1000_BATH
			&& *type != EM_12S_SS
			&& *type != EM_12DP_SS
			&& *type != EM_12DS_SS
			&& *type != EM_12S_SSP
			&& *type != EM_12DP_SSP
			&& *type != EM_12DS_SSP)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call nothing, try again\n");
#endif
			done = MB_NO;
			}
		else if (*type == EM_START)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call mbr_emoldraw_rd_start type %x\n",*type);
#endif
			status = mbr_emoldraw_rd_start(
				verbose,mbfp,store,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				store->kind = MB_DATA_START;
				if (expect != EM_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					*first_ss_save = first_ss;
					*more_ss_save = more_ss;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (*type == EM_STOP 
			&& expect != EM_NONE)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call nothing, expect %x but got type %x\n",expect,*type);
#endif
			store->kind = MB_DATA_DATA;
			done = MB_YES;
			expect = EM_NONE;
			*label_save_flag = MB_YES;
			}
		else if (*type == EM_STOP)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call mbr_emoldraw_rd_stop type %x\n",*type);
#endif
			status = mbr_emoldraw_rd_stop(
				verbose,mbfp,store,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				store->kind = MB_DATA_STOP;
				if (expect != EM_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					*first_ss_save = first_ss;
					*more_ss_save = more_ss;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (*type == EM_PARAMETER)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call mbr_emoldraw_rd_parameter type %x\n",*type);
#endif
			status = mbr_emoldraw_rd_parameter(
				verbose,mbfp,store,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				store->kind = MB_DATA_COMMENT;
				if (expect != EM_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					*first_ss_save = first_ss;
					*more_ss_save = more_ss;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (*type == EM_POS)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call mbr_emoldraw_rd_pos type %x\n",*type);
#endif
			status = mbr_emoldraw_rd_pos(
				verbose,mbfp,store,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				store->kind = MB_DATA_NAV;
				if (expect != EM_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					*first_ss_save = first_ss;
					*more_ss_save = more_ss;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (*type == EM_SVP)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call mbr_emoldraw_rd_svp type %x\n",*type);
#endif
			status = mbr_emoldraw_rd_svp(
				verbose,mbfp,store,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				store->kind = MB_DATA_VELOCITY_PROFILE;
				if (expect != EM_NONE)
					{
					*expect_save = expect;
					*expect_save_flag = MB_YES;
					*first_type_save = first_type;
					*first_ss_save = first_ss;
					*more_ss_save = more_ss;
					}
				else
					*expect_save_flag = MB_NO;
				}
			}
		else if (*type == EM_12S_BATH 
			&& expect != EM_NONE 
			&& expect != EM_12S_BATH)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call nothing, expect %x but got type %x\n",expect,*type);
#endif
			store->kind = MB_DATA_DATA;
			done = MB_YES;
			expect = EM_NONE;
			*label_save_flag = MB_YES;
			}
		else if (*type == EM_12S_BATH)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call mbr_emoldraw_rd_em12bath type %x\n",*type);
#endif
			status = mbr_emoldraw_rd_em12bath(
				verbose,mbfp,store,
				EM_SWATH_CENTER,error);
			if (status == MB_SUCCESS)
				{
				(*num_bathrec)++;
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"check num_bathrec:%d\n",*num_bathrec);
#endif
				store->kind = MB_DATA_DATA;
				if (first_type == EM_NONE
					&& *file_has_ss == MB_YES)
					{
					done = MB_NO;
					first_type = EM_12S_BATH;
					expect = EM_12S_SS;
					}
				else
					{
					done = MB_YES;
					expect = EM_NONE;
					}
				}
			}
		else if (*type == EM_12DP_BATH 
			&& expect != EM_NONE 
			&& expect != EM_12DP_BATH)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call nothing, expect %x but got type %x\n",expect,*type);
#endif
			store->kind = MB_DATA_DATA;
			done = MB_YES;
			expect = EM_NONE;
			*label_save_flag = MB_YES;
			}
		else if (*type == EM_12DP_BATH)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call mbr_emoldraw_rd_em12bath type %x\n",*type);
#endif
			status = mbr_emoldraw_rd_em12bath(
				verbose,mbfp,store,
				EM_SWATH_PORT,error);
			if (status == MB_SUCCESS)
				{
				(*num_bathrec)++;
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"num_bathrec:%d\n",*num_bathrec);
#endif
				store->kind = MB_DATA_DATA;
				if (first_type == EM_NONE
					&& *file_has_ss == MB_YES)
					{
					done = MB_NO;
					first_type = EM_12DP_BATH;
					expect = EM_12DP_SS;
					}
				else
					{
					done = MB_YES;
					expect = EM_NONE;
					}
				}
			}
		else if (*type == EM_12DS_BATH 
			&& expect != EM_NONE 
			&& expect != EM_12DS_BATH)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call nothing, expect %x but got type %x\n",expect,*type);
#endif
			store->kind = MB_DATA_DATA;
			done = MB_YES;
			expect = EM_NONE;
			*label_save_flag = MB_YES;
			}
		else if (*type == EM_12DS_BATH)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call mbr_emoldraw_rd_em12bath type %x\n",*type);
#endif
			status = mbr_emoldraw_rd_em12bath(
				verbose,mbfp,store,
				EM_SWATH_STARBOARD,error);
			if (status == MB_SUCCESS)
				{
				(*num_bathrec)++;
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"num_bathrec:%d\n",*num_bathrec);
#endif
				store->kind = MB_DATA_DATA;
				if (first_type == EM_NONE
					&& *file_has_ss == MB_YES)
					{
					done = MB_NO;
					first_type = EM_12DS_BATH;
					expect = EM_12DS_SS;
					}
				else
					{
					done = MB_YES;
					expect = EM_NONE;
					}
				}
			}
		else if (*type == EM_121_BATH 
			&& expect != EM_NONE 
			&& expect != EM_12S_BATH)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call nothing, expect %x but got type %x\n",expect,*type);
#endif
			store->kind = MB_DATA_DATA;
			done = MB_YES;
			expect = EM_NONE;
			*label_save_flag = MB_YES;
			}
		else if (*type == EM_121_BATH)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call mbr_emoldraw_rd_em121bath type %x\n",*type);
#endif
			status = mbr_emoldraw_rd_em121bath(
				verbose,mbfp,store,error);
			if (status == MB_SUCCESS)
				{
				(*num_bathrec)++;
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"num_bathrec:%d\n",*num_bathrec);
#endif
				store->kind = MB_DATA_DATA;
				if (first_type == EM_NONE
					&& *file_has_ss == MB_YES)
					{
					done = MB_NO;
					first_type = EM_121_BATH;
					expect = EM_12S_SS;
					}
				else
					{
					done = MB_YES;
					expect = EM_NONE;
					}
				}
			}
		else if (*type == EM_1000_BATH 
			&& expect != EM_NONE 
			&& expect != EM_12S_BATH)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call nothing, expect %x but got type %x\n",expect,*type);
#endif
			store->kind = MB_DATA_DATA;
			done = MB_YES;
			expect = EM_NONE;
			*label_save_flag = MB_YES;
			}
		else if (*type == EM_1000_BATH)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call mbr_emoldraw_rd_em1000bath type %x\n",*type);
#endif
			status = mbr_emoldraw_rd_em1000bath(
				verbose,mbfp,store,error);
			if (status == MB_SUCCESS)
				{
				(*num_bathrec)++;
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"num_bathrec:%d\n",*num_bathrec);
#endif
				store->kind = MB_DATA_DATA;
				if (first_type == EM_NONE
					&& *file_has_ss == MB_YES)
					{
					done = MB_NO;
					first_type = EM_12DS_BATH;
					expect = EM_12S_SS;
					}
				else
					{
					done = MB_YES;
					expect = EM_NONE;
					}
				}
			}
		else if (*type == EM_12S_SS 
			&& expect != EM_NONE 
			&& expect != EM_12S_SS)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call nothing, expect %x but got type %x\n",expect,*type);
#endif
			store->kind = MB_DATA_DATA;
			done = MB_YES;
			expect = EM_NONE;
			*label_save_flag = MB_YES;
			}
		else if (*type == EM_12S_SS)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call mbr_emoldraw_rd_ss type %x\n",*type);
#endif
			status = mbr_emoldraw_rd_ss(
				verbose,mbfp,store,
				EM_SWATH_CENTER,
				first_ss,&more_ss,error);
			if (status == MB_SUCCESS
				&& first_ss == MB_YES)
				(*num_ssrec)++;
			if (status == MB_SUCCESS 
				&& more_ss == MB_NO)
				{
				*file_has_ss = MB_YES;
				if (first_type == EM_NONE)
					{
					done = MB_NO;
					first_type = EM_12S_SS;
					expect = EM_12S_BATH;
					}
				else
					{
					done = MB_YES;
					expect = EM_NONE;
					}
				first_ss = MB_YES;
				}
			else if (status == MB_SUCCESS 
				&& more_ss == MB_YES)
				{
				done = MB_NO;
				expect = EM_12S_SS;
				first_ss = MB_NO;
				}
			else if (status == MB_FAILURE)
				{
				if (first_type == EM_NONE)
					{
					done = MB_NO;
					first_type = EM_12S_SS;
					expect = EM_12S_BATH;
					}
				else
					{
					done = MB_YES;
					expect = EM_NONE;
					}
				first_ss = MB_YES;
				}
			}
		else if (*type == EM_12DP_SS 
			&& expect != EM_NONE 
			&& expect != EM_12DP_SS)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call nothing, expect %x but got type %x\n",expect,*type);
#endif
			store->kind = MB_DATA_DATA;
			done = MB_YES;
			expect = EM_NONE;
			*label_save_flag = MB_YES;
			}
		else if (*type == EM_12DP_SS)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call mbr_emoldraw_rd_ss type %x\n",*type);
#endif
			status = mbr_emoldraw_rd_ss(
				verbose,mbfp,store,
				EM_SWATH_PORT,
				first_ss,&more_ss,error);
			if (status == MB_SUCCESS
				&& first_ss == MB_YES)
				(*num_ssrec)++;
			if (status == MB_SUCCESS 
				&& more_ss == MB_NO)
				{
				*file_has_ss = MB_YES;
				if (first_type == EM_NONE)
					{
					done = MB_NO;
					first_type = EM_12DP_SS;
					expect = EM_12DP_BATH;
					}
				else
					{
					done = MB_YES;
					expect = EM_NONE;
					}
				first_ss = MB_YES;
				}
			else if (status == MB_SUCCESS 
				&& more_ss == MB_YES)
				{
				done = MB_NO;
				expect = EM_12DP_SS;
				first_ss = MB_NO;
				}
			else if (status == MB_FAILURE)
				{
				if (first_type == EM_NONE)
					{
					done = MB_NO;
					first_type = EM_12DP_SS;
					expect = EM_12DP_BATH;
					}
				else
					{
					done = MB_YES;
					expect = EM_NONE;
					}
				first_ss = MB_YES;
				}
			}

		else if (*type == EM_12DS_SS 
			&& expect != EM_NONE 
			&& expect != EM_12DS_SS)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call nothing, expect %x but got type %x\n",expect,*type);
#endif
			store->kind = MB_DATA_DATA;
			done = MB_YES;
			expect = EM_NONE;
			*label_save_flag = MB_YES;
			}
		else if (*type == EM_12DS_SS)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call mbr_emoldraw_rd_ss type %x\n",*type);
#endif
			status = mbr_emoldraw_rd_ss(
				verbose,mbfp,store,
				EM_SWATH_STARBOARD,
				first_ss,&more_ss,error);
			if (status == MB_SUCCESS
				&& first_ss == MB_YES)
				(*num_ssrec)++;
			if (status == MB_SUCCESS 
				&& more_ss == MB_NO)
				{
				*file_has_ss = MB_YES;
				if (first_type == EM_NONE)
					{
					done = MB_NO;
					first_type = EM_12DS_SS;
					expect = EM_12DS_BATH;
					}
				else
					{
					done = MB_YES;
					expect = EM_NONE;
					}
				first_ss = MB_YES;
				}
			else if (status == MB_SUCCESS 
				&& more_ss == MB_YES)
				{
				done = MB_NO;
				expect = EM_12DS_SS;
				first_ss = MB_NO;
				}
			else if (status == MB_FAILURE)
				{
				if (first_type == EM_NONE)
					{
					done = MB_NO;
					first_type = EM_12DS_SS;
					expect = EM_12DS_BATH;
					}
				else
					{
					done = MB_YES;
					expect = EM_NONE;
					}
				first_ss = MB_YES;
				}
			}
		else if (*type == EM_12S_SSP 
			&& expect != EM_NONE 
			&& expect != EM_12S_SS)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call nothing, expect %x but got type %x\n",expect,*type);
#endif
			store->kind = MB_DATA_DATA;
			done = MB_YES;
			expect = EM_NONE;
			*label_save_flag = MB_YES;
			}
		else if (*type == EM_12S_SSP)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call mbr_emoldraw_rd_ssp type %x\n",*type);
#endif
			status = mbr_emoldraw_rd_ssp(
				verbose,mbfp,store,
				EM_SWATH_CENTER,
				first_ss,&more_ss,error);
			if (status == MB_SUCCESS
				&& first_ss == MB_YES)
				(*num_ssrec)++;
			if (status == MB_SUCCESS 
				&& more_ss == MB_NO)
				{
				*file_has_ss = MB_YES;
				if (first_type == EM_NONE)
					{
					done = MB_NO;
					first_type = EM_12S_SSP;
					expect = EM_12S_BATH;
					}
				else
					{
					done = MB_YES;
					expect = EM_NONE;
					}
				first_ss = MB_YES;
				}
			else if (status == MB_SUCCESS 
				&& more_ss == MB_YES)
				{
				done = MB_NO;
				expect = EM_12S_SS;
				first_ss = MB_NO;
				}
			else if (status == MB_FAILURE)
				{
				if (first_type == EM_NONE)
					{
					done = MB_NO;
					first_type = EM_12S_SSP;
					expect = EM_12S_BATH;
					}
				else
					{
					done = MB_YES;
					expect = EM_NONE;
					}
				first_ss = MB_YES;
				}
			}
		else if (*type == EM_12DP_SSP 
			&& expect != EM_NONE 
			&& expect != EM_12DP_SS)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call nothing, expect %x but got type %x\n",expect,*type);
#endif
			store->kind = MB_DATA_DATA;
			done = MB_YES;
			expect = EM_NONE;
			*label_save_flag = MB_YES;
			}
		else if (*type == EM_12DP_SSP)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call mbr_emoldraw_rd_ssp type %x\n",*type);
#endif
			status = mbr_emoldraw_rd_ssp(
				verbose,mbfp,store,
				EM_SWATH_PORT,
				first_ss,&more_ss,error);
			if (status == MB_SUCCESS
				&& first_ss == MB_YES)
				(*num_ssrec)++;
			if (status == MB_SUCCESS 
				&& more_ss == MB_NO)
				{
				*file_has_ss = MB_YES;
				if (first_type == EM_NONE)
					{
					done = MB_NO;
					first_type = EM_12DP_SSP;
					expect = EM_12DP_BATH;
					}
				else
					{
					done = MB_YES;
					expect = EM_NONE;
					}
				first_ss = MB_YES;
				}
			else if (status == MB_SUCCESS 
				&& more_ss == MB_YES)
				{
				done = MB_NO;
				expect = EM_12DP_SS;
				first_ss = MB_NO;
				}
			else if (status == MB_FAILURE)
				{
				if (first_type == EM_NONE)
					{
					done = MB_NO;
					first_type = EM_12DP_SSP;
					expect = EM_12DP_BATH;
					}
				else
					{
					done = MB_YES;
					expect = EM_NONE;
					}
				first_ss = MB_YES;
				}
			}

		else if (*type == EM_12DS_SSP 
			&& expect != EM_NONE 
			&& expect != EM_12DS_SS)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call nothing, expect %x but got type %x\n",expect,*type);
#endif
			store->kind = MB_DATA_DATA;
			done = MB_YES;
			expect = EM_NONE;
			*label_save_flag = MB_YES;
			}
		else if (*type == EM_12DS_SSP)
			{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call mbr_emoldraw_rd_ssp type %x\n",*type);
#endif
			status = mbr_emoldraw_rd_ssp(
				verbose,mbfp,store,
				EM_SWATH_STARBOARD,
				first_ss,&more_ss,error);
			if (status == MB_SUCCESS
				&& first_ss == MB_YES)
				(*num_ssrec)++;
			if (status == MB_SUCCESS 
				&& more_ss == MB_NO)
				{
				*file_has_ss = MB_YES;
				if (first_type == EM_NONE)
					{
					done = MB_NO;
					first_type = EM_12DS_SSP;
					expect = EM_12DS_BATH;
					}
				else
					{
					done = MB_YES;
					expect = EM_NONE;
					}
				first_ss = MB_YES;
				}
			else if (status == MB_SUCCESS 
				&& more_ss == MB_YES)
				{
				done = MB_NO;
				expect = EM_12DS_SS;
				first_ss = MB_NO;
				}
			else if (status == MB_FAILURE)
				{
				if (first_type == EM_NONE)
					{
					done = MB_NO;
					first_type = EM_12DS_SSP;
					expect = EM_12DS_BATH;
					}
				else
					{
					done = MB_YES;
					expect = EM_NONE;
					}
				first_ss = MB_YES;
				}
			}

		/* bail out if there is an error */
		if (status == MB_FAILURE)
			done = MB_YES;

#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"end of mbr_emoldraw_rd_data loop:\n");
	fprintf(stderr,"status:%d error:%d\n",status, *error);
	fprintf(stderr,"done:%d\n",done);
	fprintf(stderr,"expect:%x\n",expect);
	fprintf(stderr,"type:%x\n",*type);
#endif
		}
		
	/* get file position */
	if (*label_save_flag == MB_YES)
		mb_io_ptr->file_bytes = ftell(mbfp) - 2;
	else if (*expect_save_flag != MB_YES)
		mb_io_ptr->file_bytes = ftell(mbfp);

	/* swap label if saved and byteswapped */
#ifdef BYTESWAPPED
	if (*label_save_flag == MB_YES)
		*type = (short int) mb_swap_short(*type);
#endif


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
int mbr_emoldraw_chk_label(int verbose, void *mbio_ptr, short type)
{
	char	*function_name = "mbr_emoldraw_chk_label";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	char	*startid;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       type:       %d\n",type);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

		/* swap bytes if necessary */
#ifdef BYTESWAPPED
		type = (short) mb_swap_short(type);
	if (verbose >= 2)
		{
		fprintf(stderr,"dbg2  Input values byte swapped:\n");
		fprintf(stderr,"dbg2       type:       %d\n",type);
		}
#endif

	/* check for valid label */
	if (type != EM_START
		&& type != EM_STOP
		&& type != EM_PARAMETER
		&& type != EM_POS
		&& type != EM_SVP
		&& type != EM_12DS_BATH
		&& type != EM_12DP_BATH
		&& type != EM_12S_BATH
		&& type != EM_121_BATH
		&& type != EM_1000_BATH
		&& type != EM_12DP_SS
		&& type != EM_12DS_SS
		&& type != EM_12S_SS
		&& type != EM_12DP_SSP
		&& type != EM_12DS_SSP
		&& type != EM_12S_SSP)
		{
		status = MB_FAILURE;
		startid = (char *) &type;
		if (verbose >= 1 && *startid == 2)
			{
			fprintf(stderr, "Bad datagram type: %4.4hX  %d\n", type, type);
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_emoldraw_rd_start(int verbose, FILE *mbfp, 
		struct mbsys_simrad_struct *store, int *error)
{
	char	*function_name = "mbr_emoldraw_rd_start";
	int	status = MB_SUCCESS;
	char	line[EM_START_SIZE];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}

	/* read record into char array */
	status = fread(line,1,EM_START_SIZE,mbfp);
	if (status == EM_START_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get data */
	if (status == MB_SUCCESS)
		{
		store->kind = MB_DATA_START;
		mb_get_int(&(store->par_day),          line,      2);
		mb_get_int(&(store->par_month),        line+2,    2);
		mb_get_int(&(store->par_year),         line+4,    2);
		mb_get_int(&(store->par_hour),         line+7,    2);
		mb_get_int(&(store->par_minute),       line+9,    2);
		mb_get_int(&(store->par_second),       line+11,   2);
		mb_get_int(&(store->par_centisecond),  line+13,   2);
		mb_get_int(&(store->pos_type),         line+20,   1);
		mb_get_double(&(store->pos_delay),     line+26,   5);
		mb_get_double(&(store->roll_offset),   line+36,   5);
		mb_get_double(&(store->pitch_offset),  line+46,   5);
		mb_get_double(&(store->heading_offset),line+56,   5);
		mb_get_double(&(store->em100_td),      line+70,   5);
		mb_get_double(&(store->em100_tx),      line+84,   5);
		mb_get_double(&(store->em100_ty),      line+98,   5);
		mb_get_double(&(store->em12_td),       line+111,  5);
		mb_get_double(&(store->em12_tx),       line+124,  5);
		mb_get_double(&(store->em12_ty),       line+137,  5);
		mb_get_double(&(store->em1000_td),     line+152,  5);
		mb_get_double(&(store->em1000_tx),     line+167,  5);
		mb_get_double(&(store->em1000_ty),     line+182,  5);
		for(i=0;i<128;i++)
			store->spare_parameter[i] = line[188+i];
		mb_get_int(&(store->survey_line),      line+328,  4);
		for(i=0;i<80;i++)
			store->comment[i] = line[341+i];
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",store->par_year);
		fprintf(stderr,"dbg5       month:            %d\n",store->par_month);
		fprintf(stderr,"dbg5       day:              %d\n",store->par_day);
		fprintf(stderr,"dbg5       hour:             %d\n",store->par_hour);
		fprintf(stderr,"dbg5       minute:           %d\n",store->par_minute);
		fprintf(stderr,"dbg5       sec:              %d\n",store->par_second);
		fprintf(stderr,"dbg5       centisecond:      %d\n",store->par_centisecond);
		fprintf(stderr,"dbg5       pos_type:         %d\n",store->pos_type);
		fprintf(stderr,"dbg5       pos_delay:        %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       roll_offset:      %f\n",store->roll_offset);
		fprintf(stderr,"dbg5       pitch_offset:     %f\n",store->pitch_offset);
		fprintf(stderr,"dbg5       heading_offset:   %f\n",store->heading_offset);
		fprintf(stderr,"dbg5       em100_td:         %f\n",store->em100_td);
		fprintf(stderr,"dbg5       em100_tx:         %f\n",store->em100_tx);
		fprintf(stderr,"dbg5       em100_ty:         %f\n",store->em100_ty);
		fprintf(stderr,"dbg5       em12_td:          %f\n",store->em12_td);
		fprintf(stderr,"dbg5       em12_tx:          %f\n",store->em12_tx);
		fprintf(stderr,"dbg5       em12_ty:          %f\n",store->em12_ty);
		fprintf(stderr,"dbg5       em1000_td:        %f\n",store->em1000_td);
		fprintf(stderr,"dbg5       em1000_tx:        %f\n",store->em1000_tx);
		fprintf(stderr,"dbg5       em1000_ty:        %f\n",store->em1000_ty);
		fprintf(stderr,"dbg5       survey_line:      %d\n",store->survey_line);
		fprintf(stderr,"dbg5       comment:          %s\n",store->comment);
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
int mbr_emoldraw_rd_stop(int verbose, FILE *mbfp, 
		struct mbsys_simrad_struct *store, int *error)
{
	char	*function_name = "mbr_emoldraw_rd_stop";
	int	status = MB_SUCCESS;
	char	line[EM_STOP_SIZE];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}

	/* read record into char array */
	status = fread(line,1,EM_STOP_SIZE,mbfp);
	if (status == EM_STOP_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get data */
	if (status == MB_SUCCESS)
		{
		store->kind = MB_DATA_STOP;
		mb_get_int(&(store->par_day),          line,      2);
		mb_get_int(&(store->par_month),        line+2,    2);
		mb_get_int(&(store->par_year),         line+4,    2);
		mb_get_int(&(store->par_hour),         line+7,    2);
		mb_get_int(&(store->par_minute),       line+9,    2);
		mb_get_int(&(store->par_second),       line+11,   2);
		mb_get_int(&(store->par_centisecond),  line+13,   2);
		mb_get_int(&(store->pos_type),         line+20,   1);
		mb_get_double(&(store->pos_delay),     line+26,   5);
		mb_get_double(&(store->roll_offset),   line+36,   5);
		mb_get_double(&(store->pitch_offset),  line+46,   5);
		mb_get_double(&(store->heading_offset),line+56,   5);
		mb_get_double(&(store->em100_td),      line+70,   5);
		mb_get_double(&(store->em100_tx),      line+84,   5);
		mb_get_double(&(store->em100_ty),      line+98,   5);
		mb_get_double(&(store->em12_td),       line+111,  5);
		mb_get_double(&(store->em12_tx),       line+124,  5);
		mb_get_double(&(store->em12_ty),       line+137,  5);
		mb_get_double(&(store->em1000_td),     line+152,  5);
		mb_get_double(&(store->em1000_tx),     line+167,  5);
		mb_get_double(&(store->em1000_ty),     line+182,  5);
		for(i=0;i<128;i++)
			store->spare_parameter[i] = line[188+i];
		mb_get_int(&(store->survey_line),      line+328,  4);
		for(i=0;i<80;i++)
			store->comment[i] = line[341+i];
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",store->par_year);
		fprintf(stderr,"dbg5       month:            %d\n",store->par_month);
		fprintf(stderr,"dbg5       day:              %d\n",store->par_day);
		fprintf(stderr,"dbg5       hour:             %d\n",store->par_hour);
		fprintf(stderr,"dbg5       minute:           %d\n",store->par_minute);
		fprintf(stderr,"dbg5       sec:              %d\n",store->par_second);
		fprintf(stderr,"dbg5       centisecond:      %d\n",store->par_centisecond);
		fprintf(stderr,"dbg5       pos_type:         %d\n",store->pos_type);
		fprintf(stderr,"dbg5       pos_delay:        %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       roll_offset:      %f\n",store->roll_offset);
		fprintf(stderr,"dbg5       pitch_offset:     %f\n",store->pitch_offset);
		fprintf(stderr,"dbg5       heading_offset:   %f\n",store->heading_offset);
		fprintf(stderr,"dbg5       em100_td:         %f\n",store->em100_td);
		fprintf(stderr,"dbg5       em100_tx:         %f\n",store->em100_tx);
		fprintf(stderr,"dbg5       em100_ty:         %f\n",store->em100_ty);
		fprintf(stderr,"dbg5       em12_td:          %f\n",store->em12_td);
		fprintf(stderr,"dbg5       em12_tx:          %f\n",store->em12_tx);
		fprintf(stderr,"dbg5       em12_ty:          %f\n",store->em12_ty);
		fprintf(stderr,"dbg5       em1000_td:        %f\n",store->em1000_td);
		fprintf(stderr,"dbg5       em1000_tx:        %f\n",store->em1000_tx);
		fprintf(stderr,"dbg5       em1000_ty:        %f\n",store->em1000_ty);
		fprintf(stderr,"dbg5       survey_line:      %d\n",store->survey_line);
		fprintf(stderr,"dbg5       comment:          %s\n",store->comment);
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
int mbr_emoldraw_rd_parameter(int verbose, FILE *mbfp, 
		struct mbsys_simrad_struct *store, int *error)
{
	char	*function_name = "mbr_emoldraw_rd_parameter";
	int	status = MB_SUCCESS;
	char	line[EM_PARAMETER_SIZE];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}

	/* read record into char array */
	status = fread(line,1,EM_PARAMETER_SIZE,mbfp);
	if (status == EM_PARAMETER_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get data */
	if (status == MB_SUCCESS)
		{
		store->kind = MB_DATA_COMMENT;
		mb_get_int(&(store->par_day),          line,      2);
		mb_get_int(&(store->par_month),        line+2,    2);
		mb_get_int(&(store->par_year),         line+4,    2);
		mb_get_int(&(store->par_hour),         line+7,    2);
		mb_get_int(&(store->par_minute),       line+9,    2);
		mb_get_int(&(store->par_second),       line+11,   2);
		mb_get_int(&(store->par_centisecond),  line+13,   2);
		mb_get_int(&(store->pos_type),         line+20,   1);
		mb_get_double(&(store->pos_delay),     line+26,   5);
		mb_get_double(&(store->roll_offset),   line+36,   5);
		mb_get_double(&(store->pitch_offset),  line+46,   5);
		mb_get_double(&(store->heading_offset),line+56,   5);
		mb_get_double(&(store->em100_td),      line+70,   5);
		mb_get_double(&(store->em100_tx),      line+84,   5);
		mb_get_double(&(store->em100_ty),      line+98,   5);
		mb_get_double(&(store->em12_td),       line+111,  5);
		mb_get_double(&(store->em12_tx),       line+124,  5);
		mb_get_double(&(store->em12_ty),       line+137,  5);
		mb_get_double(&(store->em1000_td),     line+152,  5);
		mb_get_double(&(store->em1000_tx),     line+167,  5);
		mb_get_double(&(store->em1000_ty),     line+182,  5);
		for(i=0;i<128;i++)
			store->spare_parameter[i] = line[188+i];
		mb_get_int(&(store->survey_line),      line+328,  4);
		for(i=0;i<80;i++)
			store->comment[i] = line[341+i];
			
		/* infer sonar type from transducer depths */
		if (store->em12_td != 0.0)
		    store->sonar = MBSYS_SIMRAD_EM12S;
		else if (store->em1000_td != 0.0)
		    store->sonar = MBSYS_SIMRAD_EM1000;
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",store->par_year);
		fprintf(stderr,"dbg5       month:            %d\n",store->par_month);
		fprintf(stderr,"dbg5       day:              %d\n",store->par_day);
		fprintf(stderr,"dbg5       hour:             %d\n",store->par_hour);
		fprintf(stderr,"dbg5       minute:           %d\n",store->par_minute);
		fprintf(stderr,"dbg5       sec:              %d\n",store->par_second);
		fprintf(stderr,"dbg5       centisecond:      %d\n",store->par_centisecond);
		fprintf(stderr,"dbg5       pos_type:         %d\n",store->pos_type);
		fprintf(stderr,"dbg5       pos_delay:        %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       roll_offset:      %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       pitch_offset:     %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       heading_offset:   %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em100_td:         %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em100_tx:         %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em100_ty:         %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em12_td:          %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em12_tx:          %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em12_ty:          %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em1000_td:        %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em1000_tx:        %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em1000_ty:        %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       survey_line:      %d\n",store->survey_line);
		fprintf(stderr,"dbg5       comment:          %s\n",store->comment);
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
int mbr_emoldraw_rd_pos(int verbose, FILE *mbfp, 
		struct mbsys_simrad_struct *store, int *error)
{
	char	*function_name = "mbr_emoldraw_rd_pos";
	int	status = MB_SUCCESS;
	char	line[EM_POS_SIZE];
	int	degree;
	double	minute;
	char	hemisphere;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}

	/* read record into char array */
	status = fread(line,1,EM_POS_SIZE,mbfp);
	if (status == EM_POS_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get data */
	if (status == MB_SUCCESS)
		{
		store->kind = MB_DATA_NAV;
		mb_get_int(&(store->pos_day),          line,      2);
		mb_get_int(&(store->pos_month),        line+2,    2);
		mb_get_int(&(store->pos_year),         line+4,    2);
		mb_get_int(&(store->pos_hour),         line+7,    2);
		mb_get_int(&(store->pos_minute),       line+9,    2);
		mb_get_int(&(store->pos_second),       line+11,   2);
		mb_get_int(&(store->pos_centisecond),  line+13,   2);
		mb_get_int(&degree,                   line+16,   2);
		mb_get_double(&minute,                line+18,   7);
		hemisphere = line[25];
		store->pos_latitude = degree + minute/60.0;
		if (hemisphere == 'S' || hemisphere == 's')
			store->pos_latitude = -store->pos_latitude;
		mb_get_int(&degree,                   line+27,   3);
		mb_get_double(&minute,                line+30,   7);
		hemisphere = line[37];
		store->pos_longitude = degree + minute/60.0;
		if (hemisphere == 'W' || hemisphere == 'w')
			store->pos_longitude = -store->pos_longitude;
		mb_get_double(&(store->utm_northing),  line+39,  11);
		mb_get_double(&(store->utm_easting),   line+51,   9);
		mb_get_int(&(store->utm_zone),         line+61,   2);
		mb_get_int(&degree,                   line+64,   3);
		mb_get_double(&minute,                line+67,   7);
		hemisphere = line[74];
		store->utm_zone_lon = degree + minute/60.0;
		if (hemisphere == 'W' || hemisphere == 'w')
			store->utm_zone_lon = -store->utm_zone_lon;
		mb_get_int(&(store->utm_system),       line+76,   1);
		mb_get_int(&(store->pos_quality),      line+78,   1);
		mb_get_double(&(store->speed),         line+80,   4);
		mb_get_double(&(store->line_heading),  line+85,   5);
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",store->pos_year);
		fprintf(stderr,"dbg5       month:            %d\n",store->pos_month);
		fprintf(stderr,"dbg5       day:              %d\n",store->pos_day);
		fprintf(stderr,"dbg5       hour:             %d\n",store->pos_hour);
		fprintf(stderr,"dbg5       minute:           %d\n",store->pos_minute);
		fprintf(stderr,"dbg5       sec:              %d\n",store->pos_second);
		fprintf(stderr,"dbg5       centisecond:      %d\n",store->pos_centisecond);
		fprintf(stderr,"dbg5       longitude:        %f\n",store->pos_longitude);
		fprintf(stderr,"dbg5       latitude:         %f\n",store->pos_latitude);
		fprintf(stderr,"dbg5       utm_northing:     %f\n",store->utm_northing);
		fprintf(stderr,"dbg5       utm_easting:      %f\n",store->utm_easting);
		fprintf(stderr,"dbg5       utm_zone:         %d\n",store->utm_zone);
		fprintf(stderr,"dbg5       utm_zone_lon:     %f\n",store->utm_zone_lon);
		fprintf(stderr,"dbg5       utm_system:       %d\n",store->utm_system);
		fprintf(stderr,"dbg5       pos_quality:      %d\n",store->pos_quality);
		fprintf(stderr,"dbg5       speed:            %f\n",store->speed);
		fprintf(stderr,"dbg5       line_heading:     %f\n",store->line_heading);
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
int mbr_emoldraw_rd_svp(int verbose, FILE *mbfp, 
		struct mbsys_simrad_struct *store, int *error)
{
	char	*function_name = "mbr_emoldraw_rd_svp";
	int	status = MB_SUCCESS;
	char	line[EM_SVP_SIZE];
	short	short_val;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}

	/* read record into char array */
	status = fread(line,1,EM_SVP_SIZE,mbfp);
	if (status == EM_SVP_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get data */
	if (status == MB_SUCCESS)
		{
		store->kind = MB_DATA_VELOCITY_PROFILE;
		mb_get_int(&(store->svp_day),          line,      2);
		mb_get_int(&(store->svp_month),        line+2,    2);
		mb_get_int(&(store->svp_year),         line+4,    2);
		mb_get_int(&(store->svp_hour),         line+6,    2);
		mb_get_int(&(store->svp_minute),       line+8,    2);
		mb_get_int(&(store->svp_second),       line+10,   2);
		mb_get_int(&(store->svp_centisecond),  line+12,   2);
		mb_get_binary_short(MB_YES, &line[14], &short_val);
		store->svp_num = short_val;
		for (i=0;i<store->svp_num;i++)
			{
			mb_get_binary_short(MB_YES, &line[16+4*i], &short_val);
			store->svp_depth[i] = short_val;
			mb_get_binary_short(MB_YES, &line[18+4*i], &short_val);
			store->svp_vel[i] = short_val;
			}
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",store->svp_year);
		fprintf(stderr,"dbg5       month:            %d\n",store->svp_month);
		fprintf(stderr,"dbg5       day:              %d\n",store->svp_day);
		fprintf(stderr,"dbg5       hour:             %d\n",store->svp_hour);
		fprintf(stderr,"dbg5       minute:           %d\n",store->svp_minute);
		fprintf(stderr,"dbg5       sec:              %d\n",store->svp_second);
		fprintf(stderr,"dbg5       centisecond:      %d\n",store->svp_centisecond);
		fprintf(stderr,"dbg5       svp_num:          %d\n",store->svp_num);
		for (i=0;i<store->svp_num;i++)
			fprintf(stderr,"dbg5       depth: %d     vel: %d\n",
				store->svp_depth[i],store->svp_vel[i]);
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
int mbr_emoldraw_rd_em1000bath(int verbose, FILE *mbfp, 
		struct mbsys_simrad_struct *store, int *error)
{
	char	*function_name = "mbr_emoldraw_rd_em1000bath";
	int	status = MB_SUCCESS;
	struct mbsys_simrad_survey_struct *ping;
	char	line[EM_1000_BATH_SIZE];
	char	beamarray[11];
	short int short_val;
	short int *short_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}

	/* read record into char array */
	status = fread(line,1,EM_1000_BATH_SIZE,mbfp);
	if (status == EM_1000_BATH_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get data */
	if (status == MB_SUCCESS)
		{
		/* get ping structure */
		ping = (struct mbsys_simrad_survey_struct *) store->ping;
		
		/* set sonar id */
		store->sonar = MBSYS_SIMRAD_EM1000;
		
		/* get ascii stuff */
		store->kind = MB_DATA_DATA;
		mb_get_int(&(store->day),              line,      2);
		mb_get_int(&(store->month),            line+2,    2);
		mb_get_int(&(store->year),             line+4,    2);
		mb_get_int(&(store->hour),             line+6,    2);
		mb_get_int(&(store->minute),           line+8,    2);
		mb_get_int(&(store->second),           line+10,   2);
		mb_get_int(&(store->centisecond),      line+12,   2);

		/* get binary stuff */
		mb_get_binary_short(MB_YES, &line[14], &short_val); ping->ping_number = (int) short_val;
		ping->bath_mode = (int) line[16];
		ping->bath_quality = (int) line[17];
		mb_get_binary_short(MB_YES, &line[18], &short_val); ping->keel_depth = (int) short_val;
		mb_get_binary_short(MB_YES, &line[20], &short_val); ping->heading = (int) short_val;
		mb_get_binary_short(MB_YES, &line[22], &short_val); ping->roll = (int) short_val;
		mb_get_binary_short(MB_YES, &line[24], &short_val); ping->pitch = (int) short_val;
		mb_get_binary_short(MB_YES, &line[26], &short_val); ping->xducer_pitch = (int) short_val;
		mb_get_binary_short(MB_YES, &line[28], &short_val); ping->ping_heave = (int) short_val;
		mb_get_binary_short(MB_YES, &line[30], &short_val); ping->sound_vel = (int) short_val;
		for (i=0;i<MBSYS_EM1000_MAXBEAMS;i++)
			{
			for (j=0;j<11;j++)
				beamarray[j] = line[32+11*i+j];
			short_ptr = (short int *) beamarray; 
			mb_get_binary_short(MB_YES, &short_ptr[0], &ping->bath[i]);
			mb_get_binary_short(MB_YES, &short_ptr[1], &ping->bath_acrosstrack[i]);
			mb_get_binary_short(MB_YES, &short_ptr[2], &ping->bath_alongtrack[i]);
			mb_get_binary_short(MB_YES, &short_ptr[3], &ping->tt[i]);
			ping->amp[i] = (mb_s_char) beamarray[8];
			ping->quality[i] = (mb_u_char) beamarray[9];
			ping->heave[i] = (mb_s_char) beamarray[10];
			}
		ping->bath_res = 0;
		if (ping->bath_mode >= 3 && ping->bath_mode <= 7)
			ping->beams_bath = MBSYS_EM1000_MAXBEAMS;
		else
			ping->beams_bath = 48;
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",store->year);
		fprintf(stderr,"dbg5       month:            %d\n",store->month);
		fprintf(stderr,"dbg5       day:              %d\n",store->day);
		fprintf(stderr,"dbg5       hour:             %d\n",store->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",store->minute);
		fprintf(stderr,"dbg5       sec:              %d\n",store->second);
		fprintf(stderr,"dbg5       centisecond:      %d\n",store->centisecond);
		fprintf(stderr,"dbg5       ping_number:      %d\n",ping->ping_number);
		fprintf(stderr,"dbg5       beams_bath:       %d\n",ping->beams_bath);
		fprintf(stderr,"dbg5       bath_mode:        %d\n",ping->bath_mode);
		fprintf(stderr,"dbg5       bath_res:         %d\n",ping->bath_res);
		fprintf(stderr,"dbg5       bath_quality:     %d\n",ping->ping_number);
		fprintf(stderr,"dbg5       keel_depth:       %d\n",ping->keel_depth);
		fprintf(stderr,"dbg5       heading:          %d\n",ping->heading);
		fprintf(stderr,"dbg5       roll:             %d\n",ping->roll);
		fprintf(stderr,"dbg5       pitch:            %d\n",ping->pitch);
		fprintf(stderr,"dbg5       xducer_pitch:     %d\n",ping->xducer_pitch);
		fprintf(stderr,"dbg5       ping_heave:       %d\n",ping->ping_heave);
		fprintf(stderr,"dbg5       sound_vel:        %d\n",ping->sound_vel);
		fprintf(stderr,"dbg5       beam bath xtrack ltrack tt amp qual heave\n");
		for (i=0;i<ping->beams_bath;i++)
			fprintf(stderr,"dbg5       beam:%d  bath:%d  xtrck:%d  ltrck:%d tt:%d  amp:%d  qual:%d  heave:%d\n",
				i,ping->bath[i],ping->bath_acrosstrack[i],
				ping->bath_alongtrack[i],ping->tt[i],
				ping->amp[i],ping->quality[i],ping->heave[i]);
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
int mbr_emoldraw_rd_em12bath(int verbose, FILE *mbfp, 
		struct mbsys_simrad_struct *store, int swath_id, int *error)
{
	char	*function_name = "mbr_emoldraw_rd_em12bath";
	int	status = MB_SUCCESS;
	struct mbsys_simrad_survey_struct *ping;
	char	line[EM_12S_BATH_SIZE];
	char	beamarray[11];
	short int short_val;
	short int *short_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       swath_id:   %d\n",swath_id);
		}
	
	/* read record into char array */
	status = fread(line,1,EM_12S_BATH_SIZE,mbfp);
	if (status == EM_12S_BATH_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get data */
	if (status == MB_SUCCESS)
		{
		/* get ping structure */
		ping = (struct mbsys_simrad_survey_struct *) store->ping;
		
		/* set sonar id */
		if (swath_id == EM_SWATH_CENTER)
		    store->sonar = MBSYS_SIMRAD_EM12S;
		else
		    store->sonar = MBSYS_SIMRAD_EM12D;
		
		/* get ascii stuff */
		store->kind = MB_DATA_DATA;
		mb_get_int(&(store->day),              line,      2);
		mb_get_int(&(store->month),            line+2,    2);
		mb_get_int(&(store->year),             line+4,    2);
		mb_get_int(&(store->hour),             line+6,    2);
		mb_get_int(&(store->minute),           line+8,    2);
		mb_get_int(&(store->second),           line+10,   2);
		mb_get_int(&(store->centisecond),      line+12,   2);

		/* set swath id */
		ping->swath_id = swath_id;

		/* get binary stuff */
		mb_get_binary_short(MB_YES, &line[14], &short_val); ping->ping_number = (int) short_val;
		ping->beams_bath = MBSYS_EM12_MAXBEAMS;
		ping->bath_res = (int) line[16];
		ping->bath_quality = (int) line[17];
		mb_get_binary_short(MB_YES, &line[18], &short_val); ping->keel_depth = (int) short_val;
		mb_get_binary_short(MB_YES, &line[20], &short_val); ping->heading = (int) short_val;
		mb_get_binary_short(MB_YES, &line[22], &short_val); ping->roll = (int) short_val;
		mb_get_binary_short(MB_YES, &line[24], &short_val); ping->pitch = (int) short_val;
		mb_get_binary_short(MB_YES, &line[26], &short_val); ping->ping_heave = (int) short_val;
		mb_get_binary_short(MB_YES, &line[28], &short_val); ping->sound_vel = (int) short_val;
		ping->bath_mode = (int) line[30];
		for (i=0;i<ping->beams_bath;i++)
			{
			for (j=0;j<11;j++)
				beamarray[j] = line[32+11*i+j];
			short_ptr = (short int *) beamarray; 
			mb_get_binary_short(MB_YES, &short_ptr[0], &ping->bath[i]);
			mb_get_binary_short(MB_YES, &short_ptr[1], &ping->bath_acrosstrack[i]);
			mb_get_binary_short(MB_YES, &short_ptr[2], &ping->bath_alongtrack[i]);
			mb_get_binary_short(MB_YES, &short_ptr[3], &ping->tt[i]);
			ping->amp[i] = (mb_s_char) beamarray[8];
			ping->quality[i] = (mb_u_char) beamarray[9];
			ping->heave[i] = (mb_s_char) beamarray[10];
			}
		}


	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",store->year);
		fprintf(stderr,"dbg5       month:            %d\n",store->month);
		fprintf(stderr,"dbg5       day:              %d\n",store->day);
		fprintf(stderr,"dbg5       hour:             %d\n",store->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",store->minute);
		fprintf(stderr,"dbg5       sec:              %d\n",store->second);
		fprintf(stderr,"dbg5       centisecond:      %d\n",store->centisecond);
		fprintf(stderr,"dbg5       ping_number:      %d\n",ping->ping_number);
		fprintf(stderr,"dbg5       beams_bath:       %d\n",ping->beams_bath);
		fprintf(stderr,"dbg5       bath_mode:        %d\n",ping->bath_mode);
		fprintf(stderr,"dbg5       bath_res:         %d\n",ping->bath_res);
		fprintf(stderr,"dbg5       bath_quality:     %d\n",ping->bath_quality);
		fprintf(stderr,"dbg5       keel_depth:       %d\n",ping->keel_depth);
		fprintf(stderr,"dbg5       heading:          %d\n",ping->heading);
		fprintf(stderr,"dbg5       roll:             %d\n",ping->roll);
		fprintf(stderr,"dbg5       pitch:            %d\n",ping->pitch);
		fprintf(stderr,"dbg5       ping_heave:       %d\n",ping->ping_heave);
		fprintf(stderr,"dbg5       sound_vel:        %d\n",ping->sound_vel);
		fprintf(stderr,"dbg5       beam bath xtrack ltrack tt amp qual heave\n");
		for (i=0;i<ping->beams_bath;i++)
			fprintf(stderr,"dbg5       beam:%d  bath:%d  xtrck:%d  ltrck:%d tt:%d  amp:%d  qual:%d  heave:%d\n",
				i,ping->bath[i],ping->bath_acrosstrack[i],
				ping->bath_alongtrack[i],ping->tt[i],
				ping->amp[i],ping->quality[i],ping->heave[i]);
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
int mbr_emoldraw_rd_em121bath(int verbose, FILE *mbfp, 
		struct mbsys_simrad_struct *store, int *error)
{
	char	*function_name = "mbr_emoldraw_rd_em121bath";
	int	status = MB_SUCCESS;
	struct mbsys_simrad_survey_struct *ping;
	char	line[EM_121_BATH_SIZE];
	char	beamarray[11];
	short int short_val;
	short int *short_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}

	/* get ping structure */
	ping = (struct mbsys_simrad_survey_struct *) store->ping;

	/* read record into char array */
	status = fread(line,1,EM_121_BATH_SIZE,mbfp);
	if (status == EM_121_BATH_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get data */
	if (status == MB_SUCCESS)
		{
		/* get ascii stuff */
		store->kind = MB_DATA_DATA;
		mb_get_int(&(store->day),              line,      2);
		mb_get_int(&(store->month),            line+2,    2);
		mb_get_int(&(store->year),             line+4,    2);
		mb_get_int(&(store->hour),             line+6,    2);
		mb_get_int(&(store->minute),           line+8,    2);
		mb_get_int(&(store->second),           line+10,   2);
		mb_get_int(&(store->centisecond),      line+12,   2);

		/* get binary stuff */
		mb_get_binary_short(MB_YES, &line[14], &short_val); ping->ping_number = (int) short_val;
		ping->bath_mode = (int) line[16];
		ping->bath_res = 0;
		ping->bath_quality = (int) line[17];
		ping->bath_num = (int) line[18];
		ping->beams_bath = ping->bath_num;
		ping->pulse_length = (int) line[19];
		ping->beam_width = (int) line[20];
		ping->power_level = (int) line[21];
		ping->tx_status = (int) line[22];
		ping->rx_status = (int) line[23];
		mb_get_binary_short(MB_YES, &line[24], &short_val); ping->keel_depth = (int) short_val;
		mb_get_binary_short(MB_YES, &line[26], &short_val); ping->heading = (int) short_val;
		mb_get_binary_short(MB_YES, &line[28], &short_val); ping->roll = (int) short_val;
		mb_get_binary_short(MB_YES, &line[30], &short_val); ping->pitch = (int) short_val;
		mb_get_binary_short(MB_YES, &line[32], &short_val); ping->ping_heave = (int) short_val;
		mb_get_binary_short(MB_YES, &line[34], &short_val); ping->sound_vel = (int) short_val;
		ping->along_res = (int) line[36];
		ping->across_res = (int) line[37];
		ping->depth_res = (int) line[38];
		ping->range_res = (int) line[39];
		for (i=0;i<ping->beams_bath;i++)
			{
			for (j=0;j<11;j++)
				beamarray[j] = line[44+11*i+j];
			short_ptr = (short int *) beamarray; 
			mb_get_binary_short(MB_YES, &short_ptr[0], &ping->bath[i]);
			mb_get_binary_short(MB_YES, &short_ptr[1], &ping->bath_acrosstrack[i]);
			mb_get_binary_short(MB_YES, &short_ptr[2], &ping->bath_alongtrack[i]);
			mb_get_binary_short(MB_YES, &short_ptr[3], &ping->tt[i]);
			ping->amp[i] = (mb_s_char) beamarray[8];
			ping->quality[i] = (mb_u_char) beamarray[9];
			ping->heave[i] = (mb_s_char) beamarray[10];
			}
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",store->year);
		fprintf(stderr,"dbg5       month:            %d\n",store->month);
		fprintf(stderr,"dbg5       day:              %d\n",store->day);
		fprintf(stderr,"dbg5       hour:             %d\n",store->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",store->minute);
		fprintf(stderr,"dbg5       sec:              %d\n",store->second);
		fprintf(stderr,"dbg5       centisecond:      %d\n",store->centisecond);
		fprintf(stderr,"dbg5       ping_number:      %d\n",ping->ping_number);
		fprintf(stderr,"dbg5       beams_bath:       %d\n",ping->beams_bath);
		fprintf(stderr,"dbg5       bath_mode:        %d\n",ping->bath_mode);
		fprintf(stderr,"dbg5       bath_quality:     %d\n",ping->bath_quality);
		fprintf(stderr,"dbg5       bath_num:         %d\n",ping->bath_num);
		fprintf(stderr,"dbg5       pulse_length:     %d\n",ping->pulse_length);
		fprintf(stderr,"dbg5       beam_width:       %d\n",ping->beam_width);
		fprintf(stderr,"dbg5       power_level:      %d\n",ping->power_level);
		fprintf(stderr,"dbg5       tx_status:        %d\n",ping->tx_status);
		fprintf(stderr,"dbg5       rx_status:        %d\n",ping->rx_status);
		fprintf(stderr,"dbg5       along_res:        %d\n",ping->along_res);
		fprintf(stderr,"dbg5       across_res:       %d\n",ping->across_res);
		fprintf(stderr,"dbg5       depth_res:        %d\n",ping->depth_res);
		fprintf(stderr,"dbg5       range_res:        %d\n",ping->range_res);
		fprintf(stderr,"dbg5       keel_depth:       %d\n",ping->keel_depth);
		fprintf(stderr,"dbg5       heading:          %d\n",ping->heading);
		fprintf(stderr,"dbg5       roll:             %d\n",ping->roll);
		fprintf(stderr,"dbg5       pitch:            %d\n",ping->pitch);
		fprintf(stderr,"dbg5       ping_heave:       %d\n",ping->ping_heave);
		fprintf(stderr,"dbg5       sound_vel:        %d\n",ping->sound_vel);
		fprintf(stderr,"dbg5       beam bath xtrack ltrack tt amp qual heave\n");
		for (i=0;i<ping->beams_bath;i++)
			fprintf(stderr,"dbg5       beam:%d  bath:%d  xtrck:%d  ltrck:%d tt:%d  amp:%d  qual:%d  heave:%d\n",
				i,ping->bath[i],ping->bath_acrosstrack[i],
				ping->bath_alongtrack[i],ping->tt[i],
				ping->amp[i],ping->quality[i],ping->heave[i]);
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
int mbr_emoldraw_rd_ss(int verbose, FILE *mbfp, 
		struct mbsys_simrad_struct *store, 
		int swath_id, int first, int *more, int *error)
{
	char	*function_name = "mbr_emoldraw_rd_ss";
	int	status = MB_SUCCESS;
	struct mbsys_simrad_survey_struct *ping;
	char	line[EM_SS_SIZE];
	int	num_datagrams;
	int	datagram;
	int	num_beams;
	int	beamlist[MBSYS_SIMRAD_MAXBEAMS];
	mb_s_char *beam_ss;
	int	ioffset;
	int	npixelsum;
	short int short_val;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       swath_id:   %d\n",swath_id);
		fprintf(stderr,"dbg2       first:      %d\n",first);
		}

	/* get ping structure */
	ping = (struct mbsys_simrad_survey_struct *) store->ping;

	/* if first call for current ping, initialize */
	if (first == MB_YES)
		{
		ping->pixels_ssraw = 0;
		for (i=0;i<ping->beams_bath;i++)
			{
			ping->beam_samples[i] = 0;
			ping->beam_center_sample[i] = 0;
			ping->beam_start_sample[i] = 0;
			}
		}

	/* read first record into char array */
	status = fread(line,1,EM_SS_SIZE,mbfp);
	if (status == EM_SS_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get data */
	if (status == MB_SUCCESS)
		{		
		/* get ascii stuff */
		store->kind = MB_DATA_DATA;
		mb_get_int(&(store->day),              line,      2);
		mb_get_int(&(store->month),            line+2,    2);
		mb_get_int(&(store->year),             line+4,    2);
		mb_get_int(&(store->hour),             line+6,    2);
		mb_get_int(&(store->minute),           line+8,    2);
		mb_get_int(&(store->second),           line+10,   2);
		mb_get_int(&(store->centisecond),      line+12,   2);

		/* set swath id */
		ping->swath_id = swath_id;

		/* get binary stuff */
		mb_get_binary_short(MB_YES, &line[14], &short_val); ping->ping_number = (int) short_val;
/*		mb_get_binary_short(MB_YES, &line[16], &short_val); ping->sound_vel = (int) short_val;*/
		ping->ss_mode = (int) line[18];
		num_datagrams = (int) line[19];
		datagram = (int) line[20];
		num_beams = (int) line[21];
		
		/* check for good values */
		if (num_datagrams < 1 || num_datagrams > 255
		    || datagram < 1 || datagram > 255
		    || num_beams < 1 || num_beams > MBSYS_SIMRAD_MAXBEAMS)
			{
			num_beams = 0;			
			}

		/* get number of pixels */
		npixelsum = 0;
		for (i=0;i<num_beams;i++)
			{
			beamlist[i] = ((int) line[22+6*i]) - 1;
			ping->beam_frequency[beamlist[i]] = (short int) line[23+6*i];
			mb_get_binary_short(MB_YES, &line[24+6*i], &ping->beam_samples[beamlist[i]]);
			mb_get_binary_short(MB_YES, &line[26+6*i], &ping->beam_center_sample[beamlist[i]]); 
			npixelsum += ping->beam_samples[beamlist[i]];
			}
			
		/* check for bad numbers of pixels indicating a broken
		    record */
		if (npixelsum > 523)
		    for (i=0;i<num_beams;i++)
			{
			ping->beam_samples[beamlist[i]] = 0;
			}
		    
		/* load up the sidescan for each beam */
		ioffset = 22+6*num_beams;
		for (i=0;i<num_beams;i++)
			{
			/* do not ever load more data than can be
			    in the data record */
			if (ping->pixels_ssraw + ping->beam_samples[beamlist[i]]
				> MBSYS_SIMRAD_MAXRAWPIXELS)
				ping->beam_samples[beamlist[i]] = 0;
			
			/* get the sidescan */
			ping->beam_start_sample[beamlist[i]] = ping->pixels_ssraw;
			for (j=0;j<ping->beam_samples[beamlist[i]];j++)
				{
				ping->ssraw[ping->pixels_ssraw] = 
					(mb_s_char) line[ioffset];
				ioffset++;
				ping->pixels_ssraw++;
				}
			}
		}
		
	/* set status */
	if (status == MB_SUCCESS)
		ping->ss_status = EM_SS_AMPONLY;
	else
		ping->ss_status = EM_SS_NONE;

	/* set flag if another sidescan record needs to be read */
	if (status == MB_SUCCESS && datagram < num_datagrams)
		*more = MB_YES;
	else
		*more = MB_NO;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",store->year);
		fprintf(stderr,"dbg5       month:            %d\n",store->month);
		fprintf(stderr,"dbg5       day:              %d\n",store->day);
		fprintf(stderr,"dbg5       hour:             %d\n",store->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",store->minute);
		fprintf(stderr,"dbg5       sec:              %d\n",store->second);
		fprintf(stderr,"dbg5       centisecond:      %d\n",store->centisecond);
		fprintf(stderr,"dbg5       ping_number:      %d\n",ping->ping_number);
		fprintf(stderr,"dbg5       sound_vel:        %d\n",ping->sound_vel);
		fprintf(stderr,"dbg5       ss_mode:          %d\n",ping->ss_mode);
		fprintf(stderr,"dbg5       num_datagrams:    %d\n",num_datagrams);
		fprintf(stderr,"dbg5       datagram:         %d\n",datagram);
		fprintf(stderr,"dbg5       num_beams:        %d\n",num_beams);
		fprintf(stderr,"dbg5       beam frequency samples center\n");
		for (i=0;i<num_beams;i++)
			fprintf(stderr,"dbg5       beam:%d  frequency:%d  samples:%d  center:%d  start:%d\n",
				beamlist[i],ping->beam_frequency[beamlist[i]],
				ping->beam_samples[beamlist[i]],
				ping->beam_center_sample[beamlist[i]],
				ping->beam_start_sample[beamlist[i]]);
		for (i=0;i<num_beams;i++)
			{
			beam_ss = &ping->ssraw[ping->beam_start_sample[beamlist[i]]];
			for (j=0;j<ping->beam_samples[beamlist[i]];j++)
				fprintf(stderr,"dbg5       beam:%d pixel:%d  amp:%d\n",
					beamlist[i],j,beam_ss[j]);
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       more:       %d\n",*more);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_emoldraw_rd_ssp(int verbose, FILE *mbfp, 
		struct mbsys_simrad_struct *store, 
		int swath_id, int first, int *more, int *error)
{
	char	*function_name = "mbr_emoldraw_rd_ssp";
	int	status = MB_SUCCESS;
	struct mbsys_simrad_survey_struct *ping;
	char	line[EM_SSP_SIZE];
	char	*char_ptr;
	int	num_datagrams;
	int	datagram;
	int	num_beams;
	int	beamlist[MBSYS_SIMRAD_MAXBEAMS];
	mb_s_char *beam_ss;
	short int *beam_ssp;
	int	ioffset;
	int	npixelsum;
	short int short_val;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		fprintf(stderr,"dbg2       swath_id:   %d\n",swath_id);
		fprintf(stderr,"dbg2       first:      %d\n",first);
		}

	/* get ping structure */
	ping = (struct mbsys_simrad_survey_struct *) store->ping;

	/* if first call for current ping, initialize */
	if (first == MB_YES)
		{
		ping->pixels_ssraw = 0;
		for (i=0;i<ping->beams_bath;i++)
			{
			ping->beam_samples[i] = 0;
			ping->beam_center_sample[i] = 0;
			ping->beam_start_sample[i] = 0;
			}
		}

	/* read first record into char array */
	status = fread(line,1,EM_SSP_SIZE,mbfp);
	if (status == EM_SSP_SIZE)
		status = MB_SUCCESS;
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* get data */
	if (status == MB_SUCCESS)
		{
		/* get ascii stuff */
		store->kind = MB_DATA_DATA;
		mb_get_int(&(store->day),              line,      2);
		mb_get_int(&(store->month),            line+2,    2);
		mb_get_int(&(store->year),             line+4,    2);
		mb_get_int(&(store->hour),             line+6,    2);
		mb_get_int(&(store->minute),           line+8,    2);
		mb_get_int(&(store->second),           line+10,   2);
		mb_get_int(&(store->centisecond),      line+12,   2);

		/* set swath id */
		ping->swath_id = swath_id;

		/* get binary stuff */
		mb_get_binary_short(MB_YES, &line[14], &short_val); ping->ping_number = (int) short_val;
/*		mb_get_binary_short(MB_YES, &line[16], &short_val); ping->sound_vel = (int) short_val;*/
		ping->ss_mode = (int) line[18];
		num_datagrams = (int) line[19];
		datagram = (int) line[20];
		num_beams = (int) line[21];
		
		/* check for good values */
		if (num_datagrams < 1 || num_datagrams > 255
		    || datagram < 1 || datagram > 255
		    || num_beams < 1 || num_beams > MBSYS_SIMRAD_MAXBEAMS)
			{
			num_beams = 0;			
			}
		
		/* get number of pixels */
		npixelsum = 0;
		for (i=0;i<num_beams;i++)
			{
			beamlist[i] = ((int) line[22+6*i]) - 1;
			ping->beam_frequency[beamlist[i]] = (short int) line[23+6*i];
			mb_get_binary_short(MB_YES, &line[24+6*i], &ping->beam_samples[beamlist[i]]);
			mb_get_binary_short(MB_YES, &line[26+6*i], &ping->beam_center_sample[beamlist[i]]); 
			npixelsum += ping->beam_samples[beamlist[i]];
			}
			
		/* check for bad numbers of pixels indicating a broken
		    record */
		if (npixelsum > 523)
		    for (i=0;i<num_beams;i++)
			{
			ping->beam_samples[beamlist[i]] = 0;
			}
		    
		/* load up the sidescan for each beam */
		ioffset = 22+6*num_beams;
		char_ptr = (char *) &short_val;
		for (i=0;i<num_beams;i++)
			{
			/* do not ever load more data than we can store */
			if (ping->pixels_ssraw + ping->beam_samples[beamlist[i]]
				> MBSYS_SIMRAD_MAXRAWPIXELS)
				ping->beam_samples[beamlist[i]] = 0;
			
			/* get the sidescan */
			ping->beam_start_sample[beamlist[i]] = ping->pixels_ssraw;
			for (j=0;j<ping->beam_samples[beamlist[i]];j++)
				{
				ping->ssraw[ping->pixels_ssraw] = (mb_s_char) line[ioffset];
				char_ptr[0] = line[ioffset+1];
				char_ptr[1] = line[ioffset+2];
				mb_get_binary_short(MB_YES, (short *)char_ptr, &ping->ssp[ping->pixels_ssraw]);
				ioffset += 3;
				ping->pixels_ssraw++;
				}
			}
		}
		
	/* set status */
	if (status == MB_SUCCESS)
		ping->ss_status = EM_SS_AMPPHASE;
	else
		ping->ss_status = EM_SS_NONE;

	/* set flag if another sidescan record needs to be read */
	if (status == MB_SUCCESS && datagram < num_datagrams)
		*more = MB_YES;
	else
		*more = MB_NO;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",store->year);
		fprintf(stderr,"dbg5       month:            %d\n",store->month);
		fprintf(stderr,"dbg5       day:              %d\n",store->day);
		fprintf(stderr,"dbg5       hour:             %d\n",store->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",store->minute);
		fprintf(stderr,"dbg5       sec:              %d\n",store->second);
		fprintf(stderr,"dbg5       centisecond:      %d\n",store->centisecond);
		fprintf(stderr,"dbg5       ping_number:      %d\n",ping->ping_number);
		fprintf(stderr,"dbg5       sound_vel:        %d\n",ping->sound_vel);
		fprintf(stderr,"dbg5       ss_mode:          %d\n",ping->ss_mode);
		fprintf(stderr,"dbg5       num_datagrams:    %d\n",num_datagrams);
		fprintf(stderr,"dbg5       datagram:         %d\n",datagram);
		fprintf(stderr,"dbg5       num_beams:        %d\n",num_beams);
		fprintf(stderr,"dbg5       beam frequency samples center\n");
		for (i=0;i<num_beams;i++)
			fprintf(stderr,"dbg5       beam:%d  frequency:%d  samples:%d  center:%d  start:%d\n",
				beamlist[i],ping->beam_frequency[beamlist[i]],
				ping->beam_samples[beamlist[i]],
				ping->beam_center_sample[beamlist[i]],
				ping->beam_start_sample[beamlist[i]]);
		for (i=0;i<num_beams;i++)
			{
			beam_ss = &ping->ssraw[ping->beam_start_sample[beamlist[i]]];
			beam_ssp = &ping->ssp[ping->beam_start_sample[beamlist[i]]];
			for (j=0;j<ping->beam_samples[beamlist[i]];j++)
				fprintf(stderr,"dbg5       beam:%d pixel:%d  amp:%d phase:%d\n",
					beamlist[i],j,beam_ss[j],beam_ssp[j]);
			}
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       more:       %d\n",*more);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbr_emoldraw_wr_data(int verbose, void *mbio_ptr, 
				void *store_ptr, int *error)
{
	char	*function_name = "mbr_emoldraw_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_simrad_struct *store;
	struct mbsys_simrad_survey_struct *ping;
	FILE	*mbfp;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	store = (struct mbsys_simrad_struct *) store_ptr;
	ping = (struct mbsys_simrad_survey_struct *) store->ping;
	mbfp = mb_io_ptr->mbfp;

	if (store->kind == MB_DATA_COMMENT)
		{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call mbr_emoldraw_wr_parameter\n");
#endif
		status = mbr_emoldraw_wr_parameter(verbose,mbfp,store,error);
		}
	else if (store->kind == MB_DATA_START)
		{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call mbr_emoldraw_wr_start\n");
#endif
		status = mbr_emoldraw_wr_start(verbose,mbfp,store,error);
		}
	else if (store->kind == MB_DATA_STOP)
		{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call mbr_emoldraw_wr_stop\n");
#endif
		status = mbr_emoldraw_wr_stop(verbose,mbfp,store,error);
		}
	else if (store->kind == MB_DATA_NAV)
		{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call mbr_emoldraw_wr_pos\n");
#endif
		status = mbr_emoldraw_wr_pos(verbose,mbfp,store,error);
		}
	else if (store->kind == MB_DATA_VELOCITY_PROFILE)
		{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call mbr_emoldraw_wr_svp\n");
#endif
		status = mbr_emoldraw_wr_svp(verbose,mbfp,store,error);
		}
	else if (store->kind == MB_DATA_DATA
		&& (store->sonar == MBSYS_SIMRAD_EM12S
		    || store->sonar == MBSYS_SIMRAD_EM12D))
		{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call mbr_emoldraw_wr_em12bath\n");
#endif
		status = mbr_emoldraw_wr_em12bath(verbose,mbfp,store,error);
		if (ping->ss_status == EM_SS_AMPONLY)
		    {
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call mbr_emoldraw_wr_ss\n");
#endif
		    status = mbr_emoldraw_wr_ss(verbose,mbfp,store,error);
		    }
		else if (ping->ss_status == EM_SS_AMPPHASE)
		    {
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call mbr_emoldraw_wr_ssp\n");
#endif
		    status = mbr_emoldraw_wr_ssp(verbose,mbfp,store,error);
		    }
		}
	else if (store->kind == MB_DATA_DATA
		&& (store->sonar == MBSYS_SIMRAD_EM1000))
		{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call mbr_emoldraw_wr_em1000bath\n");
#endif
		status = mbr_emoldraw_wr_em1000bath(verbose,mbfp,store,error);
		if (ping->ss_status == EM_SS_AMPONLY)
		    {
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call mbr_emoldraw_wr_ss\n");
#endif
		    status = mbr_emoldraw_wr_ss(verbose,mbfp,store,error);
		    }
		else if (ping->ss_status == EM_SS_AMPPHASE)
		    {
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call mbr_emoldraw_wr_ssp\n");
#endif
		    status = mbr_emoldraw_wr_ssp(verbose,mbfp,store,error);
		    }
		}
	else if (store->kind == MB_DATA_DATA
		&& (store->sonar == MBSYS_SIMRAD_EM121))
		{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call mbr_emoldraw_wr_em121bath\n");
#endif
		status = mbr_emoldraw_wr_em121bath(verbose,mbfp,store,error);
		if (ping->ss_status == EM_SS_AMPONLY)
		    {
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call mbr_emoldraw_wr_ss\n");
#endif
		    status = mbr_emoldraw_wr_ss(verbose,mbfp,store,error);
		    }
		else if (ping->ss_status == EM_SS_AMPPHASE)
		    {
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call mbr_emoldraw_wr_ssp\n");
#endif
		    status = mbr_emoldraw_wr_ssp(verbose,mbfp,store,error);
		    }
		}
	else
		{
#ifdef MBR_EMOLDRAW_DEBUG
	fprintf(stderr,"call nothing - kind:%d sonar:%d\n", store->kind, store->sonar);
#endif
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_KIND;
		}

	/* print output debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Data record kind in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       kind:       %d\n",store->kind);
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
int mbr_emoldraw_wr_start(int verbose, FILE *mbfp, 
				struct mbsys_simrad_struct *store, int *error)
{
	char	*function_name = "mbr_emoldraw_wr_start";
	int	status = MB_SUCCESS;
	char	line[EM_START_SIZE];
	short int label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short int short_val;
	mb_u_char   *uchar_ptr;
	char   *char_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",store->par_year);
		fprintf(stderr,"dbg5       month:            %d\n",store->par_month);
		fprintf(stderr,"dbg5       day:              %d\n",store->par_day);
		fprintf(stderr,"dbg5       hour:             %d\n",store->par_hour);
		fprintf(stderr,"dbg5       minute:           %d\n",store->par_minute);
		fprintf(stderr,"dbg5       sec:              %d\n",store->par_second);
		fprintf(stderr,"dbg5       centisecond:      %d\n",store->par_centisecond);
		fprintf(stderr,"dbg5       pos_type:         %d\n",store->pos_type);
		fprintf(stderr,"dbg5       pos_delay:        %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       roll_offset:      %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       pitch_offset:     %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       heading_offset:   %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em100_td:         %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em100_tx:         %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em100_ty:         %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em12_td:          %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em12_tx:          %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em12_ty:          %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em1000_td:        %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em1000_tx:        %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em1000_ty:        %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       survey_line:      %d\n",store->survey_line);
		fprintf(stderr,"dbg5       comment:          %s\n",store->comment);
		}

	/* write the record size */
	mb_put_binary_int(MB_NO, (int) (EM_START_SIZE + 2), (void *) &write_size);
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	mb_put_binary_short(MB_NO, (short) EM_START, (void *) &label);
	write_len = fwrite(&label,1,2,mbfp);
	if (write_len != 2)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* construct record */
		sprintf(line,"%2.2d%2.2d%2.2d,",
			store->par_day,store->par_month,store->par_year);
		sprintf(line+7,"%2.2d%2.2d%2.2d%2.2d,",
			store->par_hour,store->par_minute,
			store->par_second,store->par_centisecond);
		sprintf(line+16,  "PIS=%1d,",   store->pos_type);
		sprintf(line+22,  "PTD=%5.1f,",   store->pos_delay);
		sprintf(line+32,  "MSR=%5.2f,",   store->roll_offset);
		sprintf(line+42,  "MSP=%5.2f,",   store->pitch_offset);
		sprintf(line+52,  "MSG=%5.2f,",   store->heading_offset);
		sprintf(line+62,  "EM100TD=%5.1f,",   store->em100_td);
		sprintf(line+76,  "EM100TX=%5.1f,",   store->em100_tx);
		sprintf(line+90,  "EM100TY=%5.1f,",   store->em100_ty);
		sprintf(line+104, "EM12TD=%5.1f,",   store->em12_td);
		sprintf(line+117, "EM12TX=%5.1f,",   store->em12_tx);
		sprintf(line+130, "EM12TY=%5.1f,",   store->em12_ty);
		sprintf(line+143, "EM1000TD=%5.1f,",   store->em1000_td);
		sprintf(line+158, "EM1000TX=%5.1f,",   store->em1000_tx);
		sprintf(line+173, "EM1000TY=%5.1f,",   store->em1000_ty);
		for (i=0;i<128;i++)
			line[188+i] = store->spare_parameter[i];
		sprintf(line+316, "SURVEY_LINE_%4.4d,", store->survey_line);
		sprintf(line+333, "COMMENT:");
		for (i=0;i<80;i++)
			line[341+i] = store->comment[i];
		line[EM_START_SIZE-3] = 0x03;
		
		/* compute and set checksum */
		uchar_ptr = (mb_u_char *) line;
		checksum = 0;
		for (j=0;j<EM_START_SIZE-3;j++)
		    checksum += uchar_ptr[j];
		mb_put_binary_short(MB_YES, (short) checksum, (void *) &short_val);
		char_ptr = (char *) &short_val;
		line[EM_START_SIZE-2] = char_ptr[0];
		line[EM_START_SIZE-1] = char_ptr[1];

		/* write out data */
		write_len = fwrite(line,1,EM_START_SIZE,mbfp);
		
		if (write_len != EM_START_SIZE)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
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
int mbr_emoldraw_wr_stop(int verbose, FILE *mbfp,  
				struct mbsys_simrad_struct *store, int *error)
{
	char	*function_name = "mbr_emoldraw_wr_stop";
	int	status = MB_SUCCESS;
	char	line[EM_STOP_SIZE];
	short int label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short int short_val;
	mb_u_char   *uchar_ptr;
	char   *char_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",store->par_year);
		fprintf(stderr,"dbg5       month:            %d\n",store->par_month);
		fprintf(stderr,"dbg5       day:              %d\n",store->par_day);
		fprintf(stderr,"dbg5       hour:             %d\n",store->par_hour);
		fprintf(stderr,"dbg5       minute:           %d\n",store->par_minute);
		fprintf(stderr,"dbg5       sec:              %d\n",store->par_second);
		fprintf(stderr,"dbg5       centisecond:      %d\n",store->par_centisecond);
		fprintf(stderr,"dbg5       pos_type:         %d\n",store->pos_type);
		fprintf(stderr,"dbg5       pos_delay:        %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       roll_offset:      %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       pitch_offset:     %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       heading_offset:   %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em100_td:         %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em100_tx:         %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em100_ty:         %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em12_td:          %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em12_tx:          %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em12_ty:          %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em1000_td:        %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em1000_tx:        %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em1000_ty:        %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       survey_line:      %d\n",store->survey_line);
		fprintf(stderr,"dbg5       comment:          %s\n",store->comment);
		}

	/* write the record size */
	mb_put_binary_int(MB_NO, (int) (EM_STOP_SIZE + 2), (void *) &write_size);
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	mb_put_binary_short(MB_NO, (short) EM_STOP, (void *) &label);
	write_len = fwrite(&label,1,2,mbfp);
	if (write_len != 2)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* construct record */
		sprintf(line,"%2.2d%2.2d%2.2d,",
			store->par_day,store->par_month,store->par_year);
		sprintf(line+7,"%2.2d%2.2d%2.2d%2.2d,",
			store->par_hour,store->par_minute,
			store->par_second,store->par_centisecond);
		sprintf(line+16,  "PIS=%1d,",   store->pos_type);
		sprintf(line+22,  "PTD=%5.1f,",   store->pos_delay);
		sprintf(line+32,  "MSR=%5.2f,",   store->roll_offset);
		sprintf(line+42,  "MSP=%5.2f,",   store->pitch_offset);
		sprintf(line+52,  "MSG=%5.2f,",   store->heading_offset);
		sprintf(line+62,  "EM100TD=%5.1f,",   store->em100_td);
		sprintf(line+76,  "EM100TX=%5.1f,",   store->em100_tx);
		sprintf(line+90,  "EM100TY=%5.1f,",   store->em100_ty);
		sprintf(line+104, "EM12TD=%5.1f,",   store->em12_td);
		sprintf(line+117, "EM12TX=%5.1f,",   store->em12_tx);
		sprintf(line+130, "EM12TY=%5.1f,",   store->em12_ty);
		sprintf(line+143, "EM1000TD=%5.1f,",   store->em1000_td);
		sprintf(line+158, "EM1000TX=%5.1f,",   store->em1000_tx);
		sprintf(line+173, "EM1000TY=%5.1f,",   store->em1000_ty);
		for (i=0;i<128;i++)
			line[188+i] = store->spare_parameter[i];
		sprintf(line+316, "SURVEY_LINE_%4.4d,", store->survey_line);
		sprintf(line+333, "COMMENT:");
		for (i=0;i<80;i++)
			line[341+i] = store->comment[i];
		line[EM_STOP_SIZE-3] = 0x03;
		
		/* compute and set checksum */
		uchar_ptr = (mb_u_char *) line;
		checksum = 0;
		for (j=0;j<EM_STOP_SIZE-3;j++)
		    checksum += uchar_ptr[j];
		mb_put_binary_short(MB_YES, (short) checksum, (void *) &short_val);
		char_ptr = (char *) &short_val;
		line[EM_STOP_SIZE-2] = char_ptr[0];
		line[EM_STOP_SIZE-1] = char_ptr[1];

		/* write out data */
		write_len = fwrite(line,1,EM_STOP_SIZE,mbfp);
		if (write_len != EM_STOP_SIZE)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
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
int mbr_emoldraw_wr_parameter(int verbose, FILE *mbfp,  
				struct mbsys_simrad_struct *store, int *error)
{
	char	*function_name = "mbr_emoldraw_wr_parameter";
	int	status = MB_SUCCESS;
	char	line[EM_PARAMETER_SIZE];
	short int label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short int short_val;
	mb_u_char   *uchar_ptr;
	char   *char_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",store->par_year);
		fprintf(stderr,"dbg5       month:            %d\n",store->par_month);
		fprintf(stderr,"dbg5       day:              %d\n",store->par_day);
		fprintf(stderr,"dbg5       hour:             %d\n",store->par_hour);
		fprintf(stderr,"dbg5       minute:           %d\n",store->par_minute);
		fprintf(stderr,"dbg5       sec:              %d\n",store->par_second);
		fprintf(stderr,"dbg5       centisecond:      %d\n",store->par_centisecond);
		fprintf(stderr,"dbg5       pos_type:         %d\n",store->pos_type);
		fprintf(stderr,"dbg5       pos_delay:        %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       roll_offset:      %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       pitch_offset:     %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       heading_offset:   %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em100_td:         %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em100_tx:         %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em100_ty:         %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em12_td:          %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em12_tx:          %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em12_ty:          %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em1000_td:        %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em1000_tx:        %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       em1000_ty:        %f\n",store->pos_delay);
		fprintf(stderr,"dbg5       survey_line:      %d\n",store->survey_line);
		fprintf(stderr,"dbg5       comment:          %s\n",store->comment);
		}

	/* write the record size */
	mb_put_binary_int(MB_NO, (int) (EM_PARAMETER_SIZE + 2), (void *) &write_size);
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	mb_put_binary_short(MB_NO, (short) EM_PARAMETER, (void *) &label);
	write_len = fwrite(&label,1,2,mbfp);
	if (write_len != 2)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* construct record */
		sprintf(line,"%2.2d%2.2d%2.2d,",
			store->par_day,store->par_month,store->par_year);
		sprintf(line+7,"%2.2d%2.2d%2.2d%2.2d,",
			store->par_hour,store->par_minute,
			store->par_second,store->par_centisecond);
		sprintf(line+16,  "PIS=%1d,",   store->pos_type);
		sprintf(line+22,  "PTD=%5.1f,",   store->pos_delay);
		sprintf(line+32,  "MSR=%5.2f,",   store->roll_offset);
		sprintf(line+42,  "MSP=%5.2f,",   store->pitch_offset);
		sprintf(line+52,  "MSG=%5.2f,",   store->heading_offset);
		sprintf(line+62,  "EM100TD=%5.1f,",   store->em100_td);
		sprintf(line+76,  "EM100TX=%5.1f,",   store->em100_tx);
		sprintf(line+90,  "EM100TY=%5.1f,",   store->em100_ty);
		sprintf(line+104, "EM12TD=%5.1f,",   store->em12_td);
		sprintf(line+117, "EM12TX=%5.1f,",   store->em12_tx);
		sprintf(line+130, "EM12TY=%5.1f,",   store->em12_ty);
		sprintf(line+143, "EM1000TD=%5.1f,",   store->em1000_td);
		sprintf(line+158, "EM1000TX=%5.1f,",   store->em1000_tx);
		sprintf(line+173, "EM1000TY=%5.1f,",   store->em1000_ty);
		for (i=0;i<128;i++)
			line[188+i] = store->spare_parameter[i];
		sprintf(line+316, "SURVEY_LINE_%4.4d,", store->survey_line);
		sprintf(line+333, "COMMENT:");
		for (i=0;i<80;i++)
			line[341+i] = store->comment[i];
		line[EM_PARAMETER_SIZE-3] = 0x03;
		
		/* compute and set checksum */
		uchar_ptr = (mb_u_char *) line;
		checksum = 0;
		for (j=0;j<EM_PARAMETER_SIZE-3;j++)
		    checksum += uchar_ptr[j];
		mb_put_binary_short(MB_YES, (short) checksum, (void *) &short_val);
		char_ptr = (char *) &short_val;
		line[EM_PARAMETER_SIZE-2] = char_ptr[0];
		line[EM_PARAMETER_SIZE-1] = char_ptr[1];

		/* write out data */
		write_len = fwrite(line,1,EM_PARAMETER_SIZE,mbfp);
		if (write_len != EM_PARAMETER_SIZE)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
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
int mbr_emoldraw_wr_pos(int verbose, FILE *mbfp,  
				struct mbsys_simrad_struct *store, int *error)
{
	char	*function_name = "mbr_emoldraw_wr_pos";
	int	status = MB_SUCCESS;
	char	line[EM_POS_SIZE];
	short int label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short int short_val;
	mb_u_char   *uchar_ptr;
	char   *char_ptr;
	double	degree_dec;
	int	degree;
	double	minute;
	char	hemisphere;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",store->pos_year);
		fprintf(stderr,"dbg5       month:            %d\n",store->pos_month);
		fprintf(stderr,"dbg5       day:              %d\n",store->pos_day);
		fprintf(stderr,"dbg5       hour:             %d\n",store->pos_hour);
		fprintf(stderr,"dbg5       minute:           %d\n",store->pos_minute);
		fprintf(stderr,"dbg5       sec:              %d\n",store->pos_second);
		fprintf(stderr,"dbg5       centisecond:      %d\n",store->pos_centisecond);
		fprintf(stderr,"dbg5       longitude:        %f\n",store->pos_longitude);
		fprintf(stderr,"dbg5       latitude:         %f\n",store->pos_latitude);
		fprintf(stderr,"dbg5       utm_northing:     %f\n",store->utm_northing);
		fprintf(stderr,"dbg5       utm_easting:      %f\n",store->utm_easting);
		fprintf(stderr,"dbg5       utm_zone:         %d\n",store->utm_zone);
		fprintf(stderr,"dbg5       utm_zone_lon:     %f\n",store->utm_zone_lon);
		fprintf(stderr,"dbg5       utm_system:       %d\n",store->utm_system);
		fprintf(stderr,"dbg5       pos_quality:      %d\n",store->pos_quality);
		fprintf(stderr,"dbg5       speed:            %f\n",store->speed);
		fprintf(stderr,"dbg5       line_heading:     %f\n",store->line_heading);
		}

	/* write the record size */
	mb_put_binary_int(MB_NO, (int) (EM_POS_SIZE + 2), (void *) &write_size);
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	mb_put_binary_short(MB_NO, (short) EM_POS, (void *) &label);
	write_len = fwrite(&label,1,2,mbfp);
	if (write_len != 2)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* construct record */
		sprintf(line,"%2.2d%2.2d%2.2d,",
			store->pos_day,store->pos_month,store->pos_year);
		sprintf(line+7,"%2.2d%2.2d%2.2d%2.2d,",
			store->pos_hour,store->pos_minute,
			store->pos_second,store->pos_centisecond);
		if (store->pos_latitude > 0.0)
			{
			hemisphere = 'N';
			degree_dec = store->pos_latitude;
			}
		else
			{
			hemisphere = 'S';
			degree_dec = -store->pos_latitude;
			}
		degree = (int) floor(degree_dec);
		minute = 60.0*(degree_dec - degree);
		sprintf(line+16,"%2.2d%7.4f%c,",degree,minute,hemisphere);
		if (store->pos_longitude > 180.0)
			store->pos_longitude = store->pos_longitude - 360.0;
		if (store->pos_longitude <= -180.0)
			store->pos_longitude = store->pos_longitude + 360.0;
		if (store->pos_longitude > 0.0)
			{
			hemisphere = 'E';
			degree_dec = store->pos_longitude;
			}
		else
			{
			hemisphere = 'W';
			degree_dec = -store->pos_longitude;
			}
		degree = (int) floor(degree_dec);
		minute = 60.0*(degree_dec - degree);
		sprintf(line+27,"%3.3d%7.4f%c,",degree,minute,hemisphere);
		sprintf(line+39,"%11.1f,%9.1f,%2.2d,",
			store->utm_northing,store->utm_easting,store->utm_zone);
		if (store->utm_zone_lon > 180.0)
			store->utm_zone_lon = store->utm_zone_lon - 360.0;
		if (store->utm_zone_lon <= -180.0)
			store->utm_zone_lon = store->utm_zone_lon + 360.0;
		if (store->utm_zone_lon > 0.0)
			{
			hemisphere = 'E';
			degree_dec = store->utm_zone_lon;
			}
		else
			{
			hemisphere = 'W';
			degree_dec = -store->utm_zone_lon;
			}
		degree = (int) floor(degree_dec);
		minute = 60.0*(degree_dec - degree);
		sprintf(line+64,"%3.3d%7.4f%c,",degree,minute,hemisphere);
		sprintf(line+76,"%1.1d,%1.1d,%4.1f,%5.1f",
			store->utm_system,store->pos_quality,
			store->speed,store->line_heading);
		line[EM_POS_SIZE-3] = 0x03;
		
		/* compute and set checksum */
		uchar_ptr = (mb_u_char *) line;
		checksum = 0;
		for (j=0;j<EM_POS_SIZE-3;j++)
		    checksum += uchar_ptr[j];
		mb_put_binary_short(MB_YES, (short) checksum, (void *) &short_val);
		char_ptr = (char *) &short_val;
		line[EM_POS_SIZE-2] = char_ptr[0];
		line[EM_POS_SIZE-1] = char_ptr[1];

		/* write out data */
		write_len = fwrite(line,1,EM_POS_SIZE,mbfp);
		if (write_len != EM_POS_SIZE)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
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
int mbr_emoldraw_wr_svp(int verbose, FILE *mbfp,  
				struct mbsys_simrad_struct *store, int *error)
{
	char	*function_name = "mbr_emoldraw_wr_svp";
	int	status = MB_SUCCESS;
	char	line[EM_SVP_SIZE];
	short int label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short int short_val;
	mb_u_char   *uchar_ptr;
	char   *char_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",store->svp_year);
		fprintf(stderr,"dbg5       month:            %d\n",store->svp_month);
		fprintf(stderr,"dbg5       day:              %d\n",store->svp_day);
		fprintf(stderr,"dbg5       hour:             %d\n",store->svp_hour);
		fprintf(stderr,"dbg5       minute:           %d\n",store->svp_minute);
		fprintf(stderr,"dbg5       sec:              %d\n",store->svp_second);
		fprintf(stderr,"dbg5       centisecond:      %d\n",store->svp_centisecond);
		fprintf(stderr,"dbg5       svp_num:          %d\n",store->svp_num);
		for (i=0;i<store->svp_num;i++)
			fprintf(stderr,"dbg5       depth: %d     vel: %d\n",
				store->svp_depth[i],store->svp_vel[i]);
		}

	/* write the record size */
	mb_put_binary_int(MB_NO, (int) (EM_SVP_SIZE + 2), (void *) &write_size);
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	mb_put_binary_short(MB_NO, (short) EM_SVP, (void *) &label);
	write_len = fwrite(&label,1,2,mbfp);
	if (write_len != 2)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* construct record */
		sprintf(line,"%2.2d%2.2d%2.2d",
			store->svp_day,store->svp_month,store->svp_year);
		sprintf(line+6,"%2.2d%2.2d%2.2d%2.2d",
			store->svp_hour,store->svp_minute,
			store->svp_second,store->svp_centisecond);
		mb_put_binary_short(MB_YES, (short) store->svp_num, (void *) &line[14]);
		for (i=0;i<store->svp_num;i++)
			{
			mb_put_binary_short(MB_YES, (short) store->svp_depth[i], (void *) &line[16+4*i]);
			mb_put_binary_short(MB_YES, (short) store->svp_vel[i], (void *) &line[18+4*i]);
			}
		for (i=store->svp_num;i<100;i++)
			{
			mb_put_binary_short(MB_YES, (short) 0, (void *) &line[16+4*i]);
			mb_put_binary_short(MB_YES, (short) 0, (void *) &line[18+4*i]);
			}
		line[EM_SVP_SIZE-3] = 0x03;
		
		/* compute and set checksum */
		uchar_ptr = (mb_u_char *) line;
		checksum = 0;
		for (j=0;j<EM_SVP_SIZE-3;j++)
		    checksum += uchar_ptr[j];
		mb_put_binary_short(MB_YES, (short) checksum, (void *) &short_val);
		char_ptr = (char *) &short_val;
		line[EM_SVP_SIZE-2] = char_ptr[0];
		line[EM_SVP_SIZE-1] = char_ptr[1];

		/* write out data */
		write_len = fwrite(line,1,EM_SVP_SIZE,mbfp);
		if (write_len != EM_SVP_SIZE)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
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
int mbr_emoldraw_wr_em1000bath(int verbose, FILE *mbfp,  
				struct mbsys_simrad_struct *store, int *error)
{
	char	*function_name = "mbr_emoldraw_wr_em1000bath";
	int	status = MB_SUCCESS;
	struct mbsys_simrad_survey_struct *ping;
	char	line[EM_1000_BATH_SIZE];
	char	beamarray[11];
	short int label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short int short_val;
	mb_u_char   *uchar_ptr;
	char   *char_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}

	/* get ping structure */
	ping = (struct mbsys_simrad_survey_struct *) store->ping;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",store->year);
		fprintf(stderr,"dbg5       month:            %d\n",store->month);
		fprintf(stderr,"dbg5       day:              %d\n",store->day);
		fprintf(stderr,"dbg5       hour:             %d\n",store->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",store->minute);
		fprintf(stderr,"dbg5       sec:              %d\n",store->second);
		fprintf(stderr,"dbg5       centisecond:      %d\n",store->centisecond);
		fprintf(stderr,"dbg5       ping_number:      %d\n",ping->ping_number);
		fprintf(stderr,"dbg5       beams_bath:       %d\n",ping->beams_bath);
		fprintf(stderr,"dbg5       bath_mode:        %d\n",ping->bath_mode);
		fprintf(stderr,"dbg5       bath_res:         %d\n",ping->bath_res);
		fprintf(stderr,"dbg5       bath_quality:     %d\n",ping->ping_number);
		fprintf(stderr,"dbg5       keel_depth:       %d\n",ping->keel_depth);
		fprintf(stderr,"dbg5       heading:          %d\n",ping->heading);
		fprintf(stderr,"dbg5       roll:             %d\n",ping->roll);
		fprintf(stderr,"dbg5       pitch:            %d\n",ping->pitch);
		fprintf(stderr,"dbg5       xducer_pitch:     %d\n",ping->xducer_pitch);
		fprintf(stderr,"dbg5       ping_heave:       %d\n",ping->ping_heave);
		fprintf(stderr,"dbg5       sound_vel:        %d\n",ping->sound_vel);
		fprintf(stderr,"dbg5       beam bath xtrack ltrack tt amp qual heave\n");
		for (i=0;i<ping->beams_bath;i++)
			fprintf(stderr,"dbg5       beam:%d  bath:%d  xtrck:%d  ltrck:%d tt:%d  amp:%d  qual:%d  heave:%d\n",
				i,ping->bath[i],ping->bath_acrosstrack[i],
				ping->bath_alongtrack[i],ping->tt[i],
				ping->amp[i],ping->quality[i],ping->heave[i]);
		}

	/* write the record size */
	mb_put_binary_int(MB_NO, (int) (EM_1000_BATH_SIZE + 2), (void *) &write_size);
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	mb_put_binary_short(MB_NO, (short) EM_1000_BATH, (void *) &label);
	write_len = fwrite(&label,1,2,mbfp);
	if (write_len != 2)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* construct record */
		sprintf(line,"%2.2d%2.2d%2.2d",
			store->day,store->month,store->year);
		sprintf(line+6,"%2.2d%2.2d%2.2d%2.2d",
			store->hour,store->minute,
			store->second,store->centisecond);
		mb_put_binary_short(MB_YES, (short) ping->ping_number, (void *) &line[14]);
		line[16] = (char) ping->bath_mode;
		line[17] = (char) ping->bath_quality;
		mb_put_binary_short(MB_YES, (short) ping->keel_depth, (void *) &line[18]);
		mb_put_binary_short(MB_YES, (short) ping->heading, (void *) &line[20]);
		mb_put_binary_short(MB_YES, (short) ping->roll, (void *) &line[22]);
		mb_put_binary_short(MB_YES, (short) ping->pitch, (void *) &line[24]);
		mb_put_binary_short(MB_YES, (short) ping->xducer_pitch, (void *) &line[26]);
		mb_put_binary_short(MB_YES, (short) ping->ping_heave, (void *) &line[28]);
		mb_put_binary_short(MB_YES, (short) ping->sound_vel, (void *) &line[30]);
		for (i=0;i<MBSYS_EM1000_MAXBEAMS;i++)
			{
			mb_put_binary_short(MB_YES, (short) ping->bath[i], (void *) &beamarray[0]);
			mb_put_binary_short(MB_YES, (short) ping->bath_acrosstrack[i], (void *) &beamarray[2]);
			mb_put_binary_short(MB_YES, (short) ping->bath_alongtrack[i], (void *) &beamarray[4]);
			mb_put_binary_short(MB_YES, (short) ping->tt[i], (void *) &beamarray[6]);
			beamarray[8] = (char) ping->amp[i];
			beamarray[9] = (char) ping->quality[i];
			beamarray[10] = (char) ping->heave[i];
			for (j=0;j<11;j++)
				line[32+11*i+j] = beamarray[j];
			}
		line[EM_1000_BATH_SIZE-3] = 0x03;
		
		/* compute and set checksum */
		uchar_ptr = (mb_u_char *) line;
		checksum = 0;
		for (j=0;j<EM_1000_BATH_SIZE-3;j++)
		    checksum += uchar_ptr[j];
		mb_put_binary_short(MB_YES, (short) checksum, (void *) &short_val);
		char_ptr = (char *) &short_val;
		line[EM_1000_BATH_SIZE-2] = char_ptr[0];
		line[EM_1000_BATH_SIZE-1] = char_ptr[1];

		/* write out data */
		write_len = fwrite(line,1,EM_1000_BATH_SIZE,mbfp);
		if (write_len != EM_1000_BATH_SIZE)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
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
int mbr_emoldraw_wr_em12bath(int verbose, FILE *mbfp,  
				struct mbsys_simrad_struct *store, int *error)
{
	char	*function_name = "mbr_emoldraw_wr_em12bath";
	int	status = MB_SUCCESS;
	struct mbsys_simrad_survey_struct *ping;
	char	line[EM_12S_BATH_SIZE];
	char	beamarray[11];
	short int label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short int short_val;
	mb_u_char   *uchar_ptr;
	char   *char_ptr;
	short int *short_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}

	/* get ping structure */
	ping = (struct mbsys_simrad_survey_struct *) store->ping;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",store->year);
		fprintf(stderr,"dbg5       month:            %d\n",store->month);
		fprintf(stderr,"dbg5       day:              %d\n",store->day);
		fprintf(stderr,"dbg5       hour:             %d\n",store->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",store->minute);
		fprintf(stderr,"dbg5       sec:              %d\n",store->second);
		fprintf(stderr,"dbg5       centisecond:      %d\n",store->centisecond);
		fprintf(stderr,"dbg5       ping_number:      %d\n",ping->ping_number);
		fprintf(stderr,"dbg5       beams_bath:       %d\n",ping->beams_bath);
		fprintf(stderr,"dbg5       bath_mode:        %d\n",ping->bath_mode);
		fprintf(stderr,"dbg5       bath_res:         %d\n",ping->bath_res);
		fprintf(stderr,"dbg5       bath_quality:     %d\n",ping->bath_quality);
		fprintf(stderr,"dbg5       keel_depth:       %d\n",ping->keel_depth);
		fprintf(stderr,"dbg5       heading:          %d\n",ping->heading);
		fprintf(stderr,"dbg5       roll:             %d\n",ping->roll);
		fprintf(stderr,"dbg5       pitch:            %d\n",ping->pitch);
		fprintf(stderr,"dbg5       ping_heave:       %d\n",ping->ping_heave);
		fprintf(stderr,"dbg5       sound_vel:        %d\n",ping->sound_vel);
		fprintf(stderr,"dbg5       beam bath xtrack ltrack tt amp qual heave\n");
		for (i=0;i<ping->beams_bath;i++)
			fprintf(stderr,"dbg5       beam:%d  bath:%d  xtrck:%d  ltrck:%d tt:%d  amp:%d  qual:%d  heave:%d\n",
				i,ping->bath[i],ping->bath_acrosstrack[i],
				ping->bath_alongtrack[i],ping->tt[i],
				ping->amp[i],ping->quality[i],ping->heave[i]);
		}

	/* write the record size */
	mb_put_binary_int(MB_NO, (int) (EM_12S_BATH_SIZE + 2), (void *) &write_size);
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (ping->swath_id == EM_SWATH_CENTER)
		mb_put_binary_short(MB_NO, (short) EM_12S_BATH, (void *) &label);
	else if (ping->swath_id == EM_SWATH_PORT)
		mb_put_binary_short(MB_NO, (short) EM_12DP_BATH, (void *) &label);
	else
		mb_put_binary_short(MB_NO, (short) EM_12DS_BATH, (void *) &label);
	write_len = fwrite(&label,1,2,mbfp);
	if (write_len != 2)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* construct record */
		sprintf(line,"%2.2d%2.2d%2.2d",
			store->day,store->month,store->year);
		sprintf(line+6,"%2.2d%2.2d%2.2d%2.2d",
			store->hour,store->minute,
			store->second,store->centisecond);
		mb_put_binary_short(MB_YES, (short) ping->ping_number, (void *) &line[14]);
		line[16] = (char) ping->bath_res;
		line[17] = (char) ping->bath_quality;
		mb_put_binary_short(MB_YES, (short) ping->keel_depth, (void *) &line[18]);
		mb_put_binary_short(MB_YES, (short) ping->heading, (void *) &line[20]);
		mb_put_binary_short(MB_YES, (short) ping->roll, (void *) &line[22]);
		mb_put_binary_short(MB_YES, (short) ping->pitch, (void *) &line[24]);
		mb_put_binary_short(MB_YES, (short) ping->ping_heave, (void *) &line[26]);
		mb_put_binary_short(MB_YES, (short) ping->sound_vel, (void *) &line[28]);
		line[30] = (char) ping->bath_mode;
		line[31] = (char) 0;
		for (i=0;i<MBSYS_EM12_MAXBEAMS;i++)
			{
			mb_put_binary_short(MB_YES, (short) ping->bath[i], (void *) &beamarray[0]);
			mb_put_binary_short(MB_YES, (short) ping->bath_acrosstrack[i], (void *) &beamarray[2]);
			mb_put_binary_short(MB_YES, (short) ping->bath_alongtrack[i], (void *) &beamarray[4]);
			mb_put_binary_short(MB_YES, (short) ping->tt[i], (void *) &beamarray[6]);
			beamarray[8] = (char) ping->amp[i];
			beamarray[9] = (char) ping->quality[i];
			beamarray[10] = (char) ping->heave[i];
			for (j=0;j<11;j++)
				line[32+11*i+j] = beamarray[j];
			}
		line[EM_12S_BATH_SIZE-3] = 0x03;
		
		/* compute and set checksum */
		uchar_ptr = (mb_u_char *) line;
		checksum = 0;
		for (j=0;j<EM_12S_BATH_SIZE-3;j++)
		    checksum += uchar_ptr[j];
		mb_put_binary_short(MB_YES, (short) checksum, (void *) &short_val);
		char_ptr = (char *) &short_val;
		line[EM_12S_BATH_SIZE-2] = char_ptr[0];
		line[EM_12S_BATH_SIZE-1] = char_ptr[1];

		/* write out data */
		write_len = fwrite(line,1,EM_12S_BATH_SIZE,mbfp);
		if (write_len != EM_12S_BATH_SIZE)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
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
int mbr_emoldraw_wr_em121bath(int verbose, FILE *mbfp,  
				struct mbsys_simrad_struct *store, int *error)
{
	char	*function_name = "mbr_emoldraw_wr_em121bath";
	int	status = MB_SUCCESS;
	struct mbsys_simrad_survey_struct *ping;
	char	line[EM_121_BATH_SIZE];
	char	beamarray[11];
	short int label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short int short_val;
	mb_u_char   *uchar_ptr;
	char   *char_ptr;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}

	/* get ping structure */
	ping = (struct mbsys_simrad_survey_struct *) store->ping;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",store->year);
		fprintf(stderr,"dbg5       month:            %d\n",store->month);
		fprintf(stderr,"dbg5       day:              %d\n",store->day);
		fprintf(stderr,"dbg5       hour:             %d\n",store->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",store->minute);
		fprintf(stderr,"dbg5       sec:              %d\n",store->second);
		fprintf(stderr,"dbg5       centisecond:      %d\n",store->centisecond);
		fprintf(stderr,"dbg5       ping_number:      %d\n",ping->ping_number);
		fprintf(stderr,"dbg5       beams_bath:       %d\n",ping->beams_bath);
		fprintf(stderr,"dbg5       bath_mode:        %d\n",ping->bath_mode);
		fprintf(stderr,"dbg5       bath_quality:     %d\n",ping->bath_quality);
		fprintf(stderr,"dbg5       bath_num:         %d\n",ping->bath_num);
		fprintf(stderr,"dbg5       pulse_length:     %d\n",ping->pulse_length);
		fprintf(stderr,"dbg5       beam_width:       %d\n",ping->beam_width);
		fprintf(stderr,"dbg5       power_level:      %d\n",ping->power_level);
		fprintf(stderr,"dbg5       tx_status:        %d\n",ping->tx_status);
		fprintf(stderr,"dbg5       rx_status:        %d\n",ping->rx_status);
		fprintf(stderr,"dbg5       along_res:        %d\n",ping->along_res);
		fprintf(stderr,"dbg5       across_res:       %d\n",ping->across_res);
		fprintf(stderr,"dbg5       depth_res:        %d\n",ping->depth_res);
		fprintf(stderr,"dbg5       range_res:        %d\n",ping->range_res);
		fprintf(stderr,"dbg5       keel_depth:       %d\n",ping->keel_depth);
		fprintf(stderr,"dbg5       heading:          %d\n",ping->heading);
		fprintf(stderr,"dbg5       roll:             %d\n",ping->roll);
		fprintf(stderr,"dbg5       pitch:            %d\n",ping->pitch);
		fprintf(stderr,"dbg5       ping_heave:       %d\n",ping->ping_heave);
		fprintf(stderr,"dbg5       sound_vel:        %d\n",ping->sound_vel);
		fprintf(stderr,"dbg5       beam bath xtrack ltrack tt amp qual heave\n");
		for (i=0;i<ping->beams_bath;i++)
			fprintf(stderr,"dbg5       beam:%d  bath:%d  xtrck:%d  ltrck:%d tt:%d  amp:%d  qual:%d  heave:%d\n",
				i,ping->bath[i],ping->bath_acrosstrack[i],
				ping->bath_alongtrack[i],ping->tt[i],
				ping->amp[i],ping->quality[i],ping->heave[i]);
		}

	/* write the record size */
	mb_put_binary_int(MB_NO, (int) (EM_121_BATH_SIZE + 2), (void *) &write_size);
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	mb_put_binary_short(MB_NO, (short) EM_121_BATH, (void *) &label);
	write_len = fwrite(&label,1,2,mbfp);
	if (write_len != 2)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* construct record */
		sprintf(line,"%2.2d%2.2d%2.2d",
			store->day,store->month,store->year);
		sprintf(line+6,"%2.2d%2.2d%2.2d%2.2d",
			store->hour,store->minute,
			store->second,store->centisecond);
		mb_put_binary_short(MB_YES, (short) ping->ping_number, (void *) &line[14]);
		line[16] = (char) ping->bath_mode;
		line[17] = (char) ping->bath_quality;
		line[18] = (char) ping->bath_num;
		line[19] = (char) ping->pulse_length;
		line[20] = (char) ping->beam_width;
		line[21] = (char) ping->power_level;
		line[22] = (char) ping->tx_status;
		line[23] = (char) ping->rx_status;
		mb_put_binary_short(MB_YES, (short) ping->keel_depth, (void *) &line[24]);
		mb_put_binary_short(MB_YES, (unsigned short) ping->heading, (void *) &line[26]);
		mb_put_binary_short(MB_YES, (short) ping->roll, (void *) &line[28]);
		mb_put_binary_short(MB_YES, (short) ping->pitch, (void *) &line[30]);
		mb_put_binary_short(MB_YES, (short) ping->ping_heave, (void *) &line[32]);
		mb_put_binary_short(MB_YES, (short) ping->sound_vel, (void *) &line[34]);
		line[36] = (char) ping->along_res;
		line[37] = (char) ping->across_res;
		line[38] = (char) ping->depth_res;
		line[39] = (char) ping->range_res;
		for (i=0;i<MBSYS_EM121_MAXBEAMS;i++)
			{
			mb_put_binary_short(MB_YES, (short) ping->bath[i], (void *) &beamarray[0]);
			mb_put_binary_short(MB_YES, (short) ping->bath_acrosstrack[i], (void *) &beamarray[2]);
			mb_put_binary_short(MB_YES, (short) ping->bath_alongtrack[i], (void *) &beamarray[4]);
			mb_put_binary_short(MB_YES, (short) ping->tt[i], (void *) &beamarray[6]);
			beamarray[8] = (char) ping->amp[i];
			beamarray[9] = (char) ping->quality[i];
			beamarray[10] = (char) ping->heave[i];
			for (j=0;j<11;j++)
				line[44+11*i+j] = beamarray[j];
			}
		line[EM_121_BATH_SIZE-3] = 0x03;
		
		/* compute and set checksum */
		uchar_ptr = (mb_u_char *) line;
		checksum = 0;
		for (j=0;j<EM_121_BATH_SIZE-3;j++)
		    checksum += uchar_ptr[j];
		mb_put_binary_short(MB_YES, (short) checksum, (void *) &short_val);
		char_ptr = (char *) &short_val;
		line[EM_121_BATH_SIZE-2] = char_ptr[0];
		line[EM_121_BATH_SIZE-1] = char_ptr[1];

		/* write out data */
		write_len = fwrite(line,1,EM_121_BATH_SIZE,mbfp);
		if (write_len != EM_121_BATH_SIZE)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
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
int mbr_emoldraw_wr_ss(int verbose, FILE *mbfp,  
				struct mbsys_simrad_struct *store, int *error)
{
	char	*function_name = "mbr_emoldraw_wr_ss";
	int	status = MB_SUCCESS;
	struct mbsys_simrad_survey_struct *ping;
	char	line[EM_SS_SIZE];
	short int label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short int short_val;
	mb_u_char   *uchar_ptr;
	char   *char_ptr;
	int	num_datagrams;
	int	datagram;
	int	num_beams;
	int	datagram_start[MBSYS_EM1000_MAXBEAMS+1];
	int	datagram_end[MBSYS_EM1000_MAXBEAMS+1];
	int	datagram_size[MBSYS_EM1000_MAXBEAMS+1];
	int	new_datagram_size;
	mb_s_char *beam_ss;
	int	ioffset;
	int	odatagram;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}

	/* get ping structure */
	ping = (struct mbsys_simrad_survey_struct *) store->ping;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",store->year);
		fprintf(stderr,"dbg5       month:            %d\n",store->month);
		fprintf(stderr,"dbg5       day:              %d\n",store->day);
		fprintf(stderr,"dbg5       hour:             %d\n",store->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",store->minute);
		fprintf(stderr,"dbg5       sec:              %d\n",store->second);
		fprintf(stderr,"dbg5       centisecond:      %d\n",store->centisecond);
		fprintf(stderr,"dbg5       ping_number:      %d\n",ping->ping_number);
		fprintf(stderr,"dbg5       sound_vel:        %d\n",ping->sound_vel);
		fprintf(stderr,"dbg5       ss_mode:          %d\n",ping->ss_mode);
		fprintf(stderr,"dbg5       beam frequency samples center start\n");
		for (i=0;i<MBSYS_EM1000_MAXBEAMS;i++)
			fprintf(stderr,"dbg5       beam:%d  frequency:%d  samples:%d  center:%d  start:%d\n",
				i,ping->beam_frequency[i],
				ping->beam_samples[i],
				ping->beam_center_sample[i],
				ping->beam_start_sample[i]);
		for (i=0;i<MBSYS_EM1000_MAXBEAMS;i++)
			{
			beam_ss = &ping->ssraw[ping->beam_start_sample[i]];
			for (j=0;j<ping->beam_samples[i];j++)
				fprintf(stderr,"dbg5       beam:%d pixel:%d  amp:%d\n",
					i,j,beam_ss[j]);
			}
		}

	/* preprocess data to figure out how many 
		sidescan datagrams to output */
	num_datagrams = 0;
	datagram_size[0] = 22;
	datagram_start[0] = 0;
	datagram_end[0] = 0;
	for (i=0;i<MBSYS_EM1000_MAXBEAMS;i++)
		{
		new_datagram_size = datagram_size[num_datagrams] + 6 + ping->beam_samples[i];
		if (new_datagram_size > 551 
			&& i == MBSYS_EM1000_MAXBEAMS - 1)
			{
			datagram_end[num_datagrams] = i - 1;
			num_datagrams++;
			datagram_start[num_datagrams] = i;
			datagram_end[num_datagrams] = 
				MBSYS_EM1000_MAXBEAMS - 1;
			datagram_size[num_datagrams] = 
				22 + 6 + ping->beam_samples[i];
			num_datagrams++;
			}
		else if (new_datagram_size > 551)
			{
			datagram_end[num_datagrams] = i - 1;
			num_datagrams++;
			datagram_start[num_datagrams] = i;
			datagram_end[num_datagrams] = 
				MBSYS_EM1000_MAXBEAMS - 1;
			datagram_size[num_datagrams] = 
				22 + 6 + ping->beam_samples[i];
			}
		else if (new_datagram_size == 551)
			{
			datagram_end[num_datagrams] = i;
			datagram_size[num_datagrams] = new_datagram_size;
			num_datagrams++;
			datagram_start[num_datagrams] = i + 1;
			datagram_end[num_datagrams] = 
				MBSYS_EM1000_MAXBEAMS - 1;
			datagram_size[num_datagrams] = 22;
			}
		else if (i == MBSYS_EM1000_MAXBEAMS - 1)
			{
			datagram_end[num_datagrams] = i;
			datagram_size[num_datagrams] = new_datagram_size;
			num_datagrams++;
			}
		else
			datagram_size[num_datagrams] = new_datagram_size;
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       num_datagrams:    %d\n",num_datagrams);
		for (datagram=0;datagram<num_datagrams;datagram++)
			{
			fprintf(stderr,"\ndbg5       datagram[%d]:  beam %d to beam %d\n",
				datagram,datagram_start[datagram],datagram_end[datagram]);
			for (i=datagram_start[datagram];i<=datagram_end[datagram];i++)
				fprintf(stderr,"dbg5       beam:%d  frequency:%d  samples:%d  center:%d\n",
					i,ping->beam_frequency[i],
					ping->beam_samples[i],
					ping->beam_center_sample[i]);
			}
		}

	/* now loop over all of the sidescan datagrams to be written */
	for (datagram=0;datagram<num_datagrams;datagram++)
	{
	num_beams = datagram_end[datagram] 
		- datagram_start[datagram] + 1;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",store->year);
		fprintf(stderr,"dbg5       month:            %d\n",store->month);
		fprintf(stderr,"dbg5       day:              %d\n",store->day);
		fprintf(stderr,"dbg5       hour:             %d\n",store->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",store->minute);
		fprintf(stderr,"dbg5       sec:              %d\n",store->second);
		fprintf(stderr,"dbg5       centisecond:      %d\n",store->centisecond);
		fprintf(stderr,"dbg5       ping_number:      %d\n",ping->ping_number);
		fprintf(stderr,"dbg5       sound_vel:        %d\n",ping->sound_vel);
		fprintf(stderr,"dbg5       ss_mode:          %d\n",ping->ss_mode);
		fprintf(stderr,"dbg5       num_datagrams:    %d\n",num_datagrams);
		fprintf(stderr,"dbg5       datagram:         %d\n",datagram);
		fprintf(stderr,"dbg5       num_beams:        %d\n",num_beams);
		for (i=datagram_start[datagram];i<=datagram_end[datagram];i++)
			fprintf(stderr,"dbg5       beam:%d  frequency:%d  samples:%d  center:%d  start:%d\n",
				i,ping->beam_frequency[i],
				ping->beam_samples[i],
				ping->beam_center_sample[i],
				ping->beam_start_sample[i]);
		for (i=datagram_start[datagram];i<=datagram_end[datagram];i++)
			{
			beam_ss = &ping->ssraw[ping->beam_start_sample[i]];
			for (j=0;j<ping->beam_samples[i];j++)
				fprintf(stderr,"dbg5       beam:%d pixel:%d  amp:%d\n",
					i,j,beam_ss[j]);
			}
		}

	/* write the record size */
	mb_put_binary_int(MB_NO, (int) (EM_SS_SIZE + 2), (void *) &write_size);
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (ping->swath_id == EM_SWATH_CENTER)
		mb_put_binary_short(MB_NO, (short) EM_12S_SS, (void *) &label);
	else if (ping->swath_id == EM_SWATH_PORT)
		mb_put_binary_short(MB_NO, (short) EM_12DP_SS, (void *) &label);
	else
		mb_put_binary_short(MB_NO, (short) EM_12DS_SS, (void *) &label);
	write_len = fwrite(&label,1,2,mbfp);
	if (write_len != 2)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* construct record */
		sprintf(line,"%2.2d%2.2d%2.2d",
			store->day,store->month,store->year);
		sprintf(line+6,"%2.2d%2.2d%2.2d%2.2d",
			store->hour,store->minute,
			store->second,store->centisecond);
		mb_put_binary_short(MB_YES, (short) ping->ping_number, (void *) &line[14]);
		mb_put_binary_short(MB_YES, (short) ping->sound_vel, (void *) &line[16]);
		line[18] = (char) ping->ss_mode;
		line[19] = (char) num_datagrams;
		odatagram = datagram + 1;
		line[20] = (char) odatagram;
		line[21] = (char) num_beams;
		j=0;
		for (i=datagram_start[datagram];i<=datagram_end[datagram];i++)
			{
			line[22 + 6*j] = (char) (i + 1);
			line[23 + 6*j] = (char) ping->beam_frequency[i];
			mb_put_binary_short(MB_YES, (short) ping->beam_samples[i], (void *) &line[24 + 6*j]);
			mb_put_binary_short(MB_YES, (short) ping->beam_center_sample[i], (void *) &line[26 + 6*j]);
			j++;
			}
		ioffset = 22 + 6*num_beams;
		for (i=datagram_start[datagram];i<=datagram_end[datagram];i++)
			{
			beam_ss = &ping->ssraw[ping->beam_start_sample[i]];
			for (j=0;j<ping->beam_samples[i];j++)
				{
				line[ioffset + j] = (char) beam_ss[j];
				}
			ioffset = ioffset + ping->beam_samples[i];
			}
		for (i=ioffset;i<EM_SS_SIZE;i++)
			line[i] = (char) 0;
		line[EM_SS_SIZE-3] = 0x03;
		
		/* compute and set checksum */
		uchar_ptr = (mb_u_char *) line;
		checksum = 0;
		for (j=0;j<EM_SS_SIZE-3;j++)
		    checksum += uchar_ptr[j];
		mb_put_binary_short(MB_YES, (short) checksum, (void *) &short_val);
		char_ptr = (char *) &short_val;
		line[EM_SS_SIZE-2] = char_ptr[0];
		line[EM_SS_SIZE-1] = char_ptr[1];

		/* write out data */
		write_len = fwrite(line,1,EM_SS_SIZE,mbfp);
		if (write_len != EM_SS_SIZE)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		}

	/* end loop over datagrams */
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
int mbr_emoldraw_wr_ssp(int verbose, FILE *mbfp,  
				struct mbsys_simrad_struct *store, int *error)
{
	char	*function_name = "mbr_emoldraw_wr_ssp";
	int	status = MB_SUCCESS;
	struct mbsys_simrad_survey_struct *ping;
	char	line[EM_SSP_SIZE];
	short int label;
	int	write_len;
	int	write_size;
	unsigned short checksum;
	short int short_val;
	mb_u_char   *uchar_ptr;
	char   *char_ptr;
	int	num_datagrams;
	int	datagram;
	int	num_beams;
	int	datagram_start[MBSYS_SIMRAD_MAXBEAMS+1];
	int	datagram_end[MBSYS_SIMRAD_MAXBEAMS+1];
	int	datagram_size[MBSYS_SIMRAD_MAXBEAMS+1];
	int	new_datagram_size;
	mb_s_char *beam_ss;
	short int *beam_ssp;
	int	ioffset;
	int	odatagram;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       store:      %d\n",store);
		}

	/* get ping structure */
	ping = (struct mbsys_simrad_survey_struct *) store->ping;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",store->year);
		fprintf(stderr,"dbg5       month:            %d\n",store->month);
		fprintf(stderr,"dbg5       day:              %d\n",store->day);
		fprintf(stderr,"dbg5       hour:             %d\n",store->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",store->minute);
		fprintf(stderr,"dbg5       sec:              %d\n",store->second);
		fprintf(stderr,"dbg5       centisecond:      %d\n",store->centisecond);
		fprintf(stderr,"dbg5       ping_number:      %d\n",ping->ping_number);
		fprintf(stderr,"dbg5       sound_vel:        %d\n",ping->sound_vel);
		fprintf(stderr,"dbg5       ss_mode:          %d\n",ping->ss_mode);
		fprintf(stderr,"dbg5       beam frequency samples center start\n");
		for (i=0;i<MBSYS_SIMRAD_MAXBEAMS;i++)
			fprintf(stderr,"dbg5       beam:%d  frequency:%d  samples:%d  center:%d  start:%d\n",
				i,ping->beam_frequency[i],
				ping->beam_samples[i],
				ping->beam_center_sample[i],
				ping->beam_start_sample[i]);
		for (i=0;i<MBSYS_SIMRAD_MAXBEAMS;i++)
			{
			beam_ss = &ping->ssraw[ping->beam_start_sample[i]];
			beam_ssp = &ping->ssp[ping->beam_start_sample[i]];
			for (j=0;j<ping->beam_samples[i];j++)
				fprintf(stderr,"dbg5       beam:%d pixel:%d  amp:%d phase:%d\n",
					i,j,beam_ss[j],beam_ssp[j]);
			}
		}

	/* preprocess data to figure out how many 
		sidescan datagrams to output */
	num_datagrams = 0;
	datagram_size[0] = 22;
	datagram_start[0] = 0;
	datagram_end[0] = 0;
	for (i=0;i<MBSYS_SIMRAD_MAXBEAMS;i++)
		{
		new_datagram_size = datagram_size[num_datagrams] + 6 + 3 * ping->beam_samples[i];
		if (new_datagram_size > 1465 
			&& i == MBSYS_SIMRAD_MAXBEAMS - 1)
			{
			datagram_end[num_datagrams] = i - 1;
			num_datagrams++;
			datagram_start[num_datagrams] = i;
			datagram_end[num_datagrams] = 
				MBSYS_SIMRAD_MAXBEAMS - 1;
			datagram_size[num_datagrams] = 
				22 + 6 + 3 * ping->beam_samples[i];
			num_datagrams++;
			}
		else if (new_datagram_size > 1465)
			{
			datagram_end[num_datagrams] = i - 1;
			num_datagrams++;
			datagram_start[num_datagrams] = i;
			datagram_end[num_datagrams] = 
				MBSYS_SIMRAD_MAXBEAMS - 1;
			datagram_size[num_datagrams] = 
				22 + 6 + 3 * ping->beam_samples[i];
			}
		else if (new_datagram_size == 1465)
			{
			datagram_end[num_datagrams] = i;
			datagram_size[num_datagrams] = new_datagram_size;
			num_datagrams++;
			datagram_start[num_datagrams] = i + 1;
			datagram_end[num_datagrams] = 
				MBSYS_SIMRAD_MAXBEAMS - 1;
			datagram_size[num_datagrams] = 22;
			}
		else if (i == MBSYS_SIMRAD_MAXBEAMS - 1)
			{
			datagram_end[num_datagrams] = i;
			datagram_size[num_datagrams] = new_datagram_size;
			num_datagrams++;
			}
		else
			datagram_size[num_datagrams] = new_datagram_size;
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       num_datagrams:    %d\n",num_datagrams);
		for (datagram=0;datagram<num_datagrams;datagram++)
			{
			fprintf(stderr,"\ndbg5       datagram[%d]:  beam %d to beam %d\n",
				datagram,datagram_start[datagram],datagram_end[datagram]);
			for (i=datagram_start[datagram];i<=datagram_end[datagram];i++)
				fprintf(stderr,"dbg5       beam:%d  frequency:%d  samples:%d  center:%d\n",
					i,ping->beam_frequency[i],
					ping->beam_samples[i],
					ping->beam_center_sample[i]);
			}
		}

	/* now loop over all of the sidescan datagrams to be written */
	for (datagram=0;datagram<num_datagrams;datagram++)
	{
	num_beams = datagram_end[datagram] 
		- datagram_start[datagram] + 1;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",store->year);
		fprintf(stderr,"dbg5       month:            %d\n",store->month);
		fprintf(stderr,"dbg5       day:              %d\n",store->day);
		fprintf(stderr,"dbg5       hour:             %d\n",store->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",store->minute);
		fprintf(stderr,"dbg5       sec:              %d\n",store->second);
		fprintf(stderr,"dbg5       centisecond:      %d\n",store->centisecond);
		fprintf(stderr,"dbg5       ping_number:      %d\n",ping->ping_number);
		fprintf(stderr,"dbg5       sound_vel:        %d\n",ping->sound_vel);
		fprintf(stderr,"dbg5       ss_mode:          %d\n",ping->ss_mode);
		fprintf(stderr,"dbg5       num_datagrams:    %d\n",num_datagrams);
		fprintf(stderr,"dbg5       datagram:         %d\n",datagram);
		fprintf(stderr,"dbg5       num_beams:        %d\n",num_beams);
		for (i=datagram_start[datagram];i<=datagram_end[datagram];i++)
			fprintf(stderr,"dbg5       beam:%d  frequency:%d  samples:%d  center:%d  start:%d\n",
				i,ping->beam_frequency[i],
				ping->beam_samples[i],
				ping->beam_center_sample[i],
				ping->beam_start_sample[i]);
		for (i=datagram_start[datagram];i<=datagram_end[datagram];i++)
			{
			beam_ss = &ping->ssraw[ping->beam_start_sample[i]];
			beam_ssp = &ping->ssp[ping->beam_start_sample[i]];
			for (j=0;j<ping->beam_samples[i];j++)
				fprintf(stderr,"dbg5       beam:%d pixel:%d  amp:%d phase:%d\n",
					i,j,beam_ss[j],beam_ssp[j]);
			}
		}

	/* write the record size */
	mb_put_binary_int(MB_NO, (int) (EM_12S_SSP_SIZE + 2), (void *) &write_size);
	write_len = fwrite(&write_size,1,4,mbfp);
	if (write_len != 4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write the record label */
	if (ping->swath_id == EM_SWATH_CENTER)
		mb_put_binary_short(MB_NO, (short) EM_12S_SSP, (void *) &label);
	else if (ping->swath_id == EM_SWATH_PORT)
		mb_put_binary_short(MB_NO, (short) EM_12DP_SSP, (void *) &label);
	else
		mb_put_binary_short(MB_NO, (short) EM_12DS_SSP, (void *) &label);
	write_len = fwrite(&label,1,2,mbfp);
	if (write_len != 2)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	else
		status = MB_SUCCESS;

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* construct record */
		sprintf(line,"%2.2d%2.2d%2.2d",
			store->day,store->month,store->year);
		sprintf(line+6,"%2.2d%2.2d%2.2d%2.2d",
			store->hour,store->minute,
			store->second,store->centisecond);
		mb_put_binary_short(MB_YES, (short) ping->ping_number, (void *) &line[14]);
		mb_put_binary_short(MB_YES, (short) ping->sound_vel, (void *) &line[16]);
		line[18] = (char) ping->ss_mode;
		line[19] = (char) num_datagrams;
		odatagram = datagram + 1;
		line[20] = (char) odatagram;
		line[21] = (char) num_beams;
		j=0;
		for (i=datagram_start[datagram];i<=datagram_end[datagram];i++)
			{
			line[22 + 6*j] = (char) (i + 1);
			line[23 + 6*j] = (char) ping->beam_frequency[i];
			mb_put_binary_short(MB_YES, (short) ping->beam_samples[i], (void *) &line[24 + 6*j]);
			mb_put_binary_short(MB_YES, (short) ping->beam_center_sample[i], (void *) &line[26 + 6*j]);
			j++;
			}
		ioffset = 22 + 6*num_beams;
		char_ptr = (char *) &short_val;
		for (i=datagram_start[datagram];i<=datagram_end[datagram];i++)
			{
			beam_ss = &ping->ssraw[ping->beam_start_sample[i]];
			beam_ssp = &ping->ssp[ping->beam_start_sample[i]];
			for (j=0;j<ping->beam_samples[i];j++)
				{
				line[ioffset] = (char) beam_ss[j];
				mb_put_binary_short(MB_YES, (short) beam_ssp[j], (void *) &short_val);
				line[ioffset+1] = char_ptr[0];
				line[ioffset+2] = char_ptr[1];
				ioffset += 3;
				}
			}
		for (i=ioffset;i<EM_SSP_SIZE;i++)
			line[i] = (char) 0;

		line[EM_SSP_SIZE-3] = 0x03;
		
		/* compute and set checksum */
		uchar_ptr = (mb_u_char *) line;
		checksum = 0;
		for (j=0;j<EM_SSP_SIZE-3;j++)
		    checksum += uchar_ptr[j];
		mb_put_binary_short(MB_YES, (short) checksum, (void *) &short_val);
		char_ptr = (char *) &short_val;
		line[EM_SSP_SIZE-2] = char_ptr[0];
		line[EM_SSP_SIZE-1] = char_ptr[1];

		/* write out data */
		write_len = fwrite(line,1,EM_SSP_SIZE,mbfp);
		if (write_len != EM_SSP_SIZE)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		}

	/* end loop over datagrams */
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
