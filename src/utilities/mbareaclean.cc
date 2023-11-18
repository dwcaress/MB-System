/*--------------------------------------------------------------------
 *    The MB-system:	mbareaclean.c	2/27/2003
 *
 *    Copyright (c) 2003-2023 by
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
 * mbareaclean identifies and flags artifacts in swath sonar bathymetry data.
 * The edit events are output to edit save files which can be applied
 * to the data by the program mbprocess. These are the same edit save
 * files created and/or modified by mbclean and mbedit.
 * The input data are one swath file or a datalist referencing multiple
 * swath files. An area is specified in longitude and latitude bounds,
 * along with a bin size in meters. The area is divided into a grid with
 * square cells of the specified bin size. As the data are read, each of
 * the soundings that fall within one of the bins is stored. Once all of
 * data are read, one or more statistical tests are performed on the soundings
 * within each bin, providing there are a sufficient number of soundings.
 * The user may specify one or both of the following actions:
 *   1) Previously unflagged soundings that fail a test are flagged as bad.
 *   2) Previously flagged soundings that pass all tests are unflagged.
 * If a sounding's flag status is changed, that flagging action is output
 * to the edit save file of the swath file containing that sounding. This
 * program will create edit save files if necessary, or append to those that
 * already exist.
 *
 * Author:	D. W. Caress
 * Date:	February 27, 2003
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <algorithm>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_info.h"
#include "mb_io.h"
#include "mb_process.h"
#include "mb_status.h"
#include "mb_swap.h"

/* allocation */
constexpr int FILEALLOCNUM = 16;
constexpr int PINGALLOCNUM = 128;
constexpr int SNDGALLOCNUM = 128;

struct mbareaclean_file_struct {
	char filelist[MB_PATH_MAXLINE];
	int file_format;
	int nping;
	int nping_alloc;
	int nnull;
	int nflag;
	int ngood;
	int nunflagged;
	int nflagged;
	double *ping_time_d;
	int *pingmultiplicity;
	double *ping_altitude;
	int nsndg;
	int nsndg_alloc;
	int sndg_countstart;
	int beams_bath;
	struct mbareaclean_sndg_struct *sndg;
};
struct mbareaclean_sndg_struct {
	int sndg_file;
	int sndg_ping;
	int sndg_beam;
	double sndg_depth;
	double sndg_x;
	double sndg_y;
	char sndg_beamflag_org;
	char sndg_beamflag_esf;
	char sndg_beamflag;
	bool sndg_edit;
};

/* sounding storage values and arrays */
int nfile = 0;
int nfile_alloc = 0;
struct mbareaclean_file_struct *files = nullptr;
int nsndg = 0;
int nsndg_alloc = 0;
int sndg_countstart = 0;
int **gsndg = nullptr;
int *gsndgnum = nullptr;
int *gsndgnum_alloc = nullptr;
struct mbareaclean_sndg_struct *sndg = nullptr;

constexpr char program_name[] = "MBAREACLEAN";
constexpr char help_message[] = "MBAREACLEAN identifies and flags artifacts in swath bathymetry data";
constexpr char usage_message[] =
    "mbareaclean [-Fformat -Iinfile -Rwest/east/south/north -B -G -Sbinsize\n"
    "\t -Mthreshold/nmin -Dthreshold[/nmin[/nmax]] -Ttype -N[-]minbeam/maxbeam]";

/*--------------------------------------------------------------------*/
int getsoundingptr(int verbose, int soundingid, struct mbareaclean_sndg_struct **sndgptr, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       soundingid:      %d\n", soundingid);
		fprintf(stderr, "dbg2       sndgptr:         %p\n", (void *)sndgptr);
	}

	/* loop over the files until the sounding is found */
	*sndgptr = nullptr;
	for (int i = 0; i < nfile && *sndgptr == nullptr; i++) {
		if (soundingid >= files[i].sndg_countstart && soundingid < files[i].sndg_countstart + files[i].nsndg) {
			const int j = soundingid - files[i].sndg_countstart;
			*sndgptr = &(files[i].sndg[j]);
		}
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       *sndgptr:        %p\n", (void *)sndgptr);
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/

int flag_sounding(int verbose, bool flag, bool output_bad, bool output_good, struct mbareaclean_sndg_struct *sndg, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:       %d\n", verbose);
		fprintf(stderr, "dbg2       flag:          %d\n", flag);
		fprintf(stderr, "dbg2       output_bad:    %d\n", output_bad);
		fprintf(stderr, "dbg2       output_good:   %d\n", output_good);
		fprintf(stderr, "dbg2       sndg->sndg_edit:     %d\n", sndg->sndg_edit);
		fprintf(stderr, "dbg2       sndg->sndg_beam:     %d\n", sndg->sndg_beam);
		fprintf(stderr, "dbg2       sndg->sndg_beamflag: %d\n", sndg->sndg_beamflag);
	}

	if (sndg->sndg_edit) {
		if (output_bad && mb_beam_ok(sndg->sndg_beamflag) && flag) {
			sndg->sndg_beamflag = MB_FLAG_FLAG + MB_FLAG_FILTER;
			files[sndg->sndg_file].nflagged++;
		}

		else if (output_good && !mb_beam_ok(sndg->sndg_beamflag) && sndg->sndg_beamflag != MB_FLAG_NULL && !flag) {
			sndg->sndg_beamflag = MB_FLAG_NONE;
			files[sndg->sndg_file].nunflagged++;
		}

		else if (output_good && !mb_beam_ok(sndg->sndg_beamflag) && sndg->sndg_beamflag != MB_FLAG_NULL && flag) {
			sndg->sndg_edit = false;
		}
	}

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       sndg->sndg_edit:     %d\n", sndg->sndg_edit);
		fprintf(stderr, "dbg2       sndg->sndg_beamflag: %d\n", sndg->sndg_beamflag);
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

	/* reset all defaults but the format and lonflip */
	format = 0;
	pings = 1;
	bounds[0] = -360.;
	bounds[1] = 360.;
	bounds[2] = -90.;
	bounds[3] = 90.;
	btime_i[0] = 1962;
	btime_i[1] = 2;
	btime_i[2] = 21;
	btime_i[3] = 10;
	btime_i[4] = 30;
	btime_i[5] = 0;
	btime_i[6] = 0;
	etime_i[0] = 2062;
	etime_i[1] = 2;
	etime_i[2] = 21;
	etime_i[3] = 10;
	etime_i[4] = 30;
	etime_i[5] = 0;
	etime_i[6] = 0;
	speedmin = 0.0;
	timegap = 1000000000.0;

	char read_file[MB_PATH_MAXLINE] = "datalist.mb-1";
	bool output_bad = false;
	bool std_dev_filter = false;
	double std_dev_threshold = 2.0;
	int std_dev_nmin = 10;
	bool median_filter = false;
	double median_filter_threshold = 0.25;
	int median_filter_nmin = 10;
	bool mediandensity_filter = false;
	int mediandensity_filter_nmax = 0;
	bool limit_beams = false;
	bool output_good = false;
	bool beam_in = true;
	int min_beam = 0;
	int max_beam = 0;
	int max_beam_no = 0;
	bool plane_fit = false;
	// double plane_fit_threshold = 0.05;
	// int plane_fit_nmin = 10;
	double areabounds[4];
	bool areaboundsset = false;
	double binsize = 0.0;
	bool binsizeset = false;
	int flag_detect = MB_DETECT_AMPLITUDE;
	bool use_detect = false;

	{
		bool errflg = false;
		int c;
		bool help = false;
		while ((c = getopt(argc, argv, "VvHhBbGgD:d:F:f:I:i:M:m:N:n:P:p:S:sT:t::R:r:")) != -1)
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
			case 'B':
			case 'b':
				output_bad = true;
				break;
			case 'D':
			case 'd':
				std_dev_filter = true;
				sscanf(optarg, "%lf/%d", &std_dev_threshold, &std_dev_nmin);
				break;
			case 'F':
			case 'f':
				sscanf(optarg, "%d", &format);
				break;
			case 'G':
			case 'g':
				output_good = true;
				break;
			case 'I':
			case 'i':
				sscanf(optarg, "%1023s", read_file);
				break;
			case 'M':
			case 'm':
			{
				median_filter = true;
				double d1;
				int i1;
				int i2;
				const int n = sscanf(optarg, "%lf/%d/%d", &d1, &i1, &i2);
				if (n > 0)
					median_filter_threshold = d1;
				if (n > 1)
					median_filter_nmin = i1;
				if (n > 2) {
					mediandensity_filter = true;
					mediandensity_filter_nmax = i2;
				}
				break;
			}
			case 'N':
			case 'n':
				limit_beams = true;
				sscanf(optarg, "%d/%d", &min_beam, &max_beam_no);
				if (optarg[0] == '-') {
					min_beam = -min_beam;
					beam_in = false;
				}
				if (max_beam_no < 0)
					max_beam_no = -max_beam_no;
				max_beam = max_beam_no;
				if (max_beam < min_beam)
					max_beam = min_beam;
				break;
			case 'P':
			case 'p':
			{
				// TODO(schwehr): -p not in the man page.
				plane_fit = true;
				// sscanf(optarg, "%lf", &plane_fit_threshold);
				// double d1;
				// double d2;
				// int i1;
				// const int n = sscanf(optarg, "%lf/%d/%lf", &d1, &i1, &d2);
				// if (n > 0)
				// 	plane_fit_threshold = d1;
				// if (n > 1)
				//	plane_fit_nmin = i1;
				break;
			}
			case 'R':
			case 'r':
				mb_get_bounds(optarg, areabounds);
				areaboundsset = true;
				break;
			case 'S':
			case 's':
				sscanf(optarg, "%lf", &binsize);
				binsizeset = true;
				break;
			case 'T':
			case 't':
				use_detect = true;
				sscanf(optarg, "%d", &flag_detect);
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

		/* turn on median filter if nothing specified */
		if (!median_filter && !plane_fit && !std_dev_filter)
			median_filter = true;

		/* turn on output bad if nothing specified */
		if (!output_bad && !output_good)
			output_bad = true;

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
			fprintf(stderr, "dbg2       pings:          %d\n", pings);
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
			fprintf(stderr, "dbg2       data format:    %d\n", format);
			fprintf(stderr, "dbg2       input file:     %s\n", read_file);
			fprintf(stderr, "dbg2       median_filter:             %d\n", median_filter);
			fprintf(stderr, "dbg2       median_filter_threshold:   %f\n", median_filter_threshold);
			fprintf(stderr, "dbg2       median_filter_nmin:        %d\n", median_filter_nmin);
			fprintf(stderr, "dbg2       mediandensity_filter:      %d\n", mediandensity_filter);
			fprintf(stderr, "dbg2       mediandensity_filter_nmax: %d\n", mediandensity_filter_nmax);
			fprintf(stderr, "dbg2       plane_fit:                 %d\n", plane_fit);
			// fprintf(stderr, "dbg2       plane_fit_threshold:       %f\n", plane_fit_threshold);
			// fprintf(stderr, "dbg2       plane_fit_nmin:            %d\n", plane_fit_nmin);
			fprintf(stderr, "dbg2       std_dev_filter:            %d\n", std_dev_filter);
			fprintf(stderr, "dbg2       std_dev_threshold:         %f\n", std_dev_threshold);
			fprintf(stderr, "dbg2       std_dev_nmin:              %d\n", std_dev_nmin);
			fprintf(stderr, "dbg2       use_detect:                %d\n", use_detect);
			fprintf(stderr, "dbg2       flag_detect:               %d\n", flag_detect);
			fprintf(stderr, "dbg2       limit_beams:               %d\n", limit_beams);
			fprintf(stderr, "dbg2       beam_in:                   %d\n", beam_in);
			fprintf(stderr, "dbg2       min_beam:                  %d\n", min_beam);
			fprintf(stderr, "dbg2       max_beam_no                %d\n", max_beam_no);
			fprintf(stderr, "dbg2       output_good:    %d\n", output_good);
			fprintf(stderr, "dbg2       output_bad:     %d\n", output_bad);
			fprintf(stderr, "dbg2       areaboundsset:  %d\n", areaboundsset);
			fprintf(stderr, "dbg2       areabounds[0]:  %f\n", areabounds[0]);
			fprintf(stderr, "dbg2       areabounds[1]:  %f\n", areabounds[1]);
			fprintf(stderr, "dbg2       areabounds[2]:  %f\n", areabounds[2]);
			fprintf(stderr, "dbg2       areabounds[3]:  %f\n", areabounds[3]);
			fprintf(stderr, "dbg2       binsizeset:     %d\n", binsizeset);
			fprintf(stderr, "dbg2       binsize:        %f\n", binsize);
		}

		if (help) {
			fprintf(stderr, "\n%s\n", help_message);
			fprintf(stderr, "\nusage: %s\n", usage_message);
			exit(MB_ERROR_NO_ERROR);
		}
	}

	int error = MB_ERROR_NO_ERROR;
	int formatread;
	void *datalist;

	/* if bounds not set get bounds of input data */
	if (!areaboundsset) {
		formatread = format;
		struct mb_info_struct mb_info;
		memset(&mb_info, 0, sizeof(struct mb_info_struct));
		status &= mb_get_info_datalist(verbose, read_file, &formatread, &mb_info, lonflip, &error);

		areabounds[0] = mb_info.lon_min;
		areabounds[1] = mb_info.lon_max;
		areabounds[2] = mb_info.lat_min;
		areabounds[3] = mb_info.lat_max;

		if (!binsizeset)
			binsize = 0.2 * mb_info.altitude_max;
	}

	/* calculate grid properties */
	double mtodeglon;
	double mtodeglat;
	mb_coor_scale(verbose, 0.5 * (areabounds[2] + areabounds[3]), &mtodeglon, &mtodeglat);
	if (binsize <= 0.0)
		binsize = (areabounds[1] - areabounds[0]) / 101 / mtodeglon;
	double dx = binsize * mtodeglon;
	double dy = binsize * mtodeglat;
	const int nx = 1 + (int)((areabounds[1] - areabounds[0]) / dx);
	const int ny = 1 + (int)((areabounds[3] - areabounds[2]) / dy);
	if (nx > 1 && ny > 1) {
		dx = (areabounds[1] - areabounds[0]) / (nx - 1);
		dy = (areabounds[3] - areabounds[2]) / (ny - 1);
	}

	/* allocate grid arrays */
	nsndg = 0;
	nsndg_alloc = 0;
	status &= mb_mallocd(verbose, __FILE__, __LINE__, nx * ny * sizeof(int *), (void **)&gsndg, &error);
	if (status == MB_SUCCESS)
		status &= mb_mallocd(verbose, __FILE__, __LINE__, nx * ny * sizeof(int), (void **)&gsndgnum, &error);
	if (status == MB_SUCCESS)
		status &= mb_mallocd(verbose, __FILE__, __LINE__, nx * ny * sizeof(int), (void **)&gsndgnum_alloc, &error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR || status != MB_SUCCESS) {
		char *message = nullptr;
		mb_error(verbose, error, &message);
		fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}

	/* if error initializing memory then quit */
	for (int i = 0; i < nx * ny; i++) {
		gsndg[i] = nullptr;
		gsndgnum[i] = 0;
		gsndgnum_alloc[i] = 0;
	}

	/* give the statistics */
	if (verbose >= 0) {
		fprintf(stderr, "Area of interest:\n");
		fprintf(stderr, "     Minimum Longitude: %.6f Maximum Longitude: %.6f\n", areabounds[0], areabounds[1]);
		fprintf(stderr, "     Minimum Latitude:  %.6f Maximum Latitude:  %.6f\n", areabounds[2], areabounds[3]);
		fprintf(stderr, "     Bin Size:   %f\n", binsize);
		fprintf(stderr, "     Dimensions: %d %d\n", nx, ny);
		fprintf(stderr, "Cleaning algorithms:\n");
		if (median_filter) {
			fprintf(stderr, "     Median filter: ON\n");
			fprintf(stderr, "     Median filter threshold:    %f\n", median_filter_threshold);
			fprintf(stderr, "     Median filter minimum N:    %d\n", median_filter_nmin);
		}
		else
			fprintf(stderr, "     Median filter: OFF\n");
		if (mediandensity_filter) {
			fprintf(stderr, "     Median Density filter: ON\n");
			fprintf(stderr, "     Median Density filter maximum N:    %d\n", mediandensity_filter_nmax);
		}
		else
			fprintf(stderr, "     Median Density filter: OFF\n");
		if (plane_fit) {
			fprintf(stderr, "     Plane fit:     ON\n");
			fprintf(stderr, "     Plane fit threshold:        %f\n", median_filter_threshold);
			fprintf(stderr, "     Plane fit minimum N:        %d\n", median_filter_nmin);
		}
		else
			fprintf(stderr, "     Plane fit:     OFF\n");
		if (std_dev_filter) {
			fprintf(stderr, "     Standard deviation filter: ON\n");
			fprintf(stderr, "     Standard deviation filter threshold:    %f\n", std_dev_threshold);
			fprintf(stderr, "     Standard deviation filter minimum N:    %d\n", std_dev_nmin);
		}
		else
			fprintf(stderr, "     Standard deviation filter: OFF\n");
		fprintf(stderr, "Restrictions:\n");
		if (use_detect) {
			fprintf(stderr, "     Only flag if bottom detection algorithn is: ");
			if (flag_detect == MB_DETECT_UNKNOWN)
				fprintf(stderr, "UNKNOWN\n");
			else if (flag_detect == MB_DETECT_AMPLITUDE)
				fprintf(stderr, "AMPLITUDE\n");
			else if (flag_detect == MB_DETECT_PHASE)
				fprintf(stderr, "PHASE\n");
			else
				fprintf(stderr, "%d\n", flag_detect);
		}
		if (limit_beams) {
			fprintf(stderr, "     Only flag if beams ");
			if (beam_in)
				fprintf(stderr, "between");
			else
				fprintf(stderr, "outside");
			fprintf(stderr, " beams %d - %d\n", min_beam, max_beam_no);
		}
		else
			fprintf(stderr, "     Flag all beams\n");
		fprintf(stderr, "Output:\n");
		if (output_bad)
			fprintf(stderr, "     Flag unflagged soundings identified as bad:  ON\n");
		else
			fprintf(stderr, "     Flag unflagged soundings identified as bad:  OFF\n");
		if (output_good)
			fprintf(stderr, "     Unflag flagged soundings identified as good: ON\n");
		else
			fprintf(stderr, "     Unflag flagged soundings identified as good: OFF\n");
	}

	/* get format if required */
	if (format == 0)
		mb_get_format(verbose, read_file, nullptr, &format, &error);

	/* determine whether to read one file or a list of files */
	const bool read_datalist = format < 0;
	bool read_data;
	char swathfile[MB_PATH_MAXLINE];
	char swathfileread[MB_PATH_MAXLINE];
	char dfile[MB_PATH_MAXLINE];
	double file_weight;

	/* open file list */
	if (read_datalist) {
		const int look_processed = MB_DATALIST_LOOK_UNSET;
		if (mb_datalist_open(verbose, &datalist, read_file, look_processed, &error) != MB_SUCCESS) {
			error = MB_ERROR_OPEN_FAIL;
			fprintf(stderr, "\nUnable to open data list file: %s\n", read_file);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}
		read_data = mb_datalist_read(verbose, datalist, swathfile, dfile, &format, &file_weight, &error) == MB_SUCCESS;
	} else {
		// else copy single filename to be read
		strcpy(swathfile, read_file);
		read_data = true;
	}

	void *mbio_ptr = nullptr;
	double btime_d;
	double etime_d;
	int beams_bath;
	int beams_amp;
	int pixels_ss;
	char *beamflag = nullptr;
	char *beamflagorg = nullptr;
	int *detect = nullptr;
	double *bath = nullptr;
	double *amp = nullptr;
	double *bathlon = nullptr;
	double *bathlat = nullptr;
	double *ss = nullptr;
	double *sslon = nullptr;
	double *sslat = nullptr;

	/* save file control variables */
	char esffile[MB_PATH_MAXLINE];
	struct mb_esf_struct esf;
  memset(&esf, 0, sizeof(struct mb_esf_struct));
	int files_tot = 0;

	int kind;
	int pingsread;
	int time_i[7];
	double time_d;
	double navlon;
	double navlat;
	double speed;
	double heading;
	double distance;
	double altitude;
	double sensordepth;
	char comment[MB_COMMENT_MAXLINE];

  void *store_ptr = nullptr;

	int pingmultiplicity;
	int pings_tot = 0;
	int beams_tot = 0;
	int beams_good_org_tot = 0;
	int beams_flag_org_tot = 0;
	int beams_null_org_tot = 0;

	/* loop over all files to be read */
	while (read_data) {

		/* check format and get format flags */
		int variable_beams;
		int traveltime;
		int beam_flagging;
		if ((status = mb_format_flags(verbose, &format, &variable_beams, &traveltime, &beam_flagging, &error)) != MB_SUCCESS) {
			char *message = nullptr;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error returned from function <mb_format_flags> regarding input format %d:\n%s\n", format,
			        message);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}

		/* check for "fast bathymetry" or "fbt" file */
		strcpy(swathfileread, swathfile);
		formatread = format;
		if (!use_detect)
			mb_get_fbt(verbose, swathfileread, &formatread, &error);

		/* initialize reading the input swath sonar file */
		if (mb_read_init(verbose, swathfileread, formatread, pings, lonflip, bounds, btime_i, etime_i, speedmin,
		                           timegap, &mbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) !=
		    MB_SUCCESS) {
			char *message = nullptr;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
			fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", swathfileread);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}

		/* initialize and increment counting variables */
		int pings_file = 0;
		/* int beams_file = 0; */

		/* give the statistics */
		if (verbose >= 0) {
			fprintf(stderr, "\nProcessing %s\n", swathfileread);
		}

		/* allocate memory for data arrays */
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(int), (void **)&detect, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlon, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathlat, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslon, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&sslat, &error);
		if (error == MB_ERROR_NO_ERROR)
			status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflagorg, &error);

		/* if error initializing memory then quit */
		if (error != MB_ERROR_NO_ERROR) {
			char *message = nullptr;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}

		/* update memory for files */
		if (nfile >= nfile_alloc) {
			nfile_alloc += FILEALLOCNUM;
			status &= mb_reallocd(verbose, __FILE__, __LINE__, nfile_alloc * sizeof(struct mbareaclean_file_struct),
			                     (void **)&files, &error);

			/* if error initializing memory then quit */
			if (error != MB_ERROR_NO_ERROR) {
				char *message = nullptr;
				mb_error(verbose, error, &message);
				fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
				fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
				exit(error);
			}
		}

		/* initialize current file */
		strcpy(files[nfile].filelist, swathfile);
		files[nfile].file_format = format;
		files[nfile].nping = 0;
		files[nfile].nping_alloc = PINGALLOCNUM;
		files[nfile].nnull = 0;
		files[nfile].nflag = 0;
		files[nfile].ngood = 0;
		files[nfile].nflagged = 0;
		files[nfile].nunflagged = 0;
		files[nfile].ping_time_d = nullptr;
		files[nfile].pingmultiplicity = nullptr;
		files[nfile].ping_altitude = nullptr;
		files[nfile].nsndg = 0;
		files[nfile].nsndg_alloc = SNDGALLOCNUM;
		files[nfile].sndg_countstart = nsndg;
		files[nfile].beams_bath = beams_bath;
		files[nfile].sndg = nullptr;
		status &= mb_mallocd(verbose, __FILE__, __LINE__, files[nfile].nping_alloc * sizeof(double),
		                    (void **)&(files[nfile].ping_time_d), &error);
		if (status == MB_SUCCESS)
			status &= mb_mallocd(verbose, __FILE__, __LINE__, files[nfile].nping_alloc * sizeof(int),
			                    (void **)&(files[nfile].pingmultiplicity), &error);
		if (status == MB_SUCCESS)
			status &= mb_mallocd(verbose, __FILE__, __LINE__, files[nfile].nping_alloc * sizeof(double),
			                    (void **)&(files[nfile].ping_altitude), &error);
		if (status == MB_SUCCESS)
			status &= mb_mallocd(verbose, __FILE__, __LINE__, files[nfile].nsndg_alloc * sizeof(struct mbareaclean_sndg_struct),
			                    (void **)&(files[nfile].sndg), &error);
		if (error != MB_ERROR_NO_ERROR) {
			char *message = nullptr;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}
		nfile++;

		/* now deal with old edit save file */
		if (status == MB_SUCCESS) {
			/* handle esf edits */
      int esf_error = MB_ERROR_NO_ERROR;
			int esf_status = mb_esf_load(verbose, (char *)program_name, swathfile, true, false, esffile, &esf, &esf_error);
		}

		/* read */
		bool done = false;
		files_tot++;
		pings_file = 0;
		/* beams_file = 0; */
		int beams_good_org_file = 0;
		int beams_flag_org_file = 0;
		int beams_null_org_file = 0;
		while (!done) {
			if (verbose > 1)
				fprintf(stderr, "\n");

			/* read next record */
			error = MB_ERROR_NO_ERROR;
			status &= mb_read(verbose, mbio_ptr, &kind, &pingsread, time_i, &time_d, &navlon, &navlat, &speed, &heading, &distance,
			                 &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp, bathlon, bathlat,
			                 ss, sslon, sslat, comment, &error);
			if (verbose >= 2) {
				fprintf(stderr, "\ndbg2  current data status:\n");
				fprintf(stderr, "dbg2    kind:       %d\n", kind);
				fprintf(stderr, "dbg2    status:     %d\n", status);
			}
			if (status == MB_SUCCESS && kind == MB_DATA_DATA) {
				/* save the beamflags */
				for (int i = 0; i < beams_bath; i++)
					beamflagorg[i] = beamflag[i];

				/* get detections and ping multiplicity */
				/* status = */ mb_get_store(verbose, mbio_ptr, &store_ptr, &error);
				int detect_error;
				const int detect_status = mb_detects(verbose, mbio_ptr, store_ptr, &kind, &beams_bath, detect, &detect_error);
				if (detect_status != MB_SUCCESS) {
					// status = MB_SUCCESS;
					for (int i = 0; i < beams_bath; i++) {
						detect[i] = MB_DETECT_UNKNOWN;
					}
				}
				int sensorhead;
				int sensorhead_error = MB_ERROR_NO_ERROR;
				const int sensorhead_status = mb_sensorhead(verbose, mbio_ptr, store_ptr, &sensorhead, &sensorhead_error);

				/* allocate memory if necessary */
				if (files[nfile - 1].nping >= files[nfile - 1].nping_alloc) {
					files[nfile - 1].nping_alloc += PINGALLOCNUM;
					status = mb_reallocd(verbose, __FILE__, __LINE__, files[nfile - 1].nping_alloc * sizeof(double),
					                     (void **)&(files[nfile - 1].ping_time_d), &error);
					if (status == MB_SUCCESS)
						status = mb_reallocd(verbose, __FILE__, __LINE__, files[nfile - 1].nping_alloc * sizeof(int),
						                     (void **)&(files[nfile - 1].pingmultiplicity), &error);
					if (status == MB_SUCCESS)
						/* status = */ mb_reallocd(verbose, __FILE__, __LINE__, files[nfile - 1].nping_alloc * sizeof(double),
						                     (void **)&(files[nfile - 1].ping_altitude), &error);
					if (error != MB_ERROR_NO_ERROR) {
						char *message = nullptr;
						mb_error(verbose, error, &message);
						fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
						fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
						exit(error);
					}
				}

				/* store the ping data */
				if (sensorhead_status == MB_SUCCESS) {
					pingmultiplicity = sensorhead;
				}
				else if (files[nfile - 1].nping > 0
						 && fabs(time_d - files[nfile - 1].ping_time_d[files[nfile - 1].nping - 1]) < MB_ESF_MAXTIMEDIFF) {
					pingmultiplicity
					    = files[nfile - 1].pingmultiplicity[files[nfile - 1].nping - 1] + 1;
				}
				else {
					pingmultiplicity = 0;
				}
        status = mb_esf_apply(verbose, &esf, time_d, pingmultiplicity, beams_bath, beamflagorg, &error);

				/* update counters */
				pings_tot++;
				pings_file++;
				for (int i = 0; i < beams_bath; i++) {
					if (mb_beam_ok(beamflagorg[i])) {
						beams_tot++;
						/* beams_file++; */
						beams_good_org_tot++;
						beams_good_org_file++;
						files[nfile - 1].ngood++;
					}
					else if (beamflagorg[i] == MB_FLAG_NULL) {
						beams_null_org_tot++;
						beams_null_org_file++;
						files[nfile - 1].nnull++;
					}
					else {
						beams_tot++;
						/* beams_file++; */
						beams_flag_org_tot++;
						beams_flag_org_file++;
						files[nfile - 1].nflag++;
					}
				}

				files[nfile - 1].ping_time_d[files[nfile - 1].nping] = time_d;
				files[nfile - 1].pingmultiplicity[files[nfile - 1].nping] = pingmultiplicity;
				files[nfile - 1].ping_altitude[files[nfile - 1].nping] = altitude;
				files[nfile - 1].nping++;

				/* check beam range */
				if (limit_beams && max_beam_no == 0)
					max_beam = beams_bath - min_beam;

				/* now loop over the beams and store the soundings in the grid bins */
				for (int ib = 0; ib < beams_bath; ib++) {
					if (beamflagorg[ib] != MB_FLAG_NULL) {
						/* get bin for current beam */
						const int ix = (bathlon[ib] - areabounds[0] - 0.5 * dx) / dx;
						const int iy = (bathlat[ib] - areabounds[2] - 0.5 * dy) / dy;
						const int kgrid = ix * ny + iy;

						/* add sounding */
						if (ix >= 0 && ix < nx && iy >= 0 && iy < ny) {
							if (files[nfile - 1].nsndg >= files[nfile - 1].nsndg_alloc) {
								files[nfile - 1].nsndg_alloc += SNDGALLOCNUM;
								status = mb_reallocd(verbose, __FILE__, __LINE__,
								                     files[nfile - 1].nsndg_alloc * sizeof(struct mbareaclean_sndg_struct),
								                     (void **)&files[nfile - 1].sndg, &error);
								if (error != MB_ERROR_NO_ERROR) {
									char *message = nullptr;
									mb_error(verbose, error, &message);
									fprintf(stderr, "\nMBIO Error allocating sounding arrays:\n%s\n", message);
									fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
									exit(error);
								}
							}

							/* allocate space for sounding if needed */
							if (gsndgnum[kgrid] >= gsndgnum_alloc[kgrid]) {
								gsndgnum_alloc[kgrid] += SNDGALLOCNUM;
								status = mb_reallocd(verbose, __FILE__, __LINE__, gsndgnum_alloc[kgrid] * sizeof(int),
								                     (void **)&gsndg[kgrid], &error);
								if (error != MB_ERROR_NO_ERROR) {
									char *message = nullptr;
									mb_error(verbose, error, &message);
									fprintf(stderr, "\nMBIO Error allocating sounding arrays:\n%s\n", message);
									fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
									exit(error);
								}
							}

							/* store sounding data */
							sndg = &(files[nfile - 1].sndg[files[nfile - 1].nsndg]);
							sndg->sndg_file = nfile - 1;
							sndg->sndg_ping = files[nfile - 1].nping - 1;
							sndg->sndg_beam = ib;
							sndg->sndg_depth = bath[ib];
							sndg->sndg_x = bathlon[ib];
							sndg->sndg_y = bathlat[ib];
							sndg->sndg_beamflag_org = beamflag[ib];
							sndg->sndg_beamflag_esf = beamflagorg[ib];
							sndg->sndg_beamflag = beamflagorg[ib];
							sndg->sndg_edit = true;
							if (use_detect && detect[ib] != flag_detect)
								sndg->sndg_edit = false;
							if (limit_beams) {
								if (min_beam <= ib && ib <= max_beam) {
									if (!beam_in)
										sndg->sndg_edit = false;
								}
								else {
									if (beam_in)
										sndg->sndg_edit = false;
								}
							}
							files[nfile - 1].nsndg++;
							nsndg++;
							gsndg[kgrid][gsndgnum[kgrid]] = files[nfile - 1].sndg_countstart + files[nfile - 1].nsndg - 1;
							gsndgnum[kgrid]++;
						}
					}
				}
			}
			else if (error > MB_ERROR_NO_ERROR) {
				done = true;
			}

			/* process a record */

			/* reset counters and data */
		}

		/* close the files */
		status = mb_close(verbose, &mbio_ptr, &error);
		mb_esf_close(verbose, &esf, &error);

		/* check memory */
		if (verbose >= 4)
			status &= mb_memory_list(verbose, &error);

		/* give the statistics */
		if (verbose >= 0) {
			fprintf(stderr, "pings:%4d  beams: %7d good %7d flagged %7d null \n", pings_file, beams_good_org_file,
			        beams_flag_org_file, beams_null_org_file);
		}

		/* figure out whether and what to read next */
		if (read_datalist) {
			if (/* status = */ mb_datalist_read(verbose, datalist, swathfile, dfile, &format, &file_weight, &error) == MB_SUCCESS)
				read_data = true;
			else
				read_data = false;
		}
		else {
			read_data = false;
		}

		/* end loop over files in list */
	}
	if (read_datalist)
		mb_datalist_close(verbose, &datalist, &error);


	/* median filter parameters */
	double *bindepths;
	double threshold;

	/* counting parameters */

	/* loop over grid cells to find maximum number of soundings */
	int binnummax = 0;
	// double xx;
	// double yy;
	for (int ix = 0; ix < nx; ix++) {
		for (int iy = 0; iy < ny; iy++) {
			/* get cell id */
			const int kgrid = ix * ny + iy;
			// xx = areabounds[0] + 0.5 * dx + ix * dx;
			// yy = areabounds[3] + 0.5 * dy + iy * dy;
			binnummax = std::max(binnummax, gsndgnum[kgrid]);
		}
        }
	/* status = */ mb_mallocd(verbose, __FILE__, __LINE__, binnummax * sizeof(double), (void **)&(bindepths), &error);
	if (error != MB_ERROR_NO_ERROR) {
		char *message = nullptr;
		mb_error(verbose, error, &message);
		fprintf(stderr, "\nMBIO Error allocating sounding sorting array:\n%s\n", message);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}

	/* deal with median filter */
	if (median_filter) {
		/* loop over grid cells applying median filter test */
		for (int ix = 0; ix < nx; ix++)
			for (int iy = 0; iy < ny; iy++) {
				/* get cell id */
				const int kgrid = ix * ny + iy;
				// xx = areabounds[0] + 0.5 * dx + ix * dx;
				// yy = areabounds[3] + 0.5 * dy + iy * dy;

				/* load up array */
				int binnum = 0;
				for (int i = 0; i < gsndgnum[kgrid]; i++) {
					getsoundingptr(verbose, gsndg[kgrid][i], &sndg, &error);
					if (mb_beam_ok(sndg->sndg_beamflag)) {
						bindepths[binnum] = sndg->sndg_depth;
						binnum++;
					}
				}

				/* apply median filter only if there are enough soundings */
				if (binnum >= median_filter_nmin) {
					/* run qsort */
					qsort((void *)bindepths, binnum, sizeof(double), mb_double_compare);
					const double median_depth = bindepths[binnum / 2];
					double median_depth_low;
					double median_depth_high;
					if (mediandensity_filter && binnum / 2 - mediandensity_filter_nmax / 2 >= 0)
						median_depth_low = bindepths[binnum / 2 + mediandensity_filter_nmax / 2];
					else
						median_depth_low = bindepths[0];
					if (mediandensity_filter && binnum / 2 + mediandensity_filter_nmax / 2 < binnum)
						median_depth_high = bindepths[binnum / 2 + mediandensity_filter_nmax / 2];
					else
						median_depth_high = bindepths[binnum - 1];

					/* process the soundings */
					for (int i = 0; i < gsndgnum[kgrid]; i++) {
						getsoundingptr(verbose, gsndg[kgrid][i], &sndg, &error);
						threshold = fabs(median_filter_threshold * files[sndg->sndg_file].ping_altitude[sndg->sndg_ping]);
						bool flagsounding = false;
						if (fabs(sndg->sndg_depth - median_depth) > threshold)
							flagsounding = true;
						if (mediandensity_filter &&
						    (sndg->sndg_depth > median_depth_high || sndg->sndg_depth < median_depth_low))
							flagsounding = true;
						flag_sounding(verbose, flagsounding, output_bad, output_good, sndg, &error);
					}
				}
			}
	}

	/* deal with standard deviation filter */
	if (std_dev_filter) {
		/* loop over grid cells applying std dev filter test */
		for (int ix = 0; ix < nx; ix++)
			for (int iy = 0; iy < ny; iy++) {
				/* get cell id */
				const int kgrid = ix * ny + iy;
				const double xx = areabounds[0] + 0.5 * dx + ix * dx;
				const double yy = areabounds[3] + 0.5 * dy + iy * dy;

				/* get mean */
				double mean = 0.0;
				int binnum = 0;
				for (int i = 0; i < gsndgnum[kgrid]; i++) {
					getsoundingptr(verbose, gsndg[kgrid][i], &sndg, &error);
					if (mb_beam_ok(sndg->sndg_beamflag)) {
						mean += sndg->sndg_depth;
						binnum++;
					}
				}
				mean /= binnum;

				/* get standard deviation */
				double std_dev = 0.0;
				for (int i = 0; i < gsndgnum[kgrid]; i++) {
					getsoundingptr(verbose, gsndg[kgrid][i], &sndg, &error);
					if (mb_beam_ok(sndg->sndg_beamflag))
						std_dev += (sndg->sndg_depth - mean) * (sndg->sndg_depth - mean);
				}
				std_dev = sqrt(std_dev / binnum);

				threshold = std_dev * std_dev_threshold;

				if (binnum > 0)
					fprintf(stderr, "bin: %d %d %d  pos: %f %f  nsoundings:%d / %d mean:%f std_dev:%f\n", ix, iy, kgrid, xx, yy,
					        binnum, gsndgnum[kgrid], mean, std_dev);

				/* apply standard deviation threshold only if there are enough soundings */
				if (binnum >= std_dev_nmin) {

					/* process the soundings */
					for (int i = 0; i < gsndgnum[kgrid]; i++) {
						getsoundingptr(verbose, gsndg[kgrid][i], &sndg, &error);
						flag_sounding(verbose, fabs(sndg->sndg_depth - mean) > threshold, output_bad, output_good, sndg, &error);
					}
				}
			}
	}


	/* loop over files checking for changed soundings */
	for (int i = 0; i < nfile; i++) {
		/* open esf file */
		status = mb_esf_load(verbose, (char *)program_name, files[i].filelist, false, true, esffile, &esf, &error);
		bool esffile_open = false;
		if (status == MB_SUCCESS && esf.esffp != nullptr)
			esffile_open = true;
		if (status == MB_FAILURE && error == MB_ERROR_OPEN_FAIL) {
			esffile_open = false;
			fprintf(stderr, "\nUnable to open new edit save file %s\n", esf.esffile);
		}
		// TODO(schwehr): What about status == MB_FAILURE && error != MB_ERROR_OPEN_FAIL?

		/* loop over all of the soundings */
		for (int j = 0; j < files[i].nsndg; j++) {
			sndg = &(files[i].sndg[j]);
			if (sndg->sndg_beamflag != sndg->sndg_beamflag_org) {
				int action = 0;
				if (mb_beam_ok(sndg->sndg_beamflag)) {
					action = MBP_EDIT_UNFLAG;
				}
				else if (mb_beam_check_flag_manual(sndg->sndg_beamflag)) {
					action = MBP_EDIT_FLAG;
				}
				else if (mb_beam_check_flag_filter(sndg->sndg_beamflag)) {
					action = MBP_EDIT_FILTER;
				}
				mb_esf_save(verbose, &esf, files[i].ping_time_d[sndg->sndg_ping],
				            sndg->sndg_beam + files[i].pingmultiplicity[sndg->sndg_ping] * MB_ESF_MULTIPLICITY_FACTOR, action,
				            &error);
			}
		}

		/* close esf file */
		mb_esf_close(verbose, &esf, &error);

		/* update mbprocess parameter file */
		if (esffile_open) {
			/* update mbprocess parameter file */
			status &= mb_pr_update_format(verbose, files[i].filelist, true, files[i].file_format, &error);
			status &= mb_pr_update_edit(verbose, files[i].filelist, MBP_EDIT_ON, esffile, &error);
		}
	}

	/* give the total statistics */
	if (verbose >= 0) {
		fprintf(stderr, "\nMBareaclean Processing Totals:\n");
		fprintf(stderr, "-------------------------\n");
		fprintf(stderr, "%d total swath data files processed\n", files_tot);
		fprintf(stderr, "%d total pings processed\n", pings_tot);
		fprintf(stderr, "%d total soundings processed\n", beams_tot);
		fprintf(stderr, "-------------------------\n");
		for (int i = 0; i < nfile; i++) {
			fprintf(stderr, "%3d soundings:%7d flagged:%7d unflagged:%7d  file:%s\n", i, files[i].ngood + files[i].nflag,
			        files[i].nflagged, files[i].nunflagged, files[i].filelist);
		}
	}

	mb_freed(verbose, __FILE__, __LINE__, (void **)&bindepths, &error);
	for (int i = 0; i < nx * ny; i++)
		if (gsndg[i] != nullptr)
			mb_freed(verbose, __FILE__, __LINE__, (void **)&gsndg[i], &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&gsndg, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&gsndgnum, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&gsndgnum_alloc, &error);

	for (int i = 0; i < nfile; i++) {
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(files[nfile - 1].ping_time_d), &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(files[nfile - 1].pingmultiplicity), &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(files[nfile - 1].ping_altitude), &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&(files[nfile - 1].sndg), &error);
	}
	mb_freed(verbose, __FILE__, __LINE__, (void **)&files, &error);

	// status = MB_SUCCESS;

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
