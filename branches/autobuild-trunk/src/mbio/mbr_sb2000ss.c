/* Added HAVE_CONFIG_H for autogen files */
#ifdef HAVE_CONFIG_H
#  include <mbsystem_config.h>
#endif


/*--------------------------------------------------------------------
 *    The MB-system:	mbr_sb2000ss.c	10/14/94
 *	$Id: mbr_sb2000ss.c 1907 2011-11-10 04:33:03Z caress $
 *
 *    Copyright (c) 1994-2011 by
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
 * mbr_sb2000ss.c contains the functions for reading and writing
 * multibeam data in the SB2000SS format.  
 * These functions include:
 *   mbr_alm_sb2000ss	- allocate read/write memory
 *   mbr_dem_sb2000ss	- deallocate read/write memory
 *   mbr_rt_sb2000ss	- read and translate data
 *   mbr_wt_sb2000ss	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	October 14, 1994
 * $Log: mbr_sb2000ss.c,v $
 * Revision 5.13  2005/11/05 00:48:05  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.12  2003/05/20 18:05:32  caress
 * Added svp_source to data source parameters.
 *
 * Revision 5.11  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.10  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.9  2002/04/08 21:29:49  caress
 * Removed errant debug message.
 *
 * Revision 5.8  2002/04/08 20:59:38  caress
 * Release 5.0.beta17
 *
 * Revision 5.7  2002/02/26 07:50:41  caress
 * Release 5.0.beta14
 *
 * Revision 5.6  2002/02/22 09:03:43  caress
 * Release 5.0.beta13
 *
 * Revision 5.5  2001/07/20 00:32:54  caress
 * Release 5.0.beta03
 *
 * Revision 5.4  2001/04/06  22:05:59  caress
 * Consolidated xse formats into one format.
 *
 * Revision 5.3  2001/03/22  20:50:02  caress
 * Trying to make version 5.0.beta0
 *
 * Revision 5.1  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.9  2000/10/11  01:03:21  caress
 * Convert to ANSI C
 *
 * Revision 4.8  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.7  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.6  1997/07/25  14:19:53  caress
 * Version 4.5beta2.
 * Much mucking, particularly with Simrad formats.
 *
 * Revision 4.5  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.5  1997/04/17  15:07:36  caress
 * MB-System 4.5 Beta Release
 *
 * Revision 4.4  1996/04/22  13:21:19  caress
 * Now have DTR and MIN/MAX defines in mb_define.h
 *
 * Revision 4.3  1995/03/06  19:38:54  caress
 * Changed include strings.h to string.h for POSIX compliance.
 *
 * Revision 4.2  1994/12/21  20:21:09  caress
 * Changes to support high resolution SeaBeam 2000 sidescan files
 * from R/V Melville data.
 *
 * Revision 4.0  1994/10/21  12:34:58  caress
 * Release V4.0
 *
 * Revision 1.1  1994/10/21  12:20:01  caress
 * Initial revision
 *
 *
 */
/*--------------------------------------------------------------------
 * Notes on the MBF_SB2000SS data format:
 *   1. This data format is used to store sidescan data from 
 *      Sea Beam 2000 sonars.
 *      This format was created and used by the Scripps
 *      Institution of Oceanography; most data files in this format
 *      consist of Sea Beam data collected on the R/V George Melville.
 *      This format is one of the "swathbathy" formats created by
 *      Jim Charters of Scripps.
 *   2. The data records consist of three logical records: the header
 *      record, the sensor specific record and the data record.  
 *   3. The header record consists of 36 bytes, including the sizes
 *      of the following sensor specific and data records.
 *   4. The sensor specific records are 32 bytes long.  
 *   5. The data record lengths are variable.
 *   6. Comments are included in text records, which are of variable
 *      length.
 *   7. Information on this format was obtained from the Geological
 *      Data Center and the Shipboard Computer Group at the Scripps 
 *      Institution of Oceanography
 *
 * The kind value in the mbf_sb2000ss_struct indicates whether the
 * mbf_sb2000ss_data_struct structure holds data (kind = 1) or an
 * ascii comment record (kind = 2).
 */
 
/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_define.h"
#include "mbsys_sb2000.h"

/* include for byte swapping on little-endian machines */
#ifdef BYTESWAPPED
#include "mb_swap.h"
#endif

/* essential function prototypes */
int mbr_register_sb2000ss(int verbose, void *mbio_ptr, 
		int *error);
int mbr_info_sb2000ss(int verbose, 
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
int mbr_alm_sb2000ss(int verbose, void *mbio_ptr, int *error);
int mbr_dem_sb2000ss(int verbose, void *mbio_ptr, int *error);
int mbr_rt_sb2000ss(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_sb2000ss(int verbose, void *mbio_ptr, void *store_ptr, int *error);

static char rcs_id[]="$Id: mbr_sb2000ss.c 1907 2011-11-10 04:33:03Z caress $";

/*--------------------------------------------------------------------*/
int mbr_register_sb2000ss(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_register_sb2000ss";
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
	status = mbr_info_sb2000ss(verbose, 
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_sb2000ss;
	mb_io_ptr->mb_io_format_free = &mbr_dem_sb2000ss; 
	mb_io_ptr->mb_io_store_alloc = &mbsys_sb2000_alloc; 
	mb_io_ptr->mb_io_store_free = &mbsys_sb2000_deall; 
	mb_io_ptr->mb_io_read_ping = &mbr_rt_sb2000ss; 
	mb_io_ptr->mb_io_write_ping = &mbr_wt_sb2000ss; 
	mb_io_ptr->mb_io_dimensions = &mbsys_sb2000_dimensions; 
	mb_io_ptr->mb_io_extract = &mbsys_sb2000_extract; 
	mb_io_ptr->mb_io_insert = &mbsys_sb2000_insert; 
	mb_io_ptr->mb_io_extract_nav = &mbsys_sb2000_extract_nav; 
	mb_io_ptr->mb_io_insert_nav = &mbsys_sb2000_insert_nav; 
	mb_io_ptr->mb_io_extract_altitude = &mbsys_sb2000_extract_altitude; 
	mb_io_ptr->mb_io_insert_altitude = NULL; 
	mb_io_ptr->mb_io_extract_svp = NULL; 
	mb_io_ptr->mb_io_insert_svp = NULL; 
	mb_io_ptr->mb_io_ttimes = &mbsys_sb2000_ttimes; 
	mb_io_ptr->mb_io_detects = &mbsys_sb2000_detects; 
	mb_io_ptr->mb_io_copyrecord = &mbsys_sb2000_copy; 
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
		fprintf(stderr,"dbg2       detects:            %lu\n",(size_t)mb_io_ptr->mb_io_detects);
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
int mbr_info_sb2000ss(int verbose, 
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
	char	*function_name = "mbr_info_sb2000ss";
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
	*system = MB_SYS_SB2000;
	*beams_bath_max = 0;
	*beams_amp_max = 0;
	*pixels_ss_max = 2000;
	strncpy(format_name, "SB2000SS", MB_NAME_LENGTH);
	strncpy(system_name, "SB2000", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_SB2000SS\nInformal Description: SIO Swath-bathy SeaBeam 2000 format\nAttributes:           SeaBeam 2000, sidescan,\n                      1000 pixels for 4-bit sidescan,\n                      2000 pixels for 12+-bit sidescan,\n                      binary,  SIO.\n", MB_DESCRIPTION_LENGTH);
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
int mbr_alm_sb2000ss(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_alm_sb2000ss";
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
	status = mb_malloc(verbose,sizeof(struct mbsys_sb2000_struct),
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
int mbr_dem_sb2000ss(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_sb2000ss";
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

	/* deallocate memory for data descriptor */
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
int mbr_rt_sb2000ss(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_sb2000ss";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_sb2000_struct *store;
	int	read_status;
	char	buffer[2*MBSYS_SB2000_PIXELS+4];
	unsigned short *short_ptr;
	short	test_sensor_size, test_data_size;
	int 	found, skip;
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
	store = (struct mbsys_sb2000_struct *) store_ptr;

	/* read next header record from file */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;
	skip = 0;
	found = MB_NO;
	if ((status = fread(buffer,1,MBSYS_SB2000_HEADER_SIZE,
			mb_io_ptr->mbfp)) == MBSYS_SB2000_HEADER_SIZE) 
		{
		mb_io_ptr->file_bytes += status;
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		
		/* check if header is ok */
		if (strncmp(&buffer[34],"SR",2) == 0
		    || strncmp(&buffer[34],"RS",2) == 0
		    || strncmp(&buffer[34],"SP",2) == 0
		    || strncmp(&buffer[34],"TR",2) == 0
		    || strncmp(&buffer[34],"IR",2) == 0
		    || strncmp(&buffer[34],"AT",2) == 0
		    || strncmp(&buffer[34],"SC",2) == 0)
		    {
		    mb_get_binary_short(MB_NO, &buffer[26], &test_sensor_size);
		    mb_get_binary_short(MB_NO, &buffer[28], &test_data_size);
		    if (test_sensor_size <= 32 && test_data_size <= 2*MBSYS_SB2000_PIXELS+4)
			found = MB_YES;
		    }
		}
	else
		{
		mb_io_ptr->file_bytes += status;
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}

	/* if not a good header search through file to find one */
	while (status == MB_SUCCESS && found == MB_NO)
		{
		/* shift bytes by one */
		for (i=0;i<MBSYS_SB2000_HEADER_SIZE-1;i++)
			buffer[i] = buffer[i+1];
		mb_io_ptr->file_pos += 1;
		skip++;

		/* read next byte */
		if ((read_status = fread(&buffer[MBSYS_SB2000_HEADER_SIZE-1],
			1,1,mb_io_ptr->mbfp)) == 1) 
			{
			mb_io_ptr->file_bytes += read_status;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			
			if (strncmp(&buffer[34],"SR",2) == 0
			    || strncmp(&buffer[34],"RS",2) == 0
			    || strncmp(&buffer[34],"SP",2) == 0
			    || strncmp(&buffer[34],"TR",2) == 0
			    || strncmp(&buffer[34],"IR",2) == 0
			    || strncmp(&buffer[34],"AT",2) == 0
			    || strncmp(&buffer[34],"SC",2) == 0)
			    {
			    mb_get_binary_short(MB_NO, &buffer[26], &test_sensor_size);
			    mb_get_binary_short(MB_NO, &buffer[28], &test_data_size);
			    if (test_sensor_size <= 32 && test_data_size <= 2*MBSYS_SB2000_PIXELS+4)
				found = MB_YES;
			    }

			}
		else
			{
			found = MB_YES;
			mb_io_ptr->file_bytes += status;
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}
		
	/* report data skips */
	if (skip > 0 && verbose >= 2) 
	    fprintf(stderr, "\ndgb2           DATA SKIPPED: %d bytes\n", skip);

	/* get header values */
	mb_get_binary_short(MB_NO, &buffer[0], &store->year);
	mb_get_binary_short(MB_NO, &buffer[2], &store->day);
	mb_get_binary_short(MB_NO, &buffer[4], &store->min);
	mb_get_binary_short(MB_NO, &buffer[6], &store->sec);
	mb_get_binary_int(MB_NO, &buffer[8], &store->lat);
	mb_get_binary_int(MB_NO, &buffer[12], &store->lon);
	mb_get_binary_short(MB_NO, &buffer[16], &store->heading);
	mb_get_binary_short(MB_NO, &buffer[18], &store->course);
	mb_get_binary_short(MB_NO, &buffer[20], &store->speed);
	mb_get_binary_short(MB_NO, &buffer[22], &store->speed_ps);
	mb_get_binary_short(MB_NO, &buffer[24], &store->quality);
	mb_get_binary_short(MB_NO, &buffer[26], (short *)&store->sensor_size);
	mb_get_binary_short(MB_NO, &buffer[28], (short *)&store->data_size);
	store->speed_ref[0] = buffer[30];
	store->speed_ref[1] = buffer[31];
	store->sensor_type[0] = buffer[32];
	store->sensor_type[1] = buffer[33];
	store->data_type[0] = buffer[34];
	store->data_type[1] = buffer[35];

	/* check for unintelligible records */
	if (status == MB_SUCCESS)
		{
		if ((strncmp(store->sensor_type,"SS",2) != 0 || 
			strncmp(store->data_type,"SC",2) != 0)
			&& strncmp(store->data_type,"TR",2) != 0
			&& strncmp(store->data_type,"SP",2) != 0)
			{
			/* read rest of record */
			for (i=0;
			    (i < store->sensor_size + store->data_size)
				&& status == MB_SUCCESS;
			    i++)
			    {
			    if ((read_status = fread(buffer,1,1,
				mb_io_ptr->mbfp)) != 1) 
				{
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				store->kind = MB_DATA_NONE;
				}
			    else
				{
				mb_io_ptr->file_bytes += read_status;
				}
			    }
			    
			/* if eof not reached set unintelligible error */
			if (status == MB_SUCCESS)
			    {
			    status = MB_FAILURE;
			    *error = MB_ERROR_UNINTELLIGIBLE;
			    store->kind = MB_DATA_NONE;
			    }
			}
		else if (strncmp(store->data_type,"SC",2) == 0)
			{
			store->kind = MB_DATA_DATA;
			}
		else if (strncmp(store->data_type,"SP",2) == 0)
			{
			store->kind = MB_DATA_VELOCITY_PROFILE;
			}
		else
			{
			store->kind = MB_DATA_COMMENT;
			}
		}

	/* fix incorrect header records */
	if (status == MB_SUCCESS 
		&& store->kind == MB_DATA_DATA 
		&& store->data_size == 1000)
		{
		store->sensor_size = 32;
		store->data_size = 1001;
		}

	/* print debug statements */
	if (status == MB_SUCCESS && verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  New header record in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5  New header values:\n");
		fprintf(stderr,"dbg5       year:       %d\n",store->year);
		fprintf(stderr,"dbg5       day:        %d\n",store->day);
		fprintf(stderr,"dbg5       min:        %d\n",store->min);
		fprintf(stderr,"dbg5       sec:        %d\n",store->sec);
		fprintf(stderr,"dbg5       lat:        %d\n",store->lat);
		fprintf(stderr,"dbg5       lon:        %d\n",store->lon);
		fprintf(stderr,"dbg5       heading:    %d\n",store->heading);
		fprintf(stderr,"dbg5       course:     %d\n",store->course);
		fprintf(stderr,"dbg5       speed:      %d\n",store->speed);
		fprintf(stderr,"dbg5       speed_ps:   %d\n",store->speed_ps);
		fprintf(stderr,"dbg5       quality:    %d\n",store->quality);
		fprintf(stderr,"dbg5       sensor size:%d\n",store->sensor_size);
		fprintf(stderr,"dbg5       data size:  %d\n",store->data_size);
		fprintf(stderr,"dbg5       speed_ref:  %c%c\n",
			store->speed_ref[0],store->speed_ref[1]);
		fprintf(stderr,"dbg5       sensor_type:%c%c\n",
			store->sensor_type[0],store->sensor_type[1]);
		fprintf(stderr,"dbg5       data_type:  %c%c\n",
			store->data_type[0],store->data_type[1]);
		}

	/* read sensor record from file */
	if (status == MB_SUCCESS && store->sensor_size > 0)
		{
		if ((status = fread(buffer,1,store->sensor_size,
			mb_io_ptr->mbfp)) == store->sensor_size) 
			{
			mb_io_ptr->file_bytes += status;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			}
		else
			{
			mb_io_ptr->file_bytes += status;
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}

	/* extract sensor data */
	if (status == MB_SUCCESS && store->sensor_size > 0)
		{
		/* extract the values */
		mb_get_binary_int(MB_NO, &buffer[0], &store->ping_number);
		mb_get_binary_short(MB_NO, &buffer[4], &store->ping_length);
		mb_get_binary_short(MB_NO, &buffer[6], &store->pixel_size);
		mb_get_binary_short(MB_NO, &buffer[8], &store->ss_min);
		mb_get_binary_short(MB_NO, &buffer[10], &store->ss_max);
		mb_get_binary_short(MB_NO, &buffer[12], &store->sample_rate);
		mb_get_binary_short(MB_NO, &buffer[14], &store->start_time);
		mb_get_binary_short(MB_NO, &buffer[16], &store->tot_slice);
		mb_get_binary_short(MB_NO, &buffer[18], &store->pixels_ss);
		for (i=0;i<store->sensor_size - 20;i++)
			store->spare_ss[i] = buffer[18+i];
		}

	/* read data record from file */
	if (status == MB_SUCCESS 
		&& store->data_size > 0)
		{
		if ((status = fread(buffer,1,store->data_size,
			mb_io_ptr->mbfp)) == store->data_size) 
			{
			mb_io_ptr->file_bytes += status;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			}
		else
			{
			mb_io_ptr->file_bytes += status;
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		}

	/* extract sidescan data */
	if (status == MB_SUCCESS 
		&& store->kind == MB_DATA_DATA)
		{

		/* fix some files with incorrect sensor records */
		if (buffer[0] == 'G'
			&& (store->data_size == 1001
			    || store->data_size == 1004)
			&& store->pixels_ss != 1000)
			{
			store->pixels_ss = 1000;
			}

		/* correct data size if needed */
		if (buffer[0] == 'G' && store->data_size == 1001)
		    {
		    store->data_size = 1004;
		    store->ss[1001] = 'G';
		    store->ss[1002] = 'G';
		    store->ss[1003] = 'G';
		    }

		/* fix some files with incorrect data size id's */
		if (buffer[0] == 'R'
			&& (2 * store->pixels_ss) > store->data_size
			&& store->pixels_ss <= MBSYS_SB2000_PIXELS)
			{
			buffer[0] = 'G';
			}
		    
		/* deal with 1-byte data */
		if (buffer[0] == 'G')
		    {
		    store->ss_type = 'G';
		    for (i=0;i<store->pixels_ss;i++)
			{
			store->ss[i] = buffer[i+1];
			}
		    }
		    
		/* deal with 2-byte data */
		else if (buffer[0] == 'R')
		    {
		    store->ss_type = 'R';
		    for (i=0;i<store->pixels_ss;i++)
			{
			mb_get_binary_short(MB_NO, (short *)&(buffer[4+2*i]), 
					    (short *)&(store->ss[2*i]));
			}
		    }
	
		/* print debug statements */
		if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg5  New data record read by MBIO function <%s>\n",function_name);
			fprintf(stderr,"dbg5  New data values:\n");
			fprintf(stderr,"dbg5       ping_number:     %d\n",
				store->ping_number);
			fprintf(stderr,"dbg5       ping_length:     %d\n",
				store->ping_length);
			fprintf(stderr,"dbg5       pixel_size:      %d\n",
				store->pixel_size);
			fprintf(stderr,"dbg5       ss_min:          %d\n",
				store->ss_min);
			fprintf(stderr,"dbg5       ss_max:          %d\n",
				store->ss_max);
			fprintf(stderr,"dbg5       sample_rate:     %d\n",
				store->sample_rate);
			fprintf(stderr,"dbg5       start_time:      %d\n",
				store->start_time);
			fprintf(stderr,"dbg5       tot_slice:       %d\n",
				store->tot_slice);
			fprintf(stderr,"dbg5       pixels_ss:       %d\n",
				store->pixels_ss);
			fprintf(stderr,"dbg5       spare_ss:        ");
			for (i=0;i<12;i++)
				fprintf(stderr,"%c", store->spare_ss[i]);
			fprintf(stderr,"dbg5       sidescan_type:%c\n",
				store->ss_type);
			if (store->ss_type == 'G')
				{
				for (i=0;i<store->pixels_ss;i++)
					fprintf(stderr,"dbg5       pixel: %d  ss: %d\n",
						i,store->ss[i]);
				}
			else if (store->ss_type == 'R')
				{
				for (i=1;i<=store->pixels_ss;i++)
					{
					short_ptr = (unsigned short *) &store->ss[i*2];
					fprintf(stderr,"dbg5       pixel: %d  ss: %d\n",
						i,*short_ptr);
					}
				}
			}
		}

	/* extract velocity profile record */
	if (status == MB_SUCCESS 
		&& store->kind == MB_DATA_VELOCITY_PROFILE)
		{
		/* extract the values */
		mb_get_binary_int(MB_NO, &buffer[0], &store->svp_mean);
		mb_get_binary_short(MB_NO, &buffer[4], &store->svp_number);
		mb_get_binary_short(MB_NO, &buffer[6], &store->svp_spare);
		for (i=0;i<MIN(store->svp_number, 30);i++)
			{
			mb_get_binary_short(MB_NO, &buffer[8+i*4], &store->svp_depth[i]);
			mb_get_binary_short(MB_NO, &buffer[10+i*4], &store->svp_vel[i]);
			}
		mb_get_binary_short(MB_NO, &buffer[128], &store->vru1);
		mb_get_binary_short(MB_NO, &buffer[130], &store->vru1_port);
		mb_get_binary_short(MB_NO, &buffer[132], &store->vru1_forward);
		mb_get_binary_short(MB_NO, &buffer[134], &store->vru1_vert);
		mb_get_binary_short(MB_NO, &buffer[136], &store->vru2);
		mb_get_binary_short(MB_NO, &buffer[138], &store->vru2_port);
		mb_get_binary_short(MB_NO, &buffer[140], &store->vru2_forward);
		mb_get_binary_short(MB_NO, &buffer[142], &store->vru2_vert);
	
		/* print debug statements */
		if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg5  New svp record read by MBIO function <%s>\n",function_name);
			fprintf(stderr,"dbg5  New svp values:\n");
			fprintf(stderr,"dbg5       svp_mean:     %d\n",
				store->svp_mean);
			fprintf(stderr,"dbg5       svp_number:   %d\n",
				store->svp_number);
			fprintf(stderr,"dbg5       svp_spare:    %d\n",
				store->svp_spare);
			for (i=0;i<30;i++)
				fprintf(stderr,"dbg5       %d  depth: %d  vel: %d\n",
					i,store->svp_depth[i],
					store->svp_vel[i]);
			fprintf(stderr,"dbg5       vru1:         %d\n",
				store->vru1);
			fprintf(stderr,"dbg5       vru1_port:    %d\n",
				store->vru1_port);
			fprintf(stderr,"dbg5       vru1_forward: %d\n",
				store->vru1_forward);
			fprintf(stderr,"dbg5       vru1_vert:    %d\n",
				store->vru1_vert);
			fprintf(stderr,"dbg5       vru2:         %d\n",
				store->vru2);
			fprintf(stderr,"dbg5       vru2_port:    %d\n",
				store->vru2_port);
			fprintf(stderr,"dbg5       vru2_forward: %d\n",
				store->vru2_forward);
			fprintf(stderr,"dbg5       vru2_vert:    %d\n",
				store->vru2_vert);
			fprintf(stderr,"dbg5       pitch_bias:    %d\n",
				store->pitch_bias);
			fprintf(stderr,"dbg5       roll_bias:    %d\n",
				store->roll_bias);
			fprintf(stderr,"dbg5       vru:          %c%c%c%c%c%c%c%c\n",
				store->vru[0],store->vru[1],store->vru[2],store->vru[3],
				store->vru[4],store->vru[5],store->vru[6],store->vru[7]);
			}
		}

	/* extract comment record */
	if (status == MB_SUCCESS && store->kind == MB_DATA_COMMENT)
		{
		strncpy(store->comment, buffer, 
			MIN(store->data_size, MBSYS_SB2000_COMMENT_LENGTH-1));
		store->comment[MIN(store->data_size, 
			MBSYS_SB2000_COMMENT_LENGTH-1)] = '\0';

		/* print debug statements */
		if (verbose >= 5)
			{
			fprintf(stderr,"\ndbg5  New comment record read by MBIO function <%s>\n",function_name);
			fprintf(stderr,"dbg5  New comment:\n");
			fprintf(stderr,"dbg5       comment:   %s\n",
				store->comment);
			}
		}

	/* set kind and error in mb_io_ptr */
	mb_io_ptr->new_kind = store->kind;
	mb_io_ptr->new_error = *error;

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
int mbr_wt_sb2000ss(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_sb2000ss";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_sb2000_struct *store;
	char	buffer[2*MBSYS_SB2000_PIXELS+4];
	unsigned short *short_ptr;
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
	store = (struct mbsys_sb2000_struct *) store_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Ready to write data in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       kind:       %d\n",
			store->kind);
		fprintf(stderr,"dbg5       error:      %d\n",*error);
		fprintf(stderr,"dbg5       status:     %d\n",status);
		}

	/* print debug statements */
	if (status == MB_SUCCESS && verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Header record to be written by MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5  Header values:\n");
		fprintf(stderr,"dbg5       year:       %d\n",store->year);
		fprintf(stderr,"dbg5       day:        %d\n",store->day);
		fprintf(stderr,"dbg5       min:        %d\n",store->min);
		fprintf(stderr,"dbg5       sec:        %d\n",store->sec);
		fprintf(stderr,"dbg5       lat:        %d\n",store->lat);
		fprintf(stderr,"dbg5       lon:        %d\n",store->lon);
		fprintf(stderr,"dbg5       heading:    %d\n",store->heading);
		fprintf(stderr,"dbg5       course:     %d\n",store->course);
		fprintf(stderr,"dbg5       speed:      %d\n",store->speed);
		fprintf(stderr,"dbg5       speed_ps:   %d\n",store->speed_ps);
		fprintf(stderr,"dbg5       quality:    %d\n",store->quality);
		fprintf(stderr,"dbg5       sensor size:%d\n",store->sensor_size);
		fprintf(stderr,"dbg5       data size:  %d\n",store->data_size);
		fprintf(stderr,"dbg5       speed_ref:  %c%c\n",
			store->speed_ref[0],store->speed_ref[1]);
		fprintf(stderr,"dbg5       sensor_type:%c%c\n",
			store->sensor_type[0],store->sensor_type[1]);
		fprintf(stderr,"dbg5       data_type:  %c%c\n",
			store->data_type[0],store->data_type[1]);
		}

	/* print debug statements */
	if (status == MB_SUCCESS && verbose >= 5 
		&& store->kind == MB_DATA_DATA)
		{
		fprintf(stderr,"\ndbg5  Sensor record to be written by MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5  Sensor values:\n");
		fprintf(stderr,"dbg5       ping_number:     %d\n",
			store->ping_number);
		fprintf(stderr,"dbg5       ping_length:     %d\n",
			store->ping_length);
		fprintf(stderr,"dbg5       pixel_size:      %d\n",
			store->pixel_size);
		fprintf(stderr,"dbg5       ss_min:          %d\n",
			store->ss_min);
		fprintf(stderr,"dbg5       ss_max:          %d\n",
			store->ss_max);
		fprintf(stderr,"dbg5       sample_rate:     %d\n",
			store->sample_rate);
		fprintf(stderr,"dbg5       start_time:      %d\n",
			store->start_time);
		fprintf(stderr,"dbg5       tot_slice:       %d\n",
			store->tot_slice);
		fprintf(stderr,"dbg5       pixels_ss:       %d\n",
			store->pixels_ss);
		fprintf(stderr,"dbg5       spare_ss:        ");
		for (i=0;i<12;i++)
			fprintf(stderr,"%c", store->spare_ss[i]);
		fprintf(stderr, "\n");
		}

	/* print debug statements */
	if (status == MB_SUCCESS && verbose >= 5 
		&& store->kind == MB_DATA_VELOCITY_PROFILE)
		{
		fprintf(stderr,"\ndbg5  SVP record to be written by MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5  SVP values:\n");
		fprintf(stderr,"dbg5       svp_mean:     %d\n",
			store->svp_mean);
		fprintf(stderr,"dbg5       svp_number:   %d\n",
			store->svp_number);
		fprintf(stderr,"dbg5       svp_spare:   %d\n",
			store->svp_spare);
		for (i=0;i<30;i++)
			fprintf(stderr,"dbg5       %d  depth: %d  vel: %d\n",
				i,store->svp_depth[i],
				store->svp_vel[i]);
		fprintf(stderr,"dbg5       vru1:         %d\n",
			store->vru1);
		fprintf(stderr,"dbg5       vru1_port:    %d\n",
			store->vru1_port);
		fprintf(stderr,"dbg5       vru1_forward: %d\n",
			store->vru1_forward);
		fprintf(stderr,"dbg5       vru1_vert:    %d\n",
			store->vru1_vert);
		fprintf(stderr,"dbg5       vru2:         %d\n",
			store->vru2);
		fprintf(stderr,"dbg5       vru2_port:    %d\n",
			store->vru2_port);
		fprintf(stderr,"dbg5       vru2_forward: %d\n",
			store->vru2_forward);
		fprintf(stderr,"dbg5       vru2_vert:    %d\n",
			store->vru2_vert);
		fprintf(stderr,"dbg5       pitch_bias:    %d\n",
			store->pitch_bias);
		fprintf(stderr,"dbg5       roll_bias:    %d\n",
			store->roll_bias);
		fprintf(stderr,"dbg5       vru:          %c%c%c%c%c%c%c%c\n",
			store->vru[0],store->vru[1],store->vru[2],store->vru[3],
			store->vru[4],store->vru[5],store->vru[6],store->vru[7]);
		}

	/* print debug statements */
	if (status == MB_SUCCESS && verbose >= 5 
		&& store->kind == MB_DATA_DATA)
		{
		fprintf(stderr,"\ndbg5  Data record to be written by MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5  Data values:\n");
		fprintf(stderr,"dbg5       sidescan_type:%c\n",
			store->ss_type);
		if (store->ss_type == 'G')
			{
			for (i=0;i<store->pixels_ss;i++)
				fprintf(stderr,"dbg5       pixel: %d  ss: %d\n",
					i,store->ss[i]);
			}
		else if (store->ss_type == 'R')
			{
			for (i=1;i<=store->pixels_ss;i++)
				{
				short_ptr = (unsigned short *) &store->ss[i*2];
				fprintf(stderr,"dbg5       pixel: %d  ss: %d\n",
					i,*short_ptr);
				}
			}
		}

	/* print debug statements */
	if (status == MB_SUCCESS && verbose >= 5 
		&& store->kind == MB_DATA_COMMENT)
		{
		fprintf(stderr,"\ndbg5  Comment record to be written by MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5  Comment:\n");
		fprintf(stderr,"dbg5       comment:   %s\n",
			store->comment);
		}

	/* put header values */
	if (status == MB_SUCCESS)
		{
		/* put header values */
		mb_put_binary_short(MB_NO, store->year, &buffer[0]);
		mb_put_binary_short(MB_NO, store->day, &buffer[2]);
		mb_put_binary_short(MB_NO, store->min, &buffer[4]);
		mb_put_binary_short(MB_NO, store->sec, &buffer[6]);
		mb_put_binary_int(MB_NO, store->lat, &buffer[8]);
		mb_put_binary_int(MB_NO, store->lon, &buffer[12]);
		mb_put_binary_short(MB_NO, store->heading, &buffer[16]);
		mb_put_binary_short(MB_NO, store->course, &buffer[18]);
		mb_put_binary_short(MB_NO, store->speed, &buffer[20]);
		mb_put_binary_short(MB_NO, store->speed_ps, &buffer[22]);
		mb_put_binary_short(MB_NO, store->quality, &buffer[24]);
		mb_put_binary_short(MB_NO, store->sensor_size, &buffer[26]);
		mb_put_binary_short(MB_NO, store->data_size, &buffer[28]);
		buffer[30] = store->speed_ref[0];
		buffer[31] = store->speed_ref[1];
		buffer[32] = store->sensor_type[0];
		buffer[33] = store->sensor_type[1];
		buffer[34] = store->data_type[0];
		buffer[35] = store->data_type[1];

		/* write header record to file */
		if ((status = fwrite(buffer,1,MBSYS_SB2000_HEADER_SIZE,
			mb_io_ptr->mbfp)) == MBSYS_SB2000_HEADER_SIZE) 
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
			
	/* put sensor data */
	if (status == MB_SUCCESS 
		&& store->kind == MB_DATA_DATA
		&& store->sensor_size > 0)
		{
		/* put sensor values */
		mb_put_binary_int(MB_NO, store->ping_number, &buffer[0]);
		mb_put_binary_short(MB_NO, store->ping_length, &buffer[4]);
		mb_put_binary_short(MB_NO, store->pixel_size, &buffer[6]);
		mb_put_binary_short(MB_NO, store->ss_min, &buffer[8]);
		mb_put_binary_short(MB_NO, store->ss_max, &buffer[10]);
		mb_put_binary_short(MB_NO, store->sample_rate, &buffer[12]);
		mb_put_binary_short(MB_NO, store->start_time, &buffer[14]);
		mb_put_binary_short(MB_NO, store->tot_slice, &buffer[16]);
		mb_put_binary_short(MB_NO, store->pixels_ss, &buffer[18]);
		for (i=0;i<store->sensor_size - 20;i++)
			buffer[18+i] = store->spare_ss[i];

		/* write sensor record to file */
		if ((status = fwrite(buffer,1,store->sensor_size,
			mb_io_ptr->mbfp)) == store->sensor_size) 
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

	if (status == MB_SUCCESS 
		&& store->kind == MB_DATA_VELOCITY_PROFILE
		&& store->data_size > 0)
		{
		/* extract the values */
		mb_put_binary_int(MB_NO, store->svp_mean, &buffer[0]);
		mb_put_binary_short(MB_NO, store->svp_number, &buffer[4]);
		mb_put_binary_short(MB_NO, store->svp_spare, &buffer[6]);
		for (i=0;i<MIN(store->svp_number, 30);i++)
			{
			mb_put_binary_short(MB_NO, store->svp_depth[i], &buffer[8+i*4]);
			mb_put_binary_short(MB_NO, store->svp_vel[i], &buffer[10+i*4]);
			}
		mb_put_binary_short(MB_NO, store->vru1, &buffer[128]);
		mb_put_binary_short(MB_NO, store->vru1_port, &buffer[130]);
		mb_put_binary_short(MB_NO, store->vru1_forward, &buffer[132]);
		mb_put_binary_short(MB_NO, store->vru1_vert, &buffer[134]);
		mb_put_binary_short(MB_NO, store->vru2, &buffer[136]);
		mb_put_binary_short(MB_NO, store->vru2_port, &buffer[138]);
		mb_put_binary_short(MB_NO, store->vru2_forward, &buffer[140]);
		mb_put_binary_short(MB_NO, store->vru2_vert, &buffer[142]);

		/* write svp profile */
		if ((status = fwrite(buffer,1,store->data_size,
			mb_io_ptr->mbfp)) == store->data_size) 
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

	if (status == MB_SUCCESS 
		&& store->kind == MB_DATA_DATA
		&& store->data_size > 0)
		{
		/* put the values */
		    
		/* deal with 1-byte data */
		if (store->ss_type == 'G')
		    {
		    buffer[0] = 'G';
		    for (i=0;i<store->pixels_ss;i++)
			{
			buffer[i+1] = store->ss[i];
			}
		    buffer[store->pixels_ss+1] = 'G';
		    buffer[store->pixels_ss+2] = 'G';
		    buffer[store->pixels_ss+3] = 'G';
		    }
		    
		/* deal with 2-byte data */
		else if (store->ss_type == 'R')
		    {
		    buffer[0] = 'R';
		    buffer[1] = 'R';
		    buffer[2] = 'R';
		    buffer[3] = 'R';
		    for (i=0;i<store->pixels_ss;i++)
			{
			mb_get_binary_short(MB_NO, (short *)&(store->ss[2*i]), 
					    (short *)&(buffer[4+2*i]));
			}
		    }

		/* write survey data */
		if ((status = fwrite(buffer,1,store->data_size,
			mb_io_ptr->mbfp)) == store->data_size) 
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

	if (status == MB_SUCCESS 
		&& store->kind == MB_DATA_COMMENT
		&& store->data_size > 0)
		{
		/* put the comment */
		strncpy(buffer, store->comment,
			MIN(store->data_size, MBSYS_SB2000_COMMENT_LENGTH-1));
		buffer[MIN(store->data_size, 
			MBSYS_SB2000_COMMENT_LENGTH-1)] = '\0';

		/* write comment */
		if ((status = fwrite(buffer,1,store->data_size,
			mb_io_ptr->mbfp)) == store->data_size) 
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
