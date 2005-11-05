/*--------------------------------------------------------------------
 *    The MB-system:	mbr_samesurf.c	6/13/2002
 *	$Id: mbr_samesurf.c,v 5.9 2005-11-05 00:48:05 caress Exp $
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
 * mbr_samesurf.c contains the functions for reading and writing
 * multibeam data in the SAMESURF format.
 * These functions include:
 *   mbr_alm_samesurf	- allocate read/write memory
 *   mbr_dem_samesurf	- deallocate read/write memory
 *   mbr_rt_samesurf	- read and translate data
 *   mbr_wt_samesurf	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	June 13, 2002
 * $Log: not supported by cvs2svn $
 * Revision 5.8  2003/11/24 21:04:29  caress
 * Implement Reinhard Holtkamp's suggested mods for better SURF format support.
 *
 * Revision 5.6  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.5  2003/03/06 00:14:52  caress
 * Put in Reinhard Holtkamp's mod's to support SVP data.
 *
 * Revision 5.4  2003/02/27 04:33:33  caress
 * Fixed handling of SURF format data.
 *
 * Revision 5.3  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.2  2002/08/02 01:01:10  caress
 * 5.0.beta22
 *
 * Revision 5.1  2002/07/20 20:42:40  caress
 * Release 5.0.beta20
 *
 * Revision 5.0  2002/06/13 22:56:59  caress
 * Initial Revision
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
#include "../../include/sapi.h"
#include "../../include/mbsys_surf.h"

/* essential function prototypes */
int mbr_register_samesurf(int verbose, void *mbio_ptr,
		int *error);
int mbr_info_samesurf(int verbose,
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
int mbr_alm_samesurf(int verbose, void *mbio_ptr, int *error);
int mbr_dem_samesurf(int verbose, void *mbio_ptr, int *error);
int mbr_rt_samesurf(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_samesurf(int verbose, void *mbio_ptr, void *store_ptr, int *error);

/*--------------------------------------------------------------------*/
int mbr_register_samesurf(int verbose, void *mbio_ptr, int *error)
{
	static char res_id[]="$Id: mbr_samesurf.c,v 5.9 2005-11-05 00:48:05 caress Exp $";
	char	*function_name = "mbr_register_samesurf";
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
	status = mbr_info_samesurf(verbose,
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
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_samesurf;
	mb_io_ptr->mb_io_format_free = &mbr_dem_samesurf;
	mb_io_ptr->mb_io_store_alloc = &mbsys_surf_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_surf_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_samesurf;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_samesurf;
	mb_io_ptr->mb_io_dimensions = &mbsys_surf_dimensions; 
	mb_io_ptr->mb_io_extract = &mbsys_surf_extract;
	mb_io_ptr->mb_io_insert = &mbsys_surf_insert;
	mb_io_ptr->mb_io_extract_nav = &mbsys_surf_extract_nav;
	mb_io_ptr->mb_io_insert_nav = &mbsys_surf_insert_nav;
	mb_io_ptr->mb_io_extract_altitude = &mbsys_surf_extract_altitude;
	mb_io_ptr->mb_io_insert_altitude = NULL;
	mb_io_ptr->mb_io_extract_svp = &mbsys_surf_extract_svp;
	mb_io_ptr->mb_io_insert_svp = &mbsys_surf_insert_svp;
	mb_io_ptr->mb_io_ttimes = &mbsys_surf_ttimes;
	mb_io_ptr->mb_io_detects = &mbsys_surf_detects;
	mb_io_ptr->mb_io_copyrecord = &mbsys_surf_copy;
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
int mbr_info_samesurf(int verbose,
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
	static char res_id[]="$Id: mbr_samesurf.c,v 5.9 2005-11-05 00:48:05 caress Exp $";
	char	*function_name = "mbr_info_samesurf";
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
	*system = MB_SYS_SURF;
	*beams_bath_max = MBSYS_SURF_MAXBEAMS;
	*beams_amp_max = MBSYS_SURF_MAXBEAMS;
	*pixels_ss_max = MBSYS_SURF_MAXPIXELS;
	strncpy(format_name, "SAMESURF", MB_NAME_LENGTH);
	strncpy(system_name, "SURF", MB_NAME_LENGTH);
	strncpy(format_description, "Format name:          MBF_SAMESURF\nInformal Description: SAM Electronics SURF format.\nAttributes:           variable beams,  bathymetry, amplitude,  and sidescan,\n                      binary, single files, SAM Electronics (formerly Krupp-Atlas Electronik). \n", MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_SURF;
	*variable_beams = MB_YES;
	*traveltime = MB_YES;
	*beam_flagging = MB_YES;
	*nav_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*vru_source = MB_DATA_DATA;
	*svp_source = MB_DATA_DATA;
	*beamwidth_xtrack = 0.0;
	*beamwidth_ltrack = 0.0;

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
int mbr_alm_samesurf(int verbose, void *mbio_ptr, int *error)
{
 static char res_id[]="$Id: mbr_samesurf.c,v 5.9 2005-11-05 00:48:05 caress Exp $";
	char	*function_name = "mbr_alm_samesurf";
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
	status = mbsys_surf_alloc(verbose,mbio_ptr,
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
int mbr_dem_samesurf(int verbose, void *mbio_ptr, int *error)
{
	char	*function_name = "mbr_dem_samesurf";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbf_samesurf_struct *data;

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
	status = mbsys_surf_deall(verbose,mbio_ptr,
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
int mbr_rt_samesurf(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_rt_samesurf";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_surf_struct *store;
	int	sapi_verbose;
	int	sapi_status;
	SurfSoundingData		*SoundingDataPtr;
	SurfTransducerParameterTable	*ActualTransducerTablePtr;
	SurfMultiBeamAngleTable		*ActualAngleTablePtr;
	SurfCProfileTable		*ActualCProfileTablePtr;
	SurfCenterPosition		*CenterPositionPtr;
	SurfSingleBeamDepth		*SingleBeamDepthPtr;
	SurfMultiBeamDepth		*MultiBeamDepthPtr;
	SurfMultiBeamTT			*MultiBeamTraveltimePtr;
	SurfMultiBeamReceive		*MultiBeamReceiveParamsPtr;
	SurfAmplitudes			*MultibeamBeamAmplitudesPtr;
	SurfExtendedAmplitudes		*MultibeamExtendedBeamAmplitudesPtr;
	SurfSignalParameter		*MultibeamSignalParametersPtr;
	SurfTxParameter			*MultibeamTransmitterParametersPtr;
	SurfSidescanData		*SidescanDataPtr;
	int	utm_zone;
	char	projection[MB_NAME_LENGTH];
	double	easting, northing, lon, lat;
	double	*refeasting, *refnorthing;
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
	store = (struct mbsys_surf_struct *) store_ptr;
	refeasting = &(mb_io_ptr->saved1);
	refnorthing = &(mb_io_ptr->saved2);

	/* set sapi verbosity */
	if (verbose > 1)
	    sapi_verbose = verbose;
	else
	    sapi_verbose = 0;

	/* read global info if the structure is blank (usually first time through) */
	if (store->initialized == MB_NO)
		{
		strncpy(store->NameOfShip, SAPI_getNameOfShip(),LABEL_SIZE);
		strncpy(store->NameOfSounder, SAPI_getNameOfSounder(),LABEL_SIZE);
		strncpy(store->TypeOfSounder, SAPI_getTypeOfSounder(),LABEL_SIZE);
		store->NrSoundings = SAPI_getNrSoundings();
		store->NrBeams = SAPI_getNrBeams();
		store->SAPI_posPresentationIsRad = SAPI_posPresentationIsRad();
		store->NrPositionsensors = SAPI_getNrPositionsensors();
		store->NrSoundvelocityProfiles = SAPI_getNrSoundvelocityProfiles();
		store->NrEvents = SAPI_getNrEvents();
		store->NrPolygonElements = SAPI_getNrPolygonElements();
		store->AbsoluteStartTimeOfProfile = SAPI_getAbsoluteStartTimeOfProfile();

		store->GlobalData = *(SAPI_getGlobalData());
		store->Statistics = *(SAPI_getStatistics());

		for (i=0; i<MIN(MBSYS_SURF_MAXCPOS,
				store->NrPositionsensors); i++)
			{
			store->PositionSensor[i] = *(SAPI_getPositionSensor(i));
			}

		/* initialize UTM projection if required */
		if (store->GlobalData.presentationOfPosition == 'X'
			&& mb_io_ptr->projection_initialized == MB_NO)
			{
			/* initialize UTM projection */
			utm_zone = (int)(((RTD * store->GlobalData.referenceMeridian + 183.0)
					/ 6.0) + 0.5);
			sprintf(projection,"UTM%2.2dN", utm_zone);
			mb_proj_init(verbose, projection, &(mb_io_ptr->pjptr), error);
			store->GlobalData.presentationOfPosition = 'E';
			mb_io_ptr->projection_initialized = MB_YES;

			/* Set reference longitude and latitude */
			easting = store->GlobalData.referenceOfPositionX;
			northing = store->GlobalData.referenceOfPositionY;
			mb_proj_inverse(verbose, mb_io_ptr->pjptr,
							easting, northing,
							&lon, &lat,
							error);
			store->GlobalData.referenceOfPositionX = DTR * lon;
			store->GlobalData.referenceOfPositionY = DTR * lat;
			*refeasting = easting;
			*refnorthing = northing;

			/* Set min longitude and latitude */
			easting = store->Statistics.minEasting;
			northing = store->Statistics.minNorthing;
			mb_proj_inverse(verbose, mb_io_ptr->pjptr,
							easting, northing,
							&lon, &lat,
							error);
			store->Statistics.minEasting = DTR * lon;
			store->Statistics.minNorthing = DTR * lat;

			/* Set max longitude and latitude */
			easting = store->Statistics.maxEasting;
			northing = store->Statistics.maxNorthing;
			mb_proj_inverse(verbose, mb_io_ptr->pjptr,
							easting, northing,
							&lon, &lat,
							error);
			store->Statistics.maxEasting = DTR * lon;
			store->Statistics.maxNorthing = DTR * lat;
			}

		store->initialized = MB_YES;
		}

	/* else get access to next sounding */
	else
		{
		sapi_status = SAPI_nextSounding(sapi_verbose);
		if (sapi_status != 0)
			{
			*error = MB_ERROR_EOF;
			status = MB_FAILURE;
			}
		}

	/* extract data for current sounding */
	if (status == MB_SUCCESS)
		{
		/* set kind */
		store->kind = MB_DATA_DATA;

		/* extract data */
		SoundingDataPtr = SAPI_getSoundingData();
		if (SoundingDataPtr != NULL)
			store->SoundingData = *SoundingDataPtr;

		ActualTransducerTablePtr = SAPI_getActualTransducerTable();
		if (ActualTransducerTablePtr != NULL)
			store->ActualTransducerTable = *ActualTransducerTablePtr;

		ActualAngleTablePtr = SAPI_getActualAngleTable();
		if (ActualAngleTablePtr != NULL)
			{
			store->ActualAngleTable = *ActualAngleTablePtr;
			for (i=1; i<MIN(MBSYS_SURF_MAXBEAMS,
					ActualAngleTablePtr->actualNumberOfBeams); i++)
				{
				store->ActualAngleTable.beamAngle[i]
					= ActualAngleTablePtr->beamAngle[i];
				}
			}

		ActualCProfileTablePtr = SAPI_getActualCProfileTable();
		if (ActualCProfileTablePtr != NULL)
			{
			store->ActualCProfileTable = *ActualCProfileTablePtr;
			for (i=1; i<MIN(MBSYS_SURF_MAXCVALUES,
					ActualCProfileTablePtr->numberOfActualValues); i++)
				{
				store->ActualCProfileTable.values[i]
					= ActualCProfileTablePtr->values[i];
				}
			}

		for (i=0; i<MIN(MBSYS_SURF_MAXCPOS,
				store->NrPositionsensors); i++)
			{
			CenterPositionPtr = SAPI_getCenterPosition(i);
			if (CenterPositionPtr != NULL)
				{
				store->CenterPosition[i] = *CenterPositionPtr;

				/* convert position from UTM easting and northing
					to lon lat if necessary */
				if (mb_io_ptr->projection_initialized == MB_YES)
					{
					easting = store->CenterPosition[i].centerPositionX
							+ *refeasting;
					northing = store->CenterPosition[i].centerPositionY
							+ *refnorthing;
					mb_proj_inverse(verbose, mb_io_ptr->pjptr,
								easting, northing,
								&lon, &lat,
								error);
					store->CenterPosition[i].centerPositionX = (float)
						(DTR * lon - store->GlobalData.referenceOfPositionX);
					store->CenterPosition[i].centerPositionY = (float)
						(DTR * lat - store->GlobalData.referenceOfPositionY);
					}
				}
			}

		SingleBeamDepthPtr = SAPI_getSingleBeamDepth();
		if (SingleBeamDepthPtr != NULL)
			store->SingleBeamDepth = *SingleBeamDepthPtr;

		i = 0;
		while ( (i < MIN(MBSYS_SURF_MAXBEAMS, store->NrBeams)) &&
				((MultiBeamDepthPtr = SAPI_getMultiBeamDepth(i)) != NULL) )
			{
			store->MultiBeamDepth[i++] = *MultiBeamDepthPtr;
			}
		store->NrDepths = i;

		i = 0;
		while ( (i < MIN(MBSYS_SURF_MAXBEAMS, store->NrBeams)) &&
				((MultiBeamTraveltimePtr = SAPI_getMultiBeamTraveltime(i)) != NULL) )
			{
			store->MultiBeamTraveltime[i++] = *MultiBeamTraveltimePtr;
			}
		store->NrTravelTimes = i;

		i = 0;
		while ( (i < MIN(MBSYS_SURF_MAXBEAMS, store->NrBeams)) &&
				((MultiBeamReceiveParamsPtr = SAPI_getMultiBeamReceiveParams(i)) != NULL) )
			{
			store->MultiBeamReceiveParams[i++] = *MultiBeamReceiveParamsPtr;
			}
		store->NrRxSets = i;

		i = 0;
		while ( (i < MIN(MBSYS_SURF_MAXBEAMS, store->NrBeams)) &&
				((MultibeamBeamAmplitudesPtr = SAPI_getMultibeamBeamAmplitudes(i)) != NULL) )
			{
			store->MultibeamBeamAmplitudes[i++] = *MultibeamBeamAmplitudesPtr;
			}
		store->NrAmplitudes = i;

		i = 0;
		while ( (i < MIN(MBSYS_SURF_MAXBEAMS, store->NrBeams)) &&
				((MultibeamExtendedBeamAmplitudesPtr = SAPI_getMultibeamExtendedBeamAmplitudes(i)) != NULL) )
			{
			store->MultibeamExtendedBeamAmplitudes[i++] = *MultibeamExtendedBeamAmplitudesPtr;
			}
		store->NrExtAmplitudes = i;

		MultibeamSignalParametersPtr = SAPI_getMultibeamSignalParameters();
		if (MultibeamSignalParametersPtr != NULL)
			{
			store->MultibeamSignalParameters = *MultibeamSignalParametersPtr;
			for (i=1; i<MIN(MBSYS_SURF_MAXRXSETS,
					MultibeamSignalParametersPtr->nrActualGainSets);
					i++)
				{
				store->MultibeamSignalParameters.rxSets[i]
					= MultibeamSignalParametersPtr->rxSets[i];
				}
			}

		store->NrTxSets = 0;
		MultibeamTransmitterParametersPtr =
			SAPI_getMultibeamTransmitterParameters(&store->NrTxSets);
		if (MultibeamTransmitterParametersPtr != NULL)
			{
			store->NrTxSets = MIN(MBSYS_SURF_MAXTXSETS,store->NrTxSets);
			store->MultibeamTransmitterParameters = *MultibeamTransmitterParametersPtr;
			for (i=1; i<store->NrTxSets; i++)
				{
				store->MultibeamTransmitterParameters.txSets[i]
					= MultibeamTransmitterParametersPtr->txSets[i];
				}
 			}

		store->NrSidescan = 0;
		SidescanDataPtr = SAPI_getSidescanData();
		if (SidescanDataPtr != NULL)
			{
			store->SidescanData = *SidescanDataPtr;
			store->NrSidescan = MIN(MBSYS_SURF_MAXPIXELS,
					SidescanDataPtr->actualNrOfSsDataPort
					+ SidescanDataPtr->actualNrOfSsDataStb);
			for (i=1; i<store->NrSidescan; i++)
				{
				store->SidescanData.ssData[i]
					= SidescanDataPtr->ssData[i];
				}
			}
		}

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = store->kind;

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
	if (verbose >= 4 && store->kind == MB_DATA_DATA)
		{
		fprintf(stderr,"\ndbg4  New ping read by MBIO function <%s>\n",
			function_name);
		fprintf(stderr,"dbg4  New ping values:\n");
		fprintf(stderr,"dbg4       kind:               %d\n", store->kind);
		fprintf(stderr,"dbg4       initialized:        %d\n", store->initialized);
		fprintf(stderr,"dbg4       NameOfShip:         %s\n", store->NameOfShip);
		fprintf(stderr,"dbg4       TypeOfSounder:      %s\n", store->TypeOfSounder);
		fprintf(stderr,"dbg4       NameOfSounder:      %s\n", store->NameOfSounder);
		fprintf(stderr,"dbg4       NrSoundings:               %d\n", store->NrSoundings);
		fprintf(stderr,"dbg4       NrBeams:                   %d\n", store->NrBeams);
		fprintf(stderr,"dbg4       SAPI_posPresentationIsRad: %d\n", store->SAPI_posPresentationIsRad);
		fprintf(stderr,"dbg4       NrPositionsensors:         %d\n", store->NrPositionsensors);
		fprintf(stderr,"dbg4       NrSoundvelocityProfiles:   %d\n", store->NrSoundvelocityProfiles);
		fprintf(stderr,"dbg4       NrEvents:                  %d\n", store->NrEvents);
		fprintf(stderr,"dbg4       NrPolygonElements:         %d\n", store->NrPolygonElements);
		fprintf(stderr,"dbg4       AbsoluteStartTimeOfProfile:%f\n", store->AbsoluteStartTimeOfProfile);

		fprintf(stderr,"dbg4       GlobalData.label:                       %s\n", store->GlobalData.label);
		fprintf(stderr,"dbg4       GlobalData.shipsName:                   %s\n", store->GlobalData.shipsName);
		fprintf(stderr,"dbg4       GlobalData.startTimeOfProfile:          %s\n", store->GlobalData.startTimeOfProfile);
		fprintf(stderr,"dbg4       GlobalData.regionOfProfile:             %s\n", store->GlobalData.regionOfProfile);
		fprintf(stderr,"dbg4       GlobalData.numberOfProfile:             %s\n", store->GlobalData.numberOfProfile);
		fprintf(stderr,"dbg4       GlobalData.chartZero:                   %f\n", store->GlobalData.chartZero);
		fprintf(stderr,"dbg4       GlobalData.tideZero:                    %f\n", store->GlobalData.tideZero);
		fprintf(stderr,"dbg4       GlobalData.numberOfMeasuredSoundings:   %d\n", store->GlobalData.numberOfMeasuredSoundings);
		fprintf(stderr,"dbg4       GlobalData.actualNumberOfSoundingSets:  %d\n", store->GlobalData.actualNumberOfSoundingSets);
		fprintf(stderr,"dbg4       GlobalData.timeDateOfTideModification:  %s\n", store->GlobalData.timeDateOfTideModification);
		fprintf(stderr,"dbg4       GlobalData.timeDateOfDepthModification: %s\n", store->GlobalData.timeDateOfDepthModification);
		fprintf(stderr,"dbg4       GlobalData.timeDateOfPosiModification:  %s\n", store->GlobalData.timeDateOfPosiModification);
		fprintf(stderr,"dbg4       GlobalData.timeDateOfParaModification:  %s\n", store->GlobalData.timeDateOfParaModification);
		fprintf(stderr,"dbg4       GlobalData.correctedParameterFlags:     %d\n", store->GlobalData.correctedParameterFlags);
		fprintf(stderr,"dbg4       GlobalData.offsetHeave:                 %f\n", store->GlobalData.offsetHeave);
		fprintf(stderr,"dbg4       GlobalData.offsetRollPort:              %f\n", store->GlobalData.offsetRollPort);
		fprintf(stderr,"dbg4       GlobalData.offsetRollStar:              %f\n", store->GlobalData.offsetRollStar);
		fprintf(stderr,"dbg4       GlobalData.offsetPitchFore:             %f\n", store->GlobalData.offsetPitchFore);
		fprintf(stderr,"dbg4       GlobalData.offsetPitchAft:              %f\n", store->GlobalData.offsetPitchAft);
		fprintf(stderr,"dbg4       GlobalData.nameOfSounder:               %s\n", store->GlobalData.nameOfSounder);
		fprintf(stderr,"dbg4       GlobalData.typeOfSounder:               %c\n", store->GlobalData.typeOfSounder);
		fprintf(stderr,"dbg4       GlobalData.highFrequency:               %f\n", store->GlobalData.highFrequency);
		fprintf(stderr,"dbg4       GlobalData.mediumFrequency:             %f\n", store->GlobalData.mediumFrequency);
		fprintf(stderr,"dbg4       GlobalData.lowFrequency:                %f\n", store->GlobalData.lowFrequency);
		fprintf(stderr,"dbg4       GlobalData.nameOfEllipsoid:             %s\n", store->GlobalData.nameOfEllipsoid);
		fprintf(stderr,"dbg4       GlobalData.semiMajorAxis:               %f\n", store->GlobalData.semiMajorAxis);
		fprintf(stderr,"dbg4       GlobalData.flattening:                  %f\n", store->GlobalData.flattening);
		fprintf(stderr,"dbg4       GlobalData.projection:                  %s\n", store->GlobalData.projection);
		fprintf(stderr,"dbg4       GlobalData.presentationOfPosition:      %c\n", store->GlobalData.presentationOfPosition);
		fprintf(stderr,"dbg4       GlobalData.referenceMeridian:           %f\n", store->GlobalData.referenceMeridian);
		fprintf(stderr,"dbg4       GlobalData.falseEasting:                %f\n", store->GlobalData.falseEasting);
		fprintf(stderr,"dbg4       GlobalData.falseNorthing:               %f\n", store->GlobalData.falseNorthing);
		fprintf(stderr,"dbg4       GlobalData.referenceOfPositionX:        %f\n", store->GlobalData.referenceOfPositionX);
		fprintf(stderr,"dbg4       GlobalData.referenceOfPositionY:        %f\n", store->GlobalData.referenceOfPositionY);
		fprintf(stderr,"dbg4       GlobalData.presentationOfRelWay:        %c\n", store->GlobalData.presentationOfRelWay);
		fprintf(stderr,"dbg4       GlobalData.planedTrackStartX:           %f\n", store->GlobalData.planedTrackStartX);
		fprintf(stderr,"dbg4       GlobalData.planedTrackStartY:           %f\n", store->GlobalData.planedTrackStartY);
		fprintf(stderr,"dbg4       GlobalData.planedTrackStopX:            %f\n", store->GlobalData.planedTrackStopX);
		fprintf(stderr,"dbg4       GlobalData.planedTrackStopY: 	   %f\n", store->GlobalData.planedTrackStopY);
		fprintf(stderr,"dbg4       GlobalData.originalTrackStartX:         %f\n", store->GlobalData.originalTrackStartX);
		fprintf(stderr,"dbg4       GlobalData.originalTrackStartY:         %f\n", store->GlobalData.originalTrackStartY);
		fprintf(stderr,"dbg4       GlobalData.originalTrackStopX:          %f\n", store->GlobalData.originalTrackStopX);
		fprintf(stderr,"dbg4       GlobalData.originalTrackStopY:          %f\n", store->GlobalData.originalTrackStopY);
		fprintf(stderr,"dbg4       GlobalData.originalStartStopDistance:   %f\n", store->GlobalData.originalStartStopDistance);
		fprintf(stderr,"dbg4       GlobalData.originalStartStopTime:       %f\n", store->GlobalData.originalStartStopTime);
		fprintf(stderr,"dbg4       GlobalData.timeDateOfTrackModification: %s\n", store->GlobalData.timeDateOfTrackModification);
		fprintf(stderr,"dbg4       GlobalData.modifiedTrackStartX:         %f\n", store->GlobalData.modifiedTrackStartX);
		fprintf(stderr,"dbg4       GlobalData.modifiedTrackStartY:         %f\n", store->GlobalData.modifiedTrackStartY);
		fprintf(stderr,"dbg4       GlobalData.modifiedTrackStopX:          %f\n", store->GlobalData.modifiedTrackStopX);
		fprintf(stderr,"dbg4       GlobalData.modifiedTrackStopY:          %f\n", store->GlobalData.modifiedTrackStopY);
		fprintf(stderr,"dbg4       GlobalData.modifiedStartStopDistance:   %f\n", store->GlobalData.modifiedStartStopDistance);

		fprintf(stderr,"dbg4       Statistics.label:                       %s\n", store->Statistics.label);
		fprintf(stderr,"dbg4       Statistics.minNorthing:                 %f\n", store->Statistics.minNorthing);
		fprintf(stderr,"dbg4       Statistics.maxNorthing:                 %f\n", store->Statistics.maxNorthing);
		fprintf(stderr,"dbg4       Statistics.minEasting:                  %f\n", store->Statistics.minEasting);
		fprintf(stderr,"dbg4       Statistics.maxEasting:                  %f\n", store->Statistics.maxEasting);
		fprintf(stderr,"dbg4       Statistics.minSpeed:                    %f\n", store->Statistics.minSpeed);
		fprintf(stderr,"dbg4       Statistics.maxSpeed:                    %f\n", store->Statistics.maxSpeed);
		fprintf(stderr,"dbg4       Statistics.minRoll:                     %f\n", store->Statistics.minRoll);
		fprintf(stderr,"dbg4       Statistics.maxRoll:                     %f\n", store->Statistics.maxRoll);
		fprintf(stderr,"dbg4       Statistics.minPitch:                    %f\n", store->Statistics.minPitch);
		fprintf(stderr,"dbg4       Statistics.maxPitch:                    %f\n", store->Statistics.maxPitch);
		fprintf(stderr,"dbg4       Statistics.minHeave:                    %f\n", store->Statistics.minHeave);
		fprintf(stderr,"dbg4       Statistics.maxHeave:                    %f\n", store->Statistics.maxHeave);
		fprintf(stderr,"dbg4       Statistics.minBeamPositionStar:         %f\n", store->Statistics.minBeamPositionStar);
		fprintf(stderr,"dbg4       Statistics.maxBeamPositionStar:         %f\n", store->Statistics.maxBeamPositionStar);
		fprintf(stderr,"dbg4       Statistics.minBeamPositionAhead:        %f\n", store->Statistics.minBeamPositionAhead);
		fprintf(stderr,"dbg4       Statistics.maxBeamPositionAhead:        %f\n", store->Statistics.maxBeamPositionAhead);
		fprintf(stderr,"dbg4       Statistics.minDepth:                    %f\n", store->Statistics.minDepth);
		fprintf(stderr,"dbg4       Statistics.maxDepth:                    %f\n", store->Statistics.maxDepth);

		for (i=0; i<MIN(MBSYS_SURF_MAXCPOS,
				store->NrPositionsensors); i++)
		{
		fprintf(stderr,"dbg4       PositionSensor[%2d].label:                   %s\n", i, store->PositionSensor[i].label);
		fprintf(stderr,"dbg4       PositionSensor[%2d].positionSensorName:      %s\n", i, store->PositionSensor[i].positionSensorName);
		fprintf(stderr,"dbg4       PositionSensor[%2d].none1:                   %f\n", i, store->PositionSensor[i].none1);
		fprintf(stderr,"dbg4       PositionSensor[%2d].none2:                   %f\n", i, store->PositionSensor[i].none2);
		fprintf(stderr,"dbg4       PositionSensor[%2d].none3:                   %f\n", i, store->PositionSensor[i].none3);
		fprintf(stderr,"dbg4       PositionSensor[%2d].none4:                   %f\n", i, store->PositionSensor[i].none4);
		fprintf(stderr,"dbg4       PositionSensor[%2d].none5:                   %f\n", i, store->PositionSensor[i].none5);
		fprintf(stderr,"dbg4       PositionSensor[%2d].none6:                   %f\n", i, store->PositionSensor[i].none6);
		fprintf(stderr,"dbg4       PositionSensor[%2d].none7:                   %f\n", i, store->PositionSensor[i].none7);
		fprintf(stderr,"dbg4       PositionSensor[%2d].none8:                   %f\n", i, store->PositionSensor[i].none8);
		fprintf(stderr,"dbg4       PositionSensor[%2d].time9:                   %s\n", i, store->PositionSensor[i].time9);
		fprintf(stderr,"dbg4       PositionSensor[%2d].none10:                  %f\n", i, store->PositionSensor[i].none10);
		fprintf(stderr,"dbg4       PositionSensor[%2d].none11:                  %f\n", i, store->PositionSensor[i].none11);
		fprintf(stderr,"dbg4       PositionSensor[%2d].none12:                  %f\n", i, store->PositionSensor[i].none12);
		fprintf(stderr,"dbg4       PositionSensor[%2d].none13:                  %f\n", i, store->PositionSensor[i].none13);
		fprintf(stderr,"dbg4       PositionSensor[%2d].none14:                  %f\n", i, store->PositionSensor[i].none14);
		fprintf(stderr,"dbg4       PositionSensor[%2d].none15:                  %f\n", i, store->PositionSensor[i].none15);
		fprintf(stderr,"dbg4       PositionSensor[%2d].none16:                  %f\n", i, store->PositionSensor[i].none16);
		fprintf(stderr,"dbg4       PositionSensor[%2d].none17:                  %f\n", i, store->PositionSensor[i].none17);
		fprintf(stderr,"dbg4       PositionSensor[%2d].sensorAntennaPositionAhead:  %f\n", store->PositionSensor[i].sensorAntennaPositionAhead);
		fprintf(stderr,"dbg4       PositionSensor[%2d].sensorAntennaPositionStar:   %f\n", store->PositionSensor[i].sensorAntennaPositionStar);
		fprintf(stderr,"dbg4       PositionSensor[%2d].sensorAntennaPositionHeight: %f\n", store->PositionSensor[i].sensorAntennaPositionHeight);
		}

		fprintf(stderr,"dbg4       SoundingData.soundingFlag:              %d\n", store->SoundingData.soundingFlag);
		fprintf(stderr,"dbg4       SoundingData.indexToAngle:              %d\n", store->SoundingData.indexToAngle);
		fprintf(stderr,"dbg4       SoundingData.indexToTransducer:         %d\n", store->SoundingData.indexToTransducer);
		fprintf(stderr,"dbg4       SoundingData.indexToCProfile:           %d\n", store->SoundingData.indexToCProfile);
		fprintf(stderr,"dbg4       SoundingData.relTime:                   %f\n", store->SoundingData.relTime);
		fprintf(stderr,"dbg4       SoundingData.relWay:                    %f\n", store->SoundingData.relWay);
		fprintf(stderr,"dbg4       SoundingData.tide:                      %f\n", store->SoundingData.tide);
		fprintf(stderr,"dbg4       SoundingData.headingWhileTransmitting:  %f\n", store->SoundingData.headingWhileTransmitting);
		fprintf(stderr,"dbg4       SoundingData.heaveWhileTransmitting:    %f\n", store->SoundingData.heaveWhileTransmitting);
		fprintf(stderr,"dbg4       SoundingData.rollWhileTransmitting:     %f\n", store->SoundingData.rollWhileTransmitting);
		fprintf(stderr,"dbg4       SoundingData.pitchWhileTransmitting:    %f\n", store->SoundingData.pitchWhileTransmitting);
		fprintf(stderr,"dbg4       SoundingData.cKeel:                     %f\n", store->SoundingData.cKeel);
		fprintf(stderr,"dbg4       SoundingData.cMean:                     %f\n", store->SoundingData.cMean);
		fprintf(stderr,"dbg4       SoundingData.dynChartZero:              %f\n", store->SoundingData.dynChartZero);

		fprintf(stderr,"dbg4       ActualTransducerTable.label:                   %s\n", store->ActualTransducerTable.label);
		fprintf(stderr,"dbg4       ActualTransducerTable.transducerDepth:         %f\n", store->ActualTransducerTable.transducerDepth);
		fprintf(stderr,"dbg4       ActualTransducerTable.transducerPositionAhead: %f\n", store->ActualTransducerTable.transducerPositionAhead);
		fprintf(stderr,"dbg4       ActualTransducerTable.transducerPositionStar:  %f\n", store->ActualTransducerTable.transducerPositionStar);
		fprintf(stderr,"dbg4       ActualTransducerTable.transducerTwoThetaHFreq: %f\n", store->ActualTransducerTable.transducerTwoThetaHFreq);
		fprintf(stderr,"dbg4       ActualTransducerTable.transducerTwoThetaMFreq: %f\n", store->ActualTransducerTable.transducerTwoThetaMFreq);
		fprintf(stderr,"dbg4       ActualTransducerTable.transducerTwoThetaLFreq: %f\n", store->ActualTransducerTable.transducerTwoThetaLFreq);

		fprintf(stderr,"dbg4       ActualAngleTable.label:                 %s\n", store->ActualAngleTable.label);
		fprintf(stderr,"dbg4       ActualAngleTable.actualNumberOfBeams:   %d\n", store->ActualAngleTable.actualNumberOfBeams);
		for (i=0; i<MIN(MBSYS_SURF_MAXBEAMS,
				store->ActualAngleTable.actualNumberOfBeams); i++)
		fprintf(stderr,"dbg4       ActualAngleTable.beamAngle[%3d]:        %f\n", i, store->ActualAngleTable.beamAngle[i]);

		fprintf(stderr,"dbg4       ActualCProfileTable.label:                     %s\n", store->ActualCProfileTable.label);
		fprintf(stderr,"dbg4       ActualCProfileTable.relTime:                   %f\n", store->ActualCProfileTable.relTime);
		fprintf(stderr,"dbg4       ActualCProfileTable.numberOfActualValues:      %d\n", store->ActualCProfileTable.numberOfActualValues);
		for (i=0; i<MIN(MBSYS_SURF_MAXCVALUES,
				store->ActualCProfileTable.numberOfActualValues); i++)
		fprintf(stderr,"dbg4       ActualCProfileTable.values[%3d]:               %f %f\n",
			i, store->ActualCProfileTable.values[i].depth, store->ActualCProfileTable.values[i].cValue);

		for (i=0; i<MIN(MBSYS_SURF_MAXCPOS,
				store->NrPositionsensors); i++)
		{
		fprintf(stderr,"dbg4       CenterPosition[%2d].positionFlag:            %d\n", i, store->CenterPosition[i].positionFlag);
		fprintf(stderr,"dbg4       CenterPosition[%2d].centerPositionX:         %f\n", i, store->CenterPosition[i].centerPositionX);
		fprintf(stderr,"dbg4       CenterPosition[%2d].centerPositionY:         %f\n", i, store->CenterPosition[i].centerPositionY);
		fprintf(stderr,"dbg4       CenterPosition[%2d].speed:                   %f\n", i, store->CenterPosition[i].speed);
		}

		fprintf(stderr,"dbg4       SingleBeamDepth.depthFlag:              %d\n", store->SingleBeamDepth.depthFlag);
		fprintf(stderr,"dbg4       SingleBeamDepth.travelTimeOfRay:        %f\n", store->SingleBeamDepth.travelTimeOfRay);
		fprintf(stderr,"dbg4       SingleBeamDepth.depthHFreq:             %f\n", store->SingleBeamDepth.depthHFreq);
		fprintf(stderr,"dbg4       SingleBeamDepth.depthMFreq:             %f\n", store->SingleBeamDepth.depthMFreq);
		fprintf(stderr,"dbg4       SingleBeamDepth.depthLFreq:             %f\n", store->SingleBeamDepth.depthLFreq);

		for (i=0; i<MIN(MBSYS_SURF_MAXBEAMS,
				store->NrBeams); i++)
		{
		fprintf(stderr,"\ndbg4       MultiBeamDepth[%3d].depthFlag:                      %d\n", i, store->MultiBeamDepth[i].depthFlag);
		fprintf(stderr,"dbg4       MultiBeamDepth[%3d].depth:                          %f\n", i, store->MultiBeamDepth[i].depth);
		fprintf(stderr,"dbg4       MultiBeamDepth[%3d].beamPositionAhead:              %f\n", i, store->MultiBeamDepth[i].beamPositionAhead);
		fprintf(stderr,"dbg4       MultiBeamDepth[%3d].beamPositionStar:               %f\n", i, store->MultiBeamDepth[i].beamPositionStar);
		fprintf(stderr,"dbg4       MultiBeamTraveltime[%3d].travelTimeOfRay:           %f\n", i, store->MultiBeamTraveltime[i].travelTimeOfRay);
		fprintf(stderr,"dbg4       MultiBeamReceiveParams[%3d].headingWhileReceiving:  %f\n", i, store->MultiBeamReceiveParams[i].headingWhileReceiving);
		fprintf(stderr,"dbg4       MultiBeamReceiveParams[%3d].heaveWhileReceiving:    %f\n", i, store->MultiBeamReceiveParams[i].heaveWhileReceiving);
		fprintf(stderr,"dbg4       MultibeamBeamAmplitudes[%3d].beamAmplitude:         %d\n", i, store->MultibeamBeamAmplitudes[i].beamAmplitude);
		fprintf(stderr,"dbg4       MultibeamBeamExtendedAmplitudes[%3d].mtau:          %f\n", i, store->MultibeamExtendedBeamAmplitudes[i].mtau);
		fprintf(stderr,"dbg4       MultibeamBeamExtendedAmplitudes[%3d].nis:           %d\n", i, store->MultibeamExtendedBeamAmplitudes[i].nis);
		fprintf(stderr,"dbg4       MultibeamBeamExtendedAmplitudes[%3d].beamAmplitude: %d\n", i, store->MultibeamExtendedBeamAmplitudes[i].beamAmplitude);
		}

		fprintf(stderr,"\ndbg4       MultibeamSignalParameters.bscatClass:          %d\n", store->MultibeamSignalParameters.bscatClass);
		fprintf(stderr,"dbg4       MultibeamSignalParameters.nrActualGainSets:    %d\n", store->MultibeamSignalParameters.nrActualGainSets);
		fprintf(stderr,"dbg4       MultibeamSignalParameters.rxGup:               %f\n", store->MultibeamSignalParameters.rxGup);
		fprintf(stderr,"dbg4       MultibeamSignalParameters.rxGain:              %f\n", store->MultibeamSignalParameters.rxGain);
		fprintf(stderr,"dbg4       MultibeamSignalParameters.ar:                  %f\n", store->MultibeamSignalParameters.ar);
		for (i=0; i<MIN(MBSYS_SURF_MAXRXSETS,
				store->MultibeamSignalParameters.nrActualGainSets); i++)
		fprintf(stderr,"dbg4       MultibeamSignalParameters.rxSets[%3d]:               %f %f\n",
			i, store->MultibeamSignalParameters.rxSets[i].time, store->MultibeamSignalParameters.rxSets[i].gain);

		fprintf(stderr,"dbg4       MultibeamTransmitterParameters.txSets[0].txBeamIndex: %d\n", store->MultibeamTransmitterParameters.txSets[0].txBeamIndex);
		fprintf(stderr,"dbg4       MultibeamTransmitterParameters.txSets[0].txLevel:     %f\n", store->MultibeamTransmitterParameters.txSets[0].txLevel);
		fprintf(stderr,"dbg4       MultibeamTransmitterParameters.txSets[0].txBeamAngle: %f\n", store->MultibeamTransmitterParameters.txSets[0].txBeamAngle);
		fprintf(stderr,"dbg4       MultibeamTransmitterParameters.txSets[0].pulseLength: %f\n", store->MultibeamTransmitterParameters.txSets[0].pulseLength);

		fprintf(stderr,"dbg4       SidescanData.sidescanFlag:              %d\n", store->SidescanData.sidescanFlag);
		fprintf(stderr,"dbg4       SidescanData.actualNrOfSsDataPort:      %d\n", store->SidescanData.actualNrOfSsDataPort);
		fprintf(stderr,"dbg4       SidescanData.actualNrOfSsDataStb:       %d\n", store->SidescanData.actualNrOfSsDataStb);
		fprintf(stderr,"dbg4       SidescanData.minSsTimePort:             %f\n", store->SidescanData.minSsTimePort);
		fprintf(stderr,"dbg4       SidescanData.minSsTimeStb:              %f\n", store->SidescanData.minSsTimeStb);
		fprintf(stderr,"dbg4       SidescanData.maxSsTimePort:             %f\n", store->SidescanData.maxSsTimePort);
		fprintf(stderr,"dbg4       SidescanData.maxSsTimeStb:              %f\n", store->SidescanData.maxSsTimeStb);
		for (i=0; i<MIN(MBSYS_SURF_MAXPIXELS,
				store->SidescanData.actualNrOfSsDataPort
				+ store->SidescanData.actualNrOfSsDataPort);
				i++)
		fprintf(stderr,"dbg4       SidescanData.ssData[%d]:        %d\n", i, store->SidescanData.ssData[i]);
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
int mbr_wt_samesurf(int verbose, void *mbio_ptr, void *store_ptr, int *error)
{
	char	*function_name = "mbr_wt_samesurf";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_surf_struct *store;

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
	store = (struct mbsys_surf_struct *) store_ptr;

	/* write failure always */
	status = MB_FAILURE;
	*error = MB_ERROR_WRITE_FAIL;

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
