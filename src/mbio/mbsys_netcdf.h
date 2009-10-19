/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_netcdf.h	4/8/2002
 *	$Id$
 *
 *    Copyright (c) 2002-2009 by
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
 * mbsys_netcdf.h defines the data structures used by MBIO functions
 * to store data from the IFREMER netCDF multibeam format.
 * The MBIO format id is:
 *      MBF_MBNETCDF : MBIO ID 75
 *
 *
 * Author:	D. W. Caress
 * Date:	April 8, 2002
 *
 * $Log: mbsys_netcdf.h,v $
 * Revision 5.6  2008/12/05 17:32:52  caress
 * Check-in mods 5 December 2008 including contributions from Gordon Keith.
 *
 * Revision 5.5  2008/05/16 22:56:24  caress
 * Release 5.1.1beta18.
 *
 * Revision 5.4  2008/03/01 09:14:03  caress
 * Some housekeeping changes.
 *
 * Revision 5.3  2005/11/05 00:48:04  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.2  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.1  2002/05/29 23:41:49  caress
 * Release 5.0.beta18
 *
 * Revision 5.0  2002/05/02 03:56:34  caress
 * Release 5.0.beta17
 *
 *
 */
/*
 * Notes on the MBF_MBNETCDF data format:
 *   1.
 *   2.
 *
 */

/* dimension lengths */
#define MBSYS_NETCDF_COMMENTLEN	    256
#define MBSYS_NETCDF_ATTRIBUTELEN   64
#define MBSYS_NETCDF_NAMELEN	    20
#define MBSYS_NETCDF_VELPROFNBR	    2

/* sonar id numbers */
#define MBSYS_NETCDF_SONAR_UNKNOWN	    0
#define MBSYS_NETCDF_SONAR_SEABEAM	    10
#define MBSYS_NETCDF_SONAR_ECHOSXD15	    20
#define MBSYS_NETCDF_SONAR_ECHOSXD60	    21
#define MBSYS_NETCDF_SONAR_HSDS		    30
#define MBSYS_NETCDF_SONAR_LENNEMOR	    40
#define MBSYS_NETCDF_SONAR_TMS5265B	    41
#define MBSYS_NETCDF_SONAR_EM100	    50
#define MBSYS_NETCDF_SONAR_EM1000	    51
#define MBSYS_NETCDF_SONAR_EM12S	    52
#define MBSYS_NETCDF_SONAR_EM12D	    53
#define MBSYS_NETCDF_SONAR_EM3000S	    54
#define MBSYS_NETCDF_SONAR_EM3000D	    55
#define MBSYS_NETCDF_SONAR_EM300	    56
#define MBSYS_NETCDF_SONAR_FURUNO	    70

/* sonar beam numbers */
#define MBSYS_NETCDF_SONAR_BEAMS_UNKNOWN    0
#define MBSYS_NETCDF_SONAR_BEAMS_SEABEAM    19
#define MBSYS_NETCDF_SONAR_BEAMS_ECHOSXD15  15
#define MBSYS_NETCDF_SONAR_BEAMS_ECHOSXD60  60
#define MBSYS_NETCDF_SONAR_BEAMS_HSDS	    59
#define MBSYS_NETCDF_SONAR_BEAMS_LENNEMOR   20
#define MBSYS_NETCDF_SONAR_BEAMS_TMS5265B   500
#define MBSYS_NETCDF_SONAR_BEAMS_EM100	    32
#define MBSYS_NETCDF_SONAR_BEAMS_EM1000	    60
#define MBSYS_NETCDF_SONAR_BEAMS_EM12S	    81
#define MBSYS_NETCDF_SONAR_BEAMS_EM12D	    162
#define MBSYS_NETCDF_SONAR_BEAMS_EM3000S    127
#define MBSYS_NETCDF_SONAR_BEAMS_EM3000D    254
#define MBSYS_NETCDF_SONAR_BEAMS_EM300	    127
#define MBSYS_NETCDF_SONAR_BEAMS_FURUNO	    45

/* seconds in day */
#define SECINDAY     86400.0

/* internal data structure */
struct mbsys_netcdf_struct
	{
	int	 kind;

	/* global attributes */
	short mbVersion;
	char mbName[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbClasse[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTimeReference[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbMeridian180[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbGeoDictionnary[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbGeoRepresentation[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbGeodesicSystem[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbEllipsoidName[MBSYS_NETCDF_COMMENTLEN];
	char mbProjParameterCode[MBSYS_NETCDF_COMMENTLEN];
	char mbShip[MBSYS_NETCDF_COMMENTLEN];
	char mbSurvey[MBSYS_NETCDF_COMMENTLEN];
	char mbReference[MBSYS_NETCDF_COMMENTLEN];
	char mbNavRef[MBSYS_NETCDF_COMMENTLEN];
	char mbTideRef[MBSYS_NETCDF_COMMENTLEN];
	short mbLevel;
	short mbNbrHistoryRec;
	int mbStartDate;
	int mbStartTime;
	int mbEndDate;
	int mbEndTime;
	double mbNorthLatitude;
	double mbSouthLatitude;
	double mbEastLongitude;
	double mbWestLongitude;
	double mbEllipsoidA;
	double mbEllipsoidInvF;
	double mbEllipsoidE2;
	short mbProjType;
	double mbProjParameterValue[10];
	short mbSounder;
	double mbAntennaOffset[3];
	double mbAntennaDelay;
	double mbSounderOffset[3];
	double mbSounderDelay;
	double mbVRUOffset[3];
	double mbVRUDelay;
	double mbHeadingBias;
	double mbRollBias;
	double mbPitchBias;
	double mbHeaveBias;
	double mbDraught;
	short mbNavType;
	short mbTideType;
	double mbMinDepth;
	double mbMaxDepth;
	int mbCycleCounter;

	/* dimensions */
	size_t CIB_BLOCK_DIM; /* not in extended format */
	size_t mbHistoryRecNbr;
	size_t mbNameLength;
	size_t mbCommentLength;
	size_t mbAntennaNbr;
	size_t mbBeamNbr;
	size_t mbCycleNbr;
	size_t mbVelocityProfilNbr;

	/* variable ids */
	int mbHistDate_id;
	int mbHistTime_id;
	int mbHistCode_id;
	int mbHistAutor_id;
	int mbHistModule_id;
	int mbHistComment_id;
	int mbCycle_id;
	int mbDate_id;
	int mbTime_id;
	int mbOrdinate_id;
	int mbAbscissa_id;
	int mbFrequency_id;
	int mbSonarFrequency_id;
	int mbSounderMode_id;
	int mbReferenceDepth_id;
	int mbDynamicDraught_id;
	int mbTide_id;
	int mbSoundVelocity_id;
	int mbHeading_id;
	int mbRoll_id;
	int mbPitch_id;
	int mbTransmissionHeave_id;
	int mbDistanceScale_id;
	int mbRangeScale_id;
	int mbDepthScale_id;
	int mbVerticalDepth_id;
	int mbCQuality_id;
	int mbCFlag_id;
	int mbInterlacing_id;
	int mbSamplingRate_id;
	int mbCompensationLayerMode_id;
	int mbTransmitBeamwidth_id;
	int mbReceiveBeamwidth_id;
	int mbTransmitPulseLength_id;
	int mbAlongDistance_id;
	int mbAcrossDistance_id;
	int mbDepth_id;
	int mbAcrossBeamAngle_id;
	int mbAzimutBeamAngle_id;
	int mbRange_id;
	int mbSoundingBias_id;
	int mbSQuality_id;
	int mbReflectivity_id;
	int mbReceptionHeave_id;
	int mbAlongSlope_id;
	int mbAcrossSlope_id;
	int mbSFlag_id;
	int mbSLengthOfDetection_id;
	int mbAntenna_id;
	int mbBeamBias_id;
	int mbBFlag_id;
	int mbBeam_id;
	int mbAFlag_id;
	int mbVelProfilRef_id;
	int mbVelProfilIdx_id;
	int mbVelProfilDate_id;
	int mbVelProfilTime_id;

	/* variable pointers */
        int *mbHistDate;
        int *mbHistTime;
        char *mbHistCode;
        char *mbHistAutor;
        char *mbHistModule;
        char *mbHistComment;

        int *mbCycle;
        int *mbDate;
        int *mbTime;
        int *mbOrdinate;
        int *mbAbscissa;
        char *mbFrequency;
        int *mbSonarFrequency;	/* extended mode */
        char *mbSounderMode;
        int *mbReferenceDepth;
        short *mbDynamicDraught;
        short *mbTide;
        short *mbSoundVelocity;
        unsigned short *mbHeading;
        short *mbRoll;
        short *mbPitch;
        short *mbTransmissionHeave;
        char *mbDistanceScale;
        short *mbRangeScale;	/* extended mode */
        char *mbDepthScale;
        int *mbVerticalDepth;
        char *mbCQuality;
        char *mbCFlag;
        char *mbInterlacing;
        short *mbSamplingRate;
        char *mbCompensationLayerMode;	/* extended mode */
        short *mbTransmitBeamwidth;	/* extended mode */
        char *mbReceiveBeamwidth;	/* extended mode */
        short *mbTransmitPulseLength;	/* extended mode */

        short *mbAlongDistance;
        short *mbAcrossDistance;
        int *mbDepth;
        short *mbAcrossBeamAngle;	/* extended mode */
        short *mbAzimutBeamAngle;	/* extended mode */
        short *mbRange;		/* extended mode */
        short *mbSoundingBias;	/* extended mode */
        char *mbSQuality;
        char *mbReflectivity ;	/* extended mode */
        char *mbReceptionHeave;	/* extended mode */
        short *mbAlongSlope;	/* extended mode */
        short *mbAcrossSlope;	/* extended mode */
        char *mbSFlag;
        char *mbSLengthOfDetection;	/* extended mode */

        char *mbAntenna;
        short *mbBeamBias;	/* not in extended mode */
        char *mbBFlag;

        short *mbBeam;
        char *mbAFlag;

        char *mbVelProfilRef;
        short *mbVelProfilIdx;
        int *mbVelProfilDate;
        int *mbVelProfilTime;

	/* variable attributes */
	char mbHistDate_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHistDate_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHistDate_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHistDate_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHistDate_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHistDate_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHistDate_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbHistDate_add_offset;
	int mbHistDate_scale_factor;
	int mbHistDate_minimum;
	int mbHistDate_maximum;
	int mbHistDate_valid_minimum;
	int mbHistDate_valid_maximum;
	int mbHistDate_missing_value;
	char mbHistTime_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHistTime_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHistTime_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHistTime_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHistTime_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHistTime_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHistTime_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbHistTime_add_offset;
	int mbHistTime_scale_factor;
	int mbHistTime_minimum;
	int mbHistTime_maximum;
	int mbHistTime_valid_minimum;
	int mbHistTime_valid_maximum;
	int mbHistTime_missing_value;
	char mbHistCode_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHistCode_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHistCode_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHistCode_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHistCode_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHistCode_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHistCode_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbHistCode_add_offset;
	int mbHistCode_scale_factor;
	int mbHistCode_minimum;
	int mbHistCode_maximum;
	int mbHistCode_valid_minimum;
	int mbHistCode_valid_maximum;
	int mbHistCode_missing_value;
	char mbHistAutor_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHistAutor_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHistAutor_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHistModule_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHistModule_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHistModule_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHistComment_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHistComment_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHistComment_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbCycle_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbCycle_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbCycle_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbCycle_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbCycle_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbCycle_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbCycle_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbCycle_add_offset;
	int mbCycle_scale_factor;
	int mbCycle_minimum;
	int mbCycle_maximum;
	int mbCycle_valid_minimum;
	int mbCycle_valid_maximum;
	int mbCycle_missing_value;
	char mbDate_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbDate_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbDate_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbDate_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbDate_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbDate_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbDate_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbDate_add_offset;
	int mbDate_scale_factor;
	int mbDate_minimum;
	int mbDate_maximum;
	int mbDate_valid_minimum;
	int mbDate_valid_maximum;
	int mbDate_missing_value;
	char mbTime_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTime_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTime_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTime_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTime_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTime_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTime_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbTime_add_offset;
	int mbTime_scale_factor;
	int mbTime_minimum;
	int mbTime_maximum;
	int mbTime_valid_minimum;
	int mbTime_valid_maximum;
	int mbTime_missing_value;
	char mbOrdinate_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbOrdinate_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbOrdinate_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbOrdinate_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbOrdinate_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbOrdinate_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbOrdinate_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	double mbOrdinate_add_offset;
	double mbOrdinate_scale_factor;
	double mbOrdinate_minimum;
	double mbOrdinate_maximum;
	int mbOrdinate_valid_minimum;
	int mbOrdinate_valid_maximum;
	int mbOrdinate_missing_value;
	char mbAbscissa_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAbscissa_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAbscissa_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAbscissa_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAbscissa_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAbscissa_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAbscissa_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	double mbAbscissa_add_offset;
	double mbAbscissa_scale_factor;
	double mbAbscissa_minimum;
	double mbAbscissa_maximum;
	int mbAbscissa_valid_minimum;
	int mbAbscissa_valid_maximum;
	int mbAbscissa_missing_value;
	char mbFrequency_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbFrequency_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbFrequency_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbFrequency_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbFrequency_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbFrequency_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbFrequency_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbFrequency_add_offset;
	int mbFrequency_scale_factor;
	int mbFrequency_minimum;
	int mbFrequency_maximum;
	int mbFrequency_valid_minimum;
	int mbFrequency_valid_maximum;
	int mbFrequency_missing_value;
	char mbSonarFrequency_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSonarFrequency_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSonarFrequency_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSonarFrequency_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSonarFrequency_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSonarFrequency_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSonarFrequency_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	double mbSonarFrequency_add_offset;
	double mbSonarFrequency_scale_factor;
	double mbSonarFrequency_minimum;
	double mbSonarFrequency_maximum;
	int mbSonarFrequency_valid_minimum;
	int mbSonarFrequency_valid_maximum;
	int mbSonarFrequency_missing_value;
	char mbSounderMode_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSounderMode_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSounderMode_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSounderMode_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSounderMode_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSounderMode_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSounderMode_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbSounderMode_add_offset;
	int mbSounderMode_scale_factor;
	int mbSounderMode_minimum;
	int mbSounderMode_maximum;
	int mbSounderMode_valid_minimum;
	int mbSounderMode_valid_maximum;
	int mbSounderMode_missing_value;
	char mbReferenceDepth_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbReferenceDepth_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbReferenceDepth_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbReferenceDepth_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbReferenceDepth_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbReferenceDepth_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbReferenceDepth_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	double mbReferenceDepth_add_offset;
	double mbReferenceDepth_scale_factor;
	double mbReferenceDepth_minimum;
	double mbReferenceDepth_maximum;
	int mbReferenceDepth_valid_minimum;
	int mbReferenceDepth_valid_maximum;
	int mbReferenceDepth_missing_value;
	char mbDynamicDraught_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbDynamicDraught_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbDynamicDraught_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbDynamicDraught_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbDynamicDraught_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbDynamicDraught_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbDynamicDraught_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	double mbDynamicDraught_add_offset;
	double mbDynamicDraught_scale_factor;
	double mbDynamicDraught_minimum;
	double mbDynamicDraught_maximum;
	int mbDynamicDraught_valid_minimum;
	int mbDynamicDraught_valid_maximum;
	int mbDynamicDraught_missing_value;
	char mbTide_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTide_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTide_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTide_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTide_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTide_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTide_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	double mbTide_add_offset;
	double mbTide_scale_factor;
	double mbTide_minimum;
	double mbTide_maximum;
	int mbTide_valid_minimum;
	int mbTide_valid_maximum;
	int mbTide_missing_value;
	char mbSoundVelocity_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSoundVelocity_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSoundVelocity_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSoundVelocity_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSoundVelocity_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSoundVelocity_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSoundVelocity_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	double mbSoundVelocity_add_offset;
	double mbSoundVelocity_scale_factor;
	double mbSoundVelocity_minimum;
	double mbSoundVelocity_maximum;
	int mbSoundVelocity_valid_minimum;
	int mbSoundVelocity_valid_maximum;
	int mbSoundVelocity_missing_value;
	char mbHeading_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHeading_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHeading_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHeading_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHeading_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHeading_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbHeading_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	double mbHeading_add_offset;
	double mbHeading_scale_factor;
	double mbHeading_minimum;
	double mbHeading_maximum;
	int mbHeading_valid_minimum;
	int mbHeading_valid_maximum;
	int mbHeading_missing_value;
	char mbRoll_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbRoll_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbRoll_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbRoll_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbRoll_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbRoll_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbRoll_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	double mbRoll_add_offset;
	double mbRoll_scale_factor;
	double mbRoll_minimum;
	double mbRoll_maximum;
	int mbRoll_valid_minimum;
	int mbRoll_valid_maximum;
	int mbRoll_missing_value;
	char mbPitch_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbPitch_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbPitch_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbPitch_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbPitch_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbPitch_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbPitch_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	double mbPitch_add_offset;
	double mbPitch_scale_factor;
	double mbPitch_minimum;
	double mbPitch_maximum;
	int mbPitch_valid_minimum;
	int mbPitch_valid_maximum;
	int mbPitch_missing_value;
	char mbTransmissionHeave_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTransmissionHeave_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTransmissionHeave_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTransmissionHeave_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTransmissionHeave_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTransmissionHeave_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTransmissionHeave_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	double mbTransmissionHeave_add_offset;
	double mbTransmissionHeave_scale_factor;
	double mbTransmissionHeave_minimum;
	double mbTransmissionHeave_maximum;
	int mbTransmissionHeave_valid_minimum;
	int mbTransmissionHeave_valid_maximum;
	int mbTransmissionHeave_missing_value;
	char mbDistanceScale_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbDistanceScale_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbDistanceScale_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbDistanceScale_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbDistanceScale_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbDistanceScale_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbDistanceScale_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	double mbDistanceScale_add_offset;
	double mbDistanceScale_scale_factor;
	double mbDistanceScale_minimum;
	double mbDistanceScale_maximum;
	int mbDistanceScale_valid_minimum;
	int mbDistanceScale_valid_maximum;
	int mbDistanceScale_missing_value;
	char mbRangeScale_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbRangeScale_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbRangeScale_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbRangeScale_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbRangeScale_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbRangeScale_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbRangeScale_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	double mbRangeScale_add_offset;
	double mbRangeScale_scale_factor;
	double mbRangeScale_minimum;
	double mbRangeScale_maximum;
	int mbRangeScale_valid_minimum;
	int mbRangeScale_valid_maximum;
	int mbRangeScale_missing_value;
	char mbDepthScale_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbDepthScale_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbDepthScale_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbDepthScale_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbDepthScale_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbDepthScale_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbDepthScale_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	double mbDepthScale_add_offset;
	double mbDepthScale_scale_factor;
	double mbDepthScale_minimum;
	double mbDepthScale_maximum;
	int mbDepthScale_valid_minimum;
	int mbDepthScale_valid_maximum;
	int mbDepthScale_missing_value;
	char mbVerticalDepth_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbVerticalDepth_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbVerticalDepth_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbVerticalDepth_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbVerticalDepth_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbVerticalDepth_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbVerticalDepth_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbVerticalDepth_add_offset;
	int mbVerticalDepth_scale_factor;
	int mbVerticalDepth_minimum;
	int mbVerticalDepth_maximum;
	int mbVerticalDepth_valid_minimum;
	int mbVerticalDepth_valid_maximum;
	int mbVerticalDepth_missing_value;
	char mbCQuality_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbCQuality_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbCQuality_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbCQuality_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbCQuality_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbCQuality_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbCQuality_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbCQuality_add_offset;
	int mbCQuality_scale_factor;
	int mbCQuality_minimum;
	int mbCQuality_maximum;
	int mbCQuality_valid_minimum;
	int mbCQuality_valid_maximum;
	int mbCQuality_missing_value;
	char mbCFlag_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbCFlag_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbCFlag_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbCFlag_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbCFlag_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbCFlag_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbCFlag_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbCFlag_add_offset;
	int mbCFlag_scale_factor;
	int mbCFlag_minimum;
	int mbCFlag_maximum;
	int mbCFlag_valid_minimum;
	int mbCFlag_valid_maximum;
	int mbCFlag_missing_value;
	char mbInterlacing_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbInterlacing_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbInterlacing_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbInterlacing_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbInterlacing_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbInterlacing_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbInterlacing_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbInterlacing_add_offset;
	int mbInterlacing_scale_factor;
	int mbInterlacing_minimum;
	int mbInterlacing_maximum;
	int mbInterlacing_valid_minimum;
	int mbInterlacing_valid_maximum;
	int mbInterlacing_missing_value;
	char mbSamplingRate_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSamplingRate_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSamplingRate_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSamplingRate_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSamplingRate_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSamplingRate_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSamplingRate_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbSamplingRate_add_offset;
	int mbSamplingRate_scale_factor;
	int mbSamplingRate_minimum;
	int mbSamplingRate_maximum;
	int mbSamplingRate_valid_minimum;
	int mbSamplingRate_valid_maximum;
	int mbSamplingRate_missing_value;
	char mbCompensationLayerMode_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbCompensationLayerMode_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbCompensationLayerMode_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbCompensationLayerMode_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbCompensationLayerMode_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbCompensationLayerMode_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbCompensationLayerMode_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbCompensationLayerMode_add_offset;
	int mbCompensationLayerMode_scale_factor;
	int mbCompensationLayerMode_minimum;
	int mbCompensationLayerMode_maximum;
	int mbCompensationLayerMode_valid_minimum;
	int mbCompensationLayerMode_valid_maximum;
	int mbCompensationLayerMode_missing_value;
	char mbTransmitBeamwidth_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTransmitBeamwidth_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTransmitBeamwidth_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTransmitBeamwidth_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTransmitBeamwidth_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTransmitBeamwidth_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTransmitBeamwidth_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbTransmitBeamwidth_add_offset;
	int mbTransmitBeamwidth_scale_factor;
	int mbTransmitBeamwidth_minimum;
	int mbTransmitBeamwidth_maximum;
	int mbTransmitBeamwidth_valid_minimum;
	int mbTransmitBeamwidth_valid_maximum;
	int mbTransmitBeamwidth_missing_value;
	char mbReceiveBeamwidth_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbReceiveBeamwidth_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbReceiveBeamwidth_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbReceiveBeamwidth_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbReceiveBeamwidth_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbReceiveBeamwidth_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbReceiveBeamwidth_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbReceiveBeamwidth_add_offset;
	int mbReceiveBeamwidth_scale_factor;
	int mbReceiveBeamwidth_minimum;
	int mbReceiveBeamwidth_maximum;
	int mbReceiveBeamwidth_valid_minimum;
	int mbReceiveBeamwidth_valid_maximum;
	int mbReceiveBeamwidth_missing_value;
	char mbTransmitPulseLength_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTransmitPulseLength_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTransmitPulseLength_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTransmitPulseLength_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTransmitPulseLength_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTransmitPulseLength_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbTransmitPulseLength_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbTransmitPulseLength_add_offset;
	int mbTransmitPulseLength_scale_factor;
	int mbTransmitPulseLength_minimum;
	int mbTransmitPulseLength_maximum;
	int mbTransmitPulseLength_valid_minimum;
	int mbTransmitPulseLength_valid_maximum;
	int mbTransmitPulseLength_missing_value;
	char mbAlongDistance_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAlongDistance_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAlongDistance_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAlongDistance_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAlongDistance_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAlongDistance_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAlongDistance_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbAlongDistance_add_offset;
	int mbAlongDistance_scale_factor;
	int mbAlongDistance_minimum;
	int mbAlongDistance_maximum;
	int mbAlongDistance_valid_minimum;
	int mbAlongDistance_valid_maximum;
	int mbAlongDistance_missing_value;
	char mbAcrossDistance_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAcrossDistance_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAcrossDistance_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAcrossDistance_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAcrossDistance_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAcrossDistance_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAcrossDistance_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbAcrossDistance_add_offset;
	int mbAcrossDistance_scale_factor;
	int mbAcrossDistance_minimum;
	int mbAcrossDistance_maximum;
	int mbAcrossDistance_valid_minimum;
	int mbAcrossDistance_valid_maximum;
	int mbAcrossDistance_missing_value;
	char mbDepth_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbDepth_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbDepth_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbDepth_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbDepth_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbDepth_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbDepth_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbDepth_add_offset;
	int mbDepth_scale_factor;
	int mbDepth_minimum;
	int mbDepth_maximum;
	int mbDepth_valid_minimum;
	int mbDepth_valid_maximum;
	int mbDepth_missing_value;
	char mbAcrossBeamAngle_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAcrossBeamAngle_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAcrossBeamAngle_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAcrossBeamAngle_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAcrossBeamAngle_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAcrossBeamAngle_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAcrossBeamAngle_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	double mbAcrossBeamAngle_add_offset;
	double mbAcrossBeamAngle_scale_factor;
	double mbAcrossBeamAngle_minimum;
	double mbAcrossBeamAngle_maximum;
	int mbAcrossBeamAngle_valid_minimum;
	int mbAcrossBeamAngle_valid_maximum;
	int mbAcrossBeamAngle_missing_value;
	char mbAzimutBeamAngle_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAzimutBeamAngle_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAzimutBeamAngle_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAzimutBeamAngle_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAzimutBeamAngle_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAzimutBeamAngle_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAzimutBeamAngle_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	double mbAzimutBeamAngle_add_offset;
	double mbAzimutBeamAngle_scale_factor;
	double mbAzimutBeamAngle_minimum;
	double mbAzimutBeamAngle_maximum;
	int mbAzimutBeamAngle_valid_minimum;
	int mbAzimutBeamAngle_valid_maximum;
	int mbAzimutBeamAngle_missing_value;
	char mbRange_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbRange_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbRange_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbRange_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbRange_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbRange_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbRange_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbRange_add_offset;
	int mbRange_scale_factor;
	int mbRange_minimum;
	int mbRange_maximum;
	int mbRange_valid_minimum;
	int mbRange_valid_maximum;
	int mbRange_missing_value;
	char mbSoundingBias_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSoundingBias_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSoundingBias_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSoundingBias_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSoundingBias_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSoundingBias_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSoundingBias_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	double mbSoundingBias_add_offset;
	double mbSoundingBias_scale_factor;
	double mbSoundingBias_minimum;
	double mbSoundingBias_maximum;
	int mbSoundingBias_valid_minimum;
	int mbSoundingBias_valid_maximum;
	int mbSoundingBias_missing_value;
	char mbSQuality_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSQuality_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSQuality_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSQuality_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSQuality_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSQuality_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSQuality_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbSQuality_add_offset;
	int mbSQuality_scale_factor;
	int mbSQuality_minimum;
	int mbSQuality_maximum;
	int mbSQuality_valid_minimum;
	int mbSQuality_valid_maximum;
	int mbSQuality_missing_value;
	char mbReflectivity_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbReflectivity_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbReflectivity_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbReflectivity_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbReflectivity_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbReflectivity_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbReflectivity_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	double mbReflectivity_add_offset;
	double mbReflectivity_scale_factor;
	double mbReflectivity_minimum;
	double mbReflectivity_maximum;
	int mbReflectivity_valid_minimum;
	int mbReflectivity_valid_maximum;
	int mbReflectivity_missing_value;
	char mbReceptionHeave_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbReceptionHeave_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbReceptionHeave_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbReceptionHeave_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbReceptionHeave_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbReceptionHeave_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbReceptionHeave_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	double mbReceptionHeave_add_offset;
	double mbReceptionHeave_scale_factor;
	double mbReceptionHeave_minimum;
	double mbReceptionHeave_maximum;
	int mbReceptionHeave_valid_minimum;
	int mbReceptionHeave_valid_maximum;
	int mbReceptionHeave_missing_value;
	char mbAlongSlope_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAlongSlope_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAlongSlope_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAlongSlope_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAlongSlope_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAlongSlope_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAlongSlope_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	double mbAlongSlope_add_offset;
	double mbAlongSlope_scale_factor;
	double mbAlongSlope_minimum;
	double mbAlongSlope_maximum;
	int mbAlongSlope_valid_minimum;
	int mbAlongSlope_valid_maximum;
	int mbAlongSlope_missing_value;
	char mbAcrossSlope_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAcrossSlope_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAcrossSlope_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAcrossSlope_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAcrossSlope_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAcrossSlope_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAcrossSlope_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	double mbAcrossSlope_add_offset;
	double mbAcrossSlope_scale_factor;
	double mbAcrossSlope_minimum;
	double mbAcrossSlope_maximum;
	int mbAcrossSlope_valid_minimum;
	int mbAcrossSlope_valid_maximum;
	int mbAcrossSlope_missing_value;
	char mbSFlag_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSFlag_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSFlag_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSFlag_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSFlag_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSFlag_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSFlag_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbSFlag_add_offset;
	int mbSFlag_scale_factor;
	int mbSFlag_minimum;
	int mbSFlag_maximum;
	int mbSFlag_valid_minimum;
	int mbSFlag_valid_maximum;
	int mbSFlag_missing_value;
	char mbSLengthOfDetection_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSLengthOfDetection_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSLengthOfDetection_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSLengthOfDetection_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSLengthOfDetection_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSLengthOfDetection_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbSLengthOfDetection_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbSLengthOfDetection_add_offset;
	int mbSLengthOfDetection_scale_factor;
	int mbSLengthOfDetection_minimum;
	int mbSLengthOfDetection_maximum;
	int mbSLengthOfDetection_valid_minimum;
	int mbSLengthOfDetection_valid_maximum;
	int mbSLengthOfDetection_missing_value;
	char mbAntenna_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAntenna_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAntenna_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAntenna_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAntenna_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAntenna_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAntenna_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbAntenna_add_offset;
	int mbAntenna_scale_factor;
	int mbAntenna_minimum;
	int mbAntenna_maximum;
	int mbAntenna_valid_minimum;
	int mbAntenna_valid_maximum;
	int mbAntenna_missing_value;
	char mbBeamBias_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbBeamBias_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbBeamBias_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbBeamBias_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbBeamBias_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbBeamBias_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbBeamBias_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	double mbBeamBias_add_offset;
	double mbBeamBias_scale_factor;
	int mbBeamBias_minimum;
	int mbBeamBias_maximum;
	int mbBeamBias_valid_minimum;
	int mbBeamBias_valid_maximum;
	int mbBeamBias_missing_value;
	char mbBFlag_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbBFlag_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbBFlag_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbBFlag_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbBFlag_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbBFlag_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbBFlag_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbBFlag_add_offset;
	int mbBFlag_scale_factor;
	int mbBFlag_minimum;
	int mbBFlag_maximum;
	int mbBFlag_valid_minimum;
	int mbBFlag_valid_maximum;
	int mbBFlag_missing_value;
	char mbBeam_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbBeam_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbBeam_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbBeam_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbBeam_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbBeam_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbBeam_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbBeam_add_offset;
	int mbBeam_scale_factor;
	int mbBeam_minimum;
	int mbBeam_maximum;
	int mbBeam_valid_minimum;
	int mbBeam_valid_maximum;
	int mbBeam_missing_value;
	char mbAFlag_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAFlag_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAFlag_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAFlag_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAFlag_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAFlag_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbAFlag_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbAFlag_add_offset;
	int mbAFlag_scale_factor;
	int mbAFlag_minimum;
	int mbAFlag_maximum;
	int mbAFlag_valid_minimum;
	int mbAFlag_valid_maximum;
	int mbAFlag_missing_value;
	char mbVelProfilRef_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbVelProfilRef_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbVelProfilRef_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbVelProfilIdx_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbVelProfilIdx_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbVelProfilIdx_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbVelProfilIdx_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbVelProfilIdx_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbVelProfilIdx_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbVelProfilIdx_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbVelProfilIdx_add_offset;
	int mbVelProfilIdx_scale_factor;
	int mbVelProfilIdx_minimum;
	int mbVelProfilIdx_maximum;
	int mbVelProfilIdx_valid_minimum;
	int mbVelProfilIdx_valid_maximum;
	int mbVelProfilIdx_missing_value;
	char mbVelProfilDate_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbVelProfilDate_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbVelProfilDate_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbVelProfilDate_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbVelProfilDate_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbVelProfilDate_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbVelProfilDate_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbVelProfilDate_add_offset;
	int mbVelProfilDate_scale_factor;
	int mbVelProfilDate_minimum;
	int mbVelProfilDate_maximum;
	int mbVelProfilDate_valid_minimum;
	int mbVelProfilDate_valid_maximum;
	int mbVelProfilDate_missing_value;
	char mbVelProfilTime_type[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbVelProfilTime_long_name[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbVelProfilTime_name_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbVelProfilTime_units[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbVelProfilTime_unit_code[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbVelProfilTime_format_C[MBSYS_NETCDF_ATTRIBUTELEN];
	char mbVelProfilTime_orientation[MBSYS_NETCDF_ATTRIBUTELEN];
	int mbVelProfilTime_add_offset;
	int mbVelProfilTime_scale_factor;
	int mbVelProfilTime_minimum;
	int mbVelProfilTime_maximum;
	int mbVelProfilTime_valid_minimum;
	int mbVelProfilTime_valid_maximum;
	int mbVelProfilTime_missing_value;

	/* storage comment string */
	char	comment[MBSYS_NETCDF_COMMENTLEN];
	};

/* system specific function prototypes */
int mbsys_netcdf_alloc(int verbose, void *mbio_ptr, void **store_ptr,
			int *error);
int mbsys_netcdf_deall(int verbose, void *mbio_ptr, void **store_ptr,
			int *error);
int mbsys_netcdf_dimensions(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_netcdf_extract(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp,
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_netcdf_insert(int verbose, void *mbio_ptr, void *store_ptr,
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp,
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_netcdf_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles,
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset,
			double *draft, double *ssv, int *error);
int mbsys_netcdf_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transducer_depth, double *altitude,
			int *error);
int mbsys_netcdf_insert_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			double transducer_depth, double altitude,
			int *error);
int mbsys_netcdf_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft,
			double *roll, double *pitch, double *heave,
			int *error);
int mbsys_netcdf_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft,
			double roll, double pitch, double heave,
			int *error);
int mbsys_netcdf_extract_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind,
			int *nsvp,
			double *depth, double *velocity,
			int *error);
int mbsys_netcdf_insert_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int nsvp,
			double *depth, double *velocity,
			int *error);
int mbsys_netcdf_copy(int verbose, void *mbio_ptr,
			void *store_ptr, void *copy_ptr,
			int *error);

