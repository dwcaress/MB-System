/* Added HAVE_CONFIG_H for autogen files */
#ifdef HAVE_CONFIG_H
#  include <mbsystem_config.h>
#endif


/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_navnetcdf.c	4/11/2002
 *	$Id: mbsys_navnetcdf.c 1907 2011-11-10 04:33:03Z caress $
 *
 *    Copyright (c) 2002-2011 by
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
 * mbsys_navnetcdf.c includes the functions used by MBIO functions
 * to extract and insert data from the IFREMER netCDF multibeam format.
 * The MBIO format id is:
 *      MBF_NVNETCDF : MBIO ID 167
 *
 * Author:	D. W. Caress
 * Date:	April 11, 2002
 *
 * $Log: mbsys_navnetcdf.c,v $
 * Revision 5.8  2008/09/13 06:08:09  caress
 * Updates to apply suggested patches to segy handling. Also fixes to remove compiler warnings.
 *
 * Revision 5.7  2008/07/10 18:02:39  caress
 * Proceeding towards 5.1.1beta20.
 *
 * Revision 5.4  2008/05/16 22:56:24  caress
 * Release 5.1.1beta18.
 *
 * Revision 5.3  2005/11/05 00:48:05  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.2  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.1  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.0  2002/05/29 23:40:15  caress
 * Release 5.0.beta18
 *
*
 */

/* standard include files */
#include <stdio.h>
#include <math.h>
#include <string.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_define.h"
#include "mbsys_navnetcdf.h"

static char rcs_id[]="$Id: mbsys_navnetcdf.c 1907 2011-11-10 04:33:03Z caress $";

/*--------------------------------------------------------------------*/
int mbsys_navnetcdf_alloc(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error)
{
	char	*function_name = "mbsys_navnetcdf_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_navnetcdf_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* allocate memory for data structure */
	status = mb_mallocd(verbose,__FILE__,__LINE__,sizeof(struct mbsys_navnetcdf_struct),
				store_ptr,error);

	/* get data structure pointer */
	store = (struct mbsys_navnetcdf_struct *) *store_ptr;
	
	/* now initialize everything */
	if (status == MB_SUCCESS)
	    {	
	    /* dimensions */
	    store->mbHistoryRecNbr = 0 ;
	    store->mbNameLength = MBSYS_NAVNETCDF_NAMELEN ;
	    store->mbCommentLength = MBSYS_NAVNETCDF_COMMENTLEN ;
	    store->mbPositionNbr = 0 ;
    
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
	    strcpy(store->mbShip, "                                                                                                                                                                                                                                                                ");
	    strcpy(store->mbSurvey, "                                                                                                                                                                                                                                                                ");
	    strcpy(store->mbReference, "                                                                                                                                                                                                                                                                ");
	    store->mbPointCounter = 0;
	    
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
	    store->mbHistTime_missing_value = -2000000000 - 147483648;
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
	    strcpy(store->mbDate_type, "integer");
	    strcpy(store->mbDate_long_name, "Date of cycle");
	    strcpy(store->mbDate_name_code, "MB_POSITION_DATE");
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
	    strcpy(store->mbTime_name_code, "MB_POSITION_TIME");
	    strcpy(store->mbTime_units, "ms");
	    strcpy(store->mbTime_unit_code, "MB_MS");
	    store->mbTime_add_offset = 0;
	    store->mbTime_scale_factor = 1;
	    store->mbTime_minimum = 0;
	    store->mbTime_maximum = 86399999;
	    store->mbTime_valid_minimum = 0;
	    store->mbTime_valid_maximum = 86399999;
	    store->mbTime_missing_value = -2000000000 - 147483648;
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
	    store->mbOrdinate_missing_value = -2000000000 - 147483648;
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
	    store->mbAbscissa_missing_value = -2000000000 - 147483648;
	    strcpy(store->mbAbscissa_format_C, "%f");
	    strcpy(store->mbAbscissa_orientation, "direct");
	    strcpy(store->mbAltitude_type, "real");
	    strcpy(store->mbAltitude_long_name, "Altitude");
	    strcpy(store->mbAltitude_name_code, "MB_POSITION_ALTITUDE");
	    strcpy(store->mbAltitude_units, "m");
	    strcpy(store->mbAltitude_unit_code, "MB_M");
	    store->mbAltitude_add_offset = 0. ;
	    store->mbAltitude_scale_factor = 0.2 ;
	    store->mbAltitude_minimum = -32767 ;
	    store->mbAltitude_maximum = 32767 ;
	    store->mbAltitude_valid_minimum = -32767 ;
	    store->mbAltitude_valid_maximum = 32767 ;
	    store->mbAltitude_missing_value = -32768 ;
	    strcpy(store->mbAltitude_format_C, "%.2f");
	    strcpy(store->mbAltitude_orientation, "direct");
	    strcpy(store->mbImmersion_type, "real");
	    strcpy(store->mbImmersion_long_name, "Immersion");
	    strcpy(store->mbImmersion_name_code, "MB_POSITION_IMMERSION");
	    strcpy(store->mbImmersion_units, "m");
	    strcpy(store->mbImmersion_unit_code, "MB_M");
	    store->mbImmersion_add_offset = 0. ;
	    store->mbImmersion_scale_factor = 0.2 ;
	    store->mbImmersion_minimum = -32767 ;
	    store->mbImmersion_maximum = 32767 ;
	    store->mbImmersion_valid_minimum = -32767 ;
	    store->mbImmersion_valid_maximum = 32767 ;
	    store->mbImmersion_missing_value = -32768 ;
	    strcpy(store->mbImmersion_format_C, "%.2f");
	    strcpy(store->mbImmersion_orientation, "direct");
	    strcpy(store->mbHeading_type, "real");
	    strcpy(store->mbHeading_long_name, "Ship heading");
	    strcpy(store->mbHeading_name_code, "MB_POSITION_HEADING");
	    strcpy(store->mbHeading_units, "degree");
	    strcpy(store->mbHeading_unit_code, "MB_DEGREE");
	    store->mbHeading_add_offset = 0. ;
	    store->mbHeading_scale_factor = 0.01 ;
	    store->mbHeading_minimum = 1 ;
	    store->mbHeading_maximum = 35999 ;
	    store->mbHeading_valid_minimum = 1 ;
	    store->mbHeading_valid_maximum = 35999 ;
	    store->mbHeading_missing_value = 65535 ;
	    strcpy(store->mbHeading_format_C, "%.2f");
	    strcpy(store->mbHeading_orientation, "direct");
	    strcpy(store->mbSpeed_type, "real");
	    strcpy(store->mbSpeed_long_name, "Vessel speed");
	    strcpy(store->mbSpeed_name_code, "MB_POSITION_SPEED");
	    strcpy(store->mbSpeed_units, "knot");
	    strcpy(store->mbSpeed_unit_code, "MB_KNOT");
	    store->mbSpeed_add_offset = 0. ;
	    store->mbSpeed_scale_factor = 0.01 ;
	    store->mbSpeed_minimum = -32767 ;
	    store->mbSpeed_maximum = 32767 ;
	    store->mbSpeed_valid_minimum = -32767 ;
	    store->mbSpeed_valid_maximum = 32767 ;
	    store->mbSpeed_missing_value = -32768 ;
	    strcpy(store->mbSpeed_format_C, "%.2f");
	    strcpy(store->mbSpeed_orientation, "direct");
	    strcpy(store->mbPType_type, "integer");
	    strcpy(store->mbPType_long_name, "PType of captor");
	    strcpy(store->mbPType_name_code, "MB_POSITION_TYPE");
	    strcpy(store->mbPType_units, "");
	    strcpy(store->mbPType_unit_code, "MB_NOT_DEFINED");
	    store->mbPType_add_offset = 0 ;
	    store->mbPType_scale_factor = 1 ;
	    store->mbPType_minimum = 1 ;
	    store->mbPType_maximum = 255 ;
	    store->mbPType_valid_minimum = 1 ;
	    store->mbPType_valid_maximum = 255 ;
	    store->mbPType_missing_value = 0 ;
	    strcpy(store->mbPType_format_C, "%d");
	    strcpy(store->mbPType_orientation, "direct");
	    strcpy(store->mbPQuality_type, "integer");
	    strcpy(store->mbPQuality_long_name, "Factor of quality    ");
	    strcpy(store->mbPQuality_name_code, "MB_POSITION_QUALITY");
	    strcpy(store->mbPQuality_units, "");
	    strcpy(store->mbPQuality_unit_code, "MB_NOT_DEFINED");
	    store->mbPQuality_add_offset = 0 ;
	    store->mbPQuality_scale_factor = 1 ;
	    store->mbPQuality_minimum = 1 ;
	    store->mbPQuality_maximum = 255 ;
	    store->mbPQuality_valid_minimum = 1 ;
	    store->mbPQuality_valid_maximum = 255 ;
	    store->mbPQuality_missing_value = 0 ;
	    strcpy(store->mbPQuality_format_C, "%d");
	    strcpy(store->mbPQuality_orientation, "direct");
	    strcpy(store->mbPFlag_type, "integer");
	    strcpy(store->mbPFlag_long_name, "Flag of position   ");
	    strcpy(store->mbPFlag_name_code, "MB_POSITION_Flag");
	    strcpy(store->mbPFlag_units, "");
	    strcpy(store->mbPFlag_unit_code, "MB_NOT_DEFINED");
	    store->mbPFlag_add_offset = 0 ;
	    store->mbPFlag_scale_factor = 1 ;
	    store->mbPFlag_minimum = -127 ;
	    store->mbPFlag_maximum = 127 ;
	    store->mbPFlag_valid_minimum = -127 ;
	    store->mbPFlag_valid_maximum = 127 ;
	    store->mbPFlag_missing_value = -128 ;
	    strcpy(store->mbPFlag_format_C, "%d");
	    strcpy(store->mbPFlag_orientation, "direct");
    
	    /* variable ids */
	    store->mbHistDate_id = -1;
	    store->mbHistTime_id = -1;
	    store->mbHistCode_id = -1;
	    store->mbHistAutor_id = -1;
	    store->mbHistModule_id = -1;
	    store->mbHistComment_id = -1;
	    store->mbDate_id = -1;
	    store->mbTime_id = -1;
	    store->mbOrdinate_id = -1;
	    store->mbAbscissa_id = -1;
	    store->mbAltitude_id = -1;
	    store->mbImmersion_id = -1;
	    store->mbHeading_id = -1;
	    store->mbSpeed_id = -1;
	    store->mbPType_id = -1;
	    store->mbPQuality_id = -1;
	    store->mbPFlag_id = -1;
    
	    /* variable pointers */
	    store->mbHistDate = NULL;
	    store->mbHistTime = NULL;
	    store->mbHistCode = NULL;
	    store->mbHistAutor = NULL;
	    store->mbHistModule = NULL;
	    store->mbHistComment = NULL;
	    }

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       store_ptr:  %d\n",*error);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_navnetcdf_deall(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error)
{
	char	*function_name = "mbsys_navnetcdf_deall";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_navnetcdf_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %d\n",*error);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_navnetcdf_struct *) *store_ptr;

	/* deallocate any allocated arrays */
	if (store->mbHistDate != NULL)
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->mbHistDate, error);
	if (store->mbHistTime != NULL)
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->mbHistTime, error);
	if (store->mbHistCode != NULL)
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->mbHistCode, error);
	if (store->mbHistAutor != NULL)
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->mbHistAutor, error);
	if (store->mbHistModule != NULL)
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->mbHistModule, error);
	if (store->mbHistComment != NULL)
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->mbHistComment, error);

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
int mbsys_navnetcdf_dimensions(int verbose, void *mbio_ptr, void *store_ptr, 
		int *kind, int *nbath, int *namp, int *nss, int *error)
{
	char	*function_name = "mbsys_navnetcdf_dimensions";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_navnetcdf_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_navnetcdf_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get beam and pixel numbers */
		*nbath = 0;
		*namp = 0;
		*nss = 0;
		}

	/* extract data from structure */
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
int mbsys_navnetcdf_extract(int verbose, void *mbio_ptr, void *store_ptr, 
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_navnetcdf_extract";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_navnetcdf_struct *store;
	int	i;
	
	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_navnetcdf_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;
	
	/* reset error and status */
	*error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get time */
		*time_d = store->mbDate * SECINDAY
			    + store->mbTime * 0.001;
		mb_get_date(verbose,*time_d,time_i);

		/* get navigation */
		*navlon = (double) store->mbAbscissa_scale_factor * store->mbAbscissa;
		*navlat = (double) store->mbOrdinate_scale_factor * store->mbOrdinate;

		/* get heading */
		*heading = store->mbHeading * store->mbHeading_scale_factor;

		/* get speed */
		*speed = store->mbSpeed * store->mbSpeed_scale_factor;
			
		/* set beamwidths in mb_io structure */
		mb_io_ptr->beamwidth_ltrack = 2.0;
		mb_io_ptr->beamwidth_xtrack = 2.0;

		/* read distance, depth, and backscatter 
			values into storage arrays */
		*nbath = 0;
		*namp = 0;
		*nss = 0;

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
				*error);
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
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
int mbsys_navnetcdf_insert(int verbose, void *mbio_ptr, void *store_ptr, 
		int kind, int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading,
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp, 
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_navnetcdf_insert";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_navnetcdf_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
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
	if (verbose >= 2 && kind == MB_DATA_COMMENT)
		{
		fprintf(stderr,"dbg2       comment:     \ndbg2       %s\n",
			comment);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_navnetcdf_struct *) store_ptr;

	/* set data kind */
	store->kind = kind;
	
	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* get stuff */
		/* get time */
		store->mbDate = (int)(time_d / SECINDAY);
		store->mbTime = (int)(1000 * (time_d - store->mbDate * SECINDAY));
    
		/* get navigation */
		store->mbAbscissa = (int)(navlon / store->mbAbscissa_scale_factor);
		store->mbOrdinate = (int)(navlat / store->mbOrdinate_scale_factor);
    
		/* get heading */
		store->mbHeading = heading / store->mbHeading_scale_factor;
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
int mbsys_navnetcdf_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams,
	double *ttimes, double *angles, 
	double *angles_forward, double *angles_null,
	double *heave, double *alongtrack_offset, 
	double *draft, double *ssv, int *error)
{
	char	*function_name = "mbsys_navnetcdf_ttimes";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_navnetcdf_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		fprintf(stderr,"dbg2       ttimes:     %lu\n",(size_t)ttimes);
		fprintf(stderr,"dbg2       angles_xtrk:%lu\n",(size_t)angles);
		fprintf(stderr,"dbg2       angles_ltrk:%lu\n",(size_t)angles_forward);
		fprintf(stderr,"dbg2       angles_null:%lu\n",(size_t)angles_null);
		fprintf(stderr,"dbg2       heave:      %lu\n",(size_t)heave);
		fprintf(stderr,"dbg2       ltrk_off:   %lu\n",(size_t)alongtrack_offset);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_navnetcdf_struct *) store_ptr;

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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
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
int mbsys_navnetcdf_detects(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams,
	int *detects, int *error)
{
	char	*function_name = "mbsys_navnetcdf_detects";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_navnetcdf_struct *store;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		fprintf(stderr,"dbg2       detects:    %lu\n",(size_t)detects);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_navnetcdf_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get nbeams */
		*nbeams = 0;

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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:       %d\n",*kind);
		}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR)
		{
		fprintf(stderr,"dbg2       nbeams:     %d\n",*nbeams);
		for (i=0;i<*nbeams;i++)
			fprintf(stderr,"dbg2       beam %d: detects:%d\n",
				i,detects[i]);
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
int mbsys_navnetcdf_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, double *transducer_depth, double *altitude, 
	int *error)
{
	char	*function_name = "mbsys_navnetcdf_extract_altitude";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_navnetcdf_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_navnetcdf_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;
	
	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* set values */
		*transducer_depth = store->mbImmersion * store->mbImmersion_scale_factor;
		*altitude = store->mbAltitude * store->mbAltitude_scale_factor;

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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
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
int mbsys_navnetcdf_insert_altitude(int verbose, void *mbio_ptr, void *store_ptr,
	double transducer_depth, double altitude, 
	int *error)
{
	char	*function_name = "mbsys_navnetcdf_insert_altitude";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_navnetcdf_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:            %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:         %lu\n",(size_t)store_ptr);
		fprintf(stderr,"dbg2       transducer_depth:  %f\n",transducer_depth);
		fprintf(stderr,"dbg2       altitude:          %f\n",altitude);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_navnetcdf_struct *) store_ptr;

	/* insert data into structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* set values */
		store->mbImmersion = (int)(transducer_depth / store->mbImmersion_scale_factor);
		store->mbAltitude = (int)(altitude / store->mbAltitude_scale_factor);

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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       error:             %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:            %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_navnetcdf_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading, double *draft, 
		double *roll, double *pitch, double *heave, 
		int *error)
{
	char	*function_name = "mbsys_navnetcdf_extract_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_navnetcdf_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_navnetcdf_struct *) store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get time */
		*time_d = store->mbDate * SECINDAY
			    + store->mbTime * 0.001;
		mb_get_date(verbose,*time_d,time_i);

		/* get navigation */
		*navlon = (double) store->mbAbscissa_scale_factor * store->mbAbscissa;
		*navlat = (double) store->mbOrdinate_scale_factor * store->mbOrdinate;

		/* get heading */
		*heading = store->mbHeading * store->mbHeading_scale_factor;

		/* set speed to zero */
		*speed = store->mbSpeed * store->mbSpeed_scale_factor;

		/* set draft to zero */
		*draft = store->mbImmersion * store->mbImmersion_scale_factor;

		/* get roll pitch and heave */
		*roll = 0.0;
		*pitch = 0.0;
		*heave = 0.0;

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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
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
int mbsys_navnetcdf_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading, double draft, 
		double roll, double pitch, double heave,
		int *error)
{
	char	*function_name = "mbsys_navnetcdf_insert_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_navnetcdf_struct *store;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
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
	store = (struct mbsys_navnetcdf_struct *) store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{
		/* get time */
		store->mbDate = (int)(time_d / SECINDAY);
		store->mbTime = (int)(1000 * (time_d - store->mbDate * SECINDAY));

		/* get navigation */
		store->mbAbscissa = (int)(navlon / store->mbAbscissa_scale_factor);
		store->mbOrdinate = (int)(navlat / store->mbOrdinate_scale_factor);

		/* get heading */
		store->mbHeading = heading / store->mbHeading_scale_factor;

		/* get speed */
		store->mbSpeed = speed / store->mbSpeed_scale_factor;
		
		/* get draft */
		store->mbImmersion = draft / store->mbImmersion_scale_factor;

		/* get roll pitch and heave */
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
int mbsys_navnetcdf_copy(int verbose, void *mbio_ptr, 
			void *store_ptr, void *copy_ptr,
			int *error)
{
	char	*function_name = "mbsys_navnetcdf_copy";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_navnetcdf_struct *store;
	struct mbsys_navnetcdf_struct *copy;
	int     i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %lu\n",(size_t)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %lu\n",(size_t)store_ptr);
		fprintf(stderr,"dbg2       copy_ptr:   %lu\n",(size_t)copy_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointers */
	store = (struct mbsys_navnetcdf_struct *) store_ptr;
	copy = (struct mbsys_navnetcdf_struct *) copy_ptr;
	
	
	/* deallocate memory if required */
	if (store->mbHistoryRecNbr > copy->mbHistoryRecNbr)
	    {
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->mbHistDate, error);
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->mbHistTime, error);
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->mbHistCode, error);
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->mbHistAutor, error);
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->mbHistModule, error);
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->mbHistComment, error);
	    }
	
	/* allocate the memory in copy */
	if (status == MB_SUCCESS)
	    {
	    copy->mbHistoryRecNbr = store->mbHistoryRecNbr;
	    copy->mbNameLength = store->mbNameLength;
	    copy->mbCommentLength = store->mbCommentLength;
	    copy->mbPositionNbr = store->mbPositionNbr;
	    status = mb_mallocd(verbose,__FILE__,__LINE__, 
			copy->mbHistoryRecNbr * sizeof(int),
			(void **)&copy->mbHistDate,error);
	    status = mb_mallocd(verbose,__FILE__,__LINE__, 
			copy->mbHistoryRecNbr * sizeof(int),
			(void **)&copy->mbHistTime,error);
	    status = mb_mallocd(verbose,__FILE__,__LINE__, 
			copy->mbHistoryRecNbr * sizeof(char),
			(void **)&copy->mbHistCode,error);
	    status = mb_mallocd(verbose,__FILE__,__LINE__, 
			copy->mbHistoryRecNbr * copy->mbNameLength * sizeof(char),
			(void **)&copy->mbHistAutor,error);
	    status = mb_mallocd(verbose,__FILE__,__LINE__, 
			copy->mbHistoryRecNbr * copy->mbNameLength * sizeof(char),
			(void **)&copy->mbHistModule,error);
	    status = mb_mallocd(verbose,__FILE__,__LINE__, 
			copy->mbHistoryRecNbr * copy->mbCommentLength * sizeof(char),
			(void **)&copy->mbHistComment,error);
	    }

	/* deal with a memory allocation failure */
	if (status == MB_FAILURE)
	    {
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->mbHistDate, error);
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->mbHistTime, error);
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->mbHistCode, error);
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->mbHistAutor, error);
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->mbHistModule, error);
	    status = mb_freed(verbose,__FILE__, __LINE__, (void **) &store->mbHistComment, error);
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
	    for (i=0;i<MBSYS_NAVNETCDF_ATTRIBUTELEN;i++)
		{
		copy->mbName[i] = store->mbName[i];
		copy->mbClasse[i] = store->mbClasse[i];
		copy->mbTimeReference[i] = store->mbTimeReference[i];
		copy->mbMeridian180[i] = store->mbMeridian180[i];
		copy->mbGeoDictionnary[i] = store->mbGeoDictionnary[i];
		copy->mbGeoRepresentation[i] = store->mbGeoRepresentation[i];
		copy->mbGeodesicSystem[i] = store->mbGeodesicSystem[i];
		}
	    for (i=0;i<MBSYS_NAVNETCDF_ATTRIBUTELEN;i++)
		{
		copy->mbEllipsoidName[i] = store->mbEllipsoidName[i];
		copy->mbProjParameterCode[i] = store->mbProjParameterCode[i];
		copy->mbShip[i] = store->mbShip[i];
		copy->mbSurvey[i] = store->mbSurvey[i];
		copy->mbReference[i] = store->mbReference[i];
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
	    copy->mbPointCounter = store->mbPointCounter;
	    
	    /* variable ids */
	    copy->mbHistDate_id = store->mbHistDate_id;
	    copy->mbHistTime_id = store->mbHistTime_id;
	    copy->mbHistCode_id = store->mbHistCode_id;
	    copy->mbHistAutor_id = store->mbHistAutor_id;
	    copy->mbHistModule_id = store->mbHistModule_id;
	    copy->mbHistComment_id = store->mbHistComment_id;
	    copy->mbDate_id = store->mbDate_id;
	    copy->mbTime_id = store->mbTime_id;
	    copy->mbOrdinate_id = store->mbOrdinate_id;
	    copy->mbAbscissa_id = store->mbAbscissa_id;
	    copy->mbAltitude_id = store->mbAltitude_id;
	    copy->mbImmersion_id = store->mbImmersion_id;
	    copy->mbHeading_id = store->mbHeading_id;
	    copy->mbSpeed_id = store->mbSpeed_id;
	    copy->mbPType_id = store->mbPType_id;
	    copy->mbPQuality_id = store->mbPQuality_id;
	    copy->mbPFlag_id = store->mbPFlag_id;
    
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
	    copy->mbDate = store->mbDate;
	    copy->mbTime = store->mbTime;
	    copy->mbOrdinate = store->mbOrdinate;
	    copy->mbAbscissa = store->mbAbscissa;
	    copy->mbAltitude = store->mbAltitude;
	    copy->mbImmersion = store->mbImmersion;
	    copy->mbHeading = store->mbHeading;
	    copy->mbSpeed = store->mbSpeed;
	    copy->mbPType = store->mbPType;
	    copy->mbPQuality = store->mbPQuality;
	    copy->mbPFlag = store->mbPFlag;
	    
	    /* variable attributes */
	    for (i=0;i<MBSYS_NAVNETCDF_ATTRIBUTELEN;i++)
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
		copy->mbAltitude_type[i] = store->mbAltitude_type[i];
		copy->mbAltitude_long_name[i] = store->mbAltitude_long_name[i];
		copy->mbAltitude_name_code[i] = store->mbAltitude_name_code[i];
		copy->mbAltitude_units[i] = store->mbAltitude_units[i];
		copy->mbAltitude_unit_code[i] = store->mbAltitude_unit_code[i];
		copy->mbAltitude_format_C[i] = store->mbAltitude_format_C[i];
		copy->mbAltitude_orientation[i] = store->mbAltitude_orientation[i];
		copy->mbImmersion_type[i] = store->mbImmersion_type[i];
		copy->mbImmersion_long_name[i] = store->mbImmersion_long_name[i];
		copy->mbImmersion_name_code[i] = store->mbImmersion_name_code[i];
		copy->mbImmersion_units[i] = store->mbImmersion_units[i];
		copy->mbImmersion_unit_code[i] = store->mbImmersion_unit_code[i];
		copy->mbImmersion_format_C[i] = store->mbImmersion_format_C[i];
		copy->mbImmersion_orientation[i] = store->mbImmersion_orientation[i];
		copy->mbHeading_type[i] = store->mbHeading_type[i];
		copy->mbHeading_long_name[i] = store->mbHeading_long_name[i];
		copy->mbHeading_name_code[i] = store->mbHeading_name_code[i];
		copy->mbHeading_units[i] = store->mbHeading_units[i];
		copy->mbHeading_unit_code[i] = store->mbHeading_unit_code[i];
		copy->mbHeading_format_C[i] = store->mbHeading_format_C[i];
		copy->mbHeading_orientation[i] = store->mbHeading_orientation[i];
		copy->mbSpeed_type[i] = store->mbSpeed_type[i];
		copy->mbSpeed_long_name[i] = store->mbSpeed_long_name[i];
		copy->mbSpeed_name_code[i] = store->mbSpeed_name_code[i];
		copy->mbSpeed_units[i] = store->mbSpeed_units[i];
		copy->mbSpeed_unit_code[i] = store->mbSpeed_unit_code[i];
		copy->mbSpeed_format_C[i] = store->mbSpeed_format_C[i];
		copy->mbSpeed_orientation[i] = store->mbSpeed_orientation[i];
		copy->mbPType_type[i] = store->mbPType_type[i];
		copy->mbPType_long_name[i] = store->mbPType_long_name[i];
		copy->mbPType_name_code[i] = store->mbPType_name_code[i];
		copy->mbPType_units[i] = store->mbPType_units[i];
		copy->mbPType_unit_code[i] = store->mbPType_unit_code[i];
		copy->mbPType_format_C[i] = store->mbPType_format_C[i];
		copy->mbPType_orientation[i] = store->mbPType_orientation[i];
		copy->mbPQuality_type[i] = store->mbPQuality_type[i];
		copy->mbPQuality_long_name[i] = store->mbPQuality_long_name[i];
		copy->mbPQuality_name_code[i] = store->mbPQuality_name_code[i];
		copy->mbPQuality_units[i] = store->mbPQuality_units[i];
		copy->mbPQuality_unit_code[i] = store->mbPQuality_unit_code[i];
		copy->mbPQuality_format_C[i] = store->mbPQuality_format_C[i];
		copy->mbPQuality_orientation[i] = store->mbPQuality_orientation[i];
		copy->mbPFlag_type[i] = store->mbPFlag_type[i];
		copy->mbPFlag_long_name[i] = store->mbPFlag_long_name[i];
		copy->mbPFlag_name_code[i] = store->mbPFlag_name_code[i];
		copy->mbPFlag_units[i] = store->mbPFlag_units[i];
		copy->mbPFlag_unit_code[i] = store->mbPFlag_unit_code[i];
		copy->mbPFlag_format_C[i] = store->mbPFlag_format_C[i];
		copy->mbPFlag_orientation[i] = store->mbPFlag_orientation[i];
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
	    copy->mbAltitude_add_offset = store->mbAltitude_add_offset;
	    copy->mbAltitude_scale_factor = store->mbAltitude_scale_factor;
	    copy->mbAltitude_minimum = store->mbAltitude_minimum;
	    copy->mbAltitude_maximum = store->mbAltitude_maximum;
	    copy->mbAltitude_valid_minimum = store->mbAltitude_valid_minimum;
	    copy->mbAltitude_valid_maximum = store->mbAltitude_valid_maximum;
	    copy->mbAltitude_missing_value = store->mbAltitude_missing_value;
	    copy->mbImmersion_add_offset = store->mbImmersion_add_offset;
	    copy->mbImmersion_scale_factor = store->mbImmersion_scale_factor;
	    copy->mbImmersion_minimum = store->mbImmersion_minimum;
	    copy->mbImmersion_maximum = store->mbImmersion_maximum;
	    copy->mbImmersion_valid_minimum = store->mbImmersion_valid_minimum;
	    copy->mbImmersion_valid_maximum = store->mbImmersion_valid_maximum;
	    copy->mbImmersion_missing_value = store->mbImmersion_missing_value;
	    copy->mbHeading_add_offset = store->mbHeading_add_offset;
	    copy->mbHeading_scale_factor = store->mbHeading_scale_factor;
	    copy->mbHeading_minimum = store->mbHeading_minimum;
	    copy->mbHeading_maximum = store->mbHeading_maximum;
	    copy->mbHeading_valid_minimum = store->mbHeading_valid_minimum;
	    copy->mbHeading_valid_maximum = store->mbHeading_valid_maximum;
	    copy->mbHeading_missing_value = store->mbHeading_missing_value;
	    copy->mbSpeed_add_offset = store->mbSpeed_add_offset;
	    copy->mbSpeed_scale_factor = store->mbSpeed_scale_factor;
	    copy->mbSpeed_minimum = store->mbSpeed_minimum;
	    copy->mbSpeed_maximum = store->mbSpeed_maximum;
	    copy->mbSpeed_valid_minimum = store->mbSpeed_valid_minimum;
	    copy->mbSpeed_valid_maximum = store->mbSpeed_valid_maximum;
	    copy->mbSpeed_missing_value = store->mbSpeed_missing_value;
	    copy->mbPType_add_offset = store->mbPType_add_offset;
	    copy->mbPType_scale_factor = store->mbPType_scale_factor;
	    copy->mbPType_minimum = store->mbPType_minimum;
	    copy->mbPType_maximum = store->mbPType_maximum;
	    copy->mbPType_valid_minimum = store->mbPType_valid_minimum;
	    copy->mbPType_valid_maximum = store->mbPType_valid_maximum;
	    copy->mbPType_missing_value = store->mbPType_missing_value;
	    copy->mbPQuality_add_offset = store->mbPQuality_add_offset;
	    copy->mbPQuality_scale_factor = store->mbPQuality_scale_factor;
	    copy->mbPQuality_minimum = store->mbPQuality_minimum;
	    copy->mbPQuality_maximum = store->mbPQuality_maximum;
	    copy->mbPQuality_valid_minimum = store->mbPQuality_valid_minimum;
	    copy->mbPQuality_valid_maximum = store->mbPQuality_valid_maximum;
	    copy->mbPQuality_missing_value = store->mbPQuality_missing_value;
	    copy->mbPFlag_add_offset = store->mbPFlag_add_offset;
	    copy->mbPFlag_scale_factor = store->mbPFlag_scale_factor;
	    copy->mbPFlag_minimum = store->mbPFlag_minimum;
	    copy->mbPFlag_maximum = store->mbPFlag_maximum;
	    copy->mbPFlag_valid_minimum = store->mbPFlag_valid_minimum;
	    copy->mbPFlag_valid_maximum = store->mbPFlag_valid_maximum;
	    copy->mbPFlag_missing_value = store->mbPFlag_missing_value;
	
	    /* storage comment string */
	    for (i=0;i<MBSYS_NAVNETCDF_COMMENTLEN;i++)
	        copy->comment[i] = store->comment[i];	    
	    }

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
