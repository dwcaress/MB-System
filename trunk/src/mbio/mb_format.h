/*--------------------------------------------------------------------
 *    The MB-system:	mb_format.h	1/19/93
 *    $Id$
 *
 *    Copyright (c) 1993-2012 by
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
 * mb_format.h defines data format identifiers used by MBIO functions 
 *
 * Author:	D. W. Caress
 * Date:	January 19, 1993
 *
 * $Log: mb_format.h,v $
 * Revision 5.27  2008/12/05 17:32:52  caress
 * Check-in mods 5 December 2008 including contributions from Gordon Keith.
 *
 * Revision 5.26  2008/10/17 07:30:22  caress
 * Added format 26 supporting Hydrosweep DS data used by SOPAC.
 *
 * Revision 5.25  2008/07/19 07:41:14  caress
 * Added formats 191 and 192 to support Imagenex Delta T multibeam data.
 *
 * Revision 5.24  2008/05/16 22:56:24  caress
 * Release 5.1.1beta18.
 *
 * Revision 5.23  2008/03/01 09:12:52  caress
 * Added support for Simrad EM710 multibeam in new formats 58 and 59.
 *
 * Revision 5.22  2007/10/17 20:26:03  caress
 * Release 5.1.1beta11
 *
 * Revision 5.21  2006/11/10 22:36:04  caress
 * Working towards release 5.1.0
 *
 * Revision 5.20  2006/10/05 18:58:28  caress
 * Changes for 5.1.0beta4
 *
 * Revision 5.19  2006/03/06 21:47:48  caress
 * Implemented changes suggested by Bob Courtney of the Geological Survey of Canada to support translating Reson data to GSF.
 *
 * Revision 5.18  2005/06/04 04:15:59  caress
 * Support for Edgetech Jstar format (id 132 and 133).
 *
 * Revision 5.17  2004/11/06 03:55:15  caress
 * Working to support the Reson 7k format.
 *
 * Revision 5.16  2004/04/27 01:46:13  caress
 * Various updates of April 26, 2004.
 *
 * Revision 5.15  2003/09/23 21:02:45  caress
 * Added formats 168 and 169 for reading xyt and yxt triples (topography instead of depth).
 *
 * Revision 5.14  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.13  2003/03/10 20:04:45  caress
 * Added mr1pr library.
 *
 * Revision 5.12  2002/09/19 22:19:00  caress
 * Release 5.0.beta23
 *
 * Revision 5.11  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.10  2002/07/20 20:42:40  caress
 * Release 5.0.beta20
 *
 * Revision 5.9  2002/05/29 23:40:48  caress
 * Release 5.0.beta18
 *
 * Revision 5.8  2002/05/02 04:00:41  caress
 * Release 5.0.beta17
 *
 * Revision 5.7  2001/09/17 23:25:13  caress
 * Added format 84
 *
 * Revision 5.6  2001/07/20 17:00:20  caress
 * Added RCS controlled update date and revision string.
 *
 * Revision 5.5  2001/07/20  00:32:54  caress
 * Release 5.0.beta03
 *
 * Revision 5.4  2001/06/30 17:40:14  caress
 * Release 5.0.beta02
 *
 * Revision 5.3  2001/04/06  22:05:59  caress
 * Consolidated xse formats into one format.
 *
 * Revision 5.2  2001/03/22  20:50:02  caress
 * Trying to make version 5.0.beta0
 *
 * Revision 5.1  2000/12/10  20:26:50  caress
 * Version 5.0.alpha02
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.29  2000/09/30  06:29:44  caress
 * Snapshot for Dale.
 *
 * Revision 4.28  2000/07/19  03:53:11  caress
 * Added some new formats.
 *
 * Revision 4.27  1999/10/21  22:40:10  caress
 * Added MBPRONAV format.
 *
 * Revision 4.26  1999/09/14  20:39:11  caress
 * Fixed bugs handling HSMD
 *
 * Revision 4.25  1999/08/08  04:16:03  caress
 * Added ELMK2XSE format.
 *
 * Revision 4.24  1999/07/16 19:24:15  caress
 * Yet another version.
 *
 * Revision 4.23  1999/04/02  00:54:32  caress
 * Changed nav record type for Elac data.
 *
 * Revision 4.22  1999/03/31  18:11:35  caress
 * MB-System 4.6beta7
 *
 * Revision 4.21  1999/02/04  23:52:54  caress
 * MB-System version 4.6beta7
 *
 * Revision 4.20  1999/01/01  23:41:06  caress
 * MB-System version 4.6beta6
 *
 * Revision 4.19  1998/12/17  23:01:15  caress
 * MB-System version 4.6beta4
 *
 * Revision 4.18  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
 *
 * Revision 4.17  1997/09/15  19:06:40  caress
 * Real Version 4.5
 *
 * Revision 4.16  1997/07/25  14:19:53  caress
 * Version 4.5beta2.
 * Much mucking, particularly with Simrad formats.
 *
 * Revision 4.15  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.15  1997/04/17  15:07:36  caress
 * MB-System 4.5 Beta Release
 *
 * Revision 4.14  1996/08/26  17:24:56  caress
 * Release 4.4 revision.
 *
 * Revision 4.13  1996/08/05  15:25:43  caress
 * Just redid i/o for Simrad sonars, including adding EM12S and EM121 support.
 *
 * Revision 4.12  1996/04/22  10:59:01  caress
 * Added SBIFREMR format.
 *
 * Revision 4.11  1996/03/12  17:23:31  caress
 * Added format 63, short HMR1 processing format.
 *
 * Revision 4.10  1996/01/26  21:27:27  caress
 * Version 4.3 distribution.
 *
 * Revision 4.9  1995/09/28  18:10:48  caress
 * Various bug fixes working toward release 4.3.
 *
 * Revision 4.8  1995/07/13  19:15:09  caress
 * Intermediate check-in during major bug-fixing flail.
 *
 * Revision 4.7  1995/02/17  22:22:20  caress
 * Changed flag tables.
 *
 * Revision 4.6  1995/01/10  17:34:16  caress
 * Added fore-aft beamwidth table.
 *
 * Revision 4.5  1994/12/21  20:21:09  caress
 * Changes to support high resolution SeaBeam 2000 sidescan files
 * from R/V Melville data.
 *
 * Revision 4.4  1994/10/21  12:11:53  caress
 * Release V4.0
 *
 * Revision 4.3  1994/07/29  18:46:51  caress
 * Changes associated with supporting Lynx OS (byte swapped) and
 * using unix second time base (for time_d values).
 *
 * Revision 4.2  1994/04/11  23:22:28  caress
 * Added table of which formats have travel time data. This
 * table is called: mb_traveltime_table
 *
 * Revision 4.1  1994/04/09  15:49:21  caress
 * Altered to fit latest iteration of SeaBeam 2100 vendor format.
 *
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
 * Revision 4.8  1994/03/05  22:51:44  caress
 * Added ability to handle Simrad EM12 system and
 * format MBF_EM12DARW.
 *
 * Revision 4.7  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.6  1994/02/22  21:49:10  caress
 * Fixed MBLDEOIH id number at 61 instead of 71.
 *
 * Revision 4.5  1994/02/21  21:00:36  caress
 * Fixed the SBSIOMRG info message.
 *
 * Revision 4.4  1994/02/21  19:50:27  caress
 * Reset some format description messages.
 *
 * Revision 4.3  1994/02/20  03:13:19  caress
 * Fixed definition of mb_amp_flag_table.
 *
 * Revision 4.2  1994/02/18  20:33:25  caress
 * Changed and added some format tables.
 *
 * Revision 4.1  1994/02/17  20:30:06  caress
 * Set maximum number of sidescan pixels for MBF_MBLDEOIH
 * format to 10000.
 *
 * Revision 3.2  1993/06/13  17:14:48  sohara
 * added 0th value to button_name list
 *
 * Revision 3.1  1993/06/13  16:01:22  sohara
 * added mb_button_name values
 *
 * Revision 3.0  1993/04/23  15:50:54  dale
 * Initial version
 *
 */

/* make sure mb_status.h has been included */
#ifndef MB_STATUS_DEF
#include "mb_status.h"
#endif

/* include this code only once */
#ifndef MB_FORMAT_DEF
#define MB_FORMAT_DEF

/* define date of last format update */
#define	MB_FORMAT_UPDATEDATE	"$Id$ $Revision: $"

/* Supported swath sonar systems */
#define	MB_SYS_NONE		0
#define	MB_SYS_SB		1
#define	MB_SYS_HSDS		2
#define	MB_SYS_SB2000		3
#define	MB_SYS_SB2100		4
#define	MB_SYS_SIMRAD		5
#define	MB_SYS_SIMRAD2		6
#define	MB_SYS_SIMRAD3		7
#define	MB_SYS_MR1		8
#define	MB_SYS_MR1B		9
#define	MB_SYS_MR1V2001		10
#define	MB_SYS_LDEOIH		11
#define	MB_SYS_RESON		12
#define	MB_SYS_RESON8K		13
#define	MB_SYS_ELAC		14
#define	MB_SYS_ELACMK2		15
#define MB_SYS_HSMD		16
#define MB_SYS_DSL		17
#define MB_SYS_GSF		18
#define MB_SYS_MSTIFF		19
#define MB_SYS_OIC		20
#define MB_SYS_HDCS		21
#define MB_SYS_SINGLEBEAM	22
#define MB_SYS_XSE		23
#define MB_SYS_HS10		24
#define	MB_SYS_NETCDF		25
#define	MB_SYS_ATLAS		26
#define	MB_SYS_NAVNETCDF	27
#define	MB_SYS_SURF		28
#define	MB_SYS_RESON7K		29
#define	MB_SYS_JSTAR		30
#define	MB_SYS_IMAGE83P		31
#define	MB_SYS_HYSWEEP		32

/* Number of supported MBIO data formats */
#define	MB_FORMATS	71

/* Data formats supported by MBIO */
#define MBF_DATALIST	-1
#define MBF_NONE	0
#define	MBF_SBSIOMRG	11	/* SeaBeam, 16 beam, bathymetry, 
 					binary, uncentered, SIO. */
#define	MBF_SBSIOCEN	12	/* SeaBeam, 19 beam, bathymetry, 
 					binary, centered, SIO. */
#define	MBF_SBSIOLSI	13	/* SeaBeam, 19 beam, bathymetry, 
 					binary, centered, obsolete, SIO. */
#define	MBF_SBURICEN	14	/* SeaBeam, 19 beam, bathymetry, 
 					binary, centered, URI. */
#define	MBF_SBURIVAX	15	/* SeaBeam, 19 beam, bathymetry, 
 					binary, centered, 
					VAX byte order, URI. */
#define	MBF_SBSIOSWB	16	/* SeaBeam, 19 beam, bathymetry, 
 					binary, centered, 
					swath-bathy, SIO. */
#define	MBF_SBIFREMR	17	/* SeaBeam, 19 beam, bathymetry, 
 					ascii, centered, 
					sounding-oriented, IFREMER. */
#define	MBF_HSATLRAW	21	/* Hydrosweep DS raw format, 59 beam, 
 					bathymetry and amplitude, 
  					ascii, Atlas Electronik. */
#define	MBF_HSLDEDMB	22	/* Hydrosweep DS, 59 beam, bathymetry, 
 					binary, NRL. */
#define	MBF_HSURICEN	23	/* Hydrosweep DS, 59 beam, bathymetry, 
 					binary, URI. */
#define	MBF_HSLDEOIH	24	/* Hydrosweep DS in-house format, 59 beam,
 					bathymetry and amplitude, 
 					binary, centered, L-DEO. */
#define	MBF_HSURIVAX	25	/* Hydrosweep DS, 59 beam, bathymetry, 
 					binary, VAX byte order, URI. */
#define	MBF_HSUNKNWN	26	/* Hydrosweep DS, 59 beam, bathymetry, 
 					bathymetry and amplitude, 
					ascii, unknown origin, SOPAC. */
#define	MBF_SB2000RW	31	/* SeaBeam 2000 vender format,
					151 beam bathymetry and amplitude,
					2000 pixel sidescan, ascii + binary, 
 					SeaBeam Instruments. */
#define	MBF_SB2000SB	32	/* SeaBeam 2000, 121 beam bathymetry, 
					binary, swath-bathy, SIO. */
#define	MBF_SB2000SS	33	/* SeaBeam 2000, 1000 pixel sidescan, 
					binary, swath-bathy, SIO. */
#define	MBF_SB2100RW	41	/* SeaBeam 2100 series vender format, 
					151 beam bathymetry and amplitude,
					2000 pixel sidescan, ascii + binary, 
					centered, SeaBeam Instruments */ 
#define	MBF_SB2100B1	42	/* SeaBeam 2100 series vendor format, 
					151 beam bathymetry and amplitude,
					2000 pixel sidescan, binary, centered,
					SeaBeam Instruments and L-DEO */ 
#define	MBF_SB2100B2	43	/* SeaBeam 2100 series vendor format, 
					151 beam bathymetry and amplitude,
					binary, centered,
					SeaBeam Instruments and L-DEO */ 
#define	MBF_EMOLDRAW	51	/* Old Simrad vendor multibeam format,
					Simrad EM1000, EM12S, EM12D, 
					EM121 multibeam sonars,
					bathymetry, amplitude, and sidescan,
					60 beams for EM1000, 81 beams for EM12S/D,
					121 beams for EM121, variable pixels,
					ascii + binary, Simrad. */ 
#define	MBF_EM12IFRM	53	/* Simrad EM12S/EM12D multibeam archive format, 
					81 beam bathymetry and 
					amplitude, 
					binary, read-only, IFREMER */ 
#define	MBF_EM12DARW	54	/* Simrad EM12 RRS Darwin processed format, 
					81 beam, bathymetry and amplitude,
					binary, centered, Oxford University */ 
#define	MBF_EM300RAW	56	/* Simrad EM300/EM3000 multibeam vendor format, 
					up to 254 beam bathymetry and 
					amplitude, variable pixel sidescan, 
					ascii + binary, Simrad */ 
#define	MBF_EM300MBA	57	/* Simrad EM300/EM3000 multibeam processing format, 
					up to 254 beam bathymetry and 
					amplitude, variable pixel sidescan, 
					ascii + binary, MBARI */ 
#define	MBF_EM710RAW	58	/* Kongsberg EM122, EM302, EM710 multibeam vendor format, 
					up to 400 beam bathymetry and 
					amplitude, variable pixel sidescan, 
					binary, Kongsberg */ 
#define	MBF_EM710MBA	59	/* Kongsberg EM122, EM302, EM710 multibeam processing format, 
					up to 400 beam bathymetry and 
					amplitude, variable pixel sidescan, 
					binary, MBARI */ 
#define	MBF_MR1PRHIG	61	/* Obsolete MR1 post processed format, 
					variable beam bathymetry, variable
					pixel sidescan, xdr binary, SOEST */ 
#define	MBF_MR1ALDEO	62	/* MR1 Lamont format with travel times, 
					variable beam bathymetry, variable
					pixel sidescan, xdr binary, L-DEO */ 
#define	MBF_MR1BLDEO	63	/* MR1 small Lamont format with travel times, 
					variable beam bathymetry, variable
					pixel sidescan, xdr binary, L-DEO */ 
#define	MBF_MR1PRVR2	64	/* MR1 post processed format, 
					variable beam bathymetry, variable
					pixel sidescan, xdr binary, SOEST */ 
#define	MBF_MBLDEOIH	71	/* Generic in-house multibeam, variable beam, 
 					bathymetry, amplitude, and sidescan
 					binary, centered, L-DEO. */
#define	MBF_MBNETCDF	75	/* CARAIBES CDF multibeam, variable beam, 
 					netCDF, IFREMER. */
#define	MBF_MBNCDFXT	76	/* CARAIBES CDF multibeam, variable beam,
 					netCDF, IFREMER. - extended format */
#define	MBF_CBAT9001	81	/* Reson SeaBat 9001 multibeam, 60 beams
 					bathymetry and amplitude,
 					binary, University of New Brunswick. */
#define	MBF_CBAT8101	82	/* Reson SeaBat 8101 multibeam, 101 beams
 					bathymetry and amplitude,
 					binary, SeaBeam Instruments. */
#define	MBF_HYPC8101	83	/* Reson SeaBat 8101 multibeam, 101 beams
 					bathymetry,
 					ASCII, read-only,
					Coastal Oceanographics. */
#define	MBF_XTFR8101	84	/* Reson SeaBat 8XXX multibeam, 250 beams
 					bathymetry,
 					binary, read-only,
					Triton-Elics XTF format. */
#define	MBF_RESONS8K	85	/* Reson SeaBat 8XXX multibeam, 250 beams
 					bathymetry,
 					binary, read-only,
					Reson 6042 format. */
#define	MBF_SBATPROC	86	/* Reson SeaBat 8XXX multibeam, 250 beams
 					bathymetry,
 					binary,
					MBARI processing format. */
#define	MBF_BCHRTUNB	91	/* Elac BottomChart multibeam, 56 beams
 					bathymetry and amplitude,
 					binary, University of New Brunswick. */
#define	MBF_ELMK2UNB	92	/* Elac BottomChart multibeam, 56 beams
 					bathymetry and amplitude,
 					binary, University of New Brunswick. */
#define	MBF_BCHRXUNB	93	/* Elac BottomChart multibeam, 56 beams
 					bathymetry and amplitude,
 					binary, University of New Brunswick. */
#define	MBF_L3XSERAW	94	/* ELAC/SeaBeam XSE vendor format
					Bottomchart MkII 50 kHz and 180 kHz multibeam, 
					SeaBeam 2120 20 KHz multibeam,
					bathymetry, amplitude and sidescan,
					variable beams and pixels, binary, 
					L3 Communications (Elac Nautik 
					and SeaBeam Instruments). */
#define MBF_HSMDARAW    101     /* Hydroseep MD multibeam raw format, 40 beam 
					bathymetry, 160 pixel sidescan,
					xdr binary, Atlas Electronik. */
#define MBF_HSMDLDIH    102     /* Hydroseep MD multibeam in-house format, 
					40 beam bathymetry, 160 pixel sidescan,
					binary, L-DEO. */
#define MBF_DSL120PF    111     /* WHOI DSL AMS-120 deep-tow, 
					2048 beam bathymetry, 2048 pixel sidescan,
					binary, parallel files, WHOI DSL. */
#define MBF_DSL120SF    112     /* WHOI DSL AMS-120 deep-tow, 
					2048 beam bathymetry, 2048 pixel sidescan,
					binary, single files, WHOI DSL. */
#define MBF_GSFGENMB    121     /* SAIC Generic Sensor Format (GSF), 
					variable beams,  bathymetry and amplitude,
					binary, single files, SAIC. */
#define MBF_MSTIFFSS    131     /* MSTIFF sidescan format,
					variable pixels,  sidescan, 
					binary TIFF variant, single files, Sea Scan */
#define MBF_EDGJSTAR    132     /* Edgetech Jstar format
					variable pixels, dual frequency sidescan and subbottom,
                      			binary SEGY variant, single files, low frequency sidescan
					returned as survey data, Edgetech. */
#define MBF_EDGJSTR2    133     /* Edgetech Jstar format
					variable pixels, dual frequency sidescan and subbottom,
                      			binary SEGY variant, single files, high frequency sidescan
					returned as survey data, Edgetech. */
#define MBF_OICGEODA    141     /* OIC swath sonar format, variable beam 
					bathymetry and amplitude, variable
					pixel sidescan, binary, Oceanic Imaging 
					Consultants */ 
#define MBF_OICMBARI    142     /* OIC-style extended swath sonar format, variable  
					beam bathymetry and amplitude, variable
					pixel sidescan, binary, MBARI */ 
#define MBF_OMGHDCSJ    151     /* UNB OMG HDCS format, variable  
					beam bathymetry and amplitude, variable
					pixel sidescan, binary, UNB */ 
#define MBF_SEGYSEGY    160     /* SEGY seismic or subbottom trace data, 
					single beam bathymetry, nav, 
					binary, SEG (SIOSEIS variant) */ 
#define MBF_MGD77DAT    161     /* NGDC MGD77 underway geophysics format, 
					single beam bathymetry, nav, magnetics, gravity, 
					ascii, NOAA NGDC */ 
#define MBF_ASCIIXYZ    162     /* XYZ (lon lat depth) soundings, ascii, generic */ 
#define MBF_ASCIIYXZ    163     /* YXZ (lat lon depth) soundings, ascii, generic */ 
#define MBF_HYDROB93    164     /* NGDC hydrographic soundings, binary */ 
#define MBF_MBARIROV    165     /* MBARI ROV navigation format, ascii, MBARI */ 
#define MBF_MBPRONAV    166     /* MB-System simple navigation format, ascii, MBARI */ 
#define MBF_NVNETCDF    167     /* CARAIBES CDF navigation, netCDF, IFREMER */ 
#define MBF_ASCIIXYT    168     /* XYT (lon lat topography) soundings, ascii, generic */ 
#define MBF_ASCIIYXT    169     /* YXT (lat lon topography) soundings, ascii, generic */ 
#define MBF_MBARROV2    170     /* MBARI ROV navigation format, ascii, MBARI */ 
#define MBF_HS10JAMS    171     /* Furuno HS10 multibeam format, 45 beams, 
					bathymetry and amplitude, ascii, JAMSTEC */ 
#define MBF_HIR2RNAV    172     /* SIO GDC R2R navigation format, ascii, navigation, SIO */ 
#define MBF_SAMESURF    181     /* STN Atlas processing multibeam format, 
					Hydrosweep DS2, Hydrosweep MD, 
					Fansweep 10, Fansweep 20, 
					bathymetry, amplitude, and sidescan,
					up to 1440 beams and 4096 pixels, 
					XDR binary, STN Atlas. */ 
#define MBF_HSDS2RAW    182     /* STN Atlas raw multibeam format, 
					Hydrosweep DS2, Hydrosweep MD, 
					Fansweep 10, Fansweep 20, 
					bathymetry, amplitude, and sidescan,
					up to 1440 beams and 4096 pixels, 
					XDR binary, STN Atlas. */
#define MBF_HSDS2LAM    183     /* L-DEO HSDS2 processing format, 
					STN Atlas multibeam sonars, 
					Hydrosweep DS2, Hydrosweep MD, 
					Fansweep 10, Fansweep 20, 
					bathymetry, amplitude, and sidescan,
					up to 1440 beams and 4096 pixels,
					XDR binary, L-DEO. */
#define MBF_RESON7KR	88	/* Reson 7K multibeam vendor format
					Reson 7K series multibeam sonars, 
					bathymetry, amplitude, three channels sidescan, and subbottom
					up to 254 beams, variable pixels, binary, Reson. */
#define MBF_RESON7KP	89	/* MBARI processing format for Reson 7K multibeam data
					Reson 7K series multibeam sonars, 
					bathymetry, amplitude, three channels sidescan, and subbottom
					up to 254 beams, variable pixels, binary, Reson. */
#define MBF_IMAGE83P	191	/* Imagenex 83p vendor format for DeltaT multibeam
					480 beams bathymetry */
#define MBF_IMAGEMBA	192	/* MBARI processing format for DeltaT multibeam
					480 beams bathymetry */
#define MBF_HYSWEEP1	201	/* HYSWEEP format for multibeam data
					variable beams,  bathymetry, amplitude, and sidescan,
					ascii text, single files, Hypack. */
					

/* format registration function prototypes */
int mbr_register_sbsiomrg(int verbose, void *mbio_ptr, int *error);
int mbr_register_sbsiocen(int verbose, void *mbio_ptr, int *error); 
int mbr_register_sbsiolsi(int verbose, void *mbio_ptr, int *error); 
int mbr_register_sburicen(int verbose, void *mbio_ptr, int *error); 
int mbr_register_sburivax(int verbose, void *mbio_ptr, int *error); 
int mbr_register_sbsioswb(int verbose, void *mbio_ptr, int *error); 
int mbr_register_sbifremr(int verbose, void *mbio_ptr, int *error); 
int mbr_register_hsldedmb(int verbose, void *mbio_ptr, int *error); 
int mbr_register_hsuricen(int verbose, void *mbio_ptr, int *error); 
int mbr_register_hsatlraw(int verbose, void *mbio_ptr, int *error); 
int mbr_register_hsldeoih(int verbose, void *mbio_ptr, int *error); 
int mbr_register_hsurivax(int verbose, void *mbio_ptr, int *error); 
int mbr_register_hsunknwn(int verbose, void *mbio_ptr, int *error); 
int mbr_register_sb2000sb(int verbose, void *mbio_ptr, int *error); 
int mbr_register_sb2000ss(int verbose, void *mbio_ptr, int *error); 
int mbr_register_sb2100rw(int verbose, void *mbio_ptr, int *error); 
int mbr_register_sb2100b1(int verbose, void *mbio_ptr, int *error); 
int mbr_register_sb2100b2(int verbose, void *mbio_ptr, int *error); 
int mbr_register_emoldraw(int verbose, void *mbio_ptr, int *error); 
int mbr_register_em12ifrm(int verbose, void *mbio_ptr, int *error); 
int mbr_register_em12darw(int verbose, void *mbio_ptr, int *error); 
int mbr_register_em300raw(int verbose, void *mbio_ptr, int *error); 
int mbr_register_em300mba(int verbose, void *mbio_ptr, int *error); 
int mbr_register_em710raw(int verbose, void *mbio_ptr, int *error); 
int mbr_register_em710mba(int verbose, void *mbio_ptr, int *error); 
int mbr_register_mr1prhig(int verbose, void *mbio_ptr, int *error); 
int mbr_register_mr1aldeo(int verbose, void *mbio_ptr, int *error); 
int mbr_register_mr1bldeo(int verbose, void *mbio_ptr, int *error); 
int mbr_register_mr1prvr2(int verbose, void *mbio_ptr, int *error); 
int mbr_register_mbldeoih(int verbose, void *mbio_ptr, int *error); 
int mbr_register_mbnetcdf(int verbose, void *mbio_ptr, int *error); 
int mbr_register_mbnetcdf(int verbose, void *mbio_ptr, int *error);
int mbr_register_cbat9001(int verbose, void *mbio_ptr, int *error); 
int mbr_register_cbat8101(int verbose, void *mbio_ptr, int *error); 
int mbr_register_hypc8101(int verbose, void *mbio_ptr, int *error); 
int mbr_register_xtfr8101(int verbose, void *mbio_ptr, int *error); 
int mbr_register_reson7kr(int verbose, void *mbio_ptr, int *error); 
int mbr_register_bchrtunb(int verbose, void *mbio_ptr, int *error); 
int mbr_register_elmk2unb(int verbose, void *mbio_ptr, int *error); 
int mbr_register_bchrxunb(int verbose, void *mbio_ptr, int *error); 
int mbr_register_hsmdaraw(int verbose, void *mbio_ptr, int *error); 
int mbr_register_hsmdldih(int verbose, void *mbio_ptr, int *error); 
int mbr_register_dsl120pf(int verbose, void *mbio_ptr, int *error); 
int mbr_register_dsl120sf(int verbose, void *mbio_ptr, int *error); 
int mbr_register_gsfgenmb(int verbose, void *mbio_ptr, int *error); 
int mbr_register_mstiffss(int verbose, void *mbio_ptr, int *error); 
int mbr_register_edgjstar(int verbose, void *mbio_ptr, int *error); 
int mbr_register_edgjstr2(int verbose, void *mbio_ptr, int *error); 
int mbr_register_oicgeoda(int verbose, void *mbio_ptr, int *error); 
int mbr_register_oicmbari(int verbose, void *mbio_ptr, int *error); 
int mbr_register_omghdcsj(int verbose, void *mbio_ptr, int *error); 
int mbr_register_segysegy(int verbose, void *mbio_ptr, int *error); 
int mbr_register_mgd77dat(int verbose, void *mbio_ptr, int *error); 
int mbr_register_asciixyz(int verbose, void *mbio_ptr, int *error); 
int mbr_register_asciiyxz(int verbose, void *mbio_ptr, int *error); 
int mbr_register_hydrob93(int verbose, void *mbio_ptr, int *error); 
int mbr_register_hydrob93(int verbose, void *mbio_ptr, int *error); 
int mbr_register_mbarirov(int verbose, void *mbio_ptr, int *error); 
int mbr_register_mbarrov2(int verbose, void *mbio_ptr, int *error); 
int mbr_register_mbpronav(int verbose, void *mbio_ptr, int *error); 
int mbr_register_nvnetcdf(int verbose, void *mbio_ptr, int *error); 
int mbr_register_asciixyt(int verbose, void *mbio_ptr, int *error); 
int mbr_register_asciiyxt(int verbose, void *mbio_ptr, int *error); 
int mbr_register_l3xseraw(int verbose, void *mbio_ptr, int *error); 
int mbr_register_hs10jams(int verbose, void *mbio_ptr, int *error); 
int mbr_register_hsds2raw(int verbose, void *mbio_ptr, int *error); 
int mbr_register_hsds2lam(int verbose, void *mbio_ptr, int *error); 
int mbr_register_samesurf(int verbose, void *mbio_ptr, int *error); 
int mbr_register_image83p(int verbose, void *mbio_ptr, int *error);
int mbr_register_imagemba(int verbose, void *mbio_ptr, int *error);
int mbr_register_hir2rnav(int verbose, void *mbio_ptr, int *error);
int mbr_register_hysweep1(int verbose, void *mbio_ptr, int *error);
int mbr_info_sbsiomrg(int verbose, 
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
int mbr_info_sbsiocen(int verbose, 
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
int mbr_info_sbsiolsi(int verbose, 
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
int mbr_info_sburicen(int verbose, 
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
int mbr_info_sburivax(int verbose, 
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
int mbr_info_sbsioswb(int verbose, 
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
int mbr_info_sbifremr(int verbose, 
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
int mbr_info_hsldedmb(int verbose, 
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
int mbr_info_hsuricen(int verbose, 
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
int mbr_info_hsatlraw(int verbose, 
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
int mbr_info_hsldeoih(int verbose, 
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
int mbr_info_hsurivax(int verbose, 
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
int mbr_info_hsunknwn(int verbose, 
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
int mbr_info_sb2000sb(int verbose, 
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
int mbr_info_sb2000ss(int verbose, 
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
int mbr_info_sb2100rw(int verbose, 
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
int mbr_info_sb2100b1(int verbose, 
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
int mbr_info_sb2100b2(int verbose, 
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
int mbr_info_emoldraw(int verbose, 
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
int mbr_info_em12ifrm(int verbose, 
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
int mbr_info_em12darw(int verbose, 
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
int mbr_info_em300raw(int verbose, 
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
int mbr_info_em300mba(int verbose, 
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
int mbr_info_em710raw(int verbose, 
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
int mbr_info_em710mba(int verbose, 
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
int mbr_info_mr1prhig(int verbose, 
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
int mbr_info_mr1aldeo(int verbose, 
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
int mbr_info_mr1bldeo(int verbose, 
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
int mbr_info_mr1prvr2(int verbose, 
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
int mbr_info_mbldeoih(int verbose, 
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
int mbr_info_mbnetcdf(int verbose, 
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
int mbr_info_mbnetcdf(int verbose, 
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
int mbr_info_mbncdfxt(int verbose, 
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
int mbr_info_cbat9001(int verbose, 
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
int mbr_info_cbat8101(int verbose, 
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
int mbr_info_hypc8101(int verbose, 
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
int mbr_info_xtfr8101(int verbose, 
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
int mbr_info_reson7kr(int verbose, 
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
int mbr_info_bchrtunb(int verbose, 
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
int mbr_info_elmk2unb(int verbose, 
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
int mbr_info_bchrxunb(int verbose, 
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
int mbr_info_hsmdaraw(int verbose, 
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
int mbr_info_hsmdldih(int verbose, 
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
int mbr_info_dsl120pf(int verbose, 
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
int mbr_info_dsl120sf(int verbose, 
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
int mbr_info_gsfgenmb(int verbose, 
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
int mbr_info_mstiffss(int verbose, 
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
int mbr_info_edgjstar(int verbose, 
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
int mbr_info_edgjstr2(int verbose, 
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
int mbr_info_oicgeoda(int verbose, 
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
int mbr_info_oicmbari(int verbose, 
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
int mbr_info_omghdcsj(int verbose, 
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
int mbr_info_segysegy(int verbose, 
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
int mbr_info_mgd77dat(int verbose, 
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
int mbr_info_asciixyz(int verbose, 
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
int mbr_info_asciiyxz(int verbose, 
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
			int *svp_source,
			double *beamwidth_xtrack, 
			double *beamwidth_ltrack, 
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
			int *svp_source,
			double *beamwidth_xtrack, 
			double *beamwidth_ltrack, 
			int *error); 
int mbr_info_mbarirov(int verbose, 
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
int mbr_info_mbarrov2(int verbose, 
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
int mbr_info_mbpronav(int verbose, 
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
int mbr_info_nvnetcdf(int verbose, 
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
int mbr_info_asciixyt(int verbose, 
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
int mbr_info_asciiyxt(int verbose, 
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
int mbr_info_l3xseraw(int verbose, 
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
int mbr_info_hs10jams(int verbose, 
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
int mbr_info_hsds2raw(int verbose, 
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
int mbr_info_hsds2lam(int verbose, 
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
int mbr_info_image83p(int verbose, 
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
int mbr_info_imagemba(int verbose, 
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
int mbr_info_hir2rnav(int verbose, 
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
int mbr_info_hysweep1(int verbose, 
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

/* end conditional include */
#endif
