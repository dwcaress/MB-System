/*--------------------------------------------------------------------
 *    The MB-system:	mbf_omghdcsj.h	3/10/99
 *	$Id: mbf_omghdcsj.h,v 4.1 2000-09-30 06:34:20 caress Exp $
 *
 *    Copyright (c) 1999, 2000 by
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
 * $Log: not supported by cvs2svn $
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
#define	MBF_OMGHDCSJ_MAX_COMMENT	252
#define	MBF_OMGHDCSJ_MAX_BEAMS		1440
#define	MBF_OMGHDCSJ_MAX_PIXELS		1024

/* define tools (sonars) supported by OMG_HDCS */
#define MBF_OMGHDCSJ_NUM_TOOLS		25   /* as of March 1998 */
#define MBF_OMGHDCSJ_None		-1
#define MBF_OMGHDCSJ_SingleBeam		0
#define MBF_OMGHDCSJ_ELAC_BottomChart	1
#define MBF_OMGHDCSJ_EM12_dual		2
#define MBF_OMGHDCSJ_EM100_depth	3
#define MBF_OMGHDCSJ_FanSweep		4
#define MBF_OMGHDCSJ_SeaBeam		5
#define MBF_OMGHDCSJ_EM3000		6
#define MBF_OMGHDCSJ_Navitronics_Smith	7
#define MBF_OMGHDCSJ_EM12_single	8
#define MBF_OMGHDCSJ_EM100_depth_ss	9
#define MBF_OMGHDCSJ_EM1000		10
#define MBF_OMGHDCSJ_LADS_2ndary	11
#define MBF_OMGHDCSJ_EM3000D 		12
#define MBF_OMGHDCSJ_SB2100		13
#define MBF_OMGHDCSJ_ISIS_Submetrix	14
#define MBF_OMGHDCSJ_EM1000_ampl	15
#define MBF_OMGHDCSJ_SB2K      		16
#define MBF_OMGHDCSJ_Seabat9001		17
#define MBF_OMGHDCSJ_FanSweep_10A	18
#define MBF_OMGHDCSJ_FanSweep_20	19
#define MBF_OMGHDCSJ_ISIS_SWA		20
#define MBF_OMGHDCSJ_SeaBeam_1180_MkII	21
#define MBF_OMGHDCSJ_SeaBat_8101	22
#define MBF_OMGHDCSJ_EM300		23
#define MBF_OMGHDCSJ_EM121A		24
#define MBF_OMGHDCSJ_COMMENT		999

/* define tables of data record sizes */
#define MBF_OMGHDCSJ_PROFILE_LENGTH	0
#define MBF_OMGHDCSJ_MAX_NO_BEAMS	1
#define MBF_OMGHDCSJ_BEAM_LENGTH	2
#define MBF_OMGHDCSJ_IMAGE_LENGTH	3
static int mbf_omghdcsj_tooldefs1[MBF_OMGHDCSJ_NUM_TOOLS][4] = 
		    {/*                            PRS     no. DRS  IRS */
                    /*   0:SingleBeam,        */  { 44 ,    1,   8  , 0    },
                    /*   1:ELAC BottomChartMk1*/  { 64 ,   56,  44  , 0    },
                    /*   2:EM12(dual)         */  { 64 ,  162,  64  , 0    },
                    /*   3:EM100 (just depths)*/  { 44 ,   32,  36  , 0    },
                    /*   4:FanSweep10 (old)   */  { 44 ,   52,  36  , 0    },
                    /*   5:Seabeam "Classic"  */  { 24 ,   19,  24  , 0    },
                    /*   6:EM3000S            */  { 64 ,  128,  64  , 0    },
                    /*   7:Navitronics        */  { 44 ,   33,  36  , 0    },
                    /*   8:EM12(single)       */  { 64 ,   81,  64  , 0    },
                    /*   9:EM100+Amplitudes   */  { 64 ,   32,  44  , 0    },
                    /*  10:EM1000             */  { 64 ,   60,  64  , 0    },
                    /*  11:LADS secondary     */  { 44 ,   24,  36  , 0    },
                    /*  12:EM3000D            */  { 64 ,  256,  64  , 0    },
                    /*  13:Seabeam 2100       */  { 44 ,  120,  44  , 0    },
                    /*  14:ISIS Submetrix     */  { 44 ,  100,  44  , 0    },
                    /*  15:EM-1000 (justampl) */  { 44 ,   60,  44  , 0    },
                    /*  16:SB2K               */  { 64 ,  121,  64  , 0    },
                    /*  17:Seabat9001         */  { 44 ,   60,  44  , 0    },
                    /*  18:FanSweep 10 A      */  { 64 ,   52,  64  , 0    },
                    /*  19:FanSweep 20        */  { 64 , 1440,  64  , 0    },
                    /*  20:ISIS SWA format    */  { 64 , 1280,  64  , 0    },
                    /*  21:SeaBeam 1180 Mk II */  { 64 ,  126,  44  , 0    },
                    /*  22:SeaBat 8101        */  { 64 ,  101,  64  , 0    },
                    /*  23:EM300              */  { 88 ,  137,  76  , 0    },
                    /*  24:EM121A             */  { 64 ,  121,  64  , 0    }
		    };
static int mbf_omghdcsj_tooldefs2[MBF_OMGHDCSJ_NUM_TOOLS][4] = 
		    {/*                            PRS     no. DRS  IRS */
                    /*   0:SingleBeam,        */  { 32 ,    1,   5  , 0    },
                    /*   1:ELAC Bottom_Chart  */  { 32 ,   56,  12  , 0    },
                    /*   2:EM12(dual)         */  { 32 ,  162,  00  , 0    },
                    /*   3:EM100 (just depths)*/  { 32 ,   32,  00  , 0    },
                    /*   4:FanSweep           */  { 32 ,   52,  00  , 0    },
                    /*   5:Seabeam            */  { 32 ,   19,  10  , 0    },
                    /*   6:EM3000S            */  { 32 ,  128,  20  , 0    },
                    /*   7:Navitronics        */  { 32 ,   33,  00  , 0    },
                    /*   8:EM12(single)       */  { 32 ,   81,  00  , 0    },
                    /*   9:EM100+Amplitudes   */  { 32 ,   32,  12  , 0    },
                    /*  10:EM1000             */  { 32 ,   60,  20  , 0    },
                    /*  11:LADS secondary     */  { 32 ,   24,  12  , 0    },
                    /*  12:EM3000D            */  { 32 ,  256,  20  , 0    },
                    /*  13:Seabeam 2100       */  { 32 ,  120,  12  , 0    },
                    /*  14:ISIS Submetrix     */  { 32 ,  100,  20  , 0    },
                    /*  15:EM-1000 (justampl) */  { 32 ,   60,  00  , 0    },
                    /*  16:SB2K               */  { 32 ,  121,  00  , 0    },
                    /*  17:Seabat9001         */  { 32 ,   60,  12  , 0    },
                    /*  18:FanSweep 10 A      */  { 32 ,   52,  20  , 0    },
                    /*  19:FanSweep 20        */  { 32 , 1440,  12  , 0    },
                    /*  20:ISIS SWA format    */  { 32 , 1280,   8  , 0    },
                    /*  21:SeaBeam 1180 Mk II */  { 32 ,  126,  12  , 0    },
                    /*  22:SeaBat 8101        */  { 32 ,  101,  12  , 0    },
                    /*  23:EM300              */  { 32 ,  137,  20  , 0    },
                    /*  23:EM121A              */  { 32 , 121,  20  , 0    }
		    };
static int mbf_omghdcsj_tooldefs3[MBF_OMGHDCSJ_NUM_TOOLS][4] = 
		    {/*                            PRS     no. DRS  IRS */
                    /*   0:SingleBeam,        */  { 32 ,    1,   5  , 0    },
                    /*   1:ELAC Bottom_Chart  */  { 32 ,   56,  12  , 0    },
                    /*   2:EM12(dual)         */  { 32 ,  162,  00  , 0    },
                    /*   3:EM100 (just depths)*/  { 32 ,   32,  00  , 0    },
                    /*   4:FanSweep           */  { 32 ,   52,  00  , 0    },
                    /*   5:Seabeam            */  { 32 ,   19,  10  , 0    },
                    /*   6:EM3000S            */  { 32 ,  128,  20  , 0    },
                    /*   7:Navitronics        */  { 32 ,   33,  00  , 0    },
                    /*   8:EM12(single)       */  { 32 ,   81,  00  , 0    },
                    /*   9:EM100+Amplitudes   */  { 32 ,   32,  12  , 0    },
                    /*  10:EM1000             */  { 32 ,   60,  20  , 0    },
                    /*  11:LADS secondary     */  { 32 ,   24,  12  , 0    },
                    /*  12:EM3000D            */  { 32 ,  256,  20  , 0    },
                    /*  13:Seabeam 2100       */  { 32 ,  120,  12  , 0    },
                    /*  14:ISIS Submetrix     */  { 32 ,  100,  20  , 0    },
                    /*  15:EM-1000 (justampl) */  { 32 ,   60,  00  , 0    },
                    /*  16:SB2K               */  { 32 ,  121,  00  , 0    },
                    /*  17:Seabat9001         */  { 32 ,   60,  12  , 0    },
                    /*  18:FanSweep 10 A      */  { 32 ,   52,  20  , 0    },
                    /*  19:FanSweep 20        */  { 32 , 1440,  12  , 0    },
                    /*  20:ISIS SWA format    */  { 32 , 1280,   8  , 0    },
                    /*  21:SeaBeam 1180 Mk II */  { 32 ,  126,  12  , 0    },
                    /*  22:SeaBat 8101        */  { 32 ,  101,  12  , 0    },
                    /*  23:EM300              */  { 56 ,  135,  28  , 0    }, /* note using the correct 135 */
                    /*  23:EM121A             */  { 32 ,  121,  20  , 0    }
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
	};

/* define beam record structure */
struct mbf_omghdcsj_beam_struct {
        int	status;           /* status is either OK (0) or bad (other) */
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
