/*--------------------------------------------------------------------
 *    The MB-system:	mbr_hsunknwn.c	10/13/2008
 *	$Id$
 *
 *    Copyright (c) 2008-2009 by
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
 * mbr_hsunknwn.c contains the functions for reading and writing
 * multibeam data in the HSUNKNWN format.  
 * These functions include:
 *   mbr_alm_hsunknwn	- allocate read/write memory
 *   mbr_dem_hsunknwn	- deallocate read/write memory
 *   mbr_rt_hsunknwn	- read and translate data
 *   mbr_wt_hsunknwn	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	October 13, 2008
 *
 * $Log: mbr_hsunknwn.c,v $
 * Revision 5.0  2008/10/17 07:28:41  caress
 * Supporting Hydrosweep DS data in format used by SOPAC.
 *
 *
 *--------------------------------------------------------------------
 *
 * Some notes on the data format:
 *
 * According to Brian Bishop of SOPAC (email 12 October 2008),
 * data in this format derive from joint Japanese & SOPAC surveys
 * from 1991 through 2005 from an unknown ship and sonar.
 * Because the number of beams is 59, I suppose that the sonar
 * was likely an Atlas Hydrosweep DS, and treat the data as
 * such.
 * 
 * Table AP5-4 Format of MBES Files (2000-2005) 
 *
 * Line No, Item                                             Format Column 
 * 1    1   Blank                                            4X      1:4 
 *      2   Date (Year/Month/Day: YYYYMMDD)                  I8      4:12 
 *      3   Blank                                            1X      13:13 
 *      4   Time (Hour/Minute/Second: HHMMSS)                I6      14:19 
 *      5   Longitude of Center (Degree)                     F12.7   20:31 
 *      6   Latitude of Center (Degree)                      F12.7   32:43 
 *      7   Dummy Data                                       2F8.1   44:59 
 *      8   Azimuth (Heading: Degree)                        F9.3    60:68 
 *      9   Water Depth of Center (m)                        F9.3    69:78 
 * 2        Water Depth (X= -29 - 0 - +29)                   59F7.1  1:411 
 * 3        Horizontal Distance (X= -29 - 0 - +29)           59F7.1  1:411 
 * 4        Acoustic Reflection Intensity (X= -29 - 0 - +29) 59F7.1  1:411 
 * 5        Dummy Data                                       59F7.1  1:411 
 * 6        Dummy Data                                       59F7.1  1:411 
 * Repeat Line 1 to 6
 *
 * Note: the data we saw has the first four characters of the first line
 * as " M  " rather than blank and has 7 null bytes before a <cr> at the
 * end of the first line, after the center water depth.
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
#include "../../include/mbsys_hsds.h"

#define LINE1SIZE 87
#define LINE2SIZE 415

/* essential function prototypes */
int mbr_register_hsunknwn(int verbose, void *mbio_ptr, 
		int *error);
int mbr_info_hsunknwn(int verbose, 
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
int mbr_alm_hsunknwn(int verbose, void *mbio_ptr, int *error);
int mbr_dem_hsunknwn(int verbose, void *mbio_ptr, int *error);
int mbr_rt_hsunknwn(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_hsunknwn(int verbose, void *mbio_ptr, void *store_ptr, int *error);

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/
int mbr_register_hsunknwn(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_register_hsunknwn";
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
	status = mbr_info_hsunknwn(verbose, 
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_hsunknwn;
	mb_io_ptr->mb_io_format_free = &mbr_dem_hsunknwn; 
	mb_io_ptr->mb_io_store_alloc = &mbsys_hsds_alloc; 
	mb_io_ptr->mb_io_store_free = &mbsys_hsds_deall; 
	mb_io_ptr->mb_io_read_ping = &mbr_rt_hsunknwn; 
	mb_io_ptr->mb_io_write_ping = &mbr_wt_hsunknwn; 
	mb_io_ptr->mb_io_dimensions = &mbsys_hsds_dimensions; 
	mb_io_ptr->mb_io_extract = &mbsys_hsds_extract; 
	mb_io_ptr->mb_io_insert = &mbsys_hsds_insert; 
	mb_io_ptr->mb_io_extract_nav = &mbsys_hsds_extract_nav; 
	mb_io_ptr->mb_io_insert_nav = &mbsys_hsds_insert_nav; 
	mb_io_ptr->mb_io_extract_altitude = &mbsys_hsds_extract_altitude; 
	mb_io_ptr->mb_io_insert_altitude = NULL; 
	mb_io_ptr->mb_io_extract_svp = NULL; 
	mb_io_ptr->mb_io_insert_svp = NULL; 
	mb_io_ptr->mb_io_ttimes = &mbsys_hsds_ttimes; 
	mb_io_ptr->mb_io_copyrecord = &mbsys_hsds_copy; 
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
int mbr_info_hsunknwn(int verbose, 
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
	char	*function_name = "mbr_info_hsunknwn";
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
	*system = MB_SYS_HSDS;
	*beams_bath_max = 59;
	*beams_amp_max = 59;
	*pixels_ss_max = 0;
	strncpy(format_name, "HSUNKNWN", MB_NAME_LENGTH);
	strncpy(system_name, "HSDS", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_HSUNKNWN\nInformal Description: Unknown Hydrosweep\nAttributes:           Hydrosweep DS, bathymetry, 59 beams, ascii, unknown origin, SOPAC.\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_NO;
	*traveltime = MB_NO;
	*beam_flagging = MB_YES;
	*nav_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_DATA;
	*svp_source = MB_DATA_NONE;
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
int mbr_alm_hsunknwn(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_alm_hsunknwn";
	int	status;
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
	status = mb_malloc(verbose,sizeof(struct mbsys_hsds_struct),
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
int mbr_dem_hsunknwn(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_hsunknwn";
	int	status;
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
int mbr_rt_hsunknwn(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_hsunknwn";
	int	status;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hsds_struct *store;
	char	line[MB_PATH_MAXLINE];
	char	*result;
	double	value;
	int	i;

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

	/* get pointer to raw data structure */
	store = (struct mbsys_hsds_struct *) store_ptr;

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* read first line of next record from file */
	if ((status = fread(line,1,LINE1SIZE,mb_io_ptr->mbfp)) == LINE1SIZE)
		{
		mb_io_ptr->file_bytes += status;
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		
		/* if comment just read the next line */
		if (strncmp(line, "COMM", 4) == 0)
			{
			store->kind = MB_DATA_COMMENT;
			result = fgets(store->comment,MB_PATH_MAXLINE,mb_io_ptr->mbfp);
			if (result == NULL)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				}
			else
				{
				mb_io_ptr->file_bytes += strlen(store->comment);
				store->comment[strlen(store->comment)-1] = 0;
				store->comment[strlen(store->comment)-1] = 0;
				}
			}
			
		else
			{		
			store->kind = MB_DATA_DATA;

			mb_get_int(    &(store->year),         line+ 4,  4);
			mb_get_int(    &(store->month),        line+ 8,  2);
			mb_get_int(    &(store->day),          line+10,  2);
			mb_get_int(    &(store->hour),         line+13,  2);
			mb_get_int(    &(store->minute),       line+15,  2);
			mb_get_int(    &(store->second),       line+17,  2);
			mb_get_double( &(store->lon),          line+19,  12);
			mb_get_double( &(store->lat),          line+31,  12);

			mb_get_double( &(store->course_true),  line+59,  9);
			mb_get_double( &(store->depth_center), line+68,  9);

			store->depth_scale = 0.1;
			store->back_scale = 0.1;
			}
		}
	else
		{
		mb_io_ptr->file_bytes += status;
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* read second line of next record from file */
	if (status == MB_SUCCESS && store->kind == MB_DATA_DATA)
		{
		if ((status = fread(line,1,LINE2SIZE,mb_io_ptr->mbfp)) == LINE2SIZE)
			{
			mb_io_ptr->file_bytes += status;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;

			for (i=0;i<MBSYS_HSDS_BEAMS;i++)
				{
				mb_get_double( &value,          line+i*7,  7);
				store->depth[i] = (int)(10.0 * value);
				}
			}
		else
			{
			mb_io_ptr->file_bytes += status;
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}

		/* read third line of next record from file */
		if ((status = fread(line,1,LINE2SIZE,mb_io_ptr->mbfp)) == LINE2SIZE)
			{
			mb_io_ptr->file_bytes += status;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;

			for (i=0;i<MBSYS_HSDS_BEAMS;i++)
				{
				mb_get_double( &value,          line+i*7,  7);
				store->distance[i] = (int)(10.0 * value);
				}
			}
		else
			{
			mb_io_ptr->file_bytes += status;
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}

		/* read fourth line of next record from file */
		if ((status = fread(line,1,LINE2SIZE,mb_io_ptr->mbfp)) == LINE2SIZE)
			{
			mb_io_ptr->file_bytes += status;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;

			for (i=0;i<MBSYS_HSDS_BEAMS;i++)
				{
				mb_get_double( &value,          line+i*7,  7);
				store->back[i] = (int)(10.0 * value);
				}
			}
		else
			{
			mb_io_ptr->file_bytes += status;
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}

		/* read fifth line of next record from file */
		if ((status = fread(line,1,LINE2SIZE,mb_io_ptr->mbfp)) == LINE2SIZE)
			{
			mb_io_ptr->file_bytes += status;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;

			for (i=0;i<MBSYS_HSDS_BEAMS;i++)
				{
				mb_get_double( &value,          line+i*7,  7);
				}
			}
		else
			{
			mb_io_ptr->file_bytes += status;
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}

		/* read sixth line of next record from file */
		if ((status = fread(line,1,LINE2SIZE,mb_io_ptr->mbfp)) == LINE2SIZE)
			{
			mb_io_ptr->file_bytes += status;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;

			for (i=0;i<MBSYS_HSDS_BEAMS;i++)
				{
				mb_get_double( &value,          line+i*7,  7);
				}
			}
		else
			{
			mb_io_ptr->file_bytes += status;
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}

	/* set kind and error in mb_io_ptr */
	mb_io_ptr->new_kind = store->kind;
	mb_io_ptr->new_error = *error;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       kind:             %d\n",store->kind);
		if (store->kind == MB_DATA_DATA)
			{
			fprintf(stderr,"dbg5       lon:              %f\n",store->lon);
			fprintf(stderr,"dbg5       lat:              %f\n",store->lat);
			fprintf(stderr,"dbg5       year:             %d\n",store->year);
			fprintf(stderr,"dbg5       month:            %d\n",store->month);
			fprintf(stderr,"dbg5       day:              %d\n",store->day);
			fprintf(stderr,"dbg5       hour:             %d\n",store->hour);
			fprintf(stderr,"dbg5       minute:           %d\n",store->minute);
			fprintf(stderr,"dbg5       second:           %d\n",store->second);
			fprintf(stderr,"dbg5       course_true:      %f\n",store->course_true);
			fprintf(stderr,"dbg5       depth_center:     %f\n",store->depth_center);
			fprintf(stderr,"dbg5       depth_scale:      %f\n",store->depth_scale);
			fprintf(stderr,"dbg5       back_scale:       %f\n",store->back_scale);
			fprintf(stderr,"dbg5       beam distance depth back:\n");
			for (i=0;i<MBSYS_HSDS_BEAMS;i++)
				fprintf(stderr,"dbg5         %d  %d  %d  %d\n",
					i,store->distance[i],store->depth[i],store->back[i]);
			}
		else if (store->kind == MB_DATA_COMMENT)
			{
			fprintf(stderr,"dbg5       comment: %s\n",store->comment);
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
int mbr_wt_hsunknwn(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_hsunknwn";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hsds_struct *store;
	double	value;
	int	i;

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

	/* get pointer to raw data structure */
	store = (struct mbsys_hsds_struct *) store_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Status at beginning of MBIO function <%s>\n",function_name);
		if (store != NULL)
			fprintf(stderr,"dbg5       store->kind:    %d\n",store->kind);
		fprintf(stderr,"dbg5       new_kind:       %d\n",mb_io_ptr->new_kind);
		fprintf(stderr,"dbg5       new_error:      %d\n",mb_io_ptr->new_error);
		fprintf(stderr,"dbg5       error:          %d\n",*error);
		fprintf(stderr,"dbg5       status:         %d\n",status);
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       kind:             %d\n",store->kind);
		if (store->kind == MB_DATA_DATA)
			{
			fprintf(stderr,"dbg5       lon:              %f\n",store->lon);
			fprintf(stderr,"dbg5       lat:              %f\n",store->lat);
			fprintf(stderr,"dbg5       year:             %d\n",store->year);
			fprintf(stderr,"dbg5       month:            %d\n",store->month);
			fprintf(stderr,"dbg5       day:              %d\n",store->day);
			fprintf(stderr,"dbg5       hour:             %d\n",store->hour);
			fprintf(stderr,"dbg5       minute:           %d\n",store->minute);
			fprintf(stderr,"dbg5       second:           %d\n",store->second);
			fprintf(stderr,"dbg5       course_true:      %f\n",store->course_true);
			fprintf(stderr,"dbg5       depth_center:     %f\n",store->depth_center);
			fprintf(stderr,"dbg5       depth_scale:      %f\n",store->depth_scale);
			fprintf(stderr,"dbg5       back_scale:       %f\n",store->back_scale);
			fprintf(stderr,"dbg5       beam distance depth back:\n");
			for (i=0;i<MBSYS_HSDS_BEAMS;i++)
				fprintf(stderr,"dbg5         %d  %d  %d  %d\n",
					i,store->distance[i],store->depth[i],store->back[i]);
			}
		else if (store->kind == MB_DATA_COMMENT)
			{
			fprintf(stderr,"dbg5       comment: %s\n",store->comment);
			}
		}

	/* write comment record to file */
	if (store->kind ==MB_DATA_COMMENT)
		{
		status = fprintf(mb_io_ptr->mbfp,"COMM                                                                                 \r\n");
		if (status > 0)
			status = fprintf(mb_io_ptr->mbfp,"%s\r\n",store->comment);
		if (status > 0) 
			{
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			}
		else
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;
			}
		}
		
	/* write data record to file */
	else if (store->kind == MB_DATA_DATA)
		{
		fprintf(mb_io_ptr->mbfp," M  %4.4d%2.2d%2.2d %2.2d%2.2d%2.2d%12.7f%12.7f%8.1f%8.1f%9.3f%9.3f        \r\n",
			store->year,store->month,store->day,
			store->hour,store->minute,store->second,
			store->lon,store->lat,0.0,0.0,store->course_true,
			store->depth_center);
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			{
			value = store->depth[i] * store->depth_scale;
			fprintf(mb_io_ptr->mbfp,"%7.1f",value);
			}
		fprintf(mb_io_ptr->mbfp,"\r\n");
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			{
			value = store->distance[i] * store->depth_scale;
			fprintf(mb_io_ptr->mbfp,"%7.1f",value);
			}
		fprintf(mb_io_ptr->mbfp,"\r\n");
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			{
			value = store->back[i] * store->back_scale;
			fprintf(mb_io_ptr->mbfp,"%7.1f",value);
			}
		fprintf(mb_io_ptr->mbfp,"\r\n");
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			{
			value = -9999.9;
			fprintf(mb_io_ptr->mbfp,"%7.1f",value);
			}
		fprintf(mb_io_ptr->mbfp,"\r\n");
		for (i=0;i<MBSYS_HSDS_BEAMS;i++)
			{
			value = 100.0;
			fprintf(mb_io_ptr->mbfp,"%7.1f",value);
			}
		fprintf(mb_io_ptr->mbfp,"\r\n");
		}
		
	else
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		if (verbose >= 5)
			fprintf(stderr,"\ndbg5  No data written in MBIO function <%s>\n",function_name);
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
