/*--------------------------------------------------------------------
 *    The MB-system:	mb_format.h	1/19/93
 *    $Id: mb_format.h,v 5.6 2001-07-20 17:00:20 caress Exp $
 *
 *    Copyright (c) 1993, 1994, 2001 by
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
 * $Log: not supported by cvs2svn $
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
static char mb_format_updatedate[] = "$Date: 2001-07-20 17:00:20 $ $Revision: 5.6 $";

/* Supported swath sonar systems */
#define	MB_SYS_NONE		0
#define	MB_SYS_SB		1
#define	MB_SYS_HSDS		2
#define	MB_SYS_SB2000		3
#define	MB_SYS_SB2100		4
#define	MB_SYS_SIMRAD		5
#define	MB_SYS_SIMRAD2		6
#define	MB_SYS_MR1		7
#define	MB_SYS_MR1B		8
#define	MB_SYS_LDEOIH		9
#define	MB_SYS_RESON		10
#define	MB_SYS_ELAC		11
#define	MB_SYS_ELACMK2		12
#define MB_SYS_HSMD		13
#define MB_SYS_DSL		14
#define MB_SYS_GSF		15
#define MB_SYS_MSTIFF		16
#define MB_SYS_OIC		17
#define MB_SYS_HDCS		18
#define MB_SYS_SINGLEBEAM	19
#define MB_SYS_XSE		20
#define MB_SYS_HS10		21
#define	MB_SYS_SURF		22

/* Number of supported MBIO data formats */
#define	MB_FORMATS	50

/* Data formats supported by MBIO */
#define MBF_DATALIST	-1
#define MBF_NONE		0
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
#define	MBF_HSSIOSWB	26	/* Hydrosweep DS, 59 beam, bathymetry, 
 					bathymetry and amplitude, 
					swath-bathy, SIO. */
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
					binary, IFREMER */ 
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
#define	MBF_MR1PRHIG	61	/* MR1 post processed format, 
					variable beam bathymetry, variable
					pixel sidescan, xdr binary, SOEST */ 
#define	MBF_MR1ALDEO	62	/* MR1 Lamont format with travel times, 
					variable beam bathymetry, variable
					pixel sidescan, xdr binary, L-DEO */ 
#define	MBF_MR1BLDEO	63	/* MR1 small Lamont format with travel times, 
					variable beam bathymetry, variable
					pixel sidescan, xdr binary, L-DEO */ 
#define	MBF_MBLDEOIH	71	/* Generic in-house multibeam, variable beam, 
 					bathymetry, amplitude, and sidescan
 					binary, centered, L-DEO. */
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
#define MBF_MGD77DAT    161     /* NGDC MGD77 underway geophysics format, 
					single beam bathymetry, nav, magnetics, gravity, 
					ascii, NOAA NGDC */ 
#define MBF_MBARIROV    165     /* MBARI ROV navigation format, ascii, MBARI */ 
#define MBF_MBPRONAV    166     /* MB-System simple navigation format, ascii, MBARI */ 
#define MBF_HS10JAMS    171     /* Furuno HS10 multibeam format, 45 beams, 
					bathymetry and amplitude, ascii, JAMSTEC */ 
#define MBF_ATLSSURF    181     /* STN Atlas processing multibeam format, 
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

/* Alias table for old (pre-version 4.0) format id's */
static int format_alias_table[] = 
	{
	0,	/* NULL */
	11,	/* MBF_SBSIOMRG */
	12,	/* MBF_SBSIOCEN */
	13,	/* MBF_SBSIOLSI */
	14,	/* MBF_SBURICEN */
	21,	/* MBF_HSATLRAW */
	22,	/* MBF_HSLDEDMB */
	23,	/* MBF_HSURICEN */
	24,	/* MBF_HSLDEOIH */
	71,	/* MBF_MBLDEOIH */
	};

/* end conditional include */
#endif
