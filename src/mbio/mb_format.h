/*--------------------------------------------------------------------
 *    The MB-system:	mb_format.h	3.00	1/19/93
 *    $Id: mb_format.h,v 3.1 1993-06-13 16:01:22 sohara Exp $
 *
 *    Copyright (c) 1993 by 
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
 * Revision 3.0  1993/04/23  15:50:54  dale
 * Initial version
 *
 */

/* Supported multibeam systems */
#define	MB_SYS_NONE	0
#define	MB_SYS_SB	1
#define	MB_SYS_HSDS	2
#define	MB_SYS_SB2000	3
#define	MB_SYS_LDEOIH	4

/* Number of supported MBIO data formats */
#define	MB_FORMATS	9

/* Data formats supported by MBIO */
#define	MBF_SBSIOMRG	1	/* Sea Beam, 16 beam, bathymetry, 
 					binary, uncentered, SIO. */
#define	MBF_SBSIOCEN	2	/* Sea Beam, 19 beam, bathymetry, 
 					binary, centered, SIO. */
#define	MBF_SBSIOLSI	3	/* Sea Beam, 19 beam, bathymetry, 
 					binary, centered, obsolete, SIO. */
#define	MBF_SBURICEN	4	/* Sea Beam, 19 beam, bathymetry, 
 					binary, centered, URI. */
#define	MBF_HSATLRAW	5	/* Hydrosweep DS raw format, 59 beam, 
 					bathymetry and backscatter, 
  					ascii, Atlas Electronik. */
#define	MBF_HSLDEDMB	6	/* Hydrosweep DS, 59 beam, bathymetry, 
 					binary, NRL. */
#define	MBF_HSURICEN	7	/* Hydrosweep DS, 59 beam, bathymetry, 
 					binary, URI. */
#define	MBF_HSLDEOIH	8	/* Hydrosweep in-house format, 59 beam,
 					bathymetry and backscatter, 
 					binary, centered, L-DEO. */
#define	MBF_MBLDEOIH	9	/* Generic in-house multibeam, variable beam, 
 					bathymetry and backscatter, 
 					binary, centered, L-DEO. */

/* Format description messages */
static char *format_description[] =
	{
	"There is no multibeam data format defined for this format id.\n",
	"Format name:          MBF_SBSIOMRG\nInformal Description: SIO merge Sea Beam\nAttributes:           Sea Beam, 16 beam, bathymetry, binary, uncentered, SIO.\n",
	"Format name:          MBF_SBSIOCEN\nInformal Description: SIO centered Sea Beam\nAttributes:           Sea Beam, 19 beam, bathymetry, binary, centered, SIO.\n",
	"Format name:          MBF_SBSIOLSI\nInformal Description: SIO LSI Sea Beam\nAttributes:           Sea Beam, 19 beam, bathymetry, binary, centered, \n                      obsolete, SIO.\n",
	"Format name:          MBF_SBURICEN\nInformal Description: URI Sea Beam\nAttributes:           Sea Beam, 19 beam, bathymetry, binary, centered, URI.\n",
	"Format name:          MBF_HSATLRAW\nInformal Description: Raw Hydrosweep\nAttributes:           Hydrosweep DS, 59 beam, bathymetry and backscatter, \n                      ascii, Atlas Electronik.\n",
	"Format name:          MBF_HSLDEDMB\nInformal Description: EDMB Hydrosweep\nAttributes:           Hydrosweep DS, 59 beam, bathymetry, binary, NRL.\n",
	"Format name:          MBF_HSURICEN\nInformal Description: URI Hydrosweep\nAttributes:           Hydrosweep DS, 59 beam, bathymetry, binary, URI.\n",
	"Format name:          MBF_HSLDEOIH\nInformal Description: L-DEO in-house binary Hydrosweep\nAttributes:           Hydrosweep DS, 59 beam, bathymetry and backscatter, \n                      binary, centered, L-DEO.\n",
	"Format name:          MBF_MBLDEOIH\nInformal Description: L-DEO in-house generic multibeam\nAttributes:           data from all sonar systems, variable beam, \n                      bathymetry and backscatter, \n                      binary, centered, L-DEO.\n"
	};

/* Table of the number of bathymetry beams for each format */
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
	200,	/* MBF_MBLDEOIH */
	};

/* Table of the number of backscatter beams for each format */
static int beams_back_table[] = 
	{
	0,	/* NULL */
	0,	/* MBF_SBSIOMRG */
	0,	/* MBF_SBSIOCEN */
	0,	/* MBF_SBSIOLSI */
	0,	/* MBF_SBURICEN */
	0,	/* MBF_HSATLRAW */
	0,	/* MBF_HSLDEDMB */
	0,	/* MBF_HSURICEN */
	59,	/* MBF_HSLDEOIH */
	200,	/* MBF_MBLDEOIH */
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
	MB_SYS_LDEOIH,	/* MBF_MBLDEOIH */
	};

/* Table of which multibeam data formats support 
	flagging of bad data using negative values */
static int mb_flag_table[] = 
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
	1,		/* MBF_MBLDEOIH */
	};

/* names of formats for use in button or label names */
static char *mb_button_name[] =
        {
        " SBSIOMRG ",
        " SBSIOCEN ",
        " SBSIOLSI ",
        " SBURICEN ",
        " HSATLRAW ",
        " HSLDEDMB ",
        " HSURICEN ",
        " HSLDEOIH ",
        " MBLDEOIH "
        };

