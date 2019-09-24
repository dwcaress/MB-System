/*--------------------------------------------------------------------
 *    The MB-system:	mbrollbias.c	5/16/93
 *
 *    Copyright (c) 1993-2019 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *    D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    and S. O'Hara (sohara@lamont.ldgo.columbia.edu)
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * MBROLLBIAS is an utility used to assess roll bias of swath
 * sonar systems using data from two swaths covering the same
 * seafloor in opposite directions. The program takes two input
 * files and calculates best fitting planes for each dataset.
 * The roll bias is calculated by solving for a common roll bias
 * factor which explains the difference between the seafloor
 * slopes observed on the two swaths.  This approach assumes that
 * pitch bias is not a factor; this assumption is most correct when
 * the heading of the two shiptracks are exactly opposite. The area is
 * divided into a number of rectangular regions and calculations are done
 * in each region containing a sufficient number of data from both
 * swaths.  A positive roll bias value means that the ship is rolled
 * to port so that apparent depths are anomalously shallow to port
 * and deep to starboard.
 *
 * Author:	D. W. Caress
 * Date:	May 16, 1993
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_status.h"

/* define minimum number of data to fit plane */
#define MINIMUM_NUMBER_DATA 100

/* structure definitions */
struct bath {
	double x;
	double y;
	double d;
	double h;
};
struct bathptr {
	struct bath *ptr;
};

static const char program_name[] = "MBROLLBIAS";
static const char help_message[] =
    "MBROLLBIAS is an utility used to assess roll bias of swath \nsonar systems using bathymetry data from two "
    "swaths covering the \nsame seafloor in opposite directions. The program takes two input  \nfiles and "
    "calculates best fitting planes for each dataset.   \nThe roll bias is calculated by solving for a common "
    "roll bias\nfactor which explains the difference between the seafloor\nslopes observed on the two swaths.  "
    "This approach assumes that \npitch bias is not a factor; this assumption is most correct when\nthe "
    "heading of the two shiptracks are exactly opposite. The area is\ndivided into a number of rectangular "
    "regions and calculations are done  \nin each region containing a sufficient number of data from both "
    "\nswaths.  A positive roll bias value means that the the vertical \nreference used by the swath system is "
    "biased to starboard, \ngiving rise to shallow bathymetry to port and deep bathymetry \nto starboard.";
static const char usage_message[] =
    "mbrollbias -Dxdim/ydim -Fformat1/format2 -Ifile1 -Jfile2 -Llonflip -Rw/e/s/n -V -H]";

/*--------------------------------------------------------------------*/
void gauss(double *a, double *vec, int n, int nstore, double test, int *ierror, int itriag) {

	/* subroutine gauss, by william menke */
	/* july 1978 (modified feb 1983, nov 85) */

	/* a subroutine to solve a system of n linear equations in n unknowns*/
	/* where n doesn't exceed 10 */
	/* gaussian reduction with partial pivoting is used */
	/*      a               (sent, destroyed)       n by n matrix           */
	/*      vec             (sent, overwritten)     n vector, replaced w/ solution*/
	/*      nstore          (sent)                  dimension of a  */
	/*      test            (sent)                  div by zero check number*/
	/*      ierror          (returned)              zero on no error*/
	/*      itriag          (sent)                  matrix triangularized only*/
	/*                                               on TRUE useful when solving*/
	/*                                               multiple systems with same a */
	static int isub[10], l1;
	int line[10], iet, ieb;
	int i = 0;
	int j, k, l, j2;
	double big, testa, b, sum;

	iet = 0; /* initial error flags, one for triagularization*/
	ieb = 0; /* one for backsolving */

	/* triangularize the matrix a*/
	/* replacing the zero elements of the triangularized matrix */
	/* with the coefficients needed to transform the vector vec */

	if (itriag) { /* triangularize matrix */

		for (j = 0; j < n; j++) { /*line is an array of flags*/
			line[j] = 0;
			/* elements of a are not moved during pivoting*/
			/* line=0 flags unused lines */
		} /*end for j*/

		for (j = 0; j < n - 1; j++) {
			/*  triangularize matrix by partial pivoting */
			big = 0.0; /* find biggest element in j-th column*/
			           /* of unused portion of matrix*/
			for (l1 = 0; l1 < n; l1++) {
				if (line[l1] == 0) {
					testa = (double)fabs((double)(*(a + l1 * nstore + j)));
					if (testa > big) {
						i = l1;
						big = testa;
					}          /*end if*/
				}              /*end if*/
			}                  /*end for l1*/
			if (big <= test) { /* test for div by 0 */
				iet = 1;
			} /*end if*/

			line[i] = 1; /* selected unused line becomes used line */
			isub[j] = i; /* isub points to j-th row of tri. matrix */

			sum = 1.0 / (*(a + i * nstore + j));
			/*reduce matrix towards triangle */
			for (k = 0; k < n; k++) {
				if (line[k] == 0) {
					b = (*(a + k * nstore + j)) * sum;
					for (l = j + 1; l < n; l++) {
						*(a + k * nstore + l) = (*(a + k * nstore + l)) - b * (*(a + i * nstore + l));
					} /*end for l*/
					*(a + k * nstore + j) = b;
				} /*end if*/
			}     /*end for k*/
		}         /*end for j*/

		for (j = 0; j < n; j++) {
			/*find last unused row and set its pointer*/
			/*  this row contains the apex of the triangle*/
			if (line[j] == 0) {
				l1 = j; /*apex of triangle*/
				isub[n - 1] = j;
				break;
			} /*end if*/
		}     /*end for j*/

	} /*end if itriag true*/

	/*start backsolving*/

	for (i = 0; i < n; i++) { /* invert pointers. line(i) now gives*/
		                      /* row no in triang matrix of i-th row*/
		                      /* of actual matrix */
		line[isub[i]] = i;
	} /*end for i*/

	for (j = 0; j < n - 1; j++) { /*transform the vector to match triang. matrix*/
		b = vec[isub[j]];
		for (k = 0; k < n; k++) {
			if (line[k] > j) { /* skip elements outside of triangle*/
				vec[k] = vec[k] - (*(a + k * nstore + j)) * b;
			} /*end if*/
		}     /*end for k*/
	}         /*end for j*/

	b = *(a + l1 * nstore + (n - 1)); /*apex of triangle*/
	if (((double)fabs((double)b)) <= test) {
		/*check for div by zero in backsolving*/
		ieb = 2;
	} /*end if*/
	vec[isub[n - 1]] = vec[isub[n - 1]] / b;

	for (j = n - 2; j >= 0; j--) { /* backsolve rest of triangle*/
		sum = vec[isub[j]];
		for (j2 = j + 1; j2 < n; j2++) {
			sum = sum - vec[isub[j2]] * (*(a + isub[j] * nstore + j2));
		} /*end for j2*/
		b = *(a + isub[j] * nstore + j);
		if (((double)fabs((double)b)) <= test) {
			/* test for div by 0 in backsolving */
			ieb = 2;
		}                       /*end if*/
		vec[isub[j]] = sum / b; /*solution returned in vec*/
	}                           /*end for j*/

	/*put the solution vector into the proper order*/

	for (i = 0; i < n; i++) {     /* reorder solution */
		for (k = i; k < n; k++) { /* search for i-th solution element */
			if (line[k] == i) {
				j = k;
				break;
			}       /*end if*/
		}           /*end for k*/
		b = vec[j]; /* swap solution and pointer elements*/
		vec[j] = vec[i];
		vec[i] = b;
		line[j] = line[i];
	} /*end for i*/

	*ierror = iet + ieb; /* set final error flag*/
}

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
	int errflg = 0;
	int c;
	int help = 0;
	int flag = 0;

	/* MBIO status variables */
	int status = MB_SUCCESS;
	int verbose = 0;
	int error = MB_ERROR_NO_ERROR;
	char *message;

	/* MBIO read control parameters */
	int format;
	int pings;
	int lonflip;
	double bounds[4];
	int btime_i[7];
	int etime_i[7];
	double btime_d;
	double etime_d;
	double speedmin;
	double timegap;
	int beams_bath;
	int beams_amp;
	int pixels_ss;
	void *mbio_ptr = NULL;

	/* mbrollbias control variables */
	int iformat;
	int jformat;
	char ifile[MB_PATH_MAXLINE];
	char jfile[MB_PATH_MAXLINE];
	int xdim, ydim;

	/* mbio read values */
	int rpings;
	int kind;
	int time_i[7];
	double time_d;
	double navlon;
	double navlat;
	double speed;
	double heading;
	double distance;
	double altitude;
	double sonardepth;
	char *beamflag = NULL;
	double *bath = NULL;
	double *bathlon = NULL;
	double *bathlat = NULL;
	double *amp = NULL;
	double *ss = NULL;
	double *sslon = NULL;
	double *sslat = NULL;
	char comment[MB_COMMENT_MAXLINE];

	/* grid variables */
	double deglontokm, deglattokm;
	double mtodeglon, mtodeglat;
	double dx, dy;
	int *icount = NULL;
	int *jcount = NULL;
	struct bathptr *idata = NULL;
	struct bathptr *jdata = NULL;
	struct bath *zone = NULL;
	int ndatafile;
	double iaa, ibb, icc, ihh;
	double jaa, jbb, jcc, jhh;
	double hx, hy, dd;
	double isine, icosine, jsine, jcosine;
	double roll_bias;

	/* matrix parameters */
	int nmatrix = 3;
	double matrix[3][3];
	double vector[3];
	double xx[3];

	/* output stream for basic stuff (stdout if verbose <= 1,
	    stderr if verbose > 1) */
	FILE *outfp;

	/* other variables */
	int j, k;
	int ii, jj, kk;
	int ib, ix, iy, indx;

	/* get current default values */
	status = mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);

	/* set default input and output */
	strcpy(ifile, "\0");
	strcpy(jfile, "\0");

	/* initialize some values */
	pings = 1;
	iformat = format;
	jformat = format;
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
	bounds[0] = 0.0;
	bounds[1] = 0.0;
	bounds[2] = 0.0;
	bounds[3] = 0.0;
	xdim = 5;
	ydim = 5;

	/* process argument list */
	while ((c = getopt(argc, argv, "VvHhL:l:R:r:F:f:I:i:J:j:D:d:")) != -1)
		switch (c) {
		case 'H':
		case 'h':
			help++;
			break;
		case 'V':
		case 'v':
			verbose++;
			break;
		case 'L':
		case 'l':
			sscanf(optarg, "%d", &lonflip);
			flag++;
			break;
		case 'R':
		case 'r':
			mb_get_bounds(optarg, bounds);
			flag++;
			break;
		case 'F':
		case 'f':
			sscanf(optarg, "%d/%d", &iformat, &jformat);
			flag++;
			break;
		case 'I':
		case 'i':
			sscanf(optarg, "%s", ifile);
			flag++;
			break;
		case 'J':
		case 'j':
			sscanf(optarg, "%s", jfile);
			flag++;
			break;
		case 'D':
		case 'd':
			sscanf(optarg, "%d/%d", &xdim, &ydim);
			flag++;
			break;
		case '?':
			errflg++;
		}

	/* set output stream */
	if (verbose <= 1)
		outfp = stdout;
	else
		outfp = stderr;

	/* if error flagged then print it and exit */
	if (errflg) {
		fprintf(outfp, "usage: %s\n", usage_message);
		fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
		error = MB_ERROR_BAD_USAGE;
		exit(error);
	}

	if (verbose == 1 || help) {
		fprintf(outfp, "\nProgram %s\n", program_name);
		fprintf(outfp, "MB-system Version %s\n", MB_VERSION);
	}

	if (verbose >= 2) {
		fprintf(outfp, "\ndbg2  Program <%s>\n", program_name);
		fprintf(outfp, "dbg2  MB-system Version %s\n", MB_VERSION);
		fprintf(outfp, "dbg2  Control Parameters:\n");
		fprintf(outfp, "dbg2       verbose:          %d\n", verbose);
		fprintf(outfp, "dbg2       help:             %d\n", help);
		fprintf(outfp, "dbg2       pings:            %d\n", pings);
		fprintf(outfp, "dbg2       lonflip:          %d\n", lonflip);
		fprintf(outfp, "dbg2       btime_i[0]:       %d\n", btime_i[0]);
		fprintf(outfp, "dbg2       btime_i[1]:       %d\n", btime_i[1]);
		fprintf(outfp, "dbg2       btime_i[2]:       %d\n", btime_i[2]);
		fprintf(outfp, "dbg2       btime_i[3]:       %d\n", btime_i[3]);
		fprintf(outfp, "dbg2       btime_i[4]:       %d\n", btime_i[4]);
		fprintf(outfp, "dbg2       btime_i[5]:       %d\n", btime_i[5]);
		fprintf(outfp, "dbg2       btime_i[6]:       %d\n", btime_i[6]);
		fprintf(outfp, "dbg2       etime_i[0]:       %d\n", etime_i[0]);
		fprintf(outfp, "dbg2       etime_i[1]:       %d\n", etime_i[1]);
		fprintf(outfp, "dbg2       etime_i[2]:       %d\n", etime_i[2]);
		fprintf(outfp, "dbg2       etime_i[3]:       %d\n", etime_i[3]);
		fprintf(outfp, "dbg2       etime_i[4]:       %d\n", etime_i[4]);
		fprintf(outfp, "dbg2       etime_i[5]:       %d\n", etime_i[5]);
		fprintf(outfp, "dbg2       etime_i[6]:       %d\n", etime_i[6]);
		fprintf(outfp, "dbg2       speedmin:         %f\n", speedmin);
		fprintf(outfp, "dbg2       timegap:          %f\n", timegap);
		fprintf(outfp, "dbg2       input file 1:     %s\n", ifile);
		fprintf(outfp, "dbg2       input file 2:     %s\n", jfile);
		fprintf(outfp, "dbg2       file 1 format:    %d\n", iformat);
		fprintf(outfp, "dbg2       file 2 format:    %d\n", jformat);
		fprintf(outfp, "dbg2       grid x dimension: %d\n", xdim);
		fprintf(outfp, "dbg2       grid y dimension: %d\n", ydim);
		fprintf(outfp, "dbg2       grid bounds[0]:   %f\n", bounds[0]);
		fprintf(outfp, "dbg2       grid bounds[1]:   %f\n", bounds[1]);
		fprintf(outfp, "dbg2       grid bounds[2]:   %f\n", bounds[2]);
		fprintf(outfp, "dbg2       grid bounds[3]:   %f\n", bounds[3]);
	}

	/* if help desired then print it and exit */
	if (help) {
		fprintf(outfp, "\n%s\n", help_message);
		fprintf(outfp, "\nusage: %s\n", usage_message);
		exit(error);
	}

	/* get format if required */
	if (format == 0)
		mb_get_format(verbose, ifile, NULL, &format, &error);

	/* if bounds not specified then quit */
	if (bounds[0] >= bounds[1] || bounds[2] >= bounds[3] || bounds[2] <= -90.0 || bounds[3] >= 90.0) {
		fprintf(outfp, "\nGrid bounds not properly specified:\n\t%f %f %f %f\n", bounds[0], bounds[1], bounds[2], bounds[3]);
		fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
		error = MB_ERROR_BAD_PARAMETER;
		exit(error);
	}

	/* calculate grid properties and other values */
	mb_coor_scale(verbose, 0.5 * (bounds[2] + bounds[3]), &mtodeglon, &mtodeglat);
	deglontokm = 0.001 / mtodeglon;
	deglattokm = 0.001 / mtodeglat;
	dx = (bounds[1] - bounds[0]) / (xdim);
	dy = (bounds[3] - bounds[2]) / (ydim);

	/* output info */
	if (verbose >= 0) {
		fprintf(outfp, "\nMBROLLBIAS Parameters:\n");
		fprintf(outfp, "Input file 1:     %s\n", ifile);
		fprintf(outfp, "Input file 2:     %s\n", jfile);
		fprintf(outfp, "Region grid bounds:\n");
		fprintf(outfp, "  Longitude: %9.4f %9.4f\n", bounds[0], bounds[1]);
		fprintf(outfp, "  Latitude:  %9.4f %9.4f\n", bounds[2], bounds[3]);
		fprintf(outfp, "Region grid dimensions: %d %d\n", xdim, ydim);
		fprintf(outfp, "Longitude interval: %f degrees or %f km\n", dx, dx * deglontokm);
		fprintf(outfp, "Latitude interval:  %f degrees or %f km\n", dy, dy * deglattokm);
		fprintf(outfp, "Longitude flipping:   %d\n", lonflip);
		fprintf(outfp, "\n");
	}

	/* allocate memory for counting arrays */
	status = mb_mallocd(verbose, __FILE__, __LINE__, xdim * ydim * sizeof(int), (void **)&icount, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, xdim * ydim * sizeof(int), (void **)&jcount, &error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR) {
		mb_error(verbose, error, &message);
		fprintf(outfp, "\nMBIO Error allocating data arrays:\n%s\n", message);
		fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}

	/* initialize arrays */
	for (int i = 0; i < xdim * ydim; i++) {
		icount[i] = 0;
		jcount[i] = 0;
	}

	/* count data in first swath file */

	/* initialize the first swath file */
	ndatafile = 0;
	if ((status = mb_read_init(verbose, ifile, iformat, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, &mbio_ptr,
	                           &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error)) != MB_SUCCESS) {
		mb_error(verbose, error, &message);
		fprintf(outfp, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
		fprintf(outfp, "\nMultibeam File <%s> not initialized for reading\n", ifile);
		fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}

	/* allocate memory for reading data arrays */
	status = mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(char), (void **)&beamflag, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double), (void **)&bath, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double), (void **)&bathlon, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double), (void **)&bathlat, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, beams_amp * sizeof(double), (void **)&amp, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, pixels_ss * sizeof(double), (void **)&ss, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, pixels_ss * sizeof(double), (void **)&sslon, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, pixels_ss * sizeof(double), (void **)&sslat, &error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR) {
		mb_error(verbose, error, &message);
		fprintf(outfp, "\nMBIO Error allocating data arrays:\n%s\n", message);
		fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}

	/* loop over reading */
	while (error <= MB_ERROR_NO_ERROR) {
		status = mb_read(verbose, mbio_ptr, &kind, &rpings, time_i, &time_d, &navlon, &navlat, &speed, &heading, &distance,
		                 &altitude, &sonardepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp, bathlon, bathlat, ss,
		                 sslon, sslat, comment, &error);

		/* time gaps are not a problem here */
		if (error == MB_ERROR_TIME_GAP) {
			error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}

		if (verbose >= 2) {
			fprintf(stderr, "\ndbg2  Ping read in program <%s>\n", program_name);
			fprintf(stderr, "dbg2       kind:           %d\n", kind);
			fprintf(stderr, "dbg2       beams_bath:     %d\n", beams_bath);
			fprintf(stderr, "dbg2       beams_amp:      %d\n", beams_amp);
			fprintf(stderr, "dbg2       pixels_ss:      %d\n", pixels_ss);
			fprintf(stderr, "dbg2       error:          %d\n", error);
			fprintf(stderr, "dbg2       status:         %d\n", status);
		}

		if (error == MB_ERROR_NO_ERROR) {
			for (ib = 0; ib < beams_bath; ib++)
				if (mb_beam_ok(beamflag[ib])) {
					ix = (bathlon[ib] - bounds[0]) / dx;
					iy = (bathlat[ib] - bounds[2]) / dy;
					if (ix >= 0 && ix < xdim && iy >= 0 && iy < ydim) {
						indx = ix + iy * xdim;
						icount[indx]++;
						ndatafile++;
					}
				}
		}
	}
	status = mb_close(verbose, &mbio_ptr, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&beamflag, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&bath, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&bathlon, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&bathlat, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&amp, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&ss, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&sslon, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&sslat, &error);
	status = MB_SUCCESS;
	error = MB_ERROR_NO_ERROR;
	if (verbose >= 2)
		fprintf(outfp, "\n");
	fprintf(outfp, "%d depth points counted in %s\n", ndatafile, ifile);

	/* count data in second swath file */

	/* initialize the second swath file */
	ndatafile = 0;
	if ((status = mb_read_init(verbose, jfile, jformat, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, &mbio_ptr,
	                           &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error)) != MB_SUCCESS) {
		mb_error(verbose, error, &message);
		fprintf(outfp, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
		fprintf(outfp, "\nMultibeam File <%s> not initialized for reading\n", jfile);
		fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}

	/* allocate memory for reading data arrays */
	status = mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(char), (void **)&beamflag, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double), (void **)&bath, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double), (void **)&bathlon, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double), (void **)&bathlat, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, beams_amp * sizeof(double), (void **)&amp, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, pixels_ss * sizeof(double), (void **)&ss, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, pixels_ss * sizeof(double), (void **)&sslon, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, pixels_ss * sizeof(double), (void **)&sslat, &error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR) {
		mb_error(verbose, error, &message);
		fprintf(outfp, "\nMBIO Error allocating data arrays:\n%s\n", message);
		fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}

	/* loop over reading */
	while (error <= MB_ERROR_NO_ERROR) {
		status = mb_read(verbose, mbio_ptr, &kind, &rpings, time_i, &time_d, &navlon, &navlat, &speed, &heading, &distance,
		                 &altitude, &sonardepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp, bathlon, bathlat, ss,
		                 sslon, sslat, comment, &error);

		/* time gaps are not a problem here */
		if (error == MB_ERROR_TIME_GAP) {
			error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}

		if (verbose >= 2) {
			fprintf(stderr, "\ndbg2  Ping read in program <%s>\n", program_name);
			fprintf(stderr, "dbg2       kind:           %d\n", kind);
			fprintf(stderr, "dbg2       beams_bath:     %d\n", beams_bath);
			fprintf(stderr, "dbg2       beams_amp:      %d\n", beams_amp);
			fprintf(stderr, "dbg2       pixels_ss:      %d\n", pixels_ss);
			fprintf(stderr, "dbg2       error:          %d\n", error);
			fprintf(stderr, "dbg2       status:         %d\n", status);
		}

		if (error == MB_ERROR_NO_ERROR) {
			for (ib = 0; ib < beams_bath; ib++)
				if (mb_beam_ok(beamflag[ib])) {
					ix = (bathlon[ib] - bounds[0]) / dx;
					iy = (bathlat[ib] - bounds[2]) / dy;
					if (ix >= 0 && ix < xdim && iy >= 0 && iy < ydim) {
						indx = ix + iy * xdim;
						jcount[indx]++;
						ndatafile++;
					}
				}
		}
	}
	status = mb_close(verbose, &mbio_ptr, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&beamflag, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&bath, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&bathlon, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&bathlat, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&amp, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&ss, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&sslon, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&sslat, &error);
	status = MB_SUCCESS;
	error = MB_ERROR_NO_ERROR;
	if (verbose >= 2)
		fprintf(outfp, "\n");
	fprintf(outfp, "%d depth points counted in %s\n", ndatafile, jfile);

	/* allocate space for data */
	status = mb_mallocd(verbose, __FILE__, __LINE__, xdim * ydim * sizeof(struct bathptr), (void **)&idata, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, xdim * ydim * sizeof(struct bathptr), (void **)&jdata, &error);
	for (int i = 0; i < xdim; i++)
		for (j = 0; j < ydim; j++) {
			k = i * ydim + j;
			idata[k].ptr = NULL;
			jdata[k].ptr = NULL;
			if (icount[k] > 0) {
				status =
				    mb_mallocd(verbose, __FILE__, __LINE__, icount[k] * sizeof(struct bath), (void **)&(idata[k].ptr), &error);
				icount[k] = 0;
			}
			if (jcount[k] > 0) {
				status =
				    mb_mallocd(verbose, __FILE__, __LINE__, jcount[k] * sizeof(struct bath), (void **)&(jdata[k].ptr), &error);
				jcount[k] = 0;
			}
		}

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR) {
		mb_error(verbose, error, &message);
		fprintf(outfp, "\nMBIO Error allocating data arrays:\n%s\n", message);
		fprintf(outfp, "Try using ping averaging to reduce the number of data.\n");
		fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}

	/* read data in first swath file */

	/* initialize the first swath file */
	ndatafile = 0;
	if ((status = mb_read_init(verbose, ifile, iformat, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, &mbio_ptr,
	                           &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error)) != MB_SUCCESS) {
		mb_error(verbose, error, &message);
		fprintf(outfp, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
		fprintf(outfp, "\nMultibeam File <%s> not initialized for reading\n", ifile);
		fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}

	/* allocate memory for reading data arrays */
	status = mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double), (void **)&beamflag, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double), (void **)&bath, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double), (void **)&bathlon, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double), (void **)&bathlat, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, beams_amp * sizeof(double), (void **)&amp, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, pixels_ss * sizeof(double), (void **)&ss, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, pixels_ss * sizeof(double), (void **)&sslon, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, pixels_ss * sizeof(double), (void **)&sslat, &error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR) {
		mb_error(verbose, error, &message);
		fprintf(outfp, "\nMBIO Error allocating data arrays:\n%s\n", message);
		fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}

	/* loop over reading */
	while (error <= MB_ERROR_NO_ERROR) {
		status = mb_read(verbose, mbio_ptr, &kind, &rpings, time_i, &time_d, &navlon, &navlat, &speed, &heading, &distance,
		                 &altitude, &sonardepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp, bathlon, bathlat, ss,
		                 sslon, sslat, comment, &error);

		/* time gaps are not a problem here */
		if (error == MB_ERROR_TIME_GAP) {
			error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}

		if (verbose >= 2) {
			fprintf(stderr, "\ndbg2  Ping read in program <%s>\n", program_name);
			fprintf(stderr, "dbg2       kind:           %d\n", kind);
			fprintf(stderr, "dbg2       beams_bath:     %d\n", beams_bath);
			fprintf(stderr, "dbg2       beams_amp:      %d\n", beams_amp);
			fprintf(stderr, "dbg2       pixels_ss:      %d\n", pixels_ss);
			fprintf(stderr, "dbg2       error:          %d\n", error);
			fprintf(stderr, "dbg2       status:         %d\n", status);
		}

		if (error == MB_ERROR_NO_ERROR) {
			for (ib = 0; ib < beams_bath; ib++)
				if (mb_beam_ok(beamflag[ib])) {
					ix = (bathlon[ib] - bounds[0]) / dx;
					iy = (bathlat[ib] - bounds[2]) / dy;
					if (ix >= 0 && ix < xdim && iy >= 0 && iy < ydim) {
						indx = ix + iy * xdim;
						zone = idata[indx].ptr;
						zone[icount[indx]].x = deglontokm * (bathlon[ib] - bounds[0]);
						zone[icount[indx]].y = deglattokm * (bathlat[ib] - bounds[2]);
						zone[icount[indx]].d = 0.001 * bath[ib];
						zone[icount[indx]].h = heading;
						icount[indx]++;
						ndatafile++;
					}
				}
		}
	}
	status = mb_close(verbose, &mbio_ptr, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&beamflag, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&bath, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&bathlon, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&bathlat, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&amp, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&ss, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&sslon, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&sslat, &error);
	status = MB_SUCCESS;
	error = MB_ERROR_NO_ERROR;
	if (verbose >= 2)
		fprintf(outfp, "\n");
	fprintf(outfp, "%d depth points read from %s\n", ndatafile, ifile);

	/* read data in second swath file */

	/* initialize the second swath file */
	ndatafile = 0;
	if ((status = mb_read_init(verbose, jfile, jformat, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, &mbio_ptr,
	                           &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error)) != MB_SUCCESS) {
		mb_error(verbose, error, &message);
		fprintf(outfp, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
		fprintf(outfp, "\nMultibeam File <%s> not initialized for reading\n", jfile);
		fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}

	/* allocate memory for reading data arrays */
	status = mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(char), (void **)&beamflag, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double), (void **)&bath, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double), (void **)&bathlon, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double), (void **)&bathlat, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, beams_amp * sizeof(double), (void **)&amp, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, pixels_ss * sizeof(double), (void **)&ss, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, pixels_ss * sizeof(double), (void **)&sslon, &error);
	status = mb_mallocd(verbose, __FILE__, __LINE__, pixels_ss * sizeof(double), (void **)&sslat, &error);

	/* if error initializing memory then quit */
	if (error != MB_ERROR_NO_ERROR) {
		mb_error(verbose, error, &message);
		fprintf(outfp, "\nMBIO Error allocating data arrays:\n%s\n", message);
		fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
		exit(error);
	}

	/* loop over reading */
	while (error <= MB_ERROR_NO_ERROR) {
		status = mb_read(verbose, mbio_ptr, &kind, &rpings, time_i, &time_d, &navlon, &navlat, &speed, &heading, &distance,
		                 &altitude, &sonardepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp, bathlon, bathlat, ss,
		                 sslon, sslat, comment, &error);

		/* time gaps are not a problem here */
		if (error == MB_ERROR_TIME_GAP) {
			error = MB_ERROR_NO_ERROR;
			status = MB_SUCCESS;
		}

		if (verbose >= 2) {
			fprintf(stderr, "\ndbg2  Ping read in program <%s>\n", program_name);
			fprintf(stderr, "dbg2       kind:           %d\n", kind);
			fprintf(stderr, "dbg2       beams_bath:     %d\n", beams_bath);
			fprintf(stderr, "dbg2       beams_amp:      %d\n", beams_amp);
			fprintf(stderr, "dbg2       pixels_ss:      %d\n", pixels_ss);
			fprintf(stderr, "dbg2       error:          %d\n", error);
			fprintf(stderr, "dbg2       status:         %d\n", status);
		}

		if (error == MB_ERROR_NO_ERROR) {
			for (ib = 0; ib < beams_bath; ib++)
				if (mb_beam_ok(beamflag[ib])) {
					ix = (bathlon[ib] - bounds[0]) / dx;
					iy = (bathlat[ib] - bounds[2]) / dy;
					if (ix >= 0 && ix < xdim && iy >= 0 && iy < ydim) {
						indx = ix + iy * xdim;
						zone = jdata[indx].ptr;
						zone[jcount[indx]].x = deglontokm * (bathlon[ib] - bounds[0]);
						zone[jcount[indx]].y = deglattokm * (bathlat[ib] - bounds[2]);
						zone[jcount[indx]].d = 0.001 * bath[ib];
						zone[jcount[indx]].h = heading;
						jcount[indx]++;
						ndatafile++;
					}
				}
		}
	}
	status = mb_close(verbose, &mbio_ptr, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&beamflag, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&bath, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&bathlon, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&bathlat, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&amp, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&ss, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&sslon, &error);
	mb_freed(verbose, __FILE__, __LINE__, (void **)&sslat, &error);
	status = MB_SUCCESS;
	error = MB_ERROR_NO_ERROR;
	if (verbose >= 2)
		fprintf(outfp, "\n");
	fprintf(outfp, "%d depth points read from %s\n", ndatafile, jfile);

	/* loop over regions */
	for (int i = 0; i < xdim; i++)
		for (j = 0; j < ydim; j++) {
			/* set index */
			indx = i + j * xdim;

			fprintf(outfp, "\nRegion %d (%d %d) bounds:\n", j + i * ydim, i, j);
			fprintf(outfp, "    Longitude: %9.4f %9.4f\n", bounds[0] + dx * i, bounds[0] + dx * (i + 1));
			fprintf(outfp, "    Latitude:  %9.4f %9.4f\n", bounds[2] + dy * j, bounds[2] + dy * (j + 1));

			/* get the best fitting planes */
			if (icount[indx] >= MINIMUM_NUMBER_DATA && jcount[indx] >= MINIMUM_NUMBER_DATA) {
				/* use data from first data file */
				zone = idata[indx].ptr;

				/* zero the arrays */
				ihh = 0.0;
				hx = 0.0;
				hy = 0.0;
				for (ii = 0; ii < nmatrix; ii++) {
					vector[ii] = 0.0;
					for (jj = 0; jj < nmatrix; jj++)
						matrix[ii][jj] = 0.0;
				}

				/* construct normal equations */
				for (kk = 0; kk < icount[indx]; kk++) {
					ihh += zone[kk].h;
					hx += sin(DTR * zone[kk].h);
					hy += cos(DTR * zone[kk].h);
					xx[0] = 1.0;
					xx[1] = zone[kk].x;
					xx[2] = zone[kk].y;
					for (ii = 0; ii < nmatrix; ii++) {
						vector[ii] += zone[kk].d * xx[ii];
						for (jj = 0; jj < nmatrix; jj++) {
							matrix[ii][jj] += xx[ii] * xx[jj];
						}
					}
				}

				/* solve the normal equations */
				gauss((double *)matrix, vector, nmatrix, nmatrix, 1.0e-08, &error, 1);

				/* get the solution */
				iaa = vector[0];
				ibb = vector[1];
				icc = vector[2];
				hx = hx / icount[indx];
				hy = hy / icount[indx];
				dd = sqrt(hx * hx + hy * hy);
				if (dd > 0.0)
					ihh = RTD * atan2((hx / dd), (hy / dd));
				else
					ihh = ihh / icount[indx];
				if (ihh > 360.0)
					ihh = ihh - 360.0;
				else if (ihh < 0.0)
					ihh = ihh + 360.0;

				/* use data from second data file */
				zone = jdata[indx].ptr;

				/* zero the arrays */
				jhh = 0.0;
				hx = 0.0;
				hy = 0.0;
				for (ii = 0; ii < nmatrix; ii++) {
					vector[ii] = 0.0;
					for (jj = 0; jj < nmatrix; jj++)
						matrix[ii][jj] = 0.0;
				}

				/* construct normal equations */
				for (kk = 0; kk < jcount[indx]; kk++) {
					jhh += zone[kk].h;
					hx += sin(DTR * zone[kk].h);
					hy += cos(DTR * zone[kk].h);
					xx[0] = 1.0;
					xx[1] = zone[kk].x;
					xx[2] = zone[kk].y;
					for (ii = 0; ii < nmatrix; ii++) {
						vector[ii] += zone[kk].d * xx[ii];
						for (jj = 0; jj < nmatrix; jj++) {
							matrix[ii][jj] += xx[ii] * xx[jj];
						}
					}
				}

				/* solve the normal equations */
				gauss((double *)matrix, vector, nmatrix, nmatrix, 1.0e-08, &error, 1);
				if (error != 0) {
					fprintf(outfp, "matrix inversion error: %d\n", error);
				}

				/* get the solution */
				jaa = vector[0];
				jbb = vector[1];
				jcc = vector[2];
				hx = hx / jcount[indx];
				hy = hy / jcount[indx];
				dd = sqrt(hx * hx + hy * hy);
				if (dd > 0.0)
					jhh = RTD * atan2((hx / dd), (hy / dd));
				else
					jhh = jhh / jcount[indx];
				if (jhh > 360.0)
					jhh = jhh - 360.0;
				else if (jhh < 0.0)
					jhh = jhh + 360.0;

				/* report results */
				fprintf(outfp, "First data file:    %s\n", ifile);
				fprintf(outfp, "    Number of data: %d\n", icount[indx]);
				fprintf(outfp, "    Mean heading:   %f\n", ihh);
				fprintf(outfp, "    Plane fit:      %f %f %f\n", iaa, ibb, icc);
				fprintf(outfp, "Second data file:   %s\n", jfile);
				fprintf(outfp, "    Number of data: %d\n", jcount[indx]);
				fprintf(outfp, "    Mean heading:   %f\n", jhh);
				fprintf(outfp, "    Plane fit:      %f %f %f\n", jaa, jbb, jcc);

				/* calculate roll bias */
				if (fabs(ihh - jhh) > 90.0) {
					isine = sin(DTR * ihh);
					icosine = cos(DTR * ihh);
					jsine = sin(DTR * jhh);
					jcosine = cos(DTR * jhh);
					if (fabs(jcosine - icosine) > 1.0) {
						roll_bias = -(ibb - jbb) / (jcosine - icosine);
					}
					else {
						roll_bias = -(icc - jcc) / (isine - jsine);
					}
					fprintf(outfp, "Roll bias:   %f (%f degrees)\n", roll_bias, atan(roll_bias) / DTR);
					fprintf(outfp, "Roll bias is positive to starboard, negative to port.\n");
					fprintf(outfp, "A positive roll bias means the vertical reference used by \n    the swath system is biased to "
					               "starboard, \n    giving rise to shallow bathymetry to port and \n    deep bathymetry to "
					               "starboard.\n");
				}
				else
					fprintf(outfp, "Track headings too similar to calculate roll bias!\n");
			}
			else
				fprintf(outfp, "Not enough data to proceed!\n");
		}

	/* deallocate space for data */
	for (int i = 0; i < xdim; i++)
		for (j = 0; j < ydim; j++) {
			k = i * ydim + j;
			if (icount[k] > 0) {
				status = mb_freed(verbose, __FILE__, __LINE__, (void **)&idata[k].ptr, &error);
			}
			if (jcount[k] > 0) {
				status = mb_freed(verbose, __FILE__, __LINE__, (void **)&jdata[k].ptr, &error);
			}
		}
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)&idata, &error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)&jdata, &error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)&icount, &error);
	status = mb_freed(verbose, __FILE__, __LINE__, (void **)&jcount, &error);

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
