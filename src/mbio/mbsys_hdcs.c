/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_hdcs.c	3/1/99
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
 * mbsys_hdcs.c contains the functions for handling the data structure
 * used by MBIO functions to store swath sonar data stored 
 * in UNB OMG HDCS formats:
 *      MBF_OMGHDCSJ : MBIO ID 151
 *
 * Author:	D. W. Caress
 * Date:	March 16, 1999
 *
 * $Log: mbsys_hdcs.c,v $
 * Revision 5.13  2008/09/13 06:08:09  caress
 * Updates to apply suggested patches to segy handling. Also fixes to remove compiler warnings.
 *
 * Revision 5.12  2008/07/10 18:02:39  caress
 * Proceeding towards 5.1.1beta20.
 *
 * Revision 5.9  2008/05/16 22:56:24  caress
 * Release 5.1.1beta18.
 *
 * Revision 5.8  2008/03/14 18:33:03  caress
 * Updated support for JHC format 151.
 *
 * Revision 5.7  2005/11/05 00:48:03  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.6  2004/04/27 01:46:14  caress
 * Various updates of April 26, 2004.
 *
 * Revision 5.5  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.4  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.3  2001/08/25 00:54:13  caress
 * Adding beamwidth values to extract functions.
 *
 * Revision 5.2  2001/07/20  00:32:54  caress
 * Release 5.0.beta03
 *
 * Revision 5.1  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.3  2000/10/11  01:03:21  caress
 * Convert to ANSI C
 *
 * Revision 4.2  2000/09/30  06:32:52  caress
 * Snapshot for Dale.
 *
 * Revision 4.1  1999/08/08  04:16:03  caress
 * Added ELMK2XSE format.
 *
 * Revision 4.0  1999/03/31  18:29:20  caress
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
#include "../../include/mbsys_hdcs.h"

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/
int mbsys_hdcs_alloc(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error)
{
	char	*function_name = "mbsys_hdcs_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hdcs_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %ld\n",(long)mbio_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* allocate memory for data structure */
	status = mb_mallocd(verbose,__FILE__, __LINE__,sizeof(struct mbsys_hdcs_struct),
				(void **)store_ptr,error);

	if (status == MB_SUCCESS)
	    {
	    /* get pointer to data structure */
	    store = (struct mbsys_hdcs_struct *) *store_ptr;
    
	    /* initialize values in structure */
	    store->kind = MB_DATA_NONE;
	    store->sensorNumber = 1;
	    store->subFileID = 1;
	    store->fileVersion = 0;
	    store->toolType = MBSYS_HDCS_None;
	    store->numProfiles = 0;
	    store->numDepths_sum = 0;
	    store->timeScale = 0;
	    store->refTime = 0;
	    store->minTime = 0;
	    store->maxTime = 0;
	    store->positionType = 0;
	    store->positionScale = 0;
	    store->refLat = 0;
	    store->minLat = 0;
	    store->maxLat = 0;
	    store->refLong = 0;
	    store->minLong = 0;
	    store->maxLong = 0;
	    store->minObsDepth = 0;
	    store->maxObsDepth = 0;
	    store->minProcDepth = 0;
	    store->maxProcDepth = 0;
	    store->status_sum = 0;
	    store->status_pro = 0;		/* status is either OK (0) 
						or no nav (1)
						or unwanted for gridding (2) */
	    store->numDepths_pro = 0;		/* Number of depths in profile        */
	    store->numSamples = 0;	/* Number of sidescan samples in parallel file        */
	    store->timeOffset = 0;	/* Time offset  wrt. header           */
	    store->vesselLatOffset = 0;	/* Latitude offset wrt. header        */
	    store->vesselLongOffset = 0;	/* Longitude offset wrt. header       */
	    store->vesselHeading = 0;	/* Heading (100 nRadians)             */
	    store->vesselHeave = 0;	/* Heave (mm)                         */
	    store->vesselPitch = 0;	/* Vessel pitch (100 nRadians)        */
	    store->vesselRoll = 0;	/* Vessel roll (100 nRadians)         */
	    store->tide = 0;		/* Tide (mm)                          */
	    store->vesselVelocity = 0;	/* Vessel Velocity (mm/s)
						note - transducer pitch is 
						generally tucked into the vel field     */
	    store->power = 0;
	    store->TVG = 0;
	    store->attenuation = 0;
	    store->edflag = 0;
	    store->soundVelocity = 0; 	/* mm/s */
	    store->lengthImageDataField = 0;
	    store->pingNo = 0;
	    store->mode = 0;  
	    store->Q_factor = 0;  
	    store->pulseLength = 0;   /* centisecs*/
	    store->unassigned = 0;  
	    store->td_sound_speed = 0;
	    store->samp_rate = 0;
	    store->z_res_cm = 0;
	    store->xy_res_cm = 0;
	    store->ssp_source = 0;
	    store->filter_ID = 0;
	    store->absorp_coeff = 0;
	    store->tx_pulse_len = 0;
	    store->tx_beam_width = 0;
	    store->max_swath_width = 0;
	    store->tx_power_reduction = 0;
	    store->rx_beam_width = 0;
	    store->rx_bandwidth = 0;
	    store->rx_gain_reduction = 0;
	    store->tvg_crossover = 0;
	    store->beam_spacing = 0;
	    store->coverage_sector = 0;
	    store->yaw_stab_mode = 0;
	    store->beams = NULL;
	    store->ss_raw = NULL;
	    store->pixel_size = 0;
	    store->pixels_ss = 0;
	    for (i=0;i<MBSYS_HDCS_MAX_PIXELS;i++)
		{
		store->ss_proc[i] = 0;
		store->ssalongtrack[i] = 0;
		}
	    store->comment[0] = 0;
	    }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       store_ptr:  %ld\n",(long)*store_ptr);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_hdcs_deall(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error)
{
	char	*function_name = "mbsys_hdcs_deall";
	int	status = MB_SUCCESS;
	struct mbsys_hdcs_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %ld\n",(long)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %ld\n",(long)*store_ptr);
		}

	/* get pointer to data structure */
	store = (struct mbsys_hdcs_struct *) *store_ptr;

	/* deallocate memory for data structures */
	if (store->beams != NULL)
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&(store->beams),error);
	if (store->ss_raw != NULL)
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&(store->ss_raw),error);

	/* deallocate memory for data structure */
	status = mb_freed(verbose,__FILE__, __LINE__, (void **)store_ptr,error);

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_hdcs_dimensions(int verbose, void *mbio_ptr, void *store_ptr, 
		int *kind, int *nbath, int *namp, int *nss, int *error)
{
	char	*function_name = "mbsys_hdcs_dimensions";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hdcs_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %ld\n",(long)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %ld\n",(long)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_hdcs_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get beam and pixel numbers */
		*nbath = store->numDepths_pro;
		*namp = store->numDepths_pro;
		*nss = store->pixels_ss;
		}
	else
		{
		/* get beam and pixel numbers */
		*nbath = 0;
		*namp = 0;
		*nss = 0;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:       %d\n",*kind);
		fprintf(stderr,"dbg2       nbath:      %d\n",*nbath);
		fprintf(stderr,"dbg2        namp:      %d\n",*namp);
		fprintf(stderr,"dbg2        nss:       %d\n",*nss);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_hdcs_extract(int verbose, void *mbio_ptr, void *store_ptr, 
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_hdcs_extract";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hdcs_struct *store;
	struct mbsys_hdcs_beam_struct *beam;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
		    function_name);
	    fprintf(stderr,"dbg2  Input arguments:\n");
	    fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
	    fprintf(stderr,"dbg2       mb_ptr:     %ld\n",(long)mbio_ptr);
	    fprintf(stderr,"dbg2       store_ptr:  %ld\n",(long)store_ptr);
	    }

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_hdcs_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
	    {
	    /* get time */
	    *time_d = 100.0 * store->refTime
					+ 1.0e-6 
					    * ((double)store->timeScale) 
					    * ((double)store->timeOffset);
	    mb_get_date(verbose,*time_d,time_i);

	    /* get navigation */
	    if (store->positionType == 1)
		{
		*navlon = 
			RTD * (1.0E-7 * (double)store->refLong
				+ 1.0E-9 * (double)store->vesselLongOffset
				    * (double)store->positionScale);
		*navlat = 
			RTD * (1.0E-7 * (double)store->refLat 
				+ 1.0E-9 * (double)store->vesselLatOffset
				    * (double)store->positionScale);
		}
	    else
		{
		*navlon = 0.0;
		*navlat = 0.0;
		}

	    /* get heading */
	    *heading = RTD * (double)store->vesselHeading
					* 1.0E-07 + M_PI / 2.0;
	    /* get speed */
	    *speed = 3.6e-3 * store->vesselVelocity;

	    /* print debug statements */
	    if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg4  Extracting data by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg4  Data type:\n");
		fprintf(stderr,"dbg4       kind:       %d\n",*kind);
		fprintf(stderr,"dbg4       toolType:   %d\n",store->toolType);
		fprintf(stderr,"dbg4       toolType:   %s\n",mbsys_hdcs_tool_names[store->toolType]);
		}
	
	    /* set beamwidths in mb_io structure */
	    if (store->rx_beam_width > 0 && store->rx_beam_width < 255 
	    	&& store->tx_beam_width > 0 && store->tx_beam_width < 65535)
		{
		mb_io_ptr->beamwidth_ltrack = 0.1 * store->tx_beam_width;
		mb_io_ptr->beamwidth_xtrack = 0.1 * store->rx_beam_width;
		}
	    else if (store->toolType == MBSYS_HDCS_SingleBeam)
		{
		mb_io_ptr->beamwidth_ltrack = 4.0;
		mb_io_ptr->beamwidth_xtrack = 4.0;
		}
	    else if (store->toolType == MBSYS_HDCS_ELAC_BottomChart)
		{
		mb_io_ptr->beamwidth_ltrack = 6.0;
		mb_io_ptr->beamwidth_xtrack = 3.0;
		}
	    else if (store->toolType == MBSYS_HDCS_SeaBeam_1180_MkII)
		{
		mb_io_ptr->beamwidth_ltrack = 1.5;
		mb_io_ptr->beamwidth_xtrack = 2.8;
		}
	    else if (store->toolType == MBSYS_HDCS_EM12_single
		|| store->toolType == MBSYS_HDCS_EM12_dual)
		{
		mb_io_ptr->beamwidth_ltrack = 1.7;
		mb_io_ptr->beamwidth_xtrack = 3.5;
		}
	    else if (store->toolType == MBSYS_HDCS_EM100_depth
		|| store->toolType == MBSYS_HDCS_EM100_depth_ss
		|| store->toolType == MBSYS_HDCS_EM1000
		|| store->toolType == MBSYS_HDCS_EM1000_ampl)
		{
		mb_io_ptr->beamwidth_ltrack = 3.3;
		mb_io_ptr->beamwidth_xtrack = 3.3;
		}
	    else if (store->toolType == MBSYS_HDCS_FanSweep
		|| store->toolType == MBSYS_HDCS_FanSweep_10A
		|| store->toolType == MBSYS_HDCS_FanSweep_20)
		{
		mb_io_ptr->beamwidth_ltrack = 2.0;
		mb_io_ptr->beamwidth_xtrack = 2.0;
		}
	    else if (store->toolType == MBSYS_HDCS_SeaBeam)
		{
		mb_io_ptr->beamwidth_ltrack = 2.67;
		mb_io_ptr->beamwidth_xtrack = 2.67;
		}
	    else if (store->toolType == MBSYS_HDCS_EM3000
		|| store->toolType == MBSYS_HDCS_EM3000D)
		{
		mb_io_ptr->beamwidth_ltrack = 1.5;
		mb_io_ptr->beamwidth_xtrack = 1.5;
		}
	    else if (store->toolType == MBSYS_HDCS_ROSS_Profiler)
		{
		mb_io_ptr->beamwidth_ltrack = 4.0;
		mb_io_ptr->beamwidth_xtrack = 4.0;
		}
	    else if (store->toolType == MBSYS_HDCS_SB2100
		|| store->toolType == MBSYS_HDCS_SB2K)
		{
		mb_io_ptr->beamwidth_ltrack = 2.0;
		mb_io_ptr->beamwidth_xtrack = 2.0;
		}
	    else if (store->toolType == MBSYS_HDCS_ISIS_Submetrix
		|| store->toolType == MBSYS_HDCS_ISIS_SWA)
		{
		mb_io_ptr->beamwidth_ltrack = 2.0;
		mb_io_ptr->beamwidth_xtrack = 2.0;
		}
	    else if (store->toolType == MBSYS_HDCS_Seabat9001)
		{
		mb_io_ptr->beamwidth_ltrack = 1.5;
		mb_io_ptr->beamwidth_xtrack = 1.5;
		}
	    else if (store->toolType == MBSYS_HDCS_SeaBat_8101)
		{
		mb_io_ptr->beamwidth_ltrack = 1.5;
		mb_io_ptr->beamwidth_xtrack = 1.5;
		}
	    else if (store->toolType == MBSYS_HDCS_EM300)
		{
		mb_io_ptr->beamwidth_ltrack = 2.0;
		mb_io_ptr->beamwidth_xtrack = 2.0;
		}
	    else if (store->toolType == MBSYS_HDCS_EM121A)
		{
		mb_io_ptr->beamwidth_ltrack = 1.0;
		mb_io_ptr->beamwidth_xtrack = 1.0;
		}
	    else if (store->toolType == MBSYS_HDCS_SM2000)
		{
		mb_io_ptr->beamwidth_ltrack = 2.0;
		mb_io_ptr->beamwidth_xtrack = 2.0;
		}
	    else if (store->toolType == MBSYS_HDCS_HydroSweep_MD2)
		{
		mb_io_ptr->beamwidth_ltrack = 2.0;
		mb_io_ptr->beamwidth_xtrack = 2.0;
		}
	    else if (store->toolType == MBSYS_HDCS_EM1002)
		{
		mb_io_ptr->beamwidth_ltrack = 2.0;
		mb_io_ptr->beamwidth_xtrack = 2.0;
		}
	    else if (store->toolType == MBSYS_HDCS_Humminbird)
		{
		mb_io_ptr->beamwidth_ltrack = 2.0;
		mb_io_ptr->beamwidth_xtrack = 2.0;
		}
	    else if (store->toolType == MBSYS_HDCS_Knudsen_320)
		{
		mb_io_ptr->beamwidth_ltrack = 2.0;
		mb_io_ptr->beamwidth_xtrack = 2.0;
		}
	    else if (store->toolType == MBSYS_HDCS_EM120)
		{
		mb_io_ptr->beamwidth_ltrack = 2.0;
		mb_io_ptr->beamwidth_xtrack = 2.0;
		}
	    else if (store->toolType == MBSYS_HDCS_SeaBat_8125)
		{
		mb_io_ptr->beamwidth_ltrack = 2.0;
		mb_io_ptr->beamwidth_xtrack = 2.0;
		}
	    else if (store->toolType == MBSYS_HDCS_SeaBat_8111)
		{
		mb_io_ptr->beamwidth_ltrack = 2.0;
		mb_io_ptr->beamwidth_xtrack = 2.0;
		}
	    else if (store->toolType == MBSYS_HDCS_SeaBat_8150)
		{
		mb_io_ptr->beamwidth_ltrack = 2.0;
		mb_io_ptr->beamwidth_xtrack = 2.0;
		}
	    else if (store->toolType == MBSYS_HDCS_EM3002)
		{
		mb_io_ptr->beamwidth_ltrack = 2.0;
		mb_io_ptr->beamwidth_xtrack = 2.0;
		}
	    else if (store->toolType == MBSYS_HDCS_Optech_Laser)
		{
		mb_io_ptr->beamwidth_ltrack = 2.0;
		mb_io_ptr->beamwidth_xtrack = 2.0;
		}
	    else if (store->toolType == MBSYS_HDCS_EM710)
		{
		mb_io_ptr->beamwidth_ltrack = 2.0;
		mb_io_ptr->beamwidth_xtrack = 2.0;
		}
	    else if (store->toolType == MBSYS_HDCS_EM3002D)
		{
		mb_io_ptr->beamwidth_ltrack = 2.0;
		mb_io_ptr->beamwidth_xtrack = 2.0;
		}
	    else if (store->toolType == MBSYS_HDCS_SeaBat_8160)
		{
		mb_io_ptr->beamwidth_ltrack = 2.0;
		mb_io_ptr->beamwidth_xtrack = 2.0;
		}
	    else if (store->toolType == MBSYS_HDCS_SEA_SwathPlus)
		{
		mb_io_ptr->beamwidth_ltrack = 2.0;
		mb_io_ptr->beamwidth_xtrack = 2.0;
		}

	    /* read distance, depth, and backscatter 
		    values into storage arrays */
	    *nbath = store->numDepths_pro;
	    *namp = store->numDepths_pro;
	    *nss = store->pixels_ss;
	    for (i=0;i<*nbath;i++)
		{
		beam = &store->beams[i];
		if (beam->observedDepth == 0)
		    beamflag[i] = MB_FLAG_NULL;
		else if (beam->status == 0)
		    beamflag[i] = MB_FLAG_NONE;
		else if (beam->status == 22)
		    beamflag[i] = MB_FLAG_MANUAL + MB_FLAG_FLAG;
		else
		    beamflag[i] = MB_FLAG_NULL;
		if (beamflag[i] != MB_FLAG_NULL)
		    bath[i] = 0.001 * (abs(beam->observedDepth)
				- store->tide);
		else
		    bath[i] = 0.0;
		bathacrosstrack[i] = 0.001 * beam->acrossTrack;
		bathalongtrack[i] =  0.001 * beam->alongTrack;
		}
	    for (i=0;i<*namp;i++)
		{
		beam = &store->beams[i];
		amp[i] = beam->reflectivity;
		}
	    for (i=0;i<*nss;i++)
		{
		ss[i] = store->ss_proc[i];
		ssacrosstrack[i] = 0.001 * (i - MBSYS_HDCS_MAX_PIXELS / 2)
					* store->pixel_size;
		ssalongtrack[i] =  0.001 * store->ssalongtrack[i];
		}

	    /* print debug statements */
	    if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg4  Data extracted by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg4  Extracted values:\n");
		fprintf(stderr,"dbg4       kind:       %d\n",
			*kind);
		fprintf(stderr,"dbg4       error:      %d\n",
			*error);
		fprintf(stderr,"dbg4       time_i[0]:  %d\n",
			time_i[0]);
		fprintf(stderr,"dbg4       time_i[1]:  %d\n",
			time_i[1]);
		fprintf(stderr,"dbg4       time_i[2]:  %d\n",
			time_i[2]);
		fprintf(stderr,"dbg4       time_i[3]:  %d\n",
			time_i[3]);
		fprintf(stderr,"dbg4       time_i[4]:  %d\n",
			time_i[4]);
		fprintf(stderr,"dbg4       time_i[5]:  %d\n",
			time_i[5]);
		fprintf(stderr,"dbg4       time_i[6]:  %d\n",
			time_i[6]);
		fprintf(stderr,"dbg4       time_d:     %f\n",
			*time_d);
		fprintf(stderr,"dbg4       longitude:  %f\n",
			*navlon);
		fprintf(stderr,"dbg4       latitude:   %f\n",
			*navlat);
		fprintf(stderr,"dbg4       speed:      %f\n",
			*speed);
		fprintf(stderr,"dbg4       heading:    %f\n",
			*heading);
		fprintf(stderr,"dbg4       nbath:      %d\n",
			*nbath);
		for (i=0;i<*nbath;i++)
		  fprintf(stderr,"dbg4       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n",
			i,beamflag[i],bath[i],bathacrosstrack[i],bathalongtrack[i]);
		fprintf(stderr,"dbg4        namp:     %d\n",
			*namp);
		for (i=0;i<*namp;i++)
		  fprintf(stderr,"dbg4        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n",
			i,amp[i],bathacrosstrack[i],bathalongtrack[i]);
		fprintf(stderr,"dbg4        nss:      %d\n",
			*nss);
		for (i=0;i<*nss;i++)
		  fprintf(stderr,"dbg4        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n",
			i,ss[i],ssacrosstrack[i],ssalongtrack[i]);
		}

	    /* done translating values */

	    }

	/* extract comment from structure */
	else if (*kind == MB_DATA_COMMENT)
	    {
	    /* copy comment */
	    strncpy(comment,store->comment,MBSYS_HDCS_MAX_COMMENT);

	    /* print debug statements */
	    if (verbose >= 4)
		{
		fprintf(stderr,"\ndbg4  New ping read by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg4  New ping values:\n");
		fprintf(stderr,"dbg4       error:      %d\n",
			*error);
		fprintf(stderr,"dbg4       comment:    %s\n",
			comment);
		}
	    }

	/* print output debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
		    function_name);
	    fprintf(stderr,"dbg2  Return values:\n");
	    fprintf(stderr,"dbg2       kind:       %d\n",*kind);
	    }
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR 
	    && *kind == MB_DATA_COMMENT)
	    {
	    fprintf(stderr,"dbg2       comment:     \ndbg2       %s\n",
		    comment);
	    }
	else if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR 
	    && *kind != MB_DATA_COMMENT)
	    {
	    fprintf(stderr,"dbg2       time_i[0]:     %d\n",time_i[0]);
	    fprintf(stderr,"dbg2       time_i[1]:     %d\n",time_i[1]);
	    fprintf(stderr,"dbg2       time_i[2]:     %d\n",time_i[2]);
	    fprintf(stderr,"dbg2       time_i[3]:     %d\n",time_i[3]);
	    fprintf(stderr,"dbg2       time_i[4]:     %d\n",time_i[4]);
	    fprintf(stderr,"dbg2       time_i[5]:     %d\n",time_i[5]);
	    fprintf(stderr,"dbg2       time_i[6]:     %d\n",time_i[6]);
	    fprintf(stderr,"dbg2       time_d:        %f\n",*time_d);
	    fprintf(stderr,"dbg2       longitude:     %f\n",*navlon);
	    fprintf(stderr,"dbg2       latitude:      %f\n",*navlat);
	    fprintf(stderr,"dbg2       speed:         %f\n",*speed);
	    fprintf(stderr,"dbg2       heading:       %f\n",*heading);
	    }
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR 
	    && *kind == MB_DATA_DATA)
	    {
	    fprintf(stderr,"dbg2       nbath:      %d\n",
		    *nbath);
	    for (i=0;i<*nbath;i++)
	      fprintf(stderr,"dbg2       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n",
		    i,beamflag[i],bath[i],
		    bathacrosstrack[i],bathalongtrack[i]);
	    fprintf(stderr,"dbg2        namp:     %d\n",
		    *namp);
	    for (i=0;i<*namp;i++)
	      fprintf(stderr,"dbg2       beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n",
		    i,amp[i],bathacrosstrack[i],bathalongtrack[i]);
	    fprintf(stderr,"dbg2        nss:      %d\n",
		    *nss);
	    for (i=0;i<*nss;i++)
	      fprintf(stderr,"dbg2        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n",
		    i,ss[i],ssacrosstrack[i],ssalongtrack[i]);
	    }
	if (verbose >= 2)
	    {
	    fprintf(stderr,"dbg2       error:      %d\n",*error);
	    fprintf(stderr,"dbg2  Return status:\n");
	    fprintf(stderr,"dbg2       status:     %d\n",status);
	    }

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_hdcs_insert(int verbose, void *mbio_ptr, void *store_ptr, 
		int kind, int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading,
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_hdcs_insert";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hdcs_struct *store;
	struct mbsys_hdcs_beam_struct *beam;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
		    function_name);
	    fprintf(stderr,"dbg2  Input arguments:\n");
	    fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
	    fprintf(stderr,"dbg2       mbio_ptr:   %ld\n",(long)mbio_ptr);
	    fprintf(stderr,"dbg2       store_ptr:  %ld\n",(long)store_ptr);
	    fprintf(stderr,"dbg2       kind:       %d\n",kind);
	    }
	if (verbose >= 2 && (kind == MB_DATA_DATA || kind == MB_DATA_NAV))
	    {
	    fprintf(stderr,"dbg2       time_i[0]:  %d\n",time_i[0]);
	    fprintf(stderr,"dbg2       time_i[1]:  %d\n",time_i[1]);
	    fprintf(stderr,"dbg2       time_i[2]:  %d\n",time_i[2]);
	    fprintf(stderr,"dbg2       time_i[3]:  %d\n",time_i[3]);
	    fprintf(stderr,"dbg2       time_i[4]:  %d\n",time_i[4]);
	    fprintf(stderr,"dbg2       time_i[5]:  %d\n",time_i[5]);
	    fprintf(stderr,"dbg2       time_i[6]:  %d\n",time_i[6]);
	    fprintf(stderr,"dbg2       time_d:     %f\n",time_d);
	    fprintf(stderr,"dbg2       navlon:     %f\n",navlon);
	    fprintf(stderr,"dbg2       navlat:     %f\n",navlat);
	    fprintf(stderr,"dbg2       speed:      %f\n",speed);
	    fprintf(stderr,"dbg2       heading:    %f\n",heading);
	    }
	if (verbose >= 2 && kind == MB_DATA_DATA)
	    {
	    fprintf(stderr,"dbg2       nbath:      %d\n",
		    nbath);
	    if (verbose >= 3) 
	     for (i=0;i<nbath;i++)
	      fprintf(stderr,"dbg3       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n",
		    i,beamflag[i],bath[i],
		    bathacrosstrack[i],bathalongtrack[i]);
	    fprintf(stderr,"dbg2       namp:       %d\n",namp);
	    if (verbose >= 3) 
	     for (i=0;i<namp;i++)
	      fprintf(stderr,"dbg3        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n",
		    i,amp[i],bathacrosstrack[i],bathalongtrack[i]);
	    fprintf(stderr,"dbg2        nss:       %d\n",nss);
	    if (verbose >= 3) 
	     for (i=0;i<nss;i++)
	      fprintf(stderr,"dbg3        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n",
		    i,ss[i],ssacrosstrack[i],ssalongtrack[i]);
	    }
	if (verbose >= 2 && kind == MB_DATA_COMMENT)
	    {
	    fprintf(stderr,"dbg2       comment:     \ndbg2       %s\n",
		    comment);
	    }

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_hdcs_struct *) store_ptr;

	/* set data kind */
	store->kind = kind;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
	    {
	    /* get time */
	    store->timeOffset = (int) rint((time_d - 100.0 * store->refTime)
				    / (1.0e-6 * store->timeScale));

	    /* get navigation */
	    if (navlon != 0.0
		|| navlat != 0.0)
		{
		store->positionType = 1;
		store->vesselLongOffset = 1.0E9 * (DTR * navlon 
						- 1.0E-7 * (double)store->refLong)
						/ ((double)store->positionScale);
		store->vesselLatOffset = 1.0E9 * (DTR * navlat 
						- 1.0E-7 * (double)store->refLat)
						/ ((double)store->positionScale);
		}

	    /* get heading */
	    store->vesselHeading = DTR * 1.0E7 
					* (heading 
						- M_PI / 2.0);

	    /* get speed */
	    store->vesselVelocity = speed / 3.6E-3;

	    /* set numbers of beams and sidescan */
	    if (store->num_beam >= nbath
		&& store->beams == NULL)
		{
		status = mb_mallocd(verbose,__FILE__, __LINE__,
			    store->num_beam * sizeof(struct mbsys_hdcs_beam_struct), 
			    (void **)&store->beams,error);
		}

	    /* get bath and sidescan */
	    if (status == MB_SUCCESS
		&& store->num_beam >= nbath)
		{
		store->numDepths_pro = nbath;
		for (i=0;i<store->numDepths_pro;i++)
		    {
		    beam = &store->beams[i];
		    if (mb_beam_check_flag_null(beamflag[i]))
			{
			beam->status = 0;
			beam->observedDepth = 0;
			}
		    else 
			{
			beam->observedDepth 
				= 1000 * bath[i] + store->tide;
			if (mb_beam_ok(beamflag[i]))
			    beam->status = 0;
			else
			    beam->status = 22;
			}
		    }
    
		/* read amplitude values into storage arrays */
		for (i=0;i<store->numDepths_pro;i++)
		    {
		    beam->reflectivity = amp[i];
		    }
		}
	    }

	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT)
	    {
	    strncpy(store->comment,comment,MBSYS_HDCS_MAX_COMMENT);
	    }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return value:\n");
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:  %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_hdcs_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams,
	double *ttimes, double *angles, 
	double *angles_forward, double *angles_null,
	double *heave, double *alongtrack_offset, 
	double *draft, double *ssv, int *error)
{
	char	*function_name = "mbsys_hdcs_ttimes";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hdcs_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
		    function_name);
	    fprintf(stderr,"dbg2  Input arguments:\n");
	    fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
	    fprintf(stderr,"dbg2       mb_ptr:     %ld\n",(long)mbio_ptr);
	    fprintf(stderr,"dbg2       store_ptr:  %ld\n",(long)store_ptr);
	    fprintf(stderr,"dbg2       ttimes:     %ld\n",(long)ttimes);
	    fprintf(stderr,"dbg2       angles_xtrk:%ld\n",(long)angles);
	    fprintf(stderr,"dbg2       angles_ltrk:%ld\n",(long)angles_forward);
	    fprintf(stderr,"dbg2       angles_null:%ld\n",(long)angles_null);
	    fprintf(stderr,"dbg2       heave:      %ld\n",(long)heave);
	    fprintf(stderr,"dbg2       ltrk_off:   %ld\n",(long)alongtrack_offset);
	    }

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_hdcs_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
	    {
	    /* get nbeams */
	    *nbeams = store->numDepths_pro;

	    /* initialize */
	    for (i=0;i<store->numDepths_pro;i++)
		{
		ttimes[i] = 0.0;
		angles[i] = 0.0;
		angles_forward[i] = 0.0;
		angles_null[i] = 0.0;
		heave[i] = 0.0;
		alongtrack_offset[i] = 0.0;
		}

	    /* get ssv */
	    *ssv = 0.0;
	    *draft = 0.0;

	    /* set status */
	    *error = MB_ERROR_NO_ERROR;
	    status = MB_SUCCESS;

	    /* done translating values */

	    }

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT)
	    {
	    /* set status */
	    *error = MB_ERROR_COMMENT;
	    status = MB_FAILURE;
	    }

	/* deal with other record type */
	else
	    {
	    /* set status */
	    *error = MB_ERROR_OTHER;
	    status = MB_FAILURE;
	    }

	/* print output debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
		    function_name);
	    fprintf(stderr,"dbg2  Return values:\n");
	    fprintf(stderr,"dbg2       kind:       %d\n",*kind);
	    }
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR)
	    {
	    fprintf(stderr,"dbg2       draft:      %f\n",*draft);
	    fprintf(stderr,"dbg2       ssv:        %f\n",*ssv);
	    fprintf(stderr,"dbg2       nbeams:     %d\n",*nbeams);
	    for (i=0;i<*nbeams;i++)
		    fprintf(stderr,"dbg2       beam %d: tt:%f  angle_xtrk:%f angle_ltrk:%f  angle_null:%f  depth_off:%f  ltrk_off:%f\n",
			    i,ttimes[i],angles[i],
			    angles_forward[i],angles_null[i],
			    heave[i],alongtrack_offset[i]);
	    }
	if (verbose >= 2)
	    {
	    fprintf(stderr,"dbg2       error:      %d\n",*error);
	    fprintf(stderr,"dbg2  Return status:\n");
	    fprintf(stderr,"dbg2       status:     %d\n",status);
	    }

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_hdcs_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, double *transducer_depth, double *altitude, 
	int *error)
{
	char	*function_name = "mbsys_hdcs_extract_altitude";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hdcs_struct *store;
	struct mbsys_hdcs_beam_struct *beam;

	/* print input debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
		    function_name);
	    fprintf(stderr,"dbg2  Input arguments:\n");
	    fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
	    fprintf(stderr,"dbg2       mb_ptr:     %ld\n",(long)mbio_ptr);
	    fprintf(stderr,"dbg2       store_ptr:  %ld\n",(long)store_ptr);
	    }

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_hdcs_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
	    {
	    *transducer_depth = 0.0;
	    if (store->num_beam > 0)
	    	{
		beam = 	(struct mbsys_hdcs_beam_struct *) &(store->beams[store->num_beam/2]);
	    	*altitude = 0.001 * beam->observedDepth - *transducer_depth;
		}
	    }

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT)
	    {
	    /* set status */
	    *error = MB_ERROR_COMMENT;
	    status = MB_FAILURE;
	    }

	/* deal with other record type */
	else
	    {
	    /* set status */
	    *error = MB_ERROR_OTHER;
	    status = MB_FAILURE;
	    }

	/* print output debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
		    function_name);
	    fprintf(stderr,"dbg2  Return values:\n");
	    fprintf(stderr,"dbg2       kind:              %d\n",*kind);
	    fprintf(stderr,"dbg2       transducer_depth:  %f\n",*transducer_depth);
	    fprintf(stderr,"dbg2       altitude:          %f\n",*altitude);
	    fprintf(stderr,"dbg2       error:             %d\n",*error);
	    fprintf(stderr,"dbg2  Return status:\n");
	    fprintf(stderr,"dbg2       status:            %d\n",status);
	    }

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_hdcs_insert_altitude(int verbose, void *mbio_ptr, void *store_ptr,
	double transducer_depth, double altitude, 
	int *error)
{
	char	*function_name = "mbsys_hdcs_insert_altitude";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hdcs_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
		    function_name);
	    fprintf(stderr,"dbg2  Input arguments:\n");
	    fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
	    fprintf(stderr,"dbg2       mb_ptr:            %ld\n",(long)mbio_ptr);
	    fprintf(stderr,"dbg2       store_ptr:         %ld\n",(long)store_ptr);
	    fprintf(stderr,"dbg2       transducer_depth:  %f\n",transducer_depth);
	    fprintf(stderr,"dbg2       altitude:          %f\n",altitude);
	    }

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_hdcs_struct *) store_ptr;

	/* insert data into structure */
	if (store->kind == MB_DATA_DATA)
	    {
	    }

	/* deal with comment */
	else if (store->kind == MB_DATA_COMMENT)
	    {
	    /* set status */
	    *error = MB_ERROR_COMMENT;
	    status = MB_FAILURE;
	    }

	/* deal with other record type */
	else
	    {
	    /* set status */
	    *error = MB_ERROR_OTHER;
	    status = MB_FAILURE;
	    }

	/* print output debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
		    function_name);
	    fprintf(stderr,"dbg2  Return values:\n");
	    fprintf(stderr,"dbg2       error:             %d\n",*error);
	    fprintf(stderr,"dbg2  Return status:\n");
	    fprintf(stderr,"dbg2       status:            %d\n",status);
	    }

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_hdcs_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading, double *draft, 
		double *roll, double *pitch, double *heave, 
		int *error)
{
	char	*function_name = "mbsys_hdcs_extract_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hdcs_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
		    function_name);
	    fprintf(stderr,"dbg2  Input arguments:\n");
	    fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
	    fprintf(stderr,"dbg2       mb_ptr:     %ld\n",(long)mbio_ptr);
	    fprintf(stderr,"dbg2       store_ptr:  %ld\n",(long)store_ptr);
	    }

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_hdcs_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
	    {
	    /* get time */
	    *time_d = 100.0 * store->refTime
					+ 1.0e-6 
					    * ((double)store->timeScale) 
					    * ((double)store->timeOffset);
	    mb_get_date(verbose,*time_d,time_i);

	    /* get navigation */
	    if (store->positionType == 1)
		{
		*navlon = 
			RTD * (1.0E-7 * (double)store->refLong
				+ 1.0E-9 * (double)store->vesselLongOffset
				    * (double)store->positionScale);
		*navlat = 
			RTD * (1.0E-7 * (double)store->refLat 
				+ 1.0E-9 * (double)store->vesselLatOffset
				    * (double)store->positionScale);
		}
	    else
		{
		*navlon = 0.0;
		*navlat = 0.0;
		}

	    /* get heading */
	    *heading = RTD * (double)store->vesselHeading
					* 1.0E-07 + M_PI / 2.0;

	    /* get speed */
	    *speed = 3.6e-3 * store->vesselVelocity;

	    /* get draft */
	    *draft = 0.0;;

	    /* get roll pitch and heave */
	    *roll = RTD * 1E-7 * store->vesselRoll;
	    *pitch = RTD * 1E-7 * store->vesselPitch;
	    *heave = 0.001 * store->vesselHeave;

	    /* print debug statements */
	    if (verbose >= 5)
		{
		fprintf(stderr,"\ndbg4  Data extracted by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg4  Extracted values:\n");
		fprintf(stderr,"dbg4       kind:       %d\n",
			*kind);
		fprintf(stderr,"dbg4       error:      %d\n",
			*error);
		fprintf(stderr,"dbg4       time_i[0]:  %d\n",
			time_i[0]);
		fprintf(stderr,"dbg4       time_i[1]:  %d\n",
			time_i[1]);
		fprintf(stderr,"dbg4       time_i[2]:  %d\n",
			time_i[2]);
		fprintf(stderr,"dbg4       time_i[3]:  %d\n",
			time_i[3]);
		fprintf(stderr,"dbg4       time_i[4]:  %d\n",
			time_i[4]);
		fprintf(stderr,"dbg4       time_i[5]:  %d\n",
			time_i[5]);
		fprintf(stderr,"dbg4       time_i[6]:  %d\n",
			time_i[6]);
		fprintf(stderr,"dbg4       time_d:     %f\n",
			*time_d);
		fprintf(stderr,"dbg4       longitude:  %f\n",
			*navlon);
		fprintf(stderr,"dbg4       latitude:   %f\n",
			*navlat);
		fprintf(stderr,"dbg4       speed:      %f\n",
			*speed);
		fprintf(stderr,"dbg4       heading:    %f\n",
			*heading);
		fprintf(stderr,"dbg4       draft:      %f\n",
			*draft);
		fprintf(stderr,"dbg4       roll:       %f\n",
			*roll);
		fprintf(stderr,"dbg4       pitch:      %f\n",
			*pitch);
		fprintf(stderr,"dbg4       heave:      %f\n",
			*heave);
		}

	    /* done translating values */

	    }

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT)
	    {
	    /* set status */
	    *error = MB_ERROR_COMMENT;
	    status = MB_FAILURE;
	    }

	/* deal with other record type */
	else
	    {
	    /* set status */
	    *error = MB_ERROR_OTHER;
	    status = MB_FAILURE;
	    }

	/* print output debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
		    function_name);
	    fprintf(stderr,"dbg2  Return values:\n");
	    fprintf(stderr,"dbg2       kind:       %d\n",*kind);
	    }
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR 
		&& *kind == MB_DATA_DATA)
	    {
	    fprintf(stderr,"dbg2       time_i[0]:     %d\n",time_i[0]);
	    fprintf(stderr,"dbg2       time_i[1]:     %d\n",time_i[1]);
	    fprintf(stderr,"dbg2       time_i[2]:     %d\n",time_i[2]);
	    fprintf(stderr,"dbg2       time_i[3]:     %d\n",time_i[3]);
	    fprintf(stderr,"dbg2       time_i[4]:     %d\n",time_i[4]);
	    fprintf(stderr,"dbg2       time_i[5]:     %d\n",time_i[5]);
	    fprintf(stderr,"dbg2       time_i[6]:     %d\n",time_i[6]);
	    fprintf(stderr,"dbg2       time_d:        %f\n",*time_d);
	    fprintf(stderr,"dbg2       longitude:     %f\n",*navlon);
	    fprintf(stderr,"dbg2       latitude:      %f\n",*navlat);
	    fprintf(stderr,"dbg2       speed:         %f\n",*speed);
	    fprintf(stderr,"dbg2       heading:       %f\n",*heading);
	    fprintf(stderr,"dbg2       draft:         %f\n",*draft);
	    fprintf(stderr,"dbg2       roll:          %f\n",*roll);
	    fprintf(stderr,"dbg2       pitch:         %f\n",*pitch);
	    fprintf(stderr,"dbg2       heave:         %f\n",*heave);
	    }
	if (verbose >= 2)
	    {
	    fprintf(stderr,"dbg2       error:      %d\n",*error);
	    fprintf(stderr,"dbg2  Return status:\n");
	    fprintf(stderr,"dbg2       status:     %d\n",status);
	    }

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_hdcs_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading, double draft, 
		double roll, double pitch, double heave,
		int *error)
{
	char	*function_name = "mbsys_hdcs_insert_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hdcs_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
		    function_name);
	    fprintf(stderr,"dbg2  Input arguments:\n");
	    fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
	    fprintf(stderr,"dbg2       mbio_ptr:   %ld\n",(long)mbio_ptr);
	    fprintf(stderr,"dbg2       store_ptr:  %ld\n",(long)store_ptr);
	    fprintf(stderr,"dbg2       time_i[0]:  %d\n",time_i[0]);
	    fprintf(stderr,"dbg2       time_i[1]:  %d\n",time_i[1]);
	    fprintf(stderr,"dbg2       time_i[2]:  %d\n",time_i[2]);
	    fprintf(stderr,"dbg2       time_i[3]:  %d\n",time_i[3]);
	    fprintf(stderr,"dbg2       time_i[4]:  %d\n",time_i[4]);
	    fprintf(stderr,"dbg2       time_i[5]:  %d\n",time_i[5]);
	    fprintf(stderr,"dbg2       time_i[6]:  %d\n",time_i[6]);
	    fprintf(stderr,"dbg2       time_d:     %f\n",time_d);
	    fprintf(stderr,"dbg2       navlon:     %f\n",navlon);
	    fprintf(stderr,"dbg2       navlat:     %f\n",navlat);
	    fprintf(stderr,"dbg2       speed:      %f\n",speed);
	    fprintf(stderr,"dbg2       heading:    %f\n",heading);
	    fprintf(stderr,"dbg2       draft:      %f\n",draft);
	    fprintf(stderr,"dbg2       roll:       %f\n",roll);
	    fprintf(stderr,"dbg2       pitch:      %f\n",pitch);
	    fprintf(stderr,"dbg2       heave:      %f\n",heave);
	    }

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_hdcs_struct *) store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
	    {
	    /* get time */
	    store->timeOffset = (int) rint((time_d - 100.0 * store->refTime)
				    / (1.0e-6 * store->timeScale));

	    /* get navigation */
	    if (navlon != 0.0
		|| navlat != 0.0)
		{
		store->positionType = 1;
		store->vesselLongOffset = 1.0E9 * (DTR * navlon 
						- 1.0E-7 * (double)store->refLong)
						/ ((double)store->positionScale);
		store->vesselLatOffset = 1.0E9 * (DTR * navlat 
						- 1.0E-7 * (double)store->refLat)
						/ ((double)store->positionScale);
		}

	    /* get heading */
	    store->vesselHeading = DTR * 1.0E7 
					* (heading 
						- M_PI / 2.0);

	    /* get speed */
	    store->vesselVelocity = speed / 3.6E-3;

	    /* get draft */

	    /* get roll pitch and heave */
	    store->vesselRoll = DTR * 1E7 * roll;
	    store->vesselPitch = DTR * 1E7 * pitch;
	    store->vesselHeave = 1000 * heave;
	    }

	/* print output debug statements */
	if (verbose >= 2)
	    {
	    fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
		    function_name);
	    fprintf(stderr,"dbg2  Return value:\n");
	    fprintf(stderr,"dbg2       error:      %d\n",*error);
	    fprintf(stderr,"dbg2  Return status:\n");
	    fprintf(stderr,"dbg2       status:  %d\n",status);
	    }

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_hdcs_copy(int verbose, void *mbio_ptr, 
			void *store_ptr, void *copy_ptr,
			int *error)
{
	char	*function_name = "mbsys_hdcs_copy";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_hdcs_struct *store;
	struct mbsys_hdcs_struct *copy;
	struct mbsys_hdcs_beam_struct *beam;
	struct mbsys_hdcs_beam_struct *cbeam;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %ld\n",(long)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %ld\n",(long)store_ptr);
		fprintf(stderr,"dbg2       copy_ptr:   %ld\n",(long)copy_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointers */
	store = (struct mbsys_hdcs_struct *) store_ptr;
	copy = (struct mbsys_hdcs_struct *) copy_ptr;
	
	/* copy the basic header data */
	if (store != NULL && copy != NULL)
	    {
	    /* type of data record */
	    copy->kind = store->kind;
	    copy->read_summary = store->read_summary;
	    copy->fileVersion = store->fileVersion;
	    copy->toolType = store->toolType;
	    copy->profile_size = store->profile_size;
	    copy->num_beam = store->num_beam;
	    copy->beam_size = store->beam_size;
	    copy->data_size = store->data_size;
	    copy->image_size = store->image_size;

	    /* summary values */
	    copy->sensorNumber = store->sensorNumber;
	    copy->subFileID = store->subFileID;
	    copy->fileVersion = store->fileVersion;
	    copy->toolType = store->toolType;
	    copy->numProfiles = store->numProfiles;
	    copy->numDepths_sum = store->numDepths_sum;
	    copy->timeScale = store->timeScale;
	    copy->refTime = store->refTime;
	    copy->minTime = store->minTime;
	    copy->maxTime = store->maxTime;
	    copy->positionType = store->positionType;
	    copy->positionScale = store->positionScale;
	    copy->refLat = store->refLat;
	    copy->minLat = store->minLat;
	    copy->maxLat = store->maxLat;
	    copy->refLong = store->refLong;
	    copy->minLong = store->minLong;
	    copy->maxLong = store->maxLong;
	    copy->minObsDepth = store->minObsDepth;
	    copy->maxObsDepth = store->maxObsDepth;
	    copy->minProcDepth = store->minProcDepth;
	    copy->maxProcDepth = store->maxProcDepth;
	    copy->status_sum = store->status_sum;

	    /* profile values */
	    copy->status_pro = store->status_pro;
	    copy->numDepths_pro = store->numDepths_pro;
	    copy->numSamples = store->numSamples;
	    copy->timeOffset = store->timeOffset;
	    copy->vesselLatOffset = store->vesselLatOffset;
	    copy->vesselLongOffset = store->vesselLongOffset;
	    copy->vesselHeading = store->vesselHeading;
	    copy->vesselHeave = store->vesselHeave;
	    copy->vesselPitch = store->vesselPitch;
	    copy->vesselRoll = store->vesselRoll;
	    copy->tide = store->tide;
	    copy->vesselVelocity = store->vesselVelocity;
	    copy->power = store->power;
	    copy->TVG = store->TVG;
	    copy->attenuation = store->attenuation;
	    copy->edflag = store->edflag;
	    copy->soundVelocity = store->soundVelocity;
	    copy->lengthImageDataField = store->lengthImageDataField;
	    copy->pingNo = store->pingNo;
	    copy->mode = store->mode;
	    copy->Q_factor = store->Q_factor;
	    copy->pulseLength = store->pulseLength;
	    copy->unassigned = store->unassigned;
	    copy->td_sound_speed = store->td_sound_speed;
	    copy->samp_rate = store->samp_rate;
	    copy->z_res_cm = store->z_res_cm;
	    copy->xy_res_cm = store->xy_res_cm;
	    copy->ssp_source = store->ssp_source;
	    copy->filter_ID = store->filter_ID;
	    copy->absorp_coeff = store->absorp_coeff;
	    copy->tx_pulse_len = store->tx_pulse_len;
	    copy->tx_beam_width = store->tx_beam_width;
	    copy->max_swath_width = store->max_swath_width;
	    copy->tx_power_reduction = store->tx_power_reduction;
	    copy->rx_beam_width = store->rx_beam_width;
	    copy->rx_bandwidth = store->rx_bandwidth;
	    copy->rx_gain_reduction = store->rx_gain_reduction;
	    copy->tvg_crossover = store->tvg_crossover;
	    copy->beam_spacing = store->beam_spacing;
	    copy->coverage_sector = store->coverage_sector;
	    copy->yaw_stab_mode = store->yaw_stab_mode;
	    
	    /* don't copy beams that aren't there */
	    if (store->beams == NULL)
		{
		if (copy->beams != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&copy->beams,error);
		}
		
	    /* else copy beams */
	    else
		{
		/* reallocate memory for beams */
		if (copy->beams != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&copy->beams,error);
		status = mb_mallocd(verbose,__FILE__, __LINE__,
				copy->num_beam * sizeof(struct mbsys_hdcs_beam_struct), 
				(void **)&copy->beams,error);
		if (status == MB_SUCCESS)
		    {
		    for (i=0;i<copy->numDepths_pro;i++)
			{
			beam = &store->beams[i];
			cbeam = &copy->beams[i];
			
			cbeam->status = beam->status;
			cbeam->observedDepth = beam->observedDepth;
			cbeam->acrossTrack = beam->acrossTrack;
			cbeam->alongTrack = beam->alongTrack;
			cbeam->latOffset = beam->latOffset;
			cbeam->longOffset = beam->longOffset;
			cbeam->processedDepth = beam->processedDepth;
			cbeam->timeOffset = beam->timeOffset;
			cbeam->depthAccuracy = beam->depthAccuracy;
			cbeam->reflectivity = beam->reflectivity;  
			cbeam->Q_factor = beam->Q_factor;
			cbeam->beam_no = beam->beam_no;
			cbeam->freq = beam->freq;
			cbeam->calibratedBackscatter = beam->calibratedBackscatter;
			cbeam->mindB = beam->mindB;		
			cbeam->maxdB = beam->maxdB;
			cbeam->pseudoAngleIndependentBackscatter = beam->pseudoAngleIndependentBackscatter;
			cbeam->range = beam->range;
			cbeam->no_samples = beam->no_samples;
			cbeam->offset = beam->offset;
			cbeam->centre_no = beam->centre_no;
			cbeam->sample_unit = beam->sample_unit;
			cbeam->sample_interval = beam->sample_interval;
			cbeam->dummy[0] = beam->dummy[0];
			cbeam->dummy[1] = beam->dummy[1];
			cbeam->samp_win_length = beam->samp_win_length;
			cbeam->beam_depress_angle = beam->beam_depress_angle;
			cbeam->beam_heading_angle = beam->beam_heading_angle;
			}
		    }
		}
	    
	    /* don't copy sidescan samples that aren't there */
	    if (store->numSamples == 0
		|| store->ss_raw == NULL)
		{
		if (copy->ss_raw != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&copy->ss_raw,error);
		copy->numSamples = 0;
		}
		
	    /* else copy sidescan samples */
	    else
		{
		/* reallocate memory for ss_raw */
		if (copy->ss_raw != NULL)
		    status = mb_freed(verbose,__FILE__, __LINE__, (void **)&copy->ss_raw,error);
		status = mb_mallocd(verbose,__FILE__, __LINE__,
				copy->numSamples, 
				(void **)&copy->ss_raw,error);
		if (status == MB_SUCCESS)
		    {
		    for (i=0;i<copy->numSamples;i++)
			{
			copy->ss_raw[i] = store->ss_raw[i];
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
	    fprintf(stderr,"dbg2       status:     %d\n",status);
	    }

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
