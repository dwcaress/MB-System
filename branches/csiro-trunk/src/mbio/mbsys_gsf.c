/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_gsf.c	3.00	8/20/94
 *	$Id$
 *
 *    Copyright (c) 1994-2014 by
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
 * mbsys_gsf.c contains the functions for handling the data structure
 * used by MBIO functions to store data
 * from Gsf BottomChart Mark II multibeam sonar systems.
 * The data formats which are commonly used to store Gsf
 * data in files include
 *      MBF_ELUNB : MBIO ID 92
 *
 * Author:	D. W. Caress
 * Date:	March 5, 1998
 *
 * $Log: mbsys_gsf.c,v $
 * Revision 5.11  2009/03/13 07:05:58  caress
 * Release 5.1.2beta02
 *
 * Revision 5.10  2007/05/14 06:17:29  caress
 * Fixed bug in handling of Simrad and Reson multibeam sidescan.
 *
 * Revision 5.9  2006/03/06 21:47:48  caress
 * Implemented changes suggested by Bob Courtney of the Geological Survey of Canada to support translating Reson data to GSF.
 *
 * Revision 5.8  2005/11/05 00:48:04  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.7  2003/07/26 17:59:32  caress
 * Changed beamflag handling code.
 *
 * Revision 5.6  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.5  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.4  2001/08/25 00:54:13  caress
 * Adding beamwidth values to extract functions.
 *
 * Revision 5.3  2001/07/20  00:32:54  caress
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
 * Revision 4.6  2000/10/11  01:03:21  caress
 * Convert to ANSI C
 *
 * Revision 4.5  2000/09/30  06:32:52  caress
 * Snapshot for Dale.
 *
 * Revision 4.4  2000/06/08  22:20:16  caress
 * Handled Reson 8101 SAIC data more properly - get angles
 * correctly even when not signed.
 *
 * Revision 4.3  2000/03/06  21:54:21  caress
 * Distribution 4.6.10
 *
 * Revision 4.2  1999/07/16  19:24:15  caress
 * Yet another version.
 *
 * Revision 4.1  1999/05/05  22:48:29  caress
 * Disabled handling of ping flags in GSF data.
 *
 * Revision 4.0  1998/10/05  18:30:03  caress
 * MB-System version 4.6beta
 *
 * Revision 1.1  1998/10/05  18:22:40  caress
 * Initial revision
 *
 * Revision 1.1  1998/10/05  17:46:15  caress
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
#include "mb_status.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_define.h"
#include "mbsys_gsf.h"

static char rcs_id[]="$Id$";

/*--------------------------------------------------------------------*/
int mbsys_gsf_alloc(int verbose, void *mbio_ptr, void **store_ptr,
			int *error)
{
	char	*function_name = "mbsys_gsf_alloc";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* allocate memory for data structure */
	status = mb_malloc(verbose,sizeof(struct mbsys_gsf_struct),
				store_ptr,error);
	memset(*store_ptr, 0, sizeof(struct mbsys_gsf_struct));

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)*store_ptr);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_gsf_deall(int verbose, void *mbio_ptr, void **store_ptr,
			int *error)
{
	char	*function_name = "mbsys_gsf_deall";
	int	status = MB_SUCCESS;
	struct mbsys_gsf_struct *store;
	gsfRecords	    *records;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)*store_ptr);
		}

	/* deallocate memory for data structure */
	store =  (struct mbsys_gsf_struct *) *store_ptr;
	records = &(store->records);
	gsfFree(records);
	status = mb_free(verbose,store_ptr,error);

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
int mbsys_gsf_dimensions(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int *nbath, int *namp, int *nss, int *error)
{
	char	*function_name = "mbsys_gsf_dimensions";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_gsf_struct *store;
	gsfDataID	    *dataID;
	gsfRecords	    *records;
	gsfSwathBathyPing   *mb_ping;
	gsfTimeSeriesIntensity	*snippet;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_gsf_struct *) store_ptr;
	dataID = &(store->dataID);
	records = &(store->records);

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get beam and pixel numbers */
		mb_ping = &(records->mb_ping);

		if (mb_ping->depth != NULL)
		    *nbath = mb_ping->number_beams;
		else
		    *nbath = 0;
		if (mb_ping->mc_amplitude != NULL
		    || mb_ping->mr_amplitude != NULL)
		    *namp = mb_ping->number_beams;
		else
		    *namp = 0;
		*nss = 0;
		if (mb_ping->brb_inten != NULL)
			{
			for (i=0;i<*nbath;i++)
				{
				snippet = &(mb_ping->brb_inten->time_series[i]);
				(*nss) += snippet->sample_count;
				}
			}
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
int mbsys_gsf_sonartype(int verbose, void *mbio_ptr, void *store_ptr,
		int *sonartype, int *error)
{
	char	*function_name = "mbsys_gsf_sonartype";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_gsf_struct *store;
	gsfDataID	    *dataID;
	gsfRecords	    *records;
	gsfSwathBathyPing   *mb_ping;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_gsf_struct *) store_ptr;
	dataID = &(store->dataID);
	records = &(store->records);
	mb_ping = &(records->mb_ping);

	/* get sonar type */
	if (mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_SEABEAM_SPECIFIC          /* 102 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_EM12_SPECIFIC      /* 103 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_EM100_SPECIFIC            /* 104 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_EM950_SPECIFIC            /* 105 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_EM121A_SPECIFIC           /* 106 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_EM121_SPECIFIC            /* 107 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_SASS_SPECIFIC             /* 108 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_SEAMAP_SPECIFIC           /* 109 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_SEABAT_SPECIFIC           /* 110 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_EM1000_SPECIFIC           /* 111 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_TYPEIII_SEABEAM_SPECIFIC  /* 112 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_SB_AMP_SPECIFIC           /* 113 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_SEABAT_II_SPECIFIC        /* 114 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_SEABAT_8101_SPECIFIC      /* 115 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_SEABEAM_2112_SPECIFIC     /* 116 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_ELAC_MKII_SPECIFIC        /* 117 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_EM3000_SPECIFIC           /* 118 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_EM1002_SPECIFIC           /* 119 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_EM300_SPECIFIC            /* 120 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_CMP_SASS_SPECIFIC         /* 121 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_RESON_8101_SPECIFIC       /* 122 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_RESON_8111_SPECIFIC       /* 123 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_RESON_8124_SPECIFIC       /* 124 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_RESON_8125_SPECIFIC       /* 125 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_RESON_8150_SPECIFIC       /* 126 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_RESON_8160_SPECIFIC       /* 127 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_EM120_SPECIFIC            /* 128 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_EM3002_SPECIFIC           /* 129 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_EM3000D_SPECIFIC          /* 130 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_EM3002D_SPECIFIC          /* 131 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_EM121A_SIS_SPECIFIC       /* 132 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_EM710_SPECIFIC            /* 133 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_EM302_SPECIFIC            /* 134 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_EM122_SPECIFIC            /* 135 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_RESON_7125_SPECIFIC       /* 138 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_EM2000_SPECIFIC           /* 139 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_EM300_RAW_SPECIFIC        /* 140 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_EM1002_RAW_SPECIFIC       /* 141 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_EM2000_RAW_SPECIFIC       /* 142 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_EM3000_RAW_SPECIFIC       /* 143 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_EM120_RAW_SPECIFIC        /* 144 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_EM3002_RAW_SPECIFIC       /* 145 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_EM3000D_RAW_SPECIFIC      /* 146 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_EM3002D_RAW_SPECIFIC      /* 147 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_EM121A_SIS_RAW_SPECIFIC   /* 148 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_EM2040_SPECIFIC           /* 149 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_DELTA_T_SPECIFIC          /* 150 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_R2SONIC_2022_SPECIFIC     /* 151 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_R2SONIC_2024_SPECIFIC     /* 152 */)
		{
		*sonartype = MB_SONARTYPE_MULTIBEAM;
		}
	else if (mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_GEOSWATH_PLUS_SPECIFIC    /* 136 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_KLEIN_5410_BSS_SPECIFIC   /* 137 */)
		{
		*sonartype = MB_SONARTYPE_INTERFEROMETRIC;
		}
	else if (mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_GEOSWATH_PLUS_SPECIFIC    /* 136 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_KLEIN_5410_BSS_SPECIFIC   /* 137 */)
		{
		*sonartype = MB_SONARTYPE_SIDESCAN;
		}
	else if (mb_ping->sensor_id == GSF_SINGLE_BEAM_SUBRECORD_ECHOTRAC_SPECIFIC     		/* 201 */
		|| mb_ping->sensor_id == GSF_SINGLE_BEAM_SUBRECORD_BATHY2000_SPECIFIC		/* 202 */
		|| mb_ping->sensor_id == GSF_SINGLE_BEAM_SUBRECORD_MGD77_SPECIFIC        	/* 203 */
		|| mb_ping->sensor_id == GSF_SINGLE_BEAM_SUBRECORD_BDB_SPECIFIC          	/* 204 */
		|| mb_ping->sensor_id == GSF_SINGLE_BEAM_SUBRECORD_NOSHDB_SPECIFIC       	/* 205 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SB_SUBRECORD_ECHOTRAC_SPECIFIC		/* 206 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SB_SUBRECORD_BATHY2000_SPECIFIC	/* 207 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SB_SUBRECORD_MGD77_SPECIFIC		/* 208 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SB_SUBRECORD_BDB_SPECIFIC		/* 209 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SB_SUBRECORD_NOSHDB_SPECIFIC		/* 210 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SB_SUBRECORD_PDD_SPECIFIC		/* 211 */
		|| mb_ping->sensor_id == GSF_SWATH_BATHY_SB_SUBRECORD_NAVISOUND_SPECIFIC	/* 212 */)
		{
		*sonartype = MB_SONARTYPE_ECHOSOUNDER;
		}
	else
		{
		*sonartype = MB_SONARTYPE_UNKNOWN;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       sensor_id:  %d\n",mb_ping->sensor_id);
		fprintf(stderr,"dbg2       sonartype:  %d\n",*sonartype);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_gsf_sidescantype(int verbose, void *mbio_ptr, void *store_ptr,
		int *ss_type, int *error)
{
	char	*function_name = "mbsys_gsf_sidescantype";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_gsf_struct *store;
	gsfDataID	    *dataID;
	gsfRecords	    *records;
	gsfSwathBathyPing   *mb_ping;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_gsf_struct *) store_ptr;
	dataID = &(store->dataID);
	records = &(store->records);
	mb_ping = &(records->mb_ping);

	/* get sidescan type */
	if (mb_ping->sensor_id == GSF_SWATH_BATHY_SUBRECORD_SEABEAM_2112_SPECIFIC)
		{
		*ss_type = MB_SIDESCAN_LINEAR;
		}
	else
		{
		*ss_type = MB_SIDESCAN_LOGARITHMIC;
		}

	/* print output debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       ss_type:    %d\n",*ss_type);
		fprintf(stderr,"dbg2       error:      %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:     %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_gsf_extract(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading,
		int *nbath, int *namp, int *nss,
		char *beamflag, double *bath, double *amp,
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_gsf_extract";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_gsf_struct *store;
	gsfDataID	    *dataID;
	gsfRecords	    *records;
	gsfSwathBathyPing   *mb_ping;
	gsfTimeSeriesIntensity	*snippet;
	double	ss_spacing, ss_spacing_use;
	double	vertical, range, beam_foot, sint, angle;
	int	gsfstatus;
	int	i, j;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_gsf_struct *) store_ptr;
	dataID = &(store->dataID);
	records = &(store->records);

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		mb_ping = &(records->mb_ping);

		/* get time */
		*time_d = mb_ping->ping_time.tv_sec
				+ 0.000000001 * mb_ping->ping_time.tv_nsec;
		mb_get_date(verbose,*time_d,time_i);

		/* get navigation */
		if (mb_ping->longitude != GSF_NULL_LONGITUDE)
			*navlon = mb_ping->longitude;
		else
			*navlon = 0.0;
		if (mb_ping->latitude != GSF_NULL_LATITUDE)
			*navlat = mb_ping->latitude;
		else
			*navlat = 0.0;

		/* get heading */
		if (mb_ping->heading != GSF_NULL_HEADING)
			*heading = mb_ping->heading;
		else
			*heading = 0.0;

		/* get speed */
		if (mb_ping->speed != GSF_NULL_SPEED)
			*speed = 1.852 * mb_ping->speed;
		else
			*speed = 0.0;

		/* set beamwidths in mb_io structure */
		gsfstatus = gsfGetSwathBathyBeamWidths(records,
			&(mb_io_ptr->beamwidth_ltrack),
			&(mb_io_ptr->beamwidth_xtrack));

		/* if not set then use hard coded values */
		if (mb_io_ptr->beamwidth_ltrack <= 0.0
			|| mb_io_ptr->beamwidth_xtrack <= 0.0)
			{
			if (mb_ping->sensor_id
			    == GSF_SWATH_BATHY_SUBRECORD_SEABEAM_SPECIFIC)
			    {
			    mb_io_ptr->beamwidth_ltrack = 2.67;
			    mb_io_ptr->beamwidth_xtrack = 2.67;
			    }
			else if (mb_ping->sensor_id
			    == GSF_SWATH_BATHY_SUBRECORD_EM100_SPECIFIC)
			    {
			    mb_io_ptr->beamwidth_ltrack = 3.3;
			    mb_io_ptr->beamwidth_xtrack = 3.3;
			    }
			else if (mb_ping->sensor_id
			    == GSF_SWATH_BATHY_SUBRECORD_EM950_SPECIFIC)
			    {
			    mb_io_ptr->beamwidth_ltrack = 3.3;
			    mb_io_ptr->beamwidth_xtrack = 3.3;
			    }
			else if (mb_ping->sensor_id
			    == GSF_SWATH_BATHY_SUBRECORD_EM121A_SPECIFIC)
			    {
			    mb_io_ptr->beamwidth_ltrack = 1.0;
			    mb_io_ptr->beamwidth_xtrack = 1.0;
			    }
			else if (mb_ping->sensor_id
			    == GSF_SWATH_BATHY_SUBRECORD_EM121_SPECIFIC)
			    {
			    mb_io_ptr->beamwidth_ltrack = 1.0;
			    mb_io_ptr->beamwidth_xtrack = 1.0;
			    }
			else if (mb_ping->sensor_id
			    == GSF_SWATH_BATHY_SUBRECORD_SEAMAP_SPECIFIC)
			    {
			    mb_io_ptr->beamwidth_ltrack = 2.0;
			    mb_io_ptr->beamwidth_xtrack = 2.0;
			    }
			else if (mb_ping->sensor_id
			    == GSF_SWATH_BATHY_SUBRECORD_SEABAT_SPECIFIC)
			    {
			    mb_io_ptr->beamwidth_ltrack = 1.5;
			    mb_io_ptr->beamwidth_xtrack = 1.5;
			    }
			else if (mb_ping->sensor_id
			    == GSF_SWATH_BATHY_SUBRECORD_EM1000_SPECIFIC)
			    {
			    mb_io_ptr->beamwidth_ltrack = 3.3;
			    mb_io_ptr->beamwidth_xtrack = 3.3;
			    }
			else if (mb_ping->sensor_id
			    == GSF_SWATH_BATHY_SUBRECORD_TYPEIII_SEABEAM_SPECIFIC)
			    {
			    mb_io_ptr->beamwidth_ltrack = 2.67;
			    mb_io_ptr->beamwidth_xtrack = 2.67;
			    }
			else if (mb_ping->sensor_id
			    == GSF_SWATH_BATHY_SUBRECORD_SB_AMP_SPECIFIC)
			    {
			    mb_io_ptr->beamwidth_ltrack = 2.0;
			    mb_io_ptr->beamwidth_xtrack = 2.0;
			    }
			else if (mb_ping->sensor_id
			    == GSF_SWATH_BATHY_SUBRECORD_SEABAT_II_SPECIFIC)
			    {
			    mb_io_ptr->beamwidth_ltrack = 1.5;
			    mb_io_ptr->beamwidth_xtrack = 1.5;
			    }
			else if (mb_ping->sensor_id
			    == GSF_SWATH_BATHY_SUBRECORD_SEABAT_8101_SPECIFIC)
			    {
			    mb_io_ptr->beamwidth_ltrack = 1.5;
			    mb_io_ptr->beamwidth_xtrack = 1.5;
			    }
			else if (mb_ping->sensor_id
			    == GSF_SWATH_BATHY_SUBRECORD_SEABEAM_2112_SPECIFIC)
			    {
			    mb_io_ptr->beamwidth_ltrack = 2.0;
			    mb_io_ptr->beamwidth_xtrack = 2.0;
			    }
			else if (mb_ping->sensor_id
			    == GSF_SWATH_BATHY_SUBRECORD_ELAC_MKII_SPECIFIC)
			    {
			    mb_io_ptr->beamwidth_ltrack = 1.5;
			    mb_io_ptr->beamwidth_xtrack = 2.8;
			    }
			else if (mb_ping->sensor_id
			    == GSF_SWATH_BATHY_SUBRECORD_ELAC_MKII_SPECIFIC)
			    {
			    mb_io_ptr->beamwidth_ltrack = 1.5;
			    mb_io_ptr->beamwidth_xtrack = 2.8;
			    }
			else if (mb_ping->sensor_id
			    == GSF_SWATH_BATHY_SUBRECORD_GEOSWATH_PLUS_SPECIFIC)
			    {
			    mb_io_ptr->beamwidth_ltrack = 1.0;
			    mb_io_ptr->beamwidth_xtrack = 0.1;
			    }
			else
			    {
			    mb_io_ptr->beamwidth_ltrack = 2.0;
			    mb_io_ptr->beamwidth_xtrack = 2.0;
			    }
			}

		/* get numbers of beams and pixels */
		if (mb_ping->depth != NULL)
		    *nbath = mb_ping->number_beams;
		else
		    *nbath = 0;
		if (mb_ping->mc_amplitude != NULL
		    || mb_ping->mr_amplitude != NULL)
		    *namp = mb_ping->number_beams;
		else
		    *namp = 0;
		*nss = 0;

		/* read depth and beam location values into storage arrays */
		for (i=0;i<*nbath;i++)
			{
			/* set null beam flag if required */
			if (mb_ping->depth[i] == 0.0
			    && mb_ping->across_track[i] == 0.0
			    && mb_ping->beam_flags[i] != MB_FLAG_NULL)
			    mb_ping->beam_flags[i] = MB_FLAG_NULL;

			beamflag[i] = mb_ping->beam_flags[i];
			bath[i] = mb_ping->depth[i];
			bathacrosstrack[i] = mb_ping->across_track[i];
			bathalongtrack[i] = mb_ping->along_track[i];
			}

		/* set beamflags if ping flag set */
/*		if (mb_ping->ping_flags != 0)
		    {
		    for (i=0;i<*nbath;i++)
			if (mb_beam_ok(beamflag[i]))
			    beamflag[i]
				= mb_beam_set_flag_manual(beamflag[i]);
		    }*/

		/* read amplitude values into storage arrays */
		if (mb_ping->mc_amplitude != NULL)
		for (i=0;i<*namp;i++)
			{
			amp[i] = mb_ping->mc_amplitude[i];
			}
		else if (mb_ping->mr_amplitude != NULL)
		for (i=0;i<*namp;i++)
			{
			amp[i] = mb_ping->mr_amplitude[i];
			}

		/* read multibeam sidescan if available */
		if (mb_ping->brb_inten != NULL)
			{
			/* get sample rate and raw sidescan sample size */
			if (mb_ping->sensor_id
					== GSF_SWATH_BATHY_SUBRECORD_RESON_8101_SPECIFIC
				|| mb_ping->sensor_id
					== GSF_SWATH_BATHY_SUBRECORD_RESON_8111_SPECIFIC
				|| mb_ping->sensor_id
					== GSF_SWATH_BATHY_SUBRECORD_RESON_8124_SPECIFIC
				|| mb_ping->sensor_id
					== GSF_SWATH_BATHY_SUBRECORD_RESON_8125_SPECIFIC
				|| mb_ping->sensor_id
					== GSF_SWATH_BATHY_SUBRECORD_RESON_8150_SPECIFIC
				|| mb_ping->sensor_id
					== GSF_SWATH_BATHY_SUBRECORD_RESON_8160_SPECIFIC)
				{
				ss_spacing = 750.0 / ((double)mb_ping->sensor_data.gsfReson8100Specific.sample_rate);
				}
			else if (mb_ping->sensor_id
					== GSF_SWATH_BATHY_SUBRECORD_EM3000_SPECIFIC
				|| mb_ping->sensor_id
					== GSF_SWATH_BATHY_SUBRECORD_EM1002_SPECIFIC
				|| mb_ping->sensor_id
					== GSF_SWATH_BATHY_SUBRECORD_EM300_SPECIFIC
				|| mb_ping->sensor_id
					== GSF_SWATH_BATHY_SUBRECORD_EM120_SPECIFIC
				|| mb_ping->sensor_id
					== GSF_SWATH_BATHY_SUBRECORD_EM3002_SPECIFIC
				|| mb_ping->sensor_id
					== GSF_SWATH_BATHY_SUBRECORD_EM3000D_SPECIFIC
				|| mb_ping->sensor_id
					== GSF_SWATH_BATHY_SUBRECORD_EM3002D_SPECIFIC)
				{
				ss_spacing = 750.0 / ((double)mb_ping->sensor_data.gsfEM3Specific.sample_rate);
				}
			*nss = 0;
			for (i=0;i<*nbath;i++)
				{
				/* get pixel sample size */
				snippet = &(mb_ping->brb_inten->time_series[i]);
				vertical = mb_ping->depth[i] - mb_ping->depth_corrector;
				range = sqrt(vertical * vertical + bathacrosstrack[i] * bathacrosstrack[i]);
				angle = 90.0 - fabs(mb_ping->beam_angle[i]);
				beam_foot = range * sin(DTR * mb_io_ptr->beamwidth_xtrack)
							/ cos(DTR * angle);
				sint = fabs(sin(DTR * angle));
				if (sint < snippet->sample_count * ss_spacing / beam_foot)
				    ss_spacing_use = beam_foot / snippet->sample_count;
				else
				    ss_spacing_use = ss_spacing / sint;
				for (j=0;j<snippet->sample_count;j++)
					{
					ss[*nss] = snippet->samples[j];
					ssacrosstrack[*nss] = bathacrosstrack[i]
						+ ss_spacing_use * (j - snippet->detect_sample);
					ssalongtrack[*nss] = bathalongtrack[i];
					(*nss)++;
					}
				}
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
			}

		/* done translating values */

		}

	/* extract comment from structure */
	else if (*kind == MB_DATA_COMMENT)
		{
		/* copy comment */
		if (records->comment.comment_length > 0
		    && records->comment.comment != NULL)
			strcpy(comment, records->comment.comment);
		else
			comment[0] = '\0';

		/* print debug statements */
		if (verbose >= 4)
			{
			fprintf(stderr,"\ndbg4  Comment extracted by MBIO function <%s>\n",
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
		  fprintf(stderr,"dbg2       pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n",
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
int mbsys_gsf_insert(int verbose, void *mbio_ptr, void *store_ptr,
		int kind, int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading,
		int nbath, int namp, int nss,
		char *beamflag, double *bath, double *amp,
		double *bathacrosstrack, double *bathalongtrack,
		double *ss, double *ssacrosstrack, double *ssalongtrack,
		char *comment, int *error)
{
	char	*function_name = "mbsys_gsf_insert";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_gsf_struct *store;
	gsfDataID	    *dataID;
	gsfRecords	    *records;
	gsfSwathBathyPing   *mb_ping;
	int	anyunflagged;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
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
		fprintf(stderr,"dbg2       nbath:      %d\n",nbath);
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
		}
	if (verbose >= 2 && kind == MB_DATA_COMMENT)
		{
		fprintf(stderr,"dbg2       comment:     \ndbg2       %s\n",
			comment);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_gsf_struct *) store_ptr;
	records = &(store->records);
	dataID = &(store->dataID);
	mb_ping = &(records->mb_ping);

	/* set data kind */
	store->kind = kind;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{
		dataID->recordID = GSF_RECORD_SWATH_BATHYMETRY_PING;
		mb_ping = &(records->mb_ping);

		/* get time */
		mb_ping->ping_time.tv_sec = (int) time_d;
		mb_ping->ping_time.tv_nsec = (int) (1000000000 * (time_d - mb_ping->ping_time.tv_sec));

		/* get navigation */
		if (navlon != 0.0)
			mb_ping->longitude = navlon;
		else
			mb_ping->longitude = GSF_NULL_LONGITUDE;
		if (navlat != 0.0)
			mb_ping->latitude = navlat;
		else
			mb_ping->longitude = GSF_NULL_LATITUDE;

		/* get heading */
		if (heading != 0.0)
			mb_ping->heading = heading;
		else
			mb_ping->heading = GSF_NULL_HEADING;

		/* get speed */
		if (speed != 0.0)
			mb_ping->speed = speed / 1.852;
		else
			mb_ping->speed = speed;

		/* get numbers of beams */
		mb_ping->number_beams = nbath;

		/* allocate memory in arrays if required */
		if (nbath > mb_ping->number_beams)
		    {
		    mb_ping->beam_flags
			= (unsigned char *)
			    realloc(mb_ping->beam_flags,
					nbath * sizeof(char));
		    mb_ping->depth
			= (double *) realloc(mb_ping->depth, nbath * sizeof(double));
		    mb_ping->across_track = (double *) realloc(mb_ping->across_track, nbath * sizeof(double));
		    mb_ping->along_track = (double *) realloc(mb_ping->along_track, nbath * sizeof(double));
		    if (mb_ping->beam_flags == NULL
			|| mb_ping->depth == NULL
			|| mb_ping->across_track == NULL
			|| mb_ping->along_track == NULL)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_MEMORY_FAIL;
			}
		    }
		if (namp > mb_ping->number_beams
		    && mb_ping->mc_amplitude != NULL)
		    {
		    mb_ping->mc_amplitude = (double *) realloc(mb_ping->mc_amplitude, namp * sizeof(double));
		    if (mb_ping->mc_amplitude == NULL)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_MEMORY_FAIL;
			}
		    }
		else if (namp > mb_ping->number_beams
		    && mb_ping->mr_amplitude != NULL)
		    {
		    mb_ping->mr_amplitude = (double *) realloc(mb_ping->mr_amplitude, namp * sizeof(double));
		    if (mb_ping->mr_amplitude == NULL)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_MEMORY_FAIL;
			}
		    }
		mb_ping->number_beams = nbath;

		/* if ping flag set check for any unset
		    beam flags - set or unset ping flag based on whether any
		    unflagged beams are found */
		anyunflagged = MB_NO;
		for (i=0;i<nbath;i++)
			{
			if (mb_beam_ok(beamflag[i]))
			    anyunflagged = MB_YES;
			}
		if (anyunflagged == MB_NO)
			mb_ping->ping_flags = GSF_IGNORE_PING;
		else
			mb_ping->ping_flags = 0;

		/* read depth and beam location values into storage arrays */
		for (i=0;i<nbath;i++)
			{
			mb_ping->beam_flags[i] = beamflag[i];
			if (beamflag[i] != MB_FLAG_NULL)
			    {
			    mb_ping->depth[i] = bath[i];
			    mb_ping->across_track[i] = bathacrosstrack[i];
			    mb_ping->along_track[i] = bathalongtrack[i];
			    }
			else
			    {
			    mb_ping->depth[i] = GSF_NULL_DEPTH;
			    mb_ping->across_track[i] = GSF_NULL_ACROSS_TRACK;
			    mb_ping->along_track[i] = GSF_NULL_ALONG_TRACK;
			    }
			}

		/* read amplitude values into storage arrays */
		if (mb_ping->mc_amplitude != NULL)
		for (i=0;i<namp;i++)
			{
			mb_ping->mc_amplitude[i] = amp[i];
			}
		else if (mb_ping->mr_amplitude != NULL)
		for (i=0;i<namp;i++)
			{
			mb_ping->mr_amplitude[i] = amp[i];
			}

		/* reset GSF scale factors if needed */
		mbsys_gsf_setscalefactors(verbose, MB_NO, mb_ping, error);
			
		}

	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT)
		{
		dataID->recordID = GSF_RECORD_COMMENT;
		if (records->comment.comment_length < strlen(comment) + 1)
		    {
		    if ((records->comment.comment
				= (char *)
				    realloc(records->comment.comment,
					strlen(comment)+1))
					    == NULL)
			{
			status = MB_FAILURE;
			*error = MB_ERROR_MEMORY_FAIL;
			records->comment.comment_length = 0;
			}
		    }
		if (status == MB_SUCCESS && records->comment.comment != NULL)
		    {
		    strcpy(records->comment.comment, comment);
		    records->comment.comment_length = strlen(comment)+1;
		    records->comment.comment_time.tv_sec = (int) time_d;
		    records->comment.comment_time.tv_nsec
			    = (int) (1000000000
				* (time_d
					- records->comment.comment_time.tv_sec));
		    }
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
int mbsys_gsf_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams,
	double *ttimes, double *angles,
	double *angles_forward, double *angles_null,
	double *heave, double *alongtrack_offset,
	double *draft, double *ssv, int *error)
{
	char	*function_name = "mbsys_gsf_ttimes";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_gsf_struct *store;
	gsfDataID	    *dataID;
	gsfRecords	    *records;
	gsfSwathBathyPing   *mb_ping;
	double	alpha, beta;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		fprintf(stderr,"dbg2       ttimes:     %p\n",(void *)ttimes);
		fprintf(stderr,"dbg2       angles_xtrk:%p\n",(void *)angles);
		fprintf(stderr,"dbg2       angles_ltrk:%p\n",(void *)angles_forward);
		fprintf(stderr,"dbg2       angles_null:%p\n",(void *)angles_null);
		fprintf(stderr,"dbg2       heave:      %p\n",(void *)heave);
		fprintf(stderr,"dbg2       ltrk_off:   %p\n",(void *)alongtrack_offset);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_gsf_struct *) store_ptr;
	records = &(store->records);
	dataID = &(store->dataID);
	mb_ping = &(records->mb_ping);

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
	    {
	    /* get nbeams */
	    *nbeams = mb_ping->number_beams;

	    /* get travel times, angles */
	    if (mb_ping->travel_time != NULL
		&& mb_ping->beam_angle != NULL)
		{
		if (mb_ping->beam_angle_forward != NULL)
		    {
		    for (i=0;i<*nbeams;i++)
			{
			ttimes[i] = mb_ping->travel_time[i];
			angles[i] = fabs(mb_ping->beam_angle[i]);
			angles_forward[i] = mb_ping->beam_angle_forward[i];
			heave[i] = mb_ping->heave;
			alongtrack_offset[i] = 0.0;
			}
		    }
		else
		    {
		    for (i=0;i<*nbeams;i++)
			{
			ttimes[i] = mb_ping->travel_time[i];
			angles[i] = fabs(mb_ping->beam_angle[i]);
			if (mb_ping->across_track[i] < 0.0
			    || mb_ping->beam_angle[i] < 0.0)
				angles_forward[i] = 180.0;
			else
				angles_forward[i] = 0.0;
			heave[i] = mb_ping->heave;
			alongtrack_offset[i] = 0.0;
			}
		    }

		/* get surface sound velocity and
		    effective array mount angles */
		if (mb_ping->sensor_id
		    == GSF_SWATH_BATHY_SUBRECORD_SEABEAM_SPECIFIC)
		    {
		    *ssv = 1500.0;
		    *draft = mb_ping->depth_corrector;
		    for (i=0;i<*nbeams;i++)
			angles_null[i] = 0.0;
		    }
		else if (mb_ping->sensor_id
		    == GSF_SWATH_BATHY_SUBRECORD_EM100_SPECIFIC)
		    {
		    *ssv = 1500.0;
		    *draft = mb_ping->depth_corrector;
		    for (i=0;i<*nbeams;i++)
			    angles_null[i] = angles[i];
		    }
		else if (mb_ping->sensor_id
		    == GSF_SWATH_BATHY_SUBRECORD_EM950_SPECIFIC)
		    {
		    *ssv = mb_ping->sensor_data.gsfEM950Specific.surface_velocity;
		    *draft = mb_ping->depth_corrector;
		    for (i=0;i<*nbeams;i++)
			    angles_null[i] = angles[i];
		    }
		else if (mb_ping->sensor_id
		    == GSF_SWATH_BATHY_SUBRECORD_EM121A_SPECIFIC)
		    {
		    *ssv = mb_ping->sensor_data.gsfEM121ASpecific.surface_velocity;
		    *draft = mb_ping->depth_corrector;
		    for (i=0;i<*nbeams;i++)
			angles_null[i] = 0.0;
		    }
		else if (mb_ping->sensor_id
		    == GSF_SWATH_BATHY_SUBRECORD_EM121_SPECIFIC)
		    {
		    *ssv = mb_ping->sensor_data.gsfEM121Specific.surface_velocity;
		    *draft = mb_ping->depth_corrector;
		    for (i=0;i<*nbeams;i++)
			angles_null[i] = 0.0;
		    }
		else if (mb_ping->sensor_id
		    == GSF_SWATH_BATHY_SUBRECORD_SEAMAP_SPECIFIC)
		    {
		    *ssv = 1500.0;
		    *draft = mb_ping->depth_corrector;
		    for (i=0;i<*nbeams;i++)
			    angles_null[i] = 50.0;
		    }
		else if (mb_ping->sensor_id
		    == GSF_SWATH_BATHY_SUBRECORD_SEABAT_SPECIFIC)
		    {
		    *ssv = mb_ping->sensor_data.gsfSeaBatSpecific.surface_velocity;
		    *draft = mb_ping->depth_corrector;
		    if (mb_ping->beam_angle_forward == NULL)
			{
			for (i=0;i<*nbeams;i++)
			    {
			    if (mb_ping->across_track[i] < 0.0
				&& mb_ping->beam_angle[i] > 0.0)
				beta = 90.0 + mb_ping->beam_angle[i];
			    else
				beta = 90.0 - mb_ping->beam_angle[i];
			    alpha = mb_ping->pitch;
			    mb_rollpitch_to_takeoff(verbose,
				alpha, beta, &angles[i],
				&angles_forward[i], error);
			    }
			}
		    for (i=0;i<*nbeams;i++)
			    angles_null[i] = angles[i];
		    }
		else if (mb_ping->sensor_id
		    == GSF_SWATH_BATHY_SUBRECORD_EM1000_SPECIFIC)
		    {
		    *ssv = mb_ping->sensor_data.gsfEM1000Specific.surface_velocity;
		    *draft = mb_ping->depth_corrector;
		    for (i=0;i<*nbeams;i++)
			    angles_null[i] = angles[i];
		    }
		else if (mb_ping->sensor_id
		    == GSF_SWATH_BATHY_SUBRECORD_TYPEIII_SEABEAM_SPECIFIC)
		    {
		    *ssv = 1500.0;
		    *draft = mb_ping->depth_corrector;
		    for (i=0;i<*nbeams;i++)
			angles_null[i] = 0.0;
		    }
		else if (mb_ping->sensor_id
		    == GSF_SWATH_BATHY_SUBRECORD_SB_AMP_SPECIFIC)
		    {
		    *ssv = 1500.0;
		    *draft = mb_ping->depth_corrector;
		    for (i=0;i<*nbeams;i++)
			angles_null[i] = 0.0;
		    }
		else if (mb_ping->sensor_id
		    == GSF_SWATH_BATHY_SUBRECORD_SEABAT_II_SPECIFIC)
		    {
		    *ssv = mb_ping->sensor_data.gsfSeaBatIISpecific.surface_velocity;
		    *draft = mb_ping->depth_corrector;
		    if (mb_ping->beam_angle_forward == NULL)
			{
			for (i=0;i<*nbeams;i++)
			    {
			    if (mb_ping->across_track[i] < 0.0
				&& mb_ping->beam_angle[i] > 0.0)
				beta = 90.0 + mb_ping->beam_angle[i];
			    else
				beta = 90.0 - mb_ping->beam_angle[i];
			    alpha = mb_ping->pitch;
			    mb_rollpitch_to_takeoff(verbose,
				alpha, beta, &angles[i],
				&angles_forward[i], error);
			    }
			}
		    for (i=0;i<*nbeams;i++)
			    angles_null[i] = angles[i];
		    }
		else if (mb_ping->sensor_id
		    == GSF_SWATH_BATHY_SUBRECORD_SEABAT_8101_SPECIFIC)
		    {
		    *ssv = mb_ping->sensor_data.gsfSeaBat8101Specific.surface_velocity;
		    *draft = mb_ping->depth_corrector;
		    if (mb_ping->beam_angle_forward == NULL)
			{
			for (i=0;i<*nbeams;i++)
			    {
			    if (mb_ping->across_track[i] < 0.0
				&& mb_ping->beam_angle[i] > 0.0)
				beta = 90.0 + mb_ping->beam_angle[i];
			    else
				beta = 90.0 - mb_ping->beam_angle[i];
			    alpha = mb_ping->pitch;
			    mb_rollpitch_to_takeoff(verbose,
				alpha, beta, &angles[i],
				&angles_forward[i], error);
			    }
			}
		    for (i=0;i<*nbeams;i++)
			    angles_null[i] = angles[i];
		    }
		else if (mb_ping->sensor_id
		    == GSF_SWATH_BATHY_SUBRECORD_RESON_8101_SPECIFIC)
		    {
		    *ssv = mb_ping->sensor_data.gsfReson8100Specific.surface_velocity;
		    *draft = mb_ping->depth_corrector;
		    for (i=0;i<*nbeams;i++)
			    angles_null[i] = angles[i];
		    }
		else if (mb_ping->sensor_id
		    == GSF_SWATH_BATHY_SUBRECORD_SEABEAM_2112_SPECIFIC)
		    {
		    *ssv = mb_ping->sensor_data.gsfSeaBeam2112Specific.surface_velocity;
		    *draft = mb_ping->depth_corrector;
		    for (i=0;i<*nbeams;i++)
			angles_null[i] = 0.0;
		    }
		else if (mb_ping->sensor_id
		    == GSF_SWATH_BATHY_SUBRECORD_ELAC_MKII_SPECIFIC)
		    {
		    *ssv = mb_ping->sensor_data.gsfElacMkIISpecific.sound_vel;
		    *draft = mb_ping->depth_corrector;
		    if (mb_ping->beam_angle_forward == NULL)
			{
			for (i=0;i<*nbeams;i++)
			    {
			    beta = 90.0 - mb_ping->beam_angle[i];
			    alpha = mb_ping->pitch;
			    mb_rollpitch_to_takeoff(verbose,
				alpha, beta, &angles[i],
				&angles_forward[i], error);
			    }
			}
		    for (i=0;i<*nbeams;i++)
			    angles_null[i] = 37.5;
		    }
		else
		    {
		    *ssv = 1500.0;
		    *draft = mb_ping->depth_corrector;
		    for (i=0;i<*nbeams;i++)
			angles_null[i] = 0.0;
		    }
		}

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
		fprintf(stderr,"dbg2       draft:      %f\n",*draft);
		fprintf(stderr,"dbg2       ssv:        %f\n",*ssv);
		fprintf(stderr,"dbg2       nbeams:     %d\n",*nbeams);
		for (i=0;i<*nbeams;i++)
			fprintf(stderr,"dbg2       beam %d: tt:%f  angle_xtrk:%f  angle_ltrk:%f  angle_null:%f  heave:%f  ltrk_off:%f\n",
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
int mbsys_gsf_detects(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, int *nbeams,
	int *detects, int *error)
{
	char	*function_name = "mbsys_gsf_detects";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_gsf_struct *store;
	gsfDataID	    *dataID;
	gsfRecords	    *records;
	gsfSwathBathyPing   *mb_ping;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		fprintf(stderr,"dbg2       detects:    %p\n",(void *)detects);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_gsf_struct *) store_ptr;
	records = &(store->records);
	dataID = &(store->dataID);
	mb_ping = &(records->mb_ping);

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get nbeams */
		*nbeams = mb_ping->number_beams;

		/* get detects */
		for (i=0;i<*nbeams;i++)
			{
			detects[i] = MB_DETECT_UNKNOWN;
			}

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
int mbsys_gsf_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
	int *kind, double *transducer_depth, double *altitude,
	int *error)
{
	char	*function_name = "mbsys_gsf_extract_altitude";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_gsf_struct *store;
	gsfDataID	    *dataID;
	gsfRecords	    *records;
	gsfSwathBathyPing   *mb_ping;
	double	bath_best;
	double	xtrack_min;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_gsf_struct *) store_ptr;
	records = &(store->records);
	dataID = &(store->dataID);
	mb_ping = &(records->mb_ping);

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
	    {
	    /* reset depth_corrector to zero if necessary */
	    if (mb_ping->depth_corrector == GSF_NULL_DEPTH_CORRECTOR)
		{
		mb_ping->depth_corrector = 0.0;
		}
	    if (mb_ping->heave == GSF_NULL_HEAVE)
		{
		mb_ping->heave = 0.0;
		}

	    /* get transducer_depth */
	    *transducer_depth = mb_ping->depth_corrector + mb_ping->heave;

	    /* get altitude if available */
	    if (mb_ping->sensor_id
		== GSF_SWATH_BATHY_SUBRECORD_SEAMAP_SPECIFIC)
		{
		*altitude = mb_ping->sensor_data.gsfSeamapSpecific.altitude;
		}

	    /* else get altitude from depth */
	    else if (mb_ping->depth != NULL)
		{
		bath_best = 0.0;
		if (mb_beam_ok(mb_ping->beam_flags[mb_ping->number_beams/2]))
			{
			bath_best = mb_ping->depth[mb_ping->number_beams/2];
			}
		else
		    {
		    xtrack_min = 99999999.9;
		    for (i=0;i<mb_ping->number_beams;i++)
			{
			if (mb_beam_ok(mb_ping->beam_flags[i])
			    && fabs(mb_ping->across_track[i]) < xtrack_min)
			    {
			    xtrack_min = fabs(mb_ping->across_track[i]);
			    bath_best = mb_ping->depth[i];
			    }
			}
		    }
		if (bath_best <= 0.0)
		    {
		    xtrack_min = 99999999.9;
		    for (i=0;i<mb_ping->number_beams;i++)
			{
			if (!mb_beam_check_flag(mb_ping->beam_flags[i])
			    && fabs(mb_ping->across_track[i]) < xtrack_min)
			    {
			    xtrack_min = fabs(mb_ping->across_track[i]);
			    bath_best = mb_ping->depth[i];
			    }
			}
		    }
		*altitude = bath_best - *transducer_depth;
		}

	    /* else no info */
	    else
		{
		*altitude = 0.0;
		}

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
int mbsys_gsf_insert_altitude(int verbose, void *mbio_ptr, void *store_ptr,
	double transducer_depth, double altitude,
	int *error)
{
	char	*function_name = "mbsys_gsf_insert_altitude";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_gsf_struct *store;
	gsfDataID	    *dataID;
	gsfRecords	    *records;
	gsfSwathBathyPing   *mb_ping;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:           %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:            %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:         %p\n",(void *)store_ptr);
		fprintf(stderr,"dbg2       transducer_depth:  %f\n",transducer_depth);
		fprintf(stderr,"dbg2       altitude:          %f\n",altitude);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_gsf_struct *) store_ptr;
	records = &(store->records);
	dataID = &(store->dataID);
	mb_ping = &(records->mb_ping);

	/* insert data into structure */
	if (store->kind == MB_DATA_DATA)
	    {
	    mb_ping->depth_corrector = transducer_depth - mb_ping->heave;

	    /* set altitude if possible */
	    if (mb_ping->sensor_id
		== GSF_SWATH_BATHY_SUBRECORD_SEAMAP_SPECIFIC)
		{
		mb_ping->sensor_data.gsfSeamapSpecific.altitude = altitude;
		}

	    /* get scale factor for bathymetry */
	    // gsfSetDefaultScaleFactor(mb_ping);

	    /* set status */
	    *error = MB_ERROR_NO_ERROR;
	    status = MB_SUCCESS;

	    /* done translating values */

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
int mbsys_gsf_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int time_i[7], double *time_d,
		double *navlon, double *navlat,
		double *speed, double *heading, double *draft,
		double *roll, double *pitch, double *heave,
		int *error)
{
	char	*function_name = "mbsys_gsf_extract_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_gsf_struct *store;
	gsfDataID	    *dataID;
	gsfRecords	    *records;
	gsfSwathBathyPing   *mb_ping;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_gsf_struct *) store_ptr;
	records = &(store->records);
	dataID = &(store->dataID);
	mb_ping = &(records->mb_ping);

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA)
		{
		/* get time */
		*time_d = mb_ping->ping_time.tv_sec
				+ 0.000000001 * mb_ping->ping_time.tv_nsec;
		mb_get_date(verbose,*time_d,time_i);

		/* get navigation */
		if (mb_ping->longitude != GSF_NULL_LONGITUDE)
			*navlon = mb_ping->longitude;
		else
			*navlon = 0.0;
		if (mb_ping->latitude != GSF_NULL_LATITUDE)
			*navlat = mb_ping->latitude;
		else
			*navlat = 0.0;

		/* get heading */
		if (mb_ping->heading != GSF_NULL_HEADING)
			*heading = mb_ping->heading;
		else
			*heading = 0.0;

		/* get speed */
		if (mb_ping->speed != GSF_NULL_SPEED)
			*speed = 1.852 * mb_ping->speed;
		else
			*speed = 0.0;

		/* get draft */
		if (mb_ping->depth_corrector != GSF_NULL_DEPTH_CORRECTOR)
			*draft = mb_ping->depth_corrector;
		else
			*draft = 0.0;

		/* get roll pitch and heave */
		*roll = mb_ping->roll;
		*pitch = mb_ping->pitch;
		*heave = mb_ping->heave;

		if (mb_ping->roll != GSF_NULL_ROLL)
			*roll = mb_ping->roll;
		else
			*roll = 0.0;
		if (mb_ping->pitch != GSF_NULL_PITCH)
			*pitch = mb_ping->pitch;
		else
			*pitch = 0.0;
		if (mb_ping->heave != GSF_NULL_HEAVE)
			*heave = mb_ping->heave;
		else
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
int mbsys_gsf_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
		int time_i[7], double time_d,
		double navlon, double navlat,
		double speed, double heading, double draft,
		double roll, double pitch, double heave,
		int *error)
{
	char	*function_name = "mbsys_gsf_insert_nav";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_gsf_struct *store;
	gsfDataID	    *dataID;
	gsfRecords	    *records;
	gsfSwathBathyPing   *mb_ping;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
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
	store = (struct mbsys_gsf_struct *) store_ptr;
	records = &(store->records);
	dataID = &(store->dataID);
	mb_ping = &(records->mb_ping);

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA)
		{

		/* get time */
		mb_ping->ping_time.tv_sec = (int) time_d;
		mb_ping->ping_time.tv_nsec
			= (int) (1000000000
			    * (time_d
				    - mb_ping->ping_time.tv_sec));

		/* get navigation */
		if (navlon != 0.0)
			mb_ping->longitude = navlon;
		else
			mb_ping->longitude = GSF_NULL_LONGITUDE;
		if (navlat != 0.0)
			mb_ping->latitude = navlat;
		else
			mb_ping->longitude = GSF_NULL_LATITUDE;

		/* get heading */
		if (heading != 0.0)
			mb_ping->heading = heading;
		else
			mb_ping->heading = GSF_NULL_HEADING;

		/* get speed */
		if (speed != 0.0)
			mb_ping->speed = speed / 1.852;
		else
			mb_ping->speed = speed;

		/* get draft */
		if (draft != 0.0)
			mb_ping->depth_corrector = draft;
		else
			mb_ping->depth_corrector = GSF_NULL_DEPTH_CORRECTOR;

		/* get roll pitch and heave */
		if (roll != 0.0)
			mb_ping->roll = roll;
		else
			mb_ping->roll = GSF_NULL_ROLL;
		if (pitch != 0.0)
			mb_ping->pitch = pitch;
		else
			mb_ping->pitch = GSF_NULL_PITCH;
		if (heave != 0.0)
			mb_ping->heave = heave;
		else
			mb_ping->heave = GSF_NULL_HEAVE;

		/* get scale factors */
		// gsfSetDefaultScaleFactor(mb_ping);
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
int mbsys_gsf_extract_svp(int verbose, void *mbio_ptr, void *store_ptr,
		int *kind, int *nsvp,
		double *depth, double *velocity,
		int *error)
{
	char	*function_name = "mbsys_gsf_extract_svp";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_gsf_struct *store;
	gsfDataID	    *dataID;
	gsfRecords	    *records;
	gsfSVP		    *svp;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mb_ptr:     %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_gsf_struct *) store_ptr;
	records = &(store->records);
	dataID = &(store->dataID);
	svp = &(records->svp);

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_VELOCITY_PROFILE)
		{
		/* get number of depth-velocity pairs */
		*nsvp = svp->number_points;

		/* get profile */
		for (i=0;i<*nsvp;i++)
			{
			depth[i] = svp->depth[i];
			velocity[i] = svp->sound_speed[i];
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
		fprintf(stderr,"\ndbg2  MBIO function <%s> completed\n",function_name);
		fprintf(stderr,"dbg2  Return values:\n");
		fprintf(stderr,"dbg2       kind:              %d\n",*kind);
		fprintf(stderr,"dbg2       nsvp:              %d\n",*nsvp);
		for (i=0;i<*nsvp;i++)
		    fprintf(stderr,"dbg2       depth[%d]: %f   velocity[%d]: %f\n",i, depth[i], i, velocity[i]);
		fprintf(stderr,"dbg2       error:             %d\n",*error);
		fprintf(stderr,"dbg2  Return status:\n");
		fprintf(stderr,"dbg2       status:            %d\n",status);
		}

	/* return status */
	return(status);
}
/*--------------------------------------------------------------------*/
int mbsys_gsf_insert_svp(int verbose, void *mbio_ptr, void *store_ptr,
		int nsvp,
		double *depth, double *velocity,
		int *error)
{
	char	*function_name = "mbsys_gsf_insert_svp";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_gsf_struct *store;
	gsfDataID	    *dataID;
	gsfRecords	    *records;
	gsfSVP		    *svp;
	int	i;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		fprintf(stderr,"dbg2       nsvp:       %d\n",nsvp);
		for (i=0;i<nsvp;i++)
		    fprintf(stderr,"dbg2       depth[%d]: %f   velocity[%d]: %f\n",i, depth[i], i, velocity[i]);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointer */
	store = (struct mbsys_gsf_struct *) store_ptr;
	records = &(store->records);
	dataID = &(store->dataID);
	svp = &(records->svp);

	/* insert data in structure */
	if (store->kind == MB_DATA_VELOCITY_PROFILE)
		{
		/* allocate memory if required */
		if (nsvp > svp->number_points
		    || svp->depth == NULL
		    || svp->sound_speed == NULL)
		    {
		    svp->depth = (double *) realloc(svp->depth, nsvp * sizeof(double));
		    svp->sound_speed = (double *) realloc(svp->sound_speed, nsvp * sizeof(double));
		    }

		/* get number of depth-velocity pairs */
		svp->number_points = nsvp;

		/* get profile */
		for (i=0;i<svp->number_points;i++)
			{
			svp->depth[i] = depth[i];
			svp->sound_speed[i] = velocity[i];
			}
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
int mbsys_gsf_copy(int verbose, void *mbio_ptr,
			void *store_ptr, void *copy_ptr,
			int *error)
{
	char	*function_name = "mbsys_gsf_copy";
	int	status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_gsf_struct *store;
	struct mbsys_gsf_struct *copy;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:    %d\n",verbose);
		fprintf(stderr,"dbg2       mbio_ptr:   %p\n",(void *)mbio_ptr);
		fprintf(stderr,"dbg2       store_ptr:  %p\n",(void *)store_ptr);
		fprintf(stderr,"dbg2       copy_ptr:   %p\n",(void *)copy_ptr);
		}

	/* get mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *) mbio_ptr;

	/* get data structure pointers */
	store = (struct mbsys_gsf_struct *) store_ptr;
	copy = (struct mbsys_gsf_struct *) copy_ptr;

	/* clear copy structure */
	gsfFree(&(copy->records));
	gsfCopyRecords(&(copy->records), &(store->records));
	copy->kind = store->kind;
	copy->dataID = store->dataID;

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
int mbsys_gsf_setscalefactors(int verbose, int reset_all, gsfSwathBathyPing *mb_ping, int *error)
{
	char	*function_name = "mbsys_gsf_setscalefactors";
	int	status = MB_SUCCESS;

	const double GSF_DEPTH_ASSUMED_HIGHEST_PRECISION = 10000;
	const double GSF_ACROSS_TRACK_ASSUMED_HIGHEST_PRECISION = 10000;
	const double GSF_ALONG_TRACK_ASSUMED_HIGHEST_PRECISION = 100000;
	const double GSF_TRAVEL_TIME_ASSUMED_HIGHEST_PRECISION = 10e6;
	const double GSF_BEAM_ANGLE_ASSUMED_HIGHEST_PRECISION = 1000;
	const double GSF_MEAN_CAL_AMPLITUDE_ASSUMED_HIGHEST_PRECISION = 10;
	const double GSF_MEAN_REL_AMPLITUDE_ASSUMED_HIGHEST_PRECISION = 100;
	const double GSF_ECHO_WIDTH_ASSUMED_HIGHEST_PRECISION = 10e5;
	const double GSF_QUALITY_FACTOR_ASSUMED_HIGHEST_PRECISION = 100;
	const double GSF_RECEIVE_HEAVE_ASSUMED_HIGHEST_PRECISION = 10000;
	const double GSF_DEPTH_ERROR_ASSUMED_HIGHEST_PRECISION = 10000;
	const double GSF_ACROSS_TRACK_ERROR_ASSUMED_HIGHEST_PRECISION = 10000;
	const double GSF_ALONG_TRACK_ERROR_ASSUMED_HIGHEST_PRECISION = 10000;
	const double GSF_NOMINAL_DEPTH_ASSUMED_HIGHEST_PRECISION = 10000;
	const double GSF_QUALITY_FLAGS_ASSUMED_HIGHEST_PRECISION = 100;
	const double GSF_BEAM_FLAGS_ASSUMED_HIGHEST_PRECISION = 100;
	const double GSF_SIGNAL_TO_NOISE_ASSUMED_HIGHEST_PRECISION = 100;
	const double GSF_BEAM_ANGLE_FORWARD_ASSUMED_HIGHEST_PRECISION = 100;
	const double GSF_VERTICAL_ERROR_ASSUMED_HIGHEST_PRECISION = 10000;
	const double GSF_HORIZONTAL_ERROR_ASSUMED_HIGHEST_PRECISION = 10000;
	const double GSF_SECTOR_NUMBER_ASSUMED_HIGHEST_PRECISION = 100;
	const double GSF_DETECTION_INFO_ASSUMED_HIGHEST_PRECISION = 100;
	const double GSF_INCIDENT_BEAM_ADJ_ASSUMED_HIGHEST_PRECISION = 100;
	const double GSF_SYSTEM_CLEANING_ASSUMED_HIGHEST_PRECISION = 100;
	const double GSF_DOPPLER_CORRECTION_ASSUMED_HIGHEST_PRECISION = 100;
	const double GSF_SONAR_VERT_UNCERT_ASSUMED_HIGHEST_PRECISION = 10000;
    
	int             i, j; /* iterators */
	double          *dptr; /* pointer to ping array */
	unsigned short  *usptr; /* pointer to ping array */
	unsigned char   *ucptr; /* pointer to ping array */
	int             id; /* type of ping array subrecord */
	double          max, min; /* min and max value of each ping array */
	double          max_scale_factor, min_scale_factor; /* min and max allowable size of values in ping array */
	double          highest_precision; /* starting value for multiplier */
	double		multiplier, offset, multiplier_min, multiplier_max;

	/* print input debug statements */
	if (verbose >= 2)
		{
		fprintf(stderr,"\ndbg2  MBIO function <%s> called\n",function_name);
		fprintf(stderr,"dbg2  Revision id: %s\n",rcs_id);
		fprintf(stderr,"dbg2  Input arguments:\n");
		fprintf(stderr,"dbg2       verbose:     %d\n",verbose);
		fprintf(stderr,"dbg2       reset_all:   %d\n",reset_all);
		fprintf(stderr,"dbg2       mb_ping:     %p\n",(void *)mb_ping);
		}

	for (i = 1; i <= GSF_MAX_PING_ARRAY_SUBRECORDS; i++)
		{
		dptr = NULL;
		usptr = NULL;
    
		switch (i)
			{
			case GSF_SWATH_BATHY_SUBRECORD_DEPTH_ARRAY:
				dptr = mb_ping->depth;
				highest_precision = GSF_DEPTH_ASSUMED_HIGHEST_PRECISION;
				id = GSF_SWATH_BATHY_SUBRECORD_DEPTH_ARRAY;
				if ((mb_ping->scaleFactors.scaleTable[i - 1].compressionFlag & 0xF0) == GSF_FIELD_SIZE_FOUR)
					{
					max_scale_factor = UINT_MAX;
					min_scale_factor = 0;
					}
				else
					{
					max_scale_factor = USHRT_MAX;
					min_scale_factor = 0;
					}
				break;
			case GSF_SWATH_BATHY_SUBRECORD_ACROSS_TRACK_ARRAY:
				dptr = mb_ping->across_track;
				highest_precision = GSF_ACROSS_TRACK_ASSUMED_HIGHEST_PRECISION;
				id = GSF_SWATH_BATHY_SUBRECORD_ACROSS_TRACK_ARRAY;
				if ((mb_ping->scaleFactors.scaleTable[i - 1].compressionFlag & 0xF0) == GSF_FIELD_SIZE_FOUR)
					{
					max_scale_factor = INT_MAX;
					min_scale_factor = INT_MIN;
					}
				else
					{
					max_scale_factor = SHRT_MAX;
					min_scale_factor = SHRT_MIN;
					}
				break;
			case GSF_SWATH_BATHY_SUBRECORD_ALONG_TRACK_ARRAY:
				dptr = mb_ping->along_track;
				highest_precision = GSF_ALONG_TRACK_ASSUMED_HIGHEST_PRECISION;
				id = GSF_SWATH_BATHY_SUBRECORD_ALONG_TRACK_ARRAY;
				if ((mb_ping->scaleFactors.scaleTable[i - 1].compressionFlag & 0xF0) == GSF_FIELD_SIZE_FOUR)
					{
					max_scale_factor = INT_MAX;
					min_scale_factor = INT_MIN;
					}
				else
					{
					max_scale_factor = SHRT_MAX;
					min_scale_factor = SHRT_MIN;
					}
				break;
			case GSF_SWATH_BATHY_SUBRECORD_TRAVEL_TIME_ARRAY:
				dptr = mb_ping->travel_time;
				highest_precision = GSF_TRAVEL_TIME_ASSUMED_HIGHEST_PRECISION;
				id = GSF_SWATH_BATHY_SUBRECORD_TRAVEL_TIME_ARRAY;
				max_scale_factor = USHRT_MAX;
				min_scale_factor = 0;
				break;
			case GSF_SWATH_BATHY_SUBRECORD_BEAM_ANGLE_ARRAY:
				dptr = mb_ping->beam_angle;
				highest_precision = GSF_BEAM_ANGLE_ASSUMED_HIGHEST_PRECISION;
				id = GSF_SWATH_BATHY_SUBRECORD_BEAM_ANGLE_ARRAY;
				if ((mb_ping->scaleFactors.scaleTable[i - 1].compressionFlag & 0xF0) == GSF_FIELD_SIZE_FOUR)
					{
					max_scale_factor = INT_MAX;
					min_scale_factor = INT_MIN;
					}
				else
					{
					max_scale_factor = SHRT_MAX;
					min_scale_factor = SHRT_MIN;
					}
				break;
			case GSF_SWATH_BATHY_SUBRECORD_MEAN_CAL_AMPLITUDE_ARRAY:
				dptr = mb_ping->mc_amplitude;
				highest_precision = GSF_MEAN_CAL_AMPLITUDE_ASSUMED_HIGHEST_PRECISION;
				id = GSF_SWATH_BATHY_SUBRECORD_MEAN_CAL_AMPLITUDE_ARRAY;
				max_scale_factor = SCHAR_MAX;
				min_scale_factor = SCHAR_MIN;
				break;
			case GSF_SWATH_BATHY_SUBRECORD_MEAN_REL_AMPLITUDE_ARRAY:
				dptr = mb_ping->mr_amplitude;
				highest_precision = GSF_MEAN_REL_AMPLITUDE_ASSUMED_HIGHEST_PRECISION;
				id = GSF_SWATH_BATHY_SUBRECORD_MEAN_REL_AMPLITUDE_ARRAY;
				max_scale_factor = UCHAR_MAX;
				min_scale_factor = 0;
				break;
			case GSF_SWATH_BATHY_SUBRECORD_ECHO_WIDTH_ARRAY:
				dptr = mb_ping->echo_width;
				highest_precision = GSF_ECHO_WIDTH_ASSUMED_HIGHEST_PRECISION;
				id = GSF_SWATH_BATHY_SUBRECORD_ECHO_WIDTH_ARRAY;
				max_scale_factor = UCHAR_MAX;
				min_scale_factor = 0;
				break;
			case GSF_SWATH_BATHY_SUBRECORD_QUALITY_FACTOR_ARRAY:
				dptr = mb_ping->quality_factor;
				highest_precision = GSF_QUALITY_FACTOR_ASSUMED_HIGHEST_PRECISION;
				id = GSF_SWATH_BATHY_SUBRECORD_QUALITY_FACTOR_ARRAY;
				max_scale_factor = UCHAR_MAX;
				min_scale_factor = 0;
				break;
			case GSF_SWATH_BATHY_SUBRECORD_RECEIVE_HEAVE_ARRAY:
				dptr = mb_ping->receive_heave;
				highest_precision = GSF_RECEIVE_HEAVE_ASSUMED_HIGHEST_PRECISION;
				id = GSF_SWATH_BATHY_SUBRECORD_RECEIVE_HEAVE_ARRAY;
				max_scale_factor = UCHAR_MAX;
				min_scale_factor = 0;
				break;
			case GSF_SWATH_BATHY_SUBRECORD_DEPTH_ERROR_ARRAY:
				dptr = mb_ping->depth_error;
				highest_precision = GSF_DEPTH_ERROR_ASSUMED_HIGHEST_PRECISION;
				id = GSF_SWATH_BATHY_SUBRECORD_DEPTH_ERROR_ARRAY;
				if ((mb_ping->scaleFactors.scaleTable[i - 1].compressionFlag & 0xF0) == GSF_FIELD_SIZE_FOUR)
					{
					max_scale_factor = UINT_MAX;
					min_scale_factor = 0;
					}
				else
					{
					max_scale_factor = USHRT_MAX;
					min_scale_factor = 0;
					}
				break;
			case GSF_SWATH_BATHY_SUBRECORD_ACROSS_TRACK_ERROR_ARRAY:
				dptr = mb_ping->across_track_error;
				highest_precision = GSF_ACROSS_TRACK_ERROR_ASSUMED_HIGHEST_PRECISION;
				id = GSF_SWATH_BATHY_SUBRECORD_ACROSS_TRACK_ERROR_ARRAY;
				if ((mb_ping->scaleFactors.scaleTable[i - 1].compressionFlag & 0xF0) == GSF_FIELD_SIZE_FOUR)
					{
					max_scale_factor = UINT_MAX;
					min_scale_factor = 0;
					}
				else
					{
					max_scale_factor = USHRT_MAX;
					min_scale_factor = 0;
					}
				break;
			case GSF_SWATH_BATHY_SUBRECORD_ALONG_TRACK_ERROR_ARRAY:
				dptr = mb_ping->along_track_error;
				highest_precision = GSF_ALONG_TRACK_ERROR_ASSUMED_HIGHEST_PRECISION;
				id = GSF_SWATH_BATHY_SUBRECORD_ALONG_TRACK_ERROR_ARRAY;
				if ((mb_ping->scaleFactors.scaleTable[i - 1].compressionFlag & 0xF0) == GSF_FIELD_SIZE_FOUR)
					{
					max_scale_factor = UINT_MAX;
					min_scale_factor = 0;
					}
				else
					{
					max_scale_factor = USHRT_MAX;
					min_scale_factor = 0;
					}
				break;
			case GSF_SWATH_BATHY_SUBRECORD_NOMINAL_DEPTH_ARRAY:
				dptr = mb_ping->nominal_depth;
				highest_precision = GSF_NOMINAL_DEPTH_ASSUMED_HIGHEST_PRECISION;
				id = GSF_SWATH_BATHY_SUBRECORD_NOMINAL_DEPTH_ARRAY;
				if ((mb_ping->scaleFactors.scaleTable[i - 1].compressionFlag & 0xF0) == GSF_FIELD_SIZE_FOUR)
					{
					max_scale_factor = UINT_MAX;
					min_scale_factor = 0;
					}
				else
					{
					max_scale_factor = USHRT_MAX;
					min_scale_factor = 0;
					}
				break;
			case GSF_SWATH_BATHY_SUBRECORD_QUALITY_FLAGS_ARRAY:
				ucptr = mb_ping->quality_flags;
				highest_precision = GSF_QUALITY_FLAGS_ASSUMED_HIGHEST_PRECISION;
				id = GSF_SWATH_BATHY_SUBRECORD_QUALITY_FLAGS_ARRAY;
				max_scale_factor = UCHAR_MAX;
				min_scale_factor = 0;
				break;
			case GSF_SWATH_BATHY_SUBRECORD_BEAM_FLAGS_ARRAY:
				ucptr = mb_ping->beam_flags;
				highest_precision = GSF_BEAM_FLAGS_ASSUMED_HIGHEST_PRECISION;
				id = GSF_SWATH_BATHY_SUBRECORD_BEAM_FLAGS_ARRAY;
				max_scale_factor = UCHAR_MAX;
				min_scale_factor = 0;
				break;
			case GSF_SWATH_BATHY_SUBRECORD_SIGNAL_TO_NOISE_ARRAY:
				dptr = mb_ping->signal_to_noise;
				highest_precision = GSF_SIGNAL_TO_NOISE_ASSUMED_HIGHEST_PRECISION;
				id = GSF_SWATH_BATHY_SUBRECORD_SIGNAL_TO_NOISE_ARRAY;
				max_scale_factor = UCHAR_MAX;
				min_scale_factor = 0;
				break;
			case GSF_SWATH_BATHY_SUBRECORD_BEAM_ANGLE_FORWARD_ARRAY:
				dptr = mb_ping->beam_angle_forward;
				highest_precision = GSF_BEAM_ANGLE_FORWARD_ASSUMED_HIGHEST_PRECISION;
				id = GSF_SWATH_BATHY_SUBRECORD_BEAM_ANGLE_FORWARD_ARRAY;
				if ((mb_ping->scaleFactors.scaleTable[i - 1].compressionFlag & 0xF0) == GSF_FIELD_SIZE_FOUR)
					{
					max_scale_factor = UINT_MAX;
					min_scale_factor = 0;
					}
				else
					{
					max_scale_factor = USHRT_MAX;
					min_scale_factor = 0;
					}
				break;
			case GSF_SWATH_BATHY_SUBRECORD_VERTICAL_ERROR_ARRAY:
				dptr = mb_ping->vertical_error;
				highest_precision = GSF_VERTICAL_ERROR_ASSUMED_HIGHEST_PRECISION;
				id = GSF_SWATH_BATHY_SUBRECORD_VERTICAL_ERROR_ARRAY;
				if ((mb_ping->scaleFactors.scaleTable[i - 1].compressionFlag & 0xF0) == GSF_FIELD_SIZE_FOUR)
					{
					max_scale_factor = UINT_MAX;
					min_scale_factor = 0;
					}
				else
					{
					max_scale_factor = USHRT_MAX;
					min_scale_factor = 0;
					}
				break;
			case GSF_SWATH_BATHY_SUBRECORD_HORIZONTAL_ERROR_ARRAY:
				dptr = mb_ping->horizontal_error;
				highest_precision = GSF_HORIZONTAL_ERROR_ASSUMED_HIGHEST_PRECISION;
				id = GSF_SWATH_BATHY_SUBRECORD_HORIZONTAL_ERROR_ARRAY;
				if ((mb_ping->scaleFactors.scaleTable[i - 1].compressionFlag & 0xF0) == GSF_FIELD_SIZE_FOUR)
					{
					max_scale_factor = UINT_MAX;
					min_scale_factor = 0;
					}
				else
					{
					max_scale_factor = USHRT_MAX;
					min_scale_factor = 0;
					}
				break;
			case GSF_SWATH_BATHY_SUBRECORD_SECTOR_NUMBER_ARRAY:
				usptr = mb_ping->sector_number;
				highest_precision = GSF_SECTOR_NUMBER_ASSUMED_HIGHEST_PRECISION;
				id = GSF_SWATH_BATHY_SUBRECORD_SECTOR_NUMBER_ARRAY;
				max_scale_factor = UCHAR_MAX;
				min_scale_factor = 0;
				break;
			case GSF_SWATH_BATHY_SUBRECORD_DETECTION_INFO_ARRAY:
				usptr = mb_ping->detection_info;
				highest_precision = GSF_DETECTION_INFO_ASSUMED_HIGHEST_PRECISION;
				id = GSF_SWATH_BATHY_SUBRECORD_DETECTION_INFO_ARRAY;
				max_scale_factor = UCHAR_MAX;
				min_scale_factor = 0;
				break;
			case GSF_SWATH_BATHY_SUBRECORD_INCIDENT_BEAM_ADJ_ARRAY:
				dptr = mb_ping->incident_beam_adj;
				highest_precision = GSF_INCIDENT_BEAM_ADJ_ASSUMED_HIGHEST_PRECISION;
				id = GSF_SWATH_BATHY_SUBRECORD_INCIDENT_BEAM_ADJ_ARRAY;
				max_scale_factor = SCHAR_MAX;
				min_scale_factor = SCHAR_MIN;
				break;
			case GSF_SWATH_BATHY_SUBRECORD_SYSTEM_CLEANING_ARRAY:
				usptr = mb_ping->system_cleaning;
				highest_precision = GSF_SYSTEM_CLEANING_ASSUMED_HIGHEST_PRECISION;
				id = GSF_SWATH_BATHY_SUBRECORD_SYSTEM_CLEANING_ARRAY;
				max_scale_factor = UCHAR_MAX;
				min_scale_factor = 0;
				break;
			case GSF_SWATH_BATHY_SUBRECORD_DOPPLER_CORRECTION_ARRAY:
				dptr = mb_ping->doppler_corr;
				highest_precision = GSF_DOPPLER_CORRECTION_ASSUMED_HIGHEST_PRECISION;
				id = GSF_SWATH_BATHY_SUBRECORD_DOPPLER_CORRECTION_ARRAY;
				max_scale_factor = SCHAR_MAX;
				min_scale_factor = SCHAR_MIN;
				break;
			case GSF_SWATH_BATHY_SUBRECORD_SONAR_VERT_UNCERT_ARRAY:
				dptr = mb_ping->sonar_vert_uncert;
				highest_precision = GSF_SONAR_VERT_UNCERT_ASSUMED_HIGHEST_PRECISION;
				id = GSF_SWATH_BATHY_SUBRECORD_SONAR_VERT_UNCERT_ARRAY;
				if ((mb_ping->scaleFactors.scaleTable[i - 1].compressionFlag & 0xF0) == GSF_FIELD_SIZE_FOUR)
					{
					max_scale_factor = UINT_MAX;
					min_scale_factor = 0;
					}
				else
					{
					max_scale_factor = USHRT_MAX;
					min_scale_factor = 0;
					}
				break;
    
			}
    
		if (dptr != NULL || usptr != NULL)
			{
			max = DBL_MIN;
			min = DBL_MAX;
	    
			if (dptr != NULL)
				{
				for (j = 0; j < mb_ping->number_beams; j++)
					{
					if (dptr[j] > max)
						max = dptr[j];
					if (dptr[j] < min)
						min = dptr[j];
					}
				}
			else if (usptr != NULL)
				{
				for (j = 0; j < mb_ping->number_beams; j++)
					{
					if (usptr[j] > max)
						max = (double) usptr[j];
					if (usptr[j] < min)
						min = (double) usptr[j];
					}
				}
			else if (ucptr != NULL)
				{
				for (j = 0; j < mb_ping->number_beams; j++)
					{
					if (ucptr[j] > max)
						max = (double) ucptr[j];
					if (ucptr[j] < min)
						min = (double) ucptr[j];
					}
				}
	    

			if (min_scale_factor == 0.0 && min < 0.0)
				offset = floor(-min) + 1.0;
			else if (id == 1 && mb_ping->depth_corrector > 0.0)
				offset = floor(MAX(-mb_ping->depth_corrector, -min)) + 1.0;				
			else
				offset = 0;
			
			/* calculate the ideal multiplier, but use no value larger than the max scale factor multiplier */
			multiplier_min = highest_precision;
			multiplier_max = highest_precision;
			if (max > 0.0 && max > offset)
				multiplier_max = floor(max_scale_factor / ( max + mb_ping->scaleFactors.scaleTable[id - 1].offset));
			if (min < 0.0 && min < -offset)
				multiplier_min = floor(min_scale_factor / ( min + mb_ping->scaleFactors.scaleTable[id - 1].offset));
			multiplier = MAX(MIN(multiplier_min, multiplier_max), 1.0);
			
			if (reset_all == MB_YES
				|| multiplier < mb_ping->scaleFactors.scaleTable[id - 1].multiplier)
				{
/* fprintf(stderr,"BEFORE Scale Factors %2d of %2d: minmax: %10f %10f compressionFlag:%5x offset:%10f multiplier:%10f\n",
id, GSF_MAX_PING_ARRAY_SUBRECORDS, min, max,
mb_ping->scaleFactors.scaleTable[id - 1].compressionFlag,
mb_ping->scaleFactors.scaleTable[id - 1].offset,
mb_ping->scaleFactors.scaleTable[id - 1].multiplier);
fprintf(stderr,"     highest_precision:%f offset:%f multiplier_min:%f multiplier_max:%f multiplier:%f\n",
highest_precision,offset,multiplier_min,multiplier_max,multiplier); */

				mb_ping->scaleFactors.scaleTable[id - 1].multiplier = multiplier;
				mb_ping->scaleFactors.scaleTable[id - 1].offset = offset;

/* fprintf(stderr,"AFTER  Scale Factors %2d of %2d: minmax: %10f %10f compressionFlag:%5x offset:%10f multiplier:%10f\n\n",
id, GSF_MAX_PING_ARRAY_SUBRECORDS, min, max,
mb_ping->scaleFactors.scaleTable[id - 1].compressionFlag,
mb_ping->scaleFactors.scaleTable[id - 1].offset,
mb_ping->scaleFactors.scaleTable[id - 1].multiplier); */
				}
			}
    
		}
				
	/* print output debug statements */
	if (verbose >= 4)
		{
		fprintf(stderr,"\ndbg4  GSF Scale Factors Calculated in MBIO function <%s>\n",function_name);
		for (i = 1; i <= GSF_MAX_PING_ARRAY_SUBRECORDS; i++)
			{
			fprintf(stderr,"dbg4       Scale Factors %2d of %2d: compressionFlag:%5x offset:%10f multiplier:%10f\n",
				i, GSF_MAX_PING_ARRAY_SUBRECORDS,
				mb_ping->scaleFactors.scaleTable[i - 1].compressionFlag,
				mb_ping->scaleFactors.scaleTable[i - 1].offset,
				mb_ping->scaleFactors.scaleTable[i - 1].multiplier);
			}
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
