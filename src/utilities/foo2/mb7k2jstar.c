/*--------------------------------------------------------------------
 *    The MB-system:	mb7k2jstar.c	5/19/2005
 *
 *    Copyright (c) 2005-2019 by
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
 * mb7k2jstar extracts Edgetech subbottom profiler and sidescan data
 * from Reson 7k format data and outputs in the Edgetech Jstar format.
 *
 * Author:	D. W. Caress
 * Date:	May 19, 2005
 *
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_jstar.h"
#include "mbsys_reson7k.h"

#define MB7K2JSTAR_SSLOW 1
#define MB7K2JSTAR_SSHIGH 2
#define MB7K2JSTAR_SBP 3
#define MB7K2JSTAR_ALL 4
#define MB7K2JSTAR_BOTTOMPICK_NONE 0
#define MB7K2JSTAR_BOTTOMPICK_BATHYMETRY 1
#define MB7K2JSTAR_BOTTOMPICK_ALTITUDE 2
#define MB7K2JSTAR_BOTTOMPICK_ARRIVAL 3
#define MB7K2JSTAR_SSGAIN_OFF 0
#define MB7K2JSTAR_SSGAIN_TVG_1OVERR 1
#define MBES_ALLOC_NUM 128
#define MBES_ROUTE_WAYPOINT_NONE 0
#define MBES_ROUTE_WAYPOINT_SIMPLE 1
#define MBES_ROUTE_WAYPOINT_TRANSIT 2
#define MBES_ROUTE_WAYPOINT_STARTLINE 3
#define MBES_ROUTE_WAYPOINT_ENDLINE 4
#define MBES_ONLINE_THRESHOLD 15.0
#define MBES_ONLINE_COUNT 30

static const char help_message[] =
    "mb7k2jstar extracts Edgetech subbottom profiler and sidescan data \nfrom "
    "Reson 7k format data and outputs in the Edgetech Jstar format.";
static const char program_name[] = "mb7k2jstar";
static const char usage_message[] =
    "mb7k2jstar [-Ifile -Atype -Bmode[/threshold] -C -Fformat "
    "-Lstartline/lineroot -Ooutfile -Rroutefile -X -H -V]";

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
	int errflg = 0;
	int help = 0;

	int verbose = 0;
	int format = 0;
	int pings;
	int lonflip;
	double bounds[4];
	int btime_i[7];
	int etime_i[7];
	double speedmin;
	double timegap;

	/* get current default values */
	int status = mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);

	/* set default input to datalist.mb-1 */
	char read_file[MB_PATH_MAXLINE];
	strcpy(read_file, "datalist.mb-1");

	int startline = 1;
	char lineroot[MB_PATH_MAXLINE];
	strcpy(lineroot, "jstar");

	int extract_sslow = MB_NO;
	int extract_sshigh = MB_NO;
	int extract_sbp = MB_NO;
	int print_comments = MB_NO;

	int mode;

	/* bottompick mode */
	int bottompickmode = MB7K2JSTAR_BOTTOMPICK_ALTITUDE;
	double bottompickthreshold = 0.4;

	/* sidescan gain mode */
	int ssflip = MB_NO;
	int gainmode = MB7K2JSTAR_SSGAIN_OFF;
	double gainfactor = 1.0;

	int checkroutebearing = MB_NO;

	char output_file[MB_PATH_MAXLINE+12] = "";
	int output_file_set = MB_NO;

	char route_file[MB_PATH_MAXLINE];
	int route_file_set = MB_NO;

	int smooth = 0;

	double timeshift = 0.0;

	int error = MB_ERROR_NO_ERROR;

	/* process argument list */
	{
	int flag = 0;
	int c;
	while ((c = getopt(argc, argv, "A:a:B:b:CcF:f:G:g:I:i:L:l:MmO:o:R:r:S:s:T:t:XxVvHh")) != -1)
		switch (c) {
		case 'H':
		case 'h':
			help++;
			break;
		case 'V':
		case 'v':
			verbose++;
			break;
		case 'A':
		case 'a':
			if (strncmp(optarg, "SSLOW", 5) == 0 || strncmp(optarg, "sslow", 5) == 0) {
				extract_sslow = MB_YES;
			}
			else if (strncmp(optarg, "SSHIGH", 6) == 0 || strncmp(optarg, "sshigh", 6) == 0) {
				extract_sshigh = MB_YES;
			}
			else if (strncmp(optarg, "SBP", 3) == 0 || strncmp(optarg, "sbp", 3) == 0) {
				extract_sbp = MB_YES;
			}
			else if (strncmp(optarg, "ALL", 3) == 0 || strncmp(optarg, "all", 3) == 0) {
				extract_sshigh = MB_YES;
				extract_sslow = MB_YES;
				extract_sbp = MB_YES;
			}
			else {
				sscanf(optarg, "%d", &mode);
				if (mode == MB7K2JSTAR_SSLOW)
					extract_sslow = MB_YES;
				else if (mode == MB7K2JSTAR_SSHIGH)
					extract_sshigh = MB_YES;
				else if (mode == MB7K2JSTAR_SBP)
					extract_sbp = MB_YES;
				else if (mode == MB7K2JSTAR_ALL) {
					extract_sshigh = MB_YES;
					extract_sslow = MB_YES;
					extract_sbp = MB_YES;
				}
			}
			flag++;
			break;
		case 'B':
		case 'b': {
			const int n = sscanf(optarg, "%d/%lf", &bottompickmode, &bottompickthreshold);
			if (n == 0)
				bottompickmode = MB7K2JSTAR_BOTTOMPICK_ALTITUDE;
			else if (n == 1 && bottompickmode == MB7K2JSTAR_BOTTOMPICK_ARRIVAL)
				bottompickthreshold = 0.5;
			flag++;
			break;
		}
		case 'C':
		case 'c':
			print_comments = MB_YES;
			break;
		case 'F':
		case 'f':
			sscanf(optarg, "%d", &format);
			flag++;
			break;
		case 'G':
		case 'g':
			sscanf(optarg, "%d/%lf", &gainmode, &gainfactor);
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf(optarg, "%s", read_file);
			flag++;
			break;
		case 'L':
		case 'l':
			sscanf(optarg, "%d/%s", &startline, lineroot);
			flag++;
			break;
		case 'M':
		case 'm':
			checkroutebearing = MB_YES;
			flag++;
			break;
		case 'O':
		case 'o':
			sscanf(optarg, "%s", output_file);
			output_file_set = MB_YES;
			flag++;
			break;
		case 'R':
		case 'r':
			sscanf(optarg, "%s", route_file);
			route_file_set = MB_YES;
			flag++;
			break;
		case 'S':
		case 's':
			sscanf(optarg, "%d", &smooth);
			flag++;
			break;
		case 'T':
		case 't':
			sscanf(optarg, "%lf", &timeshift);
			flag++;
			break;
		case 'X':
		case 'x':
			ssflip = MB_YES;
			flag++;
			break;
		case '?':
			errflg++;
		}
	}

	if (errflg) {
		fprintf(stderr, "usage: %s\n", usage_message);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
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
		/* fprintf(stderr, "dbg2       file:                %s\n", file); */
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

	char *message;

	/* MBIO read control parameters */
	int read_datalist = MB_NO;
	char current_output_file[MB_PATH_MAXLINE];
	int new_output_file = MB_YES;
	void *datalist;
	double file_weight;
	double btime_d;
	double etime_d;
	char dfile[MB_PATH_MAXLINE];
	int beams_bath;
	int beams_amp;
	int pixels_ss;

	/* MBIO read values */
	void *imbio_ptr = NULL;
	struct mb_io_struct *imb_io_ptr = NULL;
	void *istore_ptr = NULL;
	struct mbsys_reson7k_struct *istore = NULL;
	void *ombio_ptr = NULL;
	struct mb_io_struct *omb_io_ptr = NULL;
	void *ostore_ptr = NULL;
	struct mbsys_jstar_struct *ostore = NULL;
	int kind;
	int time_i[7];
	int time_j[5];
	double time_d;
	double navlon;
	double navlat;
	double speed;
	double heading;
	double distance;
	double altitude;
	double sonardepth;
	double roll;
	double pitch;
	double heave;
	char *beamflag = NULL;
	double *bath = NULL;
	double *bathacrosstrack = NULL;
	double *bathalongtrack = NULL;
	double *amp = NULL;
	double *ss = NULL;
	double *ssacrosstrack = NULL;
	double *ssalongtrack = NULL;
	double *ttimes = NULL;
	double *angles = NULL;
	double *angles_forward = NULL;
	double *angles_null = NULL;
	double *bheave = NULL;
	double *alongtrack_offset = NULL;
	double draft;
	double ssv;

	char comment[MB_COMMENT_MAXLINE];
	int icomment = 0;

	/* jstar data */
	s7k_fsdwchannel *s7kchannel;       /* Channel header and data */
	s7k_fsdwssheader *s7kssheader;     /* Edgetech sidescan header */
	s7k_fsdwsegyheader *s7ksegyheader = NULL; /* Segy header for subbottom trace */
	struct mbsys_jstar_channel_struct *channel;
	int obeams_bath;
	int obeams_amp;
	int opixels_ss;

	/* route and auto-line data */
	int nroutepoint = 0;
	double lon;
	double lat;
	double topo;
	int waypoint;
	double *routelon = NULL;
	double *routelat = NULL;
	double *routeheading = NULL;
	int *routewaypoint = NULL;
	double range;
	double rangethreshold = 50.0;
	double rangelast;
	int activewaypoint = 0;

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
	int trace_size;
	char *data;
	unsigned short *datashort;
	double value, threshold;
	double channelmax;
	int channelpick;
	double ttime_min;
	double factor;
	double mtodeglon, mtodeglat;
	double headingdiff;
	int oktowrite;
	double dx, dy;
	FILE *fp = NULL;

	int read_data;

	/* set output types if needed */
	if (extract_sbp == MB_NO && extract_sslow == MB_NO && extract_sshigh == MB_NO) {
		extract_sbp = MB_YES;
		extract_sslow = MB_YES;
		extract_sshigh = MB_YES;
	}

	/* output output types */
	fprintf(stdout, "\nData records to extract:\n");
	if (extract_sbp == MB_YES)
		fprintf(stdout, "     Subbottom\n");
	if (extract_sslow == MB_YES)
		fprintf(stdout, "     Low Sidescan\n");
	if (extract_sshigh == MB_YES)
		fprintf(stdout, "     High Sidescan\n");
	if (ssflip == MB_YES)
		fprintf(stdout, "     Sidescan port and starboard exchanged\n");

	/* set starting line number and output file if route read */
	int linenumber = 0;
	if (route_file_set == MB_YES) {
		linenumber = startline;
		sprintf(output_file, "%s_%4.4d.mb132", lineroot, linenumber);
	}

	/* new output file obviously needed */
	new_output_file = MB_YES;

	/* if specified read route file */
	if (route_file_set == MB_YES) {
		/* open the input file */
		if ((fp = fopen(route_file, "r")) == NULL) {
			fprintf(stderr, "\nUnable to open route file <%s> for reading\n", route_file);
			exit(MB_FAILURE);
		}
		int rawroutefile = MB_NO;
		int nroutepointalloc = 0;
		char *result;
		while ((result = fgets(comment, MB_PATH_MAXLINE, fp)) == comment) {
			if (comment[0] == '#') {
				if (strncmp(comment, "## Route File Version", 21) == 0) {
					rawroutefile = MB_NO;
				}
			}
			else {
				const int nget = sscanf(comment, "%lf %lf %lf %d %lf", &lon, &lat, &topo, &waypoint, &heading);
				if (comment[0] == '#') {
					fprintf(stderr, "buffer:%s", comment);
					if (strncmp(comment, "## Route File Version", 21) == 0) {
						rawroutefile = MB_NO;
					}
				}
				int point_ok;
				if ((rawroutefile == MB_YES && nget >= 2) ||
				    (rawroutefile == MB_NO && nget >= 3 && waypoint > MBES_ROUTE_WAYPOINT_NONE))
					point_ok = MB_YES;
				else
					point_ok = MB_NO;

				/* if good data check for need to allocate more space */
				if (point_ok == MB_YES && nroutepoint + 1 > nroutepointalloc) {
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
						mb_error(verbose, error, &message);
						fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
						fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
						exit(error);
					}
				}

				/* add good point to route */
				if (point_ok == MB_YES && nroutepointalloc > nroutepoint + 1) {
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
		fp = NULL;

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
		mb_get_format(verbose, read_file, NULL, &format, &error);

	/* determine whether to read one file or a list of files */
	if (format < 0)
		read_datalist = MB_YES;

	/* open file list */
	char file[MB_PATH_MAXLINE] = "";
	if (read_datalist == MB_YES) {
		int look_processed = MB_DATALIST_LOOK_YES;
		if ((status = mb_datalist_open(verbose, &datalist, read_file, look_processed, &error)) != MB_SUCCESS) {
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr, "\nUnable to open data list file: %s\n", read_file);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}
		if ((status = mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error)) == MB_SUCCESS)
			read_data = MB_YES;
		else
			read_data = MB_NO;
	}
	/* else copy single filename to be read */
	else {
		strcpy(file, read_file);
		read_data = MB_YES;
	}

	/* loop over all files to be read */
	while (read_data == MB_YES && format == MBF_RESON7KR) {

		/* initialize reading the swath file */
		if ((status = mb_read_init(verbose, file, format, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, &imbio_ptr,
		                           &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error)) != MB_SUCCESS) {
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
			fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", file);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}

		/* get pointers to data storage */
		imb_io_ptr = (struct mb_io_struct *)imbio_ptr;
		istore_ptr = imb_io_ptr->store_data;
		istore = (struct mbsys_reson7k_struct *)istore_ptr;

		if (error == MB_ERROR_NO_ERROR) {
			beamflag = NULL;
			bath = NULL;
			amp = NULL;
			bathacrosstrack = NULL;
			bathalongtrack = NULL;
			ss = NULL;
			ssacrosstrack = NULL;
			ssalongtrack = NULL;
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
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}

		/* set up output file name if needed */
		if (error == MB_ERROR_NO_ERROR) {
			if (output_file_set == MB_YES && ombio_ptr == NULL) {
				/* set flag to open new output file */
				new_output_file = MB_YES;
			}

			else if (output_file_set == MB_NO && route_file_set == MB_NO) {
				new_output_file = MB_YES;
				format_status = mb_get_format(verbose, file, output_file, &format_guess, &error);
				if (format_status != MB_SUCCESS || format_guess != format) {
					strcpy(output_file, file);
				}
				if (output_file[strlen(output_file) - 1] == 'p') {
					output_file[strlen(output_file) - 1] = '\0';
				}
				if (extract_sbp == MB_YES && extract_sslow == MB_YES && extract_sshigh == MB_YES) {
					strcat(output_file, ".jsf");
					format_output = MBF_EDGJSTAR;
				}
				else if (extract_sslow == MB_YES) {
					strcat(output_file, ".mb132");
					format_output = MBF_EDGJSTAR;
				}
				else if (extract_sshigh == MB_YES) {
					strcat(output_file, ".mb133");
					format_output = MBF_EDGJSTR2;
				}
				else if (extract_sbp == MB_YES) {
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
			status &= mb_get_all(verbose, imbio_ptr, &istore_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading,
			                    &distance, &altitude, &sonardepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp,
			                    bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);
			/*fprintf(stderr,"kind:%d %s \n\ttime_i:%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d  %f    time_i:%4.4d/%2.2d/%2.2d
			%2.2d:%2.2d:%2.2d.%6.6d  %f\n",
			kind,notice_msg[kind],time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6],time_d,
			istore->time_i[0],istore->time_i[1],istore->time_i[2],istore->time_i[3],istore->time_i[4],istore->time_i[5],istore->time_i[6],istore->time_d);*/

			/* reset nonfatal errors */
			if (kind == MB_DATA_DATA && error < 0) {
				status = MB_SUCCESS;
				error = MB_ERROR_NO_ERROR;
			}

			/* check survey data position against waypoints */
			/* fprintf(stderr,"status:%d error:%d kind:%d route_file_set:%d nroutepoint:%d navlon:%f navlat:%f\n",
			status,error,kind,route_file_set,nroutepoint,navlon,navlat); */
			if (status == MB_SUCCESS && kind == MB_DATA_DATA && route_file_set == MB_YES && nroutepoint > 1 && navlon != 0.0 &&
			    navlat != 0.0) {
				dx = (navlon - routelon[activewaypoint]) / mtodeglon;
				dy = (navlat - routelat[activewaypoint]) / mtodeglat;
				range = sqrt(dx * dx + dy * dy);
				/* fprintf(stderr,"activewaypoint:%d range:%f lon:%f %f lat:%f %f dx:%f dy:%f\n",
				activewaypoint, range, navlon, routelon[activewaypoint], navlat, routelat[activewaypoint], dx, dy);*/
				if (range < rangethreshold && (activewaypoint == 0 || range > rangelast) && activewaypoint < nroutepoint - 1) {
					/* fprintf(stderr,"New output by range to routepoint: %f\n",range); */
					/* if needed set flag to open new output file */
					if (new_output_file == MB_NO) {
						/* increment line number */
						linenumber++;

						/* set output file name */
						sprintf(output_file, "%s_%4.4d", lineroot, linenumber);
						if (extract_sbp == MB_YES && extract_sslow == MB_YES && extract_sshigh == MB_YES) {
							strcat(output_file, ".jsf");
							format_output = MBF_EDGJSTAR;
						}
						else if (extract_sslow == MB_YES) {
							strcat(output_file, ".mb132");
							format_output = MBF_EDGJSTAR;
						}
						else if (extract_sshigh == MB_YES) {
							strcat(output_file, ".mb133");
							format_output = MBF_EDGJSTR2;
						}
						else if (extract_sbp == MB_YES) {
							strcat(output_file, ".jsf");
							format_output = MBF_EDGJSTAR;
						}

						/* set to open new output file */
						new_output_file = MB_YES;
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
				int found = MB_NO;
				for (int i = 0; i < beams_bath; i++) {
					if (mb_beam_ok(beamflag[i])) {
						if (found == MB_NO || ttimes[i] < ttime_min) {
							ttime_min = ttimes[i];
							/* beam_min = i; */
							found = MB_YES;
						}
					}
				}
				if (found == MB_YES) {
					ttime_min_use = ttime_min;
				}
				/*fprintf(stderr,"found:%d beam_min:%d ttime_min_use:%f\n", found, beam_min, ttime_min_use);*/
			}

			/* nonfatal errors do not matter */
			if (error < MB_ERROR_NO_ERROR) {
				error = MB_ERROR_NO_ERROR;
				status = MB_SUCCESS;
			}

			/* if needed open new output file */
			if (status == MB_SUCCESS && new_output_file == MB_YES &&
			    ((extract_sbp == MB_YES && kind == MB_DATA_SUBBOTTOM_SUBBOTTOM) ||
			     (extract_sslow == MB_YES && kind == MB_DATA_SIDESCAN2) ||
			     (extract_sshigh == MB_YES && kind == MB_DATA_SIDESCAN3))) {

				/* close any old output file unless a single file has been specified */
				if (ombio_ptr != NULL) {
					/* close the swath file */
					status = mb_close(verbose, &ombio_ptr, &error);

					/* generate inf file */
					if (status == MB_SUCCESS) {
						status = mb_make_info(verbose, MB_YES, current_output_file, format_output, &error);
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
				new_output_file = MB_NO;
			}

			/* apply time shift if needed */
			if (status == MB_SUCCESS && timeshift != 0.0 &&
			    (kind == MB_DATA_SUBBOTTOM_SUBBOTTOM || kind == MB_DATA_SIDESCAN2 || kind == MB_DATA_SIDESCAN3)) {
				time_d += timeshift;
				mb_get_date(verbose, time_d, time_i);
				mb_get_jtime(verbose, time_i, time_j);
				/*fprintf(stderr,"Applying time shift: %f  %f\n",timeshift, time_d);*/
			}

			/* get some more values */
			if (status == MB_SUCCESS && (kind == MB_DATA_SUBBOTTOM_SUBBOTTOM || kind == MB_DATA_DATA ||
			                             kind == MB_DATA_SIDESCAN2 || kind == MB_DATA_SIDESCAN3)) {
				/*for (i=MAX(0,imb_io_ptr->nfix-5);i<imb_io_ptr->nfix;i++)
				fprintf(stderr,"dbg2       nav fix[%2d]:   %f %f %f\n",
				i, imb_io_ptr->fix_time_d[i],
				imb_io_ptr->fix_lon[i],
				imb_io_ptr->fix_lat[i]);*/
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
			if (checkroutebearing == MB_YES && nroutepoint > 1 && activewaypoint > 0) {
				headingdiff = fabs(routeheading[activewaypoint - 1] - heading);
				if (headingdiff > 180.0)
					headingdiff = 360.0 - headingdiff;
				if (headingdiff < MBES_ONLINE_THRESHOLD)
					oktowrite++;
				else
					oktowrite = 0;
				/* fprintf(stderr,"heading: %f %f %f oktowrite:%d\n",
				routeheading[activewaypoint-1],heading,headingdiff,oktowrite);*/
			}
			else
				oktowrite = MBES_ONLINE_COUNT;
			/* if (status == MB_SUCCESS)
			fprintf(stderr,"activewaypoint:%d linenumber:%d range:%f   lon: %f %f   lat: %f %f oktowrite:%d\n",
			activewaypoint,linenumber,range, navlon,
			routelon[activewaypoint], navlat, routelat[activewaypoint], oktowrite);*/

			/* handle multibeam data */
			if (status == MB_SUCCESS && kind == MB_DATA_DATA) {
				/*fprintf(stderr,"MB_DATA_DATA: status:%d error:%d kind:%d\n",status,error,kind);*/
				nreaddata++;
			}

			/* handle file header data */
			else if (status == MB_SUCCESS && kind == MB_DATA_HEADER) {
				/*fprintf(stderr,"MB_DATA_HEADER: status:%d error:%d kind:%d\n",status,error,kind);*/
				nreadheader++;
			}

			/* handle bluefin ctd data */
			else if (status == MB_SUCCESS && kind == MB_DATA_SSV) {
				/*fprintf(stderr,"MB_DATA_SSV: status:%d error:%d kind:%d\n",status,error,kind);*/
				nreadssv++;
			}

			/* handle bluefin nav data */
			else if (status == MB_SUCCESS && kind == MB_DATA_NAV2) {
				/*fprintf(stderr,"MB_DATA_NAV1: status:%d error:%d kind:%d\n",status,error,kind);*/
				nreadnav1++;
			}

			/* handle subbottom data */
			else if (status == MB_SUCCESS && kind == MB_DATA_SUBBOTTOM_SUBBOTTOM) {
				/*fprintf(stderr,"MB_DATA_SUBBOTTOM_SUBBOTTOM: status:%d error:%d kind:%d\n",status,error,kind);*/
				nreadsbp++;

				/* output data if desired */
				if (extract_sbp == MB_YES && nreadnav1 > 0 && oktowrite >= MBES_ONLINE_COUNT) {
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
						data = (char *)channel->trace;
						for (int i = 0; i < trace_size; i++)
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
								channelmax = MAX(value, channelmax);
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
								channelmax = MAX(value, channelmax);
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
				/*fprintf(stderr,"MB_DATA_SIDESCAN2: status:%d error:%d kind:%d\n",status,error,kind);*/
				nreadsslo++;

				/* output data if desired */
				if (extract_sslow == MB_YES && nreadnav1 > 0 && oktowrite >= MBES_ONLINE_COUNT) {
					/* set overall parameters */
					ostore->kind = MB_DATA_DATA;
					ostore->subsystem = 20;

					/*----------------------------------------------------------------*/
					/* copy low frequency port sidescan to jstar storage */
					if (ssflip == MB_YES)
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
					if (ssflip == MB_YES)
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
								for (int j = MAX(i - smooth, 0); j < MIN(i + smooth, channel->samples - 1); j++) {
									channel->trace[i] += datashort[j];
									n++;
									/*fprintf(stderr,"i:%d j:%d raw:%d tot:%d\n",i,j,datashort[j],channel->trace[i]);*/
								}
								channel->trace[i] /= n;
								/*fprintf(stderr,"Final data[%d] n:%d : %d\n", i, n, channel->trace[i]);*/
							}
						}
						else if (smooth < 0 && channel->dataFormat == 0) {
							datashort = (unsigned short *)s7kchannel->data;
							for (int i = 0; i < channel->samples; i++) {
								int n = 0;
								value = 0.0;
								for (int j = MAX(i + smooth, 0); j < MIN(i - smooth, channel->samples - 1); j++) {
									value += datashort[j] * datashort[j];
									n++;
									/*fprintf(stderr,"i:%d j:%d raw:%d tot:%d\n",i,j,datashort[j],channel->trace[i]);*/
								}
								channel->trace[i] = (unsigned int)(sqrt(value) / n);
								/*fprintf(stderr,"Final data[%d] n:%d : %d\n", i, n, channel->trace[i]);*/
							}
						}
						else {
							data = (char *)channel->trace;
							for (int i = 0; i < trace_size; i++) {
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
								channelmax = MAX(value, channelmax);
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
								channelmax = MAX(value, channelmax);
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
						channelpick = MAX(channelpick, 1);
						/*fprintf(stderr,"altitude:%d sampleInterval:%d
						 * channelpick:%d\n",channel->sonarAltitude,channel->sampleInterval,channelpick);*/
						for (int i = 0; i < channelpick; i++) {
							channel->trace[i] = (unsigned short)(gainfactor * channel->trace[i]);
						}
						for (int i = channelpick; i < channel->samples; i++) {
							factor = gainfactor * (((double)(i * i)) / ((double)(channelpick * channelpick)));
							/*fprintf(stderr,"sample %d: factor:%f value: %d",i,factor,channel->trace[i]);*/
							channel->trace[i] = (unsigned short)(factor * channel->trace[i]);
							/*fprintf(stderr," %d\n",channel->trace[i]);*/
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
					if (ssflip == MB_YES)
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
					if (ssflip == MB_YES)
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
								for (int j = MAX(i - smooth, 0); j < MIN(i + smooth, channel->samples - 1); j++) {
									channel->trace[i] += datashort[j];
									n++;
									/*fprintf(stderr,"i:%d j:%d raw:%d tot:%d\n",i,j,datashort[j],channel->trace[i]);*/
								}
								channel->trace[i] /= n;
								/*fprintf(stderr,"Final data[%d] n:%d : %d\n", i, n, channel->trace[i]);*/
							}
						}
						else if (smooth < 0 && channel->dataFormat == 0) {
							datashort = (unsigned short *)s7kchannel->data;
							for (int i = 0; i < channel->samples; i++) {
								int n = 0;
								value = 0.0;
								for (int j = MAX(i + smooth, 0); j < MIN(i - smooth, channel->samples - 1); j++) {
									value += datashort[j] * datashort[j];
									n++;
									/*fprintf(stderr,"i:%d j:%d raw:%d tot:%d\n",i,j,datashort[j],channel->trace[i]);*/
								}
								channel->trace[i] = (unsigned int)(sqrt(value) / n);
								/*fprintf(stderr,"Final data[%d] n:%d : %d\n", i, n, channel->trace[i]);*/
							}
						}
						else {
							data = (char *)channel->trace;
							for (int i = 0; i < trace_size; i++) {
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
								channelmax = MAX(value, channelmax);
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
								channelmax = MAX(value, channelmax);
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
						/*fprintf(stderr,"altitude:%d sampleInterval:%d
						 * channelpick:%d\n",channel->sonarAltitude,channel->sampleInterval,channelpick);*/
						channelpick = MAX(channelpick, 1);
						for (int i = channelpick; i < channel->samples; i++) {
							factor = gainfactor * (((double)(i * i)) / ((double)(channelpick * channelpick)));
							/*fprintf(stderr,"sample %d: factor:%f value: %d",i,factor,channel->trace[i]);*/
							channel->trace[i] = (unsigned short)(factor * channel->trace[i]);
							/*fprintf(stderr," %d\n",channel->trace[i]);*/
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
				/*fprintf(stderr,"MB_DATA_SIDESCAN3: status:%d error:%d kind:%d\n",status,error,kind);*/
				nreadsshi++;

				/* output data if desired */
				if (extract_sshigh == MB_YES && nreadnav1 > 0 && oktowrite >= MBES_ONLINE_COUNT) {
					/* set overall parameters */
					ostore->kind = MB_DATA_SIDESCAN2;
					ostore->subsystem = 21;

					/*----------------------------------------------------------------*/
					/* copy high frequency port sidescan to jstar storage */
					if (ssflip == MB_YES)
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
								for (int j = MAX(i - smooth, 0); j < MIN(i + smooth, channel->samples - 1); j++) {
									channel->trace[i] += datashort[j];
									n++;
									/*fprintf(stderr,"i:%d j:%d raw:%d tot:%d\n",i,j,datashort[j],channel->trace[i]);*/
								}
								channel->trace[i] /= n;
								/*fprintf(stderr,"Final data[%d] n:%d : %d\n", i, n, channel->trace[i]);*/
							}
						}
						else if (smooth < 0 && channel->dataFormat == 0) {
							datashort = (unsigned short *)s7kchannel->data;
							for (int i = 0; i < channel->samples; i++) {
								int n = 0;
								value = 0.0;
								for (int j = MAX(i + smooth, 0); j < MIN(i - smooth, channel->samples - 1); j++) {
									value += datashort[j] * datashort[j];
									n++;
									/*fprintf(stderr,"i:%d j:%d raw:%d tot:%d\n",i,j,datashort[j],channel->trace[i]);*/
								}
								channel->trace[i] = (unsigned int)(sqrt(value) / n);
								/*fprintf(stderr,"Final data[%d] n:%d : %d\n", i, n, channel->trace[i]);*/
							}
						}
						else {
							data = (char *)channel->trace;
							for (int i = 0; i < trace_size; i++) {
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
								channelmax = MAX(value, channelmax);
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
								channelmax = MAX(value, channelmax);
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
					if (ssflip == MB_YES)
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
								for (int j = MAX(i - smooth, 0); j < MIN(i + smooth, channel->samples - 1); j++) {
									channel->trace[i] += datashort[j];
									n++;
									/*fprintf(stderr,"i:%d j:%d raw:%d tot:%d\n",i,j,datashort[j],channel->trace[i]);*/
								}
								channel->trace[i] /= n;
								/*fprintf(stderr,"Final data[%d] n:%d : %d\n", i, n, channel->trace[i]);*/
							}
						}
						else if (smooth < 0 && channel->dataFormat == 0) {
							datashort = (unsigned short *)s7kchannel->data;
							for (int i = 0; i < channel->samples; i++) {
								int n = 0;
								value = 0.0;
								for (int j = MAX(i + smooth, 0); j < MIN(i - smooth, channel->samples - 1); j++) {
									value += datashort[j] * datashort[j];
									n++;
									/*fprintf(stderr,"i:%d j:%d raw:%d tot:%d\n",i,j,datashort[j],channel->trace[i]);*/
								}
								channel->trace[i] = (unsigned int)(sqrt(value) / n);
								/*fprintf(stderr,"Final data[%d] n:%d : %d\n", i, n, channel->trace[i]);*/
							}
						}
						else {
							data = (char *)channel->trace;
							for (int i = 0; i < trace_size; i++) {
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
								channelmax = MAX(value, channelmax);
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
								channelmax = MAX(value, channelmax);
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

			/* handle unknown data */
			else if (status == MB_SUCCESS) {
				/*fprintf(stderr,"DATA TYPE UNKNOWN: status:%d error:%d kind:%d\n",status,error,kind);*/
			}

			/* handle read error */
			else {
				/*fprintf(stderr,"READ FAILURE: status:%d error:%d kind:%d\n",status,error,kind);*/
			}

			if (verbose >= 2) {
				fprintf(stderr, "\ndbg2  Ping read in program <%s>\n", program_name);
				fprintf(stderr, "dbg2       kind:           %d\n", kind);
				fprintf(stderr, "dbg2       error:          %d\n", error);
				fprintf(stderr, "dbg2       status:         %d\n", status);
			}

			if (print_comments == MB_YES && kind == MB_DATA_COMMENT) {
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
		if (read_datalist == MB_YES) {
			if ((status = mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error)) == MB_SUCCESS)
				read_data = MB_YES;
			else
				read_data = MB_NO;
		}
		else {
			read_data = MB_NO;
		}

		/* end loop over files in list */
	}
	if (read_datalist == MB_YES)
		mb_datalist_close(verbose, &datalist, &error);

	/* close output file if still open */
	if (ombio_ptr != NULL) {
		/* close the swath file */
		status = mb_close(verbose, &ombio_ptr, &error);

		/* generate inf file */
		if (status == MB_SUCCESS) {
			status = mb_make_info(verbose, MB_YES, output_file, format_output, &error);
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
	if (route_file_set == MB_YES) {
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
