/*--------------------------------------------------------------------
 *    The MB-system:	mb_format.h	1/19/93
 *    $Id: mb_format.h,v 4.5 1994-12-21 20:21:09 caress Exp $
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

/* Supported multibeam systems */
#define	MB_SYS_NONE	0
#define	MB_SYS_SB	1
#define	MB_SYS_HSDS	2
#define	MB_SYS_SB2000	3
#define	MB_SYS_SB2100	4
#define	MB_SYS_SIMRAD	5
#define	MB_SYS_MR1	6
#define	MB_SYS_LDEOIH	7
#define	MB_SYS_RESON	8
#define	MB_SYS_ELAC	9

/* Number of supported MBIO data formats */
#define	MB_FORMATS	25

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
#define	MBF_SB2100RW	41	/* SeaBeam 2100/1000 series vender format, 
					151 beam bathymetry and amplitude,
					2000 pixel sidescan, ascii + binary, 
					centered, SeaBeam Instruments */ 
#define	MBF_SB2100IH	42	/* SeaBeam 2100/1000 series processed format, 
					151 beam bathymetry and amplitude,
					2000 pixel sidescan, binary, centered,
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
#define	MBF_MR1PRHIG	61	/* MR1 post processed format, 
					variable beam bathymetry, variable
					pixel sidescan, xdr binary, SOEST */ 
#define	MBF_MBLDEOIH	71	/* Generic in-house multibeam, variable beam, 
 					bathymetry, amplitude, and sidescan
 					binary, centered, L-DEO. */
#define	MBF_CBAT9001	81	/* Reson SeaBat 9001 multibeam, 60 beams
 					bathymetry and amplitude,
 					binary, University of New Brunswick. */
#define	MBF_BCHRTUNB	91	/* Elac BottomChart multibeam, 56 beams
 					bathymetry and amplitude,
 					binary, University of New Brunswick. */

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
	42,	/* MBF_SB2100IH */
	51,	/* MBF_EM1000RW */
	52,	/* MBF_EM12SRAW */
	53,	/* MBF_EM12DRAW */
	54,	/* MBF_EM12DARW */
	61,	/* MBF_MR1PRHIG */
	71,	/* MBF_MBLDEOIH */
	81,	/* MBF_CBAT9001 */
	91,	/* MBF_BCHRTUNB */
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
	0,	/* MBF_SB2100IH */
	1,	/* MBF_EM1000RW */
	0,	/* MBF_EM12SRAW */
	0,	/* MBF_EM12DRAW */
	1,	/* MBF_EM12DARW */
	1,	/* MBF_MR1PRHIG */
	1,	/* MBF_MBLDEOIH */
	1,	/* MBF_CBAT9001 */
	1,	/* MBF_BCHRTUNB */
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
	"Format name:          MBF_HSATLRAW\nInformal Description: Raw Hydrosweep\nAttributes:           Hydrosweep DS, bathymetry and amplitude, 59 beams,\n                      ascii, Atlas Electronik.\n",
	"Format name:          MBF_HSLDEDMB\nInformal Description: EDMB Hydrosweep\nAttributes:           Hydrosweep DS, bathymetry, 59 beams, binary, NRL.\n",
	"Format name:          MBF_HSURICEN\nInformal Description: URI Hydrosweep\nAttributes:           Hydrosweep DS, 59 beams, bathymetry, binary, URI.\n",
	"Format name:          MBF_HSLDEOIH\nInformal Description: L-DEO in-house binary Hydrosweep\nAttributes:           Hydrosweep DS, 59 beams, bathymetry and amplitude, \n                      binary, centered, L-DEO.\n",
	"Format name:          MBF_HSURIVAX\nInformal Description: URI Hydrosweep from VAX\nAttributes:           Hydrosweep DS, 59 beams, bathymetry, binary,\n                      VAX byte order, URI.\n",
	"Format name:          MBF_HSSIOSWB\nInformal Description: SIO Swath-bathy Hydrosweep DS\nAttributes:           Hydrosweep DS, 59 beams, bathymetry, binary,\n                      SIO.\n",
	"Format name:          MBF_SB2000RW\nInformal Description: SeaBeam 2000 vender format\nAttributes:           SeaBeam 2000, bathymetry, amplitude \n                      and sidescan, 121 beams and 2000 pixels, ascii \n                      with binary sidescan, SeaBeam Instruments.\n",
	"Format name:          MBF_SB2000SB\nInformal Description: SIO Swath-bathy SeaBeam 2000 format\nAttributes:           SeaBeam 2000, bathymetry, 121 beams, \n                      binary,  SIO.\n",
	"Format name:          MBF_SB2000SS\nInformal Description: SIO Swath-bathy SeaBeam 2000 format\nAttributes:           SeaBeam 2000, sidescan, 2000 pixels,\n                      binary,  SIO.\n",
	"Format name:          MBF_SB2100RW\nInformal Description: SeaBeam 2100/1000 series vender format\nAttributes:           SeaBeam 2100/1000, bathymetry, amplitude \n                      and sidescan, 151 beams and 2000 pixels, ascii \n                      with binary sidescan, SeaBeam Instruments.\n",
	"Format name:          MBF_SB2100IH\nInformal Description: SeaBeam 2100/1000 series processing format\nAttributes:           SeaBeam 2100/1000, bathymetry, amplitude \n                      and sidescan, 151 beams bathymetry,\n                      2000 pixels sidescan, binary,\n                      L-DEO and SeaBeam Instruments.\n",
	"Format name:          MBF_EM1000RW\nInformal Description: Simrad EM1000 vendor format\nAttributes:           Simrad EM1000, bathymetry, amplitude, and sidescan,\n                      60 beams, variable pixels, ascii + binary, Simrad.\n",
	"Format name:          MBF_EM12SRAW\nInformal Description: Simrad EM12S vendor format\nAttributes:           Simrad EM12S, bathymetry, amplitude, and sidescan,\n                      81 beams, variable pixels, ascii + binary, Simrad.\n",
	"Format name:          MBF_EM12DRAW\nInformal Description: Simrad EM12D vendor format\nAttributes:           Simrad EM12D, bathymetry, amplitude, and sidescan,\n                      162 beams, variable pixels, ascii + binary, Simrad.\n",
	"Format name:          MBF_EM12DARW\nInformal Description: Simrad EM12S RRS Darwin processed format\nAttributes:           Simrad EM12S, bathymetry and amplitude,\n                      81 beams, binary, Oxford University.\n",
	"Format name:          MBF_MR1PRHIG\nInformal Description: SOEST MR1 post processed format\nAttributes:           SOEST MR1, bathymetry and sidescan,\n                      variable beams and pixels, xdr binary, \n                      SOEST, University of Hawaii.\n",
	"Format name:          MBF_MBLDEOIH\nInformal Description: L-DEO in-house generic multibeam\nAttributes:           Data from all sonar systems, bathymetry, \n                      amplitude and sidescan, variable beams and pixels, \n                      binary, centered, L-DEO.\n",
	"Format name:          MBF_CBAT9001\nInformal Description: Reson SeaBat 9001 shallow water multibeam\nAttributes:           60 beam bathymetry and amplitude,\n                      binary, University of New Brunswick.\n",
	"Format name:          MBF_BCHRTUNB\nInformal Description: Elac BottomChart shallow water multibeam\nAttributes:           56 beam bathymetry and amplitude,\n                      binary, University of New Brunswick.\n"
	};

/* Table of which multibeam system each data format 
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
	MB_SYS_SB2100,	/* MBF_SB2100IH */
	MB_SYS_SIMRAD,	/* MBF_EM1000RW */
	MB_SYS_SIMRAD,	/* MBF_EM12SRAW */
	MB_SYS_SIMRAD,	/* MBF_EM12DRAW */
	MB_SYS_SIMRAD,	/* MBF_EM12DARW */
	MB_SYS_MR1,	/* MBF_MR1PRHIG */
	MB_SYS_LDEOIH,	/* MBF_MBLDEOIH */
	MB_SYS_RESON,	/* MBF_CBAT9001 */
	MB_SYS_ELAC,	/* MBF_BCHRTUNB */
	};

/* Table of which multibeam data formats require the XDR
	library for i/o */
static int mb_xdr_table[] = 
	{
	0,		/* NULL */
	0,		/* MBF_SBSIOMRG */
	0,		/* MBF_SBSIOCEN */
	0,		/* MBF_SBSIOLSI */
	0,		/* MBF_SBURICEN */
	0,		/* MBF_SBURIVAX */
	0,		/* MBF_SBSIOSWB */
	0,		/* MBF_HSATLRAW */
	0,		/* MBF_HSLDEDMB */
	0,		/* MBF_HSURICEN */
	0,		/* MBF_HSLDEOIH */
	0,		/* MBF_HSURIVAX */
	0,		/* MBF_HSSIOSWB */
	0,		/* MBF_SB2000RW */
	0,		/* MBF_SB2000SB */
	0,		/* MBF_SB2000SS */
	0,		/* MBF_SB2100RW */
	0,		/* MBF_SB2100IH */
	0,		/* MBF_EM1000RW */
	0,		/* MBF_EM12SRAW */
	0,		/* MBF_EM12DRAW */
	0,		/* MBF_EM12DARW */
	1,		/* MBF_MR1PRHIG */
	0,		/* MBF_MBLDEOIH */
	0,		/* MBF_CBAT9001 */
	0,		/* MBF_BCHRTUNB */
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
	151,	/* MBF_SB2100IH */
	60,	/* MBF_EM1000RW */
	81,	/* MBF_EM12SRAW */
	162,	/* MBF_EM12DRAW */
	81,	/* MBF_EM12DARW */
	203,	/* MBF_MR1PRHIG */
	200,	/* MBF_MBLDEOIH */
	60,	/* MBF_CBAT9001 */
	56,	/* MBF_BCHRTUNB */
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
	151,	/* MBF_SB2100IH */
	60,	/* MBF_EM1000RW */
	81,	/* MBF_EM12SRAW */
	162,	/* MBF_EM12DRAW */
	81,	/* MBF_EM12DARW */
	0,	/* MBF_MR1PRHIG */
	200,	/* MBF_MBLDEOIH */
	60,	/* MBF_CBAT9001 */
	56,	/* MBF_BCHRTUNB */
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
	2000,	/* MBF_SB2100IH */
	3000,	/* MBF_EM1000RW */
	4050,	/* MBF_EM12SRAW */
	8100,	/* MBF_EM12DRAW */
	0,	/* MBF_EM12DARW */
	2000,	/* MBF_MR1PRHIG */
	10000,	/* MBF_MBLDEOIH */
	0,	/* MBF_CBAT9001 */
	0,	/* MBF_BCHRTUNB */
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
	1,	/* MBF_SB2100IH */
	0,	/* MBF_EM1000RW */
	0,	/* MBF_EM12SRAW */
	0,	/* MBF_EM12DRAW */
	0,	/* MBF_EM12DARW */
	0,	/* MBF_MR1PRHIG */
	1,	/* MBF_MBLDEOIH */
	0,	/* MBF_CBAT9001 */
	1,	/* MBF_BCHRTUNB */
	};

/* Table of which multibeam data formats include 
	travel time data */
static int mb_traveltime_table[] = 
	{
	0,	/* NULL */
	0,	/* MBF_SBSIOMRG */
	0,	/* MBF_SBSIOCEN */
	0,	/* MBF_SBSIOLSI */
	0,	/* MBF_SBURICEN */
	0,	/* MBF_SBURIVAX */
	0,	/* MBF_SBSIOSWB */
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
	1,	/* MBF_SB2100IH */
	1,	/* MBF_EM1000RW */
	1,	/* MBF_EM12SRAW */
	1,	/* MBF_EM12DRAW */
	1,	/* MBF_EM12DARW */
	0,	/* MBF_MR1PRHIG */
	0,	/* MBF_MBLDEOIH */
	1,	/* MBF_CBAT9001 */
	1,	/* MBF_BCHRTUNB */
	};

/* Table of which multibeam data formats support 
	flagging of bad bathymetry data using negative values */
static int mb_bath_flag_table[] = 
	{
	1,	/* NULL */
	1,	/* MBF_SBSIOMRG */
	1,	/* MBF_SBSIOCEN */
	1,	/* MBF_SBSIOLSI */
	1,	/* MBF_SBURICEN */
	1,	/* MBF_SBURIVAX */
	1,	/* MBF_SBSIOSWB */
	0,	/* MBF_HSATLRAW */
	1,	/* MBF_HSLDEDMB */
	1,	/* MBF_HSURICEN */
	1,	/* MBF_HSLDEOIH */
	1,	/* MBF_HSURIVAX */
	1,	/* MBF_HSSIOSWB */
	1,	/* MBF_SB2000RW */
	1,	/* MBF_SB2000SB */
	0,	/* MBF_SB2000SS */
	1,	/* MBF_SB2100RW */
	1,	/* MBF_SB2100IH */
	1,	/* MBF_EM1000RW */
	1,	/* MBF_EM12SRAW */
	1,	/* MBF_EM12DRAW */
	1,	/* MBF_EM12DARW */
	1,	/* MBF_MR1PRHIG */
	1,	/* MBF_MBLDEOIH */
	1,	/* MBF_CBAT9001 */
	1,	/* MBF_BCHRTUNB */
	};

/* Table of which multibeam data formats support 
	flagging of bad amplitude data using negative values */
static int mb_amp_flag_table[] = 
	{
	0,	/* NULL */
	0,	/* MBF_SBSIOMRG */
	0,	/* MBF_SBSIOCEN */
	0,	/* MBF_SBSIOLSI */
	0,	/* MBF_SBURICEN */
	0,	/* MBF_SBURIVAX */
	0,	/* MBF_SBSIOSWB */
	0,	/* MBF_HSATLRAW */
	0,	/* MBF_HSLDEDMB */
	0,	/* MBF_HSURICEN */
	1,	/* MBF_HSLDEOIH */
	0,	/* MBF_HSURIVAX */
	0,	/* MBF_HSSIOSWB */
	0,	/* MBF_SB2000RW */
	0,	/* MBF_SB2000SB */
	0,	/* MBF_SB2000SS */
	0,	/* MBF_SB2100RW */
	0,	/* MBF_SB2100IH */
	1,	/* MBF_EM1000RW */
	1,	/* MBF_EM12SRAW */
	1,	/* MBF_EM12DRAW */
	1,	/* MBF_EM12DARW */
	0,	/* MBF_MR1PRHIG */
	1,	/* MBF_MBLDEOIH */
	0,	/* MBF_CBAT9001 */
	0,	/* MBF_BCHRTUNB */
	};

/* Table of which multibeam data formats support 
	flagging of bad sidescan data using negative values */
static int mb_ss_flag_table[] = 
	{
	0,	/* NULL */
	0,	/* MBF_SBSIOMRG */
	0,	/* MBF_SBSIOCEN */
	0,	/* MBF_SBSIOLSI */
	0,	/* MBF_SBURICEN */
	0,	/* MBF_SBURIVAX */
	0,	/* MBF_SBSIOSWB */
	0,	/* MBF_HSATLRAW */
	0,	/* MBF_HSLDEDMB */
	0,	/* MBF_HSURICEN */
	0,	/* MBF_HSLDEOIH */
	0,	/* MBF_HSURIVAX */
	0,	/* MBF_HSSIOSWB */
	0,	/* MBF_SB2000RW */
	0,	/* MBF_SB2000SB */
	0,	/* MBF_SB2000SS */
	0,	/* MBF_SB2100RW */
	0,	/* MBF_SB2100IH */
	1,	/* MBF_EM1000RW */
	1,	/* MBF_EM12SRAW */
	1,	/* MBF_EM12DRAW */
	1,	/* MBF_EM12DARW */
	1,	/* MBF_MR1PRHIG */
	0,	/* MBF_MBLDEOIH */
	0,	/* MBF_CBAT9001 */
	0,	/* MBF_BCHRTUNB */
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
        " SB2100IH ",
        " EM1000RW ",
        " EM12SRAW ",
        " EM12DRAW ",
        " EM12DARW ",
        " MR1PRHIG ",
        " MBLDEOIH ",
        " CBAT9001 ",
        " BCHRTUNB ",
        };
