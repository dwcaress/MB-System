/*--------------------------------------------------------------------
 *    The MB-system:	mb_format.h	1/19/93
 *    $Id: mb_format.h,v 4.22 1999-03-31 18:11:35 caress Exp $
 *
 *    Copyright (c) 1993, 1994 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
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

/* Supported swath sonar systems */
#define	MB_SYS_NONE	0
#define	MB_SYS_SB	1
#define	MB_SYS_HSDS	2
#define	MB_SYS_SB2000	3
#define	MB_SYS_SB2100	4
#define	MB_SYS_SIMRAD	5
#define	MB_SYS_SIMRAD2	6
#define	MB_SYS_MR1	7
#define	MB_SYS_MR1B	8
#define	MB_SYS_LDEOIH	9
#define	MB_SYS_RESON	10
#define	MB_SYS_ELAC	11
#define	MB_SYS_ELACMK2	12
#define MB_SYS_HSMD     13
#define MB_SYS_DSL      14
#define MB_SYS_GSF      15
#define MB_SYS_MSTIFF   16
#define MB_SYS_OIC	17
#define MB_SYS_HDCS	18

/* Table of the swath sonar system miniumum frequencies */
static int frequency_table[] = 
	{
	0,	/* MB_SYS_NONE */
	12,	/* MB_SYS_SB */
	15,	/* MB_SYS_HSDS */
	12,	/* MB_SYS_SB2000 */
	12,	/* MB_SYS_SB2100 */
	12,	/* MB_SYS_SIMRAD */
	300,	/* MB_SYS_SIMRAD2 */
	12,	/* MB_SYS_MR1 */
	12,	/* MB_SYS_MR1B */
	12,	/* MB_SYS_LDEOIH */
	200,	/* MB_SYS_RESON */
	50,	/* MB_SYS_ELAC */
	50,	/* MB_SYS_ELACMK2 */
	30,	/* MB_SYS_HSMD */
	120,	/* MB_SYS_DSL */
	12,	/* MB_SYS_GSF */
	100,	/* MB_SYS_MSTIFF */
	120,	/* MB_SYS_OIC */
	12,	/* MB_SYS_HDCS */
	};

/* Number of supported MBIO data formats */
#define	MB_FORMATS	46

/* Data formats supported by MBIO */
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
#define	MBF_EM1000RW	51	/* Simrad EM1000 series vendor format, 
					60 beam bathymetry and 
					amplitude, variable pixel sidescan, 
					ascii + binary, Simrad */ 
#define	MBF_EM12SRAW	52	/* Simrad EM12S series vendor format, 
					81 beam bathymetry and 
					amplitude, variable pixel sidescan, 
					ascii + binary, Simrad */ 
#define	MBF_EM12DRAW	53	/* Simrad EM12D series vendor format, 
					162 beam bathymetry and 
					amplitude, variable pixel sidescan, 
					ascii + binary, Simrad */ 
#define	MBF_EM12DARW	54	/* Simrad EM12 RRS Darwin processed format, 
					81 beam, bathymetry and amplitude,
					binary, centered, Oxford University */ 
#define	MBF_EM121RAW	55	/* Simrad EM121 series vendor format, 
					121 beam bathymetry and 
					amplitude, variable pixel sidescan, 
					ascii + binary, Simrad */ 
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
 
/* Translation table of the format id's */
static int format_table[] = 
	{
	0,	/* NULL */
	11,	/* MBF_SBSIOMRG */
	12,	/* MBF_SBSIOCEN */
	13,	/* MBF_SBSIOLSI */
	14,	/* MBF_SBURICEN */
	15,	/* MBF_SBURIVAX */
	16,	/* MBF_SBSIOSWB */
	17,	/* MBF_SBIFREMR */
	21,	/* MBF_HSATLRAW */
	22,	/* MBF_HSLDEDMB */
	23,	/* MBF_HSURICEN */
	24,	/* MBF_HSLDEOIH */
	25,	/* MBF_HSURIVAX */
	26,	/* MBF_HSSIOSWB */
	31,	/* MBF_SB2000RW */
	32,	/* MBF_SB2000SB */
	33,	/* MBF_SB2000SS */
	41,	/* MBF_SB2100RW */
	42,	/* MBF_SB2100B1 */
	43,	/* MBF_SB2100B2 */
	51,	/* MBF_EM1000RW */
	52,	/* MBF_EM12SRAW */
	53,	/* MBF_EM12DRAW */
	54,	/* MBF_EM12DARW */
	55,	/* MBF_EM121RAW */
	56,	/* MBF_EM300RAW */
	57,	/* MBF_EM300MBA */
	61,	/* MBF_MR1PRHIG */
	62,	/* MBF_MR1ALDEO */
	63,	/* MBF_MR1BLDEO */
	71,	/* MBF_MBLDEOIH */
	81,	/* MBF_CBAT9001 */
	82,	/* MBF_CBAT8101 */
	83,	/* MBF_HYPC8101 */
	91,	/* MBF_BCHRTUNB */
	92,	/* MBF_ELMK2UNB */
	93,	/* MBF_BCHRXUNB */
	101,    /* MBF_HSMDARAW */
	102,    /* MBF_HSMDLDIH */
	111,    /* MBF_DSL120PF */
	112,    /* MBF_DSL120SF */
	121,    /* MBF_GSFGENMB */
	131,    /* MBF_MSTIFFSS */
	141,    /* MBF_OICGEODA */
	142,    /* MBF_OICMBARI */
	151,    /* MBF_OMGHDCSJ */
	};

/* Table of which formats are really supported */
static int supported_format_table[] = 
	{
	0,	/* NULL */
	1,	/* MBF_SBSIOMRG */
	1,	/* MBF_SBSIOCEN */
	1,	/* MBF_SBSIOLSI */
	1,	/* MBF_SBURICEN */
	1,	/* MBF_SBURIVAX */
	1,	/* MBF_SBSIOSWB */
	1,	/* MBF_SBIFREMR */
	1,	/* MBF_HSATLRAW */
	1,	/* MBF_HSLDEDMB */
	1,	/* MBF_HSURICEN */
	1,	/* MBF_HSLDEOIH */
	1,	/* MBF_HSURIVAX */
	0,	/* MBF_HSSIOSWB */
	0,	/* MBF_SB2000RW */
	1,	/* MBF_SB2000SB */
	1,	/* MBF_SB2000SS */
	1,	/* MBF_SB2100RW */
	1,	/* MBF_SB2100B1 */
	1,	/* MBF_SB2100B2 */
	1,	/* MBF_EM1000RW */
	1,	/* MBF_EM12SRAW */
	0,	/* MBF_EM12DRAW */
	1,	/* MBF_EM12DARW */
	1,	/* MBF_EM121RAW */
	1,	/* MBF_EM300RAW */
	1,	/* MBF_EM300MBA */
	1,	/* MBF_MR1PRHIG */
	1,	/* MBF_MR1ALDEO */
	1,	/* MBF_MR1BLDEO */
	1,	/* MBF_MBLDEOIH */
	1,	/* MBF_CBAT9001 */
	1,	/* MBF_CBAT8101 */
	1,	/* MBF_HYPC8101 */
	1,	/* MBF_BCHRTUNB */
	1,	/* MBF_ELMK2UNB */
	1,	/* MBF_BCHRXUNB */
	1,	/* MBF_HSMDARAW */
	1,	/* MBF_HSMDLDIH */
	1,      /* MBF_DSL120PF */
	1,      /* MBF_DSL120SF */
	1,      /* MBF_GSFGENMB */
	1,      /* MBF_MSTIFFSS */
	1,      /* MBF_OICGEODA */
	1,      /* MBF_OICMBARI */
	1,      /* MBF_OMGHDCSJ */
	};

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

/* Format description messages */
static char *format_description[] =
	{
	"There is no multibeam data format defined for this format id.\n",
	"Format name:          MBF_SBSIOMRG\nInformal Description: SIO merge Sea Beam\nAttributes:           Sea Beam, bathymetry, 16 beams, binary, uncentered,\n                      SIO.\n",
	"Format name:          MBF_SBSIOCEN\nInformal Description: SIO centered Sea Beam\nAttributes:           Sea Beam, bathymetry, 19 beams, binary, centered,\n                      SIO.\n",
	"Format name:          MBF_SBSIOLSI\nInformal Description: SIO LSI Sea Beam\nAttributes:           Sea Beam, bathymetry, 19 beams, binary, centered, \n                      obsolete, SIO.\n",
	"Format name:          MBF_SBURICEN\nInformal Description: URI Sea Beam\nAttributes:           Sea Beam, bathymetry, 19 beams, binary, centered,\n                      URI.\n",
	"Format name:          MBF_SBURIVAX\nInformal Description: URI Sea Beam from VAX\nAttributes:           Sea Beam, bathymetry, 19 beams, binary, centered,\n                      VAX byte order, URI.\n",
	"Format name:          MBF_SBSIOSWB\nInformal Description: SIO Swath-bathy SeaBeam\nAttributes:           Sea Beam, bathymetry, 19 beams, binary, centered,\n                      SIO.\n",
	"Format name:          MBF_SBIFREMR\nInformal Description: IFREMER Archive SeaBeam\nAttributes:           Sea Beam, bathymetry, 19 beams, ascii, centered,\n                      IFREMER.\n",
	"Format name:          MBF_HSATLRAW\nInformal Description: Raw Hydrosweep\nAttributes:           Hydrosweep DS, bathymetry and amplitude, 59 beams,\n                      ascii, Atlas Electronik.\n",
	"Format name:          MBF_HSLDEDMB\nInformal Description: EDMB Hydrosweep\nAttributes:           Hydrosweep DS, bathymetry, 59 beams, binary, NRL.\n",
	"Format name:          MBF_HSURICEN\nInformal Description: URI Hydrosweep\nAttributes:           Hydrosweep DS, 59 beams, bathymetry, binary, URI.\n",
	"Format name:          MBF_HSLDEOIH\nInformal Description: L-DEO in-house binary Hydrosweep\nAttributes:           Hydrosweep DS, 59 beams, bathymetry and amplitude, \n                      binary, centered, L-DEO.\n",
	"Format name:          MBF_HSURIVAX\nInformal Description: URI Hydrosweep from VAX\nAttributes:           Hydrosweep DS, 59 beams, bathymetry, binary,\n                      VAX byte order, URI.\n",
	"Format name:          MBF_HSSIOSWB\nInformal Description: SIO Swath-bathy Hydrosweep DS\nAttributes:           Hydrosweep DS, 59 beams, bathymetry, binary,\n                      SIO.\n",
	"Format name:          MBF_SB2000RW\nInformal Description: SeaBeam 2000 vender format\nAttributes:           SeaBeam 2000, bathymetry, amplitude \n                      and sidescan, 121 beams and 2000 pixels, ascii \n                      with binary sidescan, SeaBeam Instruments.\n",
	"Format name:          MBF_SB2000SB\nInformal Description: SIO Swath-bathy SeaBeam 2000 format\nAttributes:           SeaBeam 2000, bathymetry, 121 beams, \n                      binary,  SIO.\n",
	"Format name:          MBF_SB2000SS\nInformal Description: SIO Swath-bathy SeaBeam 2000 format\nAttributes:           SeaBeam 2000, sidescan,\n                      1000 pixels for 4-bit sidescan,\n                      2000 pixels for 12+-bit sidescan,\n                      binary,  SIO.\n",
	"Format name:          MBF_SB2100RW\nInformal Description: SeaBeam 2100 series vender format\nAttributes:           SeaBeam 2100, bathymetry, amplitude \n                      and sidescan, 151 beams and 2000 pixels, ascii \n                      with binary sidescan, SeaBeam Instruments.\n",
	"Format name:          MBF_SB2100B1\nInformal Description: SeaBeam 2100 series vender format\nAttributes:           SeaBeam 2100, bathymetry, amplitude \n                      and sidescan, 151 beams bathymetry,\n                      2000 pixels sidescan, binary,\n                      SeaBeam Instruments and L-DEO.\n",
	"Format name:          MBF_SB2100B2\nInformal Description: SeaBeam 2100 series vender format\nAttributes:           SeaBeam 2100, bathymetry and amplitude,  \n                      151 beams bathymetry,\n                      binary,\n                      SeaBeam Instruments and L-DEO.\n",
	"Format name:          MBF_EM1000RW\nInformal Description: Simrad EM1000 vendor format\nAttributes:           Simrad EM1000, bathymetry, amplitude, and sidescan,\n                      60 beams, variable pixels, ascii + binary, Simrad.\n",
	"Format name:          MBF_EM12SRAW\nInformal Description: Simrad EM12S vendor format\nAttributes:           Simrad EM12S, bathymetry, amplitude, and sidescan,\n                      81 beams, variable pixels, ascii + binary, Simrad.\n",
	"Format name:          MBF_EM12DRAW\nInformal Description: Simrad EM12D vendor format\nAttributes:           Simrad EM12D, bathymetry, amplitude, and sidescan,\n                      162 beams, variable pixels, ascii + binary, Simrad.\n",
	"Format name:          MBF_EM12DARW\nInformal Description: Simrad EM12S RRS Darwin processed format\nAttributes:           Simrad EM12S, bathymetry and amplitude,\n                      81 beams, binary, Oxford University.\n",
	"Format name:          MBF_EM121RAW\nInformal Description: Simrad EM121 vendor format\nAttributes:           Simrad EM121, bathymetry, amplitude, and sidescan,\n                      121 beams, variable pixels, ascii + binary, Simrad.\n",
	"Format name:          MBF_EM300RAW\nInformal Description: Simrad EM300/EM3000 multibeam vendor format\nAttributes:           Simrad EM300/EM3000, bathymetry, amplitude, and sidescan,\n                      up to 254 beams, variable pixels, ascii + binary, Simrad.\n",
	"Format name:          MBF_EM300MBA\nInformal Description: Simrad EM300/EM3000 multibeam processing format\nAttributes:           Simrad EM300/EM3000, bathymetry, amplitude, and sidescan,\n                      up to 254 beams, variable pixels, ascii + binary, MBARI.\n",
	"Format name:          MBF_MR1PRHIG\nInformal Description: SOEST MR1 post processed format\nAttributes:           SOEST MR1, bathymetry and sidescan,\n                      variable beams and pixels, xdr binary, \n                      SOEST, University of Hawaii.\n",
	"Format name:          MBF_MR1ALDEO\nInformal Description: L-DEO MR1 post processed format with travel times\nAttributes:           L-DEO MR1, bathymetry and sidescan,\n                      variable beams and pixels, xdr binary, \n                      L-DEO.\n",
	"Format name:          MBF_MR1BLDEO\nInformal Description: L-DEO small MR1 post processed format with travel times\nAttributes:           L-DEO MR1, bathymetry and sidescan,\n                      variable beams and pixels, xdr binary, \n                      L-DEO.\n",
	"Format name:          MBF_MBLDEOIH\nInformal Description: L-DEO in-house generic multibeam\nAttributes:           Data from all sonar systems, bathymetry, \n                      amplitude and sidescan, variable beams and pixels, \n                      binary, centered, L-DEO.\n",
	"Format name:          MBF_CBAT9001\nInformal Description: Reson SeaBat 9001 shallow water multibeam\nAttributes:           60 beam bathymetry and amplitude,\n                      binary, University of New Brunswick.\n",
	"Format name:          MBF_CBAT8101\nInformal Description: Reson SeaBat 8101 shallow water multibeam\nAttributes:           101 beam bathymetry and amplitude,\n                      binary, SeaBeam Instruments.\n",
	"Format name:          MBF_HYPC8101\nInformal Description: Reson SeaBat 8101 shallow water multibeam\nAttributes:           101 beam bathymetry,\n                      ASCII, read-only, Coastal Oceanographics.\n",
	"Format name:          MBF_BCHRTUNB\nInformal Description: Elac BottomChart shallow water multibeam\nAttributes:           56 beam bathymetry and amplitude,\n                      binary, University of New Brunswick.\n",
	"Format name:          MBF_ELMK2UNB\nInformal Description: Elac BottomChart shallow water multibeam\nAttributes:           56 beam bathymetry and amplitude,\n                      binary, University of New Brunswick.\n",
	"Format name:          MBF_BCHRXUNB\nInformal Description: Elac BottomChart shallow water multibeam\nAttributes:           56 beam bathymetry and amplitude,\n                      binary, University of New Brunswick.\n",
	"Format name:          MBF_HSMDARAW\nInformal Description: Atlas HSMD medium depth multibeam raw format\nAttributes:           40 beam bathymetry, 160 pixel sidescan,\n                      XDR (binary), STN Atlas Elektronik.\n",
	"Format name:          MBF_HSMDLDIH\nInformal Description: Atlas HSMD medium depth multibeam processed format\nAttributes:           40 beam bathymetry, 160 pixel sidescan,\n                      XDR (binary), L-DEO.\n",
	"Format name:          MBF_DSL120PF\nInformal Description: WHOI DSL AMS-120 processed format\nAttributes:           2048 beam bathymetry, 8192 pixel sidescan,\n                      binary, parallel bathymetry and amplitude files, WHOI DSL.\n",
	"Format name:          MBF_DSL120SF\nInformal Description: WHOI DSL AMS-120 processed format\nAttributes:           2048 beam bathymetry, 8192 pixel sidescan,\n                      binary, single files, WHOI DSL.\n",
	"Format name:          MBF_GSFGENMB\nInformal Description: SAIC Generic Sensor Format (GSF)\nAttributes:           variable beams,  bathymetry and amplitude,\n                      binary, single files, SAIC. \n",
	"Format name:          MBF_MSTIFFSS\nInformal Description: MSTIFF sidescan format\nAttributes:           variable pixels,  sidescan,\n                      binary TIFF variant, single files, Sea Scan. \n",
	"Format name:          MBF_OICGEODA\nInformal Description: OIC swath sonar format\nAttributes:           variable beam bathymetry and\n                      amplitude, variable pixel sidescan, binary,\n		      Oceanic Imaging Consultants\n",
	"Format name:          MBF_OICMBARI\nInformal Description: OIC-style extended swath sonar format\nAttributes:           variable beam bathymetry and\n                      amplitude, variable pixel sidescan, binary,\n		      MBARI\n",
	"Format name:          MBF_OMGHDCSJ\nInformal Description: UNB OMG HDCS format (the John Hughes Clarke format)\nAttributes:           variable beam bathymetry and\n                      amplitude, variable pixel sidescan, binary,\n		      UNB\n",
	};

/* Table of which swath sonar system each data format 
	is associated with */
static int mb_system_table[] = 
	{
	MB_SYS_NONE,	/* NULL */
	MB_SYS_SB,	/* MBF_SBSIOMRG */
	MB_SYS_SB,	/* MBF_SBSIOCEN */
	MB_SYS_SB,	/* MBF_SBSIOLSI */
	MB_SYS_SB,	/* MBF_SBURICEN */
	MB_SYS_SB,	/* MBF_SBURIVAX */
	MB_SYS_SB,	/* MBF_SBSIOSWB */
	MB_SYS_SB,	/* MBF_SBIFREMR */
	MB_SYS_HSDS,	/* MBF_HSATLRAW */
	MB_SYS_HSDS,	/* MBF_HSLDEDMB */
	MB_SYS_HSDS,	/* MBF_HSURICEN */
	MB_SYS_HSDS,	/* MBF_HSLDEOIH */
	MB_SYS_HSDS,	/* MBF_HSURIVAX */
	MB_SYS_HSDS,	/* MBF_HSSIOSWB */
	MB_SYS_SB2000,	/* MBF_SB2000RW */
	MB_SYS_SB2000,	/* MBF_SB2000SB */
	MB_SYS_SB2000,	/* MBF_SB2000SS */
	MB_SYS_SB2100,	/* MBF_SB2100RW */
	MB_SYS_SB2100,	/* MBF_SB2100B1 */
	MB_SYS_SB2100,	/* MBF_SB2100B2 */
	MB_SYS_SIMRAD,	/* MBF_EM1000RW */
	MB_SYS_SIMRAD,	/* MBF_EM12SRAW */
	MB_SYS_SIMRAD,	/* MBF_EM12DRAW */
	MB_SYS_SIMRAD,	/* MBF_EM12DARW */
	MB_SYS_SIMRAD,	/* MBF_EM121RAW */
	MB_SYS_SIMRAD2,	/* MBF_EM300RAW */
	MB_SYS_SIMRAD2,	/* MBF_EM300MBA */
	MB_SYS_MR1,	/* MBF_MR1PRHIG */
	MB_SYS_MR1,	/* MBF_MR1ALDEO */
	MB_SYS_MR1B,	/* MBF_MR1BLDEO */
	MB_SYS_LDEOIH,	/* MBF_MBLDEOIH */
	MB_SYS_RESON,	/* MBF_CBAT9001 */
	MB_SYS_RESON,	/* MBF_CBAT8101 */
	MB_SYS_RESON,	/* MBF_HYPC8101 */
	MB_SYS_ELAC,	/* MBF_BCHRTUNB */
	MB_SYS_ELACMK2,	/* MBF_ELMK2UNB */
	MB_SYS_ELAC,	/* MBF_BCHRXUNB */
	MB_SYS_HSMD,    /* MBF_HSMDARAW */
	MB_SYS_HSMD,    /* MBF_HSMDLDIH */
	MB_SYS_DSL,     /* MBF_DSL120PF */
	MB_SYS_DSL,     /* MBF_DSL120SF */
	MB_SYS_GSF,     /* MBF_GSFGENMB */
	MB_SYS_MSTIFF,  /* MBF_MSTIFFSS */
	MB_SYS_OIC,	/* MBF_OICGEODA */
	MB_SYS_OIC,	/* MBF_OICMBARI */
	MB_SYS_HDCS,	/* MBF_OMGHDCSJ */
	};

/* Table of the number of parallel files required for i/o */
static int mb_numfile_table[] = 
	{
	1,		/* NULL */
	1,		/* MBF_SBSIOMRG */
	1,		/* MBF_SBSIOCEN */
	1,		/* MBF_SBSIOLSI */
	1,		/* MBF_SBURICEN */
	1,		/* MBF_SBURIVAX */
	1,		/* MBF_SBSIOSWB */
	1,		/* MBF_SBIFREMR */
	1,		/* MBF_HSATLRAW */
	1,		/* MBF_HSLDEDMB */
	1,		/* MBF_HSURICEN */
	1,		/* MBF_HSLDEOIH */
	1,		/* MBF_HSURIVAX */
	1,		/* MBF_HSSIOSWB */
	1,		/* MBF_SB2000RW */
	1,		/* MBF_SB2000SB */
	1,		/* MBF_SB2000SS */
	1,		/* MBF_SB2100RW */
	1,		/* MBF_SB2100B1 */
	1,		/* MBF_SB2100B2 */
	1,		/* MBF_EM1000RW */
	1,		/* MBF_EM12SRAW */
	1,		/* MBF_EM12DRAW */
	1,		/* MBF_EM12DARW */
	1,		/* MBF_EM121RAW */
	1,		/* MBF_EM300RAW */
	1,		/* MBF_EM300MBA */
	1,		/* MBF_MR1PRHIG */
	1,		/* MBF_MR1ALDEO */
	1,		/* MBF_MR1BLDEO */
	1,		/* MBF_MBLDEOIH */
	1,		/* MBF_CBAT9001 */
	1,		/* MBF_CBAT8101 */
	1,		/* MBF_HYPC8101 */
	1,		/* MBF_BCHRTUNB */
	1,		/* MBF_ELMK2UNB */
	1,		/* MBF_BCHRXUNB */
	1,              /* MBF_HSMDARAW */
	1,              /* MBF_HSMDLDIH */
	2,		/* MBF_DSL120PF */
	1,		/* MBF_DSL120SF */
	1,		/* MBF_GSFGENMB */
	1,		/* MBF_MSTIFFSS */
	1,		/* MBF_OICGEODA */
	1,		/* MBF_OICMBARI */
	-2,		/* MBF_OMGHDCSJ */
	};

/* Table of what type files are used by swath sonar data formats */
#define	MB_FILETYPE_NORMAL	1
#define	MB_FILETYPE_XDR		2
#define	MB_FILETYPE_GSF		3
static int mb_filetype_table[] = 
	{
	MB_FILETYPE_NORMAL,	/* NULL */
	MB_FILETYPE_NORMAL,	/* MBF_SBSIOMRG */
	MB_FILETYPE_NORMAL,	/* MBF_SBSIOCEN */
	MB_FILETYPE_NORMAL,	/* MBF_SBSIOLSI */
	MB_FILETYPE_NORMAL,	/* MBF_SBURICEN */
	MB_FILETYPE_NORMAL,	/* MBF_SBURIVAX */
	MB_FILETYPE_NORMAL,	/* MBF_SBSIOSWB */
	MB_FILETYPE_NORMAL,	/* MBF_SBIFREMR */
	MB_FILETYPE_NORMAL,	/* MBF_HSATLRAW */
	MB_FILETYPE_NORMAL,	/* MBF_HSLDEDMB */
	MB_FILETYPE_NORMAL,	/* MBF_HSURICEN */
	MB_FILETYPE_NORMAL,	/* MBF_HSLDEOIH */
	MB_FILETYPE_NORMAL,	/* MBF_HSURIVAX */
	MB_FILETYPE_NORMAL,	/* MBF_HSSIOSWB */
	MB_FILETYPE_NORMAL,	/* MBF_SB2000RW */
	MB_FILETYPE_NORMAL,	/* MBF_SB2000SB */
	MB_FILETYPE_NORMAL,	/* MBF_SB2000SS */
	MB_FILETYPE_NORMAL,	/* MBF_SB2100RW */
	MB_FILETYPE_NORMAL,	/* MBF_SB2100B1 */
	MB_FILETYPE_NORMAL,	/* MBF_SB2100B2 */
	MB_FILETYPE_NORMAL,	/* MBF_EM1000RW */
	MB_FILETYPE_NORMAL,	/* MBF_EM12SRAW */
	MB_FILETYPE_NORMAL,	/* MBF_EM12DRAW */
	MB_FILETYPE_NORMAL,	/* MBF_EM12DARW */
	MB_FILETYPE_NORMAL,	/* MBF_EM121RAW */
	MB_FILETYPE_NORMAL,	/* MBF_EM300RAW */
	MB_FILETYPE_NORMAL,	/* MBF_EM300MBA */
	MB_FILETYPE_XDR,	/* MBF_MR1PRHIG */
	MB_FILETYPE_XDR,	/* MBF_MR1ALDEO */
	MB_FILETYPE_XDR,	/* MBF_MR1BLDEO */
	MB_FILETYPE_NORMAL,	/* MBF_MBLDEOIH */
	MB_FILETYPE_NORMAL,	/* MBF_CBAT9001 */
	MB_FILETYPE_NORMAL,	/* MBF_CBAT8101 */
	MB_FILETYPE_NORMAL,	/* MBF_HYPC8101 */
	MB_FILETYPE_NORMAL,	/* MBF_BCHRTUNB */
	MB_FILETYPE_NORMAL,	/* MBF_ELMK2UNB */
	MB_FILETYPE_NORMAL,	/* MBF_BCHRXUNB */
	MB_FILETYPE_XDR,	/* MBF_HSMDARAW */
	MB_FILETYPE_XDR,	/* MBF_HSMDLDIH */
	MB_FILETYPE_NORMAL,	/* MBF_DSL120PF */
	MB_FILETYPE_NORMAL,	/* MBF_DSL120SF */
	MB_FILETYPE_GSF,	/* MBF_GSFGENMB */
	MB_FILETYPE_NORMAL,	/* MBF_MSTIFFSS */
	MB_FILETYPE_NORMAL,	/* MBF_OICGEODA */
	MB_FILETYPE_NORMAL,	/* MBF_OICMBARI */
	MB_FILETYPE_NORMAL,	/* MBF_OMGHDCSJ */
	};

/* Table of the maximum number of bathymetry beams for each format */
static int beams_bath_table[] = 
	{
	0,	/* NULL */
	19,	/* MBF_SBSIOMRG */
	19,	/* MBF_SBSIOCEN */
	19,	/* MBF_SBSIOLSI */
	19,	/* MBF_SBURICEN */
	19,	/* MBF_SBURIVAX */
	19,	/* MBF_SBSIOSWB */
	19,	/* MBF_SBIFREMR */
	59,	/* MBF_HSATLRAW */
	59,	/* MBF_HSLDEDMB */
	59,	/* MBF_HSURICEN */
	59,	/* MBF_HSLDEOIH */
	59,	/* MBF_HSURIVAX */
	59,	/* MBF_HSSIOSWB */
	121,	/* MBF_SB2000RW */
	121,	/* MBF_SB2000SB */
	0,	/* MBF_SB2000SS */
	151,	/* MBF_SB2100RW */
	151,	/* MBF_SB2100B1 */
	151,	/* MBF_SB2100B2 */
	60,	/* MBF_EM1000RW */
	81,	/* MBF_EM12SRAW */
	81,	/* MBF_EM12DRAW */
	81,	/* MBF_EM12DARW */
	121,	/* MBF_EM121RAW */
	254,	/* MBF_EM300RAW */
	254,	/* MBF_EM300MBA */
	3003,	/* MBF_MR1PRHIG */
	3003,	/* MBF_MR1ALDEO */
	153,	/* MBF_MR1BLDEO */
	200,	/* MBF_MBLDEOIH */
	60,	/* MBF_CBAT9001 */
	101,	/* MBF_CBAT8101 */
	101,	/* MBF_HYPC8101 */
	56,	/* MBF_BCHRTUNB */
	126,	/* MBF_ELMK2UNB */
	56,	/* MBF_BCHRXUNB */
	79,     /* MBF_HSMDARAW */
	79,     /* MBF_HSMDLDIH */
	2048,   /* MBF_DSL120PF */
	2048,   /* MBF_DSL120SF */
	151,    /* MBF_GSFGENMB */
	0,      /* MBF_MSTIFFSS */
	1024,	/* MBF_OICGEODA */
	1024,	/* MBF_OICMBARI */
	1440,	/* MBF_OMGHDCSJ */
	};

/* Table of the maximum number of amplitude beams for each format */
static int beams_amp_table[] = 
	{
	0,	/* NULL */
	0,	/* MBF_SBSIOMRG */
	0,	/* MBF_SBSIOCEN */
	0,	/* MBF_SBSIOLSI */
	0,	/* MBF_SBURICEN */
	0,	/* MBF_SBURIVAX */
	0,	/* MBF_SBSIOSWB */
	0,	/* MBF_SBIFREMR */
	59,	/* MBF_HSATLRAW */
	0,	/* MBF_HSLDEDMB */
	0,	/* MBF_HSURICEN */
	59,	/* MBF_HSLDEOIH */
	0,	/* MBF_HSURIVAX */
	0,	/* MBF_HSSIOSWB */
	0,	/* MBF_SB2000RW */
	0,	/* MBF_SB2000SB */
	0,	/* MBF_SB2000SS */
	151,	/* MBF_SB2100RW */
	151,	/* MBF_SB2100B1 */
	151,	/* MBF_SB2100B2 */
	60,	/* MBF_EM1000RW */
	81,	/* MBF_EM12SRAW */
	81,	/* MBF_EM12DRAW */
	81,	/* MBF_EM12DARW */
	121,	/* MBF_EM121RAW */
	254,	/* MBF_EM300RAW */
	254,	/* MBF_EM300MBA */
	0,	/* MBF_MR1PRHIG */
	0,	/* MBF_MR1ALDEO */
	0,	/* MBF_MR1BLDEO */
	200,	/* MBF_MBLDEOIH */
	60,	/* MBF_CBAT9001 */
	101,	/* MBF_CBAT8101 */
	101,	/* MBF_HYPC8101 */
	56,	/* MBF_BCHRTUNB */
	126,	/* MBF_ELMK2UNB */
	56,	/* MBF_BCHRXUNB */
	0,    	/* MBF_HSMDARAW */
	0,    	/* MBF_HSMDLDIH */
	0,      /* MBF_DSL120PF */
	0,      /* MBF_DSL120SF */
	151,    /* MBF_GSFGENMB */
	0,      /* MBF_MSTIFFSS */
	256,	/* MBF_OICGEODA */
	256,	/* MBF_OICMBARI */
	1440,	/* MBF_OMGHDCSJ */
	};

/* Table of the maximum number of sidescan pixels for each format */
static int pixels_ss_table[] = 
	{
	0,	/* NULL */
	0,	/* MBF_SBSIOMRG */
	0,	/* MBF_SBSIOCEN */
	0,	/* MBF_SBSIOLSI */
	0,	/* MBF_SBURICEN */
	0,	/* MBF_SBURIVAX */
	0,	/* MBF_SBSIOSWB */
	0,	/* MBF_SBIFREMR */
	0,	/* MBF_HSATLRAW */
	0,	/* MBF_HSLDEDMB */
	0,	/* MBF_HSURICEN */
	0,	/* MBF_HSLDEOIH */
	0,	/* MBF_HSURIVAX */
	0,	/* MBF_HSSIOSWB */
	2000,	/* MBF_SB2000RW */
	0,	/* MBF_SB2000SB */
	2000,	/* MBF_SB2000SS */
	2000,	/* MBF_SB2100RW */
	2000,	/* MBF_SB2100B1 */
	0,	/* MBF_SB2100B2 */
	6050,	/* MBF_EM1000RW */
	4050,	/* MBF_EM12SRAW */
	4050,	/* MBF_EM12DRAW */
	0,	/* MBF_EM12DARW */
	6050,	/* MBF_EM121RAW */
	1024,	/* MBF_EM300RAW */
	1024,	/* MBF_EM300MBA */
	7003,	/* MBF_MR1PRHIG */
	7003,	/* MBF_MR1ALDEO */
	4003,	/* MBF_MR1BLDEO */
	10000,	/* MBF_MBLDEOIH */
	0,	/* MBF_CBAT9001 */
	0,	/* MBF_CBAT8101 */
	0,	/* MBF_HYPC8101 */
	0,	/* MBF_BCHRTUNB */
	0,	/* MBF_ELMK2UNB */
	0,	/* MBF_BCHRXUNB */
	319,	/* MBF_HSMDARAW */
	319,	/* MBF_HSMDARAW */
	8192,   /* MBF_DSL120PF */
	8192,   /* MBF_DSL120SF */
	0,      /* MBF_GSFGENMB */
	1024,   /* MBF_MSTIFFSS */
	2048,   /* MBF_OICGEODA */
	2048,   /* MBF_OICMBARI */
	1024,   /* MBF_OMGHDCSJ */
	};

/* Table of which data formats have variable numbers of beams */
static int variable_beams_table[] = 
	{
	0,	/* NULL */
	0,	/* MBF_SBSIOMRG */
	0,	/* MBF_SBSIOCEN */
	0,	/* MBF_SBSIOLSI */
	0,	/* MBF_SBURICEN */
	0,	/* MBF_SBURIVAX */
	0,	/* MBF_SBSIOSWB */
	0,	/* MBF_SBIFREMR */
	0,	/* MBF_HSATLRAW */
	0,	/* MBF_HSLDEDMB */
	0,	/* MBF_HSURICEN */
	0,	/* MBF_HSLDEOIH */
	0,	/* MBF_HSURIVAX */
	0,	/* MBF_HSSIOSWB */
	0,	/* MBF_SB2000RW */
	0,	/* MBF_SB2000SB */
	0,	/* MBF_SB2000SS */
	1,	/* MBF_SB2100RW */
	1,	/* MBF_SB2100B1 */
	1,	/* MBF_SB2100B2 */
	0,	/* MBF_EM1000RW */
	0,	/* MBF_EM12SRAW */
	0,	/* MBF_EM12DRAW */
	0,	/* MBF_EM12DARW */
	0,	/* MBF_EM121RAW */
	1,	/* MBF_EM300RAW */
	1,	/* MBF_EM300MBA */
	0,	/* MBF_MR1PRHIG */
	0,	/* MBF_MR1ALDEO */
	0,	/* MBF_MR1BLDEO */
	1,	/* MBF_MBLDEOIH */
	0,	/* MBF_CBAT9001 */
	0,	/* MBF_CBAT8101 */
	0,	/* MBF_HYPC8101 */
	1,	/* MBF_BCHRTUNB */
	1,	/* MBF_ELMK2UNB */
	1,	/* MBF_BCHRXUNB */
	0,      /* MBF_HSMDARAW */
	0,      /* MBF_HSMDLDIH */
	0,      /* MBF_DSL120PF */
	0,      /* MBF_DSL120SF */
	1,      /* MBF_GSFGENMB */
	0,      /* MBF_MSTIFFSS */
	1,      /* MBF_OICGEODA */
	1,      /* MBF_OICMBARI */
	1,      /* MBF_OMGHDCSJ */
	};

/* Table of which swath sonar data formats include 
	travel time and angle data */
static int mb_traveltime_table[] = 
	{
	0,	/* NULL */
	0,	/* MBF_SBSIOMRG */
	0,	/* MBF_SBSIOCEN */
	0,	/* MBF_SBSIOLSI */
	0,	/* MBF_SBURICEN */
	0,	/* MBF_SBURIVAX */
	0,	/* MBF_SBSIOSWB */
	0,	/* MBF_SBIFREMR */
	1,	/* MBF_HSATLRAW */
	0,	/* MBF_HSLDEDMB */
	0,	/* MBF_HSURICEN */
	1,	/* MBF_HSLDEOIH */
	0,	/* MBF_HSURIVAX */
	0,	/* MBF_HSSIOSWB */
	0,	/* MBF_SB2000RW */
	0,	/* MBF_SB2000SB */
	0,	/* MBF_SB2000SS */
	1,	/* MBF_SB2100RW */
	1,	/* MBF_SB2100B1 */
	1,	/* MBF_SB2100B2 */
	0,	/* MBF_EM1000RW */
	1,	/* MBF_EM12SRAW */
	1,	/* MBF_EM12DRAW */
	1,	/* MBF_EM12DARW */
	0,	/* MBF_EM121RAW */
	1,	/* MBF_EM300RAW */
	1,	/* MBF_EM300MBA */
	1,	/* MBF_MR1PRHIG */
	1,	/* MBF_MR1ALDEO */
	1,	/* MBF_MR1BLDEO */
	0,	/* MBF_MBLDEOIH */
	1,	/* MBF_CBAT9001 */
	1,	/* MBF_CBAT8101 */
	1,	/* MBF_HYPC8101 */
	1,	/* MBF_BCHRTUNB */
	1,	/* MBF_ELMK2UNB */
	1,	/* MBF_BCHRXUNB */
	1,	/* MBF_HSMDARAW */
	1,	/* MBF_HSMDLDIH */
	0,      /* MBF_DSL120PF */
	0,      /* MBF_DSL120SF */
	1,      /* MBF_GSFGENMB */
	0,      /* MBF_MSTIFFSS */
	1,      /* MBF_OICGEODA */
	1,      /* MBF_OICMBARI */
	1,      /* MBF_OMGHDCSJ */
	};

/* Table of which swath sonar data formats CANNOT support 
	flagging of bad beam data (bath and amp) values */
static int mb_no_flag_table[] = 
	{
	0,	/* NULL */
	0,	/* MBF_SBSIOMRG */
	0,	/* MBF_SBSIOCEN */
	0,	/* MBF_SBSIOLSI */
	0,	/* MBF_SBURICEN */
	0,	/* MBF_SBURIVAX */
	0,	/* MBF_SBSIOSWB */
	0,	/* MBF_SBIFREMR */
	1,	/* MBF_HSATLRAW */
	0,	/* MBF_HSLDEDMB */
	0,	/* MBF_HSURICEN */
	0,	/* MBF_HSLDEOIH */
	0,	/* MBF_HSURIVAX */
	0,	/* MBF_HSSIOSWB */
	0,	/* MBF_SB2000RW */
	0,	/* MBF_SB2000SB */
	0,	/* MBF_SB2000SS */
	0,	/* MBF_SB2100RW */
	0,	/* MBF_SB2100B1 */
	0,	/* MBF_SB2100B2 */
	0,	/* MBF_EM1000RW */
	0,	/* MBF_EM12SRAW */
	0,	/* MBF_EM12DRAW */
	0,	/* MBF_EM12DARW */
	0,	/* MBF_EM121RAW */
	1,	/* MBF_EM300RAW */
	0,	/* MBF_EM300MBA */
	0,	/* MBF_MR1PRHIG */
	0,	/* MBF_MR1ALDEO */
	0,	/* MBF_MR1BLDEO */
	0,	/* MBF_MBLDEOIH */
	0,	/* MBF_CBAT9001 */
	0,	/* MBF_CBAT8101 */
	0,	/* MBF_HYPC8101 */
	0,	/* MBF_BCHRTUNB */
	0,	/* MBF_ELMK2UNB */
	0,	/* MBF_BCHRXUNB */
	0,	/* MBF_HSMDARAW */
	0,	/* MBF_HSMDLDIH */
	0,      /* MBF_DSL120PF */
	0,      /* MBF_DSL120SF */
	0,      /* MBF_GSFGENMB */
	1,      /* MBF_MSTIFFSS */
	0,      /* MBF_OICGEODA */
	0,      /* MBF_OICMBARI */
	0,      /* MBF_OMGHDCSJ */
	};

/* Table of the data record types containing the primary navigation */
static float mb_nav_source[] = 
	{
	MB_DATA_NONE,	/* NULL */
	MB_DATA_DATA,	/* MBF_SBSIOMRG */
	MB_DATA_DATA,	/* MBF_SBSIOCEN */
	MB_DATA_DATA,	/* MBF_SBSIOLSI */
	MB_DATA_DATA,	/* MBF_SBURICEN */
	MB_DATA_DATA,	/* MBF_SBURIVAX */
	MB_DATA_DATA,	/* MBF_SBSIOSWB */
	MB_DATA_DATA,	/* MBF_SBIFREMR */
	MB_DATA_DATA,	/* MBF_HSATLRAW */
	MB_DATA_DATA,	/* MBF_HSLDEDMB */
	MB_DATA_DATA,	/* MBF_HSURICEN */ 
	MB_DATA_DATA,	/* MBF_HSLDEOIH */
	MB_DATA_DATA,	/* MBF_HSURIVAX */
	MB_DATA_DATA,	/* MBF_HSSIOSWB */
	MB_DATA_DATA,	/* MBF_SB2000RW */
	MB_DATA_DATA,	/* MBF_SB2000SB */
	MB_DATA_DATA,	/* MBF_SB2000SS */
	MB_DATA_DATA,	/* MBF_SB2100RW */
	MB_DATA_DATA,	/* MBF_SB2100B1 */
	MB_DATA_DATA,	/* MBF_SB2100B2 */
	MB_DATA_NAV,	/* MBF_EM1000RW */
	MB_DATA_NAV,	/* MBF_EM12SRAW */
	MB_DATA_NAV,	/* MBF_EM12DRAW */
	MB_DATA_DATA,	/* MBF_EM12DARW */
	MB_DATA_NAV,	/* MBF_EM121RAW */
	MB_DATA_NAV,	/* MBF_EM300RAW */
	MB_DATA_DATA,	/* MBF_EM300MBA */
	MB_DATA_DATA,	/* MBF_MR1PRHIG */
	MB_DATA_DATA,	/* MBF_MR1ALDEO */
	MB_DATA_DATA,	/* MBF_MR1BLDEO */
	MB_DATA_DATA,	/* MBF_MBLDEOIH */
	MB_DATA_DATA,	/* MBF_CBAT9001 */
	MB_DATA_DATA,	/* MBF_CBAT8101 */
	MB_DATA_DATA,	/* MBF_HYPC8101 */
	MB_DATA_DATA,	/* MBF_BCHRTUNB */
	MB_DATA_DATA,	/* MBF_ELMK2UNB */
	MB_DATA_DATA,	/* MBF_BCHRXUNB */
	MB_DATA_DATA,   /* MBF_HSMDARAW */
	MB_DATA_DATA,   /* MBF_HSMDLDIH */
	MB_DATA_DATA,   /* MBF_DSL120PF */
	MB_DATA_DATA,   /* MBF_DSL120SF */
	MB_DATA_DATA,   /* MBF_GSFGENMB */
	MB_DATA_DATA,   /* MBF_MSTIFFSS */
	MB_DATA_DATA,   /* MBF_OICGEODA */
	MB_DATA_DATA,   /* MBF_OICMBARI */
	MB_DATA_DATA,   /* MBF_OMGHDCSJ */
	};

/* Table of the data record types containing the primary heading */
static float mb_heading_source[] = 
	{
	MB_DATA_NONE,	/* NULL */
	MB_DATA_DATA,	/* MBF_SBSIOMRG */
	MB_DATA_DATA,	/* MBF_SBSIOCEN */
	MB_DATA_DATA,	/* MBF_SBSIOLSI */
	MB_DATA_DATA,	/* MBF_SBURICEN */
	MB_DATA_DATA,	/* MBF_SBURIVAX */
	MB_DATA_DATA,	/* MBF_SBSIOSWB */
	MB_DATA_DATA,	/* MBF_SBIFREMR */
	MB_DATA_DATA,	/* MBF_HSATLRAW */
	MB_DATA_DATA,	/* MBF_HSLDEDMB */
	MB_DATA_DATA,	/* MBF_HSURICEN */ 
	MB_DATA_DATA,	/* MBF_HSLDEOIH */
	MB_DATA_DATA,	/* MBF_HSURIVAX */
	MB_DATA_DATA,	/* MBF_HSSIOSWB */
	MB_DATA_DATA,	/* MBF_SB2000RW */
	MB_DATA_DATA,	/* MBF_SB2000SB */
	MB_DATA_DATA,	/* MBF_SB2000SS */
	MB_DATA_DATA,	/* MBF_SB2100RW */
	MB_DATA_DATA,	/* MBF_SB2100B1 */
	MB_DATA_DATA,	/* MBF_SB2100B2 */
	MB_DATA_DATA,	/* MBF_EM1000RW */
	MB_DATA_DATA,	/* MBF_EM12SRAW */
	MB_DATA_DATA,	/* MBF_EM12DRAW */
	MB_DATA_DATA,	/* MBF_EM12DARW */
	MB_DATA_DATA,	/* MBF_EM121RAW */
	MB_DATA_DATA,	/* MBF_EM300RAW */
	MB_DATA_DATA,	/* MBF_EM300MBA */
	MB_DATA_DATA,	/* MBF_MR1PRHIG */
	MB_DATA_DATA,	/* MBF_MR1ALDEO */
	MB_DATA_DATA,	/* MBF_MR1BLDEO */
	MB_DATA_DATA,	/* MBF_MBLDEOIH */
	MB_DATA_DATA,	/* MBF_CBAT9001 */
	MB_DATA_DATA,	/* MBF_CBAT8101 */
	MB_DATA_DATA,	/* MBF_HYPC8101 */
	MB_DATA_DATA,	/* MBF_BCHRTUNB */
	MB_DATA_DATA,	/* MBF_ELMK2UNB */
	MB_DATA_DATA,	/* MBF_BCHRXUNB */
	MB_DATA_DATA,   /* MBF_HSMDARAW */
	MB_DATA_DATA,   /* MBF_HSMDLDIH */
	MB_DATA_DATA,   /* MBF_DSL120PF */
	MB_DATA_DATA,   /* MBF_DSL120SF */
	MB_DATA_DATA,   /* MBF_GSFGENMB */
	MB_DATA_DATA,   /* MBF_MSTIFFSS */
	MB_DATA_DATA,   /* MBF_OICGEODA */
	MB_DATA_DATA,   /* MBF_OICMBARI */
	MB_DATA_DATA,   /* MBF_OMGHDCSJ */
	};

/* Table of the data record types containing the primary vru */
static float mb_vru_source[] = 
	{
	MB_DATA_NONE,	/* NULL */
	MB_DATA_DATA,	/* MBF_SBSIOMRG */
	MB_DATA_DATA,	/* MBF_SBSIOCEN */
	MB_DATA_DATA,	/* MBF_SBSIOLSI */
	MB_DATA_DATA,	/* MBF_SBURICEN */
	MB_DATA_DATA,	/* MBF_SBURIVAX */
	MB_DATA_DATA,	/* MBF_SBSIOSWB */
	MB_DATA_DATA,	/* MBF_SBIFREMR */
	MB_DATA_DATA,	/* MBF_HSATLRAW */
	MB_DATA_DATA,	/* MBF_HSLDEDMB */
	MB_DATA_DATA,	/* MBF_HSURICEN */ 
	MB_DATA_DATA,	/* MBF_HSLDEOIH */
	MB_DATA_DATA,	/* MBF_HSURIVAX */
	MB_DATA_DATA,	/* MBF_HSSIOSWB */
	MB_DATA_DATA,	/* MBF_SB2000RW */
	MB_DATA_DATA,	/* MBF_SB2000SB */
	MB_DATA_DATA,	/* MBF_SB2000SS */
	MB_DATA_DATA,	/* MBF_SB2100RW */
	MB_DATA_DATA,	/* MBF_SB2100B1 */
	MB_DATA_DATA,	/* MBF_SB2100B2 */
	MB_DATA_DATA,	/* MBF_EM1000RW */
	MB_DATA_DATA,	/* MBF_EM12SRAW */
	MB_DATA_DATA,	/* MBF_EM12DRAW */
	MB_DATA_DATA,	/* MBF_EM12DARW */
	MB_DATA_DATA,	/* MBF_EM121RAW */
	MB_DATA_ATTITUDE,	/* MBF_EM300RAW */
	MB_DATA_ATTITUDE,	/* MBF_EM300MBA */
	MB_DATA_DATA,	/* MBF_MR1PRHIG */
	MB_DATA_DATA,	/* MBF_MR1ALDEO */
	MB_DATA_DATA,	/* MBF_MR1BLDEO */
	MB_DATA_DATA,	/* MBF_MBLDEOIH */
	MB_DATA_DATA,	/* MBF_CBAT9001 */
	MB_DATA_DATA,	/* MBF_CBAT8101 */
	MB_DATA_DATA,	/* MBF_HYPC8101 */
	MB_DATA_DATA,	/* MBF_BCHRTUNB */
	MB_DATA_DATA,	/* MBF_ELMK2UNB */
	MB_DATA_DATA,	/* MBF_BCHRXUNB */
	MB_DATA_DATA,   /* MBF_HSMDARAW */
	MB_DATA_DATA,   /* MBF_HSMDLDIH */
	MB_DATA_DATA,   /* MBF_DSL120PF */
	MB_DATA_DATA,   /* MBF_DSL120SF */
	MB_DATA_DATA,   /* MBF_GSFGENMB */
	MB_DATA_DATA,   /* MBF_MSTIFFSS */
	MB_DATA_DATA,   /* MBF_OICGEODA */
	MB_DATA_DATA,   /* MBF_OICMBARI */
	MB_DATA_DATA,   /* MBF_OMGHDCSJ */
	};

/* Table of the fore-aft beamwidths */
static float mb_foreaft_beamwidth_table[] = 
	{
	0.00,	/* NULL */
	2.67,	/* MBF_SBSIOMRG */
	2.67,	/* MBF_SBSIOCEN */
	2.67,	/* MBF_SBSIOLSI */
	2.67,	/* MBF_SBURICEN */
	2.67,	/* MBF_SBURIVAX */
	2.67,	/* MBF_SBSIOSWB */
	2.67,	/* MBF_SBIFREMR */
	2.00,	/* MBF_HSATLRAW */
	2.00,	/* MBF_HSLDEDMB */
	2.00,	/* MBF_HSURICEN */ 
	2.00,	/* MBF_HSLDEOIH */
	2.00,	/* MBF_HSURIVAX */
	2.00,	/* MBF_HSSIOSWB */
	2.00,	/* MBF_SB2000RW */
	2.00,	/* MBF_SB2000SB */
	2.00,	/* MBF_SB2000SS */
	2.00,	/* MBF_SB2100RW */
	2.00,	/* MBF_SB2100B1 */
	2.00,	/* MBF_SB2100B2 */
	2.00,	/* MBF_EM1000RW */
	2.00,	/* MBF_EM12SRAW */
	2.00,	/* MBF_EM12DRAW */
	2.00,	/* MBF_EM12DARW */
	2.00,	/* MBF_EM121RAW */
	2.00,	/* MBF_EM300RAW */
	2.00,	/* MBF_EM300MBA */
	2.00,	/* MBF_MR1PRHIG */
	2.00,	/* MBF_MR1ALDEO */
	2.00,	/* MBF_MR1BLDEO */
	2.00,	/* MBF_MBLDEOIH */
	2.00,	/* MBF_CBAT9001 */
	2.00,	/* MBF_CBAT8101 */
	2.00,	/* MBF_HYPC8101 */
	6.00,	/* MBF_BCHRTUNB */
	3.00,	/* MBF_ELMK2UNB */
	6.00,	/* MBF_BCHRXUNB */
	1.70,   /* MBF_HSMDARAW */
	1.70,   /* MBF_HSMDLDIH */
	2.00,   /* MBF_DSL120PF */
	2.00,   /* MBF_DSL120SF */
	2.00,   /* MBF_GSFGENMB */
	1.00,   /* MBF_MSTIFFSS */
	1.00,   /* MBF_OICGEODA */
	1.00,   /* MBF_OICMBARI */
	1.00,   /* MBF_OMGHDCSJ */
	};

/* names of formats for use in button or label names */
static char *mb_button_name[] =
        {
	" INVALID ",
        " SBSIOMRG ",
        " SBSIOCEN ",
        " SBSIOLSI ",
        " SBURICEN ",
        " SBURIVAX ",
        " SBSIOSWB ",
        " SBIFREMR ",
        " HSATLRAW ",
        " HSLDEDMB ",
        " HSURICEN ",
        " HSLDEOIH ",
        " HSURIVAX ",
        " HSSIOSWB ",
        " SB2000RW ",
        " SB2000SB ",
        " SB2000SS ",
        " SB2100RW ",
        " SB2100B1 ",
        " SB2100B2 ",
        " EM1000RW ",
        " EM12SRAW ",
        " EM12DRAW ",
        " EM12DARW ",
        " EM121RAW ",
        " EM300RAW ",
        " MR1PRHIG ",
        " MR1ALDEO ",
        " MR1BLDEO ",
        " MBLDEOIH ",
        " CBAT9001 ",
        " CBAT8101 ",
        " HYPC8101 ",
        " BCHRTUNB ",
        " ELMK2UNB ",
	" HSMDARAW ",
	" HSMDLDIH ",
	" DSL120PF ", 
	" DSL120SF ", 
	" GSFGENMB ", 
	" MSTIFFSS ", 
	" OICGEODA ", 
	" OICMBARI ", 
	" OMGHDCSJ ", 
        };


/* end conditional include */
#endif
