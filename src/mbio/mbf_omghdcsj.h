/*--------------------------------------------------------------------
 *    The MB-system:	mbf_omghdcsj.h	3/10/99
 *	$Id$
 *
 *    Copyright (c) 1999-2009 by
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
 * mbf_omghdcsj.h defines the data structures used by MBIO functions
 * to store multibeam data read from the  MBF_OMGHDCSJ format (MBIO id 141).  
 *
 * Author:	D. W. Caress
 * Date:	March 10, 1999
 *
 * $Log: mbf_omghdcsj.h,v $
 * Revision 5.3  2008/03/14 18:33:21  caress
 * Updated support for JHC format 151.
 *
 * Revision 5.2  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
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
 * Revision 4.0  1999/03/31  18:29:20  caress
 * MB-System 4.6beta7
 *
 * Revision 1.1  1999/03/31  18:11:35  caress
 * Initial revision
 *

 *
 */
/*
 * Notes on the MBF_OMGHDCSJ data format:
 *   1. The OMG-HDCS format is a collection of similar data
 *      formats used by the seafloor mapping software 
 *      developed by Dr. John Hughes Clarke (Ocean Mapping 
 *      Group of the University of New Brunswick). A variety
 *      of sonars are supported in OMG-HDCS.
 *   2. OMG-HDCS files all begin with a summary header that
 *      specifies the format version, the data source
 *      (type of sonar), the number of records,  and the
 *      minimum and maximum values of position and data
 *      values.
 *   3. The summary header is followed by a set of uniformly
 *      sized data records. The data record size is determined
 *      by the format version and data source. Ecch record is
 *      is divided into a profile (header) and an array of
 *      beam structures.
 *   4. Sidescan imagery can be stored in parallel files in
 *      the same directory as the primary bathymetry and
 *      amplitude files. The sidescan files have the suffix
 *      ".ss_data" added to the end of the primary file's name.
 *   5. Comment records are encoded in MBIO by setting the first
 *      eight bytes of the data record header (profile) to '#' 
 *      values. The profile part of the comment record will have
 *      the same size as expected for data from the tool type
 *      listed in the summary. However, the data section for
 *      comment records will always be 256 bytes long regardless
 *      of the tool type. This mechanism is supported only by 
 *      MB-System.
 *
 */

/* defines sizes and maximums */
#define	MBF_OMGHDCSJ_SUMMARY_SIZE	96
#define	MBF_OMGHDCSJ_SUMMARY_V4EXTRA_SIZE  168
#define	MBF_OMGHDCSJ_MAX_COMMENT	252
#define	MBF_OMGHDCSJ_MAX_BEAMS		1440
#define	MBF_OMGHDCSJ_MAX_PIXELS		1024

/* define tables of data record sizes */
#define MBF_OMGHDCSJ_PROFILE_LENGTH	0
#define MBF_OMGHDCSJ_MAX_NO_BEAMS	1
#define MBF_OMGHDCSJ_BEAM_LENGTH	2
#define MBF_OMGHDCSJ_IMAGE_LENGTH	3

static int mbf_omghdcsj_tooldefs1[MBSYS_HDCS_NUM_TOOLS][4] = 
		    {/*                            PRS     no. DRS  IRS */
                    /*   0:SingleBeam,        */  { 44,     1,   8, 0},
                    /*   1:ELAC BottomChartMk1*/  { 64,    56,  44, 0},
                    /*   2:EM12(dual)         */  { 64,   162,  64, 0},
                    /*   3:EM100 (just depths)*/  { 44,    32,  36, 0},
                    /*   4:FanSweep10 (old)   */  { 44,    52,  36, 0},
                    /*   5:Seabeam "Classic"  */  { 24,    19,  24, 0},
                    /*   6:EM3000S            */  { 64,   128,  64, 0},
                    /*   7:Navitronics        */  { 44,    33,  36, 0},
                    /*   8:EM12(single)       */  { 64,    81,  64, 0},
                    /*   9:EM100+Amplitudes   */  { 64,    32,  44, 0},
                    /*  10:EM1000             */  { 64,    60,  64, 0},
                    /*  11:LADS secondary     */  { 44,    24,  36, 0},
                    /*  12:EM3000D            */  { 64,   256,  64, 0},
                    /*  13:Seabeam 2100       */  { 44,   120,  44, 0},
                    /*  14:ISIS Submetrix     */  { 44,   100,  44, 0},
                    /*  15:EM-1000 (justampl) */  { 44,    60,  44, 0},
                    /*  16:SB2K               */  { 64,   121,  64, 0},
                    /*  17:Seabat9001         */  { 44,    60,  44, 0},
                    /*  18:FanSweep 10 A      */  { 64,    52,  64, 0},
                    /*  19:FanSweep 20        */  { 64,  1440,  64, 0},
                    /*  20:ISIS SWA format    */  { 64,  1280,  64, 0},
                    /*  21:SeaBeam 1180 Mk II */  { 64,    42,  64, 0},
                    /*  22:SeaBat 8101        */  { 64,   101,  64, 0},
                    /*  23:EM300              */  { 88,   137,  76, 0},
                    /*  24:EM121A             */  { 64,   121,  64, 0},
		    /*  25:SM2000             */  { 64,   128,  64, 0},
		    /*  26:HydroSweep MD2     */  {280,   320,  76, 0},
		    /*  23:EM1002             */  { 88,   111,  76, 0},
		    /*  28:HUMMIN'BIRD        */  { 64,     6,  44, 0},
		    /*  29:Knudsen 320        */  {  0,     2,   0, 0},
		    /*  30: EM 120            */  { 88,   191,  76, 0},
		    /*  31:SeaBat 8125        */  { 64,   240,  64, 0},
		    /*  32:SeaBat 8111        */  { 64,   101,  64, 0},
		    /*  33:SeaBat 8150        */  { 64,   234,  64, 0},
		    /*  34:EM3002             */  { 64,   256,  64, 0},
		    /*  35:Optech Laser       */  { 64,   100,  64, 0},
		    /*  36:EM710 400 beam     */  { 64,   400,  64, 0},
		    /*  37:EM3002D            */  { 64,   512,  64, 0},
		    /*  38:SeaBat 8160        */  { 64,   126,  64, 0},
		    /*  39:SEA SwathPlus      */  { 64,   600,  64, 0},
		    /*  40:EM122	      */  { 64,   432,  64, 0},
		    /*  41:EM302 432??	      */  { 64,   432,  64, 0},
		    /*  42:SeaBat 7125	      */  { 64,   512,  64, 0},
		    /*  43:R2Sonic 2024       */  { 64,   256,  64, 0},
		    /*  44:SeaBat 7150	      */  { 64,   880,  64, 0},
		    /*  45:OMG GLORIA	      */  { 1,    1024, 1,  0}

		    };
static int mbf_omghdcsj_tooldefs2[MBSYS_HDCS_NUM_TOOLS][4] = 
		    {/*                            PRS     no. DRS  IRS */
                    /*   0:SingleBeam,        */  { 32,     1,   5, 0},
                    /*   1:ELAC Bottom_Chart  */  { 32,    56,  12, 0},
                    /*   2:EM12(dual)         */  { 32,   162,  00, 0},
                    /*   3:EM100 (just depths)*/  { 32,    32,  00, 0},
                    /*   4:FanSweep           */  { 32,    52,  00, 0},
                    /*   5:Seabeam            */  { 32,    19,  10, 0},
                    /*   6:EM3000S            */  { 32,   128,  20, 0},
                    /*   7:Navitronics        */  { 32,    33,  00, 0},
                    /*   8:EM12(single)       */  { 32,    81,  00, 0},
                    /*   9:EM100+Amplitudes   */  { 32,    32,  12, 0},
                    /*  10:EM1000             */  { 32,    60,  20, 0},
                    /*  11:LADS secondary     */  { 32,    24,  12, 0},
                    /*  12:EM3000D            */  { 32,   256,  20, 0},
                    /*  13:Seabeam 2100       */  { 32,   120,  12, 0},
                    /*  14:ISIS Submetrix     */  { 32,   100,  20, 0},
                    /*  15:EM-1000 (justampl) */  { 32,    60,  00, 0},
                    /*  16:SB2K               */  { 32,   121,  00, 0},
                    /*  17:Seabat9001         */  { 32,    60,  12, 0},
                    /*  18:FanSweep 10 A      */  { 32,    52,  20, 0},
                    /*  19:FanSweep 20        */  { 32,  1440,  12, 0},
                    /*  20:ISIS SWA format    */  { 32,  1280,   8, 0},
                    /*  21:SeaBeam 1180 Mk II */  { 32,   126,  12, 0},
                    /*  22:SeaBat 8101        */  { 32,   101,  12, 0},
                    /*  23:EM300              */  { 32,   137,  20, 0},
                    /*  23:EM121A             */  { 32,   121,  20, 0},
		    /*  25:SM2000             */  { 32,   128,  20, 0},
		    /*  26:HydroSweep MD2     */  { 32,   320,  20, 0},
		    /*  27:EM1002             */  { 32,   111,  20, 0},
		    /*  28:HUMMIN'BIRD        */  { 32,     6,  12, 0},
		    /*  29:Knudsen 320        */  {  0,     2,   0, 0},
		    /*  30: EM 120            */  { 32,   191,  20, 0},
		    /*  31:SeaBat 8125        */  { 32,   240,  12, 0},
		    /*  32:SeaBat 8111        */  { 32,   101,  12, 0},
		    /*  33:SeaBat 8150        */  { 32,   234,  12, 0},
		    /*  34:EM3002             */  { 32,   256,  20, 0},
		    /*  35:Optech Laser       */  { 32,   100,  20, 0},
		    /*  36:EM710 400 beam     */  { 32,   400,  20, 0},
		    /*  37:EM3002D            */  { 32,   512,  20, 0},
		    /*  38:SeaBat 8160        */  { 32,   126,  12, 0},
		    /*  39:SEA SwathPlus      */  { 32,   600,  12, 0},
                    /*  40:EM122 432 beam     */  { 32,   432,  20, 0},
                    /*  41:EM302 432??????    */  { 32,   432,  20, 0},
                    /*  42:SeaBat 7125        */  { 64,   512,  64, 0},
                    /*  43:R2Sonic 2024        */ { 32,   256,  12, 0},
                    /*  44:SeaBat 7150        */  { 32,   880,  12, 0},
                    /*  45:OMG GLORIA        */   {  1,   1024,  1, 0}
		    };
static int mbf_omghdcsj_tooldefs3[MBSYS_HDCS_NUM_TOOLS][4] = 
		    {/*                            PRS     no. DRS  IRS */
                    /*   0:SingleBeam,        */  { 32,     1,   5, 0},
                    /*   1:ELAC Bottom_Chart  */  { 32,    56,  12, 0},
                    /*   2:EM12(dual)         */  { 32,   162,  00, 0},
                    /*   3:EM100 (just depths)*/  { 32,    32,  00, 0},
                    /*   4:FanSweep           */  { 32,    52,  00, 0},
                    /*   5:Seabeam            */  { 32,    19,  10, 0},
                    /*   6:EM3000S            */  { 32,   128,  20, 0},
                    /*   7:Navitronics        */  { 32,    33,  00, 0},
                    /*   8:EM12(single)       */  { 32,    81,  00, 0},
                    /*   9:EM100+Amplitudes   */  { 32,    32,  12, 0},
                    /*  10:EM1000             */  { 32,    60,  20, 0},
                    /*  11:LADS secondary     */  { 32,    24,  12, 0},
                    /*  12:EM3000D            */  { 32,   256,  20, 0},
                    /*  13:Seabeam 2100       */  { 32,   120,  12, 0},
                    /*  14:ISIS Submetrix     */  { 32,   100,  20, 0},
                    /*  15:EM-1000 (justampl) */  { 32,    60,  00, 0},
                    /*  16:SB2K               */  { 32,   121,  00, 0},
                    /*  17:Seabat9001         */  { 32,    60,  12, 0},
                    /*  18:FanSweep 10 A      */  { 32,    52,  20, 0},
                    /*  19:FanSweep 20        */  { 32,  1440,  12, 0},
                    /*  20:ISIS SWA format    */  { 32,  1280,   8, 0},
                    /*  21:SeaBeam 1180 Mk II */  {272,    42,  28, 0},
                    /*  22:SeaBat 8101        */  { 32,   101,  12, 0},
                    /*  23:EM300              */  { 56,   135,  28, 0},
                    /*  23:EM121A             */  { 32,   121,  20, 0},
		    /*  25:SM2000             */  { 56,   128,  28, 0},
		    /*  26:HydroSweep MD2     */  {272,   320,  28, 0},
		    /*  27:EM1002             */  { 56,   111,  28, 0},
		    /*  28:HUMMIN'BIRD        */  { 32,     6,  28, 0},
		    /*  29:Knudsen 320        */  { 36,     2,  28, 0},
		    /*  30: EM 120            */  { 56,   191,  28, 0},
		    /*  31:SeaBat 8125        */  {272,   240,  28, 0},
		    /*  32:SeaBat 8111        */  {272,   101,  28, 0},
		    /*  33:SeaBat 8150        */  {272,   234,  28, 0},
		    /*  34:EM3002             */  { 32,   256,  28, 0},
		    /*  35:Optech Laser       */  { 32,   100,  28, 0},
		    /*  36:EM710 400 beam     */  { 32,   400,  28, 0},
		    /*  37:EM3002D            */  { 32,   512,  28, 0},
		    /*  38:SeaBat 8160        */  {272,   126,  28, 0},
		    /*  39:SEA SwathPlus      */  { 32,   600,  28, 0},
                    /*  40:EM122 432 beam     */  { 32,   432,  28, 0},
                    /*  41:EM302 432??????    */  { 32,   432,  28, 0},
                    /*  42:SeaBat 7125        */  { 272,  512,  28, 0}, /* Using 272 for now until v4 setup for it */
                    /*  43:R2Sonic 2024       */  { 272,  256,  28, 0},
                    /*  44:SeaBat 7150        */  { 272,  880,  28, 0},
                    /*  45:OMG GLORIA         */  {   1, 1024,   1, 0}
		    };

/* define OMG-HDCS summary header structure */
struct mbf_omghdcsj_summary_struct
        {
        int    sensorNumber;	/*  1 = depth file */
        int    subFileID;	/*  1 = data (as opposed to index) */
        int    fileVersion;	/* 1 = original format */
				/* 2 = packed format for EM1000 and others */
				/* 3 = packed format for EM300 */
        int    toolType;	/* Tool Type implies Profile Record Size
					and Depth Record Size
					and Image Record Size */
        int    numProfiles;	/* # of profiles in the file             */
        int    numDepths;	/* # of depths in the file             */
        int    timeScale;	/* time scale (# of uSec. units) */
        int    refTime;		/* Reference time (100 sec. units)       */
        int    minTime;		/* Minimum time (offset wrt. ref.)       */
        int    maxTime;		/* Maximum time (offset wrt. ref.)       */
        int    positionType;	/* Geographic(1)/ UTM(2) */
        int    positionScale;	/* Position scale (# of nRad. units)     */
        int    refLat;		/* Reference latitude (100 nRadians)     */
        int    minLat;		/* Minimum latitude (offset wrt. ref.)   */
        int    maxLat;		/* Maximum latitude (offset wrt. ref.)   */
        int    refLong;		/* Reference longitude (100 nRadians)    */
        int    minLong;		/* Minimum longitude (offset wrt. ref.)  */
        int    maxLong;		/* Maximum longitude (offset wrt. ref.)  */
        int    minObsDepth;	/* Minimum depth (mm)                    */
        int    maxObsDepth;	/* Maximum depth (mm)                    */
        int    minProcDepth;	/* Minimum depth (mm)                    */
        int    maxProcDepth;	/* Maximum depth (mm)                    */
        int    status;		/* status not actually used at all ....  */

	/*** V4 ***/
	int totalProfileBytes;
	int Profile_BitsDefining[20];
	int totalBeamBytes;
	int Beam_BitsDefining[20];

        };


/* specific FOR ATLAS SAPI data */
struct mbf_omghdcs_profile_subparams_struct
{

  unsigned short txBeamIndex;
  unsigned short txLevel;
  short txBeamAngle;
  unsigned short txPulseLength;

  unsigned int ss_offset;
  unsigned short no_skipped_ss;
  unsigned short no_acquired_ss;
  unsigned short ss_sample_interval;

  unsigned short bscatClass;
  unsigned short nrActualGainSets;
  short rxGup;
  short rxGain;
  short ar;
  unsigned short rxtime[20];
  short rxgain[20];

};


/* define profile record structure */
struct mbf_omghdcsj_profile_struct{
        int    status;		/* status is either OK (0) 
					or no nav (1)
					or unwanted for gridding (2) 
					or comment record (999) (MB-System only) */
        int    numDepths;	/* Number of depths in profile        */
        int    numSamples;	/* Number of sidescan samples in parallel file        */
        int    timeOffset;	/* Time offset  wrt. header           */
        int    vesselLatOffset;	/* Latitude offset wrt. header        */
        int    vesselLongOffset;/* Longitude offset wrt. header       */
        int    vesselHeading;	/* Heading (100 nRadians)             */
        int    vesselHeave;	/* Heave (mm)                         */
        int    vesselPitch;	/* Vessel pitch (100 nRadians)        */
        int    vesselRoll;	/* Vessel roll (100 nRadians)         */
        int    tide;		/* Tide (mm)                          */
        int    vesselVelocity;	/* Vessel Velocity (mm/s)
					note - transducer pitch is 
					generally tucked into the vel field     */

	/* the above fields are sufficient for original EM100 data without imagery
		but later data often requires more fields below */
	char    power;
	char    TVG;
	char    attenuation;
	char    edflag;
        int	soundVelocity; 	/* mm/s */
	int	lengthImageDataField;
        int	pingNo;
	char	mode;  
	char	Q_factor;  
	char	pulseLength;   /* centisecs*/
	mb_u_char unassigned;  

	/* the fields below were added to cope with EM300 */
	unsigned short td_sound_speed;
	unsigned short samp_rate;
	mb_u_char z_res_cm;
	mb_u_char xy_res_cm;
	mb_u_char ssp_source;
	mb_u_char filter_ID;
	unsigned short absorp_coeff;
	unsigned short tx_pulse_len;
	unsigned short tx_beam_width;
	unsigned short max_swath_width;
	mb_u_char tx_power_reduction;
	mb_u_char rx_beam_width;
	mb_u_char rx_bandwidth;
	mb_u_char rx_gain_reduction;
	mb_u_char tvg_crossover;
	mb_u_char beam_spacing;
	mb_u_char coverage_sector;
	mb_u_char yaw_stab_mode;

	/* V4 */
	/* extra from HydroSweep MD2 Surf data */
  	struct mbf_omghdcs_profile_subparams_struct params[2];

  	int transducerDepth;		/* transducer or  towfish depth */
  	int transducerPitch;		/* Transducer pitch (100 nRadians)    */
  	int transducerRoll;		/* Transducer roll (100 nRadians)     */
  	/* enough for dyn. stab transducer */
  	int transducerHeading;	/* Transducer pitch (100 nRadians)    */
  	int transducerLatOffset;	/* Latitude offset wrt. vessel        */
  	int transducerLongOffset;	/* Longitude offset wrt. vessel       */
  	int transducerSlantRange;	/*slantRange(mm) wrt. vessel (cable out) */
  	int transducerAcross;		/* horizontal Range (mm) wrt. vessel */
  	int transducerAlong;		/* horizontal Range (mm) wrt. vessel */
  	int transducerBearing;	/* Bearing (100nRads) wrt. vessel       */

  	/* NEW EXTRA Fields that can be added in V4  based on bits in the extended summary header */
  	short longperiod_heaveCorrection;
  	short dynamic_draftCorrection;

  	short deepdraftoffset_in_metres;
  	short draft_at_Tx;

  	short alternateRoll;
  	short alternatePitch;
  	short alternateHeave;
  	short alternateHeading;

  	short standaloneHeading;

  	short RTK_at_RP;  /* in cm units so that can support +/- 320m. */
  	short Lowpass_RTK_at_RP;  /* in cm units so that can support +/- 320m. */
  	short WLZ;
  	unsigned short samp_rate_SecondHead;

  	signed int clock_drift_millis;

  	/* added June 2005 to support EM3002, EM710 water column telegrams */
  	unsigned int watercol_offset;
  	unsigned int watercol_size;

  	/* added June 2008 to support second head of EM3002D water column telegrams */
  	unsigned int watercol_offset_2nd;
  	unsigned int watercol_size_2nd;

  	/* New fields to accomodate un-TVG-ing kongsberg data */
  	unsigned short range_to_normal_incidence;

	/* units of 1,000,000,000 of the Optech laser microsecond(?) time stamp */
  	unsigned int laser_timestampRef;
		
  	/* Added to accomodate EM710 (and EM302/EM122) transmit sector info */
  
  	unsigned int tx_sector_offset;
  	unsigned short num_tx_sectors;

  	/* New fields for Reson 7K systems */
  	unsigned int sonar_settings_offset;
  	unsigned int ping_number;

  	/* These aren't done yet */
  	unsigned short multi_ping_sequence;
  	unsigned int num_beams;	/* Which is different than numDepths... */
					/* (which is usually the number of possible depths and not ACTUAL depths */
					/* e.g. high-density vs. low-density mode in KM systems) */
  	unsigned char layer_compensation_flag;
  	float    bs_beam_position;
  	unsigned int bs_control_flags;
  	unsigned short bs_num_beams_per_side;
  	unsigned short bs_current_beam_number;
  	unsigned char bs_sample_descriptor;
  	unsigned int snippet_sample_descriptor;
	};

/* define beam record structure */
struct mbf_omghdcsj_beam_struct {
        int	status;           /* status is either OK (0) or bad (other) */
        /* V4 Only */
        mb_u_char scaling_factor;
        /* unsigned char nothing[3]; */
        /* Done V4 */
        int	observedDepth;    /* Depth (mm)                            */
        int	acrossTrack;      /* Across track position of depth (mm)   */
        int	alongTrack;       /* Along track position of depth (mm)    */
        int	latOffset;        /* Latitude offset wrt. profile          */
        int	longOffset;       /* Longitude offset wrt. profile         */
        int	processedDepth;    /* Depth (mm)                            */
	int	timeOffset;
        int	depthAccuracy;    /* Depth accuracy (mm)                   */
	mb_u_char reflectivity;  
	char	Q_factor;  /* phase or amplitude detection */ 
	char	beam_no;  
	char	freq;   /* 12.7, 13.0, 13.3, 95.0, Smii, GLORIA */

	char	calibratedBackscatter; /* effects of power/TVG and atten. 
					    removed*/
	char	mindB;		
	char	maxdB;
	mb_u_char	pseudoAngleIndependentBackscatter;		
					/* corrected for mean angular dependence
					    for geological visualisation */
	int	range;   /* other option on EM 12 */
	int	no_samples;
	int	offset;
	int	centre_no;
	char	sample_unit; /* whether in time or distance */
	char	sample_interval; /* seconds or metres */
	char	dummy[2];
	mb_u_char	samp_win_length;
	short	beam_depress_angle;
	unsigned short	beam_heading_angle;

	/* NEW EXTRA Fields that can be added depending on bits in the V4 summary header */
	unsigned short other_range;
	signed short Tx_steer;
	signed short Rc_steer;

	mb_u_char TxSector;
	unsigned int timestampOffset; /* really is a 64 bit integer, trying to compress */
				/* would'nt even need if didn't have to relate wavefile 
			 	by this number */
	unsigned short no_RAMAN; 
	unsigned short no_IR; 
	unsigned short no_GAPD; 
	unsigned short no_PMT;
	mb_u_char prim_depth_conf; 
	mb_u_char seco_depth_conf; 
	signed short scan_azimuth; 	/* 100ths of degree */
	unsigned short nadir_angle; 	/* 100ths of degree */
				/* always dynamically compressed for v4 using scaling factor */
	signed int secondaryDepth;	/* Depth (mm) remember can be +ve or -ve  */
	signed short wave_height;

  /* even NEWER stuff to support custom Pim extra bottom detect solution for Optech laser trace data. */
	signed int opaqueDepth_PMT;		/* Depth (mm)                            */
	signed int extinctionDepth_PMT;	/* Depth (mm)                            */
	signed int pimDepth_PMT;		/* Depth (mm)                            */

	signed int opaqueDepth_GAPD;		/* Depth (mm)                            */
	signed int extinctionDepth_GAPD;	/* Depth (mm)                            */
	signed int pimDepth_GAPD;		/* Depth (mm)                            */

	/*A few extras for Reson 7k systems */
	float twtt;
	unsigned int snippet_first_sample;
	unsigned int snippet_last_sample;
	float intensity;
	};

/* define data holding structure */
struct mbf_omghdcsj_data_struct
	{
	struct mbf_omghdcsj_beam_struct *beams;
	mb_s_char	*ss_raw;
	int	pixel_size;	/* processed sidescan pixel size in mm */
	int	pixels_ss;	/* number of processed sidescan pixels stored */
	short	ss_proc[MBF_OMGHDCSJ_MAX_PIXELS];
				/* the processed sidescan ordered port to starboard */
	int	ssalongtrack[MBF_OMGHDCSJ_MAX_PIXELS];
				/* the processed sidescan alongtrack distances 
					in mm */
	};

/* structure to hold everything */
struct mbf_omghdcsj_struct
	{
	int	kind;
	struct mbf_omghdcsj_summary_struct summary;
	struct mbf_omghdcsj_profile_struct profile;
	char	comment[MBF_OMGHDCSJ_MAX_COMMENT];
	struct mbf_omghdcsj_data_struct	    data;
	char	*buffer;
	};

/* BIT Masks */
/* ZEROTH LONG WORD */
/* ---------------------------------------------------------------------- */
#define BEAM_ui_status					0x00000001
#define BEAM_uc_scaling_factor				0x00000002
#define BEAM_si_observedDepth				0x00000004
#define BEAM_si_acrossTrack				0x00000008
#define BEAM_si_alongTrack				0x00000010
#define BEAM_si_latOffset				0x00000020
#define BEAM_si_longOffset				0x00000040
#define BEAM_si_processedDepth				0x00000080
#define BEAM_si_timeOffset				0x00000100
#define BEAM_si_depthAccuracy				0x00000200

/* ---------------------------------------------------------------------- */
/* ONE'TH LONG WORD */
/* this is stuff you'd only have if you have a single value per beam */
#define BEAM_uc_reflectivity				0x00000001
#define BEAM_sc_Q_factor				0x00000002
#define BEAM_uc_beam_no					0x00000004
#define BEAM_uc_freq					0x00000008
#define BEAM_uc_calibratedBackscatter			0x00000010
#define BEAM_uc_mindB					0x00000020
#define BEAM_uc_maxdB					0x00000040
#define BEAM_uc_pseudoAngleIndependentBackscatter	0x00000080


/* ---------------------------------------------------------------------- */
/* TWO'TH LONG WORD */
/* this is stuff you'd only have if you have snippets */
#define BEAM_ui_range					0x00000001
#define BEAM_ui_no_samples				0x00000002
#define BEAM_ui_offset					0x00000004
#define BEAM_si_centre_no				0x00000008
#define BEAM_uc_sample_unit				0x00000010
#define BEAM_uc_sample_interval				0x00000020
#define BEAM_uc_dummy0					0x00000040
#define BEAM_uc_dummy1					0x00000080
#define BEAM_uc_samp_win_length				0x00000100

/*---------------------------------------------------------------------- */
/* THREE'TH LONG WORD */
/* this is stuff you'd only have if you are serious about recalcuating the beam vector */
#define	BEAM_ss_beam_depress_angle			0x00000001
#define BEAM_us_beam_heading_angle			0x00000002
#define BEAM_us_other_range  				0x00000004
#define BEAM_ss_Tx_steer  				0x00000008
#define BEAM_ss_Rc_steer  				0x00000010
#define BEAM_uc_TxSector				0x00000020

/* ---------------------------------------------------------------------- */
/* FOUR'TH LONG WORD */
/* this is stuff for SHOAL/CHARTS laser bathymetry */
#define	BEAM_ui_timestampOffset			0x00000001
#define	BEAM_us_no_RAMAN			0x00000002
#define	BEAM_us_no_IR				0x00000004
#define	BEAM_us_no_GAPD				0x00000008
#define	BEAM_us_no_PMT				0x00000010
#define	BEAM_uc_prim_depth_conf			0x00000020
#define	BEAM_uc_seco_depth_conf			0x00000040
#define	BEAM_ss_scan_azimuth			0x00000080
#define	BEAM_us_nadir_angle			0x00000100
#define	BEAM_si_secondaryDepth			0x00000200
#define	BEAM_ss_wave_height			0x00000400
#define BEAM_si_opaqueDepth_PMT			0x00000800
#define BEAM_si_extinctionDepth_PMT		0x00001000
#define BEAM_si_pimDepth_PMT			0x00002000
#define BEAM_si_opaqueDepth_GAPD		0x00004000
#define BEAM_si_extinctionDepth_GAPD		0x00008000
#define BEAM_si_pimDepth_GAPD			0x00010000

/* ---------------------------------------------------------------------- */
/* FIFTH LONG WORD */
/* Stuff for Reson 7K systems */
#define	BEAM_f_twtt				0x00000001
#define	BEAM_ui_snippet_first_sample		0x00000002
#define	BEAM_ui_snippet_last_sample		0x00000004
#define	BEAM_f_intensity			0x00000008


/***************************************************************************
 * Profile BitFlags
****************************************************************************/
/* ZEROTH 32 BITS ------------------------------------------------------- */
#define PROF_ui_status  		0x00000001
#define PROF_ui_numDepths		0x00000002	/* Number of depths in profile        */
#define PROF_ui_timeOffset		0x00000004	/* Time offset  wrt. header           */
#define PROF_ui_vesselLatOffset		0x00000008	/* Latitude offset wrt. header        */
#define PROF_ui_vesselLongOffset	0x00000010	/* Longitude offset wrt. header       */
#define PROF_ui_vesselHeading		0x00000020	/* Heading (100 nRadians)             */
#define PROF_si_vesselHeave		0x00000040	/* Heave (mm)                   */
#define PROF_si_vesselPitch		0x00000080	/* Vessel pitch (100 nRadians)        */
#define PROF_si_vesselRoll		0x00000100	/* Vessel roll (100 nRadians)         */
#define PROF_si_tide			0x00000200	/* Tide (mm)                          */
#define PROF_ui_vesselVelocity		0x00000400	/* Vessel Velocity (mm/s)             */

/* FIRST 32 BITS ------------------------------------------------------- */
#define PROF_uc_power			0x00000001
#define PROF_uc_TVG			0x00000002
#define PROF_uc_attenuation	 	0x00000004
#define PROF_uc_edflag			0x00000008
#define PROF_ui_soundVelocity 		0x00000010	/* mm/s */
#define PROF_ui_lengthImageDataField	0x00000020
#define PROF_ui_pingNo 			0x00000040
#define PROF_uc_mode			0x00000080
#define PROF_uc_Q_factor		0x00000100
#define PROF_uc_pulseLength		0x00000200
#define PROF_uc_unassigned		0x00000400
#define PROF_us_td_sound_speed		0x00000800
#define PROF_us_samp_rate		0x00001000
#define PROF_uc_z_res_cm		0x00002000
#define PROF_uc_xy_res_cm		0x00004000

/* SECOND 32 BITS ------------------------------------------------------- */
/* extra from runtime telegram */
#define PROF_uc_ssp_source		0x00000001
#define PROF_uc_filter_ID		0x00000002
#define PROF_us_absorp_coeff		0x00000004
#define PROF_us_tx_pulse_len		0x00000008
#define PROF_us_tx_beam_width		0x00000010
#define PROF_us_max_swath_width		0x00000020
#define PROF_uc_tx_power_reduction	0x00000040
#define PROF_uc_rx_beam_width		0x00000080
#define PROF_uc_rx_bandwidth		0x00000100
#define PROF_uc_rx_gain_reduction	0x00000200
#define PROF_uc_tvg_crossover		0x00000400
#define PROF_uc_beam_spacing		0x00000800
#define PROF_uc_coverage_sector		0x00001000
#define PROF_uc_yaw_stab_mode		0x00002000


/* THIRD 32 BITS ------------------------------------------------------- */
#define PROF_ss_longperiod_heaveCorrection 	0x00000100	/* was 32 not anymore */
#define PROF_ss_dynamic_draftCorrection 	0x00000200
#define PROF_ss_deepdraftoffset_in_metres	0x00000400
#define PROF_ss_draft_at_Tx			0x00000800
#define PROF_ss_alternateRoll 			0x00001000
#define PROF_ss_alternatePitch 			0x00002000
#define PROF_ss_alternateHeave 			0x00004000
#define PROF_us_alternateHeading 		0x00008000
#define PROF_us_standaloneHeading 		0x00010000
#define PROF_ss_RTK_at_RP 			0x00020000
#define PROF_ss_Lowpass_RTK_at_RP 		0x00040000
#define PROF_ss_WLZ 				0x00080000
#define PROF_us_samp_rate_SecondHead 		0x00100000
#define PROF_si_clock_drift_millis 		0x00200000
#define PROF_ui_watercol_offset 		0x00400000
#define PROF_ui_watercol_size	 		0x00800000
#define PROF_us_range_to_normal_incidence	0x01000000
#define PROF_ui_laser_timestampRef		0x02000000
#define PROF_ui_tx_sector_offset		0x04000000
#define PROF_us_num_tx_sectors			0x08000000
#define PROF_ui_watercol_offset_2nd 		0x10000000
#define PROF_ui_watercol_size_2nd 		0x20000000

/* FOURTH 32 BITS ------------------------------------------------------- */
/* extra from HydroSweep MD2 Surf data */
#define PROF_st_params_PORT		0x00000001	/* presence of port time series */
#define PROF_st_params_STBD		0x00000002	/* presence of stbd time series */

#define PROF_us_txBeamIndex		0x00000004
#define PROF_us_txLevel			0x00000008
#define PROF_ss_txBeamAngle		0x00000010
#define PROF_us_txPulseLength		0x00000020
#define PROF_ui_ss_offset		0x00000040
#define PROF_us_no_skipped_ss		0x00000080
#define PROF_us_no_acquired_ss		0x00000100
#define PROF_us_ss_sample_interval	0x00000200
#define PROF_us_bscatClass		0x00000400
#define PROF_us_nrActualGainSets	0x00000800
#define PROF_ss_rxGup			0x00001000
#define PROF_ss_rxGain			0x00002000
#define PROF_ss_ar			0x00004000
#define PROF_us_rxtimeARRAY		0x00008000
#define PROF_ss_rxgainARRAY		0x00010000

/* FIFTH 32 BITS ------------------------------------------------------- */
#define PROF_si_transducerDepth		0x00000001
#define PROF_si_transducerPitch		0x00000002
#define PROF_si_transducerRoll		0x00000004
#define PROF_ui_transducerHeading	0x00000008
#define PROF_si_transducerLatOffset	0x00000010
#define PROF_si_transducerLongOffset	0x00000020
#define PROF_ui_transducerSlantRange	0x00000040
#define PROF_si_transducerAcross 	0x00000080
#define PROF_si_transducerAlong	 	0x00000100
#define PROF_ui_transducerBearing 	0x00000200

/* SIXTH 32 BITS ------------------------------------------------------- */
/* Added for Reson 7K systems */
#define PROF_ui_sonar_settings_offset	0x00000001
#define PROF_ui_ping_number		0x00000002
#define PROF_us_multi_ping_sequence	0x00000004	
#define PROF_ui_num_beams		0x00000008
#define PROF_uc_layer_compensation_flag 0x00000010
#define PROF_f_bs_beam_position		0x00000020
#define PROF_ui_bs_control_flags	0x00000040
#define PROF_us_bs_num_beams_per_side	0x00000080
#define PROF_us_bs_current_beam_number  0x00000100
#define PROF_uc_bs_sample_descriptor	0x00000200
#define PROF_ui_snippet_sample_descriptor 0x00000400

