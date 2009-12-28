/*--------------------------------------------------------------------
 *    The MB-system:	mbr_hs10jams.c	12/4/00
 *	$Id$
 *
 *    Copyright (c) 2000-2009 by
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
 * mbr_hs10jams.c contains the functions for reading and writing
 * navigation data in the HS10JAMS format.
 * These functions include:
 *   mbr_alm_hs10jams	- allocate read/write memory
 *   mbr_dem_hs10jams	- deallocate read/write memory
 *   mbr_rt_hs10jams	- read and translate data
 *   mbr_wt_hs10jams	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	December 4, 2000
 *
 * $Log: mbr_hs10jams.c,v $
 * Revision 5.7  2005/11/05 00:48:05  caress
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
 * Revision 5.3  2001/07/20 00:31:11  caress
 * Release 5.0.beta03
 *
 * Revision 5.2  2001/03/22  20:50:02  caress
 * Trying to make version 5.0.beta0
 *
 * Revision 5.1  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2000/12/10  20:24:25  caress
 * Initial revision.
 *
 *
 *
 */
/*
 * Notes on the MBF_HS10JAMS data format:
 *   1. The Furuno HS-10 multibeam sonar generated 45 beams
 *      of bathymetry and amplitude.
 *   2. To my knowledge, only one Furuno HS-10 multibeam sonar
 *      has been operated. It was installed on S/V Yokosuka,
 *      a JAMSTEC research vessel. The Furuno HS-10 has since been
 *      replaced by a SeaBeam 2112 multibeam sonar.
 *   3. A specification for the raw HS-10 data format was
 *      provided by JAMSTEC, and is included below. The raw format
 *      consists of 800 byte binary records in which only the
 *      lower 4 bits of each byte are used.
 *   4. The actual data files provided to WHOI seem to be simple
 *      717 byte ASCII records with time, lat, lon, heading, 
 *      center beam depth, 45 depths, 45 acrosstrack distances, 
 *      45 beam amplitudes, and a <CR><LF> terminator. Format 
 *      171 supports the actual data we received.
 *   5. The data received use 5 characters each for depth, 
 *      acrosstrack, and amplitude values. Null beams have
 *      depth values of 29999 and acrosstrack values of 99999.
 *      MB-System supports beam flagging by setting flagged
 *      beams negative.
 *   6. The internal data structure supports the data included
 *      in the format 171 files, and does not yet include values
 *      listed in the raw format spec but not seen in the data
 *      provided.
 *   7. Comment records are supported as an MB-System extension
 *      where the first two bytes of the record are "##".
 *      Comment records are variable length.
 *   8. The raw data format specification is as follows:
 * 
 *      ----------------------------------------------------------
 *      HS-10 MNBES Data Format - JAMSTEC
 *      
 *      800 bytes/record, 10 records/block
 *      
 *      Note: 4 bits from LSB is effective in each byte.
 *           zB. 30 30 35 39 ---> 0 0 5 9 (HEX) = 89 (DEC)
 *               30 30 32 3D ---> 0 0 2 D (HEX) = 45 (DEC)
 *      The HS-10 processor calculates the water depth by use of
 *      average sound velocity and by correcting the difference 
 *      between the true angle of the sound path (obtained by the 
 *      true sound velocity profile) and the nominal angle of each 
 *      beam (every 2 degrees). The horizontal distance of the n-th 
 *      beam is
 *              Distance(n) = Depth(n) * tan[T(n)],
 *      where T(n) is the nominal angle of the n-th beam: 
 *              ( T(n) = 2 * (n-23) degrees, n=1,45 ).
 *      
 *      No.  Bytes  Data
 *       1.    4    Year
 *       2.    4    Month
 *       3.    4    Day
 *       4.    4    Hour
 *       5.    4    Minute
 *       6.    4    Second
 *       7.    8    Latitude in 1/10000 minute
 *       8.    8    Longitude in 1/10000 minute
 *       9.    8    X in 1/10 metre
 *      10.    8    Y in 1/10 metre
 *      11.    4    Ship's speed in 1/10 knot
 *      12.    4    Ship's heading in 1/10 degree
 *      13. 4x45    45 Water depths in metre
 *      14. 4x45    45 Intensity of reflection in dB
 *      15.    4    Selection of navigation
 *                    0:HYB, 1:ANS, 2:MANU(L/L) 3:MANU(X/Y)
 *      16.    4    Surface sound velocity in 1/10 m/sec
 *      17.    8    Initial latitude in 1/10000 minute
 *      18.    8    Initial longitude in 1/10000 minute
 *      19.    8    Initial X in 1/10 metre
 *      20.    8    Initial Y in 1/10 metre
 *      21.    4    Manual bearing in 1/10 degree
 *      22.    4    Manual ship's speed in 1/10 knot
 *      23.    4    Ship's draft in 1/10 metre
 *      24.    4    Offset X in 1/10 metre
 *      25.    4    Offset Y in 1/10 metre
 *      26.    4    Selection of sound velocity
 *                    0:no correction, 1:manual input, 2:calculation correction
 *      27.    4    Average sound velocity in 1/10 m/sec
 *      28.    4    Input selection of water temperature
 *                    0:AUTO, 1:MANUAL
 *      29.    4    Water temperature in 1/10 degree
 *      30.    4    Tide level in 1/10 metre
 *      31. 4x10    10 Depth of layer in metre
 *      32. 4x10    10 Temperature of layer in 1/10 degree
 *      33. 4x10    10 Salinity in 1/10 per mille
 *      34. 4x10    10 Sound velocity in 1/10 m/sec
 *      35.    4    Transmitted pulse width
 *                    0:1m, 1:2m, 2:4m, 3:8m
 *      36.    4    Level of transmission [1-16]
 *                    1:Off, 16:Max, -2dB in each step
 *      37.    4    Selection of period of tranmission
 *                    0:Auto, 1:Manual
 *      38:    4    Period of tranmission in second
 *      39:    4    Pre-amp ATT
 *                    0:OFF, 1:ON
 *      40:    4    Receiving gain [1-16]
 *                    1:Off, 16:Max, -2dB in each step
 *      41.    4    TVG [1-4]
 *      42.    4    AVG [1-4]
 *      43.    4    Threshold [1-16]
 *      44.    4    Gate width (R/L) [1-4]
 *      45.    4    Gate width (F/B) [1-4]
 *      46.    4    Selection of beam pattern [1-3]
 *      47.    4    Interferance removal
 *                    0:OFF, 1:ON
 *      48.    4    KP shift [1-32]
 *      49.    4    Sonar mode [0]
 *      50.         not used
 *      ----------------------------------------------------------
 * 
 */
 
#define	MBF_HS10JAMS_MAXLINE	800
#define	MBF_HS10JAMS_LENGTH	717

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_io.h"
#include "../../include/mb_define.h"
#include "../../include/mbsys_hs10.h"

/* essential function prototypes */
int mbr_register_hs10jams(int verbose, void *mbio_ptr, 
		int *error);
int mbr_info_hs10jams(int verbose, 
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
int mbr_alm_hs10jams(int verbose, void *mbio_ptr, int *error);
int mbr_dem_hs10jams(int verbose, void *mbio_ptr, int *error);
int mbr_rt_hs10jams(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_hs10jams(int verbose, void *mbio_ptr, void *store_ptr, int *error);

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/
int mbr_register_hs10jams(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_register_hs10jams";
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
	status = mbr_info_hs10jams(verbose, 
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_hs10jams;
	mb_io_ptr->mb_io_format_free = &mbr_dem_hs10jams; 
	mb_io_ptr->mb_io_store_alloc = &mbsys_hs10_alloc; 
	mb_io_ptr->mb_io_store_free = &mbsys_hs10_deall; 
	mb_io_ptr->mb_io_read_ping = &mbr_rt_hs10jams; 
	mb_io_ptr->mb_io_write_ping = &mbr_wt_hs10jams; 
	mb_io_ptr->mb_io_dimensions = &mbsys_hs10_dimensions; 
	mb_io_ptr->mb_io_extract = &mbsys_hs10_extract; 
	mb_io_ptr->mb_io_insert = &mbsys_hs10_insert; 
	mb_io_ptr->mb_io_extract_nav = &mbsys_hs10_extract_nav; 
	mb_io_ptr->mb_io_insert_nav = &mbsys_hs10_insert_nav; 
	mb_io_ptr->mb_io_extract_altitude = &mbsys_hs10_extract_altitude; 
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = NULL; 
	mb_io_ptr->mb_io_insert_svp = NULL; 
	mb_io_ptr->mb_io_ttimes = &mbsys_hs10_ttimes; 
	mb_io_ptr->mb_io_copyrecord = &mbsys_hs10_copy; 
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
int mbr_info_hs10jams(int verbose, 
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
	char	*function_name = "mbr_info_hs10jams";
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
	*system = MB_SYS_SINGLEBEAM;
	*beams_bath_max = 45;
	*beams_amp_max = 45;
	*pixels_ss_max = 0;
	strncpy(format_name, "HS10JAMS", MB_NAME_LENGTH);
	strncpy(system_name, "HS10", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_HS10JAMS\nInformal Description: Furuno HS-10 multibeam format,\nAttributes:           45 beams bathymetry and amplitude,\n                      ascii, JAMSTEC\n", MB_DESCRIPTION_LENGTH);
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
int mbr_alm_hs10jams(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_alm_hs10jams";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

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
	mb_io_ptr->structure_size = 0;
	mb_io_ptr->data_structure_size = 0;
	status = mb_malloc(verbose,sizeof(struct mbsys_hs10_struct),
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
int mbr_dem_hs10jams(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_hs10jams";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

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
int mbr_rt_hs10jams(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_hs10jams";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hs10_struct *store;
	char	line[MBF_HS10JAMS_MAXLINE];
	char	*line_ptr;
	int	shift;
	int	i;

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

	/* get pointers to mbio descriptor and data structures */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	store = (struct mbsys_hs10_struct *) store_ptr;

	/* set file position */
	mb_io_ptr->file_bytes = ftell(mb_io_ptr->mbfp);
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* read next record */
	if ((line_ptr = fgets(line, MBF_HS10JAMS_MAXLINE, 
			mb_io_ptr->mbfp)) != NULL) 
		{
		mb_io_ptr->file_bytes += strlen(line);
		if (strlen(line) >= MBF_HS10JAMS_LENGTH-2
			|| line[0] == '#')
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
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_EOF;
		}
	mb_io_ptr->file_bytes = ftell(mb_io_ptr->mbfp);

	/* translate values to data storage structure */
	if (status == MB_SUCCESS
		&& store != NULL)
		{
		/* deal with comment */
		if (line[0] == '#')
			{
			for (i=0;i<MBSYS_HS10_COMMENT;i++)
				store->comment[i] = '\0';
			store->kind = MB_DATA_COMMENT;
			strncpy(store->comment, &line[2], 
				MIN(MBSYS_HS10_COMMENT, 
				    strlen(&line[2]) - 2));
			}
		
		/* deal with survey ping */
		else
			{
			/* get time stamp */
			shift = 0;
			store->kind = MB_DATA_DATA;
			mb_get_int(&store->year, &line[shift], 2); shift += 2;
			mb_get_int(&store->month, &line[shift], 2); shift += 2;
			mb_get_int(&store->day, &line[shift], 2); shift += 2;
			mb_get_int(&store->hour, &line[shift], 2); shift += 2;
			mb_get_int(&store->minute, &line[shift], 2); shift += 2;
			mb_get_int(&store->tenth_second, &line[shift], 3); shift += 3;

			/* get navigation */
			store->NorS = line[shift]; shift += 1;
			mb_get_int(&store->latdeg, &line[shift], 3); shift += 3;
			mb_get_int(&store->latmin, &line[shift], 5); shift += 5;
			store->EorW = line[shift]; shift += 1;
			mb_get_int(&store->londeg, &line[shift], 3); shift += 3;
			mb_get_int(&store->lonmin, &line[shift], 5); shift += 5;
			mb_get_int(&store->heading, &line[shift], 4); shift += 4;
			mb_get_int(&store->center_depth, &line[shift], 5); shift += 5;
			
			/* get depth */
			for (i=0;i<MBSYS_HS10_BEAMS;i++)
				 {
				 mb_get_int(&store->depth[i], &line[shift], 5); shift += 5;
				 }
			
			/* get acrosstrack */
			for (i=0;i<MBSYS_HS10_BEAMS;i++)
				 {
				 mb_get_int(&store->acrosstrack[i], &line[shift], 5); shift += 5;
				 }
			
			/* get amplitude */
			for (i=0;i<MBSYS_HS10_BEAMS;i++)
				 {
				 mb_get_int(&store->amplitude[i], &line[shift], 5); shift += 5;
				 }
			}
		}

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = store->kind;

	/* print debug statements */
	if (verbose >= 5 
		&& status == MB_SUCCESS 
		&& store->kind == MB_DATA_DATA)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       year:             %d\n",store->year);
		fprintf(stderr,"dbg5       month:            %d\n",store->month);
		fprintf(stderr,"dbg5       day:              %d\n",store->day);
		fprintf(stderr,"dbg5       hour:             %d\n",store->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",store->minute);
		fprintf(stderr,"dbg5       tenth_second:     %d\n",store->tenth_second);
		fprintf(stderr,"dbg5       NorS:             %c\n",store->NorS);
		fprintf(stderr,"dbg5       latdeg:           %d\n",store->latdeg);
		fprintf(stderr,"dbg5       latmin:           %d\n",store->latmin);
		fprintf(stderr,"dbg5       EorW:             %c\n",store->NorS);
		fprintf(stderr,"dbg5       londeg:           %d\n",store->londeg);
		fprintf(stderr,"dbg5       lonmin:           %d\n",store->lonmin);
		fprintf(stderr,"dbg5       heading:          %d\n",store->heading);
		fprintf(stderr,"dbg5       center_depth:     %d\n",store->center_depth);
		fprintf(stderr,"dbg5       beam values (beam depth acrosstrack amplitude):\n");
		for (i=0;i<MBSYS_HS10_BEAMS;i++)
			fprintf(stderr,"dbg5       %2d %5d %5d %5d\n",
				i,store->depth[i],store->acrosstrack[i],store->amplitude[i]);
		}
	else if (verbose >= 5 
		&& status == MB_SUCCESS 
		&& store->kind == MB_DATA_COMMENT)
		{
		fprintf(stderr,"\ndbg5  Comment read in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       comment: %s\n",store->comment);
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
int mbr_wt_hs10jams(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_hs10jams";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hs10_struct *store;
	char	line[MBF_HS10JAMS_MAXLINE];
	int	shift;
	int	i;

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

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to storage data structure */
	store = (struct mbsys_hs10_struct *) store_ptr;

	/* write out debug info */
	if (verbose >= 5 
		&& store->kind == MB_DATA_DATA)
		{
		fprintf(stderr,"\ndbg5  Values to write in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       year:             %d\n",store->year);
		fprintf(stderr,"dbg5       month:            %d\n",store->month);
		fprintf(stderr,"dbg5       day:              %d\n",store->day);
		fprintf(stderr,"dbg5       hour:             %d\n",store->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",store->minute);
		fprintf(stderr,"dbg5       tenth_second:     %d\n",store->tenth_second);
		fprintf(stderr,"dbg5       NorS:             %c\n",store->NorS);
		fprintf(stderr,"dbg5       londeg:           %d\n",store->londeg);
		fprintf(stderr,"dbg5       lonmin:           %d\n",store->lonmin);
		fprintf(stderr,"dbg5       heading:          %d\n",store->heading);
		fprintf(stderr,"dbg5       center_depth:     %d\n",store->center_depth);
		fprintf(stderr,"dbg5       beam values (beam depth acrosstrack amplitude):\n");
		for (i=0;i<MBSYS_HS10_BEAMS;i++)
			fprintf(stderr,"dbg5       %2d %5d %5d %5d\n",
				i,store->depth[i],store->acrosstrack[i],store->amplitude[i]);
		}
	else if (verbose >= 5 
		&& store->kind == MB_DATA_COMMENT)
		{
		fprintf(stderr,"\ndbg5  Comment to write in MBIO function <%s>\n",function_name);
		fprintf(stderr,"dbg5       comment: %s\n",store->comment);
		}

	/* translate data from data storage structure */
	if (store->kind == MB_DATA_COMMENT)
		{
		/* deal with comment */
		for (i=0;i<MBF_HS10JAMS_MAXLINE;i++)
			line[i] = '\0';
		line[0] = '#';
		line[1] = '#';
		strncpy(&line[2], store->comment, MBSYS_HS10_COMMENT);
		line[strlen(store->comment)+2] = '\r';
		line[strlen(store->comment)+3] = '\n';
		}
	else
		{
		/* deal with survey ping */
		sprintf(line, 
		    "%2d%2d%2d%2d%2d%3d%c%3d%5d%c%3d%5d%4d%5d", 
		    store->year, 
		    store->month, 
		    store->day, 
		    store->hour, 
		    store->minute, 
		    store->tenth_second, 
		    store->NorS, 
		    store->latdeg, 
		    store->latmin, 
		    store->EorW, 
		    store->londeg, 
		    store->lonmin, 
		    store->heading, 
		    store->center_depth);
		shift = 40;
		for (i=0;i<MBSYS_HS10_BEAMS;i++)
			{
			sprintf(&line[shift], "%5d", store->depth[i]);
			shift += 5;
			}
		for (i=0;i<MBSYS_HS10_BEAMS;i++)
			{
			sprintf(&line[shift], "%5d", store->acrosstrack[i]);
			shift += 5;
			}
		for (i=0;i<MBSYS_HS10_BEAMS;i++)
			{
			sprintf(&line[shift], "%5d", store->amplitude[i]);
			shift += 5;
			}
		line[shift] = '\r'; shift += 1;
		line[shift] = '\n'; shift += 1;
		line[shift] = '\0';
		}

	/* write next data to file */
	if (fputs(line,mb_io_ptr->mbfp) == EOF)
		{
		*error = MB_ERROR_WRITE_FAIL;
		status = MB_FAILURE;
		}
	else
		{
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
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
