/*--------------------------------------------------------------------
 *    The MB-system:	mbr_gsfgenmb.c	2/27/98
 *	$Id: mbr_gsfgenmb.c,v 5.2 2001-03-22 20:50:02 caress Exp $
 *
 *    Copyright (c) 1998, 2000 by
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
 * mbr_gsfgenmb.c contains the functions for reading and writing
 * multibeam data in the GSFGENMB format.  
 * These functions include:
 *   mbr_alm_gsfgenmb	- allocate read/write memory
 *   mbr_dem_gsfgenmb	- deallocate read/write memory
 *   mbr_rt_gsfgenmb	- read and translate data
 *   mbr_wt_gsfgenmb	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	February 27, 1998
 * $Log: not supported by cvs2svn $
 * Revision 5.1  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.4  2000/10/11  01:03:21  caress
 * Convert to ANSI C
 *
 * Revision 4.3  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.2  2000/07/19  03:51:38  caress
 * Fixed some things.
 *
 * Revision 4.1  1999/05/05  22:48:29  caress
 * Disabled handling of ping flags in GSF data.
 *
 * Revision 4.0  1998/10/05  18:30:03  caress
 * MB-System version 4.6beta
 *
 * Revision 1.1  1998/10/05  17:46:15  caress
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
#include "../../include/gsf.h"
#include "../../include/mbf_gsfgenmb.h"
#include "../../include/mbsys_gsf.h"

/* GSF error value */
extern int gsfError;

/* essential function prototypes */
int mbr_register_gsfgenmb(int verbose, char *mbio_ptr, 
		int *error);
int mbr_info_gsfgenmb(int verbose, 
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
int mbr_alm_gsfgenmb(int verbose, char *mbio_ptr, int *error);
int mbr_dem_gsfgenmb(int verbose, char *mbio_ptr, int *error);
int mbr_rt_gsfgenmb(int verbose, char *mbio_ptr, char *store_ptr, int *error);
int mbr_wt_gsfgenmb(int verbose, char *mbio_ptr, char *store_ptr, int *error);

/*--------------------------------------------------------------------*/
int mbr_register_gsfgenmb(int verbose, char *mbio_ptr, int *error)
{
	static char res_id[]="$Id: mbr_gsfgenmb.c,v 5.2 2001-03-22 20:50:02 caress Exp $";
	char	*function_name = "mbr_register_gsfgenmb";
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
	status = mbr_info_gsfgenmb(verbose, 
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_gsfgenmb;
	mb_io_ptr->mb_io_format_free = &mbr_dem_gsfgenmb; 
	mb_io_ptr->mb_io_store_alloc = &mbsys_gsf_alloc; 
	mb_io_ptr->mb_io_store_free = &mbsys_gsf_deall; 
	mb_io_ptr->mb_io_read_ping = &mbr_rt_gsfgenmb; 
	mb_io_ptr->mb_io_write_ping = &mbr_wt_gsfgenmb; 
	mb_io_ptr->mb_io_extract = &mbsys_gsf_extract; 
	mb_io_ptr->mb_io_insert = &mbsys_gsf_insert; 
	mb_io_ptr->mb_io_extract_nav = &mbsys_gsf_extract_nav; 
	mb_io_ptr->mb_io_insert_nav = &mbsys_gsf_insert_nav; 
	mb_io_ptr->mb_io_extract_altitude = &mbsys_gsf_extract_altitude; 
	mb_io_ptr->mb_io_insert_altitude = &mbsys_gsf_insert_altitude; 
	mb_io_ptr->mb_io_extract_svp = &mbsys_gsf_extract_svp; 
	mb_io_ptr->mb_io_insert_svp = &mbsys_gsf_insert_svp; 
	mb_io_ptr->mb_io_ttimes = &mbsys_gsf_ttimes; 
	mb_io_ptr->mb_io_copyrecord = &mbsys_gsf_copy; 
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
int mbr_info_gsfgenmb(int verbose, 
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
	static char res_id[]="$Id: mbr_gsfgenmb.c,v 5.2 2001-03-22 20:50:02 caress Exp $";
	char	*function_name = "mbr_info_gsfgenmb";
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
	*system = MB_SYS_GSF;
	*beams_bath_max = 254;
	*beams_amp_max = 254;
	*pixels_ss_max = 0;
	strncpy(format_name, "GSFGENMB", MB_NAME_LENGTH);
	strncpy(system_name, "GSF", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_GSFGENMB\nInformal Description: SAIC Generic Sensor Format (GSF)\nAttributes:           variable beams,  bathymetry and amplitude,\n                      binary, single files, SAIC. \n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_GSF;
	*variable_beams = MB_YES;
	*traveltime = MB_YES;
	*beam_flagging = MB_YES;
	*nav_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_DATA;
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
int mbr_alm_gsfgenmb(int verbose, char *mbio_ptr, int *error)
{
 static char res_id[]="$Id: mbr_gsfgenmb.c,v 5.2 2001-03-22 20:50:02 caress Exp $";
	char	*function_name = "mbr_alm_gsfgenmb";
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

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set initial status */
	status = MB_SUCCESS;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = sizeof(struct mbf_gsfgenmb_struct);
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	memset(mb_io_ptr->raw_data, 0, mb_io_ptr->structure_size);
	status = mbsys_gsf_alloc(verbose,mbio_ptr,
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
int mbr_dem_gsfgenmb(int verbose, char *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_gsfgenmb";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_gsfgenmb_struct *data;
	gsfRecords	    *records;

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
	data = (struct mbf_gsfgenmb_struct *) mb_io_ptr->raw_data;
	records = &(data->records);

	/* deallocate memory for data descriptor */
	/*gsfFree(records);*/
	status = mb_free(verbose,&mb_io_ptr->raw_data,error);
	status = mbsys_gsf_deall(verbose,mbio_ptr,
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
int mbr_rt_gsfgenmb(int verbose, char *mbio_ptr, char *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_gsfgenmb";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_gsfgenmb_struct *data;
	struct mbsys_gsf_struct *store;
	gsfDataID	    *dataID;
	gsfRecords	    *records;
	gsfSwathBathyPing   *mb_ping;
	int	ret;
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

	/* get pointer to mbio descriptor and data structure */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	store = (struct mbsys_gsf_struct *) store_ptr;

	/* get pointer to raw data structure */
	data = (struct mbf_gsfgenmb_struct *) mb_io_ptr->raw_data;
	
	/* get pointers to GSF structures */
	dataID = &(data->dataID);
	records = &(data->records);
	mb_ping = &(records->mb_ping);
	
	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* read next record from file */
	ret = gsfRead((int)mb_io_ptr->mbfp, GSF_NEXT_RECORD, dataID, records, NULL, 0);
	
	/* deal with errors */
	if (ret < 0)
	    {
	    if (gsfError == GSF_READ_TO_END_OF_FILE)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
	    else
		{	
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
		}
	    }
	    
	/* else deal with data */
	else
	    {
	    if (dataID->recordID == GSF_RECORD_HISTORY)
		{
		data->kind = MB_DATA_HISTORY;
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
		
	    else if (dataID->recordID == GSF_RECORD_SWATH_BATHY_SUMMARY)
		{
		data->kind = MB_DATA_SUMMARY;
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
		
	    else if (dataID->recordID == GSF_RECORD_PROCESSING_PARAMETERS)
		{
		data->kind = MB_DATA_PROCESSING_PARAMETERS;
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
		
	    else if (dataID->recordID == GSF_RECORD_SENSOR_PARAMETERS)
		{
		data->kind = MB_DATA_PROCESSING_PARAMETERS;
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
		
	    else if (dataID->recordID == GSF_RECORD_NAVIGATION_ERROR)
		{
		data->kind = MB_DATA_NAVIGATION_ERROR;
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}		
		
	    else if (dataID->recordID == GSF_RECORD_SOUND_VELOCITY_PROFILE)
		{
		data->kind = MB_DATA_VELOCITY_PROFILE;
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}

	    else if (dataID->recordID == GSF_RECORD_COMMENT)
		{
		/* copy comment */
		data->kind = MB_DATA_COMMENT;
		if (records->comment.comment != NULL)
		    {
		    status = MB_SUCCESS;
		    *error = MB_ERROR_NO_ERROR;
		    }
		else
		    {
		    status = MB_FAILURE;
		    *error = MB_ERROR_UNINTELLIGIBLE;
		    }
		}
		
	    else if (dataID->recordID == GSF_RECORD_SWATH_BATHYMETRY_PING)
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		data->kind = MB_DATA_DATA;
		}
		
	    else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
		}	
	    }

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = data->kind;
	
	/* output debug info */
	if (verbose >= 4)
		{
		fprintf(stderr,"\ndbg4  New record read by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg4  New record kind:\n");
		fprintf(stderr,"dbg4       error:      %d\n",
			mb_io_ptr->new_error);
		fprintf(stderr,"dbg4       kind:       %d\n",
			mb_io_ptr->new_kind);
		}
	if (verbose >= 4 && data->kind == MB_DATA_COMMENT)
		{
		fprintf(stderr,"\ndbg4  New comment read by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg4  New comment values:\n");
		fprintf(stderr,"dbg4       kind:              %d\n",
			data->kind);
		fprintf(stderr,"dbg4       comment time sec:  %d\n",
			records->comment.comment_time.tv_sec);
		fprintf(stderr,"dbg4       comment time nsec: %d\n",
			records->comment.comment_time.tv_nsec);
		fprintf(stderr,"dbg4       comment length:    %d\n",
			records->comment.comment_length);
		fprintf(stderr,"dbg4       comment:           %s\n",
			records->comment.comment);
		}
	if (verbose >= 4 && data->kind == MB_DATA_DATA)
		{
		fprintf(stderr,"\ndbg4  New ping to be written by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg4  New ping values:\n");
		fprintf(stderr,"dbg4       kind:       %d\n",
			data->kind);
		fprintf(stderr,"dbg4       time sec:   %d\n",
			mb_ping->ping_time.tv_sec);
		fprintf(stderr,"dbg4       time nsec:  %d\n",
			mb_ping->ping_time.tv_nsec);
		fprintf(stderr,"dbg4       longitude:  %f\n",
			mb_ping->longitude);
		fprintf(stderr,"dbg4       latitude:   %f\n",
			mb_ping->latitude);
		fprintf(stderr,"dbg4       speed:      %f\n",
			mb_ping->speed);
		fprintf(stderr,"dbg4       heading:    %f\n",
			mb_ping->heading);
		fprintf(stderr,"dbg4       beams:      %d\n",
			mb_ping->number_beams);
		for (i=0;i<mb_ping->number_beams;i++)
		  fprintf(stderr,"dbg4       beam:%d  flag:%d  bath:%f  acrosstrack:%f  alongtrack:%f\n",
			i,mb_ping->beam_flags[i],
			mb_ping->depth[i],
			mb_ping->across_track[i],
			mb_ping->along_track[i]);
		if (mb_ping->mc_amplitude != NULL)
		for (i=0;i<mb_ping->number_beams;i++)
		  fprintf(stderr,"dbg4       beam:%d  amp:%f  acrosstrack:%f  alongtrack:%f\n",
			i,mb_ping->mc_amplitude[i],
			mb_ping->across_track[i],
			mb_ping->along_track[i]);
		if (mb_ping->mr_amplitude != NULL)
		for (i=0;i<mb_ping->number_beams;i++)
		  fprintf(stderr,"dbg4       beam:%d  amp:%f  acrosstrack:%f  alongtrack:%f\n",
			i,mb_ping->mr_amplitude[i],
			mb_ping->across_track[i],
			mb_ping->along_track[i]);
		}

	/* translate values to data storage structure */
	if (status == MB_SUCCESS
		&& store != NULL)
		{
		gsfFree(&(store->records));
		gsfCopyRecords(&(store->records), records);
		store->dataID = *dataID;
		store->kind = data->kind;
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
int mbr_wt_gsfgenmb(int verbose, char *mbio_ptr, char *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_gsfgenmb";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_gsfgenmb_struct *data;
	struct mbsys_gsf_struct *store;
	gsfDataID		    *dataID;
	gsfRecords	    *records;
	gsfSwathBathyPing   *mb_ping;
	int	ret;
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

	/* get pointer to mbio descriptor and data structure */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	store = (struct mbsys_gsf_struct *) store_ptr;

	/* get pointer to raw data structure */
	data = (struct mbf_gsfgenmb_struct *) mb_io_ptr->raw_data;
	
	/* get pointers to GSF structures */
	dataID = &(data->dataID);
	records = &(data->records);
	mb_ping = &(records->mb_ping);

	/* translate values to data storage structure */
	if (status == MB_SUCCESS
		&& store != NULL)
		{
		gsfFree(records);
		gsfCopyRecords(records, &(store->records));
		*dataID = store->dataID;
		data->kind = store->kind;
		}
	
	/* output debug info */
	if (verbose >= 4)
		{
		fprintf(stderr,"\ndbg4  New record to be written by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg4  New record kind:\n");
		fprintf(stderr,"dbg4       kind:              %d\n",
			data->kind);
		}
	if (verbose >= 4 && data->kind == MB_DATA_COMMENT)
		{
		fprintf(stderr,"\ndbg4  New comment to be written by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg4  New comment values:\n");
		fprintf(stderr,"dbg4       kind:              %d\n",
			data->kind);
		fprintf(stderr,"dbg4       comment time sec:  %d\n",
			records->comment.comment_time.tv_sec);
		fprintf(stderr,"dbg4       comment time nsec: %d\n",
			records->comment.comment_time.tv_nsec);
		fprintf(stderr,"dbg4       comment length:    %d\n",
			records->comment.comment_length);
		fprintf(stderr,"dbg4       comment:           %s\n",
			records->comment.comment);
		}
	if (verbose >= 4 && data->kind == MB_DATA_DATA)
		{
		fprintf(stderr,"\ndbg4  New ping to be written by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg4  New ping values:\n");
		fprintf(stderr,"dbg4       kind:       %d\n",
			data->kind);
		fprintf(stderr,"dbg4       time sec:   %d\n",
			mb_ping->ping_time.tv_sec);
		fprintf(stderr,"dbg4       time nsec:  %d\n",
			mb_ping->ping_time.tv_nsec);
		fprintf(stderr,"dbg4       longitude:  %f\n",
			mb_ping->longitude);
		fprintf(stderr,"dbg4       latitude:   %f\n",
			mb_ping->latitude);
		fprintf(stderr,"dbg4       speed:      %f\n",
			mb_ping->speed);
		fprintf(stderr,"dbg4       heading:    %f\n",
			mb_ping->heading);
		fprintf(stderr,"dbg4       beams:      %d\n",
			mb_ping->number_beams);
		for (i=0;i<mb_ping->number_beams;i++)
		  fprintf(stderr,"dbg4       beam:%d  flag:%d  bath:%f  acrosstrack:%f  alongtrack:%f\n",
			i,mb_ping->beam_flags[i],
			mb_ping->depth[i],
			mb_ping->across_track[i],
			mb_ping->along_track[i]);
		if (mb_ping->mc_amplitude != NULL)
		for (i=0;i<mb_ping->number_beams;i++)
		  fprintf(stderr,"dbg4       beam:%d  amp:%f  acrosstrack:%f  alongtrack:%f\n",
			i,mb_ping->mc_amplitude[i],
			mb_ping->across_track[i],
			mb_ping->along_track[i]);
		if (mb_ping->mr_amplitude != NULL)
		for (i=0;i<mb_ping->number_beams;i++)
		  fprintf(stderr,"dbg4       beam:%d  amp:%f  acrosstrack:%f  alongtrack:%f\n",
			i,mb_ping->mr_amplitude[i],
			mb_ping->across_track[i],
			mb_ping->along_track[i]);
		}

	/* write gsf data to file */
	if (status == MB_SUCCESS)
	    {
	    if ((ret = gsfWrite((int)mb_io_ptr->mbfp, dataID, records)) < 0)
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
