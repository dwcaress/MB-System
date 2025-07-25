/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_xse.c	3/27/2000
 *
 *    Copyright (c) 2000-2025 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes 
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *     
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbsys_xse.h contains the functions for handling
 * the data structure used by MBIO functions
 * to store swath sonar data in the XSE Data Exchange Format
 * developed by L-3 Communications ELAC Nautik.
 * This format is used for data from ELAC Bottomchart multibeam sonars
 * and SeaBeam 2100 multibeam sonars (made by L-3 Communications
 * SeaBeam Instruments).
 * The data format associated with XSE is:
 *      MBF_L3XSERAW : MBIO ID 94
 *
 * Author:	D. W. Caress
 * Date:	August 1,  1999
 * Additional Authors:	P. A. Cohen and S. Dzurenko
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_xse.h"

/*--------------------------------------------------------------------*/
int mbsys_xse_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* allocate memory for data structure */
	const int status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_xse_struct), store_ptr, error);

	/* get data structure pointer */
	struct mbsys_xse_struct *store = (struct mbsys_xse_struct *)*store_ptr;

	/* initialize everything */
	/* type of data record */
	store->kind = MB_DATA_NONE; /* Survey, nav, Comment */

	/* parameter (ship frames) */
	store->par_parameter = false;    /* boolean flag for parameter group */
	store->par_source = 0;           /* sensor id */
	store->par_sec = 0;              /* sec since 1/1/1901 00:00 */
	store->par_usec = 0;             /* microseconds */
	store->par_roll_bias = 0.0;      /* radians */
	store->par_pitch_bias = 0.0;     /* radians */
	store->par_heading_bias = 0.0;   /* radians */
	store->par_time_delay = 0.0;     /* nav time lag, seconds */
	store->par_trans_x_port = 0.0;   /* port transducer x position, meters */
	store->par_trans_y_port = 0.0;   /* port transducer y position, meters */
	store->par_trans_z_port = 0.0;   /* port transducer z position, meters */
	store->par_trans_x_stbd = 0.0;   /* starboard transducer x position, meters */
	store->par_trans_y_stbd = 0.0;   /* starboard transducer y position, meters */
	store->par_trans_z_stbd = 0.0;   /* starboard transducer z position, meters */
	store->par_trans_err_port = 0.0; /* port transducer rotation in roll direction, radians */
	store->par_trans_err_stbd = 0.0; /* starboard transducer rotation in roll direction, radians */
	store->par_nav_x = 0.0;          /* navigation antenna x position, meters */
	store->par_nav_y = 0.0;          /* navigation antenna y position, meters */
	store->par_nav_z = 0.0;          /* navigation antenna z position, meters */
	store->par_hrp_x = 0.0;          /* motion sensor x position, meters */
	store->par_hrp_y = 0.0;          /* motion sensor y position, meters */
	store->par_hrp_z = 0.0;          /* motion sensor z position, meters */

	/* navigation and motion (ship frames) */
	store->par_navigationandmotion = false; /* boolean flag for navigationandmotion group */
	store->par_nam_roll_bias = 0.0;         /* roll bias, radians */
	store->par_nam_pitch_bias = 0.0;        /* pitch bias, radians */
	store->par_nam_heave_bias = 0.0;        /* heave bias, meters */
	store->par_nam_heading_bias = 0.0;      /* heading/gyro bias, radians */
	store->par_nam_time_delay = 0.0;        /* nav time lag, seconds */
	store->par_nam_nav_x = 0.0;             /* navigation antenna x position, meters */
	store->par_nam_nav_y = 0.0;             /* navigation antenna y position, meters */
	store->par_nam_nav_z = 0.0;             /* navigation antenna z position, meters */
	store->par_nam_hrp_x = 0.0;             /* motion sensor x position, meters */
	store->par_nam_hrp_y = 0.0;             /* motion sensor y position, meters */
	store->par_nam_hrp_z = 0.0;             /* motion sensor z position, meters */

	store->par_xdr_num_transducer = 0; /* number of transducers */
	for (int i = 0; i < MBSYS_XSE_MAX_TRANSDUCERS; i++) {
		store->par_xdr_sensorid[i] = 0;           /* sensor ids */
		store->par_xdr_transducer[i] = 0;         /* transducer type:
		                                  0: hydrophone
		                                  1: projector
		                                  2: transducer */
		store->par_xdr_frequency[i] = 0;          /* frequency (Hz) */
		store->par_xdr_side[i] = 0;               /* transducer side:
		                                      0: undefined
		                                      1: port
		                                      2: starboard
		                                      3: midship
		                                      4: system defined */
		store->par_xdr_mountingroll[i] = 0.0;     /* array mounting angle roll (radians) */
		store->par_xdr_mountingpitch[i] = 0.0;    /* array mounting angle roll (radians) */
		store->par_xdr_mountingazimuth[i] = 0.0;  /* array mounting angle roll (radians) */
		store->par_xdr_mountingdistance[i] = 0.0; /* horizontal distance between
		                              innermost elements of the
		                              transducer arrays to the
		                              ship center line in a
		                              V-shaped configuration (m) */
		store->par_xdr_x[i] = 0.0;                /* transducer center across track offset (m) */
		store->par_xdr_y[i] = 0.0;                /* transducer center along track offset (m) */
		store->par_xdr_z[i] = 0.0;                /* transducer center vertical offset (m) */
		store->par_xdr_roll[i] = 0.0;             /* beamforming roll bias (radians - port up positive) */
		store->par_xdr_pitch[i] = 0.0;            /* beamforming pitch bias (radians - bow up positive) */
		store->par_xdr_azimuth[i] = 0.0;          /* beamforming azimuth bias (radians
		                                  - projector axis clockwise with
		                                  respect to compass positive) */
	}
	store->par_xdx_num_transducer = 0; /* number of transducers */
	for (int i = 0; i < MBSYS_XSE_MAX_TRANSDUCERS; i++) {
		store->par_xdx_roll[i] = 0.0;    /* mounting mode roll (0: auto, 1: manual) */
		store->par_xdx_pitch[i] = 0.0;   /* mounting mode pitch (0: auto, 1: manual) */
		store->par_xdx_azimuth[i] = 0.0; /* mounting mode azimuth (0: auto, 1: manual) */
	}

	/* svp (sound velocity frames) */
	store->svp_source = 0; /* sensor id */
	store->svp_sec = 0;    /* sec since 1/1/1901 00:00 */
	store->svp_usec = 0;   /* microseconds */
	store->svp_nsvp = 0;   /* number of depth values */
	store->svp_nctd = 0;   /* number of ctd values */
	store->svp_ssv = 0.0;  /* m/s */
	for (int i = 0; i < MBSYS_XSE_MAXSVP; i++) {
		store->svp_depth[i] = 0.0;        /* m */
		store->svp_velocity[i] = 0.0;     /* m/s */
		store->svp_conductivity[i] = 0.0; /* mmho/cm */
		store->svp_salinity[i] = 0.0;     /* o/oo */
		store->svp_temperature[i] = 0.0;  /* degree celcius */
		store->svp_pressure[i] = 0.0;     /* bar */
	}
	store->svp_ssv_depth = 0.0;   /* m */
	store->svp_ssv_depthflag = 0; /* 0 = invalid depth, otherwise depth valid */

	/* position (navigation frames) */
	store->nav_group_general = false;  /* boolean flag */
	store->nav_group_position = false; /* boolean flag */
	store->nav_group_accuracy = false; /* boolean flag */
	store->nav_group_motiongt = false; /* boolean flag */
	store->nav_group_motiontw = false; /* boolean flag */
	store->nav_group_track = false;    /* boolean flag */
	store->nav_group_hrp = false;      /* boolean flag */
	store->nav_group_heave = false;    /* boolean flag */
	store->nav_group_roll = false;     /* boolean flag */
	store->nav_group_pitch = false;    /* boolean flag */
	store->nav_group_heading = false;  /* boolean flag */
	store->nav_group_log = false;      /* boolean flag */
	store->nav_group_gps = false;      /* boolean flag */
	store->nav_source = 0;             /* sensor id */
	store->nav_sec = 0;                /* sec since 1/1/1901 00:00 */
	store->nav_usec = 0;               /* microseconds */
	store->nav_quality = 0;
	store->nav_status = 0;
	store->nav_description_len = 0;
	for (int i = 0; i < MBSYS_XSE_DESCRIPTION_LENGTH; i++)
		store->nav_description[i] = 0;
	store->nav_x = 0.0;                     /* eastings (m) or
	                                longitude (radians) */
	store->nav_y = 0.0;                     /* northings (m) or
	                                latitude (radians) */
	store->nav_z = 0.0;                     /* height (m) or
	                                ellipsoidal height (m) */
	store->nav_acc_quality = 0;             /* GPS quality:
	                                0: invalid
	                                1: SPS
	                                2: SPS differential
	                                3: PPS
	                                4. RTK
	                                5: Float RTK
	                                6: Estimated
	                                7: Manual
	                                8: Simulator */
	store->nav_acc_numsatellites = 0;       /* number of satellites */
	store->nav_acc_horizdilution = 0.0;     /* horizontal dilution of precision */
	store->nav_acc_diffage = 0.0;           /* age of differential data (sec since last update) */
	store->nav_acc_diffref = 0;             /* differential reference station */
	store->nav_speed_ground = 0.0;          /* m/s */
	store->nav_course_ground = 0.0;         /* radians */
	store->nav_speed_water = 0.0;           /* m/s */
	store->nav_course_water = 0.0;          /* radians */
	store->nav_trk_offset_track = 0.0;      /* offset track (m) */
	store->nav_trk_offset_sol = 0.0;        /* offset SOL (m) */
	store->nav_trk_offset_eol = 0.0;        /* offset EOL (m) */
	store->nav_trk_distance_sol = 0.0;      /* distance SOL (m) */
	store->nav_trk_azimuth_sol = 0.0;       /* azimuth SOL (radians) */
	store->nav_trk_distance_eol = 0.0;      /* distance EOL (m) */
	store->nav_trk_azimuth_eol = 0.0;       /* azimuth EOL (radians) */
	store->nav_hrp_heave = 0.0;             /* heave (m) */
	store->nav_hrp_roll = 0.0;              /* roll (radians) */
	store->nav_hrp_pitch = 0.0;             /* pitch (radians) */
	store->nav_hea_heave = 0.0;             /* heave (m) */
	store->nav_rol_roll = 0.0;              /* roll (radians) */
	store->nav_pit_pitch = 0.0;             /* pitch (radians) */
	store->nav_hdg_heading = 0.0;           /* heading (radians) */
	store->nav_log_speed = 0.0;             /* speed (m/s) */
	store->nav_gps_altitude = 0.0;          /* altitude with respect to geoid */
	store->nav_gps_geoidalseparation = 0.0; /* difference between WGS84 ellipsoid and geoid (m)
	                        (positive means sea level geoid is above
	                        ellipsoid) */

	/* survey depth (multibeam frames) */
	store->mul_frame = false;              /* boolean flag - multibeam frame read */
	store->mul_group_beam = false;         /* boolean flag - beam group read */
	store->mul_group_tt = false;           /* boolean flag - tt group read */
	store->mul_group_quality = false;      /* boolean flag - quality group read */
	store->mul_group_amp = false;          /* boolean flag - amp group read */
	store->mul_group_delay = false;        /* boolean flag - delay group read */
	store->mul_group_lateral = false;      /* boolean flag - lateral group read */
	store->mul_group_along = false;        /* boolean flag - along group read */
	store->mul_group_depth = false;        /* boolean flag - depth group read */
	store->mul_group_angle = false;        /* boolean flag - angle group read */
	store->mul_group_heave = false;        /* boolean flag - heave group read */
	store->mul_group_roll = false;         /* boolean flag - roll group read */
	store->mul_group_pitch = false;        /* boolean flag - pitch group read */
	store->mul_group_gates = false;        /* boolean flag - gates group read */
	store->mul_group_noise = false;        /* boolean flag - noise group read */
	store->mul_group_length = false;       /* boolean flag - length group read */
	store->mul_group_hits = false;         /* boolean flag - hits group read */
	store->mul_group_heavereceive = false; /* boolean flag - heavereceive group read */
	store->mul_group_azimuth = false;      /* boolean flag - azimuth group read */
	store->mul_group_properties = false;   /* boolean flag - properties group read */
	store->mul_group_normamp = false;      /* boolean flag - normalized amplitude group read */
	store->mul_group_mbsystemnav = false;  /* boolean flag - mbsystemnav group read */
	store->mul_source = 0;                 /* sensor id */
	store->mul_sec = 0;                    /* sec since 1/1/1901 00:00 */
	store->mul_usec = 0;                   /* microseconds */
	store->mul_lon = 0.0;                  /* interpolated longitude in radians */
	store->mul_lat = 0.0;                  /* interpolated latitude in radians */
	store->mul_heading = 0.0;              /* interpolated heading in radians */
	store->mul_speed = 0.0;                /* interpolated speed in m/s */
	store->mul_ping = 0;                   /* ping number */
	store->mul_frequency = 0.0;            /* transducer frequency (Hz) */
	store->mul_pulse = 0.0;                /* transmit pulse length (sec) */
	store->mul_power = 0.0;                /* transmit power (dB) */
	store->mul_bandwidth = 0.0;            /* receive bandwidth (Hz) */
	store->mul_sample = 0.0;               /* receive sample interval (sec) */
	store->mul_swath = 0.0;                /* swath width (radians) */
	store->mul_num_beams = 0;              /* number of beams */
	for (int i = 0; i < MBSYS_XSE_MAXBEAMS; i++) {
		store->beams[i].tt = 0.0;
		store->beams[i].delay = 0.0;
		store->beams[i].lateral = 0.0;
		store->beams[i].along = 0.0;
		store->beams[i].depth = 0.0;
		store->beams[i].angle = 0.0;
		store->beams[i].heave = 0.0;
		store->beams[i].roll = 0.0;
		store->beams[i].pitch = 0.0;
		store->beams[i].beam = i + 1;
		store->beams[i].quality = 0;
		store->beams[i].amplitude = 0;
		store->beams[i].gate_angle = 0.0;
		store->beams[i].gate_start = 0.0;
		store->beams[i].gate_stop = 0.0;
		store->beams[i].noise = 0.0;
		store->beams[i].length = 0.0;
		store->beams[i].hits = 0;
		store->beams[i].heavereceive = 0.0;
		store->beams[i].azimuth = 0.0;
	}
	store->mul_num_properties = 0; /* number of properties */
	for (int i = 0; i < MBSYS_XSE_MAXPROPERTIES; i++) {
		store->mul_properties_type[i] = 0;
		store->mul_properties_value[i] = 0.0;
	}
	for (int i = 0; i < MBSYS_XSE_MAXPROPERTIES; i++) {
		store->mul_properties_reserved[i] = 0;
	}

	/* survey sidescan (sidescan frames) */
	store->sid_frame = false;           /* boolean flag - sidescan frame read */
	store->sid_group_avt = false;       /* boolean flag - amp vs time group read */
	store->sid_group_pvt = false;       /* boolean flag - phase vs time group read */
	store->sid_group_avl = false;       /* boolean flag - amp vs lateral group read */
	store->sid_group_pvl = false;       /* boolean flag - phase vs lateral group read */
	store->sid_group_signal = false;    /* boolean flag - phase vs lateral group read */
	store->sid_group_ping = false;      /* boolean flag - phase vs lateral group read */
	store->sid_group_complex = false;   /* boolean flag - phase vs lateral group read */
	store->sid_group_weighting = false; /* boolean flag - phase vs lateral group read */
	store->sid_source = 0;              /* sensor id */
	store->sid_sec = 0;                 /* sec since 1/1/1901 00:00 */
	store->sid_usec = 0;                /* microseconds */
	store->sid_ping = 0;                /* ping number */
	store->sid_frequency = 0.0;         /* transducer frequency (Hz) */
	store->sid_pulse = 0.0;             /* transmit pulse length (sec) */
	store->sid_power = 0.0;             /* transmit power (dB) */
	store->sid_bandwidth = 0.0;         /* receive bandwidth (Hz) */
	store->sid_sample = 0.0;            /* receive sample interval (sec) */
	store->sid_avt_sampleus = 0;        /* sample interval (usec) */
	store->sid_avt_offset = 0;          /* time offset (usec) */
	store->sid_avt_num_samples = 0;     /* number of samples */
	for (int i = 0; i < MBSYS_XSE_MAXPIXELS; i++)
		store->sid_avt_amp[i] = 0;  /* sidescan amplitude (dB) */
	store->sid_pvt_sampleus = 0;    /* sample interval (usec) */
	store->sid_pvt_offset = 0;      /* time offset (usec) */
	store->sid_pvt_num_samples = 0; /* number of samples */
	for (int i = 0; i < MBSYS_XSE_MAXPIXELS; i++)
		store->sid_pvt_phase[i] = 0; /* sidescan phase (radians) */
	store->sid_avl_binsize = 0;      /* bin size (mm) */
	store->sid_avl_offset = 0;       /* lateral offset (mm) */
	store->sid_avl_num_samples = 0;  /* number of samples */
	for (int i = 0; i < MBSYS_XSE_MAXPIXELS; i++)
		store->sid_avl_amp[i] = 0;  /* sidescan amplitude (dB) */
	store->sid_pvl_binsize = 0;     /* bin size (mm) */
	store->sid_pvl_offset = 0;      /* lateral offset (mm) */
	store->sid_pvl_num_samples = 0; /* number of samples */
	for (int i = 0; i < MBSYS_XSE_MAXPIXELS; i++)
		store->sid_pvl_phase[i] = 0; /* sidescan phase (radians) */
	store->sid_sig_ping = 0;         /* ping number */
	store->sid_sig_channel = 0;      /* channel number */
	store->sid_sig_offset = 0.0;     /* start offset */
	store->sid_sig_sample = 0.0;     /* bin size / sample interval */
	store->sid_sig_num_samples = 0;  /* number of samples */
	for (int i = 0; i < MBSYS_XSE_MAXPIXELS; i++)
		store->sid_sig_phase[i] = 0;     /* sidescan phase in radians */
	store->sid_png_pulse = 0;            /* pulse type (0=constant, 1=linear sweep) */
	store->sid_png_startfrequency = 0.0; /* start frequency (Hz) */
	store->sid_png_endfrequency = 0.0;   /* end frequency (Hz) */
	store->sid_png_duration = 0.0;       /* pulse duration (msec) */
	store->sid_png_mancode = 0;          /* manufacturer code (1=Edgetech, 2=Elac) */
	store->sid_png_pulseid = 0;          /* pulse identifier */
	for (int i = 0; i < MBSYS_XSE_DESCRIPTION_LENGTH; i++)
		store->sid_png_pulsename[i] = 0; /* pulse name */
	store->sid_cmp_ping = 0;             /* ping number */
	store->sid_cmp_channel = 0;          /* channel number */
	store->sid_cmp_offset = 0.0;         /* start offset (usec) */
	store->sid_cmp_sample = 0.0;         /* bin size / sample interval (usec) */
	store->sid_cmp_num_samples = 0;      /* number of samples */
	for (int i = 0; i < MBSYS_XSE_MAXPIXELS; i++)
		store->sid_cmp_real[i] = 0; /* real sidescan signal */
	for (int i = 0; i < MBSYS_XSE_MAXPIXELS; i++)
		store->sid_cmp_imaginary[i] = 0; /* imaginary sidescan signal */
	store->sid_wgt_factorleft = 0; /* weighting factor for block floating
	               point expansion  --
	               defined as 2^(-N) volts for lsb */
	store->sid_wgt_samplesleft = 0;  /* number of left samples */
	store->sid_wgt_factorright = 0;  /* weighting factor for block floating
	                 point expansion  --
	                 defined as 2^(-N) volts for lsb */
	store->sid_wgt_samplesright = 0; /* number of right samples */

	/* seabeam (seabeam frames) */
	store->sbm_properties = false;    /* boolean flag - sbm properties group read */
	store->sbm_hrp = false;           /* boolean flag - sbm hrp group read */
	store->sbm_signal = false;        /* boolean flag - sbm signal group read */
	store->sbm_sweepsegments = false; /* boolean flag - sbm sweep segments group read */
	store->sbm_spacingmode = false;   /* boolean flag - sbm spacing mode group read */
	store->sbm_message = false;       /* boolean flag - sbm message group read */
	store->sbm_source = 0;            /* sensor id */
	store->sbm_sec = 0;               /* sec since 1/1/1901 00:00 */
	store->sbm_usec = 0;              /* microseconds */
	store->sbm_ping = 0;              /* ping number */
	store->sbm_ping_gain = 0.0;       /* ping gain (dB) */
	store->sbm_pulse_width = 0.0;     /* pulse width (s) */
	store->sbm_transmit_power = 0.0;  /* transmit power (dB) */
	store->sbm_pixel_width = 0.0;     /* pixel width (m) */
	store->sbm_swath_width = 0.0;     /* swath width (radians) */
	store->sbm_time_slice = 0.0;      /* time slice (s) */
	store->sbm_depth_mode = 0;        /* depth mode (1=shallow, 2=deep) */
	store->sbm_beam_mode = 0;         /* focused beam mode (0=off, 1=on) */
	store->sbm_ssv = 0.0;             /* surface sound velocity (m/s) */
	store->sbm_frequency = 0.0;       /* sonar frequency (kHz) */
	store->sbm_bandwidth = 0.0;       /* receiver bandwidth (kHz) */
	store->sbm_heave = 0.0;           /* heave (m) */
	store->sbm_roll = 0.0;            /* roll (radians) */
	store->sbm_pitch = 0.0;           /* pitch (radians) */
	store->sbm_signal_beam = 0;       /* beam number for signal */
	store->sbm_signal_count = 0;      /* number of samples in signal */
	for (int i = 0; i < MBSYS_XSE_MAXSAMPLES; i++)
		store->sbm_signal_amp[i] = 0.0; /* signal values */
	store->sbm_message_id = 0;          /* seabeam message id */
	store->sbm_message_len = 0;         /* seabeam message length */
	for (int i = 0; i < MBSYS_XSE_COMMENT_LENGTH; i++)
		store->sbm_message_txt[i] = 0;          /* seabeam message */
	store->sbm_sweep_direction = 0;             /* sweep direction 0=static, 1=port, 2=starboard */
	store->sbm_sweep_azimuth = 0.0;             /* effective azimuth (radians) */
	store->sbm_sweep_segments = 0;              /* number of segments */
	store->sbm_sweep_seconds = 0;               /* seconds since start of ping and end of sweep segment */
	store->sbm_sweep_micro = 0;                 /* microseconds of seconds */
	store->sbm_sweep_extrapolateazimuth = 0.0;  /* extrapolated azimuth at center of sweep segment (radians) */
	store->sbm_sweep_interpolatedazimuth = 0.0; /* interpolated azimuth at center of sweep segment (radians) */
	store->sbm_sweep_extrapolatepitch = 0.0;    /* extrapolated pitch at center of sweep segment (radians) */
	store->sbm_sweep_interpolatedpitch = 0.0;   /* interpolated pitch at center of sweep segment (radians) */
	store->sbm_sweep_extrapolateroll = 0.0;     /* extrapolated roll at center of sweep segment (radians) */
	store->sbm_sweep_interpolatedroll = 0.0;    /* interpolated roll at center of sweep segment (radians) */
	store->sbm_sweep_stabilizedangle = 0.0;     /* sweep segment stabilized angle (radians) */

	/* comment */
	for (int i = 0; i < MBSYS_XSE_COMMENT_LENGTH; i++)
		store->comment[i] = 0;

	/* unsupported frame */
	store->rawsize = 0;
	for (int i = 0; i < MBSYS_XSE_COMMENT_LENGTH; i++)
		store->raw[i] = 0;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)*store_ptr);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_xse_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error) {

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)*store_ptr);
	}

	/* deallocate memory for data structure */
	const int status = mb_freed(verbose, __FILE__, __LINE__, (void **)store_ptr, error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_xse_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_xse_struct *store = (struct mbsys_xse_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get beam and pixel numbers */
		*nbath = 0;
		*namp = 0;
		*nss = 0;
		if (store->mul_frame) {
			*nbath = store->mul_num_beams;
			if (store->mul_group_amp)
				*namp = store->mul_num_beams;
		}

		if (store->sid_frame) {
			if (store->sid_group_avl)
				*nss = store->sid_avl_num_samples;
		}
	}

	/* extract data from structure */
	else {
		/* get beam and pixel numbers */
		*nbath = 0;
		*namp = 0;
		*nss = 0;
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
		fprintf(stderr, "dbg2       nbath:      %d\n", *nbath);
		fprintf(stderr, "dbg2        namp:      %d\n", *namp);
		fprintf(stderr, "dbg2        nss:       %d\n", *nss);
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_xse_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
                      double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss, char *beamflag,
                      double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss,
                      double *ssacrosstrack, double *ssalongtrack, char *comment, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_xse_struct *store = (struct mbsys_xse_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get time */
		*time_d = store->mul_sec - MBSYS_XSE_TIME_OFFSET + 0.000001 * store->mul_usec;
		mb_get_date(verbose, *time_d, time_i);

		/* get navigation */
		*navlon = RTD * store->mul_lon;
		*navlat = RTD * store->mul_lat;

		/* get heading and speed */
		*heading = RTD * store->mul_heading;

		/* get speed  */
		*speed = 3.6 * store->mul_speed;

		/* set beamwidths in mb_io structure */
		if (store->mul_frequency >= 50.0 || store->mul_frequency <= 0.0) {
			mb_io_ptr->beamwidth_ltrack = 2.8;
			mb_io_ptr->beamwidth_xtrack = 1.5;
		}
		else {
			mb_io_ptr->beamwidth_ltrack = 1.0;
			mb_io_ptr->beamwidth_xtrack = 1.0;
		}

		double dsign;
		/* get distance and depth values */
		*nbath = 0;
		*namp = 0;
		*nss = 0;
		if (store->mul_frame) {
			/* set number of beams */
			*nbath = store->mul_num_beams;
			if (store->mul_group_amp)
				*namp = store->mul_num_beams;

			/* determine whether beams are ordered
			port to starboard or starboard to port */
			double xtrackmin = 0.0;
			double xtrackmax = 0.0;
			int ixtrackmin = 0;
			int ixtrackmax = 0;
			for (int i = 0; i < store->mul_num_beams; i++) {
				if (store->beams[i].lateral < xtrackmin) {
					xtrackmin = store->beams[i].lateral;
					ixtrackmin = i;
				}
				if (store->beams[i].lateral > xtrackmax) {
					xtrackmax = store->beams[i].lateral;
					ixtrackmax = i;
				}
			}
			if (ixtrackmax > ixtrackmin)
				dsign = -1.0;
			else
				dsign = 1.0;

			/* now extract the bathymetry */
			for (int i = 0; i < store->mul_num_beams; i++) {
				const int j = store->mul_num_beams - store->beams[i].beam;
				if (store->beams[i].quality == 1)
					beamflag[j] = MB_FLAG_NONE;
				else if (store->beams[i].quality < 8)
					beamflag[j] = (char)(MB_FLAG_SONAR | MB_FLAG_FLAG);
				else if (store->beams[i].quality == 8)
					beamflag[j] = MB_FLAG_NULL;
				else if (store->beams[i].quality == 10)
					beamflag[j] = (char)(MB_FLAG_MANUAL | MB_FLAG_FLAG);
				else if (store->beams[i].quality == 20)
					beamflag[j] = (char)(MB_FLAG_FILTER | MB_FLAG_FLAG);
				else
					beamflag[j] = MB_FLAG_NULL;

				/* bathymetry from SeaBeam 2100 multibeams is already heave and draft compensated
				    - bathymetry from Bottomchart multibeams need to have
				    heave and draft applied */
				bath[j] = store->beams[i].depth;
				if (store->par_ship_nsensor > 0 &&
				    (store->par_ship_sensor_type[0] < 2000 || store->par_ship_sensor_type[0] > 3000)) {
					bath[j] += store->beams[i].heave;
					if (store->beams[i].lateral < 0.0)
						bath[j] += store->par_trans_z_port;
					else
						bath[j] += store->par_trans_z_stbd;
				}
				bathacrosstrack[j] = dsign * store->beams[i].lateral;
				bathalongtrack[j] = store->beams[i].along;
				amp[j] = store->beams[i].amplitude;
			}
		}

		/* get sidescan */
		if (store->sid_frame) {
			if (store->sid_group_avl) {
				*nss = store->sid_avl_num_samples;
				for (int i = 0; i < *nss; i++) {
					const int j = *nss - i - 1;
					ss[j] = store->sid_avl_amp[i];
					// TODO(schwehr): dsign might be uninitialized.
					ssacrosstrack[j] = dsign * 0.001 * store->sid_avl_binsize * (i - *nss / 2);
					if (store->mul_frame)
						ssalongtrack[j] =
						    0.5 * store->nav_speed_ground *
						    (store->sid_sec + 0.000001 * store->sid_usec - (store->mul_sec + 0.000001 * store->mul_usec));
					else
						ssalongtrack[j] = 0.0;
				}
			}
		}

		/* done translating values */
	}

	/* extract data from structure */
	else if (*kind == MB_DATA_NAV) {
		/* get time */
		*time_d = store->nav_sec - MBSYS_XSE_TIME_OFFSET + 0.000001 * store->nav_usec;
		mb_get_date(verbose, *time_d, time_i);

		/* get navigation */
		*navlon = RTD * store->nav_x;
		*navlat = RTD * store->nav_y;

		/* get heading */
		if (store->nav_group_heading)
			*heading = RTD * store->nav_hdg_heading;
		else if (store->nav_group_motiongt)
			*heading = RTD * store->nav_course_ground;
		else if (store->nav_group_motiontw)
			*heading = RTD * store->nav_course_water;
		else
			mb_hedint_interp(verbose, mbio_ptr, *time_d, heading, error);

		/* get speed  */
		if (store->nav_group_log)
			*speed = 3.6 * store->nav_log_speed;
		else if (store->nav_group_motiongt)
			*speed = 3.6 * store->nav_speed_ground;
		else if (store->nav_group_motiontw)
			*speed = 3.6 * store->nav_speed_water;

		/* get distance and depth values */
		*nbath = 0;
		*namp = 0;
		*nss = 0;

		/* done translating values */
	}

	/* extract comment from structure */
	else if (*kind == MB_DATA_COMMENT) {
		/* copy comment */
    memset((void *)comment, 0, MB_COMMENT_MAXLINE);
    strncpy(comment, store->comment, MIN(MB_COMMENT_MAXLINE, MBSYS_XSE_COMMENT_LENGTH) - 1);

		if (verbose >= 4) {
			fprintf(stderr, "\ndbg4  New ping read by MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  New ping values:\n");
			fprintf(stderr, "dbg4       error:      %d\n", *error);
			fprintf(stderr, "dbg4       comment:    %s\n", comment);
		}
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
	}
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR && *kind == MB_DATA_COMMENT) {
		fprintf(stderr, "dbg2       comment:     \ndbg2       %s\n", comment);
	}
	else if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR && *kind != MB_DATA_COMMENT) {
		fprintf(stderr, "dbg2       time_i[0]:     %d\n", time_i[0]);
		fprintf(stderr, "dbg2       time_i[1]:     %d\n", time_i[1]);
		fprintf(stderr, "dbg2       time_i[2]:     %d\n", time_i[2]);
		fprintf(stderr, "dbg2       time_i[3]:     %d\n", time_i[3]);
		fprintf(stderr, "dbg2       time_i[4]:     %d\n", time_i[4]);
		fprintf(stderr, "dbg2       time_i[5]:     %d\n", time_i[5]);
		fprintf(stderr, "dbg2       time_i[6]:     %d\n", time_i[6]);
		fprintf(stderr, "dbg2       time_d:        %f\n", *time_d);
		fprintf(stderr, "dbg2       longitude:     %f\n", *navlon);
		fprintf(stderr, "dbg2       latitude:      %f\n", *navlat);
		fprintf(stderr, "dbg2       speed:         %f\n", *speed);
		fprintf(stderr, "dbg2       heading:       %f\n", *heading);
	}
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR && *kind == MB_DATA_DATA) {
		fprintf(stderr, "dbg2       nbath:      %d\n", *nbath);
		for (int i = 0; i < *nbath; i++)
			fprintf(stderr, "dbg2       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n", i, beamflag[i], bath[i],
			        bathacrosstrack[i], bathalongtrack[i]);
		fprintf(stderr, "dbg2        namp:     %d\n", *namp);
		for (int i = 0; i < *namp; i++)
			fprintf(stderr, "dbg2       beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n", i, amp[i], bathacrosstrack[i],
			        bathalongtrack[i]);
		fprintf(stderr, "dbg2        nss:      %d\n", *nss);
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_xse_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d, double navlon,
                     double navlat, double speed, double heading, int nbath, int namp, int nss, char *beamflag, double *bath,
                     double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss, double *ssacrosstrack,
                     double *ssalongtrack, char *comment, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       kind:       %d\n", kind);
	}
	if (verbose >= 2 && (kind == MB_DATA_DATA || kind == MB_DATA_NAV)) {
		fprintf(stderr, "dbg2       time_i[0]:  %d\n", time_i[0]);
		fprintf(stderr, "dbg2       time_i[1]:  %d\n", time_i[1]);
		fprintf(stderr, "dbg2       time_i[2]:  %d\n", time_i[2]);
		fprintf(stderr, "dbg2       time_i[3]:  %d\n", time_i[3]);
		fprintf(stderr, "dbg2       time_i[4]:  %d\n", time_i[4]);
		fprintf(stderr, "dbg2       time_i[5]:  %d\n", time_i[5]);
		fprintf(stderr, "dbg2       time_i[6]:  %d\n", time_i[6]);
		fprintf(stderr, "dbg2       time_d:     %f\n", time_d);
		fprintf(stderr, "dbg2       navlon:     %f\n", navlon);
		fprintf(stderr, "dbg2       navlat:     %f\n", navlat);
		fprintf(stderr, "dbg2       speed:      %f\n", speed);
		fprintf(stderr, "dbg2       heading:    %f\n", heading);
	}
	if (verbose >= 2 && kind == MB_DATA_DATA) {
		fprintf(stderr, "dbg2       nbath:      %d\n", nbath);
		if (verbose >= 3)
			for (int i = 0; i < nbath; i++)
				fprintf(stderr, "dbg3       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n", i, beamflag[i], bath[i],
				        bathacrosstrack[i], bathalongtrack[i]);
		fprintf(stderr, "dbg2       namp:       %d\n", namp);
		if (verbose >= 3)
			for (int i = 0; i < namp; i++)
				fprintf(stderr, "dbg3        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n", i, amp[i], bathacrosstrack[i],
				        bathalongtrack[i]);
		fprintf(stderr, "dbg4       nss:        %d\n", nss);
		for (int i = 0; i < nss; i++)
			fprintf(stderr, "dbg4        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n", i, ss[i], ssacrosstrack[i],
			        ssalongtrack[i]);
	}
	if (verbose >= 2 && kind == MB_DATA_COMMENT) {
		fprintf(stderr, "dbg2       comment:     \ndbg2       %s\n", comment);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_xse_struct *store = (struct mbsys_xse_struct *)store_ptr;

	/* set data kind */
	store->kind = kind;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA) {
		/* get time */
		store->mul_sec = (unsigned int)(time_d + MBSYS_XSE_TIME_OFFSET);
		store->mul_usec = (time_d - ((int)time_d)) * 1000000;
		store->sid_sec = store->mul_sec;
		store->sid_usec = store->mul_usec;

		/*get navigation */
		store->mul_lon = DTR * navlon;
		store->mul_lat = DTR * navlat;

		/* get heading */
		store->mul_heading = DTR * heading;

		/* get speed */
		store->mul_speed = speed / 3.6;

		/* insert distance and depth values into storage arrays */
		if (store->mul_frame) {
			double xtrackmin = 0.0;
			double xtrackmax = 0.0;
			int ixtrackmin = 0;
			int ixtrackmax = 0;
			/* determine whether beams are ordered
			port to starboard or starboard to port */
			for (int i = 0; i < store->mul_num_beams; i++) {
				if (store->beams[i].lateral < xtrackmin) {
					xtrackmin = store->beams[i].lateral;
					ixtrackmin = i;
				}
				if (store->beams[i].lateral > xtrackmax) {
					xtrackmax = store->beams[i].lateral;
					ixtrackmax = i;
				}
			}

			const double dsign = ixtrackmax > ixtrackmin ? -1.0 : 1.0;

			/* now insert the bathymetry */
			for (int i = 0; i < store->mul_num_beams; i++) {
				const int j = store->mul_num_beams - store->beams[i].beam;
				if (j < nbath) {
					if (mb_beam_check_flag(beamflag[j])) {
						if (mb_beam_check_flag_null(beamflag[j]))
							store->beams[i].quality = 8;
						else if (mb_beam_check_flag_manual(beamflag[j]))
							store->beams[i].quality = 10;
						else if (mb_beam_check_flag_filter(beamflag[j]))
							store->beams[i].quality = 20;
						else if (store->beams[i].quality == 1)
							store->beams[i].quality = 7;
					}
					else
						store->beams[i].quality = 1;
					store->beams[i].lateral = dsign * bathacrosstrack[j];
					store->beams[i].along = bathalongtrack[j];
					store->beams[i].amplitude = (int)(amp[j]);

					/* bathymetry from SeaBeam 2100 multibeams stored heave and draft compensated
					    - bathymetry from Bottomchart multibeams need to have
					    heave and draft removed before storage */
					store->beams[i].depth = bath[j];
					if (store->par_ship_nsensor > 0 &&
					    (store->par_ship_sensor_type[0] < 2000 || store->par_ship_sensor_type[0] > 3000)) {
						if (store->beams[i].lateral < 0.0)
							store->beams[i].depth -= (store->beams[i].heave + store->par_trans_z_port);
						else
							store->beams[i].depth -= (store->beams[i].heave + store->par_trans_z_stbd);
					}
				}
			}
		}

		/* now insert the sidescan */
		if (store->sid_frame) {
			store->sid_group_avl = true;
			if (nss != store->sid_avl_num_samples) {
				store->sid_avl_num_samples = nss;
				double maxoffset = 0.0;
				double imaxoffset = -1;
				for (int i = 0; i < nss; i++) {
					if (fabs(ssacrosstrack[i]) > maxoffset) {
						maxoffset = fabs(ssacrosstrack[i]);
						imaxoffset = i - (nss / 2);
					}
				}
				if (maxoffset > 0.0 && imaxoffset != 0)
					store->sid_avl_binsize = (int)(1000 * maxoffset / imaxoffset);
			}
			for (int i = 0; i < store->sid_avl_num_samples; i++) {
				const int j = store->sid_avl_num_samples - i - 1;
				if (j < nss) {
					store->sid_avl_amp[i] = ss[j];
				}
			}
		}
	}
	else if (store->kind == MB_DATA_NAV) {
		/* get time */
		store->nav_sec = (unsigned int)(time_d + MBSYS_XSE_TIME_OFFSET);
		store->nav_usec = (time_d - ((int)time_d)) * 1000000;

		/*get navigation */
		store->nav_group_position = true;
		store->nav_x = DTR * navlon;
		store->nav_y = DTR * navlat;

		/* get heading */
		store->nav_group_heading = true;
		store->nav_hdg_heading = DTR * heading;

		/* get speed */
		store->nav_group_log = true;
		store->nav_log_speed = speed / 3.6;
	}

	/* insert comment in structure */
	else if (store->kind == MB_DATA_COMMENT) {
    memset((void *)store->comment, 0, MBSYS_XSE_COMMENT_LENGTH);
    strncpy(store->comment, comment, MIN(MBSYS_XSE_COMMENT_LENGTH, MB_COMMENT_MAXLINE) - 1);
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_xse_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes, double *angles,
                     double *angles_forward, double *angles_null, double *heave, double *alongtrack_offset, double *draft,
                     double *ssv, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       ttimes:     %p\n", (void *)ttimes);
		fprintf(stderr, "dbg2       angles_xtrk:%p\n", (void *)angles);
		fprintf(stderr, "dbg2       angles_ltrk:%p\n", (void *)angles_forward);
		fprintf(stderr, "dbg2       angles_null:%p\n", (void *)angles_null);
		fprintf(stderr, "dbg2       heave:      %p\n", (void *)heave);
		fprintf(stderr, "dbg2       ltrk_off:   %p\n", (void *)alongtrack_offset);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_xse_struct *store = (struct mbsys_xse_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get draft */
		if (store->par_parameter)
			*draft = 0.5 * (store->par_trans_z_port + store->par_trans_z_stbd);
		else
			*draft = store->par_ship_draft;

		/* get ssv */
		if (store->sbm_ssv > 0.0)
			*ssv = store->sbm_ssv;
		else
			*ssv = store->svp_ssv;

		/* get travel times, angles */
		*nbeams = 0;
		if (store->mul_frame) {
			/* determine whether beams are ordered
			port to starboard or starboard to port */
			double xtrackmin = 0.0;
			double xtrackmax = 0.0;
			int ixtrackmin = 0;
			int ixtrackmax = 0;
			for (int i = 0; i < store->mul_num_beams; i++) {
				if (store->beams[i].lateral < xtrackmin) {
					xtrackmin = store->beams[i].lateral;
					ixtrackmin = i;
				}
				if (store->beams[i].lateral > xtrackmax) {
					xtrackmax = store->beams[i].lateral;
					ixtrackmax = i;
				}
			}

			const double dsign = ixtrackmax > ixtrackmin ? -1.0 : 1.0;

			/* loop over beams */
			for (int i = 0; i < store->mul_num_beams; i++) {
				const int j = store->mul_num_beams - store->beams[i].beam;
				*nbeams = MAX(store->beams[i].beam, *nbeams);
				ttimes[j] = store->beams[i].tt;
				const double beta = 90.0 - dsign * RTD * store->beams[i].angle;
				const double alpha = RTD * store->beams[i].pitch;
				mb_rollpitch_to_takeoff(verbose, alpha, beta, &angles[j], &angles_forward[j], error);
				if (store->mul_frequency >= 50000.0 || store->mul_frequency <= 0.0) {
					if (store->beams[j].angle < 0.0) {
						angles_null[j] = 37.5 + RTD * store->par_trans_err_port;
					}
					else {
						angles_null[j] = 37.5 + RTD * store->par_trans_err_stbd;
					}
				}
				else {
					angles_null[j] = 0.0;
				}
				heave[j] = store->beams[i].heave;
				alongtrack_offset[j] = 0.5 * store->nav_speed_ground * store->beams[i].delay;
			}
		}

		/* set status */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;

		/* done translating values */
	}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT) {
		/* set status */
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
	}

	/* deal with other record type */
	else {
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
	}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR) {
		fprintf(stderr, "dbg2       draft:      %f\n", *draft);
		fprintf(stderr, "dbg2       ssv:        %f\n", *ssv);
		fprintf(stderr, "dbg2       nbeams:     %d\n", *nbeams);
		for (int i = 0; i < *nbeams; i++)
			fprintf(stderr, "dbg2       beam %d: tt:%f  angle_xtrk:%f  angle_ltrk:%f  angle_null:%f  heave:%f  ltrk_off:%f\n", i,
			        ttimes[i], angles[i], angles_forward[i], angles_null[i], heave[i], alongtrack_offset[i]);
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_xse_detects(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *detects, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       detects:    %p\n", (void *)detects);
	}

	/* get mbio descriptor */
	struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_xse_struct *store = (struct mbsys_xse_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get nbeams */
		*nbeams = mb_io_ptr->beams_bath_max;

		/* get detects */
		for (int i = 0; i < *nbeams; i++) {
			detects[i] = MB_DETECT_AMPLITUDE;
		}

		/* get nbeams and detects */
		*nbeams = 0;
		if (store->mul_frame) {
			/* loop over beams to get nbeams */
			for (int i = 0; i < store->mul_num_beams; i++) {
				// const int j = store->mul_num_beams - store->beams[i].beam;
				*nbeams = MAX(store->beams[i].beam, *nbeams);
			}

			/* loop over beams to set detects */
			for (int i = 0; i < *nbeams; i++) {
				detects[i] = MB_DETECT_UNKNOWN;
			}
		}

		/* set status */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;

		/* done translating values */
	}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT) {
		/* set status */
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
	}

	/* deal with other record type */
	else {
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
	}
	if (verbose >= 2 && *error == MB_ERROR_NO_ERROR) {
		fprintf(stderr, "dbg2       nbeams:     %d\n", *nbeams);
		for (int i = 0; i < *nbeams; i++)
			fprintf(stderr, "dbg2       beam %d: detects:%d\n", i, detects[i]);
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_xse_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth,
                               double *altitude, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_xse_struct *store = (struct mbsys_xse_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get draft */
		if (store->par_parameter)
			*transducer_depth = 0.5 * (store->par_trans_z_port + store->par_trans_z_stbd);
		else
			*transducer_depth = store->par_ship_draft;

		double bath_best = 0.0;
		if (store->mul_num_beams > 0) {
			*transducer_depth -= store->beams[store->mul_num_beams / 2].heave;
			if (store->beams[store->mul_num_beams / 2].quality == 1)
				bath_best = store->beams[store->mul_num_beams / 2].depth;
			else {
				double xtrack_min = 99999999.9;
				for (int i = 0; i < store->mul_num_beams; i++) {
					if (store->beams[i].quality == 1 && fabs(store->beams[i].lateral) < xtrack_min) {
						xtrack_min = fabs(store->beams[i].lateral);
						bath_best = store->beams[i].depth;
					}
				}
			}
			if (bath_best <= 0.0) {
				double xtrack_min = 99999999.9;
				for (int i = 0; i < store->mul_num_beams; i++) {
					if (store->beams[i].quality < 8 && fabs(store->beams[i].lateral) < xtrack_min) {
						xtrack_min = fabs(store->beams[i].lateral);
						bath_best = store->beams[i].depth;
					}
				}
			}
		}
		*altitude = bath_best - *transducer_depth;

		/* set status */
		*error = MB_ERROR_NO_ERROR;
		status = MB_SUCCESS;

		/* done translating values */
	}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT) {
		/* set status */
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
	}

	/* deal with other record type */
	else {
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:              %d\n", *kind);
		fprintf(stderr, "dbg2       transducer_depth:  %f\n", *transducer_depth);
		fprintf(stderr, "dbg2       altitude:          %f\n", *altitude);
		fprintf(stderr, "dbg2       error:             %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:            %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_xse_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
                          double *navlat, double *speed, double *heading, double *draft, double *roll, double *pitch,
                          double *heave, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_xse_struct *store = (struct mbsys_xse_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_DATA) {
		/* get time */
		*time_d = store->mul_sec - MBSYS_XSE_TIME_OFFSET + 0.000001 * store->mul_usec;
		mb_get_date(verbose, *time_d, time_i);

		/* get navigation */
		*navlon = RTD * store->mul_lon;
		*navlat = RTD * store->mul_lat;

		/* get heading */
		*heading = RTD * store->mul_heading;

		/* get speed  */
		*speed = 3.6 * store->mul_speed;

		/* get draft */
		if (store->par_parameter)
			*draft = 0.5 * (store->par_trans_z_port + store->par_trans_z_stbd);
		else
			*draft = store->par_ship_draft;

		/* get roll pitch and heave */
		if (store->mul_num_beams > 0) {
			*roll = RTD * store->beams[store->mul_num_beams / 2].roll;
			*pitch = RTD * store->beams[store->mul_num_beams / 2].pitch;
			*heave = store->beams[store->mul_num_beams / 2].heave;
		}
		else if (store->nav_group_hrp) {
			*roll = RTD * store->nav_hrp_roll;
			*pitch = RTD * store->nav_hrp_pitch;
			*heave = store->nav_hrp_heave;
		}
		else if (store->nav_group_heave && store->nav_group_roll && store->nav_group_pitch) {
			*roll = RTD * store->nav_rol_roll;
			*pitch = RTD * store->nav_pit_pitch;
			*heave = store->nav_hea_heave;
		}
		else {
			*roll = 0.0;
			*pitch = 0.0;
			*heave = 0.0;
		}

		/* done translating values */
	}

	/* extract data from structure */
	else if (*kind == MB_DATA_NAV) {
		/* get time */
		*time_d = store->nav_sec - MBSYS_XSE_TIME_OFFSET + 0.000001 * store->nav_usec;
		mb_get_date(verbose, *time_d, time_i);

		/* get heading */
		if (store->nav_group_heading)
			*heading = RTD * store->nav_hdg_heading;
		else if (store->nav_group_motiongt)
			*heading = RTD * store->nav_course_ground;
		else if (store->nav_group_motiontw)
			*heading = RTD * store->nav_course_water;
		else
			mb_hedint_interp(verbose, mbio_ptr, *time_d, heading, error);

		/* get speed if possible */
		if (store->nav_group_log)
			*speed = 3.6 * store->nav_log_speed;
		else if (store->nav_group_motiongt)
			*speed = 3.6 * store->nav_speed_ground;
		else if (store->nav_group_motiontw)
			*speed = 3.6 * store->nav_speed_water;
		else
			*speed = 0.0;

		/* get navigation */
		if (store->nav_group_position) {
			*navlon = RTD * store->nav_x;
			*navlat = RTD * store->nav_y;
		}
		else
			mb_navint_interp(verbose, mbio_ptr, *time_d, *heading, *speed, navlon, navlat, speed, error);

		/* get draft */
		if (store->par_parameter)
			*draft = 0.5 * (store->par_trans_z_port + store->par_trans_z_stbd);
		else
			*draft = store->par_ship_draft;

		/* get roll pitch and heave */
		if (store->nav_group_hrp) {
			*roll = RTD * store->nav_hrp_roll;
			*pitch = RTD * store->nav_hrp_pitch;
			*heave = store->nav_hrp_heave;
		}
		else if (store->nav_group_heave && store->nav_group_roll && store->nav_group_pitch) {
			*roll = RTD * store->nav_rol_roll;
			*pitch = RTD * store->nav_pit_pitch;
			*heave = store->nav_hea_heave;
		}
		else if (store->mul_num_beams > 0) {
			*roll = RTD * store->beams[store->mul_num_beams / 2].roll;
			*pitch = RTD * store->beams[store->mul_num_beams / 2].pitch;
			*heave = store->beams[store->mul_num_beams / 2].heave;
		}
		else {
			*roll = 0.0;
			*pitch = 0.0;
			*heave = 0.0;
		}

		/* done translating values */
	}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT) {
		/* set status */
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
	}

	/* deal with other record type */
	else {
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:       %d\n", *kind);
	}
	if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR && *kind == MB_DATA_DATA) {
		fprintf(stderr, "dbg2       time_i[0]:     %d\n", time_i[0]);
		fprintf(stderr, "dbg2       time_i[1]:     %d\n", time_i[1]);
		fprintf(stderr, "dbg2       time_i[2]:     %d\n", time_i[2]);
		fprintf(stderr, "dbg2       time_i[3]:     %d\n", time_i[3]);
		fprintf(stderr, "dbg2       time_i[4]:     %d\n", time_i[4]);
		fprintf(stderr, "dbg2       time_i[5]:     %d\n", time_i[5]);
		fprintf(stderr, "dbg2       time_i[6]:     %d\n", time_i[6]);
		fprintf(stderr, "dbg2       time_d:        %f\n", *time_d);
		fprintf(stderr, "dbg2       longitude:     %f\n", *navlon);
		fprintf(stderr, "dbg2       latitude:      %f\n", *navlat);
		fprintf(stderr, "dbg2       speed:         %f\n", *speed);
		fprintf(stderr, "dbg2       heading:       %f\n", *heading);
		fprintf(stderr, "dbg2       roll:          %f\n", *roll);
		fprintf(stderr, "dbg2       pitch:         %f\n", *pitch);
		fprintf(stderr, "dbg2       heave:         %f\n", *heave);
	}
	if (verbose >= 2) {
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_xse_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon, double navlat,
                         double speed, double heading, double draft, double roll, double pitch, double heave, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       time_i[0]:  %d\n", time_i[0]);
		fprintf(stderr, "dbg2       time_i[1]:  %d\n", time_i[1]);
		fprintf(stderr, "dbg2       time_i[2]:  %d\n", time_i[2]);
		fprintf(stderr, "dbg2       time_i[3]:  %d\n", time_i[3]);
		fprintf(stderr, "dbg2       time_i[4]:  %d\n", time_i[4]);
		fprintf(stderr, "dbg2       time_i[5]:  %d\n", time_i[5]);
		fprintf(stderr, "dbg2       time_i[6]:  %d\n", time_i[6]);
		fprintf(stderr, "dbg2       time_d:     %f\n", time_d);
		fprintf(stderr, "dbg2       navlon:     %f\n", navlon);
		fprintf(stderr, "dbg2       navlat:     %f\n", navlat);
		fprintf(stderr, "dbg2       speed:      %f\n", speed);
		fprintf(stderr, "dbg2       heading:    %f\n", heading);
		fprintf(stderr, "dbg2       roll:       %f\n", roll);
		fprintf(stderr, "dbg2       pitch:      %f\n", pitch);
		fprintf(stderr, "dbg2       heave:      %f\n", heave);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_xse_struct *store = (struct mbsys_xse_struct *)store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_DATA) {
		/* get time */
		store->mul_sec = (unsigned int)(time_d + MBSYS_XSE_TIME_OFFSET);
		store->mul_usec = (time_d - ((int)time_d)) * 1000000;
		store->sid_sec = store->mul_sec;
		store->sid_usec = store->mul_usec;

		/*get navigation */
		store->mul_lon = DTR * navlon;
		store->mul_lat = DTR * navlat;

		/* get heading */
		store->mul_heading = DTR * heading;

		/* get speed */
		store->mul_speed = speed / 3.6;

		/* get draft */
		if (store->par_parameter) {
			store->par_trans_z_port = draft;
			store->par_trans_z_stbd = draft;
		}
		else {
			store->par_ship_draft = draft;
		}

		/* get roll pitch and heave */
	}

	/* insert data in structure */
	else if (store->kind == MB_DATA_NAV) {
		/* get time */
		store->nav_sec = (unsigned int)(time_d + MBSYS_XSE_TIME_OFFSET);
		store->nav_usec = (time_d - ((int)time_d)) * 1000000;

		/*get navigation */
		store->nav_group_position = true;
		store->nav_x = DTR * navlon;
		store->nav_y = DTR * navlat;

		/* get heading */
		store->nav_group_heading = true;
		store->nav_hdg_heading = DTR * heading;

		/* get speed */
		store->nav_group_log = true;
		store->nav_log_speed = speed / 3.6;

		/* get draft */
		if (store->par_parameter) {
			store->par_trans_z_port = draft;
			store->par_trans_z_stbd = draft;
		}
		else {
			store->par_ship_draft = draft;
		}

		/* get roll pitch and heave */
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_xse_extract_svp(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nsvp, double *depth, double *velocity,
                          int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_xse_struct *store = (struct mbsys_xse_struct *)store_ptr;

	/* get data kind */
	*kind = store->kind;

	int status = MB_SUCCESS;

	/* extract data from structure */
	if (*kind == MB_DATA_VELOCITY_PROFILE) {
		/* get number of depth-velocity pairs */
		*nsvp = store->svp_nsvp;

		/* get profile */
		for (int i = 0; i < *nsvp; i++) {
			depth[i] = store->svp_depth[i];
			velocity[i] = store->svp_velocity[i];
		}

		/* done translating values */
	}

	/* deal with comment */
	else if (*kind == MB_DATA_COMMENT) {
		/* set status */
		*error = MB_ERROR_COMMENT;
		status = MB_FAILURE;
	}

	/* deal with other record type */
	else {
		/* set status */
		*error = MB_ERROR_OTHER;
		status = MB_FAILURE;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       kind:              %d\n", *kind);
		fprintf(stderr, "dbg2       nsvp:              %d\n", *nsvp);
		for (int i = 0; i < *nsvp; i++)
			fprintf(stderr, "dbg2       depth[%d]: %f   velocity[%d]: %f\n", i, depth[i], i, velocity[i]);
		fprintf(stderr, "dbg2       error:             %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:            %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_xse_insert_svp(int verbose, void *mbio_ptr, void *store_ptr, int nsvp, double *depth, double *velocity, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       nsvp:       %d\n", nsvp);
		for (int i = 0; i < nsvp; i++)
			fprintf(stderr, "dbg2       depth[%d]: %f   velocity[%d]: %f\n", i, depth[i], i, velocity[i]);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointer */
	struct mbsys_xse_struct *store = (struct mbsys_xse_struct *)store_ptr;

	/* insert data in structure */
	if (store->kind == MB_DATA_VELOCITY_PROFILE) {
		/* get number of depth-velocity pairs */
		store->svp_nsvp = MIN(nsvp, MBSYS_XSE_MAXSVP);

		/* get profile */
		for (int i = 0; i < store->svp_nsvp; i++) {
			store->svp_depth[i] = depth[i];
			store->svp_velocity[i] = velocity[i];
		}
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return value:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_xse_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
		fprintf(stderr, "dbg2       copy_ptr:   %p\n", (void *)copy_ptr);
	}

	/* get mbio descriptor */
	// struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* get data structure pointers */
	struct mbsys_xse_struct *store = (struct mbsys_xse_struct *)store_ptr;
	struct mbsys_xse_struct *copy = (struct mbsys_xse_struct *)copy_ptr;

	/* copy the data */
	*copy = *store;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:     %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
