/*--------------------------------------------------------------------
 *    The MB-system:	mbbackangle.c	1/6/95
 *
 *    Copyright (c) 1995-2023 by
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
 * MBBACKANGLE reads a swath sonar data file and generates a table
 * of the average amplitude or sidescan values as a function of
 * the grazing angle with the seafloor. If bathymetry is
 * not available,  the seafloor is assumed to be flat. The takeoff
 * angle for each beam or pixel arrival is projected to the seafloor;
 * no raytracing is done.
 * The results are dumped to stdout.
 *
 * Author:	D. W. Caress
 * Date:	January 6, 1995
 */

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <getopt.h>
#include <sys/stat.h>
#include <unistd.h>

#include "mb_aux.h"
#include "mb_define.h"
#include "mb_format.h"
#include "mb_process.h"
#include "mb_status.h"

/* mode defines */
typedef enum {
    MBBACKANGLE_AMP = 1,
    MBBACKANGLE_SS = 2,
} backangle_kind_t;
constexpr double MBBACKANGLE_INNERSWATHLIMIT = 15.0;
typedef enum {
    MBBACKANGLE_BEAMPATTERN_EMPIRICAL = 0,
    MBBACKANGLE_BEAMPATTERN_SIDESCAN = 1,
} beampattern_t;

/* define grid structure */
struct mbba_grid_struct {
	mb_path file;
	mb_path projectionname;
	int projection_mode;
	mb_path projection_id;
	float nodatavalue;
	int nxy;
	int n_columns;
	int n_rows;
	double min;
	double max;
	double xmin;
	double xmax;
	double ymin;
	double ymax;
	double dx;
	double dy;
	float *data;
};

constexpr char program_name[] = "mbbackangle";
constexpr char help_message[] =
    "MBbackangle reads a swath sonar data file and generates a set\n"
    "  of tables containing the average amplitude an/or sidescan values\n"
    "  as a function of the angle of interaction (grazing angle)\n"
    "  with the seafloor. Each table represents the symmetrical\n"
    "  average function for a user defined number of pings. The tables\n"
    "  are output to a \".aga\" and \".sga\" files that can be applied\n"
    "  by MBprocess.";
constexpr char usage_message[] =
    "mbbackangle -Ifile "
    "[-Akind -Bmode[/beamwidth/depression] -Fformat -Ggridmode/angle/min/max/n_columns/n_rows "
    "-Nnangles/angle_max -Ppings -Q -Rrefangle -Ttopogridfile -Zaltitude -V -H]";

/*--------------------------------------------------------------------*/
int output_table(int verbose, FILE *tfp, int ntable, int nping, double time_d, int nangles, double angle_max, double dangle,
                 bool symmetry, int *nmean, double *mean, double *sigma, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBBACKANGLE function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       tfp:             %p\n", (void *)tfp);
		fprintf(stderr, "dbg2       ntable:          %d\n", ntable);
		fprintf(stderr, "dbg2       nping:           %d\n", nping);
		fprintf(stderr, "dbg2       time_d:          %f\n", time_d);
		fprintf(stderr, "dbg2       nangles:         %d\n", nangles);
		fprintf(stderr, "dbg2       angle_max:       %f\n", angle_max);
		fprintf(stderr, "dbg2       dangle:          %f\n", dangle);
		fprintf(stderr, "dbg2       symmetry:        %d\n", symmetry);
		fprintf(stderr, "dbg2       mean and sigma:\n");
		for (int i = 0; i < nangles; i++)
			fprintf(stderr, "dbg2         %d %f %d %f %f\n", i, (i * dangle), nmean[i], mean[i], sigma[i]);
	}

	/* process sums and print out results */
	int time_i[7];
	mb_get_date(verbose, time_d, time_i);
	fprintf(tfp, "# table: %d\n", ntable);
	fprintf(tfp, "# nping: %d\n", nping);
	fprintf(tfp, "# time:  %4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d    %16.6f\n", time_i[0], time_i[1], time_i[2], time_i[3],
	        time_i[4], time_i[5], time_i[6], time_d);
	fprintf(tfp, "# nangles: %d\n", nangles);

	for (int i = 0; i < nangles; i++) {
		const double angle = -angle_max + i * dangle;
		int i0;
		int i1;
		if (fabs(angle) > MBBACKANGLE_INNERSWATHLIMIT) {
			i0 = std::max(i - 1, 0);
			i1 = std::min(i + 1, nangles - 1);
		}
		else {
			i0 = i;
			i1 = i;
		}
		double amean = 0.0;
		double sum = 0.0;
		double sumsq = 0.0;
		double sumn = 0.0;
		for (int ii = i0; ii <= i1; ii++) {
			sum += mean[ii];
			sumsq += sigma[ii];
			sumn += nmean[ii];
			if (symmetry) {
				const int jj = nangles - ii - 1;
				sum += mean[jj];
				sumsq += sigma[jj];
				sumn += nmean[jj];
			}
		}
		double asigma = 0.0;
		if (sumn > 0.0) {
			amean = sum / sumn;
			asigma = sqrt((sumsq / sumn) - amean * amean);
		}
		fprintf(tfp, "%7.4f %12.4f %12.4f\n", angle, amean, asigma);
	}
	fprintf(tfp, "#\n");
	fprintf(tfp, "#\n");

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBBACKANGLE function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:           %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:          %d\n", status);
	}

	return (status);
}
/*--------------------------------------------------------------------*/
int output_model(int verbose, FILE *tfp, double beamwidth, double depression, double ref_angle, int ntable, int nping,
                 double time_d, double altitude, int nangles, double angle_max, double dangle, bool symmetry, int *nmean,
                 double *mean, double *sigma, int *error) {
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBBACKANGLE function <%s> called\n", __func__);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
		fprintf(stderr, "dbg2       tfp:             %p\n", (void *)tfp);
		fprintf(stderr, "dbg2       beamwidth:       %f\n", beamwidth);
		fprintf(stderr, "dbg2       depression:      %f\n", depression);
		fprintf(stderr, "dbg2       ref_angle:       %f\n", ref_angle);
		fprintf(stderr, "dbg2       ntable:          %d\n", ntable);
		fprintf(stderr, "dbg2       nping:           %d\n", nping);
		fprintf(stderr, "dbg2       time_d:          %f\n", time_d);
		fprintf(stderr, "dbg2       altitude:        %f\n", altitude);
		fprintf(stderr, "dbg2       nangles:         %d\n", nangles);
		fprintf(stderr, "dbg2       angle_max:       %f\n", angle_max);
		fprintf(stderr, "dbg2       dangle:          %f\n", dangle);
		fprintf(stderr, "dbg2       symmetry:        %d\n", symmetry);
		fprintf(stderr, "dbg2       mean and sigma:\n");
		for (int i = 0; i < nangles; i++)
			fprintf(stderr, "dbg2         %d %f %d %f %f\n", i, (i * dangle), nmean[i], mean[i], sigma[i]);
	}

	/* get average amplitude at reference angle */
	const int iref = (angle_max - ref_angle) / dangle;
	const int i0 = std::max(iref - 1, 0);
	const int i1 = std::min(iref + 1, nangles - 1);
	double ref_amp = 0.0;
	double sum = 0.0;
	double sumsq = 0.0;
	double sumn = 0.0;
	for (int ii = i0; ii <= i1; ii++) {
		sum += mean[ii];
		sumsq += sigma[ii];
		sumn += nmean[ii];
		const int jj = nangles - ii - 1;
		sum += mean[jj];
		sumsq += sigma[jj];
		sumn += nmean[jj];
	}
	double asigma = 0.0;
	if (sumn > 0.0) {
		ref_amp = sum / sumn;
		asigma = sqrt((sumsq / sumn) - ref_amp * ref_amp);
	}

	/* get model that combines gaussian with 1/r
	    - gaussian must drop to 0.7 max at 0.5 * beamwidth
	    - model must equal ref_amp at ref_angle */
	double del = (90.0 - depression) - 0.5 * beamwidth;
	const double aa = -log(0.1) / (del * del);
	del = 90.0 - depression - ref_angle;
	double range = altitude / cos(DTR * ref_angle);
	const double factor = ref_amp * range * range / exp(-aa * del * del);

	/* process sums and print out results */
	int time_i[7];
	mb_get_date(verbose, time_d, time_i);
	fprintf(tfp, "# table: %d\n", ntable);
	fprintf(tfp, "# nping: %d\n", nping);
	fprintf(tfp, "# time:  %4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d    %16.6f\n", time_i[0], time_i[1], time_i[2], time_i[3],
	        time_i[4], time_i[5], time_i[6], time_d);
	fprintf(tfp, "# nangles: %d\n", nangles);
	for (int i = 0; i < nangles; i++) {
		const double angle = -angle_max + i * dangle;
		del = fabs(angle) - (90 - depression);
		range = altitude / cos(DTR * fabs(angle));
		const double amean = factor * exp(-aa * del * del) / (range * range);
		fprintf(tfp, "%7.4f %12.4f %12.4f\n", angle, amean, asigma);
	}
	fprintf(tfp, "#\n");
	fprintf(tfp, "#\n");

	const int status = MB_SUCCESS;

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBBACKANGLE function <%s> completed\n", __func__);
		fprintf(stderr, "dbg2  Return values:\n");
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
	/* reset pings and timegap */
	pings = 1;
	timegap = 10000000.0;

	backangle_kind_t ampkind;
	bool sidescan_on = false;
	bool amplitude_on = false;
	beampattern_t beammode = MBBACKANGLE_BEAMPATTERN_EMPIRICAL;
	double ssbeamwidth = 50.0;
	double ssdepression = 20.0;
	bool dump = false;
	bool symmetry = false;
	int corr_symmetry = MBP_SSCORR_ASYMMETRIC;
	int mode;
	double angle;
	double ampmin;
	double ampmax;
	bool gridamp = false;
	double gridampangle = 0.0;
	double gridampmin = 0.0;
	double gridampmax = 0.0;
	int gridampn_columns = 0;
	int gridampn_rows = 0;
	double gridampdx = 0.0;
	double gridampdy = 0.0;
	bool gridss = false;
	double gridssangle = 0.0;
	double gridssmin = 0.0;
	double gridssmax = 0.0;
	int gridssn_columns = 0;
	int gridssn_rows = 0;
	double gridssdx = 0.0;
	double gridssdy = 0.0;
	char read_file[MB_PATH_MAXLINE] = "datalist.mb-1";
	int nangles = 81;
	double angle_max = 80.0;
	int pings_avg = 50;
	bool corr_slope = false;
	double ref_angle = 30.0;
	/* topography parameters */
	struct mbba_grid_struct grid;
	memset(&grid, 0, sizeof(struct mbba_grid_struct));
	bool corr_topogrid = false;
	double altitude_default = 0.0;

	int error = MB_ERROR_NO_ERROR;
  char user[256], host[256], date[32];
  status = mb_user_host_date(verbose, user, host, date, &error);

	{
		bool errflg = false;
		int c;
		bool help = false;

		while ((c = getopt(argc, argv, "A:a:B:b:CcDdF:f:G:g:HhI:i:N:n:P:p:QqR:r:T:t:VvZ:z:")) != -1)
		{
			switch (c) {
			case 'A':
			case 'a':
			{
				int tmp;
				sscanf(optarg, "%d", &tmp);
				ampkind = (backangle_kind_t)tmp;
				if (ampkind == MBBACKANGLE_SS)
					sidescan_on = true;
				if (ampkind == MBBACKANGLE_AMP)
					amplitude_on = true;
				break;
			}
			case 'B':
			case 'b':
			{
				double d1;
				double d2;
				int tmp;
				const int n = sscanf(optarg, "%d/%lf/%lf", &tmp, &d1, &d2);
				beammode = (beampattern_t)tmp;  // TODO(schwehr): Range check
				if (beammode == MBBACKANGLE_BEAMPATTERN_SIDESCAN) {
					if (n >= 2)
						ssbeamwidth = d1;  // TODO(schwehr): Range check
					if (n >= 3)
						ssdepression = d2;  // TODO(schwehr): Range check
				}
				break;
			}
			case 'C':
			case 'c':
				symmetry = true;
				corr_symmetry = MBP_SSCORR_SYMMETRIC;
				break;
			case 'D':
			case 'd':
				dump = true;
				break;
			case 'F':
			case 'f':
				sscanf(optarg, "%d", &format);
				break;
			case 'G':
			case 'g':
			{
				int i;
				int j;
				int n = sscanf(optarg, "%d/%lf/%lf/%lf/%d/%d", &mode, &angle, &ampmin, &ampmax, &i, &j);
				if (n == 5) {
					/* n = */ sscanf(optarg, "%d/%lf/%lf/%d/%d", &mode, &angle, &ampmax, &i, &j);
					ampmin = 0.0;
					n = 6;
				}
				if (mode == MBBACKANGLE_AMP && n == 6) {
					gridamp = true;
					gridampangle = angle;
					gridampmin = ampmin;
					gridampmax = ampmax;
					gridampn_columns = i;
					gridampn_rows = j;
					gridampdx = 2.0 * gridampangle / (gridampn_columns - 1);
					gridampdy = (gridampmax - gridampmin) / (gridampn_rows - 1);
				}
				else if (mode == MBBACKANGLE_SS && n == 6) {
					gridss = true;
					gridssangle = angle;
					gridssmin = ampmin;
					gridssmax = ampmax;
					gridssn_columns = i;
					gridssn_rows = j;
					gridssdx = 2.0 * gridssangle / (gridssn_columns - 1);
					gridssdy = (gridssmax - gridssmin) / (gridssn_rows - 1);
				}
				break;
			}
			case 'H':
			case 'h':
				help = true;
				break;
			case 'I':
			case 'i':
				sscanf(optarg, "%1023s", read_file);  // MB_PATH_MAXLINE - 1
				break;
			case 'N':
			case 'n':
				sscanf(optarg, "%d/%lf", &nangles, &angle_max);
				break;
			case 'P':
			case 'p':
				sscanf(optarg, "%d", &pings_avg);
				break;
			case 'Q':
			case 'q':
				corr_slope = true;
				break;
			case 'R':
			case 'r':
				sscanf(optarg, "%lf", &ref_angle);
				break;
			case 'T':
			case 't':
				sscanf(optarg, "%1023s", grid.file);  // MB_PATH_MAXLINE - 1
				corr_topogrid = true;
				break;
			case 'V':
			case 'v':
				verbose++;
				break;
			case 'Z':
			case 'z':
				sscanf(optarg, "%lf", &altitude_default);
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

		if (help) {
			fprintf(stderr, "\n%s\n", help_message);
			fprintf(stderr, "\nusage: %s\n", usage_message);
			exit(MB_ERROR_NO_ERROR);
		}
	} // end command line arg parsing

	double btime_d;
	double etime_d;

	void *datalist;
	double file_weight;
	char swathfile[MB_PATH_MAXLINE];
	char dfile[MB_PATH_MAXLINE];
	char amptablefile[MB_PATH_MAXLINE];
	char sstablefile[MB_PATH_MAXLINE];
	FILE *atfp = nullptr;
	FILE *stfp = nullptr;
	int beams_bath;
	int beams_amp;
	int pixels_ss;

	/* ESF File read */
	struct mb_esf_struct esf;

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

	/* slope calculation variables */
	int nsmooth = 5;
	int ndepths;
	double *depths = nullptr;
	double *depthsmooth = nullptr;
	double *depthacrosstrack = nullptr;
	int nslopes;
	double *slopes = nullptr;
	double *slopeacrosstrack = nullptr;

	/* angle function variables */
	double dangle;
	double angle_start;
	int ntotavg = 0;
	int *nmeanamp = nullptr;
	double *meanamp = nullptr;
	double *sigmaamp = nullptr;
	int *nmeanss = nullptr;
	double *meanss = nullptr;
	double *sigmass = nullptr;
	int *nmeantotamp = nullptr;
	double *meantotamp = nullptr;
	double *sigmatotamp = nullptr;
	int *nmeantotss = nullptr;
	double *meantotss = nullptr;
	double *sigmatotss = nullptr;
	double time_d_totavg;
	double altitude_totavg;
	int amp_corr_type;
	int amp_corr_slope = MBP_AMPCORR_IGNORESLOPE;
	int ss_corr_slope = MBP_SSCORR_IGNORESLOPE;
	int ss_type;
	int ss_corr_type;

	/* amp vs angle grid variables */
	float *gridamphist = nullptr;
	float *gridsshist = nullptr;
	mb_path gridfile;
	const char *xlabel = "Grazing Angle (degrees)";
	const char *ylabel = "Amplitude";
	mb_path zlabel;
	mb_path title;
	mb_command plot_cmd;
	const char *projection = "GenericLinear";

	double mtodeglon, mtodeglat;
	double headingx, headingy;
	double r[3], rr;
	double v1[3], v2[3], v[3], vv;
	double slope;
	double bathy;
	double norm;
	int nrectot = 0;
	int namptot = 0;
	int nsstot = 0;
	int ntabletot = 0;
	int plot_status;

	int ix, jy, kgrid;
	int kgrid00, kgrid10, kgrid01, kgrid11;

	/* set mode if necessary */
	if (!amplitude_on && !sidescan_on) {
		amplitude_on = true;
		sidescan_on = true;
	}
	if (!corr_slope && !corr_topogrid) {
		amp_corr_slope = MBP_AMPCORR_IGNORESLOPE;
		ss_corr_slope = MBP_SSCORR_IGNORESLOPE;
	}
	else if (corr_slope && !corr_topogrid) {
		amp_corr_slope = MBP_AMPCORR_USESLOPE;
		ss_corr_slope = MBP_SSCORR_USESLOPE;
	}
	else if (!corr_slope && corr_topogrid) {
		amp_corr_slope = MBP_AMPCORR_USETOPO;
		ss_corr_slope = MBP_SSCORR_USETOPO;
	}
	else if (corr_slope && corr_topogrid) {
		amp_corr_slope = MBP_AMPCORR_USETOPOSLOPE;
		ss_corr_slope = MBP_SSCORR_USETOPOSLOPE;
	}

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
		fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(stderr, "dbg2  Control Parameters:\n");
		fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
		fprintf(stderr, "dbg2       format:       %d\n", format);
		fprintf(stderr, "dbg2       pings:        %d\n", pings);
		fprintf(stderr, "dbg2       lonflip:      %d\n", lonflip);
		fprintf(stderr, "dbg2       bounds[0]:    %f\n", bounds[0]);
		fprintf(stderr, "dbg2       bounds[1]:    %f\n", bounds[1]);
		fprintf(stderr, "dbg2       bounds[2]:    %f\n", bounds[2]);
		fprintf(stderr, "dbg2       bounds[3]:    %f\n", bounds[3]);
		fprintf(stderr, "dbg2       btime_i[0]:   %d\n", btime_i[0]);
		fprintf(stderr, "dbg2       btime_i[1]:   %d\n", btime_i[1]);
		fprintf(stderr, "dbg2       btime_i[2]:   %d\n", btime_i[2]);
		fprintf(stderr, "dbg2       btime_i[3]:   %d\n", btime_i[3]);
		fprintf(stderr, "dbg2       btime_i[4]:   %d\n", btime_i[4]);
		fprintf(stderr, "dbg2       btime_i[5]:   %d\n", btime_i[5]);
		fprintf(stderr, "dbg2       btime_i[6]:   %d\n", btime_i[6]);
		fprintf(stderr, "dbg2       etime_i[0]:   %d\n", etime_i[0]);
		fprintf(stderr, "dbg2       etime_i[1]:   %d\n", etime_i[1]);
		fprintf(stderr, "dbg2       etime_i[2]:   %d\n", etime_i[2]);
		fprintf(stderr, "dbg2       etime_i[3]:   %d\n", etime_i[3]);
		fprintf(stderr, "dbg2       etime_i[4]:   %d\n", etime_i[4]);
		fprintf(stderr, "dbg2       etime_i[5]:   %d\n", etime_i[5]);
		fprintf(stderr, "dbg2       etime_i[6]:   %d\n", etime_i[6]);
		fprintf(stderr, "dbg2       speedmin:     %f\n", speedmin);
		fprintf(stderr, "dbg2       timegap:      %f\n", timegap);
		fprintf(stderr, "dbg2       read_file:    %s\n", read_file);
		fprintf(stderr, "dbg2       dump:         %d\n", dump);
		fprintf(stderr, "dbg2       symmetry:     %d\n", symmetry);
		fprintf(stderr, "dbg2       amplitude_on: %d\n", amplitude_on);
		fprintf(stderr, "dbg2       sidescan_on:  %d\n", sidescan_on);
		fprintf(stderr, "dbg2       corr_slope:   %d\n", corr_slope);
		fprintf(stderr, "dbg2       corr_topogrid:%d\n", corr_topogrid);
		fprintf(stderr, "dbg2       grid.file:    %s\n", grid.file);
		fprintf(stderr, "dbg2       nangles:      %d\n", nangles);
		fprintf(stderr, "dbg2       angle_max:    %f\n", angle_max);
		fprintf(stderr, "dbg2       ref_angle:    %f\n", ref_angle);
		fprintf(stderr, "dbg2       beammode:     %d\n", beammode);
		fprintf(stderr, "dbg2       ssbeamwidth:  %f\n", ssbeamwidth);
		fprintf(stderr, "dbg2       ssdepression: %f\n", ssdepression);
		fprintf(stderr, "dbg2       pings_avg:    %d\n", pings_avg);
		fprintf(stderr, "dbg2       angle_max:    %f\n", angle_max);
		fprintf(stderr, "dbg2       altitude:     %f\n", altitude_default);
		fprintf(stderr, "dbg2       gridamp:      %d\n", gridamp);
		fprintf(stderr, "dbg2       gridampangle: %f\n", gridampangle);
		fprintf(stderr, "dbg2       gridampmin:   %f\n", gridampmin);
		fprintf(stderr, "dbg2       gridampmax:   %f\n", gridampmax);
		fprintf(stderr, "dbg2       gridampn_columns:    %d\n", gridampn_columns);
		fprintf(stderr, "dbg2       gridampn_rows:    %d\n", gridampn_rows);
		fprintf(stderr, "dbg2       gridampdx:    %f\n", gridampdx);
		fprintf(stderr, "dbg2       gridampdy:    %f\n", gridampdy);
		fprintf(stderr, "dbg2       gridss:       %d\n", gridss);
		fprintf(stderr, "dbg2       gridssangle:  %f\n", gridssangle);
		fprintf(stderr, "dbg2       gridssmin:    %f\n", gridssmin);
		fprintf(stderr, "dbg2       gridssmax:    %f\n", gridssmax);
		fprintf(stderr, "dbg2       gridssn_columns:     %d\n", gridssn_columns);
		fprintf(stderr, "dbg2       gridssn_rows:     %d\n", gridssn_rows);
		fprintf(stderr, "dbg2       gridssdx:     %f\n", gridssdx);
		fprintf(stderr, "dbg2       gridssdy:     %f\n", gridssdy);
	}

	/* allocate memory for angle arrays */
	if (amplitude_on) {
		// if (error == MB_ERROR_NO_ERROR)
		/* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nangles * sizeof(int), (void **)&nmeanamp, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nangles * sizeof(double), (void **)&meanamp, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nangles * sizeof(double), (void **)&sigmaamp, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nangles * sizeof(int), (void **)&nmeantotamp, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nangles * sizeof(double), (void **)&meantotamp, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nangles * sizeof(double), (void **)&sigmatotamp, &error);
	}
	if (sidescan_on) {
		if (error == MB_ERROR_NO_ERROR)
			/* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nangles * sizeof(int), (void **)&nmeanss, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nangles * sizeof(double), (void **)&meanss, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nangles * sizeof(double), (void **)&sigmass, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nangles * sizeof(int), (void **)&nmeantotss, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nangles * sizeof(double), (void **)&meantotss, &error);
		if (error == MB_ERROR_NO_ERROR)
			/* status = */ mb_mallocd(verbose, __FILE__, __LINE__, nangles * sizeof(double), (void **)&sigmatotss, &error);
	}

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR) {
		char *message;
		mb_error(verbose, error, &message);
		fprintf(stderr, "\nMBIO Error allocating angle arrays:\n%s\n", message);
		fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}

	/* check grid modes */
	if (gridamp && !amplitude_on)
		gridamp = false;
	if (gridss && !sidescan_on)
		gridss = false;

	/* output some information */
	if (verbose > 0) {
		fprintf(stderr, "\nPings to average:    %d\n", pings_avg);
		fprintf(stderr, "Number of angle bins: %d\n", nangles);
		fprintf(stderr, "Maximum angle:         %f\n", angle_max);
		fprintf(stderr, "Default altitude:      %f\n", altitude_default);
		if (amplitude_on)
			fprintf(stderr, "Working on beam amplitude data...\n");
		if (sidescan_on)
			fprintf(stderr, "Working on sidescan data...\n");
		if (beammode == MBBACKANGLE_BEAMPATTERN_EMPIRICAL)
			fprintf(stderr, "Generating empirical correction tables...\n");
		else if (beammode == MBBACKANGLE_BEAMPATTERN_SIDESCAN)
			fprintf(stderr, "Generating sidescan model correction tables...\n");
		if (corr_slope)
			fprintf(stderr, "Using seafloor slope in calculating correction tables...\n");
		else
			fprintf(stderr, "Using flat bottom assumption in calculating correction tables...\n");
		if (gridamp)
			fprintf(stderr, "Outputting gridded histograms of beam amplitude vs grazing angle...\n");
		if (gridss)
			fprintf(stderr, "Outputting gridded histograms of sidescan amplitude vs grazing angle...\n");
	}

	/* get size of bins */
	dangle = 2 * angle_max / (nangles - 1);
	angle_start = -angle_max - 0.5 * dangle;

	/* initialize histogram */
	if (amplitude_on)
		for (int i = 0; i < nangles; i++) {
			nmeanamp[i] = 0;
			meanamp[i] = 0.0;
			sigmaamp[i] = 0.0;
			nmeantotamp[i] = 0;
			meantotamp[i] = 0.0;
			sigmatotamp[i] = 0.0;
		}
	if (sidescan_on)
		for (int i = 0; i < nangles; i++) {
			nmeanss[i] = 0;
			meanss[i] = 0.0;
			sigmass[i] = 0.0;
			nmeantotss[i] = 0;
			meantotss[i] = 0.0;
			sigmatotss[i] = 0.0;
		}

	/* get topography grid if specified */
	if (corr_topogrid) {
		grid.data = nullptr;
		status = mb_read_gmt_grd(verbose, grid.file, &grid.projection_mode, grid.projection_id, &grid.nodatavalue, &grid.nxy,
		                         &grid.n_columns, &grid.n_rows, &grid.min, &grid.max, &grid.xmin, &grid.xmax, &grid.ymin, &grid.ymax,
		                         &grid.dx, &grid.dy, &grid.data, nullptr, nullptr, &error);
		if (status == MB_FAILURE) {
			fprintf(stderr, "\nUnable to read grd file: %s\n", grid.file);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			exit(MB_ERROR_OPEN_FAIL);
		}

		/* rationalize grid bounds and lonflip */
		if (lonflip == -1) {
			if (grid.xmax > 180.0) {
				grid.xmin -= 360.0;
				grid.xmax -= 360.0;
			}
		}
		else if (lonflip == 0) {
			if (grid.xmin > 180.0) {
				grid.xmin -= 360.0;
				grid.xmax -= 360.0;
			}
			else if (grid.xmax < -180.0) {
				grid.xmin += 360.0;
				grid.xmax += 360.0;
			}
		}
		else if (lonflip == 1) {
			if (grid.xmin < -180.0) {
				grid.xmin += 360.0;
				grid.xmax += 360.0;
			}
		}
		if (grid.xmax > 180.0) {
			lonflip = 1;
		}
		else if (grid.xmin < -180.0) {
			lonflip = -1;
		}
		else {
			lonflip = 0;
		}
	}

	/* initialize counting variables */
	ntotavg = 0;
	time_d_totavg = 0.0;
	altitude_totavg = 0.0;

	/* initialize grids */
	if (gridamp) {
		/* allocate memory for output grids */
		status = mb_mallocd(verbose, __FILE__, __LINE__, gridampn_columns * gridampn_rows * sizeof(float), (void **)&gridamphist, &error);

		/* if error initializing memory then quit */
		if (error != MB_ERROR_NO_ERROR) {
			char *message;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
		}
	}
	if (gridss) {
		/* allocate memory for output grids */
		status = mb_mallocd(verbose, __FILE__, __LINE__, gridssn_columns * gridssn_rows * sizeof(float), (void **)&gridsshist, &error);

		/* if error initializing memory then quit */
		if (error != MB_ERROR_NO_ERROR) {
			char *message;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
			fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
			mb_memory_clear(verbose, &error);
			exit(error);
		}
	}

	/* get format if required */
	if (format == 0)
		mb_get_format(verbose, read_file, nullptr, &format, &error);

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
		read_data = mb_datalist_read(verbose, datalist, swathfile, dfile, &format, &file_weight, &error) == MB_SUCCESS;
	} else {
		// else copy single filename to be read
		strcpy(swathfile, read_file);
		read_data = true;
	}

	/* loop over all files to be read */
	while (read_data) {

		bool ok_to_process = true;

		/* obtain format array location - format id will
		    be aliased to current id if old format id given */
		mb_format(verbose, &format, &error);

		/* check that the format supports amplitude or sidescan data */
		{
			int cformat = format;
			int csystem = 0;
			int cbeams_bath_max = 0;
			int cbeams_amp_max = 0;
			int cpixels_ss_max = 0;
			mb_path cformat_name;
			mb_path csystem_name;
			mb_path cformat_description;
			int cnumfile = 0;
			int cfiletype = 0;
			int cvariable_beams = 0;
			int ctraveltime = 0;
			int cbeam_flagging = 0;
			int cplatform_source = 0;
			int cnav_source = 0;
			int csensordepth_source = 0;
			int cheading_source = 0;
			int cattitude_source = 0;
			int csvp_source = 0;
			double cbeamwidth_xtrack = 0.0;
			double cbeamwidth_ltrack = 0.0;
			status = mb_format_info(verbose, &cformat, &csystem, &cbeams_bath_max, &cbeams_amp_max, &cpixels_ss_max,
	                   cformat_name, csystem_name, cformat_description, &cnumfile, &cfiletype,
	                   &cvariable_beams, &ctraveltime, &cbeam_flagging, &cplatform_source, &cnav_source,
	                   &csensordepth_source, &cheading_source, &cattitude_source, &csvp_source, &cbeamwidth_xtrack,
	                   &cbeamwidth_ltrack, &error);
			if (amplitude_on && cbeams_amp_max <= 0) {
				ok_to_process = false;
				fprintf(stderr, "Skipping swath file: %s because format %d does not include amplitude data\n", swathfile, format);
			}
			if (sidescan_on && cpixels_ss_max <= 0) {
				ok_to_process = false;
				fprintf(stderr, "Skipping swath file: %s because format %d does not include sidescan data\n", swathfile, format);
			}
		}

		if (ok_to_process) {

			/* output information */
			if (verbose > 0) {
				fprintf(stderr, "\nprocessing swath file: %s %d\n", swathfile, format);
			}

			/* initialize reading the swath sonar file */
			if (mb_read_init(verbose, swathfile, format, 1, lonflip, bounds, btime_i, etime_i, speedmin, timegap, &mbio_ptr,
			                           &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
				char *message;
				mb_error(verbose, error, &message);
				fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
				fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", swathfile);
				fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
				exit(error);
			}

			/* set correction modes according to format */
			if (format == MBF_SB2100RW || format == MBF_SB2100B1 || format == MBF_SB2100B2 || format == MBF_EDGJSTAR ||
			    format == MBF_EDGJSTR2 || format == MBF_RESON7KR || format == MBF_RESON7K3)
				ss_corr_type = MBP_SSCORR_DIVISION;
			else if (format == MBF_MBLDEOIH)
				ss_corr_type = MBP_SSCORR_UNKNOWN;
			else
				ss_corr_type = MBP_SSCORR_SUBTRACTION;
	    	if (format == MBF_3DWISSLR || format == MBF_3DWISSLP || format == MBF_RESON7K3)
				amp_corr_type = MBP_AMPCORR_DIVISION;
		  	else
				amp_corr_type = MBP_AMPCORR_SUBTRACTION;

			/* allocate memory for data arrays */
			if (error == MB_ERROR_NO_ERROR)
				status &= mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(char), (void **)&beamflag, &error);
			if (error == MB_ERROR_NO_ERROR)
				status &= mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double), (void **)&bath, &error);
			if (error == MB_ERROR_NO_ERROR)
				status &= mb_mallocd(verbose, __FILE__, __LINE__, beams_amp * sizeof(double), (void **)&amp, &error);
			if (error == MB_ERROR_NO_ERROR)
				status &= mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double), (void **)&bathacrosstrack, &error);
			if (error == MB_ERROR_NO_ERROR)
				status &= mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double), (void **)&bathalongtrack, &error);
			if (error == MB_ERROR_NO_ERROR)
				status &= mb_mallocd(verbose, __FILE__, __LINE__, pixels_ss * sizeof(double), (void **)&ss, &error);
			if (error == MB_ERROR_NO_ERROR)
				status &= mb_mallocd(verbose, __FILE__, __LINE__, pixels_ss * sizeof(double), (void **)&ssacrosstrack, &error);
			if (error == MB_ERROR_NO_ERROR)
				status &= mb_mallocd(verbose, __FILE__, __LINE__, pixels_ss * sizeof(double), (void **)&ssalongtrack, &error);
			if (error == MB_ERROR_NO_ERROR)
				status &= mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double), (void **)&depths, &error);
			if (error == MB_ERROR_NO_ERROR)
				status &= mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double), (void **)&depthsmooth, &error);
			if (error == MB_ERROR_NO_ERROR)
				status &= mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double), (void **)&depthacrosstrack, &error);
			if (error == MB_ERROR_NO_ERROR)
				status &= mb_mallocd(verbose, __FILE__, __LINE__, (beams_bath + 1) * sizeof(double), (void **)&slopes, &error);
			if (error == MB_ERROR_NO_ERROR)
				status &=
				    mb_mallocd(verbose, __FILE__, __LINE__, (beams_bath + 1) * sizeof(double), (void **)&slopeacrosstrack, &error);
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
			if (error == MB_ERROR_NO_ERROR)
				status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&depths, &error);
			if (error == MB_ERROR_NO_ERROR)
				status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&depthsmooth, &error);
			if (error == MB_ERROR_NO_ERROR)
				status &=
				    mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&depthacrosstrack, &error);
			if (error == MB_ERROR_NO_ERROR)
				status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&slopes, &error);
			if (error == MB_ERROR_NO_ERROR)
				status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, 2 * sizeof(double), (void **)&slopeacrosstrack,
				                           &error);
			if (error == MB_ERROR_NO_ERROR)
				status &= mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, 2 * sizeof(double), (void **)&bathalongtrack,
				                           &error);

			/* if error initializing memory then quit */
			if (error != MB_ERROR_NO_ERROR) {
				char *message;
				mb_error(verbose, error, &message);
				fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
				fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
				exit(error);
			}

	  		/* Deal with esf file if avialable */
	  		if (status == MB_SUCCESS) {
	  			mb_path esffile;
	  			mb_esf_load(verbose, program_name, swathfile, true, false, esffile, &esf, &error);
	      		error = MB_ERROR_NO_ERROR;
	  		}

			/* initialize grid arrays */
			if (error == MB_ERROR_NO_ERROR) {
				if (gridamp) {
					/* initialize the memory */
					for (int i = 0; i < gridampn_columns * gridampn_rows; i++) {
						gridamphist[i] = 0.0;
					}
				}
				if (gridss) {
					/* initialize the memory */
					for (int i = 0; i < gridssn_columns * gridssn_rows; i++) {
						gridsshist[i] = 0.0;
					}
				}
			}

			/* open output files */
			if (error == MB_ERROR_NO_ERROR && dump) {
				atfp = stdout;
				stfp = stdout;
			}
			else if (error == MB_ERROR_NO_ERROR) {
				if (amplitude_on) {
					strcpy(amptablefile, swathfile);
					strcat(amptablefile, ".aga");
					if ((atfp = fopen(amptablefile, "w")) == nullptr) {
						char *message;
						mb_error(verbose, error, &message);
						fprintf(stderr, "\nUnable to open output table file %s\n", amptablefile);
						fprintf(stderr, "Program %s aborted!\n", program_name);
						exit(MB_ERROR_OPEN_FAIL);
					}
				}
				if (sidescan_on) {
					strcpy(sstablefile, swathfile);
					strcat(sstablefile, ".sga");
					if ((stfp = fopen(sstablefile, "w")) == nullptr) {
						char *message;
						mb_error(verbose, error, &message);
						fprintf(stderr, "\nUnable to open output table file %s\n", sstablefile);
						fprintf(stderr, "Program %s aborted!\n", program_name);
						exit(MB_ERROR_OPEN_FAIL);
					}
				}
			}

			/* write to output file */
			if (error == MB_ERROR_NO_ERROR) {
	  		char user[256], host[256], date[32];
	  		status = mb_user_host_date(verbose, user, host, date, &error);

				/* set comments in table files */
				if (amplitude_on) {
					fprintf(atfp, "## Amplitude correction table files generated by program %s\n", program_name);
					fprintf(atfp, "## MB-system Version %s\n", MB_VERSION);
					fprintf(atfp, "## Table file format: 1.0.0\n");
					fprintf(atfp, "## Run by user <%s> on cpu <%s> at <%s>\n", user, host, date);
					fprintf(atfp, "## Input swath file:      %s\n", swathfile);
					fprintf(atfp, "## Input swath format:    %d\n", format);
					fprintf(atfp, "## Output table file:     %s\n", amptablefile);
					fprintf(atfp, "## Pings to average:      %d\n", pings_avg);
					fprintf(atfp, "## Number of angle bins:  %d\n", nangles);
					fprintf(atfp, "## Maximum angle:         %f\n", angle_max);
					fprintf(atfp, "## Default altitude:      %f\n", altitude_default);
					fprintf(atfp, "## Slope correction:      %d\n", amp_corr_slope);
					fprintf(atfp, "## Data type:             beam amplitude\n");
				}

				if (sidescan_on) {
					fprintf(stfp, "## Sidescan correction table files generated by program %s\n", program_name);
					fprintf(stfp, "## MB-system Version %s\n", MB_VERSION);
					fprintf(stfp, "## Table file format: 1.0.0\n");
					fprintf(stfp, "## Run by user <%s> on cpu <%s> at <%s>\n", user, host, date);
					fprintf(stfp, "## Input swath file:      %s\n", swathfile);
					fprintf(stfp, "## Input swath format:    %d\n", format);
					fprintf(stfp, "## Output table file:     %s\n", sstablefile);
					fprintf(stfp, "## Pings to average:      %d\n", pings_avg);
					fprintf(stfp, "## Number of angle bins:  %d\n", nangles);
					fprintf(stfp, "## Maximum angle:         %f\n", angle_max);
					fprintf(stfp, "## Default altitude:      %f\n", altitude_default);
					fprintf(stfp, "## Slope Correction:      %d\n", ss_corr_slope);
					fprintf(stfp, "## Data type:             sidescan\n");
				}
			}

			/* initialize counting variables */
			int nrec = 0;
			int namp = 0;
			int nss = 0;
			int navg = 0;
			int ntable = 0;
			double time_d_avg = 0.0;
			double altitude_avg = 0.0;

			/* read and process data */
			while (error <= MB_ERROR_NO_ERROR) {

				/* read a ping of data */
				status = mb_get(verbose, mbio_ptr, &kind, &pings, time_i, &time_d, &navlon, &navlat, &speed, &heading, &distance,
				                &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp, bathacrosstrack,
				                bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);

				/* Apply ESF Edits if available */
				if (esf.nedit > 0 && error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
					status = mb_esf_apply(verbose, &esf, time_d, 0, beams_bath, beamflag, &error);
				}

				if ((navg > 0 && (error == MB_ERROR_TIME_GAP || error == MB_ERROR_EOF)) || (navg >= pings_avg) ||
				    (navg == 0 && error == MB_ERROR_EOF)) {
					/* write out tables */
					time_d_avg /= navg;
					altitude_avg /= navg;
					if (beammode == MBBACKANGLE_BEAMPATTERN_EMPIRICAL) {
						if (amplitude_on) {
							output_table(verbose, atfp, ntable, navg, time_d_avg, nangles, angle_max, dangle, symmetry, nmeanamp,
							             meanamp, sigmaamp, &error);
					  }
						if (sidescan_on) {
							output_table(verbose, stfp, ntable, navg, time_d_avg, nangles, angle_max, dangle, symmetry, nmeanss,
							             meanss, sigmass, &error);
					  }
					}
					else if (beammode == MBBACKANGLE_BEAMPATTERN_SIDESCAN) {
						if (amplitude_on) {
							output_model(verbose, atfp, ssbeamwidth, ssdepression, ref_angle, ntable, navg, time_d_avg, altitude_avg,
							             nangles, angle_max, dangle, symmetry, nmeanamp, meanamp, sigmaamp, &error);
					  }
						if (sidescan_on) {
							output_model(verbose, stfp, ssbeamwidth, ssdepression, ref_angle, ntable, navg, time_d_avg, altitude_avg,
							             nangles, angle_max, dangle, symmetry, nmeanss, meanss, sigmass, &error);
					  }
					}
					ntable++;

					/* reinitialize arrays */
					navg = 0;
					time_d_avg = 0.0;
					altitude_avg = 0.0;
					if (amplitude_on) {
						for (int i = 0; i < nangles; i++) {
							nmeanamp[i] = 0;
							meanamp[i] = 0.0;
							sigmaamp[i] = 0.0;
						}
	        		}
					if (sidescan_on) {
						for (int i = 0; i < nangles; i++) {
							nmeanss[i] = 0;
							meanss[i] = 0.0;
							sigmass[i] = 0.0;
						}
	        		}
				}

				/* process the pings */
				if (error == MB_ERROR_NO_ERROR || error == MB_ERROR_TIME_GAP) {
					/* if needed, attempt to get sidescan correction type */
					if (ss_corr_type == MBP_SSCORR_UNKNOWN) {
						status = mb_sidescantype(verbose, mbio_ptr, nullptr, &ss_type, &error);
						if (status == MB_SUCCESS) {
							if (ss_type == MB_SIDESCAN_LINEAR) {
								ss_corr_type = MBP_SSCORR_DIVISION;
	            			}
							else {
								ss_corr_type = MBP_SSCORR_SUBTRACTION;
	            			}
						}
						else {
							status = MB_SUCCESS;
							error = MB_ERROR_NO_ERROR;
							ss_corr_type = MBP_SSCORR_SUBTRACTION;
						}
					}

					/* increment record counter */
					nrec++;
					navg++;
					ntotavg++;

					/* increment time */
					time_d_avg += time_d;
					altitude_avg += altitude;
					time_d_totavg += time_d;
					altitude_totavg += altitude;

					/* get the seafloor slopes */
					if (beams_bath > 0)
						mb_pr_set_bathyslope(verbose, nsmooth, beams_bath, beamflag, bath, bathacrosstrack, &ndepths, depths,
						                     depthacrosstrack, &nslopes, slopes, slopeacrosstrack, depthsmooth, &error);

					/* get distance scaling and heading vector */
					mb_coor_scale(verbose, navlat, &mtodeglon, &mtodeglat);
					headingx = sin(heading * DTR);
					headingy = cos(heading * DTR);

					/* do the amplitude */
					double altitude_use = 0.0;
					if (amplitude_on)
						for (int i = 0; i < beams_amp; i++) {
							if (mb_beam_ok(beamflag[i])) {
								namp++;
								if (corr_topogrid) {
									/* get position in grid */
									r[0] = headingy * bathacrosstrack[i] + headingx * bathalongtrack[i];
									r[1] = -headingx * bathacrosstrack[i] + headingy * bathalongtrack[i];
									ix = (navlon + r[0] * mtodeglon - grid.xmin + 0.5 * grid.dx) / grid.dx;
									jy = (navlat + r[1] * mtodeglat - grid.ymin + 0.5 * grid.dy) / grid.dy;
									kgrid = ix * grid.n_rows + jy;
									kgrid00 = (ix - 1) * grid.n_rows + jy - 1;
									kgrid01 = (ix - 1) * grid.n_rows + jy + 1;
									kgrid10 = (ix + 1) * grid.n_rows + jy - 1;
									kgrid11 = (ix + 1) * grid.n_rows + jy + 1;
									if (ix > 0 && ix < grid.n_columns - 1 && jy > 0 && jy < grid.n_rows - 1 &&
									    grid.data[kgrid] > grid.nodatavalue && grid.data[kgrid00] > grid.nodatavalue &&
									    grid.data[kgrid01] > grid.nodatavalue && grid.data[kgrid10] > grid.nodatavalue &&
									    grid.data[kgrid11] > grid.nodatavalue) {
										/* get look vector for data */
										bathy = -grid.data[kgrid];
										r[2] = grid.data[kgrid] + sensordepth;
										rr = -sqrt(r[0] * r[0] + r[1] * r[1] + r[2] * r[2]);
										r[0] /= rr;
										r[1] /= rr;
										r[2] /= rr;

										/* get normal vector to grid surface */
										if (corr_slope) {
											v1[0] = 2.0 * grid.dx / mtodeglon;
											v1[1] = 2.0 * grid.dy / mtodeglat;
											v1[2] = grid.data[kgrid11] - grid.data[kgrid00];
											v2[0] = -2.0 * grid.dx / mtodeglon;
											v2[1] = 2.0 * grid.dy / mtodeglat;
											v2[2] = grid.data[kgrid01] - grid.data[kgrid10];
											v[0] = v1[1] * v2[2] - v2[1] * v1[2];
											v[1] = v2[0] * v1[2] - v1[0] * v2[2];
											v[2] = v1[0] * v2[1] - v2[0] * v1[1];
											vv = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
											v[0] /= vv;
											v[1] /= vv;
											v[2] /= vv;
										}
										else {
											v[0] = 0.0;
											v[1] = 0.0;
											v[2] = 1.0;
										}

										/* angle between look vector and surface normal
										    is the acos(r dot v) */
										angle = RTD * acos(r[0] * v[0] + r[1] * v[1] + r[2] * v[2]);
										if (bathacrosstrack[i] < 0.0)
											angle = -angle;
									}
									else {
										if (ix >= 0 && ix < grid.n_columns && jy >= 0 && jy < grid.n_rows && grid.data[kgrid] > grid.nodatavalue)
											bathy = -grid.data[kgrid];
										else if (altitude > 0.0)
											bathy = altitude + sensordepth;
										else
											bathy = altitude_default + sensordepth;
										angle = RTD * atan(bathacrosstrack[i] / (bathy - sensordepth));
										slope = 0.0;
									}
								}
								else if (beams_bath == beams_amp) {
									status = mb_pr_get_bathyslope(verbose, ndepths, depths, depthacrosstrack, nslopes, slopes,
									                              slopeacrosstrack, bathacrosstrack[i], &bathy, &slope, &error);
									if (status != MB_SUCCESS) {
										if (altitude > 0.0)
											bathy = altitude + sensordepth;
										else
											bathy = altitude_default + sensordepth;
										slope = 0.0;
										status = MB_SUCCESS;
										error = MB_ERROR_NO_ERROR;
									}
									altitude_use = bathy - sensordepth;
									angle = RTD * atan(bathacrosstrack[i] / altitude_use);
									if (corr_slope)
										angle += RTD * atan(slope);
								}
								else {
									if (altitude > 0.0)
										bathy = altitude + sensordepth;
									else
										bathy = altitude_default + sensordepth;
									slope = 0.0;
									altitude_use = bathy - sensordepth;
									angle = RTD * atan(bathacrosstrack[i] / altitude_use);
								}
								if (bathy > 0.0) {
									/* load amplitude into table */
									const int j = (angle - angle_start) / dangle;
									if (j >= 0 && j < nangles) {
										meanamp[j] += amp[i];
										sigmaamp[j] += amp[i] * amp[i];
										nmeanamp[j]++;
										meantotamp[j] += amp[i];
										sigmatotamp[j] += amp[i] * amp[i];
										nmeantotamp[j]++;
									}

									/* load amplitude into grid */
									if (gridamp) {
										ix = (angle + gridampangle) / gridampdx;
										jy = (amp[i] - gridampmin) / gridampdy;
										if (ix >= 0 && ix < gridampn_columns && jy >= 0 && jy < gridampn_rows) {
											const int k = ix * gridampn_rows + jy;
											gridamphist[k] += 1.0;
										}
									}
								}

								if (verbose >= 5) {
									fprintf(stderr, "dbg5       %d %d: slope:%f altitude:%f xtrack:%f ang:%f\n", nrec, i, slope,
									        altitude_use, bathacrosstrack[i], angle);
								}
							}
						}

					/* do the sidescan */
					if (sidescan_on)
						for (int i = 0; i < pixels_ss; i++) {
							if (ss[i] > MB_SIDESCAN_NULL) {
								nss++;
								if (corr_topogrid) {
									/* get position in grid */
									r[0] = headingy * ssacrosstrack[i] + headingx * ssalongtrack[i];
									r[1] = -headingx * ssacrosstrack[i] + headingy * ssalongtrack[i];
									ix = (navlon + r[0] * mtodeglon - grid.xmin + 0.5 * grid.dx) / grid.dx;
									jy = (navlat + r[1] * mtodeglat - grid.ymin + 0.5 * grid.dy) / grid.dy;
									kgrid = ix * grid.n_rows + jy;
									kgrid00 = (ix - 1) * grid.n_rows + jy - 1;
									kgrid01 = (ix - 1) * grid.n_rows + jy + 1;
									kgrid10 = (ix + 1) * grid.n_rows + jy - 1;
									kgrid11 = (ix + 1) * grid.n_rows + jy + 1;
									if (ix > 0 && ix < grid.n_columns - 1 && jy > 0 && jy < grid.n_rows - 1 &&
									    grid.data[kgrid] > grid.nodatavalue && grid.data[kgrid00] > grid.nodatavalue &&
									    grid.data[kgrid01] > grid.nodatavalue && grid.data[kgrid10] > grid.nodatavalue &&
									    grid.data[kgrid11] > grid.nodatavalue) {
										/* get look vector for data */
										bathy = -grid.data[kgrid];
										r[2] = grid.data[kgrid] + sensordepth;
										rr = -sqrt(r[0] * r[0] + r[1] * r[1] + r[2] * r[2]);
										r[0] /= rr;
										r[1] /= rr;
										r[2] /= rr;

										/* get normal vector to grid surface */
										if (corr_slope) {
											v1[0] = 2.0 * grid.dx / mtodeglon;
											v1[1] = 2.0 * grid.dy / mtodeglat;
											v1[2] = grid.data[kgrid11] - grid.data[kgrid00];
											v2[0] = -2.0 * grid.dx / mtodeglon;
											v2[1] = 2.0 * grid.dy / mtodeglat;
											v2[2] = grid.data[kgrid01] - grid.data[kgrid10];
											v[0] = v1[1] * v2[2] - v2[1] * v1[2];
											v[1] = v2[0] * v1[2] - v1[0] * v2[2];
											v[2] = v1[0] * v2[1] - v2[0] * v1[1];
											vv = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
											v[0] /= vv;
											v[1] /= vv;
											v[2] /= vv;
										}
										else {
											v[0] = 0.0;
											v[1] = 0.0;
											v[2] = 1.0;
										}

										/* angle between look vector and surface normal
										    is the acos(r dot v) */
										angle = RTD * acos(r[0] * v[0] + r[1] * v[1] + r[2] * v[2]);
										if (ssacrosstrack[i] < 0.0)
											angle = -angle;
									}
									else {
										if (ix >= 0 && ix < grid.n_columns && jy >= 0 && jy < grid.n_rows && grid.data[kgrid] > grid.nodatavalue)
											bathy = -grid.data[kgrid];
										else if (altitude > 0.0)
											bathy = altitude + sensordepth;
										else
											bathy = altitude_default + sensordepth;
										angle = RTD * atan(ssacrosstrack[i] / (bathy - sensordepth));
										slope = 0.0;
									}
								}
								else if (beams_bath > 0) {
									status = mb_pr_get_bathyslope(verbose, ndepths, depths, depthacrosstrack, nslopes, slopes,
									                              slopeacrosstrack, ssacrosstrack[i], &bathy, &slope, &error);
									if (status != MB_SUCCESS || bathy <= 0.0) {
										if (altitude > 0.0)
											bathy = altitude + sensordepth;
										else
											bathy = altitude_default;
										slope = 0.0;
										status = MB_SUCCESS;
										error = MB_ERROR_NO_ERROR;
									}
									altitude_use = bathy - sensordepth;
									angle = RTD * atan(ssacrosstrack[i] / altitude_use);
									if (corr_slope)
										angle += RTD * atan(slope);
								}
								else {
									if (altitude > 0.0)
										bathy = altitude + sensordepth;
									else
										bathy = altitude_default;
									slope = 0.0;
									altitude_use = bathy - sensordepth;
									angle = RTD * atan(ssacrosstrack[i] / altitude_use);
								}
								if (bathy > 0.0) {
									/* load amplitude into table */
									const int j = (angle - angle_start) / dangle;
									if (j >= 0 && j < nangles) {
										meanss[j] += ss[i];
										sigmass[j] += ss[i] * ss[i];
										nmeanss[j]++;
										meantotss[j] += ss[i];
										sigmatotss[j] += ss[i] * ss[i];
										nmeantotss[j]++;
									}

									/* load amplitude into grid */
									if (gridss) {
										ix = (angle + gridssangle) / gridssdx;
										jy = (ss[i] - gridssmin) / gridssdy;
										if (ix >= 0 && ix < gridssn_columns && jy >= 0 && jy < gridssn_rows) {
											const int k = ix * gridssn_rows + jy;
											gridsshist[k] += 1.0;
										}
									}
								}

								if (verbose >= 5) {
									fprintf(stderr, "dbg5kkk       %d %d: slope:%f altitude:%f xtrack:%f ang:%f\n", nrec, i,
									        slope, altitude_use, ssacrosstrack[i], angle);
								}
							}
						}
				}
			}

			/* close the swath sonar file */
	    	status = MB_SUCCESS;
	    	error = MB_ERROR_NO_ERROR;
			status = mb_close(verbose, &mbio_ptr, &error);

			/* Close ESF file if avialable and open */
			if (esf.edit != nullptr || esf.esffp != nullptr)
				mb_esf_close(verbose, &esf, &error);

			if (!dump && amplitude_on)
				fclose(atfp);
			if (!dump && sidescan_on)
				fclose(stfp);
			ntabletot += ntable;
			nrectot += nrec;
			namptot += namp;
			nsstot += nss;

			/* output grids */
			if (gridamp) {
				/* normalize the grid */
				ampmax = 0.0;
				for (ix = 0; ix < gridampn_columns; ix++) {
					norm = 0.0;
					for (jy = 0; jy < gridampn_rows; jy++) {
						const int k = ix * gridampn_rows + jy;
						norm += gridamphist[k];
					}
					if (norm > 0.0) {
						norm *= 0.001;
						for (jy = 0; jy < gridampn_rows; jy++) {
							const int k = ix * gridampn_rows + jy;
							gridamphist[k] /= norm;
							ampmax = std::max(ampmax, static_cast<double>(gridamphist[k]));
						}
					}
				}

				/* set the strings */
				strcpy(gridfile, swathfile);
				strcat(gridfile, "_aga.grd");
				strcpy(zlabel, "Beam Amplitude PDF (X1000)");
				strcpy(title, "Beam Amplitude vs. Grazing Angle PDF");

				/* output the grid */
				mb_write_gmt_grd(verbose, gridfile, gridamphist, MB_DEFAULT_GRID_NODATA, gridampn_columns, gridampn_rows,
				                 (double)(-gridampangle), gridampangle, gridampmin, gridampmax, (double)0.0, ampmax, gridampdx,
				                 gridampdy, xlabel, ylabel, zlabel, title, projection, argc, argv, &error);

				/* run mbm_grdplot */
	      		memset(plot_cmd, 0, sizeof(plot_cmd));
				snprintf(plot_cmd, sizeof(plot_cmd), "mbm_grdplot -I%s -JX9/5 -G1 -MGQ100 -MXI%s -L\"File %s - %s:%s\"", gridfile, amptablefile,
				        gridfile, title, zlabel);
				if (verbose) {
					fprintf(stderr, "\nexecuting mbm_grdplot...\n%s\n", plot_cmd);
				}
				plot_status = system(plot_cmd);
				if (plot_status == -1) {
					fprintf(stderr, "\nError executing mbm_grdplot on grid file %s\n", gridfile);
				}
			}
			if (gridss) {
				/* normalize the grid */
				ampmax = 0.0;
				for (ix = 0; ix < gridssn_columns; ix++) {
					norm = 0.0;
					for (jy = 0; jy < gridssn_rows; jy++) {
						const int k = ix * gridssn_rows + jy;
						norm += gridsshist[k];
					}
					if (norm > 0.0) {
						norm *= 0.001;
						for (jy = 0; jy < gridssn_rows; jy++) {
							const int k = ix * gridssn_rows + jy;
							gridsshist[k] /= norm;
							ampmax = std::max(ampmax, static_cast<double>(gridsshist[k]));
						}
					}
				}

				/* set the strings */
				strcpy(gridfile, swathfile);
				strcat(gridfile, "_sga.grd");
				strcpy(zlabel, "Sidescan Amplitude PDF (X1000)");
				strcpy(title, "Sidescan Amplitude vs. Grazing Angle PDF");

				/* output the grid */
				mb_write_gmt_grd(verbose, gridfile, gridsshist, MB_DEFAULT_GRID_NODATA, gridssn_columns, gridssn_rows, (double)(-gridssangle),
				                 gridssangle, gridssmin, gridssmax, (double)0.0, ampmax, gridssdx, gridssdy, xlabel, ylabel, zlabel,
				                 title, projection, argc, argv, &error);

				/* run mbm_grdplot */
	      		memset(plot_cmd, 0, sizeof(plot_cmd));
				snprintf(plot_cmd, sizeof(plot_cmd), "mbm_grdplot -I%s -JX9/5 -G1 -MGQ100 -MXI%s -L\"File %s - %s:%s\"", gridfile, sstablefile,
				        gridfile, title, zlabel);
				if (verbose) {
					fprintf(stderr, "\nexecuting mbm_grdplot...\n%s\n", plot_cmd);
				}
				plot_status = system(plot_cmd);
				if (plot_status == -1) {
					fprintf(stderr, "\nError executing mbm_grdplot on grid file %s\n", gridfile);
				}
			}

			/* set amplitude correction in parameter file */
			if (amplitude_on) {
				status &= mb_pr_update_ampcorr(verbose, swathfile, true, amptablefile, amp_corr_type, corr_symmetry, ref_angle,
				                              amp_corr_slope, grid.file, &error);
	    	}

			/* set sidescan correction in parameter file */
			if (sidescan_on) {
				status &= mb_pr_update_sscorr(verbose, swathfile, true, sstablefile, ss_corr_type, corr_symmetry, ref_angle,
				                             ss_corr_slope, grid.file, &error);
	    	}

			/* output information */
			if (error == MB_ERROR_NO_ERROR && verbose > 0) {
				fprintf(stderr, "%d records processed\n", nrec);
				if (amplitude_on) {
					fprintf(stderr, "%d amplitude data processed\n", namp);
					fprintf(stderr, "%d tables written to %s\n", ntable, amptablefile);
				}
				if (sidescan_on) {
					fprintf(stderr, "%d sidescan data processed\n", nss);
					fprintf(stderr, "%d tables written to %s\n", ntable, sstablefile);
				}
			}
			
		}

		/* figure out whether and what to read next */
		if (read_datalist) {
			if ((status = mb_datalist_read(verbose, datalist, swathfile, dfile, &format, &file_weight, &error)) == MB_SUCCESS)
				read_data = true;
			else
				read_data = false;
		}
		else {
			read_data = false;
		}
    	status = MB_SUCCESS;
    	error = MB_ERROR_NO_ERROR;

		/* end loop over files in list */
	}
	if (read_datalist)
		mb_datalist_close(verbose, &datalist, &error);

	/* write out total tables */
	time_d_totavg /= ntotavg;
	altitude_totavg /= ntotavg;
	if (!dump && amplitude_on) {
		strcpy(amptablefile, read_file);
		strcat(amptablefile, "_tot.aga");
		if ((atfp = fopen(amptablefile, "w")) == nullptr) {
			error = MB_ERROR_OPEN_FAIL;
			char *message;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nUnable to open output table file %s\n", amptablefile);
			fprintf(stderr, "Program %s aborted!\n", program_name);
			exit(error);
		}
		fprintf(atfp, "## Amplitude correction table files generated by program %s\n", program_name);
		fprintf(atfp, "## MB-system Version %s\n", MB_VERSION);
		fprintf(atfp, "## Table file format: 1.0.0\n");
		fprintf(atfp, "## Run by user <%s> on cpu <%s> at <%s>\n", user, host, date);
		fprintf(atfp, "## Input file:            %s\n", read_file);
		fprintf(atfp, "## Output table file:     %s\n", amptablefile);
		fprintf(atfp, "## Pings to average:      %d\n", pings_avg);
		fprintf(atfp, "## Number of angle bins:  %d\n", nangles);
		fprintf(atfp, "## Maximum angle:         %f\n", angle_max);
		fprintf(atfp, "## Default altitude:      %f\n", altitude_default);
		fprintf(atfp, "## Slope correction:      %d\n", amp_corr_slope);
		fprintf(atfp, "## Data type:             beam amplitude\n");
		if (beammode == MBBACKANGLE_BEAMPATTERN_EMPIRICAL) {
			output_table(verbose, atfp, 0, ntotavg, time_d_totavg, nangles, angle_max, dangle, symmetry, nmeantotamp, meantotamp,
			             sigmatotamp, &error);
    }
		else if (beammode == MBBACKANGLE_BEAMPATTERN_SIDESCAN) {
			output_model(verbose, atfp, ssbeamwidth, ssdepression, ref_angle, 0, ntotavg, time_d_totavg, altitude_totavg, nangles,
			             angle_max, dangle, symmetry, nmeantotamp, meantotamp, sigmatotamp, &error);
    }
		fclose(atfp);
	}
	if (!dump && sidescan_on) {
		strcpy(sstablefile, read_file);
		strcat(sstablefile, "_tot.sga");
		if ((stfp = fopen(sstablefile, "w")) == nullptr) {
			error = MB_ERROR_OPEN_FAIL;
			char *message;
			mb_error(verbose, error, &message);
			fprintf(stderr, "\nUnable to open output table file %s\n", sstablefile);
			fprintf(stderr, "Program %s aborted!\n", program_name);
			exit(error);
		}
		fprintf(stfp, "## Sidescan correction table files generated by program %s\n", program_name);
		fprintf(stfp, "## MB-system Version %s\n", MB_VERSION);
		fprintf(stfp, "## Table file format: 1.0.0\n");
		fprintf(stfp, "## Run by user <%s> on cpu <%s> at <%s>\n", user, host, date);
		fprintf(stfp, "## Input file:            %s\n", read_file);
		fprintf(stfp, "## Output table file:     %s\n", sstablefile);
		fprintf(stfp, "## Pings to average:      %d\n", pings_avg);
		fprintf(stfp, "## Number of angle bins:  %d\n", nangles);
		fprintf(stfp, "## Maximum angle:         %f\n", angle_max);
		fprintf(stfp, "## Default altitude:      %f\n", altitude_default);
		fprintf(stfp, "## Slope Correction:      %d\n", ss_corr_slope);
		fprintf(stfp, "## Data type:             sidescan\n");
		if (beammode == MBBACKANGLE_BEAMPATTERN_EMPIRICAL) {
			output_table(verbose, stfp, 0, ntotavg, time_d_totavg, nangles, angle_max, dangle, symmetry, nmeantotss, meantotss,
			             sigmatotss, &error);
    }
		else if (beammode == MBBACKANGLE_BEAMPATTERN_SIDESCAN) {
			output_model(verbose, stfp, ssbeamwidth, ssdepression, ref_angle, 0, ntotavg, time_d_totavg, altitude_totavg, nangles,
			             angle_max, dangle, symmetry, nmeantotss, meantotss, sigmatotss, &error);
    }
		fclose(stfp);
	}

	/* output information */
	if (error == MB_ERROR_NO_ERROR && verbose > 0) {
		fprintf(stderr, "\n%d total records processed\n", nrectot);
		if (amplitude_on) {
			fprintf(stderr, "%d total amplitude data processed\n", namptot);
			fprintf(stderr, "%d total aga tables written\n", ntabletot);
		}
		if (sidescan_on) {
			fprintf(stderr, "%d total sidescan data processed\n", nsstot);
			fprintf(stderr, "%d total sga tables written\n", ntabletot);
		}
	}

	/* deallocate memory used for data arrays */
	if (amplitude_on) {
		mb_freed(verbose, __FILE__, __LINE__, (void **)&nmeanamp, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&meanamp, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&sigmaamp, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&nmeantotamp, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&meantotamp, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&sigmatotamp, &error);
		if (gridamp) {
			mb_freed(verbose, __FILE__, __LINE__, (void **)&gridamphist, &error);
		}
	}
	if (sidescan_on) {
		mb_freed(verbose, __FILE__, __LINE__, (void **)&nmeanss, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&meanss, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&sigmass, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&nmeantotss, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&meantotss, &error);
		mb_freed(verbose, __FILE__, __LINE__, (void **)&sigmatotss, &error);
		if (gridss) {
			mb_freed(verbose, __FILE__, __LINE__, (void **)&gridsshist, &error);
		}
	}
	if (grid.data != nullptr) {
		mb_freed(verbose, __FILE__, __LINE__, (void **)&grid.data, &error);
	}

	// status = MB_SUCCESS;

	/* check memory */
	if (verbose >= 4)
		status = mb_memory_list(verbose, &error);

	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  Program <%s> completed\n", program_name);
		fprintf(stderr, "dbg2  Ending status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	if (verbose > 0)
		fprintf(stderr, "\n");

	exit(error);
}
/*--------------------------------------------------------------------*/
