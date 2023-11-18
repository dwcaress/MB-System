/*--------------------------------------------------------------------
 *    The MB-system:	mbrolltimelag.c	11/10/2005
 *
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
 * MBrolltimelag extracts the roll time series and the apparent bottom
 * slope (linear fit to unflagged soundings for each ping) time series
 * from swath data, and then calculates the cross correlation between
 * the roll and the slope minus roll for a specified set of time lags.
 * The suite of cross correlation calculations are made for each
 * successive npings pings (default = 100) in each swath file. The
 * results are output to files, and cross correlation plots are
 * generated.
 *
 * Author:	D. W. Caress
 * Date:	November 11, 2005
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

constexpr int MBRTL_ALLOC_CHUNK = 1000;

constexpr char program_name[] = "MBrolltimelag";
constexpr char help_message[] =
    "MBrolltimelag extracts the roll time series and the apparent\n"
    "bottom slope time series from swath data, and then calculates\n"
    "the cross correlation between the roll and the slope minus roll\n"
    "for a specified set of time lags.";
constexpr char usage_message[] =
    "mbrolltimelag -Iswathdata [-Fformat -Krollsource -Nnping -Ooutputname -Snavchannel -Tnlag/lagmin/lagmax -V -H ]";

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
	int verbose = 0;
	double rthreshold = 0.9;
	int format = 0;
	int kind = MB_DATA_DATA;
	int npings = 100;
	char outroot[MB_PATH_MAXLINE];
	bool outroot_defined = false;
	int navchannel = 1;
	int nlag = 41;
	double lagstart = -2.0;
	double lagend = 2.0;

	char swathdata[MB_PATH_MAXLINE];
	strcpy(swathdata, "datalist.mb-1");

	{
		bool errflg = false;
		int c;
		bool help = false;
		while ((c = getopt(argc, argv, "VvHhC:c:F:f:I:i:K:k:O:o:N:n:S:s:T:t:")) != -1)
			switch (c) {
			case 'H':
			case 'h':
				help = true;
				break;
			case 'V':
			case 'v':
				verbose++;
				break;
			case 'C':
			case 'c':
				sscanf(optarg, "%lf", &rthreshold);
				break;
			case 'F':
			case 'f':
				sscanf(optarg, "%d", &format);
				break;
			case 'I':
			case 'i':
				sscanf(optarg, "%1023s", swathdata);
				break;
			case 'K':
			case 'k':
				sscanf(optarg, "%d", &kind);
				break;
			case 'N':
			case 'n':
				sscanf(optarg, "%d", &npings);
				break;
			case 'O':
			case 'o':
				sscanf(optarg, "%1023s", outroot);
				outroot_defined = true;
				break;
			case 'S':
			case 's':
				sscanf(optarg, "%d", &navchannel);
				if (navchannel > 0)
					kind = MB_DATA_NONE;
				break;
			case 'T':
			case 't':
				sscanf(optarg, "%d/%lf/%lf", &nlag, &lagstart, &lagend);
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
			fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
			fprintf(stderr, "dbg2       help:            %d\n", help);
			fprintf(stderr, "dbg2       format:          %d\n", format);
			fprintf(stderr, "dbg2       rthreshold:      %f\n", rthreshold);
			fprintf(stderr, "dbg2       swathdata:       %s\n", swathdata);
			fprintf(stderr, "dbg2       npings:          %d\n", npings);
			fprintf(stderr, "dbg2       nlag:            %d\n", nlag);
			fprintf(stderr, "dbg2       lagstart:        %f\n", lagstart);
			fprintf(stderr, "dbg2       lagend:          %f\n", lagend);
			fprintf(stderr, "dbg2       navchannel:      %d\n", navchannel);
			fprintf(stderr, "dbg2       kind:            %d\n", kind);
		}

		if (help) {
			fprintf(stderr, "\n%s\n", help_message);
			fprintf(stderr, "\nusage: %s\n", usage_message);
			exit(MB_ERROR_NO_ERROR);
		}
	}

	int error = MB_ERROR_NO_ERROR;

	/* get format if required */
	{
		int formatguess = 0;
		char swathroot[MB_PATH_MAXLINE];
		mb_get_format(verbose, swathdata, swathroot, &formatguess, &error);
		if (format == 0)
			format = formatguess;
		if (!outroot_defined)
			strcpy(outroot, swathroot);
        }

	/* determine whether to read one file or a list of files */
	const bool read_datalist = format < 0;
	bool read_data = false;

	/* get time lag step */
	const double lagstep = (lagend - lagstart) / (nlag - 1);

	// TODO(schwehr): Why realloc?
	double *rr = nullptr;  // cross correlation parameters
	int status = mb_reallocd(verbose, __FILE__, __LINE__, nlag * sizeof(double), (void **)&rr, &error);

	int *timelaghistogram = nullptr;
	status &= mb_reallocd(verbose, __FILE__, __LINE__, nlag * sizeof(int), (void **)&timelaghistogram, &error);

	if (verbose > 0) {
		fprintf(stderr, "Program %s parameters:\n", program_name);
		fprintf(stderr, "  Input:                           %s\n", swathdata);
		fprintf(stderr, "  Format:                          %d\n", format);
		fprintf(stderr, "  Number of pings per estimate:    %d\n", npings);
		fprintf(stderr, "  Number of time lag calculations: %d\n", nlag);
		fprintf(stderr, "  Start time lag reported:         %f\n", lagstart);
		fprintf(stderr, "  End time lag reported:           %f\n", lagend);
		fprintf(stderr, "  Time lag step:                   %f\n", lagstep);
	}

	/* first get roll data from the entire swathdata (which can be a datalist ) */
	char cmdfile[5*MB_PATH_MAXLINE+200];
	if (kind > MB_DATA_NONE)
		snprintf(cmdfile, sizeof(cmdfile), "mbnavlist -I%s -F%d -K%d -OMR", swathdata, format, kind);
	else
		snprintf(cmdfile, sizeof(cmdfile), "mbnavlist -I%s -F%d -N%d -OMR", swathdata, format, navchannel);
	fprintf(stderr, "\nRunning %s...\n", cmdfile);

	int nroll = 0;
	int nroll_alloc = 0;
	double *roll_time_d = nullptr;
	double *roll_roll = nullptr;
	FILE *fp = popen(cmdfile, "r");
	double time_d;
	double roll;
  int nscan;
  while ((nscan = fscanf(fp, "%lf %lf", &time_d, &roll)) == 2) {
		if (nroll >= nroll_alloc) {
			nroll_alloc += MBRTL_ALLOC_CHUNK;
			status &= mb_reallocd(verbose, __FILE__, __LINE__, nroll_alloc * sizeof(double), (void **)&roll_time_d, &error);
			status &= mb_reallocd(verbose, __FILE__, __LINE__, nroll_alloc * sizeof(double), (void **)&roll_roll, &error);
		}
		if (nroll == 0 || time_d > roll_time_d[nroll - 1]) {
			roll_time_d[nroll] = time_d;
			roll_roll[nroll] = roll;
			nroll++;
		}
  }
	pclose(fp);
	fprintf(stderr, "%d roll data read from %s\n", nroll, swathdata);

	/* open total cross correlation file */
	char xcorfiletot[MB_PATH_MAXLINE+10];
	FILE *fpt = nullptr;
	if (read_datalist) {
		snprintf(xcorfiletot, sizeof(xcorfiletot), "%s_xcorr.txt", outroot);
		if ((fpt = fopen(xcorfiletot, "w")) == nullptr) {
			fprintf(stderr, "\nUnable to open cross correlation output: %s\n", xcorfiletot);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_ERROR_OPEN_FAIL);
		}
	}

	/* open time lag estimate file */
	char estimatefile[MB_PATH_MAXLINE+20];
	snprintf(estimatefile, sizeof(estimatefile), "%s_timelagest.txt", outroot);
	FILE *fpe = fopen(estimatefile, "w");
	if (fpe == nullptr) {
		fprintf(stderr, "\nUnable to open estimate output: %s\n", estimatefile);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(MB_ERROR_OPEN_FAIL);
	}

	/* open time lag histogram file */
	char histfile[MB_PATH_MAXLINE+20];
	snprintf(histfile, sizeof(histfile), "%s_timelaghist.txt", outroot);
	FILE *fph = fopen(histfile, "w");
	if (fph == nullptr) {
		fprintf(stderr, "\nUnable to open histogram output: %s\n", histfile);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(MB_ERROR_OPEN_FAIL);
	}

	/* open time lag model file */
	char modelfile[MB_PATH_MAXLINE+20];
	snprintf(modelfile, sizeof(modelfile), "%s_timelagmodel.txt", outroot);
	FILE *fpm = fopen(modelfile, "w");
	if (fpm == nullptr) {
		fprintf(stderr, "\nUnable to open time lag model output: %s\n", modelfile);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(MB_ERROR_OPEN_FAIL);
	}

	/* open file list */
	void *datalist;
	char swathfile[MB_PATH_MAXLINE];
	char dfile[MB_PATH_MAXLINE];
	if (read_datalist) {
		const int look_processed = MB_DATALIST_LOOK_UNSET;
		if (mb_datalist_open(verbose, &datalist, swathdata, look_processed, &error) != MB_SUCCESS) {
			fprintf(stderr, "\nUnable to open data list file: %s\n", swathdata);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_ERROR_OPEN_FAIL);
		}
		double file_weight;
		read_data = mb_datalist_read(verbose, datalist, swathfile, dfile, &format, &file_weight, &error) == MB_SUCCESS;
	} else {
		/* else copy single filename to be read */
		strcpy(swathfile, swathdata);
		read_data = true;
	}

	/* slope data */
	int nslopetot = 0;
	int nslope_alloc = 0;
	double *slope_time_d = nullptr;
	double *slope_slope = nullptr;
	double *slope_roll = nullptr;

	double slope;
	double timelag;
	double sumsloperoll;
	double sumslopesq;
	double sumrollsq;
	double slopeminusmean;
	double rollminusmean;
	double r;
	// double sum_x = 0.0;
	// double sum_y = 0.0;
	// double sum_xy = 0.0;
	// double sum_x2 = 0.0;
	// double sum_y2 = 0.0;

	int nrollmean;
	double rollmean;
	double slopemean;

	int nestimate = 0;
	int nmodel = 0;

	int nr;
	double rollint;

	int peakk = 0;
	double peakr = 0.0;
	double peaktimelag = 0.0;
	double maxr = 0.0;
	double maxtimelag = 0.0;

	/* loop over all files to be read */
	while (read_data) {
		nestimate = 0;
		int nslope = 0;
		double time_d_avg = 0.0;
		snprintf(cmdfile, sizeof(cmdfile), "mblist -I%s -F%d -OMAR", swathfile, format);
		fprintf(stderr, "\nRunning %s...\n", cmdfile);
		fp = popen(cmdfile, "r");
		while ((nscan = fscanf(fp, "%lf %lf %lf", &time_d, &slope, &roll)) == 3) {
			if (nslope >= nslope_alloc) {
				nslope_alloc += MBRTL_ALLOC_CHUNK;
				status &= mb_reallocd(verbose, __FILE__, __LINE__, nslope_alloc * sizeof(double), (void **)&slope_time_d, &error);
				status &= mb_reallocd(verbose, __FILE__, __LINE__, nslope_alloc * sizeof(double), (void **)&slope_slope, &error);
				status &= mb_reallocd(verbose, __FILE__, __LINE__, nslope_alloc * sizeof(double), (void **)&slope_roll, &error);
			}
			if (nslope == 0 || time_d > slope_time_d[nslope - 1]) {
				slope_time_d[nslope] = time_d;
				time_d_avg += time_d;
				slope_slope[nslope] = roll - slope;
				slope_roll[nslope] = roll;
				nslope++;
			}
		}
		pclose(fp);
		nslopetot += nslope;
		if (nslope > 0)
			time_d_avg /= nslope;
		fprintf(stderr, "%d slope data read from %s\n", nslope, swathfile);

		/* open time lag histogram file */
		char fhistfile[MB_PATH_MAXLINE+20];
		snprintf(fhistfile, sizeof(fhistfile), "%s_timelaghist.txt", swathfile);
		FILE *fpf = fopen(fhistfile, "w");
		if (fpf == nullptr) {
			fprintf(stderr, "\nUnable to open histogram output: %s\n", fhistfile);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_ERROR_OPEN_FAIL);
		}

		/* open cross correlation file */
		char xcorfile[MB_PATH_MAXLINE+20];
		snprintf(xcorfile, sizeof(xcorfile), "%s_xcorr.txt", swathfile);
		FILE *fpx = fopen(xcorfile, "w");
		if (fpx == nullptr) {
			fprintf(stderr, "\nUnable to open cross correlation output: %s\n", xcorfile);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_ERROR_OPEN_FAIL);
		}

		/* initialize time lag histogram */
		for (int k = 0; k < nlag; k++) {
			timelaghistogram[k] = 0;
		}

		/* now do cross correlation calculations */
		for (int i = 0; i < nslope / npings; i++) {
			/* get ping range in this chunk */
			const int j0 = i * npings;
			const int j1 = j0 + npings - 1;

			/* get mean slope in this chunk */
			slopemean = 0.0;
			for (int j = j0; j <= j1; j++) {
				slopemean += slope_slope[j];
			}
			slopemean /= npings;

			/* get mean roll in this chunk */
			rollmean = 0.0;
			nrollmean = 0;
			for (int j = 0; j < nroll; j++) {
				if ((roll_time_d[j] >= slope_time_d[j0] + lagstart) && (roll_time_d[j] <= slope_time_d[j1] + lagend)) {
					rollmean += roll_roll[j];
					nrollmean++;
				}
			}

			if (nrollmean > 0) {
				rollmean /= nrollmean;

				/* calculate cross correlation for the specified time lags */
				fprintf(fpx, ">\n");
				if (fpt != nullptr)
					fprintf(fpt, ">\n");
				for (int k = 0; k < nlag; k++) {
					timelag = lagstart + k * lagstep;
					sumsloperoll = 0.0;
					sumslopesq = 0.0;
					sumrollsq = 0.0;
					nr = 0;

					for (int j = j0; j <= j1; j++) {
						/* interpolate lagged roll value */
						bool found = false;
						time_d = slope_time_d[j] + timelag;
						for (int l = nr; l < nroll - 1 && !found; l++) {
							if (time_d >= roll_time_d[l] && time_d <= roll_time_d[l + 1]) {
								nr = l;
								found = true;
							}
						}
						if (!found && time_d < roll_time_d[0]) {
							rollint = roll_roll[0];
						}
						else if (!found && time_d > roll_time_d[nroll - 1]) {
							rollint = roll_roll[nroll - 1];
						}
						else {
							rollint = roll_roll[nr] + (roll_roll[nr + 1] - roll_roll[nr]) * (time_d - roll_time_d[nr]) /
							                              (roll_time_d[nr + 1] - roll_time_d[nr]);
						}

						/* add to sums */
						slopeminusmean = (slope_slope[j] - slopemean);
						rollminusmean = (rollint - rollmean);
						sumslopesq += slopeminusmean * slopeminusmean;
						sumrollsq += rollminusmean * rollminusmean;
						sumsloperoll += slopeminusmean * rollminusmean;
					}

					if (sumslopesq > 0.0 && sumrollsq > 0.0)
						r = sumsloperoll / sqrt(sumslopesq) / sqrt(sumrollsq);
					else
						r = 0.0;
					rr[k] = r;

					/* output results */
					fprintf(fpx, "%5.3f %5.3f \n", timelag, r);
					if (fpt != nullptr)
						fprintf(fpt, "%5.3f %5.3f \n", timelag, r);
				}

				/* get max and closest peak cross correlations */
				maxr = 0.0;
				peakr = 0.0;
				peaktimelag = 0.0;
				for (int k = 0; k < nlag; k++) {
					timelag = lagstart + k * lagstep;
					if (timelag >= lagstart && timelag <= lagend) {
						if (rr[k] > maxr) {
							maxr = rr[k];
							maxtimelag = timelag;
						}
						if (k == 0) {
							peakk = k;
							peakr = rr[k];
							peaktimelag = timelag;
						}
						else if (k < nlag - 1 && rr[k] > 0.0 && rr[k] > rr[k - 1] && rr[k] > rr[k + 1] &&
						         (peaktimelag == lagstart || rr[k] > peakr)) {
							peakk = k;
							peakr = rr[k];
							peaktimelag = timelag;
						}
						else if (k == nlag - 1 && peaktimelag == lagstart && rr[k] > peakr) {
							peakk = k;
							peakr = rr[k];
							peaktimelag = timelag;
						}
					}
				}
			}

			/* print out best correlated time lag estimates */
			if (peakr > rthreshold) {
				timelaghistogram[peakk]++;

				/* augment histogram */
				fprintf(fpe, "%10.3f %6.3f\n", slope_time_d[(j0 + j1) / 2], peaktimelag);
				fprintf(fpf, "%6.3f\n", peaktimelag);
				fprintf(fph, "%6.3f\n", peaktimelag);
				// sum_x += slope_time_d[(j0 + j1) / 2];
				// sum_y += peaktimelag;
				// sum_xy += slope_time_d[(j0 + j1) / 2] * peaktimelag;
				// sum_x2 += slope_time_d[(j0 + j1) / 2] * slope_time_d[(j0 + j1) / 2];
				// sum_y2 += peaktimelag * peaktimelag;
				nestimate++;
			}

			/* print out max and closest peak cross correlations */
			if (verbose > 0) {
				fprintf(stderr, "cross correlation pings %5d - %5d: max: %6.3f %5.3f  peak: %6.3f %5.3f\n", j0, j1, maxtimelag,
				        maxr, peaktimelag, peakr);
			}
		}

		/* close cross correlation and histogram files */
		fclose(fpx);
		fclose(fpf);

		/* generate plot shellscript for cross correlation file */
		snprintf(cmdfile, sizeof(cmdfile), "mbm_xyplot -I%s -N", xcorfile);
		fprintf(stderr, "Running: %s...\n", cmdfile);
		/* int shellstatus = */ system(cmdfile);

		/* generate plot shellscript for time lag histogram */
		snprintf(cmdfile, sizeof(cmdfile), "mbm_histplot -I%s -C%g -L\"Frequency Histogram of %s:Time Lag (sec):Frequency:\"", fhistfile, lagstep,
		        swathfile);
		fprintf(stderr, "Running: %s...\n", cmdfile);
		/* int shellstatus = */ system(cmdfile);

		/* output peak time lag */
		peakk = 0;
		int peakkmax = 0;
		int peakksum = 0;
		timelag = 0.0;
		for (int k = 0; k < nlag; k++) {
			if (timelaghistogram[k] > peakkmax) {
				peakkmax = timelaghistogram[k];
				peakk = k;
			}
			peakksum += timelaghistogram[k];
		}
		if (nslope > 0 && peakksum > 0 && peakkmax > 1 && peakkmax > peakksum / 5) {
			timelag = lagstart + peakk * lagstep;
			fprintf(fpm, "%f %f\n", time_d_avg, timelag);
			nmodel++;
			fprintf(stderr, "Time lag model point: %f %f | nslope:%d peakksum:%d peakkmax:%d\n", time_d_avg, timelag, nslope,
			        peakksum, peakkmax);
		}
		else {
			if (peakkmax > 0)
				timelag = lagstart + peakk * lagstep;
			fprintf(stderr, "Time lag model point: %f %f | nslope:%d peakksum:%d peakkmax:%d | REJECTED\n", time_d_avg, timelag,
			        nslope, peakksum, peakkmax);
		}

		/* figure out whether and what to read next */
		if (read_datalist) {
			double file_weight;
			read_data = mb_datalist_read(verbose, datalist, swathfile, dfile, &format, &file_weight, &error) == MB_SUCCESS;
		} else {
			read_data = false;
		}

		/* end loop over files in list */
	}
	if (read_datalist) {
		mb_datalist_close(verbose, &datalist, &error);
		fclose(fpt);
	}

	fclose(fpe);
	fclose(fph);
	fclose(fpm);

	/* generate plot shellscript for cross correlation file */
	if (read_datalist) {
		snprintf(cmdfile, sizeof(cmdfile), "mbm_xyplot -I%s -N -L\"Roll Correlation With Acrosstrack Slope:Time Lag (sec):Correlation:\"",
		        xcorfiletot);
		fprintf(stderr, "Running: %s...\n", cmdfile);
		/* int shellstatus = */ system(cmdfile);
	}

	/* generate plot shellscript for time lag histogram */
	snprintf(cmdfile, sizeof(cmdfile), "mbm_histplot -I%s -C%g -L\"Frequency Histogram of %s:Time Lag (sec):Frequency:\"", histfile, lagstep,
	        swathdata);
	fprintf(stderr, "Running: %s...\n", cmdfile);
	/* int shellstatus = */ system(cmdfile);

	/* generate plot shellscript for time lag model if it exists */
	if (nmodel > 1 || nestimate > 1) {
		// const double mmm = (nestimate * sum_xy - sum_x * sum_y) / (nestimate * sum_x2 - sum_x * sum_x);
		// const double bbb = (sum_y - mmm * sum_x) / nestimate; */

		snprintf(cmdfile, sizeof(cmdfile), "mbm_xyplot -I%s -ISc0.05:%s -I%s -ISc0.1:%s -L\"Time lag model of %s:Time (sec):Time Lag (sec):\"",
		        modelfile, estimatefile, modelfile, modelfile, swathdata);
		fprintf(stderr, "Running: %s...\n", cmdfile);
		/* shellstatus = */ system(cmdfile);
	}

	/* deallocate memory for data arrays */
	mb_freed(verbose, __FILE__, __LINE__, (void **)&slope_time_d, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&slope_slope, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&slope_roll, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&roll_time_d, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&roll_roll, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&rr, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&timelaghistogram, &error);

	/* check memory */
	if (verbose >= 4)
		status &= mb_memory_list(verbose, &error);

	/* give the statistics */
	if (verbose >= 1) {
		fprintf(stderr, "\n%d input roll records\n", nroll);
		fprintf(stderr, "%d input slope\n", nslopetot);
	}

	if (status == MB_FAILURE) {
		fprintf(stderr, "WARNING: status is MB_FAILURE\n");
	}

	exit(error);
}
/*--------------------------------------------------------------------*/
