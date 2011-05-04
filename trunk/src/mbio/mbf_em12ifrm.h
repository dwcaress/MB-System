/*--------------------------------------------------------------------
 *    The MB-system:	mbf_em12ifrm.h	12/5/00
 *	$Id$
 *
 *    Copyright (c) 2000-2011 by
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
 * mbf_em12ifrm.h defines the data structures used by MBIO functions
 * to store multibeam data read from the MBF_EM12IFRM format (MBIO id 58).  
 *
 * Author:	D. W. Caress
 * Date:	December 5, 2000
 * $Log: mbf_em12ifrm.h,v $
 * Revision 5.4  2004/10/18 04:15:46  caress
 * Minor change.
 *
 * Revision 5.3  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.2  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.1  2001/03/22 20:50:02  caress
 * Trying to make version 5.0.beta0
 *
 * Revision 5.0  2000/12/10  20:24:25  caress
 * Initial revision.
 *
 *
 *
 */
/*
 * Notes on the MBF_EM12IFRM data format:
 *   1. IFREMER uses their own software to process swath data.
 *      Currently (2004) the software is called CARAIBES, but
 *      during the 1990's they used an earlier package called 
 *      TRISMUS to process multibeam data. This format was used 
 *      and generated as part of TRISMUS processing of EM12S and
 *      EM12D multibeam data. TRISMUS recast the data as parallel 
 *      files for bathymetry (.SO suffix), imagery (.IM suffix), 
 *      and asynchronous navigation (.NA suffix). We recommend 
 *      that users translate the TRISMUS data to the current 
 *      mbsystem format for processing Simrad multibeam data
 *      (EM300MBA - format 57) before processing.
 *   2. This format is supported read-only in MB-System.
 *   3. The systems of interest:
 *         EM-12S:   Deep water 12kHz multibeam sonar with up to 
 *                   81 beams of bathymetry and up to 523 sidescan 
 *                   samples per bathymetry beam.
 *         EM-12D:   Deep water 12kHz multibeam sonar with up to 
 *                   81 beams of bathymetry and up to 523 sidescan 
 *                   samples per bathymetry beam. This is a dual
 *                   system which produces alternating port and
 *                   starboard pings to provide a 150 degree swath.
 *   4. The navigation data contain NMEA type ascii strings
 *      beginning with "$CASTM". The sources of these strings
 *      include "NACOU", "NAGP1",  and "NAGP2".
 *      MB-System uses only the "NACOU" strings.
 *   5. Each bathymetry record is 1032 bytes long and begins 
 *      with a 35 character string containing the identifier 
 *      and time stamp. The first six characters are one of
 *      the following:
 *           "$12SOC": EM12S ping
 *           "$12SOB": EM12D port ping
 *           "$12SOT": EM12S starboard ping
 *           "$COMM:": Comment (MB-System only)
 *   6. The imagery records are of variable length and begin 
 *      with a 35 character string containing the identifier 
 *      and time stamp. The first six characters are one of
 *      the following:
 *           "$12IMC": EM12S ping
 *           "$12IMB": EM12D port ping
 *           "$12IMT": EM12S starboard ping
 *      As with the Simrad vendor formats, multiple sidescan 
 *      datagrams are recorded for each ping.
 *
 */

/* maximum number of beams and pixels */
#define	MBF_EM12IFRM_MAXBEAMS	81
#define	MBF_EM12IFRM_MAXRAWPIXELS	50*MBF_EM12IFRM_MAXBEAMS
#define	MBF_EM12IFRM_MAXPIXELS	1024

struct mbf_em12ifrm_struct
	{
	/* type of data record */
	int	kind;			/* Data vs Comment */

	/* type of sonar */
	int	sonar;			/* Type of Simrad sonar */

	/* parameter info (start, stop and parameter datagrams) */
	int	par_year;
	int	par_month;
	int	par_day;
	int	par_hour;
	int	par_minute;
	int	par_second;
	int	par_centisecond;
	int	pos_type;	/* positioning system type */
	double	pos_delay;	/* positioning system delay (sec) */
	double	roll_offset;	/* roll offset (degrees) */
	double	pitch_offset;	/* pitch offset (degrees) */
	double	heading_offset;	/* heading offset (degrees) */
	double	em100_td;	/* EM-100 tranducer depth (meters) */
	double	em100_tx;	/* EM-100 tranducer fore-aft 
					offset (meters) */
	double	em100_ty;	/* EM-100 tranducer athwartships 
					offset (meters) */
	double	em12_td;	/* EM-12 tranducer depth (meters) */
	double	em12_tx;	/* EM-12 tranducer fore-aft 
					offset (meters) */
	double	em12_ty;	/* EM-12 tranducer athwartships 
					offset (meters) */
	double	em1000_td;	/* EM-1000 tranducer depth (meters) */
	double	em1000_tx;	/* EM-1000 tranducer fore-aft 
					offset (meters) */
	double	em1000_ty;	/* EM-1000 tranducer athwartships 
					offset (meters) */
	char	spare_parameter[128];
	int	survey_line;
	char	comment[80];

	/* position (position datagrams) */
	int	pos_year;
	int	pos_month;
	int	pos_day;
	int	pos_hour;
	int	pos_minute;
	int	pos_second;
	int	pos_centisecond;
	double	latitude;
	double	longitude;
	double	utm_northing;
	double	utm_easting;
	int	utm_zone;
	double	utm_zone_lon;
	int	utm_system;
	int	pos_quality;
	double	speed;			/* meters/second */
	double	line_heading;		/* degrees */

	/* sound velocity profile */
	int	svp_year;
	int	svp_month;
	int	svp_day;
	int	svp_hour;
	int	svp_minute;
	int	svp_second;
	int	svp_centisecond;
	int	svp_num;
	int	svp_depth[100]; /* meters */
	int	svp_vel[100];	/* 0.1 meters/sec */

	/* time stamp */
	int	year;
	int	month;
	int	day;
	int	hour;
	int	minute;
	int	second;
	int	centisecond;

	/* swath id */
	int	swath_id;	/* EM_SWATH_CENTER:	0
				   EM_SWATH_PORT:	-1  (EM12D only)
				   EM_SWATH_STARBOARD:	1   (EM12D only) */

	/* bathymetry */
	int	ping_number;
	int	beams_bath;	/* EM-1000:  60
				   EM12S:    81
				   EM12D:    81 */
	int	bath_mode;	/* EM-1000: 1=deep; 2=medium; 3=shallow 
				   EM-12S:  1=shallow equiangle spacing
				            2=deep equiangle spacing
				            3=shallow equidistant spacing
				            4=deep 120 degree equidistant
				            5=deep 105 degree equidistant
				            6=deep 90 degree equidistant
				   EM-12D:  1=shallow equiangle spacing
				            2=deep equiangle spacing
				            3=shallow equidistant spacing
				            4=deep 150 degree equidistant
				            5=deep 140 degree equidistant
				            6=deep 128 degree equidistant
				            7=deep 114 degree equidistant
				            8=deep 98 degree equidistant */
	int	bath_res;	/* EM-12 only: 1=high res; 2=low res */
	int	bath_quality;	/* number of good beams, 
					negative if ping rejected */
	int	keel_depth;	/* depth of most vertical beam:
					EM-1000:        0.02 meters 
					EM-12 high res: 0.10 meters 
					EM-12 low res:  0.20 meters */
	int	heading;	/* 0.1 degrees */
	int	roll;		/* 0.01 degrees */
	int	pitch;		/* 0.01 degrees */
	int	xducer_pitch;	/* 0.01 degrees */
	int	ping_heave;	/* 0.01 meters */
	int	sound_vel;	/* 0.1 meters/sec */
	short int bath[MBF_EM12IFRM_MAXBEAMS];	
				/* depths:
					EM-1000:        0.02 meters 
					EM-12 high res: 0.10 meters 
					EM-12 low res:  0.20 meters */
	short int bath_acrosstrack[MBF_EM12IFRM_MAXBEAMS];
				/* acrosstrack distances:
					EM-1000:         0.1 meters 
					EM-12 high res:  0.2 meters 
					EM-12 low res:   0.5 meters */
	short int bath_alongtrack[MBF_EM12IFRM_MAXBEAMS];
				/* alongtrack distances:
					EM-1000:         0.1 meters 
					EM-12 high res:  0.2 meters 
					EM-12 low res:   0.5 meters */
	short int tt[MBF_EM12IFRM_MAXBEAMS];	/* meters */
				/* travel times:
					EM-1000:         0.05 msec 
					EM-12 high res:  0.20 msec 
					EM-12 low res:   0.80 msec */
	mb_s_char	amp[MBF_EM12IFRM_MAXBEAMS];	    /* 0.5 dB */
	mb_u_char	quality[MBF_EM12IFRM_MAXBEAMS];	    /* meters */
	mb_s_char	heave[MBF_EM12IFRM_MAXBEAMS];	    /* 0.1 meters */
	
	/* sidescan */
	int	pixels_ssraw;	/* total number of samples for this ping */
	int	ss_mode;	/* 1 = EM-12 shallow:   0.6 m/sample
				   2 = EM-12 deep:      2.4 m/sample
				   3 = EM-1000 deep:    0.3 m/sample
				   4 = EM-1000 medium:  0.3 m/sample
				   5 = EM-1000 shallow: 0.15 m/sample */
	short int beam_frequency[MBF_EM12IFRM_MAXBEAMS]; 
				/*	0 = 12.67 kHz
					1 = 13.00 kHz
					2 = 13.33 kHz
					3 = 95.00 kHz */
	short int beam_samples[MBF_EM12IFRM_MAXBEAMS];	
				/* number of sidescan samples derived from
					each beam */
	short int beam_center_sample[MBF_EM12IFRM_MAXBEAMS];
				/* center beam sample number among samples
					from one beam */	
	short int beam_start_sample[MBF_EM12IFRM_MAXBEAMS];
				/* start beam sample number among samples
					from entire ping */
	mb_s_char ssraw[MBF_EM12IFRM_MAXRAWPIXELS];
	short int ssp[MBF_EM12IFRM_MAXRAWPIXELS];
	int	pixel_size;	/* processed sidescan pixel size in cm */
	int	pixels_ss;	/* number of processed sidescan pixels stored */
	short	ss[MBF_EM12IFRM_MAXPIXELS];
				/* the processed sidescan ordered port to starboard */
	short	ssalongtrack[MBF_EM12IFRM_MAXPIXELS];
				/* the processed sidescan alongtrack distances 
					in distance resolution units */

};

