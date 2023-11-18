/*--------------------------------------------------------------------
 *    The MB-system:	mbnavlist.c	2/1/93
 *
 *    Copyright (c) 1993-2023 by
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
/*
 * mbnavlist prints the specified contents of navigation records
 * in a swath sonar data file to stdout. The form of the
 * output is quite flexible; mbnavlist is tailored to produce
 * ascii files in spreadsheet style with data columns separated by tabs.
 *
 * Author:	D. W. Caress
 * Date:	November 11, 1999
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <getopt.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_status.h"

constexpr int MAX_OPTIONS = 25;
typedef enum {
    MBNAVLIST_SEGMENT_MODE_NONE = 0,
    MBNAVLIST_SEGMENT_MODE_TAG = 1,
    MBNAVLIST_SEGMENT_MODE_SWATHFILE = 2,
    MBNAVLIST_SEGMENT_MODE_DATALIST = 3,
} segment_mode_t;

constexpr char program_name[] = "mbnavlist";
constexpr char help_message[] =
    "mbnavlist prints the specified contents of navigation records\n"
    "in a swath sonar data file to stdout. The form of the\n"
    "output is quite flexible; mbnavlist is tailored to produce\n"
    "ascii files in spreadsheet style with data columns separated by tabs.";
constexpr const char usage_message[] =
    "mbnavlist [-Byr/mo/da/hr/mn/sc -Ddecimate -Eyr/mo/da/hr/mn/sc\n"
    "    -Fformat -Gdelimiter -H -Ifile -Kkind -Llonflip\n"
    "    -Ooptions -Rw/e/s/n -Sspeed\n"
    "    -Ttimegap -V -Zsegment]";

/*--------------------------------------------------------------------*/
int printsimplevalue(int verbose, double value, int width, int precision, bool ascii, bool *invert, bool *flipsign, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBlist function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       value:           %f\n", value);
		fprintf(stderr, "dbg2       width:           %d\n", width);
		fprintf(stderr, "dbg2       precision:       %d\n", precision);
		fprintf(stderr, "dbg2       ascii:           %d\n", ascii);
		fprintf(stderr, "dbg2       invert:          %d\n", *invert);
		fprintf(stderr, "dbg2       flipsign:        %d\n", *flipsign);
	}

	/* make print format */
	char format[24];
	format[0] = '%';
	if (*invert)
		strcpy(format, "%g");
	else if (width > 0)
		snprintf(&format[1], 23, "%d.%df", width, precision);
	else
		snprintf(&format[1], 23, ".%df", precision);

	/* invert value if desired */
	if (*invert) {
		*invert = false;
		if (value != 0.0)
			value = 1.0 / value;
	}

	/* flip sign value if desired */
	if (*flipsign) {
		*flipsign = false;
		value = -value;
	}

	/* print value */
	if (ascii)
		printf(format, value);
	else
		fwrite(&value, sizeof(double), 1, stdout);

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBlist function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       invert:          %d\n", *invert);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
	int verbose = 0;
	int format;
	int pings;
	int lonflip;
	double bounds[4];
	int btime_i[7];
	int etime_i[7];
	double speedmin;
	double timegap;
	int status = mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);

	char read_file[MB_PATH_MAXLINE] = "datalist.mb-1";
	int decimate = 1;
	int data_kind = -1;
	int aux_nav_channel = -1;
	bool ascii = true;
	bool segment = false;
	char segment_tag[MB_PATH_MAXLINE];
	bool use_projection = false;

	/* set up the default list controls
	    (lon, lat, along-track distance, center beam depth) */
	char list[MAX_OPTIONS] = "tMXYHs";
	int n_list = 6;
	char delimiter[MB_PATH_MAXLINE] = "\t";
	char projection_pars[MB_PATH_MAXLINE] = "";

	segment_mode_t segment_mode = MBNAVLIST_SEGMENT_MODE_NONE;

	{
		bool errflg = false;
		bool help = false;
		int c;
		while ((c = getopt(argc, argv, "AaB:b:D:d:E:e:F:f:G:g:I:i:J:j:K:k:L:l:N:n:O:o:R:r:S:s:T:t:Z:z:VvHh")) != -1)
		{
			switch (c) {
			case 'H':
			case 'h':
				help = true;
				break;
			case 'V':
			case 'v':
				verbose++;
				break;
			case 'A':
			case 'a':
				ascii = false;
				break;
			case 'B':
			case 'b':
				sscanf(optarg, "%d/%d/%d/%d/%d/%d", &btime_i[0], &btime_i[1], &btime_i[2], &btime_i[3], &btime_i[4], &btime_i[5]);
				btime_i[6] = 0;
				break;
			case 'D':
			case 'd':
				sscanf(optarg, "%d", &decimate);
				break;
			case 'E':
			case 'e':
				sscanf(optarg, "%d/%d/%d/%d/%d/%d", &etime_i[0], &etime_i[1], &etime_i[2], &etime_i[3], &etime_i[4], &etime_i[5]);
				etime_i[6] = 0;
				break;
			case 'F':
			case 'f':
				sscanf(optarg, "%d", &format);
				break;
			case 'G':
			case 'g':
				sscanf(optarg, "%1023s", delimiter);
				break;
			case 'I':
			case 'i':
				sscanf(optarg, "%1023s", read_file);
				break;
			case 'J':
			case 'j':
				sscanf(optarg, "%1023s", projection_pars);
				use_projection = true;
				break;
			case 'K':
			case 'k':
				sscanf(optarg, "%d", &data_kind);
				break;
			case 'L':
			case 'l':
				sscanf(optarg, "%d", &lonflip);
				break;
			case 'N':
			case 'n':
				sscanf(optarg, "%d", &aux_nav_channel);
				break;
			case 'O':
			case 'o':
        if (strlen(optarg) > 0) {
          n_list = MIN(strlen(optarg), MAX_OPTIONS);
          for (int j = 0; j < n_list; j++){
            if (j < MAX_OPTIONS) {
              list[j] = optarg[j];
              if (list[j] == '^')
                use_projection = true;
            }
          }
        }
				break;
			case 'R':
			case 'r':
				mb_get_bounds(optarg, bounds);
				break;
			case 'S':
			case 's':
				sscanf(optarg, "%lf", &speedmin);
				break;
			case 'T':
			case 't':
				sscanf(optarg, "%lf", &timegap);
				break;
			case 'Z':
			case 'z':
				segment = true;
				sscanf(optarg, "%1023s", segment_tag);
				if (strcmp(segment_tag, "swathfile") == 0)
					segment_mode = MBNAVLIST_SEGMENT_MODE_SWATHFILE;
				else if (strcmp(segment_tag, "datalist") == 0)
					segment_mode = MBNAVLIST_SEGMENT_MODE_DATALIST;
				else
					segment_mode = MBNAVLIST_SEGMENT_MODE_TAG;
				break;
			case '?':
				errflg = true;
			}
		}

		if (errflg) {
			fprintf(stderr, "usage: %s\n", usage_message);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_ERROR_BAD_USAGE);
		}

		if (verbose == 1 || help) {
			fprintf(stderr, "\nProgram %s\n", program_name);
			fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
		}

		if (verbose >= 2) {
			fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
			fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
			fprintf(stderr, "dbg2  Control Parameters:\n");
			fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
			fprintf(stderr, "dbg2       help:           %d\n", help);
			fprintf(stderr, "dbg2       format:         %d\n", format);
			fprintf(stderr, "dbg2       pings:          %d\n", pings);
			fprintf(stderr, "dbg2       lonflip:        %d\n", lonflip);
			fprintf(stderr, "dbg2       decimate:       %d\n", decimate);
			fprintf(stderr, "dbg2       bounds[0]:      %f\n", bounds[0]);
			fprintf(stderr, "dbg2       bounds[1]:      %f\n", bounds[1]);
			fprintf(stderr, "dbg2       bounds[2]:      %f\n", bounds[2]);
			fprintf(stderr, "dbg2       bounds[3]:      %f\n", bounds[3]);
			fprintf(stderr, "dbg2       btime_i[0]:     %d\n", btime_i[0]);
			fprintf(stderr, "dbg2       btime_i[1]:     %d\n", btime_i[1]);
			fprintf(stderr, "dbg2       btime_i[2]:     %d\n", btime_i[2]);
			fprintf(stderr, "dbg2       btime_i[3]:     %d\n", btime_i[3]);
			fprintf(stderr, "dbg2       btime_i[4]:     %d\n", btime_i[4]);
			fprintf(stderr, "dbg2       btime_i[5]:     %d\n", btime_i[5]);
			fprintf(stderr, "dbg2       btime_i[6]:     %d\n", btime_i[6]);
			fprintf(stderr, "dbg2       etime_i[0]:     %d\n", etime_i[0]);
			fprintf(stderr, "dbg2       etime_i[1]:     %d\n", etime_i[1]);
			fprintf(stderr, "dbg2       etime_i[2]:     %d\n", etime_i[2]);
			fprintf(stderr, "dbg2       etime_i[3]:     %d\n", etime_i[3]);
			fprintf(stderr, "dbg2       etime_i[4]:     %d\n", etime_i[4]);
			fprintf(stderr, "dbg2       etime_i[5]:     %d\n", etime_i[5]);
			fprintf(stderr, "dbg2       etime_i[6]:     %d\n", etime_i[6]);
			fprintf(stderr, "dbg2       speedmin:       %f\n", speedmin);
			fprintf(stderr, "dbg2       timegap:        %f\n", timegap);
			fprintf(stderr, "dbg2       aux_nav_channel:%d\n", aux_nav_channel);
			fprintf(stderr, "dbg2       data_kind:      %d\n", data_kind);
			fprintf(stderr, "dbg2       ascii:          %d\n", ascii);
			fprintf(stderr, "dbg2       segment:        %d\n", segment);
			fprintf(stderr, "dbg2       segment_mode:   %d\n", segment_mode);
			fprintf(stderr, "dbg2       segment_tag:    %s\n", segment_tag);
			fprintf(stderr, "dbg2       delimiter:      %s\n", delimiter);
			fprintf(stderr, "dbg2       use_projection: %d\n", use_projection);
			fprintf(stderr, "dbg2       projection_pars:%s\n", projection_pars);
			fprintf(stderr, "dbg2       n_list:         %d\n", n_list);
			for (int i = 0; i < n_list; i++)
				fprintf(stderr, "dbg2         list[%d]:      %c\n", i, list[i]);
		}

		if (help) {
			fprintf(stderr, "\n%s\n", help_message);
			fprintf(stderr, "\nusage: %s\n", usage_message);
			exit(MB_ERROR_NO_ERROR);
		}
	}
	int error = MB_ERROR_NO_ERROR;

	/* get format if required */
	if (format == 0)
		mb_get_format(verbose, read_file, nullptr, &format, &error);

	/* determine whether to read one file or a list of files */
	const bool read_datalist = format < 0;
	bool read_data;
	char file[MB_PATH_MAXLINE];
	void *datalist;
	double file_weight;
	char dfile[MB_PATH_MAXLINE];

	/* open file list */
	if (read_datalist) {
		const int look_processed = MB_DATALIST_LOOK_UNSET;
		if (mb_datalist_open(verbose, &datalist, read_file, look_processed, &error) != MB_SUCCESS) {
			fprintf(stderr, "\nUnable to open data list file: %s\n", read_file);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_ERROR_OPEN_FAIL);
		}
		read_data = mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error) == MB_SUCCESS;
	} else {
		// else copy single filename to be read
		strcpy(file, read_file);
		read_data = true;
	}

	bool invert_next_value = false;
	bool signflip_next_value = false;

	double btime_d;
	double etime_d;
	int beams_bath;
	int beams_amp;
	int pixels_ss;

	/* data record source types */
	int platform_source;
	int nav_source;
	int heading_source;
	int sensordepth_source;
	int attitude_source;
	int svp_source;

	/* output format list controls */
	int time_j[5];
	bool projectednav_next_value = false;

	/* MBIO read values */
	void *store_ptr;
	int time_i[7];
	double draft;
	double roll;
	double pitch;
	double heave;
	char *beamflag = nullptr;
	double *bath = nullptr;
	double *bathacrosstrack = nullptr;
	double *bathalongtrack = nullptr;
	double *amp = nullptr;
	double *ss = nullptr;
	double *ssacrosstrack = nullptr;
	double *ssalongtrack = nullptr;
	char comment[MB_COMMENT_MAXLINE];
	int atime_i[7 * MB_ASYNCH_SAVE_MAX];
	double atime_d[MB_ASYNCH_SAVE_MAX];
	double anavlon[MB_ASYNCH_SAVE_MAX];
	double anavlat[MB_ASYNCH_SAVE_MAX];
	double aspeed[MB_ASYNCH_SAVE_MAX];
	double aheading[MB_ASYNCH_SAVE_MAX];
	double adraft[MB_ASYNCH_SAVE_MAX];
	double aroll[MB_ASYNCH_SAVE_MAX];
	double apitch[MB_ASYNCH_SAVE_MAX];
	double aheave[MB_ASYNCH_SAVE_MAX];

	/* additional time variables */
	bool first_m = true;
	double time_d_ref;
	bool first_u = true;
	time_t time_u;
	time_t time_u_ref;
	double seconds;

	/* course calculation variables */
	double dlon;
	double dlat;
	double minutes;
	int degrees;
	double mtodeglon;
	double mtodeglat;
	double course;
	double time_d_old;
	double time_interval;
	double speed_made_good;
	double navlon_old;
	double navlat_old;

	/* projected coordinate system */
	int proj_status;
	void *pjptr = nullptr;
	double reference_lon;
	double reference_lat;
	double naveasting;
	double navnorthing;
	double deasting;

	/* loop over all files to be read */
	while (read_data) {
		/* check format and get data sources */
		if ((status = mb_format_source(verbose, &format, &platform_source, &nav_source, &sensordepth_source, &heading_source,
		                               &attitude_source, &svp_source, &error)) == MB_FAILURE) {
			char *message;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error returned from function <mb_format_source>:\n%s\n", message);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}

		/* set auxiliary nav source if requested
		    - note this is superseded by data_kind if the -K option is used */
		if (aux_nav_channel > 0) {
			if (aux_nav_channel == 1)
				nav_source = MB_DATA_NAV1;
			else if (aux_nav_channel == 2)
				nav_source = MB_DATA_NAV2;
			else if (aux_nav_channel == 3)
				nav_source = MB_DATA_NAV3;
		}

		/* initialize reading the swath file */
		void *mbio_ptr = nullptr;
		if (mb_read_init(verbose, file, format, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, &mbio_ptr,
		                           &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
			char *message;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
			fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", file);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}

		/* allocate memory for data arrays */
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &=
			    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &=
			    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack, &error);

		/* if error initializing memory then quit */
		if (error != MB_ERROR_NO_ERROR) {
			char *message;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}

		/* output separator for GMT style segment file output */
		if (segment && ascii) {
			if (segment_mode == MBNAVLIST_SEGMENT_MODE_TAG)
				printf("%s\n", segment_tag);
			else if (segment_mode == MBNAVLIST_SEGMENT_MODE_SWATHFILE)
				printf("# %s\n", file);
			else if (segment_mode == MBNAVLIST_SEGMENT_MODE_DATALIST)
				printf("# %s\n", dfile);
		}

		/* read and print data */
		double distance_total = 0.0;
		int nread = 0;
		int nnav = 0;
		bool first = true;
		double course_old;
		double speed_made_good_old;
		while (error <= MB_ERROR_NO_ERROR) {
			/* read a ping of data */
			int kind;
			double navlon;
			double navlat;
			double time_d;
			double speed;
			double heading;
			double distance;
			double altitude;
			double sensordepth;
			status = mb_get_all(verbose, mbio_ptr, &store_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading,
			                    &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp,
			                    bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);

			/* time gaps are not a problem here */
			if (error == MB_ERROR_TIME_GAP) {
				error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
			}

			/* check for appropriate navigation record */

			/* if the -K option is used look for a particular
			    sort of data record */
			if (error <= MB_ERROR_NO_ERROR) {
				if (data_kind > 0) {
					if (kind == data_kind) {
						error = MB_ERROR_NO_ERROR;
						status = MB_SUCCESS;
					}
					else {
						error = MB_ERROR_IGNORE;
						status = MB_FAILURE;
					}
				}
				else {
					if (kind == nav_source) {
						error = MB_ERROR_NO_ERROR;
						status = MB_SUCCESS;
					}
					else {
						error = MB_ERROR_IGNORE;
						status = MB_FAILURE;
					}
				}
			}

			/* extract additional nav info */
			int n;
			if (error == MB_ERROR_NO_ERROR) {
				status = mb_extract_nnav(verbose, mbio_ptr, store_ptr, MB_ASYNCH_SAVE_MAX, &kind, &n, atime_i, atime_d, anavlon,
				                         anavlat, aspeed, aheading, adraft, aroll, apitch, aheave, &error);
			}

			if (error == MB_ERROR_NO_ERROR)
				nread++;

			if (verbose >= 2) {
				fprintf(stderr, "\ndbg2  Data read in program <%s>\n", program_name);
				fprintf(stderr, "dbg2       kind:           %d\n", kind);
				fprintf(stderr, "dbg2       error:          %d\n", error);
				fprintf(stderr, "dbg2       status:         %d\n", status);
				fprintf(stderr, "dbg2       n:              %d\n", n);
			}

			/* loop over the n navigation points, outputting each one */
			/* calculate course made good and distance */
			if (error == MB_ERROR_NO_ERROR && n > 0) {
				for (int inav = 0; inav < n; inav++) {
					/* get data */
					for (int j = 0; j < 7; j++)
						time_i[j] = atime_i[inav * 7 + j];
					time_d = atime_d[inav];
					navlon = anavlon[inav];
					navlat = anavlat[inav];
					speed = aspeed[inav];
					heading = aheading[inav];
					draft = adraft[inav];
					roll = aroll[inav];
					pitch = apitch[inav];
					heave = aheave[inav];
					sensordepth = draft - heave;

					/* calculate course made good and distance */
					mb_coor_scale(verbose, navlat, &mtodeglon, &mtodeglat);
					if (first) {
						time_interval = 0.0;
						course = heading;
						speed_made_good = 0.0;
						// course_old = heading;
						// speed_made_good_old = speed;
						distance = 0.0;
					}
					else {
						time_interval = time_d - time_d_old;
						const double dx = (navlon - navlon_old) / mtodeglon;
						const double dy = (navlat - navlat_old) / mtodeglat;
						distance = sqrt(dx * dx + dy * dy);
						if (distance > 0.0)
							course = RTD * atan2(dx / distance, dy / distance);
						else
							course = course_old;
						if (course < 0.0)
							course = course + 360.0;
						if (time_interval > 0.0)
							speed_made_good = 3.6 * distance / time_interval;
						else
							speed_made_good = speed_made_good_old;
					}
					distance_total += 0.001 * distance;

					/* get projected navigation if needed */
					if (use_projection) {
						/* set up projection if this is the first data */
						if (pjptr == nullptr) {
							/* Default projection is UTM */
							if (strlen(projection_pars) == 0)
								strcpy(projection_pars, "U");

							char projection_id[MB_PATH_MAXLINE];
							/* check for UTM with undefined zone */
							if (strcmp(projection_pars, "UTM") == 0 || strcmp(projection_pars, "U") == 0 ||
							    strcmp(projection_pars, "utm") == 0 || strcmp(projection_pars, "u") == 0) {
								reference_lon = navlon;
								if (reference_lon < 180.0)
									reference_lon += 360.0;
								if (reference_lon >= 180.0)
									reference_lon -= 360.0;
								const int utm_zone = (int)(((reference_lon + 183.0) / 6.0) + 0.5);
								reference_lat = navlat;
								if (reference_lat >= 0.0)
									snprintf(projection_id, sizeof(projection_id), "UTM%2.2dN", utm_zone);
								else
									snprintf(projection_id, sizeof(projection_id), "UTM%2.2dS", utm_zone);
							}
							else
								strcpy(projection_id, projection_pars);

							/* set projection flag */
							proj_status = mb_proj_init(verbose, projection_id, &(pjptr), &error);

							/* if projection not successfully initialized then quit */
							if (proj_status != MB_SUCCESS) {
								fprintf(stderr, "\nOutput projection %s not found in database\n", projection_id);
								fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
								mb_memory_clear(verbose, &error);
								exit(MB_ERROR_BAD_PARAMETER);
							}
						}

						/* get projected navigation */
						mb_proj_forward(verbose, pjptr, navlon, navlat, &naveasting, &navnorthing, &error);
					}

					/* reset old values */
					navlon_old = navlon;
					navlat_old = navlat;
					course_old = course;
					speed_made_good_old = speed_made_good;
					time_d_old = time_d;

					/* now loop over list of output parameters */
					if (nnav % decimate == 0)
						for (int i = 0; i < n_list; i++) {
							switch (list[i]) {
							case '/': /* Inverts next simple value */
								invert_next_value = true;
								break;
							case '-': /* Flip sign on next simple value */
								signflip_next_value = true;
								break;
							case 'c': /* Sonar transducer depth (m) */
								printsimplevalue(verbose, sensordepth, 0, 4, ascii, &invert_next_value, &signflip_next_value,
								                 &error);
								break;
							case 'H': /* heading */
								printsimplevalue(verbose, heading, 7, 3, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							case 'h': /* course */
								printsimplevalue(verbose, course, 7, 3, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							case 'J': /* time string */
								mb_get_jtime(verbose, time_i, time_j);
								seconds = time_i[5] + 0.000001 * time_i[6];
								if (ascii) {
									printf("%.4d %.3d %.2d %.2d %9.6f", time_j[0], time_j[1], time_i[3], time_i[4], seconds);
								}
								else {
									double b = time_j[0];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_j[1];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_i[3];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_i[4];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_i[5];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_i[6];
									fwrite(&b, sizeof(double), 1, stdout);
								}
								break;
							case 'j': /* time string */
								mb_get_jtime(verbose, time_i, time_j);
								seconds = time_i[5] + 0.000001 * time_i[6];
								if (ascii) {
									printf("%.4d %.3d %.4d %9.6f", time_j[0], time_j[1], time_j[2], seconds);
								}
								else {
									double b = time_j[0];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_j[1];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_j[2];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_j[3];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_j[4];
									fwrite(&b, sizeof(double), 1, stdout);
								}
								break;
							case 'L': /* along-track distance (km) */
								printsimplevalue(verbose, distance_total, 8, 4, ascii, &invert_next_value, &signflip_next_value,
								                 &error);
								break;
							case 'l': /* along-track distance (m) */
								printsimplevalue(verbose, 1000.0 * distance_total, 8, 4, ascii, &invert_next_value,
								                 &signflip_next_value, &error);
								break;
							case 'M': /* Decimal unix seconds since
							        1/1/70 00:00:00 */
								printsimplevalue(verbose, time_d, 0, 6, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							case 'm': /* time in decimal seconds since
							        first record */
							{
								if (first_m) {
									time_d_ref = time_d;
									first_m = false;
								}
								double b = time_d - time_d_ref;
								printsimplevalue(verbose, b, 0, 6, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							}
							case 'P': /* pitch */
								printsimplevalue(verbose, pitch, 6, 3, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							case 'p': /* draft */
								printsimplevalue(verbose, draft, 7, 4, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							case 'R': /* roll */
								printsimplevalue(verbose, roll, 6, 3, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							case 'r': /* heave */
								printsimplevalue(verbose, heave, 7, 4, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							case 'S': /* speed */
								printsimplevalue(verbose, speed, 6, 3, ascii, &invert_next_value, &signflip_next_value, &error);
								break;
							case 's': /* speed made good */
								printsimplevalue(verbose, speed_made_good, 6, 3, ascii, &invert_next_value, &signflip_next_value,
								                 &error);
								break;
							case 'T': /* yyyy/mm/dd/hh/mm/ss time string */
								seconds = time_i[5] + 1e-6 * time_i[6];
								if (ascii) {
									printf("%.4d/%.2d/%.2d/%.2d/%.2d/%09.6f", time_i[0], time_i[1], time_i[2], time_i[3],
									       time_i[4], seconds);
								} else {
									double b = time_i[0];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_i[1];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_i[2];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_i[3];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_i[4];
									fwrite(&b, sizeof(double), 1, stdout);
									b = seconds;
									fwrite(&b, sizeof(double), 1, stdout);
								}
								break;
							case 't': /* yyyy mm dd hh mm ss time string */
								seconds = time_i[5] + 1e-6 * time_i[6];
								if (ascii) {
									printf("%.4d %.2d %.2d %.2d %.2d %09.6f", time_i[0], time_i[1], time_i[2], time_i[3],
									       time_i[4], seconds);
								} else {
									double b = time_i[0];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_i[1];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_i[2];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_i[3];
									fwrite(&b, sizeof(double), 1, stdout);
									b = time_i[4];
									fwrite(&b, sizeof(double), 1, stdout);
									b = seconds;
									fwrite(&b, sizeof(double), 1, stdout);
								}
								break;
							case 'U': /* unix time in seconds since 1/1/70 00:00:00 */
								time_u = (int)time_d;
								if (ascii) {
									printf("%ld", time_u);
								} else {
									double b = time_u;
									fwrite(&b, sizeof(double), 1, stdout);
								}
								break;
							case 'u': /* time in seconds since first record */
								time_u = (int)time_d;
								if (first_u) {
									time_u_ref = time_u;
									first_u = false;
								}
								if (ascii) {
									printf("%ld", time_u - time_u_ref);
								} else {
									double b = time_u - time_u_ref;
									fwrite(&b, sizeof(double), 1, stdout);
								}
								break;
							case 'V': /* time in seconds since last ping */
							case 'v':
								if (ascii) {
									if (fabs(time_interval) > 100.)
										printf("%g", time_interval);
									else
										printf("%7.3f", time_interval);
								}
								else {
									fwrite(&time_interval, sizeof(double), 1, stdout);
								}
								break;
							case 'X': /* longitude decimal degrees */
								if (!projectednav_next_value) {
									dlon = navlon;
									printsimplevalue(verbose, dlon, 15, 10, ascii, &invert_next_value, &signflip_next_value,
									                 &error);
								}
								else {
									deasting = naveasting;
									printsimplevalue(verbose, deasting, 15, 3, ascii, &invert_next_value, &signflip_next_value,
									                 &error);
								}
								projectednav_next_value = false;
								break;
							case 'x': /* longitude degress + decimal minutes */
								dlon = navlon;
								char hemi;
								if (dlon < 0.0) {
									hemi = 'W';
									dlon = -dlon;
								}
								else
									hemi = 'E';
								degrees = (int)dlon;
								minutes = 60.0 * (dlon - degrees);
								if (ascii) {
									printf("%3d %11.8f%c", degrees, minutes, hemi);
								} else {
									double b = degrees;
									if (hemi == 'W')
										b = -b;
									fwrite(&b, sizeof(double), 1, stdout);
									b = minutes;
									fwrite(&b, sizeof(double), 1, stdout);
								}
								break;
							case 'Y': /* latitude decimal degrees */
								if (!projectednav_next_value) {
									dlat = navlat;
									printsimplevalue(verbose, dlat, 15, 10, ascii, &invert_next_value, &signflip_next_value,
									                 &error);
								} else {
									const double dnorthing = navnorthing;
									printsimplevalue(verbose, dnorthing, 15, 3, ascii, &invert_next_value, &signflip_next_value,
									                 &error);
								}
								projectednav_next_value = false;
								break;
							case 'y': /* latitude degrees + decimal minutes */
								dlat = navlat;
								if (dlat < 0.0) {
									hemi = 'S';
									dlat = -dlat;
								} else {
									hemi = 'N';
								}
								degrees = (int)dlat;
								minutes = 60.0 * (dlat - degrees);
								if (ascii) {
									printf("%3d %11.8f%c", degrees, minutes, hemi);
								} else {
									double b = degrees;
									if (hemi == 'S')
										b = -b;
									fwrite(&b, sizeof(double), 1, stdout);
									b = minutes;
									fwrite(&b, sizeof(double), 1, stdout);
								}
								break;
							default:
								if (ascii)
									printf("<Invalid Option: %c>", list[i]);
								break;
							}
							if (ascii) {
								if (i < (n_list - 1))
									printf("%s", delimiter);
								else
									printf("\n");
							}
						}
					nnav++;
					first = false;
				}
			}
		}

		status &= mb_close(verbose, &mbio_ptr, &error);

		/* figure out whether and what to read next */
		if (read_datalist) {
			read_data = mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error) == MB_SUCCESS;
		} else {
			read_data = false;
		}

		/* end loop over files in list */
	}
	if (read_datalist)
		mb_datalist_close(verbose, &datalist, &error);

  /* free projection */
  if (use_projection && pjptr != NULL) {
    mb_proj_free(verbose, &(pjptr), &error);
  }

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose, &error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s> completed\n", program_name);
		fprintf(stderr, "dbg2  Ending status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	exit(error);
}
/*--------------------------------------------------------------------*/
