/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_netcdf.c	4/11/2002
 *	$Id: mbsys_netcdf.c,v 5.1 2002-05-29 23:40:48 caress Exp $
 *
 *    Copyright (c) 2002 by
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
 * mbsys_netcdf.c includes the functions used by MBIO functions
 * to extract and insert data from the IFREMER netCDF multibeam format.
 * The MBIO format id is:
 *      MBF_MBNETCDF : MBIO ID 75
 *
 * Author:	D. W. Caress
 * Date:	April 11, 2002
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.0  2002/05/02 03:56:34  caress
 * Release 5.0.beta17
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
#include "../../include/mbsys_netcdf.h"

static char res_id[]="$Id: mbsys_netcdf.c,v 5.1 2002-05-29 23:40:48 caress Exp $";

/*--------------------------------------------------------------------*/
int mbsys_netcdf_alloc(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error)
{
	char	*function_name = "mbsys_netcdf_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_netcdf_struct *store;
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

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* allocate memory for data structure */
	status = mb_malloc(verbose,sizeof(struct mbsys_netcdf_struct),
				store_ptr,error);

	/* get data structure pointer */
	store = (struct mbsys_netcdf_struct *) *store_ptr;
	
	/* now initialize everything */
	if (status == MB_SUCCESS)
	    {	
	    /* dimensions */
	    store->mbHistoryRecNbr = 0 ;
	    store->mbNameLength = MBSYS_NETCDF_NAMELEN ;
	    store->mbCommentLength = MBSYS_NETCDF_COMMENTLEN ;
	    store->mbAntennaNbr = 1 ;
	    store->mbBeamNbr = 0 ;
	    store->mbCycleNbr = 0 ;
	    store->mbVelocityProfilNbr = MBSYS_NETCDF_VELPROFNBR ;
    
	    /* global attributes */
	    store->mbVersion = 108;
	    strcpy(store->mbName, " ");
	    strcpy(store->mbClasse, "MB_SWATH");
	    store->mbLevel = 0;
	    store->mbNbrHistoryRec = 0;
	    strcpy(store->mbTimeReference, "Julian date for 1970/01/01 = 2 440 588");
	    store->mbStartDate = 0;
	    store->mbStartTime = 0;
	    store->mbEndDate = 0;
	    store->mbEndTime = 0;
	    store->mbNorthLatitude = 0.;
	    store->mbSouthLatitude = 0.;
	    store->mbEastLongitude = 0.;
	    store->mbWestLongitude = 0.;
	    strcpy(store->mbMeridian180, " ");
	    strcpy(store->mbGeoDictionnary, "                    ");
	    strcpy(store->mbGeoRepresentation, "                    ");
	    strcpy(store->mbGeodesicSystem, "                    ");
	    strcpy(store->mbEllipsoidName, "                                                                                                                                                                                                                                                                ");
	    store->mbEllipsoidA = 0.;
	    store->mbEllipsoidInvF = 0.;
	    store->mbEllipsoidE2 = 0.;
	    store->mbProjType = -1;
	    for (i=0;i<10;i++)
		store->mbProjParameterValue[i] = 0.;
	    strcpy(store->mbProjParameterCode, "                                                                                                                                                                                                                                                                ");
	    store->mbSounder = MBSYS_NETCDF_SONAR_UNKNOWN;
	    strcpy(store->mbShip, "                                                                                                                                                                                                                                                                ");
	    strcpy(store->mbSurvey, "                                                                                                                                                                                                                                                                ");
	    strcpy(store->mbReference, "                                                                                                                                                                                                                                                                ");
	    for (i=0;i<3;i++)
		store->mbAntennaOffset[i] = 0.;
	    store->mbAntennaDelay = 0.;
	    for (i=0;i<3;i++)
		store->mbSounderOffset[i] = 0.;
	    store->mbSounderDelay = 0.;
	    for (i=0;i<3;i++)
		store->mbVRUOffset[i] = 0.;
	    store->mbVRUDelay = 0.;
	    store->mbHeadingBias = 0.;
	    store->mbRollBias = 0.;
	    store->mbPitchBias = 0.;
	    store->mbHeaveBias = 0.;
	    store->mbDraught = 0.;
	    store->mbNavType = 0;
	    strcpy(store->mbNavRef, "                                                                                                                                                                                                                                                                ");
	    store->mbTideType = 0;
	    strcpy(store->mbTideRef, "                                                                                                                                                                                                                                                                ");
	    store->mbMinDepth = 0.;
	    store->mbMaxDepth = 0.;
	    store->mbCycleCounter = 0;
	    
	    /* variable attributes */
	    strcpy(store->mbHistDate_type, "integer");
	    strcpy(store->mbHistDate_long_name, "History date");
	    strcpy(store->mbHistDate_name_code, "MB_HISTORY_DATE");
	    strcpy(store->mbHistDate_units, "Julian_date");
	    strcpy(store->mbHistDate_unit_code, "MB_JULIAN_DATE");
	    store->mbHistDate_add_offset = 2440588;
	    store->mbHistDate_scale_factor = 1;
	    store->mbHistDate_minimum = -25567;
	    store->mbHistDate_maximum = 47482;
	    store->mbHistDate_valid_minimum = -25567;
	    store->mbHistDate_valid_maximum = 47482;
	    store->mbHistDate_missing_value = 2147483647;
	    strcpy(store->mbHistDate_format_C, "%d");
	    strcpy(store->mbHistDate_orientation, "direct");
	    strcpy(store->mbHistTime_type, "integer");
	    strcpy(store->mbHistTime_long_name, "History time (UT)");
	    strcpy(store->mbHistTime_name_code, "MB_HISTORY_TIME");
	    strcpy(store->mbHistTime_units, "ms");
	    strcpy(store->mbHistTime_unit_code, "MB_MS");
	    store->mbHistTime_add_offset = 0;
	    store->mbHistTime_scale_factor = 1;
	    store->mbHistTime_minimum = 0;
	    store->mbHistTime_maximum = 86399999;
	    store->mbHistTime_valid_minimum = 0;
	    store->mbHistTime_valid_maximum = 86399999;
	    store->mbHistTime_missing_value = -2147483648;
	    strcpy(store->mbHistTime_format_C, "%d");
	    strcpy(store->mbHistTime_orientation, "direct");
	    strcpy(store->mbHistCode_type, "integer");
	    strcpy(store->mbHistCode_long_name, "History code");
	    strcpy(store->mbHistCode_name_code, "MB_HISTORY_CODE");
	    strcpy(store->mbHistCode_units, "");
	    strcpy(store->mbHistCode_unit_code, "MB_NOT_DEFINED");
	    store->mbHistCode_add_offset = 0;
	    store->mbHistCode_scale_factor = 1;
	    store->mbHistCode_minimum = 1;
	    store->mbHistCode_maximum = 255;
	    store->mbHistCode_valid_minimum = 1;
	    store->mbHistCode_valid_maximum = 255;
	    store->mbHistCode_missing_value = 0;
	    strcpy(store->mbHistCode_format_C, "%d");
	    strcpy(store->mbHistCode_orientation, "direct");
	    strcpy(store->mbHistAutor_type, "string");
	    strcpy(store->mbHistAutor_long_name, "History autor");
	    strcpy(store->mbHistAutor_name_code, "MB_HISTORY_AUTOR");
	    strcpy(store->mbHistModule_type, "string");
	    strcpy(store->mbHistModule_long_name, "History module");
	    strcpy(store->mbHistModule_name_code, "MB_HISTORY_MODULE");
	    strcpy(store->mbHistComment_type, "string");
	    strcpy(store->mbHistComment_long_name, "History comment");
	    strcpy(store->mbHistComment_name_code, "MB_HISTORY_COMMENT");
	    strcpy(store->mbCycle_type, "integer");
	    strcpy(store->mbCycle_long_name, "Cycle number");
	    strcpy(store->mbCycle_name_code, "MB_CYCLE_NUMBER");
	    strcpy(store->mbCycle_units, "");
	    strcpy(store->mbCycle_unit_code, "MB_NOT_DEFINED");
	    store->mbCycle_add_offset = 0;
	    store->mbCycle_scale_factor = 1;
	    store->mbCycle_minimum = 1;
	    store->mbCycle_maximum = 65535;
	    store->mbCycle_valid_minimum = 1;
	    store->mbCycle_valid_maximum = 65535;
	    store->mbCycle_missing_value = 0;
	    strcpy(store->mbCycle_format_C, "%d");
	    strcpy(store->mbCycle_orientation, "direct");
	    strcpy(store->mbDate_type, "integer");
	    strcpy(store->mbDate_long_name, "Date of cycle");
	    strcpy(store->mbDate_name_code, "MB_CYCLE_DATE");
	    strcpy(store->mbDate_units, "Julian_date");
	    strcpy(store->mbDate_unit_code, "MB_JULIAN_DATE");
	    store->mbDate_add_offset = 2440588;
	    store->mbDate_scale_factor = 1;
	    store->mbDate_minimum = -25567;
	    store->mbDate_maximum = 47482;
	    store->mbDate_valid_minimum = -25567;
	    store->mbDate_valid_maximum = 47482;
	    store->mbDate_missing_value = 2147483647;
	    strcpy(store->mbDate_format_C, "%d");
	    strcpy(store->mbDate_orientation, "direct");
	    strcpy(store->mbTime_type, "integer");
	    strcpy(store->mbTime_long_name, "Time of cycle");
	    strcpy(store->mbTime_name_code, "MB_CYCLE_TIME");
	    strcpy(store->mbTime_units, "ms");
	    strcpy(store->mbTime_unit_code, "MB_MS");
	    store->mbTime_add_offset = 0;
	    store->mbTime_scale_factor = 1;
	    store->mbTime_minimum = 0;
	    store->mbTime_maximum = 86399999;
	    store->mbTime_valid_minimum = 0;
	    store->mbTime_valid_maximum = 86399999;
	    store->mbTime_missing_value = -2147483648;
	    strcpy(store->mbTime_format_C, "%d");
	    strcpy(store->mbTime_orientation, "direct");
	    strcpy(store->mbOrdinate_type, "real");
	    strcpy(store->mbOrdinate_long_name, "Latitude");
	    strcpy(store->mbOrdinate_name_code, "MB_POSITION_LATITUDE");
	    strcpy(store->mbOrdinate_units, "degree");
	    strcpy(store->mbOrdinate_unit_code, "MB_DEGREE");
	    store->mbOrdinate_add_offset = 0.;
	    store->mbOrdinate_scale_factor = 5.e-08;
	    store->mbOrdinate_minimum = -1800000000;
	    store->mbOrdinate_maximum = 1800000000;
	    store->mbOrdinate_valid_minimum = -1800000000;
	    store->mbOrdinate_valid_maximum = 1800000000;
	    store->mbOrdinate_missing_value = -2147483648;
	    strcpy(store->mbOrdinate_format_C, "%f");
	    strcpy(store->mbOrdinate_orientation, "direct");
	    strcpy(store->mbAbscissa_type, "real");
	    strcpy(store->mbAbscissa_long_name, "Longitude");
	    strcpy(store->mbAbscissa_name_code, "MB_POSITION_LONGITUDE");
	    strcpy(store->mbAbscissa_units, "degree");
	    strcpy(store->mbAbscissa_unit_code, "MB_DEGREE");
	    store->mbAbscissa_add_offset = 0.;
	    store->mbAbscissa_scale_factor = 1.e-07;
	    store->mbAbscissa_minimum = -1800000000;
	    store->mbAbscissa_maximum = 1800000000;
	    store->mbAbscissa_valid_minimum = -1800000000;
	    store->mbAbscissa_valid_maximum = 1800000000;
	    store->mbAbscissa_missing_value = -2147483648;
	    strcpy(store->mbAbscissa_format_C, "%f");
	    strcpy(store->mbAbscissa_orientation, "direct");
	    strcpy(store->mbFrequency_type, "integer");
	    strcpy(store->mbFrequency_long_name, "Frequency of cycle");
	    strcpy(store->mbFrequency_name_code, "MB_CYCLE_FREQUENCY");
	    strcpy(store->mbFrequency_units, "");
	    strcpy(store->mbFrequency_unit_code, "MB_NOT_DEFINED");
	    store->mbFrequency_add_offset = 0;
	    store->mbFrequency_scale_factor = 1;
	    store->mbFrequency_minimum = 1;
	    store->mbFrequency_maximum = 255;
	    store->mbFrequency_valid_minimum = 1;
	    store->mbFrequency_valid_maximum = 255;
	    store->mbFrequency_missing_value = 0;
	    strcpy(store->mbFrequency_format_C, "%d");
	    strcpy(store->mbFrequency_orientation, "direct");
	    strcpy(store->mbSounderMode_type, "integer");
	    strcpy(store->mbSounderMode_long_name, "Sounder mode");
	    strcpy(store->mbSounderMode_name_code, "MB_CYCLE_MODE");
	    strcpy(store->mbSounderMode_units, "");
	    strcpy(store->mbSounderMode_unit_code, "MB_NOT_DEFINED");
	    store->mbSounderMode_add_offset = 0;
	    store->mbSounderMode_scale_factor = 1;
	    store->mbSounderMode_minimum = 1;
	    store->mbSounderMode_maximum = 255;
	    store->mbSounderMode_valid_minimum = 1;
	    store->mbSounderMode_valid_maximum = 255;
	    store->mbSounderMode_missing_value = 0;
	    strcpy(store->mbSounderMode_format_C, "%d");
	    strcpy(store->mbSounderMode_orientation, "direct");
	    strcpy(store->mbReferenceDepth_type, "real");
	    strcpy(store->mbReferenceDepth_long_name, "Depth of the reference point");
	    strcpy(store->mbReferenceDepth_name_code, "MB_CYCLE_REFERENCE_DEPTH");
	    strcpy(store->mbReferenceDepth_units, "m");
	    strcpy(store->mbReferenceDepth_unit_code, "MB_M");
	    store->mbReferenceDepth_add_offset = 0.;
	    store->mbReferenceDepth_scale_factor = 0.1;
	    store->mbReferenceDepth_minimum = -32767;
	    store->mbReferenceDepth_maximum = 32767;
	    store->mbReferenceDepth_valid_minimum = -32767;
	    store->mbReferenceDepth_valid_maximum = 32767;
	    store->mbReferenceDepth_missing_value = -32768;
	    strcpy(store->mbReferenceDepth_format_C, "%.2f");
	    strcpy(store->mbReferenceDepth_orientation, "direct");
	    strcpy(store->mbDynamicDraught_type, "real");
	    strcpy(store->mbDynamicDraught_long_name, "Dynamic draught correction");
	    strcpy(store->mbDynamicDraught_name_code, "MB_CYCLE_DRAUGHT");
	    strcpy(store->mbDynamicDraught_units, "m");
	    strcpy(store->mbDynamicDraught_unit_code, "MB_M");
	    store->mbDynamicDraught_add_offset = 0.;
	    store->mbDynamicDraught_scale_factor = 0.01;
	    store->mbDynamicDraught_minimum = -1000;
	    store->mbDynamicDraught_maximum = 1000;
	    store->mbDynamicDraught_valid_minimum = -1000;
	    store->mbDynamicDraught_valid_maximum = 1000;
	    store->mbDynamicDraught_missing_value = -32768;
	    strcpy(store->mbDynamicDraught_format_C, "%.2f");
	    strcpy(store->mbDynamicDraught_orientation, "direct");
	    strcpy(store->mbTide_type, "real");
	    strcpy(store->mbTide_long_name, "Tide correction");
	    strcpy(store->mbTide_name_code, "MB_CYCLE_TIDE");
	    strcpy(store->mbTide_units, "m");
	    strcpy(store->mbTide_unit_code, "MB_M");
	    store->mbTide_add_offset = 0.;
	    store->mbTide_scale_factor = 0.01;
	    store->mbTide_minimum = -2000;
	    store->mbTide_maximum = 2000;
	    store->mbTide_valid_minimum = -2000;
	    store->mbTide_valid_maximum = 2000;
	    store->mbTide_missing_value = -32768;
	    strcpy(store->mbTide_format_C, "%.2f");
	    strcpy(store->mbTide_orientation, "direct");
	    strcpy(store->mbSoundVelocity_type, "real");
	    strcpy(store->mbSoundVelocity_long_name, "Surface sound velocity");
	    strcpy(store->mbSoundVelocity_name_code, "MB_CYCLE_VELOCITY");
	    strcpy(store->mbSoundVelocity_units, "m/s");
	    strcpy(store->mbSoundVelocity_unit_code, "MB_M/S");
	    store->mbSoundVelocity_add_offset = 1400.;
	    store->mbSoundVelocity_scale_factor = 0.01;
	    store->mbSoundVelocity_minimum = 0;
	    store->mbSoundVelocity_maximum = 32767;
	    store->mbSoundVelocity_valid_minimum = 0;
	    store->mbSoundVelocity_valid_maximum = 32767;
	    store->mbSoundVelocity_missing_value = -32768;
	    strcpy(store->mbSoundVelocity_format_C, "%.2f");
	    strcpy(store->mbSoundVelocity_orientation, "direct");
	    strcpy(store->mbHeading_type, "real");
	    strcpy(store->mbHeading_long_name, "Heading of ship");
	    strcpy(store->mbHeading_name_code, "MB_CYCLE_HEADING");
	    strcpy(store->mbHeading_units, "degree");
	    strcpy(store->mbHeading_unit_code, "MB_DEGREE");
	    store->mbHeading_add_offset = 0.;
	    store->mbHeading_scale_factor = 0.01;
	    store->mbHeading_minimum = 0;
	    store->mbHeading_maximum = 35999;
	    store->mbHeading_valid_minimum = 0;
	    store->mbHeading_valid_maximum = 35999;
	    store->mbHeading_missing_value = 65535;
	    strcpy(store->mbHeading_format_C, "%.2f");
	    strcpy(store->mbHeading_orientation, "direct");
	    strcpy(store->mbRoll_type, "real");
	    strcpy(store->mbRoll_long_name, "Roll");
	    strcpy(store->mbRoll_name_code, "MB_CYCLE_ROLL");
	    strcpy(store->mbRoll_units, "degree");
	    strcpy(store->mbRoll_unit_code, "MB_DEGREE");
	    store->mbRoll_add_offset = 0.;
	    store->mbRoll_scale_factor = 0.01;
	    store->mbRoll_minimum = -2100;
	    store->mbRoll_maximum = 2100;
	    store->mbRoll_valid_minimum = -2100;
	    store->mbRoll_valid_maximum = 2100;
	    store->mbRoll_missing_value = -32768;
	    strcpy(store->mbRoll_format_C, "%.2f");
	    strcpy(store->mbRoll_orientation, "direct");
	    strcpy(store->mbPitch_type, "real");
	    strcpy(store->mbPitch_long_name, "Pitch");
	    strcpy(store->mbPitch_name_code, "MB_CYCLE_PITCH");
	    strcpy(store->mbPitch_units, "degree");
	    strcpy(store->mbPitch_unit_code, "MB_DEGREE");
	    store->mbPitch_add_offset = 0.;
	    store->mbPitch_scale_factor = 0.01;
	    store->mbPitch_minimum = -2100;
	    store->mbPitch_maximum = 2100;
	    store->mbPitch_valid_minimum = -2100;
	    store->mbPitch_valid_maximum = 2100;
	    store->mbPitch_missing_value = -32768;
	    strcpy(store->mbPitch_format_C, "%.2f");
	    strcpy(store->mbPitch_orientation, "direct");
	    strcpy(store->mbTransmissionHeave_type, "real");
	    strcpy(store->mbTransmissionHeave_long_name, "Emission heave");
	    strcpy(store->mbTransmissionHeave_name_code, "MB_CYCLE_HEAVE");
	    strcpy(store->mbTransmissionHeave_units, "m");
	    strcpy(store->mbTransmissionHeave_unit_code, "MB_M");
	    store->mbTransmissionHeave_add_offset = 0.;
	    store->mbTransmissionHeave_scale_factor = 0.01;
	    store->mbTransmissionHeave_minimum = -1000;
	    store->mbTransmissionHeave_maximum = 1000;
	    store->mbTransmissionHeave_valid_minimum = -1000;
	    store->mbTransmissionHeave_valid_maximum = 1000;
	    store->mbTransmissionHeave_missing_value = -32768;
	    strcpy(store->mbTransmissionHeave_format_C, "%.2f");
	    strcpy(store->mbTransmissionHeave_orientation, "direct");
	    strcpy(store->mbDistanceScale_type, "real");
	    strcpy(store->mbDistanceScale_long_name, "Horizontal scale");
	    strcpy(store->mbDistanceScale_name_code, "MB_CYCLE_XY_SCALE");
	    strcpy(store->mbDistanceScale_units, "m");
	    strcpy(store->mbDistanceScale_unit_code, "MB_M");
	    store->mbDistanceScale_add_offset = 0.;
	    store->mbDistanceScale_scale_factor = 0.01;
	    store->mbDistanceScale_minimum = 1;
	    store->mbDistanceScale_maximum = 255;
	    store->mbDistanceScale_valid_minimum = 1;
	    store->mbDistanceScale_valid_maximum = 255;
	    store->mbDistanceScale_missing_value = 0;
	    strcpy(store->mbDistanceScale_format_C, "%.2f");
	    strcpy(store->mbDistanceScale_orientation, "direct");
	    strcpy(store->mbDepthScale_type, "real");
	    strcpy(store->mbDepthScale_long_name, "Depth scale");
	    strcpy(store->mbDepthScale_name_code, "MB_CYCLE_Z_SCALE");
	    strcpy(store->mbDepthScale_units, "m");
	    strcpy(store->mbDepthScale_unit_code, "MB_M");
	    store->mbDepthScale_add_offset = 0.;
	    store->mbDepthScale_scale_factor = 0.01;
	    store->mbDepthScale_minimum = 1;
	    store->mbDepthScale_maximum = 255;
	    store->mbDepthScale_valid_minimum = 1;
	    store->mbDepthScale_valid_maximum = 255;
	    store->mbDepthScale_missing_value = 0;
	    strcpy(store->mbDepthScale_format_C, "%.2f");
	    strcpy(store->mbDepthScale_orientation, "direct");
	    strcpy(store->mbVerticalDepth_type, "integer");
	    strcpy(store->mbVerticalDepth_long_name, "Vertical depth");
	    strcpy(store->mbVerticalDepth_name_code, "MB_CYCLE_DEPTH");
	    strcpy(store->mbVerticalDepth_units, "");
	    strcpy(store->mbVerticalDepth_unit_code, "MB_NOT_DEFINED");
	    store->mbVerticalDepth_add_offset = 0;
	    store->mbVerticalDepth_scale_factor = 1;
	    store->mbVerticalDepth_minimum = 1;
	    store->mbVerticalDepth_maximum = 65535;
	    store->mbVerticalDepth_valid_minimum = 1;
	    store->mbVerticalDepth_valid_maximum = 65535;
	    store->mbVerticalDepth_missing_value = 0;
	    strcpy(store->mbVerticalDepth_format_C, "%d");
	    strcpy(store->mbVerticalDepth_orientation, "direct");
	    strcpy(store->mbCQuality_type, "integer");
	    strcpy(store->mbCQuality_long_name, "Quality factor");
	    strcpy(store->mbCQuality_name_code, "MB_CYCLE_QUALITY");
	    strcpy(store->mbCQuality_units, "");
	    strcpy(store->mbCQuality_unit_code, "MB_NOT_DEFINED");
	    store->mbCQuality_add_offset = 0;
	    store->mbCQuality_scale_factor = 1;
	    store->mbCQuality_minimum = 1;
	    store->mbCQuality_maximum = 255;
	    store->mbCQuality_valid_minimum = 1;
	    store->mbCQuality_valid_maximum = 255;
	    store->mbCQuality_missing_value = 0;
	    strcpy(store->mbCQuality_format_C, "%d");
	    strcpy(store->mbCQuality_orientation, "direct");
	    strcpy(store->mbCFlag_type, "integer");
	    strcpy(store->mbCFlag_long_name, "Flag of cycle");
	    strcpy(store->mbCFlag_name_code, "MB_CYCLE_FLAG");
	    strcpy(store->mbCFlag_units, "");
	    strcpy(store->mbCFlag_unit_code, "MB_NOT_DEFINED");
	    store->mbCFlag_add_offset = 0;
	    store->mbCFlag_scale_factor = 1;
	    store->mbCFlag_minimum = -127;
	    store->mbCFlag_maximum = 127;
	    store->mbCFlag_valid_minimum = -127;
	    store->mbCFlag_valid_maximum = 127;
	    store->mbCFlag_missing_value = -128;
	    strcpy(store->mbCFlag_format_C, "%d");
	    strcpy(store->mbCFlag_orientation, "direct");
	    strcpy(store->mbInterlacing_type, "integer");
	    strcpy(store->mbInterlacing_long_name, "Interlacing 1=Port 2=Starboard");
	    strcpy(store->mbInterlacing_name_code, "MB_CYCLE_INTERLACING");
	    strcpy(store->mbInterlacing_units, "");
	    strcpy(store->mbInterlacing_unit_code, "MB_NOT_DEFINED");
	    store->mbInterlacing_add_offset = 0;
	    store->mbInterlacing_scale_factor = 1;
	    store->mbInterlacing_minimum = 1;
	    store->mbInterlacing_maximum = 2;
	    store->mbInterlacing_valid_minimum = 1;
	    store->mbInterlacing_valid_maximum = 2;
	    store->mbInterlacing_missing_value = 0;
	    strcpy(store->mbInterlacing_format_C, "%d");
	    strcpy(store->mbInterlacing_orientation, "direct");
	    strcpy(store->mbSamplingRate_type, "integer");
	    strcpy(store->mbSamplingRate_long_name, "Sampling Rate");
	    strcpy(store->mbSamplingRate_name_code, "MB_SAMPLING_RATE");
	    strcpy(store->mbSamplingRate_units, "Hz");
	    strcpy(store->mbSamplingRate_unit_code, "MB_HZ");
	    store->mbSamplingRate_add_offset = 0;
	    store->mbSamplingRate_scale_factor = 1;
	    store->mbSamplingRate_minimum = 1;
	    store->mbSamplingRate_maximum = 65535;
	    store->mbSamplingRate_valid_minimum = 1;
	    store->mbSamplingRate_valid_maximum = 65535;
	    store->mbSamplingRate_missing_value = 0;
	    strcpy(store->mbSamplingRate_format_C, "%d");
	    strcpy(store->mbSamplingRate_orientation, "direct");
	    strcpy(store->mbAlongDistance_type, "integer");
	    strcpy(store->mbAlongDistance_long_name, "Along distance");
	    strcpy(store->mbAlongDistance_name_code, "MB_SOUNDING_X");
	    strcpy(store->mbAlongDistance_units, "");
	    strcpy(store->mbAlongDistance_unit_code, "MB_NOT_DEFINED");
	    store->mbAlongDistance_add_offset = 0;
	    store->mbAlongDistance_scale_factor = 1;
	    store->mbAlongDistance_minimum = -32767;
	    store->mbAlongDistance_maximum = 32767;
	    store->mbAlongDistance_valid_minimum = -32767;
	    store->mbAlongDistance_valid_maximum = 32767;
	    store->mbAlongDistance_missing_value = -32768;
	    strcpy(store->mbAlongDistance_format_C, "%d");
	    strcpy(store->mbAlongDistance_orientation, "direct");
	    strcpy(store->mbAcrossDistance_type, "integer");
	    strcpy(store->mbAcrossDistance_long_name, "Across distance");
	    strcpy(store->mbAcrossDistance_name_code, "MB_SOUNDING_Y");
	    strcpy(store->mbAcrossDistance_units, "");
	    strcpy(store->mbAcrossDistance_unit_code, "MB_NOT_DEFINED");
	    store->mbAcrossDistance_add_offset = 0;
	    store->mbAcrossDistance_scale_factor = 1;
	    store->mbAcrossDistance_minimum = -32767;
	    store->mbAcrossDistance_maximum = 32767;
	    store->mbAcrossDistance_valid_minimum = -32767;
	    store->mbAcrossDistance_valid_maximum = 32767;
	    store->mbAcrossDistance_missing_value = -32768;
	    strcpy(store->mbAcrossDistance_format_C, "%d");
	    strcpy(store->mbAcrossDistance_orientation, "direct");
	    strcpy(store->mbDepth_type, "integer");
	    strcpy(store->mbDepth_long_name, "Depth");
	    strcpy(store->mbDepth_name_code, "MB_SOUNDING_Z");
	    strcpy(store->mbDepth_units, "");
	    strcpy(store->mbDepth_unit_code, "MB_NOT_DEFINED");
	    store->mbDepth_add_offset = 0;
	    store->mbDepth_scale_factor = 1;
	    store->mbDepth_minimum = 1;
	    store->mbDepth_maximum = 65535;
	    store->mbDepth_valid_minimum = 1;
	    store->mbDepth_valid_maximum = 65535;
	    store->mbDepth_missing_value = 0;
	    strcpy(store->mbDepth_format_C, "%d");
	    strcpy(store->mbDepth_orientation, "inverse");
	    strcpy(store->mbSQuality_type, "integer");
	    strcpy(store->mbSQuality_long_name, "Quality factor");
	    strcpy(store->mbSQuality_name_code, "MB_SOUNDING_QUALITY");
	    strcpy(store->mbSQuality_units, "");
	    strcpy(store->mbSQuality_unit_code, "MB_NOT_DEFINED");
	    store->mbSQuality_add_offset = 0;
	    store->mbSQuality_scale_factor = 1;
	    store->mbSQuality_minimum = 1;
	    store->mbSQuality_maximum = 255;
	    store->mbSQuality_valid_minimum = 1;
	    store->mbSQuality_valid_maximum = 255;
	    store->mbSQuality_missing_value = 0;
	    strcpy(store->mbSQuality_format_C, "%d");
	    strcpy(store->mbSQuality_orientation, "direct");
	    strcpy(store->mbSFlag_type, "integer");
	    strcpy(store->mbSFlag_long_name, "Flag of sounding");
	    strcpy(store->mbSFlag_name_code, "MB_SOUNDING_FLAG");
	    strcpy(store->mbSFlag_units, "");
	    strcpy(store->mbSFlag_unit_code, "MB_NOT_DEFINED");
	    store->mbSFlag_add_offset = 0;
	    store->mbSFlag_scale_factor = 1;
	    store->mbSFlag_minimum = -127;
	    store->mbSFlag_maximum = 127;
	    store->mbSFlag_valid_minimum = -127;
	    store->mbSFlag_valid_maximum = 127;
	    store->mbSFlag_missing_value = -128;
	    strcpy(store->mbSFlag_format_C, "%d");
	    strcpy(store->mbSFlag_orientation, "direct");
	    strcpy(store->mbAntenna_type, "integer");
	    strcpy(store->mbAntenna_long_name, "Antenna index");
	    strcpy(store->mbAntenna_name_code, "MB_BEAM_ANTENNA");
	    strcpy(store->mbAntenna_units, "");
	    strcpy(store->mbAntenna_unit_code, "MB_NOT_DEFINED");
	    store->mbAntenna_add_offset = 0;
	    store->mbAntenna_scale_factor = 1;
	    store->mbAntenna_minimum = 0;
	    store->mbAntenna_maximum = 127;
	    store->mbAntenna_valid_minimum = 0;
	    store->mbAntenna_valid_maximum = 127;
	    store->mbAntenna_missing_value = -128;
	    strcpy(store->mbAntenna_format_C, "%d");
	    strcpy(store->mbAntenna_orientation, "direct");
	    strcpy(store->mbBeamBias_type, "real");
	    strcpy(store->mbBeamBias_long_name, "Beam bias");
	    strcpy(store->mbBeamBias_name_code, "MB_BEAM_BIAS");
	    strcpy(store->mbBeamBias_units, "m");
	    strcpy(store->mbBeamBias_unit_code, "MB_M");
	    store->mbBeamBias_add_offset = 0.;
	    store->mbBeamBias_scale_factor = 0.1;
	    store->mbBeamBias_minimum = -32767;
	    store->mbBeamBias_maximum = 32767;
	    store->mbBeamBias_valid_minimum = -32767;
	    store->mbBeamBias_valid_maximum = 32767;
	    store->mbBeamBias_missing_value = -32768;
	    strcpy(store->mbBeamBias_format_C, "%d");
	    strcpy(store->mbBeamBias_orientation, "direct");
	    strcpy(store->mbBFlag_type, "integer");
	    strcpy(store->mbBFlag_long_name, "Flag of beam");
	    strcpy(store->mbBFlag_name_code, "MB_BEAM_FLAG");
	    strcpy(store->mbBFlag_units, "");
	    strcpy(store->mbBFlag_unit_code, "MB_NOT_DEFINED");
	    store->mbBFlag_add_offset = 0;
	    store->mbBFlag_scale_factor = 1;
	    store->mbBFlag_minimum = -127;
	    store->mbBFlag_maximum = 127;
	    store->mbBFlag_valid_minimum = -127;
	    store->mbBFlag_valid_maximum = 127;
	    store->mbBFlag_missing_value = -128;
	    strcpy(store->mbBFlag_format_C, "%d");
	    strcpy(store->mbBFlag_orientation, "direct");
	    strcpy(store->mbBeam_type, "integer");
	    strcpy(store->mbBeam_long_name, "Number of beams");
	    strcpy(store->mbBeam_name_code, "MB_ANTENNA_BEAM");
	    strcpy(store->mbBeam_units, "");
	    strcpy(store->mbBeam_unit_code, "MB_NOT_DEFINED");
	    store->mbBeam_add_offset = 0;
	    store->mbBeam_scale_factor = 1;
	    store->mbBeam_minimum = 1;
	    store->mbBeam_maximum = 65535;
	    store->mbBeam_valid_minimum = 1;
	    store->mbBeam_valid_maximum = 65535;
	    store->mbBeam_missing_value = 0;
	    strcpy(store->mbBeam_format_C, "%d");
	    strcpy(store->mbBeam_orientation, "direct");
	    strcpy(store->mbAFlag_type, "integer");
	    strcpy(store->mbAFlag_long_name, "Flag of antenna");
	    strcpy(store->mbAFlag_name_code, "MB_ANTENNA_FLAG");
	    strcpy(store->mbAFlag_units, "");
	    strcpy(store->mbAFlag_unit_code, "MB_NOT_DEFINED");
	    store->mbAFlag_add_offset = 0;
	    store->mbAFlag_scale_factor = 1;
	    store->mbAFlag_minimum = -127;
	    store->mbAFlag_maximum = 127;
	    store->mbAFlag_valid_minimum = -127;
	    store->mbAFlag_valid_maximum = 127;
	    store->mbAFlag_missing_value = -128;
	    strcpy(store->mbAFlag_format_C, "%d");
	    strcpy(store->mbAFlag_orientation, "direct");
	    strcpy(store->mbVelProfilRef_type, "string");
	    strcpy(store->mbVelProfilRef_long_name, "Reference to a velocity file");
	    strcpy(store->mbVelProfilRef_name_code, "MB_PROFIL_REF");
	    strcpy(store->mbVelProfilIdx_type, "integer");
	    strcpy(store->mbVelProfilIdx_long_name, "Index of the sound velocity profil");
	    strcpy(store->mbVelProfilIdx_name_code, "MB_PROFIL_IDX");
	    strcpy(store->mbVelProfilIdx_units, "");
	    strcpy(store->mbVelProfilIdx_unit_code, "MB_NOT_DEFINED");
	    store->mbVelProfilIdx_add_offset = 0;
	    store->mbVelProfilIdx_scale_factor = 1;
	    store->mbVelProfilIdx_minimum = 0;
	    store->mbVelProfilIdx_maximum = 32767;
	    store->mbVelProfilIdx_valid_minimum = 0;
	    store->mbVelProfilIdx_valid_maximum = 32767;
	    store->mbVelProfilIdx_missing_value = -32768;
	    strcpy(store->mbVelProfilIdx_format_C, "%d");
	    strcpy(store->mbVelProfilIdx_orientation, "direct");
	    strcpy(store->mbVelProfilDate_type, "integer");
	    strcpy(store->mbVelProfilDate_long_name, "Date of cycle");
	    strcpy(store->mbVelProfilDate_name_code, "MB_PROFIL_DATE");
	    strcpy(store->mbVelProfilDate_units, "Julian_date");
	    strcpy(store->mbVelProfilDate_unit_code, "MB_JULIAN_DATE");
	    store->mbVelProfilDate_add_offset = 2440588;
	    store->mbVelProfilDate_scale_factor = 1;
	    store->mbVelProfilDate_minimum = -25567;
	    store->mbVelProfilDate_maximum = 47482;
	    store->mbVelProfilDate_valid_minimum = -25567;
	    store->mbVelProfilDate_valid_maximum = 47482;
	    store->mbVelProfilDate_missing_value = 2147483647;
	    strcpy(store->mbVelProfilDate_format_C, "%d");
	    strcpy(store->mbVelProfilDate_orientation, "direct");
	    strcpy(store->mbVelProfilTime_type, "integer");
	    strcpy(store->mbVelProfilTime_long_name, "Time of cycle");
	    strcpy(store->mbVelProfilTime_name_code, "MB_PROFIL_TIME");
	    strcpy(store->mbVelProfilTime_units, "ms");
	    strcpy(store->mbVelProfilTime_unit_code, "MB_MS");
	    store->mbVelProfilTime_add_offset = 0;
	    store->mbVelProfilTime_scale_factor = 1;
	    store->mbVelProfilTime_minimum = 0;
	    store->mbVelProfilTime_maximum = 86399999;
	    store->mbVelProfilTime_valid_minimum = 0;
	    store->mbVelProfilTime_valid_maximum = 86399999;
	    store->mbVelProfilTime_missing_value = -2147483648;
	    strcpy(store->mbVelProfilTime_format_C, "%d");
	    strcpy(store->mbVelProfilTime_orientation, "direct");
    
	    /* variable ids */
	    store->mbHistDate_id = -1;
	    store->mbHistTime_id = -1;
	    store->mbHistCode_id = -1;
	    store->mbHistAutor_id = -1;
	    store->mbHistModule_id = -1;
	    store->mbHistComment_id = -1;
	    store->mbCycle_id = -1;
	    store->mbDate_id = -1;
	    store->mbTime_id = -1;
	    store->mbOrdinate_id = -1;
	    store->mbAbscissa_id = -1;
	    store->mbFrequency_id = -1;
	    store->mbSounderMode_id = -1;
	    store->mbReferenceDepth_id = -1;
	    store->mbDynamicDraught_id = -1;
	    store->mbTide_id = -1;
	    store->mbSoundVelocity_id = -1;
	    store->mbHeading_id = -1;
	    store->mbRoll_id = -1;
	    store->mbPitch_id = -1;
	    store->mbTransmissionHeave_id = -1;
	    store->mbDistanceScale_id = -1;
	    store->mbDepthScale_id = -1;
	    store->mbVerticalDepth_id = -1;
	    store->mbCQuality_id = -1;
	    store->mbCFlag_id = -1;
	    store->mbInterlacing_id = -1;
	    store->mbSamplingRate_id = -1;
	    store->mbAlongDistance_id = -1;
	    store->mbAcrossDistance_id = -1;
	    store->mbDepth_id = -1;
	    store->mbSQuality_id = -1;
	    store->mbSFlag_id = -1;
	    store->mbAntenna_id = -1;
	    store->mbBeamBias_id = -1;
	    store->mbBFlag_id = -1;
	    store->mbBeam_id = -1;
	    store->mbAFlag_id = -1;
	    store->mbVelProfilRef_id = -1;
	    store->mbVelProfilIdx_id = -1;
	    store->mbVelProfilDate_id = -1;
	    store->mbVelProfilTime_id = -1;
    
	    /* variable pointers */
	    store->mbHistDate = NULL;
	    store->mbHistTime = NULL;
	    store->mbHistCode = NULL;
	    store->mbHistAutor = NULL;
	    store->mbHistModule = NULL;
	    store->mbHistComment = NULL;
	    store->mbCycle = NULL;
	    store->mbDate = NULL;
	    store->mbTime = NULL;
	    store->mbOrdinate = NULL;
	    store->mbAbscissa = NULL;
	    store->mbFrequency = NULL;
	    store->mbSounderMode = NULL;
	    store->mbReferenceDepth = NULL;
	    store->mbDynamicDraught = NULL;
	    store->mbTide = NULL;
	    store->mbSoundVelocity = NULL;
	    store->mbHeading = NULL;
	    store->mbRoll = NULL;
	    store->mbPitch = NULL;
	    store->mbTransmissionHeave = NULL;
	    store->mbDistanceScale = NULL;
	    store->mbDepthScale = NULL;
	    store->mbVerticalDepth = NULL;
	    store->mbCQuality = NULL;
	    store->mbCFlag = NULL;
	    store->mbInterlacing = NULL;
	    store->mbSamplingRate = NULL;
	    store->mbAlongDistance = NULL;
	    store->mbAcrossDistance = NULL;
	    store->mbDepth = NULL;
	    store->mbSQuality = NULL;
	    store->mbSFlag = NULL;
	    store->mbAntenna = NULL;
	    store->mbBeamBias = NULL;
	    store->mbBFlag = NULL;
	    store->mbBeam = NULL;
	    store->mbAFlag = NULL;
	    store->mbVelProfilRef = NULL;
	    store->mbVelProfilIdx = NULL;
	    store->mbVelProfilDate = NULL;
	    store->mbVelProfilTime = NULL;
    
	    /* go ahead and allocate history arrays */
/*	    status = mb_malloc(verbose, 
			store->mbHistoryRecNbr * sizeof(int),
			&store->mbHistDate,error);
	    if (status == MB_SUCCESS)
	    status = mb_malloc(verbose, 
			store->mbHistoryRecNbr * sizeof(int),
			&store->mbHistTime,error);
	    if (status == MB_SUCCESS)
	    status = mb_malloc(verbose, 
			store->mbHistoryRecNbr * sizeof(char),
			&store->mbHistCode,error);
	    if (status == MB_SUCCESS)
	    status = mb_malloc(verbose, 
			store->mbHistoryRecNbr * store->mbNameLength * sizeof(char),
			&store->mbHistAutor,error);
	    if (status == MB_SUCCESS)
	    status = mb_malloc(verbose, 
			store->mbHistoryRecNbr * store->mbNameLength * sizeof(char),
			&store->mbHistModule,error);
	    if (status == MB_SUCCESS)
	    status = mb_malloc(verbose, 
			store->mbHistoryRecNbr * store->mbCommentLength * sizeof(char),
			&store->mbHistComment,error);
*/
	    }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",
			function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       store_ptr:  %d\n",*store_ptr);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_netcdf_deall(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error)
{
	char	*function_name = "mbsys_netcdf_deall";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_netcdf_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",*store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_netcdf_struct *) *store_ptr;

	/* deallocate any allocated arrays */
	if (store->mbHistDate != NULL)
	    status = mb_free(verbose, &store->mbHistDate, error);
	if (store->mbHistTime != NULL)
	    status = mb_free(verbose, &store->mbHistTime, error);
	if (store->mbHistCode != NULL)
	    status = mb_free(verbose, &store->mbHistCode, error);
	if (store->mbHistAutor != NULL)
	    status = mb_free(verbose, &store->mbHistAutor, error);
	if (store->mbHistModule != NULL)
	    status = mb_free(verbose, &store->mbHistModule, error);
	if (store->mbHistComment != NULL)
	    status = mb_free(verbose, &store->mbHistComment, error);
	if (store->mbCycle != NULL)
	    status = mb_free(verbose, &store->mbCycle, error);
	if (store->mbDate != NULL)
	    status = mb_free(verbose, &store->mbDate, error);
	if (store->mbTime != NULL)
	    status = mb_free(verbose, &store->mbTime, error);
	if (store->mbOrdinate != NULL)
	    status = mb_free(verbose, &store->mbOrdinate, error);
	if (store->mbAbscissa != NULL)
	    status = mb_free(verbose, &store->mbAbscissa, error);
	if (store->mbFrequency != NULL)
	    status = mb_free(verbose, &store->mbFrequency, error);
	if (store->mbSounderMode != NULL)
	    status = mb_free(verbose, &store->mbSounderMode, error);
	if (store->mbReferenceDepth != NULL)
	    status = mb_free(verbose, &store->mbReferenceDepth, error);
	if (store->mbDynamicDraught != NULL)
	    status = mb_free(verbose, &store->mbDynamicDraught, error);
	if (store->mbTide != NULL)
	    status = mb_free(verbose, &store->mbTide, error);
	if (store->mbSoundVelocity != NULL)
	    status = mb_free(verbose, &store->mbSoundVelocity, error);
	if (store->mbHeading != NULL)
	    status = mb_free(verbose, &store->mbHeading, error);
	if (store->mbRoll != NULL)
	    status = mb_free(verbose, &store->mbRoll, error);
	if (store->mbPitch != NULL)
	    status = mb_free(verbose, &store->mbPitch, error);
	if (store->mbTransmissionHeave != NULL)
	    status = mb_free(verbose, &store->mbTransmissionHeave, error);
	if (store->mbDistanceScale != NULL)
	    status = mb_free(verbose, &store->mbDistanceScale, error);
	if (store->mbDepthScale != NULL)
	    status = mb_free(verbose, &store->mbDepthScale, error);
	if (store->mbVerticalDepth != NULL)
	    status = mb_free(verbose, &store->mbVerticalDepth, error);
	if (store->mbCQuality != NULL)
	    status = mb_free(verbose, &store->mbCQuality, error);
	if (store->mbCFlag != NULL)
	    status = mb_free(verbose, &store->mbCFlag, error);
	if (store->mbInterlacing != NULL)
	    status = mb_free(verbose, &store->mbInterlacing, error);
	if (store->mbSamplingRate != NULL)
	    status = mb_free(verbose, &store->mbSamplingRate, error);
	if (store->mbAlongDistance != NULL)
	    status = mb_free(verbose, &store->mbAlongDistance, error);
	if (store->mbAcrossDistance != NULL)
	    status = mb_free(verbose, &store->mbAcrossDistance, error);
	if (store->mbDepth != NULL)
	    status = mb_free(verbose, &store->mbDepth, error);
	if (store->mbSQuality != NULL)
	    status = mb_free(verbose, &store->mbSQuality, error);
	if (store->mbSFlag != NULL)
	    status = mb_free(verbose, &store->mbSFlag, error);
	if (store->mbAntenna != NULL)
	    status = mb_free(verbose, &store->mbAntenna, error);
	if (store->mbBeamBias != NULL)
	    status = mb_free(verbose, &store->mbBeamBias, error);
	if (store->mbBFlag != NULL)
	    status = mb_free(verbose, &store->mbBFlag, error);
	if (store->mbBeam != NULL)
	    status = mb_free(verbose, &store->mbBeam, error);
	if (store->mbAFlag != NULL)
	    status = mb_free(verbose, &store->mbAFlag, error);
	if (store->mbVelProfilRef != NULL)
	    status = mb_free(verbose, &store->mbVelProfilRef, error);
	if (store->mbVelProfilIdx != NULL)
	    status = mb_free(verbose, &store->mbVelProfilIdx, error);
	if (store->mbVelProfilDate != NULL)
	    status = mb_free(verbose, &store->mbVelProfilDate, error);
	if (store->mbVelProfilTime != NULL)
	    status = mb_free(verbose, &store->mbVelProfilTime, error);

	/* deallocate memory for data structure */
	status = mb_free(verbose,store_ptr,error);

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
int mbsys_netcdf_extract(int verbose, void *mbio_ptr, void *store_ptr, 
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_netcdf_extract";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_netcdf_struct *store;
	double	depthscale, distancescale;
	int	i;
	
	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_netcdf_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;
	
	/* reset error and status */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get time */
		*time_d = store->mbDate[0] * SECINDAY
			    + store->mbTime[0] * 0.001;
		mb_get_date(verbose,*time_d,time_i);

		/* get navigation */
		*navlon = (double) store->mbAbscissa_scale_factor * store->mbAbscissa[0];
		*navlat = (double) store->mbOrdinate_scale_factor * store->mbOrdinate[0];
		if (mb_io_ptr->lonflip < 0)
			{
			if (*navlon > 0.) 
				*navlon = *navlon - 360.;
			else if (*navlon < -360.)
				*navlon = *navlon + 360.;
			}
		else if (mb_io_ptr->lonflip == 0)
			{
			if (*navlon > 180.) 
				*navlon = *navlon - 360.;
			else if (*navlon < -180.)
				*navlon = *navlon + 360.;
			}
		else
			{
			if (*navlon > 360.) 
				*navlon = *navlon - 360.;
			else if (*navlon < 0.)
				*navlon = *navlon + 360.;
			}

		/* get heading */
		*heading = store->mbHeading[0] * store->mbHeading_scale_factor;

		/* set speed to zero */
		*speed = 0.0;
			
		/* set beamwidths in mb_io structure */
		mb_io_ptr->beamwidth_ltrack = 2.0;
		mb_io_ptr->beamwidth_xtrack = 2.0;

		/* read distance, depth, and backscatter 
			values into storage arrays */
		*nbath = store->mbBeamNbr;
		*namp = 0;
		*nss = 0;
		depthscale = store->mbDepthScale[0] * store->mbDepthScale_scale_factor;
		distancescale = store->mbDistanceScale[0] * store->mbDistanceScale_scale_factor;
		for (i=0;i<*nbath;i++)
			{
			if (store->mbSFlag[i] == 0)
			    beamflag[i] = MB_FLAG_NULL;
			else if (store->mbSFlag[i] == 2)
			    beamflag[i] = MB_FLAG_NONE;
			else
			    beamflag[i] = MB_FLAG_FLAG;
			bath[i] = depthscale * store->mbDepth[i];
			bathacrosstrack[i] = distancescale * store->mbAcrossDistance[i];
			bathalongtrack[i] = distancescale * store->mbAlongDistance[i];
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
		strcpy(comment, store->comment);

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  New ping read by MBIO function <%s>\n",
				function_name);
			fprintf(stderr,"dbg4  New ping values:\n");
			fprintf(stderr,"dbg4       error:      %d\n",
				error);
			fprintf(stderr,"dbg4       comment:    %s\n",
				comment);
			}
		}

	/* else set error */
	else
		{
		/* set error */
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
int mbsys_netcdf_insert(int verbose, void *mbio_ptr, void *store_ptr, 
		int kind, int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading,
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_netcdf_insert";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_netcdf_struct *store;
	double	depthscale, distancescale;
	double	depthmax, distancemax;
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
		fprintf(stderr,"dbg2       kind:       %d\n",kind);
		}
	if (verbose >= 2 && (kind == MB_DATA_DATA))
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
	store = (struct mbsys_netcdf_struct *) store_ptr;

	/* set data kind */
	store->kind = kind;
	
	/* allocate memory if needed */
	if (store->kind == MB_DATA_DATA
	    && nbath > 0
	    && store->mbAntennaNbr <= 0
	    && store->mbBeamNbr <= 0)
	    {
	    /* set sonar system */
	    if (nbath  == MBSYS_NETCDF_SONAR_BEAMS_SEABEAM)
		{
		store->mbSounder = MBSYS_NETCDF_SONAR_SEABEAM;
		store->mbBeamNbr = MBSYS_NETCDF_SONAR_BEAMS_SEABEAM;
		store->mbAntennaNbr = 1;
		}
	    else if (nbath  == MBSYS_NETCDF_SONAR_BEAMS_ECHOSXD15)
		{
		store->mbSounder = MBSYS_NETCDF_SONAR_ECHOSXD15;
		store->mbBeamNbr = MBSYS_NETCDF_SONAR_BEAMS_ECHOSXD15;
		store->mbAntennaNbr = 1;
		}
	    else if (nbath  == MBSYS_NETCDF_SONAR_BEAMS_ECHOSXD60)
		{
		store->mbSounder = MBSYS_NETCDF_SONAR_ECHOSXD60;
		store->mbBeamNbr = MBSYS_NETCDF_SONAR_BEAMS_ECHOSXD60;
		store->mbAntennaNbr = 1;
		}
	    else if (nbath  == MBSYS_NETCDF_SONAR_BEAMS_HSDS)
		{
		store->mbSounder = MBSYS_NETCDF_SONAR_HSDS;
		store->mbBeamNbr = MBSYS_NETCDF_SONAR_BEAMS_HSDS;
		store->mbAntennaNbr = 1;
		}
	    else if (nbath  == MBSYS_NETCDF_SONAR_BEAMS_LENNEMOR)
		{
		store->mbSounder = MBSYS_NETCDF_SONAR_LENNEMOR;
		store->mbBeamNbr = MBSYS_NETCDF_SONAR_BEAMS_LENNEMOR;
		store->mbAntennaNbr = 1;
		}
	    else if (nbath  == MBSYS_NETCDF_SONAR_BEAMS_TMS5265B)
		{
		store->mbSounder = MBSYS_NETCDF_SONAR_TMS5265B;
		store->mbBeamNbr = MBSYS_NETCDF_SONAR_BEAMS_TMS5265B;
		store->mbAntennaNbr = 1;
		}
	    else if (nbath  == MBSYS_NETCDF_SONAR_BEAMS_EM100)
		{
		store->mbSounder = MBSYS_NETCDF_SONAR_EM100;
		store->mbBeamNbr = MBSYS_NETCDF_SONAR_BEAMS_EM100;
		store->mbAntennaNbr = 1;
		}
	    else if (nbath  == MBSYS_NETCDF_SONAR_BEAMS_EM1000)
		{
		store->mbSounder = MBSYS_NETCDF_SONAR_EM1000;
		store->mbBeamNbr = MBSYS_NETCDF_SONAR_BEAMS_EM1000;
		store->mbAntennaNbr = 1;
		}
	    else if (nbath  == MBSYS_NETCDF_SONAR_BEAMS_EM12S)
		{
		store->mbSounder = MBSYS_NETCDF_SONAR_EM12S;
		store->mbBeamNbr = MBSYS_NETCDF_SONAR_BEAMS_EM12S;
		store->mbAntennaNbr = 1;
		}
	    else if (nbath  == MBSYS_NETCDF_SONAR_BEAMS_EM12D)
		{
		store->mbSounder = MBSYS_NETCDF_SONAR_EM12D;
		store->mbBeamNbr = MBSYS_NETCDF_SONAR_BEAMS_EM12D;
		store->mbAntennaNbr = 2;
		}
	    else if (nbath  == MBSYS_NETCDF_SONAR_BEAMS_EM3000S)
		{
		store->mbSounder = MBSYS_NETCDF_SONAR_EM3000S;
		store->mbBeamNbr = MBSYS_NETCDF_SONAR_BEAMS_EM3000S;
		store->mbAntennaNbr = 1;
		}
	    else if (nbath  == MBSYS_NETCDF_SONAR_BEAMS_EM3000D)
		{
		store->mbSounder = MBSYS_NETCDF_SONAR_EM3000D;
		store->mbBeamNbr = MBSYS_NETCDF_SONAR_BEAMS_EM3000D;
		store->mbAntennaNbr = 2;
		}
	    else if (nbath  == MBSYS_NETCDF_SONAR_BEAMS_EM300)
		{
		store->mbSounder = MBSYS_NETCDF_SONAR_EM300;
		store->mbBeamNbr = MBSYS_NETCDF_SONAR_BEAMS_EM300;
		store->mbAntennaNbr = 1;
		}
	    else if (nbath  == MBSYS_NETCDF_SONAR_BEAMS_FURUNO)
		{
		store->mbSounder = MBSYS_NETCDF_SONAR_FURUNO;
		store->mbBeamNbr = MBSYS_NETCDF_SONAR_BEAMS_FURUNO;
		store->mbAntennaNbr = 1;
		}
	    else if (nbath > 0)
		{
		store->mbSounder = MBSYS_NETCDF_SONAR_UNKNOWN;
		store->mbBeamNbr = nbath;
		store->mbAntennaNbr = 1;
		}
		
	    /* allocate arrays */
	    status = mb_malloc(verbose, 
			store->mbAntennaNbr * sizeof(short),
			&store->mbCycle,error);
	    status = mb_malloc(verbose, 
			store->mbAntennaNbr * sizeof(int),
			&store->mbDate,error);
	    status = mb_malloc(verbose, 
			store->mbAntennaNbr * sizeof(int),
			&store->mbTime,error);
	    status = mb_malloc(verbose, 
			store->mbAntennaNbr * sizeof(int),
			&store->mbOrdinate,error);
	    status = mb_malloc(verbose, 
			store->mbAntennaNbr * sizeof(int),
			&store->mbAbscissa,error);
	    status = mb_malloc(verbose, 
			store->mbAntennaNbr * sizeof(char),
			&store->mbFrequency,error);
	    status = mb_malloc(verbose, 
			store->mbAntennaNbr * sizeof(char),
			&store->mbSounderMode,error);
	    status = mb_malloc(verbose, 
			store->mbAntennaNbr * sizeof(short),
			&store->mbReferenceDepth,error);
	    status = mb_malloc(verbose, 
			store->mbAntennaNbr * sizeof(short),
			&store->mbDynamicDraught,error);
	    status = mb_malloc(verbose, 
			store->mbAntennaNbr * sizeof(short),
			&store->mbTide,error);
	    status = mb_malloc(verbose, 
			store->mbAntennaNbr * sizeof(short),
			&store->mbSoundVelocity,error);
	    status = mb_malloc(verbose, 
			store->mbAntennaNbr * sizeof(short),
			&store->mbHeading,error);
	    status = mb_malloc(verbose, 
			store->mbAntennaNbr * sizeof(short),
			&store->mbRoll,error);
	    status = mb_malloc(verbose, 
			store->mbAntennaNbr * sizeof(short),
			&store->mbPitch,error);
	    status = mb_malloc(verbose, 
			store->mbAntennaNbr * sizeof(short),
			&store->mbTransmissionHeave,error);
	    status = mb_malloc(verbose, 
			store->mbAntennaNbr * sizeof(char),
			&store->mbDistanceScale,error);
	    status = mb_malloc(verbose, 
			store->mbAntennaNbr * sizeof(char),
			&store->mbDepthScale,error);
	    status = mb_malloc(verbose, 
			store->mbAntennaNbr * sizeof(short),
			&store->mbVerticalDepth,error);
	    status = mb_malloc(verbose, 
			store->mbAntennaNbr * sizeof(char),
			&store->mbCQuality,error);
	    status = mb_malloc(verbose, 
			store->mbAntennaNbr * sizeof(char),
			&store->mbCFlag,error);
	    status = mb_malloc(verbose, 
			store->mbAntennaNbr * sizeof(char),
			&store->mbInterlacing,error);
	    status = mb_malloc(verbose, 
			store->mbAntennaNbr * sizeof(short),
			&store->mbSamplingRate,error);
	    status = mb_malloc(verbose, 
			store->mbBeamNbr * sizeof(short),
			&store->mbAlongDistance,error);
	    status = mb_malloc(verbose, 
			store->mbBeamNbr * sizeof(short),
			&store->mbAcrossDistance,error);
	    status = mb_malloc(verbose, 
			store->mbBeamNbr * sizeof(short),
			&store->mbDepth,error);
	    status = mb_malloc(verbose, 
			store->mbBeamNbr * sizeof(char),
			&store->mbSQuality,error);
	    status = mb_malloc(verbose, 
			store->mbBeamNbr * sizeof(char),
			&store->mbSFlag,error);
	    status = mb_malloc(verbose, 
			store->mbBeamNbr * sizeof(char),
			&store->mbAntenna,error);
	    status = mb_malloc(verbose, 
			store->mbBeamNbr * sizeof(short),
			&store->mbBeamBias,error);
	    status = mb_malloc(verbose, 
			store->mbBeamNbr * sizeof(char),
			&store->mbBFlag,error);
	    status = mb_malloc(verbose, 
			store->mbAntennaNbr * sizeof(short),
			&store->mbBeam,error);
	    status = mb_malloc(verbose, 
			store->mbAntennaNbr * sizeof(char),
			&store->mbAFlag,error);
	    status = mb_malloc(verbose, 
			store->mbVelocityProfilNbr * store->mbCommentLength * sizeof(char),
			&store->mbVelProfilRef,error);
	    status = mb_malloc(verbose, 
			store->mbVelocityProfilNbr * sizeof(short),
			&store->mbVelProfilIdx,error);
	    status = mb_malloc(verbose, 
			store->mbVelocityProfilNbr * sizeof(int),
			&store->mbVelProfilDate,error);
	    status = mb_malloc(verbose, 
			store->mbVelocityProfilNbr * sizeof(int),
			&store->mbVelProfilTime,error);

	    /* deal with a memory allocation failure */
	    if (status == MB_FAILURE)
		{
		status = mb_free(verbose, &store->mbCycle, error);
		status = mb_free(verbose, &store->mbDate, error);
		status = mb_free(verbose, &store->mbTime, error);
		status = mb_free(verbose, &store->mbOrdinate, error);
		status = mb_free(verbose, &store->mbAbscissa, error);
		status = mb_free(verbose, &store->mbFrequency, error);
		status = mb_free(verbose, &store->mbSounderMode, error);
		status = mb_free(verbose, &store->mbReferenceDepth, error);
		status = mb_free(verbose, &store->mbDynamicDraught, error);
		status = mb_free(verbose, &store->mbTide, error);
		status = mb_free(verbose, &store->mbSoundVelocity, error);
		status = mb_free(verbose, &store->mbHeading, error);
		status = mb_free(verbose, &store->mbRoll, error);
		status = mb_free(verbose, &store->mbPitch, error);
		status = mb_free(verbose, &store->mbTransmissionHeave, error);
		status = mb_free(verbose, &store->mbDistanceScale, error);
		status = mb_free(verbose, &store->mbDepthScale, error);
		status = mb_free(verbose, &store->mbVerticalDepth, error);
		status = mb_free(verbose, &store->mbCQuality, error);
		status = mb_free(verbose, &store->mbCFlag, error);
		status = mb_free(verbose, &store->mbInterlacing, error);
		status = mb_free(verbose, &store->mbSamplingRate, error);
		status = mb_free(verbose, &store->mbAlongDistance, error);
		status = mb_free(verbose, &store->mbAcrossDistance, error);
		status = mb_free(verbose, &store->mbDepth, error);
		status = mb_free(verbose, &store->mbSQuality, error);
		status = mb_free(verbose, &store->mbSFlag, error);
		status = mb_free(verbose, &store->mbAntenna, error);
		status = mb_free(verbose, &store->mbBeamBias, error);
		status = mb_free(verbose, &store->mbBFlag, error);
		status = mb_free(verbose, &store->mbBeam, error);
		status = mb_free(verbose, &store->mbAFlag, error);
		status = mb_free(verbose, &store->mbVelProfilRef, error);
		status = mb_free(verbose, &store->mbVelProfilIdx, error);
		status = mb_free(verbose, &store->mbVelProfilDate, error);
		status = mb_free(verbose, &store->mbVelProfilTime, error);
		status = MB_FAILURE;
		*error = MB_ERROR_MEMORY_FAIL;
		if (verbose >= 2)
			{
			fprintf(stderr,"\ndbg2  MBIO function <%s> terminated with error\n",
				function_name);
			fprintf(stderr,"dbg2  Return values:\n");
			fprintf(stderr,"dbg2       error:      %d\n",*error);
			fprintf(stderr,"dbg2  Return status:\n");
			fprintf(stderr,"dbg2       status:  %d\n",status);
			}
		return(status);
		}
	    }
	
	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* reset lon and lat attributes */
		if (strcmp(store->mbOrdinate_name_code, "MB_POSITION_LATITUDE") != 0)
		    {
		    strcpy(store->mbOrdinate_type, "real");
		    strcpy(store->mbOrdinate_long_name, "Latitude");
		    strcpy(store->mbOrdinate_name_code, "MB_POSITION_LATITUDE");
		    strcpy(store->mbOrdinate_units, "degree");
		    strcpy(store->mbOrdinate_unit_code, "MB_DEGREE");
		    store->mbOrdinate_add_offset = 0.;
		    store->mbOrdinate_scale_factor = 5.e-08;
		    store->mbOrdinate_minimum = -1800000000;
		    store->mbOrdinate_maximum = 1800000000;
		    store->mbOrdinate_valid_minimum = -1800000000;
		    store->mbOrdinate_valid_maximum = 1800000000;
		    store->mbOrdinate_missing_value = -2147483648;
		    strcpy(store->mbOrdinate_format_C, "%f");
		    strcpy(store->mbOrdinate_orientation, "direct");
		    }
		if (strcmp(store->mbAbscissa_name_code, "MB_POSITION_LONGITUDE") != 0)
		    {
		    strcpy(store->mbAbscissa_type, "real");
		    strcpy(store->mbAbscissa_long_name, "Longitude");
		    strcpy(store->mbAbscissa_name_code, "MB_POSITION_LONGITUDE");
		    strcpy(store->mbAbscissa_units, "degree");
		    strcpy(store->mbAbscissa_unit_code, "MB_DEGREE");
		    store->mbAbscissa_add_offset = 0.;
		    store->mbAbscissa_scale_factor = 1.e-07;
		    store->mbAbscissa_minimum = -1800000000;
		    store->mbAbscissa_maximum = 1800000000;
		    store->mbAbscissa_valid_minimum = -1800000000;
		    store->mbAbscissa_valid_maximum = 1800000000;
		    store->mbAbscissa_missing_value = -2147483648;
		    strcpy(store->mbAbscissa_format_C, "%f");
		    strcpy(store->mbAbscissa_orientation, "direct");
		    }

		/* get stuff */
		for (i=0;i<store->mbAntennaNbr;i++)
		    {
		    /* get time */
		    store->mbDate[i] = (int)(time_d / SECINDAY);
		    store->mbTime[i] = (int)(1000 * (time_d - store->mbDate[0] * SECINDAY));
    
		    /* get navigation */
		    store->mbAbscissa[i] = (int)(navlon / store->mbAbscissa_scale_factor);
		    store->mbOrdinate[i] = (int)(navlat / store->mbOrdinate_scale_factor);
    
		    /* get heading */
		    store->mbHeading[i] = heading / store->mbHeading_scale_factor;
		    }
		    
		/* get depth and distance scales */
		depthmax = 0.0;
		distancemax = 0.0;
		for (i=0;i<nbath;i++)
		    {
		    if (beamflag[i] != MB_FLAG_NULL)
			{
			depthmax = MAX(fabs(bath[i]), depthmax);
			distancemax = MAX(fabs(bathacrosstrack[i]), distancemax);
			}
		    }
		depthscale = 2.1 * depthmax / ((double)store->mbDepth_valid_maximum);
		distancescale = 2.1 * distancemax / store->mbAcrossDistance_valid_maximum;

		/* put distance, depth, and backscatter values 
			into data structure */
		store->mbBeamNbr = nbath;
		/* if (store->mbDepthScale[0] <= 0
		    || (depthmax */
		for (i=0;i<store->mbAntennaNbr;i++)
		    {
		    store->mbDepthScale[i] = 1 + (int)(depthscale / store->mbDepthScale_scale_factor);
		    store->mbDistanceScale[i] = 1 + (int)(distancescale / store->mbDistanceScale_scale_factor);
		    }
		depthscale = store->mbDepthScale[0] * store->mbDepthScale_scale_factor;
		distancescale = store->mbDistanceScale[0] * store->mbDistanceScale_scale_factor;
		for (i=0;i<nbath;i++)
			{
			if (beamflag[i] == MB_FLAG_NONE)
			    store->mbSFlag[i] = 2;
			else if (beamflag[i] == MB_FLAG_NULL)
			    store->mbSFlag[i] = 0;
			else if (beamflag[i] == MB_FLAG_NULL)
			    store->mbSFlag[i] = 4;
			if (beamflag[i] != MB_FLAG_NULL)
			    {
			    store->mbDepth[i] = (int)(bath[i] / depthscale);
			    store->mbAcrossDistance[i] = (int)(bathacrosstrack[i] / distancescale);
			    store->mbAlongDistance[i] = (int)(bathalongtrack[i] / distancescale);
			    }
			else
			    {
			    store->mbDepth[i] = 0;
			    store->mbAcrossDistance[i] = 0;
			    store->mbAlongDistance[i] = 0;
			    }
			}
		}

	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT)
		{
		/* copy in comment */
		strcpy(store->comment, comment);
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
int mbsys_netcdf_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams,
	double *ttimes, double *angles, 
	double *angles_forward, double *angles_null,
	double *heave, double *alongtrack_offset, 
	double *draft, double *ssv, int *error)
{
	char	*function_name = "mbsys_netcdf_ttimes";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_netcdf_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		fprintf(stderr,"dbg2       ttimes:     %d\n",ttimes);
		fprintf(stderr,"dbg2       angles_xtrk:%d\n",angles);
		fprintf(stderr,"dbg2       angles_ltrk:%d\n",angles_forward);
		fprintf(stderr,"dbg2       angles_null:%d\n",angles_null);
		fprintf(stderr,"dbg2       heave:      %d\n",heave);
		fprintf(stderr,"dbg2       ltrk_off:   %d\n",alongtrack_offset);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_netcdf_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* no travel times in this data format */
		*nbeams = 0;

		/* set status */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;

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
int mbsys_netcdf_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, double *transducer_depth, double *altitude, 
	int *error)
{
	char	*function_name = "mbsys_netcdf_extract_altitude";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_netcdf_struct *store;
	double depthscale;
	double distancescale;
	double xtrackminbest;
	double vdepthbest;
	double xtrackmin;
	double vdepth;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_netcdf_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;
	
	/* set starting values */
	*transducer_depth = 0.0;
	*altitude = 0.0;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get transducer depth */
		if (store->mbDynamicDraught > 0)
		    {
		    *transducer_depth = store->mbDynamicDraught[0] * store->mbDynamicDraught_scale_factor;
		    }
		else
		    {
		    *transducer_depth = store->mbDraught
					    + store->mbTransmissionHeave[0] 
						    * store->mbTransmissionHeave_scale_factor;
		    }
		    
		/* get altitude if nonzero */
		if (store->mbVerticalDepth[0] > 0)
		    {
		    *altitude = store->mbVerticalDepth[0] * store->mbVerticalDepth_scale_factor
			    - *transducer_depth;
		    }
		else
		    {
		    /* loop over soundings */
		    depthscale = store->mbDepthScale[0] * store->mbDepthScale_scale_factor;
		    distancescale = store->mbDistanceScale[0] * store->mbDistanceScale_scale_factor;
		    xtrackminbest = 10000000.0;
		    vdepthbest = 0.0;
		    xtrackmin = 10000000.0;
		    vdepth = 0.0;
		    for (i=0;i<store->mbBeamNbr;i++)
			{
			if (store->mbSFlag[i] == 2)
			    {
			    if (fabs((double) store->mbAcrossDistance[i]) < xtrackminbest)
				{
				xtrackminbest = (double) store->mbAcrossDistance[i];
				vdepthbest = (double) store->mbDepth[i];
				}
			    }
			if (store->mbSFlag[i] != 0)
			    {
			    if (fabs((double) store->mbAcrossDistance[i]) < xtrackmin)
				{
				xtrackmin = (double) store->mbAcrossDistance[i];
				vdepth = (double) store->mbDepth[i];
				}
			    }
			}
		    if (vdepthbest > 0.0)
			{
			*altitude = depthscale * vdepthbest - *transducer_depth;
			}
		    else if (vdepth > 0.0)
			{
			*altitude = depthscale * vdepth - *transducer_depth;
			}
		    }

		/* set status */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
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
int mbsys_netcdf_insert_altitude(int verbose, void *mbio_ptr, void *store_ptr,
	double transducer_depth, double altitude, 
	int *error)
{
	char	*function_name = "mbsys_netcdf_insert_altitude";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_netcdf_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:            %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:         %d\n",store_ptr);
		fprintf(stderr,"dbg2       transducer_depth:  %f\n",transducer_depth);
		fprintf(stderr,"dbg2       altitude:          %f\n",altitude);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_netcdf_struct *) store_ptr;

	/* insert data into structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* get stuff */
		for (i=0;i<store->mbAntennaNbr;i++)
		    {
		    /* set draft */
		    store->mbDynamicDraught[i] = (int)(transducer_depth 
						    / store->mbDynamicDraught_scale_factor);
    
		    /* set vertical depth */
		    store->mbVerticalDepth[i] = (int)((altitude + transducer_depth) 
						    / store->mbVerticalDepth_scale_factor);
		    }

		/* set status */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;
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
int mbsys_netcdf_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading, double *draft, 
		double *roll, double *pitch, double *heave, 
		int *error)
{
	char	*function_name = "mbsys_netcdf_extract_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_netcdf_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_netcdf_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get time */
		*time_d = store->mbDate[0] * SECINDAY
			    + store->mbTime[0] * 0.001;
		mb_get_date(verbose,*time_d,time_i);

		/* get navigation */
		*navlon = (double) store->mbAbscissa_scale_factor * store->mbAbscissa[0];
		*navlat = (double) store->mbOrdinate_scale_factor * store->mbOrdinate[0];
		if (mb_io_ptr->lonflip < 0)
			{
			if (*navlon > 0.) 
				*navlon = *navlon - 360.;
			else if (*navlon < -360.)
				*navlon = *navlon + 360.;
			}
		else if (mb_io_ptr->lonflip == 0)
			{
			if (*navlon > 180.) 
				*navlon = *navlon - 360.;
			else if (*navlon < -180.)
				*navlon = *navlon + 360.;
			}
		else
			{
			if (*navlon > 360.) 
				*navlon = *navlon - 360.;
			else if (*navlon < 0.)
				*navlon = *navlon + 360.;
			}

		/* get heading */
		*heading = store->mbHeading[0] * store->mbHeading_scale_factor;

		/* set speed to zero */
		*speed = 0.0;

		/* set draft to zero */
		*draft = store->mbDraught;

		/* get roll pitch and heave */
		*roll = store->mbRoll[0] * store->mbRoll_scale_factor;
		*pitch = store->mbPitch[0] * store->mbPitch_scale_factor;
		*heave = store->mbTransmissionHeave[0] * store->mbTransmissionHeave_scale_factor;

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
		/* set status */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;

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
int mbsys_netcdf_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading, double draft, 
		double roll, double pitch, double heave,
		int *error)
{
	char	*function_name = "mbsys_netcdf_insert_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_netcdf_struct *store;
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
	store = (struct mbsys_netcdf_struct *) store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* reset lon and lat attributes */
		if (strcmp(store->mbOrdinate_name_code, "MB_POSITION_LATITUDE") != 0)
		    {
		    strcpy(store->mbOrdinate_type, "real");
		    strcpy(store->mbOrdinate_long_name, "Latitude");
		    strcpy(store->mbOrdinate_name_code, "MB_POSITION_LATITUDE");
		    strcpy(store->mbOrdinate_units, "degree");
		    strcpy(store->mbOrdinate_unit_code, "MB_DEGREE");
		    store->mbOrdinate_add_offset = 0.;
		    store->mbOrdinate_scale_factor = 5.e-08;
		    store->mbOrdinate_minimum = -1800000000;
		    store->mbOrdinate_maximum = 1800000000;
		    store->mbOrdinate_valid_minimum = -1800000000;
		    store->mbOrdinate_valid_maximum = 1800000000;
		    store->mbOrdinate_missing_value = -2147483648;
		    strcpy(store->mbOrdinate_format_C, "%f");
		    strcpy(store->mbOrdinate_orientation, "direct");
		    }
		if (strcmp(store->mbAbscissa_name_code, "MB_POSITION_LONGITUDE") != 0)
		    {
		    strcpy(store->mbAbscissa_type, "real");
		    strcpy(store->mbAbscissa_long_name, "Longitude");
		    strcpy(store->mbAbscissa_name_code, "MB_POSITION_LONGITUDE");
		    strcpy(store->mbAbscissa_units, "degree");
		    strcpy(store->mbAbscissa_unit_code, "MB_DEGREE");
		    store->mbAbscissa_add_offset = 0.;
		    store->mbAbscissa_scale_factor = 1.e-07;
		    store->mbAbscissa_minimum = -1800000000;
		    store->mbAbscissa_maximum = 1800000000;
		    store->mbAbscissa_valid_minimum = -1800000000;
		    store->mbAbscissa_valid_maximum = 1800000000;
		    store->mbAbscissa_missing_value = -2147483648;
		    strcpy(store->mbAbscissa_format_C, "%f");
		    strcpy(store->mbAbscissa_orientation, "direct");
		    }

		/* get stuff */
		for (i=0;i<store->mbAntennaNbr;i++)
		    {
		    /* get time */
		    store->mbDate[i] = (int)(time_d / SECINDAY);
		    store->mbTime[i] = (int)(1000 * (time_d - store->mbDate[0] * SECINDAY));
    
		    /* get navigation */
		    store->mbAbscissa[i] = (int)(navlon / store->mbAbscissa_scale_factor);
		    store->mbOrdinate[i] = (int)(navlat / store->mbOrdinate_scale_factor);
    
		    /* get heading */
		    store->mbHeading[i] = heading / store->mbHeading_scale_factor;
    
		    /* get speed */
		    
		    /* get draft */
		    store->mbDraught = draft;
    
		    /* get roll pitch and heave */
		    store->mbRoll[i] = roll / store->mbRoll_scale_factor;
		    store->mbPitch[i] = pitch / store->mbPitch_scale_factor;
		    store->mbTransmissionHeave[i] = heave / store->mbTransmissionHeave_scale_factor;
		    }

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
int mbsys_netcdf_copy(int verbose, void *mbio_ptr, 
			void *store_ptr, void *copy_ptr,
			int *error)
{
	char	*function_name = "mbsys_netcdf_copy";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_netcdf_struct *store;
	struct mbsys_netcdf_struct *copy;
	int     i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",
			function_name);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %d\n",mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",store_ptr);
		fprintf(stderr,"dbg2       copy_ptr:   %d\n",copy_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointers */
	store = (struct mbsys_netcdf_struct *) store_ptr;
	copy = (struct mbsys_netcdf_struct *) copy_ptr;
	
	
	/* deallocate memory if required */
	if (store->mbHistoryRecNbr > copy->mbHistoryRecNbr)
	    {
	    status = mb_free(verbose, (char **)&store->mbHistDate, error);
	    status = mb_free(verbose, (char **)&store->mbHistTime, error);
	    status = mb_free(verbose, (char **)&store->mbHistCode, error);
	    status = mb_free(verbose, (char **)&store->mbHistAutor, error);
	    status = mb_free(verbose, (char **)&store->mbHistModule, error);
	    status = mb_free(verbose, (char **)&store->mbHistComment, error);
	    }
	if (store->mbAntennaNbr > copy->mbAntennaNbr)
	    {
	    status = mb_free(verbose, (char **)&store->mbCycle, error);
	    status = mb_free(verbose, (char **)&store->mbDate, error);
	    status = mb_free(verbose, (char **)&store->mbTime, error);
	    status = mb_free(verbose, (char **)&store->mbOrdinate, error);
	    status = mb_free(verbose, (char **)&store->mbAbscissa, error);
	    status = mb_free(verbose, (char **)&store->mbFrequency, error);
	    status = mb_free(verbose, (char **)&store->mbSounderMode, error);
	    status = mb_free(verbose, (char **)&store->mbReferenceDepth, error);
	    status = mb_free(verbose, (char **)&store->mbDynamicDraught, error);
	    status = mb_free(verbose, (char **)&store->mbTide, error);
	    status = mb_free(verbose, (char **)&store->mbSoundVelocity, error);
	    status = mb_free(verbose, (char **)&store->mbHeading, error);
	    status = mb_free(verbose, (char **)&store->mbRoll, error);
	    status = mb_free(verbose, (char **)&store->mbPitch, error);
	    status = mb_free(verbose, (char **)&store->mbTransmissionHeave, error);
	    status = mb_free(verbose, (char **)&store->mbDistanceScale, error);
	    status = mb_free(verbose, (char **)&store->mbDepthScale, error);
	    status = mb_free(verbose, (char **)&store->mbVerticalDepth, error);
	    status = mb_free(verbose, (char **)&store->mbCQuality, error);
	    status = mb_free(verbose, (char **)&store->mbCFlag, error);
	    status = mb_free(verbose, (char **)&store->mbInterlacing, error);
	    status = mb_free(verbose, (char **)&store->mbSamplingRate, error);
	    status = mb_free(verbose, (char **)&store->mbBeam, error);
	    status = mb_free(verbose, (char **)&store->mbAFlag, error);
	    }
	if (store->mbBeamNbr > copy->mbBeamNbr)
	    {
	    status = mb_free(verbose, (char **)&store->mbAlongDistance, error);
	    status = mb_free(verbose, (char **)&store->mbAcrossDistance, error);
	    status = mb_free(verbose, (char **)&store->mbDepth, error);
	    status = mb_free(verbose, (char **)&store->mbSQuality, error);
	    status = mb_free(verbose, (char **)&store->mbSFlag, error);
	    status = mb_free(verbose, (char **)&store->mbAntenna, error);
	    status = mb_free(verbose, (char **)&store->mbBeamBias, error);
	    status = mb_free(verbose, (char **)&store->mbBFlag, error);
	    }
	if (store->mbVelocityProfilNbr > copy->mbVelocityProfilNbr)
	    {
	    status = mb_free(verbose, (char **)&store->mbVelProfilRef, error);
	    status = mb_free(verbose, (char **)&store->mbVelProfilIdx, error);
	    status = mb_free(verbose, (char **)&store->mbVelProfilDate, error);
	    status = mb_free(verbose, (char **)&store->mbVelProfilTime, error);
	    }
	
	/* allocate the memory in copy */
	if (status == MB_SUCCESS)
	    {
	    copy->mbHistoryRecNbr = store->mbHistoryRecNbr;
	    copy->mbNameLength = store->mbNameLength;
	    copy->mbCommentLength = store->mbCommentLength;
	    copy->mbAntennaNbr = store->mbAntennaNbr;
	    copy->mbBeamNbr = store->mbBeamNbr;
	    copy->mbCycleNbr = store->mbCycleNbr;
	    copy->mbVelocityProfilNbr = store->mbVelocityProfilNbr;
	    status = mb_malloc(verbose, 
			copy->mbHistoryRecNbr * sizeof(int),
			(char **)&copy->mbHistDate,error);
	    status = mb_malloc(verbose, 
			copy->mbHistoryRecNbr * sizeof(int),
			(char **)&copy->mbHistTime,error);
	    status = mb_malloc(verbose, 
			copy->mbHistoryRecNbr * sizeof(char),
			(char **)&copy->mbHistCode,error);
	    status = mb_malloc(verbose, 
			copy->mbHistoryRecNbr * copy->mbNameLength * sizeof(char),
			(char **)&copy->mbHistAutor,error);
	    status = mb_malloc(verbose, 
			copy->mbHistoryRecNbr * copy->mbNameLength * sizeof(char),
			(char **)&copy->mbHistModule,error);
	    status = mb_malloc(verbose, 
			copy->mbHistoryRecNbr * copy->mbCommentLength * sizeof(char),
			(char **)&copy->mbHistComment,error);
	    status = mb_malloc(verbose, 
			copy->mbAntennaNbr * sizeof(short),
			(char **)&copy->mbCycle,error);
	    status = mb_malloc(verbose, 
			copy->mbAntennaNbr * sizeof(int),
			(char **)&copy->mbDate,error);
	    status = mb_malloc(verbose, 
			copy->mbAntennaNbr * sizeof(int),
			(char **)&copy->mbTime,error);
	    status = mb_malloc(verbose, 
			copy->mbAntennaNbr * sizeof(int),
			(char **)&copy->mbOrdinate,error);
	    status = mb_malloc(verbose, 
			copy->mbAntennaNbr * sizeof(int),
			(char **)&copy->mbAbscissa,error);
	    status = mb_malloc(verbose, 
			copy->mbAntennaNbr * sizeof(char),
			(char **)&copy->mbFrequency,error);
	    status = mb_malloc(verbose, 
			copy->mbAntennaNbr * sizeof(char),
			(char **)&copy->mbSounderMode,error);
	    status = mb_malloc(verbose, 
			copy->mbAntennaNbr * sizeof(short),
			(char **)&copy->mbReferenceDepth,error);
	    status = mb_malloc(verbose, 
			copy->mbAntennaNbr * sizeof(short),
			(char **)&copy->mbDynamicDraught,error);
	    status = mb_malloc(verbose, 
			copy->mbAntennaNbr * sizeof(short),
			(char **)&copy->mbTide,error);
	    status = mb_malloc(verbose, 
			copy->mbAntennaNbr * sizeof(short),
			(char **)&copy->mbSoundVelocity,error);
	    status = mb_malloc(verbose, 
			copy->mbAntennaNbr * sizeof(short),
			(char **)&copy->mbHeading,error);
	    status = mb_malloc(verbose, 
			copy->mbAntennaNbr * sizeof(short),
			(char **)&copy->mbRoll,error);
	    status = mb_malloc(verbose, 
			copy->mbAntennaNbr * sizeof(short),
			(char **)&copy->mbPitch,error);
	    status = mb_malloc(verbose, 
			copy->mbAntennaNbr * sizeof(short),
			(char **)&copy->mbTransmissionHeave,error);
	    status = mb_malloc(verbose, 
			copy->mbAntennaNbr * sizeof(char),
			(char **)&copy->mbDistanceScale,error);
	    status = mb_malloc(verbose, 
			copy->mbAntennaNbr * sizeof(char),
			(char **)&copy->mbDepthScale,error);
	    status = mb_malloc(verbose, 
			copy->mbAntennaNbr * sizeof(short),
			(char **)&copy->mbVerticalDepth,error);
	    status = mb_malloc(verbose, 
			copy->mbAntennaNbr * sizeof(char),
			(char **)&copy->mbCQuality,error);
	    status = mb_malloc(verbose, 
			copy->mbAntennaNbr * sizeof(char),
			(char **)&copy->mbCFlag,error);
	    status = mb_malloc(verbose, 
			copy->mbAntennaNbr * sizeof(char),
			(char **)&copy->mbInterlacing,error);
	    status = mb_malloc(verbose, 
			copy->mbAntennaNbr * sizeof(short),
			(char **)&copy->mbSamplingRate,error);
	    status = mb_malloc(verbose, 
			copy->mbBeamNbr * sizeof(short),
			(char **)&copy->mbAlongDistance,error);
	    status = mb_malloc(verbose, 
			copy->mbBeamNbr * sizeof(short),
			(char **)&copy->mbAcrossDistance,error);
	    status = mb_malloc(verbose, 
			copy->mbBeamNbr * sizeof(short),
			(char **)&copy->mbDepth,error);
	    status = mb_malloc(verbose, 
			copy->mbBeamNbr * sizeof(char),
			(char **)&copy->mbSQuality,error);
	    status = mb_malloc(verbose, 
			copy->mbBeamNbr * sizeof(char),
			(char **)&copy->mbSFlag,error);
	    status = mb_malloc(verbose, 
			copy->mbBeamNbr * sizeof(char),
			(char **)&copy->mbAntenna,error);
	    status = mb_malloc(verbose, 
			copy->mbBeamNbr * sizeof(short),
			(char **)&copy->mbBeamBias,error);
	    status = mb_malloc(verbose, 
			copy->mbBeamNbr * sizeof(char),
			(char **)&copy->mbBFlag,error);
	    status = mb_malloc(verbose, 
			copy->mbAntennaNbr * sizeof(short),
			(char **)&copy->mbBeam,error);
	    status = mb_malloc(verbose, 
			copy->mbAntennaNbr * sizeof(char),
			(char **)&copy->mbAFlag,error);
	    status = mb_malloc(verbose, 
			copy->mbVelocityProfilNbr * copy->mbCommentLength * sizeof(char),
			(char **)&copy->mbVelProfilRef,error);
	    status = mb_malloc(verbose, 
			copy->mbVelocityProfilNbr * sizeof(short),
			(char **)&copy->mbVelProfilIdx,error);
	    status = mb_malloc(verbose, 
			copy->mbVelocityProfilNbr * sizeof(int),
			(char **)&copy->mbVelProfilDate,error);
	    status = mb_malloc(verbose, 
			copy->mbVelocityProfilNbr * sizeof(int),
			(char **)&copy->mbVelProfilTime,error);
	    }

	/* deal with a memory allocation failure */
	if (status == MB_FAILURE)
	    {
	    status = mb_free(verbose, (char **)&store->mbHistDate, error);
	    status = mb_free(verbose, (char **)&store->mbHistTime, error);
	    status = mb_free(verbose, (char **)&store->mbHistCode, error);
	    status = mb_free(verbose, (char **)&store->mbHistAutor, error);
	    status = mb_free(verbose, (char **)&store->mbHistModule, error);
	    status = mb_free(verbose, (char **)&store->mbHistComment, error);
	    status = mb_free(verbose, (char **)&store->mbCycle, error);
	    status = mb_free(verbose, (char **)&store->mbDate, error);
	    status = mb_free(verbose, (char **)&store->mbTime, error);
	    status = mb_free(verbose, (char **)&store->mbOrdinate, error);
	    status = mb_free(verbose, (char **)&store->mbAbscissa, error);
	    status = mb_free(verbose, (char **)&store->mbFrequency, error);
	    status = mb_free(verbose, (char **)&store->mbSounderMode, error);
	    status = mb_free(verbose, (char **)&store->mbReferenceDepth, error);
	    status = mb_free(verbose, (char **)&store->mbDynamicDraught, error);
	    status = mb_free(verbose, (char **)&store->mbTide, error);
	    status = mb_free(verbose, (char **)&store->mbSoundVelocity, error);
	    status = mb_free(verbose, (char **)&store->mbHeading, error);
	    status = mb_free(verbose, (char **)&store->mbRoll, error);
	    status = mb_free(verbose, (char **)&store->mbPitch, error);
	    status = mb_free(verbose, (char **)&store->mbTransmissionHeave, error);
	    status = mb_free(verbose, (char **)&store->mbDistanceScale, error);
	    status = mb_free(verbose, (char **)&store->mbDepthScale, error);
	    status = mb_free(verbose, (char **)&store->mbVerticalDepth, error);
	    status = mb_free(verbose, (char **)&store->mbCQuality, error);
	    status = mb_free(verbose, (char **)&store->mbCFlag, error);
	    status = mb_free(verbose, (char **)&store->mbInterlacing, error);
	    status = mb_free(verbose, (char **)&store->mbSamplingRate, error);
	    status = mb_free(verbose, (char **)&store->mbAlongDistance, error);
	    status = mb_free(verbose, (char **)&store->mbAcrossDistance, error);
	    status = mb_free(verbose, (char **)&store->mbDepth, error);
	    status = mb_free(verbose, (char **)&store->mbSQuality, error);
	    status = mb_free(verbose, (char **)&store->mbSFlag, error);
	    status = mb_free(verbose, (char **)&store->mbAntenna, error);
	    status = mb_free(verbose, (char **)&store->mbBeamBias, error);
	    status = mb_free(verbose, (char **)&store->mbBFlag, error);
	    status = mb_free(verbose, (char **)&store->mbBeam, error);
	    status = mb_free(verbose, (char **)&store->mbAFlag, error);
	    status = mb_free(verbose, (char **)&store->mbVelProfilRef, error);
	    status = mb_free(verbose, (char **)&store->mbVelProfilIdx, error);
	    status = mb_free(verbose, (char **)&store->mbVelProfilDate, error);
	    status = mb_free(verbose, (char **)&store->mbVelProfilTime, error);
	    status = MB_FAILURE;
	    *error = MB_ERROR_MEMORY_FAIL;
	    if (verbose >= 2)
		    {
		    fprintf(stderr,"\ndbg2  MBIO function <%s> terminated with error\n",
			    function_name);
		    fprintf(stderr,"dbg2  Return values:\n");
		    fprintf(stderr,"dbg2       error:      %d\n",*error);
		    fprintf(stderr,"dbg2  Return status:\n");
		    fprintf(stderr,"dbg2       status:  %d\n",status);
		    }
	    return(status);
	    }

	/* copy the data */
	if (status == MB_SUCCESS)
	    {
	    copy->kind = store->kind;
    
	    /* global attributes */
	    copy->mbVersion = store->mbVersion;
	    for (i=0;i<64;i++)
		{
		copy->mbName[i] = store->mbName[i];
		copy->mbClasse[i] = store->mbClasse[i];
		copy->mbTimeReference[i] = store->mbTimeReference[i];
		copy->mbMeridian180[i] = store->mbMeridian180[i];
		copy->mbGeoDictionnary[i] = store->mbGeoDictionnary[i];
		copy->mbGeoRepresentation[i] = store->mbGeoRepresentation[i];
		copy->mbGeodesicSystem[i] = store->mbGeodesicSystem[i];
		}
	    for (i=0;i<256;i++)
		{
		copy->mbEllipsoidName[i] = store->mbEllipsoidName[i];
		copy->mbProjParameterCode[i] = store->mbProjParameterCode[i];
		copy->mbShip[i] = store->mbShip[i];
		copy->mbSurvey[i] = store->mbSurvey[i];
		copy->mbReference[i] = store->mbReference[i];
		copy->mbNavRef[i] = store->mbNavRef[i];
		copy->mbTideRef[i] = store->mbTideRef[i];
		}
	    copy->mbLevel = store->mbLevel;
	    copy->mbNbrHistoryRec = store->mbNbrHistoryRec;
	    copy->mbStartDate = store->mbStartDate;
	    copy->mbStartTime = store->mbStartTime;
	    copy->mbEndDate = store->mbEndDate;
	    copy->mbEndTime = store->mbEndTime;
	    copy->mbNorthLatitude = store->mbNorthLatitude;
	    copy->mbSouthLatitude = store->mbSouthLatitude;
	    copy->mbEastLongitude = store->mbEastLongitude;
	    copy->mbWestLongitude = store->mbWestLongitude;
	    copy->mbEllipsoidA = store->mbEllipsoidA;
	    copy->mbEllipsoidInvF = store->mbEllipsoidInvF;
	    copy->mbEllipsoidE2 = store->mbEllipsoidE2;
	    copy->mbProjType = store->mbProjType;
	    for (i=0;i<10;i++)
		copy->mbProjParameterValue[i] = store->mbProjParameterValue[i];
	    copy->mbSounder = store->mbSounder = store->mbSounder;
	    for (i=0;i<3;i++)
		{
		copy->mbAntennaOffset[i] = store->mbAntennaOffset[i];
		copy->mbSounderOffset[i] = store->mbSounderOffset[i];
		copy->mbVRUOffset[i] = store->mbVRUOffset[i];
		}
	    copy->mbAntennaDelay = store->mbAntennaDelay;
	    copy->mbSounderDelay = store->mbSounderDelay;
	    copy->mbVRUDelay = store->mbVRUDelay;
	    copy->mbHeadingBias = store->mbHeadingBias;
	    copy->mbRollBias = store->mbRollBias;
	    copy->mbPitchBias = store->mbPitchBias;
	    copy->mbHeaveBias = store->mbHeaveBias;
	    copy->mbDraught = store->mbDraught;
	    copy->mbNavType = store->mbNavType;
	    copy->mbTideType = store->mbTideType;
	    copy->mbMinDepth = store->mbMinDepth;
	    copy->mbMaxDepth = store->mbMaxDepth;
	    copy->mbCycleCounter = store->mbCycleCounter;
	    
	    /* variable ids */
	    copy->mbHistDate_id = store->mbHistDate_id;
	    copy->mbHistTime_id = store->mbHistTime_id;
	    copy->mbHistCode_id = store->mbHistCode_id;
	    copy->mbHistAutor_id = store->mbHistAutor_id;
	    copy->mbHistModule_id = store->mbHistModule_id;
	    copy->mbHistComment_id = store->mbHistComment_id;
	    copy->mbCycle_id = store->mbCycle_id;
	    copy->mbDate_id = store->mbDate_id;
	    copy->mbTime_id = store->mbTime_id;
	    copy->mbOrdinate_id = store->mbOrdinate_id;
	    copy->mbAbscissa_id = store->mbAbscissa_id;
	    copy->mbFrequency_id = store->mbFrequency_id;
	    copy->mbSounderMode_id = store->mbSounderMode_id;
	    copy->mbReferenceDepth_id = store->mbReferenceDepth_id;
	    copy->mbDynamicDraught_id = store->mbDynamicDraught_id;
	    copy->mbTide_id = store->mbTide_id;
	    copy->mbSoundVelocity_id = store->mbSoundVelocity_id;
	    copy->mbHeading_id = store->mbHeading_id;
	    copy->mbRoll_id = store->mbRoll_id;
	    copy->mbPitch_id = store->mbPitch_id;
	    copy->mbTransmissionHeave_id = store->mbTransmissionHeave_id;
	    copy->mbDistanceScale_id = store->mbDistanceScale_id;
	    copy->mbDepthScale_id = store->mbDepthScale_id;
	    copy->mbVerticalDepth_id = store->mbVerticalDepth_id;
	    copy->mbCQuality_id = store->mbCQuality_id;
	    copy->mbCFlag_id = store->mbCFlag_id;
	    copy->mbInterlacing_id = store->mbInterlacing_id;
	    copy->mbSamplingRate_id = store->mbSamplingRate_id;
	    copy->mbAlongDistance_id = store->mbAlongDistance_id;
	    copy->mbAcrossDistance_id = store->mbAcrossDistance_id;
	    copy->mbDepth_id = store->mbDepth_id;
	    copy->mbSQuality_id = store->mbSQuality_id;
	    copy->mbSFlag_id = store->mbSFlag_id;
	    copy->mbAntenna_id = store->mbAntenna_id;
	    copy->mbBeamBias_id = store->mbBeamBias_id;
	    copy->mbBFlag_id = store->mbBFlag_id;
	    copy->mbBeam_id = store->mbBeam_id;
	    copy->mbAFlag_id = store->mbAFlag_id;
	    copy->mbVelProfilRef_id = store->mbVelProfilRef_id;
	    copy->mbVelProfilIdx_id = store->mbVelProfilIdx_id;
	    copy->mbVelProfilDate_id = store->mbVelProfilDate_id;
	    copy->mbVelProfilTime_id = store->mbVelProfilTime_id;
    
	    /* variable pointers */
	    for (i=0;i<copy->mbHistoryRecNbr;i++)
	        {
	        copy->mbHistDate[i] = store->mbHistDate[i];
	        copy->mbHistTime[i] = store->mbHistTime[i];
	        copy->mbHistCode[i] = store->mbHistCode[i];
	        copy->mbHistAutor[i] = store->mbHistAutor[i];
	        copy->mbHistModule[i] = store->mbHistModule[i];
	        copy->mbHistComment[i] = store->mbHistComment[i];
	        }
	    for (i=0;i<copy->mbAntennaNbr;i++)
	        {
		copy->mbCycle[i] = store->mbCycle[i];
		copy->mbDate[i] = store->mbDate[i];
		copy->mbTime[i] = store->mbTime[i];
		copy->mbOrdinate[i] = store->mbOrdinate[i];
		copy->mbAbscissa[i] = store->mbAbscissa[i];
		copy->mbFrequency[i] = store->mbFrequency[i];
		copy->mbSounderMode[i] = store->mbSounderMode[i];
		copy->mbReferenceDepth[i] = store->mbReferenceDepth[i];
		copy->mbDynamicDraught[i] = store->mbDynamicDraught[i];
		copy->mbTide[i] = store->mbTide[i];
		copy->mbSoundVelocity[i] = store->mbSoundVelocity[i];
		copy->mbHeading[i] = store->mbHeading[i];
		copy->mbRoll[i] = store->mbRoll[i];
		copy->mbPitch[i] = store->mbPitch[i];
		copy->mbTransmissionHeave[i] = store->mbTransmissionHeave[i];
		copy->mbDistanceScale[i] = store->mbDistanceScale[i];
		copy->mbDepthScale[i] = store->mbDepthScale[i];
		copy->mbVerticalDepth[i] = store->mbVerticalDepth[i];
		copy->mbCQuality[i] = store->mbCQuality[i];
		copy->mbCFlag[i] = store->mbCFlag[i];
		copy->mbInterlacing[i] = store->mbInterlacing[i];
		copy->mbSamplingRate[i] = store->mbSamplingRate[i];
		}
	    for (i=0;i<copy->mbBeamNbr;i++)
	        {
		copy->mbAlongDistance[i] = store->mbAlongDistance[i];
		copy->mbAcrossDistance[i] = store->mbAcrossDistance[i];
		copy->mbDepth[i] = store->mbDepth[i];
		copy->mbSQuality[i] = store->mbSQuality[i];
		copy->mbSFlag[i] = store->mbSFlag[i];
		copy->mbAntenna[i] = store->mbAntenna[i];
		copy->mbBeamBias[i] = store->mbBeamBias[i];
		copy->mbBFlag[i] = store->mbBFlag[i];
		}
	    for (i=0;i<copy->mbAntennaNbr;i++)
	        {
		copy->mbBeam[i] = store->mbBeam[i];
		copy->mbAFlag[i] = store->mbAFlag[i];
		}
	    for (i=0;i<copy->mbVelocityProfilNbr;i++)
	        {
		copy->mbVelProfilRef[i] = store->mbVelProfilRef[i];
		copy->mbVelProfilIdx[i] = store->mbVelProfilIdx[i];
		copy->mbVelProfilDate[i] = store->mbVelProfilDate[i];
		copy->mbVelProfilTime[i] = store->mbVelProfilTime[i];
		}
	    
	    /* variable attributes */
	    for (i=0;i<64;i++)
		{
		copy->mbHistDate_type[i] = store->mbHistDate_type[i];
		copy->mbHistDate_long_name[i] = store->mbHistDate_long_name[i];
		copy->mbHistDate_name_code[i] = store->mbHistDate_name_code[i];
		copy->mbHistDate_units[i] = store->mbHistDate_units[i];
		copy->mbHistDate_unit_code[i] = store->mbHistDate_unit_code[i];
		copy->mbHistDate_format_C[i] = store->mbHistDate_format_C[i];
		copy->mbHistDate_orientation[i] = store->mbHistDate_orientation[i];
		copy->mbHistTime_type[i] = store->mbHistTime_type[i];
		copy->mbHistTime_long_name[i] = store->mbHistTime_long_name[i];
		copy->mbHistTime_name_code[i] = store->mbHistTime_name_code[i];
		copy->mbHistTime_units[i] = store->mbHistTime_units[i];
		copy->mbHistTime_unit_code[i] = store->mbHistTime_unit_code[i];
		copy->mbHistTime_format_C[i] = store->mbHistTime_format_C[i];
		copy->mbHistTime_orientation[i] = store->mbHistTime_orientation[i];
		copy->mbHistCode_type[i] = store->mbHistCode_type[i];
		copy->mbHistCode_long_name[i] = store->mbHistCode_long_name[i];
		copy->mbHistCode_name_code[i] = store->mbHistCode_name_code[i];
		copy->mbHistCode_units[i] = store->mbHistCode_units[i];
		copy->mbHistCode_unit_code[i] = store->mbHistCode_unit_code[i];
		copy->mbHistCode_format_C[i] = store->mbHistCode_format_C[i];
		copy->mbHistCode_orientation[i] = store->mbHistCode_orientation[i];
		copy->mbHistAutor_type[i] = store->mbHistAutor_type[i];
		copy->mbHistAutor_long_name[i] = store->mbHistAutor_long_name[i];
		copy->mbHistAutor_name_code[i] = store->mbHistAutor_name_code[i];
		copy->mbHistModule_type[i] = store->mbHistModule_type[i];
		copy->mbHistModule_long_name[i] = store->mbHistModule_long_name[i];
		copy->mbHistModule_name_code[i] = store->mbHistModule_name_code[i];
		copy->mbHistComment_type[i] = store->mbHistComment_type[i];
		copy->mbHistComment_long_name[i] = store->mbHistComment_long_name[i];
		copy->mbHistComment_name_code[i] = store->mbHistComment_name_code[i];
		copy->mbCycle_type[i] = store->mbCycle_type[i];
		copy->mbCycle_long_name[i] = store->mbCycle_long_name[i];
		copy->mbCycle_name_code[i] = store->mbCycle_name_code[i];
		copy->mbCycle_units[i] = store->mbCycle_units[i];
		copy->mbCycle_unit_code[i] = store->mbCycle_unit_code[i];
		copy->mbCycle_format_C[i] = store->mbCycle_format_C[i];
		copy->mbCycle_orientation[i] = store->mbCycle_orientation[i];
		copy->mbDate_type[i] = store->mbDate_type[i];
		copy->mbDate_long_name[i] = store->mbDate_long_name[i];
		copy->mbDate_name_code[i] = store->mbDate_name_code[i];
		copy->mbDate_units[i] = store->mbDate_units[i];
		copy->mbDate_unit_code[i] = store->mbDate_unit_code[i];
		copy->mbDate_format_C[i] = store->mbDate_format_C[i];
		copy->mbDate_orientation[i] = store->mbDate_orientation[i];
		copy->mbTime_type[i] = store->mbTime_type[i];
		copy->mbTime_long_name[i] = store->mbTime_long_name[i];
		copy->mbTime_name_code[i] = store->mbTime_name_code[i];
		copy->mbTime_units[i] = store->mbTime_units[i];
		copy->mbTime_unit_code[i] = store->mbTime_unit_code[i];
		copy->mbTime_format_C[i] = store->mbTime_format_C[i];
		copy->mbTime_orientation[i] = store->mbTime_orientation[i];
		copy->mbOrdinate_type[i] = store->mbOrdinate_type[i];
		copy->mbOrdinate_long_name[i] = store->mbOrdinate_long_name[i];
		copy->mbOrdinate_name_code[i] = store->mbOrdinate_name_code[i];
		copy->mbOrdinate_units[i] = store->mbOrdinate_units[i];
		copy->mbOrdinate_unit_code[i] = store->mbOrdinate_unit_code[i];
		copy->mbOrdinate_format_C[i] = store->mbOrdinate_format_C[i];
		copy->mbOrdinate_orientation[i] = store->mbOrdinate_orientation[i];
		copy->mbAbscissa_type[i] = store->mbAbscissa_type[i];
		copy->mbAbscissa_long_name[i] = store->mbAbscissa_long_name[i];
		copy->mbAbscissa_name_code[i] = store->mbAbscissa_name_code[i];
		copy->mbAbscissa_units[i] = store->mbAbscissa_units[i];
		copy->mbAbscissa_unit_code[i] = store->mbAbscissa_unit_code[i];
		copy->mbAbscissa_format_C[i] = store->mbAbscissa_format_C[i];
		copy->mbAbscissa_orientation[i] = store->mbAbscissa_orientation[i];
		copy->mbFrequency_type[i] = store->mbFrequency_type[i];
		copy->mbFrequency_long_name[i] = store->mbFrequency_long_name[i];
		copy->mbFrequency_name_code[i] = store->mbFrequency_name_code[i];
		copy->mbFrequency_units[i] = store->mbFrequency_units[i];
		copy->mbFrequency_unit_code[i] = store->mbFrequency_unit_code[i];
		copy->mbFrequency_format_C[i] = store->mbFrequency_format_C[i];
		copy->mbFrequency_orientation[i] = store->mbFrequency_orientation[i];
		copy->mbSounderMode_type[i] = store->mbSounderMode_type[i];
		copy->mbSounderMode_long_name[i] = store->mbSounderMode_long_name[i];
		copy->mbSounderMode_name_code[i] = store->mbSounderMode_name_code[i];
		copy->mbSounderMode_units[i] = store->mbSounderMode_units[i];
		copy->mbSounderMode_unit_code[i] = store->mbSounderMode_unit_code[i];
		copy->mbSounderMode_format_C[i] = store->mbSounderMode_format_C[i];
		copy->mbSounderMode_orientation[i] = store->mbSounderMode_orientation[i];
		copy->mbReferenceDepth_type[i] = store->mbReferenceDepth_type[i];
		copy->mbReferenceDepth_long_name[i] = store->mbReferenceDepth_long_name[i];
		copy->mbReferenceDepth_name_code[i] = store->mbReferenceDepth_name_code[i];
		copy->mbReferenceDepth_units[i] = store->mbReferenceDepth_units[i];
		copy->mbReferenceDepth_unit_code[i] = store->mbReferenceDepth_unit_code[i];
		copy->mbReferenceDepth_format_C[i] = store->mbReferenceDepth_format_C[i];
		copy->mbReferenceDepth_orientation[i] = store->mbReferenceDepth_orientation[i];
		copy->mbDynamicDraught_type[i] = store->mbDynamicDraught_type[i];
		copy->mbDynamicDraught_long_name[i] = store->mbDynamicDraught_long_name[i];
		copy->mbDynamicDraught_name_code[i] = store->mbDynamicDraught_name_code[i];
		copy->mbDynamicDraught_units[i] = store->mbDynamicDraught_units[i];
		copy->mbDynamicDraught_unit_code[i] = store->mbDynamicDraught_unit_code[i];
		copy->mbDynamicDraught_format_C[i] = store->mbDynamicDraught_format_C[i];
		copy->mbDynamicDraught_orientation[i] = store->mbDynamicDraught_orientation[i];
		copy->mbTide_type[i] = store->mbTide_type[i];
		copy->mbTide_long_name[i] = store->mbTide_long_name[i];
		copy->mbTide_name_code[i] = store->mbTide_name_code[i];
		copy->mbTide_units[i] = store->mbTide_units[i];
		copy->mbTide_unit_code[i] = store->mbTide_unit_code[i];
		copy->mbTide_format_C[i] = store->mbTide_format_C[i];
		copy->mbTide_orientation[i] = store->mbTide_orientation[i];
		copy->mbSoundVelocity_type[i] = store->mbSoundVelocity_type[i];
		copy->mbSoundVelocity_long_name[i] = store->mbSoundVelocity_long_name[i];
		copy->mbSoundVelocity_name_code[i] = store->mbSoundVelocity_name_code[i];
		copy->mbSoundVelocity_units[i] = store->mbSoundVelocity_units[i];
		copy->mbSoundVelocity_unit_code[i] = store->mbSoundVelocity_unit_code[i];
		copy->mbSoundVelocity_format_C[i] = store->mbSoundVelocity_format_C[i];
		copy->mbSoundVelocity_orientation[i] = store->mbSoundVelocity_orientation[i];
		copy->mbHeading_type[i] = store->mbHeading_type[i];
		copy->mbHeading_long_name[i] = store->mbHeading_long_name[i];
		copy->mbHeading_name_code[i] = store->mbHeading_name_code[i];
		copy->mbHeading_units[i] = store->mbHeading_units[i];
		copy->mbHeading_unit_code[i] = store->mbHeading_unit_code[i];
		copy->mbHeading_format_C[i] = store->mbHeading_format_C[i];
		copy->mbHeading_orientation[i] = store->mbHeading_orientation[i];
		copy->mbRoll_type[i] = store->mbRoll_type[i];
		copy->mbRoll_long_name[i] = store->mbRoll_long_name[i];
		copy->mbRoll_name_code[i] = store->mbRoll_name_code[i];
		copy->mbRoll_units[i] = store->mbRoll_units[i];
		copy->mbRoll_unit_code[i] = store->mbRoll_unit_code[i];
		copy->mbRoll_format_C[i] = store->mbRoll_format_C[i];
		copy->mbRoll_orientation[i] = store->mbRoll_orientation[i];
		copy->mbPitch_type[i] = store->mbPitch_type[i];
		copy->mbPitch_long_name[i] = store->mbPitch_long_name[i];
		copy->mbPitch_name_code[i] = store->mbPitch_name_code[i];
		copy->mbPitch_units[i] = store->mbPitch_units[i];
		copy->mbPitch_unit_code[i] = store->mbPitch_unit_code[i];
		copy->mbPitch_format_C[i] = store->mbPitch_format_C[i];
		copy->mbPitch_orientation[i] = store->mbPitch_orientation[i];
		copy->mbTransmissionHeave_type[i] = store->mbTransmissionHeave_type[i];
		copy->mbTransmissionHeave_long_name[i] = store->mbTransmissionHeave_long_name[i];
		copy->mbTransmissionHeave_name_code[i] = store->mbTransmissionHeave_name_code[i];
		copy->mbTransmissionHeave_units[i] = store->mbTransmissionHeave_units[i];
		copy->mbTransmissionHeave_unit_code[i] = store->mbTransmissionHeave_unit_code[i];
		copy->mbTransmissionHeave_format_C[i] = store->mbTransmissionHeave_format_C[i];
		copy->mbTransmissionHeave_orientation[i] = store->mbTransmissionHeave_orientation[i];
		copy->mbDistanceScale_type[i] = store->mbDistanceScale_type[i];
		copy->mbDistanceScale_long_name[i] = store->mbDistanceScale_long_name[i];
		copy->mbDistanceScale_name_code[i] = store->mbDistanceScale_name_code[i];
		copy->mbDistanceScale_units[i] = store->mbDistanceScale_units[i];
		copy->mbDistanceScale_unit_code[i] = store->mbDistanceScale_unit_code[i];
		copy->mbDistanceScale_format_C[i] = store->mbDistanceScale_format_C[i];
		copy->mbDistanceScale_orientation[i] = store->mbDistanceScale_orientation[i];
		copy->mbDepthScale_type[i] = store->mbDepthScale_type[i];
		copy->mbDepthScale_long_name[i] = store->mbDepthScale_long_name[i];
		copy->mbDepthScale_name_code[i] = store->mbDepthScale_name_code[i];
		copy->mbDepthScale_units[i] = store->mbDepthScale_units[i];
		copy->mbDepthScale_unit_code[i] = store->mbDepthScale_unit_code[i];
		copy->mbDepthScale_format_C[i] = store->mbDepthScale_format_C[i];
		copy->mbDepthScale_orientation[i] = store->mbDepthScale_orientation[i];
		copy->mbVerticalDepth_type[i] = store->mbVerticalDepth_type[i];
		copy->mbVerticalDepth_long_name[i] = store->mbVerticalDepth_long_name[i];
		copy->mbVerticalDepth_name_code[i] = store->mbVerticalDepth_name_code[i];
		copy->mbVerticalDepth_units[i] = store->mbVerticalDepth_units[i];
		copy->mbVerticalDepth_unit_code[i] = store->mbVerticalDepth_unit_code[i];
		copy->mbVerticalDepth_format_C[i] = store->mbVerticalDepth_format_C[i];
		copy->mbVerticalDepth_orientation[i] = store->mbVerticalDepth_orientation[i];
		copy->mbCQuality_type[i] = store->mbCQuality_type[i];
		copy->mbCQuality_long_name[i] = store->mbCQuality_long_name[i];
		copy->mbCQuality_name_code[i] = store->mbCQuality_name_code[i];
		copy->mbCQuality_units[i] = store->mbCQuality_units[i];
		copy->mbCQuality_unit_code[i] = store->mbCQuality_unit_code[i];
		copy->mbCQuality_format_C[i] = store->mbCQuality_format_C[i];
		copy->mbCQuality_orientation[i] = store->mbCQuality_orientation[i];
		copy->mbCFlag_type[i] = store->mbCFlag_type[i];
		copy->mbCFlag_long_name[i] = store->mbCFlag_long_name[i];
		copy->mbCFlag_name_code[i] = store->mbCFlag_name_code[i];
		copy->mbCFlag_units[i] = store->mbCFlag_units[i];
		copy->mbCFlag_unit_code[i] = store->mbCFlag_unit_code[i];
		copy->mbCFlag_format_C[i] = store->mbCFlag_format_C[i];
		copy->mbCFlag_orientation[i] = store->mbCFlag_orientation[i];
		copy->mbInterlacing_type[i] = store->mbInterlacing_type[i];
		copy->mbInterlacing_long_name[i] = store->mbInterlacing_long_name[i];
		copy->mbInterlacing_name_code[i] = store->mbInterlacing_name_code[i];
		copy->mbInterlacing_units[i] = store->mbInterlacing_units[i];
		copy->mbInterlacing_unit_code[i] = store->mbInterlacing_unit_code[i];
		copy->mbInterlacing_format_C[i] = store->mbInterlacing_format_C[i];
		copy->mbInterlacing_orientation[i] = store->mbInterlacing_orientation[i];
		copy->mbSamplingRate_type[i] = store->mbSamplingRate_type[i];
		copy->mbSamplingRate_long_name[i] = store->mbSamplingRate_long_name[i];
		copy->mbSamplingRate_name_code[i] = store->mbSamplingRate_name_code[i];
		copy->mbSamplingRate_units[i] = store->mbSamplingRate_units[i];
		copy->mbSamplingRate_unit_code[i] = store->mbSamplingRate_unit_code[i];
		copy->mbSamplingRate_format_C[i] = store->mbSamplingRate_format_C[i];
		copy->mbSamplingRate_orientation[i] = store->mbSamplingRate_orientation[i];
		copy->mbAlongDistance_type[i] = store->mbAlongDistance_type[i];
		copy->mbAlongDistance_long_name[i] = store->mbAlongDistance_long_name[i];
		copy->mbAlongDistance_name_code[i] = store->mbAlongDistance_name_code[i];
		copy->mbAlongDistance_units[i] = store->mbAlongDistance_units[i];
		copy->mbAlongDistance_unit_code[i] = store->mbAlongDistance_unit_code[i];
		copy->mbAlongDistance_format_C[i] = store->mbAlongDistance_format_C[i];
		copy->mbAlongDistance_orientation[i] = store->mbAlongDistance_orientation[i];
		copy->mbAcrossDistance_type[i] = store->mbAcrossDistance_type[i];
		copy->mbAcrossDistance_long_name[i] = store->mbAcrossDistance_long_name[i];
		copy->mbAcrossDistance_name_code[i] = store->mbAcrossDistance_name_code[i];
		copy->mbAcrossDistance_units[i] = store->mbAcrossDistance_units[i];
		copy->mbAcrossDistance_unit_code[i] = store->mbAcrossDistance_unit_code[i];
		copy->mbAcrossDistance_format_C[i] = store->mbAcrossDistance_format_C[i];
		copy->mbAcrossDistance_orientation[i] = store->mbAcrossDistance_orientation[i];
		copy->mbDepth_type[i] = store->mbDepth_type[i];
		copy->mbDepth_long_name[i] = store->mbDepth_long_name[i];
		copy->mbDepth_name_code[i] = store->mbDepth_name_code[i];
		copy->mbDepth_units[i] = store->mbDepth_units[i];
		copy->mbDepth_unit_code[i] = store->mbDepth_unit_code[i];
		copy->mbDepth_format_C[i] = store->mbDepth_format_C[i];
		copy->mbDepth_orientation[i] = store->mbDepth_orientation[i];
		copy->mbSQuality_type[i] = store->mbSQuality_type[i];
		copy->mbSQuality_long_name[i] = store->mbSQuality_long_name[i];
		copy->mbSQuality_name_code[i] = store->mbSQuality_name_code[i];
		copy->mbSQuality_units[i] = store->mbSQuality_units[i];
		copy->mbSQuality_unit_code[i] = store->mbSQuality_unit_code[i];
		copy->mbSQuality_format_C[i] = store->mbSQuality_format_C[i];
		copy->mbSQuality_orientation[i] = store->mbSQuality_orientation[i];
		copy->mbSFlag_type[i] = store->mbSFlag_type[i];
		copy->mbSFlag_long_name[i] = store->mbSFlag_long_name[i];
		copy->mbSFlag_name_code[i] = store->mbSFlag_name_code[i];
		copy->mbSFlag_units[i] = store->mbSFlag_units[i];
		copy->mbSFlag_unit_code[i] = store->mbSFlag_unit_code[i];
		copy->mbSFlag_format_C[i] = store->mbSFlag_format_C[i];
		copy->mbSFlag_orientation[i] = store->mbSFlag_orientation[i];
		copy->mbAntenna_type[i] = store->mbAntenna_type[i];
		copy->mbAntenna_long_name[i] = store->mbAntenna_long_name[i];
		copy->mbAntenna_name_code[i] = store->mbAntenna_name_code[i];
		copy->mbAntenna_units[i] = store->mbAntenna_units[i];
		copy->mbAntenna_unit_code[i] = store->mbAntenna_unit_code[i];
		copy->mbAntenna_format_C[i] = store->mbAntenna_format_C[i];
		copy->mbAntenna_orientation[i] = store->mbAntenna_orientation[i];
		copy->mbBeamBias_type[i] = store->mbBeamBias_type[i];
		copy->mbBeamBias_long_name[i] = store->mbBeamBias_long_name[i];
		copy->mbBeamBias_name_code[i] = store->mbBeamBias_name_code[i];
		copy->mbBeamBias_units[i] = store->mbBeamBias_units[i];
		copy->mbBeamBias_unit_code[i] = store->mbBeamBias_unit_code[i];
		copy->mbBeamBias_format_C[i] = store->mbBeamBias_format_C[i];
		copy->mbBeamBias_orientation[i] = store->mbBeamBias_orientation[i];
		copy->mbBFlag_type[i] = store->mbBFlag_type[i];
		copy->mbBFlag_long_name[i] = store->mbBFlag_long_name[i];
		copy->mbBFlag_name_code[i] = store->mbBFlag_name_code[i];
		copy->mbBFlag_units[i] = store->mbBFlag_units[i];
		copy->mbBFlag_unit_code[i] = store->mbBFlag_unit_code[i];
		copy->mbBFlag_format_C[i] = store->mbBFlag_format_C[i];
		copy->mbBFlag_orientation[i] = store->mbBFlag_orientation[i];
		copy->mbBeam_type[i] = store->mbBeam_type[i];
		copy->mbBeam_long_name[i] = store->mbBeam_long_name[i];
		copy->mbBeam_name_code[i] = store->mbBeam_name_code[i];
		copy->mbBeam_units[i] = store->mbBeam_units[i];
		copy->mbBeam_unit_code[i] = store->mbBeam_unit_code[i];
		copy->mbBeam_format_C[i] = store->mbBeam_format_C[i];
		copy->mbBeam_orientation[i] = store->mbBeam_orientation[i];
		copy->mbAFlag_type[i] = store->mbAFlag_type[i];
		copy->mbAFlag_long_name[i] = store->mbAFlag_long_name[i];
		copy->mbAFlag_name_code[i] = store->mbAFlag_name_code[i];
		copy->mbAFlag_units[i] = store->mbAFlag_units[i];
		copy->mbAFlag_unit_code[i] = store->mbAFlag_unit_code[i];
		copy->mbAFlag_format_C[i] = store->mbAFlag_format_C[i];
		copy->mbAFlag_orientation[i] = store->mbAFlag_orientation[i];
		copy->mbVelProfilRef_type[i] = store->mbVelProfilRef_type[i];
		copy->mbVelProfilRef_long_name[i] = store->mbVelProfilRef_long_name[i];
		copy->mbVelProfilRef_name_code[i] = store->mbVelProfilRef_name_code[i];
		copy->mbVelProfilIdx_type[i] = store->mbVelProfilIdx_type[i];
		copy->mbVelProfilIdx_long_name[i] = store->mbVelProfilIdx_long_name[i];
		copy->mbVelProfilIdx_name_code[i] = store->mbVelProfilIdx_name_code[i];
		copy->mbVelProfilIdx_units[i] = store->mbVelProfilIdx_units[i];
		copy->mbVelProfilIdx_unit_code[i] = store->mbVelProfilIdx_unit_code[i];
		copy->mbVelProfilIdx_format_C[i] = store->mbVelProfilIdx_format_C[i];
		copy->mbVelProfilIdx_orientation[i] = store->mbVelProfilIdx_orientation[i];
		copy->mbVelProfilDate_type[i] = store->mbVelProfilDate_type[i];
		copy->mbVelProfilDate_long_name[i] = store->mbVelProfilDate_long_name[i];
		copy->mbVelProfilDate_name_code[i] = store->mbVelProfilDate_name_code[i];
		copy->mbVelProfilDate_units[i] = store->mbVelProfilDate_units[i];
		copy->mbVelProfilDate_unit_code[i] = store->mbVelProfilDate_unit_code[i];
		copy->mbVelProfilDate_format_C[i] = store->mbVelProfilDate_format_C[i];
		copy->mbVelProfilDate_orientation[i] = store->mbVelProfilDate_orientation[i];
		copy->mbVelProfilTime_type[i] = store->mbVelProfilTime_type[i];
		copy->mbVelProfilTime_long_name[i] = store->mbVelProfilTime_long_name[i];
		copy->mbVelProfilTime_name_code[i] = store->mbVelProfilTime_name_code[i];
		copy->mbVelProfilTime_units[i] = store->mbVelProfilTime_units[i];
		copy->mbVelProfilTime_unit_code[i] = store->mbVelProfilTime_unit_code[i];
		copy->mbVelProfilTime_format_C[i] = store->mbVelProfilTime_format_C[i];
		copy->mbVelProfilTime_orientation[i] = store->mbVelProfilTime_orientation[i];
		}
	    copy->mbHistDate_add_offset = store->mbHistDate_add_offset;
	    copy->mbHistDate_scale_factor = store->mbHistDate_scale_factor;
	    copy->mbHistDate_minimum = store->mbHistDate_minimum;
	    copy->mbHistDate_maximum = store->mbHistDate_maximum;
	    copy->mbHistDate_valid_minimum = store->mbHistDate_valid_minimum;
	    copy->mbHistDate_valid_maximum = store->mbHistDate_valid_maximum;
	    copy->mbHistDate_missing_value = store->mbHistDate_missing_value;
	    copy->mbHistTime_add_offset = store->mbHistTime_add_offset;
	    copy->mbHistTime_scale_factor = store->mbHistTime_scale_factor;
	    copy->mbHistTime_minimum = store->mbHistTime_minimum;
	    copy->mbHistTime_maximum = store->mbHistTime_maximum;
	    copy->mbHistTime_valid_minimum = store->mbHistTime_valid_minimum;
	    copy->mbHistTime_valid_maximum = store->mbHistTime_valid_maximum;
	    copy->mbHistTime_missing_value = store->mbHistTime_missing_value;
	    copy->mbHistCode_add_offset = store->mbHistCode_add_offset;
	    copy->mbHistCode_scale_factor = store->mbHistCode_scale_factor;
	    copy->mbHistCode_minimum = store->mbHistCode_minimum;
	    copy->mbHistCode_maximum = store->mbHistCode_maximum;
	    copy->mbHistCode_valid_minimum = store->mbHistCode_valid_minimum;
	    copy->mbHistCode_valid_maximum = store->mbHistCode_valid_maximum;
	    copy->mbHistCode_missing_value = store->mbHistCode_missing_value;
	    copy->mbCycle_add_offset = store->mbCycle_add_offset;
	    copy->mbCycle_scale_factor = store->mbCycle_scale_factor;
	    copy->mbCycle_minimum = store->mbCycle_minimum;
	    copy->mbCycle_maximum = store->mbCycle_maximum;
	    copy->mbCycle_valid_minimum = store->mbCycle_valid_minimum;
	    copy->mbCycle_valid_maximum = store->mbCycle_valid_maximum;
	    copy->mbCycle_missing_value = store->mbCycle_missing_value;
	    copy->mbDate_add_offset = store->mbDate_add_offset;
	    copy->mbDate_scale_factor = store->mbDate_scale_factor;
	    copy->mbDate_minimum = store->mbDate_minimum;
	    copy->mbDate_maximum = store->mbDate_maximum;
	    copy->mbDate_valid_minimum = store->mbDate_valid_minimum;
	    copy->mbDate_valid_maximum = store->mbDate_valid_maximum;
	    copy->mbDate_missing_value = store->mbDate_missing_value;
	    copy->mbTime_add_offset = store->mbTime_add_offset;
	    copy->mbTime_scale_factor = store->mbTime_scale_factor;
	    copy->mbTime_minimum = store->mbTime_minimum;
	    copy->mbTime_maximum = store->mbTime_maximum;
	    copy->mbTime_valid_minimum = store->mbTime_valid_minimum;
	    copy->mbTime_valid_maximum = store->mbTime_valid_maximum;
	    copy->mbTime_missing_value = store->mbTime_missing_value;
	    copy->mbOrdinate_add_offset = store->mbOrdinate_add_offset;
	    copy->mbOrdinate_scale_factor = store->mbOrdinate_scale_factor;
	    copy->mbOrdinate_minimum = store->mbOrdinate_minimum;
	    copy->mbOrdinate_maximum = store->mbOrdinate_maximum;
	    copy->mbOrdinate_valid_minimum = store->mbOrdinate_valid_minimum;
	    copy->mbOrdinate_valid_maximum = store->mbOrdinate_valid_maximum;
	    copy->mbOrdinate_missing_value = store->mbOrdinate_missing_value;
	    copy->mbAbscissa_add_offset = store->mbAbscissa_add_offset;
	    copy->mbAbscissa_scale_factor = store->mbAbscissa_scale_factor;
	    copy->mbAbscissa_minimum = store->mbAbscissa_minimum;
	    copy->mbAbscissa_maximum = store->mbAbscissa_maximum;
	    copy->mbAbscissa_valid_minimum = store->mbAbscissa_valid_minimum;
	    copy->mbAbscissa_valid_maximum = store->mbAbscissa_valid_maximum;
	    copy->mbAbscissa_missing_value = store->mbAbscissa_missing_value;
	    copy->mbFrequency_add_offset = store->mbFrequency_add_offset;
	    copy->mbFrequency_scale_factor = store->mbFrequency_scale_factor;
	    copy->mbFrequency_minimum = store->mbFrequency_minimum;
	    copy->mbFrequency_maximum = store->mbFrequency_maximum;
	    copy->mbFrequency_valid_minimum = store->mbFrequency_valid_minimum;
	    copy->mbFrequency_valid_maximum = store->mbFrequency_valid_maximum;
	    copy->mbFrequency_missing_value = store->mbFrequency_missing_value;
	    copy->mbSounderMode_add_offset = store->mbSounderMode_add_offset;
	    copy->mbSounderMode_scale_factor = store->mbSounderMode_scale_factor;
	    copy->mbSounderMode_minimum = store->mbSounderMode_minimum;
	    copy->mbSounderMode_maximum = store->mbSounderMode_maximum;
	    copy->mbSounderMode_valid_minimum = store->mbSounderMode_valid_minimum;
	    copy->mbSounderMode_valid_maximum = store->mbSounderMode_valid_maximum;
	    copy->mbSounderMode_missing_value = store->mbSounderMode_missing_value;
	    copy->mbReferenceDepth_add_offset = store->mbReferenceDepth_add_offset;
	    copy->mbReferenceDepth_scale_factor = store->mbReferenceDepth_scale_factor;
	    copy->mbReferenceDepth_minimum = store->mbReferenceDepth_minimum;
	    copy->mbReferenceDepth_maximum = store->mbReferenceDepth_maximum;
	    copy->mbReferenceDepth_valid_minimum = store->mbReferenceDepth_valid_minimum;
	    copy->mbReferenceDepth_valid_maximum = store->mbReferenceDepth_valid_maximum;
	    copy->mbReferenceDepth_missing_value = store->mbReferenceDepth_missing_value;
	    copy->mbDynamicDraught_add_offset = store->mbDynamicDraught_add_offset;
	    copy->mbDynamicDraught_scale_factor = store->mbDynamicDraught_scale_factor;
	    copy->mbDynamicDraught_minimum = store->mbDynamicDraught_minimum;
	    copy->mbDynamicDraught_maximum = store->mbDynamicDraught_maximum;
	    copy->mbDynamicDraught_valid_minimum = store->mbDynamicDraught_valid_minimum;
	    copy->mbDynamicDraught_valid_maximum = store->mbDynamicDraught_valid_maximum;
	    copy->mbDynamicDraught_missing_value = store->mbDynamicDraught_missing_value;
	    copy->mbTide_add_offset = store->mbTide_add_offset;
	    copy->mbTide_scale_factor = store->mbTide_scale_factor;
	    copy->mbTide_minimum = store->mbTide_minimum;
	    copy->mbTide_maximum = store->mbTide_maximum;
	    copy->mbTide_valid_minimum = store->mbTide_valid_minimum;
	    copy->mbTide_valid_maximum = store->mbTide_valid_maximum;
	    copy->mbTide_missing_value = store->mbTide_missing_value;
	    copy->mbSoundVelocity_add_offset = store->mbSoundVelocity_add_offset;
	    copy->mbSoundVelocity_scale_factor = store->mbSoundVelocity_scale_factor;
	    copy->mbSoundVelocity_minimum = store->mbSoundVelocity_minimum;
	    copy->mbSoundVelocity_maximum = store->mbSoundVelocity_maximum;
	    copy->mbSoundVelocity_valid_minimum = store->mbSoundVelocity_valid_minimum;
	    copy->mbSoundVelocity_valid_maximum = store->mbSoundVelocity_valid_maximum;
	    copy->mbSoundVelocity_missing_value = store->mbSoundVelocity_missing_value;
	    copy->mbHeading_add_offset = store->mbHeading_add_offset;
	    copy->mbHeading_scale_factor = store->mbHeading_scale_factor;
	    copy->mbHeading_minimum = store->mbHeading_minimum;
	    copy->mbHeading_maximum = store->mbHeading_maximum;
	    copy->mbHeading_valid_minimum = store->mbHeading_valid_minimum;
	    copy->mbHeading_valid_maximum = store->mbHeading_valid_maximum;
	    copy->mbHeading_missing_value = store->mbHeading_missing_value;
	    copy->mbRoll_add_offset = store->mbRoll_add_offset;
	    copy->mbRoll_scale_factor = store->mbRoll_scale_factor;
	    copy->mbRoll_minimum = store->mbRoll_minimum;
	    copy->mbRoll_maximum = store->mbRoll_maximum;
	    copy->mbRoll_valid_minimum = store->mbRoll_valid_minimum;
	    copy->mbRoll_valid_maximum = store->mbRoll_valid_maximum;
	    copy->mbRoll_missing_value = store->mbRoll_missing_value;
	    copy->mbPitch_add_offset = store->mbPitch_add_offset;
	    copy->mbPitch_scale_factor = store->mbPitch_scale_factor;
	    copy->mbPitch_minimum = store->mbPitch_minimum;
	    copy->mbPitch_maximum = store->mbPitch_maximum;
	    copy->mbPitch_valid_minimum = store->mbPitch_valid_minimum;
	    copy->mbPitch_valid_maximum = store->mbPitch_valid_maximum;
	    copy->mbPitch_missing_value = store->mbPitch_missing_value;
	    copy->mbTransmissionHeave_add_offset = store->mbTransmissionHeave_add_offset;
	    copy->mbTransmissionHeave_scale_factor = store->mbTransmissionHeave_scale_factor;
	    copy->mbTransmissionHeave_minimum = store->mbTransmissionHeave_minimum;
	    copy->mbTransmissionHeave_maximum = store->mbTransmissionHeave_maximum;
	    copy->mbTransmissionHeave_valid_minimum = store->mbTransmissionHeave_valid_minimum;
	    copy->mbTransmissionHeave_valid_maximum = store->mbTransmissionHeave_valid_maximum;
	    copy->mbTransmissionHeave_missing_value = store->mbTransmissionHeave_missing_value;
	    copy->mbDistanceScale_add_offset = store->mbDistanceScale_add_offset;
	    copy->mbDistanceScale_scale_factor = store->mbDistanceScale_scale_factor;
	    copy->mbDistanceScale_minimum = store->mbDistanceScale_minimum;
	    copy->mbDistanceScale_maximum = store->mbDistanceScale_maximum;
	    copy->mbDistanceScale_valid_minimum = store->mbDistanceScale_valid_minimum;
	    copy->mbDistanceScale_valid_maximum = store->mbDistanceScale_valid_maximum;
	    copy->mbDistanceScale_missing_value = store->mbDistanceScale_missing_value;
	    copy->mbDepthScale_add_offset = store->mbDepthScale_add_offset;
	    copy->mbDepthScale_scale_factor = store->mbDepthScale_scale_factor;
	    copy->mbDepthScale_minimum = store->mbDepthScale_minimum;
	    copy->mbDepthScale_maximum = store->mbDepthScale_maximum;
	    copy->mbDepthScale_valid_minimum = store->mbDepthScale_valid_minimum;
	    copy->mbDepthScale_valid_maximum = store->mbDepthScale_valid_maximum;
	    copy->mbDepthScale_missing_value = store->mbDepthScale_missing_value;
	    copy->mbVerticalDepth_add_offset = store->mbVerticalDepth_add_offset;
	    copy->mbVerticalDepth_scale_factor = store->mbVerticalDepth_scale_factor;
	    copy->mbVerticalDepth_minimum = store->mbVerticalDepth_minimum;
	    copy->mbVerticalDepth_maximum = store->mbVerticalDepth_maximum;
	    copy->mbVerticalDepth_valid_minimum = store->mbVerticalDepth_valid_minimum;
	    copy->mbVerticalDepth_valid_maximum = store->mbVerticalDepth_valid_maximum;
	    copy->mbVerticalDepth_missing_value = store->mbVerticalDepth_missing_value;
	    copy->mbCQuality_add_offset = store->mbCQuality_add_offset;
	    copy->mbCQuality_scale_factor = store->mbCQuality_scale_factor;
	    copy->mbCQuality_minimum = store->mbCQuality_minimum;
	    copy->mbCQuality_maximum = store->mbCQuality_maximum;
	    copy->mbCQuality_valid_minimum = store->mbCQuality_valid_minimum;
	    copy->mbCQuality_valid_maximum = store->mbCQuality_valid_maximum;
	    copy->mbCQuality_missing_value = store->mbCQuality_missing_value;
	    copy->mbCFlag_add_offset = store->mbCFlag_add_offset;
	    copy->mbCFlag_scale_factor = store->mbCFlag_scale_factor;
	    copy->mbCFlag_minimum = store->mbCFlag_minimum;
	    copy->mbCFlag_maximum = store->mbCFlag_maximum;
	    copy->mbCFlag_valid_minimum = store->mbCFlag_valid_minimum;
	    copy->mbCFlag_valid_maximum = store->mbCFlag_valid_maximum;
	    copy->mbCFlag_missing_value = store->mbCFlag_missing_value;
	    copy->mbInterlacing_add_offset = store->mbInterlacing_add_offset;
	    copy->mbInterlacing_scale_factor = store->mbInterlacing_scale_factor;
	    copy->mbInterlacing_minimum = store->mbInterlacing_minimum;
	    copy->mbInterlacing_maximum = store->mbInterlacing_maximum;
	    copy->mbInterlacing_valid_minimum = store->mbInterlacing_valid_minimum;
	    copy->mbInterlacing_valid_maximum = store->mbInterlacing_valid_maximum;
	    copy->mbInterlacing_missing_value = store->mbInterlacing_missing_value;
	    copy->mbSamplingRate_add_offset = store->mbSamplingRate_add_offset;
	    copy->mbSamplingRate_scale_factor = store->mbSamplingRate_scale_factor;
	    copy->mbSamplingRate_minimum = store->mbSamplingRate_minimum;
	    copy->mbSamplingRate_maximum = store->mbSamplingRate_maximum;
	    copy->mbSamplingRate_valid_minimum = store->mbSamplingRate_valid_minimum;
	    copy->mbSamplingRate_valid_maximum = store->mbSamplingRate_valid_maximum;
	    copy->mbSamplingRate_missing_value = store->mbSamplingRate_missing_value;
	    copy->mbAlongDistance_add_offset = store->mbAlongDistance_add_offset;
	    copy->mbAlongDistance_scale_factor = store->mbAlongDistance_scale_factor;
	    copy->mbAlongDistance_minimum = store->mbAlongDistance_minimum;
	    copy->mbAlongDistance_maximum = store->mbAlongDistance_maximum;
	    copy->mbAlongDistance_valid_minimum = store->mbAlongDistance_valid_minimum;
	    copy->mbAlongDistance_valid_maximum = store->mbAlongDistance_valid_maximum;
	    copy->mbAlongDistance_missing_value = store->mbAlongDistance_missing_value;
	    copy->mbAcrossDistance_add_offset = store->mbAcrossDistance_add_offset;
	    copy->mbAcrossDistance_scale_factor = store->mbAcrossDistance_scale_factor;
	    copy->mbAcrossDistance_minimum = store->mbAcrossDistance_minimum;
	    copy->mbAcrossDistance_maximum = store->mbAcrossDistance_maximum;
	    copy->mbAcrossDistance_valid_minimum = store->mbAcrossDistance_valid_minimum;
	    copy->mbAcrossDistance_valid_maximum = store->mbAcrossDistance_valid_maximum;
	    copy->mbAcrossDistance_missing_value = store->mbAcrossDistance_missing_value;
	    copy->mbDepth_add_offset = store->mbDepth_add_offset;
	    copy->mbDepth_scale_factor = store->mbDepth_scale_factor;
	    copy->mbDepth_minimum = store->mbDepth_minimum;
	    copy->mbDepth_maximum = store->mbDepth_maximum;
	    copy->mbDepth_valid_minimum = store->mbDepth_valid_minimum;
	    copy->mbDepth_valid_maximum = store->mbDepth_valid_maximum;
	    copy->mbDepth_missing_value = store->mbDepth_missing_value;
	    copy->mbSQuality_add_offset = store->mbSQuality_add_offset;
	    copy->mbSQuality_scale_factor = store->mbSQuality_scale_factor;
	    copy->mbSQuality_minimum = store->mbSQuality_minimum;
	    copy->mbSQuality_maximum = store->mbSQuality_maximum;
	    copy->mbSQuality_valid_minimum = store->mbSQuality_valid_minimum;
	    copy->mbSQuality_valid_maximum = store->mbSQuality_valid_maximum;
	    copy->mbSQuality_missing_value = store->mbSQuality_missing_value;
	    copy->mbSFlag_add_offset = store->mbSFlag_add_offset;
	    copy->mbSFlag_scale_factor = store->mbSFlag_scale_factor;
	    copy->mbSFlag_minimum = store->mbSFlag_minimum;
	    copy->mbSFlag_maximum = store->mbSFlag_maximum;
	    copy->mbSFlag_valid_minimum = store->mbSFlag_valid_minimum;
	    copy->mbSFlag_valid_maximum = store->mbSFlag_valid_maximum;
	    copy->mbSFlag_missing_value = store->mbSFlag_missing_value;
	    copy->mbAntenna_add_offset = store->mbAntenna_add_offset;
	    copy->mbAntenna_scale_factor = store->mbAntenna_scale_factor;
	    copy->mbAntenna_minimum = store->mbAntenna_minimum;
	    copy->mbAntenna_maximum = store->mbAntenna_maximum;
	    copy->mbAntenna_valid_minimum = store->mbAntenna_valid_minimum;
	    copy->mbAntenna_valid_maximum = store->mbAntenna_valid_maximum;
	    copy->mbAntenna_missing_value = store->mbAntenna_missing_value;
	    copy->mbBeamBias_add_offset = store->mbBeamBias_add_offset;
	    copy->mbBeamBias_scale_factor = store->mbBeamBias_scale_factor;
	    copy->mbBeamBias_minimum = store->mbBeamBias_minimum;
	    copy->mbBeamBias_maximum = store->mbBeamBias_maximum;
	    copy->mbBeamBias_valid_minimum = store->mbBeamBias_valid_minimum;
	    copy->mbBeamBias_valid_maximum = store->mbBeamBias_valid_maximum;
	    copy->mbBeamBias_missing_value = store->mbBeamBias_missing_value;
	    copy->mbBFlag_add_offset = store->mbBFlag_add_offset;
	    copy->mbBFlag_scale_factor = store->mbBFlag_scale_factor;
	    copy->mbBFlag_minimum = store->mbBFlag_minimum;
	    copy->mbBFlag_maximum = store->mbBFlag_maximum;
	    copy->mbBFlag_valid_minimum = store->mbBFlag_valid_minimum;
	    copy->mbBFlag_valid_maximum = store->mbBFlag_valid_maximum;
	    copy->mbBFlag_missing_value = store->mbBFlag_missing_value;
	    copy->mbBeam_add_offset = store->mbBeam_add_offset;
	    copy->mbBeam_scale_factor = store->mbBeam_scale_factor;
	    copy->mbBeam_minimum = store->mbBeam_minimum;
	    copy->mbBeam_maximum = store->mbBeam_maximum;
	    copy->mbBeam_valid_minimum = store->mbBeam_valid_minimum;
	    copy->mbBeam_valid_maximum = store->mbBeam_valid_maximum;
	    copy->mbBeam_missing_value = store->mbBeam_missing_value;
	    copy->mbAFlag_add_offset = store->mbAFlag_add_offset;
	    copy->mbAFlag_scale_factor = store->mbAFlag_scale_factor;
	    copy->mbAFlag_minimum = store->mbAFlag_minimum;
	    copy->mbAFlag_maximum = store->mbAFlag_maximum;
	    copy->mbAFlag_valid_minimum = store->mbAFlag_valid_minimum;
	    copy->mbAFlag_valid_maximum = store->mbAFlag_valid_maximum;
	    copy->mbAFlag_missing_value = store->mbAFlag_missing_value;
	    copy->mbVelProfilIdx_add_offset = store->mbVelProfilIdx_add_offset;
	    copy->mbVelProfilIdx_scale_factor = store->mbVelProfilIdx_scale_factor;
	    copy->mbVelProfilIdx_minimum = store->mbVelProfilIdx_minimum;
	    copy->mbVelProfilIdx_maximum = store->mbVelProfilIdx_maximum;
	    copy->mbVelProfilIdx_valid_minimum = store->mbVelProfilIdx_valid_minimum;
	    copy->mbVelProfilIdx_valid_maximum = store->mbVelProfilIdx_valid_maximum;
	    copy->mbVelProfilIdx_missing_value = store->mbVelProfilIdx_missing_value;
	    copy->mbVelProfilDate_add_offset = store->mbVelProfilDate_add_offset;
	    copy->mbVelProfilDate_scale_factor = store->mbVelProfilDate_scale_factor;
	    copy->mbVelProfilDate_minimum = store->mbVelProfilDate_minimum;
	    copy->mbVelProfilDate_maximum = store->mbVelProfilDate_maximum;
	    copy->mbVelProfilDate_valid_minimum = store->mbVelProfilDate_valid_minimum;
	    copy->mbVelProfilDate_valid_maximum = store->mbVelProfilDate_valid_maximum;
	    copy->mbVelProfilDate_missing_value = store->mbVelProfilDate_missing_value;
	    copy->mbVelProfilTime_add_offset = store->mbVelProfilTime_add_offset;
	    copy->mbVelProfilTime_scale_factor = store->mbVelProfilTime_scale_factor;
	    copy->mbVelProfilTime_minimum = store->mbVelProfilTime_minimum;
	    copy->mbVelProfilTime_maximum = store->mbVelProfilTime_maximum;
	    copy->mbVelProfilTime_valid_minimum = store->mbVelProfilTime_valid_minimum;
	    copy->mbVelProfilTime_valid_maximum = store->mbVelProfilTime_valid_maximum;
	    copy->mbVelProfilTime_missing_value = store->mbVelProfilTime_missing_value;
	
	    /* storage comment string */
	    for (i=0;i<MBSYS_NETCDF_COMMENTLEN;i++)
	        copy->comment[i] = store->comment[i];	    
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
