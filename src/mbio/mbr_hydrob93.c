/*--------------------------------------------------------------------
 *    The MB-system:	mbr_hydrob93.c	9/19/2002
 *	$Id: mbr_hydrob93.c,v 5.2 2003-04-17 21:05:23 caress Exp $
 *
 *    Copyright (c) 2002, 2003 by
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
 * mbr_hydrob93.c contains the functions for reading and writing
 * hydrographic sounding data in the HYD93 Binary Format used by
 * the National Geophysical Data Center.
 * These functions include:
 *   mbr_alm_hydrob93	- allocate read/write memory
 *   mbr_dem_hydrob93	- deallocate read/write memory
 *   mbr_rt_hydrob93	- read and translate data
 *   mbr_wt_hydrob93	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	September 19, 2002
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.1  2002/09/20 17:45:43  caress
 * Release 5.0.beta23
 *
 * Revision 5.0  2002/09/19 22:18:06  caress
 * Initial Revision
 * l.
 * l
 *
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
#include "../../include/mbsys_singlebeam.h"

/* local defines */
#define MBF_HYDROB93_RECORD_LENGTH 14

/* essential function prototypes */
int mbr_register_hydrob93(int verbose, void *mbio_ptr, 
		int *error);
int mbr_info_hydrob93(int verbose, 
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
int mbr_alm_hydrob93(int verbose, void *mbio_ptr, int *error);
int mbr_dem_hydrob93(int verbose, void *mbio_ptr, int *error);
int mbr_rt_hydrob93(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_hydrob93(int verbose, void *mbio_ptr, void *store_ptr, int *error);

static char res_id[]="$Id: mbr_hydrob93.c,v 5.2 2003-04-17 21:05:23 caress Exp $";

/*--------------------------------------------------------------------*/
int mbr_register_hydrob93(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_register_hydrob93";
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
	status = mbr_info_hydrob93(verbose, 
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_hydrob93;
	mb_io_ptr->mb_io_format_free = &mbr_dem_hydrob93; 
	mb_io_ptr->mb_io_store_alloc = &mbsys_singlebeam_alloc; 
	mb_io_ptr->mb_io_store_free = &mbsys_singlebeam_deall; 
	mb_io_ptr->mb_io_read_ping = &mbr_rt_hydrob93; 
	mb_io_ptr->mb_io_write_ping = &mbr_wt_hydrob93; 
	mb_io_ptr->mb_io_extract = &mbsys_singlebeam_extract; 
	mb_io_ptr->mb_io_insert = &mbsys_singlebeam_insert; 
	mb_io_ptr->mb_io_extract_nav = &mbsys_singlebeam_extract_nav; 
	mb_io_ptr->mb_io_insert_nav = &mbsys_singlebeam_insert_nav; 
	mb_io_ptr->mb_io_extract_altitude = &mbsys_singlebeam_extract_altitude; 
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = NULL; 
	mb_io_ptr->mb_io_insert_svp = NULL;
	mb_io_ptr->mb_io_ttimes = &mbsys_singlebeam_ttimes; 
	mb_io_ptr->mb_io_copyrecord = &mbsys_singlebeam_copy; 
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
int mbr_info_hydrob93(int verbose, 
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
	char	*function_name = "mbr_info_hydrob93";
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
	*system = MB_SYS_SINGLEBEAM;
	*beams_bath_max = 1;
	*beams_amp_max = 0;
	*pixels_ss_max = 0;
	strncpy(format_name, "HYDROB93", MB_NAME_LENGTH);
	strncpy(system_name, "SINGLEBEAM", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:         MBF_HYDROB93\nInformal Description: NGDC binary hydrographic sounding format\nAttributes:           XYZ (lon lat depth) binary soundings\n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_NO;
	*traveltime = MB_NO;
	*beam_flagging = MB_YES;
	*nav_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_DATA;
	*beamwidth_xtrack = 5.0;
	*beamwidth_ltrack = 5.0;

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
int mbr_alm_hydrob93(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_alm_hydrob93";
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
	mb_io_ptr->structure_size = 0;
	mb_io_ptr->data_structure_size = 0;
	status = mb_malloc(verbose,sizeof(struct mbsys_singlebeam_struct),
				(char **)&mb_io_ptr->store_data,error);

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	
	/* set number of header records read to zero */
	mb_io_ptr->save1 = 0;

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
int mbr_dem_hydrob93(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_hydrob93";
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
	status = mb_free(verbose,(char **)&mb_io_ptr->store_data,error);

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
int mbr_rt_hydrob93(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_hydrob93";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_singlebeam_struct *store;
	char	line[MBF_HYDROB93_RECORD_LENGTH];
	int	read_len;
	int	ilongitude, ilatitude, idepth;
	short	itype;
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

	/* get pointers to mbio descriptor and data structures */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;
	store = (struct mbsys_singlebeam_struct *) store_ptr;

	/* set file position */
	mb_io_ptr->file_bytes = ftell(mb_io_ptr->mbfp);
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

	/* read next record from file */
	mb_io_ptr->file_pos = mb_io_ptr->file_bytes;
	if ((status = fread(line,1,MBF_HYDROB93_RECORD_LENGTH,
			mb_io_ptr->mbfp)) == MBF_HYDROB93_RECORD_LENGTH) 
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

	/* handle the data */
	if (status == MB_SUCCESS)
	    {
	    /* parse data */
	    mb_get_binary_int(MB_YES, &line[0], (int *) &ilatitude); 
	    mb_get_binary_int(MB_YES, &line[4], (int *) &ilongitude); 
	    mb_get_binary_int(MB_YES, &line[8], (int *) &idepth); 
	    mb_get_binary_short(MB_YES, &line[12], (short *) &itype); 
	    store->longitude = (ilongitude) * 0.000001;
	    store->latitude = (ilatitude) * 0.000001;
	    store->bath = (idepth) * 0.1;
	    if (itype == 711)
	    	store->flag = MB_FLAG_NONE;
	    else if (itype == 10711)
	    	store->flag = MB_FLAG_FLAG + MB_FLAG_MANUAL;
	    else
	    	store->flag = MB_FLAG_NULL;
	    store->time_d = MB_TIME_D_UNKNOWN;
	    mb_get_date(verbose, store->time_d, store->time_i, error);
	    store->heading = 0.0;
	    store->speed = 0.0;
	    store->roll = 0.0;
	    store->pitch = 0.0;
	    store->heave = 0.0;
	    
	    /* set kind */
	    if (itype == 711 || itype == 10711)
	    	store->kind = MB_DATA_DATA;
	    else
	    	store->kind = MB_DATA_RAW_LINE;

	    if (status == MB_SUCCESS)
	        {
		/* print output debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  Data read in MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  Values,read:\n");
			fprintf(stderr,"dbg4       time_i[0]:    %d\n",store->time_i[0]);
			fprintf(stderr,"dbg4       time_i[1]:    %d\n",store->time_i[1]);
			fprintf(stderr,"dbg4       time_i[2]:    %d\n",store->time_i[2]);
			fprintf(stderr,"dbg4       time_i[3]:    %d\n",store->time_i[3]);
			fprintf(stderr,"dbg4       time_i[4]:    %d\n",store->time_i[4]);
			fprintf(stderr,"dbg4       time_i[5]:    %d\n",store->time_i[5]);
			fprintf(stderr,"dbg4       time_i[6]:    %d\n",store->time_i[6]);
			fprintf(stderr,"dbg4       time_d:       %f\n",store->time_d);
			fprintf(stderr,"dbg4       latitude:     %f\n",store->latitude);
			fprintf(stderr,"dbg4       longitude:    %f\n",store->longitude);
			fprintf(stderr,"dbg4       bath:         %f\n",store->bath);
			fprintf(stderr,"dbg4       flag:         %d\n",store->flag);
			fprintf(stderr,"dbg4       heading:      %f\n",store->heading);
			fprintf(stderr,"dbg4       speed:        %f\n",store->speed);
			fprintf(stderr,"dbg4       roll:         %f\n",store->roll);
			fprintf(stderr,"dbg4       pitch:        %f\n",store->pitch);
			fprintf(stderr,"dbg4       heave:        %f\n",store->heave);
			fprintf(stderr,"dbg4       error:        %d\n",*error);
			fprintf(stderr,"dbg4       status:       %d\n",status);
			}
	    	}
	    	
	    else
	    	{	    	
	    	status = MB_FAILURE;
	   	*error = MB_ERROR_UNINTELLIGIBLE;
	    	}
		
	    }

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = store->kind;

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
int mbr_wt_hydrob93(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_hydrob93";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_singlebeam_struct *store;
	char	line[MBF_HYDROB93_RECORD_LENGTH];
	int	ilongitude, ilatitude, idepth;
	short	itype;
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
	store = (struct mbsys_singlebeam_struct *) store_ptr;

	/* handle the data */
	if (store->kind == MB_DATA_DATA)
	    {
	    /* print output debug statements */
	    if (verbose >= 4)
		    {
		    fprintf(stderr,"\ndbg4  Data to be written in MBIO function <%s>\n",
			    function_name);
		    fprintf(stderr,"dbg4  Values,read:\n");
		    fprintf(stderr,"dbg4       latitude:     %f\n",store->latitude);
		    fprintf(stderr,"dbg4       longitude:    %f\n",store->longitude);
		    fprintf(stderr,"dbg4       bath:         %f\n",store->bath);
		    fprintf(stderr,"dbg4       flag:         %d\n",store->flag);
		    fprintf(stderr,"dbg4       error:        %d\n",*error);
		    fprintf(stderr,"dbg4       status:       %d\n",status);
		    }
		    
	    /* put data into buffer */
	    ilongitude = (int)(1000000 * store->longitude);
	    ilatitude = (int)(1000000 * store->longitude);
	    idepth = (int)(10 * store->bath);
	    if (mb_beam_ok(store->flag))
	    	itype = 711;
	    else if (store->flag == MB_FLAG_NULL)
	    	itype = 0;
	    else
	    	itype = 10711;
	    mb_put_binary_int(MB_YES, (int) ilatitude, (void *) &line[0]); 
	    mb_put_binary_int(MB_YES, (int) ilongitude, (void *) &line[4]); 
	    mb_put_binary_int(MB_YES, (int) idepth, (void *) &line[8]); 
	    mb_put_binary_short(MB_YES, (short) itype, (void *) &line[12]); 
	    }

	if ((status = fwrite(line,1,MBF_HYDROB93_RECORD_LENGTH,
		mb_io_ptr->mbfp)) == MBF_HYDROB93_RECORD_LENGTH) 
		{
		status = MB_SUCCESS;
		*error = MB_ERROR_NO_ERROR;
		}
	else
		{
		status = MB_FAILURE;
		*error = MB_ERROR_WRITE_FAIL;
		}

	/* print output debug statements */
	if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg5  Data record kind in MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg5       kind:       %d\n",store->kind);
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
