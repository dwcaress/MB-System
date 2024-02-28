/*--------------------------------------------------------------------
 *    The MB-system:	mb_info.h			8/21/2006
 *
 *    Copyright (c) 2006-2024 by
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
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/**
   @file
 * @brief Define file information and data structures used by
 * mbproject.
 *
 * Author:	D. W. Caress
 * Date:	August 21, 2006
 *
 *
 */

#ifndef MB_INFO_H_
#define MB_INFO_H_

#define MB_INFO_MASK_DIM 20

/* structure holding output of mbinfo -N -G */
struct mb_info_struct {
	int loaded;
	char file[MB_PATH_MAXLINE];

	int nrecords;
	int nrecords_ss1;
	int nrecords_ss2;
	int nrecords_sbp;
	int nbeams_bath;
	int nbeams_bath_total;
	int nbeams_bath_good;
	int nbeams_bath_zero;
	int nbeams_bath_flagged;
	int nbeams_amp;
	int nbeams_amp_total;
	int nbeams_amp_good;
	int nbeams_amp_zero;
	int nbeams_amp_flagged;
	int npixels_ss;
	int npixels_ss_total;
	int npixels_ss_good;
	int npixels_ss_zero;
	int npixels_ss_flagged;

	double time_total;
	double dist_total;
	double speed_avg;

	double time_start;
	double lon_start;
	double lat_start;
	double depth_start;
	double heading_start;
	double speed_start;
	double sensordepth_start;
	double sonaraltitude_start;

	double time_end;
	double lon_end;
	double lat_end;
	double depth_end;
	double heading_end;
	double speed_end;
	double sensordepth_end;
	double sonaraltitude_end;

	double lon_min;
	double lon_max;
	double lat_min;
	double lat_max;
	double sensordepth_min;
	double sensordepth_max;
	double altitude_min;
	double altitude_max;
	double depth_min;
	double depth_max;
	double amp_min;
	double amp_max;
	double ss_min;
	double ss_max;

	int problem_nodata;
	int problem_zeronav;
	int problem_toofast;
	int problem_avgtoofast;
	int problem_toodeep;
	int problem_baddatagram;

	// int	mask_nx;
	// int	mask_ny;
	// double	mask_dx;
	// double	mask_dy;
	// int	mask_alloc;
	// int	*mask;
};

#ifdef __cplusplus
extern "C" {
#endif

/* MBIO mb_info function prototypes */
int mb_info_init(int verbose, struct mb_info_struct *mb_info, int *error);
int mb_get_info(int verbose, char *file, struct mb_info_struct *mb_info, int lonflip, int *error);
int mb_get_info_datalist(int verbose, char *read_file, int *format, struct mb_info_struct *mb_info, int lonflip, int *error);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif  /* MB_INFO_H_ */
