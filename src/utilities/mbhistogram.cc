/*--------------------------------------------------------------------
 *    The MB-system:	mbhistogram.c	12/28/94
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
 * MBHISTOGRAM reads a swath sonar data file and generates a histogram
 * of the bathymetry,  amplitude,  or sidescan values. Alternatively,
 * mbhistogram can output a list of values which break up the
 * distribution into equal sized regions.
 * The results are dumped to stdout.
 *
 * Author:	D. W. Caress
 * Date:	December 28, 1994
 */

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_status.h"

typedef enum {
    MBHISTOGRAM_BATH = 0,
    MBHISTOGRAM_AMP = 1,
    MBHISTOGRAM_SS = 2,
} histogram_mode_t;

constexpr char program_name[] = "MBHISTOGRAM";
constexpr char help_message[] =
    "MBHISTOGRAM reads a swath sonar data file and generates a histogram\n"
    "\tof the bathymetry,  amplitude, or sidescan values. Alternatively,\n"
    "\tmbhistogram can output a list of values which break up the\n"
    "\tdistribution into equal sized regions.\n"
    "\tThe results are dumped to stdout.";
constexpr char usage_message[] =
    "mbhistogram [-Akind -Byr/mo/da/hr/mn/sc -Dmin/max -Eyr/mo/da/hr/mn/sc -Fformat -G -Ifile -Llonflip "
    "-Mnintervals -Nnbins -Ppings -Rw/e/s/n -Sspeed -V -H]";

/*--------------------------------------------------------------------*/

/* double qsnorm(p)
 * double	p;
 *
 * Function to invert the cumulative normal probability
 * function.  If z is a standardized normal random deviate,
 * and Q(z) = p is the cumulative Gaussian probability
 * function, then z = qsnorm(p).
 *
 * Note that 0.0 < p < 1.0.  Data values outside this range
 * will return +/- a large number (1.0e6).
 * To compute p from a sample of data to test for Normalcy,
 * sort the N samples into non-decreasing order, label them
 * i=[1, N], and then compute p = i/(N+1).
 *
 * Author:	Walter H. F. Smith
 * Date:	19 February, 1991-1995.
 *
 * Based on a Fortran subroutine by R. L. Parker.  I had been
 * using IMSL library routine DNORIN(DX) to do what qsnorm(p)
 * does, when I was at the Lamont-Doherty Geological Observatory
 * which had a site license for IMSL.  I now need to invert the
 * gaussian CDF without calling IMSL; hence, this routine.
 *
 */

double qsnorm(double p) {
	if (p <= 0.0) {
		return (-1.0e6);
	}
	if (p >= 1.0) {
		return (1.0e6);
	}
	if (p == 0.5) {
		return (0.0);
	}
	if (p > 0.5) {
		const double t = sqrt(-2.0 * log(1.0 - p));
		const double z = t - (2.515517 + t * (0.802853 + t * 0.010328)) / (1.0 + t * (1.432788 + t * (0.189269 + t * 0.001308)));
		return (z);
	}

	const double t = sqrt(-2.0 * log(p));
	const double z = t - (2.515517 + t * (0.802853 + t * 0.010328)) / (1.0 + t * (1.432788 + t * (0.189269 + t * 0.001308)));
	return (-z);
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

	char read_file[MB_PATH_MAXLINE] = "stdin";
	histogram_mode_t mode = MBHISTOGRAM_SS;
	double value_min = 0.0;
	double value_max = 128.0;
	bool gaussian = false;
	int nintervals = 0;
	int nbins = 0;
	FILE *output;

	{
		bool errflg = false;
		bool help = false;
		int c;
		while ((c = getopt(argc, argv, "A:a:B:b:D:d:E:e:F:f:GgHhI:i:L:l:M:m:N:n:P:p:R:r:S:s:T:t:Vv")) != -1)
		{
			switch (c) {
			case 'A':
			case 'a':
			{
				int tmp;
				sscanf(optarg, "%d", &tmp);
				// TODO(schwehr): Range check.
				mode = (histogram_mode_t)tmp;
				break;
			}
			case 'B':
			case 'b':
				sscanf(optarg, "%d/%d/%d/%d/%d/%d", &btime_i[0], &btime_i[1], &btime_i[2], &btime_i[3], &btime_i[4], &btime_i[5]);
				btime_i[6] = 0;
				break;
			case 'D':
			case 'd':
				sscanf(optarg, "%lf/%lf", &value_min, &value_max);
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
				gaussian = true;
				break;
			case 'H':
			case 'h':
				help = true;
				break;
			case 'I':
			case 'i':
				sscanf(optarg, "%1023s", read_file);
				break;
			case 'L':
			case 'l':
				sscanf(optarg, "%d", &lonflip);
				break;
			case 'M':
			case 'm':
				sscanf(optarg, "%d", &nintervals);
				break;
			case 'N':
			case 'n':
				sscanf(optarg, "%d", &nbins);
				break;
			case 'P':
			case 'p':
				sscanf(optarg, "%d", &pings);
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
			case 'V':
			case 'v':
				verbose++;
				break;
			case '?':
				errflg = true;
			}
		}

		if (verbose <= 1)
			output = stdout;
		else
			output = stderr;

		if (errflg) {
			fprintf(output, "usage: %s\n", usage_message);
			fprintf(output, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_ERROR_BAD_USAGE);
		}

		if (verbose == 1 || help) {
			fprintf(output, "\nProgram %s\n", program_name);
			fprintf(output, "MB-system Version %s\n", MB_VERSION);
		}

		if (help) {
			fprintf(output, "\n%s\n", help_message);
			fprintf(output, "\nusage: %s\n", usage_message);
			exit(MB_ERROR_NO_ERROR);
		}
	}

	int error = MB_ERROR_NO_ERROR;

	if (format == 0)
		mb_get_format(verbose, read_file, nullptr, &format, &error);

	/* figure out histogram dimensions */
	if (nintervals > 0 && nbins <= 0)
		nbins = 50 * nintervals;
	if (nbins <= 0)
		nbins = 16;

	if (verbose >= 2) {
		fprintf(output, "\ndbg2  Program <%s>\n", program_name);
		fprintf(output, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(output, "dbg2  Control Parameters:\n");
		fprintf(output, "dbg2       verbose:    %d\n", verbose);
		fprintf(output, "dbg2       format:     %d\n", format);
		fprintf(output, "dbg2       pings:      %d\n", pings);
		fprintf(output, "dbg2       lonflip:    %d\n", lonflip);
		fprintf(output, "dbg2       bounds[0]:  %f\n", bounds[0]);
		fprintf(output, "dbg2       bounds[1]:  %f\n", bounds[1]);
		fprintf(output, "dbg2       bounds[2]:  %f\n", bounds[2]);
		fprintf(output, "dbg2       bounds[3]:  %f\n", bounds[3]);
		fprintf(output, "dbg2       btime_i[0]: %d\n", btime_i[0]);
		fprintf(output, "dbg2       btime_i[1]: %d\n", btime_i[1]);
		fprintf(output, "dbg2       btime_i[2]: %d\n", btime_i[2]);
		fprintf(output, "dbg2       btime_i[3]: %d\n", btime_i[3]);
		fprintf(output, "dbg2       btime_i[4]: %d\n", btime_i[4]);
		fprintf(output, "dbg2       btime_i[5]: %d\n", btime_i[5]);
		fprintf(output, "dbg2       btime_i[6]: %d\n", btime_i[6]);
		fprintf(output, "dbg2       etime_i[0]: %d\n", etime_i[0]);
		fprintf(output, "dbg2       etime_i[1]: %d\n", etime_i[1]);
		fprintf(output, "dbg2       etime_i[2]: %d\n", etime_i[2]);
		fprintf(output, "dbg2       etime_i[3]: %d\n", etime_i[3]);
		fprintf(output, "dbg2       etime_i[4]: %d\n", etime_i[4]);
		fprintf(output, "dbg2       etime_i[5]: %d\n", etime_i[5]);
		fprintf(output, "dbg2       etime_i[6]: %d\n", etime_i[6]);
		fprintf(output, "dbg2       speedmin:   %f\n", speedmin);
		fprintf(output, "dbg2       timegap:    %f\n", timegap);
		fprintf(output, "dbg2       file:       %s\n", read_file);
		fprintf(output, "dbg2       mode:       %d\n", mode);
		fprintf(output, "dbg2       gaussian:   %d\n", gaussian);
		fprintf(output, "dbg2       nbins:      %d\n", nbins);
		fprintf(output, "dbg2       nintervals: %d\n", nintervals);
		fprintf(output, "dbg2       value_min:  %f\n", value_min);
		fprintf(output, "dbg2       value_max:  %f\n", value_max);
	}


	/* MBIO read control parameters */
	void *datalist;
	double file_weight;
	double btime_d;
	double etime_d;
	char file[MB_PATH_MAXLINE];
	char dfile[MB_PATH_MAXLINE];
	int beams_bath;
	int beams_amp;
	int pixels_ss;

	/* MBIO read values */
	void *mbio_ptr = nullptr;
	int kind;
	int time_i[7];
	double time_d;
	double navlon;
	double navlat;
	double speed;
	double heading;
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
	char comment[MB_COMMENT_MAXLINE];

	/* histogram variables */
	double dvalue_bin;
	double *histogram = nullptr;
	double *intervals = nullptr;
	double total;
	double target;
	double dinterval;
	double bin_fraction;
	int ibin;

	int nrectot = 0;
	int nvaluetot = 0;

	/* allocate memory for histogram arrays */
	if (error == MB_ERROR_NO_ERROR)
		status = mb_mallocd(verbose, __FILE__, __LINE__, nbins * sizeof(double), (void **)&histogram, &error);
	if (error == MB_ERROR_NO_ERROR)
		status = mb_mallocd(verbose, __FILE__, __LINE__, nintervals * sizeof(double), (void **)&intervals, &error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR) {
		char *message;
		mb_error(verbose, error, &message);
		fprintf(output, "\nMBIO Error allocating histogram arrays:\n%s\n", message);
		fprintf(output, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}

	/* output some information */
	if (verbose > 0) {
		fprintf(stderr, "\nNumber of data bins: %d\n", nbins);
		fprintf(stderr, "Minimum value:         %f\n", value_min);
		fprintf(stderr, "Maximum value:         %f\n", value_max);
		if (mode == MBHISTOGRAM_BATH)
			fprintf(stderr, "Working on bathymetry data...\n");
		else if (mode == MBHISTOGRAM_AMP)
			fprintf(stderr, "Working on beam amplitude data...\n");
		else
			fprintf(stderr, "Working on sidescan data...\n");
	}

	/* get size of bins */
	dvalue_bin = (value_max - value_min) / (nbins - 1);
	const double value_bin_min = value_min - 0.5 * dvalue_bin;
	// const double value_bin_max = value_max + 0.5 * dvalue_bin;

	/* initialize histogram */
	for (int i = 0; i < nbins; i++)
		histogram[i] = 0;

	/* determine whether to read one file or a list of files */
	const bool read_datalist = format < 0;
	bool read_data;

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

	double data_min = INFINITY;
	double data_max = -INFINITY;
	bool data_first = true;

	/* loop over all files to be read */
	while (read_data) {

		/* obtain format array location - format id will
		    be aliased to current id if old format id given */
		status = mb_format(verbose, &format, &error);

		/* initialize reading the swath sonar data file */
		if (mb_read_init(verbose, file, format, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, &mbio_ptr,
		                           &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
			char *message;
			mb_error(verbose, error, &message);
			fprintf(output, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
			fprintf(output, "\nMultibeam File <%s> not initialized for reading\n", file);
			fprintf(output, "\nProgram <%s> Terminated\n", program_name);
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
			fprintf(output, "\nMBIO Error allocating data arrays:\n%s\n", message);
			fprintf(output, "\nProgram <%s> Terminated\n", program_name);
			exit(error);
		}

		/* output information */
		if (error == MB_ERROR_NO_ERROR && verbose > 0) {
			fprintf(stderr, "\nprocessing file: %s %d\n", file, format);
		}

		/* initialize counting variables */
		int nrec = 0;
		int nvalue = 0;

		/* read and process data */
		while (error <= MB_ERROR_NO_ERROR) {

			/* read a ping of data */
			status = mb_get(verbose, mbio_ptr, &kind, &pings, time_i, &time_d, &navlon, &navlat, &speed, &heading, &distance,
			                &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp, bathacrosstrack,
			                bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);

			/* process the pings */
			if (error == MB_ERROR_NO_ERROR || error == MB_ERROR_TIME_GAP) {
				/* increment record counter */
				nrec++;

				/* do the bathymetry */
				if (mode == MBHISTOGRAM_BATH)
					for (int i = 0; i < beams_bath; i++) {
						if (mb_beam_ok(beamflag[i])) {
							nvalue++;
							const int j = (bath[i] - value_bin_min) / dvalue_bin;
							if (j >= 0 && j < nbins)
								histogram[j]++;
							if (data_first) {
								data_min = bath[i];
								data_max = bath[i];
								data_first = false;
							}
							else {
								data_min = std::min(bath[i], data_min);
								data_max = std::max(bath[i], data_max);
							}
						}
					}

				/* do the amplitude */
				if (mode == MBHISTOGRAM_AMP)
					for (int i = 0; i < beams_amp; i++) {
						if (mb_beam_ok(beamflag[i])) {
							nvalue++;
							const int j = (amp[i] - value_bin_min) / dvalue_bin;
							if (j >= 0 && j < nbins)
								histogram[j]++;
							if (data_first) {
								data_min = amp[i];
								data_max = amp[i];
								data_first = false;
							}
							else {
								data_min = std::min(amp[i], data_min);
								data_max = std::max(amp[i], data_max);
							}
						}
					}

				/* do the sidescan */
				if (mode == MBHISTOGRAM_SS)
					for (int i = 0; i < pixels_ss; i++) {
						if (ss[i] > MB_SIDESCAN_NULL) {
							nvalue++;
							const int j = (ss[i] - value_bin_min) / dvalue_bin;
							if (j >= 0 && j < nbins)
								histogram[j]++;
							if (data_first) {
								data_min = ss[i];
								data_max = ss[i];
								data_first = false;
							}
							else {
								data_min = std::min(ss[i], data_min);
								data_max = std::max(ss[i], data_max);
							}
						}
					}
			}
		}

		/* close the swath sonar data file */
		status &= mb_close(verbose, &mbio_ptr, &error);
		nrectot += nrec;
		nvaluetot += nvalue;

		/* output information */
		if (error == MB_ERROR_NO_ERROR && verbose > 0) {
			fprintf(stderr, "%d records processed\n%d data processed\n", nrec, nvalue);
		}

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

	/* output information */
	if (error == MB_ERROR_NO_ERROR && verbose > 0) {
		fprintf(stderr, "\n%d total records processed\n", nrectot);
		fprintf(stderr, "%d total data processed\n\n", nvaluetot);
	}

	/* recast histogram as gaussian */
	if (gaussian) {
		/* get total number of good values */
		total = 0.0;
		for (int i = 0; i < nbins; i++)
			total = total + histogram[i];

		/* recast histogram */
		double sum = 0.0;
		for (int i = 0; i < nbins; i++) {
			const double p = (histogram[i] / 2 + sum) / (total + 1);
			sum = sum + histogram[i];
			histogram[i] = qsnorm(p);
		}
	}

	/* calculate gaussian intervals if required */
	if (nintervals > 0 && gaussian) {
		/* get interval spacing */
		double target_min = -2.0;
		double target_max = 2.0;
		dinterval = (target_max - target_min) / (nintervals - 1);

		/* get intervals */
		intervals[0] = std::max(data_min, value_min);
		intervals[nintervals - 1] = std::min(data_max, value_max);
		ibin = 0;
		for (int j = 1; j < nintervals - 1; j++) {
			target = target_min + j * dinterval;
			while (ibin < nbins - 1 && histogram[ibin] < target)
				ibin++;
			if (ibin > 0)
				bin_fraction = 1.0 - (histogram[ibin] - target) / (histogram[ibin] - histogram[ibin - 1]);
			else
				bin_fraction = 0.0;
			intervals[j] = value_bin_min + dvalue_bin * ibin + bin_fraction * dvalue_bin;
		}
	}

	/* calculate linear intervals if required */
	else if (nintervals > 0) {
		/* get total number of good values */
		total = 0.0;
		for (int i = 0; i < nbins; i++)
			total = total + histogram[i];

		/* get interval spacing */
		dinterval = total / (nintervals - 1);

		/* get intervals */
		intervals[0] = value_bin_min;
		total = 0.0;
		ibin = -1;
		for (int j = 1; j < nintervals; j++) {
			target = j * dinterval;
			while (total < target && ibin < nbins - 1) {
				ibin++;
				total = total + histogram[ibin];
				if (total <= 0.0)
					intervals[0] = value_bin_min + dvalue_bin * ibin;
			}
			bin_fraction = 1.0 - (total - target) / histogram[ibin];
			intervals[j] = value_bin_min + dvalue_bin * ibin + bin_fraction * dvalue_bin;
		}
	}

	/* print out the results */
	if (nintervals <= 0 && gaussian) {
		for (int i = 0; i < nbins; i++) {
			fprintf(output, "%f %f\n", value_min + i * dvalue_bin, histogram[i]);
		}
	}
	else if (nintervals <= 0) {
		for (int i = 0; i < nbins; i++) {
			fprintf(output, "%f %d\n", value_min + i * dvalue_bin, (int)histogram[i]);
		}
	}
	else {
		for (int i = 0; i < nintervals; i++)
			fprintf(output, "%f\n", intervals[i]);
	}

	/* deallocate memory used for data arrays */
	mb_freed(verbose, __FILE__, __LINE__, (void **)&histogram, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&intervals, &error);

	/* check memory */
	if (verbose >= 4)
		status &= mb_memory_list(verbose, &error);

	if (verbose >= 2) {
		fprintf(output, "\ndbg2  Program <%s> completed\n", program_name);
		fprintf(output, "dbg2  Ending status:\n");
		fprintf(output, "dbg2       status:  %d\n", status);
	}

	fprintf(output, "\n");
	exit(error);
}
/*--------------------------------------------------------------------*/
