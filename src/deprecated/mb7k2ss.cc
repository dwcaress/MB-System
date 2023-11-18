/*--------------------------------------------------------------------
 *    The MB-system:	mb7k2ss.c		8/15/2007
 *
 *    Copyright (c) 2007-2023 by
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
 * mb7k2ss extracts Edgetech sidescan data
 * from Reson 7k format data, lays the sidescan on the bottom,
 * and outputs in format 71 mbldeoih.
 *
 * Author:	D. W. Caress
 * Date:	August 15, 2007
 *              R/V Atlantis, Axial Seamount
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <unistd.h>

#include <algorithm>

#include "mb_aux.h"
#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_ldeoih.h"
#include "mbsys_reson7k.h"

typedef enum {
    MB7K2SS_SS_FLAT_BOTTOM = 0,
    MB7K2SS_SS_3D_BOTTOM = 1,
} layout_t;
constexpr int MB7K2SS_SSDIMENSION = 4001;
typedef enum {
    MB7K2SS_SSLOW = 1,
    MB7K2SS_SSHIGH = 2,
} extract_t;

typedef enum {
    MB7K2SS_BOTTOMPICK_NONE = 0,
    MB7K2SS_BOTTOMPICK_BATHYMETRY = 1,
    MB7K2SS_BOTTOMPICK_ALTITUDE = 2,
    MB7K2SS_BOTTOMPICK_ARRIVAL = 3,
    MB7K2SS_BOTTOMPICK_3DBATHY = 4,
} bottompick_t;

typedef enum {
    MB7K2SS_SSGAIN_OFF = 0,
    MB7K2SS_SSGAIN_TVG_1OVERR = 1,
} ssgain_t;

constexpr int MB7K2SS_ALLOC_NUM = 128;
constexpr int MB7K2SS_ALLOC_CHUNK = 1024;

typedef enum {
    MB7K2SS_ROUTE_WAYPOINT_NONE = 0,
    MB7K2SS_ROUTE_WAYPOINT_SIMPLE = 1,
    MB7K2SS_ROUTE_WAYPOINT_TRANSIT = 2,
    MB7K2SS_ROUTE_WAYPOINT_STARTLINE = 3,
    MB7K2SS_ROUTE_WAYPOINT_ENDLINE = 4,
} waypoint_t;

constexpr double MB7K2SS_ONLINE_THRESHOLD = 15.0;
constexpr int MB7K2SS_ONLINE_COUNT = 30;

constexpr int MB7K2SS_NUM_ANGLES = 171;
constexpr double MB7K2SS_ANGLE_MAX = 85.0;

constexpr char program_name[] = "mb7k2ss";
constexpr char help_message[] =
    "mb7k2ss extracts sidescan sonar data from Reson 7k format data,\n"
    "bins and lays the sidescan onto the seafloor, and outputs files \n"
    "in the MBF_MBLDEOIH formst (MBIO format id 71).\n";
constexpr char usage_message[] =
    "mb7k2ss [-Ifile -Atype -Bmode[/threshold] -C -D -Fformat -Lstartline/lineroot -Ooutfile -Rroutefile "
    "-Ttopogridfile -X -H -V]";

/*--------------------------------------------------------------------*/
int mb7k2ss_get_flatbottom_table(int verbose, int nangle, double angle_min, double angle_max, double navlon, double navlat,
                                 double altitude, double pitch, double *table_angle, double *table_xtrack, double *table_ltrack,
                                 double *table_altitude, double *table_range, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MB7K2SS function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       nangle:          %d\n", nangle);
		fprintf(stderr, "dbg2       angle_min:       %f\n", angle_min);
		fprintf(stderr, "dbg2       angle_max:       %f\n", angle_max);
		fprintf(stderr, "dbg2       navlon:          %f\n", navlon);
		fprintf(stderr, "dbg2       navlat:          %f\n", navlat);
		fprintf(stderr, "dbg2       pitch:           %f\n", pitch);
	}

	/* loop over all of the angles */
	const double dangle = (angle_max - angle_min) / (nangle - 1);
	const double alpha = pitch;
	const double zz = altitude;
	for (int i = 0; i < nangle; i++) {
		/* get angles in takeoff coordinates */
		table_angle[i] = angle_min + dangle * i;
		double beta = 90.0 - table_angle[i];
		double theta;
		double phi;
		mb_rollpitch_to_takeoff(verbose, alpha, beta, &theta, &phi, error);

		/* calculate range required to achieve desired altitude */
		const double rr = zz / cos(DTR * theta);

		/* get the position */
		const double xx = rr * sin(DTR * theta);
		table_xtrack[i] = xx * cos(DTR * phi);
		table_ltrack[i] = xx * sin(DTR * phi);
		table_altitude[i] = zz;
		table_range[i] = rr;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MB7K2SS function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       Lookup tables:\n");
		for (int i = 0; i < nangle; i++)
			fprintf(stderr, "dbg2         %d %f %f %f %f %f\n", i, table_angle[i], table_xtrack[i], table_ltrack[i],
			        table_altitude[i], table_range[i]);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", MB_SUCCESS);
	}

	return (MB_SUCCESS);
}

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
	int verbose = 0;
	int format = 0;
	int pings;
	int lonflip;
	double bounds[4];
	int btime_i[7];
	int etime_i[7];
	double speedmin;
	double timegap;
	int status = mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);

	int error = MB_ERROR_NO_ERROR;

	bool read_datalist = false;  // TODO(schwehr): Probable bug with this var.
	char output_file[MB_PATH_MAXLINE+50];
	mb_path current_output_file;
	bool new_output_file = true;
	bool output_file_set = false;
	double file_weight;
	double btime_d;
	double etime_d;
	mb_path file;
	mb_path dfile;
	int beams_bath;
	int beams_amp;
	int pixels_ss;

	int startline = 1;
	mb_path lineroot = "sidescan";


	/* set default input to datalist.mb-1 */
	mb_path read_file = "datalist.mb-1";

	int mode;

	/* extract modes */
	extract_t  extract_type = MB7K2SS_SSLOW;
	int target_kind = MB_DATA_SIDESCAN2;
	bool print_comments = false;

	/* bottompick mode */
	bottompick_t bottompickmode = MB7K2SS_BOTTOMPICK_ALTITUDE;
	double bottompickthreshold = 0.4;

	/* sidescan interpolation scale */
	int interpbins = 0;

	/* sidescan gain mode */
	ssgain_t gainmode = MB7K2SS_SSGAIN_OFF;  // TODO(schwehr): Not used.
	double gainfactor = 1.0;

	bool checkroutebearing = false;
	mb_path timelist_file;
	bool timelist_file_set = false;
	mb_path route_file;
	bool route_file_set = false;
	int smooth = 0;
	mb_path topogridfile;
	layout_t sslayoutmode = MB7K2SS_SS_FLAT_BOTTOM;
	double rangethreshold = 50.0;
	bool swath_width_set = false;
	double swath_width = -1.0;
	bool ssflip = false;

	/* process argument list */
	{
		bool errflg = false;
		int c;
		bool help = false;
		while ((c = getopt(argc, argv, "A:a:B:b:CcD:d:F:f:G:g:I:i:L:l:MmO:o:Q:q:R:r:S:s:T:t:U:u:W:w:XxVvHh")) != -1)
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
				if (strncmp(optarg, "SSLOW", 5) == 0 || strncmp(optarg, "sslow", 5) == 0) {
					extract_type = MB7K2SS_SSLOW;
					target_kind = MB_DATA_SIDESCAN2;
				}
				else if (strncmp(optarg, "SSHIGH", 6) == 0 || strncmp(optarg, "sshigh", 6) == 0) {
					extract_type = MB7K2SS_SSHIGH;
					target_kind = MB_DATA_SIDESCAN3;
				}
				else {
					sscanf(optarg, "%d", &mode);
					if (mode == MB7K2SS_SSLOW) {
						extract_type = MB7K2SS_SSLOW;
						target_kind = MB_DATA_SIDESCAN2;
					}
					else if (mode == MB7K2SS_SSHIGH) {
						extract_type = MB7K2SS_SSHIGH;
						target_kind = MB_DATA_SIDESCAN3;
					}
				}
				break;
			case 'B':
			case 'b':
			{
				int tmp;
				const int n = sscanf(optarg, "%d/%lf", &tmp, &bottompickthreshold);
				bottompickmode = (bottompick_t)tmp;
				if (n == 0)
					bottompickmode = MB7K2SS_BOTTOMPICK_ALTITUDE;
				else if (n == 1 && bottompickmode == MB7K2SS_BOTTOMPICK_ARRIVAL)
					bottompickthreshold = 0.5;
				break;
			}
			case 'C':
			case 'c':
				print_comments = true;
				break;
			case 'D':
			case 'd':
				sscanf(optarg, "%d", &interpbins);
				break;
			case 'F':
			case 'f':
				sscanf(optarg, "%d", &format);
				break;
			case 'G':
			case 'g':
			{
				int tmp;
				sscanf(optarg, "%d/%lf", &tmp, &gainfactor);
				gainmode = (ssgain_t)tmp;
				break;
			}
			case 'I':
			case 'i':
				sscanf(optarg, "%1023s", read_file);
				break;
			case 'L':
			case 'l':
				sscanf(optarg, "%d/%1023s", &startline, lineroot);
				break;
			case 'M':
			case 'm':
				checkroutebearing = true;
				break;
			case 'O':
			case 'o':
				sscanf(optarg, "%1023s", output_file);
				output_file_set = true;
				break;
			case 'Q':
			case 'q':
				sscanf(optarg, "%1023s", timelist_file);
				timelist_file_set = true;
				break;
			case 'R':
			case 'r':
				sscanf(optarg, "%1023s", route_file);
				route_file_set = true;
				break;
			case 'S':
			case 's':
				sscanf(optarg, "%d", &smooth);
				break;
			case 'T':
			case 't':
				sscanf(optarg, "%1023s", topogridfile);
				sslayoutmode = MB7K2SS_SS_3D_BOTTOM;
				break;
			case 'U':
			case 'u':
				sscanf(optarg, "%lf", &rangethreshold);
				break;
			case 'W':
			case 'w':
				sscanf(optarg, "%lf", &swath_width);
				if (swath_width > 0.0)
					swath_width_set = true;
				break;
			case 'X':
			case 'x':
				ssflip = true;
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
			fprintf(stderr, "dbg2       verbose:             %d\n", verbose);
			fprintf(stderr, "dbg2       help:                %d\n", help);
			fprintf(stderr, "dbg2       format:              %d\n", format);
			fprintf(stderr, "dbg2       pings:               %d\n", pings);
			fprintf(stderr, "dbg2       lonflip:             %d\n", lonflip);
			fprintf(stderr, "dbg2       bounds[0]:           %f\n", bounds[0]);
			fprintf(stderr, "dbg2       bounds[1]:           %f\n", bounds[1]);
			fprintf(stderr, "dbg2       bounds[2]:           %f\n", bounds[2]);
			fprintf(stderr, "dbg2       bounds[3]:           %f\n", bounds[3]);
			fprintf(stderr, "dbg2       btime_i[0]:          %d\n", btime_i[0]);
			fprintf(stderr, "dbg2       btime_i[1]:          %d\n", btime_i[1]);
			fprintf(stderr, "dbg2       btime_i[2]:          %d\n", btime_i[2]);
			fprintf(stderr, "dbg2       btime_i[3]:          %d\n", btime_i[3]);
			fprintf(stderr, "dbg2       btime_i[4]:          %d\n", btime_i[4]);
			fprintf(stderr, "dbg2       btime_i[5]:          %d\n", btime_i[5]);
			fprintf(stderr, "dbg2       btime_i[6]:          %d\n", btime_i[6]);
			fprintf(stderr, "dbg2       etime_i[0]:          %d\n", etime_i[0]);
			fprintf(stderr, "dbg2       etime_i[1]:          %d\n", etime_i[1]);
			fprintf(stderr, "dbg2       etime_i[2]:          %d\n", etime_i[2]);
			fprintf(stderr, "dbg2       etime_i[3]:          %d\n", etime_i[3]);
			fprintf(stderr, "dbg2       etime_i[4]:          %d\n", etime_i[4]);
			fprintf(stderr, "dbg2       etime_i[5]:          %d\n", etime_i[5]);
			fprintf(stderr, "dbg2       etime_i[6]:          %d\n", etime_i[6]);
			fprintf(stderr, "dbg2       speedmin:            %f\n", speedmin);
			fprintf(stderr, "dbg2       timegap:             %f\n", timegap);
			fprintf(stderr, "dbg2       bottompickmode:      %d\n", bottompickmode);
			fprintf(stderr, "dbg2       bottompickthreshold: %f\n", bottompickthreshold);
			fprintf(stderr, "dbg2       smooth:              %d\n", smooth);
			fprintf(stderr, "dbg2       swath_width_set:     %d\n", swath_width_set);
			fprintf(stderr, "dbg2       swath_width:         %f\n", swath_width);
			fprintf(stderr, "dbg2       interpbins:          %d\n", interpbins);
			fprintf(stderr, "dbg2       gainmode:            %d\n", gainmode);
			fprintf(stderr, "dbg2       gainfactor:          %f\n", gainfactor);
			fprintf(stderr, "dbg2       sslayoutmode:        %d\n", sslayoutmode);
			fprintf(stderr, "dbg2       topogridfile:        %s\n", topogridfile);
			fprintf(stderr, "dbg2       timelist_file_set:   %d\n", timelist_file_set);
			fprintf(stderr, "dbg2       timelist_file:       %s\n", timelist_file);
			fprintf(stderr, "dbg2       route_file_set:      %d\n", route_file_set);
			fprintf(stderr, "dbg2       route_file:          %s\n", route_file);
			fprintf(stderr, "dbg2       checkroutebearing:   %d\n", checkroutebearing);
			fprintf(stderr, "dbg2       output_file:         %s\n", output_file);
			fprintf(stderr, "dbg2       output_file_set:     %d\n", output_file_set);
			fprintf(stderr, "dbg2       lineroot:            %s\n", lineroot);
			fprintf(stderr, "dbg2       extract_type:        %d\n", extract_type);
			fprintf(stderr, "dbg2       print_comments:      %d\n", print_comments);
		}

		if (help) {
			fprintf(stderr, "\n%s\n", help_message);
			fprintf(stderr, "\nusage: %s\n", usage_message);
			exit(status);
		}
	}

	if (verbose == 1) {
		fprintf(stderr, "\nProgram <%s>\n", program_name);
		fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "Control Parameters:\n");
		if (bottompickmode == MB7K2SS_BOTTOMPICK_BATHYMETRY)
			fprintf(stderr, "     bottompickmode:      Bathymetry\n");
		else if (bottompickmode == MB7K2SS_BOTTOMPICK_ALTITUDE)
			fprintf(stderr, "     bottompickmode:      Altitude\n");
		else if (bottompickmode == MB7K2SS_BOTTOMPICK_ARRIVAL) {
			fprintf(stderr, "     bottompickmode:      Sidescan first arrival\n");
			fprintf(stderr, "     bottompickthreshold: %f\n", bottompickthreshold);
		}
		else if (bottompickmode == MB7K2SS_BOTTOMPICK_3DBATHY) {
			fprintf(stderr, "     bottompickmode:      3D Bathymetry\n");
			fprintf(stderr, "     topogridfile:        %s\n", topogridfile);
		}
		fprintf(stderr, "     bottompickthreshold: %f\n", bottompickthreshold);
		fprintf(stderr, "     smooth:              %d\n", smooth);
		if (swath_width_set)
			fprintf(stderr, "     swath_width:         %f\n", swath_width);
		else
			fprintf(stderr, "     swath_width:         Maximum available\n");
		if (gainmode == MB7K2SS_SSGAIN_OFF)
			fprintf(stderr, "     gainmode:            Off\n");
		else {
			fprintf(stderr, "     gainmode:            TVG applied as gainfactor/R\n");
			fprintf(stderr, "     gainfactor:          %f\n", gainfactor);
		}
		if (sslayoutmode == MB7K2SS_SS_FLAT_BOTTOM)
			fprintf(stderr, "     sslayoutmode:        Flat bottom\n");
		else if (sslayoutmode == MB7K2SS_SS_3D_BOTTOM) {
			fprintf(stderr, "     sslayoutmode:        3D bottom\n");
			fprintf(stderr, "     topogridfile:        %s\n", topogridfile);
		}
		fprintf(stderr, "     interpolation bins:  %d\n", interpbins);
		if (timelist_file_set)
			fprintf(stderr, "     timelist_file:       %s\n", timelist_file);
		if (route_file_set)
			fprintf(stderr, "     route_file:          %s\n", route_file);
		fprintf(stderr, "     checkroutebearing:   %d\n", checkroutebearing);
		if (output_file_set)
			fprintf(stderr, "     output_file:         %s\n", output_file);
		fprintf(stderr, "     lineroot:            %s\n", lineroot);
		fprintf(stderr, "     extract_type:        %d\n", extract_type);
		fprintf(stderr, "     print_comments:      %d\n", print_comments);
	}

	/* output output types */
	fprintf(stdout, "\nData records to extract:\n");
	if (extract_type == MB7K2SS_SSLOW)
		fprintf(stdout, "     Low Sidescan\n");
	else if (extract_type == MB7K2SS_SSHIGH)
		fprintf(stdout, "     High Sidescan\n");
	if (ssflip)
		fprintf(stdout, "     Sidescan port and starboard exchanged\n");

	int linenumber = 0;
	/* set starting line number and output file if route read */
	if (route_file_set || timelist_file_set) {
		linenumber = startline;
		if (extract_type == MB7K2SS_SSLOW)
			snprintf(output_file, sizeof(output_file), "%s_%4.4d_sslo.mb71", lineroot, linenumber);
		else if (extract_type == MB7K2SS_SSHIGH)
			snprintf(output_file, sizeof(output_file), "%s_%4.4d_sshi.mb71", lineroot, linenumber);
	}

	/* new output file obviously needed */
	new_output_file = true;

	double *routelon = nullptr;
	double *routelat = nullptr;
	double *routeheading = nullptr;
	int *routewaypoint = nullptr;
	double *routetime_d = nullptr;
	int ntimepoint = 0;
	bool linechange;
	double mtodeglon;
	double mtodeglat;
	int activewaypoint = 0;
	double rangelast = 0.0;  // TODO(schwehr): Might not be always set correctly.
	int oktowrite;
	double topo;
	int nroutepoint = 0;
	int nroutepointalloc = 0;
	double time_d;
	char comment[MB_COMMENT_MAXLINE];
	double heading;

	/* if specified read route time list file */
	if (timelist_file_set) {
		/* open the input file */
		FILE *fp = fopen(timelist_file, "r");
		if (fp == nullptr) {
			error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
			fprintf(stderr, "\nUnable to open time list file <%s> for reading\n", timelist_file);
			exit(status);
		}
		int ntimepointalloc = 0;
		char *result;
		while ((result = fgets(comment, MB_PATH_MAXLINE, fp)) == comment) {
			if (comment[0] != '#') {
				int i;
				int waypoint;
				double lon;
				double lat;
				/* const int nget = */
				sscanf(comment, "%d %d %lf %lf %lf %lf", &i, &waypoint, &lon, &lat, &heading, &time_d);

				/* if good data check for need to allocate more space */
				if (ntimepoint + 1 > ntimepointalloc) {
					ntimepointalloc += MB7K2SS_ALLOC_NUM;
					status =
					    mb_reallocd(verbose, __FILE__, __LINE__, ntimepointalloc * sizeof(double), (void **)&routelon, &error);
					status &=
					    mb_reallocd(verbose, __FILE__, __LINE__, ntimepointalloc * sizeof(double), (void **)&routelat, &error);
					status &= mb_reallocd(verbose, __FILE__, __LINE__, ntimepointalloc * sizeof(double), (void **)&routeheading,
					                     &error);
					status &=
					    mb_reallocd(verbose, __FILE__, __LINE__, ntimepointalloc * sizeof(int), (void **)&routewaypoint, &error);
					status &=
					    mb_reallocd(verbose, __FILE__, __LINE__, ntimepointalloc * sizeof(double), (void **)&routetime_d, &error);
					if (status != MB_SUCCESS) {
						char *message;
						mb_error(verbose, error, &message);
						fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
						fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
						exit(error);
					}
				}

				/* add good point to route */
				if (ntimepointalloc > ntimepoint) {
					routewaypoint[ntimepoint] = waypoint;
					routelon[ntimepoint] = lon;
					routelat[ntimepoint] = lat;
					routeheading[ntimepoint] = heading;
					routetime_d[ntimepoint] = time_d;
					ntimepoint++;
				}
			}
		}

		/* close the file */
		fclose(fp);
		fp = nullptr;

		activewaypoint = 1;
		mb_coor_scale(verbose, routelat[activewaypoint], &mtodeglon, &mtodeglat);
		rangelast = 1000 * rangethreshold;
		oktowrite = 0;
		linechange = false;

		/* output status */
		if (verbose > 0) {
			/* output info on file output */
			fprintf(stderr, "Read %d waypoints from time list file: %s\n", ntimepoint, timelist_file);
		}
	}

	/* if specified read route file */
	else if (route_file_set) {
		/* open the input file */
		FILE *fp = nullptr;
		if ((fp = fopen(route_file, "r")) == nullptr) {
			error = MB_ERROR_OPEN_FAIL;
			status = MB_FAILURE;
			fprintf(stderr, "\nUnable to open route file <%s> for reading\n", route_file);
			exit(status);
		}
		bool rawroutefile = false;
		char *result;
		while ((result = fgets(comment, MB_PATH_MAXLINE, fp)) == comment) {
			if (comment[0] == '#') {
				if (strncmp(comment, "## Route File Version", 21) == 0) {
					rawroutefile = false;
				}
			}
			else {
				int waypoint_tmp;
				double lon;
				double lat;
				const int nget = sscanf(comment, "%lf %lf %lf %d %lf", &lon, &lat, &topo, &waypoint_tmp, &heading);
				const waypoint_t waypoint = (waypoint_t)waypoint_tmp;
				if (comment[0] == '#') {
					fprintf(stderr, "buffer:%s", comment);
					if (strncmp(comment, "## Route File Version", 21) == 0) {
						rawroutefile = false;
					}
				}
				const bool point_ok =
					(rawroutefile && nget >= 2) ||
					(!rawroutefile && nget >= 3 && waypoint > MB7K2SS_ROUTE_WAYPOINT_NONE);

				/* if good data check for need to allocate more space */
				if (point_ok && nroutepoint + 1 > nroutepointalloc) {
					nroutepointalloc += MB7K2SS_ALLOC_NUM;
					status =
					    mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double), (void **)&routelon, &error);
					status &=
					    mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double), (void **)&routelat, &error);
					status &= mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double), (void **)&routeheading,
					                     &error);
					status &=
					    mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(int), (void **)&routewaypoint, &error);
					if (status != MB_SUCCESS) {
						char *message;
						mb_error(verbose, error, &message);
						fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
						fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
						exit(error);
					}
				}

				/* add good point to route */
				if (point_ok && nroutepointalloc > nroutepoint + 1) {
					routelon[nroutepoint] = lon;
					routelat[nroutepoint] = lat;
					routeheading[nroutepoint] = heading;
					routewaypoint[nroutepoint] = waypoint;
					nroutepoint++;
				}
			}
		}

		/* close the file */
		fclose(fp);
		fp = nullptr;

		/* set starting values */
		activewaypoint = 1;
		mb_coor_scale(verbose, routelat[activewaypoint], &mtodeglon, &mtodeglat);
		rangelast = 1000 * rangethreshold;
		oktowrite = 0;
		linechange = false;

		/* output status */
		if (verbose > 0) {
			/* output info on file output */
			fprintf(stderr, "\nImported %d waypoints from route file: %s\n", nroutepoint, route_file);
		}
	}

	/* read topography grid if 3D bottom correction specified */
	void *topogrid_ptr = nullptr;
	if (sslayoutmode == MB7K2SS_SS_3D_BOTTOM) {
		status = mb_topogrid_init(verbose, topogridfile, &lonflip, &topogrid_ptr, &error);
	}
	if (error != MB_ERROR_NO_ERROR) {
		char *message;
		mb_error(verbose, error, &message);
		fprintf(stderr, "\nMBIO Error loading topography grid: %s\n%s\n", topogridfile, message);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		mb_memory_clear(verbose, &error);
		exit(error);
	}

	/* set up plotting script file */
	char scriptfile[MB_PATH_MAXLINE+20];
	if ((route_file_set && nroutepoint > 1) || (timelist_file_set && ntimepoint > 1)) {
		snprintf(scriptfile, sizeof(scriptfile), "%s_ssswathplot.cmd", lineroot);
	}
	else if (!output_file_set || read_datalist) {
		snprintf(scriptfile, sizeof(scriptfile), "%s_ssswathplot.cmd", read_file);
	}
	else {
		snprintf(scriptfile, sizeof(scriptfile), "%s_ssswathplot.cmd", file);
	}
	FILE *sfp = fopen(scriptfile, "w");
	if (sfp == nullptr) {
		error = MB_ERROR_OPEN_FAIL;
		status = MB_FAILURE;
		fprintf(stderr, "\nUnable to open plotting script file <%s> \n", scriptfile);
		exit(status);
	}

	/* get format if required */
	if (format == 0)
		mb_get_format(verbose, read_file, nullptr, &format, &error);

	/* determine whether to read one file or a list of files */
	if (format < 0)
		read_datalist = true;

	bool read_data;
	void *datalist;

	/* open file list */
	if (read_datalist) {
		const int look_processed = MB_DATALIST_LOOK_YES;
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

	void *imbio_ptr = nullptr;
	struct mb_io_struct *imb_io_ptr = nullptr;
	int nreaddata = 0;
	int nreaddatatot = 0;

	char *beamflag = nullptr;
	double *bath = nullptr;
	double *amp = nullptr;
	double *ss = nullptr;
	double *bathacrosstrack = nullptr;
	double *bathalongtrack = nullptr;
	double *ssacrosstrack = nullptr;
	double *ssalongtrack = nullptr;
	double *ttimes = nullptr;
	double *angles = nullptr;
	double *angles_forward = nullptr;
	double *angles_null = nullptr;
	double *bheave = nullptr;
	double *alongtrack_offset = nullptr;

	int kind;
	int time_i[7];
	double navlon;
	double navlat;
	double speed;
	double distance;
	double altitude;
	double sonardepth;
	double roll;
	double pitch;
	double heave;
	double draft;

	/* synchronous navigation, heading, attitude data (from multibeam bathymetry records) */
	int ndat = 0;
	int ndat_alloc = 0;
	double *dat_time_d = nullptr;
	double *dat_lon = nullptr;
	double *dat_lat = nullptr;
	double *dat_speed = nullptr;
	double *dat_sonardepth = nullptr;
	double *dat_heading = nullptr;
	double *dat_draft = nullptr;
	double *dat_roll = nullptr;
	double *dat_pitch = nullptr;
	double *dat_heave = nullptr;
	double *dat_altitude = nullptr;

	/* first read and store all navigation, attitude, heading, sonar depth, and altitude
	   data from the survey (multibeam) records - loop over all files to be read
	   - use fbt files if available */

	while (read_data && format == MBF_RESON7KR) {
		/* use fbt file if available as source for processed navigation and attitude */
		mb_get_fbt(verbose, file, &format, &error);

		/* initialize reading the swath file */
		if (mb_read_init(verbose, file, format, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, &imbio_ptr,
		                           &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
			char *message;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
			fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", file);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}

		imb_io_ptr = (struct mb_io_struct *)imbio_ptr;
		void *istore_ptr = imb_io_ptr->store_data;
		nreaddata = 0;

		if (error == MB_ERROR_NO_ERROR) {
			beamflag = nullptr;
			bath = nullptr;
			amp = nullptr;
			bathacrosstrack = nullptr;
			bathalongtrack = nullptr;
			ss = nullptr;
			ssacrosstrack = nullptr;
			ssalongtrack = nullptr;
		}
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
		if (error == MB_ERROR_NO_ERROR)
			status =
			    mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status =
			    mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&ttimes, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&angles, &error);
		if (error == MB_ERROR_NO_ERROR)
			status =
			    mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&angles_forward, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&angles_null, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bheave, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&alongtrack_offset,
			                           &error);

		/* if error initializing memory then quit */
		if (error != MB_ERROR_NO_ERROR) {
			char *message;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}

		/* loop over reading data from current file */
		while (error <= MB_ERROR_NO_ERROR) {
			/* reset error */
			error = MB_ERROR_NO_ERROR;

			/* read next data record */
			status &= mb_get_all(verbose, imbio_ptr, &istore_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading,
			                    &distance, &altitude, &sonardepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp,
			                    bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);

			/* reset nonfatal errors */
			if (kind == MB_DATA_DATA && error < 0) {
				status = MB_SUCCESS;
				error = MB_ERROR_NO_ERROR;
			}

			/* get desired information */
			if (status == MB_SUCCESS && kind == MB_DATA_DATA) {
				status = mb_extract_nav(verbose, imbio_ptr, istore_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed,
				                        &heading, &draft, &roll, &pitch, &heave, &error);

				/* allocate memory for altitude arrays if needed */
				if (ndat + 1 >= ndat_alloc) {
					ndat_alloc += MB7K2SS_ALLOC_CHUNK;
					status = mb_reallocd(verbose, __FILE__, __LINE__, ndat_alloc * sizeof(double), (void **)&dat_time_d, &error);
					status &= mb_reallocd(verbose, __FILE__, __LINE__, ndat_alloc * sizeof(double), (void **)&dat_lon, &error);
					status &= mb_reallocd(verbose, __FILE__, __LINE__, ndat_alloc * sizeof(double), (void **)&dat_lat, &error);
					status &= mb_reallocd(verbose, __FILE__, __LINE__, ndat_alloc * sizeof(double), (void **)&dat_speed, &error);
					status &=
					    mb_reallocd(verbose, __FILE__, __LINE__, ndat_alloc * sizeof(double), (void **)&dat_sonardepth, &error);
					status &= mb_reallocd(verbose, __FILE__, __LINE__, ndat_alloc * sizeof(double), (void **)&dat_heading, &error);
					status &= mb_reallocd(verbose, __FILE__, __LINE__, ndat_alloc * sizeof(double), (void **)&dat_draft, &error);
					status &= mb_reallocd(verbose, __FILE__, __LINE__, ndat_alloc * sizeof(double), (void **)&dat_roll, &error);
					status &= mb_reallocd(verbose, __FILE__, __LINE__, ndat_alloc * sizeof(double), (void **)&dat_pitch, &error);
					status &= mb_reallocd(verbose, __FILE__, __LINE__, ndat_alloc * sizeof(double), (void **)&dat_heave, &error);
					status &=
					    mb_reallocd(verbose, __FILE__, __LINE__, ndat_alloc * sizeof(double), (void **)&dat_altitude, &error);
					if (error != MB_ERROR_NO_ERROR) {
						char *message;
						mb_error(verbose, error, &message);
						fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
						fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
						exit(error);
					}
				}

				/* store the altitude data */
				if (ndat == 0 || dat_time_d[ndat - 1] < time_d) {
					dat_time_d[ndat] = time_d;
					dat_lon[ndat] = navlon;
					dat_lat[ndat] = navlat;
					dat_speed[ndat] = speed;
					dat_sonardepth[ndat] = sonardepth;
					dat_heading[ndat] = heading;
					dat_draft[ndat] = draft;
					dat_roll[ndat] = roll;
					dat_pitch[ndat] = pitch;
					dat_heave[ndat] = heave;
					dat_altitude[ndat] = altitude;
					ndat++;
					nreaddata++;
				}
			}
		}

		/* close the swath file */
		status = mb_close(verbose, &imbio_ptr, &error);

		/* output counts */
		fprintf(stdout, "Read %6d nav and attitude data from: %s\n", nreaddata, file);
		nreaddatatot += nreaddata;

		/* figure out whether and what to read next */
		if (read_datalist) {
			read_data = mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error) == MB_SUCCESS;
		} else {
			read_data = false;
		}
	}  // end loop over files in list

	if (read_datalist)
		mb_datalist_close(verbose, &datalist, &error);

	/* output counts */
	fprintf(stdout, "\nRead %6d nav and attitude data from: %s\n", nreaddatatot, read_file);
	nreaddatatot = 0;
	nreaddata = 0;

	/* open file list */
	if (read_datalist) {
		const int look_processed = MB_DATALIST_LOOK_YES;
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

	void *ombio_ptr = nullptr;
	struct mb_io_struct *omb_io_ptr = nullptr;
	void *ostore_ptr = nullptr;
	struct mbsys_ldeoih_struct *ostore = nullptr;
	double ssv;
	double ssv_use = 1500.0;

	int icomment = 0;

	/* sidescan data data */
	s7k_fsdwchannel *sschannelport; /* Port hannel header and data */
	s7k_fsdwssheader *ssheaderport; /* Port Edgetech sidescan header */
	s7k_fsdwchannel *sschannelstbd; /* Starboard channel header and data */
	s7k_fsdwssheader *ssheaderstbd; /* Starboard Edgetech sidescan header */

	/* output sidescan data */
	int obeams_bath;
	int obeams_amp;
	int opixels_ss;
	double oss[MB7K2SS_SSDIMENSION];
	double ossacrosstrack[MB7K2SS_SSDIMENSION];
	double ossalongtrack[MB7K2SS_SSDIMENSION];
	int ossbincount[MB7K2SS_SSDIMENSION];

	/* sidescan layout mode */
	double ss_altitude = 0.0;  // TODO(schwehr): Might not always be set correctly.

	/* route and auto-line data */
	double range = 0.0;  // TODO(schwehr): Might not always be set correctly.

	/* bottom layout parameters */
	int nangle = MB7K2SS_NUM_ANGLES;
	double angle_min = -MB7K2SS_ANGLE_MAX;
	double angle_max = MB7K2SS_ANGLE_MAX;
	double table_angle[MB7K2SS_NUM_ANGLES];
	double table_xtrack[MB7K2SS_NUM_ANGLES];
	double table_ltrack[MB7K2SS_NUM_ANGLES];
	double table_altitude[MB7K2SS_NUM_ANGLES];
	double table_range[MB7K2SS_NUM_ANGLES];

	/* counting variables */
	int nwritesslo = 0;
	int nwritesshi = 0;
	int nreadheadertot = 0;
	int nreadssvtot = 0;
	int nreadnav1tot = 0;
	int nreadsbptot = 0;
	int nreadsslotot = 0;
	int nreadsshitot = 0;
	int nwritesslotot = 0;
	int nwritesshitot = 0;

	int format_guess;
	const int format_output = MBF_MBLDEOIH;
	double ttime;
	double ttime_min;
	double ttime_min_use = 0.0;  // TODO(schwehr): Might not be properly set in all cases.

	/* loop over all files to be read */
	while (read_data && format == MBF_RESON7KR) {

		/* initialize reading the swath file */
		if (mb_read_init(verbose, file, format, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, &imbio_ptr,
		                           &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
			char *message;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
			fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", file);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}

		/* get pointers to data storage */
		imb_io_ptr = (struct mb_io_struct *)imbio_ptr;
		void *istore_ptr = imb_io_ptr->store_data;
		struct mbsys_reson7k_struct *istore = (struct mbsys_reson7k_struct *)istore_ptr;
		int itime = 0;

		if (error == MB_ERROR_NO_ERROR) {
			beamflag = nullptr;
			bath = nullptr;
			amp = nullptr;
			bathacrosstrack = nullptr;
			bathalongtrack = nullptr;
			ss = nullptr;
			ssacrosstrack = nullptr;
			ssalongtrack = nullptr;
		}
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
		if (error == MB_ERROR_NO_ERROR)
			status =
			    mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status =
			    mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&ttimes, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&angles, &error);
		if (error == MB_ERROR_NO_ERROR)
			status =
			    mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&angles_forward, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&angles_null, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bheave, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&alongtrack_offset,
			                           &error);

		/* if error initializing memory then quit */
		if (error != MB_ERROR_NO_ERROR) {
			char *message;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}

		/* set up output file name if needed */
		if (error == MB_ERROR_NO_ERROR) {
			if (output_file_set && ombio_ptr == nullptr) {
				/* set flag to open new output file */
				new_output_file = true;
			}

			else if (!output_file_set && !route_file_set && !timelist_file_set) {
				new_output_file = true;
				const int format_status = mb_get_format(verbose, file, output_file, &format_guess, &error);
				if (format_status != MB_SUCCESS || format_guess != format) {
					strcpy(output_file, file);
				}
				if (output_file[strlen(output_file) - 1] == 'p') {
					output_file[strlen(output_file) - 1] = '\0';
				}
				if (extract_type == MB7K2SS_SSLOW) {
					strcat(output_file, "_sslo.mb71");
					// format_output = MBF_MBLDEOIH;
				}
				else if (extract_type == MB7K2SS_SSHIGH) {
					strcat(output_file, "_sshi.mb71");
					// format_output = MBF_MBLDEOIH;
				}
			}
		}

		/* read and print data */
		nreaddata = 0;
		int nreadheader = 0;
		int nreadssv = 0;
		int nreadnav1 = 0;
		int nreadsbp = 0;
		int nreadsslo = 0;
		int nreadsshi = 0;
		bool ttime_min_ok = false;

		while (error <= MB_ERROR_NO_ERROR) {
			/* reset error */
			error = MB_ERROR_NO_ERROR;

			/* read next data record */
			status = mb_get_all(verbose, imbio_ptr, &istore_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading,
			                    &distance, &altitude, &sonardepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp,
			                    bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);

			/* reset nonfatal errors */
			if (error < 0) {
				status = MB_SUCCESS;
				error = MB_ERROR_NO_ERROR;
			}

			/* get nav and attitude */
			if (status == MB_SUCCESS &&
			    (kind == MB_DATA_SUBBOTTOM_SUBBOTTOM || kind == MB_DATA_SIDESCAN2 || kind == MB_DATA_SIDESCAN3)) {
				// int intstat;
				/* intstat = */ mb_linear_interp(verbose, dat_time_d - 1, dat_lon - 1, ndat, time_d, &navlon, &itime, &error);
				/* intstat = */ mb_linear_interp(verbose, dat_time_d - 1, dat_lat - 1, ndat, time_d, &navlat, &itime, &error);
				/* intstat = */ mb_linear_interp(verbose, dat_time_d - 1, dat_speed - 1, ndat, time_d, &speed, &itime, &error);
				// intstat =
				mb_linear_interp(verbose, dat_time_d - 1, dat_sonardepth - 1, ndat, time_d, &sonardepth, &itime, &error);
				/* intstat = */ mb_linear_interp(verbose, dat_time_d - 1, dat_heading - 1, ndat, time_d, &heading, &itime, &error);
				/* intstat = */ mb_linear_interp(verbose, dat_time_d - 1, dat_draft - 1, ndat, time_d, &draft, &itime, &error);
				/* intstat = */ mb_linear_interp(verbose, dat_time_d - 1, dat_roll - 1, ndat, time_d, &roll, &itime, &error);
				/* intstat = */ mb_linear_interp(verbose, dat_time_d - 1, dat_pitch - 1, ndat, time_d, &pitch, &itime, &error);
				/* intstat = */ mb_linear_interp(verbose, dat_time_d - 1, dat_heave - 1, ndat, time_d, &heave, &itime, &error);
				/* intstat = */ mb_linear_interp(verbose, dat_time_d - 1, dat_altitude - 1, ndat, time_d, &altitude, &itime, &error);
			}

			/* save last nav and heading */
			/* if (status == MB_SUCCESS && kind == target_kind) { */
			/* 	if (navlon != 0.0) */
			/* 		lastlon = navlon; */
			/* 	if (navlat != 0.0) */
			/* 		lastlat = navlat; */
			/* 	if (heading != 0.0) */
			/* 		lastheading = heading; */
			/* } */

			/* check survey data position against time list or waypoints */
			if (status == MB_SUCCESS && kind == target_kind && navlon != 0.0 && navlat != 0.0) {
				/* to set lines check survey data time against time list */
				if (ntimepoint > 1) {
					const double dx = (navlon - routelon[activewaypoint]) / mtodeglon;
					const double dy = (navlat - routelat[activewaypoint]) / mtodeglat;
					range = sqrt(dx * dx + dy * dy);
					if (activewaypoint < ntimepoint && time_d >= routetime_d[activewaypoint]) {
						linechange = true;
					}
				}

				/* else to set lines check survey data position against waypoints */
				else if (nroutepoint > 1 && navlon != 0.0 && navlat != 0.0) {
					const double dx = (navlon - routelon[activewaypoint]) / mtodeglon;
					const double dy = (navlat - routelat[activewaypoint]) / mtodeglat;
					range = sqrt(dx * dx + dy * dy);
					if (range < rangethreshold && (activewaypoint == 0 || range > rangelast) &&
					    activewaypoint < nroutepoint - 1) {
						linechange = true;
					}
				}

				/* check survey data position against waypoints */
				if (linechange) {
					/* increment line number */
					linenumber++;

					/* set output file name */
					if (extract_type == MB7K2SS_SSLOW)
						snprintf(output_file, sizeof(output_file), "%s_%4.4d_sslo.mb71", lineroot, linenumber);
					else if (extract_type == MB7K2SS_SSHIGH)
						snprintf(output_file, sizeof(output_file), "%s_%4.4d_sshi.mb71", lineroot, linenumber);
					// format_output = MBF_MBLDEOIH;

					/* set to open new output file */
					new_output_file = true;

					/* increment active waypoint */
					activewaypoint++;
					mb_coor_scale(verbose, routelat[activewaypoint], &mtodeglon, &mtodeglat);
					rangelast = 1000 * rangethreshold;
					oktowrite = 0;
					linechange = false;
				}
				else
					rangelast = range;
			}

			if (kind == MB_DATA_DATA && error <= MB_ERROR_NO_ERROR) {
				/* extract travel times */
				status = mb_ttimes(verbose, imbio_ptr, istore_ptr, &kind, &beams_bath, ttimes, angles, angles_forward,
				                   angles_null, bheave, alongtrack_offset, &draft, &ssv, &error);

				/* check surface sound velocity */
				if (ssv > 0.0)
					ssv_use = ssv;

				/* get bottom arrival time, if possible */
				ttime_min = 0.0;
				bool found = false;
				for (int i = 0; i < beams_bath; i++) {
					if (mb_beam_ok(beamflag[i])) {
						if (!found || ttimes[i] < ttime_min) {
							ttime_min = ttimes[i];
							/* nadir_depth = bath[i]; */
							/* beam_min = i; */
							found = true;
						}
					}
				}
				if (found) {
					ttime_min_use = ttime_min;
					ttime_min_ok = true;
				}
			}

			/* nonfatal errors do not matter */
			if (error < MB_ERROR_NO_ERROR) {
				error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
			}

			/* if needed open new output file */
			if (status == MB_SUCCESS && new_output_file &&
			    (((extract_type == MB7K2SS_SSLOW && kind == MB_DATA_SIDESCAN2) ||
			      (extract_type == MB7K2SS_SSHIGH && kind == MB_DATA_SIDESCAN3)))) {

				/* close any old output file unless a single file has been specified */
				if (ombio_ptr != nullptr) {
					/* close the swath file */
					status = mb_close(verbose, &ombio_ptr, &error);

					/* generate inf file */
					if (status == MB_SUCCESS) {
						status = mb_make_info(verbose, true, current_output_file, format_output, &error);
					}

					/* output counts */
					fprintf(stdout, "\nData records written to: %s\n", current_output_file);
					fprintf(stdout, "     Low Sidescan:  %d\n", nwritesslo);
					fprintf(stdout, "     High Sidescan: %d\n", nwritesshi);
					nwritesslotot += nwritesslo;
					nwritesshitot += nwritesshi;

					/* output commands to first cut plotting script file */
					fprintf(sfp, "# Generate swath plot of sidescan file: %s\n", current_output_file);
					fprintf(sfp, "mbm_plot -I %s -N -G5 -S -Pb -V -O %s_ssrawplot\n", current_output_file, current_output_file);
					fprintf(sfp, "%s_ssrawplot.cmd\n\n", current_output_file);
				}

				/* open the new file */
				nwritesslo = 0;
				nwritesshi = 0;
				if ((status &= mb_write_init(verbose, output_file, MBF_MBLDEOIH, &ombio_ptr, &obeams_bath, &obeams_amp,
				                            &opixels_ss, &error)) != MB_SUCCESS) {
					char *message;
					mb_error(verbose, error, &message);
					fprintf(stderr, "\nMBIO Error returned from function <mb_write_init>:\n%s\n", message);
					fprintf(stderr, "\nMultibeam File <%s> not initialized for writing\n", output_file);
					fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
					exit(error);
				}

				/* save current_output_file */
				strcpy(current_output_file, output_file);

				/* get pointers to data storage */
				omb_io_ptr = (struct mb_io_struct *)ombio_ptr;
				ostore_ptr = omb_io_ptr->store_data;
				ostore = (struct mbsys_ldeoih_struct *)ostore_ptr;

				/* reset new_output_file */
				new_output_file = false;
			}

			/* if following a route check that the vehicle has come on line
			        (within MB7K2SS_ONLINE_THRESHOLD degrees)
			        before writing any data */
			if (checkroutebearing && nroutepoint > 1 && activewaypoint > 0) {
				double headingdiff = fabs(routeheading[activewaypoint - 1] - heading);
				if (headingdiff > 180.0)
					headingdiff = 360.0 - headingdiff;
				if (headingdiff < MB7K2SS_ONLINE_THRESHOLD)
					oktowrite++;
				else
					oktowrite = 0;
			}
			else
				oktowrite = MB7K2SS_ONLINE_COUNT;

			/* handle multibeam data */
			if (status == MB_SUCCESS && kind == MB_DATA_DATA) {
				nreaddata++;
			}

			/* handle file header data */
			else if (status == MB_SUCCESS && kind == MB_DATA_HEADER) {
				nreadheader++;
			}

			/* handle bluefin ctd data */
			else if (status == MB_SUCCESS && kind == MB_DATA_SSV) {
				nreadssv++;
			}

			/* handle bluefin nav data */
			else if (status == MB_SUCCESS && kind == MB_DATA_NAV2) {
				nreadnav1++;
			}

			/* handle subbottom data */
			else if (status == MB_SUCCESS && kind == MB_DATA_SUBBOTTOM_SUBBOTTOM) {
				nreadsbp++;
			}

			/* handle low frequency sidescan data */
			else if (status == MB_SUCCESS && kind == MB_DATA_SIDESCAN2) {
				nreadsslo++;

				/* output data if desired */
				if (extract_type == MB7K2SS_SSLOW && nreadnav1 > 0 && oktowrite >= MB7K2SS_ONLINE_COUNT) {
					/* get channels */
					if (ssflip) {
						sschannelport = (s7k_fsdwchannel *)&(istore->fsdwsslo.channel[1]);
						ssheaderport = (s7k_fsdwssheader *)&(istore->fsdwsslo.ssheader[1]);
						sschannelstbd = (s7k_fsdwchannel *)&(istore->fsdwsslo.channel[0]);
						ssheaderstbd = (s7k_fsdwssheader *)&(istore->fsdwsslo.ssheader[0]);
					}
					else {
						sschannelport = (s7k_fsdwchannel *)&(istore->fsdwsslo.channel[0]);
						ssheaderport = (s7k_fsdwssheader *)&(istore->fsdwsslo.ssheader[0]);
						sschannelstbd = (s7k_fsdwchannel *)&(istore->fsdwsslo.channel[1]);
						ssheaderstbd = (s7k_fsdwssheader *)&(istore->fsdwsslo.ssheader[1]);
					}

					/* set some values */
					ostore->depth_scale = 0;
					ostore->distance_scale = 0;
					ostore->beam_xwidth = 0.9;
					ostore->beam_lwidth = 0.9;
					ostore->kind = MB_DATA_DATA;
					ostore->ss_type = MB_SIDESCAN_LINEAR; /* sets sidescan to be scaled linear */
					opixels_ss = MB7K2SS_SSDIMENSION;

					/* reset the sonar altitude using the specified mode */
					if (bottompickmode == MB7K2SS_BOTTOMPICK_ARRIVAL) {
						/* get bottom arrival in port trace */
						unsigned short *datashort = (unsigned short *)sschannelport->data;
						double channelmax = 0.0;
						for (unsigned int i = 0; i < ssheaderport->samples; i++) {
							const double value =
								ssheaderport->dataFormat == EDGETECH_TRACEFORMAT_ANALYTIC
								? sqrt(
								    (double)(datashort[2 * i] * datashort[2 * i] + datashort[2 * i + 1] * datashort[2 * i + 1]))
								: (double)(datashort[i]);
							channelmax = std::max(value, channelmax);
						}
						int portchannelpick = 0;
						double threshold = bottompickthreshold * channelmax;
						for (unsigned int i = 0; i < ssheaderport->samples && portchannelpick == 0; i++) {
							const double value =
								ssheaderport->dataFormat == EDGETECH_TRACEFORMAT_ANALYTIC
								? sqrt(
								    (double)(datashort[2 * i] * datashort[2 * i] + datashort[2 * i + 1] * datashort[2 * i + 1]))
								: (double)(datashort[i]);
							if (value >= threshold)
								portchannelpick = i;
						}

						/* get bottom arrival in starboard trace */
						datashort = (unsigned short *)sschannelstbd->data;
						channelmax = 0.0;
						for (unsigned int i = 0; i < ssheaderstbd->samples; i++) {
							const double value =
								ssheaderstbd->dataFormat == EDGETECH_TRACEFORMAT_ANALYTIC
								? sqrt(
								    (double)(datashort[2 * i] * datashort[2 * i] + datashort[2 * i + 1] * datashort[2 * i + 1]))
								: (double)(datashort[i]);
							channelmax = std::max(value, channelmax);
						}
						int stbdchannelpick = 0;
						threshold = bottompickthreshold * channelmax;
						for (unsigned int i = 0; i < ssheaderstbd->samples && stbdchannelpick == 0; i++) {
							const double value =
								ssheaderstbd->dataFormat == EDGETECH_TRACEFORMAT_ANALYTIC
								? sqrt(
								    (double)(datashort[2 * i] * datashort[2 * i] + datashort[2 * i + 1] * datashort[2 * i + 1]))
								: (double)(datashort[i]);
							if (value >= threshold)
								stbdchannelpick = i;
						}

						/* set sonar altitude */
						ttime = 0.0000000005 * ((portchannelpick + stbdchannelpick) * ssheaderport->sampleInterval);
						ss_altitude = 0.5 * ssv_use * ttime;
					}
					else if (bottompickmode == MB7K2SS_BOTTOMPICK_BATHYMETRY) {
						if (ttime_min_ok) {
							ss_altitude = 0.5 * ssv_use * ttime_min_use;
						}
					}

					/* else if getting altitude from topography model set initial value zero */
					else if (bottompickmode == MB7K2SS_BOTTOMPICK_3DBATHY) {
						mb_topogrid_topo(verbose, topogrid_ptr, navlon, navlat, &topo, &error);
						ss_altitude = -sonardepth - topo;
					}

					/* else use the altitude we already have */
					else {
						ss_altitude = altitude;
					}

					/* get flat bottom layout table */
					if (sslayoutmode == MB7K2SS_SS_FLAT_BOTTOM)
						mb7k2ss_get_flatbottom_table(verbose, nangle, angle_min, angle_max, navlon, navlat, ss_altitude, 0.0,
						                             table_angle, table_xtrack, table_ltrack, table_altitude, table_range,
						                             &error);
					/* else get 3D bottom layout table */
					else
						mb_topogrid_getangletable(verbose, topogrid_ptr, nangle, angle_min, angle_max, navlon, navlat, heading,
						                          ss_altitude, sonardepth, pitch, table_angle, table_xtrack, table_ltrack,
						                          table_altitude, table_range, &error);

					/* get swath width and pixel size */
					double rr = 0.0000000005 * ssv_use * (ssheaderport->samples * ssheaderport->sampleInterval);
					if (!swath_width_set)
						swath_width = 2.2 * sqrt(rr * rr - ss_altitude * ss_altitude);
					const double pixel_width = swath_width / (opixels_ss - 1);

					/* initialize the output sidescan */

					for (int j = 0; j < opixels_ss; j++) {
						oss[j] = 0.0;
						ossacrosstrack[j] = pixel_width * (double)(j - (opixels_ss / 2));
						ossalongtrack[j] = 0.0;
						ossbincount[j] = 0;
					}

					/* find minimum range */
					double rangemin = table_range[0];
					int kstart = 0;
					for (int kangle = 1; kangle < nangle; kangle++) {
						if (table_range[kangle] < rangemin) {
							rangemin = table_range[kangle];
							kstart = kangle;
						}
					}

					/* bin port trace */
					unsigned short *datashort = (unsigned short *)sschannelport->data;
					// istart = ss_altitude / (0.0000000005 * ssv_use * ssheaderport->sampleInterval);
					int istart = rangemin / (0.0000000005 * ssv_use * ssheaderport->sampleInterval);
					double weight = exp(MB_LN_2 * ((double)ssheaderport->weightingFactor));
					for (unsigned int i = istart; i < ssheaderport->samples; i++) {
						const double value =
							ssheaderport->dataFormat == EDGETECH_TRACEFORMAT_ANALYTIC
							? sqrt((double)(datashort[2 * i] * datashort[2 * i] + datashort[2 * i + 1] * datashort[2 * i + 1]))
							: (double)(datashort[i]);

						/* get sample range */
						rr = 0.0000000005 * ssv_use * (i * ssheaderport->sampleInterval);

						/* look up position(s) for this range */
						bool done = false;
						for (int kangle = kstart; kangle > 0 && !done; kangle--) {
							bool found = false;
							double xtrack;
							double ltrack;
							if (rr <= table_range[kstart]) {
								xtrack = table_xtrack[kstart];
								ltrack = table_ltrack[kstart];
								done = true;
								found = true;
							}
							else if (rr > table_range[kangle] && rr <= table_range[kangle - 1]) {
								const double factor = (rr - table_range[kangle]) / (table_range[kangle - 1] - table_range[kangle]);
								xtrack = table_xtrack[kangle] + factor * (table_xtrack[kangle - 1] - table_xtrack[kangle]);
								ltrack = table_ltrack[kangle] + factor * (table_ltrack[kangle - 1] - table_ltrack[kangle]);
								found = true;
								done = true;
							}
							else if (rr < table_range[kangle] && rr >= table_range[kangle - 1]) {
								const double factor = (rr - table_range[kangle]) / (table_range[kangle - 1] - table_range[kangle]);
								xtrack = table_xtrack[kangle] + factor * (table_xtrack[kangle - 1] - table_xtrack[kangle]);
								ltrack = table_ltrack[kangle] + factor * (table_ltrack[kangle - 1] - table_ltrack[kangle]);
								found = true;
								done = true;
							}

							/* bin the value and position */
							if (found) {
								const int j = opixels_ss / 2 + (int)(xtrack / pixel_width);
								if (j >= 0 && j < opixels_ss) {
									oss[j] += value / weight;
									ossbincount[j]++;
									ossalongtrack[j] += ltrack;
								}
							}
						}
					}

					/* find minimum range */
					rangemin = table_range[0];
					kstart = 0;
					for (int kangle = 1; kangle < nangle; kangle++) {
						if (table_range[kangle] < rangemin) {
							rangemin = table_range[kangle];
							kstart = kangle;
						}
					}

					/* bin stbd trace */
					datashort = (unsigned short *)sschannelstbd->data;
					// istart = ss_altitude / (0.0000000005 * ssv_use * ssheaderstbd->sampleInterval);
					istart = rangemin / (0.0000000005 * ssv_use * ssheaderstbd->sampleInterval);
					weight = exp(MB_LN_2 * ((double)ssheaderstbd->weightingFactor));
					for (unsigned int i = istart; i < ssheaderstbd->samples; i++) {
						const double value =
							ssheaderstbd->dataFormat == EDGETECH_TRACEFORMAT_ANALYTIC
							? sqrt((double)(datashort[2 * i] * datashort[2 * i] + datashort[2 * i + 1] * datashort[2 * i + 1]))
							: (double)(datashort[i]);

						/* get sample range */
						rr = 0.0000000005 * ssv_use * (i * ssheaderstbd->sampleInterval);

						/* look up position for this range */
						bool done = false;
						for (int kangle = kstart; kangle < nangle - 1 && !done; kangle++) {
							bool found = false;
							double xtrack;
							double ltrack;
							if (rr <= table_range[kstart]) {
								xtrack = table_xtrack[kstart];
								ltrack = table_ltrack[kstart];

								found = true;
								done = true;
							}
							else if (rr > table_range[kangle] && rr <= table_range[kangle + 1]) {
								const double factor = (rr - table_range[kangle]) / (table_range[kangle + 1] - table_range[kangle]);
								xtrack = table_xtrack[kangle] + factor * (table_xtrack[kangle + 1] - table_xtrack[kangle]);
								ltrack = table_ltrack[kangle] + factor * (table_ltrack[kangle + 1] - table_ltrack[kangle]);
								found = true;
								done = true;
							}
							else if (rr < table_range[kangle] && rr >= table_range[kangle + 1]) {
								const double factor = (rr - table_range[kangle]) / (table_range[kangle + 1] - table_range[kangle]);
								xtrack = table_xtrack[kangle] + factor * (table_xtrack[kangle + 1] - table_xtrack[kangle]);
								ltrack = table_ltrack[kangle] + factor * (table_ltrack[kangle + 1] - table_ltrack[kangle]);
								found = true;
								done = true;
							}

							/* bin the value and position */
							if (found) {
								const int j = opixels_ss / 2 + (int)(xtrack / pixel_width);
								if (j >= 0 && j < opixels_ss) {
									oss[j] += value / weight;
									ossbincount[j]++;
									ossalongtrack[j] += ltrack;
								}
							}
						}
					}

					/* calculate the output sidescan */
					int jport = -1;
					for (int j = 0; j < opixels_ss; j++) {
						if (ossbincount[j] > 0) {
							oss[j] /= (double)ossbincount[j];
							ossalongtrack[j] /= (double)ossbincount[j];
							if (jport < 0)
								jport = j;
						}
						else
							oss[j] = MB_SIDESCAN_NULL;
					}

					/* interpolate gaps in the output sidescan */
					int previous = opixels_ss;
					for (int j = 0; j < opixels_ss; j++) {
						if (ossbincount[j] > 0) {
							const int interpable = j - previous - 1;
							if (interpable > 0 && interpable <= interpbins) {
								const double dss = oss[j] - oss[previous];
								const double dssl = ossalongtrack[j] - ossalongtrack[previous];
								for (int jj = previous + 1; jj < j; jj++) {
									const double fraction = ((double)(jj - previous)) / ((double)(j - previous));
									oss[jj] = oss[previous] + fraction * dss;
									ossalongtrack[jj] = ossalongtrack[previous] + fraction * dssl;
								}
							}
							previous = j;
						}
					}

					/* insert data */
					mb_insert_nav(verbose, ombio_ptr, (void *)ostore, time_i, time_d, navlon, navlat, speed, heading, draft, roll,
					              pitch, heave, &error);
					status = mb_insert_altitude(verbose, ombio_ptr, (void *)ostore, sonardepth, ss_altitude, &error);
					status &= mb_insert(verbose, ombio_ptr, (void *)ostore, MB_DATA_DATA, time_i, time_d, navlon, navlat, speed,
					                   heading, beams_bath, beams_amp, opixels_ss, beamflag, bath, amp, bathacrosstrack,
					                   bathalongtrack, oss, ossacrosstrack, ossalongtrack, comment, &error);

					/* write the record */
					nwritesslo++;
					mb_write_ping(verbose, ombio_ptr, (void *)ostore, &error);
				}
			}

			/* handle high frequency sidescan data */
			else if (status == MB_SUCCESS && kind == MB_DATA_SIDESCAN3) {
				nreadsshi++;

				/* output data if desired */
				if (extract_type == MB7K2SS_SSHIGH && nreadnav1 > 0 && oktowrite >= MB7K2SS_ONLINE_COUNT) {
					/* get channels */
					if (ssflip) {
						sschannelport = (s7k_fsdwchannel *)&(istore->fsdwsshi.channel[1]);
						ssheaderport = (s7k_fsdwssheader *)&(istore->fsdwsshi.ssheader[1]);
						sschannelstbd = (s7k_fsdwchannel *)&(istore->fsdwsshi.channel[0]);
						ssheaderstbd = (s7k_fsdwssheader *)&(istore->fsdwsshi.ssheader[0]);
					}
					else {
						sschannelport = (s7k_fsdwchannel *)&(istore->fsdwsshi.channel[0]);
						ssheaderport = (s7k_fsdwssheader *)&(istore->fsdwsshi.ssheader[0]);
						sschannelstbd = (s7k_fsdwchannel *)&(istore->fsdwsshi.channel[1]);
						ssheaderstbd = (s7k_fsdwssheader *)&(istore->fsdwsshi.ssheader[1]);
					}

					/* set some values */
					ostore->depth_scale = 0;
					ostore->distance_scale = 0;
					ostore->beam_xwidth = 0.6;
					ostore->beam_lwidth = 0.6;
					ostore->kind = MB_DATA_DATA;
					ostore->ss_type = MB_SIDESCAN_LINEAR; /* sets sidescan to be scaled linear */
					opixels_ss = MB7K2SS_SSDIMENSION;

					/* reset the sonar altitude using the specified mode */
					if (bottompickmode == MB7K2SS_BOTTOMPICK_ARRIVAL) {
						/* get bottom arrival in port trace */
						unsigned short *datashort = (unsigned short *)sschannelport->data;
						double channelmax = 0.0;
						for (unsigned int i = 0; i < ssheaderport->samples; i++) {
							const double value =
								ssheaderport->dataFormat == EDGETECH_TRACEFORMAT_ANALYTIC
								? sqrt(
								    (double)(datashort[2 * i] * datashort[2 * i] + datashort[2 * i + 1] * datashort[2 * i + 1]))
								: (double)(datashort[i]);
							channelmax = std::max(value, channelmax);
						}
						int portchannelpick = 0;
						double threshold = bottompickthreshold * channelmax;
						for (unsigned int i = 0; i < ssheaderport->samples && portchannelpick == 0; i++) {
							const double value =
								ssheaderport->dataFormat == EDGETECH_TRACEFORMAT_ANALYTIC
								? sqrt(
								    (double)(datashort[2 * i] * datashort[2 * i] + datashort[2 * i + 1] * datashort[2 * i + 1]))
								: (double)(datashort[i]);
							if (value >= threshold)
								portchannelpick = i;
						}

						/* get bottom arrival in starboard trace */
						datashort = (unsigned short *)sschannelstbd->data;
						channelmax = 0.0;
						for (unsigned int i = 0; i < ssheaderstbd->samples; i++) {
							const double value =
								ssheaderstbd->dataFormat == EDGETECH_TRACEFORMAT_ANALYTIC
								? sqrt(
								    (double)(datashort[2 * i] * datashort[2 * i] + datashort[2 * i + 1] * datashort[2 * i + 1]))
								: (double)(datashort[i]);
							channelmax = std::max(value, channelmax);
						}
						int stbdchannelpick = 0;
						threshold = bottompickthreshold * channelmax;
						for (unsigned int i = 0; i < ssheaderstbd->samples && stbdchannelpick == 0; i++) {
							const double value =
								ssheaderstbd->dataFormat == EDGETECH_TRACEFORMAT_ANALYTIC
								? sqrt(
								    (double)(datashort[2 * i] * datashort[2 * i] + datashort[2 * i + 1] * datashort[2 * i + 1]))
								: (double)(datashort[i]);
							if (value >= threshold)
								stbdchannelpick = i;
						}

						/* set sonar altitude */
						ttime = 0.0000000005 * ((portchannelpick + stbdchannelpick) * ssheaderport->sampleInterval);
						ss_altitude = 0.5 * ssv_use * ttime;
					}
					else if (bottompickmode == MB7K2SS_BOTTOMPICK_BATHYMETRY) {
						if (ttime_min_ok) {
							ss_altitude = 0.5 * ssv_use * ttime_min_use;
						}
					}
					/* else use the altitude we already have */
					else {
						ss_altitude = altitude;
					}

					/* get flat bottom layout table */
					if (sslayoutmode == MB7K2SS_SS_FLAT_BOTTOM)
						mb7k2ss_get_flatbottom_table(verbose, nangle, angle_min, angle_max, navlon, navlat, ss_altitude, 0.0,
						                             table_angle, table_xtrack, table_ltrack, table_altitude, table_range,
						                             &error);
					/* else get 3D bottom layout table */
					else
						mb_topogrid_getangletable(verbose, topogrid_ptr, nangle, angle_min, angle_max, navlon, navlat, heading,
						                          ss_altitude, sonardepth, pitch, table_angle, table_xtrack, table_ltrack,
						                          table_altitude, table_range, &error);

					/* get swath width and pixel size */
					double rr = 0.0000000005 * ssv_use * (ssheaderport->samples * ssheaderport->sampleInterval);
					if (!swath_width_set)
						swath_width = 2.2 * sqrt(rr * rr - ss_altitude * ss_altitude);
					const double pixel_width = swath_width / (opixels_ss - 1);

					/* initialize the output sidescan */
					for (int j = 0; j < opixels_ss; j++) {
						oss[j] = 0.0;
						ossacrosstrack[j] = pixel_width * (double)(j - (opixels_ss / 2));
						ossalongtrack[j] = 0.0;
						ossbincount[j] = 0;
					}

					/* find minimum range */
					double rangemin = table_range[0];
					int kstart = 0;
					for (int kangle = 1; kangle < nangle; kangle++) {
						if (table_range[kangle] < rangemin) {
							rangemin = table_range[kangle];
							kstart = kangle;
						}
					}

					/* bin port trace */
					unsigned short *datashort = (unsigned short *)sschannelport->data;
					// istart = ss_altitude / (0.0000000005 * ssv_use * ssheaderport->sampleInterval);
					int istart = rangemin / (0.0000000005 * ssv_use * ssheaderport->sampleInterval);
					double weight = exp(MB_LN_2 * ((double)ssheaderport->weightingFactor));
					for (unsigned int i = istart; i < ssheaderport->samples; i++) {
						/* get sample value */
						const double value =
							ssheaderport->dataFormat == EDGETECH_TRACEFORMAT_ANALYTIC
							? sqrt((double)(datashort[2 * i] * datashort[2 * i] + datashort[2 * i + 1] * datashort[2 * i + 1]))
							: (double)(datashort[i]);

						/* get sample range */
						rr = 0.0000000005 * ssv_use * (i * ssheaderport->sampleInterval);

						/* look up position(s) for this range */
						bool done = false;
						for (int kangle = kstart; kangle > 0 && !done; kangle--) {
							bool found = false;
							double xtrack;
							double ltrack;
							if (rr <= table_range[kstart]) {
								xtrack = table_xtrack[kstart];
								ltrack = table_ltrack[kstart];
								found = true;
								done = true;
							}
							else if (rr > table_range[kangle] && rr <= table_range[kangle - 1]) {
								const double factor = (rr - table_range[kangle]) / (table_range[kangle - 1] - table_range[kangle]);
								xtrack = table_xtrack[kangle] + factor * (table_xtrack[kangle - 1] - table_xtrack[kangle]);
								ltrack = table_ltrack[kangle] + factor * (table_ltrack[kangle - 1] - table_ltrack[kangle]);
								found = true;
								done = true;
							}
							else if (rr < table_range[kangle] && rr >= table_range[kangle - 1]) {
								const double factor = (rr - table_range[kangle]) / (table_range[kangle - 1] - table_range[kangle]);
								xtrack = table_xtrack[kangle] + factor * (table_xtrack[kangle - 1] - table_xtrack[kangle]);
								ltrack = table_ltrack[kangle] + factor * (table_ltrack[kangle - 1] - table_ltrack[kangle]);
								found = true;
								done = true;
							}

							/* bin the value and position */
							if (found) {
								const int j = opixels_ss / 2 + (int)(xtrack / pixel_width);
								if (j >= 0 && j < opixels_ss) {
									oss[j] += value / weight;
									ossbincount[j]++;
									ossalongtrack[j] += ltrack;
								}
							}
						}
					}

					/* find minimum range */
					rangemin = table_range[0];
					kstart = 0;
					for (int kangle = 1; kangle < nangle; kangle++) {
						if (table_range[kangle] < rangemin) {
							rangemin = table_range[kangle];
							kstart = kangle;
						}
					}

					/* bin stbd trace */
					datashort = (unsigned short *)sschannelstbd->data;
					istart = ss_altitude / (0.0000000005 * ssv_use * ssheaderstbd->sampleInterval);
					weight = exp(MB_LN_2 * ((double)ssheaderstbd->weightingFactor));
					for (unsigned int i = istart; i < ssheaderstbd->samples; i++) {
						const double value =
							ssheaderstbd->dataFormat == EDGETECH_TRACEFORMAT_ANALYTIC
							? sqrt((double)(datashort[2 * i] * datashort[2 * i] + datashort[2 * i + 1] * datashort[2 * i + 1]))
							: (double)(datashort[i]);

						/* get sample range */
						rr = 0.0000000005 * ssv_use * (i * ssheaderstbd->sampleInterval);

						/* look up position for this range */
						bool done = false;
						for (int kangle = kstart; kangle < nangle - 1 && !done; kangle++) {
							bool found = false;
							double xtrack;
							double ltrack;
							if (rr <= table_range[kstart]) {
								xtrack = table_xtrack[kstart];
								ltrack = table_ltrack[kstart];
								found = true;
								done = true;
							}
							else if (rr > table_range[kangle] && rr <= table_range[kangle + 1]) {
								const double factor = (rr - table_range[kangle]) / (table_range[kangle + 1] - table_range[kangle]);
								xtrack = table_xtrack[kangle] + factor * (table_xtrack[kangle + 1] - table_xtrack[kangle]);
								ltrack = table_ltrack[kangle] + factor * (table_ltrack[kangle + 1] - table_ltrack[kangle]);
								found = true;
								done = true;
							}
							else if (rr < table_range[kangle] && rr >= table_range[kangle + 1]) {
								const double factor = (rr - table_range[kangle]) / (table_range[kangle + 1] - table_range[kangle]);
								xtrack = table_xtrack[kangle] + factor * (table_xtrack[kangle + 1] - table_xtrack[kangle]);
								ltrack = table_ltrack[kangle] + factor * (table_ltrack[kangle + 1] - table_ltrack[kangle]);
								found = true;
								done = true;
							}

							/* bin the value and position */
							if (found) {
								const int j = opixels_ss / 2 + (int)(xtrack / pixel_width);
								if (j >= 0 && j < opixels_ss) {
									oss[j] += value / weight;
									ossbincount[j]++;
									ossalongtrack[j] += ltrack;
								}
							}
						}
					}

					/* calculate the output sidescan */
					for (int j = 0; j < opixels_ss; j++) {
						if (ossbincount[j] > 0) {
							oss[j] /= (double)ossbincount[j];
							ossalongtrack[j] /= (double)ossbincount[j];
						}
						else
							oss[j] = MB_SIDESCAN_NULL;
					}

					/* interpolate gaps in the output sidescan */
					int previous = opixels_ss;
					for (int j = 0; j < opixels_ss; j++) {
						if (ossbincount[j] > 0) {
							const int interpable = j - previous - 1;
							if (interpable > 0 && interpable <= interpbins) {
								const double dss = oss[j] - oss[previous];
								const double dssl = ossalongtrack[j] - ossalongtrack[previous];
								for (int jj = previous + 1; jj < j; jj++) {
									const double fraction = ((double)(jj - previous)) / ((double)(j - previous));
									oss[jj] = oss[previous] + fraction * dss;
									ossalongtrack[jj] = ossalongtrack[previous] + fraction * dssl;
								}
							}
							previous = j;
						}
					}

					/* insert data */
					mb_insert_nav(verbose, ombio_ptr, (void *)ostore, time_i, time_d, navlon, navlat, speed, heading, draft, roll,
					              pitch, heave, &error);
					status = mb_insert_altitude(verbose, ombio_ptr, (void *)ostore, sonardepth, ss_altitude, &error);
					status &= mb_insert(verbose, ombio_ptr, (void *)ostore, MB_DATA_DATA, time_i, time_d, navlon, navlat, speed,
					                   heading, beams_bath, beams_amp, opixels_ss, beamflag, bath, amp, bathacrosstrack,
					                   bathalongtrack, oss, ossacrosstrack, ossalongtrack, comment, &error);

					/* write the record */
					nwritesshi++;
					mb_write_ping(verbose, ombio_ptr, (void *)ostore, &error);
				}
			}

			/* handle unknown data */
			else if (status == MB_SUCCESS) {
				fprintf(stderr,"DATA TYPE UNKNOWN: status:%d error:%d kind:%d\n",status,error,kind);
			}

			/* handle read error */
			else {
				fprintf(stderr,"READ FAILURE: status:%d error:%d kind:%d\n",status,error,kind);
			}

			if (verbose >= 2) {
				fprintf(stderr, "\ndbg2  Ping read in program <%s>\n", program_name);
				fprintf(stderr, "dbg2       kind:           %d\n", kind);
				fprintf(stderr, "dbg2       error:          %d\n", error);
				fprintf(stderr, "dbg2       status:         %d\n", status);
			}

			if (print_comments && kind == MB_DATA_COMMENT) {
				if (icomment == 0) {
					fprintf(stderr, "\nComments:\n");
					icomment++;
				}
				fprintf(stderr, "%s\n", comment);
			}
		}

		/* close the swath file */
		status &= mb_close(verbose, &imbio_ptr, &error);

		/* output counts */
		fprintf(stdout, "\nData records read from: %s\n", file);
		fprintf(stdout, "     Survey:        %d\n", nreaddata);
		fprintf(stdout, "     File Header:   %d\n", nreadheader);
		fprintf(stdout, "     Bluefin CTD:   %d\n", nreadssv);
		fprintf(stdout, "     Bluefin Nav:   %d\n", nreadnav1);
		fprintf(stdout, "     Subbottom:     %d\n", nreadsbp);
		fprintf(stdout, "     Low Sidescan:  %d\n", nreadsslo);
		fprintf(stdout, "     High Sidescan: %d\n", nreadsshi);
		nreaddatatot += nreaddata;
		nreadheadertot += nreadheader;
		nreadssvtot += nreadssv;
		nreadnav1tot += nreadnav1;
		nreadsbptot += nreadsbp;
		nreadsslotot += nreadsslo;
		nreadsshitot += nreadsshi;

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

	/* close output file if still open */
	if (ombio_ptr != nullptr) {
		/* close the swath file */
		status = mb_close(verbose, &ombio_ptr, &error);

		/* generate inf file */
		if (status == MB_SUCCESS) {
			status = mb_make_info(verbose, true, output_file, format_output, &error);
		}

		/* output counts */
		fprintf(stdout, "\nData records written to: %s\n", current_output_file);
		fprintf(stdout, "     Low Sidescan:  %d\n", nwritesslo);
		fprintf(stdout, "     High Sidescan: %d\n", nwritesshi);
		nwritesslotot += nwritesslo;
		nwritesshitot += nwritesshi;

		/* output commands to first cut plotting script file */
		fprintf(sfp, "# Generate swath plot of sidescan file: %s\n", current_output_file);
		fprintf(sfp, "mbm_plot -I %s -N -G5 -S -Pb -V -O %s_ssrawplot\n", current_output_file, current_output_file);
		fprintf(sfp, "%s_ssrawplot.cmd\n\n", current_output_file);
	}

	/* close plotting script file */
	fclose(sfp);

	char command[MB_PATH_MAXLINE+30];
	snprintf(command, sizeof(command), "chmod +x %s", scriptfile);
	/* int shellstatus = */ system(command);

	/* output counts */
	fprintf(stdout, "\nTotal data records read:\n");
	fprintf(stdout, "     Survey:        %d\n", nreaddatatot);
	fprintf(stdout, "     File Header:   %d\n", nreadheadertot);
	fprintf(stdout, "     Bluefin CTD:   %d\n", nreadssvtot);
	fprintf(stdout, "     Bluefin Nav:   %d\n", nreadnav1tot);
	fprintf(stdout, "     Subbottom:     %d\n", nreadsbptot);
	fprintf(stdout, "     Low Sidescan:  %d\n", nreadsslotot);
	fprintf(stdout, "     High Sidescan: %d\n", nreadsshitot);
	fprintf(stdout, "Total data records written:\n");
	fprintf(stdout, "     Low Sidescan:  %d\n", nwritesslotot);
	fprintf(stdout, "     High Sidescan: %d\n", nwritesshitot);

	/* deallocate route arrays */
	if (route_file_set) {
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&routelon, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routelat, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routeheading, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routewaypoint, &error);
	}

	/* deallocate topography grid array if necessary */
	if (sslayoutmode == MB7K2SS_SS_3D_BOTTOM)
		status &= mb_topogrid_deall(verbose, &topogrid_ptr, &error);

	/* check memory */
	if (verbose >= 4)
		status &= mb_memory_list(verbose, &error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s> completed\n", program_name);
		fprintf(stderr, "dbg2  Ending status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	exit(error);
}
/*--------------------------------------------------------------------*/
