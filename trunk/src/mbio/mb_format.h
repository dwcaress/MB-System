/*--------------------------------------------------------------------
 *    The MB-system:	mb_format.h	1/19/93
 *    $Id: mb_format.h,v 4.2 1994-04-11 23:22:28 caress Exp $
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
#define	MB_SYS_EM12	5
#define	MB_SYS_LDEOIH	6

/* Number of supported MBIO data formats */
#define	MB_FORMATS	12

/* Data formats supported by MBIO */
#define	MBF_SBSIOMRG	11	/* SeaBeam, 16 beam, bathymetry, 
 					binary, uncentered, SIO. */
#define	MBF_SBSIOCEN	12	/* SeaBeam, 19 beam, bathymetry, 
 					binary, centered, SIO. */
#define	MBF_SBSIOLSI	13	/* SeaBeam, 19 beam, bathymetry, 
 					binary, centered, obsolete, SIO. */
#define	MBF_SBURICEN	14	/* SeaBeam, 19 beam, bathymetry, 
 					binary, centered, URI. */
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
#define	MBF_SB2100RW	41	/* SeaBeam 2100/1000 series vender format, 
					151 beam bathymetry and amplitude,
					2000 pixel sidescan, ascii, centered,
					SeaBeam Instruments */ 
#define	MBF_SB2100IH	42	/* SeaBeam 2100/1000 series processed format, 
					151 beam bathymetry and amplitude,
					2000 pixel sidescan, binary, centered,
					SeaBeam Instruments and L-DEO */ 
#define	MBF_EM12DARW	51	/* Simrad EM12 RRS Darwin processed format, 
					81 beam, bathymetry and amplitude,
					binary, centered, Oxford University */ 
#define	MBF_MBLDEOIH	61	/* Generic in-house multibeam, variable beam, 
 					bathymetry, amplitude, and sidescan
 					binary, centered, L-DEO. */

/* Translation table of the format id's */
static int format_table[] = 
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
	41,	/* MBF_SB2100RW */
	42,	/* MBF_SB2100IH */
	51,	/* MBF_EM12DARW */
	61,	/* MBF_MBLDEOIH */
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
	61,	/* MBF_MBLDEOIH */
	};

/* Format description messages */
static char *format_description[] =
	{
	"There is no multibeam data format defined for this format id.\n",
	"Format name:          MBF_SBSIOMRG\nInformal Description: SIO merge Sea Beam\nAttributes:           Sea Beam, bathymetry, 16 beams, binary, uncentered,\n                      SIO.\n",
	"Format name:          MBF_SBSIOCEN\nInformal Description: SIO centered Sea Beam\nAttributes:           Sea Beam, bathymetry, 19 beams, binary, centered,\n                      SIO.\n",
	"Format name:          MBF_SBSIOLSI\nInformal Description: SIO LSI Sea Beam\nAttributes:           Sea Beam, bathymetry, 19 beams, binary, centered, \n                      obsolete, SIO.\n",
	"Format name:          MBF_SBURICEN\nInformal Description: URI Sea Beam\nAttributes:           Sea Beam, bathymetry, 19 beams, binary, centered,\n                      URI.\n",
	"Format name:          MBF_HSATLRAW\nInformal Description: Raw Hydrosweep\nAttributes:           Hydrosweep DS, bathymetry and amplitude, 59 beams,\n                      ascii, Atlas Electronik.\n",
	"Format name:          MBF_HSLDEDMB\nInformal Description: EDMB Hydrosweep\nAttributes:           Hydrosweep DS, bathymetry, 59 beams, binary, NRL.\n",
	"Format name:          MBF_HSURICEN\nInformal Description: URI Hydrosweep\nAttributes:           Hydrosweep DS, 59 beams, bathymetry, binary, URI.\n",
	"Format name:          MBF_HSLDEOIH\nInformal Description: L-DEO in-house binary Hydrosweep\nAttributes:           Hydrosweep DS, 59 beams, bathymetry and amplitude, \n                      binary, centered, L-DEO.\n",
	"Format name:          MBF_SB2100RW\nInformal Description: SeaBeam 2100/1000 series vender format\nAttributes:           SeaBeam 2100/1000, bathymetry, amplitude \n                      and sidescan, 151 beams and 2000 pixels, ascii \n                      with binary sidescan, SeaBeam Instruments.\n",
	"Format name:          MBF_SB2100IH\nInformal Description: SeaBeam 2100/1000 series processing format\nAttributes:           SeaBeam 2100/1000, bathymetry, amplitude \n                      and sidescan, 151 beams bathymetry,\n                      2000 pixels sidescan, binary,\n                      L-DEO and SeaBeam Instruments.\n",
	"Format name:          MBF_EM12DARW\nInformal Description: Simrad EM12 RRS Darwin processed format\nAttributes:           Simrad EM12, bathymetry and amplitude,\n                      81 beams, binary, Oxford University.\n",
	"Format name:          MBF_MBLDEOIH\nInformal Description: L-DEO in-house generic multibeam\nAttributes:           Data from all sonar systems, bathymetry, \n                      amplitude and sidescan, variable beams and pixels, \n                      binary, centered, L-DEO.\n"
	};

/* Table of the maximum number of bathymetry beams for each format */
static int beams_bath_table[] = 
	{
	0,	/* NULL */
	19,	/* MBF_SBSIOMRG */
	19,	/* MBF_SBSIOCEN */
	19,	/* MBF_SBSIOLSI */
	19,	/* MBF_SBURICEN */
	59,	/* MBF_HSATLRAW */
	59,	/* MBF_HSLDEDMB */
	59,	/* MBF_HSURICEN */
	59,	/* MBF_HSLDEOIH */
	151,	/* MBF_SB2100RW */
	151,	/* MBF_SB2100IH */
	81,	/* MBF_EM12DARW */
	200,	/* MBF_MBLDEOIH */
	};

/* Table of the maximum number of amplitude beams for each format */
static int beams_amp_table[] = 
	{
	0,	/* NULL */
	0,	/* MBF_SBSIOMRG */
	0,	/* MBF_SBSIOCEN */
	0,	/* MBF_SBSIOLSI */
	0,	/* MBF_SBURICEN */
	59,	/* MBF_HSATLRAW */
	0,	/* MBF_HSLDEDMB */
	0,	/* MBF_HSURICEN */
	59,	/* MBF_HSLDEOIH */
	151,	/* MBF_SB2100RW */
	151,	/* MBF_SB2100IH */
	81,	/* MBF_EM12DARW */
	200,	/* MBF_MBLDEOIH */
	};

/* Table of the maximum number of sidescan beams for each format */
static int beams_ss_table[] = 
	{
	0,	/* NULL */
	0,	/* MBF_SBSIOMRG */
	0,	/* MBF_SBSIOCEN */
	0,	/* MBF_SBSIOLSI */
	0,	/* MBF_SBURICEN */
	0,	/* MBF_HSATLRAW */
	0,	/* MBF_HSLDEDMB */
	0,	/* MBF_HSURICEN */
	0,	/* MBF_HSLDEOIH */
	2000,	/* MBF_SB2100RW */
	2000,	/* MBF_SB2100IH */
	0,	/* MBF_EM12DARW */
	10000,	/* MBF_MBLDEOIH */
	};

/* Table of which data formats have variable numbers of beams */
static int variable_beams_table[] = 
	{
	0,	/* NULL */
	0,	/* MBF_SBSIOMRG */
	0,	/* MBF_SBSIOCEN */
	0,	/* MBF_SBSIOLSI */
	0,	/* MBF_SBURICEN */
	0,	/* MBF_HSATLRAW */
	0,	/* MBF_HSLDEDMB */
	0,	/* MBF_HSURICEN */
	0,	/* MBF_HSLDEOIH */
	1,	/* MBF_SB2100RW */
	1,	/* MBF_SB2100IH */
	0,	/* MBF_EM12DARW */
	1,	/* MBF_MBLDEOIH */
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
	MB_SYS_HSDS,	/* MBF_HSATLRAW */
	MB_SYS_HSDS,	/* MBF_HSLDEDMB */
	MB_SYS_HSDS,	/* MBF_HSURICEN */
	MB_SYS_HSDS,	/* MBF_HSLDEOIH */
	MB_SYS_SB2100,	/* MBF_SB2100RW */
	MB_SYS_SB2100,	/* MBF_SB2100IH */
	MB_SYS_EM12,	/* MBF_EM12DARW */
	MB_SYS_LDEOIH,	/* MBF_MBLDEOIH */
	};

/* Table of which multibeam data formats include 
	travel time data */
static int mb_traveltime_table[] = 
	{
	0,		/* NULL */
	0,		/* MBF_SBSIOMRG */
	0,		/* MBF_SBSIOCEN */
	0,		/* MBF_SBSIOLSI */
	0,		/* MBF_SBURICEN */
	1,		/* MBF_HSATLRAW */
	0,		/* MBF_HSLDEDMB */
	0,		/* MBF_HSURICEN */
	1,		/* MBF_HSLDEOIH */
	1,		/* MBF_SB2100RW */
	1,		/* MBF_SB2100IH */
	1,		/* MBF_EM12DARW */
	0,		/* MBF_MBLDEOIH */
	};

/* Table of which multibeam data formats support 
	flagging of bad bathymetry data using negative values */
static int mb_bath_flag_table[] = 
	{
	1,		/* NULL */
	1,		/* MBF_SBSIOMRG */
	1,		/* MBF_SBSIOCEN */
	1,		/* MBF_SBSIOLSI */
	1,		/* MBF_SBURICEN */
	0,		/* MBF_HSATLRAW */
	1,		/* MBF_HSLDEDMB */
	1,		/* MBF_HSURICEN */
	1,		/* MBF_HSLDEOIH */
	1,		/* MBF_SB2100RW */
	1,		/* MBF_SB2100IH */
	1,		/* MBF_EM12DARW */
	1,		/* MBF_MBLDEOIH */
	};

/* Table of which multibeam data formats support 
	flagging of bad amplitude data using negative values */
static int mb_amp_flag_table[] = 
	{
	0,		/* NULL */
	0,		/* MBF_SBSIOMRG */
	0,		/* MBF_SBSIOCEN */
	0,		/* MBF_SBSIOLSI */
	0,		/* MBF_SBURICEN */
	0,		/* MBF_HSATLRAW */
	0,		/* MBF_HSLDEDMB */
	0,		/* MBF_HSURICEN */
	1,		/* MBF_HSLDEOIH */
	0,		/* MBF_SB2100RW */
	0,		/* MBF_SB2100IH */
	1,		/* MBF_EM12DARW */
	1,		/* MBF_MBLDEOIH */
	};

/* Table of which multibeam data formats support 
	flagging of bad sidescan data using negative values */
static int mb_ss_flag_table[] = 
	{
	0,		/* NULL */
	0,		/* MBF_SBSIOMRG */
	0,		/* MBF_SBSIOCEN */
	0,		/* MBF_SBSIOLSI */
	0,		/* MBF_SBURICEN */
	0,		/* MBF_HSATLRAW */
	0,		/* MBF_HSLDEDMB */
	0,		/* MBF_HSURICEN */
	0,		/* MBF_HSLDEOIH */
	0,		/* MBF_SB2100RW */
	0,		/* MBF_SB2100IH */
	0,		/* MBF_EM12DARW */
	0,		/* MBF_MBLDEOIH */
	};

/* names of formats for use in button or label names */
static char *mb_button_name[] =
        {
	" INVALID ",
        " SBSIOMRG ",
        " SBSIOCEN ",
        " SBSIOLSI ",
        " SBURICEN ",
        " HSATLRAW ",
        " HSLDEDMB ",
        " HSURICEN ",
        " HSLDEOIH ",
        " SB2100RW ",
        " SB2100IH ",
        " EM12DARW ",
        " MBLDEOIH "
        };

