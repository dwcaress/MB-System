/*--------------------------------------------------------------------
 *    The MB-system:	mbr_em12ifrm.c	12/4/00
 *	$Id: mbr_em12ifrm.c,v 5.1 2001-01-22 07:43:34 caress Exp $
 *
 *    Copyright (c) 2000 by
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
 * mbr_em12ifrm.c contains the functions for reading and writing
 * multibeam data in the EM12IFRM format.  
 * These functions include:
 *   mbr_alm_em12ifrm	- allocate read/write memory
 *   mbr_dem_em12ifrm	- deallocate read/write memory
 *   mbr_rt_em12ifrm	- read and translate data
 *   mbr_wt_em12ifrm	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	December 4, 2000
 * $Log: not supported by cvs2svn $
 * Revision 5.0  2000/12/10  20:24:25  caress
 * Initial revision.
 *
 *
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

/* mbio include files */
#include "../../include/mb_status.h"
#include "../../include/mb_format.h"
#include "../../include/mb_io.h"
#include "../../include/mb_define.h"
#include "../../include/mbsys_simrad.h"
#include "../../include/mbf_em12ifrm.h"

/* include for byte swapping on little-endian machines */
#ifdef BYTESWAPPED
#include "../../include/mb_swap.h"
#endif

/* define IFREMER EM12 Archive format record size */
#define MBF_EM12IFRM_RECORD_SIZE    1032
#define MBF_EM12IFRM_SSHEADER_SIZE   42
#define MBF_EM12IFRM_SSBEAMHEADER_SIZE   6

/* essential function prototypes */
int mbr_info_em12ifrm(int verbose, 
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
			int (**format_alloc)(), 
			int (**format_free)(), 
			int (**store_alloc)(), 
			int (**store_free)(), 
			int (**read_ping)(), 
			int (**write_ping)(), 
			int (**extract)(), 
			int (**insert)(), 
			int (**extract_nav)(), 
			int (**insert_nav)(), 
			int (**extract_altitude)(), 
			int (**insert_altitude)(), 
			int (**extract_svp)(), 
			int (**insert_svp)(), 
			int (**ttimes)(), 
			int (**copyrecord)(), 
			int *error);
int mbr_alm_em12ifrm(int verbose, char *mbio_ptr, int *error);
int mbr_dem_em12ifrm(int verbose, char *mbio_ptr, int *error);
int mbr_rt_em12ifrm(int verbose, char *mbio_ptr, char *store_ptr, int *error);
int mbr_wt_em12ifrm(int verbose, char *mbio_ptr, char *store_ptr, int *error);

/*--------------------------------------------------------------------*/
int mbr_info_em12ifrm(int verbose, 
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
			int (**format_alloc)(), 
			int (**format_free)(), 
			int (**store_alloc)(), 
			int (**store_free)(), 
			int (**read_ping)(), 
			int (**write_ping)(), 
			int (**extract)(), 
			int (**insert)(), 
			int (**extract_nav)(), 
			int (**insert_nav)(), 
			int (**extract_altitude)(), 
			int (**insert_altitude)(), 
			int (**extract_svp)(), 
			int (**insert_svp)(), 
			int (**ttimes)(), 
			int (**copyrecord)(), 
			int *error)
{
	static char res_id[]="$Id: mbr_em12ifrm.c,v 5.1 2001-01-22 07:43:34 caress Exp $";
	char	*function_name = "mbr_info_em12ifrm";
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
	*beams_bath_max = MBF_EM12IFRM_MAXBEAMS;
	*beams_amp_max = MBF_EM12IFRM_MAXBEAMS;
	*pixels_ss_max = 4050;
	strncpy(format_name, "EM12IFRM", MB_NAME_LENGTH);
	strncpy(system_name, "SIMRAD", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_EM12IFRM\nInformal Description: IFREMER TRISMUS format for Simrad EM12\nAttributes:           Simrad EM12S and EM12D,\n                      bathymetry, amplitude, and sidescan\n                      81 beams, variable pixels, binary, IFREMER.\n", MB_DESCRIPTION_LENGTH);
	*numfile = -3;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_NO;
	*traveltime = MB_YES;
	*beam_flagging = MB_YES;
	*nav_source = MB_DATA_NAV;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_DATA;
	*beamwidth_xtrack = 2.0;
	*beamwidth_ltrack = 2.0;

	/* set format and system specific function pointers */
	*format_alloc = &mbr_alm_em12ifrm;
	*format_free = &mbr_dem_em12ifrm; 
	*store_alloc = &mbsys_simrad_alloc; 
	*store_free = &mbsys_simrad_deall; 
	*read_ping = &mbr_rt_em12ifrm; 
	*write_ping = &mbr_wt_em12ifrm; 
	*extract = &mbsys_simrad_extract; 
	*insert = &mbsys_simrad_insert; 
	*extract_nav = &mbsys_simrad_extract_nav; 
	*insert_nav = &mbsys_simrad_insert_nav; 
	*extract_altitude = &mbsys_simrad_extract_altitude; 
	*insert_altitude = NULL; 
	*extract_svp = NULL; 
	*insert_svp = NULL; 
	*ttimes = &mbsys_simrad_ttimes; 
	*copyrecord = &mbsys_simrad_copy; 

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
		fprintf(stderr,"dbg2       format_alloc:       %d\n",*format_alloc);
		fprintf(stderr,"dbg2       format_free:        %d\n",*format_free);
		fprintf(stderr,"dbg2       store_alloc:        %d\n",*store_alloc);
		fprintf(stderr,"dbg2       store_free:         %d\n",*store_free);
		fprintf(stderr,"dbg2       read_ping:          %d\n",*read_ping);
		fprintf(stderr,"dbg2       write_ping:         %d\n",*write_ping);
		fprintf(stderr,"dbg2       extract:            %d\n",*extract);
		fprintf(stderr,"dbg2       insert:             %d\n",*insert);
		fprintf(stderr,"dbg2       extract_nav:        %d\n",*extract_nav);
		fprintf(stderr,"dbg2       insert_nav:         %d\n",*insert_nav);
		fprintf(stderr,"dbg2       extract_altitude:   %d\n",*extract_altitude);
		fprintf(stderr,"dbg2       insert_altitude:    %d\n",*insert_altitude);
		fprintf(stderr,"dbg2       extract_svp:        %d\n",*extract_svp);
		fprintf(stderr,"dbg2       insert_svp:         %d\n",*insert_svp);
		fprintf(stderr,"dbg2       ttimes:             %d\n",*ttimes);
		fprintf(stderr,"dbg2       copyrecord:         %d\n",*copyrecord);
		fprintf(stderr,"dbg2       error:              %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:         %d\n",status);
		}

	/* return status */
	return(status);
}


/*--------------------------------------------------------------------*/
int mbr_alm_em12ifrm(int verbose, char *mbio_ptr, int *error)
{
 static char res_id[]="$Id: mbr_em12ifrm.c,v 5.1 2001-01-22 07:43:34 caress Exp $";
	char	*function_name = "mbr_alm_em12ifrm";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct stat imgfile_status, navfile_status;
	int	imgfile_stat, navfile_stat;
	char	path[MB_NAME_LENGTH];
	char	imgtest[MB_NAME_LENGTH];
	char	navtest[MB_NAME_LENGTH];
	char	*tag_ptr;
	char	*name;

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

	/* allocate memory for data structure */
	mb_io_ptr->structure_size = sizeof(struct mbf_em12ifrm_struct);
	mb_io_ptr->data_structure_size = 0;
	status = mb_malloc(verbose,mb_io_ptr->structure_size,
				&mb_io_ptr->raw_data,error);
	status = mbsys_simrad_alloc(
			verbose,mbio_ptr,
			&mb_io_ptr->store_data,error);

	/* initialize everything to zeros */
	mbr_zero_em12ifrm(verbose,mb_io_ptr->raw_data,error);
	mb_io_ptr->save1 = MB_DATA_NONE;
	mb_io_ptr->save2 = MB_YES;
	mb_io_ptr->save3 = MB_YES;
	mb_io_ptr->save4 = MB_NO;
	
	/* now handle parallel files 
	   - works only if main input is *.SO
	   - nav file is then *.NA
	   - imagery file is then *.IM */
	if ((tag_ptr = strstr(mb_io_ptr->file, ".SO")) != NULL)
		{
		/* get the corresponding nav file name */
		strcpy(imgtest, mb_io_ptr->file);
		tag_ptr = strstr(imgtest, ".SO");
		tag_ptr[0] = '.';
		tag_ptr[1] = 'I';
		tag_ptr[2] = 'M';

		/* get the corresponding imagery file name */
		strcpy(navtest, mb_io_ptr->file);
		tag_ptr = strstr(navtest, ".SO");
		tag_ptr[0] = '.';
		tag_ptr[1] = 'N';
		tag_ptr[2] = 'A';
		
		/* check if nav and imagery files are in same directory */
		imgfile_stat = stat(imgtest, &imgfile_status);
		navfile_stat = stat(navtest, &navfile_status);
		if (imgfile_stat == 0 && navfile_stat == 0
		    && (imgfile_status.st_mode & S_IFMT) != S_IFDIR
		    && (navfile_status.st_mode & S_IFMT) != S_IFDIR)
		    {
		    strcpy(mb_io_ptr->file2, imgtest);
		    strcpy(mb_io_ptr->file3, navtest);
		    }
		/* if nav and imagery files don't exist then check if
		   files in IFREMER TRISMUS archive directories */
		else
		    {
		    /* get path */
		    if ((name = strrchr(mb_io_ptr->file, '/')) != NULL)
			{
			strcpy(path, mb_io_ptr->file);
			path[strlen(path) - strlen(name)] = '\0';
			sprintf(imgtest, "%s/../imag%s", path, name);
			sprintf(navtest, "%s/../nav%s", path, name);
			}
		    else
			{
			sprintf(imgtest, "../imag/%s", mb_io_ptr->file);
			sprintf(navtest, "../nav/%s", mb_io_ptr->file);
			}
		    tag_ptr = strstr(imgtest, ".SO");
		    tag_ptr[0] = '.';
		    tag_ptr[1] = 'I';
		    tag_ptr[2] = 'M';
		    tag_ptr = strstr(navtest, ".SO");
		    tag_ptr[0] = '.';
		    tag_ptr[1] = 'N';
		    tag_ptr[2] = 'A';
		    
		    /* check if files exist */
		    imgfile_stat = stat(imgtest, &imgfile_status);
		    navfile_stat = stat(navtest, &navfile_status);
		    if (imgfile_stat == 0 && navfile_stat == 0
			&& (imgfile_status.st_mode & S_IFMT) != S_IFDIR
			&& (navfile_status.st_mode & S_IFMT) != S_IFDIR)
			{
			strcpy(mb_io_ptr->file2, imgtest);
			strcpy(mb_io_ptr->file3, navtest);
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
int mbr_dem_em12ifrm(int verbose, char *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_em12ifrm";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
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

	/* deallocate memory for data descriptor */
	status = mb_free(verbose,&mb_io_ptr->raw_data,error);
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
int mbr_zero_em12ifrm(int verbose, char *data_ptr, int *error)
{
	char	*function_name = "mbr_zero_em12ifrm";
	int	status = MB_SUCCESS;
	struct mbf_em12ifrm_struct *data;
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
	data = (struct mbf_em12ifrm_struct *) data_ptr;

	/* initialize everything to zeros */
	if (data != NULL)
		{
		data->kind = MB_DATA_NONE;
		data->sonar = MBSYS_SIMRAD_EM12S;

		/* parameter datagram */
		data->par_year = 0;
		data->par_month = 0;
		data->par_day = 0;
		data->par_hour = 0;
		data->par_minute = 0;
		data->par_second = 0;
		data->par_centisecond = 0;
		data->pos_type = 0;	/* positioning system type */
		data->pos_delay = 0.0;	/* positioning system delay (sec) */
		data->roll_offset = 0.0;	/* roll offset (degrees) */
		data->pitch_offset = 0.0;	/* pitch offset (degrees) */
		data->heading_offset = 0.0;	/* heading offset (degrees) */
		data->em100_td = 0.0;	/* EM-100 tranducer depth (meters) */
		data->em100_tx = 0.0;	/* EM-100 tranducer fore-aft 
						offset (meters) */
		data->em100_ty = 0.0;	/* EM-100 tranducer athwartships 
						offset (meters) */
		data->em12_td = 0.0;	/* EM-12 tranducer depth (meters) */
		data->em12_tx = 0.0;	/* EM-12 tranducer fore-aft 
						offset (meters) */
		data->em12_ty = 0.0;	/* EM-12 tranducer athwartships 
						offset (meters) */
		data->em1000_td = 0.0;	/* EM-1000 tranducer depth (meters) */
		data->em1000_tx = 0.0;	/* EM-1000 tranducer fore-aft 
						offset (meters) */
		data->em1000_ty = 0.0;	/* EM-1000 tranducer athwartships 
						offset (meters) */
		for (i=0;i<128;i++)
			data->spare_parameter[i] = '\0';
		data->survey_line = 0;
		for (i=0;i<80;i++)
			data->comment[i] = '\0';

		/* position (position datagrams) */
		data->pos_year = 0;
		data->pos_month = 0;
		data->pos_day = 0;
		data->pos_hour = 0;
		data->pos_minute = 0;
		data->pos_second = 0;
		data->pos_centisecond = 0;
		data->latitude = 0.0;
		data->longitude = 0.0;
		data->utm_northing = 0.0;
		data->utm_easting = 0.0;
		data->utm_zone = 0;
		data->utm_zone_lon = 0.0;
		data->utm_system = 0;
		data->pos_quality = 0;
		data->speed = 0.0;			/* meters/second */
		data->line_heading = 0.0;		/* degrees */

		/* sound velocity profile */
		data->svp_year = 0;
		data->svp_month = 0;
		data->svp_day = 0;
		data->svp_hour = 0;
		data->svp_minute = 0;
		data->svp_second = 0;
		data->svp_centisecond = 0;
		data->svp_num = 0;
		for (i=0;i<100;i++)
			{
			data->svp_depth[i] = 0; /* meters */
			data->svp_vel[i] = 0;	/* 0.1 meters/sec */
			}

		/* time stamp */
		data->year = 0;
		data->month = 0;
		data->day = 0;
		data->hour = 0;
		data->minute = 0;
		data->second = 0;
		data->centisecond = 0;
		data->swath_id = EM_SWATH_CENTER;
		data->ping_number = 0;
		data->beams_bath = MBF_EM12IFRM_MAXBEAMS;
		data->bath_mode = 0;
		data->bath_res = 0;
		data->bath_quality = 0;
		data->keel_depth = 0;
		data->heading = 0;
		data->roll = 0;
		data->pitch = 0;
		data->xducer_pitch = 0;
		data->ping_heave = 0;
		data->sound_vel = 0;
		data->pixels_ss = 0;
		data->ss_mode = 0;
		for (i=0;i<MBF_EM12IFRM_MAXBEAMS;i++)
			{
			data->bath[i] = 0;
			data->bath_acrosstrack[i] = 0;
			data->bath_alongtrack[i] = 0;
			data->tt[i] = 0;
			data->amp[i] = 0;
			data->quality[i] = 0;
			data->heave[i] = 0;
			data->beam_frequency[i] = 0;
			data->beam_samples[i] = 0;
			data->beam_center_sample[i] = 0;
			data->beam_start_sample[i] = 0;
			}
		for (i=0;i<MBF_EM12IFRM_MAXPIXELS;i++)
			{
			data->ss[i] = 0;
			data->ssp[i] = 0;
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
int mbr_rt_em12ifrm(int verbose, char *mbio_ptr, char *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_em12ifrm";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_em12ifrm_struct *data;
	struct mbsys_simrad_struct *store;
	struct mbsys_simrad_survey_struct *ping;
	int	*save_ss;
	double	ss_spacing;
	int	ntime_i[7];
	double	ntime_d;
	int	ptime_i[7];
	double	ptime_d;
	double	plon, plat, pspeed;
	double	dd, dt, dx, dy;
	double	mtodeglon, mtodeglat;
	double	headingx, headingy;
	double	depthscale, dacrscale, daloscale, ttscale, reflscale;
	int	ifix;
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
	data = (struct mbf_em12ifrm_struct *) mb_io_ptr->raw_data;
	store = (struct mbsys_simrad_struct *) store_ptr;	
	save_ss = (int *) &mb_io_ptr->save4;	

	/* read next data from file */
	status = mbr_em12ifrm_rd_data(verbose,mbio_ptr,error);

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = data->kind;

	/* save fix if nav data */
	if (status == MB_SUCCESS
		&& data->kind == MB_DATA_NAV)
		{				
		/* make room for latest fix */
		if (mb_io_ptr->nfix >= MB_NAV_SAVE_MAX)
			{
			for (i=0;i<mb_io_ptr->nfix-1;i++)
				{
				mb_io_ptr->fix_time_d[i]
				    = mb_io_ptr->fix_time_d[i+1];
				mb_io_ptr->fix_lon[i]
				    = mb_io_ptr->fix_lon[i+1];
				mb_io_ptr->fix_lat[i]
				    = mb_io_ptr->fix_lat[i+1];
				}
			mb_io_ptr->nfix--;
			}
			
		/* get nav time */
		mb_fix_y2k(verbose, data->pos_year, 
			    &ntime_i[0]);
		ntime_i[1] = data->pos_month;
		ntime_i[2] = data->pos_day;
		ntime_i[3] = data->pos_hour;
		ntime_i[4] = data->pos_minute;
		ntime_i[5] = data->pos_second;
		ntime_i[6] = 10000 * data->pos_centisecond;
		mb_get_time(verbose, ntime_i, &ntime_d);
		
		/* add latest fix */
		mb_io_ptr->fix_time_d[mb_io_ptr->nfix] 
			= ntime_d;
		mb_io_ptr->fix_lon[mb_io_ptr->nfix] 
			= data->longitude;
		mb_io_ptr->fix_lat[mb_io_ptr->nfix] 
			= data->latitude;
		mb_io_ptr->nfix++;
		}

	/* handle navigation interpolation */
	if (status == MB_SUCCESS
		&& data->kind == MB_DATA_DATA)
		{			
		/* get ping time */
		mb_fix_y2k(verbose, data->year, 
			    &ptime_i[0]);
		ptime_i[1] = data->month;
		ptime_i[2] = data->day;
		ptime_i[3] = data->hour;
		ptime_i[4] = data->minute;
		ptime_i[5] = data->second;
		ptime_i[6] = 10000*data->centisecond;
		mb_get_time(verbose, ptime_i, &ptime_d);

		/* interpolate from saved nav if possible */
		if (mb_io_ptr->nfix > 1)
			{
			/* get speed if necessary */
			if (data->speed <= 0.0)
			    {
                            mb_coor_scale(verbose,
                                mb_io_ptr->fix_lat[mb_io_ptr->nfix-1],
                                &mtodeglon,&mtodeglat);
                            dx = (mb_io_ptr->fix_lon[mb_io_ptr->nfix-1]
                                - mb_io_ptr->fix_lon[0])/mtodeglon;
                            dy = (mb_io_ptr->fix_lat[mb_io_ptr->nfix-1]
                                - mb_io_ptr->fix_lat[0])/mtodeglat;
                            dt = mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1]
                                - mb_io_ptr->fix_time_d[0];
                            pspeed = 3.6 * sqrt(dx*dx + dy*dy)/dt; /* km/hr */
			    data->speed = pspeed / 3.6;
			    }
			else
			    {
			    pspeed = 3.6 * data->speed;
			    }
			if (pspeed > 100.0)
			    pspeed = 0.0;

			/* interpolation possible */
			if (ptime_d >= mb_io_ptr->fix_time_d[0]
			    && ptime_d <= mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1])
			    {
			    ifix = 0;
			    while (ptime_d > mb_io_ptr->fix_time_d[ifix+1])
				ifix++;
			    plon = mb_io_ptr->fix_lon[ifix]
				+ (mb_io_ptr->fix_lon[ifix+1] 
				    - mb_io_ptr->fix_lon[ifix])
				* (ptime_d
				    - mb_io_ptr->fix_time_d[ifix])
				/ (mb_io_ptr->fix_time_d[ifix+1]
				    - mb_io_ptr->fix_time_d[ifix]);
			    plat = mb_io_ptr->fix_lat[ifix]
				+ (mb_io_ptr->fix_lat[ifix+1] 
				    - mb_io_ptr->fix_lat[ifix])
				* (ptime_d
				    - mb_io_ptr->fix_time_d[ifix])
				/ (mb_io_ptr->fix_time_d[ifix+1]
				    - mb_io_ptr->fix_time_d[ifix]);
			    }
			
			/* extrapolate from first fix */
			else if (ptime_d 
				< mb_io_ptr->fix_time_d[0]
				&& pspeed > 0.0)
			    {
			    dd = (ptime_d 
				- mb_io_ptr->fix_time_d[0])
				* pspeed / 3.6;
			    mb_coor_scale(verbose,mb_io_ptr->fix_lat[0],
				&mtodeglon,&mtodeglat);
			    headingx = sin(DTR*(data->line_heading));
			    headingy = cos(DTR*(data->line_heading));
			    plon = mb_io_ptr->fix_lon[0] 
				+ headingx*mtodeglon*dd;
			    plat = mb_io_ptr->fix_lat[0] 
				+ headingy*mtodeglat*dd;
			    }
			
			/* extrapolate from last fix */
			else if (ptime_d 
				> mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1]
				&& pspeed > 0.0)
			    {
			    dd = (ptime_d 
				- mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1])
				* pspeed / 3.6;
			    mb_coor_scale(verbose,mb_io_ptr->fix_lat[mb_io_ptr->nfix-1],
				&mtodeglon,&mtodeglat);
			    headingx = sin(DTR*(data->line_heading));
			    headingy = cos(DTR*(data->line_heading));
			    plon = mb_io_ptr->fix_lon[mb_io_ptr->nfix-1] 
				+ headingx*mtodeglon*dd;
			    plat = mb_io_ptr->fix_lat[mb_io_ptr->nfix-1] 
				+ headingy*mtodeglat*dd;
			    }
			
			/* use last fix */
			else
			    {
			    plon = mb_io_ptr->fix_lon[mb_io_ptr->nfix-1];
			    plat = mb_io_ptr->fix_lat[mb_io_ptr->nfix-1];
			    }
			}
			
		/* else extrapolate from only fix */
		else if (mb_io_ptr->nfix == 1
			&& data->speed > 0.0)
			{
			pspeed = 3.6 * data->speed;
			dd = (ptime_d - mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1])
				* pspeed / 3.6;
			mb_coor_scale(verbose,mb_io_ptr->fix_lat[mb_io_ptr->nfix-1],
				&mtodeglon,&mtodeglat);
			headingx = sin(DTR*(data->line_heading));
			headingy = cos(DTR*(data->line_heading));
			plon = mb_io_ptr->fix_lon[mb_io_ptr->nfix-1] 
				+ headingx*mtodeglon*dd;
			plat = mb_io_ptr->fix_lat[mb_io_ptr->nfix-1] 
				+ headingy*mtodeglat*dd;
			}

		/* else just take last position */
		else if (mb_io_ptr->nfix == 1)
			{
			plon = mb_io_ptr->fix_lon[mb_io_ptr->nfix-1];
			plat = mb_io_ptr->fix_lat[mb_io_ptr->nfix-1];
			pspeed = 0.0;
			}
		else
			{
			plon = 0.0;
			plat = 0.0;
			pspeed = 0.0;
			}
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
			fprintf(stderr,"dbg4       Interpolated Navigation:\n",
				plon);
			fprintf(stderr,"dbg4       longitude:  %f\n",
				plon);
			fprintf(stderr,"dbg4       latitude:   %f\n",
				plat);
			fprintf(stderr,"dbg4       speed:      %f\n",
				pspeed);
			}
		}

	/* translate values to simrad data storage structure */
	if (status == MB_SUCCESS
		&& store != NULL)
		{
		store->kind = data->kind;
		store->sonar = data->sonar;

		/* parameter datagram */
		store->par_year = data->par_year;
		store->par_month = data->par_month;
		store->par_day = data->par_day;
		store->par_hour = data->par_hour;
		store->par_minute = data->par_minute;
		store->par_second = data->par_second;
		store->par_centisecond = data->par_centisecond;
		store->pos_type = data->pos_type;
		store->pos_delay = data->pos_delay;
		store->roll_offset = data->roll_offset;
		store->pitch_offset = data->pitch_offset;
		store->heading_offset = data->heading_offset;
		store->em100_td = data->em100_td;
		store->em100_tx = data->em100_tx;
		store->em100_ty = data->em100_ty;
		store->em12_td = data->em12_td;
		store->em12_tx = data->em12_tx;
		store->em12_ty = data->em12_ty;
		store->em1000_td = data->em1000_td;
		store->em1000_tx = data->em1000_tx;
		store->em1000_ty = data->em1000_ty;
		for (i=0;i<128;i++)
			store->spare_parameter[i] = data->spare_parameter[i];
		store->survey_line = data->survey_line;
		for (i=0;i<80;i++)
			store->comment[i] = data->comment[i];

		/* position (position datagrams) */
		store->pos_year = data->pos_year;
		store->pos_month = data->pos_month;
		store->pos_day = data->pos_day;
		store->pos_hour = data->pos_hour;
		store->pos_minute = data->pos_minute;
		store->pos_second = data->pos_second;
		store->pos_centisecond = data->pos_centisecond;
		store->pos_latitude = data->latitude;
		store->pos_longitude = data->longitude;
		store->utm_northing = data->utm_northing;
		store->utm_easting = data->utm_easting;
		store->utm_zone = data->utm_zone;
		store->utm_zone_lon = data->utm_zone_lon;
		store->utm_system = data->utm_system;
		store->pos_quality = data->pos_quality;
		store->speed = data->speed;
		store->line_heading = data->line_heading;

		/* sound velocity profile */
		store->svp_year = data->svp_year;
		store->svp_month = data->svp_month;
		store->svp_day = data->svp_day;
		store->svp_hour = data->svp_hour;
		store->svp_minute = data->svp_minute;
		store->svp_second = data->svp_second;
		store->svp_centisecond = data->svp_centisecond;
		store->svp_num = data->svp_num;
		for (i=0;i<100;i++)
			{
			store->svp_depth[i] = data->svp_depth[i];
			store->svp_vel[i] = data->svp_vel[i];
			}

		/* time stamp */
		store->year = data->year;
		store->month = data->month;
		store->day = data->day;
		store->hour = data->hour;
		store->minute = data->minute;
		store->second = data->second;
		store->centisecond = data->centisecond;
		
		/* allocate secondary data structure for
			survey data if needed */
		if (data->kind == MB_DATA_DATA
			&& store->ping == NULL)
			{
			status = mbsys_simrad_survey_alloc(
					verbose,mbio_ptr,
					store_ptr,error);
			}
		
		/* deal with putting survey data into
		secondary data structure */
		if (status == MB_SUCCESS 
			&& data->kind == MB_DATA_DATA)
			{
			/* get data structure pointer */
			ping = (struct mbsys_simrad_survey_struct *) 
				store->ping;

			/* copy data */
			ping->longitude = plon;
			ping->latitude = plat;
			ping->swath_id = data->swath_id;
			ping->ping_number = data->ping_number;
			ping->beams_bath = data->beams_bath;
			ping->bath_mode = data->bath_mode;
			ping->bath_res = data->bath_res;
			ping->bath_quality = data->bath_quality;
			ping->keel_depth = data->keel_depth;
			ping->heading = data->heading;
			ping->roll = data->roll;
			ping->pitch = data->pitch;
			ping->xducer_pitch = data->xducer_pitch;
			ping->ping_heave = data->ping_heave;
			ping->sound_vel = data->sound_vel;
			ping->pixels_ss = 0;
			ping->ss_mode = 0;
			for (i=0;i<ping->beams_bath;i++)
				{
				ping->bath[i] = data->bath[i];
				ping->bath_acrosstrack[i] = data->bath_acrosstrack[i];
				ping->bath_alongtrack[i] = data->bath_alongtrack[i];
				ping->tt[i] = data->tt[i];
				ping->amp[i] = data->amp[i];
				ping->quality[i] = data->quality[i];
				ping->heave[i] = data->heave[i];
				ping->beam_frequency[i] = 0;
				ping->beam_samples[i] = 0;
				ping->beam_center_sample[i] = 0;
				ping->beam_start_sample[i] = 0;
				}
			if (*save_ss == MB_NO)
				{
				ping->pixels_ss = data->pixels_ss;
				ping->ss_mode = data->ss_mode;
				for (i=0;i<ping->beams_bath;i++)
					{
					ping->beam_frequency[i] = data->beam_frequency[i];
					ping->beam_samples[i] = data->beam_samples[i];
					ping->beam_center_sample[i] 
						= data->beam_center_sample[i];
					ping->beam_start_sample[i] 
						= data->beam_start_sample[i];
					}
				for (i=0;i<ping->pixels_ss;i++)
					{
					ping->ss[i] = data->ss[i];
					ping->ssp[i] = data->ssp[i];
					}
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
int mbr_wt_em12ifrm(int verbose, char *mbio_ptr, char *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_em12ifrm";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_em12ifrm_struct *data;
	char	*data_ptr;
	struct mbsys_simrad_struct *store;
	struct mbsys_simrad_survey_struct *ping;
	double	scalefactor;
	int	time_j[5];
	double	depthscale, dacrscale, daloscale, ttscale, reflscale;
	int	iss;
	mb_s_char *data_ss;
	mb_s_char *store_ss;
	short int *data_ssp;
	short int *store_ssp;
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
	data = (struct mbf_em12ifrm_struct *) mb_io_ptr->raw_data;
	data_ptr = (char *) data;
	store = (struct mbsys_simrad_struct *) store_ptr;

	/* first translate values from data storage structure */
	if (store != NULL)
		{
		data->kind = store->kind;
		data->sonar = store->sonar;

		/* parameter datagram */
		data->par_year = store->par_year;
		data->par_month = store->par_month;
		data->par_day = store->par_day;
		data->par_hour = store->par_hour;
		data->par_minute = store->par_minute;
		data->par_second = store->par_second;
		data->par_centisecond = store->par_centisecond;
		data->pos_type = store->pos_type;
		data->pos_delay = store->pos_delay;
		data->roll_offset = store->roll_offset;
		data->pitch_offset = store->pitch_offset;
		data->heading_offset = store->heading_offset;
		data->em100_td = store->em100_td;
		data->em100_tx = store->em100_tx;
		data->em100_ty = store->em100_ty;
		data->em12_td = store->em12_td;
		data->em12_tx = store->em12_tx;
		data->em12_ty = store->em12_ty;
		data->em1000_td = store->em1000_td;
		data->em1000_tx = store->em1000_tx;
		data->em1000_ty = store->em1000_ty;
		for (i=0;i<128;i++)
			data->spare_parameter[i] = store->spare_parameter[i];
		data->survey_line = store->survey_line;
		for (i=0;i<80;i++)
			data->comment[i] = store->comment[i];

		/* position (position datagrams) */
		data->pos_year = store->pos_year;
		data->pos_month = store->pos_month;
		data->pos_day = store->pos_day;
		data->pos_hour = store->pos_hour;
		data->pos_minute = store->pos_minute;
		data->pos_second = store->pos_second;
		data->pos_centisecond = store->pos_centisecond;
		data->latitude = store->pos_latitude;
		data->longitude = store->pos_longitude;
		data->utm_northing = store->utm_northing;
		data->utm_easting = store->utm_easting;
		data->utm_zone = store->utm_zone;
		data->utm_zone_lon = store->utm_zone_lon;
		data->utm_system = store->utm_system;
		data->pos_quality = store->pos_quality;
		data->speed = store->speed;
		data->line_heading = store->line_heading;

		/* sound velocity profile */
		data->svp_year = store->svp_year;
		data->svp_month = store->svp_month;
		data->svp_day = store->svp_day;
		data->svp_hour = store->svp_hour;
		data->svp_minute = store->svp_minute;
		data->svp_second = store->svp_second;
		data->svp_centisecond = store->svp_centisecond;
		data->svp_num = store->svp_num;
		for (i=0;i<100;i++)
			{
			data->svp_depth[i] = store->svp_depth[i];
			data->svp_vel[i] = store->svp_vel[i];
			}

		/* time stamp */
		data->year = store->year;
		data->month = store->month;
		data->day = store->day;
		data->hour = store->hour;
		data->minute = store->minute;
		data->second = store->second;
		data->centisecond = store->centisecond;
		
		/* deal with survey data 
			in secondary data structure */
		if (store->ping != NULL)
			{
			/* get data structure pointer */
			ping = (struct mbsys_simrad_survey_struct *) 
				store->ping;

			/* copy survey data */
			data->swath_id = ping->swath_id;
			data->ping_number = ping->ping_number;
			data->beams_bath = ping->beams_bath;
			data->bath_mode = ping->bath_mode;
			data->bath_res = ping->bath_res;
			data->bath_quality = ping->bath_quality;
			data->keel_depth = ping->keel_depth;
			data->heading = ping->heading;
			data->roll = ping->roll;
			data->pitch = ping->pitch;
			data->xducer_pitch = ping->xducer_pitch;
			data->ping_heave = ping->ping_heave;
			data->sound_vel = ping->sound_vel;
			data->pixels_ss = ping->pixels_ss;
			data->ss_mode = ping->ss_mode;
			for (i=0;i<data->beams_bath;i++)
				{
				data->bath[i] = ping->bath[i];
				data->bath_acrosstrack[i] = ping->bath_acrosstrack[i];
				data->bath_alongtrack[i] = ping->bath_alongtrack[i];
				data->tt[i] = ping->tt[i];
				data->amp[i] = ping->amp[i];
				data->quality[i] = ping->quality[i];
				data->heave[i] = ping->heave[i];
				data->beam_frequency[i] = ping->beam_frequency[i];
				data->beam_samples[i] = ping->beam_samples[i];
				data->beam_center_sample[i] 
					= ping->beam_center_sample[i];
				data->beam_start_sample[i] 
					= ping->beam_start_sample[i];
				if (data->beam_samples[i] > 0)
					{
					data_ss = (mb_s_char *) &data->ss[data->beam_start_sample[i]];
					store_ss = (mb_s_char *) &ping->ss[ping->beam_start_sample[i]];
					data_ssp = &data->ssp[data->beam_start_sample[i]];
					store_ssp = &ping->ssp[ping->beam_start_sample[i]];
					for (j=0;j<data->beam_samples[i];j++)
						{
						data_ss[j] = store_ss[j];
						data_ssp[j] = store_ssp[j];
						}
					}
				}
			}
		}
		
	/* set error as this is a read only format */
	status = MB_FAILURE;
	*error = MB_ERROR_WRITE_FAIL;	

	/* write next data to file */
	/*status = mbr_em12ifrm_wr_data(verbose,mbio_ptr,data_ptr,error);*/

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

int mbr_em12ifrm_rd_data(int verbose, char *mbio_ptr, int *error)
{
	char	*function_name = "mbr_em12ifrm_rd_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_em12ifrm_struct *data;
	int	*save_data;
	int	*nav_available;
	int	*ss_available;
	int	*save_ss;
	int	done;
	int	ptime_i[7];
	double	ptime_d;
	int	read_status;
	char	line[MBF_EM12IFRM_RECORD_SIZE];
	int	shift;
	short	short_value;
	int	len;
	int	beamlist[MBF_EM12IFRM_MAXBEAMS];
	mb_s_char *beam_ss;
	int	*ss_swath_id;
	int	*ss_day;
	int	*ss_month;
	int	*ss_year;
	int	*ss_hour;
	int	*ss_minute;
	int	*ss_second;
	int	*ss_centisecond;
	int	*ss_ping_number;
	int	*ss_num_beams;
	int	navdone;
	char	*result;
	char	NorS, EorW;
	int	latdeg, londeg;
	double	latmin, lonmin;
	int	i, j, k;

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
	data = (struct mbf_em12ifrm_struct *) mb_io_ptr->raw_data;
	
	/* set file position */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;
	
	/* check if any data is required */
	done = MB_NO;
	save_data = (int *) &mb_io_ptr->save1;	
	nav_available = (int *) &mb_io_ptr->save2;	
	ss_available = (int *) &mb_io_ptr->save3;	
	save_ss = (int *) &mb_io_ptr->save4;	
	ss_swath_id = (int *) &mb_io_ptr->save5;
	ss_day = (int *) &mb_io_ptr->save6;
	ss_month = (int *) &mb_io_ptr->save7;
	ss_year = (int *) &mb_io_ptr->save8;
	ss_hour = (int *) &mb_io_ptr->save9;
	ss_minute = (int *) &mb_io_ptr->save10;
	ss_second = (int *) &mb_io_ptr->save11;
	ss_centisecond = (int *) &mb_io_ptr->save12;
	ss_ping_number = (int *) &mb_io_ptr->save13;
	ss_num_beams = (int *) &mb_io_ptr->save14;
	if (*save_data == MB_DATA_DATA)
		{
		data->kind = MB_DATA_DATA;
		*save_data = MB_DATA_NONE;
		done = MB_YES;
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
	else if (*save_data == MB_DATA_NAV)
		{
		data->kind = MB_DATA_NAV;
		*save_data = MB_DATA_NONE;		
		done = MB_YES;
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;		
		}

	/* if not done and no data saved then read next primary record */
	if (done == MB_NO && *save_data == MB_DATA_NONE)
		{
		if ((read_status = fread(line,1,MBF_EM12IFRM_RECORD_SIZE,
				mb_io_ptr->mbfp)) == MBF_EM12IFRM_RECORD_SIZE) 
			{
			mb_io_ptr->file_bytes += read_status;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			}
		else
			{
			mb_io_ptr->file_bytes += read_status;
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
	
		/* translate values to em12 data storage structure */
		if (status == MB_SUCCESS)
			{
			/* figure out type of data record */
			if (strncmp(line, "$12SOC", 6) == 0)
				{
				data->kind = MB_DATA_DATA;
				data->sonar = MBSYS_SIMRAD_EM12S;
				}
			else if (strncmp(line, "$12SOB", 6) == 0)
				{
				data->kind = MB_DATA_DATA;
				data->sonar = MBSYS_SIMRAD_EM12D;
				}
			else if (strncmp(line, "$12SOT", 6) == 0)
				{
				data->kind = MB_DATA_DATA;
				data->sonar = MBSYS_SIMRAD_EM12D;
				}
			else if (strncmp(line, "$COMM:", 6) == 0)
				{
				data->kind = MB_DATA_COMMENT;
				data->sonar = MBSYS_SIMRAD_EM12S;
				}
			else
				{
				*error = MB_ERROR_UNINTELLIGIBLE;
				status = MB_FAILURE;
				}
			}
			
		/* deal with data record */
		if (status == MB_SUCCESS
			&& data->kind == MB_DATA_DATA)
			{		
			/* get kind of ping */
			shift = 5;
			if (line[shift] == 'C')
				data->swath_id = EM_SWATH_CENTER;
			else if (line[shift] == 'B')
				data->swath_id = EM_SWATH_PORT;
			else if (line[shift] == 'T')
				data->swath_id = EM_SWATH_STARBOARD;
			shift += 2;
	
			/* get time */
			mb_get_int(&data->day, &line[shift], 2); shift += 3;
			mb_get_int(&data->month, &line[shift], 2); shift += 3;
			mb_get_int(&data->year, &line[shift], 2); shift += 3;
			mb_get_int(&data->hour, &line[shift], 2); shift += 3;
			mb_get_int(&data->minute, &line[shift], 2); shift += 3;
			mb_get_int(&data->second, &line[shift], 2); shift += 3;
			mb_get_int(&data->centisecond, &line[shift], 2); shift += 11;
			
			/* no navigation in this format - imagine that!!!! */
			data->longitude = 0.0;
			data->latitude = 0.0;
	
			/* get binary header */
			mb_get_binary_short(MB_NO, &line[shift], &short_value);
			data->ping_number = (int) short_value; shift += 2;
			data->bath_res = (int) line[shift]; shift += 1;
			data->bath_quality = (int) line[shift]; shift += 1;
			mb_get_binary_short(MB_NO, &line[shift], &short_value);
			data->keel_depth = (int) short_value; shift += 2;
			mb_get_binary_short(MB_NO, &line[shift], &short_value);
			data->heading = (int) short_value; shift += 2;
			mb_get_binary_short(MB_NO, &line[shift], &short_value);
			data->roll = (int) short_value; shift += 2;
			mb_get_binary_short(MB_NO, &line[shift], &short_value);
			data->pitch = (int) short_value; shift += 2;
			data->xducer_pitch = data->pitch;
			mb_get_binary_short(MB_NO, &line[shift], &short_value);
			data->ping_heave = (int) short_value; shift += 2;
			mb_get_binary_short(MB_NO, &line[shift], &short_value);
			data->sound_vel = (int) short_value; shift += 2;
			data->bath_mode = (int) line[shift]; shift += 2;
			
			/* get bathymetry */
			data->beams_bath = MBF_EM12IFRM_MAXBEAMS;
			for (i=0;i<MBF_EM12IFRM_MAXBEAMS;i++)
				{
				mb_get_binary_short(MB_NO, &line[shift], &short_value);
				data->bath[i] = (int) short_value; shift += 2;
				mb_get_binary_short(MB_NO, &line[shift], &short_value);
				data->bath_acrosstrack[i] = (int) short_value; shift += 2;
				mb_get_binary_short(MB_NO, &line[shift], &short_value);
				data->bath_alongtrack[i] = (int) short_value; shift += 2;
				mb_get_binary_short(MB_NO, &line[shift], &short_value);
				data->tt[i] = (int) short_value; shift += 2;
				data->amp[i] = (int) line[shift]; shift += 1;
				data->quality[i] = (int) line[shift]; shift += 1;
				data->heave[i] = (int) line[shift]; shift += 2;
				}
			
			/* use sidescan if saved */
			if (*save_ss == MB_YES
				&& *ss_ping_number == data->ping_number
				&& *ss_swath_id == data->swath_id)
				{
				done = MB_YES;
				}
			
			/* initialize sidescan if none saved */
			else if (*save_ss == MB_NO)
				{
				data->pixels_ss = 0;
				for (i=0;i<MBF_EM12IFRM_MAXBEAMS;i++)
					{
					beamlist[i] = 0;
					data->beam_frequency[i] = 0;
					data->beam_samples[i] = 0;
					data->beam_start_sample[i] = 0;
					data->beam_start_sample[i] = 0;
					}
				for (i=0;i<MBF_EM12IFRM_MAXPIXELS;i++)
					{
					data->ss[i] = 0;
					data->ssp[i] = 0;
					}
				}
			}
	
		/* deal with comment */
		else if (status == MB_SUCCESS
			&& data->kind == MB_DATA_COMMENT)
			{
			shift = 6;
			mb_get_int(&len, &line[shift], 3); shift += 4;
			strncpy(data->comment, &line[shift], 
				MIN(len, MBSYS_SIMRAD_COMMENT_LENGTH-1));
			data->comment[MIN(len+1, MBSYS_SIMRAD_COMMENT_LENGTH)] = '\0';
			done = MB_YES;
			}
		}

	/* if not done and no data saved and good bathy record read
	    then read next sidescan record if available */
	if (status == MB_SUCCESS 
		&& done == MB_NO && *save_data == MB_DATA_NONE
		&& mb_io_ptr->mbfp2 != NULL)
		{
		/* read sidescan until it matches ping number and side */
		*ss_ping_number = 0;
		while (done == MB_NO 
		    && *ss_available == MB_YES
		    && *ss_ping_number <= data->ping_number)
		    {
		    /* read sidescan header from sidescan file */
		    if ((read_status = fread(line,1,MBF_EM12IFRM_SSHEADER_SIZE,
				mb_io_ptr->mbfp2)) == MBF_EM12IFRM_SSHEADER_SIZE)
			{
			mb_io_ptr->file2_bytes += read_status;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			}
		    else
			{
			mb_io_ptr->file2_bytes += read_status;
			status = MB_FAILURE;
			*error = MB_ERROR_EOF;
			}
			
		    /* deal with data record */
		    if (status == MB_SUCCESS)
			{
			/* get kind of ping */
			shift = 5;
			if (line[shift] == 'C')
				*ss_swath_id = EM_SWATH_CENTER;
			else if (line[shift] == 'B')
				*ss_swath_id = EM_SWATH_PORT;
			else if (line[shift] == 'T')
				*ss_swath_id = EM_SWATH_STARBOARD;
			shift += 2;
	
			/* get time */
			mb_get_int(ss_day, &line[shift], 2); shift += 3;
			mb_get_int(ss_month, &line[shift], 2); shift += 3;
			mb_get_int(ss_year, &line[shift], 2); shift += 3;
			mb_get_int(ss_hour, &line[shift], 2); shift += 3;
			mb_get_int(ss_minute, &line[shift], 2); shift += 3;
			mb_get_int(ss_second, &line[shift], 2); shift += 3;
			mb_get_int(ss_centisecond, &line[shift], 2); shift += 11;
			
			/* get binary header */
			mb_get_binary_short(MB_NO, &line[shift], &short_value);
			*ss_ping_number = (int) short_value; shift += 2;
			data->ss_mode = (int) line[shift]; shift += 1;
			shift += 1;
			shift += 1;
			*ss_num_beams = (int) line[shift]; shift += 1;

			/* loop over all beams */
			for (i=0;i<*ss_num_beams;i++)
				{
				if ((read_status = fread(line,1,MBF_EM12IFRM_SSBEAMHEADER_SIZE,
						mb_io_ptr->mbfp2)) != MBF_EM12IFRM_SSBEAMHEADER_SIZE)
					{
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
					}
				shift = 0;
				beamlist[i] = ((int) line[shift]) - 1; shift += 1;
				data->beam_frequency[beamlist[i]] = (int) line[shift]; shift += 1;
				mb_get_binary_short(MB_NO, &line[shift], &short_value);
				data->beam_samples[beamlist[i]] = (int) short_value; shift += 2;
				mb_get_binary_short(MB_NO, &line[shift], &short_value);
				data->beam_center_sample[beamlist[i]] = (int) short_value; shift += 2;
				}
			    
			/* load up the sidescan for each beam */
			for (i=0;i<*ss_num_beams;i++)
				{
				if ((read_status = fread(line,1,data->beam_samples[beamlist[i]],
						mb_io_ptr->mbfp2)) != data->beam_samples[beamlist[i]])
					{
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
					}
				mb_io_ptr->file2_bytes += read_status;
					
				/* do not ever load more data than we can store */
				if (data->pixels_ss + data->beam_samples[beamlist[i]]
					> MBF_EM12IFRM_MAXPIXELS)
					data->beam_samples[beamlist[i]] = 0;
				
				/* get the sidescan */
				data->beam_start_sample[beamlist[i]] = data->pixels_ss;
				shift = 0;
				for (j=0;j<data->beam_samples[beamlist[i]];j++)
					{
					data->ss[data->pixels_ss] = (mb_s_char) line[shift];
					shift++;
					data->pixels_ss++;
					}
				}
			
			/* read last few bytes of record */
			line[0] = 0;
			while (status == MB_SUCCESS && line[0] != '\n')
				{
				if ((read_status = fread(line,1,1,
						mb_io_ptr->mbfp2)) != 1)
					{
					status = MB_FAILURE;
					*error = MB_ERROR_EOF;
					}
				}
			}
			
		    /* now check status */
		    if (status == MB_SUCCESS 
			&& *ss_ping_number == data->ping_number
			&& *ss_swath_id == data->swath_id)
			{
			done = MB_YES;
			*save_ss = MB_NO;
			}
		    else if (status == MB_SUCCESS 
			&& *ss_ping_number > data->ping_number)
			{
			done = MB_YES;
			*save_ss = MB_YES;
			}
		    else if (status == MB_SUCCESS)
			{
			done = MB_NO;
			*save_ss = MB_NO;
			}
		    else if (status == MB_FAILURE)
			{
			done = MB_YES;
			*ss_available = MB_NO;
			status = MB_SUCCESS;
			*error = MB_ERROR_NO_ERROR;
			}
		    }
		}
		
	/* now check if nav needed */
	if (status == MB_SUCCESS 
		&& done == MB_YES && data->kind == MB_DATA_DATA
		&& mb_io_ptr->mbfp3 != NULL)
		{
		/* get ping time */
		mb_fix_y2k(verbose, data->year, 
			    &ptime_i[0]);
		ptime_i[1] = data->month;
		ptime_i[2] = data->day;
		ptime_i[3] = data->hour;
		ptime_i[4] = data->minute;
		ptime_i[5] = data->second;
		ptime_i[6] = 10000*data->centisecond;
		mb_get_time(verbose, ptime_i, &ptime_d);

		/* see if nav is needed and potentially available */
		if (*nav_available == MB_YES
		    && (mb_io_ptr->nfix == 0
			    || mb_io_ptr->fix_time_d[mb_io_ptr->nfix-1]
				< ptime_d))
			{
			navdone = MB_NO;
			while (navdone == MB_NO)
			    {
			    if ((result = fgets(line,MBF_EM12IFRM_RECORD_SIZE,
						mb_io_ptr->mbfp3)) != line)
				{
				navdone = MB_YES;
				*nav_available = MB_NO;
				}
			    else
				{
				mb_io_ptr->file3_bytes += strlen(line);
				shift = 29;
				if (strncmp(&line[shift], "NACOU", 5) == 0)
				    {	
				    /* get time */
				    shift = 7;
				    mb_get_int(&data->pos_day, &line[shift], 2); shift += 3;
				    mb_get_int(&data->pos_month, &line[shift], 2); shift += 3;
				    mb_get_int(&data->pos_year, &line[shift], 2); shift += 3;
				    mb_get_int(&data->pos_hour, &line[shift], 2); shift += 3;
				    mb_get_int(&data->pos_minute, &line[shift], 2); shift += 3;
				    mb_get_int(&data->pos_second, &line[shift], 2); shift += 3;
				    mb_get_int(&data->pos_centisecond, &line[shift], 2); shift += 10;
				    
				    /* get navigation */
				    NorS = line[shift]; shift += 2;
				    mb_get_int(&latdeg, &line[shift], 2); shift += 3;
				    mb_get_double(&latmin, &line[shift], 8); shift += 9;
				    EorW = line[shift]; shift += 2;
				    mb_get_int(&londeg, &line[shift], 3); shift += 4;
				    mb_get_double(&lonmin, &line[shift], 8); shift += 42;
				    mb_get_double(&data->line_heading, &line[shift], 6); shift += 6;
				    data->latitude = latdeg + latmin / 60.0;
				    if (NorS == 'S') 
					data->latitude = -data->latitude;
				    data->longitude = londeg + lonmin / 60.0;
				    if (EorW == 'W') 
					data->longitude = -data->longitude;
				    data->speed = 0.0;
/*fprintf(stderr, "time: %2.2d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%2.2d lat: %c %2d %8.5f  lon: %c %3d %9.5f\n",
data->pos_day, data->pos_month, data->pos_year, 
data->pos_hour, data->pos_minute, data->pos_second, data->pos_centisecond, 
NorS, latdeg, latmin, EorW, londeg, lonmin);*/
					
				    navdone = MB_YES;
				    data->kind = MB_DATA_NAV;
				    *save_data = MB_DATA_DATA;
				    done = MB_YES;
				    status = MB_SUCCESS;
				    *error = MB_ERROR_NO_ERROR;
				    }
				}
			    }
			}
		}	

	/* print debug statements */
	if (verbose >= 5
		&& data->kind == MB_DATA_DATA)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       month:            %d\n",data->month);
		fprintf(stderr,"dbg5       day:              %d\n",data->day);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",data->minute);
		fprintf(stderr,"dbg5       sec:              %d\n",data->second);
		fprintf(stderr,"dbg5       centisecond:      %d\n",data->centisecond);
		fprintf(stderr,"dbg5       ping_number:      %d\n",data->ping_number);
		fprintf(stderr,"dbg5       beams_bath:       %d\n",data->beams_bath);
		fprintf(stderr,"dbg5       bath_res:         %d\n",data->bath_res);
		fprintf(stderr,"dbg5       bath_quality:     %d\n",data->bath_quality);
		fprintf(stderr,"dbg5       keel_depth:       %d\n",data->keel_depth);
		fprintf(stderr,"dbg5       heading:          %d\n",data->heading);
		fprintf(stderr,"dbg5       roll:             %d\n",data->roll);
		fprintf(stderr,"dbg5       pitch:            %d\n",data->pitch);
		fprintf(stderr,"dbg5       ping_heave:       %d\n",data->ping_heave);
		fprintf(stderr,"dbg5       sound_vel:        %d\n",data->sound_vel);
		fprintf(stderr,"dbg5       bath_mode:        %d\n",data->bath_mode);
		fprintf(stderr,"dbg5       beam bath xtrack ltrack tt amp qual heave\n");
		for (i=0;i<data->beams_bath;i++)
			fprintf(stderr,"dbg5       beam:%d  bath:%d  xtrck:%d  ltrck:%d tt:%d  amp:%d  qual:%d  heave:%d\n",
				i,data->bath[i],data->bath_acrosstrack[i],
				data->bath_alongtrack[i],data->tt[i],
				data->amp[i],data->quality[i],data->heave[i]);
		fprintf(stderr,"dbg5       year:             %d\n",*ss_year);
		fprintf(stderr,"dbg5       month:            %d\n",*ss_month);
		fprintf(stderr,"dbg5       day:              %d\n",*ss_day);
		fprintf(stderr,"dbg5       hour:             %d\n",*ss_hour);
		fprintf(stderr,"dbg5       minute:           %d\n",*ss_minute);
		fprintf(stderr,"dbg5       sec:              %d\n",*ss_second);
		fprintf(stderr,"dbg5       centisecond:      %d\n",*ss_centisecond);
		fprintf(stderr,"dbg5       ping_number:      %d\n",*ss_ping_number);
		fprintf(stderr,"dbg5       ss_mode:          %d\n",data->ss_mode);
		fprintf(stderr,"dbg5       ss_num_beams:     %d\n",*ss_num_beams);
		fprintf(stderr,"dbg5       beam frequency samples center\n");
		for (i=0;i<*ss_num_beams;i++)
			fprintf(stderr,"dbg5       beam:%d  frequency:%d  samples:%d  center:%d  start:%d\n",
				beamlist[i],data->beam_frequency[beamlist[i]],
				data->beam_samples[beamlist[i]],
				data->beam_center_sample[beamlist[i]],
				data->beam_start_sample[beamlist[i]]);
		k = 0;
		for (i=0;i<*ss_num_beams;i++)
			{
			beam_ss = &data->ss[data->beam_start_sample[beamlist[i]]];
			for (j=0;j<data->beam_samples[beamlist[i]];j++)
				{
				fprintf(stderr,"dbg5       beam:%d pixel:%d  amp:%d\n",
					beamlist[i],k,beam_ss[j]);
				k++;
				}
			}
		}

	/* print debug statements */
	if (verbose >= 5
		&& data->kind == MB_DATA_COMMENT)
		{
		fprintf(stderr,"\ndbg5  Values read in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       comment:          %s\n",data->comment);
		}

	/* set kind and error in mb_io_ptr */
	mb_io_ptr->new_kind = data->kind;
	mb_io_ptr->new_error = *error;

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
int mbr_em12ifrm_wr_data(int verbose, char *mbio_ptr, char *data_ptr, int *error)
{
	char	*function_name = "mbr_em12ifrm_wr_data";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_em12ifrm_struct *data;
	char	line[MBF_EM12IFRM_RECORD_SIZE];
	int	write_status;
	int	shift;
	short	short_value;
	char	char_value;
	int	len;
	int	i;

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
	data = (struct mbf_em12ifrm_struct *) data_ptr;

	/* print debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Ready to write data in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       kind:       %d\n", mb_io_ptr->new_kind);
		fprintf(stderr,"dbg5       error:      %d\n",*error);
		fprintf(stderr,"dbg5       status:     %d\n",status);
		}

	/* print debug statements */
	if (verbose >= 5
		&& data->kind == MB_DATA_DATA)
		{
		fprintf(stderr,"\ndbg5  Values to write in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       year:             %d\n",data->year);
		fprintf(stderr,"dbg5       month:            %d\n",data->month);
		fprintf(stderr,"dbg5       day:              %d\n",data->day);
		fprintf(stderr,"dbg5       hour:             %d\n",data->hour);
		fprintf(stderr,"dbg5       minute:           %d\n",data->minute);
		fprintf(stderr,"dbg5       sec:              %d\n",data->second);
		fprintf(stderr,"dbg5       centisecond:      %d\n",data->centisecond);
		fprintf(stderr,"dbg5       ping_number:      %d\n",data->ping_number);
		fprintf(stderr,"dbg5       beams_bath:       %d\n",data->beams_bath);
		fprintf(stderr,"dbg5       bath_mode:        %d\n",data->bath_mode);
		fprintf(stderr,"dbg5       bath_res:         %d\n",data->bath_res);
		fprintf(stderr,"dbg5       bath_quality:     %d\n",data->ping_number);
		fprintf(stderr,"dbg5       keel_depth:       %d\n",data->keel_depth);
		fprintf(stderr,"dbg5       heading:          %d\n",data->heading);
		fprintf(stderr,"dbg5       roll:             %d\n",data->roll);
		fprintf(stderr,"dbg5       pitch:            %d\n",data->pitch);
		fprintf(stderr,"dbg5       ping_heave:       %d\n",data->ping_heave);
		fprintf(stderr,"dbg5       sound_vel:        %d\n",data->sound_vel);
		fprintf(stderr,"dbg5       beam bath xtrack ltrack tt amp qual heave\n");
		for (i=0;i<data->beams_bath;i++)
			fprintf(stderr,"dbg5       beam:%d  bath:%d  xtrck:%d  ltrck:%d tt:%d  amp:%d  qual:%d  heave:%d\n",
				i,data->bath[i],data->bath_acrosstrack[i],
				data->bath_alongtrack[i],data->tt[i],
				data->amp[i],data->quality[i],data->heave[i]);
		}
		
	/* handle survey ping record */
	if (data->kind == MB_DATA_DATA)
		{
		/* set ascii header */
		shift = 0;
		if (data->swath_id == EM_SWATH_CENTER)
			char_value = 'C';
		else if (data->swath_id == EM_SWATH_PORT)
			char_value = 'B';
		else if (data->swath_id == EM_SWATH_STARBOARD)
			char_value = 'T';
		sprintf(&line[shift], "$12SO%c,%2.2d/%2.2d/%2.2d,%2.2d:%2.2d:%2.2d.%2.2d0,VOIE%c,", 
				char_value, data->day, data->month, data->year, 
				data->hour, data->minute, data->second, 
				data->centisecond, char_value); shift += 35;

		/* set binary header */
		line[shift] = '\0'; shift += 1;
		short_value = (short) data->ping_number;
		mb_put_binary_short(MB_NO, short_value, &line[shift]); shift += 2;
		line[shift] = (char) data->bath_res; shift += 1;
		line[shift] = (char) data->bath_quality; shift += 1;
		short_value = (short) data->keel_depth;
		mb_put_binary_short(MB_NO, short_value, &line[shift]); shift += 2;
		short_value = (short) data->heading;
		mb_put_binary_short(MB_NO, short_value, &line[shift]); shift += 2;
		short_value = (short) data->roll;
		mb_put_binary_short(MB_NO, short_value, &line[shift]); shift += 2;
		short_value = (short) data->pitch;
		mb_put_binary_short(MB_NO, short_value, &line[shift]); shift += 2;
		short_value = (short) data->ping_heave;
		mb_put_binary_short(MB_NO, short_value, &line[shift]); shift += 2;
		short_value = (short) data->sound_vel;
		mb_put_binary_short(MB_NO, short_value, &line[shift]); shift += 2;
		line[shift] = (char) data->bath_mode; shift += 1;
		line[shift] = ','; shift += 1;
		
		/* set bathymetry */
		data->beams_bath = MBF_EM12IFRM_MAXBEAMS;
		for (i=0;i<data->beams_bath;i++)
			{
			short_value = (short) data->bath[i];
			mb_put_binary_short(MB_NO, short_value, &line[shift]); shift += 2;
			short_value = (short) data->bath_acrosstrack[i];
			mb_put_binary_short(MB_NO, short_value, &line[shift]); shift += 2;
			short_value = (short) data->bath_alongtrack[i];
			mb_put_binary_short(MB_NO, short_value, &line[shift]); shift += 2;
			short_value = (short) data->tt[i];
			mb_put_binary_short(MB_NO, short_value, &line[shift]); shift += 2;
			line[shift] = (char) data->amp[i]; shift += 1;
			line[shift] = (char) data->quality[i]; shift += 1;
			line[shift] = (char) data->heave[i]; shift += 1;
			line[shift] = (char) 0; shift += 1;
			}
		line[shift] = (char) 0; shift += 1;
		line[shift] = (char) 0; shift += 1;
		line[shift] = (char) 0; shift += 1;
		line[shift] = ','; shift += 1;
		line[shift] = '\r'; shift += 1;
		line[shift] = '\n'; shift += 1;
		}
		
	/* handle comment record */
	else if (data->kind == MB_DATA_COMMENT)
		{
		shift = 0;
		len = MIN(strlen(data->comment), MBSYS_SIMRAD_COMMENT_LENGTH - 1);
		sprintf(&line[shift], "$COMM:%3.3d:", len);
		shift += 10;
		strncpy(&line[shift], data->comment, len);
		shift += len;
		for (i=shift;i<MBF_EM12IFRM_RECORD_SIZE-6;i++)
		    line[i] = '\0';
		shift = MBF_EM12IFRM_RECORD_SIZE - 6;
		line[shift] = (char) 0; shift += 1;
		line[shift] = (char) 0; shift += 1;
		line[shift] = (char) 0; shift += 1;
		line[shift] = ','; shift += 1;
		line[shift] = '\r'; shift += 1;
		line[shift] = '\n'; shift += 1;		
		}

	/* write next record to file */
	if (status == MB_SUCCESS)
		{
		if ((status = fwrite(line,1,MBF_EM12IFRM_RECORD_SIZE,
				mb_io_ptr->mbfp)) 
				== MBF_EM12IFRM_RECORD_SIZE) 
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
