/*--------------------------------------------------------------------
 *    The MB-system:	mbr_omghdcsj.c	3/10/99
 *	$Id: mbr_omghdcsj.c,v 5.2 2001-03-22 20:50:02 caress Exp $
 *
 *    Copyright (c) 1999, 2000 by
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
 * mbr_omghdcsj.c contains the functions for reading and writing
 * multibeam data in the MBF_OMGHDCSJ format.  
 * These functions include:
 *   mbr_alm_omghdcsj	- allocate read/write memory
 *   mbr_dem_omghdcsj	- deallocate read/write memory
 *   mbr_rt_omghdcsj	- read and translate data
 *   mbr_wt_omghdcsj	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	March 10, 1999
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.1  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.5  2000/10/11  01:03:21  caress
 * Convert to ANSI C
 *
 * Revision 4.4  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.3  1999/08/08  04:16:03  caress
 * Added ELMK2XSE format.
 *
 * Revision 4.2  1999/04/21  05:45:32  caress
 * Fixed handling of bad beam index values.
 *
 * Revision 4.1  1999/04/20  06:43:57  caress
 * Fixed bug in sidescan handling.
 *
 * Revision 4.0  1999/03/31  18:29:20  caress
 * MB-System 4.6beta7
 *
 * Revision 1.1  1999/03/31  18:11:35  caress
 * Initial revision
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_io.h"
#include "../../include/mb_define.h"
#include "../../include/mbf_omghdcsj.h"
#include "../../include/mbsys_hdcs.h"

/* include for byte swapping on little-endian machines */
#ifdef BYTESWAPPED
#include "../../include/mb_swap.h"
#endif

/* essential function prototypes */
int mbr_register_omghdcsj(int verbose, char *mbio_ptr, 
		int *error);
int mbr_info_omghdcsj(int verbose, 
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
int mbr_alm_omghdcsj(int verbose, char *mbio_ptr, int *error);
int mbr_dem_omghdcsj(int verbose, char *mbio_ptr, int *error);
int mbr_rt_omghdcsj(int verbose, char *mbio_ptr, char *store_ptr, int *error);
int mbr_wt_omghdcsj(int verbose, char *mbio_ptr, char *store_ptr, int *error);

/*--------------------------------------------------------------------*/
int mbr_register_omghdcsj(int verbose, char *mbio_ptr, int *error)
{
	static char res_id[]="$Id: mbr_omghdcsj.c,v 5.2 2001-03-22 20:50:02 caress Exp $";
	char	*function_name = "mbr_register_omghdcsj";
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
	status = mbr_info_omghdcsj(verbose, 
			&mb_io_ptr->system, 
			&mb_io_ptr->beams_bath_max, 
			&mb_io_ptr->beams_amp_max, 
			&mb_io_ptr->pixels_ss_max, 
			&mb_io_ptr->format_name, 
			&mb_io_ptr->system_name, 
			&mb_io_ptr->format_description, 
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_omghdcsj;
	mb_io_ptr->mb_io_format_free = &mbr_dem_omghdcsj; 
	mb_io_ptr->mb_io_store_alloc = &mbsys_hdcs_alloc; 
	mb_io_ptr->mb_io_store_free = &mbsys_hdcs_deall; 
	mb_io_ptr->mb_io_read_ping = &mbr_rt_omghdcsj; 
	mb_io_ptr->mb_io_write_ping = &mbr_wt_omghdcsj; 
	mb_io_ptr->mb_io_extract = &mbsys_hdcs_extract; 
	mb_io_ptr->mb_io_insert = &mbsys_hdcs_insert; 
	mb_io_ptr->mb_io_extract_nav = &mbsys_hdcs_extract_nav; 
	mb_io_ptr->mb_io_insert_nav = &mbsys_hdcs_insert_nav; 
	mb_io_ptr->mb_io_extract_altitude = &mbsys_hdcs_extract_altitude; 
	mb_io_ptr->mb_io_insert_altitude = &mbsys_hdcs_insert_altitude; 
	mb_io_ptr->mb_io_extract_svp = NULL; 
	mb_io_ptr->mb_io_insert_svp = NULL; 
	mb_io_ptr->mb_io_ttimes = &mbsys_hdcs_ttimes; 
	mb_io_ptr->mb_io_copyrecord = &mbsys_hdcs_copy; 
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
int mbr_info_omghdcsj(int verbose, 
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
	static char res_id[]="$Id: mbr_omghdcsj.c,v 5.2 2001-03-22 20:50:02 caress Exp $";
	char	*function_name = "mbr_info_omghdcsj";
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
	*system = MB_SYS_HDCS;
	*beams_bath_max = 1440;
	*beams_amp_max = 1440;
	*pixels_ss_max = 1024;
	strncpy(format_name, "OMGHDCSJ", MB_NAME_LENGTH);
	strncpy(system_name, "HDCS", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_OMGHDCSJ\nInformal Description: UNB OMG HDCS format (the John Hughes Clarke format)\nAttributes:           variable beam bathymetry and\n                      amplitude, variable pixel sidescan, binary,\n		      UNB\n", MB_DESCRIPTION_LENGTH);
	*numfile = -2;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_YES;
	*traveltime = MB_YES;
	*beam_flagging = MB_YES;
	*nav_source = MB_DATA_NAV;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_ATTITUDE;
	*beamwidth_xtrack = 0.0;
	*beamwidth_ltrack = 0.0;

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
int mbr_alm_omghdcsj(int verbose, char *mbio_ptr, int *error)
{
 static char res_id[]="$Id: mbr_omghdcsj.c,v 5.2 2001-03-22 20:50:02 caress Exp $";
	char	*function_name = "mbr_alm_omghdcsj";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_omghdcsj_struct *dataplus;
	struct mbf_omghdcsj_summary_struct *summary;
	struct mbf_omghdcsj_profile_struct *profile;
	struct mbf_omghdcsj_data_struct *data;
	int	*read_summary;
	int	*fileVersion;
	int	*toolType;
	int	*profile_size;
	int	*num_beam;
	int	*beam_size;
	int	*data_size;
	int	*image_size;
	double	*pixel_size;
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

	/* set initial status */
	status = MB_SUCCESS;
	
	/* set name of possible parallel sidescan file */
	if (strlen(mb_io_ptr->file) < 248)
	    {
	    strcpy(mb_io_ptr->file2, mb_io_ptr->file);
	    strcat(mb_io_ptr->file2, ".ss_data");
	    }
	

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = sizeof(struct mbf_omghdcsj_struct);
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);

	/* get pointers */
	if (status == MB_SUCCESS)
	    {
	    dataplus = (struct mbf_omghdcsj_struct *) mb_io_ptr->raw_data;
	    summary = &(dataplus->summary);
	    profile = &(dataplus->profile);
	    data = &(dataplus->data);
	    read_summary = (int *) &mb_io_ptr->save1;
	    fileVersion = (int *) &mb_io_ptr->save2;
	    toolType = (int *) &mb_io_ptr->save3;
	    profile_size = (int *) &mb_io_ptr->save4;
	    num_beam = (int *) &mb_io_ptr->save5;
	    beam_size = (int *) &mb_io_ptr->save6;
	    data_size = (int *) &mb_io_ptr->save7;
	    image_size = (int *) &mb_io_ptr->save8;
	    pixel_size = (double *) &mb_io_ptr->saved1;
	    dataplus->buffer = NULL;
	    dataplus->kind = MB_DATA_NONE;
	    status = mb_malloc(verbose,MBF_OMGHDCSJ_SUMMARY_SIZE,
				    &dataplus->buffer,error);
	    
	    /* initialize saved values */
	    *read_summary = MB_NO;
	    *fileVersion = 0;
	    *toolType = MBF_OMGHDCSJ_None;
	    *profile_size = 0;
	    *num_beam = 0;
	    *beam_size = 0;
	    *data_size = 0;
	    *image_size = 0;
	    *pixel_size = 0.0;
	    
	    /* initialize summary values */
	    summary->sensorNumber = 1;
	    summary->subFileID = 1;
	    summary->fileVersion = 0;
	    summary->toolType = MBF_OMGHDCSJ_None;
	    summary->numProfiles = 0;
	    summary->numDepths = 0;
	    summary->timeScale = 0;
	    summary->refTime = 0;
	    summary->minTime = 0;
	    summary->maxTime = 0;
	    summary->positionType = 0;
	    summary->positionScale = 0;
	    summary->refLat = 0;
	    summary->minLat = 0;
	    summary->maxLat = 0;
	    summary->refLong = 0;
	    summary->minLong = 0;
	    summary->maxLong = 0;
	    summary->minObsDepth = 0;
	    summary->maxObsDepth = 0;
	    summary->minProcDepth = 0;
	    summary->maxProcDepth = 0;
	    summary->status = 0;
	    
	    /* initialize profile */
	    profile->status = 0;		/* status is either OK (0) 
						or no nav (1)
						or unwanted for gridding (2) */
	    profile->numDepths = 0;		/* Number of depths in profile        */
	    profile->numSamples = 0;	/* Number of sidescan samples in parallel file        */
	    profile->timeOffset = 0;	/* Time offset  wrt. header           */
	    profile->vesselLatOffset = 0;	/* Latitude offset wrt. header        */
	    profile->vesselLongOffset = 0;	/* Longitude offset wrt. header       */
	    profile->vesselHeading = 0;	/* Heading (100 nRadians)             */
	    profile->vesselHeave = 0;	/* Heave (mm)                         */
	    profile->vesselPitch = 0;	/* Vessel pitch (100 nRadians)        */
	    profile->vesselRoll = 0;	/* Vessel roll (100 nRadians)         */
	    profile->tide = 0;		/* Tide (mm)                          */
	    profile->vesselVelocity = 0;	/* Vessel Velocity (mm/s)
						note - transducer pitch is 
						generally tucked into the vel field     */
	    profile->power = 0;
	    profile->TVG = 0;
	    profile->attenuation = 0;
	    profile->edflag = 0;
	    profile->soundVelocity = 0; 	/* mm/s */
	    profile->lengthImageDataField = 0;
	    profile->pingNo = 0;
	    profile->mode = 0;  
	    profile->Q_factor = 0;  
	    profile->pulseLength = 0;   /* centisecs*/
	    profile->unassigned = 0;  
	    profile->td_sound_speed = 0;
	    profile->samp_rate = 0;
	    profile->z_res_cm = 0;
	    profile->xy_res_cm = 0;
	    profile->ssp_source = 0;
	    profile->filter_ID = 0;
	    profile->absorp_coeff = 0;
	    profile->tx_pulse_len = 0;
	    profile->tx_beam_width = 0;
	    profile->max_swath_width = 0;
	    profile->tx_power_reduction = 0;
	    profile->rx_beam_width = 0;
	    profile->rx_bandwidth = 0;
	    profile->rx_gain_reduction = 0;
	    profile->tvg_crossover = 0;
	    profile->beam_spacing = 0;
	    profile->coverage_sector = 0;
	    profile->yaw_stab_mode = 0;
    
	    /* initialize data structure */
	    data->beams = NULL;
	    data->ss_raw = NULL;
	    data->pixel_size = 0.0;
	    data->pixels_ss = 0;
	    for (i=0;i<MBF_OMGHDCSJ_MAX_PIXELS;i++)
		{
		data->ss_proc[i] = 0;
		data->ssalongtrack[i] = 0;
		}
	    dataplus->comment[0] = 0;
	    status = mbsys_hdcs_alloc(verbose,mbio_ptr,
		    &mb_io_ptr->store_data,error);
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
int mbr_dem_omghdcsj(int verbose, char *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_omghdcsj";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_omghdcsj_struct *dataplus;
	struct mbf_omghdcsj_profile_struct *profile;
	struct mbf_omghdcsj_data_struct *data;
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
	dataplus = (struct mbf_omghdcsj_struct *) mb_io_ptr->raw_data;
	profile = &(dataplus->profile);
	data = &(dataplus->data);

	/* deallocate memory for data descriptor */
	if (data->beams != NULL)
	    status = mb_free(verbose,&(data->beams),error);
	if (data->ss_raw != NULL)
	    status = mb_free(verbose,&(data->ss_raw),error);
	if (dataplus->buffer != NULL)
	    status = mb_free(verbose,&(dataplus->buffer),error);
	status = mb_free(verbose,&mb_io_ptr->raw_data,error);
	status = mbsys_hdcs_deall(verbose,mbio_ptr,
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
int mbr_rt_omghdcsj(int verbose, char *mbio_ptr, char *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_omghdcsj";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_omghdcsj_struct *dataplus;
	struct mbf_omghdcsj_summary_struct *summary;
	struct mbf_omghdcsj_profile_struct *profile;
	struct mbf_omghdcsj_data_struct *data;
	struct mbf_omghdcsj_beam_struct *beam;
	struct mbsys_hdcs_struct *store;
	struct mbsys_hdcs_beam_struct *sbeam;
	int	*read_summary;
	int	*fileVersion;
	int	*toolType;
	int	*profile_size;
	int	*num_beam;
	int	*beam_size;
	int	*data_size;
	int	*image_size;
	char	*comment;
	char	*buffer;
	int	read_size;
	int	buff_size;
	char	*char_ptr;
	short	*short_ptr;
	int	*int_ptr;
	mb_u_char   scaling_factor;
	int	ScaleFactor;
	int	MaxVal;
	int	offset, offset_start, sample_count;
	int	nrawpixels, ssrawoffset, firstgoodbeam;
	double	bathsort[MBF_OMGHDCSJ_MAX_BEAMS];
	int	nbathsort;
	double	*pixel_size, pixel_size_calc, swathwidth, xtrack, ss_spacing;
	int	pixels_ss;
	double	ss[MBF_OMGHDCSJ_MAX_PIXELS];
	double	ss_alongtrack[MBF_OMGHDCSJ_MAX_PIXELS];
	int	ss_cnt[MBF_OMGHDCSJ_MAX_PIXELS];
	int	ifix;
	int	first, last, k1, k2;
	int	i, j, k, jj, kk;

	/* compare function for qsort */
	int mb_double_compare();

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

	/* get pointer to mbio descriptor and data structure */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	store = (struct mbsys_hdcs_struct *) store_ptr;

	/* get pointer to raw data structure */
	dataplus = (struct mbf_omghdcsj_struct *) mb_io_ptr->raw_data;
	summary = &(dataplus->summary);
	profile = &(dataplus->profile);
	data = &(dataplus->data);
	comment = dataplus->comment;
	buffer = dataplus->buffer;
	read_summary = (int *) &mb_io_ptr->save1;
	fileVersion = (int *) &mb_io_ptr->save2;
	toolType = (int *) &mb_io_ptr->save3;
	profile_size = (int *) &mb_io_ptr->save4;
	num_beam = (int *) &mb_io_ptr->save5;
	beam_size = (int *) &mb_io_ptr->save6;
	data_size = (int *) &mb_io_ptr->save7;
	image_size = (int *) &mb_io_ptr->save8;
	pixel_size = (double *) &mb_io_ptr->saved1;

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;
	mb_io_ptr->file2_pos = mb_io_ptr->file2_bytes;
	
	/* read next four bytes */
	if ((read_size = fread(buffer,1,
	    4,mb_io_ptr->mbfp)) == 4)
	    {
	    mb_io_ptr->file_bytes += 4;
	    if (buffer[0] == '#'
		&& buffer[1] == '#'
		&& buffer[2] == '#'
		&& buffer[3] == '#')
		{
		dataplus->kind = MB_DATA_COMMENT;
		}
	    else if (buffer[0] == 'H'
		&& buffer[1] == 'D'
		&& buffer[2] == 'C'
		&& buffer[3] == 'S')
		{
		dataplus->kind = MB_DATA_SUMMARY;
		}
	    else
		{
		dataplus->kind = MB_DATA_DATA;
		}
	    }
	else
	    {
	    status = MB_FAILURE;
	    *error = MB_ERROR_EOF;		    
	    }
	
	/* read summary */
	if (status == MB_SUCCESS 
	    && dataplus->kind == MB_DATA_SUMMARY)
	    {
	    if ((read_size = fread(&buffer[4],1,
		MBF_OMGHDCSJ_SUMMARY_SIZE-4,mb_io_ptr->mbfp))
		!= MBF_OMGHDCSJ_SUMMARY_SIZE-4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}
	    /* parse the summary data */
	    else
		{
		mb_io_ptr->file_bytes += read_size;
		offset = 4;
#ifdef BYTESWAPPED
		int_ptr = (int *) &buffer[offset];
		summary->sensorNumber = mb_swap_int(*int_ptr); offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->subFileID = mb_swap_int(*int_ptr); offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->fileVersion = mb_swap_int(*int_ptr); offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->toolType = mb_swap_int(*int_ptr); offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->numProfiles = mb_swap_int(*int_ptr); offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->numDepths = mb_swap_int(*int_ptr); offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->timeScale = mb_swap_int(*int_ptr); offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->refTime = mb_swap_int(*int_ptr); offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->minTime = mb_swap_int(*int_ptr); offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->maxTime = mb_swap_int(*int_ptr); offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->positionType = mb_swap_int(*int_ptr); offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->positionScale = mb_swap_int(*int_ptr); offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->refLat = mb_swap_int(*int_ptr); offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->minLat = mb_swap_int(*int_ptr); offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->maxLat = mb_swap_int(*int_ptr); offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->refLong = mb_swap_int(*int_ptr); offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->minLong = mb_swap_int(*int_ptr); offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->maxLong = mb_swap_int(*int_ptr); offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->minObsDepth = mb_swap_int(*int_ptr); offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->maxObsDepth = mb_swap_int(*int_ptr); offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->minProcDepth = mb_swap_int(*int_ptr); offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->maxProcDepth = mb_swap_int(*int_ptr); offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->status = *int_ptr;
#else
		int_ptr = (int *) &buffer[offset];
		summary->sensorNumber = *int_ptr; offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->subFileID = *int_ptr; offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->fileVersion = *int_ptr; offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->toolType = *int_ptr; offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->numProfiles = *int_ptr; offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->numDepths = *int_ptr; offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->timeScale = *int_ptr; offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->refTime = *int_ptr; offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->minTime = *int_ptr; offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->maxTime = *int_ptr; offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->positionType = *int_ptr; offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->positionScale = *int_ptr; offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->refLat = *int_ptr; offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->minLat = *int_ptr; offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->maxLat = *int_ptr; offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->refLong = *int_ptr; offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->minLong = *int_ptr; offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->maxLong = *int_ptr; offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->minObsDepth = *int_ptr; offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->maxObsDepth = *int_ptr; offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->minProcDepth = *int_ptr; offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->maxProcDepth = *int_ptr; offset +=4;
		int_ptr = (int *) &buffer[offset];
		summary->status = *int_ptr;
#endif

		/* set values to saved including data record sizes */
		*read_summary = MB_YES;
		*fileVersion = summary->fileVersion;
		*toolType = summary->toolType;
		if (*fileVersion == 1)
		    {
		    *profile_size = mbf_omghdcsj_tooldefs1[*toolType][MBF_OMGHDCSJ_PROFILE_LENGTH];
		    *num_beam = mbf_omghdcsj_tooldefs1[*toolType][MBF_OMGHDCSJ_MAX_NO_BEAMS];
		    *beam_size = mbf_omghdcsj_tooldefs1[*toolType][MBF_OMGHDCSJ_BEAM_LENGTH];
		    *data_size = (*num_beam) * (*beam_size);
		    *image_size = mbf_omghdcsj_tooldefs1[*toolType][MBF_OMGHDCSJ_IMAGE_LENGTH];
		    }
		else if (*fileVersion == 2)
		    {
		    *profile_size = mbf_omghdcsj_tooldefs2[*toolType][MBF_OMGHDCSJ_PROFILE_LENGTH];
		    *num_beam = mbf_omghdcsj_tooldefs2[*toolType][MBF_OMGHDCSJ_MAX_NO_BEAMS];
		    *beam_size = mbf_omghdcsj_tooldefs2[*toolType][MBF_OMGHDCSJ_BEAM_LENGTH];
		    *data_size = (*num_beam) * (*beam_size);
		    *image_size = mbf_omghdcsj_tooldefs2[*toolType][MBF_OMGHDCSJ_IMAGE_LENGTH];
		    }
		else
		    {
		    *profile_size = mbf_omghdcsj_tooldefs3[*toolType][MBF_OMGHDCSJ_PROFILE_LENGTH];
		    *num_beam = mbf_omghdcsj_tooldefs3[*toolType][MBF_OMGHDCSJ_MAX_NO_BEAMS];
		    *beam_size = mbf_omghdcsj_tooldefs3[*toolType][MBF_OMGHDCSJ_BEAM_LENGTH];
		    *data_size = (*num_beam) * (*beam_size);
		    *image_size = mbf_omghdcsj_tooldefs3[*toolType][MBF_OMGHDCSJ_IMAGE_LENGTH];
		    }
		    
		/* allocate buffer at required size */
		if (dataplus->buffer != NULL)
		    status = mb_free(verbose,&dataplus->buffer,error);
		buff_size = MAX(*profile_size, MBF_OMGHDCSJ_SUMMARY_SIZE);
		buff_size = MAX(buff_size, *image_size);
		buff_size = MAX(buff_size, *data_size);
		status = mb_malloc(verbose,buff_size, &dataplus->buffer,error);
		if (status == MB_SUCCESS)
		    {
		    buffer = dataplus->buffer;
		    if (data->beams != NULL)
			status = mb_free(verbose,&data->beams,error);
		    if (status == MB_SUCCESS)
			status = mb_malloc(verbose,
				    *num_beam * sizeof(struct mbf_omghdcsj_beam_struct), 
				    &data->beams,error);
		    }
		}
	    }
	    
	/* or read comment */
	else if (status == MB_SUCCESS 
	    && dataplus->kind == MB_DATA_COMMENT)
	    {
	    if ((read_size = fread(dataplus->comment,1,
		MBF_OMGHDCSJ_MAX_COMMENT,mb_io_ptr->mbfp))
		!= MBF_OMGHDCSJ_MAX_COMMENT)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}
	    else
		mb_io_ptr->file_bytes += read_size;
	    }
	    
	/* or read data record */
	else if (status == MB_SUCCESS
	    && dataplus->kind == MB_DATA_DATA)
	    {
	    /* read profile */
	    if ((read_size = fread(&buffer[4],1,
		*profile_size-4,mb_io_ptr->mbfp))
		!= *profile_size-4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}
	    else
		mb_io_ptr->file_bytes += read_size;
		
	    /* now parse profile */
	    if (status == MB_SUCCESS)
		{
		offset = 0;
		if (*fileVersion == 1)
		    {
#ifdef BYTESWAPPED
		    int_ptr = (int *) &buffer[offset];
		    profile->status = mb_swap_int(*int_ptr); offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    profile->numDepths = mb_swap_int(*int_ptr); offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    profile->timeOffset = mb_swap_int(*int_ptr); offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    profile->vesselLatOffset = mb_swap_int(*int_ptr); offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    profile->vesselLongOffset = mb_swap_int(*int_ptr); offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    profile->vesselHeading = mb_swap_int(*int_ptr); offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    if (offset < *profile_size)
			{
			profile->vesselHeave = mb_swap_int(*int_ptr); offset +=4;
			int_ptr = (int *) &buffer[offset];
			profile->vesselPitch = mb_swap_int(*int_ptr); offset +=4;
			int_ptr = (int *) &buffer[offset];
			profile->vesselRoll = mb_swap_int(*int_ptr); offset +=4;
			int_ptr = (int *) &buffer[offset];
			profile->tide = mb_swap_int(*int_ptr); offset +=4;
			int_ptr = (int *) &buffer[offset];
			profile->vesselVelocity = mb_swap_int(*int_ptr); offset +=4;
			}
		    if (offset < *profile_size)
			{
			profile->power = buffer[offset]; offset +=1;
			profile->TVG = buffer[offset]; offset +=1;
			profile->attenuation = buffer[offset]; offset +=1;
			profile->edflag = buffer[offset]; offset +=1;
			int_ptr = (int *) &buffer[offset];
			profile->soundVelocity = mb_swap_int(*int_ptr); offset +=4;
			int_ptr = (int *) &buffer[offset];
			profile->lengthImageDataField = mb_swap_int(*int_ptr); offset +=4;
			int_ptr = (int *) &buffer[offset];
			profile->pingNo = mb_swap_int(*int_ptr); offset +=4;
			profile->mode = buffer[offset]; offset +=1;
			profile->Q_factor = buffer[offset]; offset +=1;
			profile->pulseLength = buffer[offset]; offset +=1;
			profile->unassigned = buffer[offset]; offset +=1;
			}
#else
		    int_ptr = (int *) &buffer[offset];
		    profile->status = *int_ptr; offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    profile->numDepths = *int_ptr; offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    profile->timeOffset = *int_ptr; offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    profile->vesselLatOffset = *int_ptr; offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    profile->vesselLongOffset = *int_ptr; offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    profile->vesselHeading = *int_ptr; offset +=4;
		    if (offset < *profile_size)
			{
			int_ptr = (int *) &buffer[offset];
			profile->vesselHeave = *int_ptr; offset +=4;
			int_ptr = (int *) &buffer[offset];
			profile->vesselPitch = *int_ptr; offset +=4;
			int_ptr = (int *) &buffer[offset];
			profile->vesselRoll = *int_ptr; offset +=4;
			int_ptr = (int *) &buffer[offset];
			profile->tide = *int_ptr; offset +=4;
			int_ptr = (int *) &buffer[offset];
			profile->vesselVelocity = *int_ptr; offset +=4;
			}
		    if (offset < *profile_size)
			{
			profile->power = buffer[offset]; offset +=1;
			profile->TVG = buffer[offset]; offset +=1;
			profile->attenuation = buffer[offset]; offset +=1;
			profile->edflag = buffer[offset]; offset +=1;
			int_ptr = (int *) &buffer[offset];
			profile->soundVelocity = *int_ptr; offset +=4;
			int_ptr = (int *) &buffer[offset];
			profile->lengthImageDataField = *int_ptr; offset +=4;
			int_ptr = (int *) &buffer[offset];
			profile->pingNo = *int_ptr; offset +=4;
			profile->mode = buffer[offset]; offset +=1;
			profile->Q_factor = buffer[offset]; offset +=1;
			profile->pulseLength = buffer[offset]; offset +=1;
			profile->unassigned = buffer[offset]; offset +=1;
			}
#endif
		    profile->numSamples = 0;
		    profile->td_sound_speed = 0;
		    profile->samp_rate = 0;
		    profile->z_res_cm = 0;
		    profile->xy_res_cm = 0;
		    profile->ssp_source = 0;
		    profile->filter_ID = 0;
		    profile->absorp_coeff = 0;
		    profile->tx_pulse_len = 0;
		    profile->tx_beam_width = 0;
		    profile->max_swath_width = 0;
		    profile->tx_power_reduction = 0;
		    profile->rx_beam_width = 0;
		    profile->rx_bandwidth = 0;
		    profile->rx_gain_reduction = 0;
		    profile->tvg_crossover = 0;
		    profile->beam_spacing = 0;
		    profile->coverage_sector = 0;
		    profile->yaw_stab_mode = 0;
		    }
		else if (*fileVersion == 2)
		    {
#ifdef BYTESWAPPED
		    int_ptr = (int *) &buffer[offset];
		    profile->timeOffset = mb_swap_int(*int_ptr); offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    profile->vesselLatOffset = mb_swap_int(*int_ptr); offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    profile->vesselLongOffset = mb_swap_int(*int_ptr); offset +=4;
		    short_ptr = (short *) &buffer[offset];
		    profile->vesselHeading = 10000 * (mb_swap_short(*short_ptr)); offset +=2;
		    short_ptr = (short *) &buffer[offset];
		    profile->vesselHeave = mb_swap_short(*short_ptr); offset +=2;
		    short_ptr = (short *) &buffer[offset];
		    profile->vesselPitch = 1000 * (mb_swap_short(*short_ptr)); offset +=2;
		    short_ptr = (short *) &buffer[offset];
		    profile->vesselRoll = 1000 * (mb_swap_short(*short_ptr)); offset +=2;
		    short_ptr = (short *) &buffer[offset];
		    profile->tide = mb_swap_short(*short_ptr); offset +=2;
		    short_ptr = (short *) &buffer[offset];
		    profile->numDepths = mb_swap_short(*short_ptr); offset +=2;
		    profile->power = buffer[offset]; offset +=1;
		    profile->TVG = buffer[offset]; offset +=1;
		    profile->attenuation = buffer[offset]; offset +=1;
		    profile->pulseLength = buffer[offset]; offset +=1;
		    profile->mode = buffer[offset]; offset +=1;
		    profile->status = buffer[offset]; offset +=1;
		    profile->edflag = buffer[offset]; offset +=1;
		    profile->unassigned = buffer[offset]; offset +=1;
#else
		    int_ptr = (int *) &buffer[offset];
		    profile->timeOffset = *int_ptr; offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    profile->vesselLatOffset = *int_ptr; offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    profile->vesselLongOffset = *int_ptr; offset +=4;
		    short_ptr = (short *) &buffer[offset];
		    profile->vesselHeading = 10000 * (*short_ptr); offset +=2;
		    short_ptr = (short *) &buffer[offset];
		    profile->vesselHeave = *short_ptr; offset +=2;
		    short_ptr = (short *) &buffer[offset];
		    profile->vesselPitch = 1000 * (*short_ptr); offset +=2;
		    short_ptr = (short *) &buffer[offset];
		    profile->vesselRoll = 1000 * (*short_ptr); offset +=2;
		    short_ptr = (short *) &buffer[offset];
		    profile->tide = *short_ptr; offset +=2;
		    short_ptr = (short *) &buffer[offset];
		    profile->numDepths = *short_ptr; offset +=2;
		    profile->power = buffer[offset]; offset +=1;
		    profile->TVG = buffer[offset]; offset +=1;
		    profile->attenuation = buffer[offset]; offset +=1;
		    profile->pulseLength = buffer[offset]; offset +=1;
		    profile->mode = buffer[offset]; offset +=1;
		    profile->status = buffer[offset]; offset +=1;
		    profile->edflag = buffer[offset]; offset +=1;
		    profile->unassigned = buffer[offset]; offset +=1;
#endif
		    profile->numSamples = 0;
		    profile->soundVelocity = 0;
		    profile->lengthImageDataField = 0;
		    profile->pingNo = 0;
		    profile->Q_factor = 0;
		    profile->td_sound_speed = 0;
		    profile->samp_rate = 0;
		    profile->z_res_cm = 0;
		    profile->xy_res_cm = 0;
		    profile->ssp_source = 0;
		    profile->filter_ID = 0;
		    profile->absorp_coeff = 0;
		    profile->tx_pulse_len = 0;
		    profile->tx_beam_width = 0;
		    profile->max_swath_width = 0;
		    profile->tx_power_reduction = 0;
		    profile->rx_beam_width = 0;
		    profile->rx_bandwidth = 0;
		    profile->rx_gain_reduction = 0;
		    profile->tvg_crossover = 0;
		    profile->beam_spacing = 0;
		    profile->coverage_sector = 0;
		    profile->yaw_stab_mode = 0;
		    }
		else if (*fileVersion == 3)
		    {
#ifdef BYTESWAPPED
		    int_ptr = (int *) &buffer[offset];
		    profile->timeOffset = mb_swap_int(*int_ptr); offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    profile->vesselLatOffset = mb_swap_int(*int_ptr); offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    profile->vesselLongOffset = mb_swap_int(*int_ptr); offset +=4;
		    short_ptr = (short *) &buffer[offset];
		    profile->vesselHeading = 10000 * (mb_swap_short(*short_ptr)); offset +=2;
		    short_ptr = (short *) &buffer[offset];
		    profile->vesselHeave = mb_swap_short(*short_ptr); offset +=2;
		    short_ptr = (short *) &buffer[offset];
		    profile->vesselPitch = 1000 * (mb_swap_short(*short_ptr)); offset +=2;
		    short_ptr = (short *) &buffer[offset];
		    profile->vesselRoll = 1000 * (mb_swap_short(*short_ptr)); offset +=2;
		    short_ptr = (short *) &buffer[offset];
		    profile->tide = mb_swap_short(*short_ptr); offset +=2;
		    short_ptr = (short *) &buffer[offset];
		    profile->numDepths = mb_swap_short(*short_ptr); offset +=2;
		    profile->power = buffer[offset]; offset +=1;
		    profile->TVG = buffer[offset]; offset +=1;
		    profile->attenuation = buffer[offset]; offset +=1;
		    profile->pulseLength = buffer[offset]; offset +=1;
		    profile->mode = buffer[offset]; offset +=1;
		    profile->status = buffer[offset]; offset +=1;
		    profile->edflag = buffer[offset]; offset +=1;
		    profile->unassigned = buffer[offset]; offset +=1;
		    if (offset < *profile_size)
			{
			short_ptr = (short *) &buffer[offset];
			profile->td_sound_speed = mb_swap_short(*short_ptr); offset +=2;
			short_ptr = (short *) &buffer[offset];
			profile->samp_rate = mb_swap_short(*short_ptr); offset +=2;
			profile->z_res_cm = buffer[offset]; offset +=1;
			profile->xy_res_cm = buffer[offset]; offset +=1;
			profile->ssp_source = buffer[offset]; offset +=1;
			profile->filter_ID = buffer[offset]; offset +=1;
			short_ptr = (short *) &buffer[offset];
			profile->absorp_coeff = mb_swap_short(*short_ptr); offset +=2;
			short_ptr = (short *) &buffer[offset];
			profile->tx_pulse_len = mb_swap_short(*short_ptr); offset +=2;
			short_ptr = (short *) &buffer[offset];
			profile->tx_beam_width = mb_swap_short(*short_ptr); offset +=2;
			short_ptr = (short *) &buffer[offset];
			profile->max_swath_width = mb_swap_short(*short_ptr); offset +=2;
			profile->tx_power_reduction = buffer[offset]; offset +=1;
			profile->rx_beam_width = buffer[offset]; offset +=1;
			profile->rx_bandwidth = buffer[offset]; offset +=1;
			profile->rx_gain_reduction = buffer[offset]; offset +=1;
			profile->tvg_crossover = buffer[offset]; offset +=1;
			profile->beam_spacing = buffer[offset]; offset +=1;
			profile->coverage_sector = buffer[offset]; offset +=1;
			profile->yaw_stab_mode = buffer[offset]; offset +=1;
			}
#else
		    int_ptr = (int *) &buffer[offset];
		    profile->timeOffset = *int_ptr; offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    profile->vesselLatOffset = *int_ptr; offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    profile->vesselLongOffset = *int_ptr; offset +=4;
		    short_ptr = (short *) &buffer[offset];
		    profile->vesselHeading = 10000 * (*short_ptr); offset +=2;
		    short_ptr = (short *) &buffer[offset];
		    profile->vesselHeave = *short_ptr; offset +=2;
		    short_ptr = (short *) &buffer[offset];
		    profile->vesselPitch = 1000 * (*short_ptr); offset +=2;
		    short_ptr = (short *) &buffer[offset];
		    profile->vesselRoll = 1000 * (*short_ptr); offset +=2;
		    short_ptr = (short *) &buffer[offset];
		    profile->tide = *short_ptr; offset +=2;
		    short_ptr = (short *) &buffer[offset];
		    profile->numDepths = *short_ptr; offset +=2;
		    profile->power = buffer[offset]; offset +=1;
		    profile->TVG = buffer[offset]; offset +=1;
		    profile->attenuation = buffer[offset]; offset +=1;
		    profile->pulseLength = buffer[offset]; offset +=1;
		    profile->mode = buffer[offset]; offset +=1;
		    profile->status = buffer[offset]; offset +=1;
		    profile->edflag = buffer[offset]; offset +=1;
		    profile->unassigned = buffer[offset]; offset +=1;
		    if (offset < *profile_size)
			{
			short_ptr = (short *) &buffer[offset];
			profile->td_sound_speed = *short_ptr; offset +=2;
			short_ptr = (short *) &buffer[offset];
			profile->samp_rate = *short_ptr; offset +=2;
			profile->z_res_cm = buffer[offset]; offset +=1;
			profile->xy_res_cm = buffer[offset]; offset +=1;
			profile->ssp_source = buffer[offset]; offset +=1;
			profile->filter_ID = buffer[offset]; offset +=1;
			short_ptr = (short *) &buffer[offset];
			profile->absorp_coeff = *short_ptr; offset +=2;
			short_ptr = (short *) &buffer[offset];
			profile->tx_pulse_len = *short_ptr; offset +=2;
			short_ptr = (short *) &buffer[offset];
			profile->tx_beam_width = *short_ptr; offset +=2;
			short_ptr = (short *) &buffer[offset];
			profile->max_swath_width = *short_ptr; offset +=2;
			profile->tx_power_reduction = buffer[offset]; offset +=1;
			profile->rx_beam_width = buffer[offset]; offset +=1;
			profile->rx_bandwidth = buffer[offset]; offset +=1;
			profile->rx_gain_reduction = buffer[offset]; offset +=1;
			profile->tvg_crossover = buffer[offset]; offset +=1;
			profile->beam_spacing = buffer[offset]; offset +=1;
			profile->coverage_sector = buffer[offset]; offset +=1;
			profile->yaw_stab_mode = buffer[offset]; offset +=1;
			}
#endif
		    profile->numSamples = 0;
		    profile->soundVelocity = 0;
		    profile->lengthImageDataField = 0;
		    profile->pingNo = 0;
		    profile->Q_factor = 0;
		    }		
		}
		
	    /* now read next data */
	    if (status == MB_SUCCESS)
		{
		if ((read_size = fread(buffer,1,
		    *data_size,mb_io_ptr->mbfp))
		    != *data_size)
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_EOF;		    
		    }
		else
		    mb_io_ptr->file_bytes += read_size;
		}
	    
	    /* now parse data */
	    if (status == MB_SUCCESS)
		{
		offset = 0;
		for (i=0;i<profile->numDepths;i++)
		    {
		    offset_start = offset;
		    beam = &data->beams[i];
		    if (*fileVersion == 1)
			{
#ifdef BYTESWAPPED
			int_ptr = (int *) &buffer[offset];
			beam->status = mb_swap_int(*int_ptr); offset+=4;
			int_ptr = (int *) &buffer[offset];
			beam->observedDepth = mb_swap_int(*int_ptr); offset+=4;
			if ((offset - offset_start) < *beam_size)
			    {
			    int_ptr = (int *) &buffer[offset];
			    beam->acrossTrack = mb_swap_int(*int_ptr); offset+=4;
			    int_ptr = (int *) &buffer[offset];
			    beam->alongTrack = mb_swap_int(*int_ptr); offset+=4;
			    int_ptr = (int *) &buffer[offset];
			    beam->latOffset = mb_swap_int(*int_ptr); offset+=4;
			    int_ptr = (int *) &buffer[offset];
			    beam->longOffset = mb_swap_int(*int_ptr); offset+=4;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    int_ptr = (int *) &buffer[offset];
			    beam->processedDepth = mb_swap_int(*int_ptr); offset+=4;
			    int_ptr = (int *) &buffer[offset];
			    beam->timeOffset = mb_swap_int(*int_ptr); offset+=4;
			    int_ptr = (int *) &buffer[offset];
			    beam->depthAccuracy = mb_swap_int(*int_ptr); offset+=4;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    beam->reflectivity = buffer[offset]; offset+=1;
			    beam->Q_factor = buffer[offset]; offset+=1;
			    beam->beam_no = buffer[offset]; offset+=1;
			    beam->freq = buffer[offset]; offset+=1;
			    beam->calibratedBackscatter = buffer[offset]; offset+=1;
			    beam->mindB = buffer[offset]; offset+=1;
			    beam->maxdB = buffer[offset]; offset+=1;
			    beam->pseudoAngleIndependentBackscatter = buffer[offset]; offset+=1;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    int_ptr = (int *) &buffer[offset];
			    beam->range = mb_swap_int(*int_ptr); offset+=4;
			    int_ptr = (int *) &buffer[offset];
			    beam->no_samples = mb_swap_int(*int_ptr); offset+=4;
			    int_ptr = (int *) &buffer[offset];
			    beam->offset = mb_swap_int(*int_ptr); offset+=4;
			    int_ptr = (int *) &buffer[offset];
			    beam->centre_no = mb_swap_int(*int_ptr); offset+=4;
			    beam->sample_unit = buffer[offset]; offset+=1;
			    beam->sample_interval = buffer[offset]; offset+=1;
			    beam->dummy[0] = buffer[offset]; offset+=1;
			    beam->dummy[1] = buffer[offset]; offset+=1;
			    }
#else
			int_ptr = (int *) &buffer[offset];
			beam->status = *int_ptr; offset+=4;
			int_ptr = (int *) &buffer[offset];
			beam->observedDepth = *int_ptr; offset+=4;
			if ((offset - offset_start) < *beam_size)
			    {
			    int_ptr = (int *) &buffer[offset];
			    beam->acrossTrack = *int_ptr; offset+=4;
			    int_ptr = (int *) &buffer[offset];
			    beam->alongTrack = *int_ptr; offset+=4;
			    int_ptr = (int *) &buffer[offset];
			    beam->latOffset = *int_ptr; offset+=4;
			    int_ptr = (int *) &buffer[offset];
			    beam->longOffset = *int_ptr; offset+=4;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    int_ptr = (int *) &buffer[offset];
			    beam->processedDepth = *int_ptr; offset+=4;
			    int_ptr = (int *) &buffer[offset];
			    beam->timeOffset = *int_ptr; offset+=4;
			    int_ptr = (int *) &buffer[offset];
			    beam->depthAccuracy = *int_ptr; offset+=4;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    beam->reflectivity = buffer[offset]; offset+=1;
			    beam->Q_factor = buffer[offset]; offset+=1;
			    beam->beam_no = buffer[offset]; offset+=1;
			    beam->freq = buffer[offset]; offset+=1;
			    beam->calibratedBackscatter = buffer[offset]; offset+=1;
			    beam->mindB = buffer[offset]; offset+=1;
			    beam->maxdB = buffer[offset]; offset+=1;
			    beam->pseudoAngleIndependentBackscatter = buffer[offset]; offset+=1;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    int_ptr = (int *) &buffer[offset];
			    beam->range = *int_ptr; offset+=4;
			    int_ptr = (int *) &buffer[offset];
			    beam->no_samples = *int_ptr; offset+=4;
			    int_ptr = (int *) &buffer[offset];
			    beam->offset = *int_ptr; offset+=4;
			    int_ptr = (int *) &buffer[offset];
			    beam->centre_no = *int_ptr; offset+=4;
			    beam->sample_unit = buffer[offset]; offset+=1;
			    beam->sample_interval = buffer[offset]; offset+=1;
			    beam->dummy[0] = buffer[offset]; offset+=1;
			    beam->dummy[1] = buffer[offset]; offset+=1;
			    }
#endif
			beam->samp_win_length = 0;
			beam->beam_depress_angle = 0;
			beam->beam_heading_angle = 0;
			}
		    else if (*fileVersion == 2)
			{
#ifdef BYTESWAPPED
			if ((offset - offset_start) < *beam_size)
			    {
			    short_ptr = (short *) &buffer[offset];
			    beam->observedDepth = mb_swap_short(*short_ptr); offset+=2;
			    short_ptr = (short *) &buffer[offset];
			    beam->acrossTrack = mb_swap_short(*short_ptr); offset+=2;
			    beam->status = buffer[offset]; offset+=1;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    beam->reflectivity = buffer[offset]; offset+=1;
			    beam->calibratedBackscatter = buffer[offset]; offset+=1;
			    beam->pseudoAngleIndependentBackscatter = buffer[offset]; offset+=1;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    short_ptr = (short *) &buffer[offset];
			    beam->alongTrack = mb_swap_short(*short_ptr); offset+=2;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    short_ptr = (short *) &buffer[offset];
			    beam->range = mb_swap_short(*short_ptr); offset+=2;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    int_ptr = (int *) &buffer[offset];
			    beam->offset = mb_swap_int(*int_ptr); offset+=4;
			    short_ptr = (short *) &buffer[offset];
			    beam->no_samples = mb_swap_short(*short_ptr); offset+=2;
			    short_ptr = (short *) &buffer[offset];
			    beam->centre_no = mb_swap_short(*short_ptr); offset+=2;
			    }
#else
			if ((offset - offset_start) < *beam_size)
			    {
			    short_ptr = (short *) &buffer[offset];
			    beam->observedDepth = *short_ptr; offset+=2;
			    short_ptr = (short *) &buffer[offset];
			    beam->acrossTrack = *short_ptr; offset+=2;
			    beam->status = buffer[offset]; offset+=1;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    beam->reflectivity = buffer[offset]; offset+=1;
			    beam->calibratedBackscatter = buffer[offset]; offset+=1;
			    beam->pseudoAngleIndependentBackscatter = buffer[offset]; offset+=1;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    short_ptr = (short *) &buffer[offset];
			    beam->alongTrack = *short_ptr; offset+=2;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    short_ptr = (short *) &buffer[offset];
			    beam->range = *short_ptr; offset+=2;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    int_ptr = (int *) &buffer[offset];
			    beam->offset = *int_ptr; offset+=4;
			    short_ptr = (short *) &buffer[offset];
			    beam->no_samples = *short_ptr; offset+=2;
			    short_ptr = (short *) &buffer[offset];
			    beam->centre_no = *short_ptr; offset+=2;
			    }
#endif
			beam->latOffset = 0;
			beam->longOffset = 0;
			beam->processedDepth = 0;
			beam->timeOffset = 0;
			beam->depthAccuracy = 0;
			beam->reflectivity = 0;
			beam->Q_factor = 0;
			beam->beam_no = 0;
			beam->freq = 0;
			beam->mindB = 0;
			beam->maxdB = 0;
			beam->sample_unit = 0;
			beam->sample_interval = 0;
			beam->dummy[0] = 0;
			beam->dummy[1] = 0;
			beam->samp_win_length = 0;
			beam->beam_depress_angle = 0;
			beam->beam_heading_angle = 0;
			if(beam->alongTrack < -13000) 
			    {
			    ScaleFactor = 1;
			    beam->alongTrack +=20000;
			    } 
			else if(beam->alongTrack < -5000) 
			    {
			    ScaleFactor = 10;
			    beam->alongTrack +=10000;
			    } 
			else if(beam->alongTrack < 5000) 
			    {
			    ScaleFactor = 100;
			    beam->alongTrack +=0000;
			    } 
			else if(beam->alongTrack < 15000)
			    {
			    ScaleFactor = 1000;
			    beam->alongTrack -=10000;
			    }
			beam->observedDepth = beam->observedDepth * ScaleFactor;	
			beam->acrossTrack = beam->acrossTrack * ScaleFactor;	
			beam->alongTrack = beam->alongTrack * ScaleFactor;	
			beam->Q_factor = beam->reflectivity;
			}
		    else if (*fileVersion == 3)
			{
#ifdef BYTESWAPPED
			if ((offset - offset_start) < *beam_size)
			    {
			    short_ptr = (short *) &buffer[offset];
			    beam->observedDepth = mb_swap_short(*short_ptr); offset+=2;
			    short_ptr = (short *) &buffer[offset];
			    beam->acrossTrack = mb_swap_short(*short_ptr); offset+=2;
			    beam->status = buffer[offset]; offset+=1;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    beam->reflectivity = buffer[offset]; offset+=1;
			    beam->calibratedBackscatter = buffer[offset]; offset+=1;
			    beam->pseudoAngleIndependentBackscatter = buffer[offset]; offset+=1;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    short_ptr = (short *) &buffer[offset];
			    beam->alongTrack = mb_swap_short(*short_ptr); offset+=2;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    short_ptr = (short *) &buffer[offset];
			    beam->range = mb_swap_short(*short_ptr); offset+=2;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    int_ptr = (int *) &buffer[offset];
			    beam->offset = mb_swap_int(*int_ptr); offset+=4;
			    short_ptr = (short *) &buffer[offset];
			    beam->no_samples = mb_swap_short(*short_ptr); offset+=2;
			    short_ptr = (short *) &buffer[offset];
			    beam->centre_no = mb_swap_short(*short_ptr); offset+=2;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    short_ptr = (short *) &buffer[offset];
			    beam->beam_depress_angle = mb_swap_short(*short_ptr); offset+=2;
			    short_ptr = (short *) &buffer[offset];
			    beam->beam_heading_angle = mb_swap_short(*short_ptr); offset+=2;
			    beam->samp_win_length = buffer[offset]; offset+=1;
			    scaling_factor = buffer[offset]; offset+=1;
			    beam->Q_factor = buffer[offset]; offset+=1;
			    offset+=1;
			    }
#else
			if ((offset - offset_start) < *beam_size)
			    {
			    short_ptr = (short *) &buffer[offset];
			    beam->observedDepth = *short_ptr; offset+=2;
			    short_ptr = (short *) &buffer[offset];
			    beam->acrossTrack = *short_ptr; offset+=2;
			    beam->status = buffer[offset]; offset+=1;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    beam->reflectivity = buffer[offset]; offset+=1;
			    beam->calibratedBackscatter = buffer[offset]; offset+=1;
			    beam->pseudoAngleIndependentBackscatter = buffer[offset]; offset+=1;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    short_ptr = (short *) &buffer[offset];
			    beam->alongTrack = *short_ptr; offset+=2;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    short_ptr = (short *) &buffer[offset];
			    beam->range = *short_ptr; offset+=2;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    int_ptr = (int *) &buffer[offset];
			    beam->offset = *int_ptr; offset+=4;
			    short_ptr = (short *) &buffer[offset];
			    beam->no_samples = *short_ptr; offset+=2;
			    short_ptr = (short *) &buffer[offset];
			    beam->centre_no = *short_ptr; offset+=2;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    short_ptr = (short *) &buffer[offset];
			    beam->beam_depress_angle = *short_ptr; offset+=2;
			    short_ptr = (short *) &buffer[offset];
			    beam->beam_heading_angle = *short_ptr; offset+=2;
			    beam->samp_win_length = buffer[offset]; offset+=1;
			    scaling_factor = buffer[offset]; offset+=1;
			    beam->Q_factor = buffer[offset]; offset+=1;
			    offset+=1;
			    }
#endif
			beam->latOffset = 0;
			beam->longOffset = 0;
			beam->processedDepth = 0;
			beam->timeOffset = 0;
			beam->depthAccuracy = 0;
			beam->reflectivity = 0;
			beam->beam_no = 0;
			beam->freq = 0;
			beam->mindB = 0;
			beam->maxdB = 0;
			beam->sample_unit = 0;
			beam->sample_interval = 0;
			beam->dummy[0] = 0;
			beam->dummy[1] = 0;
			
			/* get scaling factor
			    - if maximum of abs(acrossTrack) and abs(observedDepth)
				    less than     32m, 1mm res,
				    less than     64m, 2mm res,
				    less than    128m, 4mm res,
				    less than  4,096m, 12.8cm res   
				    less than 40,960m, 1.28m res */
			ScaleFactor = pow(2.0,(double)scaling_factor);
			beam->observedDepth = beam->observedDepth * ScaleFactor;	
			beam->acrossTrack = beam->acrossTrack * ScaleFactor;	
			beam->alongTrack = beam->alongTrack * ScaleFactor;	
			}
		    }
		}

	    /* now deal with sidescan in parallel file */
	    if (status == MB_SUCCESS 
		&& mb_io_ptr->mbfp2 != NULL
		&& (summary->toolType == MBF_OMGHDCSJ_EM1000
		    || summary->toolType == MBF_OMGHDCSJ_EM12_single
		    || summary->toolType == MBF_OMGHDCSJ_EM12_dual
		    || summary->toolType == MBF_OMGHDCSJ_EM300
		    || summary->toolType == MBF_OMGHDCSJ_EM3000
		    || summary->toolType == MBF_OMGHDCSJ_EM3000D
		    || summary->toolType == MBF_OMGHDCSJ_EM121A))
		{
		/* count samples and get first offset */
		nrawpixels = 0;
		ssrawoffset = 0;
		firstgoodbeam = MB_YES;
		for (i=0;i<profile->numDepths;i++)
		    {
		    beam = &data->beams[i];
		    if (beam->no_samples > 0)
			{
			nrawpixels += beam->no_samples;
			if (firstgoodbeam == MB_YES)
			    {
			    ssrawoffset = beam->offset;
			    firstgoodbeam = MB_NO;
			    }
			}
		    }
		    
		/* allocate memory if required */
		if (*image_size < nrawpixels
		    || data->ss_raw == NULL)
		    {
		    *image_size = nrawpixels;
		    if (data->ss_raw != NULL)
			status = mb_free(verbose,&data->ss_raw,error);
		    status = mb_malloc(verbose,
				    *image_size, 
				    &data->ss_raw,error);
		    }
		    
		/* read the sidescan */
		if (status == MB_SUCCESS)
		    {
		    /* first read spare bytes if any */
		    if (ssrawoffset > mb_io_ptr->file2_bytes)
			{
			for (i=mb_io_ptr->file2_bytes;i<ssrawoffset;i++)
			    {
			    if ((read_size = fread(data->ss_raw,1,
				1,mb_io_ptr->mbfp2)) != 1)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;		    
				}
			    else
				{
				mb_io_ptr->file2_bytes += read_size;
				}
			    }
			}
		    
		    /* now read the real data */
		    if ((read_size = fread(data->ss_raw,1,
			nrawpixels,mb_io_ptr->mbfp2))
			!= nrawpixels)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;		    
			}
		    else
			{
			mb_io_ptr->file2_bytes += read_size;
			profile->numSamples = nrawpixels;
			}
		    }
		}
	    }

	/* set kind and error in mb_io_ptr */
	mb_io_ptr->new_kind = dataplus->kind;
	mb_io_ptr->new_error = *error;

	/* print debug statements */
	if (verbose >= 5 && status == MB_FAILURE)
	    {
	    fprintf(stderr,"\ndbg5  Read failure in function <%s>\n",
		    function_name);
	    fprintf(stderr,"dbg5       status:     %d\n",status);
	    fprintf(stderr,"dbg5       error:      %d\n",*error);
	    }

	/* print debug statements */
	else if (verbose >= 5 && dataplus->kind == MB_DATA_SUMMARY)
	    {
	    fprintf(stderr,"\ndbg5  Summary read in function <%s>\n",
		    function_name);
	    fprintf(stderr,"dbg5       kind:                   %d\n",dataplus->kind);
	    fprintf(stderr,"dbg5       sensorNumber:           %d\n",summary->sensorNumber);
	    fprintf(stderr,"dbg5       subFileID:              %d\n",summary->subFileID);
	    fprintf(stderr,"dbg5       fileVersion:            %d\n",summary->fileVersion);
	    fprintf(stderr,"dbg5       toolType:               %d\n",summary->toolType);
	    fprintf(stderr,"dbg5       numProfiles:            %d\n",summary->numProfiles);
	    fprintf(stderr,"dbg5       numDepths:              %d\n",summary->numDepths);
	    fprintf(stderr,"dbg5       timeScale:              %d\n",summary->timeScale);
	    fprintf(stderr,"dbg5       refTime:                %d\n",summary->refTime);
	    fprintf(stderr,"dbg5       minTime:                %d\n",summary->minTime);
	    fprintf(stderr,"dbg5       maxTime:                %d\n",summary->maxTime);
	    fprintf(stderr,"dbg5       positionType:           %d\n",summary->positionType);
	    fprintf(stderr,"dbg5       positionScale:          %d\n",summary->positionScale);
	    fprintf(stderr,"dbg5       refLat:                 %d\n",summary->refLat);
	    fprintf(stderr,"dbg5       minLat:                 %d\n",summary->minLat);
	    fprintf(stderr,"dbg5       maxLat:                 %d\n",summary->maxLat);
	    fprintf(stderr,"dbg5       refLong:                %d\n",summary->refLong);
	    fprintf(stderr,"dbg5       minLong:                %d\n",summary->minLong);
	    fprintf(stderr,"dbg5       maxLong:                %d\n",summary->maxLong);
	    fprintf(stderr,"dbg5       minObsDepth:            %d\n",summary->minObsDepth);
	    fprintf(stderr,"dbg5       maxObsDepth:            %d\n",summary->maxObsDepth);
	    fprintf(stderr,"dbg5       minProcDepth:           %d\n",summary->minProcDepth);
	    fprintf(stderr,"dbg5       maxProcDepth:           %d\n",summary->maxProcDepth);
	    fprintf(stderr,"dbg5       status:                 %d\n",summary->status);
	    fprintf(stderr,"dbg5       status:                 %d\n",status);
	    fprintf(stderr,"dbg5       error:                  %d\n",*error);
	    }

	/* print debug statements */
	else if (verbose >= 5 && dataplus->kind == MB_DATA_COMMENT)
	    {
	    fprintf(stderr,"\ndbg5  New comment read in function <%s>\n",
		    function_name);
	    fprintf(stderr,"dbg5       kind:                   %d\n",dataplus->kind);
	    fprintf(stderr,"dbg5       comment:                %s\n",dataplus->comment);
	    fprintf(stderr,"dbg5       status:                 %d\n",status);
	    fprintf(stderr,"dbg5       error:                  %d\n",*error);
	    }

	/* print debug statements */
	else if (verbose >= 5 && dataplus->kind == MB_DATA_DATA)
	    {
	    fprintf(stderr,"\ndbg5  New profile read in function <%s>\n",
		    function_name);
	    fprintf(stderr,"dbg5       kind:                   %d\n",dataplus->kind);
	    fprintf(stderr,"dbg5       status:                 %d\n",profile->status);
	    fprintf(stderr,"dbg5       numDepths:              %d\n",profile->numDepths);
	    fprintf(stderr,"dbg5       numSamples:             %d\n",profile->numSamples);
	    fprintf(stderr,"dbg5       timeOffset:             %d\n",profile->timeOffset);
	    fprintf(stderr,"dbg5       vesselLatOffset:        %d\n",profile->vesselLatOffset);
	    fprintf(stderr,"dbg5       vesselLongOffset:       %d\n",profile->vesselLongOffset);
	    fprintf(stderr,"dbg5       vesselHeading:          %d\n",profile->vesselHeading);
	    fprintf(stderr,"dbg5       vesselHeave:            %d\n",profile->vesselHeave);
	    fprintf(stderr,"dbg5       vesselPitch:            %d\n",profile->vesselPitch);
	    fprintf(stderr,"dbg5       vesselRoll:             %d\n",profile->vesselRoll);
	    fprintf(stderr,"dbg5       tide:                   %d\n",profile->tide);
	    fprintf(stderr,"dbg5       vesselVelocity:         %d\n",profile->vesselVelocity);
	    fprintf(stderr,"dbg5       power:                  %d\n",profile->power);
	    fprintf(stderr,"dbg5       TVG:                    %d\n",profile->TVG);
	    fprintf(stderr,"dbg5       attenuation:            %d\n",profile->attenuation);
	    fprintf(stderr,"dbg5       edflag:                 %d\n",profile->edflag);
	    fprintf(stderr,"dbg5       soundVelocity:          %d\n",profile->soundVelocity);
	    fprintf(stderr,"dbg5       lengthImageDataField:   %d\n",profile->lengthImageDataField);
	    fprintf(stderr,"dbg5       pingNo:                 %d\n",profile->pingNo);
	    fprintf(stderr,"dbg5       mode:                   %d\n",profile->mode);
	    fprintf(stderr,"dbg5       Q_factor:               %d\n",profile->Q_factor);
	    fprintf(stderr,"dbg5       pulseLength:            %d\n",profile->pulseLength);
	    fprintf(stderr,"dbg5       unassigned:             %d\n",profile->unassigned);
	    fprintf(stderr,"dbg5       td_sound_speed:         %d\n",profile->td_sound_speed);
	    fprintf(stderr,"dbg5       samp_rate:              %d\n",profile->samp_rate);
	    fprintf(stderr,"dbg5       z_res_cm:               %d\n",profile->z_res_cm);
	    fprintf(stderr,"dbg5       xy_res_cm:              %d\n",profile->xy_res_cm);
	    fprintf(stderr,"dbg5       ssp_source:             %d\n",profile->ssp_source);
	    fprintf(stderr,"dbg5       filter_ID:              %d\n",profile->filter_ID);
	    fprintf(stderr,"dbg5       absorp_coeff:           %d\n",profile->absorp_coeff);
	    fprintf(stderr,"dbg5       tx_pulse_len:           %d\n",profile->tx_pulse_len);
	    fprintf(stderr,"dbg5       tx_beam_width:          %d\n",profile->tx_beam_width);
	    fprintf(stderr,"dbg5       max_swath_width:        %d\n",profile->max_swath_width);
	    fprintf(stderr,"dbg5       tx_power_reduction:     %d\n",profile->tx_power_reduction);
	    fprintf(stderr,"dbg5       rx_beam_width:          %d\n",profile->rx_beam_width);
	    fprintf(stderr,"dbg5       rx_bandwidth:           %d\n",profile->rx_bandwidth);
	    fprintf(stderr,"dbg5       rx_gain_reduction:      %d\n",profile->rx_gain_reduction);
	    fprintf(stderr,"dbg5       tvg_crossover:          %d\n",profile->tvg_crossover);
	    fprintf(stderr,"dbg5       beam_spacing:           %d\n",profile->beam_spacing);
	    fprintf(stderr,"dbg5       coverage_sector:        %d\n",profile->coverage_sector);
	    fprintf(stderr,"dbg5       yaw_stab_mode:          %d\n",profile->yaw_stab_mode);
	    fprintf(stderr,"dbg5       status:                 %d\n",status);
	    fprintf(stderr,"dbg5       error:                  %d\n",*error);
	    for (i=0;i<profile->numDepths;i++)
		{
		beam = &data->beams[i];
		fprintf(stderr,"dbg5       status[%4d]:            %d\n",
			     i, beam->status);
		fprintf(stderr,"dbg5       observedDepth[%4d]:     %d\n",
			     i, beam->observedDepth);
		fprintf(stderr,"dbg5       acrossTrack[%4d]:       %d\n",
			     i, beam->acrossTrack);
		fprintf(stderr,"dbg5       alongTrack[%4d]:        %d\n",
			     i, beam->alongTrack);
		fprintf(stderr,"dbg5       latOffset[%4d]:         %d\n",
			     i, beam->latOffset);
		fprintf(stderr,"dbg5       longOffset[%4d]:        %d\n",
			     i, beam->longOffset);
		fprintf(stderr,"dbg5       processedDepth[%4d]:    %d\n",
			     i, beam->processedDepth);
		fprintf(stderr,"dbg5       timeOffset[%4d]:        %d\n",
			     i, beam->timeOffset);
		fprintf(stderr,"dbg5       depthAccuracy[%4d]:     %d\n",
			     i, beam->depthAccuracy);
		fprintf(stderr,"dbg5       reflectivity[%4d]:      %d\n",
			     i, beam->reflectivity);
		fprintf(stderr,"dbg5       Q_factor[%4d]:          %d\n",
			     i, beam->Q_factor);
		fprintf(stderr,"dbg5       beam_no[%4d]:           %d\n",
			     i, beam->beam_no);
		fprintf(stderr,"dbg5       freq[%4d]:              %d\n",
			     i, beam->freq);
		fprintf(stderr,"dbg5       calibBackscatter[%4d]:  %d\n",
			     i, beam->calibratedBackscatter);
		fprintf(stderr,"dbg5       mindB[%4d]:             %d\n",
			     i, beam->mindB);
		fprintf(stderr,"dbg5       maxdB[%4d]:             %d\n",
			     i, beam->maxdB);
		fprintf(stderr,"dbg5       AngleIndepBacks[%4d]:   %d\n",
			     i, beam->pseudoAngleIndependentBackscatter);
		fprintf(stderr,"dbg5       range[%4d]:             %d\n",
			     i, beam->range);
		fprintf(stderr,"dbg5       no_samples[%4d]:        %d\n",
			     i, beam->no_samples);
		fprintf(stderr,"dbg5       offset[%4d]:            %d\n",
			     i, beam->offset);
		fprintf(stderr,"dbg5       centre_no[%4d]:         %d\n",
			     i, beam->centre_no);
		fprintf(stderr,"dbg5       sample_unit[%4d]:       %d\n",
			     i, beam->sample_unit);
		fprintf(stderr,"dbg5       sample_interval[%4d]:   %d\n",
			     i, beam->sample_interval);
		fprintf(stderr,"dbg5       dummy0[%4d]:            %d\n",
			     i, beam->dummy[0]);
		fprintf(stderr,"dbg5       dummy1[%4d]:            %d\n",
			     i, beam->dummy[1]);
		fprintf(stderr,"dbg5       samp_win_length[%4d]:   %d\n",
			     i, beam->samp_win_length);
		fprintf(stderr,"dbg5       beam_depress_angle[%4d]:%d\n",
			     i, beam->beam_depress_angle);
		fprintf(stderr,"dbg5       beam_heading_angle[%4d]:%d\n",
			     i, beam->beam_heading_angle);
		}
	    for (i=0;i<profile->numSamples;i++)
		fprintf(stderr,"dbg5       sidescan sample[%4d]:%d\n",
			     i, data->ss_raw[i]);
	    fprintf(stderr,"dbg5       status:     %d\n",status);
	    fprintf(stderr,"dbg5       error:      %d\n",*error);
	    }
	
	/* process sidescan */
	if (status == MB_SUCCESS 
		&& dataplus->kind == MB_DATA_DATA)
	    {

	    /* read depth and beam location values into storage arrays */
	    nbathsort = 0;
	    swathwidth = 0.0;
	    for (i=0;i<profile->numDepths;i++)
		{
		beam = &data->beams[i];
		if (beam->observedDepth != 0
		    && (beam->status == 0 || beam->status == 22))
		    {
		    bathsort[nbathsort] = 0.001 * (abs(beam->observedDepth)
							- profile->tide);
		    nbathsort++;
		    swathwidth = MAX(swathwidth, 2.5 + 90.0 - 0.01 * beam->beam_depress_angle);
		    }
		}
		
	    /* process raw sidescan if available */
	    if (profile->numSamples <= 0
		|| profile->numSamples > 10000)
		{
		pixels_ss = 0;
		}
	    else
		{
		/* zero arrays */
		for (k=0;k<MBF_OMGHDCSJ_MAX_PIXELS;k++)
		    {
		    ss[k] = 0.0;
		    ss_alongtrack[k] = 0.0;
		    ss_cnt[k] = 0;
		    }

		/* get median depth and sidescan pixel size */
		qsort((char *)bathsort, nbathsort, sizeof(double),mb_double_compare);
		pixel_size_calc = 2 * tan(DTR * swathwidth) * bathsort[nbathsort/2] 
				    / MBF_OMGHDCSJ_MAX_PIXELS;
		pixel_size_calc = MAX(pixel_size_calc, bathsort[nbathsort/2] * sin(DTR * 0.1));
		if (*pixel_size <= 0.0)
		    *pixel_size = pixel_size_calc;
		else if (0.95 * (*pixel_size) > pixel_size_calc)
		    *pixel_size = 0.95 * (*pixel_size);
		else if (1.05 * (*pixel_size) < pixel_size_calc)
		    *pixel_size = 1.05 * (*pixel_size);
		else
		    *pixel_size = pixel_size_calc;
    
		/* get raw pixel size */
		if (profile->samp_rate > 0)
		    ss_spacing = 750.0 / profile->samp_rate;		
		else if (summary->toolType == MBF_OMGHDCSJ_EM3000 
			|| summary->toolType == MBF_OMGHDCSJ_EM3000D)
		    ss_spacing = 750.0 / 14000;
		else if (summary->toolType == MBF_OMGHDCSJ_EM300)
		    ss_spacing = 750.0 / 4512;
		else if (summary->toolType == MBF_OMGHDCSJ_EM1000
			|| summary->toolType == MBF_OMGHDCSJ_EM12_single
			|| summary->toolType == MBF_OMGHDCSJ_EM12_dual
			|| summary->toolType == MBF_OMGHDCSJ_EM121A)
		    {
		    if (profile->power == 1)
			    ss_spacing = 0.6;
		    else if (profile->power == 2)
			    ss_spacing = 2.4;
		    else if (profile->power == 3)
			    ss_spacing = 0.3;
		    else if (profile->power == 4)
			    ss_spacing = 0.3;
		    else
			    ss_spacing = 0.15;
		    }
		    
		/* loop over raw sidescan, putting each raw pixel into
			the binning arrays */
		offset_start = -1;
		sample_count = 0;
		pixels_ss = MBF_OMGHDCSJ_MAX_PIXELS;
		for (i=0;i<profile->numDepths;i++)
		    {
		    beam = &data->beams[i];
		    if (beam->observedDepth != 0
			&& (beam->status == 0 || beam->status == 22))
			{
			if (offset_start == -1
				&& beam->no_samples > 0)
			    offset_start = beam->offset;
			else if ((beam->offset - offset_start)
			    > sample_count)
			    offset_start = beam->offset - sample_count;
			else if (beam->offset <= 0 && offset_start > 0)
			    beam->offset = offset_start + sample_count;
			sample_count += beam->no_samples;
/*fprintf(stderr, "i:%d flag:%d samples:%d sample_cnt:%d offset:%d offset_start:%d\n", 
i, mb_io_ptr->new_beamflag[i], beam->no_samples, 
sample_count, beam->offset, offset_start);
*/
			for (j=0;j<beam->no_samples;j++)
			    {
			    jj = j + beam->offset - offset_start;

			    /* interpolate based on range */
			    xtrack = 0.001 * beam->acrossTrack
				+ ss_spacing * (j - abs(beam->centre_no));

			    k = MBF_OMGHDCSJ_MAX_PIXELS / 2 
				+ (int)(xtrack / *pixel_size);
			    if (beam->status == 0
				&& k > 0 && k < MBF_OMGHDCSJ_MAX_PIXELS)
				{
				ss[k] += 0.5*((double)data->ss_raw[jj]) + 64.0;
				ss_alongtrack[k] += 0.001 * beam->alongTrack;
				ss_cnt[k]++;
				}
			    }
			}
		    }
			
		/* average the sidescan */
		first = MBF_OMGHDCSJ_MAX_PIXELS;
		last = -1;
		for (k=0;k<MBF_OMGHDCSJ_MAX_PIXELS;k++)
			{
			if (ss_cnt[k] > 0)
				{
				ss[k] /= ss_cnt[k];
				ss_alongtrack[k] /= ss_cnt[k];
				first = MIN(first, k);
				last = k;
				}
			}
			
		/* interpolate the sidescan */
		k1 = first;
		k2 = first;
		for (k=first+1;k<last;k++)
		    {
		    if (ss_cnt[k] <= 0)
			{
			if (k2 <= k)
			    {
			    k2 = k+1;
			    while (ss_cnt[k2] <= 0 && k2 < last)
				k2++;
			    }
			ss[k] = ss[k1]
			    + (ss[k2] - ss[k1])
				* ((double)(k - k1)) / ((double)(k2 - k1));
			ss_alongtrack[k] = ss_alongtrack[k1]
			    + (ss_alongtrack[k2] - ss_alongtrack[k1])
				* ((double)(k - k1)) / ((double)(k2 - k1));
			}
		    else
			{
			k1 = k;
			}
		    }
		}

	    /* done processing sidescan */

	    }

	/* translate values to data storage structure */
	if (status == MB_SUCCESS
		&& store != NULL)
	    {
	    /* type of data record */
	    store->kind = dataplus->kind;
	    store->read_summary = *read_summary;
	    store->fileVersion = *fileVersion;
	    store->toolType = *toolType;
	    store->profile_size = *profile_size;
	    store->num_beam = *num_beam;
	    store->beam_size = *beam_size;
	    store->data_size = *data_size;
	    store->image_size = *image_size;
    	    
	    if (dataplus->kind == MB_DATA_SUMMARY
		|| dataplus->kind == MB_DATA_DATA)
		{    
		/* summary values */
		store->sensorNumber = summary->sensorNumber;
		store->subFileID = summary->subFileID;
		store->fileVersion = summary->fileVersion;
		store->toolType = summary->toolType;
		store->numProfiles = summary->numProfiles;
		store->numDepths_sum = summary->numDepths;
		store->timeScale = summary->timeScale;
		store->refTime = summary->refTime;
		store->minTime = summary->minTime;
		store->maxTime = summary->maxTime;
		store->positionType = summary->positionType;
		store->positionScale = summary->positionScale;
		store->refLat = summary->refLat;
		store->minLat = summary->minLat;
		store->maxLat = summary->maxLat;
		store->refLong = summary->refLong;
		store->minLong = summary->minLong;
		store->maxLong = summary->maxLong;
		store->minObsDepth = summary->minObsDepth;
		store->maxObsDepth = summary->maxObsDepth;
		store->minProcDepth = summary->minProcDepth;
		store->maxProcDepth = summary->maxProcDepth;
		store->status_sum = summary->status;
		}
   	    
	    if (dataplus->kind == MB_DATA_DATA)
		{    
		/* profile values */
		store->status_pro = profile->status;
		store->numDepths_pro = profile->numDepths;
		store->timeOffset = profile->timeOffset;
		store->vesselLatOffset = profile->vesselLatOffset;
		store->vesselLongOffset = profile->vesselLongOffset;
		store->vesselHeading = profile->vesselHeading;
		store->vesselHeave = profile->vesselHeave;
		store->vesselPitch = profile->vesselPitch;
		store->vesselRoll = profile->vesselRoll;
		store->tide = profile->tide;
		store->vesselVelocity = profile->vesselVelocity;
		store->power = profile->power;
		store->TVG = profile->TVG;
		store->attenuation = profile->attenuation;
		store->edflag = profile->edflag;
		store->soundVelocity = profile->soundVelocity;
		store->lengthImageDataField = profile->lengthImageDataField;
		store->pingNo = profile->pingNo;
		store->mode = profile->mode;
		store->Q_factor = profile->Q_factor;
		store->pulseLength = profile->pulseLength;
		store->unassigned = profile->unassigned;
		store->td_sound_speed = profile->td_sound_speed;
		store->samp_rate = profile->samp_rate;
		store->z_res_cm = profile->z_res_cm;
		store->xy_res_cm = profile->xy_res_cm;
		store->ssp_source = profile->ssp_source;
		store->filter_ID = profile->filter_ID;
		store->absorp_coeff = profile->absorp_coeff;
		store->tx_pulse_len = profile->tx_pulse_len;
		store->tx_beam_width = profile->tx_beam_width;
		store->max_swath_width = profile->max_swath_width;
		store->tx_power_reduction = profile->tx_power_reduction;
		store->rx_beam_width = profile->rx_beam_width;
		store->rx_bandwidth = profile->rx_bandwidth;
		store->rx_gain_reduction = profile->rx_gain_reduction;
		store->tvg_crossover = profile->tvg_crossover;
		store->beam_spacing = profile->beam_spacing;
		store->coverage_sector = profile->coverage_sector;
		store->yaw_stab_mode = profile->yaw_stab_mode;
		
		/* beams */
		if (store->beams != NULL)
		    status = mb_free(verbose,&store->beams,error);
		status = mb_malloc(verbose,
				*num_beam * sizeof(struct mbsys_hdcs_beam_struct), 
				&store->beams,error);
		if (status == MB_SUCCESS)
		    {
		    for (i=0;i<profile->numDepths;i++)
			{
			beam = &data->beams[i];
			sbeam = &store->beams[i];
			
			sbeam->status = beam->status;
			sbeam->observedDepth = beam->observedDepth;
			sbeam->acrossTrack = beam->acrossTrack;
			sbeam->alongTrack = beam->alongTrack;
			sbeam->latOffset = beam->latOffset;
			sbeam->longOffset = beam->longOffset;
			sbeam->processedDepth = beam->processedDepth;
			sbeam->timeOffset = beam->timeOffset;
			sbeam->depthAccuracy = beam->depthAccuracy;
			sbeam->reflectivity = beam->reflectivity;  
			sbeam->Q_factor = beam->Q_factor;
			sbeam->beam_no = beam->beam_no;
			sbeam->freq = beam->freq;
			sbeam->calibratedBackscatter = beam->calibratedBackscatter;
			sbeam->mindB = beam->mindB;		
			sbeam->maxdB = beam->maxdB;
			sbeam->pseudoAngleIndependentBackscatter = beam->pseudoAngleIndependentBackscatter;
			sbeam->range = beam->range;
			sbeam->no_samples = beam->no_samples;
			sbeam->offset = beam->offset;
			sbeam->centre_no = beam->centre_no;
			sbeam->sample_unit = beam->sample_unit;
			sbeam->sample_interval = beam->sample_interval;
			sbeam->dummy[0] = beam->dummy[0];
			sbeam->dummy[1] = beam->dummy[1];
			sbeam->samp_win_length = beam->samp_win_length;
			sbeam->beam_depress_angle = beam->beam_depress_angle;
			sbeam->beam_heading_angle = beam->beam_heading_angle;
			}
		    }
		
		/* raw sidescan */
		if (profile->numSamples > 0
		    && store->numSamples < profile->numSamples
		    && store->ss_raw != NULL)
		    status = mb_free(verbose,&store->ss_raw,error);
		if (profile->numSamples > 0
		    && data->ss_raw != NULL
		    && store->ss_raw == NULL)
		    {
		    status = mb_malloc(verbose,
				    profile->numSamples, 
				    &store->ss_raw,error);
		    }
		if (status == MB_SUCCESS
		    && profile->numSamples > 0
		    && data->ss_raw != NULL
		    && store->ss_raw != NULL)
		    {
		    store->numSamples = profile->numSamples;
		    for (i=0;i<profile->numSamples;i++)
			{
			store->ss_raw[i] = data->ss_raw[i];
			}
		    }
		    
		/* processed sidescan */
		if (pixels_ss == MBF_OMGHDCSJ_MAX_PIXELS)
		    {
		    store->pixels_ss = pixels_ss;
		    store->pixel_size = 1000 * (*pixel_size);
		    for (i=0;i<store->pixels_ss;i++)
			{
			store->ss_proc[i] = ss[i];
			store->ssalongtrack[i] 
				= (short)(1000 * ss_alongtrack[i]);
			}
		    }
		}

	    if (dataplus->kind == MB_DATA_COMMENT)
		{
		/* comment */
		for (i=0;i<MBF_OMGHDCSJ_MAX_COMMENT;i++)
		    store->comment[i] = dataplus->comment[i];
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
int mbr_wt_omghdcsj(int verbose, char *mbio_ptr, char *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_omghdcsj";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_omghdcsj_struct *dataplus;
	struct mbf_omghdcsj_summary_struct *summary;
	struct mbf_omghdcsj_profile_struct *profile;
	struct mbf_omghdcsj_data_struct *data;
	struct mbf_omghdcsj_beam_struct *beam;
	struct mbsys_hdcs_struct *store;
	struct mbsys_hdcs_beam_struct *sbeam;
	int	*write_summary;
	int	*fileVersion;
	int	*toolType;
	int	*profile_size;
	int	*num_beam;
	int	*beam_size;
	int	*data_size;
	int	*image_size;
	double	*pixel_size;
	char	*comment;
	char	*buffer;
	int	write_size;
	int	buff_size;
	char	*char_ptr;
	short	*short_ptr;
	int	*int_ptr;
	mb_u_char   scaling_factor;
	int	ScaleFactor;
	int	MaxVal;
	int	offset, offset_start;
	double	xtrack, ss_spacing;
	int	i, j, jj, k;

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

	/* get pointer to mbio descriptor and data structure */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	store = (struct mbsys_hdcs_struct *) store_ptr;

	/* get pointer to raw data structure */
	dataplus = (struct mbf_omghdcsj_struct *) mb_io_ptr->raw_data;
	summary = &(dataplus->summary);
	profile = &(dataplus->profile);
	data = &(dataplus->data);
	comment = dataplus->comment;
	buffer = dataplus->buffer;
	write_summary = (int *) &mb_io_ptr->save1;
	fileVersion = (int *) &mb_io_ptr->save2;
	toolType = (int *) &mb_io_ptr->save3;
	profile_size = (int *) &mb_io_ptr->save4;
	num_beam = (int *) &mb_io_ptr->save5;
	beam_size = (int *) &mb_io_ptr->save6;
	data_size = (int *) &mb_io_ptr->save7;
	image_size = (int *) &mb_io_ptr->save8;
	pixel_size = (double *) &mb_io_ptr->saved1;
	
	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;
	mb_io_ptr->file2_pos = mb_io_ptr->file2_bytes;

	/* first translate values from data storage structure */
	if (store != NULL)
	    {
	    /* type of data record */
	    dataplus->kind = store->kind;
	    *write_summary = store->read_summary;
	    *fileVersion = store->fileVersion;
	    *toolType = store->toolType;
	    *profile_size = store->profile_size;
	    *num_beam = store->num_beam;
	    *beam_size = store->beam_size;
	    *data_size = store->data_size;
	    *image_size = store->image_size;
	    
	    if (dataplus->kind == MB_DATA_SUMMARY
		|| dataplus->kind == MB_DATA_DATA)
		{    
		/* summary values */
		summary->sensorNumber = store->sensorNumber;
		summary->subFileID = store->subFileID;
		summary->fileVersion = store->fileVersion;
		summary->toolType = store->toolType;
		summary->numProfiles = store->numProfiles;
		summary->numDepths = store->numDepths_sum;
		summary->timeScale = store->timeScale;
		summary->refTime = store->refTime;
		summary->minTime = store->minTime;
		summary->maxTime = store->maxTime;
		summary->positionType = store->positionType;
		summary->positionScale = store->positionScale;
		summary->refLat = store->refLat;
		summary->minLat = store->minLat;
		summary->maxLat = store->maxLat;
		summary->refLong = store->refLong;
		summary->minLong = store->minLong;
		summary->maxLong = store->maxLong;
		summary->minObsDepth = store->minObsDepth;
		summary->maxObsDepth = store->maxObsDepth;
		summary->minProcDepth = store->minProcDepth;
		summary->maxProcDepth = store->maxProcDepth;
		summary->status = store->status_sum;
		}
	    
	    if (dataplus->kind == MB_DATA_SUMMARY)
		{    
		/* set values to be saved including data record sizes */
		*write_summary = MB_YES;
		*fileVersion = summary->fileVersion;
		*toolType = summary->toolType;
		if (*fileVersion == 1)
		    {
		    *profile_size = mbf_omghdcsj_tooldefs1[*toolType][MBF_OMGHDCSJ_PROFILE_LENGTH];
		    *num_beam = mbf_omghdcsj_tooldefs1[*toolType][MBF_OMGHDCSJ_MAX_NO_BEAMS];
		    *beam_size = mbf_omghdcsj_tooldefs1[*toolType][MBF_OMGHDCSJ_BEAM_LENGTH];
		    *data_size = (*num_beam) * (*beam_size);
		    *image_size = mbf_omghdcsj_tooldefs1[*toolType][MBF_OMGHDCSJ_IMAGE_LENGTH];
		    }
		else if (*fileVersion == 2)
		    {
		    *profile_size = mbf_omghdcsj_tooldefs2[*toolType][MBF_OMGHDCSJ_PROFILE_LENGTH];
		    *num_beam = mbf_omghdcsj_tooldefs2[*toolType][MBF_OMGHDCSJ_MAX_NO_BEAMS];
		    *beam_size = mbf_omghdcsj_tooldefs2[*toolType][MBF_OMGHDCSJ_BEAM_LENGTH];
		    *data_size = (*num_beam) * (*beam_size);
		    *image_size = mbf_omghdcsj_tooldefs2[*toolType][MBF_OMGHDCSJ_IMAGE_LENGTH];
		    }
		else
		    {
		    *profile_size = mbf_omghdcsj_tooldefs3[*toolType][MBF_OMGHDCSJ_PROFILE_LENGTH];
		    *num_beam = mbf_omghdcsj_tooldefs3[*toolType][MBF_OMGHDCSJ_MAX_NO_BEAMS];
		    *beam_size = mbf_omghdcsj_tooldefs3[*toolType][MBF_OMGHDCSJ_BEAM_LENGTH];
		    *data_size = (*num_beam) * (*beam_size);
		    *image_size = mbf_omghdcsj_tooldefs3[*toolType][MBF_OMGHDCSJ_IMAGE_LENGTH];
		    }
		    
		/* allocate buffer at required size */
		if (dataplus->buffer != NULL)
		    status = mb_free(verbose,&dataplus->buffer,error);
		buff_size = MAX(*profile_size, MBF_OMGHDCSJ_SUMMARY_SIZE);
		buff_size = MAX(buff_size, *image_size);
		buff_size = MAX(buff_size, *data_size);
		status = mb_malloc(verbose,buff_size, &dataplus->buffer,error);
		if (status == MB_SUCCESS)
		    {
		    buffer = dataplus->buffer;
		    if (data->beams != NULL)
			status = mb_free(verbose,&data->beams,error);
		    if (status == MB_SUCCESS)
			status = mb_malloc(verbose,
				*num_beam * sizeof(struct mbf_omghdcsj_beam_struct), 
				&data->beams,error);
		    }
		}
		
	    if (dataplus->kind == MB_DATA_DATA)
		{    
		/* profile values */
		profile->status = store->status_pro;
		profile->numDepths = store->numDepths_pro;
		profile->timeOffset = store->timeOffset;
		profile->vesselLatOffset = store->vesselLatOffset;
		profile->vesselLongOffset = store->vesselLongOffset;
		profile->vesselHeading = store->vesselHeading;
		profile->vesselHeave = store->vesselHeave;
		profile->vesselPitch = store->vesselPitch;
		profile->vesselRoll = store->vesselRoll;
		profile->tide = store->tide;
		profile->vesselVelocity = store->vesselVelocity;
		profile->power = store->power;
		profile->TVG = store->TVG;
		profile->attenuation = store->attenuation;
		profile->edflag = store->edflag;
		profile->soundVelocity = store->soundVelocity;
		profile->lengthImageDataField = store->lengthImageDataField;
		profile->pingNo = store->pingNo;
		profile->mode = store->mode;
		profile->Q_factor = store->Q_factor;
		profile->pulseLength = store->pulseLength;
		profile->unassigned = store->unassigned;
		profile->td_sound_speed = store->td_sound_speed;
		profile->samp_rate = store->samp_rate;
		profile->z_res_cm = store->z_res_cm;
		profile->xy_res_cm = store->xy_res_cm;
		profile->ssp_source = store->ssp_source;
		profile->filter_ID = store->filter_ID;
		profile->absorp_coeff = store->absorp_coeff;
		profile->tx_pulse_len = store->tx_pulse_len;
		profile->tx_beam_width = store->tx_beam_width;
		profile->max_swath_width = store->max_swath_width;
		profile->tx_power_reduction = store->tx_power_reduction;
		profile->rx_beam_width = store->rx_beam_width;
		profile->rx_bandwidth = store->rx_bandwidth;
		profile->rx_gain_reduction = store->rx_gain_reduction;
		profile->tvg_crossover = store->tvg_crossover;
		profile->beam_spacing = store->beam_spacing;
		profile->coverage_sector = store->coverage_sector;
		profile->yaw_stab_mode = store->yaw_stab_mode;
		
		/* beams */
		if (data->beams == NULL)
		    {
		    status = mb_malloc(verbose,
				*num_beam * sizeof(struct mbf_omghdcsj_beam_struct), 
				&data->beams,error);
		    }
		if (status == MB_SUCCESS)
		    {
		    for (i=0;i<store->numDepths_pro;i++)
			{
			beam = &data->beams[i];
			sbeam = &store->beams[i];
			
			beam->status = sbeam->status;
			beam->observedDepth = sbeam->observedDepth;
			beam->acrossTrack = sbeam->acrossTrack;
			beam->alongTrack = sbeam->alongTrack;
			beam->latOffset = sbeam->latOffset;
			beam->longOffset = sbeam->longOffset;
			beam->processedDepth = sbeam->processedDepth;
			beam->timeOffset = sbeam->timeOffset;
			beam->depthAccuracy = sbeam->depthAccuracy;
			beam->reflectivity = sbeam->reflectivity;
			beam->Q_factor = sbeam->Q_factor;
			beam->beam_no = sbeam->beam_no;
			beam->freq = sbeam->freq;
			beam->calibratedBackscatter = sbeam->calibratedBackscatter;
			beam->mindB = sbeam->mindB;
			beam->maxdB = sbeam->maxdB;
			beam->pseudoAngleIndependentBackscatter = sbeam->pseudoAngleIndependentBackscatter;
			beam->range = sbeam->range;
			beam->no_samples = sbeam->no_samples;
			beam->offset = sbeam->offset;
			beam->centre_no = sbeam->centre_no;
			beam->sample_unit = sbeam->sample_unit;
			beam->sample_interval = sbeam->sample_interval;
			beam->dummy[0] = sbeam->dummy[0];
			beam->dummy[1] = sbeam->dummy[1];
			beam->samp_win_length = sbeam->samp_win_length;
			beam->beam_depress_angle = sbeam->beam_depress_angle;
			beam->beam_heading_angle = sbeam->beam_heading_angle;
			}
		    for (i=store->numDepths_pro;i<store->num_beam;i++)
			{
			beam = &data->beams[i];
			sbeam = &store->beams[i];
			
			beam->status = 0;
			beam->observedDepth = 0;
			beam->acrossTrack = 0;
			beam->alongTrack = 0;
			beam->latOffset = 0;
			beam->longOffset = 0;
			beam->processedDepth = 0;
			beam->timeOffset = 0;
			beam->depthAccuracy = 0;
			beam->reflectivity = 0;
			beam->Q_factor = 0;
			beam->beam_no = 0;
			beam->freq = 0;
			beam->calibratedBackscatter = 0;
			beam->mindB = 0;
			beam->maxdB = 0;
			beam->pseudoAngleIndependentBackscatter = 0;
			beam->range = 0;
			beam->no_samples = 0;
			beam->offset = 0;
			beam->centre_no = 0;
			beam->sample_unit = 0;
			beam->sample_interval = 0;
			beam->dummy[0] = 0;
			beam->dummy[1] = 0;
			beam->samp_win_length = 0;
			beam->beam_depress_angle = 0;
			beam->beam_heading_angle = 0;
			}
		    }
		
		/* sidescan */
		if (store->numSamples > 0
		    && profile->numSamples < store->numSamples
		    && data->ss_raw != NULL)
		    status = mb_free(verbose,&data->ss_raw,error);
		if (store->numSamples > 0
		    && store->ss_raw != NULL)
		    {
		    status = mb_malloc(verbose,
				    store->numSamples, 
				    &data->ss_raw,error);
		    if (status == MB_SUCCESS)
			{
			profile->numSamples = store->numSamples;
			for (i=0;i<store->numSamples;i++)
			    {
			    data->ss_raw[i] = store->ss_raw[i];
			    }
			}
		    }
		}
		
	    if (dataplus->kind == MB_DATA_COMMENT)
		{    
		/* comment */
		for (i=0;i<MBF_OMGHDCSJ_MAX_COMMENT;i++)
		    dataplus->comment[i] = store->comment[i];
		}
	    }

	/* reset offsets in raw sidescan data */
	if (status == MB_SUCCESS
	    && profile->numSamples > 0)
	    {
	    offset = mb_io_ptr->file2_bytes;
	    for (i=0;i<profile->numDepths;i++)
		{
		beam = &data->beams[i];
		beam->offset = offset;
		offset += beam->no_samples;
		}
	    }

	/* print debug statements */
	if (verbose >= 5 && (dataplus->kind == MB_DATA_SUMMARY 
	    || dataplus->kind == MB_DATA_DATA))
	    {
	    fprintf(stderr,"\ndbg5  Summary set in function <%s>\n",
		    function_name);
	    fprintf(stderr,"dbg5       kind:                   %d\n",dataplus->kind);
	    fprintf(stderr,"dbg5       sensorNumber:           %d\n",summary->sensorNumber);
	    fprintf(stderr,"dbg5       subFileID:              %d\n",summary->subFileID);
	    fprintf(stderr,"dbg5       fileVersion:            %d\n",summary->fileVersion);
	    fprintf(stderr,"dbg5       toolType:               %d\n",summary->toolType);
	    fprintf(stderr,"dbg5       numProfiles:            %d\n",summary->numProfiles);
	    fprintf(stderr,"dbg5       numDepths:              %d\n",summary->numDepths);
	    fprintf(stderr,"dbg5       timeScale:              %d\n",summary->timeScale);
	    fprintf(stderr,"dbg5       refTime:                %d\n",summary->refTime);
	    fprintf(stderr,"dbg5       minTime:                %d\n",summary->minTime);
	    fprintf(stderr,"dbg5       maxTime:                %d\n",summary->maxTime);
	    fprintf(stderr,"dbg5       positionType:           %d\n",summary->positionType);
	    fprintf(stderr,"dbg5       positionScale:          %d\n",summary->positionScale);
	    fprintf(stderr,"dbg5       refLat:                 %d\n",summary->refLat);
	    fprintf(stderr,"dbg5       minLat:                 %d\n",summary->minLat);
	    fprintf(stderr,"dbg5       maxLat:                 %d\n",summary->maxLat);
	    fprintf(stderr,"dbg5       refLong:                %d\n",summary->refLong);
	    fprintf(stderr,"dbg5       minLong:                %d\n",summary->minLong);
	    fprintf(stderr,"dbg5       maxLong:                %d\n",summary->maxLong);
	    fprintf(stderr,"dbg5       minObsDepth:            %d\n",summary->minObsDepth);
	    fprintf(stderr,"dbg5       maxObsDepth:            %d\n",summary->maxObsDepth);
	    fprintf(stderr,"dbg5       minProcDepth:           %d\n",summary->minProcDepth);
	    fprintf(stderr,"dbg5       maxProcDepth:           %d\n",summary->maxProcDepth);
	    fprintf(stderr,"dbg5       status:                 %d\n",summary->status);
	    fprintf(stderr,"dbg5       status:                 %d\n",status);
	    fprintf(stderr,"dbg5       error:                  %d\n",*error);
	    }

	/* print debug statements */
	if (verbose >= 5 && dataplus->kind == MB_DATA_DATA)
	    {
	    fprintf(stderr,"\ndbg5  New profile read in function <%s>\n",
		    function_name);
	    fprintf(stderr,"dbg5       kind:                   %d\n",dataplus->kind);
	    fprintf(stderr,"dbg5       status:                 %d\n",profile->status);
	    fprintf(stderr,"dbg5       numDepths:              %d\n",profile->numDepths);
	    fprintf(stderr,"dbg5       numSamples:             %d\n",profile->numSamples);
	    fprintf(stderr,"dbg5       timeOffset:             %d\n",profile->timeOffset);
	    fprintf(stderr,"dbg5       vesselLatOffset:        %d\n",profile->vesselLatOffset);
	    fprintf(stderr,"dbg5       vesselLongOffset:       %d\n",profile->vesselLongOffset);
	    fprintf(stderr,"dbg5       vesselHeading:          %d\n",profile->vesselHeading);
	    fprintf(stderr,"dbg5       vesselHeave:            %d\n",profile->vesselHeave);
	    fprintf(stderr,"dbg5       vesselPitch:            %d\n",profile->vesselPitch);
	    fprintf(stderr,"dbg5       vesselRoll:             %d\n",profile->vesselRoll);
	    fprintf(stderr,"dbg5       tide:                   %d\n",profile->tide);
	    fprintf(stderr,"dbg5       vesselVelocity:         %d\n",profile->vesselVelocity);
	    fprintf(stderr,"dbg5       power:                  %d\n",profile->power);
	    fprintf(stderr,"dbg5       TVG:                    %d\n",profile->TVG);
	    fprintf(stderr,"dbg5       attenuation:            %d\n",profile->attenuation);
	    fprintf(stderr,"dbg5       edflag:                 %d\n",profile->edflag);
	    fprintf(stderr,"dbg5       soundVelocity:          %d\n",profile->soundVelocity);
	    fprintf(stderr,"dbg5       lengthImageDataField:   %d\n",profile->lengthImageDataField);
	    fprintf(stderr,"dbg5       pingNo:                 %d\n",profile->pingNo);
	    fprintf(stderr,"dbg5       mode:                   %d\n",profile->mode);
	    fprintf(stderr,"dbg5       Q_factor:               %d\n",profile->Q_factor);
	    fprintf(stderr,"dbg5       pulseLength:            %d\n",profile->pulseLength);
	    fprintf(stderr,"dbg5       unassigned:             %d\n",profile->unassigned);
	    fprintf(stderr,"dbg5       td_sound_speed:         %d\n",profile->td_sound_speed);
	    fprintf(stderr,"dbg5       samp_rate:              %d\n",profile->samp_rate);
	    fprintf(stderr,"dbg5       z_res_cm:               %d\n",profile->z_res_cm);
	    fprintf(stderr,"dbg5       xy_res_cm:              %d\n",profile->xy_res_cm);
	    fprintf(stderr,"dbg5       ssp_source:             %d\n",profile->ssp_source);
	    fprintf(stderr,"dbg5       filter_ID:              %d\n",profile->filter_ID);
	    fprintf(stderr,"dbg5       absorp_coeff:           %d\n",profile->absorp_coeff);
	    fprintf(stderr,"dbg5       tx_pulse_len:           %d\n",profile->tx_pulse_len);
	    fprintf(stderr,"dbg5       tx_beam_width:          %d\n",profile->tx_beam_width);
	    fprintf(stderr,"dbg5       max_swath_width:        %d\n",profile->max_swath_width);
	    fprintf(stderr,"dbg5       tx_power_reduction:     %d\n",profile->tx_power_reduction);
	    fprintf(stderr,"dbg5       rx_beam_width:          %d\n",profile->rx_beam_width);
	    fprintf(stderr,"dbg5       rx_bandwidth:           %d\n",profile->rx_bandwidth);
	    fprintf(stderr,"dbg5       rx_gain_reduction:      %d\n",profile->rx_gain_reduction);
	    fprintf(stderr,"dbg5       tvg_crossover:          %d\n",profile->tvg_crossover);
	    fprintf(stderr,"dbg5       beam_spacing:           %d\n",profile->beam_spacing);
	    fprintf(stderr,"dbg5       coverage_sector:        %d\n",profile->coverage_sector);
	    fprintf(stderr,"dbg5       yaw_stab_mode:          %d\n",profile->yaw_stab_mode);
	    fprintf(stderr,"dbg5       status:                 %d\n",status);
	    fprintf(stderr,"dbg5       error:                  %d\n",*error);
	    for (i=0;i<profile->numDepths;i++)
		{
		beam = &data->beams[i];
		fprintf(stderr,"dbg5       status[%4d]:            %d\n",
			     i, beam->status);
		fprintf(stderr,"dbg5       observedDepth[%4d]:     %d\n",
			     i, beam->observedDepth);
		fprintf(stderr,"dbg5       acrossTrack[%4d]:       %d\n",
			     i, beam->acrossTrack);
		fprintf(stderr,"dbg5       alongTrack[%4d]:        %d\n",
			     i, beam->alongTrack);
		fprintf(stderr,"dbg5       latOffset[%4d]:         %d\n",
			     i, beam->latOffset);
		fprintf(stderr,"dbg5       longOffset[%4d]:        %d\n",
			     i, beam->longOffset);
		fprintf(stderr,"dbg5       processedDepth[%4d]:    %d\n",
			     i, beam->processedDepth);
		fprintf(stderr,"dbg5       timeOffset[%4d]:        %d\n",
			     i, beam->timeOffset);
		fprintf(stderr,"dbg5       depthAccuracy[%4d]:     %d\n",
			     i, beam->depthAccuracy);
		fprintf(stderr,"dbg5       reflectivity[%4d]:      %d\n",
			     i, beam->reflectivity);
		fprintf(stderr,"dbg5       Q_factor[%4d]:          %d\n",
			     i, beam->Q_factor);
		fprintf(stderr,"dbg5       beam_no[%4d]:           %d\n",
			     i, beam->beam_no);
		fprintf(stderr,"dbg5       freq[%4d]:              %d\n",
			     i, beam->freq);
		fprintf(stderr,"dbg5       calibBackscatter[%4d]:  %d\n",
			     i, beam->calibratedBackscatter);
		fprintf(stderr,"dbg5       mindB[%4d]:             %d\n",
			     i, beam->mindB);
		fprintf(stderr,"dbg5       maxdB[%4d]:             %d\n",
			     i, beam->maxdB);
		fprintf(stderr,"dbg5       AngleIndepBacks[%4d]:   %d\n",
			     i, beam->pseudoAngleIndependentBackscatter);
		fprintf(stderr,"dbg5       range[%4d]:             %d\n",
			     i, beam->range);
		fprintf(stderr,"dbg5       no_samples[%4d]:        %d\n",
			     i, beam->no_samples);
		fprintf(stderr,"dbg5       offset[%4d]:            %d\n",
			     i, beam->offset);
		fprintf(stderr,"dbg5       centre_no[%4d]:         %d\n",
			     i, beam->centre_no);
		fprintf(stderr,"dbg5       sample_unit[%4d]:       %d\n",
			     i, beam->sample_unit);
		fprintf(stderr,"dbg5       sample_interval[%4d]:   %d\n",
			     i, beam->sample_interval);
		fprintf(stderr,"dbg5       dummy0[%4d]:            %d\n",
			     i, beam->dummy[0]);
		fprintf(stderr,"dbg5       dummy1[%4d]:            %d\n",
			     i, beam->dummy[1]);
		fprintf(stderr,"dbg5       samp_win_length[%4d]:   %d\n",
			     i, beam->samp_win_length);
		fprintf(stderr,"dbg5       beam_depress_angle[%4d]:%d\n",
			     i, beam->beam_depress_angle);
		fprintf(stderr,"dbg5       beam_heading_angle[%4d]:%d\n",
			     i, beam->beam_heading_angle);
		}
	    for (i=0;i<profile->numSamples;i++)
		fprintf(stderr,"dbg5       sidescan sample[%4d]:%d\n",
			     i, data->ss_raw[i]);
	    fprintf(stderr,"dbg5       status:     %d\n",status);
	    fprintf(stderr,"dbg5       error:      %d\n",*error);
	    }

	/* print debug statements */
	if (verbose >= 5 && dataplus->kind == MB_DATA_COMMENT)
	    {
	    fprintf(stderr,"\ndbg5  Comment set in function <%s>\n",
		    function_name);
	    fprintf(stderr,"dbg5       kind:                   %d\n",dataplus->kind);
	    fprintf(stderr,"dbg5       comment:                %s\n",dataplus->comment);
	    fprintf(stderr,"dbg5       status:                 %d\n",status);
	    fprintf(stderr,"dbg5       error:                  %d\n",*error);
	    }
	    
	/* reverse parse and write the summary */
	if (status == MB_SUCCESS
	    && dataplus->kind == MB_DATA_SUMMARY)
	    {
	    offset = 0;
	    buffer[offset] = 'H'; offset += 1;
	    buffer[offset] = 'D'; offset += 1;
	    buffer[offset] = 'C'; offset += 1;
	    buffer[offset] = 'S'; offset += 1;
#ifdef BYTESWAPPED
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = mb_swap_int(summary->sensorNumber); offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = mb_swap_int(summary->subFileID); offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = mb_swap_int(summary->fileVersion); offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = mb_swap_int(summary->toolType); offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = mb_swap_int(summary->numProfiles); offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = mb_swap_int(summary->numDepths); offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = mb_swap_int(summary->timeScale); offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = mb_swap_int(summary->refTime); offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = mb_swap_int(summary->minTime); offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = mb_swap_int(summary->maxTime); offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = mb_swap_int(summary->positionType); offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = mb_swap_int(summary->positionScale); offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = mb_swap_int(summary->refLat); offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = mb_swap_int(summary->minLat); offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = mb_swap_int(summary->maxLat); offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = mb_swap_int(summary->refLong); offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = mb_swap_int(summary->minLong); offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = mb_swap_int(summary->maxLong); offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = mb_swap_int(summary->minObsDepth); offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = mb_swap_int(summary->maxObsDepth); offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = mb_swap_int(summary->minProcDepth); offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = mb_swap_int(summary->maxProcDepth); offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = mb_swap_int(summary->status);
#else
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = summary->sensorNumber; offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = summary->subFileID; offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = summary->fileVersion; offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = summary->toolType; offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = summary->numProfiles; offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = summary->numDepths; offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = summary->timeScale; offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = summary->refTime; offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = summary->minTime; offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = summary->maxTime; offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = summary->positionType; offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = summary->positionScale; offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = summary->refLat; offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = summary->minLat; offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = summary->maxLat; offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = summary->refLong; offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = summary->minLong; offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = summary->maxLong; offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = summary->minObsDepth; offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = summary->maxObsDepth; offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = summary->minProcDepth; offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = summary->maxProcDepth; offset +=4;
	    int_ptr = (int *) &buffer[offset];
	    *int_ptr = summary->status;
#endif
    
	    /* write summary to file */
	    if ((write_size = fwrite(buffer,1,MBF_OMGHDCSJ_SUMMARY_SIZE,
			    mb_io_ptr->mbfp)) == MBF_OMGHDCSJ_SUMMARY_SIZE) 
		{
		mb_io_ptr->file_bytes += write_size;
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
	    else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	    }
	    
	/* else reverse parse and write the data record */
	else if (status == MB_SUCCESS
	    && dataplus->kind == MB_DATA_DATA)
	    {
	    /* first do the profile */
	    offset = 0;
	    if (*fileVersion == 1)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &buffer[offset];
		*int_ptr = mb_swap_int(profile->status); offset +=4;
		int_ptr = (int *) &buffer[offset];
		*int_ptr = mb_swap_int(profile->numDepths); offset +=4;
		int_ptr = (int *) &buffer[offset];
		*int_ptr = mb_swap_int(profile->timeOffset); offset +=4;
		int_ptr = (int *) &buffer[offset];
		*int_ptr = mb_swap_int(profile->vesselLatOffset); offset +=4;
		int_ptr = (int *) &buffer[offset];
		*int_ptr = mb_swap_int(profile->vesselLongOffset); offset +=4;
		int_ptr = (int *) &buffer[offset];
		*int_ptr = mb_swap_int(profile->vesselHeading); offset +=4;
		if (offset < *profile_size)
		    {
		    int_ptr = (int *) &buffer[offset];
		    *int_ptr = mb_swap_int(profile->vesselHeave); offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    *int_ptr = mb_swap_int(profile->vesselPitch); offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    *int_ptr = mb_swap_int(profile->vesselRoll); offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    *int_ptr = mb_swap_int(profile->tide); offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    *int_ptr = mb_swap_int(profile->vesselVelocity); offset +=4;
		    }
		if (offset < *profile_size)
		    {
		    buffer[offset] = profile->power; offset +=1;
		    buffer[offset] = profile->TVG; offset +=1;
		    buffer[offset] = profile->attenuation; offset +=1;
		    buffer[offset] = profile->edflag; offset +=1;
		    int_ptr = (int *) &buffer[offset];
		    *int_ptr = mb_swap_int(profile->soundVelocity); offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    *int_ptr = mb_swap_int(profile->lengthImageDataField); offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    *int_ptr = mb_swap_int(profile->pingNo); offset +=4;
		    buffer[offset] = profile->mode; offset +=1;
		    buffer[offset] = profile->Q_factor; offset +=1;
		    buffer[offset] = profile->pulseLength; offset +=1;
		    buffer[offset] = profile->unassigned; offset +=1;
		    }
#else
		int_ptr = (int *) &buffer[offset];
		*int_ptr = profile->status; offset +=4;
		int_ptr = (int *) &buffer[offset];
		*int_ptr = profile->numDepths; offset +=4;
		int_ptr = (int *) &buffer[offset];
		*int_ptr = profile->timeOffset; offset +=4;
		int_ptr = (int *) &buffer[offset];
		*int_ptr = profile->vesselLatOffset; offset +=4;
		int_ptr = (int *) &buffer[offset];
		*int_ptr = profile->vesselLongOffset; offset +=4;
		int_ptr = (int *) &buffer[offset];
		*int_ptr = profile->vesselHeading; offset +=4;
		if (offset < *profile_size)
		    {
		    int_ptr = (int *) &buffer[offset];
		    *int_ptr = profile->vesselHeave; offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    *int_ptr = profile->vesselPitch; offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    *int_ptr = profile->vesselRoll; offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    *int_ptr = profile->tide; offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    *int_ptr = profile->vesselVelocity; offset +=4;
		    }
		if (offset < *profile_size)
		    {
		    buffer[offset] = profile->power; offset +=1;
		    buffer[offset] = profile->TVG; offset +=1;
		    buffer[offset] = profile->attenuation; offset +=1;
		    buffer[offset] = profile->edflag; offset +=1;
		    int_ptr = (int *) &buffer[offset];
		    *int_ptr = profile->soundVelocity; offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    *int_ptr = profile->lengthImageDataField; offset +=4;
		    int_ptr = (int *) &buffer[offset];
		    *int_ptr = profile->pingNo; offset +=4;
		    buffer[offset] = profile->mode; offset +=1;
		    buffer[offset] = profile->Q_factor; offset +=1;
		    buffer[offset] = profile->pulseLength; offset +=1;
		    buffer[offset] = profile->unassigned; offset +=1;
		    }
#endif
		}
	    else if (*fileVersion == 2)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &buffer[offset];
		*int_ptr = mb_swap_int(profile->timeOffset); offset +=4;
		int_ptr = (int *) &buffer[offset];
		*int_ptr = mb_swap_int(profile->vesselLatOffset); offset +=4;
		int_ptr = (int *) &buffer[offset];
		*int_ptr = mb_swap_int(profile->vesselLongOffset); offset +=4;
		short_ptr = (short *) &buffer[offset];
		*short_ptr = mb_swap_short((short)(profile->vesselHeading / 10000)); offset +=2;
		short_ptr = (short *) &buffer[offset];
		*short_ptr = mb_swap_short((short)(profile->vesselHeave)); offset +=2;
		short_ptr = (short *) &buffer[offset];
		*short_ptr = mb_swap_short((short)(profile->vesselPitch / 1000)); offset +=2;
		short_ptr = (short *) &buffer[offset];
		*short_ptr = mb_swap_short((short)(profile->vesselRoll / 1000)); offset +=2;
		short_ptr = (short *) &buffer[offset];
		*short_ptr = mb_swap_short((short)(profile->tide)); offset +=2;
		short_ptr = (short *) &buffer[offset];
		*short_ptr = mb_swap_short((short)(profile->numDepths)); offset +=2;
		buffer[offset] = profile->power; offset +=1;
		buffer[offset] = profile->TVG; offset +=1;
		buffer[offset] = profile->attenuation; offset +=1;
		buffer[offset] = profile->pulseLength; offset +=1;
		buffer[offset] = profile->mode; offset +=1;
		buffer[offset] = profile->status; offset +=1;
		buffer[offset] = profile->edflag; offset +=1;
		buffer[offset] = profile->unassigned; offset +=1;
#else
		int_ptr = (int *) &buffer[offset];
		*int_ptr = profile->timeOffset; offset +=4;
		int_ptr = (int *) &buffer[offset];
		*int_ptr = profile->vesselLatOffset; offset +=4;
		int_ptr = (int *) &buffer[offset];
		*int_ptr = profile->vesselLongOffset; offset +=4;
		short_ptr = (short *) &buffer[offset];
		*short_ptr = (short)(profile->vesselHeading / 10000); offset +=2;
		short_ptr = (short *) &buffer[offset];
		*short_ptr = (short)(profile->vesselHeave); offset +=2;
		short_ptr = (short *) &buffer[offset];
		*short_ptr = (short)(profile->vesselPitch / 1000); offset +=2;
		short_ptr = (short *) &buffer[offset];
		*short_ptr = (short)(profile->vesselRoll / 1000); offset +=2;
		short_ptr = (short *) &buffer[offset];
		*short_ptr = (short)(profile->tide); offset +=2;
		short_ptr = (short *) &buffer[offset];
		*short_ptr = (short)(profile->numDepths); offset +=2;
		buffer[offset] = profile->power; offset +=1;
		buffer[offset] = profile->TVG; offset +=1;
		buffer[offset] = profile->attenuation; offset +=1;
		buffer[offset] = profile->pulseLength; offset +=1;
		buffer[offset] = profile->mode; offset +=1;
		buffer[offset] = profile->status; offset +=1;
		buffer[offset] = profile->edflag; offset +=1;
		buffer[offset] = profile->unassigned; offset +=1;
#endif
		}
	    else if (*fileVersion == 3)
		{
#ifdef BYTESWAPPED
		int_ptr = (int *) &buffer[offset];
		*int_ptr = mb_swap_int(profile->timeOffset); offset +=4;
		int_ptr = (int *) &buffer[offset];
		*int_ptr = mb_swap_int(profile->vesselLatOffset); offset +=4;
		int_ptr = (int *) &buffer[offset];
		*int_ptr = mb_swap_int(profile->vesselLongOffset); offset +=4;
		short_ptr = (short *) &buffer[offset];
		*short_ptr = mb_swap_short((short)(profile->vesselHeading / 10000)); offset +=2;
		short_ptr = (short *) &buffer[offset];
		*short_ptr = mb_swap_short((short)(profile->vesselHeave)); offset +=2;
		short_ptr = (short *) &buffer[offset];
		*short_ptr = mb_swap_short((short)(profile->vesselPitch / 1000)); offset +=2;
		short_ptr = (short *) &buffer[offset];
		*short_ptr = mb_swap_short((short)(profile->vesselRoll / 1000)); offset +=2;
		short_ptr = (short *) &buffer[offset];
		*short_ptr = mb_swap_short((short)(profile->tide)); offset +=2;
		short_ptr = (short *) &buffer[offset];
		*short_ptr = mb_swap_short((short)(profile->numDepths)); offset +=2;
		buffer[offset] = profile->power; offset +=1;
		buffer[offset] = profile->TVG; offset +=1;
		buffer[offset] = profile->attenuation; offset +=1;
		buffer[offset] = profile->pulseLength; offset +=1;
		buffer[offset] = profile->mode; offset +=1;
		buffer[offset] = profile->status; offset +=1;
		buffer[offset] = profile->edflag; offset +=1;
		buffer[offset] = profile->unassigned; offset +=1;
		if (offset < *profile_size)
		    {
		    short_ptr = (short *) &buffer[offset];
		    *short_ptr = mb_swap_short((short)(profile->td_sound_speed)); offset +=2;
		    short_ptr = (short *) &buffer[offset];
		    *short_ptr = mb_swap_short((short)(profile->samp_rate)); offset +=2;
		    buffer[offset] = profile->z_res_cm; offset +=1;
		    buffer[offset] = profile->xy_res_cm; offset +=1;
		    buffer[offset] = profile->ssp_source; offset +=1;
		    buffer[offset] = profile->filter_ID; offset +=1;
		    short_ptr = (short *) &buffer[offset];
		    *short_ptr = mb_swap_short((short)(profile->absorp_coeff)); offset +=2;
		    short_ptr = (short *) &buffer[offset];
		    *short_ptr = mb_swap_short((short)(profile->tx_pulse_len)); offset +=2;
		    short_ptr = (short *) &buffer[offset];
		    *short_ptr = mb_swap_short((short)(profile->tx_beam_width)); offset +=2;
		    short_ptr = (short *) &buffer[offset];
		    *short_ptr = mb_swap_short((short)(profile->max_swath_width)); offset +=2;
		    buffer[offset] = profile->tx_power_reduction; offset +=1;
		    buffer[offset] = profile->rx_beam_width; offset +=1;
		    buffer[offset] = profile->rx_bandwidth; offset +=1;
		    buffer[offset] = profile->rx_gain_reduction; offset +=1;
		    buffer[offset] = profile->tvg_crossover; offset +=1;
		    buffer[offset] = profile->beam_spacing; offset +=1;
		    buffer[offset] = profile->coverage_sector; offset +=1;
		    buffer[offset] = profile->yaw_stab_mode; offset +=1;
		    }
#else
		int_ptr = (int *) &buffer[offset];
		*int_ptr = profile->timeOffset; offset +=4;
		int_ptr = (int *) &buffer[offset];
		*int_ptr = profile->vesselLatOffset; offset +=4;
		int_ptr = (int *) &buffer[offset];
		*int_ptr = profile->vesselLongOffset; offset +=4;
		short_ptr = (short *) &buffer[offset];
		*short_ptr = (short)(profile->vesselHeading / 10000); offset +=2;
		short_ptr = (short *) &buffer[offset];
		*short_ptr = (short)(profile->vesselHeave); offset +=2;
		short_ptr = (short *) &buffer[offset];
		*short_ptr = (short)(profile->vesselPitch / 1000); offset +=2;
		short_ptr = (short *) &buffer[offset];
		*short_ptr = (short)(profile->vesselRoll / 1000); offset +=2;
		short_ptr = (short *) &buffer[offset];
		*short_ptr = (short)(profile->tide); offset +=2;
		short_ptr = (short *) &buffer[offset];
		*short_ptr = (short)(profile->numDepths); offset +=2;
		buffer[offset] = profile->power; offset +=1;
		buffer[offset] = profile->TVG; offset +=1;
		buffer[offset] = profile->attenuation; offset +=1;
		buffer[offset] = profile->pulseLength; offset +=1;
		buffer[offset] = profile->mode; offset +=1;
		buffer[offset] = profile->status; offset +=1;
		buffer[offset] = profile->edflag; offset +=1;
		buffer[offset] = profile->unassigned; offset +=1;
		if (offset < *profile_size)
		    {
		    short_ptr = (short *) &buffer[offset];
		    *short_ptr = (short)(profile->td_sound_speed); offset +=2;
		    short_ptr = (short *) &buffer[offset];
		    *short_ptr = (short)(profile->samp_rate); offset +=2;
		    buffer[offset] = profile->z_res_cm; offset +=1;
		    buffer[offset] = profile->xy_res_cm; offset +=1;
		    buffer[offset] = profile->ssp_source; offset +=1;
		    buffer[offset] = profile->filter_ID; offset +=1;
		    short_ptr = (short *) &buffer[offset];
		    *short_ptr = (short)(profile->absorp_coeff); offset +=2;
		    short_ptr = (short *) &buffer[offset];
		    *short_ptr = (short)(profile->tx_pulse_len); offset +=2;
		    short_ptr = (short *) &buffer[offset];
		    *short_ptr = (short)(profile->tx_beam_width); offset +=2;
		    short_ptr = (short *) &buffer[offset];
		    *short_ptr = (short)(profile->max_swath_width); offset +=2;
		    buffer[offset] = profile->tx_power_reduction; offset +=1;
		    buffer[offset] = profile->rx_beam_width; offset +=1;
		    buffer[offset] = profile->rx_bandwidth; offset +=1;
		    buffer[offset] = profile->rx_gain_reduction; offset +=1;
		    buffer[offset] = profile->tvg_crossover; offset +=1;
		    buffer[offset] = profile->beam_spacing; offset +=1;
		    buffer[offset] = profile->coverage_sector; offset +=1;
		    buffer[offset] = profile->yaw_stab_mode; offset +=1;
		    }
#endif
		}		
    
	    /* write profile to file */
	    if ((write_size = fwrite(buffer,1,*profile_size,
			    mb_io_ptr->mbfp)) == *profile_size) 
		{
		mb_io_ptr->file_bytes += write_size;
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
	    else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}

	    /* now reverse parse and write beam data */
	    if (status == MB_SUCCESS)
		{
		offset = 0;
		for (i=0;i<profile->numDepths;i++)
		    {
		    offset_start = offset;
		    beam = &data->beams[i];
		    if (*fileVersion == 1)
			{
#ifdef BYTESWAPPED
			int_ptr = (int *) &buffer[offset];
			*int_ptr = mb_swap_int(beam->status); offset +=4;
			int_ptr = (int *) &buffer[offset];
			*int_ptr = mb_swap_int(beam->observedDepth); offset +=4;
			if ((offset - offset_start) < *beam_size)
			    {
			    int_ptr = (int *) &buffer[offset];
			    *int_ptr = mb_swap_int(beam->acrossTrack); offset +=4;
			    int_ptr = (int *) &buffer[offset];
			    *int_ptr = mb_swap_int(beam->alongTrack); offset +=4;
			    int_ptr = (int *) &buffer[offset];
			    *int_ptr = mb_swap_int(beam->latOffset); offset +=4;
			    int_ptr = (int *) &buffer[offset];
			    *int_ptr = mb_swap_int(beam->processedDepth); offset +=4;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    int_ptr = (int *) &buffer[offset];
			    *int_ptr = mb_swap_int(beam->processedDepth); offset +=4;
			    int_ptr = (int *) &buffer[offset];
			    *int_ptr = mb_swap_int(beam->timeOffset); offset +=4;
			    int_ptr = (int *) &buffer[offset];
			    *int_ptr = mb_swap_int(beam->depthAccuracy); offset +=4;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    buffer[offset] = beam->reflectivity; offset+=1;
			    buffer[offset] = beam->Q_factor; offset+=1;
			    buffer[offset] = beam->beam_no; offset+=1;
			    buffer[offset] = beam->freq; offset+=1;
			    buffer[offset] = beam->calibratedBackscatter; offset+=1;
			    buffer[offset] = beam->mindB; offset+=1;
			    buffer[offset] = beam->maxdB; offset+=1;
			    buffer[offset] = beam->pseudoAngleIndependentBackscatter; offset+=1;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    int_ptr = (int *) &buffer[offset];
			    *int_ptr = mb_swap_int(beam->range); offset +=4;
			    int_ptr = (int *) &buffer[offset];
			    *int_ptr = mb_swap_int(beam->no_samples); offset +=4;
			    int_ptr = (int *) &buffer[offset];
			    *int_ptr = mb_swap_int(beam->offset); offset +=4;
			    int_ptr = (int *) &buffer[offset];
			    *int_ptr = mb_swap_int(beam->centre_no); offset +=4;
			    buffer[offset] = beam->sample_unit; offset+=1;
			    buffer[offset] = beam->sample_interval; offset+=1;
			    buffer[offset] = beam->dummy[0]; offset+=1;
			    buffer[offset] = beam->dummy[1]; offset+=1;
			    }
#else
			int_ptr = (int *) &buffer[offset];
			*int_ptr = beam->status; offset+=4;
			int_ptr = (int *) &buffer[offset];
			*int_ptr = beam->observedDepth; offset+=4;
			if ((offset - offset_start) < *beam_size)
			    {
			    int_ptr = (int *) &buffer[offset];
			    *int_ptr = beam->acrossTrack; offset+=4;
			    int_ptr = (int *) &buffer[offset];
			    *int_ptr = beam->alongTrack; offset+=4;
			    int_ptr = (int *) &buffer[offset];
			    *int_ptr = beam->latOffset; offset+=4;
			    int_ptr = (int *) &buffer[offset];
			    *int_ptr = beam->longOffset; offset+=4;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    int_ptr = (int *) &buffer[offset];
			    *int_ptr = beam->processedDepth; offset+=4;
			    int_ptr = (int *) &buffer[offset];
			    *int_ptr = beam->timeOffset; offset+=4;
			    int_ptr = (int *) &buffer[offset];
			    *int_ptr = beam->depthAccuracy; offset+=4;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    buffer[offset] = beam->reflectivity; offset+=1;
			    buffer[offset] = beam->Q_factor; offset+=1;
			    buffer[offset] = beam->beam_no; offset+=1;
			    buffer[offset] = beam->freq; offset+=1;
			    buffer[offset] = beam->calibratedBackscatter; offset+=1;
			    buffer[offset] = beam->mindB; offset+=1;
			    buffer[offset] = beam->maxdB; offset+=1;
			    buffer[offset] = beam->pseudoAngleIndependentBackscatter; offset+=1;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    int_ptr = (int *) &buffer[offset];
			    *int_ptr = beam->range; offset+=4;
			    int_ptr = (int *) &buffer[offset];
			    *int_ptr = beam->no_samples; offset+=4;
			    int_ptr = (int *) &buffer[offset];
			    *int_ptr = beam->offset; offset+=4;
			    int_ptr = (int *) &buffer[offset];
			    *int_ptr = beam->centre_no; offset+=4;
			    buffer[offset] = beam->sample_unit; offset+=1;
			    buffer[offset] = beam->sample_interval; offset+=1;
			    buffer[offset] = beam->dummy[0]; offset+=1;
			    buffer[offset] = beam->dummy[1]; offset+=1;
			    }
#endif
			}
		    else if (*fileVersion == 2)
			{
			MaxVal = MAX(abs(beam->observedDepth), 
					abs((int)beam->acrossTrack));
			if(MaxVal < 30000 )
			    ScaleFactor = 1;
			else if(MaxVal < 300000 )
			    ScaleFactor = 10;
			else if(MaxVal < 3000000 )
			    ScaleFactor = 100;
			else 
			    ScaleFactor = 1000;
			beam->observedDepth = beam->observedDepth / ScaleFactor;	
			beam->acrossTrack = beam->acrossTrack / ScaleFactor;	
			beam->alongTrack = beam->alongTrack / ScaleFactor;	
			beam->Q_factor = beam->reflectivity;
			if (ScaleFactor == 1) 
			    beam->alongTrack += -20000;
			else if (ScaleFactor == 10) 
			    beam->alongTrack += -10000;
			else if (ScaleFactor == 100) 
			    beam->alongTrack += 0;
			else if (ScaleFactor == 1000) 
			    beam->alongTrack += 10000;
#ifdef BYTESWAPPED
			if ((offset - offset_start) < *beam_size)
			    {
			    short_ptr = (short *) &buffer[offset];
			    *short_ptr = mb_swap_short(beam->observedDepth); offset+=2;
			    short_ptr = (short *) &buffer[offset];
			    *short_ptr = mb_swap_short(beam->status); offset+=2;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    buffer[offset] = beam->reflectivity; offset+=1;
			    buffer[offset] = beam->calibratedBackscatter; offset+=1;
			    buffer[offset] = beam->pseudoAngleIndependentBackscatter; offset+=1;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    short_ptr = (short *) &buffer[offset];
			    *short_ptr = mb_swap_short(beam->alongTrack); offset+=2;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    short_ptr = (short *) &buffer[offset];
			    *short_ptr = mb_swap_short(beam->range); offset+=2;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    int_ptr = (int *) &buffer[offset];
			    *int_ptr = mb_swap_int(beam->offset); offset +=4;
			    short_ptr = (short *) &buffer[offset];
			    *short_ptr = mb_swap_short(beam->no_samples); offset+=2;
			    short_ptr = (short *) &buffer[offset];
			    *short_ptr = mb_swap_short(beam->centre_no); offset+=2;
			    }
#else
			if ((offset - offset_start) < *beam_size)
			    {
			    short_ptr = (short *) &buffer[offset];
			    *short_ptr = beam->observedDepth; offset+=2;
			    short_ptr = (short *) &buffer[offset];
			    *short_ptr = beam->acrossTrack; offset+=2;
			    buffer[offset] = beam->status; offset+=1;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    buffer[offset] = beam->reflectivity; offset+=1;
			    buffer[offset] = beam->calibratedBackscatter; offset+=1;
			    buffer[offset] = beam->pseudoAngleIndependentBackscatter; offset+=1;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    short_ptr = (short *) &buffer[offset];
			    *short_ptr = beam->alongTrack; offset+=2;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    short_ptr = (short *) &buffer[offset];
			    *short_ptr = beam->range; offset+=2;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    int_ptr = (int *) &buffer[offset];
			    *int_ptr = beam->offset; offset+=4;
			    short_ptr = (short *) &buffer[offset];
			    *short_ptr = beam->no_samples; offset+=2;
			    short_ptr = (short *) &buffer[offset];
			    *short_ptr = beam->centre_no; offset+=2;
			    }
#endif
			}
		    else if (*fileVersion == 3)
			{
			MaxVal = MAX(abs(beam->observedDepth), 
					abs(beam->acrossTrack));
			if (MaxVal < 32000 )
				{
				ScaleFactor = pow(2.0,0.0);
				scaling_factor = 0;
				} 
			else if (MaxVal < 64000 )
				{
				ScaleFactor = pow(2.0,1.0);
				scaling_factor = 1;
				} 
			else if (MaxVal < 128000 )
				{
				ScaleFactor = pow(2.0,2.0);
				scaling_factor = 2;
				} 
			else if (MaxVal < 256000 )
				{
				ScaleFactor = pow(2.0,3.0);
				scaling_factor = 3;
				} 
			else if (MaxVal < 512000 )
				{
				ScaleFactor = pow(2.0,4.0);
				scaling_factor = 4;
				} 
			else if (MaxVal < 1024000 )
				{
				ScaleFactor = pow(2.0,5.0);
				scaling_factor = 5;
				} 
			else if (MaxVal < 2048000 )
				{
				ScaleFactor = pow(2.0,6.0);
				scaling_factor = 6;
				} 
			else if (MaxVal < 4096000 )
				{
				ScaleFactor = pow(2.0,7.0);
				scaling_factor = 7;
				} 
			else if (MaxVal < 8192000 )
				{
				ScaleFactor = pow(2.0,8.0);
				scaling_factor = 8;
				} 
			else
				{
				ScaleFactor = pow(2.0,10.0);
				scaling_factor = 10;
				}
			beam->observedDepth = beam->observedDepth / ScaleFactor;	
			beam->acrossTrack = beam->acrossTrack / ScaleFactor;	
			beam->alongTrack = beam->alongTrack / ScaleFactor;	
#ifdef BYTESWAPPED
			if ((offset - offset_start) < *beam_size)
			    {
			    short_ptr = (short *) &buffer[offset];
			    *short_ptr = mb_swap_short(beam->observedDepth); offset+=2;
			    short_ptr = (short *) &buffer[offset];
			    *short_ptr = mb_swap_short(beam->acrossTrack); offset+=2;
			    buffer[offset] = beam->status; offset+=1;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    buffer[offset] = beam->reflectivity; offset+=1;
			    buffer[offset] = beam->calibratedBackscatter; offset+=1;
			    buffer[offset] = beam->pseudoAngleIndependentBackscatter; offset+=1;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    short_ptr = (short *) &buffer[offset];
			    *short_ptr = mb_swap_short(beam->alongTrack); offset+=2;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    short_ptr = (short *) &buffer[offset];
			    *short_ptr = mb_swap_short(beam->range); offset+=2;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    int_ptr = (int *) &buffer[offset];
			    *int_ptr = mb_swap_int(beam->offset); offset +=4;
			    short_ptr = (short *) &buffer[offset];
			    *short_ptr = mb_swap_short(beam->no_samples); offset+=2;
			    short_ptr = (short *) &buffer[offset];
			    *short_ptr = mb_swap_short(beam->centre_no); offset+=2;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    short_ptr = (short *) &buffer[offset];
			    *short_ptr = mb_swap_short(beam->beam_depress_angle); offset+=2;
			    short_ptr = (short *) &buffer[offset];
			    *short_ptr = mb_swap_short(beam->beam_heading_angle); offset+=2;
			    buffer[offset] = beam->samp_win_length; offset+=1;
			    buffer[offset] = scaling_factor; offset+=1;
			    buffer[offset] = beam->Q_factor; offset+=1;
			    buffer[offset] = 0; offset+=1;
			    }
#else
			if ((offset - offset_start) < *beam_size)
			    {
			    short_ptr = (short *) &buffer[offset];
			    *short_ptr = beam->observedDepth; offset+=2;
			    short_ptr = (short *) &buffer[offset];
			    *short_ptr = beam->acrossTrack; offset+=2;
			    buffer[offset] = beam->status; offset+=1;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    buffer[offset] = beam->reflectivity; offset+=1;
			    buffer[offset] = beam->calibratedBackscatter; offset+=1;
			    buffer[offset] = beam->pseudoAngleIndependentBackscatter; offset+=1;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    short_ptr = (short *) &buffer[offset];
			    *short_ptr = beam->alongTrack; offset+=2;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    short_ptr = (short *) &buffer[offset];
			    *short_ptr = beam->range; offset+=2;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    int_ptr = (int *) &buffer[offset];
			    *int_ptr = beam->offset; offset+=4;
			    short_ptr = (short *) &buffer[offset];
			    *short_ptr = beam->no_samples; offset+=2;
			    short_ptr = (short *) &buffer[offset];
			    *short_ptr = beam->centre_no; offset+=2;
			    }
			if ((offset - offset_start) < *beam_size)
			    {
			    short_ptr = (short *) &buffer[offset];
			    *short_ptr = beam->beam_depress_angle; offset+=2;
			    short_ptr = (short *) &buffer[offset];
			    *short_ptr = beam->beam_heading_angle; offset+=2;
			    buffer[offset] = beam->samp_win_length; offset+=1;
			    buffer[offset] = scaling_factor; offset+=1;
			    buffer[offset] = beam->Q_factor; offset+=1;
			    buffer[offset] = 0; offset+=1;
			    }
#endif
			}
		    }
		}
    
	    /* write beam data to file */
	    if ((write_size = fwrite(buffer,1,*data_size,
			    mb_io_ptr->mbfp)) == *data_size) 
		{
		mb_io_ptr->file_bytes += write_size;
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
	    else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}

	    /* now deal with sidescan in parallel file */
	    if (status == MB_SUCCESS 
		&& mb_io_ptr->mbfp2 != NULL
		&& (summary->toolType == MBF_OMGHDCSJ_EM1000
		    || summary->toolType == MBF_OMGHDCSJ_EM12_single
		    || summary->toolType == MBF_OMGHDCSJ_EM12_dual
		    || summary->toolType == MBF_OMGHDCSJ_EM300
		    || summary->toolType == MBF_OMGHDCSJ_EM3000
		    || summary->toolType == MBF_OMGHDCSJ_EM3000D
		    || summary->toolType == MBF_OMGHDCSJ_EM121A))
		{
		/* write the sidescan */
		if (status == MB_SUCCESS)
		    {
		    if ((write_size = fwrite(data->ss_raw,1,
			profile->numSamples,mb_io_ptr->mbfp2))
			== profile->numSamples)
			{
			mb_io_ptr->file2_bytes += write_size;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			}
		    else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		    }
		}
	    }
	    
	/* else write the comment */
	else if (status == MB_SUCCESS
	    && dataplus->kind == MB_DATA_COMMENT)
	    {
	    offset = 0;
	    buffer[offset] = '#'; offset += 1;
	    buffer[offset] = '#'; offset += 1;
	    buffer[offset] = '#'; offset += 1;
	    buffer[offset] = '#'; offset += 1;
    
	    /* write comment to file */
	    if ((write_size = fwrite(buffer,1,4,
			    mb_io_ptr->mbfp)) == 4) 
		{
		mb_io_ptr->file_bytes += write_size;
		if ((write_size = fwrite(dataplus->comment,1,
				MBF_OMGHDCSJ_MAX_COMMENT,
				mb_io_ptr->mbfp)) 
				== MBF_OMGHDCSJ_MAX_COMMENT) 
		    {
		    mb_io_ptr->file_bytes += write_size;
		    status = MB_SUCCESS;
		    *error = MB_ERROR_NO_ERROR;
		    }
		else
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_WRITE_FAIL;
		    }
		}
	    else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
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
