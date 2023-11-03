/*--------------------------------------------------------------------
 *    The MB-system:	mb7k2jstar.c	5/19/2005
 *
 *    Copyright (c) 2005-2023 by
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
 * mb7k2jstar extracts Edgetech subbottom profiler and sidescan data
 * from Reson 7k format data and outputs in the Edgetech Jstar format.
 *
 * Author:	D. W. Caress
 * Date:	May 19, 2005
 *
 */

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_jstar.h"
#include "mbsys_reson7k.h"

typedef enum {
    MB7K2JSTAR_SSLOW = 1,
    MB7K2JSTAR_SSHIGH = 2,
    MB7K2JSTAR_SBP = 3,
    MB7K2JSTAR_ALL = 4,
} mb7k2jstar_mode;
typedef enum {
    MB7K2JSTAR_BOTTOMPICK_NONE = 0,
    MB7K2JSTAR_BOTTOMPICK_BATHYMETRY = 1,
    MB7K2JSTAR_BOTTOMPICK_ALTITUDE = 2,
    MB7K2JSTAR_BOTTOMPICK_ARRIVAL = 3,
} bottompick_t;
typedef enum {
    MB7K2JSTAR_SSGAIN_OFF = 0,
    MB7K2JSTAR_SSGAIN_TVG_1OVERR = 1,
} ssgain_t;
constexpr int MBES_ALLOC_NUM = 128;
typedef enum {
    MBES_ROUTE_WAYPOINT_NONE = 0,
    MBES_ROUTE_WAYPOINT_SIMPLE = 1,
    MBES_ROUTE_WAYPOINT_TRANSIT = 2,
    MBES_ROUTE_WAYPOINT_STARTLINE = 3,
    MBES_ROUTE_WAYPOINT_ENDLINE = 4,
} waypoint_t;
constexpr int MBES_ONLINE_THRESHOLD = 15.0;
constexpr int MBES_ONLINE_COUNT = 30;

constexpr char help_message[] =
    "mb7k2jstar extracts Edgetech subbottom profiler and sidescan data\n"
    "from Reson 7k format data and outputs in the Edgetech Jstar format.";
constexpr char program_name[] = "mb7k2jstar";
constexpr char usage_message[] =
    "mb7k2jstar [-Ifile -Atype -Bmode[/threshold] -C -Fformat "
    "-Lstartline/lineroot -Ooutfile -Rroutefile -X -H -V]";

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

	char read_file[MB_PATH_MAXLINE] = "datalist.mb-1";

	int startline = 1;
	char lineroot[MB_PATH_MAXLINE];
	strcpy(lineroot, "jstar");

	bool extract_sslow = false;
	bool extract_sshigh = false;
	bool extract_sbp = false;
	bool print_comments = false;

	mb7k2jstar_mode mode;

	bottompick_t bottompickmode = MB7K2JSTAR_BOTTOMPICK_ALTITUDE;
	double bottompickthreshold = 0.4;

	bool ssflip = false;
	ssgain_t gainmode = MB7K2JSTAR_SSGAIN_OFF;
	double gainfactor = 1.0;

	bool checkroutebearing = false;

	char output_file[MB_PATH_MAXLINE+12] = "";
	bool output_file_set = false;

	char route_file[MB_PATH_MAXLINE] = "";
	bool route_file_set = false;

	int smooth = 0;

	double timeshift = 0.0;

	int error = MB_ERROR_NO_ERROR;

	/* process argument list */
	{
		bool errflg = false;
		bool help = false;
		int c;
		while ((c = getopt(argc, argv, "A:a:B:b:CcF:f:G:g:I:i:L:l:MmO:o:R:r:S:s:T:t:XxVvHh")) != -1)
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
					extract_sslow = true;
				}
				else if (strncmp(optarg, "SSHIGH", 6) == 0 || strncmp(optarg, "sshigh", 6) == 0) {
					extract_sshigh = true;
				}
				else if (strncmp(optarg, "SBP", 3) == 0 || strncmp(optarg, "sbp", 3) == 0) {
					extract_sbp = true;
				}
				else if (strncmp(optarg, "ALL", 3) == 0 || strncmp(optarg, "all", 3) == 0) {
					extract_sshigh = true;
					extract_sslow = true;
					extract_sbp = true;
				}
				else {
					int tmp;
					sscanf(optarg, "%d", &tmp);
					mode = (mb7k2jstar_mode)tmp;
					if (mode == MB7K2JSTAR_SSLOW)
						extract_sslow = true;
					else if (mode == MB7K2JSTAR_SSHIGH)
						extract_sshigh = true;
					else if (mode == MB7K2JSTAR_SBP)
						extract_sbp = true;
					else if (mode == MB7K2JSTAR_ALL) {
						extract_sshigh = true;
						extract_sslow = true;
						extract_sbp = true;
					}
				}
				break;
			case 'B':
			case 'b': {
				int tmp;
				const int n = sscanf(optarg, "%d/%lf", &tmp, &bottompickthreshold);
				bottompickmode = (bottompick_t)tmp;
				if (n == 0)
					bottompickmode = MB7K2JSTAR_BOTTOMPICK_ALTITUDE;
				else if (n == 1 && bottompickmode == MB7K2JSTAR_BOTTOMPICK_ARRIVAL)
					bottompickthreshold = 0.5;
				break;
			}
			case 'C':
			case 'c':
				print_comments = true;
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
				sscanf(optarg, "%lf", &timeshift);
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
			fprintf(stderr, "dbg2       timeshift:           %f\n", timeshift);
			fprintf(stderr, "dbg2       bottompickmode:      %d\n", bottompickmode);
			fprintf(stderr, "dbg2       bottompickthreshold: %f\n", bottompickthreshold);
			fprintf(stderr, "dbg2       smooth:              %d\n", smooth);
			fprintf(stderr, "dbg2       gainmode:            %d\n", gainmode);
			fprintf(stderr, "dbg2       gainfactor:          %f\n", gainfactor);
			fprintf(stderr, "dbg2       route_file_set:      %d\n", route_file_set);
			fprintf(stderr, "dbg2       route_file:          %s\n", route_file);
			fprintf(stderr, "dbg2       checkroutebearing:   %d\n", checkroutebearing);
			fprintf(stderr, "dbg2       output_file:         %s\n", output_file);
			fprintf(stderr, "dbg2       output_file_set:     %d\n", output_file_set);
			fprintf(stderr, "dbg2       lineroot:            %s\n", lineroot);
			fprintf(stderr, "dbg2       extract_sbp:         %d\n", extract_sbp);
			fprintf(stderr, "dbg2       extract_sslow:       %d\n", extract_sslow);
			fprintf(stderr, "dbg2       extract_sshigh:      %d\n", extract_sshigh);
			fprintf(stderr, "dbg2       print_comments:      %d\n", print_comments);
		}

		if (help) {
			fprintf(stderr, "\n%s\n", help_message);
			fprintf(stderr, "\nusage: %s\n", usage_message);
			exit(error);
		}
	}

	/* set output types if needed */
	if (!extract_sbp && !extract_sslow && !extract_sshigh) {
		extract_sbp = true;
		extract_sslow = true;
		extract_sshigh = true;
	}

	/* output output types */
	fprintf(stdout, "\nData records to extract:\n");
	if (extract_sbp)
		fprintf(stdout, "     Subbottom\n");
	if (extract_sslow)
		fprintf(stdout, "     Low Sidescan\n");
	if (extract_sshigh)
		fprintf(stdout, "     High Sidescan\n");
	if (ssflip)
		fprintf(stdout, "     Sidescan port and starboard exchanged\n");

	/* set starting line number and output file if route read */
	int linenumber = 0;
	if (route_file_set) {
		linenumber = startline;
		snprintf(output_file, sizeof(output_file), "%s_%4.4d.mb132", lineroot, linenumber);
	}

	/* new output file obviously needed */
	bool new_output_file = true;

	int nroutepoint = 0;
	double *routelon = nullptr;
	double *routelat = nullptr;
	double *routeheading = nullptr;
	int *routewaypoint = nullptr;
	int activewaypoint = 0;
	double mtodeglon;
	double mtodeglat;
	double rangelast;
	double rangethreshold = 50.0;
	int oktowrite;

	/* if specified read route file */
	if (route_file_set) {
		/* open the input file */
		FILE *fp = fopen(route_file, "r");
		if (fp == nullptr) {
			fprintf(stderr, "\nUnable to open route file <%s> for reading\n", route_file);
			exit(MB_FAILURE);
		}
		bool rawroutefile = false;
		int nroutepointalloc = 0;
		char *result;
		char comment[MB_COMMENT_MAXLINE];
		while ((result = fgets(comment, MB_PATH_MAXLINE, fp)) == comment) {
			if (comment[0] == '#') {
				if (strncmp(comment, "## Route File Version", 21) == 0) {
					rawroutefile = false;
				}
			}
			else {
				double heading;
				double lon;
				double lat;
				double topo;
				waypoint_t waypoint;
				int waypoint_tmp;
				const int nget = sscanf(comment, "%lf %lf %lf %d %lf", &lon, &lat, &topo, &waypoint_tmp, &heading);
				waypoint = (waypoint_t)waypoint_tmp;
				if (comment[0] == '#') {
					fprintf(stderr, "buffer:%s", comment);
					if (strncmp(comment, "## Route File Version", 21) == 0) {
						rawroutefile = false;
					}
				}
				const bool point_ok =
					(rawroutefile && nget >= 2) ||
					(!rawroutefile && nget >= 3 && waypoint > MBES_ROUTE_WAYPOINT_NONE);

				/* if good data check for need to allocate more space */
				if (point_ok && nroutepoint + 1 > nroutepointalloc) {
					nroutepointalloc += MBES_ALLOC_NUM;
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

		/* output status */
		if (verbose > 0) {
			/* output info on file output */
			fprintf(stderr, "\nImported %d waypoints from route file: %s\n", nroutepoint, route_file);
		}
	}

	/* get format if required */
	if (format == 0)
		mb_get_format(verbose, read_file, nullptr, &format, &error);

	/* determine whether to read one file or a list of files */
	const bool read_datalist = format < 0;

	/* open file list */
	char file[MB_PATH_MAXLINE] = "";
	void *datalist = nullptr;
	double file_weight;
	char dfile[MB_PATH_MAXLINE];
	bool read_data;
	if (read_datalist) {
		const int look_processed = MB_DATALIST_LOOK_YES;
		if (mb_datalist_open(verbose, &datalist, read_file, look_processed, &error) != MB_SUCCESS) {
			fprintf(stderr, "\nUnable to open data list file: %s\n", read_file);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_ERROR_OPEN_FAIL);
		}
		read_data = mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error) == MB_SUCCESS;
	}
	/* else copy single filename to be read */
	else {
		strcpy(file, read_file);
		read_data = true;
	}

	/* MBIO read values */
	void *ombio_ptr = nullptr;
	struct mb_io_struct *omb_io_ptr = nullptr;
	void *ostore_ptr = nullptr;
	struct mbsys_jstar_struct *ostore = nullptr;
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
	double *ttimes = nullptr;
	double *angles = nullptr;
	double *angles_forward = nullptr;
	double *angles_null = nullptr;
	double *bheave = nullptr;
	double *alongtrack_offset = nullptr;
	double draft;
	double ssv;

	int icomment = 0;

	/* jstar data */
	s7k_fsdwchannel *s7kchannel;       /* Channel header and data */
	s7k_fsdwssheader *s7kssheader;     /* Edgetech sidescan header */
	s7k_fsdwsegyheader *s7ksegyheader = nullptr; /* Segy header for subbottom trace */
	struct mbsys_jstar_channel_struct *channel;
	int obeams_bath;
	int obeams_amp;
	int opixels_ss;

	/* route and auto-line data */
	double range;

	/* counting variables */
	int nwritesbp = 0;
	int nwritesslo = 0;
	int nwritesshi = 0;
	int nreaddatatot = 0;
	int nreadheadertot = 0;
	int nreadssvtot = 0;
	int nreadnav1tot = 0;
	int nreadsbptot = 0;
	int nreadsslotot = 0;
	int nreadsshitot = 0;
	int nwritesbptot = 0;
	int nwritesslotot = 0;
	int nwritesshitot = 0;

	int format_status, format_guess;
	int format_output = MBF_EDGJSTAR;
	int shortspersample;
	char *data;
	unsigned short *datashort;
	double value, threshold;
	double channelmax;
	int channelpick;
	double ttime_min;
	double factor;
	double headingdiff;
	double dx, dy;

	char current_output_file[MB_PATH_MAXLINE];

	/* loop over all files to be read */
	while (read_data && format == MBF_RESON7KR) {

		/* initialize reading the swath file */
		double btime_d;
		double etime_d;
		int beams_bath;
		int beams_amp;
		int pixels_ss;
		void *imbio_ptr = nullptr;
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
		struct mb_io_struct *imb_io_ptr = (struct mb_io_struct *)imbio_ptr;
		void *istore_ptr = imb_io_ptr->store_data;
		struct mbsys_reson7k_struct *istore = (struct mbsys_reson7k_struct *)istore_ptr;

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

			else if (!output_file_set && !route_file_set) {
				new_output_file = true;
				format_status = mb_get_format(verbose, file, output_file, &format_guess, &error);
				if (format_status != MB_SUCCESS || format_guess != format) {
					strcpy(output_file, file);
				}
				if (output_file[strlen(output_file) - 1] == 'p') {
					output_file[strlen(output_file) - 1] = '\0';
				}
				if (extract_sbp && extract_sslow && extract_sshigh) {
					strcat(output_file, ".jsf");
					format_output = MBF_EDGJSTAR;
				}
				else if (extract_sslow) {
					strcat(output_file, ".mb132");
					format_output = MBF_EDGJSTAR;
				}
				else if (extract_sshigh) {
					strcat(output_file, ".mb133");
					format_output = MBF_EDGJSTR2;
				}
				else if (extract_sbp) {
					strcat(output_file, ".jsf");
					format_output = MBF_EDGJSTAR;
				}
			}
		}

		/* read and print data */
		int nreaddata = 0;
		int nreadheader = 0;
		int nreadssv = 0;
		int nreadnav1 = 0;
		int nreadsbp = 0;
		int nreadsslo = 0;
		int nreadsshi = 0;

		double ttime_min_use = 0.0;
		while (error <= MB_ERROR_NO_ERROR) {
			/* reset error */
			error = MB_ERROR_NO_ERROR;

			/* read next data record */
			int kind;
			int time_i[7];
			double time_d;
			double navlon;
			double navlat;
			double speed;
			double distance;
			double altitude;
			double sonardepth;
			double heading;
			char comment[MB_COMMENT_MAXLINE];
			status &= mb_get_all(verbose, imbio_ptr, &istore_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading,
			                    &distance, &altitude, &sonardepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp,
			                    bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);

			/* reset nonfatal errors */
			if (kind == MB_DATA_DATA && error < 0) {
				status = MB_SUCCESS;
				error = MB_ERROR_NO_ERROR;
			}

			/* check survey data position against waypoints */
			if (status == MB_SUCCESS && kind == MB_DATA_DATA && route_file_set && nroutepoint > 1 && navlon != 0.0 &&
			    navlat != 0.0) {
				dx = (navlon - routelon[activewaypoint]) / mtodeglon;
				dy = (navlat - routelat[activewaypoint]) / mtodeglat;
				range = sqrt(dx * dx + dy * dy);
				if (range < rangethreshold && (activewaypoint == 0 || range > rangelast) && activewaypoint < nroutepoint - 1) {
					/* if needed set flag to open new output file */
					if (!new_output_file) {
						/* increment line number */
						linenumber++;

						/* set output file name */
						snprintf(output_file, sizeof(output_file), "%s_%4.4d", lineroot, linenumber);
						if (extract_sbp && extract_sslow && extract_sshigh) {
							strcat(output_file, ".jsf");
							format_output = MBF_EDGJSTAR;
						}
						else if (extract_sslow) {
							strcat(output_file, ".mb132");
							format_output = MBF_EDGJSTAR;
						}
						else if (extract_sshigh) {
							strcat(output_file, ".mb133");
							format_output = MBF_EDGJSTR2;
						}
						else if (extract_sbp) {
							strcat(output_file, ".jsf");
							format_output = MBF_EDGJSTAR;
						}

						/* set to open new output file */
						new_output_file = true;
					}

					/* increment active waypoint */
					activewaypoint++;
					mb_coor_scale(verbose, routelat[activewaypoint], &mtodeglon, &mtodeglat);
					rangelast = 1000 * rangethreshold;
					oktowrite = 0;
				}
				else
					rangelast = range;
			}

			if (kind == MB_DATA_DATA && error <= MB_ERROR_NO_ERROR) {
				/* extract travel times */
				status = mb_ttimes(verbose, imbio_ptr, istore_ptr, &kind, &beams_bath, ttimes, angles, angles_forward,
				                   angles_null, bheave, alongtrack_offset, &draft, &ssv, &error);

				/* get bottom arrival time, if possible */
				ttime_min = 0.0;
				bool found = false;
				for (int i = 0; i < beams_bath; i++) {
					if (mb_beam_ok(beamflag[i])) {
						if (!found || ttimes[i] < ttime_min) {
							ttime_min = ttimes[i];
							/* beam_min = i; */
							found = false;
						}
					}
				}
				if (found) {
					ttime_min_use = ttime_min;
				}
			}

			/* nonfatal errors do not matter */
			if (error < MB_ERROR_NO_ERROR) {
				error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
			}

			/* if needed open new output file */
			if (status == MB_SUCCESS && new_output_file &&
			    ((extract_sbp && kind == MB_DATA_SUBBOTTOM_SUBBOTTOM) ||
			     (extract_sslow && kind == MB_DATA_SIDESCAN2) ||
			     (extract_sshigh && kind == MB_DATA_SIDESCAN3))) {

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
					fprintf(stdout, "     Subbottom:     %d\n", nwritesbp);
					fprintf(stdout, "     Low Sidescan:  %d\n", nwritesslo);
					fprintf(stdout, "     High Sidescan: %d\n", nwritesshi);
					nwritesbptot += nwritesbp;
					nwritesslotot += nwritesslo;
					nwritesshitot += nwritesshi;
				}

				/* open the new file */
				nwritesbp = 0;
				nwritesslo = 0;
				nwritesshi = 0;
				if ((status &= mb_write_init(verbose, output_file, MBF_EDGJSTAR, &ombio_ptr, &obeams_bath, &obeams_amp,
				                            &opixels_ss, &error)) != MB_SUCCESS) {
					char *message;
					mb_error(verbose, error, &message);
					fprintf(stderr, "\nMBIO Error returned from function <mb_write_init>:\n%s\n", message);
					fprintf(stderr, "\nMultibeam File <%s> not initialized for writing\n", file);
					fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
					exit(error);
				}

				/* save current_output_file */
				strcpy(current_output_file, output_file);

				/* get pointers to data storage */
				omb_io_ptr = (struct mb_io_struct *)ombio_ptr;
				ostore_ptr = omb_io_ptr->store_data;
				ostore = (struct mbsys_jstar_struct *)ostore_ptr;

				/* reset new_output_file */
				new_output_file = false;
			}

			/* apply time shift if needed */
			int time_j[5];
			if (status == MB_SUCCESS && timeshift != 0.0 &&
			    (kind == MB_DATA_SUBBOTTOM_SUBBOTTOM || kind == MB_DATA_SIDESCAN2 || kind == MB_DATA_SIDESCAN3)) {
				time_d += timeshift;
				mb_get_date(verbose, time_d, time_i);
				mb_get_jtime(verbose, time_i, time_j);
			}

			/* get some more values */
			if (status == MB_SUCCESS && (kind == MB_DATA_SUBBOTTOM_SUBBOTTOM || kind == MB_DATA_DATA ||
			                             kind == MB_DATA_SIDESCAN2 || kind == MB_DATA_SIDESCAN3)) {
				mb_get_jtime(verbose, istore->time_i, time_j);
				speed = 0.0;
				mb_hedint_interp(verbose, imbio_ptr, time_d, &heading, &error);
				mb_navint_interp(verbose, imbio_ptr, time_d, heading, speed, &navlon, &navlat, &speed, &error);
				mb_depint_interp(verbose, imbio_ptr, time_d, &sonardepth, &error);
				mb_altint_interp(verbose, imbio_ptr, time_d, &altitude, &error);
				mb_attint_interp(verbose, imbio_ptr, time_d, &heave, &roll, &pitch, &error);
			}

			/* if following a route check that the vehicle has come on line
			        (within MBES_ONLINE_THRESHOLD degrees)
			        before writing any data */
			if (checkroutebearing && nroutepoint > 1 && activewaypoint > 0) {
				headingdiff = fabs(routeheading[activewaypoint - 1] - heading);
				if (headingdiff > 180.0)
					headingdiff = 360.0 - headingdiff;
				if (headingdiff < MBES_ONLINE_THRESHOLD)
					oktowrite++;
				else
					oktowrite = 0;
			}
			else
				oktowrite = MBES_ONLINE_COUNT;

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

				/* output data if desired */
				if (extract_sbp && nreadnav1 > 0 && oktowrite >= MBES_ONLINE_COUNT) {
					/* set overall parameters */
					ostore->kind = kind;
					ostore->subsystem = 0;

					/* copy subbottom data to jstar storage */
					channel = (struct mbsys_jstar_channel_struct *)&(ostore->sbp);
					s7kchannel = (s7k_fsdwchannel *)&(istore->fsdwsb.channel);
					s7ksegyheader = (s7k_fsdwsegyheader *)&(istore->fsdwsb.segyheader);

					/* message header values */
					channel->message.start_marker = 0x1601;
					channel->message.version = 10;
					channel->message.session = 0;
					channel->message.type = 80;
					channel->message.command = 0;
					channel->message.subsystem = 0;
					channel->message.channel = 0;
					channel->message.sequence = 0;
					channel->message.reserved = 0;
					channel->message.size = 0;

					/* Trace Header */
					channel->pingTime = s7ksegyheader->sequenceNumber; /* 0-3 : Ping Time in epoch seconds [since (1/1/1970)] (Protocol Version 8 onwards) */
					channel->startDepth = s7ksegyheader->startDepth; /* 4-7 : Starting depth (window offset) in samples. */
					channel->pingNum = s7ksegyheader->pingNum;       /* 8-11: Ping number (increments with ping) ** */
					for (int i = 0; i < 2; i++) /* 12-15 */
						channel->reserved1[i] = 0; /* 16-27 */
					channel->msb = 0; /* 16-17 */
					channel->lsb1 = 0; /* 18-19 */
					channel->lsb2 = 0; /* 20-21 */
					for (int i = 0; i < 3; i++) /* 22-27 */
						channel->reserved2[i] = s7ksegyheader->unused1[i+3];

					channel->traceIDCode = s7ksegyheader->traceIDCode; /* 28-29 : ID Code (always 1 => seismic data) ** */

					channel->validityFlag = 0;
					channel->reserved3 = s7ksegyheader->unused2[1]; /* 32-33 */
					channel->dataFormat = s7ksegyheader->dataFormat;
									/* 34-35 : DataFormatType */
					                /*   0 = 1 short  per sample  - envelope data */
									/*   1 = 2 shorts per sample, - stored as real(1), imag(1), */
									/*   2 = 1 short  per sample  - before matched filter */
									/*   3 = 1 short  per sample  - real part analytic signal */
									/*   4 = 1 short  per sample  - pixel data / ceros data */
					channel->NMEAantennaeR = s7ksegyheader->NMEAantennaeR; /* 36-37 : Distance from towfish to antennae in cm */
					channel->NMEAantennaeO =
					    s7ksegyheader->NMEAantennaeO; /* 38-39 : Distance to antennae starboard direction in cm */
					for (int i = 0; i < 2; i++) {
						channel->reserved4[i] = 0;   /* 40-43 : Reserved – Do not use */
					}
					channel->kmOfPipe = 0;      /* 44-47 : Kilometers of Pipe - See Validity Flag (bytes 30 – 31). */
					for (int i = 0; i < 16; i++) {
						channel->reserved5[i] = 0;  /* 48-79 : Reserved – Do not use */
					}

					/* -------------------------------------------------------------------- */
					/* Navigation data :                                                    */
					/* If the coorUnits are seconds(2), the x values represent longitude    */
					/* and the y values represent latitude.  A positive value designates    */
					/* the number of seconds east of Greenwich Meridian or north of the     */
					/* equator.                                                             */
					/* -------------------------------------------------------------------- */
					channel->coordX = s7ksegyheader->groupCoordX;   /* 80-83 : longitude or easting  */
					channel->coordY = s7ksegyheader->groupCoordY;   /* 84-87 : latitude or northing */
					channel->coordUnits = s7ksegyheader->coordUnits;
								 /* 88-89 : Units of coordinates -
                                  *         1 = X,Y in millimeters
                                  *         2 = X,Y in iminutes of arc times 10000
                                  *         3 = X,Y in decimeters */
					for (int i = 0; i < 24; i++)
						channel->annotation[i] = s7ksegyheader->annotation[i]; /* 90-113 : Annotation string */
					channel->samples = s7ksegyheader->samples;
								 /* 114-115 : Samples in this packet
	                              *           Large sample sizes require multiple packets.
	                              *           For protocol versions 0xA and above, the
	                              *           MSB1 field should include the MSBs
	                              *           (Most Significant Bits) needed to
	                              *           determine the number of samples.
	                              *           See bits 8-11 in bytes 16-17. Field MSB1
	                              *           for MSBs for large sample sizes. */
					channel->sampleInterval =
					    s7ksegyheader->sampleInterval;
								 /* 116-119 : Sampling Interval in Nanoseconds
                                  *           NOTE: For protocol versions 0xB and
                                  *           above, see the LSBs field should
                                  *           include the fractional component
                                  *           needed to determine the sample interval.
                                  *           See bits 0-7 in bytes 18-19. Field LSB1
                                  *           for LSBs for increased precision. */
					channel->ADCGain = s7ksegyheader->ADCGain;         /* 120-121 : Gain factor of ADC */
					channel->pulsePower = s7ksegyheader->pulsePower;   /* 122-123 : user pulse power setting (0 - 100) percent */
					channel->reserved6 = s7ksegyheader->correlated;   /* 124-125 : correlated data 1 - No, 2 - Yes */
					channel->startFreq = s7ksegyheader->startFreq;     /* 126-127 : Starting frequency in 10 * Hz */
					channel->endFreq = s7ksegyheader->endFreq;         /* 128-129 : Ending frequency in 10 * Hz */
					channel->sweepLength = s7ksegyheader->sweepLength; /* 130-131 : Sweep length in ms */
					channel->pressure = 0; /* 132-135 : Pressure in Milli PSI (1 unit = 1/1000 PSI)
                                  *           See Validity Flag (bytes 30-31) */
					channel->sonarDepth = 0; /* 136-139 : Depth in Millimeters (if not = 0)
                                  *           See Validity Flag (bytes 30-31). */
					channel->sampleFreq = s7ksegyheader->aliasFreq;       /* 140-141 : alias Frequency (sample frequency / 2) */
					channel->pulseID = s7ksegyheader->pulseID;           /* 142-143 : Unique pulse identifier */
					channel->sonarAltitude = 0;           /* 144-147 : Altitude in Millimeters
                                  *           A zero implies not filled. See Validity Flag (bytes 30-31) */
					channel->soundspeed = 0.0;            /* 148-151 : Sound Speed in Meters per Second.
                                  *           See Validity Flag (bytes 30-31). */
					channel->mixerFrequency = 0.0;        /* 152-155 : Mixer Frequency in Hertz
                                  *           For single pulses systems this should
                                  *           be close to the center frequency.
                                  *           For multi pulse systems this should
                                  *           be the approximate center frequency
                                  *           of the span of all the pulses. */

					channel->year = istore->time_i[0];                   /* 156-157 : Year data recorded (CPU time) */
					channel->day = time_j[1];                            /* 158-159 : day */
					channel->hour = istore->time_i[3];                   /* 160-161 : hour */
					channel->minute = istore->time_i[4];                 /* 162-163 : minute */
					channel->second = istore->time_i[5];                 /* 164-165 : second */
					channel->timeBasis = s7ksegyheader->timeBasis; /* 166-167 : Always 3 (other not specified by standard) */
					channel->weightingFactor =
					    s7ksegyheader->weightingFactor; /* 168-169 :  weighting factor for block floating point expansion */
					                                    /*            -- defined as 2 -N volts for lsb */
					channel->numberPulses =  0; /* 170-171 : Number of Pulses in the Water */
					/* -------------------------------------------------------------------- */
					/* From pitch/roll/temp/heading sensor */
					/* -------------------------------------------------------------------- */
					channel->heading =
					    s7ksegyheader->heading; /* 172-173 : Compass heading (100 * degrees) -180.00 to 180.00 degrees */
					channel->pitch = s7ksegyheader->pitch;             /* 174-175 : Pitch */
					channel->roll = s7ksegyheader->roll;               /* 176-177 : Roll */
					channel->temperature = 0;   /* 178-179 : Reserved */
					/* -------------------------------------------------------------------- */
					/* User defined area from 180-239                                       */
					/* -------------------------------------------------------------------- */
					channel->reserved9 = 0; /* 180-181 : Reserved */
					channel->triggerSource = s7ksegyheader->trigSource;   /* 182-183 : TriggerSource (0 = internal, 1 = external) */
					channel->markNumber = s7ksegyheader->markNumber;   /* 184-185 : Mark Number (0 = no mark) */
					channel->NMEAHour = s7ksegyheader->NMEAHour;       /* 186-187 : Hour */
					channel->NMEAMinutes = s7ksegyheader->NMEAMinutes; /* 188-189 : Minutes */
					channel->NMEASeconds = s7ksegyheader->NMEASeconds; /* 190-191 : Seconds */
					channel->NMEACourse = s7ksegyheader->NMEACourse;   /* 192-193 : Course */
					channel->NMEASpeed = s7ksegyheader->NMEASpeed;     /* 194-195 : Speed */
					channel->NMEADay = s7ksegyheader->NMEADay;         /* 196-197 : Day */
					channel->NMEAYear = s7ksegyheader->NMEAYear;       /* 198-199 : Year */
					channel->millisecondsToday =
					    0.001 * istore->time_i[6] /* 200-203 : Millieconds today */
					    + 1000 * (istore->time_i[5] + 60.0 * (istore->time_i[4] + 60.0 * istore->time_i[3]));
					channel->ADCMax =
					    s7ksegyheader->ADCMax; /* 204-205 : Maximum absolute value for ADC samples for this packet */
					channel->reserved10 = 0;   /* 206-207 : Reserved */
					channel->reserved11 = 0; /* 208-209 : Reserved */
					for (int i = 0; i < 6; i++)
						channel->softwareVersion[i] = s7ksegyheader->softwareVersion[i]; /* 210-215 : Software version number */
					/* Following items are not in X-Star */
					channel->sphericalCorrection =
					    s7ksegyheader
					        ->sphericalCorrection; /* 216-219 : Initial spherical correction factor (useful for multiping /*/
					                               /* deep application) * 100 */
					channel->packetNum =
					    s7ksegyheader->packetNum; /* 220-221 : Packet number (1 - N) (Each ping starts with packet 1) */
					channel->ADCDecimation = s7ksegyheader->ADCDecimation; /* 222-223 : A/D decimation before FFT */
					channel->reserved12 = 0;       /* 224-225 : Reserved */
					channel->temperature = 0;        /* 226-227 : Water Temperature in Units of 1/10 Degree C.
                               *           See Validity Flag (bytes 30-31).*/
					channel->layback = 0;            /* 227-231 : Layback
                               *           Distance to the sonar in meters. */
					channel->reserved13 = 0;           /* 232-235 : Reserved */
					channel->cableOut = 0;           /* 236-239 : Cable Out in Decimeters
                               *           See Validity Flag (bytes 30-31). */
					channel->reserved14 = 0;         /* 236-239 : Reserved */

					/* allocate memory for the trace */
					if (channel->dataFormat == 1)
						shortspersample = 2;
					else
						shortspersample = 1;
					unsigned int trace_size = shortspersample * channel->samples * sizeof(short);
					channel->message.size = shortspersample * channel->samples * sizeof(short);
					if (channel->trace_alloc < trace_size) {
						if ((status = mb_reallocd(verbose, __FILE__, __LINE__, trace_size, (void **)&(channel->trace), &error)) ==
						    MB_SUCCESS) {
							channel->trace_alloc = trace_size;
						}
					}

					/* copy the trace */
					if (status == MB_SUCCESS) {
						data = (char *)channel->trace;
						for (unsigned int i = 0; i < trace_size; i++)
							data[i] = s7kchannel->data[i];
					}

					/* set the sonar altitude using the specified mode */
					if (bottompickmode == MB7K2JSTAR_BOTTOMPICK_ARRIVAL) {
						/* get bottom arrival in trace */
						if (channel->dataFormat == MBSYS_JSTAR_TRACEFORMAT_ANALYTIC) {
							channelmax = 0.0;
							for (int i = 0; i < channel->samples; i++) {
								value = sqrt((double)(channel->trace[2 * i] * channel->trace[2 * i] +
								                      channel->trace[2 * i + 1] * channel->trace[2 * i + 1]));
								channelmax = std::max(value, channelmax);
							}
							channelpick = 0;
							threshold = bottompickthreshold * channelmax;
							for (int i = 0; i < channel->samples && channelpick == 0; i++) {
								value = sqrt((double)(channel->trace[2 * i] * channel->trace[2 * i] +
								                      channel->trace[2 * i + 1] * channel->trace[2 * i + 1]));
								if (value >= threshold)
									channelpick = i;
							}
						}
						else {
							channelmax = 0.0;
							for (int i = 0; i < channel->samples; i++) {
								value = (double)(channel->trace[i]);
								channelmax = std::max(value, channelmax);
							}
							channelpick = 0;
							threshold = bottompickthreshold * channelmax;
							for (int i = 0; i < channel->samples && channelpick == 0; i++) {
								value = (double)(channel->trace[i]);
								if (value >= threshold)
									channelpick = i;
							}
						}

						/* set sonar altitude */
						channel->sonarAltitude = 0.00075 * channelpick * channel->sampleInterval;
					}
					else if (bottompickmode == MB7K2JSTAR_BOTTOMPICK_BATHYMETRY) {
						channel->sonarAltitude = (int)(750000.0 * ttime_min_use);
						if (channel->sonarAltitude == 0)
							channel->sonarAltitude = 1000 * altitude;
					}
					else {
						channel->sonarAltitude = 1000 * altitude;
					}

					/* reset navigation and other values */
					if (navlon < 180.0)
						navlon = navlon + 360.0;
					if (navlon > 180.0)
						navlon = navlon - 360.0;
					channel->coordX = (int)(360000.0 * navlon);
					channel->coordY = (int)(360000.0 * navlat);
					channel->coordUnits = 2;
					channel->heading = (short)(100.0 * heading);
					channel->startDepth = sonardepth / channel->sampleInterval / 0.00000075;
					channel->sonarDepth = 1000 * sonardepth;
					channel->roll = (short)(32768 * roll / 180.0);
					channel->pitch = (short)(32768 * pitch / 180.0);

					/* write the record */
					mb_write_ping(verbose, ombio_ptr, ostore_ptr, &error);
					nwritesbp++;
				}
			}

			/* handle low frequency sidescan data */
			else if (status == MB_SUCCESS && kind == MB_DATA_SIDESCAN2) {
				nreadsslo++;

				/* output data if desired */
				if (extract_sslow && nreadnav1 > 0 && oktowrite >= MBES_ONLINE_COUNT) {
					/* set overall parameters */
					ostore->kind = MB_DATA_DATA;
					ostore->subsystem = 20;

					/*----------------------------------------------------------------*/
					/* copy low frequency port sidescan to jstar storage */
					if (ssflip)
						channel = (struct mbsys_jstar_channel_struct *)&(ostore->ssstbd);
					else
						channel = (struct mbsys_jstar_channel_struct *)&(ostore->ssport);
					s7kchannel = (s7k_fsdwchannel *)&(istore->fsdwsslo.channel[0]);
					s7kssheader = (s7k_fsdwssheader *)&(istore->fsdwsslo.ssheader[0]);

					/* message header values */
					channel->message.start_marker = 0x1601;
					channel->message.version = 0;
					channel->message.session = 0;
					channel->message.type = 80;
					channel->message.command = 0;
					channel->message.subsystem = 20;
					if (ssflip)
						channel->message.channel = 1;
					else
						channel->message.channel = 0;
					channel->message.sequence = 0;
					channel->message.reserved = 0;
					channel->message.size = 0;

					/* Trace Header */
					channel->pingTime = 0;                   /* 0-3 : Ping Time in epoch seconds [since (1/1/1970)] (Protocol Version 8 onwards) */
					channel->startDepth = s7kssheader->startDepth; /* 4-7 : Starting depth (window offset) in samples. */
					channel->pingNum = s7kssheader->pingNum;       /* 8-11: Ping number (increments with ping) ** */
					for (int i = 0; i < 2; i++) /* 12-15 */
						channel->reserved1[i] = 0; /* 16-27 */
					channel->msb = 0; /* 16-17 */
					channel->lsb1 = 0; /* 18-19 */
					channel->lsb2 = 0; /* 20-21 */
					for (int i = 0; i < 3; i++) /* 22-27 */
						channel->reserved2[i] = 0;

					channel->traceIDCode = 1; /* 28-29 : ID Code (always 1 => seismic data) ** */

					channel->validityFlag = 0;
					channel->reserved3 = 0; /* 32-33 */
					channel->dataFormat = s7kssheader->dataFormat; /* 34-35 : DataFormatType */
					                                               /*   0 = 1 short  per sample  - envelope data */
					                                               /*   1 = 2 shorts per sample, - stored as real(1), imag(1), */
					                                               /*   2 = 1 short  per sample  - before matched filter */
					                                               /*   3 = 1 short  per sample  - real part analytic signal */
					                                               /*   4 = 1 short  per sample  - pixel data / ceros data */
					for (int i = 0; i < 2; i++) {
						channel->reserved4[i] = 0;   /* 40-43 : Reserved – Do not use */
					}
					channel->kmOfPipe = 0;      /* 44-47 : Kilometers of Pipe - See Validity Flag (bytes 30 – 31). */
					for (int i = 0; i < 16; i++) {
						channel->reserved5[i] = 0;  /* 48-79 : Reserved – Do not use */
					}

					/* -------------------------------------------------------------------- */
					/* Navigation data :                                                    */
					/* If the coorUnits are seconds(2), the x values represent longitude    */
					/* and the y values represent latitude.  A positive value designates    */
					/* the number of seconds east of Greenwich Meridian or north of the     */
					/* equator.                                                             */
					/* -------------------------------------------------------------------- */
					channel->coordX = 0;  /* 80-83 : mm or 10000 * (Minutes of Arc) */
					channel->coordY = 0;  /* 84-87 : mm or 10000 * (Minutes of Arc) */
					channel->coordUnits = 0;   /* 88-89 : Units of coordinates - 1->length (x /y), 2->seconds of arc */
					for (int i = 0; i < 24; i++)
						channel->annotation[i] = 0;          /* 90-113 : Annotation string */
					channel->samples = s7kssheader->samples; /* 114-115 : Samples in this packet ** */
					                                         /* Note:  Large sample sizes require multiple packets. */
					channel->sampleInterval = s7kssheader->sampleInterval; /* 116-119 : Sample interval in ns of stored data ** */
					channel->ADCGain = s7kssheader->ADCGain;               /* 120-121 : Gain factor of ADC */
					channel->pulsePower = 0;  /* 122-123 : user pulse power setting (0 - 100) percent */
					channel->reserved6 = 0;  /* 124-125 : correlated data 1 - No, 2 - Yes */
					channel->startFreq = 0;   /* 126-127 : Starting frequency in 10 * Hz */
					channel->endFreq = 0;     /* 128-129 : Ending frequency in 10 * Hz */
					channel->sweepLength = 0; /* 130-131 : Sweep length in ms */
					channel->pressure = 0; /* 132-135 : Pressure in Milli PSI (1 unit = 1/1000 PSI)
                                  *           See Validity Flag (bytes 30-31) */
					channel->sonarDepth = 0; /* 136-139 : Depth in Millimeters (if not = 0)
                                  *           See Validity Flag (bytes 30-31). */
					channel->sampleFreq = 0;       /* 140-141 : alias Frequency (sample frequency / 2) */
					channel->pulseID = s7ksegyheader->pulseID;           /* 142-143 : Unique pulse identifier */
					channel->sonarAltitude = 0;           /* 144-147 : Altitude in Millimeters
                                  *           A zero implies not filled. See Validity Flag (bytes 30-31) */
					channel->soundspeed = 0.0;            /* 148-151 : Sound Speed in Meters per Second.
                                  *           See Validity Flag (bytes 30-31). */
					channel->mixerFrequency = 0.0;        /* 152-155 : Mixer Frequency in Hertz
                                  *           For single pulses systems this should
                                  *           be close to the center frequency.
                                  *           For multi pulse systems this should
                                  *           be the approximate center frequency
                                  *           of the span of all the pulses. */

					channel->year = istore->time_i[0];   /* 156-157 : Year data recorded (CPU time) */
					channel->day = time_j[1];            /* 158-159 : day */
					channel->hour = istore->time_i[3];   /* 160-161 : hour */
					channel->minute = istore->time_i[4]; /* 162-163 : minute */
					channel->second = istore->time_i[5]; /* 164-165 : second */
					channel->timeBasis = 3;              /* 166-167 : Always 3 (other not specified by standard) */
					channel->weightingFactor =
					    s7kssheader->weightingFactor; /* 168-169 :  weighting factor for block floating point expansion */
					                                  /*            -- defined as 2 -N volts for lsb */
					channel->numberPulses =  0; /* 170-171 : Number of Pulses in the Water */
					/* -------------------------------------------------------------------- */
					/* From pitch/roll/temp/heading sensor */
					/* -------------------------------------------------------------------- */
					channel->heading =
					    s7kssheader->heading;            /* 172-173 : Compass heading (100 * degrees) -180.00 to 180.00 degrees */
					channel->pitch = s7kssheader->pitch; /* 174-175 : Pitch */
					channel->roll = s7kssheader->roll;   /* 176-177 : Roll */
					channel->temperature = s7kssheader->temperature; /* 178-179 : Temperature (10 * degrees C) */
					/* -------------------------------------------------------------------- */
					/* User defined area from 180-239                                       */
					/* -------------------------------------------------------------------- */
					channel->reserved9 = 0;                /* 180-181 : Reserved */
					channel->triggerSource = s7kssheader->trigSource; /* 182-183 : TriggerSource (0 = internal, 1 = external) */
					channel->markNumber = s7kssheader->markNumber; /* 184-185 : Mark Number (0 = no mark) */
					channel->NMEAHour = 0;                         /* 186-187 : Hour */
					channel->NMEAMinutes = 0;                      /* 188-189 : Minutes */
					channel->NMEASeconds = 0;                      /* 190-191 : Seconds */
					channel->NMEACourse = 0;                       /* 192-193 : Course */
					channel->NMEASpeed = 0;                        /* 194-195 : Speed */
					channel->NMEADay = 0;                          /* 196-197 : Day */
					channel->NMEAYear = 0;                         /* 198-199 : Year */
					channel->millisecondsToday =
					    0.001 * istore->time_i[6] /* 200-203 : Millieconds today */
					    + 1000 * (istore->time_i[5] + 60.0 * (istore->time_i[4] + 60.0 * istore->time_i[3]));
					channel->ADCMax = s7kssheader->ADCMax; /* 204-205 : Maximum absolute value for ADC samples for this packet */
					channel->reserved10 = 0;   /* 206-207 : Reserved */
					channel->reserved11 = 0; /* 208-209 : Reserved */
					for (int i = 0; i < 6; i++)
						channel->softwareVersion[i] = 0; /* 210-215 : Software version number */
					/* Following items are not in X-Star */
					channel->sphericalCorrection = s7ksegyheader->sphericalCorrection;
												/* 216-219 : Initial spherical correction factor (useful for multiping /*/
					                            /* deep application) * 100 */
					channel->packetNum = s7kssheader->packetNum; /* 220-221 : Packet number (1 - N) (Each ping starts with packet 1) */
					channel->ADCDecimation = 0; /* 222-223 : A/D decimation before FFT */
					channel->reserved12 = 0;       /* 224-225 : Reserved */
					channel->temperature = 0;        /* 226-227 : Water Temperature in Units of 1/10 Degree C.
                               *           See Validity Flag (bytes 30-31).*/
					channel->layback = 0;            /* 227-231 : Layback
                               *           Distance to the sonar in meters. */
					channel->reserved13 = 0;           /* 232-235 : Reserved */
					channel->cableOut = 0;           /* 236-239 : Cable Out in Decimeters
                               *           See Validity Flag (bytes 30-31). */
					channel->reserved14 = 0;         /* 236-239 : Reserved */

					/* allocate memory for the trace */
					if (channel->dataFormat == 1)
						shortspersample = 2;
					else
						shortspersample = 1;
					unsigned int trace_size = shortspersample * channel->samples * sizeof(short);
					channel->message.size = shortspersample * channel->samples * sizeof(short);
					if (channel->trace_alloc < trace_size) {
						if ((status = mb_reallocd(verbose, __FILE__, __LINE__, trace_size, (void **)&(channel->trace), &error)) ==
						    MB_SUCCESS) {
							channel->trace_alloc = trace_size;
						}
					}

					/* copy the trace */
					if (status == MB_SUCCESS) {
						if (smooth > 0 && channel->dataFormat == 0) {
							datashort = (unsigned short *)s7kchannel->data;
							for (int i = 0; i < channel->samples; i++) {
								int n = 0;
								channel->trace[i] = 0.0;
								for (int j = std::max(i - smooth, 0); j < std::min(i + smooth, channel->samples - 1); j++) {
									channel->trace[i] += datashort[j];
									n++;
								}
								channel->trace[i] /= n;
							}
						}
						else if (smooth < 0 && channel->dataFormat == 0) {
							datashort = (unsigned short *)s7kchannel->data;
							for (int i = 0; i < channel->samples; i++) {
								int n = 0;
								value = 0.0;
								for (int j = std::max(i + smooth, 0); j < std::min(i - smooth, channel->samples - 1); j++) {
									value += datashort[j] * datashort[j];
									n++;
								}
								channel->trace[i] = (unsigned int)(sqrt(value) / n);
							}
						}
						else {
							data = (char *)channel->trace;
							for (unsigned int i = 0; i < trace_size; i++) {
								data[i] = s7kchannel->data[i];
							}
						}
					}

					/* set the sonar altitude using the specified mode */
					if (bottompickmode == MB7K2JSTAR_BOTTOMPICK_ARRIVAL) {
						/* get bottom arrival in trace */
						if (channel->dataFormat == MBSYS_JSTAR_TRACEFORMAT_ANALYTIC) {
							channelmax = 0.0;
							for (int i = 0; i < channel->samples; i++) {
								value = sqrt((double)(channel->trace[2 * i] * channel->trace[2 * i] +
								                      channel->trace[2 * i + 1] * channel->trace[2 * i + 1]));
								channelmax = std::max(value, channelmax);
							}
							channelpick = 0;
							threshold = bottompickthreshold * channelmax;
							for (int i = 0; i < channel->samples && channelpick == 0; i++) {
								value = sqrt((double)(channel->trace[2 * i] * channel->trace[2 * i] +
								                      channel->trace[2 * i + 1] * channel->trace[2 * i + 1]));
								if (value >= threshold)
									channelpick = i;
							}
						}
						else {
							channelmax = 0.0;
							for (int i = 0; i < channel->samples; i++) {
								value = (double)(channel->trace[i]);
								channelmax = std::max(value, channelmax);
							}
							channelpick = 0;
							threshold = bottompickthreshold * channelmax;
							for (int i = 0; i < channel->samples && channelpick == 0; i++) {
								value = (double)(channel->trace[i]);
								if (value >= threshold)
									channelpick = i;
							}
						}

						/* set sonar altitude */
						channel->sonarAltitude = 0.00075 * channelpick * channel->sampleInterval;
					}
					else if (bottompickmode == MB7K2JSTAR_BOTTOMPICK_BATHYMETRY) {
						channel->sonarAltitude = (int)(750000.0 * ttime_min_use);
						if (channel->sonarAltitude == 0)
							channel->sonarAltitude = 1000 * altitude;
					}
					else {
						channel->sonarAltitude = 1000 * altitude;
					}

					/* apply gain if specified */
					if (gainmode == MB7K2JSTAR_SSGAIN_TVG_1OVERR) {
						channelpick = (int)(((double)channel->sonarAltitude) / 0.00075 / ((double)channel->sampleInterval));
						channelpick = std::max(channelpick, 1);
						for (int i = 0; i < channelpick; i++) {
							channel->trace[i] = (unsigned short)(gainfactor * channel->trace[i]);
						}
						for (int i = channelpick; i < channel->samples; i++) {
							factor = gainfactor * (((double)(i * i)) / ((double)(channelpick * channelpick)));
							channel->trace[i] = (unsigned short)(factor * channel->trace[i]);
						}
					}

					/* reset navigation and other values */
					if (navlon < 180.0)
						navlon = navlon + 360.0;
					if (navlon > 180.0)
						navlon = navlon - 360.0;
					channel->coordX = (int)(360000.0 * navlon);
					channel->coordY = (int)(360000.0 * navlat);
					channel->coordUnits = 2;
					channel->heading = (short)(100.0 * heading);
					channel->startDepth = sonardepth / channel->sampleInterval / 0.00000075;
					channel->sonarDepth = 1000 * sonardepth;
					channel->roll = (short)(32768 * roll / 180.0);
					channel->pitch = (short)(32768 * pitch / 180.0);

					/*----------------------------------------------------------------*/
					/* copy low frequency starboard sidescan to jstar storage */
					if (ssflip)
						channel = (struct mbsys_jstar_channel_struct *)&(ostore->ssport);
					else
						channel = (struct mbsys_jstar_channel_struct *)&(ostore->ssstbd);
					s7kchannel = (s7k_fsdwchannel *)&(istore->fsdwsslo.channel[1]);
					s7kssheader = (s7k_fsdwssheader *)&(istore->fsdwsslo.ssheader[1]);

					/* message header values */
					channel->message.start_marker = 0x1601;
					channel->message.version = 0;
					channel->message.session = 0;
					channel->message.type = 80;
					channel->message.command = 0;
					channel->message.subsystem = 20;
					if (ssflip)
						channel->message.channel = 0;
					else
						channel->message.channel = 1;
					channel->message.sequence = 0;
					channel->message.reserved = 0;
					channel->message.size = 0;

					/* Trace Header */
					channel->pingTime = 0;                   /* 0-3 : Ping Time in epoch seconds [since (1/1/1970)] (Protocol Version 8 onwards) */
					channel->startDepth = s7kssheader->startDepth; /* 4-7 : Starting depth (window offset) in samples. */
					channel->pingNum = s7kssheader->pingNum;       /* 8-11: Ping number (increments with ping) ** */
					for (int i = 0; i < 2; i++) /* 12-15 */
						channel->reserved1[i] = 0; /* 16-27 */
					channel->msb = 0; /* 16-17 */
					channel->lsb1 = 0; /* 18-19 */
					channel->lsb2 = 0; /* 20-21 */
					for (int i = 0; i < 3; i++) /* 22-27 */
						channel->reserved2[i] = 0;

					channel->traceIDCode = 1; /* 28-29 : ID Code (always 1 => seismic data) ** */

					channel->validityFlag = 0;
					channel->reserved3 = 0; /* 32-33 */
					channel->dataFormat = s7kssheader->dataFormat; /* 34-35 : DataFormatType */
					                                               /*   0 = 1 short  per sample  - envelope data */
					                                               /*   1 = 2 shorts per sample, - stored as real(1), imag(1), */
					                                               /*   2 = 1 short  per sample  - before matched filter */
					                                               /*   3 = 1 short  per sample  - real part analytic signal */
					                                               /*   4 = 1 short  per sample  - pixel data / ceros data */
					for (int i = 0; i < 2; i++) {
						channel->reserved4[i] = 0;   /* 40-43 : Reserved – Do not use */
					}
					channel->kmOfPipe = 0;      /* 44-47 : Kilometers of Pipe - See Validity Flag (bytes 30 – 31). */
					for (int i = 0; i < 16; i++) {
						channel->reserved5[i] = 0;  /* 48-79 : Reserved – Do not use */
					}

					/* -------------------------------------------------------------------- */
					/* Navigation data :                                                    */
					/* If the coorUnits are seconds(2), the x values represent longitude    */
					/* and the y values represent latitude.  A positive value designates    */
					/* the number of seconds east of Greenwich Meridian or north of the     */
					/* equator.                                                             */
					/* -------------------------------------------------------------------- */
					channel->coordX = 0;  /* 80-83 : mm or 10000 * (Minutes of Arc) */
					channel->coordY = 0;  /* 84-87 : mm or 10000 * (Minutes of Arc) */
					channel->coordUnits = 0;   /* 88-89 : Units of coordinates - 1->length (x /y), 2->seconds of arc */
					for (int i = 0; i < 24; i++)
						channel->annotation[i] = 0;          /* 90-113 : Annotation string */
					channel->samples = s7kssheader->samples; /* 114-115 : Samples in this packet ** */
					                                         /* Note:  Large sample sizes require multiple packets. */
					channel->sampleInterval = s7kssheader->sampleInterval; /* 116-119 : Sample interval in ns of stored data ** */
					channel->ADCGain = s7kssheader->ADCGain;               /* 120-121 : Gain factor of ADC */
					channel->pulsePower = 0;  /* 122-123 : user pulse power setting (0 - 100) percent */
					channel->reserved6 = 0;  /* 124-125 : correlated data 1 - No, 2 - Yes */
					channel->startFreq = 0;   /* 126-127 : Starting frequency in 10 * Hz */
					channel->endFreq = 0;     /* 128-129 : Ending frequency in 10 * Hz */
					channel->sweepLength = 0; /* 130-131 : Sweep length in ms */
					channel->pressure = 0; /* 132-135 : Pressure in Milli PSI (1 unit = 1/1000 PSI)
                                  *           See Validity Flag (bytes 30-31) */
					channel->sonarDepth = 0; /* 136-139 : Depth in Millimeters (if not = 0)
                                  *           See Validity Flag (bytes 30-31). */
					channel->sampleFreq = 0;       /* 140-141 : alias Frequency (sample frequency / 2) */
					channel->pulseID = s7ksegyheader->pulseID;           /* 142-143 : Unique pulse identifier */
					channel->sonarAltitude = 0;           /* 144-147 : Altitude in Millimeters
                                  *           A zero implies not filled. See Validity Flag (bytes 30-31) */
					channel->soundspeed = 0.0;            /* 148-151 : Sound Speed in Meters per Second.
                                  *           See Validity Flag (bytes 30-31). */
					channel->mixerFrequency = 0.0;        /* 152-155 : Mixer Frequency in Hertz
                                  *           For single pulses systems this should
                                  *           be close to the center frequency.
                                  *           For multi pulse systems this should
                                  *           be the approximate center frequency
                                  *           of the span of all the pulses. */

					channel->year = istore->time_i[0];   /* 156-157 : Year data recorded (CPU time) */
					channel->day = time_j[1];            /* 158-159 : day */
					channel->hour = istore->time_i[3];   /* 160-161 : hour */
					channel->minute = istore->time_i[4]; /* 162-163 : minute */
					channel->second = istore->time_i[5]; /* 164-165 : second */
					channel->timeBasis = 3;              /* 166-167 : Always 3 (other not specified by standard) */
					channel->weightingFactor =
					    s7kssheader->weightingFactor; /* 168-169 :  weighting factor for block floating point expansion */
					                                  /*            -- defined as 2 -N volts for lsb */
					channel->numberPulses =  0; /* 170-171 : Number of Pulses in the Water */
					/* -------------------------------------------------------------------- */
					/* From pitch/roll/temp/heading sensor */
					/* -------------------------------------------------------------------- */
					channel->heading =
					    s7kssheader->heading;            /* 172-173 : Compass heading (100 * degrees) -180.00 to 180.00 degrees */
					channel->pitch = s7kssheader->pitch; /* 174-175 : Pitch */
					channel->roll = s7kssheader->roll;   /* 176-177 : Roll */
					channel->temperature = s7kssheader->temperature; /* 178-179 : Temperature (10 * degrees C) */
					/* -------------------------------------------------------------------- */
					/* User defined area from 180-239                                       */
					/* -------------------------------------------------------------------- */
					channel->reserved9 = 0;                /* 180-181 : Reserved */
					channel->triggerSource = s7kssheader->trigSource; /* 182-183 : TriggerSource (0 = internal, 1 = external) */
					channel->markNumber = s7kssheader->markNumber; /* 184-185 : Mark Number (0 = no mark) */
					channel->NMEAHour = 0;                         /* 186-187 : Hour */
					channel->NMEAMinutes = 0;                      /* 188-189 : Minutes */
					channel->NMEASeconds = 0;                      /* 190-191 : Seconds */
					channel->NMEACourse = 0;                       /* 192-193 : Course */
					channel->NMEASpeed = 0;                        /* 194-195 : Speed */
					channel->NMEADay = 0;                          /* 196-197 : Day */
					channel->NMEAYear = 0;                         /* 198-199 : Year */
					channel->millisecondsToday =
					    0.001 * istore->time_i[6] /* 200-203 : Millieconds today */
					    + 1000 * (istore->time_i[5] + 60.0 * (istore->time_i[4] + 60.0 * istore->time_i[3]));
					channel->ADCMax = s7kssheader->ADCMax; /* 204-205 : Maximum absolute value for ADC samples for this packet */
					channel->reserved10 = 0;   /* 206-207 : Reserved */
					channel->reserved11 = 0; /* 208-209 : Reserved */
					for (int i = 0; i < 6; i++)
						channel->softwareVersion[i] = 0; /* 210-215 : Software version number */
					/* Following items are not in X-Star */
					channel->sphericalCorrection = s7ksegyheader->sphericalCorrection;
												/* 216-219 : Initial spherical correction factor (useful for multiping /*/
					                            /* deep application) * 100 */
					channel->packetNum = s7kssheader->packetNum; /* 220-221 : Packet number (1 - N) (Each ping starts with packet 1) */
					channel->ADCDecimation = 0; /* 222-223 : A/D decimation before FFT */
					channel->reserved12 = 0;       /* 224-225 : Reserved */
					channel->temperature = 0;        /* 226-227 : Water Temperature in Units of 1/10 Degree C.
                               *           See Validity Flag (bytes 30-31).*/
					channel->layback = 0;            /* 227-231 : Layback
                               *           Distance to the sonar in meters. */
					channel->reserved13 = 0;           /* 232-235 : Reserved */
					channel->cableOut = 0;           /* 236-239 : Cable Out in Decimeters
                               *           See Validity Flag (bytes 30-31). */
					channel->reserved14 = 0;         /* 236-239 : Reserved */

					/* allocate memory for the trace */
					if (channel->dataFormat == 1)
						shortspersample = 2;
					else
						shortspersample = 1;
					trace_size = shortspersample * channel->samples * sizeof(short);
					channel->message.size = shortspersample * channel->samples * sizeof(short);
					if (channel->trace_alloc < trace_size) {
						if ((status = mb_reallocd(verbose, __FILE__, __LINE__, trace_size, (void **)&(channel->trace), &error)) ==
						    MB_SUCCESS) {
							channel->trace_alloc = trace_size;
						}
					}

					/* copy the trace */
					if (status == MB_SUCCESS) {
						if (smooth > 0 && channel->dataFormat == 0) {
							datashort = (unsigned short *)s7kchannel->data;
							for (int i = 0; i < channel->samples; i++) {
								int n = 0;
								channel->trace[i] = 0.0;
								for (int j = std::max(i - smooth, 0); j < std::min(i + smooth, channel->samples - 1); j++) {
									channel->trace[i] += datashort[j];
									n++;
								}
								channel->trace[i] /= n;
							}
						}
						else if (smooth < 0 && channel->dataFormat == 0) {
							datashort = (unsigned short *)s7kchannel->data;
							for (int i = 0; i < channel->samples; i++) {
								int n = 0;
								value = 0.0;
								for (int j = std::max(i + smooth, 0); j < std::min(i - smooth, channel->samples - 1); j++) {
									value += datashort[j] * datashort[j];
									n++;
								}
								channel->trace[i] = (unsigned int)(sqrt(value) / n);
							}
						}
						else {
							data = (char *)channel->trace;
							for (unsigned int i = 0; i < trace_size; i++) {
								data[i] = s7kchannel->data[i];
							}
						}
					}

					/* set the sonar altitude using the specified mode */
					if (bottompickmode == MB7K2JSTAR_BOTTOMPICK_ARRIVAL) {
						/* get bottom arrival in trace */
						if (channel->dataFormat == MBSYS_JSTAR_TRACEFORMAT_ANALYTIC) {
							channelmax = 0.0;
							for (int i = 0; i < channel->samples; i++) {
								value = sqrt((double)(channel->trace[2 * i] * channel->trace[2 * i] +
								                      channel->trace[2 * i + 1] * channel->trace[2 * i + 1]));
								channelmax = std::max(value, channelmax);
							}
							channelpick = 0;
							threshold = bottompickthreshold * channelmax;
							for (int i = 0; i < channel->samples && channelpick == 0; i++) {
								value = sqrt((double)(channel->trace[2 * i] * channel->trace[2 * i] +
								                      channel->trace[2 * i + 1] * channel->trace[2 * i + 1]));
								if (value >= threshold)
									channelpick = i;
							}
						}
						else {
							channelmax = 0.0;
							for (int i = 0; i < channel->samples; i++) {
								value = (double)(channel->trace[i]);
								channelmax = std::max(value, channelmax);
							}
							channelpick = 0;
							threshold = bottompickthreshold * channelmax;
							for (int i = 0; i < channel->samples && channelpick == 0; i++) {
								value = (double)(channel->trace[i]);
								if (value >= threshold)
									channelpick = i;
							}
						}

						/* set sonar altitude */
						channel->sonarAltitude = 0.00075 * channelpick * channel->sampleInterval;
					}
					else if (bottompickmode == MB7K2JSTAR_BOTTOMPICK_BATHYMETRY) {
						channel->sonarAltitude = (int)(750000.0 * ttime_min_use);
						if (channel->sonarAltitude == 0)
							channel->sonarAltitude = 1000 * altitude;
					}
					else {
						channel->sonarAltitude = 1000 * altitude;
					}

					/* apply gain if specified */
					if (gainmode == MB7K2JSTAR_SSGAIN_TVG_1OVERR) {
						channelpick = (int)(((double)channel->sonarAltitude) / 0.00075 / ((double)channel->sampleInterval));
						channelpick = std::max(channelpick, 1);
						for (int i = channelpick; i < channel->samples; i++) {
							factor = gainfactor * (((double)(i * i)) / ((double)(channelpick * channelpick)));
							channel->trace[i] = (unsigned short)(factor * channel->trace[i]);
						}
					}

					/* reset navigation and other values */
					if (navlon < 180.0)
						navlon = navlon + 360.0;
					if (navlon > 180.0)
						navlon = navlon - 360.0;
					channel->coordX = (int)(360000.0 * navlon);
					channel->coordY = (int)(360000.0 * navlat);
					channel->coordUnits = 2;
					channel->heading = (short)(100.0 * heading);
					channel->startDepth = sonardepth / channel->sampleInterval / 0.00000075;
					channel->sonarDepth = 1000 * sonardepth;
					channel->roll = (short)(32768 * roll / 180.0);
					channel->pitch = (short)(32768 * pitch / 180.0);

					/* write the record */
					nwritesslo++;
					mb_write_ping(verbose, ombio_ptr, ostore_ptr, &error);
				}
			}

			/* handle high frequency sidescan data */
			else if (status == MB_SUCCESS && kind == MB_DATA_SIDESCAN3) {
				nreadsshi++;

				/* output data if desired */
				if (extract_sshigh && nreadnav1 > 0 && oktowrite >= MBES_ONLINE_COUNT) {
					/* set overall parameters */
					ostore->kind = MB_DATA_SIDESCAN2;
					ostore->subsystem = 21;

					/*----------------------------------------------------------------*/
					/* copy high frequency port sidescan to jstar storage */
					if (ssflip)
						channel = (struct mbsys_jstar_channel_struct *)&(ostore->ssstbd);
					else
						channel = (struct mbsys_jstar_channel_struct *)&(ostore->ssport);
					s7kchannel = (s7k_fsdwchannel *)&(istore->fsdwsshi.channel[0]);
					s7kssheader = (s7k_fsdwssheader *)&(istore->fsdwsshi.ssheader[0]);

					/* message header values */
					channel->message.start_marker = 0x1601;
					channel->message.version = 0;
					channel->message.session = 0;
					channel->message.type = 80;
					channel->message.command = 0;
					channel->message.subsystem = 21;
					channel->message.channel = 0;
					channel->message.sequence = 0;
					channel->message.reserved = 0;
					channel->message.size = 0;

					/* Trace Header */
					channel->pingTime = 0;                   /* 0-3 : Ping Time in epoch seconds [since (1/1/1970)] (Protocol Version 8 onwards) */
					channel->startDepth = s7kssheader->startDepth; /* 4-7 : Starting depth (window offset) in samples. */
					channel->pingNum = s7kssheader->pingNum;       /* 8-11: Ping number (increments with ping) ** */
					for (int i = 0; i < 2; i++) /* 12-15 */
						channel->reserved1[i] = 0; /* 16-27 */
					channel->msb = 0; /* 16-17 */
					channel->lsb1 = 0; /* 18-19 */
					channel->lsb2 = 0; /* 20-21 */
					for (int i = 0; i < 3; i++) /* 22-27 */
						channel->reserved2[i] = 0;

					channel->traceIDCode = 1; /* 28-29 : ID Code (always 1 => seismic data) ** */

					channel->validityFlag = 0;
					channel->reserved3 = 0; /* 32-33 */
					channel->dataFormat = s7kssheader->dataFormat; /* 34-35 : DataFormatType */
					                                               /*   0 = 1 short  per sample  - envelope data */
					                                               /*   1 = 2 shorts per sample, - stored as real(1), imag(1), */
					                                               /*   2 = 1 short  per sample  - before matched filter */
					                                               /*   3 = 1 short  per sample  - real part analytic signal */
					                                               /*   4 = 1 short  per sample  - pixel data / ceros data */
					for (int i = 0; i < 2; i++) {
						channel->reserved4[i] = 0;   /* 40-43 : Reserved – Do not use */
					}
					channel->kmOfPipe = 0;      /* 44-47 : Kilometers of Pipe - See Validity Flag (bytes 30 – 31). */
					for (int i = 0; i < 16; i++) {
						channel->reserved5[i] = 0;  /* 48-79 : Reserved – Do not use */
					}

					/* -------------------------------------------------------------------- */
					/* Navigation data :                                                    */
					/* If the coorUnits are seconds(2), the x values represent longitude    */
					/* and the y values represent latitude.  A positive value designates    */
					/* the number of seconds east of Greenwich Meridian or north of the     */
					/* equator.                                                             */
					/* -------------------------------------------------------------------- */
					channel->coordX = 0;  /* 80-83 : mm or 10000 * (Minutes of Arc) */
					channel->coordY = 0;  /* 84-87 : mm or 10000 * (Minutes of Arc) */
					channel->coordUnits = 0;   /* 88-89 : Units of coordinates - 1->length (x /y), 2->seconds of arc */
					for (int i = 0; i < 24; i++)
						channel->annotation[i] = 0;          /* 90-113 : Annotation string */
					channel->samples = s7kssheader->samples; /* 114-115 : Samples in this packet ** */
					                                         /* Note:  Large sample sizes require multiple packets. */
					channel->sampleInterval = s7kssheader->sampleInterval; /* 116-119 : Sample interval in ns of stored data ** */
					channel->ADCGain = s7kssheader->ADCGain;               /* 120-121 : Gain factor of ADC */
					channel->pulsePower = 0;  /* 122-123 : user pulse power setting (0 - 100) percent */
					channel->reserved6 = 0;  /* 124-125 : correlated data 1 - No, 2 - Yes */
					channel->startFreq = 0;   /* 126-127 : Starting frequency in 10 * Hz */
					channel->endFreq = 0;     /* 128-129 : Ending frequency in 10 * Hz */
					channel->sweepLength = 0; /* 130-131 : Sweep length in ms */
					channel->pressure = 0; /* 132-135 : Pressure in Milli PSI (1 unit = 1/1000 PSI)
                                  *           See Validity Flag (bytes 30-31) */
					channel->sonarDepth = 0; /* 136-139 : Depth in Millimeters (if not = 0)
                                  *           See Validity Flag (bytes 30-31). */
					channel->sampleFreq = 0;       /* 140-141 : alias Frequency (sample frequency / 2) */
					channel->pulseID = s7ksegyheader->pulseID;           /* 142-143 : Unique pulse identifier */
					channel->sonarAltitude = 0;           /* 144-147 : Altitude in Millimeters
                                  *           A zero implies not filled. See Validity Flag (bytes 30-31) */
					channel->soundspeed = 0.0;            /* 148-151 : Sound Speed in Meters per Second.
                                  *           See Validity Flag (bytes 30-31). */
					channel->mixerFrequency = 0.0;        /* 152-155 : Mixer Frequency in Hertz
                                  *           For single pulses systems this should
                                  *           be close to the center frequency.
                                  *           For multi pulse systems this should
                                  *           be the approximate center frequency
                                  *           of the span of all the pulses. */

					channel->year = istore->time_i[0];   /* 156-157 : Year data recorded (CPU time) */
					channel->day = time_j[1];            /* 158-159 : day */
					channel->hour = istore->time_i[3];   /* 160-161 : hour */
					channel->minute = istore->time_i[4]; /* 162-163 : minute */
					channel->second = istore->time_i[5]; /* 164-165 : second */
					channel->timeBasis = 3;              /* 166-167 : Always 3 (other not specified by standard) */
					channel->weightingFactor =
					    s7kssheader->weightingFactor; /* 168-169 :  weighting factor for block floating point expansion */
					                                  /*            -- defined as 2 -N volts for lsb */
					channel->numberPulses =  0; /* 170-171 : Number of Pulses in the Water */
					/* -------------------------------------------------------------------- */
					/* From pitch/roll/temp/heading sensor */
					/* -------------------------------------------------------------------- */
					channel->heading =
					    s7kssheader->heading;            /* 172-173 : Compass heading (100 * degrees) -180.00 to 180.00 degrees */
					channel->pitch = s7kssheader->pitch; /* 174-175 : Pitch */
					channel->roll = s7kssheader->roll;   /* 176-177 : Roll */
					channel->temperature = s7kssheader->temperature; /* 178-179 : Temperature (10 * degrees C) */
					/* -------------------------------------------------------------------- */
					/* User defined area from 180-239                                       */
					/* -------------------------------------------------------------------- */
					channel->reserved9 = 0;                /* 180-181 : Reserved */
					channel->triggerSource = s7kssheader->trigSource; /* 182-183 : TriggerSource (0 = internal, 1 = external) */
					channel->markNumber = s7kssheader->markNumber; /* 184-185 : Mark Number (0 = no mark) */
					channel->NMEAHour = 0;                         /* 186-187 : Hour */
					channel->NMEAMinutes = 0;                      /* 188-189 : Minutes */
					channel->NMEASeconds = 0;                      /* 190-191 : Seconds */
					channel->NMEACourse = 0;                       /* 192-193 : Course */
					channel->NMEASpeed = 0;                        /* 194-195 : Speed */
					channel->NMEADay = 0;                          /* 196-197 : Day */
					channel->NMEAYear = 0;                         /* 198-199 : Year */
					channel->millisecondsToday =
					    0.001 * istore->time_i[6] /* 200-203 : Millieconds today */
					    + 1000 * (istore->time_i[5] + 60.0 * (istore->time_i[4] + 60.0 * istore->time_i[3]));
					channel->ADCMax = s7kssheader->ADCMax; /* 204-205 : Maximum absolute value for ADC samples for this packet */
					channel->reserved10 = 0;   /* 206-207 : Reserved */
					channel->reserved11 = 0; /* 208-209 : Reserved */
					for (int i = 0; i < 6; i++)
						channel->softwareVersion[i] = 0; /* 210-215 : Software version number */
					/* Following items are not in X-Star */
					channel->sphericalCorrection = s7ksegyheader->sphericalCorrection;
												/* 216-219 : Initial spherical correction factor (useful for multiping /*/
					                            /* deep application) * 100 */
					channel->packetNum = s7kssheader->packetNum; /* 220-221 : Packet number (1 - N) (Each ping starts with packet 1) */
					channel->ADCDecimation = 0; /* 222-223 : A/D decimation before FFT */
					channel->reserved12 = 0;       /* 224-225 : Reserved */
					channel->temperature = 0;        /* 226-227 : Water Temperature in Units of 1/10 Degree C.
                               *           See Validity Flag (bytes 30-31).*/
					channel->layback = 0;            /* 227-231 : Layback
                               *           Distance to the sonar in meters. */
					channel->reserved13 = 0;           /* 232-235 : Reserved */
					channel->cableOut = 0;           /* 236-239 : Cable Out in Decimeters
                               *           See Validity Flag (bytes 30-31). */
					channel->reserved14 = 0;         /* 236-239 : Reserved */

					/* allocate memory for the trace */
					if (channel->dataFormat == 1)
						shortspersample = 2;
					else
						shortspersample = 1;
					unsigned int trace_size = shortspersample * channel->samples * sizeof(short);
					channel->message.size = shortspersample * channel->samples * sizeof(short);
					if (channel->trace_alloc < trace_size) {
						if ((status = mb_reallocd(verbose, __FILE__, __LINE__, trace_size, (void **)&(channel->trace), &error)) ==
						    MB_SUCCESS) {
							channel->trace_alloc = trace_size;
						}
					}

					/* copy the trace */
					if (status == MB_SUCCESS) {
						if (smooth > 0 && channel->dataFormat == 0) {
							datashort = (unsigned short *)s7kchannel->data;
							for (int i = 0; i < channel->samples; i++) {
								int n = 0;
								channel->trace[i] = 0.0;
								for (int j = std::max(i - smooth, 0); j < std::min(i + smooth, channel->samples - 1); j++) {
									channel->trace[i] += datashort[j];
									n++;
								}
								channel->trace[i] /= n;
							}
						}
						else if (smooth < 0 && channel->dataFormat == 0) {
							datashort = (unsigned short *)s7kchannel->data;
							for (int i = 0; i < channel->samples; i++) {
								int n = 0;
								value = 0.0;
								for (int j = std::max(i + smooth, 0); j < std::min(i - smooth, channel->samples - 1); j++) {
									value += datashort[j] * datashort[j];
									n++;
								}
								channel->trace[i] = (unsigned int)(sqrt(value) / n);
							}
						}
						else {
							data = (char *)channel->trace;
							for (unsigned int i = 0; i < trace_size; i++) {
								data[i] = s7kchannel->data[i];
							}
						}
					}

					/* set the sonar altitude using the specified mode */
					if (bottompickmode == MB7K2JSTAR_BOTTOMPICK_ARRIVAL) {
						/* get bottom arrival in trace */
						if (channel->dataFormat == MBSYS_JSTAR_TRACEFORMAT_ANALYTIC) {
							channelmax = 0.0;
							for (int i = 0; i < channel->samples; i++) {
								value = sqrt((double)(channel->trace[2 * i] * channel->trace[2 * i] +
								                      channel->trace[2 * i + 1] * channel->trace[2 * i + 1]));
								channelmax = std::max(value, channelmax);
							}
							channelpick = 0;
							threshold = bottompickthreshold * channelmax;
							for (int i = 0; i < channel->samples && channelpick == 0; i++) {
								value = sqrt((double)(channel->trace[2 * i] * channel->trace[2 * i] +
								                      channel->trace[2 * i + 1] * channel->trace[2 * i + 1]));
								if (value >= threshold)
									channelpick = i;
							}
						}
						else {
							channelmax = 0.0;
							for (int i = 0; i < channel->samples; i++) {
								value = (double)(channel->trace[i]);
								channelmax = std::max(value, channelmax);
							}
							channelpick = 0;
							threshold = bottompickthreshold * channelmax;
							for (int i = 0; i < channel->samples && channelpick == 0; i++) {
								value = (double)(channel->trace[i]);
								if (value >= threshold)
									channelpick = i;
							}
						}

						/* set sonar altitude */
						channel->sonarAltitude = 0.00075 * channelpick * channel->sampleInterval;
					}
					else if (bottompickmode == MB7K2JSTAR_BOTTOMPICK_BATHYMETRY) {
						channel->sonarAltitude = (int)(750000.0 * ttime_min_use);
						if (channel->sonarAltitude == 0)
							channel->sonarAltitude = 1000 * altitude;
					}
					else {
						channel->sonarAltitude = 1000 * altitude;
					}

					/* reset navigation and other values */
					if (navlon < 180.0)
						navlon = navlon + 360.0;
					if (navlon > 180.0)
						navlon = navlon - 360.0;
					channel->coordX = (int)(360000.0 * navlon);
					channel->coordY = (int)(360000.0 * navlat);
					channel->coordUnits = 2;
					channel->heading = (short)(100.0 * heading);
					channel->startDepth = sonardepth / channel->sampleInterval / 0.00000075;
					channel->sonarDepth = 1000 * sonardepth;
					channel->roll = (short)(32768 * roll / 180.0);
					channel->pitch = (short)(32768 * pitch / 180.0);

					/*----------------------------------------------------------------*/
					/* copy high frequency starboard sidescan to jstar storage */
					if (ssflip)
						channel = (struct mbsys_jstar_channel_struct *)&(ostore->ssport);
					else
						channel = (struct mbsys_jstar_channel_struct *)&(ostore->ssstbd);
					s7kchannel = (s7k_fsdwchannel *)&(istore->fsdwsshi.channel[1]);
					s7kssheader = (s7k_fsdwssheader *)&(istore->fsdwsshi.ssheader[1]);

					/* message header values */
					channel->message.start_marker = 0x1601;
					channel->message.version = 0;
					channel->message.session = 0;
					channel->message.type = 80;
					channel->message.command = 0;
					channel->message.subsystem = 21;
					channel->message.channel = 1;
					channel->message.sequence = 0;
					channel->message.reserved = 0;
					channel->message.size = 0;

					/* Trace Header */
					channel->pingTime = 0;                   /* 0-3 : Ping Time in epoch seconds [since (1/1/1970)] (Protocol Version 8 onwards) */
					channel->startDepth = s7kssheader->startDepth; /* 4-7 : Starting depth (window offset) in samples. */
					channel->pingNum = s7kssheader->pingNum;       /* 8-11: Ping number (increments with ping) ** */
					for (int i = 0; i < 2; i++) /* 12-15 */
						channel->reserved1[i] = 0; /* 16-27 */
					channel->msb = 0; /* 16-17 */
					channel->lsb1 = 0; /* 18-19 */
					channel->lsb2 = 0; /* 20-21 */
					for (int i = 0; i < 3; i++) /* 22-27 */
						channel->reserved2[i] = 0;

					channel->traceIDCode = 1; /* 28-29 : ID Code (always 1 => seismic data) ** */

					channel->validityFlag = 0;
					channel->reserved3 = 0; /* 32-33 */
					channel->dataFormat = s7kssheader->dataFormat; /* 34-35 : DataFormatType */
					                                               /*   0 = 1 short  per sample  - envelope data */
					                                               /*   1 = 2 shorts per sample, - stored as real(1), imag(1), */
					                                               /*   2 = 1 short  per sample  - before matched filter */
					                                               /*   3 = 1 short  per sample  - real part analytic signal */
					                                               /*   4 = 1 short  per sample  - pixel data / ceros data */
					for (int i = 0; i < 2; i++) {
						channel->reserved4[i] = 0;   /* 40-43 : Reserved – Do not use */
					}
					channel->kmOfPipe = 0;      /* 44-47 : Kilometers of Pipe - See Validity Flag (bytes 30 – 31). */
					for (int i = 0; i < 16; i++) {
						channel->reserved5[i] = 0;  /* 48-79 : Reserved – Do not use */
					}

					/* -------------------------------------------------------------------- */
					/* Navigation data :                                                    */
					/* If the coorUnits are seconds(2), the x values represent longitude    */
					/* and the y values represent latitude.  A positive value designates    */
					/* the number of seconds east of Greenwich Meridian or north of the     */
					/* equator.                                                             */
					/* -------------------------------------------------------------------- */
					channel->coordX = 0;  /* 80-83 : mm or 10000 * (Minutes of Arc) */
					channel->coordY = 0;  /* 84-87 : mm or 10000 * (Minutes of Arc) */
					channel->coordUnits = 0;   /* 88-89 : Units of coordinates - 1->length (x /y), 2->seconds of arc */
					for (int i = 0; i < 24; i++)
						channel->annotation[i] = 0;          /* 90-113 : Annotation string */
					channel->samples = s7kssheader->samples; /* 114-115 : Samples in this packet ** */
					                                         /* Note:  Large sample sizes require multiple packets. */
					channel->sampleInterval = s7kssheader->sampleInterval; /* 116-119 : Sample interval in ns of stored data ** */
					channel->ADCGain = s7kssheader->ADCGain;               /* 120-121 : Gain factor of ADC */
					channel->pulsePower = 0;  /* 122-123 : user pulse power setting (0 - 100) percent */
					channel->reserved6 = 0;  /* 124-125 : correlated data 1 - No, 2 - Yes */
					channel->startFreq = 0;   /* 126-127 : Starting frequency in 10 * Hz */
					channel->endFreq = 0;     /* 128-129 : Ending frequency in 10 * Hz */
					channel->sweepLength = 0; /* 130-131 : Sweep length in ms */
					channel->pressure = 0; /* 132-135 : Pressure in Milli PSI (1 unit = 1/1000 PSI)
                                  *           See Validity Flag (bytes 30-31) */
					channel->sonarDepth = 0; /* 136-139 : Depth in Millimeters (if not = 0)
                                  *           See Validity Flag (bytes 30-31). */
					channel->sampleFreq = 0;       /* 140-141 : alias Frequency (sample frequency / 2) */
					channel->pulseID = s7ksegyheader->pulseID;           /* 142-143 : Unique pulse identifier */
					channel->sonarAltitude = 0;           /* 144-147 : Altitude in Millimeters
                                  *           A zero implies not filled. See Validity Flag (bytes 30-31) */
					channel->soundspeed = 0.0;            /* 148-151 : Sound Speed in Meters per Second.
                                  *           See Validity Flag (bytes 30-31). */
					channel->mixerFrequency = 0.0;        /* 152-155 : Mixer Frequency in Hertz
                                  *           For single pulses systems this should
                                  *           be close to the center frequency.
                                  *           For multi pulse systems this should
                                  *           be the approximate center frequency
                                  *           of the span of all the pulses. */

					channel->year = istore->time_i[0];   /* 156-157 : Year data recorded (CPU time) */
					channel->day = time_j[1];            /* 158-159 : day */
					channel->hour = istore->time_i[3];   /* 160-161 : hour */
					channel->minute = istore->time_i[4]; /* 162-163 : minute */
					channel->second = istore->time_i[5]; /* 164-165 : second */
					channel->timeBasis = 3;              /* 166-167 : Always 3 (other not specified by standard) */
					channel->weightingFactor =
					    s7kssheader->weightingFactor; /* 168-169 :  weighting factor for block floating point expansion */
					                                  /*            -- defined as 2 -N volts for lsb */
					channel->numberPulses =  0; /* 170-171 : Number of Pulses in the Water */
					/* -------------------------------------------------------------------- */
					/* From pitch/roll/temp/heading sensor */
					/* -------------------------------------------------------------------- */
					channel->heading =
					    s7kssheader->heading;            /* 172-173 : Compass heading (100 * degrees) -180.00 to 180.00 degrees */
					channel->pitch = s7kssheader->pitch; /* 174-175 : Pitch */
					channel->roll = s7kssheader->roll;   /* 176-177 : Roll */
					channel->temperature = s7kssheader->temperature; /* 178-179 : Temperature (10 * degrees C) */
					/* -------------------------------------------------------------------- */
					/* User defined area from 180-239                                       */
					/* -------------------------------------------------------------------- */
					channel->reserved9 = 0;                /* 180-181 : Reserved */
					channel->triggerSource = s7kssheader->trigSource; /* 182-183 : TriggerSource (0 = internal, 1 = external) */
					channel->markNumber = s7kssheader->markNumber; /* 184-185 : Mark Number (0 = no mark) */
					channel->NMEAHour = 0;                         /* 186-187 : Hour */
					channel->NMEAMinutes = 0;                      /* 188-189 : Minutes */
					channel->NMEASeconds = 0;                      /* 190-191 : Seconds */
					channel->NMEACourse = 0;                       /* 192-193 : Course */
					channel->NMEASpeed = 0;                        /* 194-195 : Speed */
					channel->NMEADay = 0;                          /* 196-197 : Day */
					channel->NMEAYear = 0;                         /* 198-199 : Year */
					channel->millisecondsToday =
					    0.001 * istore->time_i[6] /* 200-203 : Millieconds today */
					    + 1000 * (istore->time_i[5] + 60.0 * (istore->time_i[4] + 60.0 * istore->time_i[3]));
					channel->ADCMax = s7kssheader->ADCMax; /* 204-205 : Maximum absolute value for ADC samples for this packet */
					channel->reserved10 = 0;   /* 206-207 : Reserved */
					channel->reserved11 = 0; /* 208-209 : Reserved */
					for (int i = 0; i < 6; i++)
						channel->softwareVersion[i] = 0; /* 210-215 : Software version number */
					/* Following items are not in X-Star */
					channel->sphericalCorrection = s7ksegyheader->sphericalCorrection;
												/* 216-219 : Initial spherical correction factor (useful for multiping /*/
					                            /* deep application) * 100 */
					channel->packetNum = s7kssheader->packetNum; /* 220-221 : Packet number (1 - N) (Each ping starts with packet 1) */
					channel->ADCDecimation = 0; /* 222-223 : A/D decimation before FFT */
					channel->reserved12 = 0;       /* 224-225 : Reserved */
					channel->temperature = 0;        /* 226-227 : Water Temperature in Units of 1/10 Degree C.
                               *           See Validity Flag (bytes 30-31).*/
					channel->layback = 0;            /* 227-231 : Layback
                               *           Distance to the sonar in meters. */
					channel->reserved13 = 0;           /* 232-235 : Reserved */
					channel->cableOut = 0;           /* 236-239 : Cable Out in Decimeters
                               *           See Validity Flag (bytes 30-31). */
					channel->reserved14 = 0;         /* 236-239 : Reserved */

					/* allocate memory for the trace */
					if (channel->dataFormat == 1)
						shortspersample = 2;
					else
						shortspersample = 1;
					trace_size = shortspersample * channel->samples * sizeof(short);
					channel->message.size = shortspersample * channel->samples * sizeof(short);
					if (channel->trace_alloc < trace_size) {
						if ((status = mb_reallocd(verbose, __FILE__, __LINE__, trace_size, (void **)&(channel->trace), &error)) ==
						    MB_SUCCESS) {
							channel->trace_alloc = trace_size;
						}
					}

					/* copy the trace */
					if (status == MB_SUCCESS) {
						if (smooth > 0 && channel->dataFormat == 0) {
							datashort = (unsigned short *)s7kchannel->data;
							for (int i = 0; i < channel->samples; i++) {
								int n = 0;
								channel->trace[i] = 0.0;
								for (int j = std::max(i - smooth, 0); j < std::min(i + smooth, channel->samples - 1); j++) {
									channel->trace[i] += datashort[j];
									n++;
								}
								channel->trace[i] /= n;
							}
						}
						else if (smooth < 0 && channel->dataFormat == 0) {
							datashort = (unsigned short *)s7kchannel->data;
							for (int i = 0; i < channel->samples; i++) {
								int n = 0;
								value = 0.0;
								for (int j = std::max(i + smooth, 0); j < std::min(i - smooth, channel->samples - 1); j++) {
									value += datashort[j] * datashort[j];
									n++;
								}
								channel->trace[i] = (unsigned int)(sqrt(value) / n);
							}
						}
						else {
							data = (char *)channel->trace;
							for (unsigned int i = 0; i < trace_size; i++) {
								data[i] = s7kchannel->data[i];
							}
						}
					}

					/* set the sonar altitude using the specified mode */
					if (bottompickmode == MB7K2JSTAR_BOTTOMPICK_ARRIVAL) {
						/* get bottom arrival in trace */
						if (channel->dataFormat == MBSYS_JSTAR_TRACEFORMAT_ANALYTIC) {
							channelmax = 0.0;
							for (int i = 0; i < channel->samples; i++) {
								value = sqrt((double)(channel->trace[2 * i] * channel->trace[2 * i] +
								                      channel->trace[2 * i + 1] * channel->trace[2 * i + 1]));
								channelmax = std::max(value, channelmax);
							}
							channelpick = 0;
							threshold = bottompickthreshold * channelmax;
							for (int i = 0; i < channel->samples && channelpick == 0; i++) {
								value = sqrt((double)(channel->trace[2 * i] * channel->trace[2 * i] +
								                      channel->trace[2 * i + 1] * channel->trace[2 * i + 1]));
								if (value >= threshold)
									channelpick = i;
							}
						}
						else {
							channelmax = 0.0;
							for (int i = 0; i < channel->samples; i++) {
								value = (double)(channel->trace[i]);
								channelmax = std::max(value, channelmax);
							}
							channelpick = 0;
							threshold = bottompickthreshold * channelmax;
							for (int i = 0; i < channel->samples && channelpick == 0; i++) {
								value = (double)(channel->trace[i]);
								if (value >= threshold)
									channelpick = i;
							}
						}

						/* set sonar altitude */
						channel->sonarAltitude = 0.00075 * channelpick * channel->sampleInterval;
					}
					else if (bottompickmode == MB7K2JSTAR_BOTTOMPICK_BATHYMETRY) {
						channel->sonarAltitude = (int)(750000.0 * ttime_min_use);
						if (channel->sonarAltitude == 0)
							channel->sonarAltitude = 1000 * altitude;
					}
					else {
						channel->sonarAltitude = 1000 * altitude;
					}

					/* reset navigation and other values */
					if (navlon < 180.0)
						navlon = navlon + 360.0;
					if (navlon > 180.0)
						navlon = navlon - 360.0;
					channel->coordX = (int)(360000.0 * navlon);
					channel->coordY = (int)(360000.0 * navlat);
					channel->coordUnits = 2;
					channel->heading = (short)(100.0 * heading);
					channel->startDepth = sonardepth / channel->sampleInterval / 0.00000075;
					channel->sonarDepth = 1000 * sonardepth;
					channel->roll = (short)(32768 * roll / 180.0);
					channel->pitch = (short)(32768 * pitch / 180.0);

					/* write the record */
					nwritesshi++;
					mb_write_ping(verbose, ombio_ptr, ostore_ptr, &error);
				}
			}

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

		status = mb_close(verbose, &imbio_ptr, &error);

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
	}

	/* output counts */
	fprintf(stdout, "\nTotal data records read from: %s\n", file);
	fprintf(stdout, "     Survey:        %d\n", nreaddatatot);
	fprintf(stdout, "     File Header:   %d\n", nreadheadertot);
	fprintf(stdout, "     Bluefin CTD:   %d\n", nreadssvtot);
	fprintf(stdout, "     Bluefin Nav:   %d\n", nreadnav1tot);
	fprintf(stdout, "     Subbottom:     %d\n", nreadsbptot);
	fprintf(stdout, "     Low Sidescan:  %d\n", nreadsslotot);
	fprintf(stdout, "     High Sidescan: %d\n", nreadsshitot);
	fprintf(stdout, "Total data records written to: %s\n", output_file);
	fprintf(stdout, "     Subbottom:     %d\n", nwritesbptot);
	fprintf(stdout, "     Low Sidescan:  %d\n", nwritesslotot);
	fprintf(stdout, "     High Sidescan: %d\n", nwritesshitot);

	/* deallocate route arrays */
	if (route_file_set) {
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routelon, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routelat, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routeheading, &error);
		status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routewaypoint, &error);
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
