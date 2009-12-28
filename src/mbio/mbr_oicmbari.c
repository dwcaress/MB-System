/*--------------------------------------------------------------------
 *    The MB-system:	mbr_oicgeoda.c	2/16/99
 *	$Id$
 *
 *    Copyright (c) 1999-2009 by
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
 * mbr_oicmbari.c contains the functions for reading and writing
 * multibeam data in the MBF_OICMBARI format.  
 * These functions include:
 *   mbr_alm_oicmbari	- allocate read/write memory
 *   mbr_dem_oicmbari	- deallocate read/write memory
 *   mbr_rt_oicmbari	- read and translate data
 *   mbr_wt_oicmbari	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	February 16, 1999
 *
 * $Log: mbr_oicmbari.c,v $
 * Revision 5.10  2008/07/10 18:02:39  caress
 * Proceeding towards 5.1.1beta20.
 *
 * Revision 5.7  2005/11/05 00:48:04  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.6  2003/05/20 18:05:32  caress
 * Added svp_source to data source parameters.
 *
 * Revision 5.5  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.4  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.3  2001/07/20 00:32:54  caress
 * Release 5.0.beta03
 *
 * Revision 5.2  2001/03/22  20:50:02  caress
 * Trying to make version 5.0.beta0
 *
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
 * Revision 4.2  1999/12/29  00:34:06  caress
 * Release 4.6.8
 *
 * Revision 4.1  1999/04/07  20:38:24  caress
 * Fixes related to building under Linux.
 *
 * Revision 4.1  1999/04/03 07:36:16  caress
 * Fix bugs in byteswapped code.
 *
 * Revision 4.0  1999/03/31 18:29:20  caress
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
#include "../../include/mbf_oicmbari.h"
#include "../../include/mbsys_oic.h"

/* include for byte swapping on little-endian machines */
#ifdef BYTESWAPPED
#include "../../include/mb_swap.h"
#endif

/* essential function prototypes */
int mbr_register_oicmbari(int verbose, void *mbio_ptr, 
		int *error);
int mbr_info_oicmbari(int verbose, 
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
int mbr_alm_oicmbari(int verbose, void *mbio_ptr, int *error);
int mbr_dem_oicmbari(int verbose, void *mbio_ptr, int *error);
int mbr_rt_oicmbari(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_oicmbari(int verbose, void *mbio_ptr, void *store_ptr, int *error);

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/
int mbr_register_oicmbari(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_register_oicmbari";
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
	status = mbr_info_oicmbari(verbose, 
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_oicmbari;
	mb_io_ptr->mb_io_format_free = &mbr_dem_oicmbari; 
	mb_io_ptr->mb_io_store_alloc = &mbsys_oic_alloc; 
	mb_io_ptr->mb_io_store_free = &mbsys_oic_deall; 
	mb_io_ptr->mb_io_read_ping = &mbr_rt_oicmbari; 
	mb_io_ptr->mb_io_write_ping = &mbr_wt_oicmbari; 
	mb_io_ptr->mb_io_dimensions = &mbsys_oic_dimensions; 
	mb_io_ptr->mb_io_extract = &mbsys_oic_extract; 
	mb_io_ptr->mb_io_insert = &mbsys_oic_insert; 
	mb_io_ptr->mb_io_extract_nav = &mbsys_oic_extract_nav; 
	mb_io_ptr->mb_io_insert_nav = &mbsys_oic_insert_nav; 
	mb_io_ptr->mb_io_extract_altitude = &mbsys_oic_extract_altitude; 
	mb_io_ptr->mb_io_insert_altitude = &mbsys_oic_insert_altitude; 
	mb_io_ptr->mb_io_extract_svp = NULL; 
	mb_io_ptr->mb_io_insert_svp = NULL; 
	mb_io_ptr->mb_io_ttimes = &mbsys_oic_ttimes; 
	mb_io_ptr->mb_io_copyrecord = &mbsys_oic_copy; 
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
		fprintf(stderr,"dbg2       format_alloc:       %ld\n",(size_t)mb_io_ptr->mb_io_format_alloc);
		fprintf(stderr,"dbg2       format_free:        %ld\n",(size_t)mb_io_ptr->mb_io_format_free);
		fprintf(stderr,"dbg2       store_alloc:        %ld\n",(size_t)mb_io_ptr->mb_io_store_alloc);
		fprintf(stderr,"dbg2       store_free:         %ld\n",(size_t)mb_io_ptr->mb_io_store_free);
		fprintf(stderr,"dbg2       read_ping:          %ld\n",(size_t)mb_io_ptr->mb_io_read_ping);
		fprintf(stderr,"dbg2       write_ping:         %ld\n",(size_t)mb_io_ptr->mb_io_write_ping);
		fprintf(stderr,"dbg2       extract:            %ld\n",(size_t)mb_io_ptr->mb_io_extract);
		fprintf(stderr,"dbg2       insert:             %ld\n",(size_t)mb_io_ptr->mb_io_insert);
		fprintf(stderr,"dbg2       extract_nav:        %ld\n",(size_t)mb_io_ptr->mb_io_extract_nav);
		fprintf(stderr,"dbg2       insert_nav:         %ld\n",(size_t)mb_io_ptr->mb_io_insert_nav);
		fprintf(stderr,"dbg2       extract_altitude:   %ld\n",(size_t)mb_io_ptr->mb_io_extract_altitude);
		fprintf(stderr,"dbg2       insert_altitude:    %ld\n",(size_t)mb_io_ptr->mb_io_insert_altitude);
		fprintf(stderr,"dbg2       extract_svp:        %ld\n",(size_t)mb_io_ptr->mb_io_extract_svp);
		fprintf(stderr,"dbg2       insert_svp:         %ld\n",(size_t)mb_io_ptr->mb_io_insert_svp);
		fprintf(stderr,"dbg2       ttimes:             %ld\n",(size_t)mb_io_ptr->mb_io_ttimes);
		fprintf(stderr,"dbg2       extract_rawss:      %ld\n",(size_t)mb_io_ptr->mb_io_extract_rawss);
		fprintf(stderr,"dbg2       insert_rawss:       %ld\n",(size_t)mb_io_ptr->mb_io_insert_rawss);
		fprintf(stderr,"dbg2       copyrecord:         %ld\n",(size_t)mb_io_ptr->mb_io_copyrecord);
		fprintf(stderr,"dbg2       error:              %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		}

	/* return status */
	return(status);
}

/*--------------------------------------------------------------------*/
int mbr_info_oicmbari(int verbose, 
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
	char	*function_name = "mbr_info_oicmbari";
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
	*system = MB_SYS_OIC;
	*beams_bath_max = 1024;
	*beams_amp_max = 256;
	*pixels_ss_max = 2048;
	strncpy(format_name, "OICMBARI", MB_NAME_LENGTH);
	strncpy(system_name, "OIC", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_OICMBARI\nInformal Description: OIC-style extended swath sonar format\nAttributes:           variable beam bathymetry and\n                      amplitude, variable pixel sidescan, binary,\n		      MBARI\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_YES;
	*traveltime = MB_YES;
	*beam_flagging = MB_YES;
	*nav_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_DATA;
	*svp_source = MB_DATA_NONE;
	*beamwidth_xtrack = 0.0;
	*beamwidth_ltrack = 0.0;

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
int mbr_alm_oicmbari(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_alm_oicmbari";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_oicmbari_struct *dataplus;
	struct mbf_oicmbari_header_struct *header;
	struct mbf_oicmbari_data_struct *data;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %ld\n",(size_t)mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* set initial status */
	status = MB_SUCCESS;

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = sizeof(struct mbf_oicmbari_struct);
	status = mb_mallocd(verbose,__FILE__,__LINE__,mb_io_ptr->structure_size,
				(void **)&mb_io_ptr->raw_data,error);
	dataplus = (struct mbf_oicmbari_struct *) mb_io_ptr->raw_data;
	header = &(dataplus->header);
	data = &(dataplus->data);
	header->num_chan = 0;
	header->beams_bath = 0;
	header->beams_amp = 0;
	header->bath_chan_port = -1;
	header->bath_chan_stbd = -1;
	header->pixels_ss = 0;
	header->ss_chan_port = -1;
	header->ss_chan_stbd = -1;
	for (i=0;i<MBF_OICMBARI_MAX_CHANNELS;i++)
	    {
	    header->channel[i].offset = 0;
	    header->channel[i].num_samples = 0;
	    data->rawsize[i] = 0;
	    data->raw[i] = NULL;
	    }
	data->beams_bath_alloc = 0;
	data->beams_amp_alloc = 0;
	data->pixels_ss_alloc = 0;
	data->beamflag = NULL;
	data->bath = NULL;
	data->amp = NULL;
	data->bathacrosstrack = NULL;
	data->bathalongtrack = NULL;
	data->tt = NULL;
	data->angle = NULL;
	data->ss = NULL;
	data->ssacrosstrack = NULL;
	data->ssalongtrack = NULL;
	status = mbsys_oic_alloc(verbose,mbio_ptr,
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
int mbr_dem_oicmbari(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_oicmbari";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_oicmbari_struct *dataplus;
	struct mbf_oicmbari_header_struct *header;
	struct mbf_oicmbari_data_struct *data;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %ld\n",(size_t)mbio_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	dataplus = (struct mbf_oicmbari_struct *) mb_io_ptr->raw_data;
	header = &(dataplus->header);
	data = &(dataplus->data);

	/* deallocate memory for data descriptor */
	for (i=0;i<MBF_OICMBARI_MAX_CHANNELS;i++)
	    {
	    if (data->raw[i] != NULL)
		status = mb_freed(verbose,__FILE__, __LINE__, (void **)&(data->raw[i]),error);
	    }
	if (data->beamflag != NULL)
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&(data->beamflag),error);
	if (data->bath != NULL)
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&(data->bath),error);
	if (data->amp != NULL)
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&(data->amp),error);
	if (data->bathacrosstrack != NULL)
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&(data->bathacrosstrack),error);
	if (data->bathalongtrack != NULL)
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&(data->bathalongtrack),error);
	if (data->tt != NULL)
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&(data->tt),error);
	if (data->angle != NULL)
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&(data->angle),error);
	if (data->ss != NULL)
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&(data->ss),error);
	if (data->ssacrosstrack != NULL)
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&(data->ssacrosstrack),error);
	if (data->ssalongtrack != NULL)
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&(data->ssalongtrack),error);
	status = mb_freed(verbose,__FILE__, __LINE__, (void **)&mb_io_ptr->raw_data,error);
	status = mbsys_oic_deall(verbose,mbio_ptr,
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
int mbr_rt_oicmbari(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_oicmbari";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_oicmbari_struct *dataplus;
	struct mbf_oicmbari_header_struct *header;
	struct mbf_oicmbari_data_struct *data;
	struct mbsys_oic_struct *store;
	char	buffer[MBF_OICMBARI_HEADER_SIZE];
	char	*comment;
	int	read_size;
	int	data_size;
	char	*char_ptr;
	short	*short_ptr;
	int	*int_ptr;
	float	*float_ptr;
	int	index;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %ld\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %ld\n",(size_t)store_ptr);
		}

	/* get pointer to mbio descriptor and data structure */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	store = (struct mbsys_oic_struct *) store_ptr;

	/* get pointer to raw data structure */
	dataplus = (struct mbf_oicmbari_struct *) mb_io_ptr->raw_data;
	header = &(dataplus->header);
	data = &(dataplus->data);
	comment = dataplus->client;
	
	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* read next four bytes to look for start of header */
	if ((read_size = fread(buffer,1,4,mb_io_ptr->mbfp)) != 4)
	    {
	    status = MB_FAILURE;
	    *error = MB_ERROR_EOF;		    
	    }
	    
	/* read another byted at a time until header found */
	while (status == MB_SUCCESS
	    && (buffer[0] != 'G'
		|| buffer[1] != 'E'
		|| buffer[2] != '2'))
	    {
	    for (i=0;i<3;i++)
		buffer[i] = buffer[i+1];
	    if ((read_size = fread(&buffer[3],1,1,mb_io_ptr->mbfp)) != 1)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}
	    }
	    
	/* now read the rest of the header */
	if (status == MB_SUCCESS)
	    {
	    if ((read_size = fread(&buffer[4],
				1,MBF_OICMBARI_HEADER_SIZE-4,
				mb_io_ptr->mbfp))
		!= MBF_OICMBARI_HEADER_SIZE-4)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}
	    }
	    
	/* now parse the header */
	if (status == MB_SUCCESS)
	    {
	    index = 3;
	    header->type = buffer[index]; index += 1;
 	    mb_get_binary_int(MB_NO,&buffer[index],&header->proc_status);
	    index += 4;
 	    mb_get_binary_int(MB_NO,&buffer[index],&header->data_size);
	    index += 4;
	    header->client_size = buffer[index]; 
	    index += 1;
	    header->fish_status = buffer[index]; 
	    index += 1;
	    header->nav_used = buffer[index]; 
	    index += 1;
	    header->nav_type = buffer[index]; 
	    index += 1;
 	    mb_get_binary_int(MB_NO,&buffer[index],&header->utm_zone);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->ship_x);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->ship_y);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->ship_course);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->ship_speed);
	    index += 4;
 	    mb_get_binary_int(MB_NO,&buffer[index],&header->sec);
	    index += 4;
 	    mb_get_binary_int(MB_NO,&buffer[index],&header->usec);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->spare_gain);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->fish_heading);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->fish_depth);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->fish_range);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->fish_pulse_width);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->gain_c0);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->gain_c1);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->gain_c2);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->fish_pitch);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->fish_roll);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->fish_yaw);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->fish_x);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->fish_y);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->fish_layback);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->fish_altitude);
	    index += 4;
 	    mb_get_binary_int(MB_NO,&buffer[index],&header->fish_altitude_samples);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->fish_ping_period);
	    index += 4;
 	    mb_get_binary_float(MB_NO,&buffer[index],&header->sound_velocity);
	    index += 4;
 	    mb_get_binary_int(MB_NO,&buffer[index],&header->num_chan);
	    index += 4;
	    for (i=0;i<MBF_OICMBARI_MAX_CHANNELS;i++)
		{
 		mb_get_binary_int(MB_NO,&buffer[index],&header->channel[i].offset);
		index += 4;
		}
	    for (i=0;i<MBF_OICMBARI_MAX_CHANNELS;i++)
		{
		header->channel[i].type = buffer[index]; 
		index += 1;
		header->channel[i].side = buffer[index]; 
		index += 1;
		header->channel[i].size = buffer[index]; 
		index += 1;
		header->channel[i].empty = buffer[index]; 
		index += 1;
 		mb_get_binary_int(MB_NO,&buffer[index],&header->channel[i].frequency);
		index += 4;
 		mb_get_binary_int(MB_NO,&buffer[index],&header->channel[i].num_samples);
		index += 4;
		}
 	    mb_get_binary_int(MB_NO,&buffer[index],&header->beams_bath);
	    index += 4;
 	    mb_get_binary_int(MB_NO,&buffer[index],&header->beams_amp);
	    index += 4;
 	    mb_get_binary_int(MB_NO,&buffer[index],&header->bath_chan_port);
	    index += 4;
 	    mb_get_binary_int(MB_NO,&buffer[index],&header->bath_chan_stbd);
	    index += 4;
 	    mb_get_binary_int(MB_NO,&buffer[index],&header->pixels_ss);
	    index += 4;
 	    mb_get_binary_int(MB_NO,&buffer[index],&header->ss_chan_port);
	    index += 4;
 	    mb_get_binary_int(MB_NO,&buffer[index],&header->ss_chan_stbd);
	    index += 4;
	    }
	
	/* read client specific data */
	if (status == MB_SUCCESS && header->client_size > 0)
	    {
	    if ((read_size = fread(dataplus->client,1,
				(int)(header->client_size),mb_io_ptr->mbfp))
		== (int)(header->client_size))
		{
		if (header->client_size < MBF_OICMBARI_MAX_CLIENT)
		    dataplus->client[header->client_size] = 0;
		}
	    else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}
	    }

	/* check for comment or unintelligible records */
	if (status == MB_SUCCESS)
	    {
	    if (header->type == OIC_ID_COMMENT)
		{
		dataplus->kind = MB_DATA_COMMENT;
		}
	    else if (header->num_chan > 0 && header->num_chan <= 8)
		{
		dataplus->kind = MB_DATA_DATA;
		}
	    else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
		dataplus->kind = MB_DATA_NONE;
		}
	    }

	/* set kind and error in mb_io_ptr */
	mb_io_ptr->new_kind = dataplus->kind;
	mb_io_ptr->new_error = *error;
	    
	/* loop over each data channel and read data */
	if (status == MB_SUCCESS && header->num_chan > 0)
	    {
	    for (i=0;i<header->num_chan;i++)
		{
		/* get size of data array */
		if (header->channel[i].size == OIC_SIZE_CHAR)
		    data_size = sizeof(char) * header->channel[i].num_samples;
		else if (header->channel[i].size == OIC_SIZE_SHORT)
		    data_size = sizeof(short) * header->channel[i].num_samples;
		else if (header->channel[i].size == OIC_SIZE_INT)
		    data_size = sizeof(int) * header->channel[i].num_samples;
		else if (header->channel[i].size == OIC_SIZE_FLOAT)
		    data_size = sizeof(float) * header->channel[i].num_samples;
		else if (header->channel[i].size == OIC_SIZE_3FLOAT)
		    data_size = 3 * sizeof(float) * header->channel[i].num_samples;
		
		/* allocate data if needed */
		if (data_size > data->rawsize[i] 
		    || data->raw[i] == NULL)
		    {
		    if (data->raw[i] != NULL)
			status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(data->raw[i]), error);
		    status = mb_mallocd(verbose,__FILE__,__LINE__, data_size, (void **)&(data->raw[i]),error);
		    data->rawsize[i] = data_size;
		    }
		    
		/* read the data */
		if (status == MB_SUCCESS)
		    {
		    if ((read_size = fread(data->raw[i],1,data_size,mb_io_ptr->mbfp)) != data_size)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;		    
			}
		    }

		/* byte swap the data if necessary */
#ifdef BYTESWAPPED
		if (status == MB_SUCCESS)
		    {
		    if (header->channel[i].size == OIC_SIZE_SHORT)
			{
			short_ptr = (short *) data->raw[i];
			for (j=0;j<header->channel[i].num_samples;j++)
			    {
			    short_ptr[j] = mb_swap_short(short_ptr[j]);
			    }
			}
		    else if (header->channel[i].size == OIC_SIZE_INT)
			{
			int_ptr = (int *) data->raw[i];
			for (j=0;j<header->channel[i].num_samples;j++)
			    {
			    int_ptr[j] = mb_swap_int(int_ptr[j]);
			    }
			}
		    else if (header->channel[i].size == OIC_SIZE_FLOAT)
			{
			float_ptr = (float *) data->raw[i];
			for (j=0;j<header->channel[i].num_samples;j++)
			    {
			    mb_swap_float(&float_ptr[j]);
			    }
			}
		    else if (header->channel[i].size == OIC_SIZE_3FLOAT)
			{
			float_ptr = (float *) data->raw[i];
			for (j=0;j<3*header->channel[i].num_samples;j++)
			    {
			    mb_swap_float(&float_ptr[j]);
			    }
			}
		    }
#endif
		}
	    }

	/* print debug statements */
	if (verbose >= 5)
	    {
	    fprintf(stderr,"\ndbg5  New header read in function <%s>\n",function_name);
	    fprintf(stderr,"dbg5       type:             %d\n",header->type);
	    fprintf(stderr,"dbg5       proc_status:      %d\n",header->proc_status);
	    fprintf(stderr,"dbg5       data_size:        %d\n",header->data_size);
	    fprintf(stderr,"dbg5       client_size:      %d\n",header->client_size);
	    fprintf(stderr,"dbg5       fish_status:      %d\n",header->fish_status);
	    fprintf(stderr,"dbg5       nav_used:         %d\n",header->nav_used);
	    fprintf(stderr,"dbg5       nav_type:         %d\n",header->nav_type);
	    fprintf(stderr,"dbg5       utm_zone:         %d\n",header->utm_zone);
	    fprintf(stderr,"dbg5       ship_x:           %f\n",header->ship_x);
	    fprintf(stderr,"dbg5       ship_y:           %f\n",header->ship_y);
	    fprintf(stderr,"dbg5       ship_course:      %f\n",header->ship_course);
	    fprintf(stderr,"dbg5       ship_speed:       %f\n",header->ship_speed);
	    fprintf(stderr,"dbg5       sec:              %d\n",header->sec);
	    fprintf(stderr,"dbg5       usec:             %d\n",header->usec);
	    fprintf(stderr,"dbg5       spare_gain:       %f\n",header->spare_gain);
	    fprintf(stderr,"dbg5       fish_heading:     %f\n",header->fish_heading);
	    fprintf(stderr,"dbg5       fish_depth:       %f\n",header->fish_depth);
	    fprintf(stderr,"dbg5       fish_range:       %f\n",header->fish_range);
	    fprintf(stderr,"dbg5       fish_pulse_width: %f\n",header->fish_pulse_width);
	    fprintf(stderr,"dbg5       gain_c0:          %f\n",header->gain_c0);
	    fprintf(stderr,"dbg5       gain_c1:          %f\n",header->gain_c1);
	    fprintf(stderr,"dbg5       gain_c2:          %f\n",header->gain_c2);
	    fprintf(stderr,"dbg5       fish_pitch:       %f\n",header->fish_pitch);
	    fprintf(stderr,"dbg5       fish_roll:        %f\n",header->fish_roll);
	    fprintf(stderr,"dbg5       fish_yaw:         %f\n",header->fish_yaw);
	    fprintf(stderr,"dbg5       fish_x:           %f\n",header->fish_x);
	    fprintf(stderr,"dbg5       fish_y:           %f\n",header->fish_y);
	    fprintf(stderr,"dbg5       fish_layback:     %f\n",header->fish_layback);
	    fprintf(stderr,"dbg5       fish_altitude:    %f\n",header->fish_altitude);
	    fprintf(stderr,"dbg5       fish_altitude_samples: %d\n",header->fish_altitude_samples);
	    fprintf(stderr,"dbg5       fish_ping_period: %f\n",header->fish_ping_period);
	    fprintf(stderr,"dbg5       sound_velocity:   %f\n",header->sound_velocity);
	    fprintf(stderr,"dbg5       num_chan:         %d\n",header->num_chan);
	    fprintf(stderr,"dbg5       beams_bath:       %d\n",header->beams_bath);
	    fprintf(stderr,"dbg5       beams_amp:        %d\n",header->beams_amp);
	    fprintf(stderr,"dbg5       bath_chan_port:   %d\n",header->bath_chan_port);
	    fprintf(stderr,"dbg5       bath_chan_stbd:   %d\n",header->bath_chan_stbd);
	    fprintf(stderr,"dbg5       pixels_ss:        %d\n",header->pixels_ss);
	    fprintf(stderr,"dbg5       ss_chan_port:     %d\n",header->ss_chan_port);
	    fprintf(stderr,"dbg5       ss_chan_stbd:     %d\n",header->ss_chan_stbd);
	    for (i=0;i<header->num_chan;i++)
		{
		fprintf(stderr,"dbg5       offset[%1d]:      %d\n",
			     i, header->channel[i].offset);
		fprintf(stderr,"dbg5       type[%1d]:        %d\n", 
			     i, header->channel[i].type);
		fprintf(stderr,"dbg5       side[%1d]:        %d\n", 
			     i, header->channel[i].side);
		fprintf(stderr,"dbg5       size[%1d]:        %d\n", 
			     i, header->channel[i].size);
		fprintf(stderr,"dbg5       empty[%1d]:       %d\n", 
			     i, header->channel[i].empty);
		fprintf(stderr,"dbg5       frequency[%1d]:   %d\n", 
			     i, header->channel[i].frequency);
		fprintf(stderr,"dbg5       num_samples[%1d]: %d\n", 
			     i, header->channel[i].num_samples);
		}
	    for (i=0;i<header->num_chan;i++)
		{
		fprintf(stderr,"\ndbg5  New data read in function <%s>\n",function_name);
		fprintf(stderr,"dbg5       channel:   %d\n", i);
		if (header->channel[i].type == OIC_TYPE_SIDESCAN)
		    fprintf(stderr, "dbg5       data type: sidescan\n");
		else if (header->channel[i].type == OIC_TYPE_ANGLE)
		    fprintf(stderr, "dbg5       data type: angle\n");
		else if (header->channel[i].type == OIC_TYPE_MULTIBEAM)
		    fprintf(stderr, "dbg5       data type: multibeam\n");
		else
		    fprintf(stderr, "dbg5       data type: unknown\n");
		if (header->channel[i].side == OIC_PORT)
		    fprintf(stderr, "dbg5       side:      port\n");
		else if (header->channel[i].side == OIC_STARBOARD)
		    fprintf(stderr, "dbg5       side:      starboard\n");
		else
		    fprintf(stderr, "dbg5       side:      unknown\n");
		fprintf(stderr,"dbg5       frequency:   %d\n", header->channel[i].frequency);
		fprintf(stderr,"dbg5       num samples: %d\n", header->channel[i].num_samples);
		if (header->channel[i].size == OIC_SIZE_CHAR)
		    {
		    fprintf(stderr, "dbg5       size:      char (1 byte)\n");
		    char_ptr = (char *) data->raw[i];
		    for (j=0;j<header->channel[i].num_samples;j++)
			{
			fprintf(stderr, "dbg5      %5d  %5d\n", j, char_ptr[j]);
			}
		    }
		else if (header->channel[i].size == OIC_SIZE_SHORT)
		    {
		    fprintf(stderr, "dbg5        size:      short (2 bytes)\n");
		    short_ptr = (short *) data->raw[i];
		    for (j=0;j<header->channel[i].num_samples;j++)
			{
			fprintf(stderr, "dbg5      %5d  %5d\n", j, short_ptr[j]);
			}
		    }
		else if (header->channel[i].size == OIC_SIZE_INT)
		    {
		    fprintf(stderr, "dbg5       size:       int (4 bytes)\n");
		    int_ptr = (int *) data->raw[i];
		    for (j=0;j<header->channel[i].num_samples;j++)
			{
			fprintf(stderr, "dbg5      %5d  %5d\n", j, int_ptr[j]);
			}
		    }
		else if (header->channel[i].size == OIC_SIZE_FLOAT)
		    {
		    fprintf(stderr, "dbg5       size:       float (4 bytes)\n");
		    float_ptr = (float *) data->raw[i];
		    for (j=0;j<header->channel[i].num_samples;j++)
			{
			fprintf(stderr, "dbg5      %5d  %10f\n", j, float_ptr[j]);
			}
		    }
		else if (header->channel[i].size == OIC_SIZE_3FLOAT)
		    {
		    fprintf(stderr, "dbg5       size:       3 floats (12 bytes)\n");
		    float_ptr = (float *) data->raw[i];
		    for (j=0;j<header->channel[i].num_samples;j++)
			{
			fprintf(stderr, "dbg5      %5d  %10f %10f %10f\n", 
				j, float_ptr[3*j], float_ptr[3*j+1], float_ptr[3*j+2]);
			}
		    }
		else
		    fprintf(stderr, "dbg5      size:       unknown\n");
		}
	    fprintf(stderr,"dbg5       status:     %d\n",status);
	    fprintf(stderr,"dbg5       error:      %d\n",*error);
	    }

	/* allocate memory for processed bathymetry and sidescan data */
	if (status == MB_SUCCESS 
	    && dataplus->kind == MB_DATA_DATA)
	    {
	    /* allocate arrays if needed */
	    if (header->beams_bath > data->beams_bath_alloc
		|| data->bath == NULL)
		{
		data->beams_bath_alloc = header->beams_bath;
		if (data->bath != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(data->beamflag), error);
		if (data->bath != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(data->bath), error);
		if (data->bathacrosstrack != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(data->bathacrosstrack), error);
		if (data->bathalongtrack != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(data->bathalongtrack), error);
		if (data->tt != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(data->tt), error);
		if (data->angle != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(data->angle), error);
		status = mb_mallocd(verbose,__FILE__,__LINE__, header->beams_bath * sizeof(char), 
				    (void **)&(data->beamflag), error);
		status = mb_mallocd(verbose,__FILE__,__LINE__, header->beams_bath * sizeof(float), 
				    (void **)&(data->bath), error);
		status = mb_mallocd(verbose,__FILE__,__LINE__, header->beams_bath * sizeof(float), 
				    (void **)&(data->bathacrosstrack), error);
		status = mb_mallocd(verbose,__FILE__,__LINE__, header->beams_bath * sizeof(float), 
				    (void **)&(data->bathalongtrack), error);
		status = mb_mallocd(verbose,__FILE__,__LINE__, header->beams_bath * sizeof(float), 
				    (void **)&(data->tt), error);
		status = mb_mallocd(verbose,__FILE__,__LINE__, header->beams_bath * sizeof(float), 
				    (void **)&(data->angle), error);
		}
	    if (header->beams_amp > data->beams_amp_alloc
		|| data->amp == NULL)
		{
		data->beams_amp_alloc = header->beams_amp;
		if (data->amp != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(data->amp), error);
		status = mb_mallocd(verbose,__FILE__,__LINE__, header->beams_amp * sizeof(float), 
				    (void **)&(data->amp), error);
		}
	    if (header->pixels_ss > data->pixels_ss_alloc
		|| data->ss == NULL)
		{
		data->pixels_ss_alloc = header->pixels_ss;
		if (data->ss != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(data->ss), error);
		if (data->ssacrosstrack != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(data->ssacrosstrack), error);
		if (data->ssalongtrack != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(data->ssalongtrack), error);
		status = mb_mallocd(verbose,__FILE__,__LINE__, header->pixels_ss * sizeof(float), 
				    (void **)&(data->ss), error);
		status = mb_mallocd(verbose,__FILE__,__LINE__, header->pixels_ss * sizeof(float), 
				    (void **)&(data->ssacrosstrack), error);
		status = mb_mallocd(verbose,__FILE__,__LINE__, header->pixels_ss * sizeof(float), 
				   (void **) &(data->ssalongtrack), error);
		}
	    }

	/* read processed bathymetry */
	if (status == MB_SUCCESS 
	    && dataplus->kind == MB_DATA_DATA
	    && header->beams_bath > 0)
	    {
	    /* get beamflag array */
	    data_size = header->beams_bath * sizeof(char);
	    if ((read_size = fread(data->beamflag,1,data_size,mb_io_ptr->mbfp)) != data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}

	    /* get bath array */
	    data_size = header->beams_bath * sizeof(float);
	    if ((read_size = fread(data->bath,1,data_size,mb_io_ptr->mbfp)) != data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}

	    /* get bathacrosstrack array */
	    data_size = header->beams_bath * sizeof(float);
	    if ((read_size = fread(data->bathacrosstrack,1,data_size,mb_io_ptr->mbfp)) != data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}

	    /* get bathalongtrack array */
	    data_size = header->beams_bath * sizeof(float);
	    if ((read_size = fread(data->bathalongtrack,1,data_size,mb_io_ptr->mbfp)) != data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}

	    /* get tt array */
	    data_size = header->beams_bath * sizeof(float);
	    if ((read_size = fread(data->tt,1,data_size,mb_io_ptr->mbfp)) != data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}

	    /* get angle array */
	    data_size = header->beams_bath * sizeof(float);
	    if ((read_size = fread(data->angle,1,data_size,mb_io_ptr->mbfp)) != data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}
	    }

	/* read processed amplitude */
	if (status == MB_SUCCESS 
	    && dataplus->kind == MB_DATA_DATA
	    && header->beams_amp > 0)
	    {
	    /* get amplitude array */
	    data_size = header->beams_amp * sizeof(char);
	    if ((read_size = fread(data->amp,1,data_size,mb_io_ptr->mbfp)) != data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}
	    }

	/* read processed sidescan */
	if (status == MB_SUCCESS 
	    && dataplus->kind == MB_DATA_DATA
	    && header->pixels_ss > 0)
	    {
	    /* get ss array */
	    data_size = header->pixels_ss * sizeof(float);
	    if ((read_size = fread(data->ss,1,data_size,mb_io_ptr->mbfp)) != data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}

	    /* get ssacrosstrack array */
	    data_size = header->pixels_ss * sizeof(float);
	    if ((read_size = fread(data->ssacrosstrack,1,data_size,mb_io_ptr->mbfp)) != data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}

	    /* get ssalongtrack array */
	    data_size = header->pixels_ss * sizeof(float);
	    if ((read_size = fread(data->ssalongtrack,1,data_size,mb_io_ptr->mbfp)) != data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}
	    }

	/* byte swap the processed data if necessary */
#ifdef BYTESWAPPED
	if (status == MB_SUCCESS)
	    {
	    for (j=0;j<header->beams_bath;j++)
		{
		mb_swap_float(&data->bath[j]);
		mb_swap_float(&data->bathacrosstrack[j]);
		mb_swap_float(&data->bathalongtrack[j]);
		mb_swap_float(&data->tt[j]);
		mb_swap_float(&data->angle[j]);
		}
	    for (j=0;j<header->beams_amp;j++)
		{
		mb_swap_float(&data->amp[j]);
		}
	    for (j=0;j<header->pixels_ss;j++)
		{
		mb_swap_float(&data->ssacrosstrack[j]);
		mb_swap_float(&data->ssalongtrack[j]);
		mb_swap_float(&data->ss[j]);
		}
	    }
#endif

	/* print debug statements */
	if (verbose >= 5 && status == MB_SUCCESS)
	    {
	    fprintf(stderr,"\ndbg5  New processed data read in function <%s>\n",
		    function_name);
	    fprintf(stderr,"dbg5       beams_bath:       %d\n",header->beams_bath);
	    fprintf(stderr,"dbg5       beam   bath  xtrack ltrack   tt   angle\n");
	    for (i=0;i<header->beams_bath;i++)
		{
		fprintf(stderr,"dbg5       %4d %10f %10f %10f %10f %10f\n", 
			i, data->bath[i], data->bathacrosstrack[i], 
			data->bathalongtrack[i], 
			data->tt[i], data->angle[i]);
		}
	    fprintf(stderr,"dbg5       beams_amp:       %d\n",header->beams_amp);
	    fprintf(stderr,"dbg5       beam   amp  xtrack ltrack\n");
	    for (i=0;i<header->beams_amp;i++)
		{
		fprintf(stderr,"dbg5       %4d %10f %10f %10f\n", 
			i, data->amp[i], data->bathacrosstrack[i], 
			data->bathalongtrack[i]);
		}
	    fprintf(stderr,"dbg5       pixels_ss:       %d\n",header->pixels_ss);
	    fprintf(stderr,"dbg5       beam   ss  xtrack ltrack\n");
	    for (i=0;i<header->beams_amp;i++)
		{
		fprintf(stderr,"dbg5       %4d %10f %10f %10f\n", 
			i, data->ss[i], data->ssacrosstrack[i], 
			data->ssalongtrack[i]);
		}
	    }

	/* translate values to data storage structure */
	if (status == MB_SUCCESS
		&& store != NULL)
	    {
	    /* type of data record */
	    store->kind = dataplus->kind;
	    store->type = header->type;

	    /* status and size */
	    store->proc_status = header->proc_status;
	    store->data_size = header->data_size;
	    store->client_size = header->client_size;
	    store->fish_status = header->fish_status;

	    /* nav */
	    store->nav_used = header->nav_used;
	    store->nav_type = header->nav_type;
	    store->utm_zone = header->utm_zone;
	    store->ship_x = header->ship_x;
	    store->ship_y = header->ship_y;
	    store->ship_course = header->ship_course;
	    store->ship_speed = header->ship_speed;

	    /* time stamp */
	    store->sec = header->sec;
	    store->usec = header->usec;

	    /* more stuff */
	    store->spare_gain = header->spare_gain;
	    store->fish_heading = header->fish_heading;
	    store->fish_depth = header->fish_depth;
	    store->fish_range = header->fish_range;
	    store->fish_pulse_width = header->fish_pulse_width;
	    store->gain_c0 = header->gain_c0;
	    store->gain_c1 = header->gain_c1;
	    store->gain_c2 = header->gain_c2;
	    store->fish_pitch = header->fish_pitch;
	    store->fish_roll = header->fish_roll;
	    store->fish_yaw = header->fish_yaw;
	    store->fish_x = header->fish_x;
	    store->fish_y = header->fish_y;
	    store->fish_layback = header->fish_layback;
	    store->fish_altitude = header->fish_altitude;
	    store->fish_altitude_samples = header->fish_altitude_samples;
	    store->fish_ping_period = header->fish_ping_period;
	    store->sound_velocity = header->sound_velocity;

	    /* numbers of beams and scaling */
	    store->num_chan = header->num_chan;
	    store->beams_bath = header->beams_bath;
	    store->beams_amp = header->beams_amp;
	    store->bath_chan_port = header->bath_chan_port;
	    store->bath_chan_stbd = header->bath_chan_stbd;
	    store->pixels_ss = header->pixels_ss;
	    store->ss_chan_port = header->ss_chan_port;
	    store->ss_chan_stbd = header->ss_chan_stbd;
	    
	    /* raw data */
	    for (i=0;i<store->num_chan;i++)
		{
		/* copy channel info */
		store->channel[i].offset = header->channel[i].offset;
		store->channel[i].type = header->channel[i].type;
		store->channel[i].side = header->channel[i].side;
		store->channel[i].size = header->channel[i].size;
		store->channel[i].empty = header->channel[i].empty;
		store->channel[i].frequency = header->channel[i].frequency;
		store->channel[i].num_samples = header->channel[i].num_samples;
		
		/* allocate data if needed */
		if (data->rawsize[i] > store->rawsize[i] 
		    || store->raw[i] == NULL)
		    {
		    if (store->raw[i] != NULL)
			status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(store->raw[i]), error);
		    store->rawsize[i] = data->rawsize[i];
		    status = mb_mallocd(verbose,__FILE__,__LINE__,store->rawsize[i], (void **)&(store->raw[i]),error);
		    }
		    
		/* copy the data */
		if (status == MB_SUCCESS)
		    {
		    for (j=0;j<store->rawsize[i];j++)
			{
			store->raw[i][j] = data->raw[i][j];	    
			}
		    }
		}

	    /* depths and sidescan */
	    if (header->beams_bath > store->beams_bath_alloc
		|| store->beamflag == NULL
		|| store->bath == NULL
		|| store->bathacrosstrack == NULL
		|| store->bathalongtrack == NULL
		|| store->tt == NULL
		|| store->angle == NULL)
		{
		store->beams_bath_alloc = header->beams_bath;
		if (store->beamflag != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(store->beamflag), error);
		if (store->bath != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(store->bath), error);
		if (store->bathacrosstrack != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(store->bathacrosstrack), error);
		if (store->bathalongtrack != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(store->bathalongtrack), error);
		if (store->tt != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(store->tt), error);
		if (store->angle != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(store->angle), error);
		status = mb_mallocd(verbose,__FILE__,__LINE__,store->beams_bath_alloc * sizeof(char), 
				    (void **)&(store->beamflag),error);
		status = mb_mallocd(verbose,__FILE__,__LINE__,store->beams_bath_alloc * sizeof(float), 
				    (void **)&(store->bath),error);
		status = mb_mallocd(verbose,__FILE__,__LINE__,store->beams_bath_alloc * sizeof(float), 
				    (void **)&(store->bathacrosstrack),error);
		status = mb_mallocd(verbose,__FILE__,__LINE__,store->beams_bath_alloc * sizeof(float), 
				    (void **)&(store->bathalongtrack),error);
		status = mb_mallocd(verbose,__FILE__,__LINE__,store->beams_bath_alloc * sizeof(float), 
				    (void **)&(store->tt),error);
		status = mb_mallocd(verbose,__FILE__,__LINE__,store->beams_bath_alloc * sizeof(float), 
				    (void **)&(store->angle),error);
		}
	    if (header->beams_amp > store->beams_amp_alloc
		|| store->amp == NULL)
		{
		store->beams_amp_alloc = header->beams_amp;
		if (store->amp != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(store->amp), error);
		status = mb_mallocd(verbose,__FILE__,__LINE__,store->beams_amp_alloc * sizeof(float), 
				    (void **)&(store->amp),error);
		}
	    if (header->pixels_ss > store->pixels_ss_alloc
		|| store->ss == NULL
		|| store->ssacrosstrack == NULL
		|| store->ssalongtrack == NULL)
		{
		store->pixels_ss_alloc = header->pixels_ss;
		if (store->ss != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(store->ss), error);
		if (store->ssacrosstrack != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(store->ssacrosstrack), error);
		if (store->ssalongtrack != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(store->ssalongtrack), error);
		status = mb_mallocd(verbose,__FILE__,__LINE__,store->pixels_ss_alloc * sizeof(float), 
				    (void **)&(store->ss),error);
		status = mb_mallocd(verbose,__FILE__,__LINE__,store->pixels_ss_alloc * sizeof(float), 
				    (void **)&(store->ssacrosstrack),error);
		status = mb_mallocd(verbose,__FILE__,__LINE__,store->pixels_ss_alloc * sizeof(float), 
				    (void **)&(store->ssalongtrack),error);
		}
	    for (i=0;i<store->beams_bath;i++)
		{
		store->beamflag[i] = data->beamflag[i];
		store->bath[i] = data->bath[i];
		store->bathacrosstrack[i] = data->bathacrosstrack[i];
		store->bathalongtrack[i] = data->bathalongtrack[i];
		store->tt[i] = data->tt[i];
		store->angle[i] = data->angle[i];
		}
	    for (i=0;i<store->beams_amp;i++)
		{
		store->amp[i] = data->amp[i];
		}
	    for (i=0;i<store->pixels_ss;i++)
		{
		store->ss[i] = data->ss[i];
		store->ssacrosstrack[i] = data->ssacrosstrack[i];
		store->ssalongtrack[i] = data->ssalongtrack[i];
		}
		    
	    /* client */
	    for (i=0;i<header->client_size;i++)
		store->client[i] = dataplus->client[i];
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
int mbr_wt_oicmbari(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_oicmbari";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_oicmbari_struct *dataplus;
	struct mbf_oicmbari_header_struct *header;
	struct mbf_oicmbari_data_struct *data;
	struct mbsys_oic_struct *store;
	char	buffer[MBF_OICMBARI_HEADER_SIZE];
	char	*comment;
	int	write_size;
	int	data_size;
	char	*char_ptr;
	short	*short_ptr;
	int	*int_ptr;
	float	*float_ptr;
	int	index;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %ld\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %ld\n",(size_t)store_ptr);
		}

	/* get pointer to mbio descriptor and data storage */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	store = (struct mbsys_oic_struct *) store_ptr;

	/* get pointer to raw data structure */
	dataplus = (struct mbf_oicmbari_struct *) mb_io_ptr->raw_data;
	header = &(dataplus->header);
	data = &(dataplus->data);
	comment = dataplus->client;

	/* first translate values from data storage structure */
	if (store != NULL)
	    {
	    /* type of data record */
	    dataplus->kind = store->kind;
	    header->type = store->type;

	    /* status and size */
	    header->proc_status = store->proc_status;
	    header->data_size = store->data_size;
	    header->client_size = store->client_size;
	    header->fish_status = store->fish_status;

	    /* nav */
	    header->nav_used = store->nav_used;
	    header->nav_type = store->nav_type;
	    header->utm_zone = store->utm_zone;
	    header->ship_x = store->ship_x;
	    header->ship_y = store->ship_y;
	    header->ship_course = store->ship_course;
	    header->ship_speed = store->ship_speed;

	    /* time stamp */
	    header->sec = store->sec;
	    header->usec = store->usec;

	    /* more stuff */
	    header->spare_gain = store->spare_gain;
	    header->fish_heading = store->fish_heading;
	    header->fish_depth = store->fish_depth;
	    header->fish_range = store->fish_range;
	    header->fish_pulse_width = store->fish_pulse_width;
	    header->gain_c0 = store->gain_c0;
	    header->gain_c1 = store->gain_c1;
	    header->gain_c2 = store->gain_c2;
	    header->fish_pitch = store->fish_pitch;
	    header->fish_roll = store->fish_roll;
	    header->fish_yaw = store->fish_yaw;
	    header->fish_x = store->fish_x;
	    header->fish_y = store->fish_y;
	    header->fish_layback = store->fish_layback;
	    header->fish_altitude = store->fish_altitude;
	    header->fish_altitude_samples = store->fish_altitude_samples;
	    header->fish_ping_period = store->fish_ping_period;
	    header->sound_velocity = store->sound_velocity;

	    /* numbers of beams and scaling */
	    header->num_chan = store->num_chan;
	    header->beams_bath = store->beams_bath;
	    header->beams_amp = store->beams_amp;
	    header->bath_chan_port = store->bath_chan_port;
	    header->bath_chan_stbd = store->bath_chan_stbd;
	    header->pixels_ss = store->pixels_ss;
	    header->ss_chan_port = store->ss_chan_port;
	    header->ss_chan_stbd = store->ss_chan_stbd;
	    
	    /* raw data */
	    for (i=0;i<header->num_chan;i++)
		{
		/* copy channel info */
		header->channel[i].offset = store->channel[i].offset;
		header->channel[i].type = store->channel[i].type;
		header->channel[i].side = store->channel[i].side;
		header->channel[i].size = store->channel[i].size;
		header->channel[i].empty = store->channel[i].empty;
		header->channel[i].frequency = store->channel[i].frequency;
		header->channel[i].num_samples = store->channel[i].num_samples;
		
		/* allocate data if needed */
		if (store->rawsize[i] > data->rawsize[i] 
		    || data->raw[i] == NULL)
		    {
		    if (data->raw[i] != NULL)
			status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(data->raw[i]), error);
		    data->rawsize[i] = store->rawsize[i];
		    status = mb_mallocd(verbose,__FILE__,__LINE__, data->rawsize[i], (void **)&(data->raw[i]),error);
		    }
		    
		/* copy the data */
		if (status == MB_SUCCESS)
		    {
		    for (j=0;j<data->rawsize[i];j++)
			{
			data->raw[i][j] = store->raw[i][j];		    
			}
		    }
		}

	    /* depths and sidescan */
	    if (header->beams_bath > data->beams_bath_alloc
		|| data->beamflag == NULL
		|| data->bath == NULL
		|| data->bathacrosstrack == NULL
		|| data->bathalongtrack == NULL
		|| data->tt == NULL
		|| data->angle == NULL)
		{
		data->beams_bath_alloc = header->beams_bath;
		if (data->beamflag != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(data->beamflag), error);
		if (data->bath != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(data->bath), error);
		if (data->bathacrosstrack != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(data->bathacrosstrack), error);
		if (data->bathalongtrack != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(data->bathalongtrack), error);
		if (data->tt != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(data->tt), error);
		if (data->angle != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(data->angle), error);
		status = mb_mallocd(verbose,__FILE__,__LINE__,data->beams_bath_alloc * sizeof(char), 
				    (void **)&(data->beamflag),error);
		status = mb_mallocd(verbose,__FILE__,__LINE__,data->beams_bath_alloc * sizeof(float), 
				    (void **)&(data->bath),error);
		status = mb_mallocd(verbose,__FILE__,__LINE__,data->beams_bath_alloc * sizeof(float), 
				    (void **)&(data->bathacrosstrack),error);
		status = mb_mallocd(verbose,__FILE__,__LINE__,data->beams_bath_alloc * sizeof(float), 
				    (void **)&(data->bathalongtrack),error);
		status = mb_mallocd(verbose,__FILE__,__LINE__,data->beams_bath_alloc * sizeof(float), 
				    (void **)&(data->tt),error);
		status = mb_mallocd(verbose,__FILE__,__LINE__,data->beams_bath_alloc * sizeof(float), 
				    (void **)&(data->angle),error);
		}
	    if (header->beams_amp > data->beams_amp_alloc
		|| data->amp == NULL)
		{
		data->beams_amp_alloc = header->beams_amp;
		if (data->amp != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(data->amp), error);
		status = mb_mallocd(verbose,__FILE__,__LINE__,data->beams_amp_alloc * sizeof(float), 
				    (void **)&(data->amp),error);
		}
	    if (header->pixels_ss > data->pixels_ss_alloc
		|| data->ss == NULL
		|| data->ssacrosstrack == NULL
		|| data->ssalongtrack == NULL)
		{
		data->pixels_ss_alloc = header->pixels_ss;
		if (data->ss != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(data->ss), error);
		if (data->ssacrosstrack != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(data->ssacrosstrack), error);
		if (data->ssalongtrack != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &(data->ssalongtrack), error);
		status = mb_mallocd(verbose,__FILE__,__LINE__,data->pixels_ss_alloc * sizeof(float), 
				    (void **)&(data->ss),error);
		status = mb_mallocd(verbose,__FILE__,__LINE__,data->pixels_ss_alloc * sizeof(float), 
				    (void **)&(data->ssacrosstrack),error);
		status = mb_mallocd(verbose,__FILE__,__LINE__,data->pixels_ss_alloc * sizeof(float), 
				    (void **)&(data->ssalongtrack),error);
		}
	    for (i=0;i<header->beams_bath;i++)
		{
		data->beamflag[i] = store->beamflag[i];
		data->bath[i] = store->bath[i];
		data->bathacrosstrack[i] = store->bathacrosstrack[i];
		data->bathalongtrack[i] = store->bathalongtrack[i];
		data->tt[i] = store->tt[i];
		data->angle[i] = store->angle[i];
		}
	    for (i=0;i<header->beams_amp;i++)
		{
		data->amp[i] = store->amp[i];
		}
	    for (i=0;i<header->pixels_ss;i++)
		{
		data->ss[i] = store->ss[i];
		data->ssacrosstrack[i] = store->ssacrosstrack[i];
		data->ssalongtrack[i] = store->ssalongtrack[i];
		}
		    
	    /* client */
	    for (i=0;i<header->client_size;i++)
		dataplus->client[i] = store->client[i];
	    }

	/* print debug statements */
	if (verbose >= 5)
	    {
	    fprintf(stderr,"\ndbg5  New header set in function <%s>\n",function_name);
	    fprintf(stderr,"dbg5       type:             %d\n",header->type);
	    fprintf(stderr,"dbg5       proc_status:      %d\n",header->proc_status);
	    fprintf(stderr,"dbg5       data_size:        %d\n",header->data_size);
	    fprintf(stderr,"dbg5       client_size:      %d\n",header->client_size);
	    fprintf(stderr,"dbg5       fish_status:      %d\n",header->fish_status);
	    fprintf(stderr,"dbg5       nav_used:         %d\n",header->nav_used);
	    fprintf(stderr,"dbg5       nav_type:         %d\n",header->nav_type);
	    fprintf(stderr,"dbg5       utm_zone:         %d\n",header->utm_zone);
	    fprintf(stderr,"dbg5       ship_x:           %f\n",header->ship_x);
	    fprintf(stderr,"dbg5       ship_y:           %f\n",header->ship_y);
	    fprintf(stderr,"dbg5       ship_course:      %f\n",header->ship_course);
	    fprintf(stderr,"dbg5       ship_speed:       %f\n",header->ship_speed);
	    fprintf(stderr,"dbg5       sec:              %d\n",header->sec);
	    fprintf(stderr,"dbg5       usec:             %d\n",header->usec);
	    fprintf(stderr,"dbg5       spare_gain:       %f\n",header->spare_gain);
	    fprintf(stderr,"dbg5       fish_heading:     %f\n",header->fish_heading);
	    fprintf(stderr,"dbg5       fish_depth:       %f\n",header->fish_depth);
	    fprintf(stderr,"dbg5       fish_range:       %f\n",header->fish_range);
	    fprintf(stderr,"dbg5       fish_pulse_width: %f\n",header->fish_pulse_width);
	    fprintf(stderr,"dbg5       gain_c0:          %f\n",header->gain_c0);
	    fprintf(stderr,"dbg5       gain_c1:          %f\n",header->gain_c1);
	    fprintf(stderr,"dbg5       gain_c2:          %f\n",header->gain_c2);
	    fprintf(stderr,"dbg5       fish_pitch:       %f\n",header->fish_pitch);
	    fprintf(stderr,"dbg5       fish_roll:        %f\n",header->fish_roll);
	    fprintf(stderr,"dbg5       fish_yaw:         %f\n",header->fish_yaw);
	    fprintf(stderr,"dbg5       fish_x:           %f\n",header->fish_x);
	    fprintf(stderr,"dbg5       fish_y:           %f\n",header->fish_y);
	    fprintf(stderr,"dbg5       fish_layback:     %f\n",header->fish_layback);
	    fprintf(stderr,"dbg5       fish_altitude:    %f\n",header->fish_altitude);
	    fprintf(stderr,"dbg5       fish_altitude_samples: %d\n",header->fish_altitude_samples);
	    fprintf(stderr,"dbg5       fish_ping_period: %f\n",header->fish_ping_period);
	    fprintf(stderr,"dbg5       sound_velocity:   %f\n",header->sound_velocity);
	    fprintf(stderr,"dbg5       num_chan:         %d\n",header->num_chan);
	    fprintf(stderr,"dbg5       beams_bath:       %d\n",header->beams_bath);
	    fprintf(stderr,"dbg5       beams_amp:        %d\n",header->beams_amp);
	    fprintf(stderr,"dbg5       bath_chan_port:   %d\n",header->bath_chan_port);
	    fprintf(stderr,"dbg5       bath_chan_stbd:   %d\n",header->bath_chan_stbd);
	    fprintf(stderr,"dbg5       pixels_ss:        %d\n",header->pixels_ss);
	    fprintf(stderr,"dbg5       ss_chan_port:     %d\n",header->ss_chan_port);
	    fprintf(stderr,"dbg5       ss_chan_stbd:     %d\n",header->ss_chan_stbd);
	    for (i=0;i<header->num_chan;i++)
		{
		fprintf(stderr,"dbg5       offset[%1d]:      %d\n",
			     i, header->channel[i].offset);
		fprintf(stderr,"dbg5       type[%1d]:        %d\n", 
			     i, header->channel[i].type);
		fprintf(stderr,"dbg5       side[%1d]:        %d\n", 
			     i, header->channel[i].side);
		fprintf(stderr,"dbg5       size[%1d]:        %d\n", 
			     i, header->channel[i].size);
		fprintf(stderr,"dbg5       empty[%1d]:       %d\n", 
			     i, header->channel[i].empty);
		fprintf(stderr,"dbg5       frequency[%1d]:   %d\n", 
			     i, header->channel[i].frequency);
		fprintf(stderr,"dbg5       num_samples[%1d]: %d\n", 
			     i, header->channel[i].num_samples);
		}
	    fprintf(stderr,"dbg5       status:     %d\n",status);
	    fprintf(stderr,"dbg5       error:      %d\n",*error);
	    }

	/* print debug messages */
	if (verbose >= 5 && status == MB_SUCCESS)
	    {
	    for (i=0;i<header->num_chan;i++)
		{
		fprintf(stderr,"\ndbg5  New data set in function <%s>\n",function_name);
		fprintf(stderr,"dbg5       channel:   %d\n", i);
		if (header->channel[i].type == OIC_TYPE_SIDESCAN)
		    fprintf(stderr, "dbg5       data type: sidescan\n");
		else if (header->channel[i].type == OIC_TYPE_ANGLE)
		    fprintf(stderr, "dbg5       data type: angle\n");
		else if (header->channel[i].type == OIC_TYPE_MULTIBEAM)
		    fprintf(stderr, "dbg5       data type: multibeam\n");
		else
		    fprintf(stderr, "dbg5       data type: unknown\n");
		if (header->channel[i].side == OIC_PORT)
		    fprintf(stderr, "dbg5       side:      port\n");
		else if (header->channel[i].side == OIC_STARBOARD)
		    fprintf(stderr, "dbg5       side:      starboard\n");
		else
		    fprintf(stderr, "dbg5       side:      unknown\n");
		fprintf(stderr,"dbg5       frequency:   %d\n", header->channel[i].frequency);
		fprintf(stderr,"dbg5       num samples: %d\n", header->channel[i].num_samples);
		if (header->channel[i].size == OIC_SIZE_CHAR)
		    {
		    fprintf(stderr, "dbg5       size:      char (1 byte)\n");
		    char_ptr = (char *) data->raw[i];
		    for (j=0;j<header->channel[i].num_samples;j++)
			{
			fprintf(stderr, "dbg5      %5d  %5d\n", j, char_ptr[j]);
			}
		    }
		else if (header->channel[i].size == OIC_SIZE_SHORT)
		    {
		    fprintf(stderr, "dbg5       size:      short (2 bytes)\n");
		    short_ptr = (short *) data->raw[i];
		    for (j=0;j<header->channel[i].num_samples;j++)
			{
			fprintf(stderr, "dbg5      %5d  %5d\n", j, short_ptr[j]);
			}
		    }
		else if (header->channel[i].size == OIC_SIZE_INT)
		    {
		    fprintf(stderr, "dbg5       size:      int (4 bytes)\n");
		    int_ptr = (int *) data->raw[i];
		    for (j=0;j<header->channel[i].num_samples;j++)
			{
			fprintf(stderr, "dbg5      %5d  %5d\n", j, int_ptr[j]);
			}
		    }
		else if (header->channel[i].size == OIC_SIZE_FLOAT)
		    {
		    fprintf(stderr, "dbg5       size:      float (4 bytes)\n");
		    float_ptr = (float *) data->raw[i];
		    for (j=0;j<header->channel[i].num_samples;j++)
			{
			fprintf(stderr, "dbg5      %5d  %10f\n", j, float_ptr[j]);
			}
		    }
		else if (header->channel[i].size == OIC_SIZE_3FLOAT)
		    {
		    fprintf(stderr, "dbg5       size:      3 floats (12 bytes)\n");
		    float_ptr = (float *) data->raw[i];
		    for (j=0;j<header->channel[i].num_samples;j++)
			{
			fprintf(stderr, "dbg5      %5d  %10f %10f %10f\n", 
				j, float_ptr[3*j], float_ptr[3*j+1], float_ptr[3*j+2]);
			}
		    }
		else
		    fprintf(stderr, "dbg5       size:      unknown\n");
		}
	    }

	/* print debug statements */
	if (verbose >= 5 && status == MB_SUCCESS)
	    {
	    fprintf(stderr,"\ndbg5  New processed data set in function <%s>\n",
		    function_name);
	    fprintf(stderr,"dbg5       beams_bath:       %d\n",header->beams_bath);
	    fprintf(stderr,"dbg5       beam   bath  xtrack ltrack   tt   angle\n");
	    for (i=0;i<header->beams_bath;i++)
		{
		fprintf(stderr,"dbg5       %4d %10f %10f %10f %10f %10f\n", 
			i, data->bath[i], data->bathacrosstrack[i], 
			data->bathalongtrack[i], 
			data->tt[i], data->angle[i]);
		}
	    fprintf(stderr,"dbg5       beams_amp:       %d\n",header->beams_amp);
	    fprintf(stderr,"dbg5       beam   amp  xtrack ltrack\n");
	    for (i=0;i<header->beams_amp;i++)
		{
		fprintf(stderr,"dbg5       %4d %10f %10f %10f\n", 
			i, data->amp[i], data->bathacrosstrack[i], 
			data->bathalongtrack[i]);
		}
	    fprintf(stderr,"dbg5       pixels_ss:       %d\n",header->pixels_ss);
	    fprintf(stderr,"dbg5       beam   ss  xtrack ltrack\n");
	    for (i=0;i<header->pixels_ss;i++)
		{
		fprintf(stderr,"dbg5       %4d %10f %10f %10f\n", 
			i, data->ss[i], data->ssacrosstrack[i], 
			data->ssalongtrack[i]);
		}
	    }
	    
	/* now reverse parse the header */
	if (status == MB_SUCCESS)
	    {
	    index = 0;
	    buffer[index] = 'G'; 
	    index += 1;
	    buffer[index] = 'E'; 
	    index += 1;
	    buffer[index] = '2'; 
	    index += 1;
	    buffer[index] = header->type; 
	    index += 1;
 	    mb_put_binary_int(MB_NO,header->proc_status,&buffer[index]);
	    index += 4;
	    mb_put_binary_int(MB_NO,header->data_size,&buffer[index]);
	    index += 4;
	    buffer[index] = header->client_size; index += 1;
	    buffer[index] = header->fish_status; index += 1;
	    buffer[index] = header->nav_used; index += 1;
	    buffer[index] = header->nav_type; index += 1;
 	    mb_put_binary_int(MB_NO,header->utm_zone,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->ship_x,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->ship_y,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->ship_course,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->ship_speed,&buffer[index]);
	    index += 4;
 	    mb_put_binary_int(MB_NO,header->sec,&buffer[index]);
	    index += 4;
 	    mb_put_binary_int(MB_NO,header->usec,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->spare_gain,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->fish_heading,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->fish_depth,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->fish_range,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->fish_pulse_width,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->gain_c0,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->gain_c1,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->gain_c2,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->fish_pitch,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->fish_roll,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->fish_yaw,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->fish_x,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->fish_y,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->fish_layback,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->fish_altitude,&buffer[index]);
	    index += 4;
 	    mb_put_binary_int(MB_NO,header->fish_altitude_samples,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->fish_ping_period,&buffer[index]);
	    index += 4;
 	    mb_put_binary_float(MB_NO,header->sound_velocity,&buffer[index]);
	    index += 4;
 	    mb_put_binary_int(MB_NO,header->num_chan,&buffer[index]);
	    index += 4;
	    for (i=0;i<MBF_OICMBARI_MAX_CHANNELS;i++)
		{
 		mb_put_binary_int(MB_NO,header->channel[i].offset,&buffer[index]);
		index += 4;
		}
	    for (i=0;i<MBF_OICMBARI_MAX_CHANNELS;i++)
		{
		buffer[index] = header->channel[i].type; index += 1;
		buffer[index] = header->channel[i].side; index += 1;
		buffer[index] = header->channel[i].size; index += 1;
		buffer[index] = header->channel[i].empty; index += 1;
 		mb_put_binary_int(MB_NO,header->channel[i].frequency,&buffer[index]);
		index += 4;
 		mb_put_binary_int(MB_NO,header->channel[i].num_samples,&buffer[index]);
		index += 4;
		}
 	    mb_put_binary_int(MB_NO,header->beams_bath,&buffer[index]);
	    index += 4;
 	    mb_put_binary_int(MB_NO,header->beams_amp,&buffer[index]);
	    index += 4;
 	    mb_put_binary_int(MB_NO,header->bath_chan_port,&buffer[index]);
	    index += 4;
 	    mb_put_binary_int(MB_NO,header->bath_chan_stbd,&buffer[index]);
	    index += 4;
 	    mb_put_binary_int(MB_NO,header->pixels_ss,&buffer[index]);
	    index += 4;
 	    mb_put_binary_int(MB_NO,header->ss_chan_port,&buffer[index]);
	    index += 4;
 	    mb_put_binary_int(MB_NO,header->ss_chan_stbd,&buffer[index]);
	    index += 4;
	    }

	/* write next header to file */
	if ((write_size = fwrite(buffer,1,MBF_OICMBARI_HEADER_SIZE,
			mb_io_ptr->mbfp)) == MBF_OICMBARI_HEADER_SIZE) 
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}
	
	/* write client specific data */
	if (status == MB_SUCCESS && header->client_size > 0)
	    {
	    if ((write_size = fwrite(dataplus->client,1,
				(int)(header->client_size),mb_io_ptr->mbfp))
		!= (int)(header->client_size))
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;		    
		}
	    }
	    
	/* loop over each data channel and write data */
	if (status == MB_SUCCESS && header->num_chan > 0)
	    {
	    for (i=0;i<header->num_chan;i++)
		{
		/* get size of data array */
		if (header->channel[i].size == OIC_SIZE_CHAR)
		    data_size = sizeof(char) * header->channel[i].num_samples;
		else if (header->channel[i].size == OIC_SIZE_SHORT)
		    data_size = sizeof(short) * header->channel[i].num_samples;
		else if (header->channel[i].size == OIC_SIZE_INT)
		    data_size = sizeof(int) * header->channel[i].num_samples;
		else if (header->channel[i].size == OIC_SIZE_FLOAT)
		    data_size = sizeof(float) * header->channel[i].num_samples;
		else if (header->channel[i].size == OIC_SIZE_3FLOAT)
		    data_size = 3 * sizeof(float) * header->channel[i].num_samples;

		/* byte swap the data if necessary */
#ifdef BYTESWAPPED
		if (status == MB_SUCCESS)
		    {
		    if (header->channel[i].size == OIC_SIZE_SHORT)
			{
			short_ptr = (short *) data->raw[i];
			for (j=0;j<header->channel[i].num_samples;j++)
			    {
			    short_ptr[j] = mb_swap_short(short_ptr[j]);
			    }
			}
		    else if (header->channel[i].size == OIC_SIZE_INT)
			{
			int_ptr = (int *) data->raw[i];
			for (j=0;j<header->channel[i].num_samples;j++)
			    {
			    int_ptr[j] = mb_swap_int(int_ptr[j]);
			    }
			}
		    else if (header->channel[i].size == OIC_SIZE_FLOAT)
			{
			float_ptr = (float *) data->raw[i];
			for (j=0;j<header->channel[i].num_samples;j++)
			    {
			    mb_swap_float(&float_ptr[j]);
			    }
			}
		    else if (header->channel[i].size == OIC_SIZE_3FLOAT)
			{
			float_ptr = (float *) data->raw[i];
			for (j=0;j<3*header->channel[i].num_samples;j++)
			    {
			    mb_swap_float(&float_ptr[j]);
			    }
			}
		    }
#endif
		    
		/* write the data */
		if (status == MB_SUCCESS)
		    {
		    if ((write_size = fwrite(data->raw[i],1,data_size,mb_io_ptr->mbfp)) != data_size)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_WRITE_FAIL;		    
			}
		    }
		}
	    }

	/* byte swap the processed data if necessary */
#ifdef BYTESWAPPED
	if (status == MB_SUCCESS)
	    {
	    for (j=0;j<header->beams_bath;j++)
		{
		mb_swap_float(&data->bath[j]);
		mb_swap_float(&data->bathacrosstrack[j]);
		mb_swap_float(&data->bathalongtrack[j]);
		mb_swap_float(&data->tt[j]);
		mb_swap_float(&data->angle[j]);
		}
	    for (j=0;j<header->beams_amp;j++)
		{
		mb_swap_float(&data->amp[j]);
		}
	    for (j=0;j<header->pixels_ss;j++)
		{
		mb_swap_float(&data->ssacrosstrack[j]);
		mb_swap_float(&data->ssalongtrack[j]);
		mb_swap_float(&data->ss[j]);
		}
	    }
#endif

	/* write processed bathymetry */
	if (status == MB_SUCCESS 
	    && dataplus->kind == MB_DATA_DATA
	    && header->beams_bath > 0)
	    {
	    /* get beamflag array */
	    data_size = header->beams_bath * sizeof(char);
	    if ((write_size = fwrite(data->beamflag,1,data_size,mb_io_ptr->mbfp)) != data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}

	    /* get bath array */
	    data_size = header->beams_bath * sizeof(float);
	    if ((write_size = fwrite(data->bath,1,data_size,mb_io_ptr->mbfp)) != data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}

	    /* get bathacrosstrack array */
	    data_size = header->beams_bath * sizeof(float);
	    if ((write_size = fwrite(data->bathacrosstrack,1,data_size,mb_io_ptr->mbfp)) != data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}

	    /* get bathalongtrack array */
	    data_size = header->beams_bath * sizeof(float);
	    if ((write_size = fwrite(data->bathalongtrack,1,data_size,mb_io_ptr->mbfp)) != data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}

	    /* get tt array */
	    data_size = header->beams_bath * sizeof(float);
	    if ((write_size = fwrite(data->tt,1,data_size,mb_io_ptr->mbfp)) != data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}

	    /* get angle array */
	    data_size = header->beams_bath * sizeof(float);
	    if ((write_size = fwrite(data->angle,1,data_size,mb_io_ptr->mbfp)) != data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}
	    }

	/* write processed amplitude */
	if (status == MB_SUCCESS 
	    && dataplus->kind == MB_DATA_DATA
	    && header->beams_amp > 0)
	    {
	    /* get amplitude array */
	    data_size = header->beams_amp * sizeof(char);
	    if ((write_size = fwrite(data->amp,1,data_size,mb_io_ptr->mbfp)) != data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}
	    }

	/* write processed sidescan */
	if (status == MB_SUCCESS 
	    && dataplus->kind == MB_DATA_DATA
	    && header->pixels_ss > 0)
	    {
	    /* get ss array */
	    data_size = header->pixels_ss * sizeof(float);
	    if ((write_size = fwrite(data->ss,1,data_size,mb_io_ptr->mbfp)) != data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}

	    /* get ssacrosstrack array */
	    data_size = header->pixels_ss * sizeof(float);
	    if ((write_size = fwrite(data->ssacrosstrack,1,data_size,mb_io_ptr->mbfp)) != data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
		}

	    /* get ssalongtrack array */
	    data_size = header->pixels_ss * sizeof(float);
	    if ((write_size = fwrite(data->ssalongtrack,1,data_size,mb_io_ptr->mbfp)) != data_size)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;		    
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
