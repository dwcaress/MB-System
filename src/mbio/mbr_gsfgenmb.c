/*--------------------------------------------------------------------
 *    The MB-system:	mbr_gsfgenmb.c	2/27/98
 *	$Id: mbr_gsfgenmb.c,v 4.2 2000-07-19 03:51:38 caress Exp $
 *
 *    Copyright (c) 1998 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
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

/*--------------------------------------------------------------------*/
int mbr_alm_gsfgenmb(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
{
 static char res_id[]="$Id: mbr_gsfgenmb.c,v 4.2 2000-07-19 03:51:38 caress Exp $";
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
int mbr_dem_gsfgenmb(verbose,mbio_ptr,error)
int	verbose;
char	*mbio_ptr;
int	*error;
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
int mbr_rt_gsfgenmb(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
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
		    strncpy(mb_io_ptr->new_comment,
			    records->comment.comment,MB_COMMENT_MAXLINE);
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
		mb_ping = &(records->mb_ping);

		/* get time */
		mb_io_ptr->new_time_d = mb_ping->ping_time.tv_sec 
				+ 0.000000001 * mb_ping->ping_time.tv_nsec;
		mb_get_date(verbose,mb_io_ptr->new_time_d,mb_io_ptr->new_time_i);

		/* get navigation */
		mb_io_ptr->new_lon = mb_ping->longitude;
		mb_io_ptr->new_lat = mb_ping->latitude;
		if (mb_io_ptr->lonflip < 0)
			{
			if (mb_io_ptr->new_lon > 0.) 
				mb_io_ptr->new_lon = mb_io_ptr->new_lon - 360.;
			else if (mb_io_ptr->new_lon < -360.)
				mb_io_ptr->new_lon = mb_io_ptr->new_lon + 360.;
			}
		else if (mb_io_ptr->lonflip == 0)
			{
			if (mb_io_ptr->new_lon > 180.) 
				mb_io_ptr->new_lon = mb_io_ptr->new_lon - 360.;
			else if (mb_io_ptr->new_lon < -180.)
				mb_io_ptr->new_lon = mb_io_ptr->new_lon + 360.;
			}
		else
			{
			if (mb_io_ptr->new_lon > 360.) 
				mb_io_ptr->new_lon = mb_io_ptr->new_lon - 360.;
			else if (mb_io_ptr->new_lon < 0.)
				mb_io_ptr->new_lon = mb_io_ptr->new_lon + 360.;
			}

		/* get heading */
		mb_io_ptr->new_heading = mb_ping->heading;

		/* get speed */
		mb_io_ptr->new_speed = 1.852 * mb_ping->speed;

		/* get numbers of beams and pixels */
		if (mb_ping->depth != NULL)
		    mb_io_ptr->beams_bath = mb_ping->number_beams;
		else
		    mb_io_ptr->beams_bath = 0;
		if (mb_ping->mc_amplitude != NULL
		    || mb_ping->mr_amplitude != NULL)
		    mb_io_ptr->beams_amp = mb_ping->number_beams;
		else
		    mb_io_ptr->beams_amp = 0;
		mb_io_ptr->pixels_ss = 0;

		/* read depth and beam location values into storage arrays */
/*fprintf(stderr, "%15f   heave:%15f draft:%15f\n", 
mb_io_ptr->new_time_d, mb_ping->heave, mb_ping->depth_corrector);*/
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			/* set null beam flag if required */
			if (mb_ping->depth[i] == 0.0
			    && mb_ping->across_track[i] == 0.0
			    && mb_ping->beam_flags[i] != MB_FLAG_NULL)
			    mb_ping->beam_flags[i] = MB_FLAG_NULL;

			mb_io_ptr->new_beamflag[i] = mb_ping->beam_flags[i];
			mb_io_ptr->new_bath[i] = mb_ping->depth[i];
			mb_io_ptr->new_bath_acrosstrack[i] = mb_ping->across_track[i];
			mb_io_ptr->new_bath_alongtrack[i] = mb_ping->along_track[i];		
			}

		/* set beamflags if ping flag set */
		if (mb_ping->ping_flags != 0)
		    {
		    for (i=0;i<mb_io_ptr->beams_bath;i++)
			if (mb_beam_ok(mb_io_ptr->new_beamflag[i]))
			    mb_io_ptr->new_beamflag[i] 
				= mb_beam_set_flag_manual(mb_io_ptr->new_beamflag[i]);
		    }

		/* read amplitude values into storage arrays */
		if (mb_ping->mc_amplitude != NULL)
		for (i=0;i<mb_io_ptr->beams_amp;i++)
			{
			mb_io_ptr->new_amp[i] = mb_ping->mc_amplitude[i];
			}
		else if (mb_ping->mr_amplitude != NULL)
		for (i=0;i<mb_io_ptr->beams_amp;i++)
			{
			mb_io_ptr->new_amp[i] = mb_ping->mr_amplitude[i];
			}
		}
		
	    else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
		}	
	    }
	    
	/* set kind in mb_io_ptr */
	mb_io_ptr->new_kind = data->kind;

	/* set error in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	
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
	if (verbose >= 4 && mb_io_ptr->new_kind == MB_DATA_COMMENT)
		{
		fprintf(stderr,"\ndbg4  New comment read by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg4  New comment values:\n");
		fprintf(stderr,"dbg4       error:      %d\n",
			mb_io_ptr->new_error);
		fprintf(stderr,"dbg4       kind:       %d\n",
			mb_io_ptr->new_kind);
		fprintf(stderr,"dbg4       comment:    %s\n",
			mb_io_ptr->new_comment);
		}
	if (verbose >= 4 && mb_io_ptr->new_kind == MB_DATA_DATA)
		{
		fprintf(stderr,"\ndbg4  New ping read by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg4  New ping values:\n");
		fprintf(stderr,"dbg4       error:      %d\n",
			mb_io_ptr->new_error);
		fprintf(stderr,"dbg4       kind:       %d\n",
			mb_io_ptr->new_kind);
		fprintf(stderr,"dbg4       time_i[0]:  %d\n",
			mb_io_ptr->new_time_i[0]);
		fprintf(stderr,"dbg4       time_i[1]:  %d\n",
			mb_io_ptr->new_time_i[1]);
		fprintf(stderr,"dbg4       time_i[2]:  %d\n",
			mb_io_ptr->new_time_i[2]);
		fprintf(stderr,"dbg4       time_i[3]:  %d\n",
			mb_io_ptr->new_time_i[3]);
		fprintf(stderr,"dbg4       time_i[4]:  %d\n",
			mb_io_ptr->new_time_i[4]);
		fprintf(stderr,"dbg4       time_i[5]:  %d\n",
			mb_io_ptr->new_time_i[5]);
		fprintf(stderr,"dbg4       time_i[6]:  %d\n",
			mb_io_ptr->new_time_i[6]);
		fprintf(stderr,"dbg4       time_d:     %f\n",
			mb_io_ptr->new_time_d);
		fprintf(stderr,"dbg4       longitude:  %f\n",
			mb_io_ptr->new_lon);
		fprintf(stderr,"dbg4       latitude:   %f\n",
			mb_io_ptr->new_lat);
		fprintf(stderr,"dbg4       speed:      %f\n",
			mb_io_ptr->new_speed);
		fprintf(stderr,"dbg4       heading:    %f\n",
			mb_io_ptr->new_heading);
		fprintf(stderr,"dbg4       beams_bath: %d\n",
			mb_io_ptr->beams_bath);
		fprintf(stderr,"dbg4       beams_amp:  %d\n",
			mb_io_ptr->beams_amp);
		for (i=0;i<mb_io_ptr->beams_bath;i++)
		  fprintf(stderr,"dbg4       beam:%d  flag:%d  bath:%f  acrosstrack:%f  alongtrack:%f\n",
			i,mb_io_ptr->new_beamflag[i],
			mb_io_ptr->new_bath[i],
			mb_io_ptr->new_bath_acrosstrack[i],
			mb_io_ptr->new_bath_alongtrack[i]);
		for (i=0;i<mb_io_ptr->beams_amp;i++)
		  fprintf(stderr,"dbg4       beam:%d  amp:%f  acrosstrack:%f  alongtrack:%f\n",
			i,mb_io_ptr->new_amp[i],
			mb_io_ptr->new_bath_acrosstrack[i],
			mb_io_ptr->new_bath_alongtrack[i]);
		fprintf(stderr,"dbg4       pixels_ss:  %d\n",
			mb_io_ptr->pixels_ss);
		for (i=0;i<mb_io_ptr->pixels_ss;i++)
		  fprintf(stderr,"dbg4       pixel:%d  ss:%f acrosstrack:%f  alongtrack:%f\n",
			i,mb_io_ptr->new_ss[i],
			mb_io_ptr->new_ss_acrosstrack[i],
			mb_io_ptr->new_ss_alongtrack[i]);
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
int mbr_wt_gsfgenmb(verbose,mbio_ptr,store_ptr,error)
int	verbose;
char	*mbio_ptr;
char	*store_ptr;
int	*error;
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

	/* set kind from current ping */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR)
		data->kind = mb_io_ptr->new_kind;

	/* check for comment */
	if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_COMMENT)
		{
		/* copy comment */
		data->kind = MB_DATA_COMMENT;
		dataID->recordID = GSF_RECORD_COMMENT;
		if (records->comment.comment_length < strlen(mb_io_ptr->new_comment) + 1)
		    {
		    if ((records->comment.comment 
			= (char *) realloc(records->comment.comment,
					strlen(mb_io_ptr->new_comment)+1))
					    == NULL) 
			{
			status = MB_FAILURE;
			*error = MB_ERROR_MEMORY_FAIL;
			records->comment.comment_length = 0;
			}
		    }
		if (status = MB_SUCCESS && records->comment.comment != NULL)
		    {
		    strcpy(records->comment.comment, mb_io_ptr->new_comment);
		    records->comment.comment_length = strlen(mb_io_ptr->new_comment);
		    records->comment.comment_time.tv_sec = (int) mb_io_ptr->new_time_d;
		    records->comment.comment_time.tv_nsec 
			    = (int) (1000000000 
				* (mb_io_ptr->new_time_d 
					- records->comment.comment_time.tv_sec));
		    }
		}

	/* else translate current ping data to gsfgenmb data structure */
	else if (mb_io_ptr->new_error == MB_ERROR_NO_ERROR
		&& mb_io_ptr->new_kind == MB_DATA_DATA)
		{
		data->kind = MB_DATA_DATA;
		dataID->recordID = GSF_RECORD_SWATH_BATHYMETRY_PING;

		/* get time */
		mb_ping->ping_time.tv_sec = (int) mb_io_ptr->new_time_d;
		mb_ping->ping_time.tv_nsec 
			= (int) (1000000000 
			    * (mb_io_ptr->new_time_d 
				    - mb_ping->ping_time.tv_sec));

		/* get navigation */
		mb_ping->longitude = mb_io_ptr->new_lon;
		mb_ping->latitude = mb_io_ptr->new_lat;

		/* get heading */
		mb_ping->heading = mb_io_ptr->new_heading;

		/* get speed */
		mb_ping->speed = mb_io_ptr->new_speed / 1.852;

		/* get numbers of beams */
		mb_ping->number_beams = MAX(mb_io_ptr->beams_bath, 
					    mb_io_ptr->beams_amp);
		
		/* allocate memory in arrays if required */
		if (mb_io_ptr->beams_bath > 0)
		    {
		    mb_ping->beam_flags 
			= (unsigned char *) 
			    realloc(mb_ping->beam_flags,
					mb_io_ptr->beams_bath * sizeof(char));
		    mb_ping->depth 
			= (double *) 
			    realloc(mb_ping->depth,
					mb_io_ptr->beams_bath * sizeof(double));
		    mb_ping->across_track 
			= (double *) 
			    realloc(mb_ping->across_track,
					mb_io_ptr->beams_bath * sizeof(double));
		    mb_ping->along_track 
			= (double *) 
			    realloc(mb_ping->along_track,
					mb_io_ptr->beams_bath * sizeof(double));
		    if (mb_ping->beam_flags == NULL
			|| mb_ping->depth == NULL
			|| mb_ping->across_track == NULL
			|| mb_ping->along_track == NULL)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_MEMORY_FAIL;
			}
		    }
		if (mb_io_ptr->beams_amp > 0
		    && mb_ping->mc_amplitude != NULL)
		    {
		    mb_ping->mc_amplitude 
			= (double *) 
			    realloc(mb_ping->mc_amplitude,
					mb_io_ptr->beams_amp * sizeof(double));
		    if (mb_ping->mc_amplitude == NULL)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_MEMORY_FAIL;
			}
		    }
		else if (mb_io_ptr->beams_amp > 0)
		    {
		    mb_ping->mr_amplitude 
			= (double *) 
			    realloc(mb_ping->mr_amplitude,
					mb_io_ptr->beams_amp * sizeof(double));
		    if (mb_ping->mr_amplitude == NULL)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_MEMORY_FAIL;
			}
		    }

		/* if ping flag set check for any unset
		    beam flags - unset ping flag if any
		    good beams found */
		if (mb_ping->ping_flags != 0)
		    {
		    for (i=0;i<mb_io_ptr->beams_bath;i++)
			if (mb_beam_ok(mb_io_ptr->new_beamflag[i]))
			    mb_ping->ping_flags = 0;
		    }

		/* read depth and beam location values into storage arrays */
		for (i=0;i<mb_io_ptr->beams_bath;i++)
			{
			mb_ping->beam_flags[i] = mb_io_ptr->new_beamflag[i];
			mb_ping->depth[i] = mb_io_ptr->new_bath[i];
			mb_ping->across_track[i] = mb_io_ptr->new_bath_acrosstrack[i];
			mb_ping->along_track[i] = mb_io_ptr->new_bath_alongtrack[i];
			}

		/* read amplitude values into storage arrays */
		if (mb_ping->mc_amplitude != NULL)
		for (i=0;i<mb_io_ptr->beams_amp;i++)
			{
			mb_ping->mc_amplitude[i] = mb_io_ptr->new_amp[i];
			}
		else if (mb_ping->mr_amplitude != NULL)
		for (i=0;i<mb_io_ptr->beams_amp;i++)
			{
			mb_ping->mr_amplitude[i] = mb_io_ptr->new_amp[i];
			}
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
