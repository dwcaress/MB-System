/*--------------------------------------------------------------------
 *    The MB-system:	mb_io.h	1/19/93
 *    $Id: mb_segy.h,v 5.1 2004-05-21 23:44:49 caress Exp $
 *
 *    Copyright (c) 2004 by
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
 * mb_segy.h defines the SEG-Y trace header used by MB-System when 
 * extracting seismic reflection or subbottom profiler data from 
 * swath mapping data files. The SIOSEIS implementation of the
 * trace header is used because it incorporates a deep water delay
 * value not found in most other implementations.See the SIOSEIS
 * web pages for information:.
 *	http://sioseis.ucsd.edu/
 *	http://sioseis.ucsd.edu/segy.header.html
 *
 * Author:	D. W. Caress
 * Date:	April 13, 2004
 *
 * $Log: not supported by cvs2svn $
 * Revision 5.0  2004/04/27 01:50:15  caress
 * Adding support for Reson 7k sonar data, including segy extensions.
 *
 *
 */

#define   MB_SEGY_HEADER_LENGTH  240
struct mb_segyasciiheader_struct
	{
	char line[80][40];
	};
struct mb_segytapeheader_struct
	{
	int	jobid;
	int	line;
	int	reel;
	short	channels;
	short	aux_channels;
	short	sample_interval;
	short	sample_interval_org;
	short	number_samples;
	short	number_samples_org;
	short	format; /* 5 = IEEE floating point */
	short	cdp_fold;
	short	trace_sort;
	short	vertical_sum;
	short	sweep_start;
	short	sweep_end;
	short	sweep_length;
	short	sweep_type;
	short	sweep_trace;
	short	sweep_taper_start;
	short	sweep_taper_end;
	short	sweep_taper;
	short	correlated;
	short	binary_gain;
	short	amplitude;
	short	units;
	short	impulse_polarity;
	short	vibrate_polarity;
	char	extra[340];
	};
struct mb_segyheader_struct 
	{ 
        long            seq_num;        /* bytes 0-3, trace sequence number in the line */
        long            seq_reel;       /* bytes 4-7, trace sequence number in the reel */
        long            shot_num;       /* bytes 8-11, shot number or stacked trace number
                                                   "Original field record number"  */
        long            shot_tr;        /* bytes 12-15, trace number within the shot */
        long            espn;   	/* bytes 16-19, "Energy source point number -- used
                                                    when more than one record occurs at
                                                    the same effective surface location". */
        long            rp_num; 	/* bytes 20-23,  rp or cdp number  */
        long            rp_tr;  	/* bytes 24-27,  trace number within the cdp */
        short		trc_id; 	/* bytes 28-29,  trace id:  1= live, 2=dead  */
        short   	num_vstk;       /* bytes 30-31,  Number of traces vertically stacked */
        short   	cdp_fold;       /* bytes 32-33,  cdp fold (coverage)  */
        short   	use;            /* bytes 34-35,  Data use:  1=production, 2=test  */
        long            range;  	/* bytes 36-39,  source to receiver distance (range) */
        long            grp_elev;       /* bytes 40-43,  Receiver group elevation w.r.t.
                                                          sea level (depth is negative)  */
        long            src_elev;       /* bytes 44-47,  Source elevation  */
        long            src_depth; 	/* bytes 48-51,  Source depth below surface.
                                                        (depth is a positive number!) */
        long            grp_datum; 	/* bytes 51-55,  Datum elevation at receiver group.  */
        long            src_datum; 	/* bytes 56-59,  Datum elevation at source */
        long            src_wbd;        /* bytes 60-63,  water depth at the source */
        long            grp_wbd;        /* bytes 64-67,  water depth at the receiver group. */
        short   	elev_scalar; 	/* bytes 68-69, Scalar to be applied to elevations
                                                	and depths in bytes 41-68.
                                                	If positive use as a multiplier.
                                                	If negative, use as a divisor.  */
        short   	coord_scalar; 	/* bytes 70-71, Scalar to be applied to
                                                	coordinates in bytes 72-87.
                                                	If positive use as a multiplier.
                                                	If negative, use as a divisor.  */
        long            src_long;       /* bytes 72-75, longitude in seconds of arc.
                                                	Source X coordinate */
        long            src_lat;        /* bytes 76-79, latitude in seconds of arc.
                                         		Source Y coordinate   */
        long    	grp_long;       /* bytes 80-83, Receiver longitude or X coordinate */
        long            grp_lat;        /* bytes 84-87, Receiver latitude or Y coordinate */
        short   	coord_units; 	/* bytes 88-89, = 2, coordinate units = seconds of arc */
        short   	wvel;   	/* bytes 90-91, weathering or water velocity */
        short   	sbvel;  	/* bytes 92-93, subweathering velocity  */
        short   	src_up_vel; 	/* bytes 94-95, uphole time at source  */
        short   	grp_up_vel; 	/* bytes 96-97, uphole time at group  */
        short   	src_static; 	/* bytes 98-99, Source static correction  */
        short   	grp_static; 	/* bytes 100-101, Group static correction  */
        short   	tot_static; 	/* bytes 102-103, Total static applied */
        short   	laga;   	/* bytes 104-105, Lag time A in ms. before time 0 */
/*****  short   	lagb;   	/* bytes 106-107, Lag time B in ms. before time 0 */
        long            delay_mils; 	/* bytes 106-109, deep water delay in ms. (or meters)  */
        short   	smute_mils; 	/* bytes 110-111, start mute time in ms. */
        short   	emute_mils; 	/* bytes 112-113, end mutes time in ms.  */
        short   	nsamps; 	/* bytes 114-115, "Number of data samples in this
                                                	trace" - excludes header */
        short   	si_micros;      /* bytes 116-117, Sample interval in us for this trace  */
        short   	other_1[19]; 	/* bytes 118-155, Other short integer stuff */
        short   	year;   	/* bytes 156-157, year data was recorded. */
        short   	day_of_yr; 	/* bytes 158-159, recording day of year */
        short   	hour;   	/* bytes 160-161, recording hour of day  */
        short   	min;            /* bytes 162-163, recording minute of hour  */
        short   	sec;            /* bytes 164-165, recording second of minute */
        short   	mils;           /* bytes 166-167, recording millisecond  */
                                        /* OFFICIAL SEGY says: "time basis code"  */
        short   	tr_weight; 	/* bytes 168-169, Trace weighting factor  */
        short   	other_2[5]; 	/* bytes 170-179, Other short integer stuff */
        float   	delay;  	/* bytes 180-183, deep water delay in seconds (or meters) */
        float   	smute_sec; 	/* bytes 184-187, start mute time in seconds */
        float   	emute_sec;      /* bytes 188-191, end mute time in seconds  */
        float   	si_secs;        /* bytes 192-195, sample interval in seconds */
        float   	wbt_secs;       /* bytes 196-199, water bottom time in seconds */
        long            end_of_rp; 	/* bytes 200-203, <0, indicates the end of a gather
                                                        >0, the number of traces stacked
                                                        Also EdgeTech's Trace Scalar.  */
        float   	dummy1; 	/* bytes 204-207 */
        float   	dummy2; 	/* bytes 208-211 */
        float   	dummy3; 	/* bytes 212-215 */
        float   	dummy4; 	/* bytes 216-219 */
        float   	dummy5; 	/* bytes 220-223 */
        float   	dummy6; 	/* bytes 224-227 */
        float   	dummy7; 	/* bytes 228-231 */
        float   	dummy8; 	/* bytes 232-235 */
        float   	dummy9; 	/* bytes 236-239 */
};
