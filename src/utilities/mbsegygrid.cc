/*--------------------------------------------------------------------
 *    The MB-system:	mbsegygrid.c	6/12/2004
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
 * mbsegygrid inserts trace data from segy data files into a grid in
 * which the y-axis is some measure of trace number, range, or distance
 * along a profile, and the y-axis is time..
 *
 * Author:	D. W. Caress
 * Date:	June 12, 2004
 */

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits>

#include "mb_aux.h"
#include "mb_define.h"
#include "mb_format.h"
#include "mb_segy.h"
#include "mb_status.h"

typedef enum {
    MBSEGYGRID_USESHOT = 0,
    MBSEGYGRID_USECMP = 1,
    MBSEGYGRID_USESHOTONLY = 2,
} useshot_t;
typedef enum {
    MBSEGYGRID_PLOTBYTRACENUMBER = 0,
    MBSEGYGRID_PLOTBYDISTANCE = 1,
} plotby_t;
typedef enum {
    MBSEGYGRID_WINDOW_OFF = 0,
    MBSEGYGRID_WINDOW_ON = 1,
    MBSEGYGRID_WINDOW_SEAFLOOR = 2,
    MBSEGYGRID_WINDOW_DEPTH = 3,
} windowmode_t;
typedef enum {
    MBSEGYGRID_GAIN_OFF = 0,
    MBSEGYGRID_GAIN_TZERO = 1,
    MBSEGYGRID_GAIN_SEAFLOOR = 2,
    MBSEGYGRID_GAIN_AGCSEAFLOOR = 3,
} gainmode_t;
typedef enum {
    MBSEGYGRID_GEOMETRY_VERTICAL = 0,
    MBSEGYGRID_GEOMETRY_REAL = 1,
} geometrymode_t;
typedef enum {
    MBSEGYGRID_FILTER_OFF = 0,
    MBSEGYGRID_FILTER_COSINE = 1,
} filtermode_t;

/* output stream for basic stuff (stdout if verbose <= 1,
    stderr if verbose > 1) */
FILE *outfp;

constexpr char program_name[] = "MBsegygrid";
constexpr char help_message[] =
    "MBsegygrid grids trace data from segy data files.";
constexpr char usage_message[] =
    "MBsegygrid -Ifile -Oroot [-Ashotscale/timescale\n"
    "          -Ddecimatex/decimatey -Gmode/gain[/window] -Rdistancebin[]/startlon/startlat/endlon/endlat]\n"
    "          -Smode[/start/end[/schan/echan]] -Tsweep[/delay]\n"
    "          -Wmode/start/end -H -V]";

/*--------------------------------------------------------------------*/
/*
 * function get_segy_limits gets info for default segy gridding
 */
int get_segy_limits(int verbose, char *segyfile, int *tracemode, int *tracestart, int *traceend, int *chanstart, int *chanend,
                    double *timesweep, double *timedelay, double *startlon, double *startlat, double *endlon, double *endlat,
                    int *error) {
	if (verbose >= 2) {
		fprintf(outfp, "\ndbg2  Function <%s> called\n", __func__);
		fprintf(outfp, "dbg2  Input arguments:\n");
		fprintf(outfp, "dbg2       verbose:    %d\n", verbose);
		fprintf(outfp, "dbg2       segyfile:   %s\n", segyfile);
	}

	/* set sinf filename */
	char sinffile[MB_PATH_MAXLINE] = "";
	snprintf(sinffile, sizeof(sinffile), "%s.sinf", segyfile);

	/* check status of segy and sinf file */
	int datmodtime = 0;
	int sinfmodtime = 0;
	struct stat file_status;
	int fstat = stat(segyfile, &file_status);
	if (fstat == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
		datmodtime = file_status.st_mtime;
	}
	fstat = stat(sinffile, &file_status);
	if (fstat == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
		sinfmodtime = file_status.st_mtime;
	}

	/* if sinf file is missing or out of date, make it */
	if (datmodtime > 0 && datmodtime > sinfmodtime) {
		if (verbose >= 1)
			fprintf(stderr, "\nGenerating sinf file for %s\n", segyfile);
		char command[MB_PATH_MAXLINE] = "";
		snprintf(command, sizeof(command), "mbsegyinfo -I %s -O", segyfile);
		/* int shellstatus = */ system(command);
	}

	double delay0 = 0.0;
	double delaydel = 0.0;
	int shot0;
	int shot1;
	int shottrace0;
	int shottrace1;
	int rp0;
	int rp1;
	int rpdel = 0;
	int rptrace0;
	int rptrace1;

	/* read sinf file if possible */
	snprintf(sinffile, sizeof(sinffile), "%s.sinf", segyfile);
	FILE *sfp = fopen(sinffile, "r");
	if (sfp != nullptr) {
		/* read the sinf file */
		char line[MB_PATH_MAXLINE] = "";
		while (fgets(line, MB_PATH_MAXLINE, sfp) != nullptr) {
			if (strncmp(line, "  Trace length (sec):", 21) == 0) {
				sscanf(line, "  Trace length (sec):%lf", timesweep);
			}
			else if (strncmp(line, "    Delay (sec):", 16) == 0) {
				double delay1 = 0.0;
				sscanf(line, "    Delay (sec): %lf %lf %lf", &delay0, &delay1, &delaydel);
			}
			else if (strncmp(line, "    Shot number:", 16) == 0) {
				int shotdel;
				sscanf(line, "    Shot number: %d %d %d", &shot0, &shot1, &shotdel);
			}
			else if (strncmp(line, "    Shot trace:", 15) == 0) {
				int shottracedel;
				sscanf(line, "    Shot trace: %d %d %d", &shottrace0, &shottrace1, &shottracedel);
			}
			else if (strncmp(line, "    RP number:", 14) == 0) {
				sscanf(line, "    RP number: %d %d %d", &rp0, &rp1, &rpdel);
			}
			else if (strncmp(line, "    RP trace:", 13) == 0) {
				int rptracedel;
				sscanf(line, "    RP trace: %d %d %d", &rptrace0, &rptrace1, &rptracedel);
			}
			else if (strncmp(line, "    Start Position:", 19) == 0) {
				sscanf(line, "    Start Position: Lon: %lf     Lat:   %lf", startlon, startlat);
			}
			else if (strncmp(line, "    End Position:", 17) == 0) {
				sscanf(line, "    End Position:   Lon: %lf     Lat:   %lf", endlon, endlat);
			}
		}
		fclose(sfp);
	}

	/* set the trace mode */
	if (rpdel > 1) {
		*tracemode = MBSEGYGRID_USECMP;
		*tracestart = rp0;
		*traceend = rp1;
		*chanstart = rptrace0;
		*chanend = rptrace1;
	}
	else {
		*tracemode = MBSEGYGRID_USESHOT;
		*tracestart = shot0;
		*traceend = shot1;
		*chanstart = shottrace0;
		*chanend = shottrace1;
	}

	/* set the sweep and delay */
	if (delaydel > 0.0) {
		*timesweep += delaydel;
	}
	*timedelay = delay0;

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(outfp, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(outfp, "dbg2  Return values:\n");
		fprintf(outfp, "dbg2       tracemode:  %d\n", *tracemode);
		fprintf(outfp, "dbg2       tracestart: %d\n", *tracestart);
		fprintf(outfp, "dbg2       traceend:   %d\n", *traceend);
		fprintf(outfp, "dbg2       chanstart:  %d\n", *chanstart);
		fprintf(outfp, "dbg2       chanend:    %d\n", *chanend);
		fprintf(outfp, "dbg2       timesweep:  %f\n", *timesweep);
		fprintf(outfp, "dbg2       timedelay:  %f\n", *timedelay);
		fprintf(outfp, "dbg2       startlon:   %f\n", *startlon);
		fprintf(outfp, "dbg2       startlat:   %f\n", *startlat);
		fprintf(outfp, "dbg2       endlon:     %f\n", *endlon);
		fprintf(outfp, "dbg2       endlat:     %f\n", *endlat);
		fprintf(outfp, "dbg2       error:      %d\n", *error);
		fprintf(outfp, "dbg2  Return status:\n");
		fprintf(outfp, "dbg2       status:     %d\n", status);
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

	char segyfile[MB_PATH_MAXLINE] = "";
	double shotscale = 1.0;
	double timescale = 1.0;
	bool scale2distance = false;
	bool agcmode = false;
	double agcwindow = 0.0;
	double agcmaxvalue = 0.0;
	geometrymode_t geometrymode = MBSEGYGRID_GEOMETRY_VERTICAL;
	int decimatex = 1;
	int decimatey = 1;
	double filterwindow = 0.0;
	filtermode_t filtermode = MBSEGYGRID_FILTER_OFF;
	double gain = 0.0;
	gainmode_t gainmode = MBSEGYGRID_GAIN_OFF;
	double gainwindow = 0.0;
	double gaindelay = 0.0;
	char fileroot[MB_PATH_MAXLINE] = "";
	double distancebin = 1.0;
	double startlon = 0.0;
	double startlat = 0.0;
	double endlon = 0.0;
	double endlat = 0.0;
	plotby_t plotmode = MBSEGYGRID_PLOTBYTRACENUMBER;
	int tracestart = 0;
	int traceend = 0;
	int chanstart = 0;
	int chanend = -1;
	useshot_t tracemode = MBSEGYGRID_USESHOT;
	bool tracemode_set = false;
	double timesweep = 0.0;
	double timedelay = 0.0;
	double windowstart;
	double windowend;
	windowmode_t windowmode = MBSEGYGRID_WINDOW_OFF;

	/* process argument list */
	{
		bool errflg = false;
		int c;
		bool help = false;
		while ((c = getopt(argc, argv, "A:a:B:b:C:c:D:d:F:f:G:g:I:i:O:o:R:r:S:s:T:t:VvW:w:Hh")) != -1)
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
			{
				const int n = sscanf(optarg, "%lf/%lf", &shotscale, &timescale);
				if (n == 2)
					scale2distance = true;
				break;
			}
			case 'B':
			case 'b':
			{
				const int n = sscanf(optarg, "%lf/%lf", &agcmaxvalue, &agcwindow);
				if (n < 2)
					agcwindow = 0.0;
				agcmode = true;
				break;
			}
			case 'C':
			case 'c':
			{
				int geometrymode_tmp;
				const int n = sscanf(optarg, "%d", &geometrymode_tmp);
				geometrymode = (geometrymode_t)geometrymode_tmp;  // TODO(schwehr): Range check
				if (n < 1)
					geometrymode = MBSEGYGRID_GEOMETRY_VERTICAL;
				break;
			}
			case 'D':
			case 'd':
				/* n = */ sscanf(optarg, "%d/%d", &decimatex, &decimatey);
				break;
			case 'F':
			case 'f':
			{
				int filtermode_tmp;
				/* n = */ sscanf(optarg, "%d/%lf", &filtermode_tmp, &filterwindow);
				filtermode = (filtermode_t)filtermode_tmp;  // TODO(schwehr): Range check
				break;
			}
			case 'G':
			case 'g':
			{
				int gainmode_tmp;
				const int n = sscanf(optarg, "%d/%lf/%lf/%lf", &gainmode_tmp, &gain, &gainwindow, &gaindelay);
				gainmode = (gainmode_t)gainmode_tmp;  // TODO(schwehr): Range check
				if (n < 4)
					gaindelay = 0.0;
				if (n < 3)
					gainwindow = 0.0;
				break;
			}
			case 'I':
			case 'i':
				sscanf(optarg, "%1023s", segyfile);
				break;
			case 'O':
			case 'o':
				sscanf(optarg, "%1023s", fileroot);
				break;
			case 'R':
			case 'r':
			{
				const int n = sscanf(optarg, "%lf/%lf/%lf/%lf/%lf", &distancebin, &startlon, &endlon, &startlat, &endlat);
				plotmode = MBSEGYGRID_PLOTBYDISTANCE;
				if (n < 1) {
					distancebin = 1.0;
				}
				if (n < 25) {
					startlon = 0.0;
					startlat = 0.0;
					endlon = 0.0;
					endlat = 0.0;
				}
				break;
			}
			case 'S':
			case 's':
			{
				int tracemode_tmp;
				const int n = sscanf(optarg, "%d/%d/%d/%d/%d", &tracemode_tmp, &tracestart, &traceend, &chanstart, &chanend);
				tracemode = (useshot_t)tracemode_tmp;  // TODO(schwehr): Range check.
				if (n < 5) {
					chanstart = 0;
					chanend = -1;
				}
				if (n < 3) {
					tracestart = 0;
					traceend = 0;
				}
				if (n < 1) {
					tracemode = MBSEGYGRID_USESHOT;
				}
				else {
					tracemode_set = true;
				}
				break;
			}
			case 'T':
			case 't':
			{
				const int n = sscanf(optarg, "%lf/%lf", &timesweep, &timedelay);
				if (n < 2)
					timedelay = 0.0;
				break;
			}
			case 'W':
			case 'w':
			{
				// TODO(schwehr): Check n to make sure all 3 parts are read.
				int windowmode_tmp;
				/* n = */ sscanf(optarg, "%d/%lf/%lf", &windowmode_tmp, &windowstart, &windowend);
				windowmode = (windowmode_t)windowmode_tmp;  // TODO(schwehr): Range check
				break;
			}
			case '?':
				errflg = true;
			}

		outfp = verbose >= 2 ? stderr : stdout;

		if (errflg) {
			fprintf(outfp, "usage: %s\n", usage_message);
			fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_ERROR_BAD_USAGE);
		}

		if (verbose == 1 || help) {
			fprintf(outfp, "\nProgram %s\n", program_name);
			fprintf(outfp, "MB-system Version %s\n", MB_VERSION);
		}

		if (verbose >= 2) {
			fprintf(outfp, "\ndbg2  Program <%s>\n", program_name);
			fprintf(outfp, "dbg2  MB-system Version %s\n", MB_VERSION);
			fprintf(outfp, "dbg2  Control Parameters:\n");
			fprintf(outfp, "dbg2       verbose:        %d\n", verbose);
			fprintf(outfp, "dbg2       help:           %d\n", help);
			fprintf(outfp, "dbg2       segyfile:       %s\n", segyfile);
			fprintf(outfp, "dbg2       fileroot:       %s\n", fileroot);
			fprintf(outfp, "dbg2       decimatex:      %d\n", decimatex);
			fprintf(outfp, "dbg2       decimatey:      %d\n", decimatey);
			fprintf(outfp, "dbg2       plotmode:       %d\n", plotmode);
			fprintf(outfp, "dbg2       distancebin:    %f\n", distancebin);
			fprintf(outfp, "dbg2       startlon:       %f\n", startlon);
			fprintf(outfp, "dbg2       startlat:       %f\n", startlat);
			fprintf(outfp, "dbg2       endlon:         %f\n", endlon);
			fprintf(outfp, "dbg2       endlat:         %f\n", endlat);
			fprintf(outfp, "dbg2       tracemode:      %d\n", tracemode);
			fprintf(outfp, "dbg2       tracestart:     %d\n", tracestart);
			fprintf(outfp, "dbg2       traceend:       %d\n", traceend);
			fprintf(outfp, "dbg2       chanstart:      %d\n", chanstart);
			fprintf(outfp, "dbg2       chanend:        %d\n", chanend);
			fprintf(outfp, "dbg2       timesweep:      %f\n", timesweep);
			fprintf(outfp, "dbg2       timedelay:      %f\n", timedelay);
			// fprintf(outfp, "dbg2       ngridx:         %d\n", ngridx);
			// fprintf(outfp, "dbg2       ngridy:         %d\n", ngridy);
			// fprintf(outfp, "dbg2       ngridxy:        %d\n", ngridxy);
			fprintf(outfp, "dbg2       windowmode:     %d\n", windowmode);
			fprintf(outfp, "dbg2       windowstart:    %f\n", windowstart);
			fprintf(outfp, "dbg2       windowend:      %f\n", windowend);
			fprintf(outfp, "dbg2       agcmode:        %d\n", agcmode);
			fprintf(outfp, "dbg2       agcmaxvalue:    %f\n", agcmaxvalue);
			fprintf(outfp, "dbg2       agcwindow:      %f\n", agcwindow);
			fprintf(outfp, "dbg2       gainmode:       %d\n", gainmode);
			fprintf(outfp, "dbg2       gain:           %f\n", gain);
			fprintf(outfp, "dbg2       gainwindow:     %f\n", gainwindow);
			fprintf(outfp, "dbg2       gaindelay:      %f\n", gaindelay);
			fprintf(outfp, "dbg2       filtermode:     %d\n", filtermode);
			fprintf(outfp, "dbg2       filterwindow:   %f\n", filterwindow);
			fprintf(outfp, "dbg2       geometrymode:   %d\n", geometrymode);
			fprintf(outfp, "dbg2       scale2distance: %d\n", scale2distance);
			fprintf(outfp, "dbg2       shotscale:      %f\n", shotscale);
			fprintf(outfp, "dbg2       timescale:      %f\n", timescale);
		}

		if (help) {
			fprintf(outfp, "\n%s\n", help_message);
			fprintf(outfp, "\nusage: %s\n", usage_message);
			exit(MB_ERROR_NO_ERROR);
		}
	}

	int error = MB_ERROR_NO_ERROR;

	int sinftracemode = MBSEGYGRID_USESHOT;
	int sinftracestart = 0;
	int sinftraceend = 0;
	int sinfchanstart = 0;
	int sinfchanend = -1;

	double sinftimesweep = 0.0;
	double sinftimedelay = 0.0;
	double sinfstartlon = 0.0;
	double sinfstartlat = 0.0;
	double sinfendlon = 0.0;
	double sinfendlat = 0.0;

	/* get segy limits if required */
	if (traceend < 1 || traceend < tracestart || timesweep <= 0.0 || (plotmode == MBSEGYGRID_PLOTBYDISTANCE && startlon == 0.0)) {
		get_segy_limits(verbose, segyfile, &sinftracemode, &sinftracestart, &sinftraceend, &sinfchanstart, &sinfchanend,
		                &sinftimesweep, &sinftimedelay, &sinfstartlon, &sinfstartlat, &sinfendlon, &sinfendlat, &error);
		if (traceend < 1 || traceend < tracestart) {
			if (!tracemode_set)
		                tracemode = static_cast<useshot_t>(sinftracemode);
			tracestart = sinftracestart;
			traceend = sinftraceend;
		}
		if (chanend < 1 || chanend < chanstart) {
			chanstart = sinfchanstart;
			chanend = sinfchanend;
		}
		if (timesweep <= 0.0) {
			timesweep = sinftimesweep;
			timedelay = sinftimedelay;
		}
		if (sinfstartlon != sinfendlon && sinfstartlat != sinfendlat) {
			startlon = sinfstartlon;
			startlat = sinfstartlat;
			endlon = sinfendlon;
			endlat = sinfendlat;
		}
	}

	/* check specified parameters */
	if (traceend < 1 || traceend < tracestart) {
		fprintf(outfp, "\nBad trace numbers: %d %d specified...\n", tracestart, traceend);
		fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}
	if (timesweep <= 0.0) {
		fprintf(outfp, "\nBad time sweep: %f specified...\n", timesweep);
		fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}
	if (tracemode == MBSEGYGRID_USESHOTONLY) {
		chanstart = 0;
		chanend = -1;
	}

	/* initialize reading the segy file */
	void *mbsegyioptr;
	struct mb_segyasciiheader_struct asciiheader;
	struct mb_segyfileheader_struct fileheader;
	if (mb_segy_read_init(verbose, segyfile, &mbsegyioptr, &asciiheader, &fileheader, &error) != MB_SUCCESS) {
		char *message;
		mb_error(verbose, error, &message);
		fprintf(outfp, "\nMBIO Error returned from function <mb_segy_read_init>:\n%s\n", message);
		fprintf(outfp, "\nSEGY File <%s> not initialized for reading\n", segyfile);
		fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}

	/* calculate implied grid parameters */
	char gridfile[MB_PATH_MAXLINE] = "";
	strcpy(gridfile, fileroot);
	strcat(gridfile, ".grd");
	const int ntraces =
		chanend >= chanstart
		? (traceend - tracestart + 1) * (chanend - chanstart + 1)
		: traceend - tracestart + 1;

	int ngridx = 0;
	int ngridy = 0;
	int ngridxy = 0;
	double sampleinterval = 0.0;
	double xmin;
	double xmax;
	double ymin;
	double ymax;
	double dx;
	double dy;
	double mtodeglon;
	double mtodeglat;
	double line_dx = 0.0;
	double line_dy = 0.0;

	/* set up plotting trace by trace */
	if (plotmode == MBSEGYGRID_PLOTBYTRACENUMBER) {
		ngridx = ntraces / decimatex;
		sampleinterval = 0.000001 * (double)(fileheader.sample_interval);
		ngridy = timesweep / sampleinterval / decimatey + 1;
		ngridxy = ngridx * ngridy;
		xmin = (double)tracestart - 0.5;
		xmax = (double)traceend + 0.5;
		ymax = -(timedelay - 0.5 * sampleinterval / decimatey);
		ymin = ymax - ngridy * sampleinterval * decimatey;
		/*ymax = timedelay + timesweep + 0.5 * sampleinterval / decimatey;*/
	}

	/* set up plotting trace by distance along a line */
	else if (plotmode == MBSEGYGRID_PLOTBYDISTANCE) {
		/* get distance scaling */
		mb_coor_scale(verbose, 0.5 * (startlat + endlat), &mtodeglon, &mtodeglat);
		dx = (endlon - startlon) / mtodeglon;
		dy = (endlat - startlat) / mtodeglat;
		const double line_distance = sqrt(dx * dx + dy * dy);
		line_dx = dx / line_distance;
		line_dy = dy / line_distance;

		ngridx = (int)(line_distance / distancebin / decimatex);
		sampleinterval = 0.000001 * (double)(fileheader.sample_interval);
		ngridy = timesweep / sampleinterval / decimatey + 1;
		ngridxy = ngridx * ngridy;
		xmin = -0.5 * distancebin;
		xmax = line_distance + 0.5 * distancebin;
		ymax = -(timedelay - 0.5 * sampleinterval / decimatey);
		ymin = ymax - ngridy * sampleinterval * decimatey;
		/*ymax = timedelay + timesweep + 0.5 * sampleinterval / decimatey;*/
	}

	/* get start and end samples */
	int iystart;
	int iyend;
	if (windowmode == MBSEGYGRID_WINDOW_OFF) {
		iystart = 0;
		iyend = ngridy - 1;
	}
	else if (windowmode == MBSEGYGRID_WINDOW_ON) {
		iystart = std::max((windowstart) / sampleinterval, 0.0);
		iyend = std::min((windowend) / sampleinterval, ngridy - 1.0);
	}
	// TODO(schwehr): What about MBSEGYGRID_WINDOW_SEAFLOOR?
	// TODO(schwehr): What about MBSEGYGRID_WINDOW_DEPTH?

	float *grid = nullptr;
	status &= mb_mallocd(verbose, __FILE__, __LINE__, ngridxy * sizeof(float), (void **)&grid, &error);
	float *gridweight = nullptr;
	status &= mb_mallocd(verbose, __FILE__, __LINE__, ngridxy * sizeof(float), (void **)&gridweight, &error);

	// TODO(schwehr): When is verbose ever negative?
	if (verbose >= 0) {
		fprintf(outfp, "\nMBsegygrid Parameters:\n");
		fprintf(outfp, "Input segy file:         %s\n", segyfile);
		fprintf(outfp, "Output fileroot:         %s\n", fileroot);
		fprintf(outfp, "Input Parameters:\n");
		fprintf(outfp, "     plot mode:          %d\n", plotmode);
		fprintf(outfp, "     trace mode:         %d\n", tracemode);
		fprintf(outfp, "     trace start:        %d\n", tracestart);
		fprintf(outfp, "     trace end:          %d\n", traceend);
		fprintf(outfp, "     channel start:      %d\n", chanstart);
		fprintf(outfp, "     channel end:        %d\n", chanend);
		fprintf(outfp, "     start longitude:    %f\n", startlon);
		fprintf(outfp, "     start latitude:     %f\n", startlat);
		fprintf(outfp, "     end longitude:      %f\n", endlon);
		fprintf(outfp, "     end latitude:       %f\n", endlat);
		fprintf(outfp, "     trace decimation:   %d\n", decimatex);
		fprintf(outfp, "     time sweep:         %f seconds\n", timesweep);
		fprintf(outfp, "     time delay:         %f seconds\n", timedelay);
		fprintf(outfp, "     sample interval:    %f seconds\n", sampleinterval);
		fprintf(outfp, "     sample decimation:  %d\n", decimatey);
		fprintf(outfp, "     window mode:        %d\n", windowmode);
		fprintf(outfp, "     window start:       %f seconds\n", windowstart);
		fprintf(outfp, "     window end:         %f seconds\n", windowend);
		fprintf(outfp, "     agcmode:            %d\n", agcmode);
		fprintf(outfp, "     gain mode:          %d\n", gainmode);
		fprintf(outfp, "     gain:               %f\n", gain);
		fprintf(outfp, "     gainwindow:         %f\n", gainwindow);
		fprintf(outfp, "     gaindelay:          %f\n", gaindelay);
		fprintf(outfp, "Output Parameters:\n");
		fprintf(outfp, "     grid filename:      %s\n", gridfile);
		fprintf(outfp, "     x grid dimension:   %d\n", ngridx);
		fprintf(outfp, "     y grid dimension:   %d\n", ngridy);
		fprintf(outfp, "     grid xmin:          %f\n", xmin);
		fprintf(outfp, "     grid xmax:          %f\n", xmax);
		fprintf(outfp, "     grid ymin:          %f\n", ymin);
		fprintf(outfp, "     grid ymax:          %f\n", ymax);
		fprintf(outfp, "     NaN values used to flag regions with no data\n");
		if (scale2distance) {
			fprintf(outfp, "     shot and time scaled to distance in meters\n");
			fprintf(outfp, "     shotscale:          %f\n", shotscale);
			fprintf(outfp, "     timescale:          %f\n", timescale);
			fprintf(outfp, "     scaled grid xmin    %f\n", 0.0);
			fprintf(outfp, "     scaled grid xmax:   %f\n", shotscale * (xmax - xmin));
			fprintf(outfp, "     scaled grid ymin:   %f\n", 0.0);
			fprintf(outfp, "     scaled grid ymax:   %f\n", timescale * (ymax - ymin));
		}
	}
	if (verbose > 0)
		fprintf(outfp, "\n");

	float *worktrace = nullptr;
	float *filtertrace = nullptr;
	double gridmintot = 0.0;
	double gridmaxtot = 0.0;

	if (status == MB_SUCCESS) {
		/* initialize grid and weight arrays */
		for (int k = 0; k < ngridxy; k++) {
			grid[k] = 0.0;
			gridweight[k] = 0.0;
		}

		bool traceok;
		int filtertrace_alloc = 0;
		int worktrace_alloc = 0;
		int ix;
		int iy;
		int tracecount;
		int tracenum;
		int channum;
		double btimesave;
		double dtimesave;

		/* read and print data */
		int nread = 0;
		while (error <= MB_ERROR_NO_ERROR) {
			struct mb_segytraceheader_struct traceheader;
			float *trace = nullptr;

			error = MB_ERROR_NO_ERROR;

			/* read a trace */
			status = mb_segy_read_trace(verbose, mbsegyioptr, &traceheader, &trace, &error);

			/* now process the trace */
			if (status == MB_SUCCESS) {
				/* figure out where this trace is in the grid laterally */
				double trace_x = 0.0;
				if (plotmode == MBSEGYGRID_PLOTBYTRACENUMBER) {
					if (tracemode == MBSEGYGRID_USESHOT) {
						tracenum = traceheader.shot_num;
						channum = traceheader.shot_tr;
					}
					else if (tracemode == MBSEGYGRID_USECMP) {
						tracenum = traceheader.rp_num;
						channum = traceheader.rp_tr;
					}
					else if (tracemode == MBSEGYGRID_USESHOTONLY) {
						tracenum = traceheader.shot_num;
						channum = 0;
					}
					if (tracemode != MBSEGYGRID_USESHOTONLY && chanend >= chanstart) {
						tracecount = (tracenum - tracestart) * (chanend - chanstart + 1) + (channum - chanstart);
					}
					else {
						tracecount = tracenum - tracestart;
					}
					ix = tracecount / decimatex;

					/* now check if this is a trace of interest */
					traceok = true;
					if (tracenum < tracestart || tracenum > traceend)
						traceok = false;
					else if (chanend >= chanstart && (channum < chanstart || channum > chanend))
						traceok = false;
					else if (tracecount % decimatex != 0)
						traceok = false;
				}
				else if (plotmode == MBSEGYGRID_PLOTBYDISTANCE) {
					const double factor =
						traceheader.coord_scalar < 0
						? 1.0 / ((float)(-traceheader.coord_scalar)) / 3600.0
						: (float)traceheader.coord_scalar / 3600.0;
					// TODO(schwehr): Why cast to float when doing to a double?
					double navlon =
						traceheader.src_long != 0
						? factor * ((float)traceheader.src_long)
						: factor * ((float)traceheader.grp_long);
					double navlat =
						traceheader.src_lat != 0
						? factor * ((float)traceheader.src_lat)
						: factor * ((float)traceheader.grp_lat);
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
					dx = (navlon - startlon) / mtodeglon;
					dy = (navlat - startlat) / mtodeglat;
					trace_x = dx * line_dx + dy * line_dy;
					ix = ((int)((trace_x - 0.5 * distancebin) / distancebin)) / decimatex;
					if (ix >= 0 && ix < ngridx)
						traceok = true;
					else
						traceok = false;
				}

				/* figure out where this trace is in the grid vertically */
				double factor =
					traceheader.elev_scalar < 0
					? 1.0 / ((float)(-traceheader.elev_scalar))
					: (float)traceheader.elev_scalar;
				double btime;
				double dtime;
				if (traceheader.src_depth > 0) {
					btime = factor * traceheader.src_depth / 750.0 + 0.001 * traceheader.delay_mils;
					dtime = factor * traceheader.src_depth / 750.0;
					btimesave = btime;
					dtimesave = dtime;
				}
				else if (traceheader.src_elev > 0) {
					btime = -factor * traceheader.src_elev / 750.0 + 0.001 * traceheader.delay_mils;
					dtime = -factor * traceheader.src_elev / 750.0;
					btimesave = btime;
					dtimesave = dtime;
				}
				else {
					btime = btimesave;
					dtime = dtimesave;
				}
				double stimesave = 0.0;
				double stime;
				if (traceheader.src_wbd > 0) {
					stime = factor * traceheader.src_wbd / 750.0;
					stimesave = stime;
				}
				else {
					stime = stimesave;
				}
				const int iys = (btime - timedelay) / sampleinterval;

				/* get trace min and max */
				double tracemin = trace[0];
				double tracemax = trace[0];
				for (int i = 0; i < traceheader.nsamps; i++) {
					tracemin = std::min(tracemin, static_cast<double>(trace[i]));
					tracemax = std::max(tracemin, static_cast<double>(trace[i]));
				}

				if ((verbose == 0 && nread % 250 == 0) || (nread % 25 == 0)) {
					if (traceok)
						fprintf(outfp, "PROCESS ");
					else
						fprintf(outfp, "IGNORE  ");
					if (tracemode == MBSEGYGRID_USESHOT)
						fprintf(outfp, "read:%d position:%d shot:%d channel:%d ", nread, tracecount, tracenum, channum);
					else
						fprintf(outfp, "read:%d position:%d rp:%d channel:%d ", nread, tracecount, tracenum, channum);
					if (plotmode == MBSEGYGRID_PLOTBYDISTANCE)
						fprintf(outfp, "distance:%.3f ", trace_x);
					fprintf(outfp, "%4.4d/%3.3d %2.2d:%2.2d:%2.2d.%3.3d samples:%d interval:%d usec minmax: %f %f\n",
					        traceheader.year, traceheader.day_of_yr, traceheader.hour, traceheader.min, traceheader.sec,
					        traceheader.mils, traceheader.nsamps, traceheader.si_micros, tracemin, tracemax);
				}

				/* now actually process traces of interest */
				if (traceok) {
					/* get bounds of trace in depth window mode */
					if (windowmode == MBSEGYGRID_WINDOW_DEPTH) {
						iystart = (int)((dtime + windowstart - timedelay) / sampleinterval);
						iystart = std::max(iystart, 0);
						iyend = (int)((dtime + windowend - timedelay) / sampleinterval);
						iyend = std::min(iyend, ngridy - 1);
					}
					else if (windowmode == MBSEGYGRID_WINDOW_SEAFLOOR) {
						iystart = std::max((stime + windowstart - timedelay) / sampleinterval, 0.0);
						iyend = std::min((stime + windowend - timedelay) / sampleinterval, ngridy - 1.0);
					}

					/* apply gain if desired */
					if (gainmode == MBSEGYGRID_GAIN_TZERO || gainmode == MBSEGYGRID_GAIN_SEAFLOOR) {
						int igainstart =
							gainmode == MBSEGYGRID_GAIN_TZERO
							? (dtime - btime + gaindelay) / sampleinterval
							: (stime - btime + gaindelay) / sampleinterval;
						igainstart = std::max(0, igainstart);
						int igainend;
						if (gainwindow <= 0.0) {
							igainend = traceheader.nsamps - 1;
						} else {
							igainend = igainstart + gainwindow / sampleinterval;
							igainend = std::min(traceheader.nsamps - 1, igainend);
						}
						for (int i = 0; i <= igainstart; i++) {
							trace[i] = 0.0;
						}
						for (int i = igainstart; i <= igainend; i++) {
							const double gtime = (i - igainstart) * sampleinterval;
							factor = 1.0 + gain * gtime;
							trace[i] = trace[i] * factor;
						}
						for (int i = igainend + 1; i <= traceheader.nsamps; i++) {
							trace[i] = 0.0;
						}
					}
					else if (gainmode == MBSEGYGRID_GAIN_AGCSEAFLOOR) {
						int igainstart = (stime - btime - 0.5 * gainwindow) / sampleinterval;
						igainstart = std::max(0, igainstart);
						int igainend = (stime - btime + 0.5 * gainwindow) / sampleinterval;
						igainend = std::min(traceheader.nsamps - 1, igainend);
						double tmax = fabs(trace[igainstart]);
						for (int i = igainstart; i <= igainend; i++) {
							tmax = std::max(tmax, static_cast<double>(fabs(trace[i])));
						}
						if (tmax > 0.0)
							factor = gain / tmax;
						else
							factor = 1.0;
						for (int i = 0; i <= traceheader.nsamps; i++) {
							trace[i] *= factor;
						}
					}

					/* apply filtering if desired */
					if (filtermode != MBSEGYGRID_FILTER_OFF) {
						if (worktrace == nullptr || traceheader.nsamps > worktrace_alloc) {
							status = mb_reallocd(verbose, __FILE__, __LINE__, traceheader.nsamps * sizeof(float),
							                     (void **)&worktrace, &error);
							worktrace_alloc = traceheader.nsamps;
						}
						const int nfilter = 2 * ((int)(0.5 * filterwindow / sampleinterval)) + 1;
						if (filtertrace == nullptr || nfilter > filtertrace_alloc) {
							status =
							    mb_reallocd(verbose, __FILE__, __LINE__, nfilter * sizeof(float), (void **)&filtertrace, &error);
							filtertrace_alloc = nfilter;
						}
						// double filtersum = 0.0;
						for (int j = 0; j < nfilter; j++) {
							const double cos_arg = (0.5 * M_PI * (j - nfilter / 2)) / (0.5 * nfilter);
							filtertrace[j] = cos(cos_arg);
							// filtersum += filtertrace[j];
						}
						for (int i = 0; i <= traceheader.nsamps; i++) {
							worktrace[i] = 0.0;
							double filtersum = 0.0;
							const int jstart = std::max(nfilter / 2 - i, 0);
							const int jend = std::min(nfilter - 1, nfilter - 1 + (traceheader.nsamps - 1 - nfilter / 2 - i));
							for (int j = jstart; j <= jend; j++) {
								const int ii = i - nfilter / 2 + j;
								worktrace[i] += filtertrace[j] * trace[ii];
								filtersum += filtertrace[j];
							}
							worktrace[i] /= filtersum;
						}
						for (int i = 0; i <= traceheader.nsamps; i++) {
							trace[i] = worktrace[i];
						}
					}

					/* apply agc if desired */
					if (agcmode && agcwindow > 0.0) {
						if (worktrace == nullptr || traceheader.nsamps > worktrace_alloc) {
							status = mb_reallocd(verbose, __FILE__, __LINE__, traceheader.nsamps * sizeof(float),
							                     (void **)&worktrace, &error);
							worktrace_alloc = traceheader.nsamps;
						}
						const int iagchalfwindow = 0.5 * agcwindow / sampleinterval;
						for (int i = 0; i <= traceheader.nsamps; i++) {
							int igainstart = i - iagchalfwindow;
							igainstart = std::max(0, igainstart);
							int igainend = i + iagchalfwindow;
							igainend = std::min(traceheader.nsamps - 1, igainend);
							double tmax = 0.0;
							for (int j = igainstart; j <= igainend; j++) {
								tmax = std::max(tmax, static_cast<double>(fabs(trace[j])));
							}
							if (tmax > 0.0)
								worktrace[i] = trace[i] * agcmaxvalue / tmax;
							else
								worktrace[i] = trace[i];
						}
						for (int i = 0; i <= traceheader.nsamps; i++) {
							trace[i] = worktrace[i];
						}
					}
					else if (agcmode) {
						double tmax = 0.0;
						for (int i = 0; i <= traceheader.nsamps; i++) {
							tmax = std::max(tmax, static_cast<double>(fabs(trace[i])));
						}
						if (tmax > 0.0)
							factor = agcmaxvalue / tmax;
						else
							factor = 1.0;
						for (int i = 0; i <= traceheader.nsamps; i++) {
							trace[i] *= factor;
						}
					}

					/* process trace for simple vertical geometry */
					if (geometrymode == MBSEGYGRID_GEOMETRY_VERTICAL) {
						for (int i = 0; i < traceheader.nsamps; i++) {
							iy = (ngridy - 1) - (iys + i / decimatey);
							const int k = ix * ngridy + iy;
							if (iy >= iystart && iy <= iyend) {
								grid[k] += trace[i];
								gridweight[k] += 1.0;
							}
						}
					}

					/* process trace for real geometry using pitch */
					else /* if (geometrymode == MBSEGYGRID_GEOMETRY_REAL) */
					{
						const double cosfactor = cos(DTR * traceheader.pitch);
						for (int i = 0; i < traceheader.nsamps; i++) {
							/* get corrected y location of this sample
							  in the section grid using the pitch angle */
							const int iyc = iys + (int)(cosfactor * ((double)i)) / decimatey;

							/* get the index of the sample location */
							if (iyc >= iystart && iyc <= iyend) {
								iy = (ngridy - 1) - iyc;
								const int k = ix * ngridy + iy;
								grid[k] += trace[i];
								gridweight[k] += 1.0;
							}
						}
					}
				}
			}

			/* now process the trace */
			if (status == MB_SUCCESS)
				nread++;
		}

		/* calculate the grid */
		gridmintot = 0.0;
		gridmaxtot = 0.0;
		for (int k = 0; k < ngridxy; k++) {
			if (gridweight[k] > 0.0) {
				grid[k] = grid[k] / gridweight[k];
				gridmintot = std::min(static_cast<double>(grid[k]), gridmintot);
				gridmaxtot = std::max(static_cast<double>(grid[k]), gridmaxtot);
			}
			else {
				grid[k] = std::numeric_limits<float>::quiet_NaN();
			}
		}
	}

	/* grid controls */
	char xlabel[MB_PATH_MAXLINE] = "";
	char ylabel[MB_PATH_MAXLINE] = "";

	int plot_status;

	/* write out the grid */
	error = MB_ERROR_NO_ERROR;
	status = MB_SUCCESS;
	char projection[MB_PATH_MAXLINE] = "";
	strcpy(projection, "SeismicProfile");
	if (scale2distance) {
		strcpy(xlabel, "Distance (m)");
		strcpy(ylabel, "Depth (m)");
		xmax = shotscale * (xmax - xmin);
		xmin = 0.0;
		ymin = timescale * ymin;
		ymax = timescale * ymax;
		dx = shotscale * decimatex;
		dy = timescale * sampleinterval / decimatey;
	} else {
		strcpy(xlabel, "Trace Number");
		strcpy(ylabel, "Travel Time (seconds)");
		dx = (double)decimatex;
		dy = sampleinterval / decimatey;
	}

	char zlabel[MB_PATH_MAXLINE] = "";
	strcpy(zlabel, "Trace Signal");
	char title[MB_PATH_MAXLINE+100] = "";
	snprintf(title, sizeof(title), "Seismic Grid from %s", segyfile);
        const double NaN = std::numeric_limits<float>::quiet_NaN();
	status &= mb_write_gmt_grd(verbose, gridfile, grid, NaN, ngridx, ngridy, xmin, xmax, ymin, ymax, gridmintot, gridmaxtot, dx,
	                          dy, xlabel, ylabel, zlabel, title, projection, argc, argv, &error);

	status &= mb_segy_close(verbose, &mbsegyioptr, &error);

	/* deallocate memory for grid array */
	if (worktrace != nullptr)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&worktrace, &error);
	if (filtertrace != nullptr)
		status = mb_freed(verbose, __FILE__, __LINE__, (void **)&filtertrace, &error);
	status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&grid, &error);
	status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&gridweight, &error);

	/* run mbm_grdplot */
	const double xwidth = std::min(0.01 * (double)ngridx, 55.0);
	const double ywidth = std::min(0.01 * (double)ngridy, 28.0);
	char plot_cmd[5*MB_PATH_MAXLINE] = "";
	snprintf(plot_cmd, sizeof(plot_cmd), "mbm_grdplot -I%s -JX%f/%f -G1 -V -L\"File %s - %s:%s\"", gridfile, xwidth, ywidth, gridfile, title,
	        zlabel);
	if (verbose) {
		fprintf(outfp, "\nexecuting mbm_grdplot...\n%s\n", plot_cmd);
	}
	plot_status = system(plot_cmd);
	// TODO(schwehr): man of mbm_grdplot does not describe the return code.  Only 0 is success.
	if (plot_status != 0) {
		fprintf(outfp, "\nError executing mbm_grdplot on grid file %s\n", gridfile);
	}

	if (verbose >= 4)
		status &= mb_memory_list(verbose, &error);

	if (verbose >= 2) {
		fprintf(outfp, "\ndbg2  Program <%s> completed\n", program_name);
		fprintf(outfp, "dbg2  Ending status:\n");
		fprintf(outfp, "dbg2       status:  %d\n", status);
	}

	exit(error);
}
/*--------------------------------------------------------------------*/
