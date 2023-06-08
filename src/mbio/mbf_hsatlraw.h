/*--------------------------------------------------------------------
 *    The MB-system:	mbf_hsatlraw.h	1/20/93
 *
 *    Copyright (c) 1993-2020 by
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
 * mbf_hsatlraw.h defines the data structures used by MBIO functions
 * to store multibeam data read from the MBF_HSATLRAW format (MBIO id 21).
 *
 * Author:	D. W. Caress
 * Date:	January 20, 1993
 *
 */
/*
 * Notes on the MBF_HSATLRAW data format:
 *   1. Hydrosweep DS multibeam systems output raw data in this
 *      ascii format.  The data consists of a number of different
 *      multi-line ascii records.
 *   2. The DS systems output 59 beams of bathymetry and 59 beams
 *      of backscatter measurements, along with a plethora of other
 *      information.
 *   3. The records all include navigation and time stamp information.
 *      The record types are:
 *        ERGNHYDI:  mean and keel water velocity values
 *        ERGNPARA:  navigation when system in standby
 *        ERGNPOSI:  navigation source
 *        ERGNMESS:  across-track "survey" bathymetry
 *        ERGNEICH:  along-track "calibration" bathymetry
 *        ERGNLSZT:  travel times associated with
 *                   ERGNMESS or ERGNEICH records
 *        ERGNCTDS:  water sound velocity profile
 *        ERGNAMPL:  amplitudes associated with
 *                   ERGNMESS or ERGNEICH records
 *        LDEOCOMM:  comment records (an L-DEO extension)
 *   4. A single ping usually results in the following series of
 *      of records:
 *        1. ERGNMESS or ERGNEICH
 *        2. ERGNSLZT
 *        3. ERGNAMPL
 *      The ERGNHYDI, ERGNPARA, ERGNPOSI and ERGNCTDS records occur
 *      at system startup and when the associated operational
 *      parameters of the Hydrosweep are changed.
 *   5. The kind value in the mbf_hsatlraw_struct indicates whether the
 *      mbf_hsatlraw_data_struct structure holds data from a ping or
 *      data from some other record:
 *        kind = 1 : data from a survey ping
 *                   (ERGNMESS + ERGNSLZT + ERGNAMPL)
 *        kind = 2 : comment (LDEOCOMM)
 *        kind = 3 : data from a calibrate ping
 *                   (ERGNEICH + ERGNSLZT + ERGNAMPL)
 *        kind = 4 : mean and keel velocity (ERGNHYDI)
 *        kind = 5 : water velocity profile (ERGNCTDS)
 *        kind = 6 : standby navigation (ERGNPARA)
 *        kind = 7 : navigation source (ERGNPOSI)
 *   6. The data structure defined below includes all of the values
 *      which are passed in Hydrosweep records.
 */

#ifndef MBF_HSATLRAW_H_
#define MBF_HSATLRAW_H_

#include <stdio.h>

/* maximum number of depth-velocity pairs */
#define MBF_HSATLRAW_MAXVEL 30

/* maximum line length in characters */
#define MBF_HSATLRAW_MAXLINE 200

/* number of beams for hydrosweep */
#define MBF_HSATLRAW_BEAMS 59

/* define id's for the different types of raw Hydrosweep records */
#define MBF_HSATLRAW_RECORDS 11
#define MBF_HSATLRAW_NONE 0
#define MBF_HSATLRAW_RAW_LINE 1
#define MBF_HSATLRAW_ERGNHYDI 2
#define MBF_HSATLRAW_ERGNPARA 3
#define MBF_HSATLRAW_ERGNPOSI 4
#define MBF_HSATLRAW_ERGNEICH 5
#define MBF_HSATLRAW_ERGNMESS 6
#define MBF_HSATLRAW_ERGNSLZT 7
#define MBF_HSATLRAW_ERGNCTDS 8
#define MBF_HSATLRAW_ERGNAMPL 9
#define MBF_HSATLRAW_LDEOCMNT 10
char *mbf_hsatlraw_labels[] = {"NONE    ", "RAW_LINE", "ERGNHYDI", "ERGNPARA", "ERGNPOSI", "ERGNEICH",
                               "ERGNMESS", "ERGNSLZT", "ERGNCTDS", "ERGNAMPL", "LDEOCMNT"};

struct mbf_hsatlraw_struct {
	/* type of data record */
	int kind;

	/* position (all records ) */
	double lon;
	double lat;

	/* time stamp (all records ) */
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
	int alt_minute;
	int alt_second;

	/* additional navigation and depths (ERGNMESS and ERGNEICH) */
	double course_true;
	double speed_transverse;
	double speed;
	char speed_reference[2];
	double pitch;
	int track;
	double depth_center;
	double depth_scale;
	int spare;
	int distance[MBF_HSATLRAW_BEAMS];
	int depth[MBF_HSATLRAW_BEAMS];

	/* travel time data (ERGNSLZT) */
	double course_ground;
	double speed_ground;
	double heave;
	double roll;
	double time_center;
	double time_scale;
	int time[MBF_HSATLRAW_BEAMS];
	double gyro[11];

	/* amplitude data (ERGNAMPL) */
	char mode[2];
	int trans_strbd;
	int trans_vert;
	int trans_port;
	int pulse_len_strbd;
	int pulse_len_vert;
	int pulse_len_port;
	int gain_start;
	int r_compensation_factor;
	int compensation_start;
	int increase_start;
	int tvc_near;
	int tvc_far;
	int increase_int_near;
	int increase_int_far;
	int gain_center;
	double filter_gain;
	int amplitude_center;
	int echo_duration_center;
	int echo_scale_center;
	int gain[16];
	int amplitude[MBF_HSATLRAW_BEAMS];
	int echo_scale[16];
	int echo_duration[MBF_HSATLRAW_BEAMS];

	/* mean velocity (ERGNHYDI) */
	double draught;
	double vel_mean;
	double vel_keel;
	double tide;

	/* water velocity profile (HS_ERGNCTDS) */
	int num_vel;
	double vdepth[MBF_HSATLRAW_MAXVEL];
	double velocity[MBF_HSATLRAW_MAXVEL];

	/* navigation source (ERGNPOSI) */
	double pos_corr_x;
	double pos_corr_y;
	char sensors[10];

	/* comment (LDEOCMNT) */
	char comment[MBF_HSATLRAW_MAXLINE];
};

#ifdef __cplusplus
extern "C" {
#endif

int mbr_register_hsatlraw(int verbose, void *mbio_ptr, int *error);
int mbr_info_hsatlraw(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_alm_hsatlraw(int verbose, void *mbio_ptr, int *error);
int mbr_dem_hsatlraw(int verbose, void *mbio_ptr, int *error);
int mbr_zero_hsatlraw(int verbose, void *data_ptr, int mode, int *error);
int mbr_rt_hsatlraw(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_hsatlraw(int verbose, void *mbio_ptr, void *store_ptr, int *error);

int mbr_hsatlraw_rd_label(int verbose, FILE *mbfp, char *line, int *type, int *shift, int *error);
int mbr_hsatlraw_read_line(int verbose, FILE *mbfp, int minimum_size, char *line, int *error);
int mbr_hsatlraw_rd_ergnhydi(int verbose, FILE *mbfp, struct mbf_hsatlraw_struct *data, size_t shift, int *error);
int mbr_hsatlraw_rd_ergnpara(int verbose, FILE *mbfp, struct mbf_hsatlraw_struct *data, size_t shift, int *error);
int mbr_hsatlraw_rd_ergnposi(int verbose, FILE *mbfp, struct mbf_hsatlraw_struct *data, size_t shift, int *error);
int mbr_hsatlraw_rd_ergneich(int verbose, FILE *mbfp, struct mbf_hsatlraw_struct *data, size_t shift, int *error);
int mbr_hsatlraw_rd_ergnmess(int verbose, FILE *mbfp, struct mbf_hsatlraw_struct *data, size_t shift, int *error);
int mbr_hsatlraw_rd_ergnslzt(int verbose, FILE *mbfp, struct mbf_hsatlraw_struct *data, size_t shift, int *error);
int mbr_hsatlraw_rd_ergnctds(int verbose, FILE *mbfp, struct mbf_hsatlraw_struct *data, size_t shift, int *error);
int mbr_hsatlraw_rd_ergnampl(int verbose, FILE *mbfp, struct mbf_hsatlraw_struct *data, size_t shift, int *error);
int mbr_hsatlraw_rd_ldeocmnt(int verbose, FILE *mbfp, struct mbf_hsatlraw_struct *data, size_t shift, int *error);

int mbr_hsatlraw_rd_data(int verbose, void *mbio_ptr, int *error);
int mbr_hsatlraw_wr_data(int verbose, void *mbio_ptr, void *data_ptr, int *error);
int mbr_hsatlraw_wr_label(int verbose, FILE *mbfp, char type, int *error);
int mbr_hsatlraw_write_line(int verbose, FILE *mbfp, char *line, int *error);
int mbr_hsatlraw_wr_rawline(int verbose, FILE *mbfp, void *data_ptr, int *error);
int mbr_hsatlraw_wr_ergnhydi(int verbose, FILE *mbfp, void *data_ptr, int *error);
int mbr_hsatlraw_wr_ergnpara(int verbose, FILE *mbfp, void *data_ptr, int *error);
int mbr_hsatlraw_wr_ergnposi(int verbose, FILE *mbfp, void *data_ptr, int *error);
int mbr_hsatlraw_wr_ergneich(int verbose, FILE *mbfp, void *data_ptr, int *error);
int mbr_hsatlraw_wr_ergnmess(int verbose, FILE *mbfp, void *data_ptr, int *error);
int mbr_hsatlraw_wr_ergnslzt(int verbose, FILE *mbfp, void *data_ptr, int *error);
int mbr_hsatlraw_wr_ergnctds(int verbose, FILE *mbfp, void *data_ptr, int *error);
int mbr_hsatlraw_wr_ergnampl(int verbose, FILE *mbfp, void *data_ptr, int *error);
int mbr_hsatlraw_wr_ldeocmnt(int verbose, FILE *mbfp, void *data_ptr, int *error);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif  /* MBF_HSATLRAW_H_ */
