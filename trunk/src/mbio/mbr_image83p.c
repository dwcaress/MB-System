/*--------------------------------------------------------------------
 *    The MB-system:	mbr_image83p.c	5/5/2008
 *	$Id: mbr_image83p.c,v 5.0 2008-05-16 22:51:24 caress Exp $
 *
 *    Copyright (c) 2008 by
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
 * mbr_image83p.c contains the functions for reading and writing
 * multibeam data in the IMAGE83P format.  
 * These functions include:
 *   mbr_alm_image83p	- allocate read/write memory
 *   mbr_dem_image83p	- deallocate read/write memory
 *   mbr_rt_image83p	- read and translate data
 *   mbr_wt_image83p	- translate and write data
 *
 * Author:	Vivek Reddy, Santa Clara University
 * Date:	May 5, 2008
 *
 * $Log: not supported by cvs2svn $
 * 
 */
/*
 * Notes on the MBF_IMAGE83P data format:
 *   1. This data format is used to store 480 Beam Imagenex DeltaT multibeam bathymetry data
 *		The beam number can be variable but for now we'll only support the defualt setting
 *	 2. The data consist of 2176 byte records including 1-byte characters,
 *      2-byte integers, 4 byte integers, and null terminates strings
 *   3. The 480 depth values are stored in a seqence of 960 characters
 *   4. There is no provision for embedding comments in the data.
 *   
 * The mbf_image83p_data_struct structure is a direct representation  
 * of the ascii data structure used in the MBF_IMAGE83P format.
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
#include "../../include/mbsys_image83p.h"

/* time conversion constants */
#define MININYEAR 525600.0
#define MININDAY 1440.0

#define MBF_IMAGE83P_BUFFER_SIZE 2176

/* include for byte swapping on little-endian machines */
#ifdef BYTESWAPPED
#include "../../include/mb_swap.h"
#endif

/* essential function prototypes */
int mbr_register_image83p(int verbose, void *mbio_ptr, 
		int *error);
int mbr_info_image83p(int verbose, 
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
int mbr_alm_image83p(int verbose, void *mbio_ptr, int *error);
int mbr_dem_image83p(int verbose, void *mbio_ptr, int *error);
int mbr_rt_image83p(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_image83p(int verbose, void *mbio_ptr, void *store_ptr, int *error);

/*--------------------------------------------------------------------*/
int mbr_register_image83p(int verbose, void *mbio_ptr, int *error)
{
	static char res_id[]="$Id: mbr_image83p.c,v 5.0 2008-05-16 22:51:24 caress Exp $";
	char	*function_name = "mbr_register_image83p";
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
	status = mbr_info_image83p(verbose, 
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_image83p;
	mb_io_ptr->mb_io_format_free = &mbr_dem_image83p; 
	mb_io_ptr->mb_io_store_alloc = &mbsys_image83p_alloc; 
	mb_io_ptr->mb_io_store_free = &mbsys_image83p_deall; 
	mb_io_ptr->mb_io_read_ping = &mbr_rt_image83p; 
	mb_io_ptr->mb_io_write_ping = &mbr_wt_image83p; 
	mb_io_ptr->mb_io_dimensions = &mbsys_image83p_dimensions; 
	mb_io_ptr->mb_io_extract = &mbsys_image83p_extract; 
	mb_io_ptr->mb_io_insert = &mbsys_image83p_insert; 
	mb_io_ptr->mb_io_extract_nav = &mbsys_image83p_extract_nav; 
	mb_io_ptr->mb_io_insert_nav = &mbsys_image83p_insert_nav; 
	mb_io_ptr->mb_io_extract_altitude = &mbsys_image83p_extract_altitude; 
	mb_io_ptr->mb_io_insert_altitude = NULL; 
	mb_io_ptr->mb_io_extract_svp = NULL; 
	mb_io_ptr->mb_io_insert_svp = NULL; 
	mb_io_ptr->mb_io_ttimes = &mbsys_image83p_ttimes; 
	mb_io_ptr->mb_io_copyrecord = &mbsys_image83p_copy; 
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
		fprintf(stderr,"dbg2       svp_source:         %d\n",mb_io_ptr->svp_source);
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
int mbr_info_image83p(int verbose, 
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
	static char res_id[]="$Id: mbr_image83p.c,v 5.0 2008-05-16 22:51:24 caress Exp $";
	char	*function_name = "mbr_info_image83p";
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
	*system = MB_SYS_IMAGE83P;
	*beams_bath_max = 480;
	*beams_amp_max = 0;
	*pixels_ss_max = 0;
	strncpy(format_name, "IMAGE83P", MB_NAME_LENGTH);
	strncpy(system_name, "IMAGE83P", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_IMAGE83P\nInformal Description: Imagenex Multibeam\nAttributes:           Mulitbeam, bathymetry, 480 beams, ascii, Imagenex.\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_NO;
	*traveltime = MB_NO;
	*beam_flagging = MB_YES;
	*nav_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_NONE;
	*svp_source = MB_DATA_NONE;
	*beamwidth_xtrack = 0.75;
	*beamwidth_ltrack = 0.75;

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
int mbr_alm_image83p(int verbose, void *mbio_ptr, int *error)
{
 static char res_id[]="$Id: mbr_image83p.c,v 5.0 2008-05-16 22:51:24 caress Exp $";
	char	*function_name = "mbr_alm_image83p";
	int	status;
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
	status = mb_malloc(verbose,sizeof(struct mbsys_image83p_struct),
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
int mbr_dem_image83p(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_image83p";
	int	status;
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

	/* deallocate memory for data descriptor */
	status = mb_free(verbose,&mb_io_ptr->store_data,error);

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
int mbr_rt_image83p(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_image83p";
	int	status;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_image83p_struct *store;
	char	*datacomment;
	char	buffer[MBF_IMAGE83P_BUFFER_SIZE];
	int	done;
	int	id;
	int	count, temp_beam;
	int	index, swap;
	short short_val;
	unsigned int int_val;
	int	numberbytes, seconds_hundredths;
	double	degrees, minutes, dec_minutes;
	double	alpha, beta, theta, phi;
	double	soundspeed, rr, xx, zz;
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

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	store = (struct mbsys_image83p_struct *) store_ptr;
	swap = MB_NO;

	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* read next record header from file */
	done = MB_NO;
	for(i = 0; i < MBF_IMAGE83P_BUFFER_SIZE; i ++)
		buffer[i] = 0;
	if ((status = fread(buffer,1,6,mb_io_ptr->mbfp)) == 6)
		{
		/* check for valid header */
		if (strncmp(buffer,"83P",3) == 0)
			{
			done = MB_YES;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			}
		else
			{
			/* loop over reading bytes until valid header is found */
			while (done == MB_NO)
				{
				for (i=0;i<5;i++)
					buffer[i] = buffer[i+1];
				status = fread(&buffer[5],1,1,mb_io_ptr->mbfp);
				if (status != 1)
					{
					mb_io_ptr->file_bytes += status;
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
					done = MB_YES;
					}
				else if (strncmp(buffer,"83P",3) == 0)
					{
					done = MB_YES;
					}
				}
			}
		}
	else
		{
		mb_io_ptr->file_bytes += status;
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
    
	/* read rest of record from file */
	if (status == MB_SUCCESS)
		{
		index = 4;

		mb_get_binary_short(swap, &buffer[index], &short_val); 
		numberbytes = (int) ((unsigned short) short_val);

		if ((status = fread(&buffer[6],1,numberbytes-6,mb_io_ptr->mbfp)) != numberbytes-6)
			{
			mb_io_ptr->file_bytes += status;
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			store->kind = MB_DATA_NONE;
			}
		else
			{
			status = MB_SUCCESS;
			mb_io_ptr->file_bytes += status;
			}
		}
	
	/* if success then parse the buffer */
	if (status == MB_SUCCESS)
		{
		/* type of data record */

		store->kind = MB_DATA_DATA;

		/* parse year */
		index = 8;
		mb_get_int(&store->time_i[0], &buffer[index + 7], 4);
				
		/* parse month */
		if(buffer[index + 3] == 'J')
			{
			if(buffer[index + 4] == 'A')
				{
				store->time_i[1] = 1;
				}
			else if(buffer[index + 5] == 'N')
				{
				store->time_i[1] = 6;
				}
			else 
				{
				store->time_i[1] = 7;
				}
			}
		else if(buffer[index + 3] == 'F')
			{
			store->time_i[1] = 2;
			}
		else if(buffer[index + 3] == 'M')
			{
			if(buffer[index + 5] == 'R')
				{
				store->time_i[1] = 3;
				}
			else 
				{
				store->time_i[1] = 5;
				}
			}
		else if(buffer[index + 3] == 'A')
			{
			if(buffer[index + 4] == 'P')
				{
				store->time_i[1] = 4;
				}
			else 
				{
				store->time_i[1] = 8;
				}			
			}
		else if(buffer[index + 3] == 'S')
			{
			store->time_i[1] = 9;
			}
		else if(buffer[index + 3] == 'O')
			{
			store->time_i[1] = 10;
			}
		else if(buffer[index + 3] == 'N')
			{
			store->time_i[1] = 11;
			}
		else if(buffer[index + 3] == 'D')
			{
			store->time_i[1] = 12;
			}
	
		mb_get_int(&store->time_i[2], &buffer[index + 0], 2);
		index +=12; /*to time*/
		
		/* parse time */
		mb_get_int(&store->time_i[3], &buffer[index], 2);
		mb_get_int(&store->time_i[4], &buffer[index+3], 2);
		mb_get_int(&store->time_i[5], &buffer[index+6], 2);
		mb_get_int(&seconds_hundredths, &buffer[index+10], 2);
		store->time_i[6] = 10000 * seconds_hundredths;
		mb_get_time(verbose,store->time_i,&store->time_d);
		for (i=0;i<7;i++)
			{
			mb_io_ptr->new_time_i[i] = store->time_i[i];
			
			}
		mb_io_ptr->new_time_d = store->time_d;
		index += 13; /*to navigation latitude*/
		
		/* parse gps navigation string latitude */
		mb_get_double(&degrees, &buffer[index + 1], 2);
		mb_get_double(&minutes, &buffer[index + 4], 2);
		mb_get_double(&dec_minutes, &buffer[index + 7], 5);


		store->nav_lat = degrees + (((dec_minutes / 100000) + minutes) / 60);
		if(buffer[index + 13] == 'S' || buffer[index + 13] == 's')
			{
			store->nav_lat = -store->nav_lat;
			}
		index += 14; /*to navigation longtitude*/
	
		/* parse gps navigation string longtitude */
		mb_get_double(&degrees, &buffer[index], 3);
		mb_get_double(&minutes, &buffer[index + 4], 2);
		mb_get_double(&dec_minutes, &buffer[index + 7], 5);
		store->nav_long = degrees + (((dec_minutes / 100000) + minutes) / 60);
		if(buffer[index + 13] == 'W' || buffer[index + 13] == 'w')
			{
			store->nav_long = -store->nav_long;
			}
		index += 14;
		
		/* parse gps speed and heading */
		store->nav_speed = (int) ((mb_u_char)buffer[index]); index += 1;
		mb_get_binary_short(swap, &buffer[index], &short_val); index += 2;
		    store->nav_heading = (int) ((unsigned short) short_val);
		
		/* parse dvl attitude and heading */
		if (buffer[index] >> 7)
			{
			store->pitch = ((((mb_u_char)buffer[index]) & 0x7F) << 8) + ((mb_u_char)buffer[index+1]);
			}
		else
			{
			store->pitch = 0;
			}
		index += 2;
		if (buffer[index] >> 7)
			{
			store->roll = ((((mb_u_char)buffer[index]) & 0x7F) << 8) + ((mb_u_char)buffer[index+1]);
			}
		else
			{
			store->roll = 0;
			}
		index += 2;
		if (buffer[index] >> 7)
			{
			store->heading = ((((mb_u_char)buffer[index]) & 0x7F) << 8) + ((mb_u_char)buffer[index+1]);
			}
		else
			{
			store->heading = 0;
			}
		index += 2;
		
		/* parse beam info */
		mb_get_binary_short(swap, &buffer[index], &short_val); index += 2;
		    store->num_beams = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &buffer[index], &short_val); index += 2;
		    store->samples_per_beam = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &buffer[index], &short_val); index += 2;
		    store->sector_size = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &buffer[index], &short_val); index += 2;
		    store->start_angle = (int) ((unsigned short) short_val);
		store->angle_increment = (int) ((mb_u_char)buffer[index]); index += 1;
		mb_get_binary_short(swap, &buffer[index], &short_val); index += 2;
		    store->acoustic_range = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &buffer[index], &short_val); index += 2;
		    store->acoustic_frequency = (int) ((unsigned short) short_val);
		if (buffer[index] >> 7)
			{
			store->sound_velocity = ((((mb_u_char)buffer[index]) & 0x7F) << 8) + ((mb_u_char)buffer[index+1]);
			}
		else
			{
			store->sound_velocity = 15000;
			}
		index += 2;

		mb_get_binary_short(swap, &buffer[index], &short_val); index += 2;
		    store->range_resolution = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &buffer[index], &short_val); index += 2;
		    store->pulse_length = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &buffer[index], &short_val); index += 2;
		    store->profile_tilt_angle = (int) ((unsigned short) short_val);
		mb_get_binary_short(swap, &buffer[index], &short_val); index += 2;
		    store->rep_rate = (int) ((unsigned short) short_val);
		mb_get_binary_int(swap, &buffer[index], &int_val); index += 4;
		    store->ping_number = (int) ((unsigned int) int_val);
		index += 159;
		    
		/* get ranges */
		for (i=0;i<store->num_beams;i++)
			{
			mb_get_binary_short(swap, &buffer[index], &short_val); index += 2;
		    	store->range[i] = (int) ((unsigned short) short_val);
			}
		}
	mb_io_ptr->new_kind = store->kind;	
	mb_io_ptr->new_error = *error;
	
	/* if success then calculate bathymetry */
	if (status == MB_SUCCESS && store->kind == MB_DATA_DATA)
		{
		if (store->sound_velocity > 13000 && store->sound_velocity < 17000)
			soundspeed = 0.1 * store->sound_velocity;
		else
			soundspeed = 1500.0;
		for (i=0;i<store->num_beams;i++)
			{
			if (store->range[i] > 0)
				{
				alpha = 0.1 * (store->pitch - 900);
				beta = 270.0 - 0.01 * (store->start_angle + i * store->angle_increment) 
					+ 0.1 * (store->roll - 900);
				mb_rollpitch_to_takeoff(
					verbose, 
					alpha, beta, 
					&theta, &phi, 
					error);
				rr = (soundspeed / 1500.0) * 0.001 * store->range_resolution * store->range[i];
				xx = rr * sin(DTR * theta);
				zz = rr * cos(DTR * theta);
				store->bathacrosstrack[i] = xx * cos(DTR * phi);
				store->bathalongtrack[i] = xx * sin(DTR * phi);
				store->bath[i] = zz + 0.0;
				store->beamflag[i] = MB_FLAG_NONE;
				}
			else
				{
				store->beamflag[i] = MB_FLAG_NULL;
				store->bath[i] = 0.0;
				store->bathacrosstrack[i] = 0.0;
				store->bathalongtrack[i] = 0.0;
				}
			}
		}

	/* print output debug statements */
	if (verbose >= 1)
		{
		fprintf(stderr,"\ndbg2  Record read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg4  Data values:\n");
		fprintf(stderr,"dbg4       kind:               %d\n",store->kind);
		fprintf(stderr,"dbg4       time_i[0]:          %d\n",store->time_i[0]);
		fprintf(stderr,"dbg4       time_i[1]:          %d\n",store->time_i[1]);
		fprintf(stderr,"dbg4       time_i[2]:          %d\n",store->time_i[2]);
		fprintf(stderr,"dbg4       time_i[3]:          %d\n",store->time_i[3]);
		fprintf(stderr,"dbg4       time_i[4]:          %d\n",store->time_i[4]);
		fprintf(stderr,"dbg4       time_i[5]:          %d\n",store->time_i[5]);
		fprintf(stderr,"dbg4       time_i[6]:          %d\n",store->time_i[6]);
		fprintf(stderr,"dbg4       time_d:             %f\n",store->time_d);
		fprintf(stderr,"dbg4       nav_lat:            %f\n",store->nav_lat);
		fprintf(stderr,"dbg4       nav_long:           %f\n",store->nav_long);
		fprintf(stderr,"dbg4       nav_speed:          %d\n",store->nav_speed); /* 0.1 knots */
		fprintf(stderr,"dbg4       nav_heading:        %d\n",store->nav_heading); /*0.1 degrees */
		fprintf(stderr,"dbg4       pitch:              %d\n",store->pitch);
		fprintf(stderr,"dbg4       roll:               %d\n",store->roll);
		fprintf(stderr,"dbg4       heading:            %d\n",store->heading);
		fprintf(stderr,"dbg4       num_beams:          %d\n",store->num_beams);
		fprintf(stderr,"dbg4       samples_per_beam:   %d\n",store->samples_per_beam);
		fprintf(stderr,"dbg4       sector_size:        %d\n",store->sector_size); /* degrees */
		fprintf(stderr,"dbg4       start_angle:        %d\n",store->start_angle); /* 0.01 degrees + 180.0 */
		fprintf(stderr,"dbg4       angle_increment:    %d\n",store->angle_increment); /* 0.01 degrees */
		fprintf(stderr,"dbg4       acoustic_range:     %d\n",store->acoustic_range); /* meters */
		fprintf(stderr,"dbg4       acoustic_frequency: %d\n",store->acoustic_frequency); /* kHz */
		fprintf(stderr,"dbg4       sound_velocity:     %d\n",store->sound_velocity); /* 0.1 m/sec */
		fprintf(stderr,"dbg4       range_resolution:   %d\n",store->range_resolution); /* 0.001 meters */
		fprintf(stderr,"dbg4       pulse_length:       %d\n",store->pulse_length); /* usec */
		fprintf(stderr,"dbg4       profile_tilt_angle: %d\n",store->profile_tilt_angle); /* degrees + 180.0 */
		fprintf(stderr,"dbg4       rep_rate:           %d\n",store->rep_rate); /* msec */
		fprintf(stderr,"dbg4       ping_number:        %d\n",store->ping_number);
		for (i=0;i<store->num_beams;i++)
			fprintf(stderr,"dbg4       range:              %d\n",store->range[i]);
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
int mbr_wt_image83p(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_image83p";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_image83p_struct *store;
	int	i, j;
	int	id;
	int seconds_hundredths;
	double degrees, minutes, dec_minutes, remainder;
	char	buffer[MBF_IMAGE83P_BUFFER_SIZE];
	int swap ;
	int write_len, index;

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

	swap = MB_YES;
	
	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	store = (struct mbsys_image83p_struct *) store_ptr;
	
	

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Status at beginning of MBIO function <%s>\n",
			function_name);
		if (store != NULL)
			fprintf(stderr,"dbg5       store->kind:    %d\n",
				store->kind);
		fprintf(stderr,"dbg5       new_kind:       %d\n",
			mb_io_ptr->new_kind);
		fprintf(stderr,"dbg5       new_error:      %d\n",
			mb_io_ptr->new_error);
		fprintf(stderr,"dbg5       error:          %d\n",*error);
		fprintf(stderr,"dbg5       status:         %d\n",status);
		}


    
	/*  translate values from image83p data storage structure */
	if (store != NULL)
		{
		if (store->kind == MB_DATA_DATA)
			{
			/*put in header information*/
			index = 0;
			strncpy(buffer, "83P", 3);
			index = 4;
			write_len = MBF_IMAGE83P_BUFFER_SIZE;
			mb_put_binary_short(swap, (unsigned short)write_len, (void *) &buffer[index]);
			index = 8; /*date*/
			sprintf(&buffer[index], "%2.2d-", store->time_i[2]);
			switch(store->time_i[1]) {
			case (1)  : sprintf(&buffer[index + 3], "%s", "JAN-");
						break;
			case (2)  : sprintf(&buffer[index + 3], "%s", "FEB-");
						break;
			case (3)  : sprintf(&buffer[index + 3], "%s", "MAR-");
						break;
			case (4)  : sprintf(&buffer[index + 3], "%s", "APR-");
						break;
			case (5)  : sprintf(&buffer[index + 3], "%s", "MAY-");
						break;
			case (6)  : sprintf(&buffer[index + 3], "%s", "JUN-");
						break;
			case (7)  : sprintf(&buffer[index + 3], "%s", "JUL-");
						break;
			case (8)  : sprintf(&buffer[index + 3], "%s", "AUG-");
						break;
			case (9)  : sprintf(&buffer[index + 3], "%s", "SEP-");
						break;
			case (10)  :sprintf(&buffer[index + 3], "%s", "OCT-");
						break;
			case (11) : sprintf(&buffer[index + 3], "%s", "NOV-");
						break;
			case (12) : sprintf(&buffer[index + 3],"%s", "DEC-");
						break;
			}
			sprintf(&buffer[index + 7], "%2.2d", store->time_i[0]); 
			
			index = 20; /*time*/
			sprintf(&buffer[index], "%2.2d:%2.2d:%2.2d", store->time_i[3], store->time_i[4], store->time_i[5]);
			
			index = 29; /*hundredths of seconds*/
			buffer[index] = '.';
			seconds_hundredths = store->time_i[6] / 10000;
			for (i=0;i<7;i++)
			{
			mb_io_ptr->new_time_i[i] = store->time_i[i];
			}
			mb_io_ptr->new_time_d = store->time_d;
			mb_put_binary_short(swap, (unsigned short)seconds_hundredths, (void *) &buffer[index + 1]);
			
			index = 33; /*GPS Latitude*/
			sprintf(&buffer[index], "_%2.2d", (int)store->nav_lat);
			sprintf(&buffer[index + 3], "%s", ".");
			remainder = store->nav_lat - (int)store->nav_lat;
			minutes = remainder * 60;
			sprintf(&buffer[index + 4], "%2.2d", (int)minutes);
			remainder = minutes - (int)minutes;
			dec_minutes = remainder * 100000;
			sprintf(&buffer[index + 6], "%s", ".");
			sprintf(&buffer[index + 7], "%5.5d", (int)dec_minutes);
			if(store->nav_lat < 0.0)
			{
				sprintf(&buffer[index + 13], "%s", "S");
			}
			else
			{
				sprintf(&buffer[index + 13], "%s", "N");
			}
			index = 47; /*GPS Longtitue*/
			sprintf(&buffer[index], "%3.3d", (int)store->nav_long);
			sprintf(&buffer[index + 3], "%s", ".");
			remainder = store->nav_long - (int)store->nav_long;
			minutes = remainder * 60;
			sprintf(&buffer[index + 4], "%2.2d", (int)minutes);
			remainder = minutes - (int)minutes;
			dec_minutes = remainder * 100000;
			sprintf(&buffer[index + 6], "%s", ".");
			sprintf(&buffer[index + 7], "%5.5d", (int)dec_minutes);
			if(store->nav_long < 0.0)
			{
				sprintf(&buffer[index + 13], "%s", "W");
			}
			else
			{
				sprintf(&buffer[index + 13], "%s", "E");
			}

			index = 61; /*nav speed*/
			buffer[index] = store->nav_speed;
			index = 62; /*nav heading*/
			mb_put_binary_short(swap, (unsigned short)store->nav_heading, (void *) &buffer[index]);
			index = 64; /*pitch*/
			if(store->pitch != 0)
			{
				mb_put_binary_short(swap, (unsigned short)store->pitch, (void *)&buffer[index]);
				buffer[index] = buffer[index] | 0x80;
			}
			index = 66; /*roll*/
			if(store->roll != 0)
			{
				mb_put_binary_short(swap, (unsigned short)store->roll, (void *)&buffer[index]);
				buffer[index] = buffer[index] | 0x80;
			}
			index = 68; /*heading*/
			if(store->heading != 0)
			{
				mb_put_binary_short(swap, (unsigned short)store->heading, (void *)&buffer[index]);
				buffer[index] = buffer[index] | 0x80;
			}
			index = 70; /*beams*/
			mb_put_binary_short(swap, (unsigned short)store->num_beams, (void *)&buffer[index]);
			index += 2;
			mb_put_binary_short(swap, (unsigned short)store->samples_per_beam, (void *)&buffer[index]);
			index += 2;
			mb_put_binary_short(swap, (unsigned short)store->sector_size, (void *)&buffer[index]);
			index += 2;
			mb_put_binary_short(swap, (unsigned short)store->start_angle, (void *)&buffer[index]);
			index += 2;
			buffer[index] = store->angle_increment; index+=1;
			mb_put_binary_short(swap, (unsigned short)store->acoustic_range, (void *)&buffer[index]);
			index += 2;
			mb_put_binary_short(swap, (unsigned short)store->acoustic_frequency, (void *)&buffer[index]);
			index += 2;
			if(store->sound_velocity != 0)
			{
				mb_put_binary_short(swap, (unsigned short)store->sound_velocity, (void *)&buffer[index]);
				buffer[index] = buffer[index] | 0x80;
			}
			index += 2;
			mb_put_binary_short(swap, (unsigned short)store->range_resolution, (void *)&buffer[index]);
			index += 2;
			mb_put_binary_short(swap, (unsigned short)store->profile_tilt_angle, (void *)&buffer[index]);
			index += 2;
			mb_put_binary_short(swap, (unsigned short)store->rep_rate, (void *)&buffer[index]);
			index += 2;
			mb_put_binary_int(swap, store->ping_number, (void *)&buffer[index]);
			index = 256;
			/* get ranges */
			for (i=0;i<store->num_beams;i++)
			{
			mb_put_binary_short(swap, (unsigned short)store->range[i], &buffer[index + (i * 2)]); 
			}
			
			
			}
		}

	/* write next record to file */
	if (store->kind == MB_DATA_DATA
		|| store->kind == MB_DATA_COMMENT)
		{
		if ((status = fwrite(buffer,1,write_len,
			mb_io_ptr->mbfp)) 
			== write_len) 
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
	else
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		if (verbose >= 5)
			fprintf(stderr,"\ndbg5  No data written in MBIO function <%s>\n",
				function_name);
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
