/*--------------------------------------------------------------------
 *    The MB-system:	mbr_sb2100b2.c	3/3/94
 *	$Id: mbr_sb2100b2.c,v 5.4 2001-07-27 19:07:16 caress Exp $
 *
 *    Copyright (c) 1997, 2000 by
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
 * mbr_sb2100b2.c contains the functions for reading and writing
 * multibeam data in the SB2100B2 format.  
 * These functions include:
 *   mbr_alm_sb2100b2	- allocate read/write memory
 *   mbr_dem_sb2100b2	- deallocate read/write memory
 *   mbr_rt_sb2100b2	- read and translate data
 *   mbr_wt_sb2100b2	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	March 3, 1994
 * $Log: not supported by cvs2svn $
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
 * Revision 4.5  2000/10/11  01:03:21  caress
 * Convert to ANSI C
 *
 * Revision 4.4  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
 *
 * Revision 4.3  1999/09/14  20:39:11  caress
 * Fixed bugs handling HSMD
 *
 * Revision 4.2  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.1  1997/09/15  19:06:40  caress
 * Real Version 4.5
 *
 * Revision 4.0  1997/04/21  17:01:19  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.0  1997/04/17  15:11:34  caress
 * MB-System 4.5 Beta Release
 *
 * Revision 1.1  1997/04/17  15:07:36  caress
 * Initial revision
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
#include "../../include/mbsys_sb2100.h"
#include "../../include/mbf_sb2100b2.h"

/* include for byte swapping on little-endian machines */
#ifdef BYTESWAPPED
#include "../../include/mb_swap.h"
#endif

/* essential function prototypes */
int mbr_register_sb2100b2(int verbose, void *mbio_ptr, 
		int *error);
int mbr_info_sb2100b2(int verbose, 
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
int mbr_alm_sb2100b2(int verbose, void *mbio_ptr, int *error);
int mbr_dem_sb2100b2(int verbose, void *mbio_ptr, int *error);
int mbr_zero_sb2100b2(int verbose, char *data_ptr, int *error);
int mbr_rt_sb2100b2(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_sb2100b2(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_sb2100b2_rd_data(int verbose, void *mbio_ptr, int *error);
int mbr_sb2100b2_rd_fh(int verbose, FILE *mbfp, int record_length, int *error);
int mbr_sb2100b2_rd_pr(int verbose, FILE *mbfp, 
		struct mbf_sb2100b2_struct *data, short record_length,
		int *error);
int mbr_sb2100b2_rd_tr(int verbose, FILE *mbfp, 
		struct mbf_sb2100b2_struct *data, short record_length,
		int *error);
int mbr_sb2100b2_rd_dh(int verbose, FILE *mbfp, 
		struct mbf_sb2100b2_struct *data, short record_length,
		int *error);
int mbr_sb2100b2_rd_br(int verbose, FILE *mbfp, 
		struct mbf_sb2100b2_struct *data, short record_length,
		int *error);
int mbr_sb2100b2_rd_sr(int verbose, FILE *mbfp, 
		struct mbf_sb2100b2_struct *data, short record_length,
		int *error);
int mbr_sb2100b2_wr_data(int verbose, void *mbio_ptr, char *data_ptr, int *error);
int mbr_sb2100b2_wr_fh(int verbose, FILE *mbfp, int *error);
int mbr_sb2100b2_wr_pr(int verbose, FILE *mbfp, char *data_ptr, int *error);
int mbr_sb2100b2_wr_tr(int verbose, FILE *mbfp, char *data_ptr, int *error);
int mbr_sb2100b2_wr_dh(int verbose, FILE *mbfp, char *data_ptr, int *error);
int mbr_sb2100b2_wr_br(int verbose, FILE *mbfp, char *data_ptr, int *error);
int mbr_sb2100b2_wr_sr(int verbose, FILE *mbfp, char *data_ptr, int *error);


/*--------------------------------------------------------------------*/
int mbr_register_sb2100b2(int verbose, void *mbio_ptr, int *error)
{
	static char res_id[]="$Id: mbr_sb2100b2.c,v 5.4 2001-07-27 19:07:16 caress Exp $";
	char	*function_name = "mbr_register_sb2100b2";
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
	status = mbr_info_sb2100b2(verbose, 
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_sb2100b2;
	mb_io_ptr->mb_io_format_free = &mbr_dem_sb2100b2; 
	mb_io_ptr->mb_io_store_alloc = &mbsys_sb2100_alloc; 
	mb_io_ptr->mb_io_store_free = &mbsys_sb2100_deall; 
	mb_io_ptr->mb_io_read_ping = &mbr_rt_sb2100b2; 
	mb_io_ptr->mb_io_write_ping = &mbr_wt_sb2100b2; 
	mb_io_ptr->mb_io_extract = &mbsys_sb2100_extract; 
	mb_io_ptr->mb_io_insert = &mbsys_sb2100_insert; 
	mb_io_ptr->mb_io_extract_nav = &mbsys_sb2100_extract_nav; 
	mb_io_ptr->mb_io_insert_nav = &mbsys_sb2100_insert_nav; 
	mb_io_ptr->mb_io_extract_altitude = &mbsys_sb2100_extract_altitude; 
	mb_io_ptr->mb_io_insert_altitude = NULL; 
	mb_io_ptr->mb_io_extract_svp = &mbsys_sb2100_extract_svp; 
	mb_io_ptr->mb_io_insert_svp = &mbsys_sb2100_insert_svp; 
	mb_io_ptr->mb_io_ttimes = &mbsys_sb2100_ttimes; 
	mb_io_ptr->mb_io_copyrecord = &mbsys_sb2100_copy; 
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
int mbr_info_sb2100b2(int verbose, 
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
	static char res_id[]="$Id: mbr_sb2100b2.c,v 5.4 2001-07-27 19:07:16 caress Exp $";
	char	*function_name = "mbr_info_sb2100b2";
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
	*system = MB_SYS_SB2100;
	*beams_bath_max = 151;
	*beams_amp_max = 151;
	*pixels_ss_max = 2000;
	strncpy(format_name, "SB2100B2", MB_NAME_LENGTH);
	strncpy(system_name, "SB2100", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_SB2100B2\nInformal Description: SeaBeam 2100 series vender format\nAttributes:           SeaBeam 2100, bathymetry and amplitude,  \n                      151 beams bathymetry,\n                      binary,\n                      SeaBeam Instruments and L-DEO.\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_YES;
	*traveltime = MB_YES;
	*beam_flagging = MB_YES;
	*nav_source = MB_DATA_DATA;
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
int mbr_alm_sb2100b2(int verbose, void *mbio_ptr, int *error)
{
	static char res_id[]="$Id: mbr_sb2100b2.c,v 5.4 2001-07-27 19:07:16 caress Exp $";
	char	*function_name = "mbr_alm_sb2100b2";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_sb2100_struct *store;

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
	mb_io_ptr->structure_size = sizeof(struct mbf_sb2100b2_struct);
	mb_io_ptr->data_structure_size = 0;
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mb_malloc(verbose,sizeof(struct mbsys_sb2100_struct),
				&mb_io_ptr->store_data,error);

	/* get store structure pointer */
	store = (struct mbsys_sb2100_struct *) mb_io_ptr->store_data;
				
	/* set comment pointer */
	store->comment = (char *) &(store->roll_bias_port);

	/* initialize everything to zeros */
	mbr_zero_sb2100b2(verbose,mb_io_ptr->raw_data,error);

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
int mbr_dem_sb2100b2(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_sb2100b2";
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

	/* deallocate memory for data descriptor */
	status = mb_free(verbose,&mb_io_ptr->raw_data,error);
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
int mbr_zero_sb2100b2(int verbose, char *data_ptr, int *error)
{
	char	*function_name = "mbr_zero_sb2100b2";
	int	status = MB_SUCCESS;
	struct mbf_sb2100b2_struct *data;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to data descriptor */
	data = (struct mbf_sb2100b2_struct *) data_ptr;

	/* initialize everything to zeros */
	if (data != NULL)
		{
		/* type of data record */
		data->kind = MB_DATA_NONE;
	
		/* set comment pointer */
		data->comment = (char *) &data->pr_year;
	
		/* sonar parameters (SB21BIPR) */
		data->pr_year = 0;
		data->pr_jday = 0;
		data->pr_hour = 0;
		data->pr_minute = 0;
		data->pr_sec = 0;
		data->pr_msec = 0;
		data->roll_bias_port = 0.0;		/* deg */
		data->roll_bias_starboard = 0.0;	/* deg */
		data->pitch_bias = 0.0;			/* deg */
		data->ship_draft = 0.0;			/* m */
		data->offset_x = 0.0;			/* m */
		data->offset_y = 0.0;			/* m */
		data->offset_z = 0.0;			/* m */
		data->num_svp = 0;
		for (i=0;i<MBF_SB2100B2_MAXVEL;i++)
		    {
		    data->svp[i].depth = 0.0;
		    data->svp[i].velocity = 0.0;
		    }
		
		/* sonar data header (SB21BIDH) */
		data->year = 0;
		data->jday = 0;
		data->hour = 0;
		data->minute = 0;
		data->sec = 0;
		data->msec = 0;
		data->longitude = 0.0;			/* degrees */
		data->latitude = 0.0;			/* degrees */
		data->heading = 0.0;			/* degrees */
		data->speed = 0.0;			/* m/sec */
		data->roll = 0.0;			/* degrees */
		data->pitch = 0.0;			/* degrees */
		data->heave = 0.0;			/* m */
		data->ssv = 0.0;			/* m/sec */
		data->frequency = 'L';			/* L=12kHz; H=36kHz; 2=20kHz */
		data->depth_gate_mode = 'A';		/* A=Auto, M=Manual */
		data->ping_gain = 0;			/* dB */
		data->ping_pulse_width = 0;		/* msec */
		data->transmitter_attenuation = 0;	/* dB */
		data->ssv_source = 'M';			/* V=Velocimeter, M=Manual, 
							    T=Temperature */
		data->svp_correction = 'T';		/* 0=None; A=True Xtrack 
							    and Apparent Depth;
							    T=True Xtrack and True Depth */
		data->pixel_algorithm = 'L';		/* pixel intensity algorithm
							    D = logarithm, L = linear */
		data->pixel_size = 0.0;			/* m */
		data->nbeams = 0;			/* up to 151 */
		data->npixels = 0;			/* up to 2000 */
		data->spare1 = 0;
		data->spare2 = 0;
		data->spare3 = 0;
		data->spare4 = 0;
		data->spare5 = 0;
		data->spare6 = 0;
	
		/* bathymetry record (SB21BIBR) */
		for (i=0;i<MBF_SB2100B2_BEAMS;i++)
			{
			data->beams[i].depth = 0.0;		/* m */
			data->beams[i].acrosstrack = 0.0;	/* m */
			data->beams[i].alongtrack = 0.0;	/* m */
			data->beams[i].range = 0.0;		/* seconds */
			data->beams[i].angle_across = 0.0;	/* degrees */
			data->beams[i].angle_forward = 0.0;	/* degrees */
			data->beams[i].amplitude = 0;	/* 0.25 dB */
			data->beams[i].signal_to_noise = 0;	/* dB */
			data->beams[i].echo_length = 0;		/* samples */
			data->beams[i].quality = '0';		/* 0=no data, 
							    Q=poor quality, 
							    blank otherwise */
			data->beams[i].source = 'W';		/* B=BDI, W=WMT */
			}
	
		}

	/* assume success */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

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
int mbr_rt_sb2100b2(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_sb2100b2";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sb2100b2_struct *data;
	struct mbsys_sb2100_struct *store;
	int	time_j[5];
	double	gain_db;
	double	gain_factor;
	int	center_pixel;
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
	data = (struct mbf_sb2100b2_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_sb2100_struct *) store_ptr;

	/* read next data from file */
	status = mbr_sb2100b2_rd_data(verbose,mbio_ptr,error);

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = data->kind;

	/* translate values to sb2100 data storage structure */
	if (status == MB_SUCCESS
		&& store != NULL)
		{
		/* type of data record */
		store->kind = data->kind;
		
		/* copy comment if required */
		if (data->kind == MB_DATA_COMMENT)
		    strncpy(store->comment,data->comment,
			    MBSYS_SB2100_MAXLINE);
		    
		/* else copy data */
		else
		    {
		    
		    /* sonar parameters (SB21BIPR) */
		    if (data->kind == MB_DATA_VELOCITY_PROFILE)
			{
			store->year = data->pr_year;
			store->jday = data->pr_jday;
			store->hour = data->pr_hour;
			store->minute = data->pr_minute;
			store->sec = data->pr_sec;
			store->msec = data->pr_msec;
			}
		    store->roll_bias_port = data->roll_bias_port;
		    store->roll_bias_starboard = data->roll_bias_starboard;
		    store->pitch_bias = data->pitch_bias;
		    store->ship_draft = data->ship_draft;
		    store->offset_x = data->offset_x;
		    store->offset_y = data->offset_y;
		    store->offset_z = data->offset_z;
		    store->num_svp = data->num_svp;
		    for (i=0;i<MBF_SB2100B2_MAXVEL;i++)
			{
			store->svp[i].depth = data->svp[i].depth;
			store->svp[i].velocity = data->svp[i].velocity;
			}
		    
		    /* sonar data header (SB21BIDH) */
		    if (data->kind != MB_DATA_VELOCITY_PROFILE)
			{
			store->year = data->year;
			store->jday = data->jday;
			store->hour = data->hour;
			store->minute = data->minute;
			store->sec = data->sec;
			store->msec = data->msec;
			}
		    store->longitude = data->longitude;
		    store->latitude = data->latitude;
		    store->heading = data->heading;
		    store->speed = data->speed;
		    store->roll = data->roll;
		    store->pitch = data->pitch;
		    store->heave = data->heave;
		    store->ssv = data->ssv;
		    store->frequency = data->frequency;
		    store->depth_gate_mode = data->depth_gate_mode;
		    store->ping_gain = data->ping_gain;
		    store->ping_pulse_width = data->ping_pulse_width;
		    store->transmitter_attenuation = data->transmitter_attenuation;
		    store->ssv_source = data->ssv_source;
		    store->svp_correction = data->svp_correction;
		    store->pixel_algorithm = data->pixel_algorithm;
		    store->pixel_size = data->pixel_size;
		    store->nbeams = data->nbeams;
		    store->npixels = data->npixels;
		    store->spare1 = data->spare1;
		    store->spare2 = data->spare2;
		    store->spare3 = data->spare3;
		    store->spare4 = data->spare4;
		    store->spare5 = data->spare5;
		    store->spare6 = data->spare6;
		    
		    /* bathymetry record (SB21BIBR) */
		    for (i=0;i<MBF_SB2100B2_BEAMS;i++)
			    {
			    store->beams[i].depth = data->beams[i].depth;
			    store->beams[i].acrosstrack = data->beams[i].acrosstrack;
			    store->beams[i].alongtrack = data->beams[i].alongtrack;
			    store->beams[i].range = data->beams[i].range;
			    store->beams[i].angle_across = data->beams[i].angle_across;
			    store->beams[i].angle_forward = data->beams[i].angle_forward;
			    store->beams[i].amplitude = data->beams[i].amplitude;
			    store->beams[i].signal_to_noise = data->beams[i].signal_to_noise;
			    store->beams[i].echo_length = data->beams[i].echo_length;
			    store->beams[i].quality = data->beams[i].quality;
			    store->beams[i].source = data->beams[i].source;
			    }
		    
		    /* parameters for MBF_SB2100RW format */
		    store->range_scale = ' ';
		    store->spare_dr[0] = ' ';
		    store->spare_dr[1] = ' ';
		    store->num_algorithms = 1;
		    for (i=0;i<4;i++)
			store->algorithm_order[i] = ' ';
		    store->svp_corr_ss = 0;
		    store->ss_data_length = 4 * MBSYS_SB2100_PIXELS;
		    store->pixel_size_scale = 'D';
		    store->spare_ss = ' ';
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
int mbr_wt_sb2100b2(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_sb2100b2";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sb2100b2_struct *data;
	char	*data_ptr;
	struct mbsys_sb2100_struct *store;
	int	time_j[5];
	double	gain_db;
	double	gain_factor;
	int	center_pixel;
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

	/* get pointer to raw data structure */
	data = (struct mbf_sb2100b2_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;
	store = (struct mbsys_sb2100_struct *) store_ptr;

	/* first translate values from data storage structure */
	if (store != NULL)
		{
		/* type of data record */
		data->kind = store->kind;
		
		/* copy comment if required */
		if (store->kind == MB_DATA_COMMENT)
		    strncpy(data->comment,store->comment,
			    MBF_SB2100B2_MAXLINE);
		    
		/* else copy data */
		else
		    {
		    
		    /* sonar parameters (SB21BIPR) */
		    if (data->kind == MB_DATA_VELOCITY_PROFILE)
			{
			data->pr_year = store->year;
			data->pr_jday = store->jday;
			data->pr_hour = store->hour;
			data->pr_minute = store->minute;
			data->pr_sec = store->sec;
			data->pr_msec = store->msec;
			}
		    data->roll_bias_port = store->roll_bias_port;
		    data->roll_bias_starboard = store->roll_bias_starboard;
		    data->pitch_bias = store->pitch_bias;
		    data->ship_draft = store->ship_draft;
		    data->offset_x = store->offset_x;
		    data->offset_y = store->offset_y;
		    data->offset_z = store->offset_z;
		    data->num_svp = store->num_svp;
		    for (i=0;i<MBF_SB2100B2_MAXVEL;i++)
			{
			data->svp[i].depth = store->svp[i].depth;
			data->svp[i].velocity = store->svp[i].velocity;
			}
		    
		    /* sonar data header (SB21BIDH) */
		    if (data->kind != MB_DATA_VELOCITY_PROFILE)
			{
			data->year = store->year;
			data->jday = store->jday;
			data->hour = store->hour;
			data->minute = store->minute;
			data->sec = store->sec;
			data->msec = store->msec;
			}
		    data->longitude = store->longitude;
		    data->latitude = store->latitude;
		    data->heading = store->heading;
		    data->speed = store->speed;
		    data->roll = store->roll;
		    data->pitch = store->pitch;
		    data->heave = store->heave;
		    data->ssv = store->ssv;
		    data->frequency = store->frequency;
		    data->depth_gate_mode = store->depth_gate_mode;
		    data->ping_gain = store->ping_gain;
		    data->ping_pulse_width = store->ping_pulse_width;
		    data->transmitter_attenuation = store->transmitter_attenuation;
		    data->ssv_source = store->ssv_source;
		    data->svp_correction = store->svp_correction;
		    data->pixel_algorithm = store->pixel_algorithm;
		    data->pixel_size = store->pixel_size;
		    data->nbeams = store->nbeams;
		    data->npixels = store->npixels;
		    data->spare1 = store->spare1;
		    data->spare2 = store->spare2;
		    data->spare3 = store->spare3;
		    data->spare4 = store->spare4;
		    data->spare5 = store->spare5;
		    data->spare6 = store->spare6;
		    
		    /* bathymetry record (SB21BIBR) */
		    for (i=0;i<MBF_SB2100B2_BEAMS;i++)
			    {
			    data->beams[i].depth = store->beams[i].depth;
			    data->beams[i].acrosstrack = store->beams[i].acrosstrack;
			    data->beams[i].alongtrack = store->beams[i].alongtrack;
			    data->beams[i].range = store->beams[i].range;
			    data->beams[i].angle_across = store->beams[i].angle_across;
			    data->beams[i].angle_forward = store->beams[i].angle_forward;
			    data->beams[i].amplitude = store->beams[i].amplitude;
			    data->beams[i].signal_to_noise = store->beams[i].signal_to_noise;
			    data->beams[i].echo_length = store->beams[i].echo_length;
			    data->beams[i].quality = store->beams[i].quality;
			    data->beams[i].source = store->beams[i].source;
			    }
		    }
		}

	/* write next data to file */
	status = mbr_sb2100b2_wr_data(verbose,mbio_ptr,data_ptr,error);

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
int mbr_sb2100b2_rd_data(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_sb2100b2_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sb2100b2_struct *data;
	char	*data_ptr;
	FILE	*mbfp;
	int	done;
	char	*label;
	int	*label_save_flag;
	int	type;
	int	expect;
	short	record_length;
	char	*record_length_ptr;
	char	record_length_fh_str[8];
	int	record_length_fh;
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

	/* get pointer to raw data structure */
	data = (struct mbf_sb2100b2_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;
	mbfp = mb_io_ptr->mbfp;
	
	/* get saved values */
	label = (char *) mb_io_ptr->save_label;
	label_save_flag = (int *) &mb_io_ptr->save_label_flag;
	record_length_ptr = (char *) &record_length;

	/* initialize everything to zeros */
	mbr_zero_sb2100b2(verbose,data_ptr,error);

	done = MB_NO;
	expect = MBF_SB2100B2_NONE;
	while (done == MB_NO)
		{
		/* if no label saved get next record label */
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		if (*label_save_flag == MB_NO)
			{
			/* get next 10 bytes */
			if ((status = fread(&label[0],
				10, 
				1, mbfp)) != 1)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				}

			/* if not a format 42 label read individual 
			    bytes until label found or eof */
			while (status == MB_SUCCESS
			    && strncmp(label, "SB21BI", 6) != 0)
			    {
			    for (i=0;i<9;i++)
				label[i] = label[i+1];
			    if ((status = fread(&label[9],
				    1, 1, mbfp)) != 1)
				{
				status = MB_FAILURE;
				*error = MB_ERROR_EOF;
				}
			    }
			}
		
		/* else use saved label */
		else
			*label_save_flag = MB_NO;
			
		/* get the label type */
		if (status == MB_SUCCESS)
			{
			/* get type */
			type = MBF_SB2100B2_NONE;
			for (i=1;i<=MBF_SB2100B2_RECORDS;i++)
				if (strncmp(label, mbf_sb2100b2_labels[i], 8) == 0)
				    type = i;
			
			/* get the record length */
			if (type != MBF_SB2100B2_FH)
			    {
#ifndef BYTESWAPPED
			    record_length_ptr[0] = label[8];
			    record_length_ptr[1] = label[9];
#else
			    record_length_ptr[0] = label[9];
			    record_length_ptr[1] = label[8];
#endif
			    }
			else
			    {
			    record_length_fh_str[0] = label[8];
			    record_length_fh_str[1] = label[9];
			    if ((status = fread(&record_length_fh_str[2],
				    4, 1, mbfp)) != 1)
				    {
				    status = MB_FAILURE;
				    *error = MB_ERROR_EOF;
				    }
			    record_length_fh_str[6] = 0;
			    record_length_fh_str[7] = 0;
			    sscanf(record_length_fh_str, "%d", &record_length_fh);
			    }
			}

		/* read the appropriate data records */
		if ((status == MB_FAILURE || type == MBF_SB2100B2_NONE)
			&& expect == MBF_SB2100B2_NONE)
			{
			done = MB_YES;
			}
		else if ((status == MB_FAILURE || type == MBF_SB2100B2_NONE)
			&& expect != MBF_SB2100B2_NONE)
			{
			done = MB_YES;
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		else if (expect != MBF_SB2100B2_NONE && expect != type)
			{
			done = MB_YES;
			expect = MBF_SB2100B2_NONE;
			*label_save_flag = MB_YES;
			}
		else if (type == MBF_SB2100B2_FH)
			{
			status = mbr_sb2100b2_rd_fh(
				verbose,mbfp,record_length_fh,error);
			if (status == MB_SUCCESS)
				{
				done = MB_NO;
				expect = MBF_SB2100B2_NONE;
				data->kind = MB_DATA_NONE;
				}
			}
		else if (type == MBF_SB2100B2_PR)
			{
			status = mbr_sb2100b2_rd_pr(
				verbose,mbfp,data,record_length,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				data->kind = MB_DATA_VELOCITY_PROFILE;
				}
			}
		else if (type == MBF_SB2100B2_TR)
			{
			status = mbr_sb2100b2_rd_tr(
				verbose,mbfp,data,record_length,error);
			if (status == MB_SUCCESS)
				{
				done = MB_YES;
				data->kind = MB_DATA_COMMENT;
				}
			}
		else if (type == MBF_SB2100B2_DH)
			{
			status = mbr_sb2100b2_rd_dh(
				verbose,mbfp,data,record_length,error);
			if (status == MB_SUCCESS)
				{
				done = MB_NO;
				data->kind = MB_DATA_DATA;
				expect = MBF_SB2100B2_BR;
				}
			}
		else if (type == MBF_SB2100B2_BR)
			{
			status = mbr_sb2100b2_rd_br(
				verbose,mbfp,data,record_length,error);
			if (status == MB_SUCCESS 
				&& expect == MBF_SB2100B2_BR)
				{
				done = MB_NO;
				data->kind = MB_DATA_DATA;
				expect = MBF_SB2100B2_SR;
				}
			else if (status == MB_SUCCESS)
				{
				done = MB_YES;
				expect = MBF_SB2100B2_NONE;
				*error = MB_ERROR_UNINTELLIGIBLE;
				status = MB_FAILURE;
				}
			else if (status == MB_FAILURE)
				{
				done = MB_YES;
				expect = MBF_SB2100B2_NONE;
				}
			}
		else if (type == MBF_SB2100B2_SR)
			{
			status = mbr_sb2100b2_rd_sr(
				verbose,mbfp,data,record_length,error);
			if (status == MB_SUCCESS 
				&& expect == MBF_SB2100B2_SR)
				{
				done = MB_YES;
				}
			else if (status == MB_SUCCESS)
				{
				done = MB_YES;
				expect = MBF_SB2100B2_NONE;
				*error = MB_ERROR_UNINTELLIGIBLE;
				status = MB_FAILURE;
				}
			else if (status == MB_FAILURE
				&& *error ==  MB_ERROR_UNINTELLIGIBLE
				&& expect == MBF_SB2100B2_SR)
				{
				/* this preserves the bathymetry
				   that has already been read */
				done = MB_YES;
				status = MB_SUCCESS;
				*error = MB_ERROR_NO_ERROR;
				}
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
int mbr_sb2100b2_rd_fh(int verbose, FILE *mbfp, int record_length, int *error)
{
	char	*function_name = "mbr_sb2100b2_rd_fh";
	int	status = MB_SUCCESS;
	int	read_length;
	char	read_buffer[100];
	int	nread;
	int	nlast;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       record_len: %d\n",record_length);
		}

	/* check record size */
	if (record_length > 100000)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
		}

	/* if success read rest of record */
	if (status == MB_SUCCESS)
		{
		/* read data into buffer */
		nread = record_length / 100;
		nlast = record_length % 100;
		for (i=0;i<nread;i++)		    
		    if ((status = fread(read_buffer,
			    100, 1, mbfp)) != 1)
			    {
			    status = MB_FAILURE;
			    *error = MB_ERROR_EOF;
			    }
		if (nlast > 0)		    
		    if ((status = fread(read_buffer,
			    nlast, 1, mbfp)) != 1)
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
int mbr_sb2100b2_rd_pr(int verbose, FILE *mbfp, 
		struct mbf_sb2100b2_struct *data, short record_length,
		int *error)
{
	char	*function_name = "mbr_sb2100b2_rd_pr";
	int	status = MB_SUCCESS;
	int	read_length;
	unsigned int	*checksum_read;
	unsigned int	checksum;
	char	*checksum_ptr;
	char	eor_read[6];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		fprintf(stderr,"dbg2       record_len: %d\n",record_length);
		}

	/* check record size */
	if (record_length != MBF_SB2100B2_PR_WRITE_LEN + 6)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
		}

	/* if success read rest of record */
	if (status == MB_SUCCESS)
		{
		/* read data into structure */
		read_length = MBF_SB2100B2_PR_WRITE_LEN;
		if ((status = fread(&(data->pr_year),
			read_length, 1, mbfp)) != 1)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}

		/* read checksum and eor */
		if ((status = fread(&eor_read[0],
			6, 1, mbfp)) != 1)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		checksum_read = (unsigned int *) &eor_read[0];
#ifdef BYTESWAPPED
		*checksum_read = (unsigned int) mb_swap_int(*checksum_read);
#endif

		/* do checksum */
		if (verbose > 1)
		    {
		    checksum = 0;
		    checksum_ptr = (char *) &(data->pr_year);
		    for (i=0;i<read_length;i++)
			    checksum += (unsigned int) checksum_ptr[i];
    
		    /* check checksum */
		    if (checksum != *checksum_read)
			    {
			    status = MB_FAILURE;
			    *error = MB_ERROR_UNINTELLIGIBLE;
			    }
		    }
			
#ifdef BYTESWAPPED
		/* byte swap everything */
		data->pr_year =	    (short int) mb_swap_short(data->pr_year);
		data->pr_jday =	    (short int) mb_swap_short(data->pr_jday);
		data->pr_hour =	    (short int) mb_swap_short(data->pr_hour);
		data->pr_minute =   (short int) mb_swap_short(data->pr_minute);
		data->pr_sec =	    (short int) mb_swap_short(data->pr_sec);
		data->pr_msec =	    (short int) mb_swap_short(data->pr_msec);
		mb_swap_float(&(data->roll_bias_port));
		mb_swap_float(&(data->roll_bias_starboard));
		mb_swap_float(&(data->pitch_bias));
		mb_swap_float(&(data->ship_draft));
		mb_swap_float(&(data->offset_x));
		mb_swap_float(&(data->offset_y));
		mb_swap_float(&(data->offset_z));
		data->num_svp =	    (int) mb_swap_int(data->num_svp);
		for (i=0;i<data->num_svp;i++)
			{
			mb_swap_float(&(data->svp[i].depth));
			mb_swap_float(&(data->svp[i].velocity));
			}
#endif
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",data->pr_year);
		fprintf(stderr,"dbg5       julian day:       %d\n",data->pr_jday);
		fprintf(stderr,"dbg5       hour:             %d\n",data->pr_hour);
		fprintf(stderr,"dbg5       minute:           %d\n",data->pr_minute);
		fprintf(stderr,"dbg5       sec:              %d\n",data->pr_sec);
		fprintf(stderr,"dbg5       msec:             %d\n",data->pr_msec);
		fprintf(stderr,"dbg5       roll_bias_port:   %f\n",data->roll_bias_port);
		fprintf(stderr,"dbg5       roll_bias_strbrd: %f\n",data->roll_bias_starboard);
		fprintf(stderr,"dbg5       pitch_bias:       %f\n",data->pitch_bias);
		fprintf(stderr,"dbg5       ship_draft:       %f\n",data->ship_draft);
		fprintf(stderr,"dbg5       offset_x:         %f\n",data->offset_x);
		fprintf(stderr,"dbg5       offset_y:         %f\n",data->offset_y);
		fprintf(stderr,"dbg5       offset_z:         %f\n",data->offset_z);
		fprintf(stderr,"dbg5       num_svp:          %d\n",data->num_svp);
		fprintf(stderr,"dbg5       Sound Velocity Profile:\n");
		for (i=0;i<data->num_svp;i++)
			fprintf(stderr,"dbg5       %d  depth:%f  velocity:%f\n",
				i,data->svp[i].depth,data->svp[i].velocity);
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
int mbr_sb2100b2_rd_tr(int verbose, FILE *mbfp, 
		struct mbf_sb2100b2_struct *data, short record_length,
		int *error)
{
	char	*function_name = "mbr_sb2100b2_rd_tr";
	int	status = MB_SUCCESS;
	int	read_length;
	unsigned int	*checksum_read;
	unsigned int	checksum;
	char	*checksum_ptr;
	char	eor_read[6];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		fprintf(stderr,"dbg2       record_len: %d\n",record_length);
		}

	/* check record size */
	if (record_length > MBF_SB2100B2_MAXLINE + 6)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
		}

	/* if success read rest of record */
	if (status == MB_SUCCESS)
		{
		/* read data into structure */
		read_length = record_length - 6;
		if ((status = fread(data->comment,
			read_length, 1, mbfp)) != 1)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}

		/* read checksum and eor */
		if ((status = fread(&eor_read[0],
			6, 1, mbfp)) != 1)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		checksum_read = (unsigned int *) &eor_read[0];
#ifdef BYTESWAPPED
		*checksum_read = (unsigned int) mb_swap_int(*checksum_read);
#endif

		/* do checksum */
		if (verbose > 1)
		    {
		    checksum = 0;
		    checksum_ptr = (char *) data->comment;
		    for (i=0;i<read_length;i++)
			    checksum += (unsigned int) checksum_ptr[i];
    
		    /* check checksum */
		    if (checksum != *checksum_read)
			    {
			    status = MB_FAILURE;
			    *error = MB_ERROR_UNINTELLIGIBLE;
			    }
		    }
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Value read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       comment:          %s\n",
			data->comment);
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
int mbr_sb2100b2_rd_dh(int verbose, FILE *mbfp, 
		struct mbf_sb2100b2_struct *data, short record_length,
		int *error)
{
	char	*function_name = "mbr_sb2100b2_rd_dh";
	int	status = MB_SUCCESS;
	int	read_length;
	unsigned int	*checksum_read;
	unsigned int	checksum;
	char	*checksum_ptr;
	char	eor_read[6];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		fprintf(stderr,"dbg2       record_len: %d\n",record_length);
		}

	/* check record size */
	if (record_length != MBF_SB2100B2_DH_WRITE_LEN + 6)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
		}

	/* if success read rest of record */
	if (status == MB_SUCCESS)
		{
		/* read data into structure */
		read_length = MBF_SB2100B2_DH_WRITE_LEN;
		if ((status = fread(&(data->year),
			read_length, 1, mbfp)) != 1)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}

		/* read checksum and eor */
		if ((status = fread(&eor_read[0],
			6, 1, mbfp)) != 1)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		checksum_read = (unsigned int *) &eor_read[0];
#ifdef BYTESWAPPED
		*checksum_read = (unsigned int) mb_swap_int(*checksum_read);
#endif

		/* do checksum */
		if (verbose > 1)
		    {
		    checksum = 0;
		    checksum_ptr = (char *) &(data->year);
		    for (i=0;i<read_length;i++)
			    checksum += (unsigned int) checksum_ptr[i];
    
		    /* check checksum */
		    if (checksum != *checksum_read)
			    {
			    status = MB_FAILURE;
			    *error = MB_ERROR_UNINTELLIGIBLE;
			    }
		    }
		
#ifdef BYTESWAPPED
		/* byte swap everything */
		data->year =	    (short int) mb_swap_short(data->year);
		data->jday =	    (short int) mb_swap_short(data->jday);
		data->hour =	    (short int) mb_swap_short(data->hour);
		data->minute =   (short int) mb_swap_short(data->minute);
		data->sec =	    (short int) mb_swap_short(data->sec);
		data->msec =	    (short int) mb_swap_short(data->msec);
		mb_swap_double(&(data->longitude));
		mb_swap_double(&(data->latitude));
		mb_swap_float(&(data->heading));
		mb_swap_float(&(data->speed));
		mb_swap_float(&(data->roll));
		mb_swap_float(&(data->pitch));
		mb_swap_float(&(data->heave));
		mb_swap_float(&(data->ssv));
		mb_swap_float(&(data->pixel_size));
		data->nbeams =	    (int) mb_swap_int(data->nbeams);
		data->npixels =	    (int) mb_swap_int(data->npixels);
		data->spare1 =	    (int) mb_swap_short(data->spare1);
		data->spare2 =	    (int) mb_swap_short(data->spare2);
		data->spare3 =	    (int) mb_swap_short(data->spare3);
		data->spare4 =	    (int) mb_swap_short(data->spare4);
		data->spare5 =	    (int) mb_swap_short(data->spare5);
		data->spare6 =	    (int) mb_swap_short(data->spare6);
#endif
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       julian day:       %d\n",data->jday);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",data->minute);
		fprintf(stderr,"dbg5       sec:              %d\n",data->sec);
		fprintf(stderr,"dbg5       msec:             %d\n",data->msec);
		fprintf(stderr,"dbg5       longitude:        %f\n",data->longitude);
		fprintf(stderr,"dbg5       latitude:         %f\n",data->latitude);
		fprintf(stderr,"dbg5       heading:          %f\n",data->heading);
		fprintf(stderr,"dbg5       speed:            %f\n",data->speed);
		fprintf(stderr,"dbg5       roll:             %f\n",data->roll);
		fprintf(stderr,"dbg5       pitch:            %f\n",data->pitch);
		fprintf(stderr,"dbg5       heave:            %f\n",data->heave);
		fprintf(stderr,"dbg5       ssv:              %f\n",data->ssv);
		fprintf(stderr,"dbg5       frequency:        %c\n",data->frequency);
		fprintf(stderr,"dbg5       depth_gate_mode:  %d\n",data->depth_gate_mode);
		fprintf(stderr,"dbg5       ping_gain:        %d\n",data->ping_gain);
		fprintf(stderr,"dbg5       ping_pulse_width: %d\n",data->ping_pulse_width);
		fprintf(stderr,"dbg5       trans_atten:      %d\n",data->transmitter_attenuation);
		fprintf(stderr,"dbg5       ssv_source:       %c\n",data->ssv_source);
		fprintf(stderr,"dbg5       svp_correction:   %c\n",data->svp_correction);
		fprintf(stderr,"dbg5       pixel_algorithm:  %c\n",data->pixel_algorithm);
		fprintf(stderr,"dbg5       pixel_size:       %f\n",data->pixel_size);
		fprintf(stderr,"dbg5       nbeams:           %d\n",data->nbeams);
		fprintf(stderr,"dbg5       npixels:          %d\n",data->npixels);
		fprintf(stderr,"dbg5       spare1:           %d\n",data->spare1);
		fprintf(stderr,"dbg5       spare2:           %d\n",data->spare2);
		fprintf(stderr,"dbg5       spare3:           %d\n",data->spare3);
		fprintf(stderr,"dbg5       spare4:           %d\n",data->spare4);
		fprintf(stderr,"dbg5       spare5:           %d\n",data->spare5);
		fprintf(stderr,"dbg5       spare6:           %d\n",data->spare6);
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
int mbr_sb2100b2_rd_br(int verbose, FILE *mbfp, 
		struct mbf_sb2100b2_struct *data, short record_length,
		int *error)
{
	char	*function_name = "mbr_sb2100b2_rd_br";
	int	status = MB_SUCCESS;
	int	read_length;
	unsigned int	*checksum_read;
	unsigned int	checksum;
	char	*checksum_ptr;
	char	eor_read[6];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		fprintf(stderr,"dbg2       record_len: %d\n",record_length);
		}

	/* check record size */
	if (record_length != data->nbeams * MBF_SB2100B2_BR_WRITE_LEN + 6)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
		}

	/* if success read rest of record */
	if (status == MB_SUCCESS)
		{
		/* read data into structure */
		read_length = data->nbeams * MBF_SB2100B2_BR_WRITE_LEN;
		if (read_length > 0)
		if ((status = fread(data->beams,
			read_length, 1, mbfp)) != 1)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}

		/* read checksum and eor */
		if ((status = fread(&eor_read[0],
			6, 1, mbfp)) != 1)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		checksum_read = (unsigned int *) &eor_read[0];
#ifdef BYTESWAPPED
		*checksum_read = (unsigned int) mb_swap_int(*checksum_read);
#endif

		/* do checksum */
		if (verbose > 1)
		    {
		    checksum = 0;
		    checksum_ptr = (char *) data->beams;
		    for (i=0;i<read_length;i++)
			    checksum += (unsigned int) checksum_ptr[i];
    
		    /* check checksum */
		    if (checksum != *checksum_read)
			    {
			    status = MB_FAILURE;
			    *error = MB_ERROR_UNINTELLIGIBLE;
			    }
		    }
			
#ifdef BYTESWAPPED
		/* byte swap everything */
		for (i=0;i<data->nbeams;i++)
			{
			mb_swap_float(&(data->beams[i].depth));
			mb_swap_float(&(data->beams[i].acrosstrack));
			mb_swap_float(&(data->beams[i].alongtrack));
			mb_swap_float(&(data->beams[i].range));
			mb_swap_float(&(data->beams[i].angle_across));
			mb_swap_float(&(data->beams[i].angle_forward));
			data->beams[i].amplitude 
			    = (short int) mb_swap_short(data->beams[i].amplitude);
			data->beams[i].signal_to_noise 
			    = (short int) mb_swap_short(data->beams[i].signal_to_noise);
			data->beams[i].echo_length 
			    = (int) mb_swap_short(data->beams[i].echo_length);
			}
#endif
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       beam depth xtrack ltrack tt angle angfor amp sig2noise echo src quality\n");
		for (i=0;i<data->nbeams;i++)
		  {
		  fprintf(stderr,"dbg5       %3d %8.2f %9.2f %8.2f %6.3f %7.3f %7.3f %3d %3d %3d %c %c\n",
			i,
			data->beams[i].depth,
			data->beams[i].acrosstrack,
			data->beams[i].alongtrack,
			data->beams[i].range,
			data->beams[i].angle_across,
			data->beams[i].angle_forward,
			data->beams[i].amplitude,
			data->beams[i].signal_to_noise,
			data->beams[i].echo_length,
			data->beams[i].source,
			data->beams[i].quality);
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
int mbr_sb2100b2_rd_sr(int verbose, FILE *mbfp, 
		struct mbf_sb2100b2_struct *data, short record_length,
		int *error)
{
	char	*function_name = "mbr_sb2100b2_rd_sr";
	int	status = MB_SUCCESS;
	int	read_length;
	unsigned int	*checksum_read;
	unsigned int	checksum;
	char	*checksum_ptr;
	char	eor_read[6];
	char	ssbuffer[4*MBSYS_SB2100_PIXELS];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data:       %d\n",data);
		fprintf(stderr,"dbg2       record_len: %d\n",record_length);
		}

	/* check record size */
	if (record_length != data->npixels * MBF_SB2100B2_SR_WRITE_LEN + 6)
		{
		status = MB_FAILURE;
		*error = MB_ERROR_UNINTELLIGIBLE;
		}

	/* if success read rest of record */
	if (status == MB_SUCCESS)
		{
		/* read data into structure */
		read_length = data->npixels * MBF_SB2100B2_SR_WRITE_LEN;
		if (read_length > 0)
		if ((status = fread(ssbuffer,
			read_length, 1, mbfp)) != 1)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}

		/* read checksum and eor */
		if ((status = fread(&eor_read[0],
			6, 1, mbfp)) != 1)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
		checksum_read = (unsigned int *) &eor_read[0];
#ifdef BYTESWAPPED
		*checksum_read = (unsigned int) mb_swap_int(*checksum_read);
#endif

		/* do checksum */
		if (verbose > 1)
		    {
		    checksum = 0;
		    checksum_ptr = (char *) ssbuffer;
		    for (i=0;i<read_length;i++)
			    checksum += (unsigned int) checksum_ptr[i];
    
		    /* check checksum */
		    if (checksum != *checksum_read)
			    {
			    status = MB_FAILURE;
			    *error = MB_ERROR_UNINTELLIGIBLE;
			    }
		    }
		}
		
	/* zero the number of sidescan pixels */
	data->npixels = 0;

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
int mbr_sb2100b2_wr_data(int verbose, void *mbio_ptr, char *data_ptr, int *error)
{
	char	*function_name = "mbr_sb2100b2_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_sb2100b2_struct *data;
	FILE	*mbfp;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get pointer to raw data structure */
	data = (struct mbf_sb2100b2_struct *) data_ptr;
	mbfp = mb_io_ptr->mbfp;
	
	/* write file header if not written yet */
	if (mb_io_ptr->save_flag == MB_NO)
		{
		status = mbr_sb2100b2_wr_fh(verbose,mbfp,error);
		mb_io_ptr->save_flag = MB_YES;
		}

	if (data->kind == MB_DATA_VELOCITY_PROFILE)
		{
		status = mbr_sb2100b2_wr_pr(verbose,mbfp,data_ptr,error);
		}
	else if (data->kind == MB_DATA_COMMENT)
		{
		status = mbr_sb2100b2_wr_tr(verbose,mbfp,data_ptr,error);
		}
	else if (data->kind == MB_DATA_DATA)
		{
		/* zero the number of sidescan pixels */
		data->npixels = 0;
		
		status = mbr_sb2100b2_wr_dh(verbose,mbfp,data_ptr,error);
		status = mbr_sb2100b2_wr_br(verbose,mbfp,data_ptr,error);
		status = mbr_sb2100b2_wr_sr(verbose,mbfp,data_ptr,error);
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_BAD_KIND;
		}

	/* print output debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Data record kind in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       kind:       %d\n",data->kind);
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
int mbr_sb2100b2_wr_fh(int verbose, FILE *mbfp, int *error)
{
	char	*function_name = "mbr_sb2100b2_wr_fh";
	int	status = MB_SUCCESS;
	int	record_length;
	char	record_length_str[8];
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		}

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       file_header_text: \n%s%s\n", 
			mbf_sb2100b2_file_header_text_1, 
			mbf_sb2100b2_file_header_text_2);
		}

	/* write the record label */
	if (fwrite(mbf_sb2100b2_labels[MBF_SB2100B2_FH],
		MBF_SB2100B2_LABEL_LEN, 1, mbfp) != 1)
		{
		*error = MB_ERROR_WRITE_FAIL;
		status = MB_FAILURE;
		}
	else
		{
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		}

	/* write the record length */
	if (status == MB_SUCCESS)
		{
		record_length = strlen(mbf_sb2100b2_file_header_text_1)
		    + strlen(mbf_sb2100b2_file_header_text_2);
		sprintf(record_length_str, "%6d", record_length);
		if (fwrite(record_length_str, 6, 1, mbfp) != 1)
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

	/* write out the data */
	if (status == MB_SUCCESS)
		{			
		/* write the data */
		if (fwrite(mbf_sb2100b2_file_header_text_1, 
			strlen(mbf_sb2100b2_file_header_text_1), 
			1, mbfp) != 1)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		if (fwrite(mbf_sb2100b2_file_header_text_2, 
			strlen(mbf_sb2100b2_file_header_text_2), 
			1, mbfp) != 1)
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
int mbr_sb2100b2_wr_pr(int verbose, FILE *mbfp, char *data_ptr, int *error)
{
	char	*function_name = "mbr_sb2100b2_wr_pr";
	int	status = MB_SUCCESS;
	struct mbf_sb2100b2_struct *data;
	short	record_length;
	int	write_length;
	unsigned int	checksum;
	char	*checksum_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to raw data structure */
	data = (struct mbf_sb2100b2_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",data->pr_year);
		fprintf(stderr,"dbg5       julian day:       %d\n",data->pr_jday);
		fprintf(stderr,"dbg5       hour:             %d\n",data->pr_hour);
		fprintf(stderr,"dbg5       minute:           %d\n",data->pr_minute);
		fprintf(stderr,"dbg5       sec:              %d\n",data->pr_sec);
		fprintf(stderr,"dbg5       msec:             %d\n",data->pr_msec);
		fprintf(stderr,"dbg5       roll_bias_port:   %f\n",data->roll_bias_port);
		fprintf(stderr,"dbg5       roll_bias_strbrd: %f\n",data->roll_bias_starboard);
		fprintf(stderr,"dbg5       pitch_bias:       %f\n",data->pitch_bias);
		fprintf(stderr,"dbg5       ship_draft:       %f\n",data->ship_draft);
		fprintf(stderr,"dbg5       offset_x:         %f\n",data->offset_x);
		fprintf(stderr,"dbg5       offset_y:         %f\n",data->offset_y);
		fprintf(stderr,"dbg5       offset_z:         %f\n",data->offset_z);
		fprintf(stderr,"dbg5       num_svp:          %d\n",data->num_svp);
		fprintf(stderr,"dbg5       Sound Velocity Profile:\n");
		for (i=0;i<data->num_svp;i++)
			fprintf(stderr,"dbg5       %d  depth:%f  velocity:%f\n",
				i,data->svp[i].depth,data->svp[i].velocity);
		}

	/* write the record label */
	if (fwrite(mbf_sb2100b2_labels[MBF_SB2100B2_PR],
		MBF_SB2100B2_LABEL_LEN, 1, mbfp) != 1)
		{
		*error = MB_ERROR_WRITE_FAIL;
		status = MB_FAILURE;
		}
	else
		{
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		}

	/* write the record length */
	if (status == MB_SUCCESS)
		{
		record_length = MBF_SB2100B2_PR_WRITE_LEN + 6;
#ifdef BYTESWAPPED
		/* byte swap record length */
		record_length =	    (int) mb_swap_short(record_length);
#endif
		if (fwrite(&record_length, 2, 1, mbfp) != 1)
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

	/* write out the data */
	if (status == MB_SUCCESS)
		{			
#ifdef BYTESWAPPED
		/* byte swap everything */
		data->pr_year =	    (short int) mb_swap_short(data->pr_year);
		data->pr_jday =	    (short int) mb_swap_short(data->pr_jday);
		data->pr_hour =	    (short int) mb_swap_short(data->pr_hour);
		data->pr_minute =   (short int) mb_swap_short(data->pr_minute);
		data->pr_sec =	    (short int) mb_swap_short(data->pr_sec);
		data->pr_msec =	    (short int) mb_swap_short(data->pr_msec);
		mb_swap_float(&(data->roll_bias_port));
		mb_swap_float(&(data->roll_bias_starboard));
		mb_swap_float(&(data->pitch_bias));
		mb_swap_float(&(data->ship_draft));
		mb_swap_float(&(data->offset_x));
		mb_swap_float(&(data->offset_y));
		mb_swap_float(&(data->offset_z));
		data->num_svp =	    (int) mb_swap_int(data->num_svp);
		for (i=0;i<data->num_svp;i++)
			{
			mb_swap_float(&(data->svp[i].depth));
			mb_swap_float(&(data->svp[i].velocity));
			}
#endif

		/* do checksum */
		write_length = MBF_SB2100B2_PR_WRITE_LEN;
		checksum = 0;
		checksum_ptr = (char *) &(data->pr_year);
		for (i=0;i<write_length;i++)
			checksum += (unsigned int) checksum_ptr[i];
#ifdef BYTESWAPPED
		checksum = (unsigned int) mb_swap_int(checksum);
#endif
		
		/* write the data */
		if (fwrite(&(data->pr_year), write_length, 
			1, mbfp) != 1)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		
		/* write the checksum */
		if (fwrite(&checksum, 4, 1, mbfp) != 1)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		
		/* write the eor */
		if (fwrite(mbf_sb2100b2_eor, 2, 1, mbfp) != 1)
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
int mbr_sb2100b2_wr_tr(int verbose, FILE *mbfp, char *data_ptr, int *error)
{
	char	*function_name = "mbr_sb2100b2_wr_tr";
	int	status = MB_SUCCESS;
	struct mbf_sb2100b2_struct *data;
	short	record_length;
	int	write_length;
	unsigned int	checksum;
	char	*checksum_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to raw data structure */
	data = (struct mbf_sb2100b2_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       comment:          %s\n",
			data->comment);
		}

	/* write the record label */
	if (fwrite(mbf_sb2100b2_labels[MBF_SB2100B2_TR],
		MBF_SB2100B2_LABEL_LEN, 1, mbfp) != 1)
		{
		*error = MB_ERROR_WRITE_FAIL;
		status = MB_FAILURE;
		}
	else
		{
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		}

	/* write the record length */
	if (status == MB_SUCCESS)
		{
		record_length = strlen(data->comment) + 1;
		if (record_length >= MBF_SB2100B2_MAXLINE)
			{
			data->comment[MBF_SB2100B2_MAXLINE-1] = '\0';
			record_length = MBF_SB2100B2_MAXLINE;
			}
		record_length += 6;
#ifdef BYTESWAPPED
		/* byte swap record length */
		record_length =	    (int) mb_swap_short(record_length);
#endif
		if (fwrite(&record_length, 2, 1, mbfp) != 1)
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

	/* write out the data */
	if (status == MB_SUCCESS)
		{
		/* do checksum */
		write_length = strlen(data->comment) + 1;
		checksum = 0;
		checksum_ptr = (char *) data->comment;
		for (i=0;i<write_length;i++)
			checksum += (unsigned int) checksum_ptr[i];
#ifdef BYTESWAPPED
		checksum = (unsigned int) mb_swap_int(checksum);
#endif

		/* write the data */
		if (fwrite(&(data->pr_year), write_length, 
			1, mbfp) != 1)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		
		/* write the checksum */
		if (fwrite(&checksum, 4, 1, mbfp) != 1)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		
		/* write the eor */
		if (fwrite(mbf_sb2100b2_eor, 2, 1, mbfp) != 1)
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
int mbr_sb2100b2_wr_dh(int verbose, FILE *mbfp, char *data_ptr, int *error)
{
	char	*function_name = "mbr_sb2100b2_wr_dh";
	int	status = MB_SUCCESS;
	struct mbf_sb2100b2_struct *data;
	short	record_length;
	int	write_length;
	unsigned int	checksum;
	char	*checksum_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to raw data structure */
	data = (struct mbf_sb2100b2_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       julian day:       %d\n",data->jday);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",data->minute);
		fprintf(stderr,"dbg5       sec:              %d\n",data->sec);
		fprintf(stderr,"dbg5       msec:             %d\n",data->msec);
		fprintf(stderr,"dbg5       longitude:        %f\n",data->longitude);
		fprintf(stderr,"dbg5       latitude:         %f\n",data->latitude);
		fprintf(stderr,"dbg5       heading:          %f\n",data->heading);
		fprintf(stderr,"dbg5       speed:            %f\n",data->speed);
		fprintf(stderr,"dbg5       roll:             %f\n",data->roll);
		fprintf(stderr,"dbg5       pitch:            %f\n",data->pitch);
		fprintf(stderr,"dbg5       heave:            %f\n",data->heave);
		fprintf(stderr,"dbg5       ssv:              %f\n",data->ssv);
		fprintf(stderr,"dbg5       frequency:        %c\n",data->frequency);
		fprintf(stderr,"dbg5       depth_gate_mode:  %d\n",data->depth_gate_mode);
		fprintf(stderr,"dbg5       ping_gain:        %d\n",data->ping_gain);
		fprintf(stderr,"dbg5       ping_pulse_width: %d\n",data->ping_pulse_width);
		fprintf(stderr,"dbg5       trans_atten:      %d\n",data->transmitter_attenuation);
		fprintf(stderr,"dbg5       ssv_source:       %c\n",data->ssv_source);
		fprintf(stderr,"dbg5       svp_correction:   %c\n",data->svp_correction);
		fprintf(stderr,"dbg5       pixel_algorithm:  %c\n",data->pixel_algorithm);
		fprintf(stderr,"dbg5       pixel_size:       %f\n",data->pixel_size);
		fprintf(stderr,"dbg5       nbeams:           %d\n",data->nbeams);
		fprintf(stderr,"dbg5       npixels:          %d\n",data->npixels);
		fprintf(stderr,"dbg5       spare1:           %d\n",data->spare1);
		fprintf(stderr,"dbg5       spare2:           %d\n",data->spare2);
		fprintf(stderr,"dbg5       spare3:           %d\n",data->spare3);
		fprintf(stderr,"dbg5       spare4:           %d\n",data->spare4);
		fprintf(stderr,"dbg5       spare5:           %d\n",data->spare5);
		fprintf(stderr,"dbg5       spare6:           %d\n",data->spare6);
		}

	/* write the record label */
	if (fwrite(mbf_sb2100b2_labels[MBF_SB2100B2_DH],
		MBF_SB2100B2_LABEL_LEN, 1, mbfp) != 1)
		{
		*error = MB_ERROR_WRITE_FAIL;
		status = MB_FAILURE;
		}
	else
		{
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		}

	/* write the record length */
	if (status == MB_SUCCESS)
		{
		record_length = MBF_SB2100B2_DH_WRITE_LEN + 6;
#ifdef BYTESWAPPED
		/* byte swap record length */
		record_length =	    (int) mb_swap_short(record_length);
#endif
		if (fwrite(&record_length, 2, 1, mbfp) != 1)
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

	/* write out the data */
	if (status == MB_SUCCESS)
		{			
#ifdef BYTESWAPPED
		/* byte swap everything */
		data->year =	    (short int) mb_swap_short(data->year);
		data->jday =	    (short int) mb_swap_short(data->jday);
		data->hour =	    (short int) mb_swap_short(data->hour);
		data->minute =   (short int) mb_swap_short(data->minute);
		data->sec =	    (short int) mb_swap_short(data->sec);
		data->msec =	    (short int) mb_swap_short(data->msec);
		mb_swap_double(&(data->longitude));
		mb_swap_double(&(data->latitude));
		mb_swap_float(&(data->heading));
		mb_swap_float(&(data->speed));
		mb_swap_float(&(data->roll));
		mb_swap_float(&(data->pitch));
		mb_swap_float(&(data->heave));
		mb_swap_float(&(data->ssv));
		mb_swap_float(&(data->pixel_size));
		data->nbeams =	    (int) mb_swap_int(data->nbeams);
		data->npixels =	    (int) mb_swap_int(data->npixels);
		data->spare1 =	    (int) mb_swap_short(data->spare1);
		data->spare2 =	    (int) mb_swap_short(data->spare2);
		data->spare3 =	    (int) mb_swap_short(data->spare3);
		data->spare4 =	    (int) mb_swap_short(data->spare4);
		data->spare5 =	    (int) mb_swap_short(data->spare5);
		data->spare6 =	    (int) mb_swap_short(data->spare6);
#endif

		/* do checksum */
		write_length = MBF_SB2100B2_DH_WRITE_LEN;
		checksum = 0;
		checksum_ptr = (char *) &(data->year);
		for (i=0;i<write_length;i++)
			checksum += (unsigned int) checksum_ptr[i];
#ifdef BYTESWAPPED
		checksum = (unsigned int) mb_swap_int(checksum);
#endif
		
		/* write the data */
		if (fwrite(&(data->year), write_length, 
			1, mbfp) != 1)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		
		/* write the checksum */
		if (fwrite(&checksum, 4, 1, mbfp) != 1)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		
		/* write the eor */
		if (fwrite(mbf_sb2100b2_eor, 2, 1, mbfp) != 1)
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
int mbr_sb2100b2_wr_br(int verbose, FILE *mbfp, char *data_ptr, int *error)
{
	char	*function_name = "mbr_sb2100b2_wr_br";
	int	status = MB_SUCCESS;
	struct mbf_sb2100b2_struct *data;
	short	record_length;
	int	write_length;
	unsigned int	checksum;
	char	*checksum_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to raw data structure */
	data = (struct mbf_sb2100b2_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Values to be written in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       beam depth xtrack ltrack tt angle angfor amp sig2noise echo src quality\n");
		for (i=0;i<data->nbeams;i++)
		  {
		  fprintf(stderr,"dbg5       %3d %8.2f %9.2f %8.2f %6.3f %7.3f %7.3f %3d %3d %3d %c %c\n",
			i,
			data->beams[i].depth,
			data->beams[i].acrosstrack,
			data->beams[i].alongtrack,
			data->beams[i].range,
			data->beams[i].angle_across,
			data->beams[i].angle_forward,
			data->beams[i].amplitude,
			data->beams[i].signal_to_noise,
			data->beams[i].echo_length,
			data->beams[i].source,
			data->beams[i].quality);
		  }
		}

	/* write the record label */
	if (fwrite(mbf_sb2100b2_labels[MBF_SB2100B2_BR],
		MBF_SB2100B2_LABEL_LEN, 1, mbfp) != 1)
		{
		*error = MB_ERROR_WRITE_FAIL;
		status = MB_FAILURE;
		}
	else
		{
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		}

	/* write the record length */
	if (status == MB_SUCCESS)
		{
		record_length = data->nbeams * MBF_SB2100B2_BR_WRITE_LEN + 6;
#ifdef BYTESWAPPED
		/* byte swap record length */
		record_length =	    (int) mb_swap_short(record_length);
#endif
		if (fwrite(&record_length, 2, 1, mbfp) != 1)
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

	/* write out the data */
	if (status == MB_SUCCESS)
		{		
#ifdef BYTESWAPPED
		/* byte swap everything */
		for (i=0;i<data->nbeams;i++)
			{
			mb_swap_float(&(data->beams[i].depth));
			mb_swap_float(&(data->beams[i].acrosstrack));
			mb_swap_float(&(data->beams[i].alongtrack));
			mb_swap_float(&(data->beams[i].range));
			mb_swap_float(&(data->beams[i].angle_across));
			mb_swap_float(&(data->beams[i].angle_forward));
			data->beams[i].amplitude 
			    = (short int) mb_swap_short(data->beams[i].amplitude);
			data->beams[i].signal_to_noise 
			    = (short int) mb_swap_short(data->beams[i].signal_to_noise);
			data->beams[i].echo_length 
			    = (int) mb_swap_short(data->beams[i].echo_length);
			}
#endif

		/* do checksum */
		write_length = data->nbeams * MBF_SB2100B2_BR_WRITE_LEN;
		checksum = 0;
		checksum_ptr = (char *) data->beams;
		for (i=0;i<write_length;i++)
			checksum += (unsigned int) checksum_ptr[i];
#ifdef BYTESWAPPED
		checksum = (unsigned int) mb_swap_int(checksum);
#endif
		
		/* write the data */
		if (fwrite(data->beams, write_length, 
			1, mbfp) != 1)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		
		/* write the checksum */
		if (fwrite(&checksum, 4, 1, mbfp) != 1)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		
		/* write the eor */
		if (fwrite(mbf_sb2100b2_eor, 2, 1, mbfp) != 1)
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
int mbr_sb2100b2_wr_sr(int verbose, FILE *mbfp, char *data_ptr, int *error)
{
	char	*function_name = "mbr_sb2100b2_wr_sr";
	int	status = MB_SUCCESS;
	struct mbf_sb2100b2_struct *data;
	short	record_length;
	int	write_length;
	unsigned int	checksum;
	char	*checksum_ptr;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbfp:       %d\n",mbfp);
		fprintf(stderr,"dbg2       data_ptr:   %d\n",data_ptr);
		}

	/* get pointer to raw data structure */
	data = (struct mbf_sb2100b2_struct *) data_ptr;

	/* write the record label */
	if (fwrite(mbf_sb2100b2_labels[MBF_SB2100B2_SR],
		MBF_SB2100B2_LABEL_LEN, 1, mbfp) != 1)
		{
		*error = MB_ERROR_WRITE_FAIL;
		status = MB_FAILURE;
		}
	else
		{
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
		}

	/* write the record length */
	if (status == MB_SUCCESS)
		{
		record_length = data->npixels * MBF_SB2100B2_SR_WRITE_LEN + 6;
#ifdef BYTESWAPPED
		/* byte swap record length */
		record_length =	    (int) mb_swap_short(record_length);
#endif
		if (fwrite(&record_length, 2, 1, mbfp) != 1)
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

	/* write out the data */
	if (status == MB_SUCCESS)
		{			

		/* do checksum */
		checksum = 0;
#ifdef BYTESWAPPED
		checksum = (unsigned int) mb_swap_int(checksum);
#endif
		
		/* write the checksum */
		if (fwrite(&checksum, 4, 1, mbfp) != 1)
			{
			*error = MB_ERROR_WRITE_FAIL;
			status = MB_FAILURE;
			}
		else
			{
			*error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
			}
		
		/* write the eor */
		if (fwrite(mbf_sb2100b2_eor, 2, 1, mbfp) != 1)
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
