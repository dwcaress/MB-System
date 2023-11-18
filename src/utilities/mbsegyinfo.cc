/*--------------------------------------------------------------------
 *    The MB-system:	mbsegyinfo.c	6/2/2004
 *
 *    Copyright (c) 2004-2023 by
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
 * MBsegyinfo reads a segy data file and outputs some basic statistics.
 *
 * Author:	D. W. Caress
 * Date:	June 2, 2004
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <getopt.h>
#include <unistd.h>

#include <algorithm>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_segy.h"
#include "mb_status.h"

constexpr char program_name[] = "MBsegyinfo";
constexpr char help_message[] =
    "MBsegyinfo lists table data from a segy data file.";
constexpr char usage_message[] =
    "MBsegyinfo -Ifile [-Llonflip -O -H -V]";

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

	char read_file[MB_PATH_MAXLINE] = "";
	bool output_usefile = false;

	/* process argument list */
	{
		bool errflg = false;
		bool help = false;
		int c;
		while ((c = getopt(argc, argv, "I:i:L:l:OoVvWwHh")) != -1)
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
			case 'I':
			case 'i':
				sscanf(optarg, "%1023s", read_file);
				break;
			case 'L':
			case 'l':
				sscanf(optarg, "%d", &lonflip);
				break;
			case 'O':
			case 'o':
				output_usefile = true;
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
			fprintf(stderr, "dbg2       lonflip:        %d\n", lonflip);
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
			fprintf(stderr, "dbg2       read_file:      %s\n", read_file);
		}

		if (help) {
			fprintf(stderr, "\n%s\n", help_message);
			fprintf(stderr, "\nusage: %s\n", usage_message);
			exit(MB_ERROR_NO_ERROR);
		}
	}

	int error = MB_ERROR_NO_ERROR;

	/* initialize reading the segy file */
	void *mbsegyioptr;
	struct mb_segyfileheader_struct fileheader;
	{
		struct mb_segyasciiheader_struct asciiheader;
		if (mb_segy_read_init(verbose, read_file, &mbsegyioptr, &asciiheader, &fileheader, &error) != MB_SUCCESS) {
			char *message;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error returned from function <mb_segy_read_init>:\n%s\n", message);
			fprintf(stderr, "\nSEGY File <%s> not initialized for reading\n", read_file);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}
	}

	FILE *stream = verbose <= 1 ? stdout : stderr;

	FILE *output = nullptr;
	if (output_usefile) {
		char output_file[MB_PATH_MAXLINE];
		strcpy(output_file, read_file);
		strcat(output_file, ".sinf");
		if ((output = fopen(output_file, "w")) == nullptr)
			output = stream;
	}
	else {
		output = stream;
	}

	double delaymin = 0.0;
	double delaymax = 0.0;

	double lonmin = 0.0;
	double lonmax = 0.0;
	double latmin = 0.0;
	double latmax = 0.0;
	double lonbeg = 0.0;
	double latbeg = 0.0;
	double lonend = 0.0;
	double latend = 0.0;

	double rangemin = 0.0;
	double rangemax = 0.0;

	double receiverelevationmin = 0.0;
	double receiverelevationmax = 0.0;
	double receiverwaterdepthmin = 0.0;
	double receiverwaterdepthmax = 0.0;

	int rpmin = 0;
	int rpmax = 0;
	int rptracemin = 0;
	int rptracemax = 0;

	int shotmin = 0;
	int shotmax = 0;
	int shottracemin = 0;
	int shottracemax = 0;

	double sourcedepthmin = 0.0;
	double sourcedepthmax = 0.0;
	double sourceelevationmin = 0.0;
	double sourceelevationmax = 0.0;
	double sourcewaterdepthmin = 0.0;
	double sourcewaterdepthmax = 0.0;

	int timbeg_i[7];
	int timend_i[7];
	int timbeg_j[5];
	int timend_j[5];

	int nread = 0;
	bool first = true;
	while (error <= MB_ERROR_NO_ERROR) {
		error = MB_ERROR_NO_ERROR;

		/* read a trace */
		struct mb_segytraceheader_struct traceheader;
		float *trace;
		status = mb_segy_read_trace(verbose, mbsegyioptr, &traceheader, &trace, &error);

		/* deal with success */
		if (status == MB_SUCCESS) {
			nread++;

			/* get needed values */
			int time_j[5];
			time_j[0] = traceheader.year;
			time_j[1] = traceheader.day_of_yr;
			time_j[2] = traceheader.min + 60 * traceheader.hour;
			time_j[3] = traceheader.sec;
			time_j[4] = 1000 * traceheader.mils;
			int time_i[7];
			mb_get_itime(verbose, time_j, time_i);
			double time_d;
			mb_get_time(verbose, time_i, &time_d);
			double factor;
			if (traceheader.coord_scalar < 0)
				factor = 1.0 / ((float)(-traceheader.coord_scalar)) / 3600.0;
			else
				factor = (float)traceheader.coord_scalar / 3600.0;
			double navlon;
			if (traceheader.src_long != 0)
				navlon = factor * ((float)traceheader.src_long);
			else
				navlon = factor * ((float)traceheader.grp_long);
			double navlat;
			if (traceheader.src_lat != 0)
				navlat = factor * ((float)traceheader.src_lat);
			else
				navlat = factor * ((float)traceheader.grp_lat);
			if (lonflip < 0) {
				if (navlon > 0.)
					navlon = navlon - 360.;
				else if (navlon < -360.)
					navlon = navlon + 360.;
			}
			else if (lonflip == 0) {
				if (navlon > 180.)
					navlon = navlon - 360.;
				else if (navlon < -180.)
					navlon = navlon + 360.;
			}
			else {
				if (navlon > 360.)
					navlon = navlon - 360.;
				else if (navlon < 0.)
					navlon = navlon + 360.;
			}
			if (traceheader.elev_scalar < 0)
				factor = 1.0 / ((float)(-traceheader.elev_scalar));
			else
				factor = (float)traceheader.elev_scalar;
			const double range = (double)traceheader.range;
			const double receiverelevation = factor * ((double)traceheader.grp_elev);
			const double sourceelevation = factor * ((double)traceheader.src_elev);
			const double sourcedepth = factor * ((double)traceheader.src_depth);
			const double sourcewaterdepth = factor * ((double)traceheader.src_wbd);
			const double receiverwaterdepth = factor * ((double)traceheader.grp_wbd);
			const double delay = 0.001 * ((double)traceheader.delay_mils);

			/* get initial values */
			if (first) {
				first = false;
				shotmin = traceheader.shot_num;
				shotmax = traceheader.shot_num;
				shottracemin = traceheader.shot_tr;
				shottracemax = traceheader.shot_tr;
				rpmin = traceheader.rp_num;
				rpmax = traceheader.rp_num;
				rptracemin = traceheader.rp_tr;
				rptracemax = traceheader.rp_tr;
				delaymin = delay;
				delaymax = delay;
				lonmin = navlon;
				lonmax = navlon;
				latmin = navlat;
				latmax = navlat;
				rangemin = range;
				rangemax = range;
				receiverelevationmin = receiverelevation;
				receiverelevationmax = receiverelevation;
				sourceelevationmin = sourceelevation;
				sourceelevationmax = sourceelevation;
				sourcedepthmin = sourcedepth;
				sourcedepthmax = sourcedepth;
				sourcewaterdepthmin = sourcewaterdepth;
				sourcewaterdepthmax = sourcewaterdepth;
				receiverwaterdepthmin = receiverwaterdepth;
				receiverwaterdepthmax = receiverwaterdepth;

				lonbeg = navlon;
				latbeg = navlat;
				lonend = navlon;
				latend = navlat;
				for (int i = 0; i < 7; i++) {
					timbeg_i[i] = time_i[i];
					timend_i[i] = time_i[i];
				}
				for (int i = 0; i < 5; i++) {
					timbeg_j[i] = time_j[i];
					timend_j[i] = time_j[i];
				}
			}

			/* get min max values */
			else {
				shotmin = std::min(shotmin, traceheader.shot_num);
				shotmax = std::max(shotmax, traceheader.shot_num);
				shotmin = std::min(shotmin, traceheader.shot_num);
				shotmax = std::max(shotmax, traceheader.shot_num);
				rpmin = std::min(rpmin, traceheader.rp_num);
				rpmax = std::max(rpmax, traceheader.rp_num);
				rptracemin = std::min(rptracemin, traceheader.rp_tr);
				rptracemax = std::max(rptracemax, traceheader.rp_tr);
				delaymin = std::min(delaymin, delay);
				delaymax = std::max(delaymax, delay);
				if (navlon != 0.0 && navlat != 0.0) {
					lonmin = std::min(lonmin, navlon);
					lonmax = std::max(lonmax, navlon);
					latmin = std::min(latmin, navlat);
					latmax = std::max(latmax, navlat);
				}
				lonend = navlon;
				latend = navlat;
				for (int i = 0; i < 7; i++) {
					timend_i[i] = time_i[i];
				}
				for (int i = 0; i < 5; i++) {
					timend_j[i] = time_j[i];
				}
				rangemin = std::min(rangemin, range);
				rangemax = std::max(rangemax, range);
				receiverelevationmin = std::min(receiverelevationmin, receiverelevation);
				receiverelevationmax = std::max(receiverelevationmax, receiverelevation);
				sourceelevationmin = std::min(sourceelevationmin, sourceelevation);
				sourceelevationmax = std::max(sourceelevationmax, sourceelevation);
				sourcedepthmin = std::min(sourcedepthmin, sourcedepth);
				sourcedepthmax = std::max(sourcedepthmax, sourcedepth);
				sourcewaterdepthmin = std::min(sourcewaterdepthmin, sourcewaterdepth);
				sourcewaterdepthmax = std::max(sourcewaterdepthmax, sourcewaterdepth);
				receiverwaterdepthmin = std::min(receiverwaterdepthmin, receiverwaterdepth);
				receiverwaterdepthmax = std::max(receiverwaterdepthmax, receiverwaterdepth);
			}
		}

		if (error == MB_ERROR_NO_ERROR && first) {
			first = false;
		}
	}

	status = mb_segy_close(verbose, &mbsegyioptr, &error);

	const double tracelength = 0.000001 * (double)(fileheader.sample_interval * fileheader.number_samples);
	fprintf(output, "\nSEGY Data File:      %s\n", read_file);
	fprintf(output, "\nFile Header Info:\n");
	fprintf(output, "  Channels:                   %8d\n", fileheader.channels);
	fprintf(output, "  Auxiliary Channels:         %8d\n", fileheader.aux_channels);
	fprintf(output, "  Sample Interval (usec):     %8d\n", fileheader.sample_interval);
	fprintf(output, "  Number of Samples in Trace: %8d\n", fileheader.number_samples);
	fprintf(output, "  Trace length (sec):         %8f\n", tracelength);
	if (fileheader.format == 1)
		fprintf(output, "  Data Format:                IBM 32 bit floating point\n");
	else if (fileheader.format == 2)
		fprintf(output, "  Data Format:                32 bit integer\n");
	else if (fileheader.format == 3)
		fprintf(output, "  Data Format:                16 bit integer\n");
	else if (fileheader.format == 5)
		fprintf(output, "  Data Format:                IEEE 32 bit integer\n");
	else if (fileheader.format == 6)
		fprintf(output, "  Data Format:                IEEE 32 bit integer\n");
	else if (fileheader.format == 8)
		fprintf(output, "  Data Format:                8 bit integer\n");
	else if (fileheader.format == 11)
		fprintf(output, "  Data Format:                Little-endian IEEE 32 bit floating point\n");
	else
		fprintf(output, "  Data Format:                Unknown\n");
	fprintf(output, "  CDP Fold:                   %8d\n", fileheader.cdp_fold);
	fprintf(output, "\nData Totals:\n");
	fprintf(output, "  Number of Traces:           %8d\n", nread);
	fprintf(output, "  Min Max Delta:\n");
	fprintf(output, "    Shot number:              %8d %8d %8d\n", shotmin, shotmax, shotmax - shotmin + 1);
	fprintf(output, "    Shot trace:               %8d %8d %8d\n", shottracemin, shottracemax, shottracemax - shottracemin + 1);
	fprintf(output, "    RP number:                %8d %8d %8d\n", rpmin, rpmax, rpmax - rpmin + 1);
	fprintf(output, "    RP trace:                 %8d %8d %8d\n", rptracemin, rptracemax, rptracemax - rptracemin + 1);
	fprintf(output, "    Delay (sec):              %8f %8f %8f\n", delaymin, delaymax, delaymax - delaymin);
	fprintf(output, "    Range (m):                %8f %8f %8f\n", rangemin, rangemax, rangemax - rangemin);
	fprintf(output, "    Receiver Elevation (m):   %8f %8f %8f\n", receiverelevationmin, receiverelevationmax,
	        receiverelevationmax - receiverelevationmin);
	fprintf(output, "    Source Elevation (m):     %8f %8f %8f\n", sourceelevationmin, sourceelevationmax,
	        sourceelevationmax - sourceelevationmin);
	fprintf(output, "    Source Depth (m):         %8f %8f %8f\n", sourcedepthmin, sourcedepthmax,
	        sourcedepthmax - sourcedepthmin);
	fprintf(output, "    Receiver Water Depth (m): %8f %8f %8f\n", receiverwaterdepthmin, receiverwaterdepthmax,
	        receiverwaterdepthmax - receiverwaterdepthmin);
	fprintf(output, "    Source Water Depth (m):   %8f %8f %8f\n", sourcewaterdepthmin, sourcewaterdepthmax,
	        sourcewaterdepthmax - sourcewaterdepthmin);
	fprintf(output, "\nNavigation Totals:\n");
	fprintf(output, "\n  Start of Data:\n");
	fprintf(output, "    Start Time:  %2.2d %2.2d %4.4d %2.2d:%2.2d:%2.2d.%6.6d  JD%d\n", timbeg_i[1], timbeg_i[2], timbeg_i[0],
	        timbeg_i[3], timbeg_i[4], timbeg_i[5], timbeg_i[6], timbeg_j[1]);
	fprintf(output, "    Start Position: Lon: %14.9f     Lat: %14.9f\n", lonbeg, latbeg);
	fprintf(output, "\n  End of Data:\n");
	fprintf(output, "    End Time:    %2.2d %2.2d %4.4d %2.2d:%2.2d:%2.2d.%6.6d  JD%d\n", timend_i[1], timend_i[2], timend_i[0],
	        timend_i[3], timend_i[4], timend_i[5], timend_i[6], timend_j[1]);
	fprintf(output, "    End Position:   Lon: %14.9f     Lat: %14.9f \n", lonend, latend);
	fprintf(output, "\nLimits:\n");
	fprintf(output, "  Minimum Longitude:   %14.9f   Maximum Longitude:   %14.9f\n", lonmin, lonmax);
	fprintf(output, "  Minimum Latitude:    %14.9f   Maximum Latitude:    %14.9f\n", latmin, latmax);

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
