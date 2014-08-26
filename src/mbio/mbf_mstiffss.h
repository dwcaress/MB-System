/*--------------------------------------------------------------------
 *    The MB-system:	mbf_mstiffss.h	10/14/94
 *	$Id$
 *
 *    Copyright (c) 1998-2014 by
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
 * mbf_mstiffss.h defines the data structure used by MBIO functions
 * to store sidescan data read from the MBF_MSTIFFSS format (MBIO id 131).
 *
 * Author:	D. W. Caress
 * Date:	April 7,  1998
 *
 * $Log: mbf_mstiffss.h,v $
 * Revision 5.3  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.2  2003/01/15 20:51:48  caress
 * Release 5.0.beta28
 *
 * Revision 5.1  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.0  2000/12/01 22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.1  2000/09/30  06:34:20  caress
 * Snapshot for Dale.
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
 *
 */
/*
 * Notes on the MBF_MSTIFFSS data format:
 *   1. The MSTIFF data format is used to store raw sidescan data from
 *      Sea Scan sidescan sonars. This format is a variant of
 *      the TIFF image format with navigation and other information
 *      embedded within the file.
 *   2. The file structure consists of a bunch of pointers to
 *      data objects at various arbitrary locations within the
 *      file. The header contains a pointer to the location of
 *      the "image file directory", which in turn contains
 *      pointers to the locations of data arrays within the file.
 *   3. As far as MB-System is concerned,  this is a read-only
 *      data format.
 *   4. The raw sidescan data in the file consists of 1000 pings.
 *      Each ping produces two 512 sample arrays - one for
 *      each side (port and starboard).
 *   5. The sidescan data is not slant range corrected - the
 *      bottom detect and slant range correction is done on
 *      input by MBIO. The data stored internally by MBIO
 *      is slant range corrected.
 *   6. The MSTIFF files contain lots of information not used by
 *      MBIO,  including images of the data derived from a
 *      realtime display.
 *   7. Comments are not supported in this format.
 *   8. Each of the possible data fields is identified by a
 *      unique tag:
 *          Tag        Field Type                Default
 *          ---        ----------                -------
 *          273        Annotation
 *          272        AnnotationCount
 *          279        Annotation2
 *          278        Annotation2Count
 *          281        Annotation3
 *          280        Annotation3Count
 *          260        BinsPerChannel#
 *          258        BitsPerBin#                8 bits per sample (2 MSB = 0)
 *          254        Compression               1 (no compression)
 *          255        CondensedImage
 *          301        CreatorVersion
 *          256        Description
 *          287        Fathometer
 *          296        Fathometer2
 *          286        FathometerCount
 *          257        History
 *          263        LeftChannel#
 *          299        LeftChannel2#
 *          289        Magnetometer
 *          288        MagnetometerCount
 *          291        MagnetometerParms
 *          303        MagnetometerParms2
 *          269        Marker
 *          268        MarkerCount
 *          277        Marker2
 *          276        Marker2Count
 *          284        Marker3
 *          283        Marker3Count
 *          295        Marker4
 *          294        Marker4Count
 *          307        Marker5
 *          306        Marker5Count
 *          267        NavInfo#
 *          275        NavInfo2#
 *          282        NavInfo3#
 *          293        NavInfo4#
 *          297        NavInfo5#
 *          308        NavInfo6#
 *          266        NavInfoCount
 *          304        NavInterpolationTimeout
 *          274        PingNavInfo
 *          264        RightChannel#
 *          300        RightChannel2#
 *          261        ScrollDirection
 *          265        SonarDataInfo#
 *          292        SonarDataInfo2#
 *          298        SonarDataInfo3#
 *          259        SonarLines#
 *          271        SurveyPlotterImage
 *          270        SurveyPlotterParms
 *          290        SurveyPlotterParms2
 *          302        SurveyPlotterParms3
 *          305        SurveyPlotterParms4
 *          262        TimeCorrelation#
 *          285        Y2KTimeCorrelation#
 */

/* size of MSTIFFSS reading buffer */
#define MBF_MSTIFFSS_BUFFERSIZE 1024

/* number of sidescan pixels for Sea Scan sidescan sonars */
#define MBF_MSTIFFSS_PIXELS	1024

/* threshold of sidescan values for detecting bottom return */
#define MBF_MSTIFF_TRANSMIT_BINS	10
#define MBF_MSTIFF_BOTTOM_THRESHOLD	10

/* MSTIFF data field tags */
#define Annotation	273
#define AnnotationCount	272
#define Annotation2	279
#define Annotation2Count	278
#define Annotation3	281
#define Annotation3Count	280
#define BinsPerChannel	260
#define BitsPerBin	258
#define Compression	254
#define CondensedImage	255
#define CreatorVersion	301
#define Description	256
#define Fathometer	287
#define Fathometer2	296
#define FathometerCount	286
#define History	257
#define LeftChannel	263
#define LeftChannel2	299
#define Magnetometer	289
#define MagnetometerCount	288
#define MagnetometerParms	291
#define MagnetometerParms2	303
#define Marker	269
#define MarkerCount	268
#define Marker2	277
#define Marker2Count	276
#define Marker3	284
#define Marker3Count	283
#define Marker4	295
#define Marker4Count	294
#define Marker5	307
#define Marker5Count	306
#define NavInfo	267
#define NavInfo2	275
#define NavInfo3	282
#define NavInfo4	293
#define NavInfo5	297
#define NavInfo6	308
#define NavInfoCount	266
#define NavInterpolationTimeout	304
#define PingNavInfo	274
#define RightChannel	264
#define RightChannel2	300
#define ScrollDirection	261
#define SonarDataInfo	265
#define SonarDataInfo2	292
#define SonarDataInfo3	298
#define SonarLines	259
#define SurveyPlotterImage	271
#define SurveyPlotterParms	270
#define SurveyPlotterParms2	290
#define SurveyPlotterParms3	302
#define SurveyPlotterParms4	305
#define TimeCorrelation	262
#define Y2KTimeCorrelation	285

#define FREQ_150 0
#define FREQ_300 1
#define FREQ_600 2
#define FREQ_1200 3
#define FREQ_UNKNOWN 4

struct mbf_mstiffss_struct
	{
	/* time stamp */
	double	time_d;		/* unix time */

	/* position */
	double	lat;		/* latitude in degrees */
	double	lon;		/* longitude in degrees */

	/* other values */
	double	heading;	    /* heading in degrees */
	double	course;	    	    /* course made good in degrees */
	double	speed;		    /* fore-aft speed in km/hr */
	double	altitude;	    /* altitude in meters */
	double	slant_range_max;    /* meters */
	double	range_delay;	    /* meters */
	double	sample_interval;    /* meters */
	double	sonar_depth;	    /* sonar depth in meters */
	double	frequency;    /* sonar frequency in Hz */

	/* sidescan data */
	int	pixels_ss;	/* number of pixels */
	unsigned char	ss[MBF_MSTIFFSS_PIXELS];
	double	ssacrosstrack[MBF_MSTIFFSS_PIXELS];
	};
