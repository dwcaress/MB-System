/*--------------------------------------------------------------------
 *    The MB-system:	mbroutetime.c	5/4/2009
 *
 *    Copyright (c) 2009-2023 by
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
 * mbroutetime outputs a list of the times when a survey hit the waypoints
 * of a planned survey route. This (lon lat time_d) list can then be used by mbextractsegy
 * or mb7k2ss to extract subbottom (or sidescan) data into files corresponding
 * to the lines between waypoints. The input route files are in the MBgrdviz
 * route file format. The times are in decimal epoch seconds (seconds since 1/1/1970).
 *
 * Author:	D. W. Caress
 * Date:	May 5, 2009
 * Location:	R/V Thompson, at the dock in Apia, Samoa
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_status.h"

constexpr int MBES_ALLOC_NUM = 128;
/* constexpr int MBES_ROUTE_WAYPOINT_NONE = 0; */
/* constexpr int MBES_ROUTE_WAYPOINT_SIMPLE = 1; */
constexpr int MBES_ROUTE_WAYPOINT_TRANSIT = 2;
/* constexpr int MBES_ROUTE_WAYPOINT_STARTLINE 3; */
constexpr int  MBES_ROUTE_WAYPOINT_ENDLINE = 4;
/* constexpr double MBES_ONLINE_THRESHOLD = 15.0; */
/* constexpr int MBES_ONLINE_COUNT = 30; */

constexpr char program_name[] = "MBroutetime";
constexpr char help_message[] =
    "MBroutetime outputs a list of the times when a survey hit the waypoints\n"
    "of a planned survey route. This (lon lat time_d) list can then be used by\n"
    "mbextractsegy or mb7k2ss to extract subbottom (or sidescan) data into files\n"
    "corresponding to the lines between waypoints.";
constexpr char usage_message[] =
    "mbroutetime  -Rroutefile [-Fformat -Ifile -Owaypointtimefile -Urangethreshold -H -V]";

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

	char output_file[MB_PATH_MAXLINE + 14] = "";  // Needs extra padding for "_wpttime_d.txt"
	bool output_file_set = false;
	char route_file[MB_PATH_MAXLINE] = "";
	double rangethreshold = 25.0;

	char read_file[MB_PATH_MAXLINE] = "";
	strcpy(read_file, "datalist.mb-1");

	{
		bool errflg = false;
		int c;
		bool help = false;
		/* process argument list */
		while ((c = getopt(argc, argv, "F:f:I:i:O:o:R:r:U:u:VvHh")) != -1)
			switch (c) {
			case 'H':
			case 'h':
				help = true;
				break;
			case 'V':
			case 'v':
				verbose++;
				break;
			case 'F':
			case 'f':
				sscanf(optarg, "%d", &format);
				break;
			case 'I':
			case 'i':
				sscanf(optarg, "%1023s", read_file);
				break;
			case 'O':
			case 'o':
				sscanf(optarg, "%1023s", output_file);
				output_file_set = true;
				break;
			case 'R':
			case 'r':
				sscanf(optarg, "%1023s", route_file);
				break;
			case 'U':
			case 'u':
				sscanf(optarg, "%lf", &rangethreshold);
				break;
			case '?':
				errflg = true;
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
			fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
			fprintf(stderr, "dbg2       help:              %d\n", help);
			fprintf(stderr, "dbg2       format:            %d\n", format);
			fprintf(stderr, "dbg2       pings:             %d\n", pings);
			fprintf(stderr, "dbg2       lonflip:           %d\n", lonflip);
			fprintf(stderr, "dbg2       bounds[0]:         %f\n", bounds[0]);
			fprintf(stderr, "dbg2       bounds[1]:         %f\n", bounds[1]);
			fprintf(stderr, "dbg2       bounds[2]:         %f\n", bounds[2]);
			fprintf(stderr, "dbg2       bounds[3]:         %f\n", bounds[3]);
			fprintf(stderr, "dbg2       btime_i[0]:        %d\n", btime_i[0]);
			fprintf(stderr, "dbg2       btime_i[1]:        %d\n", btime_i[1]);
			fprintf(stderr, "dbg2       btime_i[2]:        %d\n", btime_i[2]);
			fprintf(stderr, "dbg2       btime_i[3]:        %d\n", btime_i[3]);
			fprintf(stderr, "dbg2       btime_i[4]:        %d\n", btime_i[4]);
			fprintf(stderr, "dbg2       btime_i[5]:        %d\n", btime_i[5]);
			fprintf(stderr, "dbg2       btime_i[6]:        %d\n", btime_i[6]);
			fprintf(stderr, "dbg2       etime_i[0]:        %d\n", etime_i[0]);
			fprintf(stderr, "dbg2       etime_i[1]:        %d\n", etime_i[1]);
			fprintf(stderr, "dbg2       etime_i[2]:        %d\n", etime_i[2]);
			fprintf(stderr, "dbg2       etime_i[3]:        %d\n", etime_i[3]);
			fprintf(stderr, "dbg2       etime_i[4]:        %d\n", etime_i[4]);
			fprintf(stderr, "dbg2       etime_i[5]:        %d\n", etime_i[5]);
			fprintf(stderr, "dbg2       etime_i[6]:        %d\n", etime_i[6]);
			fprintf(stderr, "dbg2       speedmin:          %f\n", speedmin);
			fprintf(stderr, "dbg2       timegap:           %f\n", timegap);
			fprintf(stderr, "dbg2       read_file:         %s\n", read_file);
			fprintf(stderr, "dbg2       route_file:        %s\n", route_file);
			fprintf(stderr, "dbg2       output_file_set:   %d\n", output_file_set);
			fprintf(stderr, "dbg2       output_file:       %s\n", output_file);
			fprintf(stderr, "dbg2       rangethreshold:    %f\n", rangethreshold);
		}

		if (help) {
			fprintf(stderr, "\n%s\n", help_message);
			fprintf(stderr, "\nusage: %s\n", usage_message);
			exit(MB_ERROR_NO_ERROR);
		}
	}

	/* read route file */
	FILE *fp = fopen(route_file, "r");
	if (fp == nullptr) {
		fprintf(stderr, "\nUnable to open route file <%s> for reading\n", route_file);
		exit(MB_FAILURE);
	}

	bool rawroutefile = false;
	char comment[MB_COMMENT_MAXLINE] = "";
	double heading;

	int nroutepoint = 0;
	int nroutepointalloc = 0;
	double *routelon = nullptr;
	double *routelat = nullptr;
	double *routeheading = nullptr;
	int *routewaypoint = nullptr;
	double *routetime_d = nullptr;

	int error = MB_ERROR_NO_ERROR;

	char *result;
	while ((result = fgets(comment, MB_PATH_MAXLINE, fp)) == comment) {
		if (comment[0] == '#') {
			if (strncmp(comment, "## Route File Version", 21) == 0) {
				rawroutefile = false;
			}
		}
		else {
			double lon;
			double lat;
			double topo;
			int waypoint;
			const int nget = sscanf(comment, "%lf %lf %lf %d %lf", &lon, &lat, &topo, &waypoint, &heading);
			if (comment[0] == '#') {
				fprintf(stderr, "buffer:%s", comment);
				if (strncmp(comment, "## Route File Version", 21) == 0) {
					rawroutefile = false;
				}
			}

			const bool point_ok =
				(rawroutefile && nget >= 2) ||
				(!rawroutefile && nget >= 3 && waypoint > MBES_ROUTE_WAYPOINT_TRANSIT);

			/* if good data check for need to allocate more space */
			if (point_ok && nroutepoint + 2 > nroutepointalloc) {
				nroutepointalloc += MBES_ALLOC_NUM;
				status = mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double), (void **)&routelon, &error);
				status &= mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double), (void **)&routelat, &error);
				status &=
				    mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double), (void **)&routeheading, &error);
				status &=
				    mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(int), (void **)&routewaypoint, &error);
				status &=
				    mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double), (void **)&routetime_d, &error);
				if (status != MB_SUCCESS) {
					char *message;
					mb_error(verbose, error, &message);
					fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
					fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
					exit(error);
				}
			}

			/* add good point to route */
			if (point_ok && nroutepointalloc > nroutepoint) {
				routelon[nroutepoint] = lon;
				routelat[nroutepoint] = lat;
				routeheading[nroutepoint] = heading;
				routewaypoint[nroutepoint] = waypoint;
				routetime_d[nroutepoint] = 0.0;
				nroutepoint++;
			}
		}
	}

	fclose(fp);
	fp = nullptr;

	/* Check that there are valid waypoints in memory */
	if (nroutepoint < 1) {
		fprintf(stderr, "\nNo line start or line end waypoints read from route file: <%s>\n", route_file);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(MB_ERROR_EOF);
	}
	else if (nroutepoint < 2) {
		fprintf(stderr, "\nOnly one line start or line end waypoint read from route file: <%s>\n", route_file);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(MB_ERROR_EOF);
	}

	/* set starting values */
	int activewaypoint = 0;
	double mtodeglon;
	double mtodeglat;
	mb_coor_scale(verbose, routelat[activewaypoint], &mtodeglon, &mtodeglat);
	double rangelast = 1000 * rangethreshold;

	/* output status */
	if (verbose > 0) {
		/* output info on file output */
		fprintf(stderr, "Read %d waypoints from route file: %s\n", nroutepoint, route_file);
	}

	/* get format if required */
	if (format == 0)
		mb_get_format(verbose, read_file, nullptr, &format, &error);

	/* determine whether to read one file or a list of files */
	const bool read_datalist = format < 0;
	bool read_data;
	void *datalist;
	char file[MB_PATH_MAXLINE] = "";
	char dfile[MB_PATH_MAXLINE] = "";

	/* open file list */
	if (read_datalist) {
		const int look_processed = MB_DATALIST_LOOK_UNSET;
		if (mb_datalist_open(verbose, &datalist, read_file, look_processed, &error) != MB_SUCCESS) {
			fprintf(stderr, "\nUnable to open data list file: %s\n", read_file);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_ERROR_OPEN_FAIL);
		}
		double file_weight;
		read_data = mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error) == MB_SUCCESS;
	} else {
		// else copy single filename to be read
		strcpy(file, read_file);
		read_data = true;
	}

	double btime_d;
	double etime_d;
	int beams_bath;
	int beams_amp;
	int pixels_ss;

	/* MBIO read values */
	void *mbio_ptr = nullptr;
	void *store_ptr = nullptr;
	int kind;
	int time_i[7];
	double time_d;
	double navlon;
	double navlat;
	double speed;
	double distance;
	double altitude;
	double sensordepth;
	char *beamflag = nullptr;
	double *bath = nullptr;
	double *bathacrosstrack = nullptr;
	double *bathalongtrack = nullptr;
	double *amp = nullptr;
	double *ss = nullptr;
	double *ssacrosstrack = nullptr;
	double *ssalongtrack = nullptr;

	int nroutepointfound = 0;

	double lasttime_d = 0.0;
	double lastheading = 0.0;
	double lastlon = 0.0;
	double lastlat = 0.0;
	double range = 0.0;

	/* loop over all files to be read */
	while (read_data) {
		/* read fnv file if possible */
		mb_get_fnv(verbose, file, &format, &error);

		/* initialize reading the swath file */
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
			status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
		if (error == MB_ERROR_NO_ERROR)
			status =
			    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status =
			    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack, &error);
		if (error == MB_ERROR_NO_ERROR)
			status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack, &error);

		/* if error initializing memory then quit */
		if (error != MB_ERROR_NO_ERROR) {
			char *message;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}

		/* read and use data */
		int nread = 0;
		while (error <= MB_ERROR_NO_ERROR && activewaypoint < nroutepoint) {
			/* reset error */
			error = MB_ERROR_NO_ERROR;

			/* read next data record */
			status = mb_get_all(verbose, mbio_ptr, &store_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading,
			                    &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp,
			                    bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);

			/* deal with nav and time from survey data only - not nav, sidescan, or subbottom */
			if (error <= MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
				/* increment counter */
				nread++;

				/* save last nav and heading */
				if (navlon != 0.0)
					lastlon = navlon;
				if (navlat != 0.0)
					lastlat = navlat;
				if (heading != 0.0)
					lastheading = heading;
				if (time_d != 0.0)
					lasttime_d = time_d;

				/* check survey data position against waypoints */
				if (navlon != 0.0 && navlat != 0.0) {
					const double dx = (navlon - routelon[activewaypoint]) / mtodeglon;
					const double dy = (navlat - routelat[activewaypoint]) / mtodeglat;
					range = sqrt(dx * dx + dy * dy);
					if (verbose > 0)
						fprintf(stderr, "> activewaypoint:%d time_d:%f range:%f   lon: %f %f   lat: %f %f\n", activewaypoint,
						        time_d, range, navlon, routelon[activewaypoint], navlat, routelat[activewaypoint]);

					if (range < rangethreshold && (activewaypoint == 0 || range > rangelast) && activewaypoint < nroutepoint) {
						fprintf(stderr, "Waypoint %d of %d found with range %f m\n", activewaypoint, nroutepoint, range);
						routetime_d[activewaypoint] = time_d;
						activewaypoint++;
						nroutepointfound++;
						mb_coor_scale(verbose, routelat[activewaypoint], &mtodeglon, &mtodeglat);
						rangelast = 1000 * rangethreshold;
					}
					else
						rangelast = range;
				}
			}

			if (verbose >= 2) {
				fprintf(stderr, "\ndbg2  Ping read in program <%s>\n", program_name);
				fprintf(stderr, "dbg2       kind:           %d\n", kind);
				fprintf(stderr, "dbg2       error:          %d\n", error);
				fprintf(stderr, "dbg2       status:         %d\n", status);
			}
		}

		/* close the swath file */
		status &= mb_close(verbose, &mbio_ptr, &error);

		/* output read statistics */
		fprintf(stderr, "%d records read from %s\n", nread, file);

		/* figure out whether and what to read next */
		if (read_datalist) {
			double file_weight;
			read_data = mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error) == MB_SUCCESS;
		} else {
			read_data = false;
		}

		/* end loop over files in list */
	}
	if (read_datalist)
		mb_datalist_close(verbose, &datalist, &error);

	/* if the last route point was not reached, add one last waypoint */
	if (nroutepointfound < nroutepoint) {
		fprintf(stderr, "Waypoint %d of %d set at end of data with range %f m to next specified waypoint\n", activewaypoint,
		        nroutepoint, range);
		routelon[nroutepointfound] = lastlon;
		routelat[nroutepointfound] = lastlat;
		routeheading[nroutepointfound] = lastheading;
		routetime_d[nroutepointfound] = lasttime_d;
		routewaypoint[nroutepointfound] = MBES_ROUTE_WAYPOINT_ENDLINE;
		nroutepointfound++;
	}

	/* output time list for the route */
	if (!output_file_set) {
		snprintf(output_file, sizeof(output_file), "%s_wpttime_d.txt", read_file);
	}
	fp = fopen(output_file, "w");
	if (fp == nullptr) {
		fprintf(stderr, "\nUnable to open output waypoint time list file <%s> for writing\n", output_file);
		exit(MB_ERROR_OPEN_FAIL);
	}
	for (int i = 0; i < nroutepointfound; i++) {
		fprintf(fp, "%3d %3d %11.6f %10.6f %10.6f %.6f\n", i, routewaypoint[i], routelon[i], routelat[i], routeheading[i],
		        routetime_d[i]);
		if (verbose > 0)
			fprintf(stderr, "%3d %3d %11.6f %10.6f %10.6f %.6f\n", i, routewaypoint[i], routelon[i], routelat[i], routeheading[i],
			        routetime_d[i]);
	}
	fclose(fp);

	/* deallocate route arrays */
	status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routelon, &error);
	status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routelat, &error);
	status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routeheading, &error);
	status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routewaypoint, &error);
	status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routetime_d, &error);

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
