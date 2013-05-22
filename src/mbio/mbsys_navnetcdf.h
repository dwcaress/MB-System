/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_navnetcdf.h	5/4/2002
 *	$Id$
 *
 *    Copyright (c) 2002-2013 by
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
 * mbsys_navnetcdf.h defines the data structures used by MBIO functions
 * to store data from the IFREMER netCDF navigation format.
 * The MBIO format id is:
 *      MBF_NVNETCDF : MBIO ID 167
 *
 *
 * Author:	D. W. Caress
 * Date:	May 4, 2002
 *
 * $Log: mbsys_navnetcdf.h,v $
 * Revision 5.3  2008/05/16 22:56:24  caress
 * Release 5.1.1beta18.
 *
 * Revision 5.2  2005/11/05 00:48:03  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.1  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.0  2002/05/29 23:41:20  caress
 * Release 5.0.beta18
 *
 *
 *
 */
/*
 * Notes on the MBF_MBNAVNETCDF data format:
 *   1.
 *   2.
 *
 */

/* dimension lengths */
#define MBSYS_NAVNETCDF_COMMENTLEN	    256
#define MBSYS_NAVNETCDF_ATTRIBUTELEN   64
#define MBSYS_NAVNETCDF_NAMELEN	    20

/* seconds in day */
#define SECINDAY     86400.0

/* internal data structure */
struct mbsys_navnetcdf_struct
	{
	int	 kind;

	/* global attributes */
	short mbVersion;
	char mbName[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbClasse[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbTimeReference[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbMeridian180[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbGeoDictionnary[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbGeoRepresentation[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbGeodesicSystem[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbEllipsoidName[MBSYS_NAVNETCDF_COMMENTLEN];
	char mbProjParameterCode[MBSYS_NAVNETCDF_COMMENTLEN];
	char mbShip[MBSYS_NAVNETCDF_COMMENTLEN];
	char mbSurvey[MBSYS_NAVNETCDF_COMMENTLEN];
	char mbReference[MBSYS_NAVNETCDF_COMMENTLEN];
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
	int mbPointCounter;

	/* dimensions */
	size_t mbHistoryRecNbr;
	size_t mbNameLength;
	size_t mbCommentLength;
	size_t mbPositionNbr;

	/* variable ids */
	int mbHistDate_id;
	int mbHistTime_id;
	int mbHistCode_id;
	int mbHistAutor_id;
	int mbHistModule_id;
	int mbHistComment_id;
	int mbDate_id;
	int mbTime_id;
	int mbOrdinate_id;
	int mbAbscissa_id;
	int mbAltitude_id;
	int mbImmersion_id;
	int mbHeading_id;
	int mbSpeed_id;
	int mbPType_id;
	int mbPQuality_id;
	int mbPFlag_id;

	/* variables */
        int *mbHistDate;
        int *mbHistTime;
        char *mbHistCode;
        char *mbHistAutor;
        char *mbHistModule;
        char *mbHistComment;
        int mbDate;
        int mbTime;
        int mbOrdinate;
        int mbAbscissa;
	short mbAltitude;
	short mbImmersion;
	int mbHeading;
	short mbSpeed;
	char mbPType;
	char mbPQuality;
	char mbPFlag;

	/* variable attributes */
	char mbHistDate_type[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHistDate_long_name[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHistDate_name_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHistDate_units[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHistDate_unit_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHistDate_format_C[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHistDate_orientation[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	int mbHistDate_add_offset;
	int mbHistDate_scale_factor;
	int mbHistDate_minimum;
	int mbHistDate_maximum;
	int mbHistDate_valid_minimum;
	int mbHistDate_valid_maximum;
	int mbHistDate_missing_value;
	char mbHistTime_type[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHistTime_long_name[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHistTime_name_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHistTime_units[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHistTime_unit_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHistTime_format_C[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHistTime_orientation[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	int mbHistTime_add_offset;
	int mbHistTime_scale_factor;
	int mbHistTime_minimum;
	int mbHistTime_maximum;
	int mbHistTime_valid_minimum;
	int mbHistTime_valid_maximum;
	int mbHistTime_missing_value;
	char mbHistCode_type[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHistCode_long_name[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHistCode_name_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHistCode_units[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHistCode_unit_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHistCode_format_C[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHistCode_orientation[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	int mbHistCode_add_offset;
	int mbHistCode_scale_factor;
	int mbHistCode_minimum;
	int mbHistCode_maximum;
	int mbHistCode_valid_minimum;
	int mbHistCode_valid_maximum;
	int mbHistCode_missing_value;
	char mbHistAutor_type[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHistAutor_long_name[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHistAutor_name_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHistModule_type[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHistModule_long_name[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHistModule_name_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHistComment_type[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHistComment_long_name[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHistComment_name_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbDate_type[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbDate_long_name[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbDate_name_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbDate_units[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbDate_unit_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbDate_format_C[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbDate_orientation[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	int mbDate_add_offset;
	int mbDate_scale_factor;
	int mbDate_minimum;
	int mbDate_maximum;
	int mbDate_valid_minimum;
	int mbDate_valid_maximum;
	int mbDate_missing_value;
	char mbTime_type[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbTime_long_name[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbTime_name_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbTime_units[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbTime_unit_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbTime_format_C[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbTime_orientation[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	int mbTime_add_offset;
	int mbTime_scale_factor;
	int mbTime_minimum;
	int mbTime_maximum;
	int mbTime_valid_minimum;
	int mbTime_valid_maximum;
	int mbTime_missing_value;
	char mbOrdinate_type[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbOrdinate_long_name[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbOrdinate_name_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbOrdinate_units[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbOrdinate_unit_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbOrdinate_format_C[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbOrdinate_orientation[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	double mbOrdinate_add_offset;
	double mbOrdinate_scale_factor;
	int mbOrdinate_minimum;
	int mbOrdinate_maximum;
	int mbOrdinate_valid_minimum;
	int mbOrdinate_valid_maximum;
	int mbOrdinate_missing_value;
	char mbAbscissa_type[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbAbscissa_long_name[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbAbscissa_name_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbAbscissa_units[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbAbscissa_unit_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbAbscissa_format_C[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbAbscissa_orientation[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	double mbAbscissa_add_offset;
	double mbAbscissa_scale_factor;
	int mbAbscissa_minimum;
	int mbAbscissa_maximum;
	int mbAbscissa_valid_minimum;
	int mbAbscissa_valid_maximum;
	int mbAbscissa_missing_value;
	char mbAltitude_type[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbAltitude_long_name[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbAltitude_name_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbAltitude_units[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbAltitude_unit_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbAltitude_format_C[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbAltitude_orientation[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	double mbAltitude_add_offset;
	double mbAltitude_scale_factor;
	int mbAltitude_minimum;
	int mbAltitude_maximum;
	int mbAltitude_valid_minimum;
	int mbAltitude_valid_maximum;
	int mbAltitude_missing_value;
	char mbImmersion_type[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbImmersion_long_name[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbImmersion_name_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbImmersion_units[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbImmersion_unit_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbImmersion_format_C[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbImmersion_orientation[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	double mbImmersion_add_offset;
	double mbImmersion_scale_factor;
	int mbImmersion_minimum;
	int mbImmersion_maximum;
	int mbImmersion_valid_minimum;
	int mbImmersion_valid_maximum;
	int mbImmersion_missing_value;
	char mbHeading_type[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHeading_long_name[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHeading_name_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHeading_units[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHeading_unit_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHeading_format_C[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbHeading_orientation[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	double mbHeading_add_offset;
	double mbHeading_scale_factor;
	int mbHeading_minimum;
	int mbHeading_maximum;
	int mbHeading_valid_minimum;
	int mbHeading_valid_maximum;
	int mbHeading_missing_value;
	char mbSpeed_type[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbSpeed_long_name[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbSpeed_name_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbSpeed_units[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbSpeed_unit_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbSpeed_format_C[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbSpeed_orientation[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	double mbSpeed_add_offset;
	double mbSpeed_scale_factor;
	int mbSpeed_minimum;
	int mbSpeed_maximum;
	int mbSpeed_valid_minimum;
	int mbSpeed_valid_maximum;
	int mbSpeed_missing_value;
	char mbPType_type[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbPType_long_name[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbPType_name_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbPType_units[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbPType_unit_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbPType_format_C[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbPType_orientation[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	int mbPType_add_offset;
	int mbPType_scale_factor;
	int mbPType_minimum;
	int mbPType_maximum;
	int mbPType_valid_minimum;
	int mbPType_valid_maximum;
	int mbPType_missing_value;
	char mbPQuality_type[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbPQuality_long_name[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbPQuality_name_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbPQuality_units[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbPQuality_unit_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbPQuality_format_C[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbPQuality_orientation[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	int mbPQuality_add_offset;
	int mbPQuality_scale_factor;
	int mbPQuality_minimum;
	int mbPQuality_maximum;
	int mbPQuality_valid_minimum;
	int mbPQuality_valid_maximum;
	int mbPQuality_missing_value;
	char mbPFlag_type[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbPFlag_long_name[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbPFlag_name_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbPFlag_units[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbPFlag_unit_code[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbPFlag_format_C[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	char mbPFlag_orientation[MBSYS_NAVNETCDF_ATTRIBUTELEN];
	int mbPFlag_add_offset;
	int mbPFlag_scale_factor;
	int mbPFlag_minimum;
	int mbPFlag_maximum;
	int mbPFlag_valid_minimum;
	int mbPFlag_valid_maximum;
	int mbPFlag_missing_value;

	/* storage comment string */
	char	comment[MBSYS_NAVNETCDF_COMMENTLEN];
	};

/* system specific function prototypes */
int mbsys_navnetcdf_alloc(int verbose, void *mbio_ptr, void **store_ptr,
			int *error);
int mbsys_navnetcdf_deall(int verbose, void *mbio_ptr, void **store_ptr,
			int *error);
int mbsys_navnetcdf_dimensions(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_navnetcdf_extract(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp,
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_navnetcdf_insert(int verbose, void *mbio_ptr, void *store_ptr,
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp,
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_navnetcdf_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles,
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset,
			double *draft, double *ssv, int *error);
int mbsys_navnetcdf_detects(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams, int *detects, int *error);
int mbsys_navnetcdf_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transducer_depth, double *altitude,
			int *error);
int mbsys_navnetcdf_insert_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			double transducer_depth, double altitude,
			int *error);
int mbsys_navnetcdf_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft,
			double *roll, double *pitch, double *heave,
			int *error);
int mbsys_navnetcdf_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft,
			double roll, double pitch, double heave,
			int *error);
int mbsys_navnetcdf_extract_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind,
			int *nsvp,
			double *depth, double *velocity,
			int *error);
int mbsys_navnetcdf_insert_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int nsvp,
			double *depth, double *velocity,
			int *error);
int mbsys_navnetcdf_copy(int verbose, void *mbio_ptr,
			void *store_ptr, void *copy_ptr,
			int *error);
