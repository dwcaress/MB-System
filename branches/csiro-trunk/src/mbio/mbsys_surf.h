/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_surf.h	6/13/02
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
 * mbsys_surf.h defines the MBIO data structures for handling data from
 * SAM Electronics multibeam sonars in the Atlas processing format SURF.
 * The relevant sonars include Hydrosweep DS2, MD2 and Fansweep sonars.
 * The older  Hydrosweep DS and MD sonars produce data in different
 * formats (e.g. 21-24 and 101-102).
 * The data format associated with the SURF format is:
 *    MBSYS_SURF formats (code in mbsys_surf.c and mbsys_surf.h):
 *      MBF_SAMESURF : MBIO ID 181 - Vendor processing format
 *
 *
 * Author:	D. W. Caress
 * Author:	D. N. Chayes
 * Date:	June 13, 2002
 *
 * $Log: mbsys_surf.h,v $
 * Revision 5.9  2005/11/05 00:48:04  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.8  2003/11/24 21:09:09  caress
 * Implement Reinhard Holtkamp's suggested mods for better SURF format support.
 *
 * Revision 5.7  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.6  2003/03/10 20:04:45  caress
 * Added mr1pr library.
 *
 * Revision 5.5  2003/02/27 04:33:33  caress
 * Fixed handling of SURF format data.
 *
 * Revision 5.4  2002/07/20 20:42:40  caress
 * Release 5.0.beta20
 *
 * Revision 5.3  2001/12/18 04:27:45  caress
 * Release 5.0.beta11.
 *
 * Revision 5.2  2001/08/10 22:41:19  dcaress
 * Release 5.0.beta07
 *
 * Revision 5.1  2001-07-19 17:32:54-07  caress
 * Release 5.0.beta03
 *
 * Revision 5.0  2001/06/29  22:49:07  caress
 * Added support for HSDS2RAW
 *
 *
 */
/*
 * Notes on the MBSYS_SURF data structure:
 *
 * 1) STN Atlas Marine Electronics (aka SAM) sonars write raw data in real-time
 *    as binary XDR encoded data. Files are stored on disk by the HYDROMAP
 *    Online workstation. The workstation on the Ewing is an HP Vectra
 *    running SuSe Linux (2.2 kernel.)
 *
 * 2) The HYDROMAP Offline software translates the data into a processing
 *    format called SURF
 *
 * 3) Multiple parallel files are created. For example:
 *      The '.six' file contains global data and reference tables
 *      The '.sda' file contains sounding dependent mass data
 *
 * 4) SAM provides an open source library (SAPI) to read and write
 *    SURF data
 */

/* include mb_define.h */
#ifndef MB_DEFINE_DEF
#include "../../include/mb_define.h"
#endif

/* include SAPI header file */
#ifndef _SAPI
#include "sapi.h"
#endif

#define	MBSYS_SURF_MAXBEAMS		1440
#define	MBSYS_SURF_MAXCVALUES		1024
#define	MBSYS_SURF_MAXCPOS		16
#define	MBSYS_SURF_MAXRXSETS		1024
#define	MBSYS_SURF_MAXTXSETS		16
#define	MBSYS_SURF_MAXPIXELS		4096

/* internal data structure for survey data */
struct mbsys_surf_struct
	{
	/* MBIO data record kind */
	int		kind;

	/* global info initialization flag */
	int		initialized;

	/* surf global info */
	char	NameOfShip[LABEL_SIZE];
	char	TypeOfSounder[LABEL_SIZE];
	char	NameOfSounder[LABEL_SIZE];
	int	NrSoundings;
	int	NrBeams;
	int NrSidescan;
	int	NrDepths;			/*	should be either 0 or NrBeams */
	int	NrTravelTimes;		/*	should be either 0 or NrBeams */
	int NrRxSets;			/*	should be either 0 or NrBeams */
	int NrAmplitudes;		/*	should be either 0 or NrBeams */
	int NrExtAmplitudes;	/*	should be either 0 or NrBeams */
	int NrTxSets;			/*  missing in "SurfTxParameter"  */
	int	SAPI_posPresentationIsRad;
	int	NrPositionsensors;
	int	NrSoundvelocityProfiles;
	int	NrEvents;
	int	NrPolygonElements;
	double	AbsoluteStartTimeOfProfile;

	/* SURF structures */
	SurfGlobalData			GlobalData;
	SurfStatistics			Statistics;
	SurfPositionAnySensor		PositionSensor[MBSYS_SURF_MAXCPOS];
	SurfSoundingData		SoundingData;
	SurfTransducerParameterTable	ActualTransducerTable;
	SurfMultiBeamAngleTable		ActualAngleTable;
	float				reserved1[MBSYS_SURF_MAXBEAMS - 1];
	SurfCProfileTable		ActualCProfileTable;
	CProfileValues			reserved2[MBSYS_SURF_MAXCVALUES - 1];
	SurfCenterPosition		CenterPosition[MBSYS_SURF_MAXCPOS];
	SurfSingleBeamDepth		SingleBeamDepth;
	SurfMultiBeamDepth		MultiBeamDepth[MBSYS_SURF_MAXBEAMS];
	SurfMultiBeamTT			MultiBeamTraveltime[MBSYS_SURF_MAXBEAMS];
	SurfMultiBeamReceive		MultiBeamReceiveParams[MBSYS_SURF_MAXBEAMS];
	SurfAmplitudes			MultibeamBeamAmplitudes[MBSYS_SURF_MAXBEAMS];
	SurfExtendedAmplitudes		MultibeamExtendedBeamAmplitudes[MBSYS_SURF_MAXBEAMS];
	SurfSignalParameter		MultibeamSignalParameters;
	TvgRxSets			reserved3[MBSYS_SURF_MAXRXSETS - 1];
	SurfTxParameter			MultibeamTransmitterParameters;
	TxSets				reserved4[MBSYS_SURF_MAXTXSETS - 1];
	SurfSidescanData		SidescanData;
	u_char				reserved5[MBSYS_SURF_MAXPIXELS - 1];
	};

/* system specific function prototypes */
int mbsys_surf_alloc(int verbose, void *mbio_ptr, void **store_ptr,
			int *error);
int mbsys_surf_deall(int verbose, void *mbio_ptr, void **store_ptr,
			int *error);
int mbsys_surf_dimensions(int verbose, void *mbio_ptr, void *store_ptr, 
			int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_surf_extract(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp,
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_surf_insert(int verbose, void *mbio_ptr, void *store_ptr,
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp,
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_surf_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles,
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset,
			double *draft, double *ssv, int *error);
int mbsys_surf_detects(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams, int *detects, int *error);
int mbsys_surf_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transducer_depth, double *altitude,
			int *error);
int mbsys_surf_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft,
			double *roll, double *pitch, double *heave,
			int *error);
int mbsys_surf_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft,
			double roll, double pitch, double heave,
			int *error);
int mbsys_surf_extract_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind,
			int *nsvp,
			double *depth, double *velocity,
			int *error);
int mbsys_surf_insert_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int nsvp,
			double *depth, double *velocity,
			int *error);
int mbsys_surf_copy(int verbose, void *mbio_ptr,
			void *store_ptr, void *copy_ptr,
			int *error);

